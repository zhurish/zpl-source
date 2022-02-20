/*
 * hal_vlan.h
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */

#ifndef ABSTRACT_HAL_HAL_VLAN_H_
#define ABSTRACT_HAL_HAL_VLAN_H_

#ifdef __cplusplus
extern "C" {
#endif

enum hal_vlan_cmd 
{
    HAL_VLAN_NONE,
	HAL_VLAN,
	HAL_VLAN_CREATE,
	HAL_VLAN_DELETE,
	HAL_VLAN_RANGE_CREATE,
    HAL_VLAN_RANGE_DELETE,
    //PORT
    HAL_VLAN_UNTAG,
    HAL_VLAN_TAG,
    HAL_VLAN_NATIVE,
    HAL_VLAN_ALLOWE,
    HAL_VLAN_RANGE_ALLOWE,
    HAL_VLAN_PVID,
    HAL_VLAN_MAX,
};

/* hal_vlan_control_t */
typedef enum hal_vlan_control_e {
    halVlanDropUnknown,             /* Drop unknown/FFF VLAN pkts or send to
                                       CPU. */
    halVlanPreferIP4,               /* Prefer IP Subnet VLAN selection. */
    halVlanPreferMAC,               /* Prefer MAC VLAN selection. */
    halVlanShared,                  /* Shared vs. Independent VLAN Learning. */
    halVlanSharedID,                /* Shared Learning VLAN ID. */
    halVlanTranslate,               /* Chip is in VLAN translate mode. */
    halVlanIgnorePktTag,            /* Ignore Packet VLAN tag. Treat packet as
                                       untagged. */
    halVlanPreferEgressTranslate,   /* Do egress translation even if ingress FP
                                       changes the outer/inner VLAN tag(s). */
    halVlanPerPortTranslate,        /* Chip is in per port VLAN translate mode. */
    halVlanIndependentStp,          /* Directly set port,vlan stp state, igoring
                                       STG in hal_vlan_stp_set(). */
    halVlanIntelligentDT,           /* Intelligent Double Tagging Mode.(BCM53115
                                       only) */
    halVlanTranslateMode,           /* Select modes of Vlan Translation. */
    halVlanBypassIgmpMld,           /* IGMP/MLD frame will bypass VLAN Ingress
                                       Filter and VLAN Egress Filter. */
    halVlanBypassArpDhcp,           /* ARP/DHCP frame will flood in VLAN domain
                                       and copy to CPU even CPU is not in VLAN
                                       group. */
    halVlanBypassMiim,              /* Enable CPU ingress frame bypass VLAN
                                       Ingress Filter and VLAN Egress Filter. */
    halVlanBypassMcast,             /* Enable known multicast frame bypass VLAN
                                       Ingress Filter and VLAN Egress Filter. */
    halVlanBypassRsvdMcast,         /* Enable reserved multicast frame bypass
                                       VLAN Ingress Filter and VLAN Egress
                                       Filter. */
    halVlanBypassL2UserAddr,        /* Enable L2 user address frame bypass VLAN
                                       Ingress Filter and VLAN Egress Filter. */
    halVlanUnknownLearn,            /* Packets whose VLAN is not registered are
                                       learned when set to 1. */
    halVlanUnknownToCpu,            /* Packets whose VLAN is not registered are
                                       sent to CPU when set to 1. */
    halVlanMemberMismatchLearn,     /* Packets' incoming port is not the member
                                       of the VLAN are learned when set to 1. */
    halVlanMemberMismatchToCpu      /* Packets' incoming port is not the member
                                       of the VLAN are sent to CPU when set to
                                       1. */
} hal_vlan_control_t;

/* VLAN multicast flood modes. */
typedef enum hal_vlan_mcast_flood_e {
    HAL_VLAN_MCAST_FLOOD_ALL = 1, 
    HAL_VLAN_MCAST_FLOOD_UNKNOWN, 
    HAL_VLAN_MCAST_FLOOD_NONE, 
    HAL_VLAN_MCAST_FLOOD_COUNT 
} hal_vlan_mcast_flood_t;

typedef struct hal_vlan_param_s
{
    zpl_bool set;
	zpl_bool enable;
	vlan_t vlan;
	vlan_t vlan_end;
    zpl_uint32 mstpid;
}hal_vlan_param_t;

extern int hal_vlan_enable(zpl_bool enable);
extern int hal_vlan_create(vlan_t vlan);
extern int hal_vlan_destroy(vlan_t vlan);

extern int hal_vlan_batch_create(vlan_t *vlan, int num);
extern int hal_vlan_batch_destroy(vlan_t *vlan, int num);

extern int hal_vlan_add_untag_port(ifindex_t ifindex, vlan_t vlan);
extern int hal_vlan_del_untag_port(ifindex_t ifindex, vlan_t vlan);

extern int hal_vlan_add_tag_port(ifindex_t ifindex, vlan_t vlan);
extern int hal_vlan_del_tag_port(ifindex_t ifindex, vlan_t vlan);

extern int hal_port_add_native_vlan(ifindex_t ifindex, vlan_t vlan);
extern int hal_port_del_native_vlan(ifindex_t ifindex, vlan_t vlan);

extern int hal_port_add_allowed_tag_vlan(ifindex_t ifindex, vlan_t vlan);
extern int hal_port_del_allowed_tag_vlan(ifindex_t ifindex, vlan_t vlan);

extern int hal_port_add_allowed_tag_batch_vlan(ifindex_t ifindex, vlan_t start, vlan_t end);
extern int hal_port_del_allowed_tag_batch_vlan(ifindex_t ifindex, vlan_t start, vlan_t end);

extern int hal_port_set_vlan(ifindex_t ifindex, vlan_t vlan);
extern int hal_port_unset_vlan(ifindex_t ifindex, vlan_t vlan);


extern int hal_vlan_stp(vlan_t vlan, zpl_index_t stpid);
extern int hal_vlan_stp_state(vlan_t vlan, ifindex_t ifindex, zpl_uint32 state);
/* Add an entry to the VLAN Translation table. */
extern int hal_vlan_translate_add(ifindex_t ifindex, vlan_t old_vid, vlan_t new_vid, int prio);
/* Delete an entry or entries from the VLAN Translation table. */
extern int hal_vlan_translate_delete(ifindex_t ifindex, vlan_t old_vid);
/* Add an entry to the egress VLAN Translation table. */
extern int hal_vlan_translate_egress_add(ifindex_t ifindex, vlan_t old_vid, vlan_t new_vid, int prio);
/* Remove an entry or entries from the VLAN egress Translation table. */
extern int hal_vlan_translate_egress_delete(ifindex_t ifindex, vlan_t old_vid);

/* Add an entry to the VLAN Translation table for double-tagging. */
extern int hal_vlan_translate_dtag_add(ifindex_t ifindex, vlan_t old_vid, vlan_t new_vid, int prio);
/* Remove an entry from the VLAN Translation table for double-tagging. */
extern int hal_vlan_translate_dtag_delete(ifindex_t ifindex, vlan_t old_vid);
/* Add a VLAN range to the VLAN Translation table. */
extern int hal_vlan_translate_range_add(ifindex_t ifindex, vlan_t old_vid_low, 
        vlan_t old_vid_high, vlan_t new_vid, int int_prio);

/* Delete a VLAN range from the VLAN Translation table. */
extern int hal_vlan_translate_range_delete(ifindex_t ifindex, vlan_t old_vid_low, 
        vlan_t old_vid_high);

/* Add VLAN range to the VLAN Translation table for double-tagging. */
extern int hal_vlan_dtag_range_add(ifindex_t ifindex, vlan_t old_vid_low, 
        vlan_t old_vid_high, vlan_t new_vid, int int_prio);

/* 
 * Remove a VLAN range from the VLAN Translation table for
 * double-tagging.
 */
extern int hal_vlan_dtag_range_delete(ifindex_t ifindex, vlan_t old_vid_low, 
        vlan_t old_vid_high);

/*vlan 映射的动作 */
extern int hal_vlan_default_action(ifindex_t ifindex, zpl_uint32, zpl_bool);
extern int hal_vlan_port_action(ifindex_t ifindex, zpl_uint32, zpl_bool);
extern int hal_vlan_port_egress_action(ifindex_t ifindex, zpl_uint32, zpl_bool);
extern int hal_vlan_protocol_action(ifindex_t ifindex, zpl_uint32 frame, zpl_uint32 ether, zpl_uint32, zpl_bool);


/* 
 * Add an association from MAC address to VLAN actions, which are
 *             used for VLAN tag/s assignment to untagged packets.
 */
extern int hal_vlan_mac_action_add(
    int unit, 
    mac_t *mac, 
    nsm_vlan_action_set_t *action);

/* 
 * Remove an association from MAC address to VLAN actions, which
 *             are used for VLAN tag assignment to untagged packets.
 */
extern int hal_vlan_mac_action_delete(
    int unit, 
    mac_t *mac);
/* 
 * Add an association from IP subnet to VLAN to use for assigning a VLAN
 * tag to untagged packets.
 */
extern int hal_vlan_ip4_add(
    int unit, 
    zpl_ipaddr_t ipaddr, 
    zpl_ipaddr_t netmask, 
    vlan_t vid, 
    int prio);

/* Remove an association from IP subnet to VLAN. */
extern int hal_vlan_ip4_delete(
    int unit, 
    zpl_ipaddr_t ipaddr, 
    zpl_ipaddr_t netmask);



/* 
 * Add an association from IP subnet to VLAN to use for assigning a VLAN
 * tag to untagged packets.
 */
extern int hal_vlan_ip_add(
    int unit, 
    nsm_vlan_ip_t *vlan_ip);

/* Remove an association from IP subnet to VLAN. */
extern int hal_vlan_ip_delete(
    int unit, 
    nsm_vlan_ip_t *vlan_ip);

/* 
 * Add an association from IP subnet to VLAN actions to use for assigning
 * VLAN tag actions to untagged packets.
 */
extern int hal_vlan_ip_action_add(
    int unit, 
    nsm_vlan_ip_t *vlan_ip, 
    nsm_vlan_action_set_t *action);

/* 
 * Delete an association from IP subnet to VLAN actions to use for
 * assigning VLAN tag actions to untagged packets.
 */
extern int hal_vlan_ip_action_delete(
    int unit, 
    nsm_vlan_ip_t *vlan_ip);



/* Set or retrieve the current vlan multicast flood behavior. */
extern int hal_vlan_mcast_flood(vlan_t vlan, hal_vlan_mcast_flood_t mcast_flood_mode);


#ifdef __cplusplus
}
#endif

#endif /* ABSTRACT_HAL_HAL_VLAN_H_ */
