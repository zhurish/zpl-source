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
extern void b53_brcm_hdr_setup(sdk_driver_t *dev, zpl_bool enable, zpl_phyport_t port);
extern int b53125_switch_manege(sdk_driver_t *dev, zpl_bool manege);
extern int b53125_switch_forwarding(sdk_driver_t *dev, zpl_bool enable);
extern int b53125_multicast_flood(sdk_driver_t *dev, zpl_bool enable);
extern int b53125_unicast_flood(sdk_driver_t *dev, zpl_bool enable);
extern int b53125_range_error(sdk_driver_t *dev, zpl_bool enable);
extern int b53125_multicast_learning(sdk_driver_t *dev, zpl_bool enable);

extern int b53125_enable_bpdu(sdk_driver_t *dev, zpl_bool enable);
extern int b53125_aging_time(sdk_driver_t *dev, int agetime);
extern int b53125_multicast_forward(sdk_driver_t *dev, u8 *mac, zpl_bool enable);

int b53125_mldqry_snoop_enable(sdk_driver_t *dev, zpl_bool enable);
int b53125_mldqry_proxy_enable(sdk_driver_t *dev, zpl_bool enable);

int b53125_mld_snoop_enable(sdk_driver_t *dev, zpl_bool enable);
int b53125_mld_proxy_enable(sdk_driver_t *dev, zpl_bool enable);

int b53125_igmpunknow_snoop_enable(sdk_driver_t *dev, zpl_bool enable);
int b53125_igmpunknow_proxy_enable(sdk_driver_t *dev, zpl_bool enable);

int b53125_igmpqry_snoop_enable(sdk_driver_t *dev, zpl_bool enable);
int b53125_igmpqry_proxy_enable(sdk_driver_t *dev, zpl_bool enable);

int b53125_igmp_snoop_enable(sdk_driver_t *dev, zpl_bool enable);
int b53125_igmp_proxy_enable(sdk_driver_t *dev, zpl_bool enable);


int b53125_igmp_ipcheck_enable(sdk_driver_t *dev, zpl_bool enable);

int b53125_arp_copycpu_enable(sdk_driver_t *dev, zpl_bool enable);
int b53125_rarp_copycpu_enable(sdk_driver_t *dev, zpl_bool enable);
int b53125_dhcp_copycpu_enable(sdk_driver_t *dev, zpl_bool enable);


int b53125_port_wan_enable(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable);
/******* IMP PORT *******/
extern int b53125_imp_enable(sdk_driver_t *dev, zpl_bool enable);
extern int b53125_imp_speed(sdk_driver_t *dev, int speed);
extern int b53125_imp_duplex(sdk_driver_t *dev, int duplex);
extern int b53125_imp_flow(sdk_driver_t *dev, zpl_bool rx, zpl_bool tx);
extern int b53125_imp_port_enable(sdk_driver_t *dev);

/******* PORT *******/
extern int b53125_pasue_transmit_enable(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable);
extern int b53125_pasue_receive_enable(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable);

extern int b53125_unknow_unicast_forward_port(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable);
extern int b53125_unknow_multicast_forward_port(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable);
extern int b53125_unknow_ipmulticast_forward_port(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable);
extern int b53125_enable_learning(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable);
extern int b53125_port_init(void);

extern int b53125_jumbo_enable(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable);
extern int b53125_jumbo_size(sdk_driver_t *dev, int size);



/******* qos *******/
extern int b53125_qos_init(void);
extern int b53125_qos_aggreation_mode(sdk_driver_t *dev, zpl_bool enable);
extern int b53125_qos_base_port(sdk_driver_t *dev, zpl_bool enable);
extern int b53125_qos_layer_sel(sdk_driver_t *dev, int sel);
extern int b53125_qos_8021p(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable);
extern int b53125_qos_diffserv(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable);

extern int b53125_8021p_map_priority(sdk_driver_t *dev, zpl_phyport_t port, int p, int priority);
extern int b53125_diffserv_map_priority(sdk_driver_t *dev, int diffserv, int priority);
extern int b53125_tc_map_queue(sdk_driver_t *dev, int priority, int queue);
//设置进入CPU报文到队列的映射
extern int b53125_qos_cpu_map_queue(sdk_driver_t *dev, int traffic, zpl_bool enable);
//设置queue调度方式
extern int b53125_queue_scheduling(sdk_driver_t *dev, int mode);
 //设置queue调度权限
extern int b53125_queue_weight(sdk_driver_t *dev, int queue, int weight);

extern int b53125_qos_class4_weight(sdk_driver_t *dev, zpl_bool strict, int weight);

//CPU接口限速
extern int b53125_qos_cpu_rate(sdk_driver_t *dev, int rate);
extern int b53125_rate_default(sdk_driver_t *dev);
extern int b53125_strom_rate(sdk_driver_t *dev, zpl_phyport_t port,int type, int cnt);
extern int b53125_ingress_rate(sdk_driver_t *dev, zpl_phyport_t port, int cnt);
extern int b53125_egress_rate(sdk_driver_t *dev, zpl_phyport_t port, int rate);

extern int b53125_qos_cfi_remarking(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable);
extern int b53125_qos_pcp_remarking(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable);
extern int b53125_priority_remarking(sdk_driver_t *dev, zpl_phyport_t port, zpl_uint32 tc, zpl_uint32 priority);


/******* STP *******/
extern int b53125_mstp_init(void);

/******* TRUNK *******/
extern int b53_trunk_init(void);


/******* VLAN *******/
extern int b53_vlan_init(void);
extern int b53125_port_vlan(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable);

extern int b53125_vlan_mstp_id(sdk_driver_t *dev, vlan_t vid, int id);

/******* mirror *******/
extern int b53125_mirror_init(void);

/******* MAC *******/
int b53125_mac_address_add(sdk_driver_t *dev, zpl_phyport_t phyport, 
	zpl_vlan_t vlanid, zpl_uint32 vrfid, mac_t *mac, zpl_uint32 pri);
int b53125_mac_address_del(sdk_driver_t *dev, zpl_phyport_t phyport, 
	zpl_vlan_t vlanid, zpl_uint32 vrfid, mac_t *mac, zpl_uint32 pri);
int b53125_mac_address_read(sdk_driver_t *dev, zpl_phyport_t phyport, 
	zpl_vlan_t vlanid, zpl_uint32 vrfid);


/******* DOS *******/
extern int b53125_dos_disable_lean(sdk_driver_t *dev, zpl_bool enable);
extern int b53125_dos_init(void);


/******* EAP *******/
extern int b53125_eap_bypass_ipaddr_set(sdk_driver_t *dev, zpl_uint32 index, u32 address, u32 mask);
extern int b53125_eap_init(void);


extern int b53125_phy_loopback(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable);
extern int b53125_phy_powerdown(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable);
extern int b53125_snooping_enable(sdk_driver_t *dev, zpl_uint32 type, zpl_bool enable);
/*********************************************************************************/




#ifdef __cplusplus
}
#endif

#endif /* __B53_DRIVER_H__ */
