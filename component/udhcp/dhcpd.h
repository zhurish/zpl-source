/* vi: set sw=4 ts=4: */
/*
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */
#ifndef UDHCP_DHCPD_H
#define UDHCP_DHCPD_H 1

//PUSH_AND_SET_FUNCTION_VISIBILITY_TO_HIDDEN
#define CONFIG_DHCPD_LEASES_FILE	"/var/lib/misc/udhcpd.leases"
/* Defaults you may want to tweak */
/* Default max_lease_sec */
#define DEFAULT_LEASE_TIME      (60*60*24 * 10)
#define LEASES_FILE             CONFIG_DHCPD_LEASES_FILE
/* Where to find the DHCP server configuration file */
#define DHCPD_CONF_FILE         "/etc/udhcpd.conf"

#include "dhcp_def.h"
#include "dhcp_pool.h"


typedef struct dhcpd_lease_state_s
{
	u_int32_t offered_expiry;

	//struct tree_cache *options[256];

	int requested_address;	/* True if client sent the
					   dhcp-requested-address option. */
	int server_identifier;	/* True if client sent the
					   dhcp-server-identifier option. */
	u_int32_t xid;
	u_int32_t ciaddr;
	u_int32_t giaddr;
	u_int8_t req_mask[256];

	int read_bytes;
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
	uint32_t 	ifindex;

	uint8_t 	server_mac[ETHER_ADDR_LEN];          /* our MAC address (used only for ARP probing) */
	uint32_t	ipaddr;

	//dyn_lease_t *lease;
	

	void		*auto_thread;
	void		*lease_thread;
	void		*arp_thread;

	dhcpd_lease_state_t state;
	//struct dhcp_packet oldpacket;

} dhcpd_interface_t;





/* client_config sits in 2nd half of bb_common_bufsiz1 */


uint32_t dhcpd_lookup_address_on_interface(dhcp_pool_t*config, u_int32 ifindex);
dhcpd_interface_t * dhcpd_lookup_interface(dhcp_pool_t*config, u_int32 ifindex);
int dhcpd_pool_add_interface(dhcp_pool_t*config, u_int32 ifindex);
int dhcpd_pool_del_interface(dhcp_pool_t*config, u_int32 ifindex);
int dhcpd_interface_clean(dhcp_pool_t*config);

int udhcp_server_handle_thread(dhcp_pool_t *pool, dhcpd_interface_t * ifter,
		struct dhcp_packet *packet);





#endif
