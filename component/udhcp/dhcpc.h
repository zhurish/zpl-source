/* vi: set sw=4 ts=4: */
/*
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */
#ifndef UDHCP_DHCPC_H
#define UDHCP_DHCPC_H 1

#include "dhcp_def.h"
#include "dhcp_option.h"

#define DHCP_DEFAULT_TIMEOUT		5
#define DHCP_DEFAULT_RETRIES		5

#define DHCP_RAW_MODE		1
#define DHCP_UDP_MODE		2

typedef struct client_state_s
{
	u_int16		mode;	/* DHCP_RAW_MODE, DHCP_UDP_MODE*/
	u_int16		state;
	//u_int16		event;
	u_int16 	dis_timeout;
	u_int16 	dis_retries;
	u_int16 	dis_cnt;
	u_int32		renew_timeout1;
	u_int32		renew_timeout2;
	u_int32 	xid;
/*	uint32_t 	server_addr;
	uint32_t 	requested_ip;*/
	int 		read_bytes;
}client_state_t;

typedef struct client_route_s
{
	NODE			node;
	uint32_t 		ifindex;
	u_int32 		destination;
	u_int32 		gateway;
}client_route_t;

typedef struct client_lease
{
	u_int32 	lease_address;
	u_int32 	lease_netmask;
	u_int32 	lease_gateway;
	u_int32 	lease_gateway2;
	u_int32 	lease_broadcast;

	u_int32 	lease_dns1;
	u_int32 	lease_dns2;

	leasetime_t expires;
	leasetime_t starts;
	leasetime_t ends;

	u_int32 	lease_timer1;
	u_int32 	lease_timer2;

	u_int32 	lease_log1;
	u_int32 	lease_log2;

	u_int32 	lease_ntp1;
	u_int32 	lease_ntp2;

	u_int32 	lease_sip1;
	u_int32 	lease_sip2;

	u_int8 		lease_ttl;
	u_int16 	lease_mtu;

	LIST 		static_route_list;

	u_int32 	server_address;
	u_int32 	gateway_address;
	uint32_t 	ciaddr; /* client IP (if client is in BOUND, RENEW or REBINDING state) */
	/* IP address of next server to use in bootstrap, returned in DHCPOFFER, DHCPACK by server */
	uint32_t 	siaddr_nip; /*若 client 需要透过网络开机，从 server 送出之 DHCP OFFER、DHCPACK、DHCPNACK封包中，
							此栏填写开机程序代码所在 server 之地址*/
	uint32_t 	gateway_nip; /* relay agent IP address */

	char 		domain_name[UDHCPD_HOSTNAME_MAX];
	char 		tftp_srv_name[UDHCPD_HOSTNAME_MAX];
}client_lease_t PACKED;

typedef struct client_interface_s {
	NODE				node;
	uint32_t 			ifindex;
	uint16_t			port;
	uint8_t				client_mac[ETHER_ADDR_LEN];          /* Our mac address */

	int					sock;
	int					udp_sock;

	uint8_t				opt_mask[DHCP_OPTION_MAX];      /* Bitmask of options to send (-O option) */
	dhcp_option_set_t	options[DHCP_OPTION_MAX];

	uint32_t			instance;
	void				*master;
	void				*r_thread;	//read thread
	void				*t_thread;	//time thread,
	void				*d_thread;	//discover thread,
	u_int32				first_secs;
	u_int32				last_secs;

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
extern client_interface_t * dhcp_client_lookup_interface(dhcp_global_t*config, u_int32 ifindex);
extern int dhcp_client_add_interface(dhcp_global_t*config, u_int32 ifindex);
extern int dhcp_client_del_interface(dhcp_global_t*config, u_int32 ifindex);
extern int dhcp_client_interface_clean(void);
extern int dhcp_client_interface_option_set(client_interface_t * ifter, uint16_t code, uint8_t *str, int len);
extern int dhcp_client_interface_request_set(client_interface_t * ifter, uint16_t code, BOOL enable);
/************************************************************************************/



//POP_SAVED_FUNCTION_VISIBILITY

#endif
