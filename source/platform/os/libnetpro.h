#ifndef __ZPL_LIBNETPRO_H__
#define __ZPL_LIBNETPRO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/if_ether.h> 

typedef enum ip_protocol_s
{
    IP_IPPROTO_ANY  = 260,
    IP_IPPROTO_IP  = 0,	   /* Dummy protocol for TCP.  */
    IP_IPPROTO_ICMP  = 1,	   /* Internet Control Message Protocol.  */
    IP_IPPROTO_IGMP  = 2,	   /* Internet Group Management Protocol. */
    IP_IPPROTO_IPIP  = 4,	   /* IPIP tunnels (older KA9Q tunnels use 94).  */
    IP_IPPROTO_TCP  = 6,	   /* Transmission Control Protocol.  */
    IP_IPPROTO_EGP  = 8,	   /* Exterior Gateway Protocol.  */
    IP_IPPROTO_PUP  = 12,	   /* PUP protocol.  */
    IP_IPPROTO_UDP  = 17,	   /* User Datagram Protocol.  */
    IP_IPPROTO_IDP  = 22,	   /* XNS IDP protocol.  */
    IP_IPPROTO_TP  = 29,	   /* SO Transport Protocol Class 4.  */
    IP_IPPROTO_DCCP  = 33,	   /* Datagram Congestion Control Protocol.  */
    IP_IPPROTO_IPV6  = 41,     /* IPv6 header.  */
    IP_IPPROTO_RSVP  = 46,	   /* Reservation Protocol.  */
    IP_IPPROTO_GRE  = 47,	   /* General Routing Encapsulation.  */
    IP_IPPROTO_ESP  = 50,      /* encapsulating security payload.  */
    IP_IPPROTO_AH  = 51,       /* authentication header.  */
    IP_IPPROTO_MTP  = 92,	   /* Multicast Transport Protocol.  */
    IP_IPPROTO_BEETPH  = 94,   /* IP option pseudo header for BEET.  */
    IP_IPPROTO_ENCAP  = 98,	   /* Encapsulation Header.  */
    IP_IPPROTO_PIM  = 103,	   /* Protocol Independent Multicast.  */
    IP_IPPROTO_COMP  = 108,	   /* Compression Header Protocol.  */
    IP_IPPROTO_SCTP  = 132,	   /* Stream Control Transmission Protocol.  */
    IP_IPPROTO_UDPLITE  = 136, /* UDP-Lite protocol.  */
    IP_IPPROTO_MPLS  = 137,    /* MPLS in IP.  */
    IP_IPPROTO_RAW  = 255,	   /* Raw IP packets.  */
    IP_IPPROTO_OSPF = 89,
    IP_IPPROTO_MAX
}ip_protocol_e;




/*
-gt	大于
-lt	小于
-ge	大于或等于
-le	小于或等于
-eq 等于
-ne 不等于
-rn <>
*/
enum operation
{
  OPT_NONE,
  OPT_EQ,
  OPT_NE,
  OPT_GT,
  OPT_LT,
  OPT_GE,
  OPT_LE,
  OPT_RN,
  OPT_MAX,
};

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


extern zpl_uint16 ip_protocol_type(const char *str);
extern const char * ip_protocol_type_string(zpl_uint16 type);
extern zpl_uint16 eth_protocol_type(const char *str);
extern const char * eth_protocol_type_string(zpl_uint16 type);
extern zpl_uint16 port_operation_type(const char *str);
extern const char * port_operation_type_string(zpl_uint16 type);


extern zpl_uint16 eth_l2protocol_type(const char *smac);
extern const char *eth_l2protocol_type_string(zpl_uint16 type);

#ifdef __cplusplus
}
#endif

#endif /* __ZPL_LIBNETPRO_H__ */