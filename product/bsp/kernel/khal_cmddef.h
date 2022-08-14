#ifndef __KHAL_CMDDEF_H__
#define __KHAL_CMDDEF_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "khal_ipcmsg.h"
#include "khal_util.h"

/* HAL_MODULE_8021X */
enum khal_8021x_cmd 
{
    HAL_8021X_NONE,
	HAL_8021X,
	HAL_8021X_PORT_MODE,
	HAL_8021X_PORT_MAC,
	HAL_8021X_PORT_STATE,
};
typedef struct khal_8021x_param_s
{
	zpl_uint32 value;
	mac_t mac[NSM_MAC_MAX];
}khal_8021x_param_t;


/*HAL_MODULE_PORT,//L2 */
enum khal_port_cmd 
{
    HAL_PORT_NONE,
	HAL_PORT,
	HAL_PORT_LINK,
	HAL_PORT_SPEED,
	HAL_PORT_DUPLEX,
	HAL_PORT_FLOW,
    HAL_PORT_PAUSE,
    HAL_PORT_JUMBO,
	HAL_PORT_LOOP,
	HAL_PORT_LEARNING,
	HAL_PORT_SWLEARNING,
	HAL_PORT_PROTECTED,
    HAL_PORT_VRF,
    HAL_PORT_MODE,
    HAL_PORT_MTU,
    HAL_PORT_MULTICAST,
    HAL_PORT_BANDWIDTH,
	//STORM
	HAL_PORT_STORM_RATELIMIT,  
    HAL_PORT_STATS,    
};
struct if_stats
{
   zpl_ulong rx_packets;   /* total packets received       */
   zpl_ulong tx_packets;   /* total packets transmitted    */
   zpl_ulong rx_bytes;     /* total bytes received         */
   zpl_ulong tx_bytes;     /* total bytes transmitted      */
   zpl_ulong rx_errors;    /* bad packets received         */
   zpl_ulong tx_errors;    /* packet transmit problems     */
   zpl_ulong rx_dropped;   /* no space in linux buffers    */
   zpl_ulong tx_dropped;   /* no space available in linux  */
   zpl_ulong rx_multicast; /* multicast packets received   */
   zpl_ulong collisions;

   /* detailed rx_errors: */
   zpl_ulong rx_length_errors;
   zpl_ulong rx_over_errors;   /* receiver ring buff overflow  */
   zpl_ulong rx_crc_errors;    /* recved pkt with crc error    */
   zpl_ulong rx_frame_errors;  /* recv'd frame alignment error */
   zpl_ulong rx_fifo_errors;   /* recv'r fifo overrun          */
   zpl_ulong rx_missed_errors; /* receiver missed packet     */
   /* detailed tx_errors */
   zpl_ulong tx_aborted_errors;
   zpl_ulong tx_carrier_errors;
   zpl_ulong tx_fifo_errors;
   zpl_ulong tx_heartbeat_errors;
   zpl_ulong tx_window_errors;
   /* for cslip etc */
   zpl_ulong rx_compressed;
   zpl_ulong tx_compressed;
};

typedef struct khal_port_param_s
{
	zpl_uint32 value;
	mac_t mac[NSM_MAC_MAX];
    zpl_uint32 value1;
    zpl_uint32 value2;
}khal_port_param_t;


/* HAL_MODULE_L3IF //L3 */
enum khal_l3if_cmd 
{
    HAL_L3IF_NONE,
	HAL_L3IF_CREATE,
	HAL_L3IF_DELETE,
	HAL_L3IF_ADDR_ADD,
	HAL_L3IF_ADDR_DEL,
	HAL_L3IF_DSTADDR_ADD,
	HAL_L3IF_DSTADDR_DEL,
    HAL_L3IF_VRF,
	HAL_L3IF_MAC,
};
typedef struct khal_l3if_param_s
{
  	khal_port_header_t  port;
  	char    ifname[64];
	zpl_uint8 l2if_type;
  	vrf_id_t vrfid;
	mac_t mac[NSM_MAC_MAX];
}khal_l3if_param_t;

typedef struct khal_l3if_addr_param_s
{
	khal_port_header_t  port;
	zpl_uint8 family;
	zpl_uint8 prefixlen;
	union g_addr address;  
	zpl_uint8 sec;
}khal_l3if_addr_param_t;


/*HAL_MODULE_ROUTE,//Route Table */
enum khal_route_cmd 
{
    HAL_ROUTE_NONE,
	HAL_ROUTE_ADD,
	HAL_ROUTE_DEL,
};
typedef struct khal_nexthop
{
  	ifindex_t kifindex;
	khal_port_header_t  port;
  	union g_addr gateway;
} 	khal_nexthop_t;


typedef struct khal_route_param_s
{
  safi_t safi;
  vrf_id_t vrf_id;
  zpl_uint8 family;
  zpl_uint32  table;
  zpl_uint8 prefixlen;
  union g_addr destination;
  union g_addr source;
  zpl_uint8 nexthop_num;
  khal_nexthop_t nexthop[16];
  zpl_uint8 processid;
  zpl_uint8 type;
  zpl_uint8 flags;
  zpl_uint8 distance;
  zpl_uint32 metric;
  zpl_uint32 tag;
  zpl_uint32 mtu;
}khal_route_param_t;

/*HAL_MODULE_MSTP HAL_MODULE_STP*/
enum khal_mstp_cmd 
{
    HAL_MSTP_NONE,
	HAL_MSTP_ENABLE,
    HAL_MSTP_CREATE,
    HAL_MSTP_ADD_VLAN,
    HAL_MSTP_DEL_VLAN,
	HAL_MSTP_STATE,
    HAL_STP_STATE,
};
typedef enum stp_state_s
{
	STP_DISABLE = 0,
	STP_LISTENING,
	STP_LEARNING,
	STP_FORWARDING,
	STP_BLOCKING,
}stp_state_t;

typedef enum khal_port_stp_state_e {
    HAL_PORT_STP_DISABLE = 1,
    HAL_PORT_STP_BLOCK,
    HAL_PORT_STP_LISTEN,
    HAL_PORT_STP_LEARN,
    HAL_PORT_STP_FORWARD
} khal_port_stp_state_t;

typedef struct khal_mstp_param_s
{
    zpl_bool enable;
	zpl_uint32 value;
	zpl_uint32 type;
	khal_port_stp_state_t state;
}khal_mstp_param_t;


/*HAL_MODULE_IGMP*/
enum khal_igmp_cmd 
{
    HAL_IGMP_NONE,
    HAL_IGMP_IPCHECK,
	HAL_IGMP_SNOOPING,
	HAL_IGMPQRY_SNOOPING,
	HAL_IGMPUNKNOW_SNOOPING,
	HAL_MLD_SNOOPING,
	HAL_MLDQRY_SNOOPING,
	HAL_ARP_COPYTOCPU,
	HAL_RARP_COPYTOCPU,
	HAL_DHCP_COPYTOCPU,
};

/* HAL_MODULE_GLOBAL */
enum khal_global_cmd 
{
    HAL_GLOBAL_NONE,
	HAL_GLOBAL_START,
    HAL_GLOBAL_JUMBO_SIZE,
	HAL_GLOBAL_MANEGE,
	HAL_GLOBAL_FORWARD,
	HAL_GLOBAL_MULTICAST_FLOOD,
	HAL_GLOBAL_UNICAST_FLOOD,
	HAL_GLOBAL_MULTICAST_LEARNING,
	HAL_GLOBAL_BPDU,
	HAL_GLOBAL_AGINT,  
	HAL_GLOBAL_WAN_PORT, 
};

/* HAL_MODULE_SWITCH */
typedef enum khal_core_cmd
{
	HAL_CORE_NONE,
  	HAL_CORE_COPY_TO_CPU,
	HAL_CORE_REDIRECT_TO_CPU,
  	HAL_CORE_FORWARED,
	HAL_CORE_DROP,
}khal_core_cmd_t;

/* HAL_MODULE_CPU */
enum khal_cpu_cmd 
{
    HAL_CPU_NONE,
	HAL_CPU_MODE,
	HAL_CPU_ENABLE,
	HAL_CPU_SPEED,
	HAL_CPU_DUPLEX,
	HAL_CPU_FLOW,
};



/*HAL_MODULE_DOS*/
enum khal_dos_cmd 
{
    HAL_DOS_CMD_NONE,
	HAL_DOS_CMD_IP_LAN_DRIP,
	HAL_DOS_CMD_TCP_BLAT_DROP,
	HAL_DOS_CMD_UDP_BLAT_DROP,
	HAL_DOS_CMD_TCP_NULLSCAN_DROP,
	HAL_DOS_CMD_TCP_XMASSCAN_DROP,
	HAL_DOS_CMD_TCP_SYNFINSCAN_DROP,
	HAL_DOS_CMD_TCP_SYNERROR_DROP,

	HAL_DOS_CMD_TCP_SHORTHDR_DROP,
	HAL_DOS_CMD_TCP_FRAGERROR_DROP,
	HAL_DOS_CMD_ICMPv4_FRAGMENT_DROP,
	HAL_DOS_CMD_ICMPv6_FRAGMENT_DROP,

	HAL_DOS_CMD_ICMPv4_LONGPING_DROP,
	HAL_DOS_CMD_ICMPv6_LONGPING_DROP,

	HAL_DOS_CMD_TCP_HDR_SIZE,
	HAL_DOS_CMD_ICMPv4_SIZE,
	HAL_DOS_CMD_ICMPv6_SIZE,
};
typedef struct khal_dos_param_s
{
	zpl_uint32 value;
}khal_dos_param_t;

/*HAL_MODULE_MAC*/
enum khal_mac_cmd 
{
    HAL_MAC_CMD_NONE,
	HAL_MAC_CMD_AGE,
	HAL_MAC_CMD_ADD,
	HAL_MAC_CMD_DEL,
	HAL_MAC_CMD_CLEAR,
	HAL_MAC_CMD_CLEARALL,
	HAL_MAC_CMD_READ,
	HAL_MAC_CMD_DUMP,
    HAL_MAC_CMD_MAX,
};
typedef struct khal_mac_param_s
{
	vlan_t vlan;
	zpl_uint32 value;
	mac_t mac[NSM_MAC_MAX];
	zpl_uint32 macnum;
	khal_mac_cache_t *mactbl;
}khal_mac_param_t;


/*HAL_MODULE_MIRROR*/
enum khal_mirror_cmd 
{
    HAL_MIRROR_CMD_NONE,
	HAL_MIRROR_CMD_DST_PORT,
	HAL_MIRROR_CMD_SRC_PORT,
	HAL_MIRROR_CMD_SRC_MAC,
    HAL_MIRROR_CMD_MAX,
};
typedef struct khal_mirror_param_s
{
	zpl_uint32 value;
	zpl_uint8 dir;
	zpl_uint8 filter;
	mac_t mac[NSM_MAC_MAX];
}khal_mirror_param_t;

typedef enum
{
	MIRROR_NONE = 0,
	MIRROR_BOTH,
	MIRROR_INGRESS,
	MIRROR_EGRESS,
}mirror_dir_en;

typedef enum mirror_filter_e {
    MIRROR_FILTER_ALL 	= 0,
    MIRROR_FILTER_DA 	= 1,
	MIRROR_FILTER_SA 	= 2,
	MIRROR_FILTER_BOTH	= 3,
} mirror_filter_t;


/*HAL_MODULE_QINQ*/
enum khal_qinq_cmd 
{
    HAL_QINQ_CMD_NONE,
	HAL_QINQ_CMD_ENABLE,
	HAL_QINQ_CMD_TPID,
	HAL_QINQ_CMD_IF_ENABLE,
    HAL_QINQ_CMD_MAX,
};
typedef struct khal_qinq_param_s
{
	zpl_uint32 value;
}khal_qinq_param_t;


/*HAL_MODULE_VLAN*/
enum khal_vlan_cmd 
{
    HAL_VLAN_NONE,
	HAL_VLAN,
	HAL_VLAN_CREATE,
	HAL_VLAN_DELETE,
	HAL_VLAN_RANGE_CREATE,
    HAL_VLAN_RANGE_DELETE,
    //PORT
    HAL_VLAN_ACCESS,
    HAL_VLAN_NATIVE,
    HAL_VLAN_ALLOWE,
    HAL_VLAN_RANGE_ALLOWE,
    HAL_VLAN_PORT_BASE,
    HAL_VLAN_TEST,
    HAL_VLAN_MAX,
};
typedef struct khal_vlan_param_s
{
	zpl_bool enable;
	vlan_t vlan;
	vlan_t vlan_end;
    zpl_vlan_bitmap_t vlanbitmap;
}khal_vlan_param_t;


/*HAL_MODULE_QOS*/
enum khal_qos_cmd 
{
    HAL_QOS_NONE,
	HAL_QOS_EN,
	HAL_QOS_IPG,
	HAL_QOS_BASE_TRUST,
	HAL_QOS_8021Q,
	HAL_QOS_DIFFSERV,

	//CLASS
	HAL_QOS_QUEUE_MAP_CLASS,
	HAL_QOS_CLASS_SCHED,
	HAL_QOS_CLASS_WEIGHT,

	//INPUT
	HAL_QOS_8021Q_MAP_QUEUE,
	HAL_QOS_DIFFSERV_MAP_QUEUE,
	HAL_QOS_IPPRE_MAP_QUEUE,
    HAL_QOS_MPLSEXP_MAP_QUEUE,		//
	HAL_QOS_PORT_MAP_QUEUE,

	//OUTPUT
	HAL_QOS_QUEUE_SCHED,
	HAL_QOS_QUEUE_WEIGHT,
	HAL_QOS_QUEUE_RATELIMIT,
	HAL_QOS_PRI_REMARK,

	//CPU
	HAL_QOS_CPU_RATELIMIT,

	HAL_QOS_PORT_INRATELIMIT,
	HAL_QOS_PORT_OUTRATELIMIT,
};

typedef struct khal_qos_param_s
{
	zpl_bool enable;
	zpl_uint32 value;
	zpl_uint32 limit;
	zpl_uint32 burst_size;
	zpl_uint32 mode;
	zpl_uint32 pri;
	zpl_uint32 diffserv;
	zpl_uint32 queue;
	zpl_uint32 class;
	zpl_uint32 type;
	zpl_uint32 weight;
}khal_qos_param_t;

typedef enum
{
	NSM_QOS_TRUST_NONE = 0,
	NSM_QOS_TRUST_PORT,
	NSM_QOS_TRUST_EXP,
	NSM_QOS_TRUST_COS,
	NSM_QOS_TRUST_DSCP,
	NSM_QOS_TRUST_IP_PRE,
}nsm_qos_trust_e;



/*HAL_MODULE_TRUNK*/
enum khal_trunk_cmd 
{
    HAL_TRUNK_CMD_NONE,
	HAL_TRUNK_CMD_ENABLE,
	HAL_TRUNK_CMD_CREATE,
	HAL_TRUNK_CMD_ADDIF,
	HAL_TRUNK_CMD_DELIF,
	HAL_TRUNK_CMD_MODE,
    HAL_TRUNK_CMD_MAX,
};
typedef struct khal_trunk_param_s
{
	zpl_bool enable;
	zpl_uint32 trunkid;
	zpl_uint32 mode;
}khal_trunk_param_t;

enum khal_misc_cmd 
{
    HAL_MISC_NONE,
	HAL_MISC_JUMBO,
	HAL_MISC_JUMBO_SIZE,
	HAL_MISC_DHCP_SNOOP,
	HAL_MISC_IGMP_SNOOP,
    HAL_MISC_EEE,
};    

/*HAL_MODULE_DEBUG*/
enum zpl_debug_cmd
{
    HAL_KLOG_MODULE,
    HAL_KNETPKT_MODULE,
    HAL_KHALCLIENT_MODULE,
};

#ifdef __cplusplus
}
#endif

#endif /* __KHAL_CMDDEF_H__ */