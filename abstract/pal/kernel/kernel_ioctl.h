/*
 * Common ioctl functions.
 * Copyright (C) 1998 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

#ifndef _ZEBRA_IOCTL_H
#define _ZEBRA_IOCTL_H

#if 1
extern void ifreq_set_name (struct ifreq *ifreq, struct interface *ifp);
extern int if_ioctl (u_long, caddr_t);
#ifdef HAVE_IPV6
extern int if_ioctl_ipv6 (u_long request, caddr_t buffer);
#endif

extern int ip_arp_stack_init();


extern int _ipkernel_linux_create (nsm_veth_t *kifp);
extern int _ipkernel_linux_destroy (nsm_veth_t *kifp);
extern int _ipkernel_linux_change (nsm_veth_t *kifp, int vlan);

extern int _ipkernel_tunnel_create(nsm_tunnel_t *tunnel);
extern int _ipkernel_tunnel_delete(nsm_tunnel_t *tunnel);
extern int _ipkernel_tunnel_change(nsm_tunnel_t *tunnel);


extern int _ipkernel_bridge_create(nsm_bridge_t *br);
extern int _ipkernel_bridge_delete(nsm_bridge_t *br);
extern int _ipkernel_bridge_add_interface(nsm_bridge_t *br, int ifindex);
extern int _ipkernel_bridge_del_interface(nsm_bridge_t *br, int ifindex);


extern int _ipkernel_bond_create(struct interface *ifp);
extern int _ipkernel_bond_delete(struct interface *ifp);


#else
/* Prototypes. */
extern void ifreq_set_name (struct ifreq *, struct interface *);
extern int if_ioctl (u_long, caddr_t);

extern int if_set_flags (struct interface *, uint64_t);
extern int if_unset_flags (struct interface *, uint64_t);
extern void if_get_flags (struct interface *);

extern int if_set_prefix (struct interface *, struct connected *);
extern int if_unset_prefix (struct interface *, struct connected *);

extern void if_get_metric (struct interface *);
extern void if_get_mtu (struct interface *);

#ifdef HAVE_IPV6
extern int if_prefix_add_ipv6 (struct interface *, struct connected *);
extern int if_prefix_delete_ipv6 (struct interface *, struct connected *);
#endif /* HAVE_IPV6 */

#ifdef SOLARIS_IPV6
extern int if_ioctl_ipv6(u_long, caddr_t);
extern struct connected *if_lookup_linklocal( struct interface *);

#define AF_IOCTL(af, request, buffer) \
        ((af) == AF_INET? if_ioctl(request, buffer) : \
                          if_ioctl_ipv6(request, buffer))
#else /* SOLARIS_IPV6 */

#define AF_IOCTL(af, request, buffer)  if_ioctl(request, buffer)

#endif /* SOLARIS_IPV6 */
#endif

#endif /* _ZEBRA_IOCTL_H */
