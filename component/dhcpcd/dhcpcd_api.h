/*
 * dhcpcd_api.h
 *
 *  Created on: Nov 16, 2018
 *      Author: zhurish
 */

#ifndef __DHCPCD_API_H__
#define __DHCPCD_API_H__

#include "zebra.h"
#include "prefix.h"
#include "table.h"
#include "if.h"
#include "vty.h"
#include "interface.h"


typedef struct dhcpcd_ctrl
{
	uint32_t		action;
	uint32_t		kifindex;
	BOOL			ipv6;
	uint32_t		option;
	char			value[256];

}dhcpcd_ctrl_head;


extern int dhcpc_module_init ();
extern int dhcpc_task_init ();
extern int dhcpc_task_exit ();
extern int dhcpc_module_exit ();

extern int dhcpc_enable_test();


extern int dhcpc_interface_enable_api(struct interface *ifp, BOOL enable);
extern int dhcpc_interface_start_api(struct interface *ifp, BOOL enable);
extern int dhcpc_interface_ipv6_enable_api(struct interface *ifp, BOOL enable);
extern int dhcpc_interface_ipv6_start_api(struct interface *ifp, BOOL enable);
extern int dhcpc_interface_renew_api(struct interface *ifp);
extern int dhcpc_interface_option_api(struct interface *ifp, BOOL set, int option, char *string);


extern int dhcpc_interface_config(struct interface *ifp, struct vty *vty);
extern int dhcpc_client_interface_show(struct interface *ifp, struct vty *vty);
extern int dhcpc_client_interface_detail_show(struct interface *ifp, struct vty *vty);


#endif /* __DHCPCD_API_H__ */
