/*
 * dhcpd_api.h
 *
 *  Created on: Sep 22, 2018
 *      Author: zhurish
 */


#ifndef __DHCPD_API_H__
#define __DHCPD_API_H__


extern void * dhcpd_pool_add_api(char *name);
extern int dhcpd_pool_del_api(char *name);
extern void * dhcpd_pool_subnet_add_api(void *pool, struct prefix subnet);
extern int dhcpd_pool_address_range_add_api(void *subnet, struct prefix startp, struct prefix endp);
extern int dhcpd_pool_option_add_api(void *subnet, int option, char *value);
extern int dhcpd_pool_option_del_api(void *subnet, int option);

extern int dhcpd_bootp_lease_set_api(void *subnet, int value);
extern int dhcpd_max_lease_set_api(void *subnet, int value);
extern int dhcpd_default_lease_set_api(void *subnet, int value);
extern int dhcpd_authoritative_set_api(void *subnet, BOOL value);
extern int dhcpd_allow_deny_set_api(void *subnet, int cmd,  BOOL value);
extern int dhcpd_server_name_set_api(void *subnet, char *servername);
extern int dhcpd_filename_set_api(void *subnet, char *filename);
extern int dhcpd_server_identifier_set_api(void *subnet, char *hostname_or_address);
extern int dhcpd_next_server_set_api(void *subnet, char *hostname_or_address);


extern int dhcps_module_init ();
extern int dhcps_module_exit ();
extern int dhcps_task_init ();
extern int dhcps_task_exit ();




















#endif /*__DHCPD_API_H__*/
