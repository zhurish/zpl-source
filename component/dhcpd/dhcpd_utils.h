/*
 * dhcpd_utils.h
 *
 *  Created on: Sep 22, 2018
 *      Author: zhurish
 */

#ifndef __DHCPD_UTILS_H__
#define __DHCPD_UTILS_H__


extern int dhcpd_option_set(struct group *group, char *optionname, char *val);
extern int dhcpd_option_unset(struct group *group, char *optionname);

extern struct class * dhcpd_class_decl_create(struct group *group, int type, char *val);

extern struct shared_network * dhcpd_shared_network_decl_create(struct group *group, char *name);
extern int dhcpd_shared_network_decl_destroy(char *name);

extern struct subnet *dhcpd_subnet_decl_create(struct shared_network *share, struct iaddr *net, struct iaddr *netmask);

extern int dhcpd_subnet_address_range(struct subnet *subnet, struct iaddr *start, struct iaddr *stop, int dynamic);

extern struct host_decl * dhcpd_host_decl_create(struct group *group, char *name, unsigned char *mac, struct iaddr *address, int num);



extern int dhcpd_bootp_lease_set(struct group *group, int val);
extern int dhcpd_max_lease_set(struct group *group, int val);
extern int dhcpd_default_lease_set(struct group *group, int val);
extern int dhcpd_authoritative_set(struct group *group, int val);
extern int dhcpd_allow_deny_set(struct group *group, int cmd, int val);
extern int dhcpd_server_name_set(struct group *group, char *value);
extern int dhcpd_filename_set(struct group *group, char *value);
extern int dhcpd_server_identifier_set(struct group *group, char *hostname_or_address);
extern int dhcpd_next_server_set(struct group *group, char *hostname_or_address);


extern struct interface_info * dhcpd_interface_lookup(int ifindex);
extern int dhcpd_interface_add(int ifindex);
extern int dhcpd_interface_del(int ifindex);


extern int dhcpd_interface_refresh(void);




/*
extern void dhcp_set_allow_deny(struct group *group, int token, int flag);
extern void dhcp_add_host_declaration(struct group *group, struct host_decl *inhost);
extern void dhcp_class_declaration(struct group *group, int type, char *val);
extern struct shared_network * dhcp_shared_net_declaration(struct group *group, char *name);
extern struct tree *dhcp_host_tree_get(char *name, unsigned char *addr, int len);
extern int dhcp_next_server(struct group *group, char *name, unsigned char *addr);
extern int dhcp_server_identifier(struct group *group, unsigned char *addr);
extern int dhcp_host_decl(struct host_decl *host_decl, struct hardware hardware, unsigned char *addr);
extern void dhcp_address_range(struct subnet *subnet, struct iaddr *start, struct iaddr *stop);

extern struct subnet * dhcp_subnet_declaration(struct shared_network *share, struct iaddr *net, struct iaddr *netmask);

extern void dhcp_option_param(struct group *group, char *optionname, char *val);
extern int dhcpd_pool_add(char *name);
extern int dhcpd_pool_set_address(char *name, struct iaddr *start, struct iaddr *stop, struct iaddr *netmask);
*/


extern int dhcpd_global_default_init();




extern void convert_num(unsigned char *buf, char *str, int base, int size);










extern void pfmsg(char c, struct lease *lp);


#endif /* __DHCPD_UTILS_H__ */
