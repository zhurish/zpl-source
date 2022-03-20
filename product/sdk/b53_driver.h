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
#include "b53_regs.h"
/*********************************************************************************/
#define B53_DEVICE_NAME	"b53mdio0"
#define B53_INVALID_LANE	0xff

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


#define BR_STATE_DISABLED 0
#define BR_STATE_LISTENING 1
#define BR_STATE_LEARNING 2
#define BR_STATE_FORWARDING 3
#define BR_STATE_BLOCKING 4
/*********************************************************************************/
#define B53_CPU_PORT_25	5
#define B53_CPU_PORT	8


typedef struct b53125_device 
{
	struct b53_mdio_device mido;

	/* chip specific data */
	u32 chip_id;
	u32 core_rev;
	u8 vta_regs[3];
	u8 duplex_reg;
	u8 jumbo_pm_reg;
	u8 jumbo_size_reg;

	u8 num_arl_entries;
	u8 num_arl_bins;
	u16 num_arl_buckets;
	/* used ports mask */
	u16 enabled_ports;
	zpl_phyport_t cpu_port;

	/* run time configuration */
	zpl_bool enable_jumbo;

	zpl_uint32 num_vlans;
	zpl_bool vlan_enabled;
	zpl_bool vlan_filtering_enabled;
	zpl_phyport_t num_ports;
}b53_device_t;



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


#define b53125_for_each_port(dev, i) \
	for (i = 0; i < B53_N_PORTS; i++) \
		if (dev->enabled_ports & BIT(i))


static inline int is531x5(struct b53125_device *dev)
{
	return dev->chip_id == BCM53115_DEVICE_ID ||
		dev->chip_id == BCM53125_DEVICE_ID ||
		dev->chip_id == BCM53128_DEVICE_ID;
}


extern struct b53125_device * b53125_device_probe(void);
extern int b53125_config_init(sdk_driver_t *dev);

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
extern int b53125_strom_rate(sdk_driver_t *dev, zpl_phyport_t port,int type, int cnt);
extern int b53125_ingress_rate(sdk_driver_t *dev, zpl_phyport_t port, int cnt);
extern int b53125_egress_rate(sdk_driver_t *dev, zpl_phyport_t port, int rate);


/******* STP *******/
extern int b53125_mstp_init(sdk_driver_t *dev);

/******* TRUNK *******/
extern int b53125_trunk_init(sdk_driver_t *dev);


/******* VLAN *******/
extern int b53125_vlan_init(sdk_driver_t *dev);
extern int b53125_port_vlan(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable);

/******* mirror *******/
extern int b53125_mirror_init(sdk_driver_t *dev);

/******* MAC *******/

extern int b53125_mac_init(sdk_driver_t *dev);

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
