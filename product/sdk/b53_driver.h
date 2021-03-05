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


typedef enum drv_port_stp_state_e {
    DRV_PORT_STP_DISABLE = 1,
    DRV_PORT_STP_BLOCK,
    DRV_PORT_STP_LISTEN,
    DRV_PORT_STP_LEARN,
    DRV_PORT_STP_FORWARD
} drv_port_stp_state_t;


typedef enum drv_port_duplex_e {
    DRV_PORT_DUPLEX_HALF = 1,
    DRV_PORT_DUPLEX_FULL
} drv_port_duplex_t;

typedef enum drv_port_speed_e {
    DRV_PORT_SPEED_10M = 1,
    DRV_PORT_SPEED_100M,
    DRV_PORT_SPEED_1G,
} drv_port_speed_t;



typedef enum {
    DRV_TRUNK_MODE_MACDASA = 0x0,
    DRV_TRUNK_MODE_MACDA = 0x1,
    DRV_TRUNK_MODE_MACSA = 0x2,
} drv_trunk_mode_t;



typedef enum drv_q_map_e {
    DRV_PORT_MAP_PRIO,		//Port base QOS
    DRV_8021P_MAP_PRIO,		//8021p
	DRV_DFSV_MAP_PRIO,		//Diffserv
} drv_q_map_t;

typedef enum drv_class_sched_e {
    DRV_CLASS_SCHED_WEIGHT = 0,
	DRV_CLASS_SCHED_STRICT,
} drv_class_sched_t;


typedef enum drv_class_e {
    DRV_CLASS_ID_0 = 0,
	DRV_CLASS_ID_1,
	DRV_CLASS_ID_2,
	DRV_CLASS_ID_3,
	DRV_CLASS_ID_4,
	DRV_CLASS_ID_5,
} drv_class_t;

typedef enum drv_queue_e {
    DRV_QUEUE_ID_0 = 0,
	DRV_QUEUE_ID_1,
	DRV_QUEUE_ID_2,
	DRV_QUEUE_ID_3,
	DRV_QUEUE_ID_4,
	DRV_QUEUE_ID_5,
	DRV_QUEUE_ID_6,
	DRV_QUEUE_ID_7,
} drv_queue_t;

typedef enum drv_priority_e {
    DRV_PRIORITY_ID_0 = 0,
	DRV_PRIORITY_ID_1,
	DRV_PRIORITY_ID_2,
	DRV_PRIORITY_ID_3,
	DRV_PRIORITY_ID_4,
	DRV_PRIORITY_ID_5,
	DRV_PRIORITY_ID_6,
	DRV_PRIORITY_ID_7,
} drv_priority_t;


typedef enum drv_bucket_size_e {
    DRV_BUCKET_SIZE_4K = 0,
	DRV_BUCKET_SIZE_8K,
	DRV_BUCKET_SIZE_16K,
	DRV_BUCKET_SIZE_32K,
	DRV_BUCKET_SIZE_64K,
	DRV_BUCKET_SIZE_500K,
} drv_bucket_size_t;

typedef enum drv_bucket_type_e {
    DRV_BUCKET_TYPE_UNICAST_HIT = 1,
	DRV_BUCKET_TYPE_MULTICAST_HIT = 2,
	DRV_BUCKET_TYPE_RSVMAC	= 4,
	DRV_BUCKET_TYPE_BROADCAST = 8,
	DRV_BUCKET_TYPE_MULTICAST_FAIL = 0X10,
	DRV_BUCKET_TYPE_UNICAST_FAIL = 0X20,
} drv_bucket_type_t;

typedef enum drv_rate_e {
    DRV_RATE_IPG_ENABLE = 1,
	DRV_RATE_ABSOLUTE = 1,
	DRV_RATE_BASE_SPEED	= 2,
} drv_rate_t;

typedef enum drv_mirror_mode_e {
    DRV_MIRROR_SOURCE_PORT 	= 1,
	DRV_MIRROR_SOURCE_MAC 	= 2,
	DRV_MIRROR_SOURCE_DIV	= 3,
} drv_mirror_mode_t;

typedef enum drv_mirror_type_e {
    DRV_MIRROR_INGRESS 	= 1,
	DRV_MIRROR_EGRESS 	= 2,
	DRV_MIRROR_BOTH	= 3,
} drv_mirror_type_t;

typedef enum drv_mirror_filter_e {
    DRV_MIRROR_FILTER_ALL 	= 0,
    DRV_MIRROR_FILTER_DA 	= 1,
	DRV_MIRROR_FILTER_SA 	= 2,
	DRV_MIRROR_FILTER_BOTH	= 3,
} drv_mirror_filter_t;

/* Dos Attack type */
typedef enum drv_dos_type_e {
    DRV_DOS_IP_LAN_DRIP = 0x1,
	DRV_DOS_TCP_BLAT,
	DRV_DOS_UDP_BLAT,
    DRV_DOS_TCP_NULL_SCAN,
    DRV_DOS_TCP_XMASS_SCAN,
    DRV_DOS_TCP_SYN_FIN_SCAN,
    DRV_DOS_TCP_SYN_ERROR,
    DRV_DOS_TCP_SHORT_HDR,
    DRV_DOS_TCP_FRAG_ERROR,
    DRV_DOS_ICMPV4_FRAGMENTS,
    DRV_DOS_ICMPV6_FRAGMENTS,
	DRV_DOS_ICMPV4_LONG_PING,
	DRV_DOS_ICMPV6_LONG_PING,


    DRV_DOS_MIN_TCP_HDR_SZ,
    DRV_DOS_MAX_ICMPV4_SIZE,
    DRV_DOS_MAX_ICMPV6_SIZE,
    DRV_DOS_DISABLE_LEARN
} drv_dos_type_t;
 
#ifdef __cplusplus
}
#endif

#endif /* __B53_DRIVER_H__ */
