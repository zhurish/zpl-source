/* Router advertisement
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

#ifndef __LIB_SOCKOPT_H
#define __LIB_SOCKOPT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sockunion.h"

extern int setsockopt_so_recvbuf (zpl_socket_t sock, zpl_uint32 size);
extern int setsockopt_so_sendbuf (const zpl_socket_t sock, zpl_uint32 size);
extern int getsockopt_so_sendbuf (const zpl_socket_t sock);

#ifdef ZPL_BUILD_IPV6
extern int setsockopt_ipv6_pktinfo (zpl_socket_t, zpl_uint32);
extern int setsockopt_ipv6_checksum (zpl_socket_t, zpl_uint32);
extern int setsockopt_ipv6_multicast_hops (zpl_socket_t, zpl_uint32);
extern int setsockopt_ipv6_unicast_hops (zpl_socket_t, zpl_uint32);
extern int setsockopt_ipv6_hoplimit (zpl_socket_t, zpl_uint32);
extern int setsockopt_ipv6_multicast_loop (zpl_socket_t, zpl_uint32);
extern int setsockopt_ipv6_tclass (zpl_socket_t, zpl_uint32);
#endif /* ZPL_BUILD_IPV6 */

/*
 * It is OK to reference ipstack_in6_pktinfo here without a protecting #if
 * because this macro will only be used #if ZPL_BUILD_IPV6, and ipstack_in6_pktinfo
 * is not optional for ZPL_BUILD_IPV6.
 */
#define SOPT_SIZE_CMSG_PKTINFO_IPV6() (sizeof (struct ipstack_in6_pktinfo));

/*
 * Size defines for control messages used to get ifindex.  We define
 * values for each method, and define a macro that can be used by code
 * that is unaware of which method is in use.
 * These values are without any alignment needed (see IPSTACK_CMSG_SPACE in RFC3542).
 */
#if defined (IP_PKTINFO)
/* Linux ipstack_in_pktinfo. */
#define SOPT_SIZE_CMSG_PKTINFO_IPV4()  (IPSTACK_CMSG_SPACE(sizeof (struct ipstack_in_pktinfo)))
/* XXX This should perhaps be defined even if IP_PKTINFO is not. */
#define SOPT_SIZE_CMSG_PKTINFO(af) \
  ((af == IPSTACK_AF_INET) ? SOPT_SIZE_CMSG_PKTINFO_IPV4() \
                   : SOPT_SIZE_CMSG_PKTINFO_IPV6()
#endif /* IP_PKTINFO */

#if defined (IP_RECVIF)
/* BSD/Solaris */

#if defined (SUNOS_5)
#define SOPT_SIZE_CMSG_RECVIF_IPV4()  (sizeof (uint_t))
#else
#define SOPT_SIZE_CMSG_RECVIF_IPV4()	(sizeof (struct ipstack_sockaddr_dl))
#endif /* SUNOS_5 */
#endif /* IP_RECVIF */

/* SOPT_SIZE_CMSG_IFINDEX_IPV4 - portable type */
#if defined (SOPT_SIZE_CMSG_PKTINFO)
#define SOPT_SIZE_CMSG_IFINDEX_IPV4() SOPT_SIZE_CMSG_PKTINFO_IPV4()
#elif defined (SOPT_SIZE_CMSG_RECVIF_IPV4)
#define SOPT_SIZE_CMSG_IFINDEX_IPV4() SOPT_SIZE_CMSG_RECVIF_IPV4()
#else /* Nothing available */
#define SOPT_SIZE_CMSG_IFINDEX_IPV4() (sizeof (zpl_char *))
#endif /* SOPT_SIZE_CMSG_IFINDEX_IPV4 */

#define SOPT_SIZE_CMSG_IFINDEX(af) \
  (((af) == IPSTACK_AF_INET) : SOPT_SIZE_CMSG_IFINDEX_IPV4() \
                    ? SOPT_SIZE_CMSG_PKTINFO_IPV6())

extern int setsockopt_ipv4_multicast_if(zpl_socket_t sock, ifindex_t ifindex);
extern int setsockopt_ipv4_multicast(zpl_socket_t sock, zpl_uint32 optname,
                                     zpl_uint32  mcast_addr,
			             ifindex_t ifindex);
extern int setsockopt_ipv4_tos(zpl_socket_t sock, zpl_uint32 tos);

/* Ask for, and get, ifindex, by whatever method is supported. */
extern int setsockopt_ifindex (zpl_family_t family, zpl_socket_t, ifindex_t);
extern ifindex_t getsockopt_ifindex (zpl_family_t family, struct ipstack_msghdr *);

/* swab the fields in iph between the host order and system order expected 
 * for IP_HDRINCL.
 */
extern int setsockopt_ipv4_multicast_loop(zpl_socket_t sock, zpl_uint32 opt);

extern void sockopt_iphdrincl_swab_htosys (struct ip *iph);
extern void sockopt_iphdrincl_swab_systoh (struct ip *iph);

extern int sockopt_tcp_rtt (zpl_socket_t);
#ifdef ZPL_KERNEL_MODULE
extern int sockopt_tcp_signature(zpl_socket_t sock, union sockunion *su,
                                 const char *password);
#endif

extern int sockopt_reuseaddr (zpl_socket_t);
extern int sockopt_reuseport (zpl_socket_t);
extern int sockopt_intval(zpl_socket_t fd, int level, int optname, int optval);
extern int sockopt_broadcast(zpl_socket_t fd);
extern int sockopt_keepalive(zpl_socket_t fd);
extern int sockopt_bindtodevice(zpl_socket_t fd, const char *iface);
extern int sockopt_v6only (zpl_family_t family, zpl_socket_t sock);

extern int sockopt_ttl (zpl_family_t family, zpl_socket_t sock, int ttl);
extern int sockopt_minttl (zpl_family_t family, zpl_socket_t sock, int minttl);
extern int sockopt_cork (zpl_socket_t sock, int onoff);
 
#ifdef __cplusplus
}
#endif

#endif /*__LIB_SOCKOPT_H */
