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

#ifdef __cplusplus
extern "C" {
#endif


extern void _ipkernel_ifreq_set_name (struct ipstack_ifreq *ipstack_ifreq, struct interface *ifp);
extern int _ipkernel_if_ioctl (zpl_uint32, caddr_t);
#ifdef HAVE_IPV6
extern int _ipkernel_if_ioctl_ipv6 (zpl_uint32 request, caddr_t buffer);
#endif

int _ipkernel_if_set_up(struct interface *ifp);
int _ipkernel_if_set_down(struct interface *ifp);
int _ipkernel_if_update_flags(struct interface *ifp, uint64_t flag);
int _ipkernel_if_get_flags(struct interface *ifp);
int _ipkernel_if_set_mtu(struct interface *ifp, zpl_uint32 mtu);
int _ipkernel_if_get_hwaddr(struct interface *ifp);
int _ipkernel_if_set_mac(struct interface *ifp, zpl_uint8 *mac, zpl_uint32 len);
int _ipkernel_if_set_metric(struct interface *ifp, zpl_uint32 metric);
int _ipkernel_if_set_dst_prefix(struct interface *ifp, struct connected *ifc);
int _ipkernel_if_unset_dst_prefix(struct interface *ifp, struct connected *ifc);
int _ipkernel_if_set_prefix(struct interface *ifp, struct connected *ifc);
int _ipkernel_if_unset_prefix(struct interface *ifp, struct connected *ifc);

#ifdef HAVE_IPV6
int _ipkernel_if_prefix_add_ipv6(struct interface *ifp, struct connected *ifc, int sec);
int _ipkernel_if_prefix_delete_ipv6(struct interface *ifp, struct connected *ifc, int sec);
#endif


#ifdef __cplusplus
}
#endif

#endif /* _ZEBRA_IOCTL_H */
