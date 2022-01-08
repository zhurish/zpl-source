/*
 * Interface's address and mask.
 * Copyright (C) 1997 Kunihiro Ishiguro
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

#ifndef _ZEBRA_CONNECTED_H
#define _ZEBRA_CONNECTED_H

#ifdef __cplusplus
extern "C" {
#endif

extern int connected_same (struct connected *ifc1, struct connected *ifc2);
extern void connected_up_ipv4 (struct interface *, struct connected *);
extern void connected_down_ipv4 (struct interface *, struct connected *);
extern struct connected *connected_lookup(struct interface *, struct prefix *);

#ifdef HAVE_IPV6
extern void connected_up_ipv6 (struct interface *, struct connected *);
extern void connected_down_ipv6 (struct interface *ifp, struct connected *);
#endif /* HAVE_IPV6 */

#ifdef ZPL_KERNEL_STACK_MODULE
extern void
connected_add_ipv4 (struct interface *ifp, zpl_uint32 flags, struct ipstack_in_addr *addr,
		    zpl_uchar prefixlen, struct ipstack_in_addr *broad,
		    const char *label);
extern void
connected_delete_ipv4 (struct interface *ifp, zpl_uint32 flags, struct ipstack_in_addr *addr,
		       zpl_uchar prefixlen, struct ipstack_in_addr *broad);
#ifdef HAVE_IPV6
extern void
connected_add_ipv6 (struct interface *ifp, zpl_uint32 flags, struct ipstack_in6_addr *address,
		    zpl_uchar prefixlen, struct ipstack_in6_addr *broad,
		    const char *label);
extern void
connected_delete_ipv6 (struct interface *ifp, struct ipstack_in6_addr *address,
		       zpl_uchar prefixlen, struct ipstack_in6_addr *broad);
#endif /* HAVE_IPV6 */
#endif
 
#ifdef __cplusplus
}
#endif

#endif /*_ZEBRA_CONNECTED_H */
