/* Header file exported by rt_netlink.c to zebra.
 * Copyright (C) 1997, 98, 99 Kunihiro Ishiguro
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

#ifndef _ZEBRA_RT_NETLINK_H
#define _ZEBRA_RT_NETLINK_H

#ifdef HAVE_NETLINK

#define NL_PKT_BUF_SIZE 8192
#define NL_DEFAULT_ROUTE_METRIC 20


int
kernel_address_add_ipv4 (struct interface *ifp, struct connected *ifc);
int
kernel_address_delete_ipv4 (struct interface *ifp, struct connected *ifc);

int
kernel_route_rib (struct prefix *p, struct rib *old, struct rib *new);

extern int interface_lookup_netlink (struct nsm_vrf *zvrf);
extern int netlink_route_read (struct nsm_vrf *zvrf);

void
kernel_init (struct nsm_vrf *zvrf);
void
kernel_terminate (struct nsm_vrf *zvrf);


#endif /* HAVE_NETLINK */

#endif /* _ZEBRA_RT_NETLINK_H */
