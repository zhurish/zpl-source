/*
 * Common ioctl functions.
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

#include <zebra.h>
#include <net/if_arp.h>
#include "linklist.h"
#include "if.h"
#include "prefix.h"
#include "log.h"
#include "rib.h"
#include "interface.h"

#include "nsm_veth.h"
#include "nsm_tunnel.h"
#include "nsm_bridge.h"

#include "pal_interface.h"
#include "kernel_ioctl.h"

#ifdef HAVE_BSD_LINK_DETECT
#include <net/if_media.h>
#endif /* HAVE_BSD_LINK_DETECT*/


/* clear and set interface name string */
void
ifreq_set_name (struct ifreq *ifreq, struct interface *ifp)
{
  strncpy (ifreq->ifr_name, ifp->k_name, IFNAMSIZ);
}

/* call ioctl system call */
int
if_ioctl (u_long request, caddr_t buffer)
{
	int sock;
	int ret = -1;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
	{
		zlog_err(ZLOG_PAL, "Cannot create datagram socket: %s",
				safe_strerror(errno));
		return ret;
	}
	ret = ioctl(sock, request, buffer);
	if (ret < 0)
	{
		zlog_err(ZLOG_PAL, "Cannot ioctl (0x%x) : %s",
				request, safe_strerror(errno));
		close(sock);
		return ret;
	}
	close(sock);
	return 0;
}

#ifdef HAVE_IPV6
int
if_ioctl_ipv6 (u_long request, caddr_t buffer)
{
	int sock;
	int ret = -1;

	sock = socket(AF_INET6, SOCK_DGRAM, 0);
	if (sock < 0)
	{
		zlog_err(ZLOG_PAL, "Cannot create IPv6 datagram socket: %s",
				safe_strerror(errno));
		return ret;
	}
	ret = ioctl(sock, request, buffer);
	if (ret < 0)
	{
		zlog_err(ZLOG_PAL, "Cannot ioctl (0x%x) : %s",
				request, safe_strerror(errno));
		close(sock);
		return ret;
	}
	close(sock);
	return 0;
}
#endif /* HAVE_IPV6 */

#if 0
static int
if_get_ifindex (char *name)
{
#ifdef SIOCGIFINDEX
  struct ifreq ifreq;
  strncpy (ifreq.ifr_name, name, IFNAMSIZ);
  if (if_ioctl (SIOCGIFINDEX, (caddr_t) &ifreq) < 0)
    return 0;
  return ifreq.ifr_ifindex;
#else /* SIOCGIFINDEX */
  return if_nametoindex(name);
#endif /* SIOCGIFINDEX */
}
#endif
/*
 * get interface metric
 *   -- if value is not avaliable set -1
 */
static int
if_get_metric (struct interface *ifp)
{
#ifdef SIOCGIFMETRIC
  struct ifreq ifreq;

  ifreq_set_name (&ifreq, ifp);

  if (if_ioctl (SIOCGIFMETRIC, (caddr_t) &ifreq) < 0) 
    return -1;
  ifp->metric = ifreq.ifr_metric;
  if (ifp->metric == 0)
    ifp->metric = 1;
#else /* SIOCGIFMETRIC */
  ifp->metric = -1;
#endif /* SIOCGIFMETRIC */
  return 0;
}

static int
if_set_metric (struct interface *ifp, int metric)
{
#ifdef SIOCGIFMETRIC
  struct ifreq ifreq;
  ifreq_set_name (&ifreq, ifp);
  ifreq.ifr_metric = metric;
  if (if_ioctl (SIOCGIFMETRIC, (caddr_t) &ifreq) < 0)
    return -1;
  return 0;
#else /* SIOCGIFMETRIC */
  return 0;
#endif /* SIOCGIFMETRIC */
}

/* get interface MTU */
static int
if_get_mtu (struct interface *ifp)
{
  struct ifreq ifreq;

  ifreq_set_name (&ifreq, ifp);

#if defined(SIOCGIFMTU)
  if (if_ioctl (SIOCGIFMTU, (caddr_t) & ifreq) < 0) 
    {
      zlog_info (ZLOG_PAL, "Can't lookup mtu by ioctl(SIOCGIFMTU)");
      ifp->mtu6 = ifp->mtu = 1500;
      return -1;
    }
#else
  zlog_err (ZLOG_PAL, "Can't lookup mtu on this system");
  ifp->mtu6 = ifp->mtu = 1500;
#endif
  return 0;
}

static int
if_set_mtu (struct interface *ifp, int mtu)
{
  struct ifreq ifreq;

  ifreq_set_name (&ifreq, ifp);

  ifreq.ifr_mtu = mtu;

#if defined(SIOCSIFMTU)
  if (if_ioctl (SIOCSIFMTU, (caddr_t) & ifreq) < 0)
    {
      zlog_info (ZLOG_PAL, "Can't lookup mtu by ioctl(SIOCGIFMTU)");
      ifp->mtu6 = ifp->mtu = -1;
      return -1;
    }
#endif
  return 0;
}


static int
if_get_hwaddr (struct interface *ifp)
{
  struct ifreq ifreq;

  ifreq_set_name (&ifreq, ifp);

#if defined(SIOCGIFHWADDR)
  if (if_ioctl (SIOCGIFHWADDR, (caddr_t) & ifreq) < 0)
    {
      zlog_info (ZLOG_PAL, "Can't lookup MAC by ioctl(SIOCGIFHWADDR)");
      return -1;
    }
#endif
 // enum zebra_link_type ll_type;
  memcpy(ifp->hw_addr, ifreq.ifr_hwaddr.sa_data, IFHWADDRLEN);
  ifp->hw_addr_len = IFHWADDRLEN;
  return 0;
}

/* get interface flags */
static int
if_get_flags (struct interface *ifp)
{
  int ret;
  struct ifreq ifreq;
  ifreq_set_name (&ifreq, ifp);

  if(ifp->k_ifindex == 0)
	  ifp->k_ifindex = if_nametoindex(ifp->k_name);
 // ifp->k_ifindex = if_get_ifindex(ifp->k_name);
  if_get_mtu(ifp);
  if_get_metric(ifp);
//  if_get_hwaddr(ifp);
  ret = if_ioctl (SIOCGIFFLAGS, (caddr_t) &ifreq);
  if (ret < 0)
    {
      zlog_err(ZLOG_PAL, "if_ioctl(SIOCGIFFLAGS) failed: %s", safe_strerror(errno));
      return -1;
    }
  ifp->flags |= ifreq.ifr_flags;
  return 0;
}


/* Set interface flags */
static int
if_set_flags (struct interface *ifp, uint64_t flags)
{
  int ret;
  struct ifreq ifreq;

  memset (&ifreq, 0, sizeof(struct ifreq));
  ifreq_set_name (&ifreq, ifp);

  ifreq.ifr_flags = ifp->flags;
  ifreq.ifr_flags |= flags;

  ret = if_ioctl (SIOCSIFFLAGS, (caddr_t) &ifreq);

  if (ret < 0)
    {
      zlog_info (ZLOG_PAL, "can't set interface flags");
      return ret;
    }
  return 0;
}

/* Unset interface's flag. */
static int
if_unset_flags (struct interface *ifp, uint64_t flags)
{
  int ret;
  struct ifreq ifreq;

  memset (&ifreq, 0, sizeof(struct ifreq));
  ifreq_set_name (&ifreq, ifp);

  ifreq.ifr_flags = ifp->flags;
  ifreq.ifr_flags &= ~flags;

  ret = if_ioctl (SIOCSIFFLAGS, (caddr_t) &ifreq);

  if (ret < 0)
    {
      zlog_info (ZLOG_PAL, "can't unset interface flags");
      return ret;
    }
  return 0;
}


#ifdef HAVE_NETLINK
/* Interface address setting via netlink interface. */
static int
if_set_prefix (struct interface *ifp, struct connected *ifc)
{
  return kernel_address_add_ipv4 (ifp, ifc);
}

/* Interface address is removed using netlink interface. */
static int
if_unset_prefix (struct interface *ifp, struct connected *ifc)
{
  return kernel_address_delete_ipv4 (ifp, ifc);
}
#else /* ! HAVE_NETLINK */
#ifdef HAVE_STRUCT_IFALIASREQ
/* Set up interface's IP address, netmask (and broadcas? ).  *BSD may
   has ifaliasreq structure.  */
static int
if_set_prefix (struct interface *ifp, struct connected *ifc)
{
  int ret;
  struct ifaliasreq addreq;
  struct sockaddr_in addr;
  struct sockaddr_in mask;
  struct prefix_ipv4 *p;

  p = (struct prefix_ipv4 *) ifc->address;

  memset (&addreq, 0, sizeof addreq);
  strncpy ((char *)&addreq.ifra_name, ifp->name, sizeof addreq.ifra_name);

  memset (&addr, 0, sizeof (struct sockaddr_in));
  addr.sin_addr = p->prefix;
  addr.sin_family = p->family;
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
  addr.sin_len = sizeof (struct sockaddr_in);
#endif
  memcpy (&addreq.ifra_addr, &addr, sizeof (struct sockaddr_in));

  memset (&mask, 0, sizeof (struct sockaddr_in));
  masklen2ip (p->prefixlen, &mask.sin_addr);
  mask.sin_family = p->family;
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
  mask.sin_len = sizeof (struct sockaddr_in);
#endif
  memcpy (&addreq.ifra_mask, &mask, sizeof (struct sockaddr_in));
  
  ret = if_ioctl (SIOCAIFADDR, (caddr_t) &addreq);
  if (ret < 0)
    return ret;
  return 0;
}

/* Set up interface's IP address, netmask (and broadcas? ).  *BSD may
   has ifaliasreq structure.  */
static int
if_unset_prefix (struct interface *ifp, struct connected *ifc)
{
  int ret;
  struct ifaliasreq addreq;
  struct sockaddr_in addr;
  struct sockaddr_in mask;
  struct prefix_ipv4 *p;

  p = (struct prefix_ipv4 *)ifc->address;

  memset (&addreq, 0, sizeof addreq);
  strncpy ((char *)&addreq.ifra_name, ifp->name, sizeof addreq.ifra_name);

  memset (&addr, 0, sizeof (struct sockaddr_in));
  addr.sin_addr = p->prefix;
  addr.sin_family = p->family;
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
  addr.sin_len = sizeof (struct sockaddr_in);
#endif
  memcpy (&addreq.ifra_addr, &addr, sizeof (struct sockaddr_in));

  memset (&mask, 0, sizeof (struct sockaddr_in));
  masklen2ip (p->prefixlen, &mask.sin_addr);
  mask.sin_family = p->family;
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
  mask.sin_len = sizeof (struct sockaddr_in);
#endif
  memcpy (&addreq.ifra_mask, &mask, sizeof (struct sockaddr_in));
  
  ret = if_ioctl (SIOCDIFADDR, (caddr_t) &addreq);
  if (ret < 0)
    return ret;
  return 0;
}
#else
/* Set up interface's address, netmask (and broadcas? ).  Linux or
   Solaris uses ifname:number semantics to set IP address aliases. */
static int
if_set_prefix (struct interface *ifp, struct connected *ifc)
{
  int ret;
  struct ifreq ifreq;
  struct sockaddr_in addr;
  struct sockaddr_in broad;
  struct sockaddr_in mask;
  struct prefix_ipv4 ifaddr;
  struct prefix_ipv4 *p;

  p = (struct prefix_ipv4 *) ifc->address;

  ifaddr = *p;

  ifreq_set_name (&ifreq, ifp);

  addr.sin_addr = p->prefix;
  addr.sin_family = p->family;
  memcpy (&ifreq.ifr_addr, &addr, sizeof (struct sockaddr_in));
  ret = if_ioctl (SIOCSIFADDR, (caddr_t) &ifreq);
  if (ret < 0)
    return ret;
  
  /* We need mask for make broadcast addr. */
  masklen2ip (p->prefixlen, &mask.sin_addr);

  if (if_is_broadcast (ifp))
    {
      apply_mask_ipv4 (&ifaddr);
      addr.sin_addr = ifaddr.prefix;

      broad.sin_addr.s_addr = (addr.sin_addr.s_addr | ~mask.sin_addr.s_addr);
      broad.sin_family = p->family;

      memcpy (&ifreq.ifr_broadaddr, &broad, sizeof (struct sockaddr_in));
      ret = if_ioctl (SIOCSIFBRDADDR, (caddr_t) &ifreq);
      if (ret < 0)
	return ret;
    }

  mask.sin_family = p->family;
#ifdef SUNOS_5
  memcpy (&mask, &ifreq.ifr_addr, sizeof (mask));
#else
  memcpy (&ifreq.ifr_netmask, &mask, sizeof (struct sockaddr_in));
#endif /* SUNOS5 */
  ret = if_ioctl (SIOCSIFNETMASK, (caddr_t) &ifreq);
  if (ret < 0)
    return ret;

  return 0;
}

/* Set up interface's address, netmask (and broadcas? ).  Linux or
   Solaris uses ifname:number semantics to set IP address aliases. */
static int
if_unset_prefix (struct interface *ifp, struct connected *ifc)
{
  int ret;
  struct ifreq ifreq;
  struct sockaddr_in addr;
  struct prefix_ipv4 *p;

  p = (struct prefix_ipv4 *) ifc->address;

  ifreq_set_name (&ifreq, ifp);

  memset (&addr, 0, sizeof (struct sockaddr_in));
  addr.sin_family = p->family;
  memcpy (&ifreq.ifr_addr, &addr, sizeof (struct sockaddr_in));
  ret = if_ioctl (SIOCSIFADDR, (caddr_t) &ifreq);
  if (ret < 0)
    return ret;

  return 0;
}
#endif /* HAVE_STRUCT_IFALIASREQ */
#endif /* HAVE_NETLINK */

static int
if_set_dst_prefix (struct interface *ifp, struct connected *ifc)
{
  int ret;
  struct ifreq ifreq;
  struct sockaddr_in addr;
  struct prefix_ipv4 *p;

  p = (struct prefix_ipv4 *) ifc->destination;

  ifreq_set_name (&ifreq, ifp);

  addr.sin_addr = p->prefix;
  addr.sin_family = p->family;
  memcpy (&ifreq.ifr_addr, &addr, sizeof (struct sockaddr_in));
  ret = if_ioctl (SIOCSIFDSTADDR, (caddr_t) &ifreq);
  if (ret < 0)
    return ret;
  return 0;
}

/* Set up interface's address, netmask (and broadcas? ).  Linux or
   Solaris uses ifname:number semantics to set IP address aliases. */
static int
if_unset_dst_prefix (struct interface *ifp, struct connected *ifc)
{
  int ret;
  struct ifreq ifreq;
  struct sockaddr_in addr;
  struct prefix_ipv4 *p;

  p = (struct prefix_ipv4 *) ifc->destination;

  ifreq_set_name (&ifreq, ifp);

  memset (&addr, 0, sizeof (struct sockaddr_in));
  addr.sin_family = p->family;
  memcpy (&ifreq.ifr_addr, &addr, sizeof (struct sockaddr_in));
  ret = if_ioctl (SIOCSIFDSTADDR, (caddr_t) &ifreq);
  if (ret < 0)
    return ret;

  return 0;
}


#ifdef HAVE_IPV6

#ifdef LINUX_IPV6
#ifndef _LINUX_IN6_H
/* linux/include/net/ipv6.h */
struct in6_ifreq 
{
  struct in6_addr ifr6_addr;
  u_int32_t ifr6_prefixlen;
  int ifr6_ifindex;
};
#endif /* _LINUX_IN6_H */

/* Interface's address add/delete functions. */
static int
if_prefix_add_ipv6 (struct interface *ifp, struct connected *ifc, int sec)
{
  int ret;
  struct prefix_ipv6 *p;
  struct in6_ifreq ifreq;

  p = (struct prefix_ipv6 *) ifc->address;

  memset (&ifreq, 0, sizeof (struct in6_ifreq));

  memcpy (&ifreq.ifr6_addr, &p->prefix, sizeof (struct in6_addr));
  ifreq.ifr6_ifindex = ifp->ifindex;
  ifreq.ifr6_prefixlen = p->prefixlen;

  ret = if_ioctl_ipv6 (SIOCSIFADDR, (caddr_t) &ifreq);

  return ret;
}

static int
if_prefix_delete_ipv6 (struct interface *ifp, struct connected *ifc, int sec)
{
  int ret;
  struct prefix_ipv6 *p;
  struct in6_ifreq ifreq;

  p = (struct prefix_ipv6 *) ifc->address;

  memset (&ifreq, 0, sizeof (struct in6_ifreq));

  memcpy (&ifreq.ifr6_addr, &p->prefix, sizeof (struct in6_addr));
  ifreq.ifr6_ifindex = ifp->ifindex;
  ifreq.ifr6_prefixlen = p->prefixlen;

  ret = if_ioctl_ipv6 (SIOCDIFADDR, (caddr_t) &ifreq);

  return ret;
}
#else /* LINUX_IPV6 */
#ifdef HAVE_STRUCT_IN6_ALIASREQ
#ifndef ND6_INFINITE_LIFETIME
#define ND6_INFINITE_LIFETIME 0xffffffffL
#endif /* ND6_INFINITE_LIFETIME */
static int
if_prefix_add_ipv6 (struct interface *ifp, struct connected *ifc)
{
  int ret;
  struct in6_aliasreq addreq;
  struct sockaddr_in6 addr;
  struct sockaddr_in6 mask;
  struct prefix_ipv6 *p;

  p = (struct prefix_ipv6 * ) ifc->address;

  memset (&addreq, 0, sizeof addreq);
  strncpy ((char *)&addreq.ifra_name, ifp->name, sizeof addreq.ifra_name);

  memset (&addr, 0, sizeof (struct sockaddr_in6));
  addr.sin6_addr = p->prefix;
  addr.sin6_family = p->family;
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
  addr.sin6_len = sizeof (struct sockaddr_in6);
#endif
  memcpy (&addreq.ifra_addr, &addr, sizeof (struct sockaddr_in6));

  memset (&mask, 0, sizeof (struct sockaddr_in6));
  masklen2ip6 (p->prefixlen, &mask.sin6_addr);
  mask.sin6_family = p->family;
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
  mask.sin6_len = sizeof (struct sockaddr_in6);
#endif
  memcpy (&addreq.ifra_prefixmask, &mask, sizeof (struct sockaddr_in6));

  addreq.ifra_lifetime.ia6t_vltime = 0xffffffff;
  addreq.ifra_lifetime.ia6t_pltime = 0xffffffff;
  
#ifdef HAVE_STRUCT_IF6_ALIASREQ_IFRA_LIFETIME 
  addreq.ifra_lifetime.ia6t_pltime = ND6_INFINITE_LIFETIME; 
  addreq.ifra_lifetime.ia6t_vltime = ND6_INFINITE_LIFETIME; 
#endif

  ret = if_ioctl_ipv6 (SIOCAIFADDR_IN6, (caddr_t) &addreq);
  if (ret < 0)
    return ret;
  return 0;
}

static int
if_prefix_delete_ipv6 (struct interface *ifp, struct connected *ifc)
{
  int ret;
  struct in6_aliasreq addreq;
  struct sockaddr_in6 addr;
  struct sockaddr_in6 mask;
  struct prefix_ipv6 *p;

  p = (struct prefix_ipv6 *) ifc->address;

  memset (&addreq, 0, sizeof addreq);
  strncpy ((char *)&addreq.ifra_name, ifp->name, sizeof addreq.ifra_name);

  memset (&addr, 0, sizeof (struct sockaddr_in6));
  addr.sin6_addr = p->prefix;
  addr.sin6_family = p->family;
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
  addr.sin6_len = sizeof (struct sockaddr_in6);
#endif
  memcpy (&addreq.ifra_addr, &addr, sizeof (struct sockaddr_in6));

  memset (&mask, 0, sizeof (struct sockaddr_in6));
  masklen2ip6 (p->prefixlen, &mask.sin6_addr);
  mask.sin6_family = p->family;
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
  mask.sin6_len = sizeof (struct sockaddr_in6);
#endif
  memcpy (&addreq.ifra_prefixmask, &mask, sizeof (struct sockaddr_in6));

#ifdef HAVE_STRUCT_IF6_ALIASREQ_IFRA_LIFETIME
  addreq.ifra_lifetime.ia6t_pltime = ND6_INFINITE_LIFETIME; 
  addreq.ifra_lifetime.ia6t_vltime = ND6_INFINITE_LIFETIME; 
#endif

  ret = if_ioctl_ipv6 (SIOCDIFADDR_IN6, (caddr_t) &addreq);
  if (ret < 0)
    return ret;
  return 0;
}
#else
static int
if_prefix_add_ipv6 (struct interface *ifp, struct connected *ifc)
{
  return 0;
}

static int
if_prefix_delete_ipv6 (struct interface *ifp, struct connected *ifc)
{
  return 0;
}
#endif /* HAVE_STRUCT_IN6_ALIASREQ */

#endif /* LINUX_IPV6 */

#endif /* HAVE_IPV6 */




static int if_set_mac (struct interface *ifp, unsigned char *mac, int len)
{
  int ret;
  struct ifreq ifreq;

  memset (&ifreq, 0, sizeof(struct ifreq));
  ifreq_set_name (&ifreq, ifp);
  ifreq.ifr_hwaddr.sa_family = ARPHRD_ETHER;
  memcpy(ifreq.ifr_hwaddr.sa_data, mac, len);
  ret = if_ioctl (SIOCSIFHWADDR, (caddr_t) &ifreq);
  if (ret < 0)
    {
      zlog_info (ZLOG_PAL, "can't unset interface flags");
      return ret;
    }
  return 0;
}

static int if_set_up(struct interface *ifp)
{
	return if_set_flags(ifp, IFF_UP|IFF_RUNNING);
}

static int if_set_down(struct interface *ifp)
{
	return if_unset_flags(ifp, IFF_UP|IFF_RUNNING);
}

static int if_update_flags(struct interface *ifp, int flag)
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
		 _PATH_PROC_NET_DEV, safe_strerror (errno));
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

static int _ipkernel_create(struct interface *ifp)
{
	int ret = -1;
	switch (ifp->if_type)
	{
	case IF_SERIAL:
	case IF_ETHERNET:
	case IF_GIGABT_ETHERNET:
	case IF_VLAN:
		{
			nsm_veth_t * veth = nsm_veth_get(ifp);
			if (veth)
			{
				ret = _ipkernel_linux_create(veth);
			}
		}
		break;
	case IF_TUNNEL:
		{
			nsm_tunnel_t * tunnel = nsm_tunnel_get(ifp);
			if (tunnel)
			{
				ret = _ipkernel_tunnel_create(tunnel);
			}
		}
		break;

	case IF_LAG:
		ret = _ipkernel_bond_create(ifp);
		break;
	case IF_LOOPBACK:
		break;

	case IF_BRIGDE:
		{
			nsm_bridge_t * bridge = nsm_bridge_get(ifp);
			if (bridge)
			{
				ret = _ipkernel_bridge_create(bridge);
			}
		}
		break;
	default:
		break;
	}
	return ret;
}

static int _ipkernel_destroy(struct interface *ifp)
{
	int ret = -1;
	switch (ifp->if_type)
	{
	case IF_SERIAL:
	case IF_ETHERNET:
	case IF_GIGABT_ETHERNET:
	case IF_VLAN:
		{
			nsm_veth_t * veth = nsm_veth_get(ifp);
			if (veth)
			{
				ret = _ipkernel_linux_destroy(veth);
			}
		}
		break;
	case IF_TUNNEL:
		{
			nsm_tunnel_t * tunnel = nsm_tunnel_get(ifp);
			if (tunnel)
			{
				ret = _ipkernel_tunnel_delete(tunnel);
			}
		}
		break;

	case IF_LAG:
		ret = _ipkernel_bond_delete(ifp);
		break;
	case IF_LOOPBACK:
		break;
	case IF_BRIGDE:
		{
			nsm_bridge_t * bridge = nsm_bridge_get(ifp);
			if (bridge)
			{
				ret = _ipkernel_bridge_delete(bridge);
			}
		}
		break;
	default:
		break;
	}
	if(ret == 0)
	{
	    if(pal_interface_up (ifp) != OK)
		{
			zlog_err(ZLOG_PAL, "Unable to set L3 interface up %s(%s).",
					ifp->name, safe_strerror(errno));
			return ERROR;
		}

		if(pal_interface_refresh_flag(ifp) != OK)
		{
			zlog_err(ZLOG_PAL, "Unable to get L3 interface  flags %s(%s).",
					ifp->name, safe_strerror(errno));
			return ERROR;
		}
	    if(pal_interface_get_lladdr(ifp) != OK)
		{
			zlog_err(ZLOG_PAL, "Unable to get L3 interface  mac %s(%s).",
					ifp->name, safe_strerror(errno));
			return ERROR;
		}
	}
	return ret;
}

static int _ipkernel_change(struct interface *ifp)
{
	int ret = -1;
	switch (ifp->if_type)
	{
	case IF_SERIAL:
	case IF_ETHERNET:
	case IF_GIGABT_ETHERNET:
	case IF_VLAN:
		break;
	case IF_TUNNEL:
		{
			nsm_tunnel_t * tunnel = nsm_tunnel_get(ifp);
			if (tunnel)
			{
				ret = _ipkernel_tunnel_change(tunnel);
			}
		}
		break;
	case IF_LAG:
		break;
	case IF_LOOPBACK:
		break;
	case IF_BRIGDE:
		break;
	default:
		break;
	}
	if(ret == 0)
	{
	    if(pal_interface_up (ifp) != OK)
		{
			zlog_err(ZLOG_PAL, "Unable to set L3 interface up %s(%s).",
					ifp->name, safe_strerror(errno));
			return ERROR;
		}

		if(pal_interface_refresh_flag(ifp) != OK)
		{
			zlog_err(ZLOG_PAL, "Unable to get L3 interface  flags %s(%s).",
					ifp->name, safe_strerror(errno));
			return ERROR;
		}
	    if(pal_interface_get_lladdr(ifp) != OK)
		{
			zlog_err(ZLOG_PAL, "Unable to get L3 interface  mac %s(%s).",
					ifp->name, safe_strerror(errno));
			return ERROR;
		}
	}
	return ret;
}

static int _ipkernel_set_vlan(struct interface *ifp, int vlan)
{
	if((if_is_ethernet(ifp) && IF_ID_GET(ifp->ifindex)))
	{
		nsm_veth_t * veth = nsm_veth_get(ifp);
		if (veth)
		{
			if(_ipkernel_linux_change(veth, vlan) == 0)
			{
				//if(ret == 0)
				{
				    if(pal_interface_up (ifp) != OK)
					{
						zlog_err(ZLOG_PAL, "Unable to set L3 interface up %s(%s).",
								ifp->name, safe_strerror(errno));
						return ERROR;
					}

					if(pal_interface_refresh_flag(ifp) != OK)
					{
						zlog_err(ZLOG_PAL, "Unable to get L3 interface  flags %s(%s).",
								ifp->name, safe_strerror(errno));
						return ERROR;
					}
				    if(pal_interface_get_lladdr(ifp) != OK)
					{
						zlog_err(ZLOG_PAL, "Unable to get L3 interface  mac %s(%s).",
								ifp->name, safe_strerror(errno));
						return ERROR;
					}
				}
			}
		}
		return ERROR;
	}
	return OK;
}

int ip_ifp_stack_init()
{
	//interface
	pal_stack.ip_stack_up = if_set_up;
	pal_stack.ip_stack_down = if_set_down;
	pal_stack.ip_stack_update_flag = if_update_flags;
	pal_stack.ip_stack_refresh_flag = if_get_flags;
	pal_stack.ip_stack_ifindex = if_nametoindex;//if_get_ifindex;
	pal_stack.ip_stack_set_vr = NULL;
	pal_stack.ip_stack_set_mtu = if_set_mtu;
	pal_stack.ip_stack_set_lladdr = if_set_mac;
	pal_stack.ip_stack_get_lladdr = if_get_hwaddr;
	pal_stack.ip_stack_set_metric = if_set_metric;

	pal_stack.ip_stack_create = _ipkernel_create;
	pal_stack.ip_stack_destroy = _ipkernel_destroy;
	pal_stack.ip_stack_change = _ipkernel_change;
	pal_stack.ip_stack_set_vlan = _ipkernel_set_vlan;
	pal_stack.ip_stack_set_vlanpri = NULL;
	pal_stack.ip_stack_promisc = NULL;
	//ip address
	pal_stack.ip_stack_dhcp = NULL;
	pal_stack.ip_stack_ipv4_dstaddr_add = if_set_dst_prefix;
	pal_stack.ip_stack_ipv4_dstaddr_del = if_unset_dst_prefix;
	pal_stack.ip_stack_ipv4_replace = NULL;
	pal_stack.ip_stack_ipv4_add = if_set_prefix;
	pal_stack.ip_stack_ipv4_delete = if_unset_prefix;

#ifdef HAVE_IPV6
	pal_stack.ip_stack_ipv6_add = if_prefix_add_ipv6;
	pal_stack.ip_stack_ipv6_delete = if_prefix_delete_ipv6;
#endif
	return OK;
}
