#ifndef __DHCP_POOL_H__
#define __DHCP_POOL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "dhcp_def.h"
#include "dhcp_option.h"
#include "dhcp_main.h"



typedef struct dhcp_pool_s
{
	NODE		node;
	ospl_uint32 	poolid;
	char 		poolname[UDHCPD_POOL_MAX];

	dhcp_global_t	*global;

	LIST 	interf;

	dhcp_option_set_t	options[DHCP_OPTION_MAX];
	//struct option_set *options;     /* list of DHCP options loaded from the config file */
	/* start,end are in host order: we need to compare start <= ip <= end */
	ospl_uint32  start_ip;              /* start address of leases, in host order */
	ospl_uint32  end_ip;                /* end of leases, in host order */

	ospl_uint32  gateway;              /* gateway address, in host order */
	ospl_uint32  gateway2;              /* gateway address, in host order */

	ospl_uint32  tftp_server;              /* gateway address, in host order */

	char * 	dns;
	char * 	dns2;

	ospl_uint32  max_lease_sec;         /* maximum lease time (host order) */
	ospl_uint32  min_lease_sec;         /* minimum lease time a client can request */
	ospl_uint32  max_leases;            /* maximum number of leases (including reserved addresses) */
	ospl_uint32  auto_time;             /* how long should udhcpd wait before writing a config file.
									 * if this is zero, it will only write one on SIGUSR1 */
	ospl_uint32  decline_time;          /* how long an address is reserved if a client returns a
									 * decline message */
	ospl_uint32  conflict_time;         /* how long an arp conflict offender is leased for */
	ospl_uint32  offer_time;            /* how long an offered address is reserved */
	ospl_uint32  siaddr_nip;            /* "next server" bootp option */

	char *notify_file;              /* what to run whenever leases are written */
	char *boot_server_name;         /* bootp server name */
	char *boot_file;                /* bootp boot file option */

	LIST 	dhcp_lease_list;
}dhcp_pool_t;

/**********************************************************************/

dhcp_pool_t * dhcpd_pool_lookup(char *name);
dhcp_pool_t * dhcpd_pool_lookup_by_poolid(ospl_uint32 poolid);
dhcp_pool_t * dhcpd_pool_create(char *name);
dhcp_pool_t * dhcpd_pool_interface_lookup(ospl_uint32 ifindex);
int dhcpd_pool_clean(void);
int dhcpd_pool_del(char *name);
char * dhcpd_pool_poolid2name(ospl_uint32 poolid);
/**********************************************************************/
int dhcpd_pool_set_address_range(dhcp_pool_t *config, ospl_uint32 , ospl_uint32 );
int dhcpd_pool_set_leases(dhcp_pool_t *config, ospl_uint32, ospl_uint32);
int dhcpd_pool_set_autotime(dhcp_pool_t *config, ospl_uint32);
int dhcpd_pool_set_decline_time(dhcp_pool_t *config, ospl_uint32);
int dhcpd_pool_set_conflict_time(dhcp_pool_t *config, ospl_uint32);
int dhcpd_pool_set_offer_time(dhcp_pool_t *config, ospl_uint32);
int dhcpd_pool_set_siaddr(dhcp_pool_t *config, ospl_uint32  );
int dhcpd_pool_set_notify_file(dhcp_pool_t *config, char *str);
int dhcpd_pool_set_sname(dhcp_pool_t *config, char *str);
int dhcpd_pool_set_boot_file(dhcp_pool_t *config, char *str);
//option	subnet	255.255.255.0
int dhcpd_pool_set_option(dhcp_pool_t *config, ospl_uint8 opc, char *str);
//00:60:08:11:CE:4E 192.168.0.54
int dhcpd_pool_set_static_lease(dhcp_pool_t *config, char *str);
//POP_SAVED_FUNCTION_VISIBILITY

//int dhcpd_config_init(dhcp_global_t *config);


 
#ifdef __cplusplus
}
#endif
 
#endif /* __DHCP_POOL_H__ */
