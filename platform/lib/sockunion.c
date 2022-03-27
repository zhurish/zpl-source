/* Socket union related function.
 * Copyright (c) 1997, 98 Kunihiro Ishiguro
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
#include "zpl_include.h"
#include "sockunion.h"
#include "log.h"
#include "zmemory.h"
#ifndef HAVE_INET_ATON
int
ipstack_inet_aton (const char *cp, struct ipstack_in_addr *inaddr)
{
  int dots = 0;
  register u_long addr = 0;
  register u_long val = 0, base = 10;

  do
    {
      register zpl_char c = *cp;

      switch (c)
	{
	case '0': case '1': case '2': case '3': case '4': case '5':
	case '6': case '7': case '8': case '9':
	  val = (val * base) + (c - '0');
	  break;
	case '.':
	  if (++dots > 3)
	    return 0;
	case '\0':
	  if (val > 255)
	    return 0;
	  addr = addr << 8 | val;
	  val = 0;
	  break;
	default:
	  return 0;
	}
    } while (*cp++) ;

  if (dots < 3)
    addr <<= 8 * (3 - dots);
  if (inaddr)
    inaddr->s_addr = htonl (addr);
  return 1;
}
#endif /* ! HAVE_INET_ATON */


#ifndef HAVE_INET_PTON
int
ipstack_inet_pton (zpl_family_t family, const char *strptr, void *addrptr)
{
  if (family == IPSTACK_AF_INET)
    {
      struct ipstack_in_addr in_val;

      if (ipstack_inet_aton (strptr, &in_val))
	{
	  memcpy (addrptr, &in_val, sizeof (struct ipstack_in_addr));
	  return 1;
	}
      return 0;
    }
  ipstack_errno = EAFNOSUPPORT;
  return -1;
}
#endif /* ! HAVE_INET_PTON */

#ifndef HAVE_INET_NTOP
const char *
ipstack_inet_ntop (zpl_family_t family, const void *addrptr, zpl_char *strptr, zpl_size_t len)
{
  zpl_uchar *p = (zpl_uchar *) addrptr;

  if (family == IPSTACK_AF_INET) 
    {
      zpl_char temp[IPSTACK_INET_ADDRSTRLEN];

      snprintf(temp, sizeof(temp), "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);

      if (strlen(temp) >= len) 
	{
	  ipstack_errno = ENOSPC;
	  return NULL;
	}
      strcpy(strptr, temp);
      return strptr;
    }

  ipstack_errno = EAFNOSUPPORT;
  return NULL;
}
#endif /* ! HAVE_INET_NTOP */

const char *
inet_sutop (const union sockunion *su, zpl_char *str)
{
  switch (su->sa.sa_family)
    {
    case IPSTACK_AF_INET:
      ipstack_inet_ntop (IPSTACK_AF_INET, &su->sin.sin_addr, str, IPSTACK_INET_ADDRSTRLEN);
      break;
#ifdef HAVE_IPV6
    case IPSTACK_AF_INET6:
      ipstack_inet_ntop (IPSTACK_AF_INET6, &su->sin6.sin6_addr, str, INET6_ADDRSTRLEN);
      break;
#endif /* HAVE_IPV6 */
    }
  return str;
}

int
str2sockunion (const char *str, union sockunion *su)
{
  int ret;

  memset (su, 0, sizeof (union sockunion));

  ret = ipstack_inet_pton (IPSTACK_AF_INET, str, &su->sin.sin_addr);
  if (ret > 0)			/* Valid IPv4 address format. */
    {
      su->sin.sin_family = IPSTACK_AF_INET;
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
      su->sin.sin_len = sizeof(struct ipstack_sockaddr_in);
#endif /* HAVE_STRUCT_SOCKADDR_IN_SIN_LEN */
      return 0;
    }
#ifdef HAVE_IPV6
  ret = ipstack_inet_pton (IPSTACK_AF_INET6, str, &su->sin6.sin6_addr);
  if (ret > 0)			/* Valid IPv6 address format. */
    {
      su->sin6.sin6_family = IPSTACK_AF_INET6;
#ifdef SIN6_LEN
      su->sin6.sin6_len = sizeof(struct ipstack_sockaddr_in6);
#endif /* SIN6_LEN */
      return 0;
    }
#endif /* HAVE_IPV6 */
  return -1;
}

const char *
sockunion2str (const union sockunion *su, zpl_char *buf, zpl_size_t len)
{
  switch (sockunion_family(su))
    {
    case IPSTACK_AF_UNSPEC:
      snprintf (buf, len, "(unspec)");
      return buf;
    case IPSTACK_AF_INET:
      return ipstack_inet_ntop (IPSTACK_AF_INET, &su->sin.sin_addr, buf, len);
#ifdef HAVE_IPV6
    case IPSTACK_AF_INET6:
      return ipstack_inet_ntop (IPSTACK_AF_INET6, &su->sin6.sin6_addr, buf, len);
#endif /* HAVE_IPV6 */
    }
  snprintf (buf, len, "(af %d)", sockunion_family(su));
  return buf;
}

union sockunion *
sockunion_str2su (const char *str)
{
  union sockunion *su = XCALLOC (MTYPE_SOCKUNION, sizeof (union sockunion));
  
  if (!str2sockunion (str, su))
    return su;
  
  XFREE (MTYPE_SOCKUNION, su);
  return NULL;
}

/* Convert IPv4 compatible IPv6 address to IPv4 address. */
static void
sockunion_normalise_mapped (union sockunion *su)
{
  struct ipstack_sockaddr_in sin;
  
#ifdef HAVE_IPV6
  if (su->sa.sa_family == IPSTACK_AF_INET6 
      && IPSTACK_IN6_IS_ADDR_V4MAPPED (&su->sin6.sin6_addr))
    {
      memset (&sin, 0, sizeof (struct ipstack_sockaddr_in));
      sin.sin_family = IPSTACK_AF_INET;
      sin.sin_port = su->sin6.sin6_port;
      memcpy (&sin.sin_addr, ((zpl_char *)&su->sin6.sin6_addr) + 12, 4);
      memcpy (su, &sin, sizeof (struct ipstack_sockaddr_in));
    }
#endif /* HAVE_IPV6 */
}

/* Return socket of sockunion. */
zpl_socket_t
sockunion_socket (const union sockunion *su)
{
  zpl_socket_t sock;

  sock = ipstack_socket (IPCOM_STACK, su->sa.sa_family, IPSTACK_SOCK_STREAM, 0);
  if (sock._fd < 0)
    {
      zlog (MODULE_DEFAULT,  ZLOG_LEVEL_WARNING, "Can't make socket : %s", ipstack_strerror (ipstack_errno));
      return (zpl_socket_t)sock;
    }

  return sock;
}

/* Return accepted new socket file descriptor. */
zpl_socket_t
sockunion_accept (zpl_socket_t sock, union sockunion *su)
{
  socklen_t len;
  zpl_socket_t client_sock;

  len = sizeof (union sockunion);
  client_sock = ipstack_accept (sock, (struct ipstack_sockaddr *) su, &len);
  
  sockunion_normalise_mapped (su);
  return client_sock;
}

/* Return sizeof union sockunion.  */
static int
sockunion_sizeof (const union sockunion *su)
{
  int ret;

  ret = 0;
  switch (su->sa.sa_family)
    {
    case IPSTACK_AF_INET:
      ret = sizeof (struct ipstack_sockaddr_in);
      break;
#ifdef HAVE_IPV6
    case IPSTACK_AF_INET6:
      ret = sizeof (struct ipstack_sockaddr_in6);
      break;
#endif /* IPSTACK_AF_INET6 */
    }
  return ret;
}

/* return sockunion structure : this function should be revised. */
static const char *
sockunion_log (const union sockunion *su, zpl_char *buf, zpl_size_t len)
{
  switch (su->sa.sa_family) 
    {
    case IPSTACK_AF_INET:
      return ipstack_inet_ntop(IPSTACK_AF_INET, &su->sin.sin_addr, buf, len);

#ifdef HAVE_IPV6
    case IPSTACK_AF_INET6:
      return ipstack_inet_ntop(IPSTACK_AF_INET6, &(su->sin6.sin6_addr), buf, len);
      break;
#endif /* HAVE_IPV6 */

    default:
      snprintf (buf, len, "af_unknown %d ", su->sa.sa_family);
      return buf;
    }
}

/* sockunion_connect returns
   -1 : error occured
   0 : connect success
   1 : connect is in progress */
enum connect_result
sockunion_connect (zpl_socket_t fd, const union sockunion *peersu, zpl_ushort port,
		   ifindex_t ifindex)
{
  int ret;
  zpl_uint32 val;
  union sockunion su;

  memcpy (&su, peersu, sizeof (union sockunion));

  switch (su.sa.sa_family)
    {
    case IPSTACK_AF_INET:
      su.sin.sin_port = port;
      break;
#ifdef HAVE_IPV6
    case IPSTACK_AF_INET6:
      su.sin6.sin6_port  = port;
#ifdef KAME
      if (IPSTACK_IN6_IS_ADDR_LINK_LOCAL(&su.sin6.sin6_addr) && ifindex)
	{
#ifdef HAVE_STRUCT_SOCKADDR_IN6_SIN6_SCOPE_ID
	  /* su.sin6.sin6_scope_id = ifindex; */
#endif /* HAVE_STRUCT_SOCKADDR_IN6_SIN6_SCOPE_ID */
	  SET_IN6_LINKLOCAL_IFINDEX (su.sin6.sin6_addr, ifindex);
	}
#endif /* KAME */
      break;
#endif /* HAVE_IPV6 */
    }      

  /* Make socket non-block. */
  //val = fcntl (fd, F_GETFL, 0);
  //fcntl (fd, F_SETFL, val|O_NONBLOCK);

  /* Call connect function. */
  ret = ipstack_connect (fd, (struct ipstack_sockaddr *) &su, sockunion_sizeof (&su));

  /* Immediate success */
  if (ret == 0)
    {
      //fcntl (fd, F_SETFL, val);
      return connect_success;
    }

  /* If connect is in progress then return 1 else it's real error. */
  if (ret < 0)
    {
      if (ipstack_errno != IPSTACK_ERRNO_EINPROGRESS)
	{
	  zpl_char str[SU_ADDRSTRLEN];
	  zlog_info (MODULE_DEFAULT, "can't connect to %s fd %d : %s",
		     sockunion_log (&su, str, sizeof str),
		     fd, ipstack_strerror (ipstack_errno));
	  return connect_error;
	}
    }

  //fcntl (fd, F_SETFL, val);

  return connect_in_progress;
}

/* Make socket from sockunion union. */
zpl_socket_t
sockunion_stream_socket (union sockunion *su)
{
  zpl_socket_t sock;

  if (su->sa.sa_family == 0)
    su->sa.sa_family = AF_INET_UNION;

  sock = ipstack_socket (IPCOM_STACK, su->sa.sa_family, IPSTACK_SOCK_STREAM, 0);

  if (sock._fd < 0)
    zlog (MODULE_DEFAULT, ZLOG_LEVEL_WARNING, "can't make socket sockunion_stream_socket");

  return sock;
}

/* Bind socket to specified address. */
int
sockunion_bind (zpl_socket_t sock, union sockunion *su, zpl_ushort port, 
		union sockunion *su_addr)
{
  int size = 0;
  int ret;

  if (su->sa.sa_family == IPSTACK_AF_INET)
    {
      size = sizeof (struct ipstack_sockaddr_in);
      su->sin.sin_port = htons (port);
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
      su->sin.sin_len = size;
#endif /* HAVE_STRUCT_SOCKADDR_IN_SIN_LEN */
      if (su_addr == NULL)
	sockunion2ip (su) = htonl (IPSTACK_INADDR_ANY);
    }
#ifdef HAVE_IPV6
  else if (su->sa.sa_family == IPSTACK_AF_INET6)
    {
      size = sizeof (struct ipstack_sockaddr_in6);
      su->sin6.sin6_port = htons (port);
#ifdef SIN6_LEN
      su->sin6.sin6_len = size;
#endif /* SIN6_LEN */
      if (su_addr == NULL)
	{
#ifdef LINUX_IPV6
	  memset (&su->sin6.sin6_addr, 0, sizeof (struct ipstack_in6_addr));
#else
	  su->sin6.sin6_addr = in6addr_any;
#endif /* LINUX_IPV6 */
	}
    }
#endif /* HAVE_IPV6 */
  

  ret = ipstack_bind (sock, (struct ipstack_sockaddr *)su, size);
  if (ret < 0)
    zlog (MODULE_DEFAULT, ZLOG_LEVEL_WARNING, "can't bind socket : %s", ipstack_strerror (ipstack_errno));

  return ret;
}

int
sockopt_reuseaddr (zpl_socket_t sock)
{
  int ret;
  int on = 1;

  ret = ipstack_setsockopt (sock, IPSTACK_SOL_SOCKET, IPSTACK_SO_REUSEADDR,
		    (void *) &on, sizeof (on));
  if (ret < 0)
    {
      zlog (MODULE_DEFAULT, ZLOG_LEVEL_WARNING, "can't set sockopt IPSTACK_SO_REUSEADDR to socket %d", sock);
      return -1;
    }
  return 0;
}

#ifdef IPSTACK_SO_REUSEPORT
int
sockopt_reuseport (zpl_socket_t sock)
{
  int ret;
  int on = 1;

  ret = ipstack_setsockopt (sock, IPSTACK_SOL_SOCKET, IPSTACK_SO_REUSEPORT,
		    (void *) &on, sizeof (on));
  if (ret < 0)
    {
      zlog (MODULE_DEFAULT, ZLOG_LEVEL_WARNING, "can't set sockopt IPSTACK_SO_REUSEPORT to socket %d", sock);
      return -1;
    }
  return 0;
}
#else
int
sockopt_reuseport (zpl_socket_t sock)
{
  return 0;
}
#endif /* 0 */


int sockopt_int(zpl_socket_t fd, int level, int optname, int optval)
{
	return ipstack_setsockopt(fd, level, optname, &optval, sizeof(int));
}

static int sockopt_SOL_SOCKET_int(zpl_socket_t fd, int optname, int optval)
{
	return sockopt_int(fd, IPSTACK_SOL_SOCKET, optname, optval);
}

static int sockopt_SOL_SOCKET_1(zpl_socket_t fd, int optname)
{
	return sockopt_SOL_SOCKET_int(fd, optname, 1);
}

int sockopt_broadcast(zpl_socket_t fd)
{
	return sockopt_SOL_SOCKET_1(fd, IPSTACK_SO_BROADCAST);
}

int sockopt_keepalive(zpl_socket_t fd)
{
	return sockopt_SOL_SOCKET_1(fd, IPSTACK_SO_KEEPALIVE);
}

#ifdef IPSTACK_SO_BINDTODEVICE
int sockopt_bindtodevice(zpl_socket_t fd, const char *iface)
{
	int r;
	struct ipstack_ifreq ifr;
	strcpy(ifr.ifr_name, iface);
	/* NB: passing (iface, strlen(iface) + 1) does not work!
	 * (maybe it works on _some_ kernels, but not on 2.6.26)
	 * Actually, ifr_name is at offset 0, and in practice
	 * just giving zpl_char[IFNAMSIZ] instead of struct ipstack_ifreq works too.
	 * But just in case it's not true on some obscure arch... */
	r = ipstack_setsockopt(fd, IPSTACK_SOL_SOCKET, IPSTACK_SO_BINDTODEVICE, &ifr, sizeof(ifr));
	if (r)
		zlog (MODULE_DEFAULT, ZLOG_LEVEL_WARNING, "can't bind to interface %s", iface);
	return r;
}
#else
int sockopt_bindtodevice(zpl_socket_t fd,
		const char *iface)
{
	zlog (MODULE_DEFAULT, ZLOG_LEVEL_WARNING, "IPSTACK_SO_BINDTODEVICE is not supported on this system");
	return -1;
}
#endif

int
sockopt_ttl (zpl_family_t family, zpl_socket_t sock, int ttl)
{
  int ret;

#ifdef IP_TTL
  if (family == IPSTACK_AF_INET)
    {
      ret = ipstack_setsockopt (sock, IPSTACK_IPPROTO_IP, IPSTACK_IP_TTL,
			(void *) &ttl, sizeof (int));
      if (ret < 0)
	{
	  zlog (MODULE_DEFAULT, ZLOG_LEVEL_WARNING, "can't set sockopt IP_TTL %d to socket %d", ttl, sock);
	  return -1;
	}
      return 0;
    }
#endif /* IP_TTL */
#ifdef HAVE_IPV6
  if (family == IPSTACK_AF_INET6)
    {
      ret = ipstack_setsockopt (sock, IPSTACK_IPPROTO_IPV6, IPSTACK_IPV6_UNICAST_HOPS,
			(void *) &ttl, sizeof (int));
      if (ret < 0)
	{
	  zlog (MODULE_DEFAULT,  ZLOG_LEVEL_WARNING, "can't set sockopt IPSTACK_IPV6_UNICAST_HOPS %d to socket %d",
		    ttl, sock);
	  return -1;
	}
      return 0;
    }
#endif /* HAVE_IPV6 */
  return 0;
}

int
sockopt_cork (zpl_socket_t sock, int onoff)
{
#ifdef TCP_CORK
  return ipstack_setsockopt (sock, IPSTACK_IPPROTO_TCP, IPSTACK_TCP_CORK, &onoff, sizeof(onoff));
#else
  return 0;
#endif
}

int
sockopt_minttl (zpl_family_t family, zpl_socket_t sock, int minttl)
{
#ifdef IP_MINTTL
  if (family == IPSTACK_AF_INET)
    {
      int ret = ipstack_setsockopt (sock, IPSTACK_IPPROTO_IP, IPSTACK_IP_MINTTL, &minttl, sizeof(minttl));
      if (ret < 0)
	  zlog (MODULE_DEFAULT, ZLOG_LEVEL_WARNING,
		"can't set sockopt IP_MINTTL to %d on socket %d: %s",
		minttl, sock, ipstack_strerror (ipstack_errno));
      return ret;
    }
#endif /* IP_MINTTL */
#ifdef IPV6_MINHOPCNT
  if (family == IPSTACK_AF_INET6)
    {
      int ret = ipstack_setsockopt (sock, IPSTACK_IPPROTO_IPV6, IPSTACK_IPV6_MINHOPCNT, &minttl, sizeof(minttl));
      if (ret < 0)
	  zlog (MODULE_DEFAULT, ZLOG_LEVEL_WARNING,
		"can't set sockopt IPV6_MINHOPCNT to %d on socket %d: %s",
		minttl, sock, ipstack_strerror (ipstack_errno));
      return ret;
    }
#endif

  ipstack_errno = IPSTACK_ERRNO_EOPNOTSUPP;
  return -1;
}

int
sockopt_v6only (zpl_family_t family, zpl_socket_t sock)
{
  int ret, on = 1;

#ifdef HAVE_IPV6
#ifdef IPV6_V6ONLY
  if (family == IPSTACK_AF_INET6)
    {
      ret = ipstack_setsockopt (sock, IPSTACK_IPPROTO_IPV6, IPSTACK_IPV6_V6ONLY,
			(void *) &on, sizeof (int));
      if (ret < 0)
	{
	  zlog (MODULE_DEFAULT,  ZLOG_LEVEL_WARNING, "can't set sockopt IPV6_V6ONLY "
		    "to socket %d", sock);
	  return -1;
	}
      return 0;
    }
#endif /* IPV6_V6ONLY */
#endif /* HAVE_IPV6 */
  return 0;
}

/* If same family and same prefix return 1. */
int
sockunion_same (const union sockunion *su1, const union sockunion *su2)
{
  int ret = 0;

  if (su1->sa.sa_family != su2->sa.sa_family)
    return 0;

  switch (su1->sa.sa_family)
    {
    case IPSTACK_AF_INET:
      ret = memcmp (&su1->sin.sin_addr, &su2->sin.sin_addr,
		    sizeof (struct ipstack_in_addr));
      break;
#ifdef HAVE_IPV6
    case IPSTACK_AF_INET6:
      ret = memcmp (&su1->sin6.sin6_addr, &su2->sin6.sin6_addr,
		    sizeof (struct ipstack_in6_addr));
      break;
#endif /* HAVE_IPV6 */
    }
  if (ret == 0)
    return 1;
  else
    return 0;
}

zpl_uint32 
sockunion_hash (const union sockunion *su)
{
  switch (sockunion_family(su))
    {
    case IPSTACK_AF_INET:
      return jhash_1word(su->sin.sin_addr.s_addr, 0);
#ifdef HAVE_IPV6
    case IPSTACK_AF_INET6:
      return jhash2(su->sin6.sin6_addr.s6_addr32, ZEBRA_NUM_OF(su->sin6.sin6_addr.s6_addr32), 0);
#endif /* HAVE_IPV6 */
    }
  return 0;
}

zpl_size_t
family2addrsize(zpl_family_t family)
{
  switch (family)
    {
    case IPSTACK_AF_INET:
      return sizeof(struct ipstack_in_addr);
#ifdef HAVE_IPV6
    case IPSTACK_AF_INET6:
      return sizeof(struct ipstack_in6_addr);
#endif /* HAVE_IPV6 */
    }
  return 0;
}

zpl_size_t
sockunion_get_addrlen(const union sockunion *su)
{
  return family2addrsize(sockunion_family(su));
}

const zpl_uchar *
sockunion_get_addr(const union sockunion *su)
{
  switch (sockunion_family(su))
    {
    case IPSTACK_AF_INET:
      return (const zpl_uchar *) &su->sin.sin_addr.s_addr;
#ifdef HAVE_IPV6
    case IPSTACK_AF_INET6:
      return (const zpl_uchar *) &su->sin6.sin6_addr;
#endif /* HAVE_IPV6 */
    }
  return NULL;
}

zpl_ushort
sockunion_get_port (const union sockunion *su)
{
  switch (sockunion_family (su))
    {
    case IPSTACK_AF_INET:
      return ntohs(su->sin.sin_port);
#ifdef HAVE_IPV6
    case IPSTACK_AF_INET6:
      return ntohs(su->sin6.sin6_port);
#endif /* HAVE_IPV6 */
    }
  return 0;
}

void
sockunion_set(union sockunion *su, zpl_family_t family, const zpl_uchar *addr, zpl_size_t bytes)
{
  if (family2addrsize(family) != bytes)
    return;

  sockunion_family(su) = family;
  switch (family)
    {
    case IPSTACK_AF_INET:
      memcpy(&su->sin.sin_addr.s_addr, addr, bytes);
      break;
#ifdef HAVE_IPV6
    case IPSTACK_AF_INET6:
      memcpy(&su->sin6.sin6_addr, addr, bytes);
      break;
#endif /* HAVE_IPV6 */
    }
}

/* After TCP connection is established.  Get local address and port. */
union sockunion *
sockunion_getsockname (zpl_socket_t fd)
{
  int ret;
  socklen_t len;
  union
  {
    struct ipstack_sockaddr sa;
    struct ipstack_sockaddr_in sin;
#ifdef HAVE_IPV6
    struct ipstack_sockaddr_in6 sin6;
#endif /* HAVE_IPV6 */
    zpl_char tmp_buffer[128];
  } name;
  union sockunion *su;

  memset (&name, 0, sizeof name);
  len = sizeof name;

  ret = ipstack_getsockname (fd, (struct ipstack_sockaddr *)&name, &len);
  if (ret < 0)
    {
      zlog_warn (MODULE_DEFAULT, "Can't get local address and port by getsockname: %s",
		 ipstack_strerror (ipstack_errno));
      return NULL;
    }

  if (name.sa.sa_family == IPSTACK_AF_INET)
    {
      su = XCALLOC (MTYPE_SOCKUNION, sizeof (union sockunion));
      memcpy (su, &name, sizeof (struct ipstack_sockaddr_in));
      return su;
    }
#ifdef HAVE_IPV6
  if (name.sa.sa_family == IPSTACK_AF_INET6)
    {
      su = XCALLOC (MTYPE_SOCKUNION, sizeof (union sockunion));
      memcpy (su, &name, sizeof (struct ipstack_sockaddr_in6));
      sockunion_normalise_mapped (su);
      return su;
    }
#endif /* HAVE_IPV6 */
  return NULL;
}

/* After TCP connection is established.  Get remote address and port. */
union sockunion *
sockunion_getpeername (zpl_socket_t fd)
{
  int ret;
  socklen_t len;
  union
  {
    struct ipstack_sockaddr sa;
    struct ipstack_sockaddr_in sin;
#ifdef HAVE_IPV6
    struct ipstack_sockaddr_in6 sin6;
#endif /* HAVE_IPV6 */
    zpl_char tmp_buffer[128];
  } name;
  union sockunion *su;

  memset (&name, 0, sizeof name);
  len = sizeof name;
  ret = ipstack_getpeername (fd, (struct ipstack_sockaddr *)&name, &len);
  if (ret < 0)
    {
      zlog (MODULE_DEFAULT, ZLOG_LEVEL_WARNING, "Can't get remote address and port: %s",
	    ipstack_strerror (ipstack_errno));
      return NULL;
    }

  if (name.sa.sa_family == IPSTACK_AF_INET)
    {
      su = XCALLOC (MTYPE_SOCKUNION, sizeof (union sockunion));
      memcpy (su, &name, sizeof (struct ipstack_sockaddr_in));
      return su;
    }
#ifdef HAVE_IPV6
  if (name.sa.sa_family == IPSTACK_AF_INET6)
    {
      su = XCALLOC (MTYPE_SOCKUNION, sizeof (union sockunion));
      memcpy (su, &name, sizeof (struct ipstack_sockaddr_in6));
      sockunion_normalise_mapped (su);
      return su;
    }
#endif /* HAVE_IPV6 */
  return NULL;
}

/* Print sockunion structure */
static void __attribute__ ((unused))
sockunion_print (const union sockunion *su)
{
  if (su == NULL)
    return;

  switch (su->sa.sa_family) 
    {
    case IPSTACK_AF_INET:
      printf ("%s\n", ipstack_inet_ntoa (su->sin.sin_addr));
      break;
#ifdef HAVE_IPV6
    case IPSTACK_AF_INET6:
      {
	zpl_char buf [SU_ADDRSTRLEN];

	printf ("%s\n", ipstack_inet_ntop (IPSTACK_AF_INET6, &(su->sin6.sin6_addr),
				 buf, sizeof (buf)));
      }
      break;
#endif /* HAVE_IPV6 */

#ifdef AF_LINK
    case IPSTACK_AF_LINK:
      {
	struct ipstack_sockaddr_dl *sdl;

	sdl = (struct ipstack_sockaddr_dl *)&(su->sa);
	printf ("link#%d\n", sdl->sdl_index);
      }
      break;
#endif /* AF_LINK */
    default:
      printf ("af_unknown %d\n", su->sa.sa_family);
      break;
    }
}

#ifdef HAVE_IPV6
static int
in6addr_cmp (const struct ipstack_in6_addr *addr1, const struct ipstack_in6_addr *addr2)
{
  zpl_uint32  i;
  zpl_uchar *p1, *p2;

  p1 = (zpl_uchar *)addr1;
  p2 = (zpl_uchar *)addr2;

  for (i = 0; i < sizeof (struct ipstack_in6_addr); i++)
    {
      if (p1[i] > p2[i])
	return 1;
      else if (p1[i] < p2[i])
	return -1;
    }
  return 0;
}
#endif /* HAVE_IPV6 */

int
sockunion_cmp (const union sockunion *su1, const union sockunion *su2)
{
  if (su1->sa.sa_family > su2->sa.sa_family)
    return 1;
  if (su1->sa.sa_family < su2->sa.sa_family)
    return -1;

  if (su1->sa.sa_family == IPSTACK_AF_INET)
    {
      if (ntohl (sockunion2ip (su1)) == ntohl (sockunion2ip (su2)))
	return 0;
      if (ntohl (sockunion2ip (su1)) > ntohl (sockunion2ip (su2)))
	return 1;
      else
	return -1;
    }
#ifdef HAVE_IPV6
  if (su1->sa.sa_family == IPSTACK_AF_INET6)
    return in6addr_cmp (&su1->sin6.sin6_addr, &su2->sin6.sin6_addr);
#endif /* HAVE_IPV6 */
  return 0;
}

/* Duplicate sockunion. */
union sockunion *
sockunion_dup (const union sockunion *su)
{
  union sockunion *dup = XCALLOC (MTYPE_SOCKUNION, sizeof (union sockunion));
  memcpy (dup, su, sizeof (union sockunion));
  return dup;
}

void
sockunion_free (union sockunion *su)
{
  XFREE (MTYPE_SOCKUNION, su);
}
