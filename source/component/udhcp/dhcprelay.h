/*
 * dhcprelay.h
 *
 *  Created on: 2019年4月29日
 *      Author: DELL
 */

#ifndef __UDHCP_DHCPRELAY_H__
#define __UDHCP_DHCPRELAY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "dhcp_def.h"

typedef struct dhcp_relay_s
{
	NODE		node;
	dhcp_global_t *global;
	zpl_socket_t sock;
	ifindex_t  	ifindex;

	void		*master;
	void		*cr_thread;	//read thread
	void		*sr_thread;	//time thread,
	void		*t_thread;	//time thread,

	zpl_uint8 	mac[ETHER_ADDR_LEN];          /* our MAC address (used only for ARP probing) */
	zpl_uint32 	ipaddr;

	zpl_socket_t client_sock;
	zpl_uint32 	dhcp_server;
	zpl_uint32 	dhcp_server2;

} dhcp_relay_t;

extern dhcp_relay_t * dhcp_relay_lookup_interface(dhcp_global_t*config, ifindex_t ifindex);
extern int dhcp_relay_add_interface(dhcp_global_t*config, ifindex_t ifindex);
extern int dhcp_relay_del_interface(dhcp_global_t*config, ifindex_t ifindex);



#ifdef __cplusplus
}
#endif
 
#endif /* __UDHCP_DHCPRELAY_H__ */
