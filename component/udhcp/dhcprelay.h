/*
 * dhcprelay.h
 *
 *  Created on: 2019年4月29日
 *      Author: DELL
 */

#ifndef __UDHCP_DHCPRELAY_H__
#define __UDHCP_DHCPRELAY_H__

#include "dhcp_def.h"

typedef struct dhcp_relay_s
{
	NODE		node;
	int			sock;
	uint32_t 	ifindex;

	uint8_t 	mac[ETHER_ADDR_LEN];          /* our MAC address (used only for ARP probing) */
	uint32_t	ipaddr;

	uint32_t	dhcp_server;
	uint32_t	dhcp_server2;

} dhcp_relay_t;

extern dhcp_relay_t * dhcp_relay_lookup_interface(dhcp_global_t*config, u_int32 ifindex);
extern int dhcp_relay_add_interface(dhcp_global_t*config, u_int32 ifindex);
extern int dhcp_relay_del_interface(dhcp_global_t*config, u_int32 ifindex);



#endif /* __UDHCP_DHCPRELAY_H__ */
