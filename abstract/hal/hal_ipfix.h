/*
 * hal_ipfix.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_IPFIX_H__
#define __HAL_IPFIX_H__
#ifdef __cplusplus
extern "C" {
#endif
/*基于IP的流*/
#define NSM_IPFIX_DSCP_MASK_COUNT   64         

/* hal_color_t */
typedef enum hal_color_e {
    halColorGreen, 
    halColorYellow, 
    halColorRed, 
    halColorDropFirst = halColorRed, 
    halColorPreserve, 
    halColorBlack, 
    halColorCount 
} hal_color_t;

typedef enum hal_ipfix_stage_e {
    NSM_IPFIX_STAGE_INGRESS, 
    NSM_IPFIX_STAGE_EGRESS 
} hal_ipfix_stage_t;

/* Flags for hal_ipfix_data_t.flags. */
#define NSM_IPFIX_DATA_VLAN_TAGGED          0x0001     
#define NSM_IPFIX_DATA_TYPE_L2              0x0002     
#define NSM_IPFIX_DATA_TYPE_IP4             0x0004     
#define NSM_IPFIX_DATA_TYPE_IP6             0x0008     
#define NSM_IPFIX_DATA_REASON_FLOW_TIMEOUT  0x0010     
#define NSM_IPFIX_DATA_REASON_FLOW_END      0x0020     
#define NSM_IPFIX_DATA_REASON_FLOW          0x0040     
#define NSM_IPFIX_DATA_REASON_COUNT         0x0080     
#define NSM_IPFIX_DATA_REASON_TIMESTAMP     0x0100     
#define NSM_IPFIX_DATA_RATE_VIOLATION       0x0200     
#define NSM_IPFIX_DATA_TYPE_MASK            0x000e     
#define NSM_IPFIX_DATA_REASON_MASK          0x01f0     

/* Export data structure */
typedef struct hal_ipfix_data_s {
    hal_ipfix_stage_t stage;    /* Ingress or egress. */
    zpl_uint32 flags;               /* Type and reason. */
    zpl_phyport_t port;            /* port. */
    vlan_t vlan;            /* L2: VLAN identifier. */
    zpl_uint16 vlan_prio;           /* L2: VLAN priority. */
    zpl_uint16 ether_type;          /* L2: type in ethernet II frame or 802.1Q tag. */
    zpl_mac_t src_mac_addr;     /* L2: source MAC address. */
    zpl_mac_t dst_mac_addr;     /* L2: destination MAC address. */
    zpl_ipaddr_t src_ip4_addr;      /* IPv4: source IP address. */
    zpl_ipaddr_t dst_ip4_addr;      /* IPv4: destination IP address. */
    zpl_ipv6addr_t src_ip6_addr;     /* IPv6: source IP address. */
    zpl_ipv6addr_t dst_ip6_addr;     /* IPv6: destination IP address. */
    zpl_uint16 ip_protocol;         /* IPv4: protocol; IPv6: next header. */
    zpl_uint16 tos;                 /* IPv4: type of service; IPv6: traffic class. */
    zpl_uint32 ip6_flow_label;      /* IPv6: flow label. */
    zpl_uint16 l4_src_port;         /* First 16 bits of L4 header such as TCP source
                                   port or ICMP type and code. */
    zpl_uint16 l4_dst_port;         /* Second 16 bits of L4 header such as TCP
                                   destination port. */
    zpl_phyport_t source_port;    /* Source port. */
    zpl_uint32 start_timestamp;     /* Session information. */
    zpl_uint32 last_timestamp;      /* Session information. */
    zpl_uint32 byte_count;          /* Session information. */
    zpl_uint32 pkt_count;           /* Session information. */
} hal_ipfix_data_t;

/* Flags for hal_ipfix_config_t.flags. */
#define NSM_IPFIX_CONFIG_ENABLE_NON_IP      0x00000001 
#define NSM_IPFIX_CONFIG_ENABLE_IP4         0x00000002 
#define NSM_IPFIX_CONFIG_ENABLE_IP6         0x00000004 
#define NSM_IPFIX_CONFIG_TCP_END_DETECT     0x00000008 
#define NSM_IPFIX_CONFIG_RECORD_NON_DISCARD_PKT 0x00000010 
#define NSM_IPFIX_CONFIG_RECORD_DISCARD_PKT 0x00000020 
#define NSM_IPFIX_CONFIG_KEY_IP4_USE_L2     0x00000040 
#define NSM_IPFIX_CONFIG_KEY_IP6_USE_L2     0x00000080 
#define NSM_IPFIX_CONFIG_KEY_SRC_IP         0x00000100 
#define NSM_IPFIX_CONFIG_KEY_DST_IP         0x00000200 
#define NSM_IPFIX_CONFIG_KEY_IP_PROT        0x00000400 
#define NSM_IPFIX_CONFIG_KEY_IP_DSCP        0x00000800 
#define NSM_IPFIX_CONFIG_KEY_IP_ECN         0x00001000 
#define NSM_IPFIX_CONFIG_KEY_L4_SRC_PORT    0x00002000 
#define NSM_IPFIX_CONFIG_KEY_L4_DST_PORT    0x00004000 
#define NSM_IPFIX_CONFIG_KEY_IP6_FLOW       0x00008000 
#define NSM_IPFIX_CONFIG_KEY_ICMP_TYPE      0x00010000 
#define NSM_IPFIX_CONFIG_KEY_ICMP_CODE      0x00020000 
#define NSM_IPFIX_CONFIG_KEY_MACDA          0x00040000 
#define NSM_IPFIX_CONFIG_KEY_MACSA          0x00080000 
#define NSM_IPFIX_CONFIG_KEY_VLAN_ID        0x00100000 
#define NSM_IPFIX_CONFIG_KEY_VLAN_PRI       0x00200000 
#define NSM_IPFIX_CONFIG_KEY_ETHER_TYPE     0x00400000 
#define NSM_IPFIX_CONFIG_KEY_VLAN_TAGGED    0x00800000 
#define NSM_IPFIX_CONFIG_TCP_FLAGS_LAST     0x01000000 
#define NSM_IPFIX_CONFIG_KEY_SOURCE_PORT_OR_INTERFACE 0x02000000 
#define NSM_IPFIX_CONFIG_ENABLE_MASK        0x00000007 
#define NSM_IPFIX_CONFIG_KEY_MASK           0x02ffffc0 

/* IPFIX port configuration structure */
typedef struct hal_ipfix_config_s {
    zpl_uint32 flags;                       /* Fields select and other flags. */
    zpl_uint8 dscp_mask[NSM_IPFIX_DSCP_MASK_COUNT]; /* DSCP value translation mapping. */
    zpl_ipaddr_t src_ip4_mask;              /* Mask to map IPv4 address to key
                                           value. */
    zpl_ipaddr_t dst_ip4_mask;              /* Mask to map IPv4 address to key
                                           value. */
    zpl_ipaddr_t tunnel_src_ip4_mask;       /* Mask to map IPv4 address to key
                                           value. */
    zpl_ipaddr_t tunnel_dst_ip4_mask;       /* Mask to map IPv4 address to key
                                           value. */
    zpl_ipv6addr_t src_ip6_mask;             /* Mask to map IPv6 address to key
                                           value. */
    zpl_ipv6addr_t dst_ip6_mask;             /* Mask to map IPv6 address to key
                                           value. */
    zpl_ipv6addr_t tunnel_src_ip6_mask;      /* Mask to map IPv6 address to key
                                           value. */
    zpl_ipv6addr_t tunnel_dst_ip6_mask;      /* Mask to map IPv6 address to key
                                           value. */
    zpl_uint32 entry_limit;                 /* Maximum number of flow entry
                                           collected for the port. */
    zpl_uint32 min_time;                    /* The flow will not be exported unless
                                           the flow has been established for
                                           more than min_time (unit is 10 ms). */
    zpl_uint32 max_time;                    /* The flow will not be exported unless
                                           the flow has been established for
                                           less than max_time (unit is 10 ms). */
    zpl_uint32 max_idle_time;               /* The flow will be exported and
                                           terminated if idle for more than
                                           max_idle_time (unit is 10 ms). */
    zpl_uint32 sample_rate;                 /* Collect one packet info for every
                                           sample_rate packets. */
} hal_ipfix_config_t;
/* Flags for hal_ipfix_rate_t.flags */
#define NSM_IPFIX_RATE_VIOLATION_WITH_ID    0x00000001 
#define NSM_IPFIX_RATE_VIOLATION_REPLACE    0x00000002 
#define NSM_IPFIX_RATE_VIOLATION_DROP       0x00000004 
#define NSM_IPFIX_RATE_VIOLATION_COPY_TO_CPU 0x00000008 
#define NSM_IPFIX_RATE_VIOLATION_DSCP_SET   0x00000010 
#define NSM_IPFIX_RATE_VIOLATION_COLOR_SET  0x00000020 
#define NSM_IPFIX_RATE_VIOLATION_PKT_PRI_SET 0x00000040 
#define NSM_IPFIX_RATE_VIOLATION_INT_PRI_SET 0x00000080 

/* Flow rate meter entry */
typedef struct hal_ipfix_rate_s {
    zpl_uint32 rate_id;    /* Flow rate meter id. */
    zpl_uint32 flags;                   /* Configuration flags. */
    zpl_uint32 count;                   /* Flow count (for get only). */
    zpl_uint32 limit;                   /* Max number of good flows to allow. */
    zpl_uint32 rate;                    /* Number of good flows per second. */
    zpl_uint8 dscp;                     /* New DSCP value. */
    hal_color_t color;              /* New color value. */
    int pkt_pri;                    /* New packet priority. */
    int int_pri;                    /* New internal priority. */
} hal_ipfix_rate_t;

/* Flags for hal_ipfix_mirror_config_t.flags. */
#define NSM_IPFIX_MIRROR_CONFIG_TTL_OFFSET_MAX 0x0001     
#define NSM_IPFIX_MIRROR_CONFIG_FRAGMENT    0x0002     
#define NSM_IPFIX_MIRROR_CONFIG_NON_FRAGMENT 0x0004     

/* IPFIX port mirror configuration */
typedef struct hal_ipfix_mirror_config_s {
    zpl_uint32 flags;           /* Configuration flags. */
    zpl_uint32 pkt_count;       /* Number of packets to be mirrored. */
    zpl_uint8 tcp_flags_mask;   /* TCP flags mask to determine if the packet will be
                               mirrored. */
    zpl_uint8 ttl_offset_max;   /* Max TTL offset allowance to be mirrored. */
} hal_ipfix_mirror_config_t;

/* hal_ipfix_config_set */
extern int hal_ipfix_config_set(
    hal_ipfix_stage_t stage, 
    zpl_phyport_t port, 
    hal_ipfix_config_t *config);

/* hal_ipfix_config_get */
extern int hal_ipfix_config_get(
    hal_ipfix_stage_t stage, 
    zpl_phyport_t port, 
    hal_ipfix_config_t *config);

/* Add an IPFIX flow rate meter entry */
extern int hal_ipfix_rate_create(
    hal_ipfix_rate_t *rate_info);

/* Delete an IPFIX flow rate meter entry */
extern int hal_ipfix_rate_destroy(
    zpl_uint32 rate_id);

/* Get IPFIX flow rate meter entry for the specified id */
extern int hal_ipfix_rate_get(
    hal_ipfix_rate_t *rate_info);


/* Delete all IPFIX flow rate meter entries */
extern int hal_ipfix_rate_destroy_all(void);

/* Add a mirror destination to the IPFIX flow rate meter entry */
extern int hal_ipfix_rate_mirror_add(
    zpl_uint32 rate_id, 
    zpl_phyport_t mirror_dest_id);

/* Delete a mirror destination from the IPFIX flow rate meter entry */
extern int hal_ipfix_rate_mirror_delete(
    zpl_uint32 rate_id, 
    zpl_phyport_t mirror_dest_id);

/* 
 * Delete all mirror destination associated to the IPFIX flow rate meter
 * entry
 */
extern int hal_ipfix_rate_mirror_delete_all(
    zpl_uint32 rate_id);

/* Get all mirror destination from the IPFIX flow rate meter entry */
extern int hal_ipfix_rate_mirror_get(
    zpl_uint32 rate_id, 
    int mirror_dest_size, 
    zpl_phyport_t *mirror_dest_id, 
    int *mirror_dest_count);

/* Set IPFIX mirror control configuration of the specified port */
extern int hal_ipfix_mirror_config_set(
    hal_ipfix_stage_t stage, 
    zpl_phyport_t port, 
    hal_ipfix_mirror_config_t *config);

/* Get all IPFIX mirror control configuration of the specified port */
extern int hal_ipfix_mirror_config_get(
    hal_ipfix_stage_t stage, 
    zpl_phyport_t port, 
    hal_ipfix_mirror_config_t *config);

/* Add an IPFIX mirror destination to the specified port */
extern int hal_ipfix_mirror_port_dest_add(
    hal_ipfix_stage_t stage, 
    zpl_phyport_t port, 
    zpl_phyport_t mirror_dest_id);

/* Delete an IPFIX mirror destination from the specified port */
extern int hal_ipfix_mirror_port_dest_delete(
    hal_ipfix_stage_t stage, 
    zpl_phyport_t port, 
    zpl_phyport_t mirror_dest_id);

/* Delete all IPFIX mirror destination from the specified port */
extern int hal_ipfix_mirror_port_dest_delete_all(
    hal_ipfix_stage_t stage, 
    zpl_phyport_t port);

/* Get IPFIX mirror destination of the specified port */
extern int hal_ipfix_mirror_port_dest_get(
    hal_ipfix_stage_t stage, 
    zpl_phyport_t port, 
    int mirror_dest_size, 
    zpl_phyport_t *mirror_dest_id, 
    int *mirror_dest_count);


#ifdef __cplusplus
}
#endif

#endif /* __HAL_IPFIX_H__ */
