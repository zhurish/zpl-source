/* vi: set sw=4 ts=4: */
/*
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */
#ifndef UDHCP_DHCPD_H
#define UDHCP_DHCPD_H 1

#ifdef __cplusplus
extern "C" {
#endif


#include "dhcp_def.h"
#include "dhcp_pool.h"


typedef struct dhcpd_lease_state_s
{
	zpl_uint32 offered_expiry;

	//struct tree_cache *options[256];

	zpl_uint32 requested_address;	/* True if client sent the
					   dhcp-requested-address option. */
	zpl_uint32 server_identifier;	/* True if client sent the
					   dhcp-server-identifier option. */
	zpl_uint32 xid;
	zpl_uint32 ciaddr;
	zpl_uint32 giaddr;
	zpl_uint8 req_mask[DHCP_OPTION_MAX];

	zpl_uint32 read_bytes;
} dhcpd_lease_state_t;


typedef struct dhcpd_interface_s
{
	NODE		node;
	dhcp_pool_t *pool;
	//TODO: ifindex, server_nip, server_mac
	// are obtained from interface name.
	// Instead of querying them *once*, create update_server_network_data_cache()
	// and call it before any usage of these fields.
	// update_server_network_data_cache() must re-query data
	// if more than N seconds have passed after last use.
	ifindex_t  	ifindex;

	zpl_uint8 	server_mac[ETHER_ADDR_LEN];          /* our MAC address (used only for ARP probing) */
	zpl_uint32 	ipaddr;

	//dyn_lease_t *lease;
	

	void		*auto_thread;
	void		*lease_thread;
	void		*arp_thread;

	dhcpd_lease_state_t state;
	//struct dhcp_packet oldpacket;

} dhcpd_interface_t;





/* client_config sits in 2nd half of bb_common_bufsiz1 */


zpl_uint32  dhcpd_lookup_address_on_interface(dhcp_pool_t*config, ifindex_t ifindex);
dhcpd_interface_t * dhcpd_lookup_interface(dhcp_pool_t*config, ifindex_t ifindex);
int dhcpd_pool_add_interface(dhcp_pool_t*config, ifindex_t ifindex);
int dhcpd_pool_del_interface(dhcp_pool_t*config, ifindex_t ifindex);
int dhcpd_interface_clean(dhcp_pool_t*config);

int udhcp_server_handle_thread(dhcp_pool_t *pool, dhcpd_interface_t * ifter,
		struct dhcp_packet *packet);




 
#ifdef __cplusplus
}
#endif
 
#endif
