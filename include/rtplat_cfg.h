/*
 * rtplat_cfg.h
 *
 *  Created on: Jan 8, 2018
 *      Author: zhurish
 */

#ifndef __RTPLAT_CFG_H__
#define __RTPLAT_CFG_H__

#ifdef __cplusplus
extern "C" {
#endif



/* 
 * IP_HDRINCL / struct ip byte order
 *
 * Linux: network byte order
 * *BSD: network, except for length and offset. (cf Stevens)
 * SunOS: nominally as per BSD. but bug: network order on LE.
 * OpenBSD: network byte order, apart from older versions which are as per 
 *          *BSD
 */
#if defined(__NetBSD__) \
   || (defined(__FreeBSD__) && (__FreeBSD_version < 1100030)) \
   || (defined(__OpenBSD__) && (OpenBSD < 200311)) \
   || (defined(__APPLE__)) \
   || (defined(SUNOS_5) && defined(WORDS_BIGENDIAN))
#define HAVE_IP_HDRINCL_BSD_ORDER
#endif


#define ZEBRA_NUM_OF(x) (sizeof (x) / sizeof (x[0]))


/* default zebra TCP port for zclient */
#define ZEBRA_PORT			2600

/* Zebra message types. */
#define ZEBRA_HELLO			               0
#define ZEBRA_INTERFACE_ADD                1
#define ZEBRA_INTERFACE_DELETE             2
#define ZEBRA_INTERFACE_ADDRESS_ADD        3
#define ZEBRA_INTERFACE_ADDRESS_DELETE     4
#define ZEBRA_INTERFACE_UP                 5
#define ZEBRA_INTERFACE_DOWN               6
#define ZEBRA_INTERFACE_MODE               7
#define ZEBRA_INTERFACE_RENAME             8


#define ZEBRA_ROUTER_ID_ADD               20
#define ZEBRA_ROUTER_ID_DELETE            21
#define ZEBRA_ROUTER_ID_UPDATE            22

#define ZEBRA_IPV4_ROUTE_ADD              30
#define ZEBRA_IPV4_ROUTE_DELETE           31
#define ZEBRA_IPV6_ROUTE_ADD              32
#define ZEBRA_IPV6_ROUTE_DELETE           33

#define ZEBRA_REDISTRIBUTE_ADD            40
#define ZEBRA_REDISTRIBUTE_DELETE         41
#define ZEBRA_REDISTRIBUTE_DEFAULT_ADD    42
#define ZEBRA_REDISTRIBUTE_DEFAULT_DELETE 43

#define ZEBRA_IPV4_NEXTHOP_LOOKUP         50
#define ZEBRA_IPV6_NEXTHOP_LOOKUP         51
#define ZEBRA_IPV4_IMPORT_LOOKUP          52
#define ZEBRA_IPV6_IMPORT_LOOKUP          53
#define ZEBRA_IPV4_NEXTHOP_LOOKUP_MRIB    54

#define ZEBRA_NEXTHOP_REGISTER            60
#define ZEBRA_NEXTHOP_UNREGISTER          61
#define ZEBRA_NEXTHOP_UPDATE              62

#define ZEBRA_VRF_REGISTER                70
#define ZEBRA_VRF_UNREGISTER              71

#define ZEBRA_MESSAGE_MAX                 90

/* Marker value used in new Zserv, in the byte location corresponding
 * the command value in the old nsm_zserv.header. To allow old and new
 * Zserv headers to be distinguished from each other.
 */
#define ZEBRA_HEADER_MARKER              255




/* Error codes of zebra. */
#define ZEBRA_ERR_NOERROR                0
#define ZEBRA_ERR_RTEXIST               -1
#define ZEBRA_ERR_RTUNREACH             -2
#define ZEBRA_ERR_EPERM                 -3
#define ZEBRA_ERR_RTNOEXIST             -4
#define ZEBRA_ERR_KERNEL                -5

/* Zebra message flags */
#define ZEBRA_FLAG_INTERNAL           0x01
#define ZEBRA_FLAG_SELFROUTE          0x02
#define ZEBRA_FLAG_BLACKHOLE          0x04
#define ZEBRA_FLAG_IBGP               0x08
#define ZEBRA_FLAG_SELECTED           0x10
#define ZEBRA_FLAG_FIB_OVERRIDE       0x20
#define ZEBRA_FLAG_STATIC             0x40
#define ZEBRA_FLAG_REJECT             0x80

/* Zebra nexthop flags. */
#define ZEBRA_NEXTHOP_IFINDEX            1
#define ZEBRA_NEXTHOP_IFNAME             2
#define ZEBRA_NEXTHOP_IPV4               3
#define ZEBRA_NEXTHOP_IPV4_IFINDEX       4
#define ZEBRA_NEXTHOP_IPV4_IFNAME        5
#define ZEBRA_NEXTHOP_IPV6               6
#define ZEBRA_NEXTHOP_IPV6_IFINDEX       7
#define ZEBRA_NEXTHOP_IPV6_IFNAME        8
#define ZEBRA_NEXTHOP_BLACKHOLE          9



/* Default Administrative Distance of each protocol. */
#define ZEBRA_KERNEL_DISTANCE_DEFAULT      0
#define ZEBRA_CONNECT_DISTANCE_DEFAULT     0
#define ZEBRA_STATIC_DISTANCE_DEFAULT      1
#define ZEBRA_RIP_DISTANCE_DEFAULT       120
#define ZEBRA_RIPNG_DISTANCE_DEFAULT     120
#define ZEBRA_OSPF_DISTANCE_DEFAULT      110
#define ZEBRA_OSPF6_DISTANCE_DEFAULT     110
#define ZEBRA_ISIS_DISTANCE_DEFAULT      115
#define ZEBRA_IBGP_DISTANCE_DEFAULT      200
#define ZEBRA_EBGP_DISTANCE_DEFAULT       20


/* For old definition. */
#ifndef IN6_ARE_ADDR_EQUAL
#define IN6_ARE_ADDR_EQUAL IN6_IS_ADDR_EQUAL
#endif /* IN6_ARE_ADDR_EQUAL */

/* 
 * OSPF Fragmentation / fragmented writes
 *
 * ospfd can support writing fragmented packets, for cases where
 * kernel will not fragment IP_HDRINCL and/or multicast destined
 * packets (ie TTBOMK all kernels, BSD, SunOS, Linux). However,
 * SunOS, probably BSD too, clobber the user supplied IP ID and IP
 * flags fields, hence user-space fragmentation will not work.
 * Only Linux is known to leave IP header unmolested.
 * Further, fragmentation really should be done the kernel, which already
 * supports it, and which avoids nasty IP ID state problems.
 *
 * Fragmentation of OSPF packets can be required on networks with router
 * with many many interfaces active in one area, or on networks with links
 * with low MTUs.
 */
#ifdef GNU_LINUX
#define WANT_OSPF_WRITE_FRAGMENT
#endif

#ifndef INADDR_LOOPBACK
#define	INADDR_LOOPBACK	0x7f000001	/* Internet address 127.0.0.1.  */
#endif


/* Address family numbers from RFC1700. */
typedef enum {
  AFI_IP  = 1,
  AFI_IP6 = 2,
  AFI_ETHER = 3,                /* RFC 1700 has "6" for 802.* */
#define AFI_MAX 4
} afi_t;

/* Subsequent Address Family Identifier. */
#define SAFI_UNICAST              1
#define SAFI_MULTICAST            2
#define SAFI_RESERVED_3           3
#define SAFI_MPLS_VPN             4
#define SAFI_ENCAP		  7 /* per IANA */
#define SAFI_MAX                  8




#ifdef HAVE_BROKEN_CMSG_FIRSTHDR
/* This bug is present in Solaris 8 and pre-patch Solaris 9 <sys/socket.h>;
   please refer to http://bugzilla.quagga.net/show_bug.cgi?id=142 */

/* Check that msg_controllen is large enough. */
#define ZCMSG_FIRSTHDR(mhdr) \
  (((size_t)((mhdr)->msg_controllen) >= sizeof(struct cmsghdr)) ? \
   CMSG_FIRSTHDR(mhdr) : (struct cmsghdr *)NULL)

#warning "CMSG_FIRSTHDR is broken on this platform, using a workaround"

#else /* HAVE_BROKEN_CMSG_FIRSTHDR */
#define ZCMSG_FIRSTHDR(M) CMSG_FIRSTHDR(M)
#endif /* HAVE_BROKEN_CMSG_FIRSTHDR */



/* 
 * RFC 3542 defines several macros for using struct cmsghdr.
 * Here, we define those that are not present
 */

/*
 * Internal defines, for use only in this file.
 * These are likely wrong on other than ILP32 machines, so warn.
 */
#ifndef _CMSG_DATA_ALIGN
#define _CMSG_DATA_ALIGN(n)           (((n) + 3) & ~3)
#endif /* _CMSG_DATA_ALIGN */

#ifndef _CMSG_HDR_ALIGN
#define _CMSG_HDR_ALIGN(n)            (((n) + 3) & ~3)
#endif /* _CMSG_HDR_ALIGN */

/*
 * CMSG_SPACE and CMSG_LEN are required in RFC3542, but were new in that
 * version.
 */
#ifndef CMSG_SPACE
#define CMSG_SPACE(l)       (_CMSG_DATA_ALIGN(sizeof(struct cmsghdr)) + \
                              _CMSG_HDR_ALIGN(l))
#warning "assuming 4-byte alignment for CMSG_SPACE"
#endif  /* CMSG_SPACE */


#ifndef CMSG_LEN
#define CMSG_LEN(l)         (_CMSG_DATA_ALIGN(sizeof(struct cmsghdr)) + (l))
#warning "assuming 4-byte alignment for CMSG_LEN"
#endif /* CMSG_LEN */


/*  The definition of struct in_pktinfo is missing in old version of
    GLIBC 2.1 (Redhat 6.1).  */
#if defined (GNU_LINUX) && ! defined (HAVE_STRUCT_IN_PKTINFO)
struct in_pktinfo
{
  int ipi_ifindex;
  struct in_addr ipi_spec_dst;
  struct in_addr ipi_addr;
};

/*struct in6_pktinfo
  {
    struct in6_addr ipi6_addr;
    ospl_uint32 ipi6_ifindex;
  };*/
#endif





/* Zebra route's types are defined in route_types.h */
#ifdef PL_NSM_MODULE
#include "route_types.h"
#endif
/* Note: whenever a new route-type or zserv-command is added the
 * corresponding {command,route}_types[] table in lib/log.c MUST be
 * updated! */

/* Map a route type to a string.  For example, ZEBRA_ROUTE_RIPNG -> "ripng". */
extern const char *zebra_route_string(ospl_uint32 route_type);
/* Map a route type to a char.  For example, ZEBRA_ROUTE_RIPNG -> 'R'. */
extern char zebra_route_char(ospl_uint32 route_type);
/* Map a zserv command type to the same string, 
 * e.g. ZEBRA_INTERFACE_ADD -> "ZEBRA_INTERFACE_ADD" */
/* Map a protocol name to its number. e.g. ZEBRA_ROUTE_BGP->9*/
extern ospl_proto_t proto_name2num(const char *s);
/* Map redistribute X argument to protocol number.
 * unlike proto_name2num, this accepts ospl_int16hands and takes
 * an AFI value to restrict input */
extern ospl_proto_t proto_redistnum(ospl_uint16 afi, const char *s);

extern const char *zserv_command_string (ospl_uint32 command);




#ifdef __cplusplus
}
#endif

#endif /* __RTPLAT_CFG_H__ */
