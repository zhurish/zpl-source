#ifndef _BCM53125_SDK_H
#define _BCM53125_SDK_H

typedef u_int8_t u8;
typedef u_int16_t u16;
typedef uint32_t u32;

#define ETH_ALEN	6

typedef enum
{
	SDK_MODULE_NONE,
	SDK_MODULE_GLOBAL,
	SDK_MODULE_CPU_PORT,
	SDK_MODULE_PORT,
	SDK_MODULE_VLAN,
	SDK_MODULE_QINQ,
	SDK_MODULE_QOS,
	SDK_MODULE_DOS,
	SDK_MODULE_MAC,
	SDK_MODULE_MIRROR,
	SDK_MODULE_MULTICAST,
	SDK_MODULE_TRUNK,
	SDK_MODULE_JUMBO,
	SDK_MODULE_STORM,
	SDK_MODULE_RATE,
	SDK_MODULE_8021X,
	SDK_MODULE_MSTP,
	SDK_MODULE_EEE,
	SDK_MODULE_SNOOPING,
	//SDK_MODULE_TRAFFIC,
	SDK_MODULE_CFP,
	//SDK_MODULE_UDF,
	SDK_MODULE_MISC,

}b53xxx_module_t;

typedef enum
{
	//switch
	CMD_SWITCH_FWDG,
	CMD_SWITCH_MODE,
	CMD_SWITCH_RESET,
	CMD_SWITCH_INIT,
	CMD_SWITCH_EN_MIB,
	CMD_SWITCH_RESET_MIB,
	CMD_BPDU_RX_ENABLE, /* Enables all ports to receive BPDUs and forwards to the IMP port */

	//CPU port
	CMD_CPU_PORT_ENABLE,
	CMD_CPU_PORT_SPEED,
	CMD_CPU_PORT_DUPLEX,
	CMD_CPU_PORT_LINK,
	CMD_CPU_PORT_FLOW,

	//Port Control
	CMD_PORT_ENABLE,
	CMD_PORT_DISABLE_LRN,	//Disable Learning
	CMD_PORT_ENABLE_SLRN,	//Software Learning
	CMD_PORT_PROTECTED,		//Port Protected
	CMD_PORT_FORWARD_MODE,	//Port Forward
	CMD_PORT_PASUE_CAPABILITY,	//Pause Capability
	CMD_PORT_FAUSE_FRAME_DETECTION,//Pause Frame Detection
	CMD_PORT_FAUSE_PASS_THROUGH_TX,//Pause Pass Through for TX
	CMD_PORT_FAUSE_PASS_THROUGH_RX,//Pause Pass Through for RX
	CMD_PORT_MULTICAST_FORWARD,	//IP Multicast are multicast addresses
	//Port State Override
	CMD_PORT_SPEED,
	CMD_PORT_DUPLEX,
	CMD_PORT_LINK,
	CMD_PORT_FLOW,

	CMD_PORT_MIB,
	//Port Status
	CMD_PORT_LINK_STATE,
	CMD_PORT_SPEED_STATE,
	CMD_PORT_DUPLEX_STATE,



	//vlan
	CMD_VLAN_ENABLE,
	CMD_VLAN_ADD,
	CMD_VLAN_DEL,
	CMD_VLAN_ADD_TAG_PORT,
	CMD_VLAN_DEL_TAG_PORT,
	CMD_VLAN_ADD_UNTAG_PORT,
	CMD_VLAN_DEL_UNTAG_PORT,
	CMD_VLAN_ADD_DEFAULT_PORT,
	CMD_VLAN_DEL_DEFAULT_PORT,
	CMD_VLAN_INTER_ACTION,
	CMD_VLAN_OUTER_ACTION,

	//QINQ
	CMD_QINQ_ENABLE,
	CMD_QINQ_DEFAULT_TPID,
	CMD_QINQ_ADD_PORT,
	CMD_QINQ_DEL_PORT,

	//MAC
	CMD_MAC_ADD,
	CMD_MAC_DEL,
	CMD_MAC_GET,
	CMD_MAC_CLR,
	CMD_MAC_AGE,

	//trunk
	CMD_TRUNK_ENABLE,
	CMD_TRUNK_MODE,
	CMD_TRUNK_ADD,
	CMD_TRUNK_DEL,

	//dos
	CMD_DOS_ENABLE,
	CMD_DOS_SET_TCP_HDR,
	CMD_DOS_SET_ICMP_SIZE,

	//jumbo
	CMD_JUMBO_ENABLE,
	CMD_JUMBO_SIZE,

	//mstp/stp
	CMD_STP_STATE,
	CMD_MSTP_ENABLE,
	CMD_MSTP_AGE,
	CMD_MSTP_STATE,
	CMD_MSTP_BYPASS,

	//mirror
	CMD_MIRROR_ENABLE,
	CMD_MIRROR_SOURCE,
	CMD_MIRROR_SOURCE_FILTER,
	/*
	 * snooping
	 */
	//igmp
	CMD_IGMP_SNOOPING_ENABLE,
	//icmp
	CMD_ICMP_SNOOPING_ENABLE,
	//icmp
	CMD_DHCP_SNOOPING_ENABLE,
	//RARP
	CMD_RARP_SNOOPING_ENABLE,
	//ARP
	CMD_ARP_SNOOPING_ENABLE,

	//Rate limit
	CMD_CPU_RATE_ENABLE,
	CMD_WAN_RATE_ENABLE,
	CMD_INGRESS_RATE_ENABLE,
	CMD_EGRESS_RATE_ENABLE,

	//Strom
	CMD_STROM_ENABLE,
	CMD_STROM_INGRESS_RATE_ENABLE,

	//QOS
	CMD_QOS_PORT_ENABLE,
	CMD_QOS_8021Q_ENABLE,
	CMD_QOS_DIFFSERV_ENABLE,
	CMD_QOS_PRI_TO_PRI,			//IEEE 802.1p priority map to PRI
	CMD_QOS_DIFFSERV_TO_PRI,	//Diffserv map to PRI
	CMD_QOS_PRI_TO_CLASS,		//PRI mapped to TX Queue ID
	CMD_QOS_CPU_TO_CLASS,		//CPU Stream mapped to TX Queue ID
	CMD_QOS_TX_QUEUE_MODE,		//设置 TX QUEUE 调度方式
	CMD_QOS_TX_QUEUE_WEIGHT,	//设置 TX QUEUE 权重
	CMD_QOS_CLASS4_QUEUE_MODE,	//设置CLASS 4 调度模式和权重
	CMD_QOS_REMARKING_ENABLE,	//优先级改写
	CMD_QOS_PRI_MAP_8021P,		//PRI mapped to IEEE 802.1p priority

	//multicast
	CMD_MULTICAST_MODE,
	CMD_MULTICAST_ADD,
	CMD_MULTICAST_DEL,
	CMD_MULTICAST_FWD,

	//EEE
	CMD_EEE_ENABLE,
	CMD_EEE_SET,
	CMD_EEE_GET,

	//8021X
	CMD_8021X_ENABLE,
	CMD_8021X_MODE,
	CMD_8021X_IP,
	CMD_8021X_MAC,
	CMD_8021X_BYPASS,

	//misc
	CMD_MISC_WAN_PORT_ENABLE,
	//Reserved Multicast Control
	CMD_MISC_RES_MULTI_LRN,
	CMD_MISC_RES_MULTI_FWD_MODE,//单播/多播，黑洞

	CMD_MISC_AGE_ACCELERATE,
	CMD_MISC_HASH_DISABLE,
	CMD_MISC_BPDU_ADDRESS,

	CMD_MISC_ISP_TPID,
	CMD_MISC_ISP_PORT_ENABLE,


}b53xxx_cmd_t;

enum
{
	MIRROR_NONE,
	MIRROR_INGRESS,
	MIRROR_EGRESS,
	MIRROR_BOTH
};


#pragma pack (1)

struct mac_table
{
	u8 		port;
	u8 		mac[ETH_ALEN];
	u16 	vid;
	u8 		pri;
};

struct vlan_table
{
	u16 	start_vid;
	u16 	end_vid;
};


struct mirror_ctl
{
	//enum {MIRROR_NONE, MIRROR_INGRESS, MIRROR_EGRESS, MIRROR_BOTH}	inout;
	u8 		mac[ETH_ALEN];
	bool 	dst;
};

struct statistic_port
{
	uint64_t TxOctets;
	uint32_t TxDropPkts;
	uint32_t TxBroadcastPkts;
	uint32_t TxMulticastPkts;
	uint32_t TxUnicastPkts;
	uint32_t TxCollisions;
	uint32_t TxSingleCollision;
	uint32_t TxMultipleCollision;
	uint32_t TxDeferredTransmit;
	uint32_t TxLateCollision;
	uint32_t TxExcessiveCollision;
	uint32_t TxPausePkts;
	uint64_t RxOctets;
	uint32_t RxUndersizePkts;
	uint32_t RxPausePkts;
	uint32_t Pkts64Octets;
	uint32_t Pkts65to127Octets;
	uint32_t Pkts128to255Octets;
	uint32_t Pkts256to511Octets;
	uint32_t Pkts512to1023Octets;
	uint32_t Pkts1024to1522Octets;
	uint32_t RxOversizePkts;
	uint32_t RxJabbers;
	uint32_t RxAlignmentErrors;
	uint32_t RxFCSErrors;
	uint64_t RxGoodOctets;
	uint32_t RxDropPkts;
	uint32_t RxUnicastPkts;
	uint32_t RxMulticastPkts;
	uint32_t RxBroadcastPkts;
	uint32_t RxSAChanges;
	uint32_t RxFragments;
	uint32_t RxJumboPkts;
	uint32_t RxSymbolErrors;
	uint32_t RxDiscarded;
};


struct b53xxx_ioctl
{
	u32		cmd;	//4
	u8		module;	//1
	bool	enable;	//1
	u8 		port;	//1
	u16 	vid;	//2
	u32 	value;	//4
	u32 	value1;	//4
	bool	ipv6;	//1
	void	*data;	//4
};

#pragma pack (0)


#define B53_IO	's'
#define B53_IO_CTL	_IO(B53_IO, 0x01)
#define B53_IO_W	_IOW(B53_IO, 0x02, struct b53xxx_ioctl)
#define B53_IO_R	_IOR(B53_IO, 0x03, struct b53xxx_ioctl)
#define B53_IO_WR	_IOWR(B53_IO, 0x04, struct b53xxx_ioctl)



#define SDK_DRV		"/dev/b531250"

extern int sdk_cmd_ioctl(caddr_t pVoid);



#endif /* _BCM53125_SDK_H */
