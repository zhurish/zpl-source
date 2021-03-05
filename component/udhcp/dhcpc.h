/* vi: set sw=4 ts=4: */
/*
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */
#ifndef UDHCP_DHCPC_H
#define UDHCP_DHCPC_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "dhcp_def.h"
#include "dhcp_option.h"

#define DHCP_DEFAULT_TIMEOUT		5
#define DHCP_DEFAULT_RETRIES		5

#define DHCP_RAW_MODE		1
#define DHCP_UDP_MODE		2

typedef struct client_state_s
{
	ospl_uint16		mode;	/* DHCP_RAW_MODE, DHCP_UDP_MODE*/
	ospl_uint16		state;
	//ospl_uint16		event;
	ospl_uint16 	dis_timeout;
	ospl_uint16 	dis_retries;
	ospl_uint16 	dis_cnt;
	ospl_uint32		renew_timeout1;
	ospl_uint32		renew_timeout2;
	ospl_uint32 	xid;
/*	ospl_uint32  	server_addr;
	ospl_uint32  	requested_ip;*/
	ospl_uint32 		read_bytes;
}client_state_t;

typedef struct client_route_s
{
	NODE			node;
	ifindex_t  		ifindex;
	ospl_uint32 		destination;
	ospl_uint32 		gateway;
}client_route_t;

typedef struct client_lease
{
	ospl_uint32 	lease_address;
	ospl_uint32 	lease_netmask;
	ospl_uint32 	lease_gateway;
	ospl_uint32 	lease_gateway2;
	ospl_uint32 	lease_broadcast;

	ospl_uint32 	lease_dns1;
	ospl_uint32 	lease_dns2;

	leasetime_t expires;
	leasetime_t starts;
	leasetime_t ends;

	ospl_uint32 	lease_timer1;
	ospl_uint32 	lease_timer2;

	ospl_uint32 	lease_log1;
	ospl_uint32 	lease_log2;

	ospl_uint32 	lease_ntp1;
	ospl_uint32 	lease_ntp2;

	ospl_uint32 	lease_sip1;
	ospl_uint32 	lease_sip2;

	ospl_uint8 		lease_ttl;
	ospl_uint16 	lease_mtu;

	LIST 		static_route_list;

	ospl_uint32 	server_address;
	ospl_uint32 	gateway_address;
	ospl_uint32  	ciaddr; /* client IP (if client is in BOUND, RENEW or REBINDING state) */
	/* IP address of next server to use in bootstrap, returned in DHCPOFFER, DHCPACK by server */
	ospl_uint32  	siaddr_nip; /*若 client 需要透过网络开机，从 server 送出之 DHCP OFFER、DHCPACK、DHCPNACK封包中，
							此栏填写开机程序代码所在 server 之地址*/
	ospl_uint32  	gateway_nip; /* relay agent IP address */

	char 		domain_name[UDHCPD_HOSTNAME_MAX];
	char 		tftp_srv_name[UDHCPD_HOSTNAME_MAX];
}client_lease_t PACKED;

typedef struct client_interface_s {
	NODE				node;
	ifindex_t  			ifindex;
	ospl_uint16			port;
	ospl_uint8				client_mac[ETHER_ADDR_LEN];          /* Our mac address */

	int					sock;
	int					udp_sock;

	ospl_uint8				opt_mask[DHCP_OPTION_MAX];      /* Bitmask of options to send (-O option) */
	dhcp_option_set_t	options[DHCP_OPTION_MAX];

	ospl_uint32 			instance;
	void				*master;
	void				*r_thread;	//read thread
	void				*t_thread;	//time thread,
	void				*d_thread;	//discover thread,
	ospl_uint32				first_secs;
	ospl_uint32				last_secs;

	client_state_t		state;
	client_lease_t		lease;

} client_interface_t FIX_ALIASING;

#if 0
#define INIT_SELECTING  0
#define REQUESTING      1
#define BOUND           2
#define RENEWING        3
#define REBINDING       4
/* manually requested renew (SIGUSR1) */
#define RENEW_REQUESTED 5
/* release, possibly manually requested (SIGUSR2) */
#define RELEASED        6
#endif

/** DHCP client states */
#define DHCP_OFF          0
/* initial state: (re)start DHCP negotiation */
#define DHCP_INIT         1
/* discover was sent, DHCPOFFER reply received */
#define DHCP_REQUESTING   2
/* select/renew was sent, DHCPACK reply received */
#define DHCP_BOUND        3
/* half of lease passed, want to renew it by sending unicast renew requests */
#define DHCP_RENEWING     4
/* renew requests were not answered, lease is almost over, send broadcast renew */
#define DHCP_REBINDING    5

#define DHCP_DECLINE    6
#define DHCP_RELEASE    7
#define DHCP_INFORM		8
#define DHCP_CHECKING   9
#define DHCP_REBOOT		10

/************************************************************************************/
extern client_interface_t * dhcp_client_lookup_interface(dhcp_global_t*config, ifindex_t ifindex);
extern int dhcp_client_add_interface(dhcp_global_t*config, ifindex_t ifindex);
extern int dhcp_client_del_interface(dhcp_global_t*config, ifindex_t ifindex);
extern int dhcp_client_interface_clean(void);
extern int dhcp_client_interface_option_set(client_interface_t * ifter, ospl_uint16 code, ospl_uint8 *str, ospl_uint32 len);
extern int dhcp_client_interface_request_set(client_interface_t * ifter, ospl_uint16 code, ospl_bool enable);
/************************************************************************************/



//POP_SAVED_FUNCTION_VISIBILITY
 
#ifdef __cplusplus
}
#endif
 
#endif
