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

#ifdef ZPL_KERNEL_SORF_FORWARDING

/* Called from interface_lookup_netlink().  This function is only used
 during bootstrap. */
static int _netlink_interface_load(struct ipstack_sockaddr_nl *snl, struct ipstack_nlmsghdr *h,
		vrf_id_t vrf_id)
{
	zpl_uint32 len;
	struct ipstack_ifinfomsg *ifi;
	struct ipstack_rtattr *tb[IPSTACK_IFLA_MAX + 1];
	struct interface *ifp;
	char *name;

	ifi = IPSTACK_NLMSG_DATA(h);

	if (h->nlmsg_type != IPSTACK_RTM_NEWLINK)
		return 0;

	len = h->nlmsg_len - IPSTACK_NLMSG_LENGTH(sizeof(struct ipstack_ifinfomsg));
	if (len < 0)
		return -1;

	/* Looking up interface name. */
	memset(tb, 0, sizeof tb);
	_netlink_parse_rtattr(tb, IPSTACK_IFLA_MAX, IPSTACK_IFLA_RTA(ifi), len);

#ifdef IPSTACK_IFLA_WIRELESS
	/* check for wireless messages to ignore */
	if ((tb[IPSTACK_IFLA_WIRELESS] != NULL) && (ifi->ifi_change == 0))
	{
		if (IS_ZEBRA_DEBUG_KERNEL)
		zlog_debug (MODULE_PAL, "%s: ignoring IPSTACK_IFLA_WIRELESS message", __func__);
		return 0;
	}
#endif /* IPSTACK_IFLA_WIRELESS */

	if (tb[IPSTACK_IFLA_IFNAME] == NULL)
		return -1;
	name = (char *) IPSTACK_RTA_DATA(tb[IPSTACK_IFLA_IFNAME]);

	/* Add interface. */
/*	if (!ifindex_lookup_by_kname(name))
		return 0;*/

	if (!if_lookup_by_kernel_name_vrf(name, vrf_id) &&
			!if_lookup_by_kernel_index_vrf(ifi->ifi_index, vrf_id))
	{
		if (h->nlmsg_type == IPSTACK_RTM_NEWLINK)
		{
			/* Add interface. */
			return 0;
		}
		return 0;
	}
	ifp = if_lookup_by_kernel_name_vrf(name, vrf_id);
	if (!ifp)
	{
		ifp = if_lookup_by_kernel_index_vrf(ifi->ifi_index, vrf_id);
		//if (!ifp)
		//	ifp = if_create_vrf(name, strlen(name), vrf_id);
	}
	if (ifp)
	{
		if_kname_set(ifp, name);
		_netlink_set_ifindex(ifp, ifi->ifi_index);
		ifp->flags = ifi->ifi_flags & 0x0000fffff;
		ifp->mtu6 = ifp->mtu = *(zpl_uint32  *) IPSTACK_RTA_DATA(tb[IPSTACK_IFLA_MTU]);
		//ifp->metric = 0;

		/* Hardware type and address. */
		ifp->ll_type = _netlink_to_zebra_link_type(ifi->ifi_type);
		_netlink_interface_update_hw_addr(tb, ifp);

		//zhurish if_add_update(ifp);
	}
	return 0;
}

/* Lookup interface IPv4/IPv6 address. */
static int _netlink_interface_address_load(struct ipstack_sockaddr_nl *snl, struct ipstack_nlmsghdr *h,
		vrf_id_t vrf_id)
{
	zpl_uint32 len;
	struct ipstack_ifaddrmsg *ifa;
	struct ipstack_rtattr *tb[IPSTACK_IFA_MAX + 1];
	struct interface *ifp;
	void *addr = NULL;
	void *broad = NULL;
	zpl_uchar flags = 0;
	char *label = NULL;

	ifa = IPSTACK_NLMSG_DATA(h);

	if (ifa->ifa_family != IPSTACK_AF_INET
#ifdef HAVE_IPV6
			&& ifa->ifa_family != IPSTACK_AF_INET6
#endif /* HAVE_IPV6 */
					)
		return 0;

	if (h->nlmsg_type != IPSTACK_RTM_NEWADDR && h->nlmsg_type != IPSTACK_RTM_DELADDR)
		return 0;

	len = h->nlmsg_len - IPSTACK_NLMSG_LENGTH(sizeof(struct ipstack_ifaddrmsg));
	if (len < 0)
		return -1;
	if (h->nlmsg_pid == snl->nl_pid)
	{
		zlog_err(MODULE_PAL,
				"netlink_interface_address_load Ignoring message from pid (self):%u",
				h->nlmsg_pid);
		return 0;
	}
	memset(tb, 0, sizeof tb);
	_netlink_parse_rtattr(tb, IPSTACK_IFA_MAX, IPSTACK_IFA_RTA(ifa), len);

	ifp = if_lookup_by_kernel_index_vrf(ifa->ifa_index, vrf_id);
	if (ifp == NULL)
	{
		char kbuf[64];
		memset(kbuf, 0, sizeof(kbuf));
		if_indextoname(ifa->ifa_index, kbuf);

		zlog_err(MODULE_PAL,
				"netlink_interface_addr can't find interface by index %d(%s) vrf %u",
				ifa->ifa_index, kbuf, vrf_id);
		return -1;
	}

	if (IS_ZEBRA_DEBUG_KERNEL) /* remove this line to see initial ifcfg */
	{
		char buf[BUFSIZ];
		zlog_debug(MODULE_PAL, "netlink_interface_addr %s %s vrf %u:",
				_netlink_msg_type_to_str(h->nlmsg_type), ifp->name, vrf_id);
		if (tb[IPSTACK_IFA_LOCAL])
			zlog_debug(MODULE_PAL, "  IPSTACK_IFA_LOCAL     %s/%d",
					inet_ntop(ifa->ifa_family, IPSTACK_RTA_DATA(tb[IPSTACK_IFA_LOCAL]), buf,
							BUFSIZ), ifa->ifa_prefixlen);
		if (tb[IPSTACK_IFA_ADDRESS])
			zlog_debug(MODULE_PAL, "  IPSTACK_IFA_ADDRESS   %s/%d",
					inet_ntop(ifa->ifa_family, IPSTACK_RTA_DATA(tb[IPSTACK_IFA_ADDRESS]), buf,
							BUFSIZ), ifa->ifa_prefixlen);
		if (tb[IPSTACK_IFA_BROADCAST])
			zlog_debug(MODULE_PAL, "  IPSTACK_IFA_BROADCAST %s/%d",
					inet_ntop(ifa->ifa_family, IPSTACK_RTA_DATA(tb[IPSTACK_IFA_BROADCAST]), buf,
							BUFSIZ), ifa->ifa_prefixlen);
		if (tb[IPSTACK_IFA_LABEL] && strcmp(ifp->k_name, IPSTACK_RTA_DATA(tb[IPSTACK_IFA_LABEL])))
			zlog_debug(MODULE_PAL, "  IPSTACK_IFA_LABEL     %s",
					(char *) IPSTACK_RTA_DATA(tb[IPSTACK_IFA_LABEL]));

		if (tb[IPSTACK_IFA_CACHEINFO])
		{
			struct ipstack_ifa_cacheinfo *ci = IPSTACK_RTA_DATA(tb[IPSTACK_IFA_CACHEINFO]);
			zlog_debug(MODULE_PAL, "  IPSTACK_IFA_CACHEINFO pref %d, valid %d",
					ci->ifa_prefered, ci->ifa_valid);
		}
	}

	/* logic copied from iproute2/ip/ipaddress.c:print_addrinfo() */
	if (tb[IPSTACK_IFA_LOCAL] == NULL)
		tb[IPSTACK_IFA_LOCAL] = tb[IPSTACK_IFA_ADDRESS];
	if (tb[IPSTACK_IFA_ADDRESS] == NULL)
		tb[IPSTACK_IFA_ADDRESS] = tb[IPSTACK_IFA_LOCAL];

	/* local interface address */
	addr = (tb[IPSTACK_IFA_LOCAL] ? IPSTACK_RTA_DATA(tb[IPSTACK_IFA_LOCAL]) : NULL);

	/* is there a peer address? */
	if (tb[IPSTACK_IFA_ADDRESS]
			&& memcmp(IPSTACK_RTA_DATA(tb[IPSTACK_IFA_ADDRESS]), IPSTACK_RTA_DATA(tb[IPSTACK_IFA_LOCAL]),
					IPSTACK_RTA_PAYLOAD(tb[IPSTACK_IFA_ADDRESS])))
	{
		broad = IPSTACK_RTA_DATA(tb[IPSTACK_IFA_ADDRESS]);
		SET_FLAG(flags, ZEBRA_IFA_PEER);
	}
	else
		/* seeking a broadcast address */
		broad = (tb[IPSTACK_IFA_BROADCAST] ? IPSTACK_RTA_DATA(tb[IPSTACK_IFA_BROADCAST]) : NULL);

	/* addr is primary key, SOL if we don't have one */
	if (addr == NULL)
	{
		zlog_debug(MODULE_PAL, "%s: NULL address", __func__);
		return -1;
	}

	/* Flags. */
	if (ifa->ifa_flags & IPSTACK_IFA_F_SECONDARY)
		SET_FLAG(flags, ZEBRA_IFA_SECONDARY);

	/* Label */
	if (tb[IPSTACK_IFA_LABEL])
		label = (char *) IPSTACK_RTA_DATA(tb[IPSTACK_IFA_LABEL]);

	if (ifp && label && strcmp(ifp->k_name, label) == 0)
		label = NULL;

	/* Register interface address to the interface. */
	if (ifa->ifa_family == IPSTACK_AF_INET)
	{
		if (h->nlmsg_type == IPSTACK_RTM_NEWADDR)
			connected_add_ipv4(ifp, flags, (struct ipstack_in_addr *) addr,
					ifa->ifa_prefixlen, (struct ipstack_in_addr *) broad, label);
		else
		{
			connected_delete_ipv4(ifp, flags, (struct ipstack_in_addr *) addr,
					ifa->ifa_prefixlen, (struct ipstack_in_addr *) broad);
		}
	}
#ifdef HAVE_IPV6
	if (ifa->ifa_family == IPSTACK_AF_INET6)
	{
		if (h->nlmsg_type == IPSTACK_RTM_NEWADDR)
			connected_add_ipv6(ifp, flags, (struct ipstack_in6_addr *) addr,
					ifa->ifa_prefixlen, (struct ipstack_in6_addr *) broad, label);
		else
			connected_delete_ipv6(ifp, (struct ipstack_in6_addr *) addr,
					ifa->ifa_prefixlen, (struct ipstack_in6_addr *) broad);
	}
#endif /* HAVE_IPV6 */

	return 0;
}

/* Looking up routing table by netlink interface. */
static int _netlink_routing_table_load(struct ipstack_sockaddr_nl *snl, struct ipstack_nlmsghdr *h,
		vrf_id_t vrf_id)
{
	zpl_uint32 len;
	struct ipstack_rtmsg *rtm;
	struct ipstack_rtattr *tb[IPSTACK_RTA_MAX + 1];
	zpl_uchar flags = 0;

	char anyaddr[16] =
	{ 0 };

	zpl_uint32 index;
	zpl_uint32 table;
	zpl_uint32 mtu = 0;

	void *dest;
	void *gate;
	void *src;

	rtm = IPSTACK_NLMSG_DATA(h);

	if (h->nlmsg_type != IPSTACK_RTM_NEWROUTE)
		return 0;
	if (rtm->rtm_type != IPSTACK_RTN_UNICAST)
		return 0;

	table = rtm->rtm_table;
#if 0                           /* we weed them out later in rib_weed_tables () */
	if (table != IPSTACK_RT_TABLE_MAIN && table != zebrad.rtm_table_default)
	return 0;
#endif

	len = h->nlmsg_len - IPSTACK_NLMSG_LENGTH(sizeof(struct ipstack_rtmsg));
	if (len < 0)
		return -1;

	memset(tb, 0, sizeof tb);
	_netlink_parse_rtattr(tb, IPSTACK_RTA_MAX, IPSTACK_RTM_RTA(rtm), len);

	if (rtm->rtm_flags & IPSTACK_RTM_F_CLONED)
		return 0;
	if (rtm->rtm_protocol == IPSTACK_RTPROT_REDIRECT)
		return 0;
	if (rtm->rtm_protocol == IPSTACK_RTPROT_KERNEL)
		return 0;

	if (rtm->rtm_src_len != 0)
		return 0;

	/* Route which inserted by Zebra. */
	if (rtm->rtm_protocol == IPSTACK_RTPROT_ZEBRA)
		flags |= ZEBRA_FLAG_SELFROUTE;

	index = 0;
	dest = NULL;
	gate = NULL;
	src = NULL;

	if (tb[IPSTACK_RTA_OIF])
		index = *(int *) IPSTACK_RTA_DATA(tb[IPSTACK_RTA_OIF]);

	if (tb[IPSTACK_RTA_DST])
		dest = IPSTACK_RTA_DATA(tb[IPSTACK_RTA_DST]);
	else
		dest = anyaddr;

	if (tb[IPSTACK_RTA_PREFSRC])
		src = IPSTACK_RTA_DATA(tb[IPSTACK_RTA_PREFSRC]);

	if (tb[IPSTACK_RTA_GATEWAY])
		gate = IPSTACK_RTA_DATA(tb[IPSTACK_RTA_GATEWAY]);

	if (tb[IPSTACK_RTA_METRICS])
	{
		struct ipstack_rtattr *mxrta[IPSTACK_RTAX_MAX + 1];

		memset(mxrta, 0, sizeof mxrta);
		_netlink_parse_rtattr(mxrta, IPSTACK_RTAX_MAX, IPSTACK_RTA_DATA(tb[IPSTACK_RTA_METRICS]),
				IPSTACK_RTA_PAYLOAD(tb[IPSTACK_RTA_METRICS]));

		if (mxrta[IPSTACK_RTAX_MTU])
			mtu = *(zpl_uint32 *) IPSTACK_RTA_DATA(mxrta[IPSTACK_RTAX_MTU]);
	}

	if (rtm->rtm_family == IPSTACK_AF_INET)
	{
		struct prefix_ipv4 p;
		p.family = IPSTACK_AF_INET;
		memcpy(&p.prefix, dest, 4);
		p.prefixlen = rtm->rtm_dst_len;

		if (!tb[IPSTACK_RTA_MULTIPATH])
			rib_add_ipv4(ZEBRA_ROUTE_KERNEL, flags, &p, gate, src,
					ifkernel2ifindex(index), vrf_id, table, 0, mtu, 0,
					SAFI_UNICAST);
		else
		{
			/* This is a multipath route */

			struct rib *rib;
			struct ipstack_rtnexthop *rtnh = (struct ipstack_rtnexthop *) IPSTACK_RTA_DATA(
					tb[IPSTACK_RTA_MULTIPATH]);

			len = IPSTACK_RTA_PAYLOAD(tb[IPSTACK_RTA_MULTIPATH]);

			rib = XCALLOC(MTYPE_RIB, sizeof(struct rib));
			rib->type = ZEBRA_ROUTE_KERNEL;
			rib->distance = 0;
			rib->flags = flags;
			rib->metric = 0;
			rib->mtu = mtu;
			rib->vrf_id = vrf_id;
			rib->table = table;
			rib->nexthop_num = 0;
			rib->uptime = time(NULL);

			for (;;)
			{
				if (len < (int) sizeof(*rtnh) || rtnh->rtnh_len > len)
					break;

				index = rtnh->rtnh_ifindex;
				gate = 0;
				if (rtnh->rtnh_len > sizeof(*rtnh))
				{
					memset(tb, 0, sizeof(tb));
					_netlink_parse_rtattr(tb, IPSTACK_RTA_MAX, IPSTACK_RTNH_DATA(rtnh),
							rtnh->rtnh_len - sizeof(*rtnh));
					if (tb[IPSTACK_RTA_GATEWAY])
						gate = IPSTACK_RTA_DATA(tb[IPSTACK_RTA_GATEWAY]);
				}

				if (gate)
				{
					if (index)
						rib_nexthop_ipv4_ifindex_add(rib, gate, src,
								ifkernel2ifindex(index));
					else
						rib_nexthop_ipv4_add(rib, gate, src);
				}
				else
					rib_nexthop_ifindex_add(rib, ifkernel2ifindex(index));

				len -= IPSTACK_NLMSG_ALIGN(rtnh->rtnh_len);
				rtnh = IPSTACK_RTNH_NEXT(rtnh);
			}

			if (rib->nexthop_num == 0)
				XFREE(MTYPE_RIB, rib);
			else
				rib_add_ipv4_multipath(&p, rib, SAFI_UNICAST);
		}
	}
#ifdef HAVE_IPV6
	if (rtm->rtm_family == IPSTACK_AF_INET6)
	{
		struct prefix_ipv6 p;
		p.family = IPSTACK_AF_INET6;
		memcpy(&p.prefix, dest, 16);
		p.prefixlen = rtm->rtm_dst_len;

		rib_add_ipv6(ZEBRA_ROUTE_KERNEL, flags, &p, gate,
				ifkernel2ifindex(index), vrf_id, table, 0, mtu, 0,
				SAFI_UNICAST);
	}
#endif /* HAVE_IPV6 */

	return 0;
}

/* Interface lookup by netlink ipstack_socket. */
int _netlink_iftbl_load(struct nsm_vrf *zvrf)
{
	int ret;

	/* Get interface information. */
	ret = _netlink_request(IPSTACK_AF_PACKET, IPSTACK_RTM_GETLINK, &zvrf->netlink_cmd);
	if (ret < 0)
		return ret;
	ret = _netlink_parse_info(_netlink_interface_load, &zvrf->netlink_cmd, zvrf);
	if (ret < 0)
		return ret;

	/* Get IPv4 address of the interfaces. */
	ret = _netlink_request(IPSTACK_AF_INET, IPSTACK_RTM_GETADDR, &zvrf->netlink_cmd);
	if (ret < 0)
		return ret;
	ret = _netlink_parse_info(_netlink_interface_address_load, &zvrf->netlink_cmd, zvrf);
	if (ret < 0)
		return ret;

#ifdef HAVE_IPV6
	/* Get IPv6 address of the interfaces. */
	ret = _netlink_request(IPSTACK_AF_INET6, IPSTACK_RTM_GETADDR, &zvrf->netlink_cmd);
	if (ret < 0)
		return ret;
	ret = _netlink_parse_info(_netlink_interface_address_load, &zvrf->netlink_cmd, zvrf);
	if (ret < 0)
		return ret;
#endif /* HAVE_IPV6 */

	return 0;
}

/* Routing table read function using netlink interface.  Only called
 bootstrap time. */
int _netlink_rib_load(struct nsm_vrf *zvrf)
{
	int ret;

	/* Get IPv4 routing table. */
	ret = _netlink_request(IPSTACK_AF_INET, IPSTACK_RTM_GETROUTE, &zvrf->netlink_cmd);
	if (ret < 0)
		return ret;
	ret = _netlink_parse_info(_netlink_routing_table_load, &zvrf->netlink_cmd, zvrf);
	if (ret < 0)
		return ret;

#ifdef HAVE_IPV6
	/* Get IPv6 routing table. */
	ret = _netlink_request(IPSTACK_AF_INET6, IPSTACK_RTM_GETROUTE, &zvrf->netlink_cmd);
	if (ret < 0)
		return ret;
	ret = _netlink_parse_info(_netlink_routing_table_load, &zvrf->netlink_cmd, zvrf);
	if (ret < 0)
		return ret;
#endif /* HAVE_IPV6 */

	return 0;
}

#endif