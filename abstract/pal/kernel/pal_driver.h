/*
 * pal_driver.h
 *
 *  Created on: Jan 28, 2018
 *      Author: zhurish
 */

#ifndef __PAL_DRIVER_H__
#define __PAL_DRIVER_H__

#ifdef __cplusplus
extern "C" {
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
	int (*ip_stack_update_flag)(struct interface *ifp, ospl_uint32 flags);

	int (*ip_stack_ifindex)(char *name);
	int (*ip_stack_set_vr)(struct interface *ifp, vrf_id_t vrf_id);
	int (*ip_stack_set_mtu)(struct interface *ifp, ospl_uint32 mtu);
	int (*ip_stack_set_metric)(struct interface *ifp, ospl_uint32 metric);
	int (*ip_stack_get_lladdr)(struct interface *ifp);
	int (*ip_stack_set_lladdr)(struct interface *ifp, ospl_uint8 *mac, ospl_uint32 len);
	int (*ip_stack_set_vlan)(struct interface *ifp, vlan_t vlan);
	int (*ip_stack_set_vlanpri)(struct interface *ifp, ospl_uint32 pri);
	int (*ip_stack_promisc)(struct interface *ifp, ospl_bool enable);

	//ip address
	int (*ip_stack_dhcp)(struct interface *ifp, ospl_bool enable);
	int (*ip_stack_ipv4_add)(struct interface *ifp,struct connected *);
	int (*ip_stack_ipv4_delete)(struct interface *ifp,struct connected *);
	int (*ip_stack_ipv4_replace)(struct interface *ifp,struct connected *);
	int (*ip_stack_ipv4_dstaddr_add)(struct interface *ifp, struct connected *);
	int (*ip_stack_ipv4_dstaddr_del)(struct interface *ifp, struct connected *);

	int (*ip_stack_ipv6_add)(struct interface *ifp,struct connected *, ospl_bool secondry);
	int (*ip_stack_ipv6_delete)(struct interface *ifp,struct connected *, ospl_bool secondry);

	//ip arp
	int (*ip_stack_arp_get)(struct interface *ifp, struct prefix *address, ospl_uint8 *mac);
	int (*ip_stack_arp_add)(struct interface *ifp, struct prefix *address, ospl_uint8 *mac);
	int (*ip_stack_arp_delete)(struct interface *ifp, struct prefix *address);
	int (*ip_stack_arp_request)(struct interface *ifp, struct prefix *address);
	int (*ip_stack_arp_gratuitousarp_enable)(ospl_bool enable);
	int (*ip_stack_arp_ttl)(ospl_uint32 ttl);
	int (*ip_stack_arp_age_timeout)(ospl_uint32 timeout);
	int (*ip_stack_arp_retry_interval)(ospl_uint32 interval);

	// ip route
	int (*ip_stack_vrf_create)(vrf_id_t vrf_id);
	int (*ip_stack_vrf_delete)(vrf_id_t vrf_id);

	int (*ip_stack_update_statistics)(struct interface *ifp);

}pal_stack_t;

extern pal_stack_t pal_stack;

extern int pal_abstract_init();

//interface
extern int pal_interface_create(struct interface *ifp);
extern int pal_interface_destroy(struct interface *ifp);
extern int pal_interface_change(struct interface *ifp);

extern int pal_interface_up(struct interface *ifp);
extern int pal_interface_down(struct interface *ifp);
extern int pal_interface_refresh_flag(struct interface *ifp);
extern int pal_interface_update_flag(struct interface *ifp, ospl_uint32 flags);
extern int pal_interface_ifindex(char *k_name);

extern int pal_interface_set_vr(struct interface *ifp, vrf_id_t vrf_id);
extern int pal_interface_set_mtu(struct interface *ifp, ospl_uint32 mtu);
extern int pal_interface_set_lladdr(struct interface *ifp, ospl_uint8 *mac, ospl_uint32 len);
extern int pal_interface_get_lladdr(struct interface *ifp);

extern int pal_interface_vlan_set(struct interface *ifp, vlan_t vlan);
extern int pal_interface_vlanpri_set(struct interface *ifp, ospl_uint32 pri);
extern int pal_interface_promisc_link(struct interface *ifp, ospl_bool enable);

//ip address
extern int pal_interface_change_dhcp(struct interface *ifp, ospl_bool enable);
extern int pal_interface_ipv4_dstaddr_add(struct interface *ifp, struct connected *ifc);
extern int pal_interface_ipv4_dstaddr_delete(struct interface *ifp, struct connected *ifc);
extern int pal_interface_ipv4_replace(struct interface *ifp, struct connected *ifc);
extern int pal_interface_ipv4_add(struct interface *ifp,struct connected *ifc);
extern int pal_interface_ipv4_delete(struct interface *ifp, struct connected *ifc);
extern int pal_interface_ipv6_add(struct interface *ifp,struct connected *ifc, ospl_bool secondry);
extern int pal_interface_ipv6_delete(struct interface *ifp, struct connected *ifc, ospl_bool secondry);
//ip arp
extern int pal_interface_arp_add(struct interface *ifp, struct prefix *address, ospl_uint8 *mac);
extern int pal_interface_arp_delete(struct interface *ifp, struct prefix *address);
extern int pal_interface_arp_request(struct interface *ifp, struct prefix *address);
extern int pal_arp_gratuitousarp_enable(ospl_bool enable);
extern int pal_arp_ttl(ospl_uint32 ttl);
extern int pal_arp_age_timeout(ospl_uint32 timeout);
extern int pal_arp_retry_interval(ospl_uint32 interval);

//route
extern int pal_create_vr(vrf_id_t vrf_id);
extern int pal_delete_vr(vrf_id_t vrf_id);


extern int pal_interface_update_statistics(struct interface *ifp);


#ifdef __cplusplus
}
#endif

#endif /* __PAL_DRIVER_H__ */
