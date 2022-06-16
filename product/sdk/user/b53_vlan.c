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
	ret |= b53125_write8(dev->sdk_device, B53_ARLIO_PAGE, ((b53_device_t*)(dev->sdk_device))->vta_regs[0], VLAN_TBL_START_CMD | op);
	for (i = 0; i < 10; i++) {
		u8 vta;
		ret |= b53125_read8(dev->sdk_device, B53_ARLIO_PAGE, ((b53_device_t*)(dev->sdk_device))->vta_regs[0], &vta);
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
	zpl_bool enable_filtering = zpl_true;
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

	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_SWITCH_MODE, mgmt);

	((b53_device_t*)(dev->sdk_device))->vlan_enabled = enable;
	((b53_device_t*)(dev->sdk_device))->vlan_filtering_enabled = enable_filtering;
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}


static int b53125_vlan_mstp_id(sdk_driver_t *dev, vlan_t vid, int id)
{
	int ret = 0;
	u32 entry = 0;
	ret |= b53125_write16(dev->sdk_device, B53_ARLIO_PAGE, ((b53_device_t*)(dev->sdk_device))->vta_regs[1], vid);
	ret |= b53125_read32(dev->sdk_device, B53_ARLIO_PAGE, ((b53_device_t*)(dev->sdk_device))->vta_regs[2], &entry);
	entry &= ~(VLAN_TBL_MSTP_INDEX_MASK<<VLAN_TBL_MSTP_INDEX_OFFSET);
	entry |= (id<<VLAN_TBL_MSTP_INDEX_OFFSET);

	ret |= b53125_write32(dev->sdk_device, B53_ARLIO_PAGE, ((b53_device_t*)(dev->sdk_device))->vta_regs[2], entry);
	ret |= b53125_do_vlan_op(dev->sdk_device, VLAN_TBL_CMD_WRITE);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}


static int b53125_add_vlan_entry(sdk_driver_t *dev, vlan_t vid)
{
	int ret = 0;
	u32 entry = 0;
	ret |= b53125_write16(dev->sdk_device, B53_ARLIO_PAGE, ((b53_device_t*)(dev->sdk_device))->vta_regs[1], vid);
	ret |= b53125_read32(dev->sdk_device, B53_ARLIO_PAGE, ((b53_device_t*)(dev->sdk_device))->vta_regs[2], &entry);
	entry |= BIT(8);
	ret |= b53125_write32(dev->sdk_device, B53_ARLIO_PAGE, ((b53_device_t*)(dev->sdk_device))->vta_regs[2], entry);

	ret |= b53125_do_vlan_op(dev, VLAN_TBL_CMD_WRITE);
	b53125_mac_address_clr(dev, -1, vid, 0);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}


static int b53125_del_vlan_entry(sdk_driver_t *dev, vlan_t vid)
{
	int ret = 0;
	u32 entry = 0, fwd = 0;
	ret |= b53125_write16(dev->sdk_device, B53_ARLIO_PAGE, ((b53_device_t*)(dev->sdk_device))->vta_regs[1], vid);

	ret |= b53125_read32(dev->sdk_device, B53_ARLIO_PAGE, ((b53_device_t*)(dev->sdk_device))->vta_regs[2], &entry);
	fwd = entry & BIT(21);
	ret |= b53125_write32(dev->sdk_device, B53_ARLIO_PAGE, ((b53_device_t*)(dev->sdk_device))->vta_regs[2], fwd);
	ret |= b53125_do_vlan_op(dev, VLAN_TBL_CMD_WRITE);
	b53125_mac_address_clr(dev, -1, vid, 0);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}


static int b53125_add_vlan_port(sdk_driver_t *dev, vlan_t vid, zpl_phyport_t port, zpl_bool tag)
{
	int ret = 0;
	u32 entry = 0;
	ret |= b53125_write16(dev->sdk_device, B53_ARLIO_PAGE, ((b53_device_t*)(dev->sdk_device))->vta_regs[1], vid);
	ret |= b53125_read32(dev->sdk_device, B53_ARLIO_PAGE, ((b53_device_t*)(dev->sdk_device))->vta_regs[2], &entry);
	if(port == dev->cpu_port)
	{
		if(tag)
		{
			entry &= ~BIT(17);
			entry |= BIT(8);
		}
		else
		{
			entry |= BIT(17);
			entry &= ~BIT(8);
		}
	}
	else
	{
		if(tag)
		{
			entry &= ~(BIT(port) << VLAN_UNTAG_OFFSET);
			entry |= BIT(port);
		}
		else
		{
			entry |= (BIT(port) << VLAN_UNTAG_OFFSET);
			entry &= ~BIT(port);
		}
	}
	b53125_port_vlan_enable(dev,  port, zpl_true);
	ret |= b53125_write32(dev->sdk_device, B53_ARLIO_PAGE, ((b53_device_t*)(dev->sdk_device))->vta_regs[2], entry);
	ret |= b53125_do_vlan_op(dev, VLAN_TBL_CMD_WRITE);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

static int b53125_del_vlan_port(sdk_driver_t *dev, vlan_t vid, zpl_phyport_t port, zpl_bool tag)
{
	int ret = 0;
	u32 entry = 0;
	ret |= b53125_write16(dev->sdk_device, B53_ARLIO_PAGE, ((b53_device_t*)(dev->sdk_device))->vta_regs[1], vid);
	ret |= b53125_read32(dev->sdk_device, B53_ARLIO_PAGE, ((b53_device_t*)(dev->sdk_device))->vta_regs[2], &entry);
	if(port == dev->cpu_port)
	{
		if(tag)
		{
			entry &= ~BIT(17);
		}
		else
		{
			entry &= ~BIT(8);
		}
	}
	else
	{
		if(tag)
		{
			entry &= ~(BIT(port) << VLAN_UNTAG_OFFSET);
		}
		else
		{
			entry &= ~BIT(port);
		}
	}
	ret |= b53125_write32(dev->sdk_device, B53_ARLIO_PAGE, ((b53_device_t*)(dev->sdk_device))->vta_regs[2], entry);
	ret |= b53125_do_vlan_op(dev, VLAN_TBL_CMD_WRITE);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
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
	if(a)
		return b53125_add_vlan_port(dev,  vid,  port, zpl_true);
	else 
		return b53125_del_vlan_port(dev,  vid,  port, zpl_true);	
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
		if(dev->ports_table[i] >= 0)
		{
			b53125_read16(dev->sdk_device, B53_PVLAN_PAGE, B53_PVLAN_PORT(i), &pvlan);
			pvlan |= BIT(cpu_port);
			b53125_write16(dev->sdk_device, B53_PVLAN_PAGE, B53_PVLAN_PORT(i), pvlan);
			_sdk_debug( "====%s page=0x%x reg=0x%x val=0x%x\n", __func__, B53_PVLAN_PAGE, B53_PVLAN_PORT(i), pvlan);
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
	b53125_imp_vlan_setup(dev, dev->cpu_port);
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

	sdk_vlan.sdk_vlan_enable = b53125_enable_vlan;
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


