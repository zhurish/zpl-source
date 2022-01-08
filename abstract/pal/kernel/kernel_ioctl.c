/*
 * Common ipstack_ioctl functions.
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

#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"
#include <net/if_arp.h>


#ifdef ZPL_NSM_VLANETH
#include "nsm_vlaneth.h"
#endif
#ifdef ZPL_NSM_TUNNEL
#include "nsm_tunnel.h"
#endif
#ifdef ZPL_NSM_BRIDGE
#include "nsm_bridge.h"
#endif
#include "kernel_ioctl.h"
#include "kernel_driver.h"

#ifdef HAVE_BSD_LINK_DETECT
#include <net/if_media.h>
#endif /* HAVE_BSD_LINK_DETECT*/

#include "pal_driver.h"

/* clear and set interface name string */
void _ipkernel_ifreq_set_name(struct ipstack_ifreq *ipstack_ifreq, struct interface *ifp)
{
  strncpy(ipstack_ifreq->ifr_name, ifp->k_name, IFNAMSIZ);
}

/* call ipstack_ioctl system call */
int _ipkernel_if_ioctl(zpl_uint32 request, caddr_t buffer)
{
  zpl_socket_t sock;
  int ret = -1;

  sock = ipstack_socket(IPCOM_STACK, IPSTACK_AF_INET, IPSTACK_SOCK_DGRAM, 0);
  if (ipstack_invalid(sock))
  {
    zlog_err(MODULE_PAL, "Cannot create datagram ipstack_socket: %s",
             ipstack_strerror(ipstack_errno));
    return ret;
  }
  ret = ipstack_ioctl(sock, request, buffer);
  if (ret < 0)
  {
    zlog_err(MODULE_PAL, "Cannot ipstack_ioctl (0x%x) : %s", request, ipstack_strerror(ipstack_errno));
    ipstack_close(sock);
    return ret;
  }
  ipstack_close(sock);
  return 0;
}

#ifdef HAVE_IPV6
int _ipkernel_if_ioctl_ipv6(zpl_uint32 request, caddr_t buffer)
{
  zpl_socket_t sock;
  int ret = -1;

  sock = ipstack_socket(IPCOM_STACK, IPSTACK_AF_INET6, IPSTACK_SOCK_DGRAM, 0);
  if (ipstack_invalid(sock))
  {
    zlog_err(MODULE_PAL, "Cannot create IPv6 datagram ipstack_socket: %s",
             ipstack_strerror(ipstack_errno));
    return ret;
  }
  ret = ipstack_ioctl(sock, request, buffer);
  if (ret < 0)
  {
    zlog_err(MODULE_PAL, "Cannot ipstack_ioctl (0x%x) : %s",
             request, ipstack_strerror(ipstack_errno));
    ipstack_close(sock);
    return ret;
  }
  ipstack_close(sock);
  return 0;
}
#endif /* HAVE_IPV6 */

#if 0
static int
if_get_ifindex (char *name)
{
#ifdef SIOCGIFINDEX
  struct ipstack_ifreq ipstack_ifreq;
  strncpy (ipstack_ifreq.ifr_name, name, IFNAMSIZ);
  if (_ipkernel_if_ioctl (SIOCGIFINDEX, (caddr_t) &ipstack_ifreq) < 0)
    return 0;
  return ipstack_ifreq.ifr_ifindex;
#else  /* SIOCGIFINDEX */
  return if_nametoindex(name);
#endif /* SIOCGIFINDEX */
}
#endif
/*
 * get interface metric
 *   -- if value is not avaliable set -1
 */
static int
if_get_metric(struct interface *ifp)
{
#ifdef IPSTACK_SIOCGIFMETRIC
  struct ipstack_ifreq ipstack_ifreq;

  _ipkernel_ifreq_set_name(&ipstack_ifreq, ifp);

  if (_ipkernel_if_ioctl(IPSTACK_SIOCGIFMETRIC, (caddr_t)&ipstack_ifreq) < 0)
    return -1;
  ifp->metric = ipstack_ifreq.ifr_metric;
  if (ifp->metric == 0)
    ifp->metric = 1;
#else  /* IPSTACK_SIOCGIFMETRIC */
  ifp->metric = -1;
#endif /* IPSTACK_SIOCGIFMETRIC */
  return 0;
}

int _ipkernel_if_set_metric(struct interface *ifp, zpl_uint32 metric)
{
#ifdef IPSTACK_SIOCGIFMETRIC
  struct ipstack_ifreq ipstack_ifreq;
  _ipkernel_ifreq_set_name(&ipstack_ifreq, ifp);
  ipstack_ifreq.ifr_metric = metric;
  if (_ipkernel_if_ioctl(IPSTACK_SIOCGIFMETRIC, (caddr_t)&ipstack_ifreq) < 0)
    return -1;
  return 0;
#else  /* IPSTACK_SIOCGIFMETRIC */
  return 0;
#endif /* IPSTACK_SIOCGIFMETRIC */
}

/* get interface MTU */
static int
if_get_mtu(struct interface *ifp)
{
  struct ipstack_ifreq ipstack_ifreq;

  _ipkernel_ifreq_set_name(&ipstack_ifreq, ifp);

#if defined(IPSTACK_SIOCGIFMTU)
  if (_ipkernel_if_ioctl(IPSTACK_SIOCGIFMTU, (caddr_t)&ipstack_ifreq) < 0)
  {
    zlog_info(MODULE_PAL, "Can't lookup mtu by ipstack_ioctl(IPSTACK_SIOCGIFMTU)");
    ifp->mtu6 = ifp->mtu = 1500;
    return -1;
  }
#else
  zlog_err(MODULE_PAL, "Can't lookup mtu on this system");
  ifp->mtu6 = ifp->mtu = 1500;
#endif
  return 0;
}

int
_ipkernel_if_set_mtu(struct interface *ifp, zpl_uint32 mtu)
{
  struct ipstack_ifreq ipstack_ifreq;

  _ipkernel_ifreq_set_name(&ipstack_ifreq, ifp);

  ipstack_ifreq.ifr_mtu = mtu;

#if defined(IPSTACK_SIOCSIFMTU)
  if (_ipkernel_if_ioctl(IPSTACK_SIOCSIFMTU, (caddr_t)&ipstack_ifreq) < 0)
  {
    zlog_info(MODULE_PAL, "Can't lookup mtu by ipstack_ioctl(IPSTACK_SIOCGIFMTU)");
    ifp->mtu6 = ifp->mtu = -1;
    return -1;
  }
#endif
  return 0;
}

int
_ipkernel_if_get_hwaddr(struct interface *ifp)
{
  struct ipstack_ifreq ipstack_ifreq;

  _ipkernel_ifreq_set_name(&ipstack_ifreq, ifp);

#if defined(IPSTACK_SIOCGIFHWADDR)
  if (_ipkernel_if_ioctl(IPSTACK_SIOCGIFHWADDR, (caddr_t)&ipstack_ifreq) < 0)
  {
    zlog_info(MODULE_PAL, "Can't lookup MAC by ipstack_ioctl(IPSTACK_SIOCGIFHWADDR)");
    return -1;
  }
#endif
  // enum zebra_link_type ll_type;
  memcpy(ifp->hw_addr, ipstack_ifreq.ifr_hwaddr.sa_data, IFHWADDRLEN);
  ifp->hw_addr_len = IFHWADDRLEN;
  return 0;
}

/* get interface flags */
int
_ipkernel_if_get_flags(struct interface *ifp)
{
  int ret;
  struct ipstack_ifreq ipstack_ifreq;
  _ipkernel_ifreq_set_name(&ipstack_ifreq, ifp);

  if (ifp->k_ifindex == 0)
    ifp->k_ifindex = if_nametoindex(ifp->k_name);
  // ifp->k_ifindex = if_get_ifindex(ifp->k_name);
  if_get_mtu(ifp);
  if_get_metric(ifp);
  //  _ipkernel_if_get_hwaddr(ifp);
  ret = _ipkernel_if_ioctl(IPSTACK_SIOCGIFFLAGS, (caddr_t)&ipstack_ifreq);
  if (ret < 0)
  {
    zlog_err(MODULE_PAL, "_ipkernel_if_ioctl(IPSTACK_SIOCGIFFLAGS) failed: %s", ipstack_strerror(ipstack_errno));
    return -1;
  }
  ifp->flags |= ipstack_ifreq.ifr_flags;
  return 0;
}

/* Set interface flags */
static int
if_set_flags(struct interface *ifp, uint64_t flags)
{
  int ret;
  struct ipstack_ifreq ipstack_ifreq;

  memset(&ipstack_ifreq, 0, sizeof(struct ipstack_ifreq));
  _ipkernel_ifreq_set_name(&ipstack_ifreq, ifp);

  ipstack_ifreq.ifr_flags = ifp->flags;
  ipstack_ifreq.ifr_flags |= flags;

  ret = _ipkernel_if_ioctl(IPSTACK_SIOCSIFFLAGS, (caddr_t)&ipstack_ifreq);

  if (ret < 0)
  {
    zlog_info(MODULE_PAL, "can't set interface flags");
    return ret;
  }
  return 0;
}

/* Unset interface's flag. */
static int
if_unset_flags(struct interface *ifp, uint64_t flags)
{
  int ret;
  struct ipstack_ifreq ipstack_ifreq;

  memset(&ipstack_ifreq, 0, sizeof(struct ipstack_ifreq));
  _ipkernel_ifreq_set_name(&ipstack_ifreq, ifp);

  ipstack_ifreq.ifr_flags = ifp->flags;
  ipstack_ifreq.ifr_flags &= ~flags;

  ret = _ipkernel_if_ioctl(IPSTACK_SIOCSIFFLAGS, (caddr_t)&ipstack_ifreq);

  if (ret < 0)
  {
    zlog_info(MODULE_PAL, "can't unset interface flags");
    return ret;
  }
  return 0;
}

#ifdef HAVE_NETLINK
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
#else /* ! HAVE_NETLINK */
#ifdef HAVE_STRUCT_IFALIASREQ
/* Set up interface's IP address, netmask (and broadcas? ).  *BSD may
   has ifaliasreq structure.  */
int _ipkernel_if_set_prefix(struct interface *ifp, struct connected *ifc)
{
  int ret;
  struct ifaliasreq addreq;
  struct ipstack_sockaddr_in addr;
  struct ipstack_sockaddr_in mask;
  struct prefix_ipv4 *p;

  p = (struct prefix_ipv4 *)ifc->address;

  memset(&addreq, 0, sizeof addreq);
  strncpy((char *)&addreq.ifra_name, ifp->name, sizeof addreq.ifra_name);

  memset(&addr, 0, sizeof(struct ipstack_sockaddr_in));
  addr.sin_addr = p->prefix;
  addr.sin_family = p->family;
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
  addr.sin_len = sizeof(struct ipstack_sockaddr_in);
#endif
  memcpy(&addreq.ifra_addr, &addr, sizeof(struct ipstack_sockaddr_in));

  memset(&mask, 0, sizeof(struct ipstack_sockaddr_in));
  masklen2ip(p->prefixlen, &mask.sin_addr);
  mask.sin_family = p->family;
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
  mask.sin_len = sizeof(struct ipstack_sockaddr_in);
#endif
  memcpy(&addreq.ifra_mask, &mask, sizeof(struct ipstack_sockaddr_in));

  ret = _ipkernel_if_ioctl(IPSTACK_SIOCAIFADDR, (caddr_t)&addreq);
  if (ret < 0)
    return ret;
  return 0;
}

/* Set up interface's IP address, netmask (and broadcas? ).  *BSD may
   has ifaliasreq structure.  */
int _ipkernel_if_unset_prefix(struct interface *ifp, struct connected *ifc)
{
  int ret;
  struct ipstack_ifaliasreq addreq;
  struct ipstack_sockaddr_in addr;
  struct ipstack_sockaddr_in mask;
  struct prefix_ipv4 *p;

  p = (struct prefix_ipv4 *)ifc->address;

  memset(&addreq, 0, sizeof addreq);
  strncpy((char *)&addreq.ifra_name, ifp->name, sizeof addreq.ifra_name);

  memset(&addr, 0, sizeof(struct ipstack_sockaddr_in));
  addr.sin_addr = p->prefix;
  addr.sin_family = p->family;
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
  addr.sin_len = sizeof(struct ipstack_sockaddr_in);
#endif
  memcpy(&addreq.ifra_addr, &addr, sizeof(struct ipstack_sockaddr_in));

  memset(&mask, 0, sizeof(struct ipstack_sockaddr_in));
  masklen2ip(p->prefixlen, &mask.sin_addr);
  mask.sin_family = p->family;
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
  mask.sin_len = sizeof(struct ipstack_sockaddr_in);
#endif
  memcpy(&addreq.ifra_mask, &mask, sizeof(struct ipstack_sockaddr_in));

  ret = _ipkernel_if_ioctl(IPSTACK_SIOCDIFADDR, (caddr_t)&addreq);
  if (ret < 0)
    return ret;
  return 0;
}
#else
/* Set up interface's address, netmask (and broadcas? ).  Linux or
   Solaris uses ifname:number semantics to set IP address aliases. */
int _ipkernel_if_set_prefix(struct interface *ifp, struct connected *ifc)
{
  int ret;
  struct ipstack_ifreq ipstack_ifreq;
  struct ipstack_sockaddr_in addr;
  struct ipstack_sockaddr_in broad;
  struct ipstack_sockaddr_in mask;
  struct prefix_ipv4 ifaddr;
  struct prefix_ipv4 *p;

  p = (struct prefix_ipv4 *)ifc->address;

  ifaddr = *p;

  _ipkernel_ifreq_set_name(&ipstack_ifreq, ifp);

  addr.sin_addr = p->prefix;
  addr.sin_family = p->family;
  memcpy(&ipstack_ifreq.ifr_addr, &addr, sizeof(struct ipstack_sockaddr_in));
  ret = _ipkernel_if_ioctl(IPSTACK_SIOCSIFADDR, (caddr_t)&ipstack_ifreq);
  if (ret < 0)
    return ret;

  /* We need mask for make broadcast addr. */
  masklen2ip(p->prefixlen, &mask.sin_addr);

  if (if_is_broadcast(ifp))
  {
    apply_mask_ipv4(&ifaddr);
    addr.sin_addr = ifaddr.prefix;

    broad.sin_addr.s_addr = (addr.sin_addr.s_addr | ~mask.sin_addr.s_addr);
    broad.sin_family = p->family;

    memcpy(&ipstack_ifreq.ifr_broadaddr, &broad, sizeof(struct ipstack_sockaddr_in));
    ret = _ipkernel_if_ioctl(IPSTACK_SIOCSIFBRDADDR, (caddr_t)&ipstack_ifreq);
    if (ret < 0)
      return ret;
  }

  mask.sin_family = p->family;
#ifdef SUNOS_5
  memcpy(&mask, &ipstack_ifreq.ifr_addr, sizeof(mask));
#else
  memcpy(&ipstack_ifreq.ifr_netmask, &mask, sizeof(struct ipstack_sockaddr_in));
#endif /* SUNOS5 */
  ret = _ipkernel_if_ioctl(IPSTACK_SIOCSIFNETMASK, (caddr_t)&ipstack_ifreq);
  if (ret < 0)
    return ret;

  return 0;
}

/* Set up interface's address, netmask (and broadcas? ).  Linux or
   Solaris uses ifname:number semantics to set IP address aliases. */
int _ipkernel_if_unset_prefix(struct interface *ifp, struct connected *ifc)
{
  int ret;
  struct ipstack_ifreq ipstack_ifreq;
  struct ipstack_sockaddr_in addr;
  struct prefix_ipv4 *p;

  p = (struct prefix_ipv4 *)ifc->address;

  _ipkernel_ifreq_set_name(&ipstack_ifreq, ifp);

  memset(&addr, 0, sizeof(struct ipstack_sockaddr_in));
  addr.sin_family = p->family;
  memcpy(&ipstack_ifreq.ifr_addr, &addr, sizeof(struct ipstack_sockaddr_in));
  ret = _ipkernel_if_ioctl(IPSTACK_SIOCSIFADDR, (caddr_t)&ipstack_ifreq);
  if (ret < 0)
    return ret;

  return 0;
}
#endif /* HAVE_STRUCT_IFALIASREQ */
#endif /* HAVE_NETLINK */

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

#ifdef HAVE_IPV6

#ifdef LINUX_IPV6
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
#else /* LINUX_IPV6 */
#ifdef HAVE_STRUCT_IN6_ALIASREQ
#ifndef ND6_INFINITE_LIFETIME
#define ND6_INFINITE_LIFETIME 0xffffffffL
#endif /* ND6_INFINITE_LIFETIME */
int _ipkernel_if_prefix_add_ipv6(struct interface *ifp, struct connected *ifc)
{
  int ret;
  struct ipstack_in6_aliasreq addreq;
  struct ipstack_sockaddr_in6 addr;
  struct ipstack_sockaddr_in6 mask;
  struct prefix_ipv6 *p;

  p = (struct prefix_ipv6 *)ifc->address;

  memset(&addreq, 0, sizeof addreq);
  strncpy((char *)&addreq.ifra_name, ifp->name, sizeof addreq.ifra_name);

  memset(&addr, 0, sizeof(struct ipstack_sockaddr_in6));
  addr.sin6_addr = p->prefix;
  addr.sin6_family = p->family;
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
  addr.sin6_len = sizeof(struct ipstack_sockaddr_in6);
#endif
  memcpy(&addreq.ifra_addr, &addr, sizeof(struct ipstack_sockaddr_in6));

  memset(&mask, 0, sizeof(struct ipstack_sockaddr_in6));
  masklen2ip6(p->prefixlen, &mask.sin6_addr);
  mask.sin6_family = p->family;
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
  mask.sin6_len = sizeof(struct ipstack_sockaddr_in6);
#endif
  memcpy(&addreq.ifra_prefixmask, &mask, sizeof(struct ipstack_sockaddr_in6));

  addreq.ifra_lifetime.ia6t_vltime = 0xffffffff;
  addreq.ifra_lifetime.ia6t_pltime = 0xffffffff;

#ifdef HAVE_STRUCT_IF6_ALIASREQ_IFRA_LIFETIME
  addreq.ifra_lifetime.ia6t_pltime = ND6_INFINITE_LIFETIME;
  addreq.ifra_lifetime.ia6t_vltime = ND6_INFINITE_LIFETIME;
#endif

  ret = _ipkernel_if_ioctl_ipv6(IPSTACK_SIOCAIFADDR_IN6, (caddr_t)&addreq);
  if (ret < 0)
    return ret;
  return 0;
}

int _ipkernel_if_prefix_delete_ipv6(struct interface *ifp, struct connected *ifc)
{
  int ret;
  struct ipstack_in6_aliasreq addreq;
  struct ipstack_sockaddr_in6 addr;
  struct ipstack_sockaddr_in6 mask;
  struct prefix_ipv6 *p;

  p = (struct prefix_ipv6 *)ifc->address;

  memset(&addreq, 0, sizeof addreq);
  strncpy((char *)&addreq.ifra_name, ifp->name, sizeof addreq.ifra_name);

  memset(&addr, 0, sizeof(struct ipstack_sockaddr_in6));
  addr.sin6_addr = p->prefix;
  addr.sin6_family = p->family;
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
  addr.sin6_len = sizeof(struct ipstack_sockaddr_in6);
#endif
  memcpy(&addreq.ifra_addr, &addr, sizeof(struct ipstack_sockaddr_in6));

  memset(&mask, 0, sizeof(struct ipstack_sockaddr_in6));
  masklen2ip6(p->prefixlen, &mask.sin6_addr);
  mask.sin6_family = p->family;
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
  mask.sin6_len = sizeof(struct ipstack_sockaddr_in6);
#endif
  memcpy(&addreq.ifra_prefixmask, &mask, sizeof(struct ipstack_sockaddr_in6));

#ifdef HAVE_STRUCT_IF6_ALIASREQ_IFRA_LIFETIME
  addreq.ifra_lifetime.ia6t_pltime = ND6_INFINITE_LIFETIME;
  addreq.ifra_lifetime.ia6t_vltime = ND6_INFINITE_LIFETIME;
#endif

  ret = _ipkernel_if_ioctl_ipv6(IPSTACK_SIOCDIFADDR_IN6, (caddr_t)&addreq);
  if (ret < 0)
    return ret;
  return 0;
}
#else
int _ipkernel_if_prefix_add_ipv6(struct interface *ifp, struct connected *ifc)
{
  return 0;
}

int _ipkernel_if_prefix_delete_ipv6(struct interface *ifp, struct connected *ifc)
{
  return 0;
}
#endif /* HAVE_STRUCT_IN6_ALIASREQ */

#endif /* LINUX_IPV6 */

#endif /* HAVE_IPV6 */

int _ipkernel_if_set_mac(struct interface *ifp, zpl_uint8 *mac, zpl_uint32 len)
{
  int ret;
  struct ipstack_ifreq ipstack_ifreq;

  memset(&ipstack_ifreq, 0, sizeof(struct ipstack_ifreq));
  _ipkernel_ifreq_set_name(&ipstack_ifreq, ifp);
  ipstack_ifreq.ifr_hwaddr.sa_family = IPSTACK_ARPHRD_ETHER;
  memcpy(ipstack_ifreq.ifr_hwaddr.sa_data, mac, len);
  ret = _ipkernel_if_ioctl(SIOCSIFHWADDR, (caddr_t)&ipstack_ifreq);
  if (ret < 0)
  {
    zlog_info(MODULE_PAL, "can't unset interface flags");
    return ret;
  }
  return 0;
}

int _ipkernel_if_set_up(struct interface *ifp)
{
  return if_set_flags(ifp, IPSTACK_IFF_UP | IPSTACK_IFF_RUNNING);
}

int _ipkernel_if_set_down(struct interface *ifp)
{
  return if_unset_flags(ifp, IPSTACK_IFF_UP | IPSTACK_IFF_RUNNING);
}

int _ipkernel_if_update_flags(struct interface *ifp, uint64_t flag)
{
  return if_set_flags(ifp, flag);
}

#if 0
/* Return statistics data pointer. */
static char *
interface_name_cut (char *buf, char **name)
{
  char *stat;

  /* Skip white space.  Line will include header spaces. */
  while (*buf == ' ')
    buf++;
  *name = buf;

  /* Cut interface name. */
  stat = strrchr (buf, ':');
  *stat++ = '\0';

  return stat;
}

/* Fetch each statistics field. */
static int
if_get_statistic (int version, char *buf, struct interface *ifp)
{
  switch (version)
    {
    case 3:
      sscanf(buf,
	     "%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld",
	     &ifp->stIfStatistics.rx_bytes,
	     &ifp->stIfStatistics.rx_packets,
	     &ifp->stIfStatistics.rx_errors,
	     &ifp->stIfStatistics.rx_dropped,
	     &ifp->stIfStatistics.rx_fifo_errors,
	     &ifp->stIfStatistics.rx_frame_errors,
	     &ifp->stIfStatistics.rx_compressed,
	     &ifp->stIfStatistics.rx_multicast,

	     &ifp->stIfStatistics.tx_bytes,
	     &ifp->stIfStatistics.tx_packets,
	     &ifp->stIfStatistics.tx_errors,
	     &ifp->stIfStatistics.tx_dropped,
	     &ifp->stIfStatistics.tx_fifo_errors,
	     &ifp->stIfStatistics.collisions,
	     &ifp->stIfStatistics.tx_carrier_errors,
	     &ifp->stIfStatistics.tx_compressed);
      break;
    case 2:
      sscanf(buf, "%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld",
	     &ifp->stIfStatistics.rx_bytes,
	     &ifp->stIfStatistics.rx_packets,
	     &ifp->stIfStatistics.rx_errors,
	     &ifp->stIfStatistics.rx_dropped,
	     &ifp->stIfStatistics.rx_fifo_errors,
	     &ifp->stIfStatistics.rx_frame_errors,

	     &ifp->stIfStatistics.tx_bytes,
	     &ifp->stIfStatistics.tx_packets,
	     &ifp->stIfStatistics.tx_errors,
	     &ifp->stIfStatistics.tx_dropped,
	     &ifp->stIfStatistics.tx_fifo_errors,
	     &ifp->stIfStatistics.collisions,
	     &ifp->stIfStatistics.tx_carrier_errors);
      ifp->stIfStatistics.rx_multicast = 0;
      break;
    case 1:
      sscanf(buf, "%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld",
	     &ifp->stIfStatistics.rx_packets,
	     &ifp->stIfStatistics.rx_errors,
	     &ifp->stIfStatistics.rx_dropped,
	     &ifp->stIfStatistics.rx_fifo_errors,
	     &ifp->stIfStatistics.rx_frame_errors,

	     &ifp->stIfStatistics.tx_packets,
	     &ifp->stIfStatistics.tx_errors,
	     &ifp->stIfStatistics.tx_dropped,
	     &ifp->stIfStatistics.tx_fifo_errors,
	     &ifp->stIfStatistics.collisions,
	     &ifp->stIfStatistics.tx_carrier_errors);
      ifp->stIfStatistics.rx_bytes = 0;
      ifp->stIfStatistics.tx_bytes = 0;
      ifp->stIfStatistics.rx_multicast = 0;
      break;
    }
  return 0;
}

/* Update interface's statistics. */
void
ifstat_update_proc (void)
{
  FILE *fp;
  char buf[PROCBUFSIZ];
  int version;
  struct interface *ifp = NULL;
  char *stat;
  char *name;

  /* Open /proc/net/dev. */
  fp = fopen (_PATH_PROC_NET_DEV, "r");
  if (fp == NULL)
    {
      zlog_warn (MTYPE_INTERFACE, "Can't open proc file %s: %s",
		 _PATH_PROC_NET_DEV, ipstack_strerror (ipstack_errno));
      return;
    }

  /* Drop header lines. */
  fgets (buf, PROCBUFSIZ, fp);
  fgets (buf, PROCBUFSIZ, fp);

  /* To detect proc format veresion, parse second line. */
  if (strstr (buf, "compressed"))
    version = 3;
  else if (strstr (buf, "bytes"))
    version = 2;
  else
    version = 1;

  /* Update each interface's statistics. */
  while (fgets (buf, PROCBUFSIZ, fp) != NULL)
  {
    stat = interface_name_cut (buf, &name);
    if (NULL != (ifp = if_lookup_by_name (name)))
    {
      if_get_statistic (version, stat, ifp);
    }
  }

  fclose(fp);

  return;
}

#endif



