/*
 * b53_mac_tbl.c
 *
 *  Created on: May 3, 2019
 *      Author: zhurish
 */

#include <zebra.h>
#include "b53_mdio.h"
#include "b53_regs.h"
#include "b53_driver.h"

/****************************************************************************************/
static int b53125_flush_mac_tbl(struct b53125_device *dev, u8 mask)
{
	unsigned int i;
	b53125_write8(dev, B53_CTRL_PAGE, B53_FAST_AGE_CTRL,
		   FAST_AGE_DONE | FAST_AGE_DYNAMIC | mask);
	for (i = 0; i < 1000; i++) {
		u8 fast_age_ctrl;
		b53125_read8(dev, B53_CTRL_PAGE, B53_FAST_AGE_CTRL, &fast_age_ctrl);
		if (!(fast_age_ctrl & FAST_AGE_DONE))
			goto out;
		os_msleep(1);
	}
	return OS_TIMEOUT;
out:
	/* Only age dynamic entries (default behavior) */
	b53125_write8(dev, B53_CTRL_PAGE, B53_FAST_AGE_CTRL, FAST_AGE_DYNAMIC);
	return OK;
}

int b53125_clear_mac_tbl_port(struct b53125_device *dev, int port)
{
	b53125_write8(dev, B53_CTRL_PAGE, B53_FAST_AGE_PORT_CTRL, port);

	return b53125_flush_mac_tbl(dev, FAST_AGE_PORT);
}

int b53125_clear_mac_tbl_vlan(struct b53125_device *dev, u16 vid)
{
	b53125_write16(dev, B53_CTRL_PAGE, B53_FAST_AGE_VID_CTRL, vid);
	return b53125_flush_mac_tbl(dev, FAST_AGE_VLAN);
}
/****************************************************************************************/
/****************************************************************************************/
static void b53125_mac_tbl_to_entry(struct b53125_mac_arl_entry *ent,
				    u64 mac_vid, u32 fwd_entry)
{
	memset(ent, 0, sizeof(*ent));
	ent->port = fwd_entry & ARLTBL_DATA_PORT_ID_MASK;
	ent->is_valid = !!(fwd_entry & ARLTBL_VALID);
	ent->is_age = !!(fwd_entry & ARLTBL_AGE);
	ent->is_static = !!(fwd_entry & ARLTBL_STATIC);
	u64_to_ether_addr(mac_vid, ent->mac);
	ent->vid = mac_vid >> ARLTBL_VID_S;
}

static void b53125_mac_tbl_from_entry(u64 *mac_vid, u32 *fwd_entry,
				      const struct b53125_mac_arl_entry *ent)
{
	*mac_vid = ether_addr_to_u64(ent->mac);
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
static int b53125_mac_tbl_op_wait(struct b53125_device *dev)
{
	unsigned int timeout = 1000;
	u8 reg;

	do {
		b53125_read8(dev, B53_ARLIO_PAGE, B53_ARLTBL_RW_CTRL, &reg);
		if (!(reg & ARLTBL_START_DONE))
			return 0;

		os_usleep(2000);
	} while (timeout--);
	return OS_TIMEOUT;
}

static int b53125_mac_tbl_rw_op(struct b53125_device *dev, unsigned int op)
{
	u8 reg;
	if (op > ARLTBL_RW)
		return ERROR;
	b53125_read8(dev, B53_ARLIO_PAGE, B53_ARLTBL_RW_CTRL, &reg);
	reg |= ARLTBL_START_DONE;
	if (op)
		reg |= ARLTBL_RW;
	else
		reg &= ~ARLTBL_RW;
	b53125_write8(dev, B53_ARLIO_PAGE, B53_ARLTBL_RW_CTRL, reg);
	return b53125_mac_tbl_op_wait(dev);
}

static int b53125_mac_tbl_read(struct b53125_device *dev, u64 mac,
			u16 vid, struct b53125_mac_arl_entry *ent, u8 *idx,
			bool is_valid)
{
	unsigned int i;
	int ret = 0;

	ret = b53125_mac_tbl_op_wait(dev);
	if (ret)
		return ret;

	/* Read the bins */
	for (i = 0; i < dev->num_arl_entries; i++) {
		u64 mac_vid;
		u32 fwd_entry;

		b53125_read64(dev, B53_ARLIO_PAGE,
			   B53_ARLTBL_MAC_VID_ENTRY(i), &mac_vid);
		b53125_read32(dev, B53_ARLIO_PAGE,
			   B53_ARLTBL_DATA_ENTRY(i), &fwd_entry);
		b53125_mac_tbl_to_entry(ent, mac_vid, fwd_entry);

		if (!(fwd_entry & ARLTBL_VALID))
			continue;
		if ((mac_vid & ARLTBL_MAC_MASK) != mac)
			continue;
		*idx = i;
	}

	return ERROR;
}

static int b53125_mac_tbl_op(struct b53125_device *dev, int op, int port,
		      const unsigned char *addr, u16 vid, BOOL is_valid)
{
	struct b53125_mac_arl_entry ent;
	u32 fwd_entry;
	u64 mac, mac_vid = 0;
	u8 idx = 0;
	int ret;

	/* Convert the array into a 64-bit MAC */
	mac = ether_addr_to_u64(addr);

	/* Perform a read for the given MAC and VID */
	b53125_write48(dev, B53_ARLIO_PAGE, B53_MAC_ADDR_IDX, mac);
	b53125_write16(dev, B53_ARLIO_PAGE, B53_VLAN_ID_IDX, vid);

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

	memset(&ent, 0, sizeof(ent));
	ent.port = port;
	ent.is_valid = is_valid;
	ent.vid = vid;
	ent.is_static = TRUE;
	memcpy(ent.mac, addr, ETH_ALEN);
	b53125_mac_tbl_from_entry(&mac_vid, &fwd_entry, &ent);

	b53125_write64(dev, B53_ARLIO_PAGE,
		    B53_ARLTBL_MAC_VID_ENTRY(idx), mac_vid);
	b53125_write32(dev, B53_ARLIO_PAGE,
		    B53_ARLTBL_DATA_ENTRY(idx), fwd_entry);

	return b53125_mac_tbl_rw_op(dev, 0);
}

int b53125_mac_tbl_add(struct b53125_device *dev, int port,
		const unsigned char *addr, u16 vid)
{
	return b53125_mac_tbl_op(dev, 0, port, addr, vid, TRUE);
}


int b53125_mac_tbl_del(struct b53125_device *dev, int port,
		const unsigned char *addr, u16 vid)
{
	return b53125_mac_tbl_op(dev, 0, port, addr, vid, FALSE);
}


static int b53125_mac_tbl_search_wait(struct b53125_device *dev)
{
	unsigned int timeout = 1000;
	u8 reg;
	do {
		b53125_read8(dev, B53_ARLIO_PAGE, B53_ARL_SRCH_CTL, &reg);
		if (!(reg & ARL_SRCH_STDN))
			return 0;
		if (reg & ARL_SRCH_VLID)
			return 0;
		os_usleep(2000);
	} while (timeout--);

	return OS_TIMEOUT;
}

static void b53125_mac_tbl_search_rd(struct b53125_device *dev, u8 idx,
			      struct b53125_mac_tbl_entry *ent)
{
	u64 mac_vid;
	u32 fwd_entry;

	b53125_read64(dev, B53_ARLIO_PAGE,
		   B53_ARL_SRCH_RSTL_MACVID(idx), &mac_vid);
	b53125_read32(dev, B53_ARLIO_PAGE,
		   B53_ARL_SRCH_RSTL(idx), &fwd_entry);
	b53125_mac_tbl_to_entry(ent, mac_vid, fwd_entry);
}

static int b53125_fdb_copy(int port, const struct b53125_mac_arl_entry *ent,
			int (*cb)(u8 *, u16, BOOL, void *), void *data)
{
	if (!ent->is_valid)
		return 0;

	if (port != ent->port)
		return 0;

	return cb(ent->mac, ent->vid, ent->is_static, data);
}

int b53125_fdb_dump(struct b53125_device *priv, int port,
		int (*cb)(u8 *, u16, BOOL, void *), void *data)
{
	struct b53125_mac_arl_entry results[2];
	unsigned int count = 0;
	int ret;
	u8 reg;

	/* Start search operation */
	reg = ARL_SRCH_STDN;
	b53125_write8(priv, B53_ARLIO_PAGE, B53_ARL_SRCH_CTL, reg);

	do {
		ret = b53125_mac_tbl_search_wait(priv);
		if (ret)
			return ret;

		b53125_mac_tbl_search_rd(priv, 0, &results[0]);
		ret = b53125_fdb_copy(port, &results[0], cb, data);
		if (ret)
			return ret;

		if (priv->num_arl_entries > 2) {
			b53125_mac_tbl_search_rd(priv, 1, &results[1]);
			ret = b53125_fdb_copy(port, &results[1], cb, data);
			if (ret)
				return ret;

			if (!results[0].is_valid && !results[1].is_valid)
				break;
		}

	} while (count++ < 1024);

	return 0;
}
/****************************************************************************************/
/****************************************************************************************/
#if 0

int b53_br_join(struct dsa_switch *ds, int port, struct net_device *br)
{
	struct b53125_device *dev = ds->priv;
	s8 cpu_port = ds->ports[port].cpu_dp->index;
	u16 pvlan, reg;
	unsigned int i;

	/* Make this port leave the all VLANs join since we will have proper
	 * VLAN entries from now on
	 */
	if (is58xx(dev)) {
		b53_read16(dev, B53_VLAN_PAGE, B53_JOIN_ALL_VLAN_EN, &reg);
		reg &= ~BIT(port);
		if ((reg & BIT(cpu_port)) == BIT(cpu_port))
			reg &= ~BIT(cpu_port);
		b53_write16(dev, B53_VLAN_PAGE, B53_JOIN_ALL_VLAN_EN, reg);
	}

	b53_read16(dev, B53_PVLAN_PAGE, B53_PVLAN_PORT_MASK(port), &pvlan);

	b53_for_each_port(dev, i) {
		if (dsa_to_port(ds, i)->bridge_dev != br)
			continue;

		/* Add this local port to the remote port VLAN control
		 * membership and update the remote port bitmask
		 */
		b53_read16(dev, B53_PVLAN_PAGE, B53_PVLAN_PORT_MASK(i), &reg);
		reg |= BIT(port);
		b53_write16(dev, B53_PVLAN_PAGE, B53_PVLAN_PORT_MASK(i), reg);
		dev->ports[i].vlan_ctl_mask = reg;

		pvlan |= BIT(i);
	}

	/* Configure the local port VLAN control membership to include
	 * remote ports and update the local port bitmask
	 */
	b53_write16(dev, B53_PVLAN_PAGE, B53_PVLAN_PORT_MASK(port), pvlan);
	dev->ports[port].vlan_ctl_mask = pvlan;

	return 0;
}

void b53_br_leave(struct dsa_switch *ds, int port, struct net_device *br)
{
	struct b53125_device *dev = ds->priv;
	struct b53_vlan *vl = &dev->vlans[0];
	s8 cpu_port = ds->ports[port].cpu_dp->index;
	unsigned int i;
	u16 pvlan, reg, pvid;

	b53_read16(dev, B53_PVLAN_PAGE, B53_PVLAN_PORT_MASK(port), &pvlan);

	b53_for_each_port(dev, i) {
		/* Don't touch the remaining ports */
		if (dsa_to_port(ds, i)->bridge_dev != br)
			continue;

		b53_read16(dev, B53_PVLAN_PAGE, B53_PVLAN_PORT_MASK(i), &reg);
		reg &= ~BIT(port);
		b53_write16(dev, B53_PVLAN_PAGE, B53_PVLAN_PORT_MASK(i), reg);
		dev->ports[port].vlan_ctl_mask = reg;

		/* Prevent self removal to preserve isolation */
		if (port != i)
			pvlan &= ~BIT(i);
	}

	b53_write16(dev, B53_PVLAN_PAGE, B53_PVLAN_PORT_MASK(port), pvlan);
	dev->ports[port].vlan_ctl_mask = pvlan;

	pvid = b53_default_pvid(dev);

	/* Make this port join all VLANs without VLAN entries */
	if (is58xx(dev)) {
		b53_read16(dev, B53_VLAN_PAGE, B53_JOIN_ALL_VLAN_EN, &reg);
		reg |= BIT(port);
		if (!(reg & BIT(cpu_port)))
			reg |= BIT(cpu_port);
		b53_write16(dev, B53_VLAN_PAGE, B53_JOIN_ALL_VLAN_EN, reg);
	} else {
		b53_get_vlan_entry(dev, pvid, vl);
		vl->members |= BIT(port) | BIT(cpu_port);
		vl->untag |= BIT(port) | BIT(cpu_port);
		b53_set_vlan_entry(dev, pvid, vl);
	}
}
#endif
