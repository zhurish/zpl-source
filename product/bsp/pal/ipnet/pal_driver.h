/*
 * pal_include.h
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
	int (*ip_stack_up)(struct interface *ifp);
	int (*ip_stack_down)(struct interface *ifp);
	int (*ip_stack_update_flag)(struct interface *ifp);
	int (*ip_stack_set_vr)(struct interface *ifp, vrf_id_t vrf_id);
	int (*ip_stack_set_mtu)(struct interface *ifp, zpl_uint32 mtu);
	int (*ip_stack_set_metric)(struct interface *ifp, zpl_uint32 metric);
	int (*ip_stack_set_lladdr)(struct interface *ifp, zpl_uint8 *mac, zpl_uint32 len);
	int (*ip_stack_set_vlan)(struct interface *ifp, vlan_t vlan);
	int (*ip_stack_set_vlanpri)(struct interface *ifp, zpl_uint32 pri);
	int (*ip_stack_promisc)(struct interface *ifp, zpl_bool enable);

	//ip address
	int (*ip_stack_dhcp)(struct interface *ifp, zpl_bool enable);
	int (*ip_stack_ipv4_add)(struct interface *ifp,struct connected *);
	int (*ip_stack_ipv4_delete)(struct interface *ifp,struct connected *);
	int (*ip_stack_ipv4_replace)(struct interface *ifp,struct connected *);
	int (*ip_stack_ipv4_dstaddr_add)(struct interface *ifp, struct connected *);
	int (*ip_stack_ipv4_dstaddr_del)(struct interface *ifp, struct connected *);

	int (*ip_stack_ipv6_add)(struct interface *ifp,struct connected *);
	int (*ip_stack_ipv6_delete)(struct interface *ifp,struct connected *);

	//ip arp
	int (*ip_stack_arp_add)(struct interface *ifp, struct prefix *address, zpl_uint8 *mac);
	int (*ip_stack_arp_delete)(struct interface *ifp, struct prefix *address);
	int (*ip_stack_arp_request)(struct interface *ifp, struct prefix *address);
	int (*ip_stack_arp_gratuitousarp_enable)(zpl_bool enable);
	int (*ip_stack_arp_ttl)(zpl_uint32 ttl);
	int (*ip_stack_arp_age_timeout)(zpl_uint32 timeout);
	int (*ip_stack_arp_retry_interval)(zpl_uint32 interval);

	// ip route
	int (*ip_stack_vrf_create)(vrf_id_t vrf_id);
	int (*ip_stack_vrf_delete)(vrf_id_t vrf_id);

}pal_stack_t;

extern pal_stack_t pal_stack;

extern int pal_ip_stack_init();

//interface
extern int pal_ip_stack_up(struct interface *ifp);
extern int pal_ip_stack_down(struct interface *ifp);
extern int pal_ip_stack_update_flag(struct interface *ifp);
extern int pal_ip_stack_set_vr(struct interface *ifp, vrf_id_t vrf_id);
extern int pal_ip_stack_set_mtu(struct interface *ifp, zpl_uint32 mtu);
extern int pal_ip_stack_set_lladdr(struct interface *ifp, zpl_uint8 *mac, zpl_uint32 len);
extern int pal_ip_stack_create(struct interface *ifp);
extern int pal_ip_stack_destroy(struct interface *ifp);
extern int pal_ip_stack_vlan_set(struct interface *ifp, vlan_t vlan);
extern int pal_ip_stack_vlanpri_set(struct interface *ifp, zpl_uint32 pri);
extern int pal_ip_stack_promisc_link(struct interface *ifp, zpl_bool enable);

//ip address
extern int pal_ip_stack_change_dhcp(struct interface *ifp, zpl_bool enable);
extern int pal_ip_stack_ipv4_dstaddr_add(struct interface *ifp, struct connected *ifc);
extern int pal_ip_stack_ipv4_dstaddr_delete(struct interface *ifp, struct connected *ifc);
extern int pal_ip_stack_ipv4_replace(struct interface *ifp, struct connected *ifc);
extern int pal_ip_stack_ipv4_add(struct interface *ifp,struct connected *ifc);
extern int pal_ip_stack_ipv4_delete(struct interface *ifp, struct connected *ifc);

//ip arp
extern int pal_ip_stack_arp_add(struct interface *ifp, struct prefix *address, zpl_uint8 *mac);
extern int pal_ip_stack_arp_delete(struct interface *ifp, struct prefix *address);
extern int pal_ip_stack_arp_request(struct interface *ifp, struct prefix *address);
extern int pal_ip_stack_arp_gratuitousarp_enable(zpl_bool enable);
extern int pal_ip_stack_arp_ttl(zpl_uint32 ttl);
extern int pal_ip_stack_arp_age_timeout(zpl_uint32 timeout);
extern int pal_ip_stack_arp_retry_interval(zpl_uint32 interval);

//route
extern int pal_ip_stack_create_vr(vrf_id_t vrf_id);
extern int pal_ip_stack_delete_vr(vrf_id_t vrf_id);

#ifdef __cplusplus
}
#endif

#endif /* __PAL_DRIVER_H__ */
