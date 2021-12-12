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
	int			sock;
	ifindex_t  	ifindex;

	zpl_uint8 	mac[ETHER_ADDR_LEN];          /* our MAC address (used only for ARP probing) */
	zpl_uint32 	ipaddr;

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
