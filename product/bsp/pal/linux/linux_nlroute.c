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

#include "auto_include.h"
#include "zplos_include.h"
#include "zebra_event.h"
#include "module.h"
#include "zmemory.h"
#include "thread.h"
#include "if.h"
#include "vty.h"
#include "command.h"
#include "prefix.h"
#include "vrf.h"
#include "nsm_debug.h"
#include "nsm_rib.h"
#include "nsm_include.h"
#include "hal_include.h"
#include "linux_driver.h"



/* This function takes a nexthop as argument and adds
 * the appropriate netlink attributes to an existing
 * netlink message.
 *
 * @param routedesc: Human readable description of route type
 *                   (direct/recursive, single-/multipath)
 * @param bytelen: Length of addresses in bytes.
 * @param nexthop: Nexthop information
 * @param nlmsg: ipstack_nlmsghdr structure to fill in.
 * @param req_size: The size allocated for the message.
 */
static void _netlink_route_build_nexthop(zpl_uint8 family, hal_nexthop_t *nexthop, 
	struct ipstack_nlmsghdr *nlmsg, struct ipstack_rtmsg *ipstack_rtmsg, size_t req_size)
{
	ipstack_rtmsg->rtm_flags |= IPSTACK_RTNH_F_ONLINK;
	if (nexthop->kifindex && family == IPSTACK_AF_INET)
	{
		_netlink_addattr_l(nlmsg, req_size, IPSTACK_RTA_GATEWAY, &nexthop->gateway.ipv4, IPV4_MAX_BYTELEN);
		_netlink_addattr32(nlmsg, req_size, IPSTACK_RTA_OIF, nexthop->kifindex);
	}
#ifdef ZPL_BUILD_IPV6
	if (nexthop->kifindex && family == IPSTACK_AF_INET6)
	{
		_netlink_addattr_l (nlmsg, req_size, IPSTACK_RTA_GATEWAY,
				&nexthop->gateway.ipv6, IPV6_MAX_BYTELEN);
		_netlink_addattr32(nlmsg, req_size, IPSTACK_RTA_OIF, nexthop->kifindex);		
	}
#endif /* ZPL_BUILD_IPV6 */
}

/* This function takes a nexthop as argument and
 * appends to the given ipstack_rtattr/ipstack_rtnexthop pair the
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
static void _netlink_route_build_more_nexthop(zpl_uint8 family, zpl_uint32 bytelen,
		hal_nexthop_t *nexthop, struct ipstack_rtattr *rta, struct ipstack_rtnexthop *rtnh)
{
	rtnh->rtnh_len = sizeof(*rtnh);
	rtnh->rtnh_flags = 0;
	rtnh->rtnh_hops = 0;
	rta->rta_len += rtnh->rtnh_len;

	rtnh->rtnh_flags |= IPSTACK_RTNH_F_ONLINK;

	if (nexthop->kifindex && family == IPSTACK_AF_INET)
	{
		_netlink_rta_addattr_l(rta, NL_PKT_BUF_SIZE, IPSTACK_RTA_GATEWAY, &nexthop->gateway.ipv4, bytelen);
		rtnh->rtnh_len += sizeof(struct ipstack_rtattr) + bytelen;
		rtnh->rtnh_ifindex = (nexthop->kifindex) ;
	}
#ifdef ZPL_BUILD_IPV6
	if (nexthop->kifindex && family == IPSTACK_AF_INET6)
	{
		_netlink_rta_addattr_l (rta, NL_PKT_BUF_SIZE, IPSTACK_RTA_GATEWAY,
				&nexthop->gateway.ipv6, bytelen);
		rtnh->rtnh_len += sizeof (struct ipstack_rtattr) + bytelen;
		rtnh->rtnh_ifindex = (nexthop->kifindex);
	}
#endif /* ZPL_BUILD_IPV6 */
	else
	{
		rtnh->rtnh_ifindex = 0;
	}
}

/* Routing table change via netlink interface. */
static int _netlink_route_multipath(zpl_uint32 cmd, hal_route_param_t *param)
{
	zpl_uint32 bytelen;
	struct ipstack_sockaddr_nl snl;
	zpl_uint32 nexthop_num;
	zpl_uint32 discard;
	struct
	{
		struct ipstack_nlmsghdr n;
		struct ipstack_rtmsg r;
		char buf[NL_PKT_BUF_SIZE];
	} req;

	memset(&req, 0, sizeof req - NL_PKT_BUF_SIZE);

	bytelen = (param->family == IPSTACK_AF_INET ? 4 : 16);

	req.n.nlmsg_len = IPSTACK_NLMSG_LENGTH(sizeof(struct ipstack_rtmsg));
	req.n.nlmsg_flags = IPSTACK_NLM_F_CREATE | IPSTACK_NLM_F_REPLACE | IPSTACK_NLM_F_REQUEST;
	req.n.nlmsg_type = cmd;
	req.r.rtm_family = param->family;
	req.r.rtm_table = param->table;
	req.r.rtm_dst_len = param->prefixlen;
	req.r.rtm_protocol = IPSTACK_RTPROT_ZEBRA;
	req.r.rtm_scope = IPSTACK_RT_SCOPE_LINK;

	if ((param->flags & ZEBRA_FLAG_BLACKHOLE) || (param->flags & ZEBRA_FLAG_REJECT))
		discard = 1;
	else
		discard = 0;

	if (cmd == IPSTACK_RTM_NEWROUTE)
	{
		if (discard)
		{
			if (param->flags & ZEBRA_FLAG_BLACKHOLE)
				req.r.rtm_type = IPSTACK_RTN_BLACKHOLE;
			else if (param->flags & ZEBRA_FLAG_REJECT)
				req.r.rtm_type = IPSTACK_RTN_UNREACHABLE;
			else
				assert(IPSTACK_RTN_BLACKHOLE != IPSTACK_RTN_UNREACHABLE); /* false */
		}
		else
			req.r.rtm_type = IPSTACK_RTN_UNICAST;
	}
	if(param->family == IPSTACK_AF_INET)
		_netlink_addattr_l(&req.n, sizeof req, IPSTACK_RTA_DST, &param->destination.ipv4, bytelen);
#ifdef ZPL_BUILD_IPV6
	else
		_netlink_addattr_l(&req.n, sizeof req, IPSTACK_RTA_DST, &param->destination.ipv6, bytelen);
#endif
	/* Metric. */
	_netlink_addattr32(&req.n, sizeof req, IPSTACK_RTA_PRIORITY, param->metric);

	if (param->mtu)
	{
		char buf[NL_PKT_BUF_SIZE];
		struct ipstack_rtattr *rta = (void *) buf;
		rta->rta_type = IPSTACK_RTA_METRICS;
		rta->rta_len = IPSTACK_RTA_LENGTH(0);
		_netlink_rta_addattr_l(rta, NL_PKT_BUF_SIZE, IPSTACK_RTAX_MTU, &param->mtu, sizeof param->mtu);
		_netlink_addattr_l(&req.n, NL_PKT_BUF_SIZE, IPSTACK_RTA_METRICS, IPSTACK_RTA_DATA(rta),
				IPSTACK_RTA_PAYLOAD(rta));
	}

	/* Singlepath case. */
	if (param->nexthop_num == 1)
	{
		_netlink_route_debug( cmd, param->family, param->prefixlen, &param->nexthop[0], param->destination, param->vrf_id);

		_netlink_route_build_nexthop(param->family, &param->nexthop[0], &req.n, &req.r, sizeof req);
		if(param->family == IPSTACK_AF_INET && param->source.ipv4.s_addr)
			_netlink_addattr_l(&req.n, sizeof req, IPSTACK_RTA_PREFSRC, &param->source.ipv4, IPV4_MAX_BYTELEN);
#ifdef ZPL_BUILD_IPV6
		else if(param->family == IPSTACK_AF_INET6)
			_netlink_addattr_l(&req.n, sizeof req, IPSTACK_RTA_PREFSRC, &param->source.ipv6, IPV6_MAX_BYTELEN);
#endif
	}
	else if (param->nexthop_num  > 1)
	{
		char buf[NL_PKT_BUF_SIZE];
		struct ipstack_rtattr *rta = (void *) buf;
		struct ipstack_rtnexthop *rtnh;
		union g_addr *src = NULL;

		rta->rta_type = IPSTACK_RTA_MULTIPATH;
		rta->rta_len = IPSTACK_RTA_LENGTH(0);
		rtnh = IPSTACK_RTA_DATA(rta);

		nexthop_num = 0;
		for (nexthop_num = 0; nexthop_num < param->nexthop_num; nexthop_num++)
		{
			_netlink_route_debug( cmd, param->family, param->prefixlen, &param->nexthop[nexthop_num], param->destination, param->vrf_id);
			_netlink_route_build_more_nexthop(param->family, bytelen, &param->nexthop[nexthop_num], rta, rtnh);
			rtnh = IPSTACK_RTNH_NEXT(rtnh);
		}
		if(param->family == IPSTACK_AF_INET && param->source.ipv4.s_addr)
			_netlink_addattr_l(&req.n, sizeof req, IPSTACK_RTA_PREFSRC, &param->source.ipv4, IPV4_MAX_BYTELEN);
#ifdef ZPL_BUILD_IPV6
		else if(param->family == IPSTACK_AF_INET6)
			_netlink_addattr_l(&req.n, sizeof req, IPSTACK_RTA_PREFSRC, &param->source.ipv6, IPV6_MAX_BYTELEN);
#endif
		if (rta->rta_len > IPSTACK_RTA_LENGTH(0))
			_netlink_addattr_l(&req.n, NL_PKT_BUF_SIZE, IPSTACK_RTA_MULTIPATH, IPSTACK_RTA_DATA(rta),
					IPSTACK_RTA_PAYLOAD(rta));
	}

	/* If there is no useful nexthop then return. */
	if (nexthop_num == 0)
	{
		if (IS_ZEBRA_DEBUG_KERNEL)
			zlog_debug(MODULE_PAL,
					"netlink_route_multipath(): No useful nexthop.");
		return 0;
	}
	/* Destination netlink address. */
	memset(&snl, 0, sizeof snl);
	snl.nl_family = IPSTACK_AF_NETLINK;
	/* Talk to netlink ipstack_socket. */
	return _netlink_talk(&req.n, &netlink_cmd, param->vrf_id);
}

int _netlink_route_rib_add(void *p, hal_route_param_t *param)
{
	return _netlink_route_multipath(IPSTACK_RTM_NEWROUTE, param);
}

int _netlink_route_rib_del(void *p, hal_route_param_t *param)
{
	return _netlink_route_multipath(IPSTACK_RTM_DELROUTE, param);
}