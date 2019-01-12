/*
 * dhcpcd_api.c
 *
 *  Created on: Nov 16, 2018
 *      Author: zhurish
 */

#include "zebra.h"
#include "if.h"

#include "dhcp-config.h"

#include "common.h"
#include "dhcp-common.h"
#include "dhcp.h"
#include "dhcpc-if.h"
#include "ipv6.h"
#include "logerr.h"
#include "dhcp-eloop.h"
#include "dhcpcd_api.h"

#define PL_DHCPC_MODULE

#include "nsm_dhcp.h"


static int dhcpcd_task_id = 0;
static int dhcpcd_ctlfd = 0;

static int dhcpcd_thread(void *p)
{
	return dhcpcd_main(p);
}

int dhcpc_module_init ()
{
	/* Make master thread emulator. */
	dhcpc_task_init ();
	return 0;

}

int dhcpc_task_init ()
{
	if(dhcpcd_task_id == 0)
		dhcpcd_task_id = os_task_create("dhcpcTask", OS_TASK_DEFAULT_PRIORITY,
	               0, dhcpcd_thread, NULL, OS_TASK_DEFAULT_STACK);
	if(dhcpcd_task_id)
		return OK;
	return ERROR;

}

int dhcpc_task_exit ()
{
	if(dhcpcd_task_id)
		os_task_destroy(dhcpcd_task_id);
	dhcpcd_task_id = 0;
	return OK;
}

int dhcpc_module_exit ()
{
	return OK;
}

int dhcpc_interface_enable_api(struct interface *ifp, BOOL enable)
{
	int ret = 0;
	dhcpcd_ctrl_head head;
	memset(&head, 0, sizeof(head));
	head.action = enable ? DHCPC_CLIENT_ADD_IF:DHCPC_CLIENT_DEL_IF;
	//strcpy(head.name, ifp->k_name);
	head.kifindex = ifp->k_ifindex;
	ret = control_send(dhcpcd_ctlfd, &head, sizeof(dhcpcd_ctrl_head));
	if((ifp && ifp->k_ifindex && !if_is_wireless(ifp)) && enable)
	{
		os_msleep(10);
		memset(&head, 0, sizeof(head));
		head.action = enable ? DHCPC_CLIENT_START_IF:DHCPC_CLIENT_STOP_IF;
		head.kifindex = ifp->k_ifindex;
		ret = control_send(dhcpcd_ctlfd, &head, sizeof(dhcpcd_ctrl_head));
	}
	return (ret > 0)? 0:-1;
}

int dhcpc_interface_ipv6_enable_api(struct interface *ifp, BOOL enable)
{
	int ret = 0;
	dhcpcd_ctrl_head head;
	memset(&head, 0, sizeof(head));
	head.action = enable ? DHCPC_CLIENT_ADD_IF:DHCPC_CLIENT_DEL_IF;
	//strcpy(head.name, ifp->k_name);
	head.kifindex = ifp->k_ifindex;
	head.ipv6 = TRUE;
	ret = control_send(dhcpcd_ctlfd, &head, sizeof(dhcpcd_ctrl_head));
	if((ifp && ifp->k_ifindex && !if_is_wireless(ifp)) && enable)
	{
		os_msleep(10);
		memset(&head, 0, sizeof(head));
		head.action = enable ? DHCPC_CLIENT_START_IF:DHCPC_CLIENT_STOP_IF;
		head.kifindex = ifp->k_ifindex;
		head.ipv6 = TRUE;
		ret = control_send(dhcpcd_ctlfd, &head, sizeof(dhcpcd_ctrl_head));
	}
	return (ret > 0)? 0:-1;
}

int dhcpc_interface_start_api(struct interface *ifp, BOOL enable)
{
	int ret = 0;
	dhcpcd_ctrl_head head;
	memset(&head, 0, sizeof(head));
	head.action = enable ? DHCPC_CLIENT_START_IF:DHCPC_CLIENT_STOP_IF;
	//strcpy(head.name, ifp->k_name);
	head.kifindex = ifp->k_ifindex;
	ret = control_send(dhcpcd_ctlfd, &head, sizeof(dhcpcd_ctrl_head));
	return (ret > 0)? 0:-1;
}

int dhcpc_interface_ipv6_start_api(struct interface *ifp, BOOL enable)
{
	int ret = 0;
	dhcpcd_ctrl_head head;
	memset(&head, 0, sizeof(head));
	head.action = enable ? DHCPC_CLIENT_START_IF:DHCPC_CLIENT_STOP_IF;
	//strcpy(head.name, ifp->k_name);
	head.kifindex = ifp->k_ifindex;
	head.ipv6 = TRUE;
	ret = control_send(dhcpcd_ctlfd, &head, sizeof(dhcpcd_ctrl_head));
	return (ret > 0)? 0:-1;
}

int dhcpc_interface_option_api(struct interface *ifp, BOOL set, int option, char *string)
{
/*	int ret = -1;
	dhcpc_ctrl_head head;
	memset(&head, 0, sizeof(head));
	head.action = set ? DHCPC_CLIENT_ADD_OPTION_IF:DHCPC_CLIENT_DEL_OPTION_IF;
	//strcpy(head.name, ifp->k_name);
	head.kifindex = ifp->k_ifindex;
	head.option = option;
	if(string)
		strcpy(head.value, string);
	ret = send_control(&head);
	return (ret > 0)? 0:-1;*/
	return OK;
}


int dhcpc_interface_renew_api(struct interface *ifp)
{
/*	int ret = -1;
	dhcpc_ctrl_head head;
	memset(&head, 0, sizeof(head));
	head.action = DHCPC_CLIENT_RESTART_IF;
	//strcpy(head.name, ifp->k_name);
	head.kifindex = ifp->k_ifindex;
	ret = send_control(&head);
	return (ret > 0)? 0:-1;*/
	return OK;
}

int dhcpc_enable_test()
{
	int ret = 0;
	dhcpcd_ctrl_head head;
	struct interface *ifp = if_lookup_by_name("ethernet 0/0/2");

	nsm_interface_update_kernel(ifp, "enp0s25");
	//nsm_interface_update_kernel(ifp, "eth0.2");

	memset(&head, 0, sizeof(head));
	head.action = 1;
	if(dhcpcd_ctlfd == 0)
		dhcpcd_ctlfd = control_open("eth0");

	head.kifindex = ifp->k_ifindex;
	ret = control_send(dhcpcd_ctlfd, &head, sizeof(dhcpcd_ctrl_head));
/*	if((ifp && ifp->k_ifindex && !if_is_wireless(ifp)))
	{
		os_msleep(10);
		memset(&head, 0, sizeof(head));
		head.action = 4;
		head.kifindex = ifp->k_ifindex;
		ret = control_send(dhcpcd_ctlfd, &head, sizeof(dhcpcd_ctrl_head));;
	}*/
	return (ret > 0)? 0:-1;
}
