/*
 * dhcp_api.h
 *
 *  Created on: Jun 30, 2019
 *      Author: zhurish
 */

#ifndef __DHCP_API_H__
#define __DHCP_API_H__

#ifdef __cplusplus
extern "C" {
#endif

int udhcp_module_init();
int udhcp_module_exit();
int udhcp_module_task_init();
int udhcp_module_task_exit ();

int dhcpc_interface_enable_api(struct interface *ifp, ospl_bool enable);

int dhcpc_interface_start_api(struct interface *ifp, ospl_bool enable);

int dhcpc_interface_option_api(struct interface *ifp, ospl_bool enable, ospl_uint32 index, char *option);
int dhcpc_interface_config(struct interface *ifp, struct vty *vty);
int dhcpc_interface_lease_show(struct vty *vty, struct interface *ifp, ospl_bool detail);

int dhcp_pool_show(struct vty *vty, ospl_bool detail);
 
#ifdef __cplusplus
}
#endif
 
#endif /* __DHCP_API_H__ */
