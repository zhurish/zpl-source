/* vi: set sw=4 ts=4: */
/*
 * Russ Dill <Russ.Dill@asu.edu> September 2001
 * Rewritten by Vladimir Oleynik <dzo@simtreas.ru> (C) 2003
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
#ifndef UDHCP_PACKET_H
#define UDHCP_PACKET_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "auto_include.h"
#include <zplos_include.h>
#include "nsm_event.h"
#include "route_types.h"
#include "zmemory.h"
#include "vty.h"
#include "command.h"
#include "if.h"
#include "vrf.h"
#include "prefix.h"
#include "sockopt.h"
#include "nsm_interface.h"
#include "nsm_dhcp.h"

#include "dhcp_config.h"

//TODO: rename ciaddr/yiaddr/chaddr
struct dhcp_packet {
	zpl_uint8 op;      /* DHCP_BOOTREQUEST or DHCP_BOOTREPLY */
	zpl_uint8 htype;   /* hardware address type. 1 = 10mb ethernet */
	zpl_uint8 hlen;    /* hardware address length */
	zpl_uint8 hops;    /* used by relay agents only */
	zpl_uint32  xid;    /* unique id */
	zpl_uint16 secs;   /* elapsed since client began acquisition/renewal */
	zpl_uint16 flags;  /* only one flag so far: */
	zpl_uint32  ciaddr; /* client IP (if client is in BOUND, RENEW or REBINDING state) */
	zpl_uint32  yiaddr; /* 'your' (client) IP address */
	/* IP address of next server to use in bootstrap, returned in DHCP_MESSAGE_OFFER, DHCP_MESSAGE_ACK by server */
	zpl_uint32  siaddr_nip; /*若 client 需要透过网络开机，从 server 送出之 DHCP OFFER、DHCPACK、DHCPNACK封包中，
							此栏填写开机程序代码所在 server 之地址*/
	zpl_uint32  gateway_nip; /* relay agent IP address */
	zpl_uint8 chaddr[DHCP_CHADDR_LEN];   /* link-layer client hardware address (MAC) */
	zpl_uint8 sname[DHCP_SNAME_LEN];    /* server host name (ASCIZ) */
	zpl_uint8 file[DHCP_FILE_LEN];    /* boot file name (ASCIZ) */
	zpl_uint32  cookie;      /* fixed first four option bytes (99,130,83,99 dec) */
	zpl_uint8 options[DHCP_OPTIONS_BUFSIZE];
} PACKED;

struct ip_udp_dhcp_packet {
	struct ipstack_iphdr ip;
	struct ipstack_udphdr udp;
	struct dhcp_packet data;
} PACKED;

struct udp_dhcp_packet {
	struct ipstack_udphdr udp;
	struct dhcp_packet data;
} PACKED;

enum {
	IP_UDP_DHCP_SIZE = sizeof(struct ip_udp_dhcp_packet),
	UDP_DHCP_SIZE    = sizeof(struct udp_dhcp_packet),
	DHCP_SIZE        = sizeof(struct dhcp_packet),
};

/* Let's see whether compiler understood us right */
/*struct BUG_bad_sizeof_struct_ip_udp_dhcp_packet {
	char c[IP_UDP_DHCP_SIZE == 576 ? 1 : -1];
};*/

/* Possible values for flags field... */
#define DHCP_PACKET_FLAG_BROADCAST	0x8000 /* "I need broadcast replies" */
#define DHCP_PACKET_FLAG_UNICAST 	0x0000 /* "I need unicast replies" */
/* Possible values for hardware type (htype) field... */
#define DHCP_HWTYPE_ETHER		1	/* Ethernet			*/
#define DHCP_HWTYPE_IPSEC_TUNNEL	31	/* IPsec Tunnel (RFC3456)	*/

#define DHCP_LEASE_EXPIRES_T1			1//0.5
#define DHCP_LEASE_EXPIRES_T2			0.875
/*** Options ***/



/* DHCP_MESSAGE_TYPE values */
#define DHCP_MESSAGE_DISCOVER            1 /* client -> server */
#define DHCP_MESSAGE_OFFER               2 /* client <- server */
#define DHCP_MESSAGE_REQUEST             3 /* client -> server */
#define DHCP_MESSAGE_DECLINE             4 /* client -> server */
#define DHCP_MESSAGE_ACK                 5 /* client <- server */
#define DHCP_MESSAGE_NAK                 6 /* client <- server */
#define DHCP_MESSAGE_RELEASE             7 /* client -> server */
#define DHCP_MESSAGE_INFORM              8 /* client -> server */

#define DHCP_MESSAGE_MINTYPE DHCP_MESSAGE_DISCOVER
#define DHCP_MESSAGE_MAXTYPE DHCP_MESSAGE_INFORM







/*
 * packet.c
*/

struct udhcp_packet_cmd {
	zpl_uint32  ip;
	zpl_uint16 port;
} PACKED;



//POP_SAVED_FUNCTION_VISIBILITY
 
#ifdef __cplusplus
}
#endif
 
#endif
