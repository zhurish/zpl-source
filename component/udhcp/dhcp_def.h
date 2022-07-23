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
#define DHCP_BOOTREQUEST             1
#define DHCP_BOOTREPLY               2

/* Sizes for BOOTP options */
#define	DHCP_CHADDR_LEN	 		16


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
	zpl_taskid_t		task_id;
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

	zpl_socket_t	sock;		//udp socket, just for server
	zpl_socket_t	rawsock;	//raw socket, just for server send MSG to client

	zpl_socket_t	sock_v6;
	zpl_socket_t	rawsock_v6;

	zpl_socket_t	client_sock;		//udp socket, just for client
	zpl_uint32		client_cnt;
	zpl_uint32		client_debug;
}dhcp_global_t;

extern dhcp_global_t dhcp_global_config;


#ifdef __cplusplus
}
#endif
 
#endif
