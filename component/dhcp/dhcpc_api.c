/*
 * dhcpc_api.c
 *
 *  Created on: Aug 28, 2018
 *      Author: zhurish
 */
#include <zebra.h>

#include "if.h"
#include "vty.h"
#include "sockunion.h"
#include "prefix.h"
#include "command.h"
#include "memory.h"
#include "log.h"
#include "vrf.h"
#include "command.h"
#include "interface.h"

#include "arp.h"
#include "bind.h"
#include "dhcpc_config.h"
#include "dhcpc_common.h"
#include "configure.h"
#include "control.h"
#include "dhcpcd.h"
#include "duid.h"
#include "dhcpc_eloop.h"
#include "if-options.h"
#include "if-pref.h"
#include "ipv4ll.h"
#include "ipv6rs.h"
#include "net.h"
#include "dhcpc_platform.h"
#include "signals.h"


static int dhcpc_task_id = 0;


int _dhcp_syslog(int pri, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vzlog (NULL, ZLOG_NSM, pri, format, args);
	va_end(args);
}

int dhcpc_hostname(char *name, int len)
{
	os_strcpy(name, "SWPlatform");
	return 0;
}

struct dhcpc_interface *
init_interface(const char *ifname)
{
	struct dhcpc_interface *iface = NULL;
	struct interface *ifp = if_lookup_by_kernel_name (iface->name);
	if(!ifp)
		return NULL;

	iface = xzalloc(sizeof(*iface));
	strlcpy(iface->name, ifname, sizeof(iface->name));
	iface->flags = ifp->flags;
	iface->ifindex = if_nametoindex(iface->name);
	/* We reserve the 100 range for virtual interfaces, if and when
	 * we can work them out. */
	iface->metric = 200 + iface->ifindex;
	if (getifssid(ifname, iface->ssid) != -1)
	{
		iface->wireless = 1;
		iface->metric += 100;
	}
	snprintf(iface->leasefile, sizeof(iface->leasefile),
	    LEASEFILE, ifname);
	iface->running = 1;

	memcpy(iface->hwaddr, ifp->hw_addr, ifp->hw_addr_len);
	iface->hwlen = ifp->hw_addr_len;

	//if (dhcpc_if_init(ifp) == -1)
	/* 0 is a valid fd, so init to -1 */
	iface->raw_fd = -1;
	iface->udp_fd = -1;
	iface->arp_fd = -1;

	goto exit;

eexit:
	free(iface);
	iface = NULL;
exit:
	return iface;
}


int
carrier_status(struct dhcpc_interface *iface)
{
	int ret = 1;
	struct interface *ifp = if_lookup_by_kernel_name (iface->name);
	if(ifp)
	{
		ret = (ifp->flags & IFF_RUNNING) ? 1 : 0;
	}
	ret = (iface->flags & IFF_RUNNING) ? 1 : 0;
	return ret;
}

int
up_interface(struct dhcpc_interface *iface)
{
	int ret = 0;
	struct interface *ifp = if_lookup_by_kernel_name (iface->name);
	if(ifp)
	{
		ret = nsm_interface_up_set_api(ifp);
		iface->flags |= ifp->flags;
	}
	iface->flags |= IFF_UP;
	return 0;
}


int
do_mtu(const char *ifname, short int mtu)
{
	struct interface *ifp = if_lookup_by_kernel_name (ifname);
	if(ifp)
	{
		int mtu = 0;
		if(nsm_interface_mtu_get_api(ifp, &mtu) == OK)
			return mtu;
	}
	return 1500;
}

int
handle_args(struct fd_list *fd, char *buf)
{
	struct dhcpc_interface *ifp;
	dhcpc_ctrl_head *head = (dhcpc_ctrl_head *) buf;

	switch (head->action)
	{
	case DHCPC_CLIENT_ADD_IF:
		ifp = dhcpc_interface_lookup(head->name, 0);
		if(!ifp)
		{
			dhcpc_socket();
			ifp = init_interface(head->name);
			if(ifp)
			{
				dhcpc_interface_add(ifp);
				start_discover(ifp);
			}
		}
		break;
	case DHCPC_CLIENT_DEL_IF:
		ifp = dhcpc_interface_lookup(head->name, 0);
		if(ifp)
		{
			send_release(ifp);
			stop_interface(ifp);
			//dhcpc_interface_del(ifp);
		}
		break;
	case DHCPC_CLIENT_START_IF:
		ifp = dhcpc_interface_lookup(head->name, 0);
		if(ifp)
		{
			start_discover(ifp);
		}
		break;
	case DHCPC_CLIENT_STOP_IF:
		ifp = dhcpc_interface_lookup(head->name, 0);
		if(ifp)
		{
			send_release(ifp);
			stop_interface(ifp);
		}
		break;
	case DHCPC_CLIENT_RESTART_IF:
		ifp = dhcpc_interface_lookup(head->name, 0);
		if(ifp)
		{
			start_renew(ifp);
		}
		break;
	case DHCPC_CLIENT_FREE_IF:
		ifp = dhcpc_interface_lookup(head->name, 0);
		if(ifp)
		{
			send_release(ifp);
			stop_interface(ifp);
		}
		break;
	default:
		break;
	}
	return 0;
}

static int dhcpc_task(void *arg)
{
/*
 * dhcpcd -b -d -h ada
 */
	int argc;
	char **argv[] = {"-b", "-d", "-h", "router"};
	os_sleep(5);
	while(1)
	{
		dhcpc_main(argc, argv);
	}
	return 0;
}



int dhcpc_module_init ()
{
	/* Make master thread emulator. */
	master_thread[MODULE_DHCP] = thread_master_module_create (MODULE_DHCP);
	master_thread[MODULE_DHCP];

	return 0;

}

int dhcpc_task_init ()
{
	dhcpc_task_id = os_task_create("dhcpcTask", OS_TASK_DEFAULT_PRIORITY,
	               0, dhcpc_task, NULL, OS_TASK_DEFAULT_STACK);
	if(dhcpc_task_id)
		return OK;
	return ERROR;

}

int dhcpc_module_exit ()
{
	return OK;
}


int dhcpc_interface_enable_api(struct interface *ifp, BOOL enable)
{
	dhcpc_ctrl_head head;
	memset(&head, 0, sizeof(head));
	head.action = enable ? DHCPC_CLIENT_ADD_IF:DHCPC_CLIENT_DEL_IF;
	strcpy(head.name, ifp->k_name);
	return send_control(&head);
}

int dhcpc_interface_renew_api(struct interface *ifp)
{
	dhcpc_ctrl_head head;
	memset(&head, 0, sizeof(head));
	head.action = DHCPC_CLIENT_RESTART_IF;
	strcpy(head.name, ifp->k_name);
	return send_control(&head);
}

/*
HCP Client 命令 ....................................................................................................................... 16
2.1 ip address dhcp ............................................................................................................................................. 16
2.2 management ip address dhcp ........................................................................................................................ 17
2.3 dhcp client request ........................................................................................................................................ 18
2.4 dhcp client client-id ...................................................................................................................................... 19
2.5 dhcp client class-id ....................................................................................................................................... 21
2.6 dhcp client lease ........................................................................................................................................... 22
2.7 dhcp client hostname .................................................................................................................................... 23
2.8 dhcp client default-router distance ............................................................................................................... 24
2.9 dhcp client broadcast-flag ............................................................................................................................. 25
2.10 debug dhcp client ........................................................................................................................................ 26
2.11 show dhcp client ......................................................................................................................................... 27
2.12 show dhcp client statistics .......................................................................................................................... 28
2.13 clear dhcp client statistics ........................................................................................................................... 29
DHCPC_CLIENT_ADD_IF,
DHCPC_CLIENT_DEL_IF,
DHCPC_CLIENT_START_IF,
DHCPC_CLIENT_STOP_IF,
DHCPC_CLIENT_RESTART_IF,
DHCPC_CLIENT_FREE_IF,
DHCPC_CLIENT_MAX,
*/
