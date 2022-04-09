/*
 * Address linked list routine.
 * Copyright (C) 1997, 98 Kunihiro Ishiguro
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

#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "if.h"
#include "log.h"
#include "prefix.h"
#include "zmemory.h"
#include "connected.h"
#ifdef ZPL_NSM_MODULE
#include "nsm_rib.h"
#endif
#ifdef ZPL_DHCP_MODULE
#include "nsm_dhcp.h"
#endif






/* Allocate connected structure. */
struct connected *
connected_new(void)
{
	return XCALLOC(MTYPE_CONNECTED, sizeof(struct connected));
}

/* Free connected structure. */
void connected_free(struct connected *connected)
{
	if (connected->address)
		prefix_free(connected->address);

	if (connected->destination)
		prefix_free(connected->destination);

	XFREE(MTYPE_CONNECTED, connected);
}

/* If two connected address has same prefix return 1. */
static int connected_same_prefix(struct prefix *p1, struct prefix *p2)
{
	if (p1->family == p2->family)
	{
		if (p1->family == IPSTACK_AF_INET &&
			IPV4_ADDR_SAME(&p1->u.prefix4, &p2->u.prefix4))
			return 1;
#ifdef ZPL_BUILD_IPV6
		if (p1->family == IPSTACK_AF_INET6 &&
			IPV6_ADDR_SAME(&p1->u.prefix6, &p2->u.prefix6))
			return 1;
#endif /* ZPL_BUILD_IPV6 */
	}
	return 0;
}

struct connected *
connected_delete_by_prefix(struct interface *ifp, struct prefix *p)
{
	struct listnode *node = NULL;
	struct listnode *next = NULL;
	struct connected *ifc = NULL;

	/* In case of same prefix come, replace it with new one. */
	for (node = listhead(ifp->connected); node; node = next)
	{
		ifc = listgetdata(node);
		next = node->next;

		if (connected_same_prefix(ifc->address, p))
		{
			listnode_delete(ifp->connected, ifc);
			return ifc;
		}
	}
	return NULL;
}

/* Find the IPv4 address on our side that will be used when packets
 are sent to dst. */
struct connected *
connected_lookup_address(struct interface *ifp, struct ipstack_in_addr dst)
{
	struct prefix addr;
	struct listnode *cnode = NULL;
	struct connected *c = NULL;
	struct connected *match = NULL;

	addr.family = IPSTACK_AF_INET;
	addr.u.prefix4 = dst;
	addr.prefixlen = IPV4_MAX_BITLEN;

	match = NULL;

	for (ALL_LIST_ELEMENTS_RO(ifp->connected, cnode, c))
	{
		if (c->address && (c->address->family == IPSTACK_AF_INET) && prefix_match(CONNECTED_PREFIX(c), &addr) && (!match || (c->address->prefixlen > match->address->prefixlen)))
			match = c;
	}
	return match;
}

struct connected *
connected_add_by_prefix(struct interface *ifp, struct prefix *p,
						struct prefix *destination)
{
	struct connected *ifc = NULL;

	/* Allocate new connected address. */
	ifc = connected_new();
	ifc->ifp = ifp;

	/* Fetch interface address */
	ifc->address = prefix_new();
	memcpy(ifc->address, p, sizeof(struct prefix));

	/* Fetch dest address */
	if (destination)
	{
		ifc->destination = prefix_new();
		memcpy(ifc->destination, destination, sizeof(struct prefix));
	}

	/* Add connected address to the interface. */
	listnode_add(ifp->connected, ifc);
	return ifc;
}

/* If same interface address is already exist... */
struct connected *
connected_check(struct interface *ifp, struct prefix *p)
{
	struct connected *ifc = NULL;
	struct listnode *node = NULL;

	for (ALL_LIST_ELEMENTS_RO(ifp->connected, node, ifc))
		if (prefix_same(ifc->address, p))
			return ifc;

	return NULL;
}


struct connected *
connected_lookup(struct interface *ifp, struct prefix *p)
{
	return connected_check(ifp, p);
}

/* Check if two ifc's describe the same address in the same state */
int connected_same(struct connected *ifc1, struct connected *ifc2)
{
	if (ifc1->ifp != ifc2->ifp)
		return 0;

	if (ifc1->destination)
		if (!ifc2->destination)
			return 0;
	if (ifc2->destination)
		if (!ifc1->destination)
			return 0;

	if (ifc1->destination && ifc2->destination)
		if (!prefix_same(ifc1->destination, ifc2->destination))
			return 0;

	if (ifc1->flags != ifc2->flags)
		return 0;

	if (ifc1->conf != ifc2->conf)
		return 0;

	return 1;
}

/* Called from if_up(). */
void connected_up_ipv4(struct interface *ifp, struct connected *ifc)
{
	struct prefix_ipv4 p;

	PREFIX_COPY_IPV4(&p, CONNECTED_PREFIX(ifc));

	/* Apply mask to the network. */
	apply_mask_ipv4(&p);

	/* In case of connected address is 0.0.0.0/0 we treat it tunnel
	 address. */
	if (prefix_ipv4_any(&p))
		return;
#ifdef ZPL_NSM_MODULE
	rib_add_ipv4(ZEBRA_ROUTE_CONNECT, 0, &p, NULL, NULL, ifp->ifindex,
				 ifp->vrf_id, IPSTACK_RT_TABLE_MAIN, ifp->metric, 0, 0, SAFI_UNICAST);

	rib_add_ipv4(ZEBRA_ROUTE_CONNECT, 0, &p, NULL, NULL, ifp->ifindex,
				 ifp->vrf_id, IPSTACK_RT_TABLE_MAIN, ifp->metric, 0, 0, SAFI_MULTICAST);

	rib_update(ifp->vrf_id);
#endif
}

void connected_down_ipv4(struct interface *ifp, struct connected *ifc)
{
	struct prefix_ipv4 p;

	/*
	if (!CHECK_FLAG(ifc->conf, ZEBRA_IFC_REAL))
		return;
*/

	PREFIX_COPY_IPV4(&p, CONNECTED_PREFIX(ifc));

	/* Apply mask to the network. */
	apply_mask_ipv4(&p);

	/* In case of connected address is 0.0.0.0/0 we treat it tunnel
	 address. */
	if (prefix_ipv4_any(&p))
		return;
#ifdef ZPL_NSM_MODULE
	/* Same logic as for connected_up_ipv4(): push the changes into the head. */
	rib_delete_ipv4(ZEBRA_ROUTE_CONNECT, 0, &p, NULL, ifp->ifindex, ifp->vrf_id,
					SAFI_UNICAST);

	rib_delete_ipv4(ZEBRA_ROUTE_CONNECT, 0, &p, NULL, ifp->ifindex, ifp->vrf_id,
					SAFI_MULTICAST);

	rib_update(ifp->vrf_id);
#endif
}

#ifdef ZPL_BUILD_IPV6
void connected_up_ipv6(struct interface *ifp, struct connected *ifc)
{
	struct prefix_ipv6 p;

	/*
	if (! CHECK_FLAG (ifc->conf, ZEBRA_IFC_REAL))
	return;
*/

	PREFIX_COPY_IPV6(&p, CONNECTED_PREFIX(ifc));

	/* Apply mask to the network. */
	apply_mask_ipv6(&p);

#ifndef LINUX
	/* XXX: It is already done by rib_bogus_ipv6 within rib_add_ipv6 */
	if (IPSTACK_IN6_IS_ADDR_UNSPECIFIED(&p.prefix))
		return;
#endif
#ifdef ZPL_NSM_MODULE
	rib_add_ipv6(ZEBRA_ROUTE_CONNECT, 0, &p, NULL, ifp->ifindex, ifp->vrf_id,
				 IPSTACK_RT_TABLE_MAIN, ifp->metric, 0, 0, SAFI_UNICAST);

	rib_update(ifp->vrf_id);
#endif
}

void connected_down_ipv6(struct interface *ifp, struct connected *ifc)
{
	struct prefix_ipv6 p;

	/*
	if (! CHECK_FLAG (ifc->conf, ZEBRA_IFC_REAL))
	return;
*/

	PREFIX_COPY_IPV6(&p, CONNECTED_PREFIX(ifc));

	apply_mask_ipv6(&p);

	if (IPSTACK_IN6_IS_ADDR_UNSPECIFIED(&p.prefix))
		return;
#ifdef ZPL_NSM_MODULE
	rib_delete_ipv6(ZEBRA_ROUTE_CONNECT, 0, &p, NULL, ifp->ifindex, ifp->vrf_id,
					SAFI_UNICAST);

	rib_update(ifp->vrf_id);
#endif
}
#endif


/* Add connected IPv4 route to the interface. */
void connected_add_ipv4(struct interface *ifp, zpl_uint32 flags, struct ipstack_in_addr *addr,
						zpl_uchar prefixlen, struct ipstack_in_addr *broad,
						const char *label)
{
	struct prefix_ipv4 *p;
	struct connected *ifc;
	if (!addr)
	{
		zlog_warn(MODULE_DEFAULT, "warning: add ip address is NULL on interface %s ",
				  ifp->name);
		return;
	}
	/* Make connected structure. */
	ifc = connected_new();
	ifc->ifp = ifp;
	ifc->flags = flags;
	/* If we get a notification from the kernel,
	 * we can safely assume the address is known to the kernel */
	//SET_FLAG(ifc->conf, ZEBRA_IFC_QUEUED);
	/* Allocate new connected address. */
	p = prefix_ipv4_new();
	p->family = IPSTACK_AF_INET;
	p->prefix = *addr;
	p->prefixlen = prefixlen;
	if (connected_lookup(ifp, p))
	{
		prefix_ipv4_free(p);
		connected_free(ifc);
		return;
	}
	ifc->address = (struct prefix *)p;

	/* If there is broadcast or peer address. */
	if (broad)
	{
		p = prefix_ipv4_new();
		p->family = IPSTACK_AF_INET;
		p->prefix = *broad;
		p->prefixlen = prefixlen;
		ifc->destination = (struct prefix *)p;

		/* validate the destination address */
		if (CONNECTED_PEER(ifc))
		{
			if (IPV4_ADDR_SAME(addr, broad))
				zlog_warn(MODULE_DEFAULT, "warning: interface %s has same local and peer "
									"address %s, routing protocols may malfunction",
						  ifp->name, ipstack_inet_ntoa(*addr));
		}
		else
		{
			if (broad->s_addr != ipv4_broadcast_addr(addr->s_addr, prefixlen))
			{
				zpl_char buf[IPSTACK_INET_ADDRSTRLEN];
				struct ipstack_in_addr bcalc;
				bcalc.s_addr = ipv4_broadcast_addr(addr->s_addr, prefixlen);
				memset(buf, 0, sizeof(buf));
				snprintf(buf, sizeof(buf), "%s", ipstack_inet_ntoa(*broad));

				zlog_warn(MODULE_DEFAULT, "warning: interface %s broadcast addr %s/%d != "
									"calculated %s, routing protocols may malfunction",
						  ifp->name,
						  buf,
						  prefixlen,
						  ipstack_inet_ntoa(bcalc));
			}
		}
	}
	else
	{
		if (CHECK_FLAG(ifc->flags, ZEBRA_IFA_PEER))
		{
			zlog_warn(MODULE_DEFAULT, "warning: %s called for interface %s "
								"with peer flag set, but no peer address supplied",
					  __func__, ifp->name);
			UNSET_FLAG(ifc->flags, ZEBRA_IFA_PEER);
		}

		/* no broadcast or destination address was supplied */
		if ((prefixlen == IPV4_MAX_PREFIXLEN) && if_is_pointopoint(ifp))
			zlog_warn(MODULE_DEFAULT, "warning: PtP interface %s with addr %s/%d needs a "
								"peer address",
					  ifp->name, ipstack_inet_ntoa(*addr), prefixlen);
	}
#ifdef ZPL_DHCP_MODULE
	if (nsm_interface_dhcp_mode_get_api(ifp) == DHCP_CLIENT)
		SET_FLAG(ifc->conf, ZEBRA_IFC_DHCPC);
#endif
	/* Label of this address. */
	//if (label)
	//  ifc->label = XSTRDUP (MTYPE_CONNECTED_LABEL, label);
	/* For all that I know an IPv4 address is always ready when we receive
	 * the notification. So it should be safe to set the REAL flag here. */
	//SET_FLAG(ifc->conf, ZEBRA_IFC_REAL);
	//connected_update(ifp, ifc);
	listnode_add(ifp->connected, ifc);
	connected_up_ipv4(ifp, ifc);
	//rib_update(ifp->vrf_id);
}
/* Delete connected IPv4 route to the interface. */

void connected_delete_ipv4(struct interface *ifp, zpl_uint32 flags, struct ipstack_in_addr *addr,
						   zpl_uchar prefixlen, struct ipstack_in_addr *broad)
{
	struct prefix_ipv4 p;
	struct connected *ifc;
	if (!addr)
	{
		zlog_warn(MODULE_DEFAULT, "warning: del ip address is NULL on interface %s ",
				  ifp->name);
		return;
	}
	memset(&p, 0, sizeof(struct prefix_ipv4));
	p.family = IPSTACK_AF_INET;
	p.prefix = *addr;
	p.prefixlen = prefixlen;

	ifc = connected_check(ifp, (struct prefix *)&p);
	if (!ifc)
		return;

	//connected_withdraw (ifc);
	listnode_delete(ifp->connected, ifc);
	connected_down_ipv4(ifp, ifc);
	//rib_update(ifp->vrf_id);
}
#ifdef ZPL_BUILD_IPV6
/* Add connected IPv6 route to the interface. */
void connected_add_ipv6(struct interface *ifp, zpl_uint32 flags, struct ipstack_in6_addr *addr,
						zpl_uchar prefixlen, struct ipstack_in6_addr *broad,
						const char *label)
{
	struct prefix_ipv6 *p;
	struct connected *ifc;
	if (!addr)
	{
		zlog_warn(MODULE_DEFAULT, "warning: add ipv6 address is NULL on interface %s ",
				  ifp->name);
		return;
	}
	/* Make connected structure. */
	ifc = connected_new();
	ifc->ifp = ifp;
	ifc->flags = flags;
	/* If we get a notification from the kernel,
	 * we can safely assume the address is known to the kernel */
	//SET_FLAG(ifc->conf, ZEBRA_IFC_QUEUED);
	/* Allocate new connected address. */
	p = prefix_ipv6_new();
	p->family = IPSTACK_AF_INET6;
	IPV6_ADDR_COPY(&p->prefix, addr);
	p->prefixlen = prefixlen;
	if (connected_lookup(ifp, p))
	{
		prefix_ipv6_free(p);
		connected_free(ifc);
		return;
	}
	ifc->address = (struct prefix *)p;

	/* If there is broadcast or peer address. */
	if (broad)
	{
		if (IPSTACK_IN6_IS_ADDR_UNSPECIFIED(broad))
			zlog_warn(MODULE_DEFAULT, "warning: %s called for interface %s with unspecified "
								"destination address; ignoring!",
					  __func__, ifp->name);
		else
		{
			p = prefix_ipv6_new();
			p->family = IPSTACK_AF_INET6;
			IPV6_ADDR_COPY(&p->prefix, broad);
			p->prefixlen = prefixlen;
			ifc->destination = (struct prefix *)p;
		}
	}
	if (CHECK_FLAG(ifc->flags, ZEBRA_IFA_PEER) && !ifc->destination)
	{
		zlog_warn(MODULE_DEFAULT, "warning: %s called for interface %s "
							"with peer flag set, but no peer address supplied",
				  __func__,
				  ifp->name);
		UNSET_FLAG(ifc->flags, ZEBRA_IFA_PEER);
	}
#ifdef ZPL_DHCP_MODULE
	if (nsm_interface_dhcp_mode_get_api(ifp) == DHCP_CLIENT)
		SET_FLAG(ifc->conf, ZEBRA_IFC_DHCPC);
#endif
	/* Label of this address. */
	//if (label)
	//  ifc->label = XSTRDUP (MTYPE_CONNECTED_LABEL, label);
	/* On Linux, we only get here when DAD is complete, therefore we can set
	 * ZEBRA_IFC_REAL.
	 *
	 * On BSD, there currently doesn't seem to be a way to check for completion of
	 * DAD, so we replicate the old behaviour and set ZEBRA_IFC_REAL, although DAD
	 * might still be running.
	 */
	//SET_FLAG(ifc->conf, ZEBRA_IFC_REAL);
	//connected_update(ifp, ifc);
	listnode_add(ifp->connected, ifc);
	connected_up_ipv6(ifp, ifc);
	//rib_update(ifp->vrf_id);
}

void connected_delete_ipv6(struct interface *ifp, struct ipstack_in6_addr *address,
						   zpl_uchar prefixlen, struct ipstack_in6_addr *broad)
{
	struct prefix_ipv6 p;
	struct connected *ifc;
	if (!address)
	{
		zlog_warn(MODULE_DEFAULT, "warning: delete ipv6 address is NULL on interface %s ",
				  ifp->name);
		return;
	}
	memset(&p, 0, sizeof(struct prefix_ipv6));
	p.family = IPSTACK_AF_INET6;
	memcpy(&p.prefix, address, sizeof(struct ipstack_in6_addr));
	p.prefixlen = prefixlen;

	ifc = connected_check(ifp, (struct prefix *)&p);
	if (!ifc)
		return;

	//connected_withdraw (ifc);
	listnode_delete(ifp->connected, ifc);
	connected_down_ipv6(ifp, ifc);
	//rib_update(ifp->vrf_id);
}
#endif /* ZPL_BUILD_IPV6 */

