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
extern int pal_interface_create(struct interface *ifp);
extern int pal_interface_destroy(struct interface *ifp);
extern int pal_interface_update(struct interface *ifp);

extern int pal_interface_add_slave(struct interface *ifp, struct interface *sifp);
extern int pal_interface_del_slave(struct interface *ifp, struct interface *sifp);

extern int pal_interface_up(struct interface *ifp);
extern int pal_interface_down(struct interface *ifp);
extern int pal_interface_refresh_flag(struct interface *ifp);
extern int pal_interface_update_flag(struct interface *ifp, zpl_uint32 flags);
extern int pal_interface_ifindex(char *k_name);

extern int pal_interface_set_vrf(struct interface *ifp, struct ip_vrf *vrf);
extern int pal_interface_unset_vrf(struct interface *ifp, struct ip_vrf *vrf);
extern int pal_interface_set_mtu(struct interface *ifp, zpl_uint32 mtu);
extern int pal_interface_set_lladdr(struct interface *ifp, zpl_uint8 *mac, zpl_uint32 len);
extern int pal_interface_get_lladdr(struct interface *ifp);

extern int pal_interface_vlan_set(struct interface *ifp, vlan_t vlan);
extern int pal_interface_vlanpri_set(struct interface *ifp, zpl_uint32 pri);

//ip address

extern int pal_interface_ipv4_dstaddr_add(struct interface *ifp, struct connected *ifc);
extern int pal_interface_ipv4_dstaddr_delete(struct interface *ifp, struct connected *ifc);
extern int pal_interface_ipv4_replace(struct interface *ifp, struct connected *ifc);
extern int pal_interface_ipv4_add(struct interface *ifp,struct connected *ifc);
extern int pal_interface_ipv4_delete(struct interface *ifp, struct connected *ifc);
extern int pal_interface_ipv6_add(struct interface *ifp,struct connected *ifc, zpl_bool secondry);
extern int pal_interface_ipv6_delete(struct interface *ifp, struct connected *ifc, zpl_bool secondry);




#ifdef __cplusplus
}
#endif


#endif /* __PAL_INTERFACE_H__ */
