#ifndef __DHCP_LEASE_H__
#define __DHCP_LEASE_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef zpl_uint32 leasetime_t;

typedef enum
{ 
	LEASE_DYNAMIC = 1,
	LEASE_STATIC = 2, 
}lease_mode_t;

typedef struct dyn_lease {
	NODE	node;
	/* Unix time when lease expires. Kept in memory in host order.
	 * When written to file, converted to network order
	 * and adjusted (current time subtracted) */
	 /* "nip": IP in network order */
	zpl_uint32 	lease_address;
	zpl_uint32 	lease_netmask;
	zpl_uint32 	lease_gateway;
	zpl_uint32 	lease_gateway2;

	leasetime_t expires;
	leasetime_t starts;
	leasetime_t ends;

	zpl_uint32 	lease_dns1;
	zpl_uint32 	lease_dns2;

	//zpl_uint32 	server_address;
	//zpl_uint8 	frominfo;
	//zpl_uint32  	cookie;
	/* We use lease_mac[6], since e.g. ARP probing uses
	 * only 6 first bytes anyway. We check received dhcp packets
	 * that their hlen == 6 and thus chaddr has only 6 significant bytes
	 * (dhcp packet has chaddr[16], not [6])
	 */
	zpl_uint8 		lease_mac[ETHER_ADDR_LEN];
	char 		hostname[UDHCPD_HOSTNAME_MAX];
	//char 		domain_name[UDHCPD_HOSTNAME_MAX];
	char 		vendor[UDHCPD_HOSTNAME_MAX];
	char 		client_id[UDHCPD_HOSTNAME_MAX];

	ifindex_t 	ifindex;
	zpl_uint32		poolid;
	lease_mode_t mode;

}dyn_lease_t PACKED;

extern dyn_lease_t * dhcp_lease_lookup_by_lease_address(LIST *lst, lease_mode_t mode, zpl_uint32 lease_address);
extern dyn_lease_t * dhcp_lease_lookup_by_lease_mac(LIST *lst, lease_mode_t mode, zpl_uint8 *lease_mac);
extern dyn_lease_t * dhcp_lease_add(LIST *lst, dyn_lease_t *lease);
extern int dhcp_lease_update(LIST *lst, dyn_lease_t *lease);
extern int dhcp_lease_del_mac(LIST *lst, zpl_uint8 *lease_mac);
extern int dhcp_lease_del_address(LIST *lst, zpl_uint32 lease_address);
extern int dhcp_lease_del(LIST *lst, dyn_lease_t *pstNode);
extern int dhcp_lease_clean(LIST *lst);
extern dyn_lease_t *dhcp_lease_lookup_expired_lease(LIST *lst);
extern int dhcpd_lease_foreach(int(*cb)(dyn_lease_t *, void *p), void *p);
extern int dhcpd_lease_save();
extern int dhcpd_lease_load();

extern int dhcp_lease_show(struct vty *vty, char *poolname, ifindex_t ifindex, zpl_bool detail);

 
#ifdef __cplusplus
}
#endif
 
#endif /* __DHCP_LEASE_H__ */
