/* Kernel routing table updates using netlink over GNU/Linux system.
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

#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"

#include "kernel_netlink.h"



/* This function takes a nexthop as argument and adds
 * the appropriate netlink attributes to an existing
 * netlink message.
 *
 * @param routedesc: Human readable description of route type
 *                   (direct/recursive, single-/multipath)
 * @param bytelen: Length of addresses in bytes.
 * @param nexthop: Nexthop information
 * @param nlmsg: nlmsghdr structure to fill in.
 * @param req_size: The size allocated for the message.
 */
static void _netlink_route_build_singlepath(const char *routedesc, zpl_uint32 bytelen,
		struct nexthop *nexthop, struct nlmsghdr *nlmsg, struct rtmsg *rtmsg,
		size_t req_size)
{
	if (CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_ONLINK))
		rtmsg->rtm_flags |= RTNH_F_ONLINK;
	if (nexthop->type == NEXTHOP_TYPE_IPV4
			|| nexthop->type == NEXTHOP_TYPE_IPV4_IFINDEX)
	{
		addattr_l(nlmsg, req_size, RTA_GATEWAY, &nexthop->gate.ipv4, bytelen);
		if (nexthop->src.ipv4.s_addr)
			addattr_l(nlmsg, req_size, RTA_PREFSRC, &nexthop->src.ipv4,
					bytelen);

		if (IS_ZEBRA_DEBUG_KERNEL)
			zlog_debug(MODULE_PAL, "netlink_route_multipath() (%s): "
					"nexthop via %s if %u", routedesc,
					inet_ntoa(nexthop->gate.ipv4), nexthop->ifindex);
	}
#ifdef HAVE_IPV6
	if (nexthop->type == NEXTHOP_TYPE_IPV6
			|| nexthop->type == NEXTHOP_TYPE_IPV6_IFNAME
			|| nexthop->type == NEXTHOP_TYPE_IPV6_IFINDEX)
	{
		addattr_l (nlmsg, req_size, RTA_GATEWAY,
				&nexthop->gate.ipv6, bytelen);

		if (IS_ZEBRA_DEBUG_KERNEL)
		zlog_debug(MODULE_PAL, "netlink_route_multipath() (%s): "
				"nexthop via %s if %u",
				routedesc,
				inet6_ntoa (nexthop->gate.ipv6),
				nexthop->ifindex);
	}
#endif /* HAVE_IPV6 */
	if (nexthop->type == NEXTHOP_TYPE_IFINDEX
			|| nexthop->type == NEXTHOP_TYPE_IFNAME
			|| nexthop->type == NEXTHOP_TYPE_IPV4_IFINDEX)
	{
		addattr32(nlmsg, req_size, RTA_OIF, ifindex2ifkernel(nexthop->ifindex));

		if (nexthop->src.ipv4.s_addr)
			addattr_l(nlmsg, req_size, RTA_PREFSRC, &nexthop->src.ipv4,
					bytelen);

		if (IS_ZEBRA_DEBUG_KERNEL)
			zlog_debug(MODULE_PAL, "netlink_route_multipath() (%s): "
					"nexthop via if %u", routedesc, nexthop->ifindex);
	}

	if (nexthop->type == NEXTHOP_TYPE_IPV6_IFINDEX
			|| nexthop->type == NEXTHOP_TYPE_IPV6_IFNAME)
	{
		addattr32(nlmsg, req_size, RTA_OIF, ifindex2ifkernel(nexthop->ifindex));

		if (IS_ZEBRA_DEBUG_KERNEL)
			zlog_debug(MODULE_PAL, "netlink_route_multipath() (%s): "
					"nexthop via if %u", routedesc, nexthop->ifindex);
	}
}

/* This function takes a nexthop as argument and
 * appends to the given rtattr/rtnexthop pair the
 * representation of the nexthop. If the nexthop
 * defines a preferred source, the src parameter
 * will be modified to point to that src, otherwise
 * it will be kept unmodified.
 *
 * @param routedesc: Human readable description of route type
 *                   (direct/recursive, single-/multipath)
 * @param bytelen: Length of addresses in bytes.
 * @param nexthop: Nexthop information
 * @param rta: rtnetlink attribute structure
 * @param rtnh: pointer to an rtnetlink nexthop structure
 * @param src: pointer pointing to a location where
 *             the prefsrc should be stored.
 */
static void _netlink_route_build_multipath(const char *routedesc, zpl_uint32 bytelen,
		struct nexthop *nexthop, struct rtattr *rta, struct rtnexthop *rtnh,
		union g_addr **src)
{
	rtnh->rtnh_len = sizeof(*rtnh);
	rtnh->rtnh_flags = 0;
	rtnh->rtnh_hops = 0;
	rta->rta_len += rtnh->rtnh_len;

	if (CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_ONLINK))
		rtnh->rtnh_flags |= RTNH_F_ONLINK;

	if (nexthop->type == NEXTHOP_TYPE_IPV4
			|| nexthop->type == NEXTHOP_TYPE_IPV4_IFINDEX)
	{
		rta_addattr_l(rta, NL_PKT_BUF_SIZE, RTA_GATEWAY, &nexthop->gate.ipv4,
				bytelen);
		rtnh->rtnh_len += sizeof(struct rtattr) + bytelen;

		if (nexthop->src.ipv4.s_addr)
			*src = &nexthop->src;

		if (IS_ZEBRA_DEBUG_KERNEL)
			zlog_debug(MODULE_PAL, "netlink_route_multipath() (%s): "
					"nexthop via %s if %u", routedesc,
					inet_ntoa(nexthop->gate.ipv4), nexthop->ifindex);
	}
#ifdef HAVE_IPV6
	if (nexthop->type == NEXTHOP_TYPE_IPV6
			|| nexthop->type == NEXTHOP_TYPE_IPV6_IFNAME
			|| nexthop->type == NEXTHOP_TYPE_IPV6_IFINDEX)
	{
		rta_addattr_l (rta, NL_PKT_BUF_SIZE, RTA_GATEWAY,
				&nexthop->gate.ipv6, bytelen);
		rtnh->rtnh_len += sizeof (struct rtattr) + bytelen;

		if (IS_ZEBRA_DEBUG_KERNEL)
		zlog_debug(MODULE_PAL, "netlink_route_multipath() (%s): "
				"nexthop via %s if %u",
				routedesc,
				inet6_ntoa (nexthop->gate.ipv6),
				nexthop->ifindex);
	}
#endif /* HAVE_IPV6 */
	/* ifindex */
	if (nexthop->type == NEXTHOP_TYPE_IPV4_IFINDEX
			|| nexthop->type == NEXTHOP_TYPE_IFINDEX
			|| nexthop->type == NEXTHOP_TYPE_IFNAME)
	{
		rtnh->rtnh_ifindex = ifindex2ifkernel(nexthop->ifindex) ;
		if (nexthop->src.ipv4.s_addr)
			*src = &nexthop->src;
		if (IS_ZEBRA_DEBUG_KERNEL)
			zlog_debug(MODULE_PAL, "netlink_route_multipath() (%s): "
					"nexthop via if %u", routedesc, nexthop->ifindex);
	}
	else if (nexthop->type == NEXTHOP_TYPE_IPV6_IFNAME
			|| nexthop->type == NEXTHOP_TYPE_IPV6_IFINDEX)
	{
		rtnh->rtnh_ifindex = ifindex2ifkernel(nexthop->ifindex);

		if (IS_ZEBRA_DEBUG_KERNEL)
			zlog_debug(MODULE_PAL, "netlink_route_multipath() (%s): "
					"nexthop via if %u", routedesc, nexthop->ifindex);
	}
	else
	{
		rtnh->rtnh_ifindex = 0;
	}
}

/* Routing table change via netlink interface. */
static int netlink_route_multipath(zpl_uint32 cmd, struct prefix *p, struct rib *rib)
{
	zpl_uint32 bytelen;
	struct sockaddr_nl snl;
	struct nexthop *nexthop = NULL, *tnexthop;
	zpl_uint32 recursing;
	zpl_uint32 nexthop_num;
	zpl_uint32 discard;
	zpl_family_t family = PREFIX_FAMILY(p);
	const char *routedesc;

	struct
	{
		struct nlmsghdr n;
		struct rtmsg r;
		char buf[NL_PKT_BUF_SIZE];
	} req;

	struct nsm_vrf *zvrf = vrf_info_lookup(rib->vrf_id);

	memset(&req, 0, sizeof req - NL_PKT_BUF_SIZE);

	bytelen = (family == AF_INET ? 4 : 16);

	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
	req.n.nlmsg_flags = NLM_F_CREATE | NLM_F_REPLACE | NLM_F_REQUEST;
	req.n.nlmsg_type = cmd;
	req.r.rtm_family = family;
	req.r.rtm_table = rib->table;
	req.r.rtm_dst_len = p->prefixlen;
	req.r.rtm_protocol = RTPROT_ZEBRA;
	req.r.rtm_scope = RT_SCOPE_LINK;

	if ((rib->flags & ZEBRA_FLAG_BLACKHOLE) || (rib->flags & ZEBRA_FLAG_REJECT))
		discard = 1;
	else
		discard = 0;

	if (cmd == RTM_NEWROUTE)
	{
		if (discard)
		{
			if (rib->flags & ZEBRA_FLAG_BLACKHOLE)
				req.r.rtm_type = RTN_BLACKHOLE;
			else if (rib->flags & ZEBRA_FLAG_REJECT)
				req.r.rtm_type = RTN_UNREACHABLE;
			else
				assert(RTN_BLACKHOLE != RTN_UNREACHABLE); /* false */
		}
		else
			req.r.rtm_type = RTN_UNICAST;
	}

	addattr_l(&req.n, sizeof req, RTA_DST, &p->u.prefix, bytelen);

	/* Metric. */
	addattr32(&req.n, sizeof req, RTA_PRIORITY, NL_DEFAULT_ROUTE_METRIC);

	if (rib->mtu || rib->nexthop_mtu)
	{
		char buf[NL_PKT_BUF_SIZE];
		struct rtattr *rta = (void *) buf;
		zpl_uint32 mtu = rib->mtu;
		if (!mtu || (rib->nexthop_mtu && rib->nexthop_mtu < mtu))
			mtu = rib->nexthop_mtu;
		rta->rta_type = RTA_METRICS;
		rta->rta_len = RTA_LENGTH(0);
		rta_addattr_l(rta, NL_PKT_BUF_SIZE, RTAX_MTU, &mtu, sizeof mtu);
		addattr_l(&req.n, NL_PKT_BUF_SIZE, RTA_METRICS, RTA_DATA(rta),
				RTA_PAYLOAD(rta));
	}

	if (discard)
	{
		if (cmd == RTM_NEWROUTE)
			for (ALL_NEXTHOPS_RO(rib->nexthop, nexthop, tnexthop, recursing))
			{
				/* We shouldn't encounter recursive nexthops on discard routes,
				 * but it is probably better to handle that case correctly anyway.
				 */
				if (CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_RECURSIVE))
					continue;
				SET_FLAG(nexthop->flags, NEXTHOP_FLAG_FIB);
			}
		goto skip;
	}

	/* Count overall nexthops so we can decide whether to use singlepath
	 * or multipath case. */
	nexthop_num = 0;
	for (ALL_NEXTHOPS_RO(rib->nexthop, nexthop, tnexthop, recursing))
	{
		if (CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_RECURSIVE))
			continue;
		if (cmd
				== RTM_NEWROUTE&& !CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_ACTIVE))
			continue;
		if (cmd == RTM_DELROUTE && !CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_FIB))
			continue;

		if (nexthop->type != NEXTHOP_TYPE_IFINDEX
				&& nexthop->type != NEXTHOP_TYPE_IFNAME)
			req.r.rtm_scope = RT_SCOPE_UNIVERSE;

		nexthop_num++;
	}

	/* Singlepath case. */
	if (nexthop_num == 1 || MULTIPATH_NUM == 1)
	{
		nexthop_num = 0;
		for (ALL_NEXTHOPS_RO(rib->nexthop, nexthop, tnexthop, recursing))
		{
			if (CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_RECURSIVE))
				continue;

			if ((cmd == RTM_NEWROUTE
					&& CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_ACTIVE))
					|| (cmd == RTM_DELROUTE
							&& CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_FIB)))
			{
				routedesc = recursing ? "recursive, 1 hop" : "single hop";

				_netlink_route_debug(cmd, p, nexthop, routedesc, family, zvrf);
				_netlink_route_build_singlepath(routedesc, bytelen, nexthop,
						&req.n, &req.r, sizeof req);

				if (cmd == RTM_NEWROUTE)
					SET_FLAG(nexthop->flags, NEXTHOP_FLAG_FIB);

				nexthop_num++;
				break;
			}
		}
	}
	else
	{
		char buf[NL_PKT_BUF_SIZE];
		struct rtattr *rta = (void *) buf;
		struct rtnexthop *rtnh;
		union g_addr *src = NULL;

		rta->rta_type = RTA_MULTIPATH;
		rta->rta_len = RTA_LENGTH(0);
		rtnh = RTA_DATA(rta);

		nexthop_num = 0;
		for (ALL_NEXTHOPS_RO(rib->nexthop, nexthop, tnexthop, recursing))
		{
			if (nexthop_num >= MULTIPATH_NUM)
				break;

			if (CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_RECURSIVE))
				continue;

			if ((cmd == RTM_NEWROUTE
					&& CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_ACTIVE))
					|| (cmd == RTM_DELROUTE
							&& CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_FIB)))
			{
				routedesc = recursing ? "recursive, multihop" : "multihop";
				nexthop_num++;

				_netlink_route_debug(cmd, p, nexthop, routedesc, family, zvrf);
				_netlink_route_build_multipath(routedesc, bytelen, nexthop, rta,
						rtnh, &src);
				rtnh = RTNH_NEXT(rtnh);

				if (cmd == RTM_NEWROUTE)
					SET_FLAG(nexthop->flags, NEXTHOP_FLAG_FIB);
			}
		}
		if (src)
			addattr_l(&req.n, sizeof req, RTA_PREFSRC, &src->ipv4, bytelen);

		if (rta->rta_len > RTA_LENGTH(0))
			addattr_l(&req.n, NL_PKT_BUF_SIZE, RTA_MULTIPATH, RTA_DATA(rta),
					RTA_PAYLOAD(rta));
	}

	/* If there is no useful nexthop then return. */
	if (nexthop_num == 0)
	{
		if (IS_ZEBRA_DEBUG_KERNEL)
			zlog_debug(MODULE_PAL,
					"netlink_route_multipath(): No useful nexthop.");
		return 0;
	}

	skip:

	/* Destination netlink address. */
	memset(&snl, 0, sizeof snl);
	snl.nl_family = AF_NETLINK;

	/* Talk to netlink socket. */
	return netlink_talk(&req.n, &zvrf->netlink_cmd, zvrf);
}

int kernel_route_rib(struct prefix *p, struct rib *old, struct rib *new)
{
	if (!old && new)
		return netlink_route_multipath(RTM_NEWROUTE, p, new);
	if (old && !new)
		return netlink_route_multipath(RTM_DELROUTE, p, old);

	/* Replace, can be done atomically if metric does not change;
	 * netlink uses [prefix, tos, priority] to identify prefix.
	 * Now metric is not sent to kernel, so we can just do atomic replace. */
	return netlink_route_multipath(RTM_NEWROUTE, p, new);
}
