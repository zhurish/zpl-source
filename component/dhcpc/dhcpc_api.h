/*
 * dhcpc_api.h
 *
 *  Created on: Aug 28, 2018
 *      Author: zhurish
 */

#ifndef __DHCPC_API_H__
#define __DHCPC_API_H__

#include "zebra.h"
#include "nsm_dhcp.h"
#include "nsm_client.h"



typedef struct dhcpc_ctrl
{
	uint32_t		action;
	uint32_t		kifindex;
	//char 			name[IF_NAMESIZE];
	uint32_t		option;
	char			value[256];

}dhcpc_ctrl_head;


extern int _dhcp_syslog(int pri, const char *format, ...);

#if 1
#define dhcp_syslog(pri, fmt,...)	_dhcp_syslog(pri, fmt, ##__VA_ARGS__)
//#define DHCP_DEBUG(fmt,...)	_dhcp_syslog(LOG_WARNING, fmt, ##__VA_ARGS__)
//#define dhcp_syslog(pri, fmt,...)	fprintf(stdout, fmt, ##__VA_ARGS__)
#define DHCP_DEBUG(fmt,...)	fprintf(stdout, fmt, ##__VA_ARGS__)
#else
#define dhcp_syslog(pri, fmt,...)
#endif

extern int dhcpc_module_init ();
extern int dhcpc_module_exit ();
extern int dhcpc_task_init ();
extern int dhcpc_task_exit ();

extern int dhcpc_interface_enable_api(struct interface *ifp, BOOL enable);
extern int dhcpc_interface_start_api(struct interface *ifp, BOOL enable);
extern int dhcpc_interface_renew_api(struct interface *ifp);
extern int dhcpc_interface_option_api(struct interface *ifp, BOOL set, int option, char *string);
extern int dhcpc_interface_config(struct interface *ifp, struct vty *vty);
extern int dhcpc_client_interface_show(struct interface *ifp, struct vty *vty);
extern int dhcpc_client_interface_detail_show(struct interface *ifp, struct vty *vty);

#endif /* __DHCPC_API_H__ */
