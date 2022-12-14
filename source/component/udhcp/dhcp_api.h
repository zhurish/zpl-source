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

int udhcp_module_init(void);
int udhcp_module_exit(void);
int udhcp_module_task_init(void);
int udhcp_module_task_exit (void);

int dhcpc_interface_enable_api(struct interface *ifp, zpl_bool enable);

int dhcpc_interface_start_api(struct interface *ifp, zpl_bool enable);

int dhcpc_interface_option_api(struct interface *ifp, zpl_bool enable, zpl_uint32 index, char *option);
int dhcpc_interface_config(struct interface *ifp, struct vty *vty);
int dhcpc_interface_lease_show(struct vty *vty, struct interface *ifp, zpl_bool detail);

int dhcp_pool_show(struct vty *vty, zpl_bool detail);
 
#ifdef __cplusplus
}
#endif
 
#endif /* __DHCP_API_H__ */
