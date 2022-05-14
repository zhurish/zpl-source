/* setsockopt functions
 * Copyright (C) 1999 Kunihiro Ishiguro
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
#include "log.h"
#include "sockopt.h"
#include "sockunion.h"
#include "if.h"

int
setsockopt_so_recvbuf (zpl_socket_t sock, zpl_uint32 size)
{
  int ret;
  
  if ( (ret = ipstack_setsockopt (sock, IPSTACK_SOL_SOCKET, IPSTACK_SO_RCVBUF, (zpl_char *)
                          &size, sizeof (zpl_uint32))) < 0)
    zlog_err (MODULE_DEFAULT, "fd %d: can't setsockopt IPSTACK_SO_RCVBUF to %d: %s",
	      sock,size,ipstack_strerror(ipstack_errno));

  return ret;
}

int
setsockopt_so_sendbuf (const zpl_socket_t sock, zpl_uint32 size)
{
  int ret = ipstack_setsockopt (sock, IPSTACK_SOL_SOCKET, IPSTACK_SO_SNDBUF,
    (zpl_char *)&size, sizeof (zpl_uint32));
  
  if (ret < 0)
    zlog_err (MODULE_DEFAULT, "fd %d: can't setsockopt IPSTACK_SO_SNDBUF to %d: %s",
      sock, size, ipstack_strerror (ipstack_errno));

  return ret;
}

int
getsockopt_so_sendbuf (const zpl_socket_t sock)
{
  zpl_uint32 optval;
  socklen_t optlen = sizeof (optval);
  int ret = ipstack_getsockopt (sock, IPSTACK_SOL_SOCKET, IPSTACK_SO_SNDBUF,
    (zpl_char *)&optval, &optlen);
  if (ret < 0)
  {
    zlog_err (MODULE_DEFAULT, "fd %d: can't getsockopt IPSTACK_SO_SNDBUF: %d (%s)",
      sock, ipstack_errno, ipstack_strerror (ipstack_errno));
    return ret;
  }
  return optval;
}

static void *
getsockopt_cmsg_data (struct ipstack_msghdr *msgh, zpl_uint32 level, zpl_uint32 type)
{
  struct ipstack_cmsghdr *cmsg;
  void *ptr = NULL;
  
  for (cmsg = IPSTACK_CMSG_FIRSTHDR(msgh);
       cmsg != NULL;
       cmsg = IPSTACK_CMSG_NXTHDR(msgh, cmsg))
    if (cmsg->cmsg_level == level && cmsg->cmsg_type)
      return (ptr = IPSTACK_CMSG_DATA(cmsg));

  return NULL;
}

#ifdef ZPL_BUILD_IPV6
/* Set IPv6 packet info to the socket. */
int
setsockopt_ipv6_pktinfo (zpl_socket_t sock, zpl_uint32 val)
{
  int ret;
    
#ifdef IPSTACK_IPV6_RECVPKTINFO		/*2292bis-01*/
  ret = ipstack_setsockopt(sock, IPSTACK_IPPROTO_IPV6, IPSTACK_IPV6_RECVPKTINFO, &val, sizeof(val));
  if (ret < 0)
    zlog_warn (MODULE_DEFAULT, "can't setsockopt IPSTACK_IPV6_RECVPKTINFO : %s", ipstack_strerror (ipstack_errno));
#else	/*RFC2292*/
  ret = ipstack_setsockopt(sock, IPSTACK_IPPROTO_IPV6, IPSTACK_IPV6_PKTINFO, &val, sizeof(val));
  if (ret < 0)
    zlog_warn (MODULE_DEFAULT, "can't setsockopt IPSTACK_IPV6_PKTINFO : %s", ipstack_strerror (ipstack_errno));
#endif /* INIA_IPV6 */
  return ret;
}

/* Set multicast hops val to the socket. */
int
setsockopt_ipv6_checksum (zpl_socket_t sock, zpl_uint32 val)
{
  int ret;

#ifdef GNU_LINUX
  ret = ipstack_setsockopt(sock, IPSTACK_IPPROTO_RAW, IPSTACK_IPV6_CHECKSUM, &val, sizeof(val));
#else
  ret = ipstack_setsockopt(sock, IPSTACK_IPPROTO_IPV6, IPSTACK_IPV6_CHECKSUM, &val, sizeof(val));
#endif /* GNU_LINUX */
  if (ret < 0)
    zlog_warn (MODULE_DEFAULT, "can't setsockopt IPSTACK_IPV6_CHECKSUM");
  return ret;
}

/* Set multicast hops val to the socket. */
int
setsockopt_ipv6_multicast_hops (zpl_socket_t sock, zpl_uint32 val)
{
  int ret;

  ret = ipstack_setsockopt(sock, IPSTACK_IPPROTO_IPV6, IPSTACK_IPV6_MULTICAST_HOPS, &val, sizeof(val));
  if (ret < 0)
    zlog_warn (MODULE_DEFAULT, "can't setsockopt IPSTACK_IPV6_MULTICAST_HOPS");
  return ret;
}

/* Set multicast hops val to the socket. */
int
setsockopt_ipv6_unicast_hops (zpl_socket_t sock, zpl_uint32 val)
{
  int ret;

  ret = ipstack_setsockopt(sock, IPSTACK_IPPROTO_IPV6, IPSTACK_IPV6_UNICAST_HOPS, &val, sizeof(val));
  if (ret < 0)
    zlog_warn (MODULE_DEFAULT, "can't setsockopt IPSTACK_IPV6_UNICAST_HOPS");
  return ret;
}

int
setsockopt_ipv6_hoplimit (zpl_socket_t sock, zpl_uint32 val)
{
  int ret;

#ifdef IPSTACK_IPV6_RECVHOPLIMIT	/*2292bis-01*/
  ret = ipstack_setsockopt (sock, IPSTACK_IPPROTO_IPV6, IPSTACK_IPV6_RECVHOPLIMIT, &val, sizeof(val));
  if (ret < 0)
    zlog_warn (MODULE_DEFAULT, "can't setsockopt IPSTACK_IPV6_RECVHOPLIMIT");
#else	/*RFC2292*/
  ret = ipstack_setsockopt (sock, IPSTACK_IPPROTO_IPV6, IPSTACK_IPV6_HOPLIMIT, &val, sizeof(val));
  if (ret < 0)
    zlog_warn (MODULE_DEFAULT, "can't setsockopt IPSTACK_IPV6_HOPLIMIT");
#endif
  return ret;
}

/* Set multicast loop zero to the socket. */
int
setsockopt_ipv6_multicast_loop (zpl_socket_t sock, zpl_uint32 val)
{
  int ret;
    
  ret = ipstack_setsockopt (sock, IPSTACK_IPPROTO_IPV6, IPSTACK_IPV6_MULTICAST_LOOP, &val,
		    sizeof (val));
  if (ret < 0)
    zlog_warn (MODULE_DEFAULT, "can't setsockopt IPSTACK_IPV6_MULTICAST_LOOP");
  return ret;
}

static int
getsockopt_ipv6_ifindex (struct ipstack_msghdr *msgh)
{
  struct ipstack_in6_pktinfo *pktinfo;
  
  pktinfo = getsockopt_cmsg_data (msgh, IPSTACK_IPPROTO_IPV6, IPSTACK_IPV6_PKTINFO);
  
  return ifkernel2ifindex(pktinfo->ipi6_ifindex);
}

int
setsockopt_ipv6_tclass(zpl_socket_t sock, zpl_uint32 tclass)
{
  int ret = 0;

#ifdef IPSTACK_IPV6_TCLASS /* RFC3542 */
  ret = ipstack_setsockopt (sock, IPSTACK_IPPROTO_IPV6, IPSTACK_IPV6_TCLASS, &tclass, sizeof (tclass));
  if (ret < 0)
    zlog_warn (MODULE_DEFAULT, "Can't set IPSTACK_IPV6_TCLASS option for fd %d to %#x: %s",
	       sock, tclass, ipstack_strerror(ipstack_errno));
#endif
  return ret;
}
#endif /* ZPL_BUILD_IPV6 */

/*
 * Process multicast socket options for IPv4 in an OS-dependent manner.
 * Supported options are IP_{ADD,DROP}_MEMBERSHIP.
 *
 * Many operating systems have a limit on the number of groups that
 * can be joined per socket (where each group and local address
 * counts).  This impacts OSPF, which joins groups on each interface
 * using a single socket.  The limit is typically 20, derived from the
 * original BSD multicast implementation.  Some systems have
 * mechanisms for increasing this limit.
 *
 * In many 4.4BSD-derived systems, multicast group operations are not
 * allowed on interfaces that are not UP.  Thus, a previous attempt to
 * leave the group may have failed, leaving it still joined, and we
 * drop/join quietly to recover.  This may not be necessary, but aims to
 * defend against unknown behavior in that we will still return an error
 * if the second join fails.  It is not clear how other systems
 * (e.g. Linux, Solaris) behave when leaving groups on down interfaces,
 * but this behavior should not be harmful if they behave the same way,
 * allow leaves, or implicitly leave all groups joined to down interfaces.
 */
int
setsockopt_ipv4_multicast(zpl_socket_t sock,
			zpl_uint32 optname, 
			zpl_uint32  mcast_addr,
			ifindex_t ifindex)
{
#ifdef HAVE_RFC3678
  struct ipstack_group_req gr;
  struct ipstack_sockaddr_in *si;
  int ret;
  memset (&gr, 0, sizeof(gr));
  si = (struct ipstack_sockaddr_in *)&gr.gr_group;
  ifindex_t k_ifindex = ifindex2ifkernel( ifindex);
  gr.gr_interface = k_ifindex;
  si->sin_family = IPSTACK_AF_INET;
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
  si->sin_len = sizeof(struct ipstack_sockaddr_in);
#endif /* HAVE_STRUCT_SOCKADDR_IN_SIN_LEN */
  si->sin_addr.s_addr = mcast_addr;
  ret = ipstack_setsockopt(sock, IPSTACK_IPPROTO_IP, (optname == IPSTACK_IP_ADD_MEMBERSHIP) ?
    IPSTACK_MCAST_JOIN_GROUP : IPSTACK_MCAST_LEAVE_GROUP, (void *)&gr, sizeof(gr));
  if ((ret < 0) && (optname == IPSTACK_IP_ADD_MEMBERSHIP) && (ipstack_errno == IPSTACK_ERRNO_EADDRINUSE))
    {
	  ipstack_setsockopt(sock, IPSTACK_IPPROTO_IP, IPSTACK_MCAST_LEAVE_GROUP, (void *)&gr, sizeof(gr));
      ret = ipstack_setsockopt(sock, IPSTACK_IPPROTO_IP, IPSTACK_MCAST_JOIN_GROUP, (void *)&gr, sizeof(gr));
    }
  return ret;

#elif defined(HAVE_STRUCT_IP_MREQN_IMR_IFINDEX) && !defined(__FreeBSD__)
  struct ipstack_ip_mreqn mreqn;
  int ret;
  ifindex_t k_ifindex = ifindex2ifkernel( ifindex);
  assert(optname == IPSTACK_IP_ADD_MEMBERSHIP || optname == IPSTACK_IP_DROP_MEMBERSHIP);
  memset (&mreqn, 0, sizeof(mreqn));

  mreqn.imr_multiaddr.s_addr = mcast_addr;
  mreqn.imr_ifindex = k_ifindex;
  
  ret = ipstack_setsockopt(sock, IPSTACK_IPPROTO_IP, optname,
                   (void *)&mreqn, sizeof(mreqn));
  if ((ret < 0) && (optname == IPSTACK_IP_ADD_MEMBERSHIP) && (ipstack_errno == IPSTACK_ERRNO_EADDRINUSE))
    {
      /* see above: handle possible problem when interface comes back up */
      zpl_char buf[1][IPSTACK_INET_ADDRSTRLEN];
      zlog_info(MODULE_DEFAULT, "setsockopt_ipv4_multicast attempting to drop and "
                "re-add (fd %d, mcast %s, ifindex %u)",
                sock,
                ipstack_inet_ntop(IPSTACK_AF_INET, &mreqn.imr_multiaddr,
                          buf[0], sizeof(buf[0])), k_ifindex);
      ipstack_setsockopt(sock, IPSTACK_IPPROTO_IP, IPSTACK_IP_DROP_MEMBERSHIP,
                 (void *)&mreqn, sizeof(mreqn));
      ret = ipstack_setsockopt(sock, IPSTACK_IPPROTO_IP, IPSTACK_IP_ADD_MEMBERSHIP,
                       (void *)&mreqn, sizeof(mreqn));
    }
  return ret;

  /* Example defines for another OS, boilerplate off other code in this
     function, AND handle optname as per other sections for consistency !! */
  /* #elif  defined(BOGON_NIX) && EXAMPLE_VERSION_CODE > -100000 */
  /* Add your favourite OS here! */

#elif defined(HAVE_BSD_STRUCT_IP_MREQ_HACK) /* #if OS_TYPE */ 
  /* standard BSD API */

  struct ipstack_in_addr m;
  struct ipstack_ip_mreq mreq;
  int ret;

  assert(optname == IPSTACK_IP_ADD_MEMBERSHIP || optname == IPSTACK_IP_DROP_MEMBERSHIP);
  ifindex_t k_ifindex = ifindex2ifkernel( ifindex);
  m.s_addr = htonl(k_ifindex);

  memset (&mreq, 0, sizeof(mreq));
  mreq.imr_multiaddr.s_addr = mcast_addr;
  mreq.imr_interface = m;
  
  ret = ipstack_setsockopt (sock, IPSTACK_IPPROTO_IP, optname, (void *)&mreq, sizeof(mreq));
  if ((ret < 0) && (optname == IPSTACK_IP_ADD_MEMBERSHIP) && (ipstack_errno == IPSTACK_ERRNO_EADDRINUSE))
    {
      /* see above: handle possible problem when interface comes back up */
      zpl_char buf[1][IPSTACK_INET_ADDRSTRLEN];
      zlog_info(MODULE_DEFAULT, "setsockopt_ipv4_multicast attempting to drop and "
                "re-add (fd %d, mcast %s, ifindex %u)",
                sock,
                ipstack_inet_ntop(IPSTACK_AF_INET, &mreq.imr_multiaddr,
                          buf[0], sizeof(buf[0])), k_ifindex);
      ipstack_setsockopt (sock, IPSTACK_IPPROTO_IP, IPSTACK_IP_DROP_MEMBERSHIP,
                  (void *)&mreq, sizeof(mreq));
      ret = ipstack_setsockopt (sock, IPSTACK_IPPROTO_IP, IPSTACK_IP_ADD_MEMBERSHIP,
                        (void *)&mreq, sizeof(mreq));
    }
  return ret;

#else
  #error "Unsupported multicast API"
#endif /* #if OS_TYPE */

}

/*
 * Set IP_MULTICAST_IF socket option in an OS-dependent manner.
 */
int
setsockopt_ipv4_multicast_if(zpl_socket_t sock, ifindex_t ifindex)
{

#ifdef HAVE_STRUCT_IP_MREQN_IMR_IFINDEX
  struct ipstack_ip_mreqn mreqn;
  memset (&mreqn, 0, sizeof(mreqn));
  ifindex_t k_ifindex = ifindex2ifkernel( ifindex);
  mreqn.imr_ifindex = k_ifindex;
  return ipstack_setsockopt(sock, IPSTACK_IPPROTO_IP, IPSTACK_IP_MULTICAST_IF, (void *)&mreqn, sizeof(mreqn));

  /* Example defines for another OS, boilerplate off other code in this
     function */
  /* #elif  defined(BOGON_NIX) && EXAMPLE_VERSION_CODE > -100000 */
  /* Add your favourite OS here! */
#elif defined(HAVE_BSD_STRUCT_IP_MREQ_HACK)
  struct ipstack_in_addr m;
  ifindex_t k_ifindex = ifindex2ifkernel( ifindex);
  m.s_addr = htonl(k_ifindex);

  return ipstack_setsockopt (sock, IPSTACK_IPPROTO_IP, IPSTACK_IP_MULTICAST_IF, (void *)&m, sizeof(m));
#elif defined(SUNOS_5)
  zpl_char ifname[IF_NAMESIZE];
  struct ipstack_ifaddrs *ifa, *ifap;
  struct ipstack_in_addr ifaddr;

  if (if_indextoname(k_ifindex, ifname) == NULL)
    return -1;

  if (ip_getifaddrs(&ifa) != 0)
    return -1;

  for (ifap = ifa; ifap != NULL; ifap = ifap->ifa_next)
    {
      struct ipstack_sockaddr_in *sa;

      if (strcmp(ifap->ifa_name, ifname) != 0)
        continue;
      if (ifap->ifa_addr->sa_family != IPSTACK_AF_INET)
        continue;
      sa = (struct ipstack_sockaddr_in*)ifap->ifa_addr;
      memcpy(&ifaddr, &sa->sin_addr, sizeof(ifaddr));
      break;
    }

  ip_freeifaddrs(ifa);
  if (!ifap) /* This means we did not find an IP */
    return -1;

  return ipstack_setsockopt(sock, IPSTACK_IPPROTO_IP, IPSTACK_IP_MULTICAST_IF, (void *)&ifaddr, sizeof(ifaddr));
#else
  #error "Unsupported multicast API"
#endif
}
  
static int
setsockopt_ipv4_ifindex (zpl_socket_t sock, ifindex_t val)
{
  int ret;

#if defined (IP_PKTINFO)
  if ((ret = ipstack_setsockopt (sock, IPSTACK_IPPROTO_IP, IPSTACK_IP_PKTINFO, &val, sizeof (val))) < 0)
    zlog_warn (MODULE_DEFAULT, "Can't set IP_PKTINFO option for fd %d to %d: %s",
	       sock,val,ipstack_strerror(ipstack_errno));
#elif defined (IP_RECVIF)
  if ((ret = ipstack_setsockopt (sock, IPSTACK_IPPROTO_IP, IPSTACK_IP_RECVIF, &val, sizeof (val))) < 0)
    zlog_warn (MODULE_DEFAULT, "Can't set IP_RECVIF option for fd %d to %d: %s",
	       sock,val,ipstack_strerror(ipstack_errno));
#else
#warning "Neither IP_PKTINFO nor IP_RECVIF is available."
#warning "Will not be able to receive link info."
#warning "Things might be seriously broken.."
  /* XXX Does this ever happen?  Should there be a zlog_warn message here? */
  ret = -1;
#endif
  return ret;
}

int
setsockopt_ipv4_tos(zpl_socket_t sock, zpl_uint32 tos)
{
  int ret;

  ret = ipstack_setsockopt (sock, IPSTACK_IPPROTO_IP, IPSTACK_IP_TOS, &tos, sizeof (tos));
  if (ret < 0)
    zlog_warn (MODULE_DEFAULT, "Can't set IP_TOS option for fd %d to %#x: %s",
	       sock, tos, ipstack_strerror(ipstack_errno));
  return ret;
}


int
setsockopt_ifindex (zpl_family_t family, zpl_socket_t sock, ifindex_t val)
{
  int ret = -1;
  
  switch (family)
    {
      case IPSTACK_AF_INET:
        ret = setsockopt_ipv4_ifindex (sock, val);
        break;
#ifdef ZPL_BUILD_IPV6
      case IPSTACK_AF_INET6:
        ret = setsockopt_ipv6_pktinfo (sock, val);
        break;
#endif
      default:
        zlog_warn (MODULE_DEFAULT, "setsockopt_ifindex: unknown address family %d", family);
    }
  return ret;
}
  
/*
 * Requires: msgh is not NULL and points to a valid struct ipstack_msghdr, which
 * may or may not have control data about the incoming interface.
 *
 * Returns the interface index (small integer >= 1) if it can be
 * determined, or else 0.
 */
static ifindex_t
getsockopt_ipv4_ifindex (struct ipstack_msghdr *msgh)
{
  /* XXX: initialize to zero?  (Always overwritten, so just cosmetic.) */
  ifindex_t ifindex = -1;

#if defined(IP_PKTINFO)
/* Linux pktinfo based ifindex retrieval */
  struct ipstack_in_pktinfo *pktinfo;
  
  pktinfo = 
    (struct ipstack_in_pktinfo *)getsockopt_cmsg_data (msgh, IPSTACK_IPPROTO_IP, IPSTACK_IP_PKTINFO);
  /* XXX Can pktinfo be NULL?  Clean up post 0.98. */
  if(pktinfo)
	  ifindex = pktinfo->ipi_ifindex;
  else
    ifindex = 0;
#elif defined(IP_RECVIF)

  /* retrieval based on IP_RECVIF */

#ifndef SUNOS_5
  /* BSD systems use a ipstack_sockaddr_dl as the control message payload. */
  struct ipstack_sockaddr_dl *sdl;
#else
  /* SUNOS_5 uses an integer with the index. */
  ifindex_t *ifindex_p;
#endif /* SUNOS_5 */

#ifndef SUNOS_5
  /* BSD */
  sdl = 
    (struct ipstack_sockaddr_dl *)getsockopt_cmsg_data (msgh, IPSTACK_IPPROTO_IP, IPSTACK_IP_RECVIF);
  if (sdl != NULL)
    ifindex = sdl->sdl_index;
  else
    ifindex = 0;
#else
  /*
   * Solaris.  On Solaris 8, IP_RECVIF is defined, but the call to
   * enable it fails with ipstack_errno=99, and the struct ipstack_msghdr has
   * controllen 0.
   */
  ifindex_p = (uint_t *)getsockopt_cmsg_data (msgh, IPSTACK_IPPROTO_IP, IPSTACK_IP_RECVIF); 
  if (ifindex_p != NULL)
    ifindex = *ifindex_p;
  else
    ifindex = 0;
#endif /* SUNOS_5 */

#else
  /*
   * Neither IP_PKTINFO nor IP_RECVIF defined - warn at compile time.
   * XXX Decide if this is a core service, or if daemons have to cope.
   * Since Solaris 8 and OpenBSD seem not to provide it, it seems that
   * daemons have to cope.
   */
#warning "getsockopt_ipv4_ifindex: Neither IP_PKTINFO nor IP_RECVIF defined."
#warning "Some daemons may fail to operate correctly!"
  ifindex = 0;

#endif /* IP_PKTINFO */ 
  //zlog_debug(MODULE_DEFAULT, "kernel %s->%d",ifkernelindex2kernelifname(ifindex),ifindex);
  return ifkernel2ifindex(ifindex);
}

/* return ifindex, 0 if none found */
ifindex_t
getsockopt_ifindex (zpl_family_t family, struct ipstack_msghdr *msgh)
{
  switch (family)
    {
      case IPSTACK_AF_INET:
        return (getsockopt_ipv4_ifindex (msgh));
        break;
#ifdef ZPL_BUILD_IPV6
      case IPSTACK_AF_INET6:
        return (getsockopt_ipv6_ifindex (msgh));
        break;
#endif
      default:
        zlog_warn (MODULE_DEFAULT, "getsockopt_ifindex: unknown address family %d", family);
        return 0;
    }
}

int
setsockopt_ipv4_multicast_loop(zpl_socket_t sock, zpl_uint32 opt)
{
  int ret;
  zpl_uint32 optval = opt;
  ret = ipstack_setsockopt (sock, IPSTACK_IPPROTO_IP, IPSTACK_IP_MULTICAST_LOOP, &optval, sizeof (optval));
  if (ret < 0)
    zlog_warn (MODULE_DEFAULT, "Can't set IP_MULTICAST_LOOP option for fd %d to %#x: %s",
	       sock, opt, ipstack_strerror(ipstack_errno));
  return ret;
}

/* swab iph between order system uses for IP_HDRINCL and host order */
void
sockopt_iphdrincl_swab_htosys (struct ip *iph)
{
  /* BSD and derived take iph in network order, except for 
   * ip_len and ip_off
   */
#ifndef HAVE_IP_HDRINCL_BSD_ORDER
  iph->ip_len = htons(iph->ip_len);
  iph->ip_off = htons(iph->ip_off);
#endif /* HAVE_IP_HDRINCL_BSD_ORDER */

  iph->ip_id = htons(iph->ip_id);
}

void
sockopt_iphdrincl_swab_systoh (struct ip *iph)
{
#ifndef HAVE_IP_HDRINCL_BSD_ORDER
  iph->ip_len = ntohs(iph->ip_len);
  iph->ip_off = ntohs(iph->ip_off);
#endif /* HAVE_IP_HDRINCL_BSD_ORDER */

  iph->ip_id = ntohs(iph->ip_id);
}

int
sockopt_tcp_rtt (zpl_socket_t sock)
{
#ifdef TCP_INFO
  struct tcp_info ti;
  socklen_t len = sizeof(ti);

  if (ipstack_getsockopt (sock, IPSTACK_IPPROTO_TCP, TCP_INFO, &ti, &len) != 0)
    return 0;

  return ti.tcpi_rtt / 1000;
#else
  return 0;
#endif
}
#ifdef ZPL_KERNEL_MODULE
int
sockopt_tcp_signature (zpl_socket_t sock, union sockunion *su, const char *password)
{
#if defined(HAVE_TCP_MD5_LINUX24) && defined(GNU_LINUX)
  /* Support for the old Linux 2.4 TCP-MD5 patch, taken from Hasso Tepper's
   * version of the Quagga patch (based on work by Rick Payne, and Bruce
   * Simpson)
   */
#define TCP_MD5_AUTH 13
#define TCP_MD5_AUTH_ADD 1
#define TCP_MD5_AUTH_DEL 2
  struct tcp_rfc2385_cmd {
    zpl_uint8     command;    /* Command - Add/Delete */
    zpl_uint32    address;    /* IPV4 address associated */
    zpl_uint8     keylen;     /* MD5 Key len (do NOT assume 0 terminated ascii) */
    void         *key;       /* MD5 Key */
  } cmd;
  struct ipstack_in_addr *addr = &su->sin.sin_addr;
  
  cmd.command = (password != NULL ? TCP_MD5_AUTH_ADD : TCP_MD5_AUTH_DEL);
  cmd.address = addr->s_addr;
  cmd.keylen = (password != NULL ? strlen (password) : 0);
  cmd.key = password;
  
  return ipstack_setsockopt (sock, IPSTACK_IPPROTO_TCP, TCP_MD5_AUTH, &cmd, sizeof cmd);
  
#elif HAVE_DECL_TCP_MD5SIG
  int ret;
#ifndef GNU_LINUX
  /*
   * XXX Need to do IPSTACK_PF_KEY operation here to add/remove an SA entry,
   * and add/remove an SP entry for this peer's packet flows also.
   */
  zpl_uint32 md5sig = password && *password ? 1 : 0;
#else
  zpl_uint32 keylen = password ? strlen (password) : 0;
  struct tcp_md5sig md5sig;
  union sockunion *su2, *susock;
  
  /* Figure out whether the socket and the sockunion are the same family..
   * adding IPSTACK_AF_INET to IPSTACK_AF_INET6 needs to be v4 mapped, you'd think..
   */
  if (!(susock = sockunion_getsockname (sock)))
    return -1;
  
  if (susock->sa.sa_family == su->sa.sa_family)
    su2 = su;
  else
    {
      /* oops.. */
      su2 = susock;
      
      if (su2->sa.sa_family == IPSTACK_AF_INET)
        {
          sockunion_free (susock);
          return 0;
        }
      
#ifdef ZPL_BUILD_IPV6
      /* If this does not work, then all users of this sockopt will need to
       * differentiate between IPv4 and IPv6, and keep seperate sockets for
       * each. 
       *
       * Sadly, it doesn't seem to work at present. It's unknown whether
       * this is a bug or not.
       */
      if (su2->sa.sa_family == IPSTACK_AF_INET6
          && su->sa.sa_family == IPSTACK_AF_INET)
        {
           su2->sin6.sin6_family = IPSTACK_AF_INET6;
           /* V4Map the address */
           memset (&su2->sin6.sin6_addr, 0, sizeof (struct ipstack_in6_addr));
           su2->sin6.sin6_addr.s6_addr32[2] = htonl(0xffff);
           memcpy (&su2->sin6.sin6_addr.s6_addr32[3], &su->sin.sin_addr, 4);
        }
#endif
    }
  
  memset (&md5sig, 0, sizeof (md5sig));
  memcpy (&md5sig.tcpm_addr, su2, sizeof (*su2));
  md5sig.tcpm_keylen = keylen;
  if (keylen)
    memcpy (md5sig.tcpm_key, password, keylen);
  sockunion_free (susock);
#endif /* GNU_LINUX */
  if ((ret = ipstack_setsockopt (sock, IPSTACK_IPPROTO_TCP, IPSTACK_TCP_MD5SIG, &md5sig, sizeof md5sig)) < 0)
    {
      /* IPSTACK_ERRNO_ENOENT is harmless.  It is returned when we clear a password for which
	 one was not previously set. */
      if (IPSTACK_ERRNO_ENOENT == ipstack_errno)
	ret = 0;
      else
	zlog_err (MODULE_DEFAULT, "sockopt_tcp_signature: setsockopt(%d): %s",
		  sock, ipstack_strerror(ipstack_errno));
    }
  return ret;
#else /* HAVE_TCP_MD5SIG */
  return -2;
#endif /* !HAVE_TCP_MD5SIG */
}
#endif
