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
#include "zpl_type.h"
#include "module.h"
#include "zmemory.h"
#include "log.h"
#include "if.h"
#include "zclient.h"
#include "nsm_debug.h"
#include "nsm_rib.h"
#include "linux_driver.h"


#ifdef ZPL_KERNEL_NETLINK

#if 0
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
static void librtnl_route_build_nexthop(zpl_uint8 family, hal_nexthop_t *nexthop, 
	struct ipstack_nlmsghdr *nlmsg, struct ipstack_rtmsg *ipstack_rtmsg, size_t req_size)
{
	ipstack_rtmsg->rtm_flags |= IPSTACK_RTNH_F_ONLINK;
	if (nexthop->kifindex && family == IPSTACK_AF_INET)
	{
		librtnl_addattr_l(nlmsg, req_size, IPSTACK_RTA_GATEWAY, &nexthop->gateway.ipv4, IPV4_MAX_BYTELEN);
		librtnl_addattr32(nlmsg, req_size, IPSTACK_RTA_OIF, nexthop->kifindex);
	}
#ifdef ZPL_BUILD_IPV6
	if (nexthop->kifindex && family == IPSTACK_AF_INET6)
	{
		librtnl_addattr_l (nlmsg, req_size, IPSTACK_RTA_GATEWAY,
				&nexthop->gateway.ipv6, IPV6_MAX_BYTELEN);
		librtnl_addattr32(nlmsg, req_size, IPSTACK_RTA_OIF, nexthop->kifindex);		
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
static void librtnl_route_build_more_nexthop(zpl_uint8 family, zpl_uint32 bytelen,
		hal_nexthop_t *nexthop, struct ipstack_rtattr *rta, struct ipstack_rtnexthop *rtnh)
{
	rtnh->rtnh_len = sizeof(*rtnh);
	rtnh->rtnh_flags = 0;
	rtnh->rtnh_hops = 0;
	rta->rta_len += rtnh->rtnh_len;

	rtnh->rtnh_flags |= IPSTACK_RTNH_F_ONLINK;

	if (nexthop->kifindex && family == IPSTACK_AF_INET)
	{
		librtnl_rta_addattr_l(rta, NL_PKT_BUF_SIZE, IPSTACK_RTA_GATEWAY, &nexthop->gateway.ipv4, bytelen);
		rtnh->rtnh_len += sizeof(struct ipstack_rtattr) + bytelen;
		rtnh->rtnh_ifindex = (nexthop->kifindex) ;
	}
#ifdef ZPL_BUILD_IPV6
	if (nexthop->kifindex && family == IPSTACK_AF_INET6)
	{
		librtnl_rta_addattr_l (rta, NL_PKT_BUF_SIZE, IPSTACK_RTA_GATEWAY,
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
static int librtnl_route_multipath(zpl_uint32 cmd, hal_route_param_t *param)
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

	if ((param->flags & NSM_RIB_FLAG_BLACKHOLE) || (param->flags & NSM_RIB_FLAG_REJECT))
		discard = 1;
	else
		discard = 0;

	if (cmd == IPSTACK_RTM_NEWROUTE)
	{
		if (discard)
		{
			if (param->flags & NSM_RIB_FLAG_BLACKHOLE)
				req.r.rtm_type = IPSTACK_RTN_BLACKHOLE;
			else if (param->flags & NSM_RIB_FLAG_REJECT)
				req.r.rtm_type = IPSTACK_RTN_UNREACHABLE;
			else
				assert(IPSTACK_RTN_BLACKHOLE != IPSTACK_RTN_UNREACHABLE); /* false */
		}
		else
			req.r.rtm_type = IPSTACK_RTN_UNICAST;
	}
	if(param->family == IPSTACK_AF_INET)
		librtnl_addattr_l(&req.n, sizeof req, IPSTACK_RTA_DST, &param->destination.ipv4, bytelen);
#ifdef ZPL_BUILD_IPV6
	else
		librtnl_addattr_l(&req.n, sizeof req, IPSTACK_RTA_DST, &param->destination.ipv6, bytelen);
#endif
	/* Metric. */
	librtnl_addattr32(&req.n, sizeof req, IPSTACK_RTA_PRIORITY, param->metric);

	if (param->mtu)
	{
		char buf[NL_PKT_BUF_SIZE];
		struct ipstack_rtattr *rta = (void *) buf;
		rta->rta_type = IPSTACK_RTA_METRICS;
		rta->rta_len = IPSTACK_RTA_LENGTH(0);
		librtnl_rta_addattr_l(rta, NL_PKT_BUF_SIZE, IPSTACK_RTAX_MTU, &param->mtu, sizeof param->mtu);
		librtnl_addattr_l(&req.n, NL_PKT_BUF_SIZE, IPSTACK_RTA_METRICS, IPSTACK_RTA_DATA(rta),
				IPSTACK_RTA_PAYLOAD(rta));
	}

	/* Singlepath case. */
	if (param->nexthop_num == 1)
	{
		librtnl_route_debug( cmd, param->family, param->prefixlen, &param->nexthop[0], param->destination, param->vrf_id);

		librtnl_route_build_nexthop(param->family, &param->nexthop[0], &req.n, &req.r, sizeof req);
		if(param->family == IPSTACK_AF_INET && param->source.ipv4.s_addr)
			librtnl_addattr_l(&req.n, sizeof req, IPSTACK_RTA_PREFSRC, &param->source.ipv4, IPV4_MAX_BYTELEN);
#ifdef ZPL_BUILD_IPV6
		else if(param->family == IPSTACK_AF_INET6)
			librtnl_addattr_l(&req.n, sizeof req, IPSTACK_RTA_PREFSRC, &param->source.ipv6, IPV6_MAX_BYTELEN);
#endif
	}
	else if (param->nexthop_num  > 1)
	{
		char buf[NL_PKT_BUF_SIZE];
		struct ipstack_rtattr *rta = (void *) buf;
		struct ipstack_rtnexthop *rtnh;

		rta->rta_type = IPSTACK_RTA_MULTIPATH;
		rta->rta_len = IPSTACK_RTA_LENGTH(0);
		rtnh = IPSTACK_RTA_DATA(rta);

		nexthop_num = 0;
		for (nexthop_num = 0; nexthop_num < param->nexthop_num; nexthop_num++)
		{
			librtnl_route_debug( cmd, param->family, param->prefixlen, &param->nexthop[nexthop_num], param->destination, param->vrf_id);
			librtnl_route_build_more_nexthop(param->family, bytelen, &param->nexthop[nexthop_num], rta, rtnh);
			rtnh = IPSTACK_RTNH_NEXT(rtnh);
		}
		if(param->family == IPSTACK_AF_INET && param->source.ipv4.s_addr)
			librtnl_addattr_l(&req.n, sizeof req, IPSTACK_RTA_PREFSRC, &param->source.ipv4, IPV4_MAX_BYTELEN);
#ifdef ZPL_BUILD_IPV6
		else if(param->family == IPSTACK_AF_INET6)
			librtnl_addattr_l(&req.n, sizeof req, IPSTACK_RTA_PREFSRC, &param->source.ipv6, IPV6_MAX_BYTELEN);
#endif
		if (rta->rta_len > IPSTACK_RTA_LENGTH(0))
			librtnl_addattr_l(&req.n, NL_PKT_BUF_SIZE, IPSTACK_RTA_MULTIPATH, IPSTACK_RTA_DATA(rta),
					IPSTACK_RTA_PAYLOAD(rta));
	}

	/* If there is no useful nexthop then return. */
	if (nexthop_num == 0)
	{
		if (IS_NSM_DEBUG_KERNEL)
			zlog_debug(MODULE_PAL,
					"librtnl_route_multipath(): No useful nexthop.");
		return 0;
	}
	/* Destination netlink address. */
	memset(&snl, 0, sizeof snl);
	snl.nl_family = IPSTACK_AF_NETLINK;
	/* Talk to netlink ipstack_socket. */
	return librtnl_talk(&netlink_cmd, &req.n);
}

#else

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
static void librtnl_route_build_singlepath(
        const char *routedesc,
        int bytelen,
        struct nexthop *nexthop,
        struct ipstack_nlmsghdr *nlmsg,
        struct ipstack_rtmsg *rtmsg,
        size_t req_size)
{
  if (CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_ONLINK))
    rtmsg->rtm_flags |= IPSTACK_RTNH_F_ONLINK;
  if (nexthop->type == NEXTHOP_TYPE_IPV4
      || nexthop->type == NEXTHOP_TYPE_IPV4_IFINDEX)
    {
      librtnl_addattr_l (nlmsg, req_size, IPSTACK_RTA_GATEWAY,
                 &nexthop->gate.ipv4, bytelen);
      if (nexthop->src.ipv4.s_addr)
        librtnl_addattr_l (nlmsg, req_size, IPSTACK_RTA_PREFSRC,
                   &nexthop->src.ipv4, bytelen);

      if (IS_NSM_DEBUG_KERNEL)
        zlog_debug(MODULE_PAL, "librtnl_route_multipath() (%s): "
                   "nexthop via %s if %u",
                   routedesc,
                   inet_ntoa (nexthop->gate.ipv4),
                   nexthop->ifindex);
    }
#ifdef ZPL_BUILD_IPV6
  if (nexthop->type == NEXTHOP_TYPE_IPV6
      || nexthop->type == NEXTHOP_TYPE_IPV6_IFNAME
      || nexthop->type == NEXTHOP_TYPE_IPV6_IFINDEX)
    {
      librtnl_addattr_l (nlmsg, req_size, IPSTACK_RTA_GATEWAY,
                 &nexthop->gate.ipv6, bytelen);

      if (IS_NSM_DEBUG_KERNEL)
        zlog_debug(MODULE_PAL, "librtnl_route_multipath() (%s): "
                   "nexthop via %s if %u",
                   routedesc,
                   inet6_ntoa (nexthop->gate.ipv6),
                   nexthop->ifindex);
    }
#endif /* ZPL_BUILD_IPV6 */
  if (nexthop->type == NEXTHOP_TYPE_IFINDEX
      || nexthop->type == NEXTHOP_TYPE_IFNAME
      || nexthop->type == NEXTHOP_TYPE_IPV4_IFINDEX)
    {
      librtnl_addattr32 (nlmsg, req_size, IPSTACK_RTA_OIF, ifindex2ifkernel(nexthop->ifindex));

      if (nexthop->src.ipv4.s_addr)
        librtnl_addattr_l (nlmsg, req_size, IPSTACK_RTA_PREFSRC,
                   &nexthop->src.ipv4, bytelen);

      if (IS_NSM_DEBUG_KERNEL)
        zlog_debug(MODULE_PAL, "librtnl_route_multipath() (%s): "
                   "nexthop via if %u", routedesc, nexthop->ifindex);
    }

  if (nexthop->type == NEXTHOP_TYPE_IPV6_IFINDEX
      || nexthop->type == NEXTHOP_TYPE_IPV6_IFNAME)
    {
      librtnl_addattr32 (nlmsg, req_size, IPSTACK_RTA_OIF, ifindex2ifkernel(nexthop->ifindex));

      if (IS_NSM_DEBUG_KERNEL)
        zlog_debug(MODULE_PAL,"librtnl_route_multipath() (%s): "
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
static void librtnl_route_build_multipath(
        const char *routedesc,
        int bytelen,
        struct nexthop *nexthop,
        struct ipstack_rtattr *rta,
        struct ipstack_rtnexthop *rtnh,
        union g_addr **src
        )
{
  rtnh->rtnh_len = sizeof (*rtnh);
  rtnh->rtnh_flags = 0;
  rtnh->rtnh_hops = 0;
  rta->rta_len += rtnh->rtnh_len;

  if (CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_ONLINK))
    rtnh->rtnh_flags |= IPSTACK_RTNH_F_ONLINK;

  if (nexthop->type == NEXTHOP_TYPE_IPV4
      || nexthop->type == NEXTHOP_TYPE_IPV4_IFINDEX)
    {
      librtnl_rta_addattr_l (rta, NL_PKT_BUF_SIZE, IPSTACK_RTA_GATEWAY,
                     &nexthop->gate.ipv4, bytelen);
      rtnh->rtnh_len += sizeof (struct ipstack_rtattr) + bytelen;

      if (nexthop->src.ipv4.s_addr)
        *src = &nexthop->src;

      if (IS_NSM_DEBUG_KERNEL)
        zlog_debug(MODULE_PAL, "librtnl_route_multipath() (%s): "
                   "nexthop via %s if %u",
                   routedesc,
                   inet_ntoa (nexthop->gate.ipv4),
                   nexthop->ifindex);
    }
#ifdef ZPL_BUILD_IPV6
  if (nexthop->type == NEXTHOP_TYPE_IPV6
      || nexthop->type == NEXTHOP_TYPE_IPV6_IFNAME
      || nexthop->type == NEXTHOP_TYPE_IPV6_IFINDEX)
    {
      librtnl_rta_addattr_l (rta, NL_PKT_BUF_SIZE, IPSTACK_RTA_GATEWAY,
                     &nexthop->gate.ipv6, bytelen);
      rtnh->rtnh_len += sizeof (struct ipstack_rtattr) + bytelen;

      if (IS_NSM_DEBUG_KERNEL)
        zlog_debug(MODULE_PAL, "librtnl_route_multipath() (%s): "
                   "nexthop via %s if %u",
                   routedesc,
                   inet6_ntoa (nexthop->gate.ipv6),
                   nexthop->ifindex);
    }
#endif /* ZPL_BUILD_IPV6 */
  /* ifindex */
  if (nexthop->type == NEXTHOP_TYPE_IPV4_IFINDEX
      || nexthop->type == NEXTHOP_TYPE_IFINDEX
      || nexthop->type == NEXTHOP_TYPE_IFNAME)
    {
      rtnh->rtnh_ifindex = ifindex2ifkernel(nexthop->ifindex);
      if (nexthop->src.ipv4.s_addr)
        *src = &nexthop->src;
      if (IS_NSM_DEBUG_KERNEL)
        zlog_debug(MODULE_PAL, "librtnl_route_multipath() (%s): "
                   "nexthop via if %u", routedesc, nexthop->ifindex);
    }
  else if (nexthop->type == NEXTHOP_TYPE_IPV6_IFNAME
      || nexthop->type == NEXTHOP_TYPE_IPV6_IFINDEX)
    {
      rtnh->rtnh_ifindex = ifindex2ifkernel(nexthop->ifindex);

      if (IS_NSM_DEBUG_KERNEL)
        zlog_debug(MODULE_PAL, "librtnl_route_multipath() (%s): "
                   "nexthop via if %u", routedesc, nexthop->ifindex);
    }
  else
    {
      rtnh->rtnh_ifindex = 0;
    }
}

/* Log debug information for librtnl_route_multipath
 * if debug logging is enabled.
 *
 * @param cmd: Netlink command which is to be processed
 * @param p: Prefix for which the change is due
 * @param nexthop: Nexthop which is currently processed
 * @param routedesc: Semantic annotation for nexthop
 *                     (recursive, multipath, etc.)
 * @param family: Address family which the change concerns
 */
static void
librtnl_route_debug(
        int cmd,
        struct prefix *p,
        struct nexthop *nexthop,
        const char *routedesc,
        int family,
        int vrf_id)
{
  if (IS_NSM_DEBUG_KERNEL)
    {
      char buf[PREFIX_STRLEN];
      zlog_debug (MODULE_PAL, "librtnl_route_multipath() (%s): %s %s vrf %u type %s",
         routedesc,
         librtnl_msg_type_to_str(cmd),
         prefix2str (p, buf, sizeof(buf)),
         vrf_id,
         nexthop_type_to_str (nexthop->type));
    }
}

/* Routing table change via netlink interface. */
static int
librtnl_route_multipath (int cmd, struct prefix *p, struct rib *rib)
{
  int bytelen;
  struct ipstack_sockaddr_nl snl;
  struct nexthop *nexthop = NULL, *tnexthop;
  int recursing;
  int nexthop_num;
  int discard;
  int family = PREFIX_FAMILY(p);
  const char *routedesc;

  struct
  {
    struct ipstack_nlmsghdr n;
    struct ipstack_rtmsg r;
    char buf[NL_PKT_BUF_SIZE];
  } req;

  memset (&req, 0, sizeof req - NL_PKT_BUF_SIZE);

  bytelen = (family == IPSTACK_AF_INET ? 4 : 16);

  req.n.nlmsg_len = IPSTACK_NLMSG_LENGTH (sizeof (struct ipstack_rtmsg));
  req.n.nlmsg_flags = IPSTACK_NLM_F_CREATE | IPSTACK_NLM_F_REPLACE | IPSTACK_NLM_F_REQUEST;
  req.n.nlmsg_type = cmd;
  req.r.rtm_family = family;
  req.r.rtm_table = rib->table;
  req.r.rtm_dst_len = p->prefixlen;
  req.r.rtm_protocol = IPSTACK_RTPROT_ZEBRA;
  req.r.rtm_scope = IPSTACK_RT_SCOPE_LINK;

  if ((rib->flags & NSM_RIB_FLAG_BLACKHOLE) || (rib->flags & NSM_RIB_FLAG_REJECT))
    discard = 1;
  else
    discard = 0;

  if (cmd == IPSTACK_RTM_NEWROUTE)
    {
      if (discard)
        {
          if (rib->flags & NSM_RIB_FLAG_BLACKHOLE)
            req.r.rtm_type = IPSTACK_RTN_BLACKHOLE;
          else if (rib->flags & NSM_RIB_FLAG_REJECT)
            req.r.rtm_type = IPSTACK_RTN_UNREACHABLE;
          else
            assert (IPSTACK_RTN_BLACKHOLE != IPSTACK_RTN_UNREACHABLE);  /* false */
        }
      else
        req.r.rtm_type = IPSTACK_RTN_UNICAST;
    }

  librtnl_addattr_l (&req.n, sizeof req, IPSTACK_RTA_DST, &p->u.prefix, bytelen);

  /* Metric. */
  librtnl_addattr32 (&req.n, sizeof req, IPSTACK_RTA_PRIORITY, NL_DEFAULT_ROUTE_METRIC);

  if (rib->mtu || rib->nexthop_mtu)
    {
      char buf[NL_PKT_BUF_SIZE];
      struct ipstack_rtattr *rta = (void *) buf;
      u_int32_t mtu = rib->mtu;
      if (!mtu || (rib->nexthop_mtu && rib->nexthop_mtu < mtu))
        mtu = rib->nexthop_mtu;
      rta->rta_type = IPSTACK_RTA_METRICS;
      rta->rta_len = IPSTACK_RTA_LENGTH(0);
      librtnl_rta_addattr_l (rta, NL_PKT_BUF_SIZE, IPSTACK_RTAX_MTU, &mtu, sizeof mtu);
      librtnl_addattr_l (&req.n, NL_PKT_BUF_SIZE, IPSTACK_RTA_METRICS, IPSTACK_RTA_DATA (rta),
                 IPSTACK_RTA_PAYLOAD (rta));
    }

  if (discard)
    {
      if (cmd == IPSTACK_RTM_NEWROUTE)
        for (ALL_NEXTHOPS_RO(rib->nexthop, nexthop, tnexthop, recursing))
          {
            /* We shouldn't encounter recursive nexthops on discard routes,
             * but it is probably better to handle that case correctly anyway.
             */
            if (CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_RECURSIVE))
              continue;
            SET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB);
          }
      goto skip;
    }

  /* Count overall nexthops so we can decide whether to use singlepath
   * or multipath case. */
  nexthop_num = 0;
  for (ALL_NEXTHOPS_RO(rib->nexthop, nexthop, tnexthop, recursing))
    {
      if (CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_RECURSIVE))
        continue;
      if (cmd == IPSTACK_RTM_NEWROUTE && !CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_ACTIVE))
        continue;
      if (cmd == IPSTACK_RTM_DELROUTE && !CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB))
        continue;

      if (nexthop->type != NEXTHOP_TYPE_IFINDEX &&
          nexthop->type != NEXTHOP_TYPE_IFNAME)
        req.r.rtm_scope = IPSTACK_RT_SCOPE_UNIVERSE;

      nexthop_num++;
    }

  /* Singlepath case. */
  if (nexthop_num == 1 || MULTIPATH_NUM == 1)
    {
      nexthop_num = 0;
      for (ALL_NEXTHOPS_RO(rib->nexthop, nexthop, tnexthop, recursing))
        {
          if (CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_RECURSIVE))
            continue;

          if ((cmd == IPSTACK_RTM_NEWROUTE
               && CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE))
              || (cmd == IPSTACK_RTM_DELROUTE
                  && CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB)))
            {
              routedesc = recursing ? "recursive, 1 hop" : "single hop";

              librtnl_route_debug(cmd, p, nexthop, routedesc, family, rib->vrf_id);
              librtnl_route_build_singlepath(routedesc, bytelen,
                                              nexthop, &req.n, &req.r,
                                              sizeof req);

              if (cmd == IPSTACK_RTM_NEWROUTE)
                SET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB);

              nexthop_num++;
              break;
            }
        }
    }
  else
    {
      char buf[NL_PKT_BUF_SIZE];
      struct ipstack_rtattr *rta = (void *) buf;
      struct ipstack_rtnexthop *rtnh;
      union g_addr *src = NULL;

      rta->rta_type = IPSTACK_RTA_MULTIPATH;
      rta->rta_len = IPSTACK_RTA_LENGTH (0);
      rtnh = IPSTACK_RTA_DATA (rta);

      nexthop_num = 0;
      for (ALL_NEXTHOPS_RO(rib->nexthop, nexthop, tnexthop, recursing))
        {
          if (nexthop_num >= MULTIPATH_NUM)
            break;

          if (CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_RECURSIVE))
            continue;

          if ((cmd == IPSTACK_RTM_NEWROUTE
               && CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE))
              || (cmd == IPSTACK_RTM_DELROUTE
                  && CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB)))
            {
              routedesc = recursing ? "recursive, multihop" : "multihop";
              nexthop_num++;

              librtnl_route_debug(cmd, p, nexthop,
                                   routedesc, family, rib->vrf_id);
              librtnl_route_build_multipath(routedesc, bytelen,
                                             nexthop, rta, rtnh, &src);
              rtnh = IPSTACK_RTNH_NEXT (rtnh);

              if (cmd == IPSTACK_RTM_NEWROUTE)
                SET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB);
            }
        }
      if (src)
        librtnl_addattr_l (&req.n, sizeof req, IPSTACK_RTA_PREFSRC, &src->ipv4, bytelen);

      if (rta->rta_len > IPSTACK_RTA_LENGTH (0))
        librtnl_addattr_l (&req.n, NL_PKT_BUF_SIZE, IPSTACK_RTA_MULTIPATH, IPSTACK_RTA_DATA (rta),
                   IPSTACK_RTA_PAYLOAD (rta));
    }

  /* If there is no useful nexthop then return. */
  if (nexthop_num == 0)
    {
      if (IS_NSM_DEBUG_KERNEL)
        zlog_debug (MODULE_PAL, "librtnl_route_multipath(): No useful nexthop.");
      return 0;
    }

skip:

  /* Destination netlink address. */
  memset (&snl, 0, sizeof snl);
  snl.nl_family = IPSTACK_AF_NETLINK;

  /* Talk to netlink socket. */
  return librtnl_talk(&netlink_cmd, &req.n);
}
#endif

int librtnl_route_rib_add(zpl_uint8 processid, safi_t safi, struct prefix *p,
						struct rib *rib, zpl_uint8 num)
{
	return librtnl_route_multipath(IPSTACK_RTM_NEWROUTE, p, rib);
}

int librtnl_route_rib_del(zpl_uint8 processid, safi_t safi, struct prefix *p,
						struct rib *rib, zpl_uint8 num)
{
	return librtnl_route_multipath(IPSTACK_RTM_DELROUTE, p, rib);
}

int librtnl_route_rib_action (struct prefix *p, struct rib *old, struct rib *new)
{
  if (!old && new)
    return librtnl_route_multipath (RTM_NEWROUTE, p, new);
  if (old && !new)
    return librtnl_route_multipath (RTM_DELROUTE, p, old);

   /* Replace, can be done atomically if metric does not change;
    * netlink uses [prefix, tos, priority] to identify prefix.
    * Now metric is not sent to kernel, so we can just do atomic replace. */
  return librtnl_route_multipath (RTM_NEWROUTE, p, new);
}

#endif

#ifdef ZPL_LIBNL_MODULE
static int rtnl_route_opt(int op, int family, char *dst, char *nexthop, char *outdev, int table, char *src, char *iif,
						  char *pref_src, int mtype, int metrics, int pri, int scope, int proto, int type)
{
	struct rtnl_nexthop *nh;
	struct nl_cache *link_cache;
	struct rtnl_route *rtnlroute;
	struct nl_addr *dstaddr, *srcaddr, *prefsrc, *nextaddr;
	int err, ival = 0, nlflags = NLM_F_EXCL;

	if ((err = rtnl_link_alloc_cache(netlink_cmd.libnl_sock, AF_UNSPEC, &link_cache)) < 0)
	{
		nl_perror(err, "Unable to allocate cache");
		return err;
	}
	rtnlroute = nl_cli_route_alloc();
	nh = rtnl_route_nh_alloc();

	nl_addr_parse(dst, rtnl_route_get_family(rtnlroute), &dstaddr);
	if ((err = rtnl_route_set_dst(rtnlroute, dstaddr)) < 0)
		nl_perror(err, "Unable to set local address");
	nl_addr_put(dstaddr);

	if (src)
	{
		nl_addr_parse(src, rtnl_route_get_family(rtnlroute), &srcaddr);
		if ((err = rtnl_route_set_src(rtnlroute, srcaddr)) < 0)
			nl_perror(err, "Unable to set src address");
		nl_addr_put(srcaddr);
	}

	if (pref_src)
	{
		nl_addr_parse(pref_src, rtnl_route_get_family(rtnlroute), &prefsrc);
		if ((err = rtnl_route_set_src(rtnlroute, prefsrc)) < 0)
			nl_perror(err, "Unable to set src address");
		nl_addr_put(prefsrc);
	}

	// nexthop
	if (outdev)
	{
		ival = rtnl_link_name2i(link_cache, outdev);
		rtnl_route_nh_set_ifindex(nh, ival);
	}
	if (nexthop)
	{
		if (rtnl_route_get_family(rtnlroute) == AF_MPLS)
		{
			nl_addr_parse(nexthop, 0, &nextaddr);

			rtnl_route_nh_set_via(nh, nextaddr);
		}
		else
		{
			nl_addr_parse(nexthop, rtnl_route_get_family(rtnlroute), &nextaddr);
			rtnl_route_nh_set_gateway(nh, nextaddr);
		}
		nl_addr_put(nextaddr);
	}
	rtnl_route_add_nexthop(rtnlroute, nh);

	rtnl_route_set_scope(rtnlroute, scope);
	rtnl_route_set_family(rtnlroute, family);

	rtnl_route_set_protocol(rtnlroute, proto);

	rtnl_route_set_priority(rtnlroute, pri);
	rtnl_route_set_table(rtnlroute, table);

	rtnl_route_set_type(rtnlroute, type);
	if (iif)
	{
		ival = rtnl_link_name2i(link_cache, iif);
		rtnl_route_set_iif(rtnlroute, ival);
	}
	rtnl_route_set_metric(rtnlroute, mtype, metrics);

	if (op)
	{
		if ((err = rtnl_route_add(netlink_cmd.libnl_sock, rtnlroute, nlflags)) < 0)
			nl_perror(err, "Unable to add neighbour");
	}
	else
	{
		if ((err = rtnl_route_delete(netlink_cmd.libnl_sock, rtnlroute, 0)) < 0)
			nl_perror(err, "Unable to delete neighbour");
	}
	return err;
}
#endif
