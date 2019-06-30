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

#include <zebra.h>

#include "linklist.h"
#include "if.h"
#include "connected.h"
#include "log.h"
#include "prefix.h"
#include "table.h"
#include "memory.h"
#include "rib.h"
#include "thread.h"
#include "vrf.h"
#include "nexthop.h"

#include "zserv.h"

#include "redistribute.h"
#include "interface.h"
#include "debug.h"

#include "kernel_netlink.h"

extern struct zebra_t zebrad;

/* Lookup interface IPv4/IPv6 address. */
static int netlink_interface_addr(struct sockaddr_nl *snl, struct nlmsghdr *h,
		vrf_id_t vrf_id)
{
	int len;
	struct ifaddrmsg *ifa;
	struct rtattr *tb[IFA_MAX + 1];
	struct interface *ifp;
	void *addr = NULL;
	void *broad = NULL;
	u_char flags = 0;
	char *label = NULL;
	char ifkname[64];
	ifa = NLMSG_DATA(h);

	if (ifa->ifa_family != AF_INET
#ifdef HAVE_IPV6
			&& ifa->ifa_family != AF_INET6
#endif /* HAVE_IPV6 */
					)
		return 0;

	if (h->nlmsg_type != RTM_NEWADDR && h->nlmsg_type != RTM_DELADDR)
		return 0;

	len = h->nlmsg_len - NLMSG_LENGTH(sizeof(struct ifaddrmsg));
	if (len < 0)
		return -1;
/*	if (h->nlmsg_pid == snl->nl_pid)
	{
		zlog_err(ZLOG_PAL,
				"netlink_interface_addr Ignoring message from pid (self):%u",
				h->nlmsg_pid);
		return 0;
	}*/
	memset(tb, 0, sizeof tb);
	netlink_parse_rtattr(tb, IFA_MAX, IFA_RTA(ifa), len);

	ifp = if_lookup_by_kernel_index_vrf(ifa->ifa_index, vrf_id);
	if (ifp == NULL)
	{
		memset(ifkname, 0, sizeof(ifkname));
		if (if_indextoname(ifa->ifa_index, ifkname) == NULL)
		{
			zlog_err(ZLOG_PAL,
					"netlink_interface_addr can't find interface by index %d vrf %u",
					ifa->ifa_index, vrf_id);
			return -1;
		}
		zlog_debug(ZLOG_PAL, "find interface %s index %d ", ifa->ifa_index, ifkname);

		ifp = if_lookup_by_kernel_name_vrf(ifkname, vrf_id);
		if (ifp == NULL)
		{
			zlog_err(ZLOG_PAL,
					"netlink_interface_addr can't find interface by index %d(%s) vrf %u",
					ifa->ifa_index, ifkname, vrf_id);
			return -1;
		}
	}

	if (IS_ZEBRA_DEBUG_KERNEL) /* remove this line to see initial ifcfg */
	{
		char buf[BUFSIZ];
		zlog_debug(ZLOG_PAL, "netlink_interface_addr %s %s vrf %u:",
				nl_msg_type_to_str(h->nlmsg_type), ifp->name, vrf_id);
		if (tb[IFA_LOCAL])
			zlog_debug(ZLOG_PAL, "  IFA_LOCAL     %s/%d",
					inet_ntop(ifa->ifa_family, RTA_DATA(tb[IFA_LOCAL]), buf,
							BUFSIZ), ifa->ifa_prefixlen);
		if (tb[IFA_ADDRESS])
			zlog_debug(ZLOG_PAL, "  IFA_ADDRESS   %s/%d",
					inet_ntop(ifa->ifa_family, RTA_DATA(tb[IFA_ADDRESS]), buf,
							BUFSIZ), ifa->ifa_prefixlen);
		if (tb[IFA_BROADCAST])
			zlog_debug(ZLOG_PAL, "  IFA_BROADCAST %s/%d",
					inet_ntop(ifa->ifa_family, RTA_DATA(tb[IFA_BROADCAST]), buf,
							BUFSIZ), ifa->ifa_prefixlen);
		if (tb[IFA_LABEL] && strcmp(ifp->k_name, RTA_DATA(tb[IFA_LABEL])))
			zlog_debug(ZLOG_PAL, "  IFA_LABEL     %s",
					(char *) RTA_DATA(tb[IFA_LABEL]));

		if (tb[IFA_CACHEINFO])
		{
			struct ifa_cacheinfo *ci = RTA_DATA(tb[IFA_CACHEINFO]);
			zlog_debug(ZLOG_PAL, "  IFA_CACHEINFO pref %d, valid %d",
					ci->ifa_prefered, ci->ifa_valid);
		}
	}

	/* logic copied from iproute2/ip/ipaddress.c:print_addrinfo() */
	if (tb[IFA_LOCAL] == NULL)
		tb[IFA_LOCAL] = tb[IFA_ADDRESS];
	if (tb[IFA_ADDRESS] == NULL)
		tb[IFA_ADDRESS] = tb[IFA_LOCAL];

	/* local interface address */
	addr = (tb[IFA_LOCAL] ? RTA_DATA(tb[IFA_LOCAL]) : NULL);

	/* is there a peer address? */
	if (tb[IFA_ADDRESS]
			&& memcmp(RTA_DATA(tb[IFA_ADDRESS]), RTA_DATA(tb[IFA_LOCAL]),
					RTA_PAYLOAD(tb[IFA_ADDRESS])))
	{
		broad = RTA_DATA(tb[IFA_ADDRESS]);
		SET_FLAG(flags, ZEBRA_IFA_PEER);
	}
	else
		/* seeking a broadcast address */
		broad = (tb[IFA_BROADCAST] ? RTA_DATA(tb[IFA_BROADCAST]) : NULL);

	/* addr is primary key, SOL if we don't have one */
	if (addr == NULL)
	{
		zlog_debug(ZLOG_PAL, "%s: NULL address", __func__);
		return -1;
	}

	/* Flags. */
	if (ifa->ifa_flags & IFA_F_SECONDARY)
		SET_FLAG(flags, ZEBRA_IFA_SECONDARY);

	/* Label */
	if (tb[IFA_LABEL])
		label = (char *) RTA_DATA(tb[IFA_LABEL]);

	if (ifp && label && strcmp(ifp->k_name, label) == 0)
		label = NULL;

	/* Register interface address to the interface. */
	if (ifa->ifa_family == AF_INET)
	{
		if (h->nlmsg_type == RTM_NEWADDR)
			connected_add_ipv4(ifp, flags, (struct in_addr *) addr,
					ifa->ifa_prefixlen, (struct in_addr *) broad, label);
		else
		{
			connected_delete_ipv4(ifp, flags, (struct in_addr *) addr,
					ifa->ifa_prefixlen, (struct in_addr *) broad);
		}
	}
#ifdef HAVE_IPV6
	if (ifa->ifa_family == AF_INET6)
	{
		if (h->nlmsg_type == RTM_NEWADDR)
			connected_add_ipv6(ifp, flags, (struct in6_addr *) addr,
					ifa->ifa_prefixlen, (struct in6_addr *) broad, label);
		else
			connected_delete_ipv6(ifp, (struct in6_addr *) addr,
					ifa->ifa_prefixlen, (struct in6_addr *) broad);
	}
#endif /* HAVE_IPV6 */

	return 0;
}

/* Routing information change from the kernel. */
static int netlink_route_change(struct sockaddr_nl *snl, struct nlmsghdr *h,
		vrf_id_t vrf_id)
{
	int len;
	struct rtmsg *rtm;
	struct rtattr *tb[RTA_MAX + 1];
	u_char zebra_flags = 0;

	char anyaddr[16] =
	{ 0 };

	ifindex_t ifkindex;
	int table;
	u_int32_t mtu = 0;

	void *dest;
	void *gate;
	void *src;

	rtm = NLMSG_DATA(h);

	if (!(h->nlmsg_type == RTM_NEWROUTE || h->nlmsg_type == RTM_DELROUTE))
	{
		/* If this is not route add/delete message print warning. */
		zlog_warn(ZLOG_PAL, "Kernel message: %d vrf %u\n", h->nlmsg_type,
				vrf_id);
		return 0;
	}

	/* Connected route. */
	if (IS_ZEBRA_DEBUG_KERNEL)
		zlog_debug(ZLOG_PAL, "%s %s %s proto %s vrf %u",
				h->nlmsg_type == RTM_NEWROUTE ? "RTM_NEWROUTE" : "RTM_DELROUTE",
				rtm->rtm_family == AF_INET ? "ipv4" : "ipv6",
				rtm->rtm_type == RTN_UNICAST ? "unicast" : "multicast",
				nl_rtproto_to_str(rtm->rtm_protocol), vrf_id);

	if (rtm->rtm_type != RTN_UNICAST)
	{
		return 0;
	}

	table = rtm->rtm_table;
	if (table != RT_TABLE_MAIN && table != zebrad.rtm_table_default)
	{
		return 0;
	}

	len = h->nlmsg_len - NLMSG_LENGTH(sizeof(struct rtmsg));
	if (len < 0)
		return -1;

	memset(tb, 0, sizeof tb);
	netlink_parse_rtattr(tb, RTA_MAX, RTM_RTA(rtm), len);

	if (rtm->rtm_flags & RTM_F_CLONED)
		return 0;
	if (rtm->rtm_protocol == RTPROT_REDIRECT)
		return 0;
	if (rtm->rtm_protocol == RTPROT_KERNEL)
		return 0;

	if (rtm->rtm_protocol == RTPROT_ZEBRA && h->nlmsg_type == RTM_NEWROUTE)
		return 0;
	if (rtm->rtm_protocol == RTPROT_ZEBRA)
		SET_FLAG(zebra_flags, ZEBRA_FLAG_SELFROUTE);

	if (rtm->rtm_src_len != 0)
	{
		zlog_warn(ZLOG_PAL, "netlink_route_change(): no src len, vrf %u",
				vrf_id);
		return 0;
	}

	ifkindex = 0;
	dest = NULL;
	gate = NULL;
	src = NULL;

	if (tb[RTA_OIF])
	{
		ifkindex = *(ifindex_t *) RTA_DATA(tb[RTA_OIF]);
/*		zlog_debug(ZLOG_PAL, "---------------- %s(%d) vrf %u",
						ifkernelindex2kernelifname(ifkindex), ifkindex, vrf_id);*/
	}
	if (tb[RTA_DST])
		dest = RTA_DATA(tb[RTA_DST]);
	else
		dest = anyaddr;

	if (tb[RTA_GATEWAY])
		gate = RTA_DATA(tb[RTA_GATEWAY]);

	if (tb[RTA_PREFSRC])
		src = RTA_DATA(tb[RTA_PREFSRC]);

	if (h->nlmsg_type == RTM_NEWROUTE)
	{
		if (tb[RTA_METRICS])
		{
			struct rtattr *mxrta[RTAX_MAX + 1];

			memset(mxrta, 0, sizeof mxrta);
			netlink_parse_rtattr(mxrta, RTAX_MAX, RTA_DATA(tb[RTA_METRICS]),
					RTA_PAYLOAD(tb[RTA_METRICS]));

			if (mxrta[RTAX_MTU])
				mtu = *(u_int32_t *) RTA_DATA(mxrta[RTAX_MTU]);
		}
	}

	if (rtm->rtm_family == AF_INET)
	{
		struct prefix_ipv4 p;
		p.family = AF_INET;
		memcpy(&p.prefix, dest, 4);
		p.prefixlen = rtm->rtm_dst_len;

		if (IS_ZEBRA_DEBUG_KERNEL)
		{
			char buf[PREFIX_STRLEN];
			zlog_debug(ZLOG_PAL, "%s %s %s(%d) vrf %u",
					h->nlmsg_type == RTM_NEWROUTE ?
							"RTM_NEWROUTE" : "RTM_DELROUTE",
					prefix2str(&p, buf, sizeof(buf)), ifkernelindex2kernelifname(ifkindex), ifkindex, vrf_id);
		}

		if (h->nlmsg_type == RTM_NEWROUTE)
		{
			if (!tb[RTA_MULTIPATH])
				rib_add_ipv4(ZEBRA_ROUTE_KERNEL, 0, &p, gate, src,
						ifkernel2ifindex(ifkindex), vrf_id, table, 0, mtu, 0,
						SAFI_UNICAST);
			else
			{
				/* This is a multipath route */

				struct rib *rib;
				struct rtnexthop *rtnh = (struct rtnexthop *) RTA_DATA(
						tb[RTA_MULTIPATH]);

				len = RTA_PAYLOAD(tb[RTA_MULTIPATH]);

				rib = XCALLOC(MTYPE_RIB, sizeof(struct rib));
				rib->type = ZEBRA_ROUTE_KERNEL;
				rib->distance = 0;
				rib->flags = 0;
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

					ifkindex = rtnh->rtnh_ifindex;
					gate = 0;
					if (rtnh->rtnh_len > sizeof(*rtnh))
					{
						memset(tb, 0, sizeof(tb));
						netlink_parse_rtattr(tb, RTA_MAX, RTNH_DATA(rtnh),
								rtnh->rtnh_len - sizeof(*rtnh));
						if (tb[RTA_GATEWAY])
							gate = RTA_DATA(tb[RTA_GATEWAY]);
					}

					if (gate)
					{
						if (ifkindex)
							rib_nexthop_ipv4_ifindex_add(rib, gate, src,
									ifkernel2ifindex(ifkindex));
						else
							rib_nexthop_ipv4_add(rib, gate, src);
					}
					else
						rib_nexthop_ifindex_add(rib, ifkernel2ifindex(ifkindex));

					len -= NLMSG_ALIGN(rtnh->rtnh_len);
					rtnh = RTNH_NEXT(rtnh);
				}

				if (rib->nexthop_num == 0)
					XFREE(MTYPE_RIB, rib);
				else
					rib_add_ipv4_multipath(&p, rib, SAFI_UNICAST);
			}
		}
		else
			rib_delete_ipv4(ZEBRA_ROUTE_KERNEL, zebra_flags, &p, gate,
					ifkernel2ifindex(ifkindex), vrf_id, SAFI_UNICAST);
	}

#ifdef HAVE_IPV6
	if (rtm->rtm_family == AF_INET6)
	{
		struct prefix_ipv6 p;

		p.family = AF_INET6;
		memcpy(&p.prefix, dest, 16);
		p.prefixlen = rtm->rtm_dst_len;

		if (IS_ZEBRA_DEBUG_KERNEL)
		{
			char buf[PREFIX_STRLEN];
			zlog_debug(ZLOG_PAL, "%s %s vrf %u",
					h->nlmsg_type == RTM_NEWROUTE ?
							"RTM_NEWROUTE" : "RTM_DELROUTE",
					prefix2str(&p, buf, sizeof(buf)), vrf_id);
		}

		if (h->nlmsg_type == RTM_NEWROUTE)
			rib_add_ipv6(ZEBRA_ROUTE_KERNEL, 0, &p, gate,
					ifkernel2ifindex(ifkindex), vrf_id, table, 0, mtu, 0,
					SAFI_UNICAST);
		else
			rib_delete_ipv6(ZEBRA_ROUTE_KERNEL, zebra_flags, &p, gate,
					ifkernel2ifindex(ifkindex), vrf_id,
					SAFI_UNICAST);
	}
#endif /* HAVE_IPV6 */

	return 0;
}

static int netlink_link_change(struct sockaddr_nl *snl, struct nlmsghdr *h,
		vrf_id_t vrf_id)
{
	int len;
	struct ifinfomsg *ifi;
	struct rtattr *tb[IFLA_MAX + 1];
	struct interface *ifp;
	char ifkname[64];
	char *pname;

	ifi = NLMSG_DATA(h);

	if (!(h->nlmsg_type == RTM_NEWLINK || h->nlmsg_type == RTM_DELLINK))
	{
		/* If this is not link add/delete message so print warning. */
		zlog_warn(ZLOG_PAL,
				"netlink_link_change: wrong kernel message %s vrf %u\n",
				nl_msg_type_to_str(h->nlmsg_type), vrf_id);
		return 0;
	}

	len = h->nlmsg_len - NLMSG_LENGTH(sizeof(struct ifinfomsg));
	if (len < 0)
		return -1;

	/* Looking up interface name. */
	memset(tb, 0, sizeof tb);
	netlink_parse_rtattr(tb, IFLA_MAX, IFLA_RTA(ifi), len);

#ifdef IFLA_WIRELESS
	/* check for wireless messages to ignore */
	if ((tb[IFLA_WIRELESS] != NULL) && (ifi->ifi_change == 0))
	{
		if (IS_ZEBRA_DEBUG_KERNEL)
		zlog_debug (ZLOG_PAL,"%s: ignoring IFLA_WIRELESS message, vrf %u", __func__,
				vrf_id);
		return 0;
	}
#endif /* IFLA_WIRELESS */

	if (tb[IFLA_IFNAME] == NULL)
		return -1;
	pname = (char *) RTA_DATA(tb[IFLA_IFNAME]);

	memset(ifkname, 0, sizeof(ifkname));
	if (if_indextoname(ifi->ifi_index, ifkname) == NULL)
		strcpy(ifkname, pname);
	if (!if_lookup_by_kernel_name_vrf(ifkname, vrf_id) &&
			!if_lookup_by_kernel_index_vrf(ifi->ifi_index, vrf_id))
	{
		if (h->nlmsg_type == RTM_NEWLINK)
		{
			/* Add interface. */
			return 0;
		}
		return 0;
	}
	/* newname interface. */
	if (h->nlmsg_type == RTM_NEWLINK)
	{
		int update_ifindex = 0;
		ifp = if_lookup_by_kernel_index_vrf(ifi->ifi_index, vrf_id);
		if(!ifp)
		{
			ifp = if_lookup_by_kernel_name_vrf(ifkname, vrf_id);
			update_ifindex = 1;
		}
		if (!ifp)
		{
/*			set_ifindex(ifp, ifi->ifi_index);
			ifp->flags = ifi->ifi_flags & 0x0000fffff;
			ifp->mtu6 = ifp->mtu = *(int *) RTA_DATA(tb[IFLA_MTU]);
			ifp->metric = 0;

			netlink_interface_update_hw_addr(tb, ifp);*/
/*			else
			{
				if (ifp == NULL || !CHECK_FLAG(ifp->status, ZEBRA_INTERFACE_ACTIVE))
				{
					if (ifp == NULL)
					{
						ifp = if_create_vrf(name, strlen(name), vrf_id);
					}
				}
			}*/
			/* If new link is added. */
			// zhurish if_add_update(ifp);
		}
		else
		{
			/* Interface status change. */
			if_kname_set(ifp, ifkname);
			set_ifindex(ifp, ifi->ifi_index);
			ifp->mtu6 = ifp->mtu = *(int *) RTA_DATA(tb[IFLA_MTU]);
			//ifp->metric = 0;

			netlink_interface_update_hw_addr(tb, ifp);

			if (if_is_operative(ifp))
			{
				ifp->flags = ifi->ifi_flags & 0x0000fffff;
				if (!if_is_operative(ifp))
					if_down(ifp);
				else
					/* Must notify client daemons of new interface status. */
					zebra_interface_up_update(ifp);
			}
			else
			{
				ifp->flags = ifi->ifi_flags & 0x0000fffff;
				if (if_is_operative(ifp))
					if_up(ifp);
			}
		}
	}
	else
	{
		/* RTM_DELLINK. */
		ifp = if_lookup_by_kernel_name_vrf(ifkname, vrf_id);

		if (ifp == NULL)
		{
			zlog_warn(ZLOG_PAL, "interface %s vrf %u is deleted but can't find",
					  ifkname, vrf_id);
			return 0;
		}

		//zhurish if_delete_update(ifp);
	}

	return 0;
}

static int netlink_information_fetch(struct sockaddr_nl *snl,
		struct nlmsghdr *h, vrf_id_t vrf_id)
{
	if (h->nlmsg_pid && h->nlmsg_pid == snl->nl_pid)
	{
		zlog_err(ZLOG_PAL, "Ignoring %s message from pid %u", nl_msg_type_to_str(h->nlmsg_type), snl->nl_pid);
		return 0;
	}
	switch (h->nlmsg_type)
	{
	case RTM_NEWROUTE:
		return netlink_route_change(snl, h, vrf_id);
		break;
	case RTM_DELROUTE:
		return netlink_route_change(snl, h, vrf_id);
		break;
	case RTM_NEWLINK:
		return netlink_link_change(snl, h, vrf_id);
		break;
	case RTM_DELLINK:
		return netlink_link_change(snl, h, vrf_id);
		break;
	case RTM_NEWADDR:
		return netlink_interface_addr(snl, h, vrf_id);
		break;
	case RTM_DELADDR:
		return netlink_interface_addr(snl, h, vrf_id);
		break;
	default:
		zlog_warn(ZLOG_PAL, "Unknown netlink nlmsg_type %s vrf %u\n",
				nl_msg_type_to_str(h->nlmsg_type), vrf_id);
		break;
	}
	return 0;
}

/* Kernel route reflection. */
static int kernel_listen(struct thread *thread)
{
	struct nsm_vrf *zvrf = (struct nsm_vrf *) THREAD_ARG(thread);
	netlink_parse_info(netlink_information_fetch, &zvrf->netlink, zvrf);
	zvrf->t_netlink = thread_add_read(zebrad.master, kernel_listen, zvrf,
			zvrf->netlink.sock);

	return 0;
}

int kernel_nllisten(struct nsm_vrf *zvrf)
{
	if(zvrf->netlink.snl.nl_pid == 0)
		zvrf->netlink.snl.nl_pid = getpid();
	zvrf->t_netlink = thread_add_read(zebrad.master, kernel_listen, zvrf,
			zvrf->netlink.sock);
	return 0;
}
