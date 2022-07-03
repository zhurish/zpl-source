/*
 * bsp_vlan.h
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */

#ifndef __BSP_VLAN_H__
#define __BSP_VLAN_H__

#ifdef __cplusplus
extern "C" {
#endif
#ifdef ZPL_SDK_KERNEL

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

#endif
typedef struct sdk_vlan_s
{
    int    (*sdk_vlan_enable)(void *, zpl_bool);
    int    (*sdk_vlan_create)(void *, zpl_bool, vlan_t);

    int    (*sdk_port_access_vlan)(void *, zpl_bool, zpl_phyport_t, vlan_t);
    int    (*sdk_port_native_vlan)(void *, zpl_bool, zpl_phyport_t, vlan_t);
    int    (*sdk_port_allowed_tag_vlan)(void *, zpl_bool, zpl_phyport_t, vlan_t);
    int    (*sdk_port_allowed_untag_vlan)(void *, zpl_bool, zpl_phyport_t, vlan_t);

    int    (*sdk_vlan_port_bridge)(void *, zpl_bool, zpl_phyport_t, zpl_phyport_t);
    int    (*sdk_vlan_port_bridge_join)(void *, zpl_bool, zpl_phyport_t, zpl_phyport_t);

    int    (*sdk_vlan_stp_state)(void *, vlan_t, zpl_phyport_t, int stp_state);

    int    (*sdk_vlan_mstp_instance)(void *, zpl_bool, vlan_t, zpl_index_t mstpid);
    int    (*sdk_vlan_translate)(void *, zpl_bool, zpl_phyport_t, vlan_t, vlan_t, int);

}sdk_vlan_t;

extern sdk_vlan_t sdk_vlan;
extern int bsp_vlan_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_VLAN_H__ */
