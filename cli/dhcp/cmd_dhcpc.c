/*
 * cmd_dhcpc.c
 *
 *  Created on: Sep 1, 2018
 *      Author: zhurish
 */


#include "zebra.h"
#include "vty.h"
#include "if.h"

#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "vrf.h"
#include "interface.h"

#include "nsm_dhcp.h"

#ifdef PL_DHCPC_MODULE
//#include "nsm_dhcp.h"

DEFUN (nsm_interface_ip_dhcp,
		nsm_interface_ip_dhcp_cmd,
		"ip address dhcp",
		"Interface Internet Protocol config commands\n"
		"Set the IP address of an interface\n"
		"DHCP configure\n")
{
	int ret = 0;
	//BOOL mode = FALSE;
	struct interface *ifp = (struct interface *) vty->index;
	if(ifp)
	{
		if(nsm_interface_dhcp_mode_get_api(ifp) == DHCP_CLIENT)
			return CMD_SUCCESS;

		if(nsm_interface_dhcp_mode_get_api(ifp) != DHCP_NONE)
		{
			vty_out (vty, "%% This interface is already enable dhcp server/relay %s", VTY_NEWLINE);
			return CMD_WARNING;
		}

/*
		if(nsm_interface_dhcp_enable(ifp, TRUE) != OK)
		{
			vty_out (vty, "%% Can not enable dhcp on this interface%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
*/

		ret = nsm_interface_dhcp_mode_set_api(ifp, DHCP_CLIENT);
		if(ret == ERROR)
			vty_out (vty, "%% Can not enable dhcp client on this interface%s",VTY_NEWLINE);
		return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	return CMD_WARNING;
}

DEFUN (no_nsm_interface_ip_dhcp,
		no_nsm_interface_ip_dhcp_cmd,
		"no ip address dhcp",
		NO_STR
		"Interface Internet Protocol config commands\n"
		"Set the IP address of an interface\n"
		"DHCP configure\n")
{
	int ret = 0;
	//BOOL mode = FALSE;
	struct interface *ifp = (struct interface *) vty->index;
	if(ifp)
	{
/*
		if(!nsm_interface_dhcp_mode_get_api(ifp) == DHCP_CLIENT)
			return CMD_SUCCESS;
*/

		if(nsm_interface_dhcp_mode_get_api(ifp) == DHCP_NONE)
			return CMD_SUCCESS;

		if(nsm_interface_dhcp_mode_get_api(ifp) != DHCP_CLIENT)
		{
			vty_out (vty, "%% This interface is already enable dhcp server/relay %s", VTY_NEWLINE);
			return CMD_WARNING;
		}

		ret = nsm_interface_dhcp_mode_set_api(ifp, DHCP_NONE);
		if(ret == ERROR)
			vty_out (vty, "%% Can not disable dhcp client on this interface%s",VTY_NEWLINE);
		return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	return CMD_WARNING;
}

/*
DEFUN (nsm_interface_dhcp_option,
		nsm_interface_dhcp_option_cmd,
		"ip address dhcp option <1-255> STRING",
		"Interface Internet Protocol config commands\n"
		"Set the IP address of an interface\n"
		"DHCP configure\n"
		"DHCP option configure\n"
		"DHCP option index\n"
		"DHCP option value\n")
{
	int ret = 0;
	struct interface *ifp = (struct interface *) vty->index;
	if(ifp)
	{
		ret = nsm_interface_dhcp_option_set_api(ifp, TRUE, atoi(argv[0]), argv[1]);
		if(ret == ERROR)
			vty_out (vty, "%% Can't set interface IP address dhcp.%s",VTY_NEWLINE);
		return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	return CMD_WARNING;
}

DEFUN (no_nsm_interface_dhcp_option,
		no_nsm_interface_dhcp_option_cmd,
		"no ip address dhcp option <1-255>",
		NO_STR
		"Interface Internet Protocol config commands\n"
		"Set the IP address of an interface\n"
		"DHCP configure\n"
		"DHCP option configure\n"
		"DHCP option index\n")
{
	int ret = 0;
	struct interface *ifp = (struct interface *) vty->index;
	if(ifp)
	{
		ret = nsm_interface_dhcp_option_set_api(ifp, FALSE, atoi(argv[0]), argv[1]);
		if(ret == ERROR)
			vty_out (vty, "%% Can't set interface IP address dhcp.%s",VTY_NEWLINE);
		return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	return CMD_WARNING;
}
*/


DEFUN (nsm_dhcp_client_request,
	nsm_dhcp_client_request_cmd,
	"dhcp client request (router|static-route|classless-static-route|classless-static-route-ms|"
		"tftp-server-address|dns-nameserver|domain-name|vendor-specific)",
	"DHCP Protocol config commands\n"
	"DHCP client\n"
	"DHCP request configure\n"
	"Router (option 3)\n"
	"Static route (option 33)\n"
	"Classless static route (option 121)\n"
	"Classless static route ms (option 249)\n"
	"TFTP Server Address (option 150)\n"
	"DNS Name server (option 6)\n"
	"Domain Name (option 15)\n"
	"Spcific Vendor (option 43)\n")
{
	int ret = 0, option = 0;
	struct interface *ifp = (struct interface *) vty->index;
	if(ifp)
	{
		if(nsm_interface_dhcp_mode_get_api(ifp) == DHCP_CLIENT)
		{
			if(os_strlen(argv[0]) > 8)
			{
				if(os_strncmp(argv[0], "tftp-server-address", 6) == 0)
				{
					option = 150;
				}
				else
				{
					if(os_strlen(argv[0]) == os_strlen("classless-static-route"))
						option = 121;
					else if(os_strlen(argv[0]) > os_strlen("classless-static-route"))
						option = 249;
				}
			}
			else if(os_strlen(argv[0]) > 8)
			{
				if(os_strncmp(argv[0], "dns-nameserver", 8) == 0)
					option = 6;
				else if(os_strncmp(argv[0], "domain-name", 8) == 0)
					option = 15;
				else if(os_strncmp(argv[0], "vendor-specific", 8) == 0)
					option = 43;
			}
			else
			{
				if(os_strncmp(argv[0], "router", 6) == 0)
					option = 3;
				else if(os_strncmp(argv[0], "static-route", 6) == 0)
					option = 33;
			}
			if(option)
			{
				ret = nsm_interface_dhcpc_option(ifp, TRUE, option, NULL);
				if(ret == ERROR)
					vty_out (vty, "%% Can't set interface dhcp client request option.%s",VTY_NEWLINE);
				return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
			}
			else
			{
				vty_out (vty, "%% Can't set interface dhcp client request option.%s",VTY_NEWLINE);
			}
		}
		else
			vty_out (vty, "%% please enable dhcp client frist on this interface %s",VTY_NEWLINE);
	}
	return CMD_WARNING;
}

DEFUN (no_nsm_dhcp_client_request,
	no_nsm_dhcp_client_request_cmd,
	"no dhcp client request (router|static-route|classless-static-route|classless-static-route-ms|"
		"tftp-server-address|dns-nameserver|domain-name|vendor-specific)",
	NO_STR
	"DHCP Protocol config commands\n"
	"DHCP client\n"
	"DHCP request configure\n"
	"Router (option 3)\n"
	"Static route (option 33)\n"
	"Classless static route (option 121)\n"
	"Classless static route ms (option 249)\n"
	"TFTP Server Address (option 150)\n"
	"DNS Name server (option 6)\n"
	"Domain Name (option 15)\n"
	"Spcific Vendor (option 43)\n")
{
	int ret = 0, option = 0;
	struct interface *ifp = (struct interface *) vty->index;
	if(ifp)
	{
		if(nsm_interface_dhcp_mode_get_api(ifp) == DHCP_CLIENT)
		{
			if(os_strlen(argv[0]) > 8)
			{
				if(os_strncmp(argv[0], "tftp-server-address", 6) == 0)
				{
					option = 150;
				}
				else
				{
					if(os_strlen(argv[0]) == os_strlen("classless-static-route"))
						option = 121;
					else if(os_strlen(argv[0]) > os_strlen("classless-static-route"))
						option = 249;
				}
			}
			else if(os_strlen(argv[0]) > 8)
			{
				if(os_strncmp(argv[0], "dns-nameserver", 8) == 0)
					option = 6;
				else if(os_strncmp(argv[0], "domain-name", 8) == 0)
					option = 15;
				else if(os_strncmp(argv[0], "vendor-specific", 8) == 0)
					option = 43;
			}
			else
			{
				if(os_strncmp(argv[0], "router", 6) == 0)
					option = 3;
				else if(os_strncmp(argv[0], "static-route", 6) == 0)
					option = 33;
			}
			if(option)
			{
				ret = nsm_interface_dhcpc_option(ifp, FALSE, option, NULL);
				if(ret == ERROR)
					vty_out (vty, "%% Can't set interface dhcp client request option.%s",VTY_NEWLINE);
				return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
			}
			else
			{
				vty_out (vty, "%% Can't set interface dhcp client request option.%s",VTY_NEWLINE);
			}
		}
		else
			vty_out (vty, "%% please enable dhcp client frist on this interface %s",VTY_NEWLINE);
	}
	return CMD_WARNING;
}

DEFUN (nsm_dhcp_client_id,
	nsm_dhcp_client_id_cmd,
	"dhcp client client-id STRING (hex|)",
	"DHCP Protocol config commands\n"
	"DHCP client\n"
	"DHCP Client ID configure\n"
	"STRING of client-id\n"
	"hex format\n")
{
	int ret = 0;
	struct interface *ifp = (struct interface *) vty->index;
	if(ifp)
	{
		if(nsm_interface_dhcp_mode_get_api(ifp) == DHCP_CLIENT)
		{
			ret = nsm_interface_dhcpc_option(ifp, TRUE, 61, argv[1]);
			if(ret == ERROR)
				vty_out (vty, "%% Can't set interface dhcp client client-id.%s",VTY_NEWLINE);
			return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
		}
		else
			vty_out (vty, "%% please enable dhcp client frist on this interface %s",VTY_NEWLINE);
	}
	return CMD_WARNING;
}

DEFUN (no_nsm_dhcp_client_id,
	no_nsm_dhcp_client_id_cmd,
	"no dhcp client client-id",
	NO_STR
	"DHCP Protocol config commands\n"
	"DHCP client\n"
	"DHCP Client ID configure\n")
{
	int ret = 0;
	struct interface *ifp = (struct interface *) vty->index;
	if(ifp)
	{
		if(nsm_interface_dhcp_mode_get_api(ifp) == DHCP_CLIENT)
		{
			ret = nsm_interface_dhcpc_option(ifp, FALSE, 61, NULL);
			if(ret == ERROR)
				vty_out (vty, "%% Can't set interface dhcp client client-id.%s",VTY_NEWLINE);
			return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
		}
		else
			vty_out (vty, "%% please enable dhcp client frist on this interface %s",VTY_NEWLINE);
	}
	return CMD_WARNING;
}



DEFUN (nsm_dhcp_class_id,
	nsm_dhcp_class_id_cmd,
	"dhcp client class-id STRING (hex|)",
	"DHCP Protocol config commands\n"
	"DHCP client\n"
	"DHCP class ID configure\n"
	"STRING of class-id\n"
	"hex format\n")
{
	int ret = 0;
	struct interface *ifp = (struct interface *) vty->index;
	if(ifp)
	{
		if(nsm_interface_dhcp_mode_get_api(ifp) == DHCP_CLIENT)
		{
			ret = nsm_interface_dhcpc_option(ifp, TRUE, 60, argv[0]);
			if(ret == ERROR)
				vty_out (vty, "%% Can't set interface dhcp client class-id.%s",VTY_NEWLINE);
			return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
		}
		else
			vty_out (vty, "%% please enable dhcp client frist on this interface %s",VTY_NEWLINE);
	}
	return CMD_WARNING;
}

DEFUN (no_nsm_dhcp_class_id,
	no_nsm_dhcp_class_id_cmd,
	"no dhcp client class-id",
	NO_STR
	"DHCP Protocol config commands\n"
	"DHCP client\n"
	"DHCP class ID configure\n")
{
	int ret = 0;
	struct interface *ifp = (struct interface *) vty->index;
	if(ifp)
	{
		if(nsm_interface_dhcp_mode_get_api(ifp) == DHCP_CLIENT)
		{
			ret = nsm_interface_dhcpc_option(ifp, FALSE, 60, NULL);
			if(ret == ERROR)
				vty_out (vty, "%% Can't set interface dhcp client class-id.%s",VTY_NEWLINE);
			return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
		}
		else
			vty_out (vty, "%% please enable dhcp client frist on this interface %s",VTY_NEWLINE);
	}
	return CMD_WARNING;
}

DEFUN (nsm_dhcp_client_lease,
	nsm_dhcp_client_lease_cmd,
	"dhcp client lease <30-100000000>",
	"DHCP Protocol config commands\n"
	"DHCP client\n"
	"DHCP lease configure\n"
	"minutes value\n")
{
	int ret = 0;
	struct interface *ifp = (struct interface *) vty->index;
	if(ifp)
	{
		if(nsm_interface_dhcp_mode_get_api(ifp) == DHCP_CLIENT)
		{
			ret = nsm_interface_dhcpc_option(ifp, FALSE, 51, argv[0]);
			if(ret == ERROR)
				vty_out (vty, "%% Can't set interface dhcp client lease time.%s",VTY_NEWLINE);
			return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
		}
		else
			vty_out (vty, "%% please enable dhcp client frist on this interface %s",VTY_NEWLINE);
	}
	return CMD_WARNING;
}

DEFUN (no_nsm_dhcp_client_lease,
	no_nsm_dhcp_client_lease_cmd,
	"no dhcp client lease",
	NO_STR
	"DHCP Protocol config commands\n"
	"DHCP client\n"
	"DHCP lease configure\n")
{
	int ret = 0;
	struct interface *ifp = (struct interface *) vty->index;
	if(ifp)
	{
		if(nsm_interface_dhcp_mode_get_api(ifp) == DHCP_CLIENT)
		{
			ret = nsm_interface_dhcpc_option(ifp, FALSE, 51, NULL);
			if(ret == ERROR)
				vty_out (vty, "%% Can't set interface dhcp client lease time.%s",VTY_NEWLINE);
			return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
		}
		else
			vty_out (vty, "%% please enable dhcp client frist on this interface %s",VTY_NEWLINE);
	}
	return CMD_WARNING;
}

DEFUN (nsm_dhcp_client_hostname,
	nsm_dhcp_client_hostname_cmd,
	"dhcp client hostname HOSTNAME",
	"DHCP Protocol config commands\n"
	"DHCP client\n"
	"DHCP hostname configure\n"
	"hostname value\n")
{
	int ret = 0;
	struct interface *ifp = (struct interface *) vty->index;
	if(ifp)
	{
		if(nsm_interface_dhcp_mode_get_api(ifp) == DHCP_CLIENT)
		{
			ret = nsm_interface_dhcpc_option(ifp, TRUE, 12, argv[0]);
			if(ret == ERROR)
				vty_out (vty, "%% Can't set interface dhcp client lease time.%s",VTY_NEWLINE);
			return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
		}
		else
			vty_out (vty, "%% please enable dhcp client frist on this interface %s",VTY_NEWLINE);
	}
	return CMD_WARNING;
}

DEFUN (no_nsm_dhcp_client_hostname,
	no_nsm_dhcp_client_hostname_cmd,
	"no dhcp client hostname",
	NO_STR
	"DHCP Protocol config commands\n"
	"DHCP client\n"
	"DHCP hostname configure\n")
{
	int ret = 0;
	struct interface *ifp = (struct interface *) vty->index;
	if(ifp)
	{
		if(nsm_interface_dhcp_mode_get_api(ifp) == DHCP_CLIENT)
		{
			ret = nsm_interface_dhcpc_option(ifp, FALSE, 12, NULL);
			if(ret == ERROR)
				vty_out (vty, "%% Can't set interface dhcp client hostname.%s",VTY_NEWLINE);
			return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
		}
		else
			vty_out (vty, "%% please enable dhcp client frist on this interface %s",VTY_NEWLINE);
	}
	return CMD_WARNING;
}

DEFUN (nsm_debug_dhcp_client_default_metric,
	nsm_debug_dhcp_client_default_metric_cmd,
	"dhcp client default-instance <1-65525>",
	"DHCP Protocol config commands\n"
	"DHCP client\n"
	"DHCP default route instance configure\n"
	"Instance value(default 1000)\n")
{
	int ret = 0;
	struct interface *ifp = (struct interface *) vty->index;
	if(ifp)
	{
		if(nsm_interface_dhcp_mode_get_api(ifp) == DHCP_CLIENT)
		{
			ret = nsm_interface_dhcpc_option(ifp, TRUE, 255+'m', argv[0]);
			if(ret == ERROR)
				vty_out (vty, "%% Can't set interface dhcp client default route instance instance.%s",VTY_NEWLINE);
			return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
		}
		else
			vty_out (vty, "%% please enable dhcp client frist on this interface %s",VTY_NEWLINE);
	}
	return CMD_WARNING;
}

DEFUN (no_nsm_debug_dhcp_client_default_metric,
	no_nsm_debug_dhcp_client_default_metric_cmd,
	"no dhcp client default-instance",
	DEBUG_STR
	"DHCP Protocol config commands\n"
	"DHCP client\n"
	"DHCP default route instance configure\n")
{
	int ret = 0;
	struct interface *ifp = (struct interface *) vty->index;
	if(ifp)
	{
		if(nsm_interface_dhcp_mode_get_api(ifp) == DHCP_CLIENT)
		{
			ret = nsm_interface_dhcpc_option(ifp, FALSE, 255+'m', NULL);
			if(ret == ERROR)
				vty_out (vty, "%% Can't set interface dhcp client default route instance instance.%s",VTY_NEWLINE);
			return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
		}
		else
			vty_out (vty, "%% please enable dhcp client frist on this interface %s",VTY_NEWLINE);
	}
	return CMD_WARNING;
}


DEFUN (nsm_debug_dhcp_client,
	nsm_debug_dhcp_client_cmd,
	"debug dhcp client (event|error|packet|all)",
	DEBUG_STR
	"DHCP Protocol config commands\n"
	"DHCP client\n"
	"DHCP event\n"
	"DHCP error\n"
	"DHCP packet\n"
	"DHCP all\n")
{
	//int ret = 0;
	struct interface *ifp = (struct interface *) vty->index;
	if(ifp)
	{
/*		ret = nsm_interface_dhcpc_option_set_api(ifp, TRUE, atoi(argv[0]), argv[1]);
		if(ret == ERROR)
			vty_out (vty, "%% Can't set interface IP address dhcp.%s",VTY_NEWLINE);
		return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;*/
	}
	return CMD_WARNING;
}

DEFUN (no_nsm_debug_dhcp_client,
	no_nsm_debug_dhcp_client_cmd,
	"no debug dhcp client (event|error|packet|all)",
	NO_STR
	DEBUG_STR
	"DHCP Protocol config commands\n"
	"DHCP client\n"
	"DHCP event\n"
	"DHCP error\n"
	"DHCP packet\n"
	"DHCP all\n")
{
	//int ret = 0;
	struct interface *ifp = (struct interface *) vty->index;
	if(ifp)
	{
/*		ret = nsm_interface_dhcpc_option_set_api(ifp, TRUE, atoi(argv[0]), argv[1]);
		if(ret == ERROR)
			vty_out (vty, "%% Can't set interface IP address dhcp.%s",VTY_NEWLINE);
		return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;*/
	}
	return CMD_WARNING;
}


DEFUN (nsm_show_dhcp_client,
	nsm_show_dhcp_client_cmd,
	"show dhcp client",
	SHOW_STR
	"DHCP Protocol config commands\n"
	"DHCP client\n")
{
	if(argc == 0)
	{
		struct listnode *node;
		struct interface *ifp;
		struct list *iftmp = if_list_get();
		for (ALL_LIST_ELEMENTS_RO(iftmp, node, ifp))
		{
			if (ifp)
			{
				nsm_interface_dhcpc_client_show(ifp, vty, FALSE);
			}
		}
		//if_list_each(nsm_interface_dhcpc_client_show, vty);
	}
	else
	{
		struct interface *ifp = NULL;
		ifindex_t ifindex = if_ifindex_make(argv[0], argv[1]);
		ifp = if_lookup_by_index(ifindex);
		if(ifp)
			nsm_interface_dhcpc_client_show(ifp, vty, FALSE);
	}
	return CMD_SUCCESS;
}

ALIAS(nsm_show_dhcp_client,
		nsm_show_dhcp_client_interface_cmd,
		"show dhcp client " CMD_INTERFACE_STR " (ethernet|gigabitethernet|brigde|wireless) " CMD_USP_STR,
		SHOW_STR
		"DHCP Protocol config commands\n"
		"DHCP client\n"
		CMD_INTERFACE_STR_HELP
		"Ethernet interface\n"
		"GigabitEthernet interface\n"
		"Brigde interface\n"
		"Wireless interface\n"
		CMD_USP_STR_HELP);


DEFUN (nsm_show_dhcp_client_detail,
	nsm_show_dhcp_client_detail_cmd,
	"show dhcp client detail",
	SHOW_STR
	"DHCP Protocol config commands\n"
	"DHCP client\n"
	"DHCP client detail information\n")
{
	if(argc == 0)
	{
		struct listnode *node;
		struct interface *ifp;
		struct list *iftmp = if_list_get();
		for (ALL_LIST_ELEMENTS_RO(iftmp, node, ifp))
		{
			if (ifp)
			{
				nsm_interface_dhcpc_client_show(ifp, vty, TRUE);
			}
		}
		//if_list_each(nsm_dhcpc_client_show_detail, vty);
	}
	else
	{
		struct interface *ifp = NULL;
		ifindex_t ifindex = if_ifindex_make(argv[0], argv[1]);
		ifp = if_lookup_by_index(ifindex);
		if(ifp)
			nsm_interface_dhcpc_client_show(ifp, vty, TRUE);
	}
	return CMD_SUCCESS;
}

ALIAS(nsm_show_dhcp_client_detail,
		nsm_show_dhcp_client_detail_interface_cmd,
		"show dhcp client " CMD_INTERFACE_STR " (ethernet|gigabitethernet|brigde|wireless) " CMD_USP_STR " detail",
		SHOW_STR
		"DHCP Protocol config commands\n"
		"DHCP client\n"
		"DHCP client debug\n"
		CMD_INTERFACE_STR_HELP
		"Ethernet interface\n"
		"GigabitEthernet interface\n"
		"Brigde interface\n"
		"Wireless interface\n"
		CMD_USP_STR_HELP
		"DHCP client detail information\n");


static void cmd_base_interface_dhcpc_init(int node)
{
	install_element(node, &nsm_interface_ip_dhcp_cmd);
	install_element(node, &no_nsm_interface_ip_dhcp_cmd);
	//install_element(node, &nsm_interface_dhcp_option_cmd);
	//install_element(node, &no_nsm_interface_dhcp_option_cmd);

	install_element(node, &nsm_dhcp_client_request_cmd);
	install_element(node, &no_nsm_dhcp_client_request_cmd);

	install_element(node, &nsm_dhcp_client_id_cmd);
	install_element(node, &no_nsm_dhcp_client_id_cmd);

	install_element(node, &nsm_dhcp_class_id_cmd);
	install_element(node, &no_nsm_dhcp_class_id_cmd);

	install_element(node, &nsm_dhcp_client_lease_cmd);
	install_element(node, &no_nsm_dhcp_client_lease_cmd);

	install_element(node, &nsm_dhcp_client_hostname_cmd);
	install_element(node, &no_nsm_dhcp_client_hostname_cmd);

	install_element(node, &nsm_debug_dhcp_client_default_metric_cmd);
	install_element(node, &no_nsm_debug_dhcp_client_default_metric_cmd);
}

static void cmd_show_dhcpc_init(int node)
{

	install_element(node, &nsm_show_dhcp_client_cmd);
	install_element(node, &nsm_show_dhcp_client_interface_cmd);

	install_element(node, &nsm_show_dhcp_client_detail_cmd);
	install_element(node, &nsm_show_dhcp_client_detail_interface_cmd);

}

void cmd_dhcpc_init(void)
{
	//cmd_base_interface_dhcpc_init(TUNNEL_INTERFACE_NODE);
	//cmd_base_interface_dhcpc_init(LOOPBACK_INTERFACE_NODE);
	//cmd_base_interface_dhcpc_init(LAG_INTERFACE_NODE);
	cmd_base_interface_dhcpc_init(INTERFACE_L3_NODE);
	cmd_base_interface_dhcpc_init(LAG_INTERFACE_L3_NODE);
	cmd_base_interface_dhcpc_init(WIRELESS_INTERFACE_NODE);
	//cmd_base_interface_dhcpc_init(BRIGDE_INTERFACE_NODE);
	//cmd_base_interface_dhcpc_init(SERIAL_INTERFACE_NODE);

	cmd_show_dhcpc_init(ENABLE_NODE);
	cmd_show_dhcpc_init(CONFIG_NODE);
	//cmd_show_dhcpc_init(ENABLE_NODE);

	install_element(ENABLE_NODE, &nsm_debug_dhcp_client_cmd);
	install_element(CONFIG_NODE, &no_nsm_debug_dhcp_client_cmd);
}

#endif

#ifdef PL_DHCP_MODULE
void cmd_dhcp_init(void)
{
#ifdef PL_DHCPC_MODULE
	cmd_dhcpc_init();
#endif
#ifdef PL_DHCPC_MODULE
	cmd_dhcps_init();
#endif
}
#endif
