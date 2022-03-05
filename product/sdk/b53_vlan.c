/*
 * b53_vlan.c
 *
 *  Created on: May 3, 2019
 *      Author: zhurish
 */

#include <zpl_include.h>
#include "hal_driver.h"
#include "sdk_driver.h"
#include "b53_driver.h"

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
	ret |= b53125_read8(dev->sdk_device, B53_VLAN_PAGE, B53_VLAN_CTRL0, &vc0);
	ret |= b53125_read8(dev->sdk_device, B53_VLAN_PAGE, B53_VLAN_CTRL1, &vc1);

	ret |= b53125_read8(dev->sdk_device, B53_VLAN_PAGE, B53_VLAN_CTRL4, &vc4);
	ret |= b53125_read8(dev->sdk_device, B53_VLAN_PAGE, B53_VLAN_CTRL5, &vc5);

	mgmt &= ~SM_SW_FWD_MODE;

	if (enable)
	{
		vc0 |= VC0_VLAN_EN | VC0_VID_CHK_EN | VC0_VID_HASH_VID;
		vc1 |= VC1_RX_MCST_UNTAG_EN | VC1_RX_MCST_FWD_EN;
		vc4 &= ~VC4_ING_VID_CHECK_MASK;
		if (enable_filtering)
		{
			vc4 |= VC4_ING_VID_VIO_DROP << VC4_ING_VID_CHECK_S;
			vc5 |= VC5_DROP_VTABLE_MISS;
		}
		else
		{
			vc4 |= VC4_ING_VID_VIO_FWD << VC4_ING_VID_CHECK_S;
			vc5 &= ~VC5_DROP_VTABLE_MISS;
		}
	}
	else
	{
		vc0 &= ~(VC0_VLAN_EN | VC0_VID_CHK_EN | VC0_VID_HASH_VID);
		vc1 &= ~(VC1_RX_MCST_UNTAG_EN | VC1_RX_MCST_FWD_EN);
		vc4 &= ~VC4_ING_VID_CHECK_MASK;
		vc5 &= ~VC5_DROP_VTABLE_MISS;

		vc4 |= VC4_ING_VID_VIO_TO_IMP << VC4_ING_VID_CHECK_S;
	}

	vc5 &= ~VC5_VID_FFF_EN;

	ret |= b53125_write8(dev->sdk_device, B53_VLAN_PAGE, B53_VLAN_CTRL0, vc0);
	ret |= b53125_write8(dev->sdk_device, B53_VLAN_PAGE, B53_VLAN_CTRL1, vc1);

	ret |= b53125_write16(dev->sdk_device, B53_VLAN_PAGE, B53_VLAN_CTRL3, 0);
	ret |= b53125_write8(dev->sdk_device, B53_VLAN_PAGE, B53_VLAN_CTRL4, vc4);
	ret |= b53125_write8(dev->sdk_device, B53_VLAN_PAGE, B53_VLAN_CTRL5, vc5);

	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_SWITCH_MODE, mgmt);

	((b53_device_t*)(dev->sdk_device))->vlan_enabled = enable;
	((b53_device_t*)(dev->sdk_device))->vlan_filtering_enabled = enable_filtering;
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

static int b53125_do_vlan_op(sdk_driver_t *dev, u8 op)
{
	int ret = 0;
	zpl_uint32 i;
	ret |= b53125_write8(dev->sdk_device, B53_ARLIO_PAGE, ((b53_device_t*)(dev->sdk_device))->vta_regs[0], VTA_START_CMD | op);
	for (i = 0; i < 1000; i++) {
		u8 vta;
		ret |= b53125_read8(dev->sdk_device, B53_ARLIO_PAGE, ((b53_device_t*)(dev->sdk_device))->vta_regs[0], &vta);
		if (!(vta & VTA_START_CMD))
			return 0;
		os_usleep(2000);
	}
	_sdk_err( " b53125_do_vlan_op timeout");
	return ERROR;
}

int b53125_vlan_mstp_id(sdk_driver_t *dev, vlan_t vid, int id)
{
	int ret = 0;
	u32 entry = 0;
	ret |= b53125_write16(dev->sdk_device, B53_ARLIO_PAGE, ((b53_device_t*)(dev->sdk_device))->vta_regs[1], vid);
	ret |= b53125_read32(dev->sdk_device, B53_ARLIO_PAGE, ((b53_device_t*)(dev->sdk_device))->vta_regs[2], &entry);
	entry &= ~(VTE_MSTP_INDEX_M<<VTE_MSTP_INDEX_S);
	entry |= (id<<VTE_MSTP_INDEX_S);

	ret |= b53125_write32(dev->sdk_device, B53_ARLIO_PAGE, ((b53_device_t*)(dev->sdk_device))->vta_regs[2], entry);
	ret |= b53125_do_vlan_op(dev->sdk_device, VTA_CMD_WRITE);
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
			    (vlan->untag << VTE_UNTAG_S) | vlan->members);
*/
	ret |= b53125_do_vlan_op(dev, VTA_CMD_WRITE);
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
	ret |= b53125_do_vlan_op(dev, VTA_CMD_WRITE);
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
			entry &= ~(port << VTE_UNTAG_S);
			entry |= BIT(port);
		}
		else
		{
			entry |= (port << VTE_UNTAG_S);
			entry &= ~BIT(port);
		}
	}
	ret |= b53125_write32(dev->sdk_device, B53_ARLIO_PAGE, ((b53_device_t*)(dev->sdk_device))->vta_regs[2], entry);
	ret |= b53125_do_vlan_op(dev, VTA_CMD_WRITE);
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
			entry &= ~(port << VTE_UNTAG_S);
		}
		else
		{
			entry &= ~BIT(port);
		}
	}
	ret |= b53125_write32(dev->sdk_device, B53_ARLIO_PAGE, ((b53_device_t*)(dev->sdk_device))->vta_regs[2], entry);
	ret |= b53125_do_vlan_op(dev, VTA_CMD_WRITE);
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

/**************************************************************************************/
int b53125_port_vlan(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = 0;
	u16 entry = 0;
	ret |= b53125_write16(dev->sdk_device, B53_PVLAN_PAGE, 2*(port), &entry);
	if(enable)
		entry |= BIT(port);
	else
		entry &= ~BIT(port);
	ret |= b53125_write16(dev->sdk_device, B53_PVLAN_PAGE, 2*(port), entry);
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

int b53_vlan_init(void)
{
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &b53125_enable_vlan_test_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &b53125_create_vlan_test_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &b53125_port_addtagport_test_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &b53125_port_adduntagport_test_cmd);
	sdk_vlan.sdk_vlan_enable = b53125_enable_vlan;
    sdk_vlan.sdk_vlan_create = b53125_vlan_create;
 
    sdk_vlan.sdk_vlan_untag_port = b53125_vlan_port_untag;
    sdk_vlan.sdk_vlan_tag_port = b53125_vlan_port_tag;
    sdk_vlan.sdk_port_native_vlan = b53125_port_pvid;//接口默认VLAN

    sdk_vlan.sdk_port_allowed_tag_vlan = b53125_vlan_port_tag;

    sdk_vlan.sdk_port_pvid_vlan = b53125_port_pvid;

    //sdk_vlan.sdk_vlan_stp_instance = NULL;
    sdk_vlan.sdk_vlan_translate = NULL;

	sdk_vlan.sdk_port_qinq_vlan = b53125_ISP_port;
    sdk_vlan.sdk_port_qinq_tpid = b53125_vlan_double_tagging_tpid;

	return OK;
}
#if 0

static void b53_get_vlan_entry(sdk_driver_t *dev, vlan_t vid,
			       struct b53_vlan *vlan)
{
	if (is5325(dev)) {
		u32 entry = 0;

		b53_write16(dev->sdk_device, B53_VLAN_PAGE, B53_VLAN_TABLE_ACCESS_25, vid |
			    VTA_RW_STATE_RD | VTA_RW_OP_EN);
		b53_read32(dev->sdk_device, B53_VLAN_PAGE, B53_VLAN_WRITE_25, &entry);

		if (dev->core_rev >= 3)
			vlan->valid = !!(entry & VA_VALID_25_R4);
		else
			vlan->valid = !!(entry & VA_VALID_25);
		vlan->members = entry & VA_MEMBER_MASK;
		vlan->untag = (entry >> VA_UNTAG_S_25) & VA_UNTAG_MASK_25;

	} else if (is5365(dev)) {
		u16 entry = 0;

		b53_write16(dev->sdk_device, B53_VLAN_PAGE, B53_VLAN_TABLE_ACCESS_65, vid |
			    VTA_RW_STATE_WR | VTA_RW_OP_EN);
		b53_read16(dev->sdk_device, B53_VLAN_PAGE, B53_VLAN_WRITE_65, &entry);

		vlan->valid = !!(entry & VA_VALID_65);
		vlan->members = entry & VA_MEMBER_MASK;
		vlan->untag = (entry >> VA_UNTAG_S_65) & VA_UNTAG_MASK_65;
	} else {
		u32 entry = 0;

		b53_write16(dev->sdk_device, B53_ARLIO_PAGE, dev->vta_regs[1], vid);
		b53_do_vlan_op(dev->sdk_device, VTA_CMD_READ);
		b53_read32(dev->sdk_device, B53_ARLIO_PAGE, dev->vta_regs[2], &entry);
		vlan->members = entry & VTE_MEMBERS;
		vlan->untag = (entry >> VTE_UNTAG_S) & VTE_MEMBERS;
		vlan->valid = true;
	}
}

int b53125_vlan_add(sdk_driver_t *dev, zpl_phyport_t port,
		  const struct switchdev_obj_port_vlan *vlan)
{
	sdk_driver_t *dev = ds->priv;
	bool untagged = vlan->flags & BRIDGE_VLAN_INFO_UNTAGGED;
	bool pvid = vlan->flags & BRIDGE_VLAN_INFO_PVID;
	struct b53_vlan *vl;
	vlan_t vid;

	for (vid = vlan->vid_begin; vid <= vlan->vid_end; ++vid) {
		vl = &dev->vlans[vid];

		b53_get_vlan_entry(dev->sdk_device, vid, vl);

		vl->members |= BIT(port);
		if (untagged && !dsa_is_cpu_port(ds, port))
			vl->untag |= BIT(port);
		else
			vl->untag &= ~BIT(port);

		b53_set_vlan_entry(dev->sdk_device, vid, vl);
		b53_fast_age_vlan(dev->sdk_device, vid);
	}

	if (pvid && !dsa_is_cpu_port(ds, port)) {
		b53_write16(dev->sdk_device, B53_VLAN_PAGE, B53_VLAN_PORT_DEF_TAG(port),
			    vlan->vid_end);
		b53_fast_age_vlan(dev->sdk_device, vid);
	}
}
EXPORT_SYMBOL(b53_vlan_add);

int b53_vlan_del(struct dsa_switch *ds, zpl_phyport_t port,
		 const struct switchdev_obj_port_vlan *vlan)
{
	struct b53_device *dev = ds->priv;
	bool untagged = vlan->flags & BRIDGE_VLAN_INFO_UNTAGGED;
	struct b53_vlan *vl;
	vlan_t vid;
	u16 pvid;

	b53_read16(dev->sdk_device, B53_VLAN_PAGE, B53_VLAN_PORT_DEF_TAG(port), &pvid);

	for (vid = vlan->vid_begin; vid <= vlan->vid_end; ++vid) {
		vl = &dev->vlans[vid];

		b53_get_vlan_entry(dev->sdk_device, vid, vl);

		vl->members &= ~BIT(port);

		if (pvid == vid)
			pvid = b53_default_pvid(dev);

		if (untagged && !dsa_is_cpu_port(ds, port))
			vl->untag &= ~(BIT(port));

		b53_set_vlan_entry(dev->sdk_device, vid, vl);
		b53_fast_age_vlan(dev->sdk_device, vid);
	}

	b53_write16(dev->sdk_device, B53_VLAN_PAGE, B53_VLAN_PORT_DEF_TAG(port), pvid);
	b53_fast_age_vlan(dev->sdk_device, pvid);

	return 0;
}

int b53_vlan_filtering(struct dsa_switch *ds, zpl_phyport_t port, bool vlan_filtering)
{
	struct b53_device *dev = ds->priv;
	struct net_device *bridge_dev;
	zpl_uint32 i;
	u16 pvid, new_pvid;

	/* Handle the case were multiple bridges span the same switch device
	 * and one of them has a different setting than what is being requested
	 * which would be breaking filtering semantics for any of the other
	 * bridge devices.
	 */
	b53_for_each_port(dev->sdk_device, i) {
		bridge_dev = dsa_to_port(ds, i)->bridge_dev;
		if (bridge_dev &&
		    bridge_dev != dsa_to_port(ds, port)->bridge_dev &&
		    br_vlan_enabled(bridge_dev) != vlan_filtering) {
			netdev_err(bridge_dev,
				   "VLAN filtering is global to the switch!\n");
			return -IPSTACK_ERRNO_EINVAL;
		}
	}

	b53_read16(dev->sdk_device, B53_VLAN_PAGE, B53_VLAN_PORT_DEF_TAG(port), &pvid);
	new_pvid = pvid;
	if (dev->vlan_filtering_enabled && !vlan_filtering) {
		/* Filtering is currently enabled, use the default PVID since
		 * the bridge does not expect tagging anymore
		 */
		dev->ports[port].pvid = pvid;
		new_pvid = b53_default_pvid(dev);
	} else if (!dev->vlan_filtering_enabled && vlan_filtering) {
		/* Filtering is currently disabled, restore the previous PVID */
		new_pvid = dev->ports[port].pvid;
	}

	if (pvid != new_pvid)
		b53_write16(dev->sdk_device, B53_VLAN_PAGE, B53_VLAN_PORT_DEF_TAG(port),
			    new_pvid);

	b53_enable_vlan(dev->sdk_device, dev->vlan_enabled, vlan_filtering);

	return 0;
}
EXPORT_SYMBOL(b53_vlan_filtering);

int b53_vlan_prepare(struct dsa_switch *ds, zpl_phyport_t port,
		     const struct switchdev_obj_port_vlan *vlan)
{
	struct b53_device *dev = ds->priv;

	if ((is5325(dev) || is5365(dev)) && vlan->vid_begin == 0)
		return -IPSTACK_ERRNO_EOPNOTSUPP;

	if (vlan->vid_end > dev->num_vlans)
		return -ERANGE;

	b53_enable_vlan(dev->sdk_device, true, dev->vlan_filtering_enabled);

	return 0;
}
EXPORT_SYMBOL(b53_vlan_prepare);
#endif

