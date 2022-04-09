/*
 * Socket union header.
 * Copyright (c) 1997 Kunihiro Ishiguro
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

#ifndef _ZEBRA_SOCKUNION_H
#define _ZEBRA_SOCKUNION_H

#ifdef __cplusplus
extern "C" {
#endif


#if 0
union sockunion {
  struct sockinet {
    zpl_uchar si_len;
    zpl_family_t si_family;
    zpl_ushort si_port;
  } su_si;
  struct ipstack_sockaddr_in  su_sin;
  struct ipstack_sockaddr_in6 su_sin6;
};
#define su_len                su_si.si_len
#define su_family     su_si.si_family
#define su_port               su_si.si_port
#endif /* 0 */

union sockunion 
{
  struct ipstack_sockaddr sa;
  struct ipstack_sockaddr_in sin;
#ifdef ZPL_BUILD_IPV6
  struct ipstack_sockaddr_in6 sin6;
#endif /* ZPL_BUILD_IPV6 */
};

enum connect_result
{
  connect_error,
  connect_success,
  connect_in_progress
};

/* Default address family. */
#ifdef ZPL_BUILD_IPV6
#define AF_INET_UNION IPSTACK_AF_INET6
#else
#define AF_INET_UNION IPSTACK_AF_INET
#endif

/* Sockunion address string length.  Same as INET6_ADDRSTRLEN. */
#define SU_ADDRSTRLEN 46

/* Macro to set link local index to the IPv6 address.  For KAME IPv6
   stack. */
#ifdef KAME
#define	IN6_LINKLOCAL_IFINDEX(a)  ((a).s6_addr[2] << 8 | (a).s6_addr[3])
#define SET_IN6_LINKLOCAL_IFINDEX(a, i) \
  do { \
    (a).s6_addr[2] = ((i) >> 8) & 0xff; \
    (a).s6_addr[3] = (i) & 0xff; \
  } while (0)
#else
#define	IN6_LINKLOCAL_IFINDEX(a)
#define SET_IN6_LINKLOCAL_IFINDEX(a, i)
#endif /* KAME */

#define sockunion_family(X)  (X)->sa.sa_family

#define sockunion2ip(X)      (X)->sin.sin_addr.s_addr

/* Prototypes. */
extern int str2sockunion (const char *, union sockunion *);
extern const char *sockunion2str (const union sockunion *, zpl_char *, zpl_size_t);
extern int sockunion_cmp (const union sockunion *, const union sockunion *);
extern int sockunion_same (const union sockunion *, const union sockunion *);
extern zpl_uint32  sockunion_hash (const union sockunion *);

extern zpl_size_t family2addrsize(zpl_family_t family);
extern zpl_size_t sockunion_get_addrlen(const union sockunion *);
extern const zpl_uchar *sockunion_get_addr(const union sockunion *);
extern zpl_ushort sockunion_get_port (const union sockunion *);
extern void sockunion_set(union sockunion *, zpl_family_t family, const zpl_uchar *addr, zpl_size_t bytes);

extern union sockunion *sockunion_str2su (const char *str);
extern zpl_socket_t sockunion_accept (zpl_socket_t sock, union sockunion *);
extern zpl_socket_t sockunion_stream_socket (union sockunion *);
extern int sockopt_reuseaddr (zpl_socket_t);
extern int sockopt_reuseport (zpl_socket_t);
extern int sockopt_int(zpl_socket_t fd, int level, int optname, int optval);
extern int sockopt_broadcast(zpl_socket_t fd);
extern int sockopt_keepalive(zpl_socket_t fd);
extern int sockopt_bindtodevice(zpl_socket_t fd, const char *iface);
extern int sockopt_v6only (zpl_family_t family, zpl_socket_t sock);
extern int sockunion_bind (zpl_socket_t sock, union sockunion *, 
                           zpl_ushort, union sockunion *);
extern int sockopt_ttl (zpl_family_t family, zpl_socket_t sock, int ttl);
extern int sockopt_minttl (zpl_family_t family, zpl_socket_t sock, int minttl);
extern int sockopt_cork (zpl_socket_t sock, int onoff);
extern zpl_socket_t sockunion_socket (const union sockunion *su);
extern const char *inet_sutop (const union sockunion *su, zpl_char *str);
extern enum connect_result sockunion_connect (zpl_socket_t fd, const union sockunion *su,
                                              zpl_ushort port,
                                              ifindex_t);
extern union sockunion *sockunion_getsockname (zpl_socket_t);
extern union sockunion *sockunion_getpeername (zpl_socket_t);
extern union sockunion *sockunion_dup (const union sockunion *);
extern void sockunion_free (union sockunion *);

#ifndef HAVE_INET_NTOP
extern const char * ipstack_inet_ntop (zpl_family_t family, const void *addrptr, 
                               zpl_char *strptr, zpl_size_t len);
#endif /* HAVE_INET_NTOP */

#ifndef HAVE_INET_PTON
extern int ipstack_inet_pton (zpl_family_t family, const char *strptr, void *addrptr);
#endif /* HAVE_INET_PTON */

#ifndef HAVE_INET_ATON
extern int ipstack_inet_aton (const char *cp, struct ipstack_in_addr *inaddr);
#endif
 
#ifdef __cplusplus
}
#endif

#endif /* _ZEBRA_SOCKUNION_H */
