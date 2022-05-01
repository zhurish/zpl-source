/*
 * pal_interface.h
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */

#ifndef __PAL_INTERFACE_H__
#define __PAL_INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

//interface
extern int apal_interface_create(struct interface *ifp);
extern int apal_interface_destroy(struct interface *ifp);
extern int apal_interface_change(struct interface *ifp);

extern int apal_interface_up(struct interface *ifp);
extern int apal_interface_down(struct interface *ifp);
extern int apal_interface_refresh_flag(struct interface *ifp);
extern int apal_interface_update_flag(struct interface *ifp, zpl_uint32 flags);
extern int apal_interface_ifindex(char *k_name);

extern int apal_interface_set_vrf(struct interface *ifp, struct ip_vrf *vrf);
extern int apal_interface_set_mtu(struct interface *ifp, zpl_uint32 mtu);
extern int apal_interface_set_lladdr(struct interface *ifp, zpl_uint8 *mac, zpl_uint32 len);
extern int apal_interface_get_lladdr(struct interface *ifp);

extern int apal_interface_vlan_set(struct interface *ifp, vlan_t vlan);
extern int apal_interface_vlanpri_set(struct interface *ifp, zpl_uint32 pri);
extern int apal_interface_promisc_link(struct interface *ifp, zpl_bool enable);

//ip address

extern int apal_interface_ipv4_dstaddr_add(struct interface *ifp, struct connected *ifc);
extern int apal_interface_ipv4_dstaddr_delete(struct interface *ifp, struct connected *ifc);
extern int apal_interface_ipv4_replace(struct interface *ifp, struct connected *ifc);
extern int apal_interface_ipv4_add(struct interface *ifp,struct connected *ifc);
extern int apal_interface_ipv4_delete(struct interface *ifp, struct connected *ifc);
extern int apal_interface_ipv6_add(struct interface *ifp,struct connected *ifc, zpl_bool secondry);
extern int apal_interface_ipv6_delete(struct interface *ifp, struct connected *ifc, zpl_bool secondry);


extern int apal_interface_update_statistics(struct interface *ifp);


#ifdef __cplusplus
}
#endif


#endif /* __PAL_INTERFACE_H__ */
