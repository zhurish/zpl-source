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

#include "linux_driver.h"


/* Interface address modification. */
static int _netlink_address(zpl_uint32 cmd, zpl_family_t family, struct interface *ifp,
		struct connected *ifc)
{
	zpl_uint32 bytelen;
	struct prefix *p;

	struct
	{
		struct ipstack_nlmsghdr n;
		struct ipstack_ifaddrmsg ifa;
		char buf[NL_PKT_BUF_SIZE];
	} req;

	struct nsm_ip_vrf *zvrf = ip_vrf_info_lookup(ifp->vrf_id);

	p = ifc->address;
	memset(&req, 0, sizeof req - NL_PKT_BUF_SIZE);

	bytelen = (family == IPSTACK_AF_INET ? 4 : 16);

	req.n.nlmsg_len = IPSTACK_NLMSG_LENGTH(sizeof(struct ipstack_ifaddrmsg));
	req.n.nlmsg_flags = IPSTACK_NLM_F_REQUEST;
	req.n.nlmsg_type = cmd;
	req.ifa.ifa_family = family;

	req.ifa.ifa_index = ifp->k_ifindex;
	req.ifa.ifa_prefixlen = p->prefixlen;

	_netlink_addattr_l(&req.n, sizeof req, IPSTACK_IFA_LOCAL, &p->u.prefix, bytelen);

	if (family == IPSTACK_AF_INET && cmd == IPSTACK_RTM_NEWADDR)
	{
		if (!CONNECTED_PEER(ifc) && ifc->destination)
		{
			p = ifc->destination;
			_netlink_addattr_l(&req.n, sizeof req, IPSTACK_IFA_BROADCAST, &p->u.prefix, bytelen);
		}
	}

	if (CHECK_FLAG(ifc->flags, ZEBRA_IFA_SECONDARY))
		SET_FLAG(req.ifa.ifa_flags, IPSTACK_IFA_F_SECONDARY);

	if (IS_ZEBRA_DEBUG_KERNEL)
		zlog_debug(MODULE_PAL, "netlink %s address on %s(%d)", (cmd == IPSTACK_RTM_NEWADDR) ? "add":"del",
				ifp->name, req.ifa.ifa_index);
	/*
	 if (ifc->label)
	 _netlink_addattr_l (&req.n, sizeof req, IPSTACK_IFA_LABEL, ifc->label,
	 strlen (ifc->label) + 1);
	 */
	return _netlink_talk(&req.n, &zvrf->netlink_cmd, zvrf);
}

static int _netlink_address_add_ipv4(struct interface *ifp, struct connected *ifc)
{
	return _netlink_address(IPSTACK_RTM_NEWADDR, IPSTACK_AF_INET, ifp, ifc);
}

static int _netlink_address_delete_ipv4(struct interface *ifp, struct connected *ifc)
{
	return _netlink_address(IPSTACK_RTM_DELADDR, IPSTACK_AF_INET, ifp, ifc);
}


/* Interface address setting via netlink interface. */
int _ipkernel_if_set_prefix(struct interface *ifp, struct connected *ifc)
{
  return _netlink_address_add_ipv4(ifp, ifc);
}

/* Interface address is removed using netlink interface. */
int _ipkernel_if_unset_prefix(struct interface *ifp, struct connected *ifc)
{
  return _netlink_address_delete_ipv4(ifp, ifc);
}

int _ipkernel_if_set_dst_prefix(struct interface *ifp, struct connected *ifc)
{
  int ret;
  struct ipstack_ifreq ipstack_ifreq;
  struct ipstack_sockaddr_in addr;
  struct prefix_ipv4 *p;

  p = (struct prefix_ipv4 *)ifc->destination;

  _ipkernel_ifreq_set_name(&ipstack_ifreq, ifp);

  addr.sin_addr = p->prefix;
  addr.sin_family = p->family;
  memcpy(&ipstack_ifreq.ifr_addr, &addr, sizeof(struct ipstack_sockaddr_in));
  ret = _ipkernel_if_ioctl(IPSTACK_SIOCSIFDSTADDR, (caddr_t)&ipstack_ifreq);
  if (ret < 0)
    return ret;
  return 0;
}

/* Set up interface's address, netmask (and broadcas? ).  Linux or
   Solaris uses ifname:number semantics to set IP address aliases. */
int _ipkernel_if_unset_dst_prefix(struct interface *ifp, struct connected *ifc)
{
  int ret;
  struct ipstack_ifreq ipstack_ifreq;
  struct ipstack_sockaddr_in addr;
  struct prefix_ipv4 *p;

  p = (struct prefix_ipv4 *)ifc->destination;

  _ipkernel_ifreq_set_name(&ipstack_ifreq, ifp);

  memset(&addr, 0, sizeof(struct ipstack_sockaddr_in));
  addr.sin_family = p->family;
  memcpy(&ipstack_ifreq.ifr_addr, &addr, sizeof(struct ipstack_sockaddr_in));
  ret = _ipkernel_if_ioctl(IPSTACK_SIOCSIFDSTADDR, (caddr_t)&ipstack_ifreq);
  if (ret < 0)
    return ret;

  return 0;
}

#ifdef ZPL_BUILD_IPV6

#ifndef _LINUX_IN6_H
/* linux/include/net/ipv6.h */
struct ipstack_in6_ifreq
{
  struct ipstack_in6_addr ifr6_addr;
  zpl_uint32 ifr6_prefixlen;
  int ifr6_ifindex;
};
#endif /* _LINUX_IN6_H */

/* Interface's address add/delete functions. */
int _ipkernel_if_prefix_add_ipv6(struct interface *ifp, struct connected *ifc, int sec)
{
  int ret;
  struct prefix_ipv6 *p;
  struct ipstack_in6_ifreq ipstack_ifreq;

  p = (struct prefix_ipv6 *)ifc->address;

  memset(&ipstack_ifreq, 0, sizeof(struct ipstack_in6_ifreq));

  memcpy(&ipstack_ifreq.ifr6_addr, &p->prefix, sizeof(struct ipstack_in6_addr));
  ipstack_ifreq.ifr6_ifindex = ifp->ifindex;
  ipstack_ifreq.ifr6_prefixlen = p->prefixlen;

  ret = _ipkernel_if_ioctl_ipv6(IPSTACK_SIOCSIFADDR, (caddr_t)&ipstack_ifreq);

  return ret;
}

int _ipkernel_if_prefix_delete_ipv6(struct interface *ifp, struct connected *ifc, int sec)
{
  int ret;
  struct prefix_ipv6 *p;
  struct ipstack_in6_ifreq ipstack_ifreq;

  p = (struct prefix_ipv6 *)ifc->address;

  memset(&ipstack_ifreq, 0, sizeof(struct ipstack_in6_ifreq));

  memcpy(&ipstack_ifreq.ifr6_addr, &p->prefix, sizeof(struct ipstack_in6_addr));
  ipstack_ifreq.ifr6_ifindex = ifp->ifindex;
  ipstack_ifreq.ifr6_prefixlen = p->prefixlen;

  ret = _ipkernel_if_ioctl_ipv6(IPSTACK_SIOCDIFADDR, (caddr_t)&ipstack_ifreq);

  return ret;
}


#endif /* ZPL_BUILD_IPV6 */