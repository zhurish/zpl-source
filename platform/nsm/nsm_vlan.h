/*
 * nsm_vlan.h
 *
 *  Created on: Jan 8, 2018
 *      Author: zhurish
 */

#ifndef __NSM_VLAN_H__
#define __NSM_VLAN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "auto_include.h"
#include "zplos_include.h"

#include "nsm_vlan_database.h"

/* VLAN Action definitions. */
typedef enum nsm_vlan_action_e {
    NSM_VLAN_ACTION_NONE,      /* Do not modify. */
    NSM_VLAN_ACTION_ADD,       /* Add VLAN tag. */
    NSM_VLAN_ACTION_REPLACE,   /* Replace VLAN tag. */
    NSM_VLAN_ACTION_DELETE,    /* Delete VLAN tag. */
    NSM_VLAN_ACTION_COPY       /* Copy VLAN tag. */
} nsm_vlan_action_t;
/* VLAN Pcp Action definitions. */
typedef enum nsm_vlan_pcp_action_e {
    bcmVlanPcpActionNone,               /* Do not modify. */
    bcmVlanPcpActionMapped,             /* Use TC/DP mapped PCP. */
    bcmVlanPcpActionIngressInnerPcp,    /* Use incoming packet's CTag PCP. */
    bcmVlanPcpActionIngressOuterPcp,    /* Use incoming packet's Stag PCP. */
    bcmVlanPcpActionPortDefault         /* Use port default PCP. */
} nsm_vlan_pcp_action_t;

/* Initialize a VLAN tag action set structure. */
typedef struct nsm_vlan_action_set_s {
    vlan_t new_outer_vlan;          /* New outer VLAN for Add/Replace
                                           actions. */
    vlan_t new_inner_vlan;          /* New inner VLAN for Add/Replace
                                           actions. */
    zpl_phyport_t ingress_if;                /* L3 Ingress Interface. */
    int priority;                       /* Internal or packet priority. */
    nsm_vlan_action_t dt_outer;         /* Outer-tag action for double-tagged
                                           packets. */
    nsm_vlan_action_t dt_outer_prio;    /* Outer-priority-tag actionfor
                                           double-tagged packets. */
    nsm_vlan_action_t dt_inner;         /* Inner-tag actionfor double-tagged
                                           packets. */
    nsm_vlan_action_t dt_inner_prio;    /* Inner-priority-tag action for
                                           double-tagged packets. */
    nsm_vlan_action_t ot_outer;         /* Outer-tag action for
                                           single-outer-tagged packets. */
    nsm_vlan_action_t ot_outer_prio;    /* Outer-priority-tag action for
                                           single-outer-tagged packets. */
    nsm_vlan_action_t ot_inner;         /* Inner-tag action for
                                           single-outer-tagged packets. */
    nsm_vlan_action_t it_outer;         /* Outer-tag action for
                                           single-inner-tagged packets. */
    nsm_vlan_action_t it_inner;         /* Inner-tag action for
                                           single-inner-tagged packets. */
    nsm_vlan_action_t it_inner_prio;    /* Inner-priority-tag action for
                                           single-inner-tagged packets. */
    nsm_vlan_action_t ut_outer;         /* Outer-tag action for untagged
                                           packets. */
    nsm_vlan_action_t ut_inner;         /* Inner-tag action for untagged
                                           packets. */
    nsm_vlan_pcp_action_t outer_pcp;    /* Outer tag's pcp field action of
                                           outgoing packets. */
    nsm_vlan_pcp_action_t inner_pcp;    /* Inner tag's pcp field action of
                                           outgoing packets. */
} nsm_vlan_action_set_t;

/* Flags for the unified IPv4/IPv6 nsm_vlan_ip_t type. */
#define NSM_VLAN_SUBNET_IP6     (1 << 14)  

/* Unified IPv4/IPv6 type. */
typedef struct nsm_vlan_ip_s {
    zpl_uint32 flags; 
    zpl_ipaddr_t ip4; 
    zpl_ipaddr_t mask; 
    zpl_ipv6addr_t ip6; 
    int prefix; 
    vlan_t vid; 
    int prio; 
} nsm_vlan_ip_t;

/* vlan 管理 */
typedef struct trunk_vlan_s
{
	vlan_t	vlan;
	vlan_t 	minvlan;
	vlan_t	maxvlan;
}trunk_vlan_t;

typedef struct nsm_vlan_s
{
	vlan_t	    native;
	vlan_t	    access;
	zpl_bool    allow_all;
	trunk_vlan_t trunk_allowed[VLAN_TABLE_MAX];
	vlan_t	    allowed_max;

    zpl_bool	qinq_enable;
	vlan_t		qinq_tpid;
}nsm_vlan_t;



extern int nsm_vlan_init(void);
extern int nsm_vlan_exit(void);
extern int nsm_vlan_cleanall(void);
extern int nsm_vlan_default(void);
extern int nsm_vlan_enable(void);
extern zpl_bool nsm_vlan_is_enable(void);

extern int nsm_vlan_interface_create_api(struct interface *ifp);
extern int nsm_vlan_interface_del_api(struct interface *ifp);

/* 设置access接口的vlan */
extern int nsm_interface_access_vlan_set_api(struct interface *ifp, vlan_t);
extern int nsm_interface_access_vlan_unset_api(struct interface *ifp, vlan_t);
extern int nsm_interface_access_vlan_get_api(struct interface *ifp, vlan_t *);
/* 设置trunk接口的默认vlan */
extern int nsm_interface_native_vlan_set_api(struct interface *ifp, vlan_t);
extern int nsm_interface_native_vlan_get_api(struct interface *ifp, vlan_t *);
/* 设置trunk接口的允许通过vlan */
extern int nsm_interface_trunk_add_allowed_vlan_lookup_api(struct interface *ifp, vlan_t );
extern int nsm_interface_trunk_add_allowed_vlan_api(struct interface *ifp, vlan_t );
extern int nsm_interface_trunk_del_allowed_vlan_api(struct interface *ifp, vlan_t );
extern int nsm_interface_trunk_add_allowed_batch_vlan_api(struct interface *ifp, vlan_t ,vlan_t);
extern int nsm_interface_trunk_del_allowed_batch_vlan_api(struct interface *ifp, vlan_t ,vlan_t);

extern int nsm_interface_trunk_allowed_vlan_list_api(int add, struct interface *ifp, const char *str);
extern int nsm_interface_trunk_allowed_vlan_list_lookup_api(struct interface *ifp, vlan_t *vlanlist, zpl_uint32 num);

#ifdef ZPL_SHELL_MODULE
extern void cmd_vlan_init (void);
extern int nsm_vlan_interface_write_config(struct vty *vty, struct interface *ifp);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __NSM_VLAN_H__ */
