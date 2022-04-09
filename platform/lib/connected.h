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
#include "zpl_type.h"
#include "auto_include.h"
#include "if.h"
#include "prefix.h"

/* Connected address structure. */
struct connected
{
   /* Attached interface. */
   struct interface *ifp;

   /* Flags for configuration. */
   zpl_uchar conf;
#define ZEBRA_IFC_CONFIGURED (1 << 1)
#define ZEBRA_IFC_DHCPC (1 << 2)
   /*
     The ZEBRA_IFC_REAL flag should be set if and only if this address
     exists in the kernel and is actually usable. (A case where it exists but
     is not yet usable would be IPv6 with DAD)
     The ZEBRA_IFC_CONFIGURED flag should be set if and only if this address
     was configured by the user from inside quagga.
     The ZEBRA_IFC_QUEUED flag should be set if and only if the address exists
     in the kernel. It may and should be set although the address might not be
     usable yet. (compare with ZEBRA_IFC_REAL)
   */

   /* Flags for connected address. */
   zpl_uchar flags;
#define ZEBRA_IFA_SECONDARY (1 << 0)
#define ZEBRA_IFA_PEER (1 << 1)
#define ZEBRA_IFA_UNNUMBERED (1 << 2)
#define ZEBRA_IFA_DHCPC (1 << 3)
   /* N.B. the ZEBRA_IFA_PEER flag should be set if and only if
     a peer address has been configured.  If this flag is set,
     the destination field must contain the peer address.  
     Otherwise, if this flag is not set, the destination address
     will either contain a broadcast address or be NULL.
   */

   /* Address of connected network. */
   struct prefix *address;

   /* Peer or Broadcast address, depending on whether ZEBRA_IFA_PEER is set.
     Note: destination may be NULL if ZEBRA_IFA_PEER is not set. */
   struct prefix *destination;

   zpl_uint32 count;
   zpl_uint32 raw_status;
};

/* Does the destination field contain a peer address? */
#define CONNECTED_PEER(C) CHECK_FLAG((C)->flags, ZEBRA_IFA_PEER)

/* Prefix to insert into the RIB */
#define CONNECTED_PREFIX(C) \
   (CONNECTED_PEER(C) ? (C)->destination : (C)->address)

/* Identifying address.  We guess that if there's a peer address, but the
   local address is in the same prefix, then the local address may be unique. */
#define CONNECTED_ID(C) \
   ((CONNECTED_PEER(C) && !prefix_match((C)->destination, (C)->address)) ? (C)->destination : (C)->address)


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
extern void connected_up_ipv4 (struct interface *, struct connected *);
extern void connected_down_ipv4 (struct interface *, struct connected *);
extern struct connected *connected_lookup(struct interface *, struct prefix *);

#ifdef ZPL_BUILD_IPV6
extern void connected_up_ipv6 (struct interface *, struct connected *);
extern void connected_down_ipv6 (struct interface *ifp, struct connected *);
#endif /* ZPL_BUILD_IPV6 */


extern void
connected_add_ipv4 (struct interface *ifp, zpl_uint32 flags, struct ipstack_in_addr *addr,
		    zpl_uchar prefixlen, struct ipstack_in_addr *broad,
		    const char *label);
extern void
connected_delete_ipv4 (struct interface *ifp, zpl_uint32 flags, struct ipstack_in_addr *addr,
		       zpl_uchar prefixlen, struct ipstack_in_addr *broad);
#ifdef ZPL_BUILD_IPV6
extern void
connected_add_ipv6 (struct interface *ifp, zpl_uint32 flags, struct ipstack_in6_addr *address,
		    zpl_uchar prefixlen, struct ipstack_in6_addr *broad,
		    const char *label);
extern void
connected_delete_ipv6 (struct interface *ifp, struct ipstack_in6_addr *address,
		       zpl_uchar prefixlen, struct ipstack_in6_addr *broad);
#endif /* ZPL_BUILD_IPV6 */

 
#ifdef __cplusplus
}
#endif

#endif /*_ZEBRA_CONNECTED_H */
