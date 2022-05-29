/*
 * b53_vlan.c
 *
 *  Created on: May 3, 2019
 *      Author: zhurish
 */

#include <zplos_include.h>
#include "hal_driver.h"
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
static int b53125_enable_vlan(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 mgmt, vc0, vc1, vc4 = 0, vc5;
	zpl_bool enable_filtering = zpl_true;
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_SWITCH_MODE, &mgmt);
	ret |= b53125_read8(dev->sdk_device, B53_VLAN_PAGE, B53_GLOBAL_8021Q, &vc0);
	ret |= b53125_read8(dev->sdk_device, B53_VLAN_PAGE, B53_GLOBAL_VLAN_CTRL1, &vc1);

	ret |= b53125_read8(dev->sdk_device, B53_VLAN_PAGE, B53_GLOBAL_VLAN_CTRL4, &vc4);
	ret |= b53125_read8(dev->sdk_device, B53_VLAN_PAGE, B53_GLOBAL_VLAN_CTRL5, &vc5);

	mgmt &= ~SM_SW_FWD_MODE;

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

	vc5 &= ~VLAN_DROP_VID_FFF_EN;
	vc5 |= VLAN_DROP_VID_INVALID;
	ret |= b53125_write8(dev->sdk_device, B53_VLAN_PAGE, B53_GLOBAL_8021Q, vc0);
	ret |= b53125_write8(dev->sdk_device, B53_VLAN_PAGE, B53_GLOBAL_VLAN_CTRL1, vc1);

	ret |= b53125_write16(dev->sdk_device, B53_VLAN_PAGE, B53_GLOBAL_VLAN_CTRL3, 0);
	ret |= b53125_write8(dev->sdk_device, B53_VLAN_PAGE, B53_GLOBAL_VLAN_CTRL4, vc4);
	ret |= b53125_write8(dev->sdk_device, B53_VLAN_PAGE, B53_GLOBAL_VLAN_CTRL5, vc5);

	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_SWITCH_MODE, mgmt);
_sdk_debug( "======%s page=0x%x reg=0x%x val=0x%x", __func__, B53_CTRL_PAGE, B53_SWITCH_MODE, mgmt);
	((b53_device_t*)(dev->sdk_device))->vlan_enabled = enable;
	((b53_device_t*)(dev->sdk_device))->vlan_filtering_enabled = enable_filtering;
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

static int b53125_do_vlan_op(sdk_driver_t *dev, u8 op)
{
	int ret = 0;
	zpl_uint32 i;
	ret |= b53125_write8(dev->sdk_device, B53_ARLIO_PAGE, ((b53_device_t*)(dev->sdk_device))->vta_regs[0], VLAN_TBL_START_CMD | op);
	for (i = 0; i < 1000; i++) {
		u8 vta;
		ret |= b53125_read8(dev->sdk_device, B53_ARLIO_PAGE, ((b53_device_t*)(dev->sdk_device))->vta_regs[0], &vta);
		if (!(vta & VLAN_TBL_START_CMD))
			return 0;
		os_usleep(2000);
	}
	_sdk_err( " b53125_do_vlan_op timeout");
	return ERROR;
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
/*
		b53125_write32(dev->sdk_device, B53_ARLIO_PAGE, dev->vta_regs[2],
			    (vlan->untag << VLAN_UNTAG_OFFSET) | vlan->members);
*/
	ret |= b53125_do_vlan_op(dev, VLAN_TBL_CMD_WRITE);
	//dev_dbg(dev->ds->dev, "VID: %d, members: 0x%04x, untag: 0x%04x\n",
	//	vid, vlan->members, vlan->untag);
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
			entry &= ~(port << VLAN_UNTAG_OFFSET);
			entry |= BIT(port);
		}
		else
		{
			entry |= (port << VLAN_UNTAG_OFFSET);
			entry &= ~BIT(port);
		}
	}
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
			entry &= ~(port << VLAN_UNTAG_OFFSET);
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

DEFUN (b53125_enable_vlan_test,
		b53125_enable_vlan_test_cmd,
		"sdk-vlan enable <0-1>",
		"sdk vlan\n"
		"config\n"
		"enable\n")
{
	int ret = 0;
	ret = b53125_enable_vlan(__msdkdriver, atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (b53125_create_vlan_test,
		b53125_create_vlan_test_cmd,
		"sdk-vlan create <0-1> <1-4094>",
		"sdk vlan\n"
		"create\n"
		"enable\n"
		"vlanid\n")
{
	int ret = 0;
	ret = b53125_vlan_create(__msdkdriver, atoi(argv[0]), atoi(argv[1]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (b53125_port_addtagport_test,
		b53125_port_addtagport_test_cmd,
		"sdk-vlan add tagport <0-8> <1-4094>",
		"sdk vlan\n"
		"create\n"
		"enable\n"
		"vlanid\n")
{
	int ret = 0;
	ret = b53125_vlan_port_tag(__msdkdriver, 1, atoi(argv[0]), atoi(argv[1]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
DEFUN (b53125_port_adduntagport_test,
		b53125_port_adduntagport_test_cmd,
		"sdk-vlan add untagport <0-8> <1-4094>",
		"sdk vlan\n"
		"create\n"
		"enable\n"
		"vlanid\n")
{
	int ret = 0;
	ret = b53125_vlan_port_untag(__msdkdriver, 1, atoi(argv[0]), atoi(argv[1]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


static int sdk_mstp_add_vlan (sdk_driver_t *dev,  zpl_index_t id, vlan_t vid)
{
	return b53125_vlan_mstp_id(dev,  vid,  id);
}
static int sdk_mstp_del_vlan (sdk_driver_t *dev,  zpl_index_t id, vlan_t vid)
{
	return b53125_vlan_mstp_id(dev,  vid,  0);
}

int b53125_vlan_init(sdk_driver_t *dev)
{
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &b53125_enable_vlan_test_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &b53125_create_vlan_test_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &b53125_port_addtagport_test_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &b53125_port_adduntagport_test_cmd);

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


