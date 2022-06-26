/*
 * hal_vlan.h
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */

#ifndef __HAL_VLAN_H__
#define __HAL_VLAN_H__

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
    HAL_VLAN_ACCESS,
    HAL_VLAN_NATIVE,
    HAL_VLAN_ALLOWE,
    HAL_VLAN_RANGE_ALLOWE,
    HAL_VLAN_PORT_BASE,
    HAL_VLAN_TEST,
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
	zpl_bool enable;
	vlan_t vlan;
	vlan_t vlan_end;
    zpl_vlan_bitmap_t vlanbitmap;
}hal_vlan_param_t;

extern int hal_vlan_enable(zpl_bool enable);
extern int hal_vlan_create(vlan_t vlan);
extern int hal_vlan_destroy(vlan_t vlan);

extern int hal_vlan_batch_create(zpl_vlan_bitmap_t vlanbitmap, vlan_t start, vlan_t end);
extern int hal_vlan_batch_destroy(zpl_vlan_bitmap_t vlanbitmap, vlan_t start, vlan_t end);

/* 设置trunk接口的默认vlan */
extern int hal_port_add_access_vlan(ifindex_t ifindex, vlan_t vlan);
extern int hal_port_del_access_vlan(ifindex_t ifindex, vlan_t vlan);

/* 设置trunk接口的默认vlan */
extern int hal_port_add_native_vlan(ifindex_t ifindex, vlan_t vlan);
extern int hal_port_del_native_vlan(ifindex_t ifindex, vlan_t vlan);
/* 设置trunk接口的允许通过vlan */
extern int hal_port_add_allowed_tag_vlan(ifindex_t ifindex, vlan_t vlan);
extern int hal_port_del_allowed_tag_vlan(ifindex_t ifindex, vlan_t vlan);

extern int hal_port_add_allowed_tag_batch_vlan(ifindex_t ifindex, vlan_t start, vlan_t end);
extern int hal_port_del_allowed_tag_batch_vlan(ifindex_t ifindex, vlan_t start, vlan_t end);

extern int hal_port_set_vlan(ifindex_t ifindex, vlan_t vlan);
extern int hal_port_unset_vlan(ifindex_t ifindex, vlan_t vlan);

int hal_vlan_test(int cmd, ifindex_t phyport, vlan_t vlan);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_VLAN_H__ */
