/*
 * hal_l3mc.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_L3MC_H__
#define __HAL_L3MC_H__
#ifdef __cplusplus
extern "C" {
#endif

typedef zpl_uint32 hal_vlan_vector_t[4098];

/* IPMC address type. */
typedef struct hal_ipmc_addr_s {
    zpl_ipaddr_t s_ip_addr;                 /* IPv4 Source address. */
    zpl_ipaddr_t mc_ip_addr;                /* IPv4 Destination address. */
    zpl_ipv6addr_t s_ip6_addr;               /* IPv6 Source address. */
    zpl_ipv6addr_t mc_ip6_addr;              /* IPv6 Destination address. */
    vlan_t vid;                     /* VLAN identifier. */
    int vrf;                      /* Virtual Router Instance. */
    int cos;                      /* COS based on dst IP multicast addr. */
    int l2_pbmp;                 /* L2 Port bitmap. */
    int l2_ubmp;                 /* L2 Untag port bitmap. */
    int l3_pbmp;                 /* L3 Port bitmap. */
    int ts;                             /* Source port or TGID bit. */
    int port_tgid;                      /* Source port or TGID. */
    int v;                              /* Valid bit. */
    int mod_id;                         /* Module ID. */
    int ipmc_index;                     /* Use this index to program IPMC table
                                           for XGS chips based on flags value.
                                           For SBX chips it is the Multicast
                                           Group index */
    zpl_uint32 flags;                       /* See BCM_IPMC_XXX flag definitions. */
    int lookup_class;                   /* Classification lookup class ID. */
    int distribution_class; /* Fabric Distribution Class. */
} hal_ipmc_addr_t;

/* IPMC address flags. */
#define BCM_IPMC_KEEP_ENTRY                 0x00000001 /* Internal use. */
#define BCM_IPMC_USE_IPMC_INDEX             0x00000002 /* When set the ipmc
                                                          add/del functions use
                                                          the ipmc_index
                                                          supplied. */
#define BCM_IPMC_REPLICATE                  0x00000004 /* Add entry allowing
                                                          replication if
                                                          (supported). */
#define BCM_IPMC_HIT                        0x00000008 /* On get, indicates if
                                                          L3 hit is on for
                                                          entry. */
#define BCM_IPMC_SOURCE_PORT_NOCHECK        0x00000010 /* Do not source port
                                                          check this entry (XGS
                                                          only). */
#define BCM_IPMC_DISABLED                   0x00000020 /* Add entry in disabled
                                                          state. */
#define BCM_IPMC_REPLACE                    0x00000040 /* Replace an existing
                                                          entry. */
#define BCM_IPMC_IP6                        0x00000080 /* IPv6 support. */
#define BCM_IPMC_HIT_CLEAR                  0x00000100 /* Clear hit bit. */
#define BCM_IPMC_SETPRI                     0x00000200 /* Pick up new priority
                                                          (XGS3 only). */
#define BCM_IPMC_PROXY_IP6                  0x00000400 /* IPv6 Proxy MC egress
                                                          ports on XGS2. */
#define BCM_IPMC_ADD_DISABLED               BCM_IPMC_DISABLED 
#define BCM_IPMC_USE_FABRIC_DISTRIBUTION    0x00001000 /* Use specified fabric
                                                          distribution class. */
#define BCM_IPMC_COPYTOCPU                  0x00002000 /* When set, Copy the
                                                          packet to CPU. */

/* IPMC counters structure. */
typedef struct hal_ipmc_counters_s {
    zpl_uint64 rmca;    /* Received L2 multicast frame. */
    zpl_uint64 tmca;    /* Transmit L2 multicast packet counter. */
    zpl_uint64 imbp;    /* Number of IPMC packets bridged. */
    zpl_uint64 imrp;    /* Number of IPMC packets routed. */
    zpl_uint64 rimdr;   /* Number of IPMC dropped in ingress. */
    zpl_uint64 timdr;   /* Number of IPMC dropped in egress. */
} hal_ipmc_counters_t;


/* Initialize the BCM IPMC subsystem. */
extern int hal_ipmc_init(void);

/* Detach the BCM IPMC subsystem. */
extern int hal_ipmc_detach(void);

/* Enable/disable IPMC support. */
extern int hal_ipmc_enable(
    int enable);

/* Enable/disable source port checking. */
extern int hal_ipmc_source_port_check(
    int enable);

/* Define the IPMC search rule. */
extern int hal_ipmc_source_ip_search(
    int enable);


/* Initialize a hal_ipmc_addr_t/halx_ipmc_addr_t. */
extern void hal_ipmc_addr_t_init(
    hal_ipmc_addr_t *data);


/* Add new IPMC group. */
extern int hal_ipmc_add(
    hal_ipmc_addr_t *data);

/* Find info of an IPMC group. */
extern int hal_ipmc_find(
    hal_ipmc_addr_t *data);

/* Remove IPMC group. */
extern int hal_ipmc_remove(
    hal_ipmc_addr_t *data);

/* Remove all IPMC groups. */
extern int hal_ipmc_remove_all(void);

/* Get an IPMC entry at a specified IPMC index. */
extern int hal_ipmc_get_by_index(
    int index, 
    hal_ipmc_addr_t *data);

/* Delete IPMC group (API superceded). */
extern int hal_ipmc_delete(
    zpl_ipaddr_t s_ip_addr, 
    zpl_ipaddr_t mc_ip_addr, 
    vlan_t vid, 
    int keep);

/* Delete all IPMC groups (API superceded). */
extern int hal_ipmc_delete_all(void);

/* Get info of an IPMC group (API superceded). */
extern int hal_ipmc_get(
    zpl_ipaddr_t s_ip_addr, 
    zpl_ipaddr_t mc_ip_addr, 
    vlan_t vid, 
    hal_ipmc_addr_t *data);

/* Enable/Disable an IPMC group (API superceded). */
extern int hal_ipmc_entry_enable_set(
    zpl_ipaddr_t s_ip_addr, 
    zpl_ipaddr_t mc_ip_addr, 
    vlan_t vid, 
    int enable);

/* Set the cos of an IPMC entry (API superceded). */
extern int hal_ipmc_cos_set(
    zpl_ipaddr_t s_ip_addr, 
    zpl_ipaddr_t mc_ip_addr, 
    vlan_t vid, 
    int cos);

/* Set the input module ID of an IPMC entry (API superceded). */
extern int hal_ipmc_port_modid_set(
    zpl_ipaddr_t s_ip_addr, 
    zpl_ipaddr_t mc_ip_addr, 
    vlan_t vid, 
    int mod_id);

/* Set the source port/trunk group ID of an IPMC entry (API superceded). */
extern int hal_ipmc_port_tgid_set(
    zpl_ipaddr_t s_ip_addr, 
    zpl_ipaddr_t mc_ip_addr, 
    vlan_t vid, 
    int ts, 
    int port_tgid);

/* Add L2 multicast ports for IPMC group (API superceded). */
extern int hal_ipmc_add_l2_ports(
    zpl_ipaddr_t s_ip_addr, 
    zpl_ipaddr_t mc_ip_addr, 
    vlan_t vid, 
    int pbmp, 
    int ut_pbmp);

/* Delete L2 multicasted ports from an IPMC entry (API superceded). */
extern int hal_ipmc_delete_l2_ports(
    zpl_ipaddr_t s_ip_addr, 
    zpl_ipaddr_t mc_ip_addr, 
    vlan_t vid, 
    zpl_phyport_t port);

/* Add ports for L3 multicast to an IPMC entry (API superceded). */
extern int hal_ipmc_add_l3_ports(
    zpl_ipaddr_t s_ip_addr, 
    zpl_ipaddr_t mc_ip_addr, 
    vlan_t vid, 
    int pbmp);

/* Delete ports for L3 multicast from an IPMC entry (API superceded). */
extern int hal_ipmc_delete_l3_port(
    zpl_ipaddr_t s_ip_addr, 
    zpl_ipaddr_t mc_ip_addr, 
    vlan_t vid, 
    zpl_phyport_t port);

/* Reinitialize/clear the egress IP multicast configuration. */
extern int hal_ipmc_egress_port_init(void);

/* Configure the IP Multicast egress properties. */
extern int hal_ipmc_egress_port_set(
    zpl_phyport_t port, 
    const zpl_mac_t mac, 
    int untag, 
    vlan_t vid, 
    int ttl_threshold);

/* Get the egress IP multicast configuration properties. */
extern int hal_ipmc_egress_port_get(
    zpl_phyport_t port, 
    zpl_mac_t mac, 
    int *untag, 
    vlan_t *vid, 
    int *ttl_threshold);

/* Get the combined IP multicast statistics. */
extern int hal_ipmc_counters_get(
    zpl_phyport_t port, 
    hal_ipmc_counters_t *counters);

/* 
 * Provides maximum IPMC index that this fabric can handle
 * (BCM5670/BCM5675).
 */
extern int hal_ipmc_bitmap_max_get(
    int *max_index);

/* Set the IPMC forwarding port bitmap (BCM5670/BCM5675). */
extern int hal_ipmc_bitmap_set(
    int ipmc_idx, 
    zpl_phyport_t port, 
    int pbmp);

/* Get the IPMC forwarding port bitmap (BCM5670/BCM5675). */
extern int hal_ipmc_bitmap_get(
    int ipmc_idx, 
    zpl_phyport_t port, 
    int *pbmp);

/* Remove IPMC forwarding port bitmap (BCM5670/BCM5675). */
extern int hal_ipmc_bitmap_del(
    int ipmc_idx, 
    zpl_phyport_t port, 
    int pbmp);

/* 
 * Return set of VLANs selected for port's replication list for IPMC
 * group (pre-XGS3).
 */
extern int hal_ipmc_repl_get(
    int index, 
    zpl_phyport_t port, 
    hal_vlan_vector_t vlan_vec);

/* 
 * Add a VLAN (or L3 Interface id for XGS3) to selected ports'
 * replication list (pre-XGS3).
 */
extern int hal_ipmc_repl_add(
    int index, 
    int pbmp, 
    vlan_t vlan);

/* 
 * Remove VLAN (or L3 Interface in the case of XGS3) from selected ports'
 * replication list (pre-XGS3).
 */
extern int hal_ipmc_repl_delete(
    int index, 
    int pbmp, 
    vlan_t vlan);

/* 
 * Remove all VLANs (or L3 Interfaces in XGS3) from selected ports'
 * replication list (pre-XGS3).
 */
extern int hal_ipmc_repl_delete_all(
    int index, 
    int pbmp);

/* 
 * Add an L3 Interface or VLAN to the egress port's replication list
 * (XGS3).
 */
extern int hal_ipmc_egress_intf_add(
    int mc_index, 
    zpl_phyport_t port, 
    int *l3_intf);

/* 
 * Remove L3 Interface or VLAN from the given egress port's replication
 * list (XGS3).
 */
extern int hal_ipmc_egress_intf_delete(
    int mc_index, 
    zpl_phyport_t port, 
    int *l3_intf);

/* 
 * Remove all L3 interfaces (or VLANs) from the given (egress) port's
 * replication list (XGS3).
 */
extern int hal_ipmc_egress_intf_delete_all(
    int mc_index, 
    zpl_phyport_t port);


/* 
 * Assign a set of VLANs as the selected port's replication list (compact
 * mode).
 */
extern int hal_ipmc_repl_set(
    int mc_index, 
    zpl_phyport_t port, 
    hal_vlan_vector_t vlan_vec);

/* 
 * Assign a list of L3 Interfaces (or VLANS) to the egress port's
 * replication list (compact mode).
 */
extern int hal_ipmc_egress_intf_set(
    int mc_index, 
    zpl_phyport_t port, 
    int if_count, 
    zpl_phyport_t *if_array);

/* 
 * Assign a list of L3 Interfaces (or VLANS) to the egress port's
 * replication list (compact mode).
 */
extern int hal_ipmc_egress_intf_get(
    int mc_index, 
    zpl_phyport_t port, 
    int if_max, 
    zpl_phyport_t *if_array, 
    int *if_count);


#ifdef __cplusplus
}
#endif

#endif /* __HAL_L2MC_H__ */
