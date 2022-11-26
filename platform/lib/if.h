/* Interface related header.
   Copyright (C) 1997, 98, 99 Kunihiro Ishiguro

This file is part of GNU Zebra.

GNU Zebra is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 2, or (at your
option) any later version.

GNU Zebra is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Zebra; see the file COPYING.  If not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#ifndef __LIB_IF_H
#define __LIB_IF_H

#ifdef __cplusplus
extern "C" {
#endif



#include "prefix.h"
#include "if_def.h"



/* Prototypes. */
extern struct list *if_list_get(void);
extern int if_hook_add(int (*add_cb)(struct interface *), int (*del_cb)(struct interface *));
extern int if_new_llc_type_mode(zpl_uint32 llc, zpl_uint32 mode);
extern int if_make_llc_type(struct interface *ifp);
extern int if_cmp_func(struct interface *, struct interface *);
extern struct interface *if_create(const char *name, zpl_uint32 namelen);
extern struct interface *if_create_dynamic(const char *name, zpl_uint32 namelen);
extern struct interface *if_lookup_by_index(ifindex_t);
extern struct interface *if_lookup_exact_address(struct ipstack_in_addr);
extern struct interface *if_lookup_address(struct ipstack_in_addr);
extern struct interface *if_lookup_prefix(struct prefix *prefix);
extern struct interface *if_create_vrf_dynamic(const char *name, zpl_uint32 namelen, vrf_id_t vrf_id);
extern struct interface *if_create_vrf(const char *name, zpl_uint32 namelen,
                                       vrf_id_t vrf_id);
extern struct interface *if_lookup_by_index_vrf(ifindex_t, vrf_id_t vrf_id);
extern struct interface *if_lookup_exact_address_vrf(struct ipstack_in_addr,
                                                     vrf_id_t vrf_id);
extern struct interface *if_lookup_address_vrf(struct ipstack_in_addr,
                                               vrf_id_t vrf_id);
extern struct interface *if_lookup_prefix_vrf(struct prefix *prefix,
                                              vrf_id_t vrf_id);

/* These 2 functions are to be used when the ifname argument is terminated
   by a '\0' character: */
extern struct interface *if_lookup_by_name(const char *ifname);

extern struct interface *if_lookup_by_name_vrf(const char *ifname,
                                               vrf_id_t vrf_id);

/* For these 2 functions, the namelen argument should be the precise length
   of the ifname string (not counting any optional trailing '\0' character).
   In most cases, strnlen should be used to calculate the namelen value. */
extern struct interface *if_lookup_by_name_len(const char *ifname,
                                               zpl_uint32 namelen);

extern struct interface *if_lookup_by_name_len_vrf(const char *ifname,
                                                   zpl_uint32 namelen, vrf_id_t vrf_id);


extern zpl_uint32 if_count_lookup_type(if_type_t type);
#ifdef IF_ENCAPSULATION_ENABLE
extern const char *if_encapsulation_string(if_enca_t enca);
#endif
extern int if_name_set(struct interface *, const char *str);
extern int if_kname_set(struct interface *, const char *str);

/* Delete and free the interface structure: calls if_delete_retain and then
   deletes it from the interface list and frees the structure. */
extern void if_delete(struct interface *);

extern zpl_bool if_is_up(struct interface *);
extern zpl_bool if_is_running(struct interface *);
extern zpl_bool if_is_operative(struct interface *);
extern zpl_bool if_is_loopback(struct interface *);
extern zpl_bool if_is_broadcast(struct interface *);
extern zpl_bool if_is_pointopoint(struct interface *);
extern zpl_bool if_is_multicast(struct interface *);
extern zpl_bool if_is_serial(struct interface *ifp);
extern zpl_bool if_is_ethernet(struct interface *ifp);
extern zpl_bool if_is_tunnel(struct interface *ifp);
extern zpl_bool if_is_lag(struct interface *ifp);
extern zpl_bool if_is_lag_member(struct interface *ifp);
extern zpl_bool if_is_vlan(struct interface *ifp);
extern zpl_bool if_is_brigde(struct interface *ifp);
extern zpl_bool if_is_brigde_member(struct interface *ifp);
extern zpl_bool if_is_loop(struct interface *ifp);
extern zpl_bool if_is_wireless(struct interface *ifp);
extern zpl_bool if_is_l3intf(struct interface *ifp);
extern zpl_bool if_is_online(struct interface *);

extern int if_online(struct interface *ifp, zpl_bool enable);


extern void if_init(void);
extern void if_terminate(void);

extern void if_dump_all(void);
extern const char *if_flag_dump(zpl_ulong);
extern const char *if_link_type_str(enum if_link_type);

/* Please use ifindex2ifname instead of if_indextoname where possible;
   ifindex2ifname uses internal interface info, whereas if_indextoname must
   make a system call. */
extern const char *ifindex2ifname(ifindex_t);
extern const char *ifindex2ifname_vrf(ifindex_t, vrf_id_t vrf_id);

extern struct interface *if_lookup_by_kernel_name(const char *ifname);
extern struct interface *if_lookup_by_kernel_name_vrf(const char *ifname,
                                                      vrf_id_t vrf_id);
extern struct interface *if_lookup_by_kernel_index_vrf(ifindex_t kifindex, vrf_id_t vrf_id);
extern struct interface *if_lookup_by_kernel_index(ifindex_t kifindex);

extern const char *ifkernelindex2kernelifname(ifindex_t);
extern const char *ifkernelindex2kernelifname_vrf(ifindex_t, vrf_id_t vrf_id);
extern ifindex_t ifname2kernelifindex(const char *ifname);
extern ifindex_t ifname2kernelifindex_vrf(const char *ifname, vrf_id_t vrf_id);
extern ifindex_t ifindex2ifkernel(ifindex_t);
extern ifindex_t ifkernel2ifindex(ifindex_t);

extern ifindex_t  vlanif2ifindex(vlan_t vid);
/* Please use ifname2ifindex instead of if_nametoindex where possible;
   ifname2ifindex uses internal interface info, whereas if_nametoindex must
   make a system call. */
   
extern ifindex_t ifname2ifindex(const char *ifname);
extern ifindex_t ifname2ifindex_vrf(const char *ifname, vrf_id_t vrf_id);

extern zpl_phyport_t  if_ifindex2phy(ifindex_t ifindex);
extern ifindex_t  if_phy2ifindex(zpl_phyport_t phyid);
extern vrf_id_t  if_ifindex2vrfid(ifindex_t ifindex);

extern zpl_phyport_t  if_ifindex2l3intfid(ifindex_t ifindex);
extern ifindex_t  if_l3intfid2ifindex(zpl_phyport_t l3intfid);


//extern unsigned int if_nametoindex (const char *__ifname);
//extern char *if_indextoname (unsigned int __ifindex, char *__ifname);

extern int if_update_phyid(ifindex_t ifindex, zpl_phyport_t phyid);
extern int if_update_phyid2(struct interface *ifp, zpl_phyport_t phyid);
extern int if_update_l3intfid(ifindex_t ifindex, zpl_phyport_t phyid);
extern int if_update_l3intfid2(struct interface *ifp, zpl_phyport_t phyid);

extern void *if_module_data(struct interface *ifp, module_t mid);

extern zpl_bool if_have_kernel(struct interface *ifp);
extern int if_kernelname_set(struct interface *ifp);

extern int if_list_each(int (*cb)(struct interface *ifp, void *pVoid), void *pVoid);

/* Connected address functions. */
extern struct connected *connected_new(void);
extern void connected_free(struct connected *);
extern void connected_add(struct interface *, struct connected *);
extern struct connected *connected_add_by_prefix(struct interface *,
                                                 struct prefix *,
                                                 struct prefix *);
extern struct connected *connected_delete_by_prefix(struct interface *,
                                                    struct prefix *);
extern struct connected *connected_lookup_address(struct interface *,
                                                  struct ipstack_in_addr);
extern struct connected *connected_check(struct interface *ifp, struct prefix *p);


extern int connected_same (struct connected *ifc1, struct connected *ifc2);
extern struct connected *connected_lookup(struct interface *, struct prefix *);

extern int if_data_lock(void);
extern int if_data_unlock(void);

extern enum if_link_type netlink_to_if_link_type(zpl_uint32  hwt);
 
#include "if_name.h"

#ifdef __cplusplus
}
#endif

#endif /* __LIB_IF_H */
