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

#ifdef ZPL_KERNEL_NETLINK
/* Interface address modification. */
//void *d, ifindex_t kifindex, zpl_family_t family, zpl_uint8 prelen, union g_addr ifc, zpl_bool sec
static int librtnl_address(zpl_uint32 cmd, int family, struct interface *ifp, struct connected *ifc, zpl_bool sec)
{
	zpl_uint32 bytelen;
  struct prefix *p;
	struct
	{
		struct ipstack_nlmsghdr n;
		struct ipstack_ifaddrmsg ifa;
		char buf[NL_PKT_BUF_SIZE];
	} req;


	memset(&req, 0, sizeof req - NL_PKT_BUF_SIZE);
  p = ifc->address;
	bytelen = (family == IPSTACK_AF_INET ? 4 : 16);

	req.n.nlmsg_len = IPSTACK_NLMSG_LENGTH(sizeof(struct ipstack_ifaddrmsg));
	req.n.nlmsg_flags = IPSTACK_NLM_F_REQUEST;
	req.n.nlmsg_type = cmd;
	req.ifa.ifa_family = family;
  req.ifa.ifa_index = ifp->ker_ifindex;
  req.ifa.ifa_prefixlen = p->prefixlen;

  if(family == IPSTACK_AF_INET)
	  librtnl_addattr_l(&req.n, sizeof req, IPSTACK_IFA_LOCAL, &p->u.prefix, bytelen);
  else  
	  librtnl_addattr_l(&req.n, sizeof req, IPSTACK_IFA_LOCAL, &p->u.prefix, bytelen);

  if (family == IPSTACK_AF_INET && cmd == IPSTACK_RTM_NEWADDR)
  {
      if (!CONNECTED_PEER(ifc) && ifc->destination)
      {
          p = ifc->destination;
          librtnl_addattr_l (&req.n, sizeof req, IPSTACK_IFA_BROADCAST, &p->u.prefix,  bytelen);
      }
  }

  if (sec)
    SET_FLAG (req.ifa.ifa_flags, IPSTACK_IFA_F_SECONDARY);
/*
  if (ifc->label)
    addattr_l (&req.n, sizeof req, IFA_LABEL, ifc->label,
               strlen (ifc->label) + 1);
*/
	if (IS_NSM_DEBUG_KERNEL)
		zlog_debug(MODULE_PAL, "netlink %s address on (%d)", (cmd == IPSTACK_RTM_NEWADDR) ? "add":"del",req.ifa.ifa_index);

	return librtnl_talk(&netlink_cmd, &req.n);
}

static int librtnl_address_add_ipv4(struct interface *ifp, struct connected *ifc)
{
	return librtnl_address(IPSTACK_RTM_NEWADDR,  IPSTACK_AF_INET, ifp, ifc, 0);
}

static int librtnl_address_delete_ipv4(struct interface *ifp, struct connected *ifc)
{
	return librtnl_address(IPSTACK_RTM_DELADDR, IPSTACK_AF_INET, ifp, ifc, 0);
}


/* Interface address setting via netlink interface. */
int linux_ioctl_if_set_prefix(struct interface *ifp, struct connected *ifc)
{
  return librtnl_address_add_ipv4(ifp, ifc);
}

/* Interface address is removed using netlink interface. */
int linux_ioctl_if_unset_prefix(struct interface *ifp, struct connected *ifc)
{
  return librtnl_address_delete_ipv4(ifp, ifc);
}

int linux_ioctl_if_set_dst_prefix(struct interface *ifp, struct connected *ifc)
{
  int ret;
  struct prefix *p;
  p = ifc->address;
  struct ipstack_ifreq ipstack_ifreq;
  struct ipstack_sockaddr_in addr;

  strcpy(ipstack_ifreq.ifr_name, ifkernelindex2kernelifname(ifp->ker_ifindex));
  addr.sin_addr = p->u.prefix4;
  addr.sin_family = IPSTACK_AF_INET;
  memcpy(&ipstack_ifreq.ifr_addr, &addr, sizeof(struct ipstack_sockaddr_in));
  ret = linux_ioctl_if_ioctl(IPSTACK_SIOCSIFDSTADDR, (caddr_t)&ipstack_ifreq);
  if (ret < 0)
    return ret;
  return 0;
}

/* Set up interface's address, netmask (and broadcas? ).  Linux or
   Solaris uses ifname:number semantics to set IP address aliases. */
int linux_ioctl_if_unset_dst_prefix(struct interface *ifp, struct connected *ifc)
{
  int ret;
  struct prefix *p;
  p = ifc->address;
  struct ipstack_ifreq ipstack_ifreq;
  struct ipstack_sockaddr_in addr;
  strcpy(ipstack_ifreq.ifr_name, ifkernelindex2kernelifname(ifp->ker_ifindex));

  memset(&addr, 0, sizeof(struct ipstack_sockaddr_in));
  addr.sin_family = IPSTACK_AF_INET;
  addr.sin_addr = p->u.prefix4;
  memcpy(&ipstack_ifreq.ifr_addr, &addr, sizeof(struct ipstack_sockaddr_in));
  ret = linux_ioctl_if_ioctl(IPSTACK_SIOCSIFDSTADDR, (caddr_t)&ipstack_ifreq);
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
int linux_ioctl_if_prefix_add_ipv6(struct interface *ifp, struct connected *ifc, zpl_bool sec)
{
  int ret;
  struct ipstack_in6_ifreq ipstack_ifreq;
  struct prefix *p;
  p = ifc->address;
  memset(&ipstack_ifreq, 0, sizeof(struct ipstack_in6_ifreq));
  //addr.sin_family = family;
  //strcpy(ipstack_ifreq.ifr_name, ifkernelindex2kernelifname(kifindex));
  memcpy(&ipstack_ifreq.ifr6_addr, &p->u.prefix6, sizeof(struct ipstack_in6_addr));
  ipstack_ifreq.ifr6_ifindex = ifp->ker_ifindex;
  ipstack_ifreq.ifr6_prefixlen = p->prefixlen;

  ret = linux_ioctl_if_ioctl_ipv6(IPSTACK_SIOCSIFADDR, (caddr_t)&ipstack_ifreq);

  return ret;
}

int linux_ioctl_if_prefix_delete_ipv6(struct interface *ifp, struct connected *ifc, zpl_bool sec)
{
  int ret;
  struct ipstack_in6_ifreq ipstack_ifreq;
  struct prefix *p;
  p = ifc->address;
  memset(&ipstack_ifreq, 0, sizeof(struct ipstack_in6_ifreq));
  //addr.sin_family = family;
  //strcpy(ipstack_ifreq.ifr_name, ifkernelindex2kernelifname(kifindex));

  memcpy(&ipstack_ifreq.ifr6_addr, &p->u.prefix6, sizeof(struct ipstack_in6_addr));
  ipstack_ifreq.ifr6_ifindex = ifp->ker_ifindex;
  ipstack_ifreq.ifr6_prefixlen = p->prefixlen;

  ret = linux_ioctl_if_ioctl_ipv6(IPSTACK_SIOCDIFADDR, (caddr_t)&ipstack_ifreq);

  return ret;
}


#endif /* ZPL_BUILD_IPV6 */

#endif /* ZPL_KERNEL_NETLINK */

#ifdef ZPL_LIBNL_MODULE
static int rtnl_addr_opt(int op, int family, char *name, char *addr, char *baddr, char *paddr, int scope)
{
  struct nl_cache *link_cache;
  struct rtnl_addr *rtnladdr;
  struct nl_addr *a, *ba, *pa;
  int err, ival = 0, nlflags = NLM_F_REPLACE | NLM_F_CREATE;

  if ((err = rtnl_link_alloc_cache(netlink_cmd.libnl_sock, AF_UNSPEC, &link_cache)) < 0)
  {
    nl_perror(err, "Unable to allocate cache");
    return err;
  }
  rtnladdr = nl_cli_addr_alloc();

  if (!(ival = rtnl_link_name2i(link_cache, name)))
    nl_perror(err, "Link does not exist");

  rtnl_addr_set_ifindex(rtnladdr, ival);

  nl_addr_parse(addr, rtnl_addr_get_family(addr), &a);
  if ((err = rtnl_addr_set_local(rtnladdr, a)) < 0)
    nl_perror(err, "Unable to set local address");
  nl_addr_put(a);

  nl_addr_parse(baddr, rtnl_addr_get_family(addr), &ba);
  if ((err = rtnl_addr_set_broadcast(rtnladdr, ba)) < 0)
    nl_perror(err, "Unable to set broadcast address");
  nl_addr_put(ba);

  nl_addr_parse(paddr, rtnl_addr_get_family(addr), &pa);
  if ((err = rtnl_addr_set_peer(addr, pa)) < 0)
    nl_perror(err, "Unable to set peer address");
  nl_addr_put(pa);

  rtnl_addr_set_scope(rtnladdr, scope);
  rtnl_addr_set_family(rtnladdr, family);
  if (op)
  {
    if ((err = rtnl_addr_add(netlink_cmd.libnl_sock, rtnladdr, nlflags)) < 0)
      nl_perror(err, "Unable to add neighbour");
  }
  else
  {
    if ((err = rtnl_addr_delete(netlink_cmd.libnl_sock, rtnladdr, 0)) < 0)
      nl_perror(err, "Unable to delete neighbour");
  }
  return err;
}
#endif
