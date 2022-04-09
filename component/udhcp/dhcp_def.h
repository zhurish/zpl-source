/* vi: set sw=4 ts=4: */
/*
 * Russ Dill <Russ.Dill@asu.edu> September 2001
 * Rewritten by Vladimir Oleynik <dzo@simtreas.ru> (C) 2003
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
#ifndef UDHCP_DEF_H
#define UDHCP_DEF_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "auto_include.h"
#include <zplos_include.h>
#include "zebra_event.h"
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
//PUSH_AND_SET_FUNCTION_VISIBILITY_TO_HIDDEN

/* Defaults you may want to tweak */
/* Default max_lease_sec */
//#define DEFAULT_LEASE_TIME      (60*60*24 * 10)
//#define LEASES_FILE             CONFIG_DHCPD_LEASES_FILE
/* Where to find the DHCP server configuration file */
//#define DHCPD_CONF_FILE         "/etc/udhcpd.conf"


/*** DHCP packet ***/

/* DHCP protocol. See RFC 2131 */
#define DHCP_MAGIC              0x63825363
#define BOOTREQUEST             1
#define BOOTREPLY               2

/* Sizes for BOOTP options */

//#define	DHCP_VEND_LEN		 	64
#define	DHCP_CHADDR_LEN	 		16
/*
#define DHCP_PKT_SNAME_LEN      64
#define DHCP_PKT_FILE_LEN      128
#define DHCP_PKT_SNAME_LEN_STR "64"
#define DHCP_PKT_FILE_LEN_STR "128"
*/

#define DHCP_UDP_OVERHEAD	(20 + /* IP header */ 8)   /* UDP header */
#define DHCP_SNAME_LEN		64
#define DHCP_FILE_LEN		128
#define DHCP_FIXED_NON_UDP	236
#define DHCP_FIXED_LEN		(DHCP_FIXED_NON_UDP + DHCP_UDP_OVERHEAD)
						/* Everything but options. */
#define DHCP_MTU_MAX		1500
#define DHCP_OPTION_LEN		(DHCP_MTU_MAX - DHCP_FIXED_LEN)

#define BOOTP_MIN_LEN		300
#define DHCP_OPTIONS_BUFSIZE DHCP_OPTION_LEN


//TODO: rename ciaddr/yiaddr/chaddr
struct dhcp_packet {
	zpl_uint8 op;      /* BOOTREQUEST or BOOTREPLY */
	zpl_uint8 htype;   /* hardware address type. 1 = 10mb ethernet */
	zpl_uint8 hlen;    /* hardware address length */
	zpl_uint8 hops;    /* used by relay agents only */
	zpl_uint32  xid;    /* unique id */
	zpl_uint16 secs;   /* elapsed since client began acquisition/renewal */
	zpl_uint16 flags;  /* only one flag so far: */
	zpl_uint32  ciaddr; /* client IP (if client is in BOUND, RENEW or REBINDING state) */
	zpl_uint32  yiaddr; /* 'your' (client) IP address */
	/* IP address of next server to use in bootstrap, returned in DHCPOFFER, DHCPACK by server */
	zpl_uint32  siaddr_nip; /*若 client 需要透过网络开机，从 server 送出之 DHCP OFFER、DHCPACK、DHCPNACK封包中，
							此栏填写开机程序代码所在 server 之地址*/
	zpl_uint32  gateway_nip; /* relay agent IP address */
	zpl_uint8 chaddr[DHCP_CHADDR_LEN];   /* link-layer client hardware address (MAC) */
	zpl_uint8 sname[DHCP_SNAME_LEN];    /* server host name (ASCIZ) */
	zpl_uint8 file[DHCP_FILE_LEN];    /* boot file name (ASCIZ) */
	zpl_uint32  cookie;      /* fixed first four option bytes (99,130,83,99 dec) */
	zpl_uint8 options[DHCP_OPTIONS_BUFSIZE + CONFIG_UDHCPC_SLACK_FOR_BUGGY_SERVERS];
} PACKED;

struct ip_udp_dhcp_packet {
	struct iphdr ip;
	struct udphdr udp;
	struct dhcp_packet data;
} PACKED;

struct udp_dhcp_packet {
	struct udphdr udp;
	struct dhcp_packet data;
} PACKED;

enum {
	IP_UDP_DHCP_SIZE = sizeof(struct ip_udp_dhcp_packet) - CONFIG_UDHCPC_SLACK_FOR_BUGGY_SERVERS,
	UDP_DHCP_SIZE    = sizeof(struct udp_dhcp_packet) - CONFIG_UDHCPC_SLACK_FOR_BUGGY_SERVERS,
	DHCP_SIZE        = sizeof(struct dhcp_packet) - CONFIG_UDHCPC_SLACK_FOR_BUGGY_SERVERS,
};

/* Let's see whether compiler understood us right */
/*struct BUG_bad_sizeof_struct_ip_udp_dhcp_packet {
	char c[IP_UDP_DHCP_SIZE == 576 ? 1 : -1];
};*/
/* Possible values for flags field... */
#define BROADCAST_FLAG	0x8000 /* "I need broadcast replies" */
#define UNICAST_FLAG 	0x0000 /* "I need unicast replies" */
/* Possible values for hardware type (htype) field... */
#define HTYPE_ETHER		1	/* Ethernet			*/
#define HTYPE_IPSEC_TUNNEL	31	/* IPsec Tunnel (RFC3456)	*/

#define DHCP_T1			1//0.5
#define DHCP_T2			0.875
/*** Options ***/

enum {
	OPTION_IP = 1,
	OPTION_IP_PAIR,
	OPTION_STRING,
	/* Opts of STRING_HOST type will be sanitized before they are passed
	 * to udhcpc script's environment: */
	OPTION_STRING_HOST,
	OPTION_BOOLEAN,
	OPTION_U8,
	OPTION_U16,
	OPTION_S16,
	OPTION_U32,
	OPTION_S32,
	OPTION_BIN,
	OPTION_STATIC_ROUTES,
	OPTION_6RD,
#if ENABLE_FEATURE_UDHCP_RFC3397 || ENABLE_FEATURE_UDHCPC6_RFC3646 || ENABLE_FEATURE_UDHCPC6_RFC4704
	OPTION_DNS_STRING,  /* RFC1035 compressed domain name list */
#endif
#if ENABLE_FEATURE_UDHCP_RFC3397
	OPTION_SIP_SERVERS,
#endif

	OPTION_TYPE_MASK = 0x0f,
	/* Client requests this option by default */
	OPTION_REQ  = 0x10,
	/* There can be a list of 1 or more of these */
	OPTION_LIST = 0x20,
};

/* DHCP option codes (partial list). See RFC 2132 and
 * http://www.iana.org/assignments/bootp-dhcp-parameters/
 * Commented out options are handled by common option machinery,
 * uncommented ones have special cases (grep for them to see).
 */
#define DHCP_PADDING            0x00
#define DHCP_SUBNET             0x01
#define DHCP_TIME_OFFSET      0x02 /* (localtime - UTC_time) in seconds. signed */
#define DHCP_ROUTER           0x03
#define DHCP_TIME_SERVER      0x04 /* RFC 868 time server (32-bit, 0 = 1.1.1900) */
#define DHCP_NAME_SERVER      0x05 /* IEN 116 _really_ ancient kind of NS */
#define DHCP_DNS_SERVER       0x06
#define DHCP_LOG_SERVER       0x07 /* port 704 UDP log (not syslog)*/
#define DHCP_COOKIE_SERVER    0x08 /* "quote of the day" server */
#define DHCP_LPR_SERVER       0x09
#define DHCP_HOST_NAME          0x0c /* either client informs server or server gives name to client */
#define DHCP_BOOT_SIZE        0x0d
#define DHCP_DOMAIN_NAME      0x0f /* server gives domain suffix */
#define DHCP_SWAP_SERVER      0x10
#define DHCP_ROOT_PATH        0x11
#define DHCP_IP_TTL           0x17
#define DHCP_MTU              0x1a
#define DHCP_BROADCAST        0x1c
#define DHCP_ROUTES           0x21
#define DHCP_NIS_DOMAIN       0x28
#define DHCP_NIS_SERVER       0x29
#define DHCP_NTP_SERVER       0x2a
#define DHCP_WINS_SERVER      0x2c
#define DHCP_REQUESTED_IP       0x32 /* sent by client if specific IP is wanted */
#define DHCP_LEASE_TIME         0x33
#define DHCP_OPTION_OVERLOAD    0x34
#define DHCP_MESSAGE_TYPE       0x35
#define DHCP_SERVER_ID          0x36 /* by default server's IP */
#define DHCP_PARAM_REQ          0x37 /* list of options client wants */
#define DHCP_ERR_MESSAGE      0x38 /* error message when sending NAK etc */
#define DHCP_MAX_SIZE           0x39
#define DHCP_RENEWAL_TIME         0x3a	/* renewal time */
#define DHCP_REBINDING_TIME         0x3b	/* rebinding time */
#define DHCP_VENDOR             0x3c /* client's vendor (a string) option 60*/
#define DHCP_CLIENT_ID          0x3d /* by default client's MAC addr, but may be arbitrarily long */
#define DHCP_TFTP_SERVER_NAME 0x42 /* same as 'sname' field */
#define DHCP_BOOT_FILE        0x43 /* same as 'file' field */
#define DHCP_USER_CLASS       0x4d /* RFC 3004. set of LASCII strings. "I am a printer" etc */
#define DHCP_FQDN               0x51 /* client asks to update DNS to map its FQDN to its new IP */
#define DHCP_OPT82            0x52 /* client asks to update DNS to map its FQDN to its new IP */
#define DHCP_DOMAIN_SEARCH    0x77 /* RFC 3397. set of ASCIZ string, DNS-style compressed */
#define DHCP_SIP_SERVERS      0x78 /* RFC 3361. flag byte, then: 0: domain names, 1: IP addrs */
#define DHCP_STATIC_ROUTES    0x79 /* RFC 3442. (mask,ip,router) tuples */
#define DHCP_VLAN_ID          0x84 /* 802.1P VLAN ID */
#define DHCP_VLAN_PRIORITY    0x85 /* 802.1Q VLAN priority */
#define DHCP_PXE_CONF_FILE    0xd1 /* RFC 5071 Configuration File */
#define DHCP_PXE_PATH_PREFIX  0xd2 /* RFC 5071 Configuration File */
#define DHCP_MS_STATIC_ROUTES 0xf9 /* Microsoft's pre-RFC 3442 code for 0x79? */
#define DHCP_WPAD             0xfc /* MSIE's Web Proxy Autodiscovery Protocol */
#define DHCP_END                0xff

/* Offsets in option byte sequence */
#define OPT_CODE                0
#define OPT_LEN                 1
#define OPT_DATA                2
/* Bits in "overload" option */
#define OPTION_FIELD            0
#define FILE_FIELD              1
#define SNAME_FIELD             2

/* DHCP_MESSAGE_TYPE values */
#define DHCPDISCOVER            1 /* client -> server */
#define DHCPOFFER               2 /* client <- server */
#define DHCPREQUEST             3 /* client -> server */
#define DHCPDECLINE             4 /* client -> server */
#define DHCPACK                 5 /* client <- server */
#define DHCPNAK                 6 /* client <- server */
#define DHCPRELEASE             7 /* client -> server */
#define DHCPINFORM              8 /* client -> server */

#define DHCP_MINTYPE DHCPDISCOVER
#define DHCP_MAXTYPE DHCPINFORM


#define OPTION_61_MAC           1
#define OPTION_61_UUID          0x02
#define OPTION_61_IAID          0xFF
/*0x01代表后面是硬件地址，
    0xff表示IAID，
    0x0002表示后面是DUID*/

struct dhcp_optflag {
	zpl_uint8 flags;
	zpl_uint8 code;
};

/*
struct option_set {
	zpl_uint8 *data;
	struct option_set *next;
};
*/


#if ENABLE_FEATURE_UDHCP_PORT
#define DHCP_SERVER_PORT  (server_config.port)
#define DHCP_SERVER_PORT6 (server_config.port)
#else
#define DHCP_SERVER_PORT  67
#define DHCP_SERVER_PORT6 547
#endif
#ifndef DHCP_CLIENT_PORT
#define DHCP_CLIENT_PORT  68
#endif
#ifndef DHCP_CLIENT_PORT6
#define DHCP_CLIENT_PORT6 546
#endif


#define DHCPC_DEBUG_STATE			0X0001
#define DHCPC_DEBUG_EVENT			0X0002
#define DHCPC_DEBUG_SEND			0X0004
#define DHCPC_DEBUG_RECV			0X0008
#define DHCPC_DEBUG_KERNEL			0X0010
#define DHCPC_DEBUG_DETAIL			0X0020

#define DHCPC_DEBUG_ON(n)		(dhcp_global_config.client_debug |= DHCPC_DEBUG_## n)
#define DHCPC_DEBUG_OFF(n)		(dhcp_global_config.client_debug &= ~DHCPC_DEBUG_## n)
#define DHCPC_DEBUG_ISON(n)		(dhcp_global_config.client_debug & DHCPC_DEBUG_## n)


typedef struct dhcp_global_s
{
	zpl_uint32		task_id;
	zpl_bool	init;
	LIST 	pool_list;
	LIST 	client_list;
	LIST 	relay_list;

	void	*eloop_master;
	void	*r_thread;

	zpl_uint16 server_port;
	zpl_uint16 client_port;

	zpl_uint16 server_port_v6;
	zpl_uint16 client_port_v6;

	zpl_socket_t		sock;		//udp socket, just for server
	zpl_socket_t		rawsock;	//raw socket, just for server send MSG to client

	zpl_socket_t		sock_v6;
	zpl_socket_t		rawsock_v6;

	zpl_socket_t		client_sock;		//udp socket, just for client
	zpl_uint32		client_cnt;
	zpl_uint32		client_debug;
}dhcp_global_t;

extern dhcp_global_t dhcp_global_config;
/*
 * common.c
*/

#if ENABLE_UDHCPC || ENABLE_UDHCPD
//extern const struct dhcp_optflag dhcp_optflags[];
//extern const char dhcp_option_strings[] ALIGN1;
#endif
//extern const zpl_uint8 dhcp_option_lengths[] ALIGN1;

//unsigned FAST_FUNC udhcp_option_idx(const char *name, const char *option_strings);
//zpl_uint32 FAST_FUNC udhcp_option_idx(const int opc);


//void udhcp_add_binary_option(struct dhcp_packet *packet, zpl_uint8 *addopt) FAST_FUNC;
#if ENABLE_UDHCPC || ENABLE_UDHCPD
//void udhcp_add_simple_option(struct dhcp_packet *packet, zpl_uint8 code, zpl_uint32  data) FAST_FUNC;
#endif

//struct option_set *udhcp_find_option(struct option_set *opt_list, zpl_uint8 code) FAST_FUNC;


// RFC 2131  Table 5: Fields and options used by DHCP clients
//
// Fields 'hops', 'yiaddr', 'siaddr', 'giaddr' are always zero
//
// Field      DHCPDISCOVER          DHCPINFORM            DHCPREQUEST           DHCPDECLINE         DHCPRELEASE
// -----      ------------          ------------          -----------           -----------         -----------
// 'xid'      selected by client    selected by client    'xid' from server     selected by client  selected by client
//                                                        DHCPOFFER message
// 'secs'     0 or seconds since    0 or seconds since    0 or seconds since    0                   0
//            DHCP process started  DHCP process started  DHCP process started
// 'flags'    Set 'BROADCAST'       Set 'BROADCAST'       Set 'BROADCAST'       0                   0
//            flag if client        flag if client        flag if client
//            requires broadcast    requires broadcast    requires broadcast
//            reply                 reply                 reply
// 'ciaddr'   0                     client's IP           0 or client's IP      0                   client's IP
//                                                        (BOUND/RENEW/REBIND)
// 'chaddr'   client's MAC          client's MAC          client's MAC          client's MAC        client's MAC
// 'sname'    options or sname      options or sname      options or sname      (unused)            (unused)
// 'file'     options or file       options or file       options or file       (unused)            (unused)
// 'options'  options               options               options               message type opt    message type opt
//
// Option                     DHCPDISCOVER  DHCPINFORM  DHCPREQUEST      DHCPDECLINE  DHCPRELEASE
// ------                     ------------  ----------  -----------      -----------  -----------
// Requested IP address       MAY           MUST NOT    MUST (in         MUST         MUST NOT
//                                                      SELECTING or
//                                                      INIT-REBOOT)
//                                                      MUST NOT (in
//                                                      BOUND or
//                                                      RENEWING)
// IP address lease time      MAY           MUST NOT    MAY              MUST NOT     MUST NOT
// Use 'file'/'sname' fields  MAY           MAY         MAY              MAY          MAY
// Client identifier          MAY           MAY         MAY              MAY          MAY
// Vendor class identifier    MAY           MAY         MAY              MUST NOT     MUST NOT
// Server identifier          MUST NOT      MUST NOT    MUST (after      MUST         MUST
//                                                      SELECTING)
//                                                      MUST NOT (after
//                                                      INIT-REBOOT,
//                                                      BOUND, RENEWING
//                                                      or REBINDING)
// Parameter request list     MAY           MAY         MAY              MUST NOT     MUST NOT
// Maximum message size       MAY           MAY         MAY              MUST NOT     MUST NOT
// Message                    SHOULD NOT    SHOULD NOT  SHOULD NOT       SHOULD       SHOULD
// Site-specific              MAY           MAY         MAY              MUST NOT     MUST NOT
// All others                 MAY           MAY         MAY              MUST NOT     MUST NOT


/*** Logging ***/
#if 0
#if defined CONFIG_UDHCP_DEBUG && CONFIG_UDHCP_DEBUG >= 1
# define IF_UDHCP_VERBOSE(...) __VA_ARGS__
extern unsigned dhcp_verbose;
# define log1(...) do { if (dhcp_verbose >= 1) bb_error_msg(__VA_ARGS__); } while (0)
# if CONFIG_UDHCP_DEBUG >= 2
void udhcp_dump_packet(struct dhcp_packet *packet) FAST_FUNC;
#  define log2(...) do { if (dhcp_verbose >= 2) bb_error_msg(__VA_ARGS__); } while (0)
# else
#  define udhcp_dump_packet(...) ((void)0)
#  define log2(...) ((void)0)
# endif
# if CONFIG_UDHCP_DEBUG >= 3
#  define log3(...) do { if (dhcp_verbose >= 3) bb_error_msg(__VA_ARGS__); } while (0)
# else
#  define log3(...) ((void)0)
# endif
#else
# define IF_UDHCP_VERBOSE(...)
//# define udhcp_dump_packet(...) ((void)0)
# define log1(...) ((void)0)
# define log2(...) ((void)0)
# define log3(...) ((void)0)
#endif
#endif

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
