/*
 * nsm_dhcp.h
 *
 *  Created on: Sep 8, 2018
 *      Author: zhurish
 */

#ifndef __NSM_DHCP_H__
#define __NSM_DHCP_H__


#ifdef __cplusplus
extern "C" {
#endif

#ifdef PL_DHCP_MODULE

#define DHCPS_NAME_MAX	64

typedef enum
{
	DHCPC_CLIENT_NONE = 0,

	DHCPC_CLIENT_ADD_IF,
	DHCPC_CLIENT_DEL_IF,

	DHCPC_CLIENT_START_IF,
	DHCPC_CLIENT_STOP_IF,
	DHCPC_CLIENT_RESTART_IF,
	DHCPC_CLIENT_RENEW_IF,
	DHCPC_CLIENT_FREE_IF,

	DHCPC_CLIENT_ADD_OPTION_IF,
	DHCPC_CLIENT_DEL_OPTION_IF,

	DHCPC_CLIENT_METRIC_IF,

	DHCPC_CLIENT_MAX,
}nsm_dhcp_action;


typedef enum
{
	DHCP_NONE = 0,
	DHCP_CLIENT,
	DHCP_SERVER,
	DHCP_RELAY,

}nsm_dhcp_type;

#ifdef PL_DHCPD_MODULE
/*
 * DHCP Server
 */
#define DHCPS_LEASE_DEFAULT 120
enum
{
	DHCPS_CMD_NETWORK = 1,
	DHCPS_CMD_NETWORK_START,
	DHCPS_CMD_NETWORK_END,
	DHCPS_CMD_GATEWAY,
	DHCPS_CMD_GATEWAY_SECONDARY,
	DHCPS_CMD_DOMAIN_NAME,
	DHCPS_CMD_NETBIOS,
	DHCPS_CMD_NETBIOS_SECONDARY,
	DHCPS_CMD_DNS,
	DHCPS_CMD_DNS_SECONDARY,
	DHCPS_CMD_TFTP,
	DHCPS_CMD_LEASE,
	DHCPS_CMD_HOST,
	DHCPS_CMD_EXCUDED,
};

typedef struct nsm_dhcps_host_s
{
	NODE			node;
	ospl_uint8 			mac[ETHER_ADDR_LEN];
	struct prefix 	address;
	ospl_uint8 			hostname[DHCPS_NAME_MAX];
}nsm_dhcps_host_t;

typedef struct nsm_dhcps_exclude_s
{
	NODE			node;
	ospl_bool			range;
	struct prefix 	start_address;
	struct prefix 	end_address;

}nsm_dhcps_exclude_t;

typedef struct nsm_dhcps_s
{
	NODE			node;
	ospl_char 			name[DHCPS_NAME_MAX];

	struct prefix 	address;
	struct prefix 	start_address;
	struct prefix 	end_address;

	struct prefix 	gateway;
	struct prefix 	gateway_secondary;

	struct prefix 	dns;
	struct prefix 	dns_secondary;

	struct prefix 	netbios;
	struct prefix 	netbios_secondary;
	struct prefix 	tftp;
	ospl_char 			domain_name[DHCPS_NAME_MAX];

	ospl_uint32				lease_time;

	LIST			hostlist;
	LIST			excludedlist;

	ospl_bool			active;

	void			*pool;

	struct interface	*ifp;
}nsm_dhcps_t;


typedef struct Gnsm_dhcps_s
{
	ospl_bool	enable;
	LIST	*dhcpslist;
	void	*mutex;
}Gnsm_dhcps_t;
#endif

/*
 * dhcp client/relay/server of interface
 */
typedef struct nsm_dhcp_ifp_s
{
	struct interface	*ifp;
	nsm_dhcp_type 		type;
	ospl_bool				hiden;
	ospl_bool				running;
	void				*client;
	void				*server;
	void				*relay;
}nsm_dhcp_ifp_t;


extern nsm_dhcp_ifp_t *nsm_dhcp_get(struct interface *ifp);

extern nsm_dhcp_type nsm_interface_dhcp_mode_get_api(struct interface *ifp);
extern int nsm_interface_dhcp_mode_set_api(struct interface *ifp, nsm_dhcp_type type, ospl_char *name);


extern int nsm_dhcp_interface_set_pravite(struct interface *ifp, nsm_dhcp_type type, void *pVoid);
extern void * nsm_dhcp_interface_get_pravite(struct interface *ifp, nsm_dhcp_type type);
extern int nsm_interface_dhcp_config(struct vty *vty, struct interface *ifp);


#ifdef PL_DHCPD_MODULE

extern int nsm_dhcps_init(void);
extern int nsm_dhcps_exit(void);

extern nsm_dhcps_t * nsm_dhcps_lookup_api(ospl_char *name);
extern int nsm_dhcps_add_api(ospl_char *name);
extern int nsm_dhcps_del_api(ospl_char *name);

extern int nsm_dhcps_set_api(nsm_dhcps_t *dhcps, ospl_uint32 cmd, void *val);
extern int nsm_dhcps_unset_api(nsm_dhcps_t *dhcps, ospl_uint32 cmd);
extern int nsm_dhcps_get_api(nsm_dhcps_t *dhcps, ospl_uint32 cmd, void *val);

extern nsm_dhcps_host_t * nsm_dhcps_host_lookup_api(nsm_dhcps_t *dhcps, ospl_char *address, ospl_uint8 *mac);
extern int nsm_dhcps_add_host_api(nsm_dhcps_t *dhcps, ospl_char *address, ospl_uint8 *mac);
extern int nsm_dhcps_del_host_api(nsm_dhcps_t *dhcps, ospl_char *address, ospl_uint8 *mac);
/******************************* exclude list **********************************/
extern nsm_dhcps_exclude_t * nsm_dhcps_exclude_list_lookup_api(nsm_dhcps_t *dhcps, ospl_char *address, ospl_char *endaddress);
extern int nsm_dhcps_add_exclude_list_api(nsm_dhcps_t *dhcps, ospl_char *address, ospl_char *endaddress);
extern int nsm_dhcps_del_exclude_list_api(nsm_dhcps_t *dhcps, ospl_char *address, ospl_char *endaddress);

extern int nsm_dhcps_lease_foreach(int(*cb)(void *, void *), void *p);

extern int nsm_dhcps_write_config(struct vty *vty);
extern int nsm_dhcps_lease_show(struct vty *vty, struct interface *ifp, ospl_char *poolname, ospl_bool detail);
extern int nsm_dhcps_pool_show(struct vty *vty, ospl_bool detail);
extern int nsm_interface_dhcps_enable(nsm_dhcps_t *pool, ifindex_t kifindex, ospl_bool enable);

extern void cmd_dhcps_init(void);
#endif


#ifdef PL_DHCPC_MODULE

extern int nsm_interface_dhcpc_enable(struct interface *ifp, ospl_bool enable);
extern int nsm_interface_dhcpc_start(struct interface *ifp, ospl_bool enable);
extern ospl_bool nsm_interface_dhcpc_is_running(struct interface *ifp);
extern int nsm_interface_dhcpc_option(struct interface *ifp,  ospl_bool enable, ospl_uint32 index, ospl_char *option);

extern int nsm_interface_dhcpc_write_config(struct interface *ifp, struct vty *vty);
extern int nsm_interface_dhcpc_client_show(struct interface *ifp, struct vty *vty, ospl_bool detail);

extern void cmd_dhcpc_init(void);
#endif


extern void cmd_dhcp_init(void);



extern int nsm_dhcp_module_init ();
extern int nsm_dhcp_module_exit ();
extern int nsm_dhcp_task_init ();
extern int nsm_dhcp_task_exit ();


#endif
 
#ifdef __cplusplus
}
#endif

#endif /* __NSM_DHCP_H__ */
