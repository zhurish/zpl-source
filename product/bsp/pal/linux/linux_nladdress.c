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
#include "nsm_include.h"
#include "linux_driver.h"


/* Interface address modification. */
//void *d, ifindex_t kifindex, zpl_family_t family, zpl_uint8 prelen, union g_addr ifc, zpl_bool sec
static int _netlink_address(zpl_uint32 cmd, void *d, ifindex_t kifindex, zpl_family_t family, zpl_uint8 prelen, union g_addr ifc, zpl_bool sec)
{
	zpl_uint32 bytelen;

	struct
	{
		struct ipstack_nlmsghdr n;
		struct ipstack_ifaddrmsg ifa;
		char buf[NL_PKT_BUF_SIZE];
	} req;


	memset(&req, 0, sizeof req - NL_PKT_BUF_SIZE);

	bytelen = (family == IPSTACK_AF_INET ? 4 : 16);

	req.n.nlmsg_len = IPSTACK_NLMSG_LENGTH(sizeof(struct ipstack_ifaddrmsg));
	req.n.nlmsg_flags = IPSTACK_NLM_F_REQUEST;
	req.n.nlmsg_type = cmd;
	req.ifa.ifa_family = family;

	req.ifa.ifa_index = kifindex;
	req.ifa.ifa_prefixlen = prelen;
  if(family == IPSTACK_AF_INET)
	  _netlink_addattr_l(&req.n, sizeof req, IPSTACK_IFA_LOCAL, &ifc.ipv4, bytelen);
  else  
	  _netlink_addattr_l(&req.n, sizeof req, IPSTACK_IFA_LOCAL, &ifc.ipv6, bytelen);

	if (family == IPSTACK_AF_INET && cmd == IPSTACK_RTM_NEWADDR)
	{

			_netlink_addattr_l(&req.n, sizeof req, IPSTACK_IFA_BROADCAST, &ifc.ipv4, bytelen);

	}

	if (sec)
		SET_FLAG(req.ifa.ifa_flags, IPSTACK_IFA_F_SECONDARY);

	if (IS_ZEBRA_DEBUG_KERNEL)
		zlog_debug(MODULE_PAL, "netlink %s address on (%d)", (cmd == IPSTACK_RTM_NEWADDR) ? "add":"del",req.ifa.ifa_index);

	return _netlink_talk(&req.n, &netlink_cmd, IF_IFINDEX_VRFID_GET(ifkernel2ifindex(kifindex)));
}

static int _netlink_address_add_ipv4(void *d, ifindex_t kifindex, zpl_family_t family, zpl_uint8 prelen, union g_addr ifc, zpl_bool sec)
{
	return _netlink_address(IPSTACK_RTM_NEWADDR, d, kifindex, family, prelen, ifc, sec);
}

static int _netlink_address_delete_ipv4(void *d, ifindex_t kifindex, zpl_family_t family, zpl_uint8 prelen, union g_addr ifc, zpl_bool sec)
{
	return _netlink_address(IPSTACK_RTM_DELADDR, d, kifindex, family, prelen, ifc, sec);
}


/* Interface address setting via netlink interface. */
int _ipkernel_if_set_prefix(void *d, ifindex_t kifindex, zpl_family_t family, zpl_uint8 prelen, union g_addr ifc, zpl_bool sec)
{
  return _netlink_address_add_ipv4(d, kifindex, family, prelen, ifc, sec);
}

/* Interface address is removed using netlink interface. */
int _ipkernel_if_unset_prefix(void *d, ifindex_t kifindex, zpl_family_t family, zpl_uint8 prelen, union g_addr ifc, zpl_bool sec)
{
  return _netlink_address_delete_ipv4(d, kifindex, family, prelen, ifc, sec);
}

int _ipkernel_if_set_dst_prefix(void *d, ifindex_t kifindex, zpl_family_t family, zpl_uint8 prelen, union g_addr ifc, zpl_bool sec)
{
  int ret;
  struct ipstack_ifreq ipstack_ifreq;
  struct ipstack_sockaddr_in addr;

  strcpy(ipstack_ifreq.ifr_name, ifkernelindex2kernelifname(kifindex));
  addr.sin_addr = ifc.ipv4;
  addr.sin_family = family;
  memcpy(&ipstack_ifreq.ifr_addr, &addr, sizeof(struct ipstack_sockaddr_in));
  ret = _ipkernel_if_ioctl(IPSTACK_SIOCSIFDSTADDR, (caddr_t)&ipstack_ifreq);
  if (ret < 0)
    return ret;
  return 0;
}

/* Set up interface's address, netmask (and broadcas? ).  Linux or
   Solaris uses ifname:number semantics to set IP address aliases. */
int _ipkernel_if_unset_dst_prefix(void *d, ifindex_t kifindex, zpl_family_t family, zpl_uint8 prelen, union g_addr ifc, zpl_bool sec)
{
  int ret;
  struct ipstack_ifreq ipstack_ifreq;
  struct ipstack_sockaddr_in addr;
  strcpy(ipstack_ifreq.ifr_name, ifkernelindex2kernelifname(kifindex));

  memset(&addr, 0, sizeof(struct ipstack_sockaddr_in));
  addr.sin_family = family;
  addr.sin_addr = ifc.ipv4;
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
int _ipkernel_if_prefix_add_ipv6(void *d, ifindex_t kifindex, zpl_family_t family, zpl_uint8 prelen, union g_addr ifc, zpl_bool sec)
{
  int ret;
  struct ipstack_in6_ifreq ipstack_ifreq;

  memset(&ipstack_ifreq, 0, sizeof(struct ipstack_in6_ifreq));
  //addr.sin_family = family;
  //strcpy(ipstack_ifreq.ifr_name, ifkernelindex2kernelifname(kifindex));
  memcpy(&ipstack_ifreq.ifr6_addr, &ifc.ipv6, sizeof(struct ipstack_in6_addr));
  ipstack_ifreq.ifr6_ifindex = kifindex;
  ipstack_ifreq.ifr6_prefixlen = prelen;

  ret = _ipkernel_if_ioctl_ipv6(IPSTACK_SIOCSIFADDR, (caddr_t)&ipstack_ifreq);

  return ret;
}

int _ipkernel_if_prefix_delete_ipv6(void *d, ifindex_t kifindex, zpl_family_t family, zpl_uint8 prelen, union g_addr ifc, zpl_bool sec)
{
  int ret;
  struct ipstack_in6_ifreq ipstack_ifreq;

  memset(&ipstack_ifreq, 0, sizeof(struct ipstack_in6_ifreq));
  //addr.sin_family = family;
  //strcpy(ipstack_ifreq.ifr_name, ifkernelindex2kernelifname(kifindex));

  memcpy(&ipstack_ifreq.ifr6_addr, &ifc.ipv6, sizeof(struct ipstack_in6_addr));
  ipstack_ifreq.ifr6_ifindex = kifindex;
  ipstack_ifreq.ifr6_prefixlen = prelen;

  ret = _ipkernel_if_ioctl_ipv6(IPSTACK_SIOCDIFADDR, (caddr_t)&ipstack_ifreq);

  return ret;
}


#endif /* ZPL_BUILD_IPV6 */

