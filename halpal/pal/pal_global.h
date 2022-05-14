/*
 * pal_global.h
 *
 *  Created on: Jan 28, 2018
 *      Author: zhurish
 */

#ifndef __PAL_GLOBAL_H__
#define __PAL_GLOBAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ZPL_NSM_FIREWALLD
#include "nsm_firewalld.h"
#endif

typedef struct pal_stack_s
{
	//interface
	int (*ip_stack_create)(struct interface *ifp);
	int (*ip_stack_destroy)(struct interface *ifp);
	int (*ip_stack_change)(struct interface *ifp);
	int (*ip_stack_up)(struct interface *ifp);
	int (*ip_stack_down)(struct interface *ifp);
	int (*ip_stack_refresh_flag)(struct interface *ifp);
	int (*ip_stack_update_flag)(struct interface *ifp, zpl_uint32 flags);

	int (*ip_stack_ifindex)(char *name);
	int (*ip_stack_set_mtu)(struct interface *ifp, zpl_uint32 mtu);
	int (*ip_stack_set_metric)(struct interface *ifp, zpl_uint32 metric);
	int (*ip_stack_get_lladdr)(struct interface *ifp);
	int (*ip_stack_set_lladdr)(struct interface *ifp, zpl_uint8 *mac, zpl_uint32 len);
	int (*ip_stack_set_vlan)(struct interface *ifp, vlan_t vlan);
	int (*ip_stack_set_vlanpri)(struct interface *ifp, zpl_uint32 pri);
	int (*ip_stack_promisc)(struct interface *ifp, zpl_bool enable);

	//ip address
	int (*ip_stack_ipv4_add)(struct interface *ifp,struct connected *);
	int (*ip_stack_ipv4_delete)(struct interface *ifp,struct connected *);
	int (*ip_stack_ipv4_replace)(struct interface *ifp,struct connected *);
	int (*ip_stack_ipv4_dstaddr_add)(struct interface *ifp, struct connected *);
	int (*ip_stack_ipv4_dstaddr_del)(struct interface *ifp, struct connected *);

	int (*ip_stack_ipv6_add)(struct interface *ifp,struct connected *, zpl_bool secondry);
	int (*ip_stack_ipv6_delete)(struct interface *ifp,struct connected *, zpl_bool secondry);

	int (*ip_stack_route_rib)(struct prefix *p, struct rib *old, struct rib *new);

#ifdef ZPL_NSM_ARP
	//ip arp
	int (*ip_stack_arp_get)(struct interface *ifp, struct prefix *address, zpl_uint8 *mac);
	int (*ip_stack_arp_add)(struct interface *ifp, struct prefix *address, zpl_uint8 *mac);
	int (*ip_stack_arp_delete)(struct interface *ifp, struct prefix *address);
	int (*ip_stack_arp_request)(struct interface *ifp, struct prefix *address);
	int (*ip_stack_arp_gratuitousarp_enable)(zpl_bool enable);
	int (*ip_stack_arp_ttl)(zpl_uint32 ttl);
	int (*ip_stack_arp_age_timeout)(zpl_uint32 timeout);
	int (*ip_stack_arp_retry_interval)(zpl_uint32 interval);
#endif

	// ip route
	int (*ip_stack_vrf_create)(struct ip_vrf *vrf);
	int (*ip_stack_vrf_delete)(struct ip_vrf *vrf);
	int (*ip_stack_set_vrf)(struct interface *ifp, struct ip_vrf *vrf);
	int (*ip_stack_update_statistics)(struct interface *ifp);

	/*firewall*/
/*
 * 端口映射
 */
#ifdef ZPL_NSM_FIREWALLD
	int (*ip_stack_firewall_portmap_rule_set)(firewall_t *rule, zpl_action action);
/*
 * 端口开放
 */
	int (*ip_stack_firewall_port_filter_rule_set)(firewall_t *rule, zpl_action action);
	int (*ip_stack_firewall_mangle_rule_set)(firewall_t *rule, zpl_action action);
	int (*ip_stack_firewall_raw_rule_set)(firewall_t *rule, zpl_action action);
	int (*ip_stack_firewall_snat_rule_set)(firewall_t *rule, zpl_action action);
	int (*ip_stack_firewall_dnat_rule_set)(firewall_t *rule, zpl_action action);
#endif
}pal_stack_t;



extern pal_stack_t pal_stack;


extern int pal_module_init(void);


#ifdef ZPL_NSM_FIREWALLD
/*
 * 端口映射
 */
int apal_firewall_portmap_rule_set(firewall_t *rule, zpl_action action);
/*
 * 端口开放
 */
int apal_firewall_port_filter_rule_set(firewall_t *rule, zpl_action action);
int apal_firewall_mangle_rule_set(firewall_t *rule, zpl_action action);
int apal_firewall_raw_rule_set(firewall_t *rule, zpl_action action);
int apal_firewall_snat_rule_set(firewall_t *rule, zpl_action action);
int apal_firewall_dnat_rule_set(firewall_t *rule, zpl_action action);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __PAL_GLOBAL_H__ */
