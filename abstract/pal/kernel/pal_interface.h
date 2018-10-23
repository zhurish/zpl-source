/*
 * pal_interface.h
 *
 *  Created on: Jan 28, 2018
 *      Author: zhurish
 */

#ifndef __PAL_INTERFACE_H_
#define __PAL_INTERFACE_H_


typedef struct pal_stack_s
{
	//interface
	int (*ip_stack_create)(struct interface *ifp);
	int (*ip_stack_destroy)(struct interface *ifp);
	int (*ip_stack_change)(struct interface *ifp);
	int (*ip_stack_up)(struct interface *ifp);
	int (*ip_stack_down)(struct interface *ifp);
	int (*ip_stack_update_flag)(struct interface *ifp);
	int (*ip_stack_ifindex)(char *name);
	int (*ip_stack_set_vr)(struct interface *ifp, vrf_id_t vrf_id);
	int (*ip_stack_set_mtu)(struct interface *ifp, int mtu);
	int (*ip_stack_set_metric)(struct interface *ifp, int metric);
	int (*ip_stack_get_lladdr)(struct interface *ifp);
	int (*ip_stack_set_lladdr)(struct interface *ifp, unsigned char *mac, int len);
	int (*ip_stack_set_vlan)(struct interface *ifp, int vlan);
	int (*ip_stack_set_vlanpri)(struct interface *ifp, int pri);
	int (*ip_stack_promisc)(struct interface *ifp, BOOL enable);

	//ip address
	int (*ip_stack_dhcp)(struct interface *ifp, BOOL enable);
	int (*ip_stack_ipv4_add)(struct interface *ifp,struct connected *);
	int (*ip_stack_ipv4_delete)(struct interface *ifp,struct connected *);
	int (*ip_stack_ipv4_replace)(struct interface *ifp,struct connected *);
	int (*ip_stack_ipv4_dstaddr_add)(struct interface *ifp, struct connected *);
	int (*ip_stack_ipv4_dstaddr_del)(struct interface *ifp, struct connected *);

	int (*ip_stack_ipv6_add)(struct interface *ifp,struct connected *, int);
	int (*ip_stack_ipv6_delete)(struct interface *ifp,struct connected *, int);

	//ip arp
	int (*ip_stack_arp_add)(struct interface *ifp, struct prefix *address, unsigned char *mac);
	int (*ip_stack_arp_delete)(struct interface *ifp, struct prefix *address);
	int (*ip_stack_arp_request)(struct interface *ifp, struct prefix *address);
	int (*ip_stack_arp_gratuitousarp_enable)(int enable);
	int (*ip_stack_arp_ttl)(int ttl);
	int (*ip_stack_arp_age_timeout)(int timeout);
	int (*ip_stack_arp_retry_interval)(int interval);

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
extern int pal_interface_update_flag(struct interface *ifp);
extern int pal_interface_ifindex(char *k_name);

extern int pal_interface_set_vr(struct interface *ifp, vrf_id_t vrf_id);
extern int pal_interface_set_mtu(struct interface *ifp, int mtu);
extern int pal_interface_set_lladdr(struct interface *ifp, unsigned char *mac, int len);
extern int pal_interface_get_lladdr(struct interface *ifp);

extern int pal_interface_vlan_set(struct interface *ifp, int vlan);
extern int pal_interface_vlanpri_set(struct interface *ifp, int pri);
extern int pal_interface_promisc_link(struct interface *ifp, BOOL enable);

//ip address
extern int pal_interface_change_dhcp(struct interface *ifp, BOOL enable);
extern int pal_interface_ipv4_dstaddr_add(struct interface *ifp, struct connected *ifc);
extern int pal_interface_ipv4_dstaddr_delete(struct interface *ifp, struct connected *ifc);
extern int pal_interface_ipv4_replace(struct interface *ifp, struct connected *ifc);
extern int pal_interface_ipv4_add(struct interface *ifp,struct connected *ifc);
extern int pal_interface_ipv4_delete(struct interface *ifp, struct connected *ifc);
extern int pal_interface_ipv6_add(struct interface *ifp,struct connected *ifc, int secondry);
extern int pal_interface_ipv6_delete(struct interface *ifp, struct connected *ifc, int secondry);
//ip arp
extern int pal_interface_arp_add(struct interface *ifp, struct prefix *address, unsigned char *mac);
extern int pal_interface_arp_delete(struct interface *ifp, struct prefix *address);
extern int pal_interface_arp_request(struct interface *ifp, struct prefix *address);
extern int pal_arp_gratuitousarp_enable(int enable);
extern int pal_arp_ttl(int ttl);
extern int pal_arp_age_timeout(int timeout);
extern int pal_arp_retry_interval(int interval);

//route
extern int pal_create_vr(vrf_id_t vrf_id);
extern int pal_delete_vr(vrf_id_t vrf_id);


extern int pal_interface_update_statistics(struct interface *ifp);


#endif /* __PAL_INTERFACE_H_ */
