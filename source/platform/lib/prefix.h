/*
 * Prefix structure.
 * Copyright (C) 1998 Kunihiro Ishiguro
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

#ifndef __LIB_PREFIX_H
#define __LIB_PREFIX_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SUNOS_5
# include <sys/ethernet.h>
#else
# ifdef GNU_LINUX
#  include <net/ethernet.h>
# else
#  include <netinet/if_ether.h>
# endif
#endif
//#include <linux/mpls.h>
#include "sockunion.h"

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN  ETHERADDRL
#endif

/*
 * there isn't a portable ethernet address type. We define our
 * own to simplify internal handling
 */
struct ipstack_ethaddr {
    zpl_uchar octet[ETHER_ADDR_LEN];
} __attribute__ ((__packed__));

/* Reference: RFC 5462, RFC 3032
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                Label                  | TC  |S|       TTL     |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *	Label:  Label Value, 20 bits
 *	TC:     Traffic Class field, 3 bits
 *	S:      Bottom of Stack, 1 bit
 *	TTL:    Time to Live, 8 bits
 */

struct ipstack_mpls {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    zpl_uint32 ttl:8;            /* Time to Live, 8 bits */
    zpl_uint32 tottom:1;            /* Bottom of Stack, 1 bit */
    zpl_uint32 tc:3;            /* Traffic Class field, 3 bits */
    zpl_uint32 label:20;             /* Label Value, 20 bits */
#endif
#if __BYTE_ORDER == __BIG_ENDIAN
    zpl_uint32 label:20;             /* Label Value, 20 bits */
    zpl_uint32 tc:3;            /* Traffic Class field, 3 bits */
    zpl_uint32 tottom:1;            /* Bottom of Stack, 1 bit */
    zpl_uint32 ttl:8;            /* Time to Live, 8 bits */
#endif
};

/*
 * A struct prefix contains an address family, a prefix length, and an
 * address.  This can represent either a 'network prefix' as defined
 * by CIDR, where the 'host bits' of the prefix are 0
 * (e.g. IPSTACK_AF_INET:10.0.0.0/8), or an address and netmask
 * (e.g. IPSTACK_AF_INET:10.0.0.9/8), such as might be configured on an
 * interface.
 */

/* different OSes use different names */
#if defined(IPSTACK_AF_PACKET)
#define AF_ETHERNET IPSTACK_AF_PACKET
#else
#if defined(IPSTACK_AF_LINK)
#define AF_ETHERNET IPSTACK_AF_LINK
#endif
#endif

/* IPv4 and IPv6 unified prefix structure. */
struct prefix
{
  zpl_family_t family;
  zpl_uchar prefixlen;
  union 
  {
    zpl_uchar prefix;
    struct ipstack_in_addr prefix4;
#ifdef ZPL_BUILD_IPV6
    struct ipstack_in6_addr prefix6;
#endif /* ZPL_BUILD_IPV6 */
    struct 
    {
      struct ipstack_in_addr id;
      struct ipstack_in_addr adv_router;
    } lp;
    struct ipstack_ethaddr prefix_eth;	/* AF_ETHERNET */
    struct ipstack_mpls mpls_label;
    zpl_uchar val[8];
    uintptr_t ptr;
  } u __attribute__ ((aligned (8)));
};

/* IPv4 prefix structure. */
struct prefix_ipv4
{
  zpl_family_t family;
  zpl_uchar prefixlen;
  struct ipstack_in_addr prefix __attribute__ ((aligned (8)));
};

/* IPv6 prefix structure. */
#ifdef ZPL_BUILD_IPV6
struct prefix_ipv6
{
  zpl_family_t family;
  zpl_uchar prefixlen;
  struct ipstack_in6_addr prefix __attribute__ ((aligned (8)));
};
#endif /* ZPL_BUILD_IPV6 */

struct prefix_ls
{
  zpl_family_t family;
  zpl_uchar prefixlen;
  struct ipstack_in_addr id __attribute__ ((aligned (8)));
  struct ipstack_in_addr adv_router;
};

/* Prefix for routing distinguisher. */
struct prefix_rd
{
  zpl_family_t family;
  zpl_uchar prefixlen;
  zpl_uchar val[8] __attribute__ ((aligned (8)));
};

/* Prefix for ethernet. */
struct prefix_eth
{
  zpl_family_t family;
  zpl_uchar prefixlen;
  struct ipstack_ethaddr eth_addr __attribute__ ((aligned (8))); /* AF_ETHERNET */
};

/* Prefix for a generic pointer */
struct prefix_ptr
{
  zpl_family_t family;
  zpl_uchar prefixlen;
  uintptr_t prefix __attribute__ ((aligned (8)));
};

struct prefix_mpls
{
  zpl_family_t family;
  zpl_uchar prefixlen;
  struct ipstack_mpls prefix __attribute__ ((aligned (8)));
};

/* helper to get type safety/avoid casts on calls
 * (w/o this, functions accepting all prefix types need casts on the caller
 * side, which strips type safety since the cast will accept any pointer
 * type.)
 */
union prefix46ptr
{
  struct prefix *p;
  struct prefix_ipv4 *p4;
  struct prefix_ipv6 *p6;
  struct prefix_mpls *mpls;
} __attribute__ ((transparent_union));

union prefix46constptr
{
  const struct prefix *p;
  const struct prefix_ipv4 *p4;
  const struct prefix_ipv6 *p6;
  const struct prefix_mpls *mpls;
} __attribute__ ((transparent_union));

#ifndef IPSTACK_INET_ADDRSTRLEN
#define IPSTACK_INET_ADDRSTRLEN 16
#endif /* IPSTACK_INET_ADDRSTRLEN */

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 46
#endif /* INET6_ADDRSTRLEN */

#ifndef INET6_BUFSIZ
#define INET6_BUFSIZ 51
#endif /* INET6_BUFSIZ */

#ifndef BUFSIZ
#define BUFSIZ 512
#endif /* BUFSIZ */

/* Maximum prefix string length (IPv6) */
#define PREFIX_STRLEN 51

/* Max bit/byte length of IPv4 address. */
#define IPV4_MAX_BYTELEN    4
#define IPV4_MAX_BITLEN    32
#define IPV4_MAX_PREFIXLEN 32
#define IPV4_ADDR_CMP(D,S)   memcmp ((D), (S), IPV4_MAX_BYTELEN)
#define IPV4_ADDR_SAME(D,S)  (memcmp ((D), (S), IPV4_MAX_BYTELEN) == 0)
#define IPV4_ADDR_COPY(D,S)  memcpy ((D), (S), IPV4_MAX_BYTELEN)

#define IPV4_NET0(a)    ((((zpl_uint32) (a)) & 0xff000000) == 0x00000000)
#define IPV4_NET127(a)  ((((zpl_uint32) (a)) & 0xff000000) == 0x7f000000)
#define IPV4_LINKLOCAL(a) ((((zpl_uint32) (a)) & 0xffff0000) == 0xa9fe0000)
#define IPV4_CLASS_DE(a)  ((((zpl_uint32) (a)) & 0xe0000000) == 0xe0000000)
#define IPV4_MULTICAST(a)  ((((zpl_uint32) (a)) & 0xe0000000) == 0xe0000000)

/* Max bit/byte length of IPv6 address. */
#define IPV6_MAX_BYTELEN    16
#define IPV6_MAX_BITLEN    128
#define IPV6_MAX_PREFIXLEN 128
#define IPV6_ADDR_CMP(D,S)   memcmp ((D), (S), IPV6_MAX_BYTELEN)
#define IPV6_ADDR_SAME(D,S)  (memcmp ((D), (S), IPV6_MAX_BYTELEN) == 0)
#define IPV6_ADDR_COPY(D,S)  memcpy ((D), (S), IPV6_MAX_BYTELEN)

/* Count prefix size from mask length */
#define PSIZE(a) (((a) + 7) / (8))

/* Prefix's family member. */
#define PREFIX_FAMILY(p)  ((p)->family)

#define NSM_MAC_IS_MULTICAST(mac)       (((mac[0]) == 0x01)&&((mac[1]) == 0x00)&&((mac[2]) == 0x5e))
#define NSM_MAC_IS_BROADCAST(mac)       ((((mac[0]) & 0xFF)==0XFF)&&(((mac[1]) & 0xFF)==0XFF)&&(((mac[2]) & 0xFF)==0XFF)&&\
												 (((mac[3]) & 0xFF)==0XFF)&&(((mac[4]) & 0xFF)==0XFF)&&(((mac[5]) & 0xFF)==0XFF) )



/* glibc defines s6_addr32 to __in6_u.__u6_addr32 if __USE_{MISC || GNU} */
#ifndef s6_addr32
#if defined(SUNOS_5)
/* Some SunOS define s6_addr32 only to kernel */
#define s6_addr32 _S6_un._S6_u32
#else
#define s6_addr32 __u6_addr.__u6_addr32
#endif /* SUNOS_5 */
#endif /*s6_addr32*/

/* Prototypes. */
extern zpl_family_t str2family(const char *);
extern zpl_family_t afi2family (afi_t);
extern afi_t family2afi (zpl_family_t);
extern const char *safi2str(safi_t safi);
extern const char *afi2str(afi_t afi);

/* Check bit of the prefix. */
extern zpl_uint32  prefix_bit (const zpl_uchar *prefix, const zpl_uchar prefixlen);
extern zpl_uint32  prefix6_bit (const struct ipstack_in6_addr *prefix, const zpl_uchar prefixlen);

extern struct prefix *prefix_new (void);
extern void prefix_free (struct prefix *);
extern const char *prefix_family_str (const struct prefix *);
extern int prefix_blen (const struct prefix *);
extern int str2prefix (const char *, struct prefix *);
extern const char *prefix2str (union prefix46constptr, zpl_char *, zpl_size_t);
extern const char *prefix_2_address_str (union prefix46constptr , zpl_char *, zpl_size_t );
extern int prefix_match (const struct prefix *, const struct prefix *);
extern int prefix_same (const struct prefix *, const struct prefix *);
extern int prefix_cmp (const struct prefix *, const struct prefix *);
extern int prefix_common_bits (const struct prefix *, const struct prefix *);
extern void prefix_copy (struct prefix *dest, const struct prefix *src);
extern void apply_mask (struct prefix *);
extern void prefix_zero (struct prefix *);

extern struct prefix *sockunion2prefix (const union sockunion *dest,
                                        const union sockunion *mask);
extern struct prefix *sockunion2hostprefix (const union sockunion *, struct prefix *p);
extern void prefix2sockunion (const struct prefix *, union sockunion *);

extern int str2prefix_eth (const char *, struct prefix_eth *);
extern int ethaddr_aton_r (const void *addrptr, struct ipstack_ethaddr *ether);

extern struct prefix_ipv4 *prefix_ipv4_new (void);
extern void prefix_ipv4_free (struct prefix_ipv4 *);
extern int str2prefix_ipv4 (const char *, struct prefix_ipv4 *);
extern void apply_mask_ipv4 (struct prefix_ipv4 *);

#define PREFIX_COPY_IPV4(DST, SRC)	\
	*((struct prefix_ipv4 *)(DST)) = *((const struct prefix_ipv4 *)(SRC));

extern int prefix_ipv4_any (const struct prefix_ipv4 *);
extern void apply_classful_mask_ipv4 (struct prefix_ipv4 *);

extern zpl_uchar ip_masklen (struct ipstack_in_addr);
extern void masklen2ip (const int, struct ipstack_in_addr *);
/* returns the network portion of the host address */
extern in_addr_t ipv4_network_addr (in_addr_t hostaddr, zpl_size_t masklen);
/* given the address of a host on a network and the network mask length,
 * calculate the broadcast address for that network;
 * special treatment for /31: returns the address of the other host
 * on the network by flipping the host bit */
extern in_addr_t ipv4_broadcast_addr (in_addr_t hostaddr, zpl_size_t masklen);

extern int netmask_str2prefix_str (const char *, const char *, zpl_char *);

#ifdef ZPL_BUILD_IPV6
extern struct prefix_ipv6 *prefix_ipv6_new (void);
extern void prefix_ipv6_free (struct prefix_ipv6 *);
extern int str2prefix_ipv6 (const char *, struct prefix_ipv6 *);
extern void apply_mask_ipv6 (struct prefix_ipv6 *);

#define PREFIX_COPY_IPV6(DST, SRC)	\
	*((struct prefix_ipv6 *)(DST)) = *((const struct prefix_ipv6 *)(SRC));

extern int ip6_masklen (struct ipstack_in6_addr);
extern void masklen2ip6 (const int, struct ipstack_in6_addr *);

extern void str2in6_addr (const char *, struct ipstack_in6_addr *);
extern const char *inet6_ntoa (struct ipstack_in6_addr);

#endif /* ZPL_BUILD_IPV6 */

extern int all_digit (const char *);

static inline int ipv4_martian (struct ipstack_in_addr *addr)
{
  in_addr_t ip = addr->s_addr;

  if (IPV4_NET0(ip) || IPV4_NET127(ip) || IPV4_CLASS_DE(ip)) {
    return 1;
  }
  return 0;
}

extern int prefix_check_addr (struct prefix *p);
extern const char *inet_address(zpl_uint32 ip);
extern const char *inet_ethernet(zpl_uint8 *mac);
extern const char *cli_inet_ethernet(zpl_uint8 *mac);
extern int cli_ethernet_get(const char *macstr, zpl_uint8 *mac);
extern int cli_ethernet_cmp(zpl_uint8 *smac, zpl_uint8 *dmac);
extern zpl_uint32 get_hostip_byname(zpl_char *hostname);
 
#ifdef __cplusplus
}
#endif

#endif /* __LIB_PREFIX_H */
