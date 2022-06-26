/*
 * b53_driver.h
 *
 *  Created on: May 2, 2019
 *      Author: zhurish
 */

#ifndef __B53_DRIVER_H__
#define __B53_DRIVER_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include "sdk_driver.h"
#include "b53_mdio.h"
#include "b53_global.h"
#include "b53_mac.h"
#ifdef ZPL_SDK_USER
/*BCM53125SKMMLG*/
/*********************************************************************************/
#define B53_DEVICE_NAME	"b53mdio0"
#define B53_INVALID_LANE	0xff
#else

#include "bsp_include.h"
#endif

#ifndef BIT
//#define BIT(n)		(1)<<(n)
#define BIT(nr) (UL(1) << (nr))
#endif

enum {

	BCM53115_DEVICE_ID = 0x53115,
	BCM53125_DEVICE_ID = 0x53125,
	BCM53128_DEVICE_ID = 0x53128,
};

#define B53_N_PORTS	9
#define B53_N_PORTS_25	6
/* Duplex, half or full. */
#define DUPLEX_HALF		0x00
#define DUPLEX_FULL		0x01
#define DUPLEX_UNKNOWN		0xff

#define SPEED_10		10
#define SPEED_100		100
#define SPEED_1000		1000
#define SPEED_2500		2500
#define SPEED_5000		5000
#define SPEED_10000		10000
#define SPEED_14000		14000
#define SPEED_20000		20000
#define SPEED_25000		25000
#define SPEED_40000		40000
#define SPEED_50000		50000
#define SPEED_56000		56000
#define SPEED_100000		100000
#define SPEED_200000		200000

#define SPEED_UNKNOWN		-1



/*********************************************************************************/

#define B53_CPU_PORT	8


typedef struct b53125_device 
{
	struct b53_mdio_device mido;
	u32 chip_id;
	u8 core_rev;
	u8 vta_regs[3];

	u8 duplex_reg;
	u8 jumbo_pm_reg;
	u8 jumbo_size_reg;

	u8 num_arl_entries;
	u8 num_arl_bins;
	u16 num_arl_buckets;

	zpl_phyport_t cpu_port;
	zpl_uint32 num_vlans;
	zpl_phyport_t num_ports;
}b53_device_t;

typedef struct b53_vlan_s {
	zpl_uint16 tag;
	zpl_uint16 untag;
	bool valid;
}b53_vlan_t;

struct b53_mib_stats
{
	zpl_ulong TxOctets;
	zpl_uint32 TxDropPkts;
	zpl_uint32 TxBroadcastPkts;
	zpl_uint32 TxMulticastPkts;
	zpl_uint32 TxUnicastPkts;
	zpl_uint32 TxCollisions;
	zpl_uint32 TxSingleCollision;
	zpl_uint32 TxMultipleCollision;
	zpl_uint32 TxDeferredTransmit;
	zpl_uint32 TxLateCollision;
	zpl_uint32 TxExcessiveCollision;
	zpl_uint32 TxPausePkts;
	zpl_ulong RxOctets;
	zpl_uint32 RxUndersizePkts;
	zpl_uint32 RxPausePkts;
	zpl_uint32 Pkts64Octets;
	zpl_uint32 Pkts65to127Octets;
	zpl_uint32 Pkts128to255Octets;
	zpl_uint32 Pkts256to511Octets;
	zpl_uint32 Pkts512to1023Octets;
	zpl_uint32 Pkts1024to1522Octets;
	zpl_uint32 RxOversizePkts;
	zpl_uint32 RxJabbers;
	zpl_uint32 RxAlignmentErrors;
	zpl_uint32 RxFCSErrors;
	zpl_ulong RxGoodOctets;
	zpl_uint32 RxDropPkts;
	zpl_uint32 RxUnicastPkts;
	zpl_uint32 RxMulticastPkts;
	zpl_uint32 RxBroadcastPkts;
	zpl_uint32 RxSAChanges;
	zpl_uint32 RxFragments;
	zpl_uint32 RxJumboPkts;
	zpl_uint32 RxSymbolErrors;
	zpl_uint32 RxDiscarded;
};

struct b53125_mac_arl_entry {
	u8 port;
	u8 mac[ETH_ALEN];
	u16 vid;
	u8 is_valid:1;
	u8 is_age:1;
	u8 is_static:1;
};

typedef int (*mac_arl_entry_cb)(struct b53125_mac_arl_entry *, void *);

#define b53_build_op(type_op_size, val_type)				\
static inline int b53125_##type_op_size(b53_device_t *dev, u8 page,	\
				     u8 reg, val_type val)		\
{									\
	int ret;							\
	ret = ((struct b53_mdio_ops*)(dev->mido.ops))->type_op_size(&dev->mido, page, reg, val);		\
	return ret;							\
}


b53_build_op(read8, u8 *);
b53_build_op(read16, u16 *);
b53_build_op(read32, u32 *);
b53_build_op(read48, u64 *);
b53_build_op(read64, u64 *);

b53_build_op(write8, u8);
b53_build_op(write16, u16);
b53_build_op(write32, u32);
b53_build_op(write48, u64);
b53_build_op(write64, u64);



static inline int is531x5(struct b53125_device *dev)
{
	return dev->chip_id == BCM53115_DEVICE_ID ||
		dev->chip_id == BCM53125_DEVICE_ID ||
		dev->chip_id == BCM53128_DEVICE_ID;
}


extern struct b53125_device * b53125_device_probe(void);
extern int b53125_device_exit(struct b53125_device *);

extern int b53125_config_start(sdk_driver_t *dev);
/*******global *******/
extern int b53125_range_error(sdk_driver_t *dev, zpl_bool enable);

extern int b53125_multicast_forward(sdk_driver_t *dev, u8 *mac, zpl_bool enable);
extern int b53125_global_init(sdk_driver_t *dev);
extern int b53125_global_start(sdk_driver_t *dev);

/*******snoop *******/
extern int b53125_snooping_init(sdk_driver_t *dev);


/******* IMP PORT *******/
extern int b53_brcm_hdr_setup(sdk_driver_t *dev, zpl_bool enable, zpl_phyport_t port);
extern int b53125_imp_enable(sdk_driver_t *dev, zpl_bool enable);
extern int b53125_imp_speed(sdk_driver_t *dev, int speed);
extern int b53125_imp_duplex(sdk_driver_t *dev, int duplex);
extern int b53125_imp_flow(sdk_driver_t *dev, zpl_bool rx, zpl_bool tx);
extern int b53125_imp_port_enable(sdk_driver_t *dev);

extern int b53125_cpu_init(sdk_driver_t *dev);

/******* PORT *******/

extern int b53125_unknow_unicast_forward_port(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable);
extern int b53125_unknow_multicast_forward_port(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable);
extern int b53125_unknow_ipmulticast_forward_port(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable);
extern int b53125_enable_learning(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable);
extern int b53125_port_init(sdk_driver_t *dev);
extern int b53125_port_start(sdk_driver_t *dev);



/******* qos *******/
extern int b53125_qos_init(sdk_driver_t *dev);
extern int b53125_qos_class4_weight(sdk_driver_t *dev, zpl_bool strict, int weight);

/******* rate *******/
extern int b53125_qos_cpu_rate(sdk_driver_t *dev, int rate);
extern int b53125_rate_default(sdk_driver_t *dev);
extern int b53125_strom_rate(sdk_driver_t *dev, zpl_phyport_t port,zpl_uint32 mode, zpl_uint32 cnt, zpl_uint32 type);
extern int b53125_ingress_rate(sdk_driver_t *dev, zpl_phyport_t port, int cnt);
extern int b53125_egress_rate(sdk_driver_t *dev, zpl_phyport_t port, int rate);


/******* STP *******/
extern int b53125_mstp_init(sdk_driver_t *dev);

/******* TRUNK *******/
extern int b53125_trunk_init(sdk_driver_t *dev);


/******* VLAN *******/
extern int b53125_vlan_init(sdk_driver_t *dev);
extern int b53125_port_vlan(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable);
extern void b53125_imp_vlan_setup(sdk_driver_t *dev, int cpu_port);
extern int b53125_enable_vlan_default(sdk_driver_t *dev, zpl_bool enable);
/******* mirror *******/
extern int b53125_mirror_init(sdk_driver_t *dev);

/******* MAC *******/
extern int b53125_clear_mac_all(sdk_driver_t *dev);
extern int b53125_mac_init(sdk_driver_t *dev);
extern int b53125_mac_address_clr(sdk_driver_t *dev, zpl_phyport_t phyport, 
	zpl_vlan_t vlanid, zpl_uint32 vrfid);
extern int b53125_clear_mac_tbl_vlan(sdk_driver_t *dev, vlan_t vid);
/******* DOS *******/
extern int b53125_dos_disable_lean(sdk_driver_t *dev, zpl_bool enable);
extern int b53125_dos_init(sdk_driver_t *dev);


/******* EAP *******/
extern int b53125_eap_init(sdk_driver_t *dev);

/******* phy *******/
extern int b53125_phy_loopback(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable);
extern int b53125_phy_powerdown(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable);

/*********************************************************************************/




#ifdef __cplusplus
}
#endif

#endif /* __B53_DRIVER_H__ */
