/*
 * b53_mac_tbl.c
 *
 *  Created on: May 3, 2019
 *      Author: zhurish
 */


#include "sdk_driver.h"
#include "b53_driver.h"
#include "b53_mac.h"

/*
#include "/home/zhurish/workspace/working/sunxi/linux-5.15.14/drivers/net/dsa/b53/b53_priv.h"

extern int b53_fdb_dump_all(struct b53_device *priv, int port,
		 dsa_fdb_dump_cb_all_t *cb, void *data);
extern int sdk_b53_vlan_enable(struct b53_device *dev, bool enable, bool vlan_filtering);
extern int sdk_b53_vlan_add(struct b53_device *dev, int port,
		 int vid,int tag);
*/
/****************************************************************************************/
static int b53125_flush_mac_tbl(sdk_driver_t *dev, u8 mask)
{
	zpl_uint32 i;
	b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_FAST_AGE_CTRL,
		   FAST_AGE_DONE | FAST_AGE_DYNAMIC | mask);
	for (i = 0; i < 10; i++) {
		u8 fast_age_ctrl;
		b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_FAST_AGE_CTRL, &fast_age_ctrl);
		if (!(fast_age_ctrl & FAST_AGE_DONE))
			goto out;
		msleep(1);
	}
	return OS_TIMEOUT;
out:
	/* Only age dynamic entries (default behavior) */
	b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_FAST_AGE_CTRL, FAST_AGE_DYNAMIC);
	return OK;
}

static int b53125_clear_mac_tbl_port(sdk_driver_t *dev, zpl_phyport_t port)
{
	b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_FAST_AGE_PORT_CTRL, port);

	return b53125_flush_mac_tbl(dev, FAST_AGE_PORT);
}

int b53125_mac_clear_all(sdk_driver_t *dev)
{
	b53125_write16(dev->sdk_device, B53_CTRL_PAGE, B53_FAST_AGE_PORT_CTRL, 0xff);
	return b53125_flush_mac_tbl(dev, FAST_AGE_PORT);
}

static int b53125_clear_mac_tbl_vlan(sdk_driver_t *dev, vlan_t vid)
{
	b53125_write16(dev->sdk_device, B53_CTRL_PAGE, B53_FAST_AGE_VID_CTRL, vid);
	return b53125_flush_mac_tbl(dev, FAST_AGE_VLAN);
}

/****************************************************************************************/
/****************************************************************************************/
static void b53125_mac_tbl_to_entry(struct b53125_mac_arl_entry *ent,
				    u64 mac_vid, u32 fwd_entry)
{
	memset(ent, 0, sizeof(struct b53125_mac_arl_entry));
	ent->port = fwd_entry & ARLTBL_DATA_PORT_ID_MASK;
	ent->is_valid = !!(fwd_entry & ARLTBL_VALID);
	ent->is_age = !!(fwd_entry & ARLTBL_AGE);
	ent->is_static = !!(fwd_entry & ARLTBL_STATIC);
	sdk_u64_ether_addr(mac_vid, ent->mac);
	ent->vid = mac_vid >> ARLTBL_VID_S;
}

static void b53125_mac_tbl_from_entry(u64 *mac_vid, u32 *fwd_entry,
				      const struct b53125_mac_arl_entry *ent)
{
	*mac_vid = sdk_ether_addr_u64(ent->mac);
	*mac_vid |= (u64)(ent->vid & ARLTBL_VID_MASK) << ARLTBL_VID_S;
	*fwd_entry = ent->port & ARLTBL_DATA_PORT_ID_MASK;
	if (ent->is_valid)
		*fwd_entry |= ARLTBL_VALID;
	if (ent->is_static)
		*fwd_entry |= ARLTBL_STATIC;
	if (ent->is_age)
		*fwd_entry |= ARLTBL_AGE;
}



/* Address Resolution Logic routines */
static int b53125_mac_tbl_op_wait(sdk_driver_t *dev)
{
	zpl_uint32 timeout = 100;
	u8 reg;
	do {
		b53125_read8(dev->sdk_device, B53_ARLIO_PAGE, B53_ARLTBL_RW_CTRL, &reg);
		if (!(reg & ARLTBL_START_DONE))
			return 0;

		usleep_range(1000, 2000);
	} while (timeout--);
	sdk_err( " b53125_mac_tbl_op_wait OS_TIMEOUT\r\n");
	return OS_TIMEOUT;
}

static int b53125_mac_tbl_rw_op(sdk_driver_t *dev, zpl_uint32 op)
{
	u8 reg;
	if (op > ARLTBL_RW)
		return ERROR;
	b53125_read8(dev->sdk_device, B53_ARLIO_PAGE, B53_ARLTBL_RW_CTRL, &reg);
	reg |= ARLTBL_START_DONE;
	if (op)
		reg |= ARLTBL_RW;
	else
		reg &= ~ARLTBL_RW;

	b53125_write8(dev->sdk_device, B53_ARLIO_PAGE, B53_ARLTBL_RW_CTRL, reg);
	return b53125_mac_tbl_op_wait(dev);
}

static int b53125_mac_tbl_read(sdk_driver_t *dev, u64 mac,
			vlan_t vid, struct b53125_mac_arl_entry *ent, u8 *idx,
			bool is_valid)
{
	zpl_uint32 i;
	int ret = 0;
	ret = b53125_mac_tbl_op_wait(dev);
	if (ret != 0)
		return ret;

	/* Read the bins */
	for (i = 0; i < ((b53_device_t *)dev->sdk_device)->num_arl_entries; i++) {
		u64 mac_vid;
		u32 fwd_entry;

		b53125_read64(dev->sdk_device, B53_ARLIO_PAGE,
			   B53_ARLTBL_MAC_VID_ENTRY(i), &mac_vid);
		b53125_read32(dev->sdk_device, B53_ARLIO_PAGE,
			   B53_ARLTBL_DATA_ENTRY(i), &fwd_entry);
		b53125_mac_tbl_to_entry(ent, mac_vid, fwd_entry);

		if (!(fwd_entry & ARLTBL_VALID))
			continue;
		if ((mac_vid & ARLTBL_MAC_MASK) != mac)
			continue;
		if (dev->vlan_enabled &&
		    ((mac_vid >> ARLTBL_VID_S) & ARLTBL_VID_MASK) != vid)
			continue;
		*idx = i;
		return 0;
	}
	return ERROR;
}

static int b53125_mac_tbl_op(sdk_driver_t *dev, int op, zpl_phyport_t port,
		      const zpl_uint8 *addr, vlan_t vid, zpl_bool is_valid)
{
	struct b53125_mac_arl_entry ent;
	u32 fwd_entry;
	u64 mac, mac_vid = 0;
	u8 idx = 0;
	int ret;
	//sdk_err( " b53125_mac_tbl_op");
	/* Convert the array into a 64-bit MAC */
	mac = sdk_ether_addr_u64(addr);

	/* Perform a read for the given MAC and VID */
	b53125_write48(dev->sdk_device, B53_ARLIO_PAGE, B53_MAC_ADDR_IDX, mac);
	b53125_write16(dev->sdk_device, B53_ARLIO_PAGE, B53_VLAN_ID_IDX, vid);

	/* Issue a read operation for this MAC */
	ret = b53125_mac_tbl_rw_op(dev, 1);
	if (ret)
		return ret;

	ret = b53125_mac_tbl_read(dev, mac, vid, &ent, &idx, is_valid);
	/* If this is a read, just finish now */
	if (op)
		return ret;

	/* We could not find a matching MAC, so reset to a new entry */
	if (ret) {
		fwd_entry = 0;
		idx = 1;
	}
	if (!sdk_is_multicast_ether_addr(addr)) {
		ent.port = port;
		ent.is_valid = is_valid;
	} else {
		if (is_valid)
			ent.port |= BIT(port);
		else
			ent.port &= ~BIT(port);

		ent.is_valid = !!(ent.port);
	}
	memset(&ent, 0, sizeof(ent));
	ent.port = port;
	ent.is_valid = is_valid;
	ent.vid = vid;
	ent.is_static = zpl_true;
	memcpy(ent.mac, addr, ETH_ALEN);
	b53125_mac_tbl_from_entry(&mac_vid, &fwd_entry, &ent);

	b53125_write64(dev->sdk_device, B53_ARLIO_PAGE,
		    B53_ARLTBL_MAC_VID_ENTRY(idx), mac_vid);
	b53125_write32(dev->sdk_device, B53_ARLIO_PAGE,
		    B53_ARLTBL_DATA_ENTRY(idx), fwd_entry);

	return b53125_mac_tbl_rw_op(dev, 0);
}

static int b53125_mac_tbl_add(sdk_driver_t *dev, zpl_phyport_t port,
		const zpl_uint8 *addr, vlan_t vid)
{
	return b53125_mac_tbl_op(dev, 0, port, addr, vid, zpl_true);
}


static int b53125_mac_tbl_del(sdk_driver_t *dev, zpl_phyport_t port,
		const zpl_uint8 *addr, vlan_t vid)
{
	return b53125_mac_tbl_op(dev, 0, port, addr, vid, zpl_false);
}


static int b53125_mac_address_add(sdk_driver_t *dev, zpl_phyport_t phyport, 
	zpl_vlan_t vlanid, zpl_uint32 vrfid, mac_t *mac, zpl_uint32 pri)
{
	return b53125_mac_tbl_add(dev,  phyport, mac, vlanid);
}

static int b53125_mac_address_del(sdk_driver_t *dev, zpl_phyport_t phyport, 
	zpl_vlan_t vlanid, zpl_uint32 vrfid, mac_t *mac, zpl_uint32 pri)
{
	return b53125_mac_tbl_del(dev, phyport, mac, vlanid);
}

int b53125_mac_address_clr(sdk_driver_t *dev, zpl_phyport_t phyport, 
	zpl_vlan_t vlanid, zpl_uint32 vrfid)
{
	if(vlanid)
		return b53125_clear_mac_tbl_vlan(dev, vlanid);
	else if(phyport >= 0)	
		return b53125_clear_mac_tbl_port(dev, phyport);
	else
		return b53125_clear_mac_tbl_port(dev, 0xff);	
}

static unsigned int b53125_max_arl_entries(sdk_driver_t *dev)
{
	unsigned int maxnum = 0;
	struct b53125_device * sdk_device = dev->sdk_device;
	maxnum = sdk_device->num_arl_buckets * sdk_device->num_arl_bins;
	return maxnum;
}

static int b53125_arl_search_wait(sdk_driver_t *dev)
{
	unsigned int timeout = 1000;
	u8 reg;

	do {
		b53125_read8(dev->sdk_device, B53_ARLIO_PAGE, B53_ARL_SRCH_CTL, &reg);
		if (!(reg & ARL_SRCH_STDN))
			return 0;

		if (reg & ARL_SRCH_VLID)
			return 0;

		usleep_range(1000, 2000);
	} while (timeout--);

	return -ETIMEDOUT;
}

static void b53125_arl_search_rd(sdk_driver_t *dev, u8 idx,
			      struct b53125_mac_arl_entry *ent)
{
	u64 mac_vid;
	u32 fwd_entry;

	b53125_read64(dev->sdk_device, B53_ARLIO_PAGE,
		   B53_ARL_SRCH_RSTL_MACVID(idx), &mac_vid);
	b53125_read32(dev->sdk_device, B53_ARLIO_PAGE,
		   B53_ARL_SRCH_RSTL(idx), &fwd_entry);
	b53125_mac_tbl_to_entry(ent, mac_vid, fwd_entry);
}

static int b53125_fdb_copy(const struct b53125_mac_arl_entry *ent,
			mac_arl_entry_cb cb, void *data)
{

	if (!ent->is_valid)
		return 0;
	if (ent->port > 8)
		return 0;
	cb(ent, data);
	return 0;
}



static int b53125_fdb_dump(sdk_driver_t *dev,
		 mac_arl_entry_cb cb, void *data)
{
	struct b53125_mac_arl_entry results[2];
	struct b53125_device * sdk_device = dev->sdk_device;
	unsigned int count = 0;
	int ret;
	u8 reg;

	/* Start search operation */
	reg = ARL_SRCH_STDN;
	b53125_write8(dev->sdk_device, B53_ARLIO_PAGE, B53_ARL_SRCH_CTL, reg);

	do {
		ret = b53125_arl_search_wait(dev);
		if (ret!= 0)
			return ret;

		b53125_arl_search_rd(dev, 0, &results[0]);
		ret = b53125_fdb_copy( &results[0], cb, data);
		if (ret)
			return ret;

		if (sdk_device->num_arl_bins > 2) {
			b53125_arl_search_rd(dev, 1, &results[1]);
			ret = b53125_fdb_copy(&results[1], cb, data);
			if (ret)
				return ret;

			if (!results[0].is_valid && !results[1].is_valid)
				break;
		}

	} while (count++ < b53125_max_arl_entries(dev) / 2);

	return 0;
}


static int b53125_mac_dump_cb(struct b53125_mac_arl_entry *entry, void *pp)
{
	static int n = 0;
	if(entry->is_valid == 0 || entry->is_static)
		return 0;
	sdk_debug( "====== read ==%d=== %02x:%02x:%02x:%02x:%02x:%02x port %d vid %d static %d age %d valid %d\r\n", 
		n++, entry->mac[0], entry->mac[1], entry->mac[2], entry->mac[3], entry->mac[4], entry->mac[5],
		entry->port, entry->vid, entry->is_static, entry->is_age, entry->is_valid);
	return OK;	
}


static int b53125_mac_address_dump(sdk_driver_t *dev, zpl_phyport_t phyport, 
	zpl_vlan_t vlanid, zpl_uint32 vrfid)
{
	sdk_debug("===========b53125_mac_address_dump====vlan_enabled:%d==========\r\n", dev->vlan_enabled);
	b53125_fdb_dump(dev, b53125_mac_dump_cb, NULL);
	return 0;
}

#if 0
int b53_mdb_add(struct dsa_switch *ds, int port,
		const struct switchdev_obj_port_mdb *mdb)
{
	struct b53_device *priv = ds->priv;

	/* 5325 and 5365 require some more massaging, but could
	 * be supported eventually
	 */
	if (is5325(priv) || is5365(priv))
		return -EOPNOTSUPP;

	return b53_arl_op(priv, 0, port, mdb->addr, mdb->vid, true);
}

int b53_mdb_del(struct dsa_switch *ds, int port,
		const struct switchdev_obj_port_mdb *mdb)
{
	struct b53_device *priv = ds->priv;
	int ret;

	ret = b53_arl_op(priv, 0, port, mdb->addr, mdb->vid, false);
	if (ret)
		dev_err(ds->dev, "failed to delete MDB entry\n");

	return ret;
}
#endif

static int b53125_mac_read_cb(struct b53125_mac_arl_entry *entry, void *pVoid)
{
	BSP_DRIVER(bspdev, pVoid);
	if(entry->is_valid == 0 || entry->is_static)
		return 0;
	bsp_driver_mac_cache_add(bspdev, entry->port, entry->mac, entry->vid, entry->is_static, entry->is_age, entry->is_valid);	
	/*if(bspdev->mac_cache_num < bspdev->mac_cache_max)
	{
		bspdev->mac_cache_entry[bspdev->mac_cache_num].port = entry->port;
		memcpy(bspdev->mac_cache_entry[bspdev->mac_cache_num].mac, entry->mac, ETH_ALEN);
		bspdev->mac_cache_entry[bspdev->mac_cache_num].vid = entry->vid;
		bspdev->mac_cache_entry[bspdev->mac_cache_num].use = 1;
		bspdev->mac_cache_entry[bspdev->mac_cache_num].is_valid = entry->is_valid;
		bspdev->mac_cache_entry[bspdev->mac_cache_num].is_age = entry->is_age;
		bspdev->mac_cache_entry[bspdev->mac_cache_num].is_static = entry->is_static;
		bspdev->mac_cache_entry[bspdev->mac_cache_num].res = 0;
		bspdev->mac_cache_num++;
	}*/
	return OK;	
}

static int b53125_mac_address_read(sdk_driver_t *dev, zpl_phyport_t phyport, 
	zpl_vlan_t vlanid, zpl_uint32 vrfid, void *pVoid)
{
	int i = 0;
	BSP_DRIVER(bspdev, pVoid);
	b53125_fdb_dump(dev, b53125_mac_read_cb, pVoid);	
	for(i = 0; i < bspdev->mac_cache_max-1; i++)
	{
		if(bspdev->mac_cache_entry[i].use == 0)
		{
			memcpy(&bspdev->mac_cache_entry[i], &bspdev->mac_cache_entry[i+1], sizeof(hal_mac_cache_t));
		}
	}		
	return OK;
}

#if defined( _SDK_CLI_DEBUG_EN)	

DEFUN (sdk_mac_dump_show,
		sdk_mac_dump_show_cmd,
		"mac show dump <0-8>",
		"sdk mac\n"
		"show\n"
		"dump\n"
		"port id [0-5]\n")
{
	int ret = 0;
	ret = b53125_mac_address_dump(__msdkdriver, 0, 0, 0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
#endif

int b53125_mac_init(sdk_driver_t *dev)
{
	#if defined( _SDK_CLI_DEBUG_EN)	
	install_element(SDK_NODE, CMD_CONFIG_LEVEL, &sdk_mac_dump_show_cmd);
	#endif
	sdk_maccb.sdk_mac_add_cb = b53125_mac_address_add;
	sdk_maccb.sdk_mac_del_cb = b53125_mac_address_del;
	sdk_maccb.sdk_mac_clr_cb = b53125_mac_address_clr;
	sdk_maccb.sdk_mac_read_cb = b53125_mac_address_read;
	sdk_maccb.sdk_mac_dump_cb = b53125_mac_address_dump;
	sdk_maccb.sdk_mac_clrall_cb = b53125_mac_clear_all;
	return OK;
}
/****************************************************************************************/
/****************************************************************************************/
#if 0

int b53_br_join(struct dsa_switch *ds, zpl_phyport_t port, struct net_device *br)
{
	sdk_driver_t *dev = ds->priv;
	s8 cpu_port = ds->ports[port].cpu_dp->index;
	u16 pvlan, reg;
	zpl_uint32 i;

	/* Make this port leave the all VLANs join since we will have proper
	 * VLAN entries from now on
	 */
	if (is58xx(dev)) {
		b53_read16(dev->sdk_device, B53_VLAN_PAGE, B53_JOIN_ALL_VLAN_EN, &reg);
		reg &= ~BIT(port);
		if ((reg & BIT(cpu_port)) == BIT(cpu_port))
			reg &= ~BIT(cpu_port);
		b53_write16(dev->sdk_device, B53_VLAN_PAGE, B53_JOIN_ALL_VLAN_EN, reg);
	}

	b53_read16(dev->sdk_device, B53_PVLAN_PAGE, B53_PVLAN_PORT(port), &pvlan);

	b53_for_each_port(dev->sdk_device, i) {
		if (dsa_to_port(ds, i)->bridge_dev != br)
			continue;

		/* Add this local port to the remote port VLAN control
		 * membership and update the remote port bitmask
		 */
		b53_read16(dev->sdk_device, B53_PVLAN_PAGE, B53_PVLAN_PORT(i), &reg);
		reg |= BIT(port);
		b53_write16(dev->sdk_device, B53_PVLAN_PAGE, B53_PVLAN_PORT(i), reg);
		dev->ports[i].vlan_ctl_mask = reg;

		pvlan |= BIT(i);
	}

	/* Configure the local port VLAN control membership to include
	 * remote ports and update the local port bitmask
	 */
	b53_write16(dev->sdk_device, B53_PVLAN_PAGE, B53_PVLAN_PORT(port), pvlan);
	dev->ports[port].vlan_ctl_mask = pvlan;

	return 0;
}

void b53_br_leave(struct dsa_switch *ds, zpl_phyport_t port, struct net_device *br)
{
	sdk_driver_t *dev = ds->priv;
	struct b53_vlan *vl = &dev->vlans[0];
	s8 cpu_port = ds->ports[port].cpu_dp->index;
	zpl_uint32 i;
	u16 pvlan, reg, pvid;

	b53_read16(dev->sdk_device, B53_PVLAN_PAGE, B53_PVLAN_PORT(port), &pvlan);

	b53_for_each_port(dev->sdk_device, i) {
		/* Don't touch the remaining ports */
		if (dsa_to_port(ds, i)->bridge_dev != br)
			continue;

		b53_read16(dev->sdk_device, B53_PVLAN_PAGE, B53_PVLAN_PORT(i), &reg);
		reg &= ~BIT(port);
		b53_write16(dev->sdk_device, B53_PVLAN_PAGE, B53_PVLAN_PORT(i), reg);
		dev->ports[port].vlan_ctl_mask = reg;

		/* Prevent self removal to preserve isolation */
		if (port != i)
			pvlan &= ~BIT(i);
	}

	b53_write16(dev->sdk_device, B53_PVLAN_PAGE, B53_PVLAN_PORT(port), pvlan);
	dev->ports[port].vlan_ctl_mask = pvlan;

	pvid = b53_default_pvid(dev);

	/* Make this port join all VLANs without VLAN entries */
	if (is58xx(dev)) {
		b53_read16(dev->sdk_device, B53_VLAN_PAGE, B53_JOIN_ALL_VLAN_EN, &reg);
		reg |= BIT(port);
		if (!(reg & BIT(cpu_port)))
			reg |= BIT(cpu_port);
		b53_write16(dev->sdk_device, B53_VLAN_PAGE, B53_JOIN_ALL_VLAN_EN, reg);
	} else {
		b53_get_vlan_entry(dev->sdk_device, pvid, vl);
		vl->members |= BIT(port) | BIT(cpu_port);
		vl->untag |= BIT(port) | BIT(cpu_port);
		b53_set_vlan_entry(dev->sdk_device, pvid, vl);
	}
}
#endif
