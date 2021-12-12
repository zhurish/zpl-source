/*
 * b53_mdio.h
 *
 *  Created on: May 1, 2019
 *      Author: zhurish
 */

#ifndef __B53_MDIO_H__
#define __B53_MDIO_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include "zpl_include.h"
#include "log.h"

typedef zpl_uint8 u8;
typedef zpl_uint16 u16;
typedef zpl_uint32 u32;
typedef zpl_uint64 u64;

struct b53125_device;

#define BIT(n)	((1)<<(n))
#pragma pack(1)
struct mido_data_b53
{
	int mii_id;
	int regnum;
	char len;
	u16 value;
};
#pragma pack(0)


/*********************************************************************************/
#define B53_DEVICE_NAME	"mdio-b530"
/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
#define B53_IO	's'
#define B53_IO_CTL	_IO(B53_IO, 0x01)
#define B53_IO_W	_IOW(B53_IO, 0x02, struct mido_data_b53)
#define B53_IO_R	_IOR(B53_IO, 0x03, struct mido_data_b53)
#define B53_IO_WR	_IOWR(B53_IO, 0x04, struct mido_data_b53)

/*********************************************************************************/
/*********************************************************************************/
/* MII registers */
#define REG_MII_PAGE    0x10    /* MII Page register */
#define REG_MII_ADDR    0x11    /* MII Address register */
#define REG_MII_DATA0   0x18    /* MII Data register 0 */
#define REG_MII_DATA1   0x19    /* MII Data register 1 */
#define REG_MII_DATA2   0x1a    /* MII Data register 2 */
#define REG_MII_DATA3   0x1b    /* MII Data register 3 */

#define REG_MII_PAGE_ENABLE     BIT(0)
#define REG_MII_ADDR_WRITE      BIT(0)
#define REG_MII_ADDR_READ       BIT(1)
/*********************************************************************************/
#define BRCM_PSEUDO_PHY_ADDR           30
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

enum {
	MLO_PAUSE_NONE,
	MLO_PAUSE_ASYM = BIT(0),
	MLO_PAUSE_SYM = BIT(1),
	MLO_PAUSE_RX = BIT(2),
	MLO_PAUSE_TX = BIT(3),
	MLO_PAUSE_TXRX_MASK = MLO_PAUSE_TX | MLO_PAUSE_RX,
	MLO_PAUSE_AN = BIT(4),

	MLO_AN_PHY = 0,	/* Conventional PHY */
	MLO_AN_FIXED,	/* Fixed-link mode */
	MLO_AN_INBAND,	/* In-band protocol */
};

#define BR_STATE_DISABLED 0
#define BR_STATE_LISTENING 1
#define BR_STATE_LEARNING 2
#define BR_STATE_FORWARDING 3
#define BR_STATE_BLOCKING 4
/*********************************************************************************/

struct b53_io_ops {
	int (*read8)(struct b53125_device *dev, u8 page, u8 reg, u8 *value);
	int (*read16)(struct b53125_device *dev, u8 page, u8 reg, u16 *value);
	int (*read32)(struct b53125_device *dev, u8 page, u8 reg, u32 *value);
	int (*read48)(struct b53125_device *dev, u8 page, u8 reg, u64 *value);
	int (*read64)(struct b53125_device *dev, u8 page, u8 reg, u64 *value);
	int (*write8)(struct b53125_device *dev, u8 page, u8 reg, u8 value);
	int (*write16)(struct b53125_device *dev, u8 page, u8 reg, u16 value);
	int (*write32)(struct b53125_device *dev, u8 page, u8 reg, u32 value);
	int (*write48)(struct b53125_device *dev, u8 page, u8 reg, u64 value);
	int (*write64)(struct b53125_device *dev, u8 page, u8 reg, u64 value);
};

#define B53_INVALID_LANE	0xff

enum {

	BCM53115_DEVICE_ID = 0x53115,
	BCM53125_DEVICE_ID = 0x53125,
	BCM53128_DEVICE_ID = 0x53128,
};

#define B53_N_PORTS	9
#define B53_N_PORTS_25	6


struct b53125_device {
	int		fd;
	//struct mutex reg_mutex;

	const struct b53_io_ops *ops;

	/* chip specific data */
	u32 chip_id;
	u32 core_rev;
	u8 vta_regs[3];
	u8 duplex_reg;
	u8 jumbo_pm_reg;
	u8 jumbo_size_reg;

	u8 num_arl_entries;

	/* used ports mask */
	u16 enabled_ports;
	zpl_uint32 cpu_port;

	/* connect specific data */
	u8 reg_page;

	/* run time configuration */
	zpl_bool enable_jumbo;

	zpl_uint32 num_vlans;
	zpl_bool vlan_enabled;
	zpl_bool vlan_filtering_enabled;
	zpl_uint32 num_ports;
};

#define b53125_for_each_port(dev, i) \
	for (i = 0; i < B53_N_PORTS; i++) \
		if (dev->enabled_ports & BIT(i))


static inline int is531x5(struct b53125_device *dev)
{
	return dev->chip_id == BCM53115_DEVICE_ID ||
		dev->chip_id == BCM53125_DEVICE_ID ||
		dev->chip_id == BCM53128_DEVICE_ID;
}

#define B53_CPU_PORT_25	5
#define B53_CPU_PORT	8


#define b53_build_op(type_op_size, val_type)				\
static inline int b53125_##type_op_size(struct b53125_device *dev, u8 page,	\
				     u8 reg, val_type val)		\
{									\
	int ret;							\
	ret = dev->ops->type_op_size(dev, page, reg, val);		\
	return ret;							\
}

/*static inline int b53125_read8(struct b53125_device *dev, u8 page, u8 reg, u8 *val)
{
	int ret;
	ret = dev->ops->read8(dev, page, reg, val);
	return ret;
}*/
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



#define ETH_ALEN	6

struct b53125_mac_arl_entry {
	u8 port;
	u8 mac[ETH_ALEN];
	u16 vid;
	u8 is_valid:1;
	u8 is_age:1;
	u8 is_static:1;
};
/**
 * ether_addr_to_u64 - Convert an Ethernet address into a u64 value.
 * @addr: Pointer to a six-byte array containing the Ethernet address
 *
 * Return a u64 value of the address
 */
static inline u64 ether_addr_to_u64(const u8 *addr)
{
	u64 u = 0;
	zpl_uint32 i;

	for (i = 0; i < ETH_ALEN; i++)
		u = u << 8 | addr[i];

	return u;
}

/**
 * u64_to_ether_addr - Convert a u64 to an Ethernet address.
 * @u: u64 to convert to an Ethernet MAC address
 * @addr: Pointer to a six-byte array to contain the Ethernet address
 */
static inline void u64_to_ether_addr(u64 u, u8 *addr)
{
	zpl_uint32 i;

	for (i = ETH_ALEN - 1; i >= 0; i--) {
		addr[i] = u & 0xff;
		u = u >> 8;
	}
}


//extern struct b53125_device *b53_device;

extern struct b53125_device * b53125_mdio_probe();
extern int b53125_config_init(struct b53125_device *dev);

/*******global *******/
void b53_brcm_hdr_setup(struct b53125_device *dev, zpl_bool enable, int port);
int b53125_switch_manege(struct b53125_device *dev, zpl_bool manege);
int b53125_switch_forwarding(struct b53125_device *dev, zpl_bool enable);
int b53125_multicast_flood(struct b53125_device *dev, zpl_bool enable);
int b53125_unicast_flood(struct b53125_device *dev, zpl_bool enable);
int b53125_range_error(struct b53125_device *dev, zpl_bool enable);
int b53125_multicast_learning(struct b53125_device *dev, zpl_bool enable);
int b53125_puase_frame_detection(struct b53125_device *dev, zpl_bool enable);
int b53125_enable_bpdu(struct b53125_device *dev, zpl_bool enable);
int b53125_aging_time(struct b53125_device *dev, int agetime);
int b53125_imp_port_enable(struct b53125_device *dev);

/******* IMP PORT *******/
int b53125_imp_enable(struct b53125_device *dev, zpl_bool enable);
int b53125_imp_speed(struct b53125_device *dev, int speed);
int b53125_imp_duplex(struct b53125_device *dev, int duplex);
int b53125_imp_flow(struct b53125_device *dev, zpl_bool rx, zpl_bool tx);

/******* PORT *******/
int b53125_port_enable(struct b53125_device *dev, int port, zpl_bool enable);
int b53125_port_protected_enable(struct b53125_device *dev, int port, zpl_bool enable);
int b53125_port_wan_enable(struct b53125_device *dev, int port, zpl_bool enable);

int b53125_pasue_transmit_enable(struct b53125_device *dev, int port, zpl_bool enable);
int b53125_pasue_receive_enable(struct b53125_device *dev, int port, zpl_bool enable);


int b53125_bpdu_forward(struct b53125_device *dev, u8 *mac, zpl_bool enable);
int b53125_unknow_unicast_forward_port(struct b53125_device *dev, int port, zpl_bool enable);
int b53125_unknow_multicast_forward_port(struct b53125_device *dev, int port, zpl_bool enable);
int b53125_unknow_ipmulticast_forward_port(struct b53125_device *dev, int port, zpl_bool enable);

int b53125_pause_pass_rx(struct b53125_device *dev, int port, zpl_bool enable);
int b53125_pause_pass_tx(struct b53125_device *dev, int port, zpl_bool enable);
int b53125_enable_learning(struct b53125_device *dev, int port, zpl_bool enable);
int b53125_software_learning(struct b53125_device *dev, int port, zpl_bool enable);

int b53125_port_set_speed(struct b53125_device *dev, int port, int speed);
int b53125_port_set_duplex(struct b53125_device *dev, int port, int duplex);
int b53125_port_set_flow(struct b53125_device *dev, int port, int flow);
int b53125_port_set_link_force(struct b53125_device *dev, int port, int link);



zpl_bool b53125_port_get_link(struct b53125_device *dev, int port);
zpl_uint32 b53125_port_get_speed(struct b53125_device *dev, int port);
zpl_uint32 b53125_port_get_duplex(struct b53125_device *dev, int port);
/******* qos *******/

int b53125_qos_aggreation_mode(struct b53125_device *dev, zpl_bool enable);
int b53125_qos_base_port(struct b53125_device *dev, zpl_bool enable);
int b53125_qos_layer_sel(struct b53125_device *dev, int sel);
int b53125_qos_8021p(struct b53125_device *dev, int port, zpl_bool enable);
int b53125_qos_diffserv(struct b53125_device *dev, int port, zpl_bool enable);
int b53125_qos_port_map_queue(struct b53125_device *dev, int port, int p, int queue);

int b53125_qos_diffserv_map_queue(struct b53125_device *dev, int diffserv, int queue);
int b53125_qos_queue_map_class(struct b53125_device *dev, int queue, int class);
int b53125_qos_class_scheduling(struct b53125_device *dev, int mode);
int b53125_qos_class_weight(struct b53125_device *dev, int class, int weight);
int b53125_qos_cpu_map_queue(struct b53125_device *dev, int traffic, zpl_bool enable);

int b53125_qos_class4_weight(struct b53125_device *dev, zpl_bool strict, int weight);
int b53125_qos_ingress_ipg(struct b53125_device *dev, zpl_bool tx, zpl_bool enable);

int b53125_qos_buck_mode(struct b53125_device *dev, int id, zpl_bool mode);
int b53125_qos_buck_type(struct b53125_device *dev, int id, zpl_uint32 type);
int b53125_qos_ingress_rate_mode(struct b53125_device *dev, int port,  zpl_uint32 type, zpl_bool enable);
int b53125_qos_ingress_rate(struct b53125_device *dev, int port, zpl_uint32 index, int bucket, int cnt);
int b53125_qos_egress_rate(struct b53125_device *dev, int port, int bucket, int cnt);
int b53125_qos_cpu_rate(struct b53125_device *dev, int rate);
int b53125_qos_cfi_remarking(struct b53125_device *dev, int port, zpl_bool enable);
int b53125_qos_pcp_remarking(struct b53125_device *dev, int port, zpl_bool enable);

int b53125_jumbo_enable(struct b53125_device *dev, int port, zpl_bool enable);
int b53125_jumbo_size(struct b53125_device *dev, int size);

/******* STP *******/

int b53125_set_stp_state(struct b53125_device *dev, int port, u8 state);
int b53125_mstp_enable(struct b53125_device *dev, zpl_bool enable);
int b53125_mstp_aging_time(struct b53125_device *dev, int aging);
int b53125_mstp_state(struct b53125_device *dev, int id, int port, int state);
int b53125_mstp_bypass(struct b53125_device *dev, int id, zpl_bool enable);
int b53125_vlan_mstp_id(struct b53125_device *dev, u16 vid, int id);

/******* TRUNK *******/

int b53125_trunk_mac_base_enable(struct b53125_device *dev, zpl_bool enable);
int b53125_trunk_mode(struct b53125_device *dev, int mode);
int b53125_trunk_add(struct b53125_device *dev, int id, int port);
int b53125_trunk_del(struct b53125_device *dev, int id, int port);


/******* VLAN *******/

int b53125_enable_vlan(struct b53125_device *dev, zpl_bool enable,
		zpl_bool enable_filtering);

int b53125_add_vlan_entry(struct b53125_device *dev, u16 vid);
int b53125_del_vlan_entry(struct b53125_device *dev, u16 vid);
int b53125_add_vlan_port(struct b53125_device *dev, u16 vid, int port, zpl_bool tag);
int b53125_del_vlan_port(struct b53125_device *dev, u16 vid, int port, zpl_bool tag);
int b53125_port_vlan(struct b53125_device *dev, int port, zpl_bool enable);
int b53125_port_pvid(struct b53125_device *dev, u16 vid, int port, int pri);
int b53125_vlan_double_tagging_tpid(struct b53125_device *dev, u16 tpid);
int b53125_ISP_port(struct b53125_device *dev, int port, zpl_bool enable);


/******* mirror *******/
int b53125_mirror_enable(struct b53125_device *dev, zpl_bool enable);
int b53125_mirror_destination_set(struct b53125_device *dev, int port);
int b53125_mirror_ingress_mac(struct b53125_device *dev, u8 *mac);
int b53125_mirror_egress_mac(struct b53125_device *dev, u8 *mac);
int b53125_mirror_ingress_div(struct b53125_device *dev, u16 div);
int b53125_mirror_egress_div(struct b53125_device *dev, u16 div);
int b53125_mirror_ingress_source(struct b53125_device *dev, int port);
int b53125_mirror_egress_source(struct b53125_device *dev, int port);
int b53125_mirror_ingress_filter(struct b53125_device *dev, zpl_uint32 type);
int b53125_mirror_egress_filter(struct b53125_device *dev, zpl_uint32 type);


/******* MAC *******/
int b53125_clear_mac_tbl_port(struct b53125_device *dev, int port);
int b53125_clear_mac_tbl_vlan(struct b53125_device *dev, u16 vid);
int b53125_mac_tbl_add(struct b53125_device *dev, int port,
		const zpl_uint8 *addr, u16 vid);
int b53125_mac_tbl_del(struct b53125_device *dev, int port,
		const zpl_uint8 *addr, u16 vid);
int b53125_fdb_dump(struct b53125_device *priv, int port,
		int (*cb)(u8 *, u16, zpl_bool, void *), void *data);

/******* DOS *******/
int b53125_dos_disable_lean(struct b53125_device *dev, zpl_bool enable);
int b53125_dos_icmp4_maxsize(struct b53125_device *dev, int minsize);
int b53125_dos_icmp6_maxsize(struct b53125_device *dev, int minsize);
int b53125_dos_tcphdr_minsize(struct b53125_device *dev, int minsize);
int b53125_dos_ip_lan_drip_drop(struct b53125_device *dev, zpl_bool enable);
int b53125_dos_tcp_blat_drop(struct b53125_device *dev, zpl_bool enable);
int b53125_dos_udp_blat_drop(struct b53125_device *dev, zpl_bool enable);
int b53125_dos_tcp_nullscan_drop(struct b53125_device *dev, zpl_bool enable);
int b53125_dos_tcp_xmassscan_drop(struct b53125_device *dev, zpl_bool enable);
int b53125_dos_tcp_synfinscan_drop(struct b53125_device *dev, zpl_bool enable);
int b53125_dos_tcp_synerror_drop(struct b53125_device *dev, zpl_bool enable);
int b53125_dos_tcp_zpl_int16hdr_drop(struct b53125_device *dev, zpl_bool enable);
int b53125_dos_tcp_fragerror_drop(struct b53125_device *dev, zpl_bool enable);
int b53125_dos_icmpv4_fragment_drop(struct b53125_device *dev, zpl_bool enable);
int b53125_dos_icmpv6_fragment_drop(struct b53125_device *dev, zpl_bool enable);
int b53125_dos_icmpv4_longping_drop(struct b53125_device *dev, zpl_bool enable);
int b53125_dos_icmpv6_longping_drop(struct b53125_device *dev, zpl_bool enable);


/******* EAP *******/
int b53125_eap_enable(struct b53125_device *dev, zpl_bool enable);
int b53125_eap_ip_address_bypass_enable(struct b53125_device *dev, zpl_bool enable);
int b53125_eap_mult_address_bypass_enable(struct b53125_device *dev, zpl_bool enable, u32 address);
int b53125_eap_ip_address_set(struct b53125_device *dev, zpl_uint32 index, u32 address, u32 mask);
int b53125_eap_mode_set(struct b53125_device *dev, int port, u32 mode);
int b53125_eap_blk_mode_set(struct b53125_device *dev, int port, u32 mode);



int b53125_phy_loopback(struct b53125_device *dev, int port, zpl_bool enable);
int b53125_snooping_enable(struct b53125_device *dev, zpl_uint32 type, zpl_bool enable);
/*********************************************************************************/

 
#ifdef __cplusplus
}
#endif

#endif /* __B53_MDIO_H__ */
