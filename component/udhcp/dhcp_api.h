/*
 * dhcp_api.h
 *
 *  Created on: Jun 30, 2019
 *      Author: zhurish
 */

#ifndef __DHCP_API_H__
#define __DHCP_API_H__

int udhcp_module_init();
int udhcp_module_exit();
int udhcp_module_task_init();
int udhcp_module_task_exit ();

int dhcpc_interface_enable_api(struct interface *ifp, BOOL enable);

int dhcpc_interface_start_api(struct interface *ifp, BOOL enable);

int dhcpc_interface_option_api(struct interface *ifp, BOOL enable, int index, char *option);
int dhcpc_interface_config(struct interface *ifp, struct vty *vty);
int dhcpc_interface_lease_show(struct vty *vty, struct interface *ifp, BOOL detail);

#endif /* __DHCP_API_H__ */
