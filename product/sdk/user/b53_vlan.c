/*
 * b53_vlan.c
 *
 *  Created on: May 3, 2019
 *      Author: zhurish
 */

#include "sdk_driver.h"
#include "b53_driver.h"
#include "b53_vlan.h"
#include "b53_mac.h"
/*
 * 基于端口的VLAN类似access port配置，在同一个VLAN的端口可以互通
 * 		路由器 的 LAN接口就是在端口VLAN下配置
 *
 * 基于8021Q的vlan 用于配置trunk接口
 */

static int b53125_port_vlan_enable(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable);


static int b53125_do_vlan_op(sdk_driver_t *dev, u8 op)
{
	int ret = 0;
	zpl_uint32 i;
	ret |= b53125_write8(dev->sdk_device, B53_ARLIO_PAGE, B53_VLAN_TBL_ACCESS, VLAN_TBL_START_CMD | op);
	for (i = 0; i < 10; i++) {
		u8 vta;
		ret |= b53125_read8(dev->sdk_device, B53_ARLIO_PAGE, B53_VLAN_TBL_ACCESS, &vta);
		if (!(vta & VLAN_TBL_START_CMD))
			return 0;
		usleep_range(100, 200);
	}
	_sdk_err( " b53125_do_vlan_op timeout");
	return ERROR;
}


static int b53125_enable_vlan(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 vc0 = 0, vc1 = 0, vc4 = 0, vc5 = 0;
	zpl_bool enable_filtering = enable;
	u8 mgmt = 0;
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_SWITCH_MODE, &mgmt);
	ret |= b53125_read8(dev->sdk_device, B53_VLAN_PAGE, B53_GLOBAL_8021Q, &vc0);
	ret |= b53125_read8(dev->sdk_device, B53_VLAN_PAGE, B53_GLOBAL_VLAN_CTRL1, &vc1);

	ret |= b53125_read8(dev->sdk_device, B53_VLAN_PAGE, B53_GLOBAL_VLAN_CTRL4, &vc4);
	ret |= b53125_read8(dev->sdk_device, B53_VLAN_PAGE, B53_GLOBAL_VLAN_CTRL5, &vc5);

	if (enable)
	{
		vc0 |= IEEE8021Q_VLAN_EN|VLAN_LEARNING_MODE(VLAN_LEARNING_MODE_IVL)|VLAN_CHANGE_PVID_EN;
		vc1 |= VLAN_MCST_UNTAG_CHECK_EN | VLAN_MCST_FWD_CHECK_EN|VLAN_RES_MCST_FWD_CHECK_EN|VLAN_RES_MCST_UNTAG_CHECK_EN;
		vc4 &= ~VLAN_ING_VID_CHECK_MASK;
		if (enable_filtering)
		{
			vc4 |= VLAN_ING_VID_VIO_DROP << VLAN_ING_VID_CHECK_S|VLAN_RSV_MCAST_FLOOD;
			vc5 |= VLAN_DROP_VID_INVALID;
		}
		else
		{
			vc4 |= VLAN_ING_VID_VIO_FWD << VLAN_ING_VID_CHECK_S;
			vc5 &= ~VLAN_DROP_VID_INVALID;
		}	
	}
	else
	{
		vc0 &= ~(IEEE8021Q_VLAN_EN|VLAN_LEARNING_MODE(VLAN_LEARNING_MODE_IVL)|VLAN_CHANGE_PVID_EN);
		vc1 &= ~(VLAN_MCST_UNTAG_CHECK_EN | VLAN_MCST_FWD_CHECK_EN|VLAN_RES_MCST_FWD_CHECK_EN|VLAN_RES_MCST_UNTAG_CHECK_EN);
		vc4 &= ~VLAN_ING_VID_CHECK_MASK;
		vc5 &= ~VLAN_DROP_VID_INVALID;

		vc4 |= VLAN_ING_VID_VIO_TO_IMP << VLAN_ING_VID_CHECK_S;
	}
	//mgmt &= ~SM_SW_FWD_MODE;
	
	vc5 &= ~VLAN_DROP_VID_FFF_EN;
	vc5 |= VLAN_DROP_VID_INVALID;
	ret |= b53125_write8(dev->sdk_device, B53_VLAN_PAGE, B53_GLOBAL_8021Q, vc0);
	ret |= b53125_write8(dev->sdk_device, B53_VLAN_PAGE, B53_GLOBAL_VLAN_CTRL1, vc1);

	ret |= b53125_write16(dev->sdk_device, B53_VLAN_PAGE, B53_GLOBAL_VLAN_CTRL3, 0);
	ret |= b53125_write8(dev->sdk_device, B53_VLAN_PAGE, B53_GLOBAL_VLAN_CTRL4, vc4);
	ret |= b53125_write8(dev->sdk_device, B53_VLAN_PAGE, B53_GLOBAL_VLAN_CTRL5, vc5);

	_sdk_debug( "%s %s vc0=%x", __func__, (ret == OK)?"OK":"ERROR", vc0);
	_sdk_debug( "%s %s vc1=%x", __func__, (ret == OK)?"OK":"ERROR", vc1);
	_sdk_debug( "%s %s vc3=%x", __func__, (ret == OK)?"OK":"ERROR", 0);
	_sdk_debug( "%s %s vc4=%x", __func__, (ret == OK)?"OK":"ERROR", vc4);
	_sdk_debug( "%s %s vc5=%x", __func__, (ret == OK)?"OK":"ERROR", vc5);

	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_SWITCH_MODE, mgmt);
	if(ret == OK)
	{
		dev->vlan_enabled = enable;
		dev->vlan_filtering_enabled = enable;
	}
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}


static int b53125_vlan_mstp_id(sdk_driver_t *dev, vlan_t vid, int id)
{
	int ret = 0;
	u32 entry = 0;
	ret |= b53125_write16(dev->sdk_device, B53_ARLIO_PAGE, B53_VLAN_TBL_INDEX, vid);
	ret |= b53125_read32(dev->sdk_device, B53_ARLIO_PAGE, B53_VLAN_TBL_ENTRY, &entry);
	entry &= ~(VLAN_TBL_MSTP_INDEX_MASK<<VLAN_TBL_MSTP_INDEX_OFFSET);
	entry |= (id<<VLAN_TBL_MSTP_INDEX_OFFSET);

	ret |= b53125_write32(dev->sdk_device, B53_ARLIO_PAGE, B53_VLAN_TBL_ENTRY, entry);
	ret |= b53125_do_vlan_op(dev->sdk_device, VLAN_TBL_CMD_WRITE);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

static int b53125_set_vlan_entry(sdk_driver_t *dev, vlan_t vid, b53_vlan_t *vlanentry)
{
	int ret = 0, vlanentryval = 0;;
	ret |= b53125_write16(dev->sdk_device, B53_ARLIO_PAGE, B53_VLAN_TBL_INDEX, vid);
	vlanentryval = (vlanentry->untag << VLAN_UNTAG_OFFSET) | vlanentry->tag;
	//vlanentryval |= VLAN_FWD_MODE_VLAN;
	vlanentryval |= BIT(8);
	ret |= b53125_write32(dev->sdk_device, B53_ARLIO_PAGE, B53_VLAN_TBL_ENTRY, vlanentryval);
	ret |= b53125_do_vlan_op(dev, VLAN_TBL_CMD_WRITE);

	_sdk_debug( "%s %s vlan=%d val=%x", __func__, (ret == OK)?"OK":"ERROR", vid, vlanentryval);
	return ret;
}

static int b53125_get_vlan_entry(sdk_driver_t *dev, vlan_t vid, b53_vlan_t *vlanentry)
{
	int ret = 0;
	u32 entry = 0;
	ret |= b53125_write16(dev->sdk_device, B53_ARLIO_PAGE, B53_VLAN_TBL_INDEX, vid);
	ret |= b53125_do_vlan_op(dev, VLAN_TBL_CMD_READ);
	ret |= b53125_read32(dev->sdk_device, B53_ARLIO_PAGE, B53_VLAN_TBL_ENTRY, &entry);

	vlanentry->tag = entry & VLAN_VID_MASK;
	vlanentry->untag = (entry >> VLAN_UNTAG_OFFSET) & VLAN_VID_MASK;
	vlanentry->valid = true;

	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

static int b53125_add_vlan_entry(sdk_driver_t *dev, vlan_t vid)
{
	int ret = 0;
	b53_vlan_t vlanentry;
	ret |= b53125_get_vlan_entry(dev,  vid, &vlanentry);
/*
	vlanentry.tag |= BIT(8);
	if (untagged && !b53_vlan_port_needs_forced_tagged(ds, port))
		vlanentry.untag |= BIT(port);
	else
		vlanentry.untag &= ~BIT(port);
*/
//	entry |= BIT(8);
	ret |= b53125_set_vlan_entry(dev,  vid, &vlanentry);
	b53125_clear_mac_tbl_vlan(dev, vid);	
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

static int b53125_del_vlan_entry(sdk_driver_t *dev, vlan_t vid)
{
	int ret = 0;
	b53_vlan_t vlanentry;
	ret |= b53125_get_vlan_entry(dev,  vid, &vlanentry);
	vlanentry.untag = vlanentry.tag = 0;
	ret |= b53125_set_vlan_entry(dev,  vid, &vlanentry);
	b53125_clear_mac_tbl_vlan(dev, vid);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}


static int b53125_add_vlan_portlst(sdk_driver_t *dev, vlan_t vid, zpl_phyport_t *port, int num, zpl_bool tag)
{
	int ret = 0, i = 0;
	zpl_bool untagged = !tag;
	b53_vlan_t vlanentry;
	ret |= b53125_get_vlan_entry(dev,  vid, &vlanentry);
	for(i = 0; i < num; i++)
	{
		vlanentry.tag |= BIT(port[i]);
		if(untagged)
		{
			vlanentry.untag |= BIT(port[i]);
		}
		else
		{
			vlanentry.untag &= ~BIT(port[i]);
		}
	}
	ret |= b53125_set_vlan_entry(dev,  vid, &vlanentry);

	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

static int b53125_del_vlan_portlst(sdk_driver_t *dev, vlan_t vid, zpl_phyport_t *port, int num, zpl_bool tag)
{
	int ret = 0, i = 0;
	bool untagged = !tag;
	b53_vlan_t vlanentry;
	ret |= b53125_get_vlan_entry(dev,  vid, &vlanentry);
	for(i = 0; i < num; i++)
	{
		vlanentry.tag &= ~BIT(port[i]);

		if (untagged)
			vlanentry.untag &= ~(BIT(port[i]));
	}

	ret |= b53125_set_vlan_entry(dev,  vid, &vlanentry);
	return ret;
}

static int b53125_add_vlan_port(sdk_driver_t *dev, vlan_t vid, zpl_phyport_t port, zpl_bool tag)
{
	int ret = 0;
	zpl_bool untagged = !tag;
	b53_vlan_t vlanentry;
	ret |= b53125_get_vlan_entry(dev,  vid, &vlanentry);
	vlanentry.tag |= BIT(port);
	if(untagged)
	{
		vlanentry.untag |= BIT(port);
	}
	else
	{
		vlanentry.untag &= ~BIT(port);
	}
	ret |= b53125_set_vlan_entry(dev,  vid, &vlanentry);
	_sdk_debug( "%s %s VID: %d, members: 0x%04x, untag: 0x%04x", __func__, (ret == OK)?"OK":"ERROR", vid, vlanentry.tag, vlanentry.untag);
	/*if (port != 8) {
		b53125_write16(dev->sdk_device, B53_VLAN_PAGE, B53_VLAN_PORT_DEF_TAG(port), vid);
		_sdk_debug( "====%s page=0x%x reg=0x%x val=0x%x\n", __func__, B53_VLAN_PAGE, B53_VLAN_PORT_DEF_TAG(port),vid);
	}*/
	return ret;
}

static int b53125_del_vlan_port(sdk_driver_t *dev, vlan_t vid, zpl_phyport_t port, zpl_bool tag)
{
	int ret = 0;
	bool untagged = !tag;
	b53_vlan_t vlanentry;
	u16 entry;
	ret |= b53125_read16(dev->sdk_device, B53_VLAN_PAGE, B53_VLAN_PORT_DEF_TAG(port), &entry);
	ret |= b53125_get_vlan_entry(dev,  vid, &vlanentry);

	vlanentry.tag &= ~BIT(port);

	if (untagged)
		vlanentry.untag &= ~(BIT(port));

	ret |= b53125_set_vlan_entry(dev,  vid, &vlanentry);
	if(entry == vid)
		entry = 1;
	ret |= b53125_write16(dev->sdk_device, B53_VLAN_PAGE, B53_VLAN_PORT_DEF_TAG(port), &entry);
	return ret;
}

static int b53125_vlan_create(sdk_driver_t *dev, zpl_bool c, vlan_t vid)
{
	if(c)
		return b53125_add_vlan_entry(dev,  vid);
	else	
		return b53125_del_vlan_entry(dev,  vid);
}

static int b53125_vlan_port_tag(sdk_driver_t *dev, zpl_bool a, zpl_phyport_t port , vlan_t vid)
{
	int ret = 0;
	_sdk_debug("%s:op=%d phyport=%x vlan=%d", __func__, a, port, vid);
	if(a)
		ret = b53125_add_vlan_port(dev,  vid,  port, zpl_true);
	else 
		ret = b53125_del_vlan_port(dev,  vid,  port, zpl_true);	
	if(ret == OK)
	{
		int i = 0;
		for(i = 0; i < PHY_PORT_MAX; i++)
		{
			if(dev->phyports_table[i].phyport == port)
			{
				if(a)
					zpl_vlan_bitmap_set(dev->phyports_table[i].vlanbitmap, vid);
				else
					zpl_vlan_bitmap_clr(dev->phyports_table[i].vlanbitmap, vid);	
			}
		}
	}
	return ret;
}

static int b53125_vlan_port_untag(sdk_driver_t *dev, zpl_bool a, zpl_phyport_t port , vlan_t vid)
{
	if(a)
		return b53125_add_vlan_port(dev,  vid,  port, zpl_false);
	else 
		return b53125_del_vlan_port(dev,  vid,  port, zpl_false);	
}


void b53125_imp_vlan_setup(sdk_driver_t *dev, int cpu_port)
{
	unsigned int i;
	u16 pvlan;
	for(i = 0; i < 8; i++)
	{
		if(dev->phyports_table[i].phyport >= 0)
		{
			b53125_read16(dev->sdk_device, B53_PVLAN_PAGE, B53_PVLAN_PORT(i), &pvlan);
			pvlan |= BIT(cpu_port);
			b53125_write16(dev->sdk_device, B53_PVLAN_PAGE, B53_PVLAN_PORT(i), pvlan);
		}
	}
}
static int b53125_port_vlan_enable(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = 0;
	u16 entry = 0;
	ret |= b53125_read16(dev->sdk_device, B53_PVLAN_PAGE, B53_PVLAN_PORT(port), &entry);
	/*
	if(enable)
		entry |= BIT(port);
	else
		entry &= ~BIT(port);*/
	entry = 0x1ff;
	ret |= b53125_write16(dev->sdk_device, B53_PVLAN_PAGE, B53_PVLAN_PORT(port), entry);

	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/**************************************************************************************/
int b53125_port_vlan(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = 0;
	u16 entry = 0;
	ret |= b53125_read16(dev->sdk_device, B53_PVLAN_PAGE, B53_PVLAN_PORT(port), &entry);
	if(enable)
		entry |= BIT(port);
	else
		entry &= ~BIT(port);
	ret |= b53125_write16(dev->sdk_device, B53_PVLAN_PAGE, B53_PVLAN_PORT(port), entry);
	b53125_imp_vlan_setup(dev, dev->cpu_port);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}


static int b53125_port_pvid(sdk_driver_t *dev, zpl_bool e, zpl_phyport_t port, vlan_t vid)
{
	int ret = 0;
	u16 entry = (ARLTBL_VID_MASK & vid) | (0<<13);
	ret |= b53125_write16(dev->sdk_device, B53_VLAN_PAGE, B53_VLAN_PORT_DEF_TAG(port), entry);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

static int b53125_vlan_double_tagging_tpid(sdk_driver_t *dev, u16 tpid)
{
	int ret = 0;
	ret |= b53125_write16(dev->sdk_device, B53_VLAN_PAGE, B53_VLAN_ISP_TPID, tpid);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

static int b53125_ISP_port(sdk_driver_t *dev, zpl_bool enable, zpl_phyport_t port)
{
	int ret = 0;
	u16 reg = 0;
	ret |= b53125_read16(dev->sdk_device, B53_VLAN_PAGE, B53_VLAN_ISP_PORT, &reg);
	if(enable)
		reg |= BIT(port);
	else
		reg &= ~BIT(port);
	ret |= b53125_write16(dev->sdk_device, B53_VLAN_PAGE, B53_VLAN_ISP_PORT, reg);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}


static int sdk_mstp_add_vlan (sdk_driver_t *dev,  zpl_index_t id, vlan_t vid)
{
	return b53125_vlan_mstp_id(dev,  vid,  id);
}
static int sdk_mstp_del_vlan (sdk_driver_t *dev,  zpl_index_t id, vlan_t vid)
{
	return b53125_vlan_mstp_id(dev,  vid,  0);
}

int b53125_enable_vlan_default(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = -1;
	unsigned int i;
	if(b53125_enable_vlan(dev,  enable) == OK)
	{
		if(b53125_vlan_create(dev, enable, 1) == OK)
		{
			if(enable == zpl_false)
				return OK;

			for(i = 0; i < PHY_PORT_MAX; i++)
			{
				if(dev->phyports_table[i].phyport >= 0)
				{
					//zpl_vlan_bitmap_set(dev->phyports_table[i].vlanbitmap, 1);
					b53125_port_pvid(dev, 1, dev->phyports_table[i].phyport, 1);
					//b53125_vlan_port_untag(dev, 1, dev->phyports_table[i].phyport, 1);
				}
			}
			//b53125_vlan_create(dev, enable, 200);
			//b53125_vlan_port_tag(dev, 1, 3 , 200);
			return OK;
		}
	}
	return ret;
}
#if defined( _SDK_CLI_DEBUG_EN)
/*
enable vlan
create vlan
port pvid
enable port(vlan)
*/

DEFUN (sdk_b53125_vlan_enable,
		sdk_b53125_vlan_enable_cmd,
		"sdk-vlan (disable|enable)",
		"sdk vlan\n"
		"config\n"
		"enable\n")
{
	int ret = 0;
	struct b53vlan_cmd test;
	b53_device_t *sdkdev;
	if(strstr(argv[0], "enable"))
		ret = b53125_enable_vlan(__msdkdriver, 1);
	else
		ret = b53125_enable_vlan(__msdkdriver, 0);	

	sdkdev = __msdkdriver->sdk_device;
	memset(&test, 0, sizeof(struct b53vlan_cmd));
	//ret = ioctl(sdkdev->mido.fd, B53_IO_VLAN, &test);

	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (sdk_b53125_vlan_create,
		sdk_b53125_vlan_create_cmd,
		"sdk-vlan create <1-4094>",
		"sdk vlan\n"
		"create\n"
		"vlanid\n")
{
	int ret = 0;
	vlan_t val = 0;
	val = (vlan_t)atoi(argv[0]);
	ret = b53125_vlan_create(__msdkdriver, 1, val);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (sdk_b53125_vlan_add_port,
		sdk_b53125_vlan_add_port_cmd,
		"sdk-vlan (add|del) (tag|untag) port <0-8> vlan <1-4094>",
		"sdk vlan\n"
		"add\n"
		"del\n"
		"tag\n"
		"untag\n"
		"port\n"
		"port id\n"
		"vlan\n"
		"vlanid\n")
{
	int ret = 0;
	if(strstr(argv[0], "add"))
	{
		if(memcmp(argv[1], "untag", 3) == 0)
		{
			ret = b53125_vlan_port_untag(__msdkdriver, 1, atoi(argv[2]), atoi(argv[3]));
		}
		else
		{
			ret = b53125_vlan_port_tag(__msdkdriver, 1, atoi(argv[2]), atoi(argv[3]));
		}
	}
	else
	{
		if(memcmp(argv[1], "untag", 3) == 0)
		{
			ret = b53125_vlan_port_untag(__msdkdriver, 0, atoi(argv[2]), atoi(argv[3]));
		}
		else
		{
			ret = b53125_vlan_port_tag(__msdkdriver, 0, atoi(argv[2]), atoi(argv[3]));
		}
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (sdk_b53125_vlan_reg,
		sdk_b53125_vlan_reg_cmd,
		"sdk-vlan reg show",
		"sdk vlan\n"
		"reg\n"
		"show\n")
{
	int ret = 0;
	u_char vc0 = 0,vc1 = 0,vc2 = 0,vc4 = 0,vc5 = 0;
	zpl_uint16 vc3 = 0;
	ret |= b53125_read8(__msdkdriver->sdk_device, B53_VLAN_PAGE, B53_GLOBAL_8021Q, &vc0);
	ret |= b53125_read8(__msdkdriver->sdk_device, B53_VLAN_PAGE, B53_GLOBAL_VLAN_CTRL1, &vc1);
	ret |= b53125_read8(__msdkdriver->sdk_device, B53_VLAN_PAGE, B53_GLOBAL_VLAN_CTRL4, &vc4);
	ret |= b53125_read8(__msdkdriver->sdk_device, B53_VLAN_PAGE, B53_GLOBAL_VLAN_CTRL5, &vc5);
	ret |= b53125_read16(__msdkdriver->sdk_device, B53_VLAN_PAGE, B53_GLOBAL_VLAN_CTRL3, &vc3);
	ret |= b53125_read8(__msdkdriver->sdk_device, B53_VLAN_PAGE, B53_GLOBAL_VLAN_CTRL5, &vc2);
	vty_out(vty, "  VC0=0x%x %s", vc0, VTY_NEWLINE);
	vty_out(vty, "  VC1=0x%x %s", vc1, VTY_NEWLINE);
	vty_out(vty, "  VC2=0x%x %s", vc2, VTY_NEWLINE);
	vty_out(vty, "  VC3=0x%x %s", vc3, VTY_NEWLINE);
	vty_out(vty, "  VC4=0x%x %s", vc4, VTY_NEWLINE);
	vty_out(vty, "  VC5=0x%x %s", vc5, VTY_NEWLINE);

	ret |= b53125_read16(__msdkdriver->sdk_device, B53_PVLAN_PAGE, B53_PVLAN_PORT(3), &vc3);
	vty_out(vty, "  port=3 pvlan val=%x %s", vc0, VTY_NEWLINE);
	ret |= b53125_read16(__msdkdriver->sdk_device, B53_VLAN_PAGE, B53_VLAN_PORT_DEF_TAG(3), &vc3);
	vty_out(vty, "  port=3 default vlan val=%x %s", vc0, VTY_NEWLINE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (sdk_b53125_vlan_macmode,
		sdk_b53125_vlan_macmode_cmd,
		"sdk-vlan macmode (svl|ivl)",
		"sdk vlan\n"
		"mac mode\n"
		"svl mode\n"
		"ivl mode\n")
{
	int ret = 0;
	u_char vc0 = 0;
	ret |= b53125_read8(__msdkdriver->sdk_device, B53_VLAN_PAGE, B53_GLOBAL_8021Q, &vc0);
#define   VC0_VID_HASH_VID		BIT(5)
#define   VC0_VID_CHK_EN		BIT(6)	/* Use VID,DA or VID,SA */
#define   VC0_VLAN_EN			BIT(7)	/* 802.1Q VLAN Enabled */
	if(strstr(argv[0], "ivl"))
		vc0 |= VC0_VLAN_EN | VC0_VID_CHK_EN | VC0_VID_HASH_VID;
	else
	{
		vc0 |= VC0_VLAN_EN;
		vc0 &= ~( VC0_VID_CHK_EN | VC0_VID_HASH_VID);
	}
	ret |= b53125_write8(__msdkdriver->sdk_device, B53_VLAN_PAGE, B53_GLOBAL_8021Q, vc0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (sdk_b53125_mac_hash,
		sdk_b53125_mac_hash_cmd,
		"sdk-vlan mac-hash <0-256>",
		"sdk vlan\n"
		"mac-hash\n"
		"reg val\n")
{
	int ret = 0;
	u_char vc0 = 0;
	ret |= b53125_read8(__msdkdriver->sdk_device, 0x04, 0, &vc0);
	vty_out(vty, "  reg Global ARL Configuration Register=0x%x %s", vc0, VTY_NEWLINE);
	vc0 = atoi(argv[0]);
	vty_out(vty, "  set Global ARL Configuration Register=0x%x %s", vc0, VTY_NEWLINE);
	ret |= b53125_write8(__msdkdriver->sdk_device, 0x04, 0, vc0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (sdk_b53125_port_pvid,
		sdk_b53125_port_pvid_cmd,
		"sdk-vlan port <0-8> pvid <0-4096>",
		"sdk vlan\n"
		"port\n"
		"port id\n"
		"vlan\n"
		"vlan id\n")
{
	int ret = 0;
	vlan_t val = 0;
	val = (vlan_t)atoi(argv[1]);
	if(val)
	{
		ret = b53125_port_vlan(__msdkdriver, atoi(argv[1]), 1);
		ret = b53125_port_pvid(__msdkdriver, 1, atoi(argv[1]), val);
	}
	else
	{
		ret = b53125_port_vlan(__msdkdriver, atoi(argv[1]), 0);
		ret = b53125_port_pvid(__msdkdriver, 1, atoi(argv[1]), 0);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
#endif

int b53125_vlan_init(sdk_driver_t *dev)
{
#if defined( _SDK_CLI_DEBUG_EN)	
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &sdk_b53125_vlan_reg_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &sdk_b53125_vlan_enable_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &sdk_b53125_vlan_create_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &sdk_b53125_vlan_add_port_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &sdk_b53125_vlan_macmode_cmd); 
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &sdk_b53125_mac_hash_cmd); 
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &sdk_b53125_port_pvid_cmd); 	
#endif 
	sdk_mstp.sdk_mstp_add_vlan = sdk_mstp_add_vlan;
	sdk_mstp.sdk_mstp_del_vlan = sdk_mstp_del_vlan;

	sdk_vlan.sdk_vlan_enable = b53125_enable_vlan_default;
    sdk_vlan.sdk_vlan_create = b53125_vlan_create;
 
    sdk_vlan.sdk_port_access_vlan = b53125_vlan_port_untag;

    sdk_vlan.sdk_port_native_vlan = b53125_port_pvid;//接口默认VLAN

    sdk_vlan.sdk_port_allowed_tag_vlan = b53125_vlan_port_tag;

    sdk_vlan.sdk_port_pvid_vlan = b53125_port_pvid;

    //sdk_vlan.sdk_vlan_stp_instance = NULL;
    sdk_vlan.sdk_vlan_translate = NULL;

	sdk_vlan.sdk_port_qinq_vlan = b53125_ISP_port;
    sdk_vlan.sdk_port_qinq_tpid = b53125_vlan_double_tagging_tpid;

	return OK;
}


/*
[  163.116544] ==b53dev==b53_enable_vlan: mgmt: 7, vc0:eb, vc1:6e, vc4:42, vc5:18
[  163.119986] bcm53xx stmmac-0:1e: Port -1 VLAN enabled: 1, filtering: 1

[  163.124909] ==b53dev==b53_set_vlan_entry: VID: 200, val: 0x0008
[  163.125000] bcm53xx stmmac-0:1e: VID: 200, members: 0x0008, untag: 0x0000
[  163.128100] ==b53dev==sdk_b53_vlan_add page=0x34 reg=0x16 val=0xc8

[  172.866615] sdk_u64_ether_addr:c8507b9de72949 -> 49:29:e7:9d:7b:50:c8:00
[  172.866716] ====== read ==0=== 50:7b:9d:e7:29:49 port 3 vid 200 static 0 age 1 valid 1
[  172.885831] sdk_u64_ether_addr:1507b9de72949 -> 49:29:e7:9d:7b:50:01:00
[  172.885924] ====== read ==1=== 50:7b:9d:e7:29:49 port 3 vid 1 static 0 age 1 valid 1
*/