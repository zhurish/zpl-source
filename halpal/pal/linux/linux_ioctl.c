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

#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"
#include "thread.h"
#include "if.h"
#include "vty.h"
#include "command.h"
#include "prefix.h"
#include "nsm_include.h"
#include "hal_include.h"
#include "linux_driver.h"
#include "pal_include.h"

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


#ifdef HAVE_BSD_LINK_DETECT
#include <net/if_media.h>
#endif /* HAVE_BSD_LINK_DETECT*/

#include "pal_include.h"

/* clear and set interface name string */
void linux_ioctl_ifreq_set_name(struct ipstack_ifreq *ipstack_ifreq, struct interface *ifp)
{
  strncpy(ipstack_ifreq->ifr_name, ifp->ker_name, IFNAMSIZ);
}

/* call ipstack_ioctl system call */
int linux_ioctl_if_ioctl(zpl_uint32 request, caddr_t buffer)
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

#ifdef ZPL_BUILD_IPV6
int linux_ioctl_if_ioctl_ipv6(zpl_uint32 request, caddr_t buffer)
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
#endif /* ZPL_BUILD_IPV6 */


/*
 * get interface metric
 *   -- if value is not avaliable set -1
 */
static int
if_get_metric(struct interface *ifp)
{
#ifdef IPSTACK_SIOCGIFMETRIC
  struct ipstack_ifreq ipstack_ifreq;

  linux_ioctl_ifreq_set_name(&ipstack_ifreq, ifp);

  if (linux_ioctl_if_ioctl(IPSTACK_SIOCGIFMETRIC, (caddr_t)&ipstack_ifreq) < 0)
    return -1;
  ifp->metric = ipstack_ifreq.ifr_metric;
  if (ifp->metric == 0)
    ifp->metric = 1;
#else  /* IPSTACK_SIOCGIFMETRIC */
  ifp->metric = -1;
#endif /* IPSTACK_SIOCGIFMETRIC */
  return 0;
}

int linux_ioctl_if_set_metric(struct interface *ifp, zpl_uint32 metric)
{
#ifdef IPSTACK_SIOCGIFMETRIC
  struct ipstack_ifreq ipstack_ifreq;
  linux_ioctl_ifreq_set_name(&ipstack_ifreq, ifp);
  ipstack_ifreq.ifr_metric = metric;
  if (linux_ioctl_if_ioctl(IPSTACK_SIOCGIFMETRIC, (caddr_t)&ipstack_ifreq) < 0)
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

  linux_ioctl_ifreq_set_name(&ipstack_ifreq, ifp);

#if defined(IPSTACK_SIOCGIFMTU)
  if (linux_ioctl_if_ioctl(IPSTACK_SIOCGIFMTU, (caddr_t)&ipstack_ifreq) < 0)
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
linux_ioctl_if_set_mtu(struct interface *ifp, zpl_uint32 mtu)
{
  struct ipstack_ifreq ipstack_ifreq;

  linux_ioctl_ifreq_set_name(&ipstack_ifreq, ifp);

  ipstack_ifreq.ifr_mtu = mtu;

#if defined(IPSTACK_SIOCSIFMTU)
  if (linux_ioctl_if_ioctl(IPSTACK_SIOCSIFMTU, (caddr_t)&ipstack_ifreq) < 0)
  {
    zlog_info(MODULE_PAL, "Can't lookup mtu by ipstack_ioctl(IPSTACK_SIOCGIFMTU)");
    ifp->mtu6 = ifp->mtu = -1;
    return -1;
  }
#endif
  return 0;
}

int
linux_ioctl_if_get_hwaddr(struct interface *ifp)
{
  struct ipstack_ifreq ipstack_ifreq;

  linux_ioctl_ifreq_set_name(&ipstack_ifreq, ifp);

#if defined(IPSTACK_SIOCGIFHWADDR)
  if (linux_ioctl_if_ioctl(IPSTACK_SIOCGIFHWADDR, (caddr_t)&ipstack_ifreq) < 0)
  {
    zlog_info(MODULE_PAL, "Can't lookup MAC by ipstack_ioctl(IPSTACK_SIOCGIFHWADDR)");
    return -1;
  }
#endif

  memcpy(ifp->hw_addr, ipstack_ifreq.ifr_hwaddr.sa_data, IFHWADDRLEN);
  ifp->hw_addr_len = IFHWADDRLEN;
  return 0;
}

/* get interface flags */
int
linux_ioctl_if_get_flags(struct interface *ifp)
{
  int ret;
  struct ipstack_ifreq ipstack_ifreq;
  linux_ioctl_ifreq_set_name(&ipstack_ifreq, ifp);

  if (ifp->ker_ifindex == 0)
    ifp->ker_ifindex = if_nametoindex(ifp->ker_name);
  // ifp->ker_ifindex = if_get_ifindex(ifp->ker_name);
  if_get_mtu(ifp);
  if_get_metric(ifp);
  //  linux_ioctl_if_get_hwaddr(ifp);
  ret = linux_ioctl_if_ioctl(IPSTACK_SIOCGIFFLAGS, (caddr_t)&ipstack_ifreq);
  if (ret < 0)
  {
    zlog_err(MODULE_PAL, "linux_ioctl_if_ioctl(IPSTACK_SIOCGIFFLAGS) failed: %s", ipstack_strerror(ipstack_errno));
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
  linux_ioctl_ifreq_set_name(&ipstack_ifreq, ifp);

  ipstack_ifreq.ifr_flags = ifp->flags;
  ipstack_ifreq.ifr_flags |= flags;

  ret = linux_ioctl_if_ioctl(IPSTACK_SIOCSIFFLAGS, (caddr_t)&ipstack_ifreq);

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
  linux_ioctl_ifreq_set_name(&ipstack_ifreq, ifp);

  ipstack_ifreq.ifr_flags = ifp->flags;
  ipstack_ifreq.ifr_flags &= ~flags;

  ret = linux_ioctl_if_ioctl(IPSTACK_SIOCSIFFLAGS, (caddr_t)&ipstack_ifreq);

  if (ret < 0)
  {
    zlog_info(MODULE_PAL, "can't unset interface flags");
    return ret;
  }
  return 0;
}



int linux_ioctl_if_set_mac(struct interface *ifp, zpl_uint8 *mac, zpl_uint32 len)
{
  int ret;
  struct ipstack_ifreq ipstack_ifreq;

  memset(&ipstack_ifreq, 0, sizeof(struct ipstack_ifreq));
  linux_ioctl_ifreq_set_name(&ipstack_ifreq, ifp);
  ipstack_ifreq.ifr_hwaddr.sa_family = IPSTACK_ARPHRD_ETHER;
  memcpy(ipstack_ifreq.ifr_hwaddr.sa_data, mac, len);
  ret = linux_ioctl_if_ioctl(SIOCSIFHWADDR, (caddr_t)&ipstack_ifreq);
  if (ret < 0)
  {
    zlog_info(MODULE_PAL, "can't unset interface flags");
    return ret;
  }
  return 0;
}

int linux_ioctl_if_set_up(struct interface *ifp)
{
  return if_set_flags(ifp, IPSTACK_IFF_UP | IPSTACK_IFF_RUNNING);
}

int linux_ioctl_if_set_down(struct interface *ifp)
{
  return if_unset_flags(ifp, IPSTACK_IFF_UP | IPSTACK_IFF_RUNNING);
}

int linux_ioctl_if_update_flags(struct interface *ifp, uint64_t flag)
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



