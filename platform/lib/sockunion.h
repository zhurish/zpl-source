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

#ifdef PL_NSM_MODULE
#include "if.h"
#else
typedef ospl_uint32  ifindex_t;
#endif
#if 0
union sockunion {
  struct sockinet {
    ospl_uchar si_len;
    ospl_family_t si_family;
    ospl_ushort si_port;
  } su_si;
  struct sockaddr_in  su_sin;
  struct sockaddr_in6 su_sin6;
};
#define su_len                su_si.si_len
#define su_family     su_si.si_family
#define su_port               su_si.si_port
#endif /* 0 */

union sockunion 
{
  struct sockaddr sa;
  struct sockaddr_in sin;
#ifdef HAVE_IPV6
  struct sockaddr_in6 sin6;
#endif /* HAVE_IPV6 */
};

enum connect_result
{
  connect_error,
  connect_success,
  connect_in_progress
};

/* Default address family. */
#ifdef HAVE_IPV6
#define AF_INET_UNION AF_INET6
#else
#define AF_INET_UNION AF_INET
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
extern const char *sockunion2str (const union sockunion *, ospl_char *, ospl_size_t);
extern int sockunion_cmp (const union sockunion *, const union sockunion *);
extern int sockunion_same (const union sockunion *, const union sockunion *);
extern ospl_uint32  sockunion_hash (const union sockunion *);

extern ospl_size_t family2addrsize(ospl_family_t family);
extern ospl_size_t sockunion_get_addrlen(const union sockunion *);
extern const ospl_uchar *sockunion_get_addr(const union sockunion *);
extern ospl_ushort sockunion_get_port (const union sockunion *);
extern void sockunion_set(union sockunion *, ospl_family_t family, const ospl_uchar *addr, ospl_size_t bytes);

extern union sockunion *sockunion_str2su (const char *str);
extern int sockunion_accept (int sock, union sockunion *);
extern int sockunion_stream_socket (union sockunion *);
extern int sockopt_reuseaddr (int);
extern int sockopt_reuseport (int);
extern int sockopt_int(int fd, int level, int optname, int optval);
extern int sockopt_broadcast(int fd);
extern int sockopt_keepalive(int fd);
extern int sockopt_bindtodevice(int fd, const char *iface);
extern int sockopt_v6only (ospl_family_t family, int sock);
extern int sockunion_bind (int sock, union sockunion *, 
                           ospl_ushort, union sockunion *);
extern int sockopt_ttl (ospl_family_t family, int sock, int ttl);
extern int sockopt_minttl (ospl_family_t family, int sock, int minttl);
extern int sockopt_cork (int sock, int onoff);
extern int sockunion_socket (const union sockunion *su);
extern const char *inet_sutop (const union sockunion *su, ospl_char *str);
extern enum connect_result sockunion_connect (int fd, const union sockunion *su,
                                              ospl_ushort port,
                                              ifindex_t);
extern union sockunion *sockunion_getsockname (int);
extern union sockunion *sockunion_getpeername (int);
extern union sockunion *sockunion_dup (const union sockunion *);
extern void sockunion_free (union sockunion *);

#ifndef HAVE_INET_NTOP
extern const char * inet_ntop (ospl_family_t family, const void *addrptr, 
                               ospl_char *strptr, ospl_size_t len);
#endif /* HAVE_INET_NTOP */

#ifndef HAVE_INET_PTON
extern int inet_pton (ospl_family_t family, const char *strptr, void *addrptr);
#endif /* HAVE_INET_PTON */

#ifndef HAVE_INET_ATON
extern int inet_aton (const char *cp, struct in_addr *inaddr);
#endif
 
#ifdef __cplusplus
}
#endif

#endif /* _ZEBRA_SOCKUNION_H */
