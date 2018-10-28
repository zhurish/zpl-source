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


DEFUN (ip_dhcp_pool,
		ip_dhcp_pool_cmd,
		"ip dhcp pool NAME",
		"Interface Internet Protocol config commands\n"
		"DHCP configure\n"
		"POOL configure\n"
		"name\n")
{
	int ret = ERROR;
	nsm_dhcps_t *dhcps = NULL;
	dhcps = nsm_dhcps_lookup_api(argv[0]);
	if(dhcps)
	{
		vty->index = dhcps;
		vty->node = DHCPS_NODE;
		ret = OK;
	}
	else
	{
		if(nsm_dhcps_add_api(argv[0]) != OK)
		{
			vty_out (vty, "%% Can not add dhcp pool %s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		dhcps = nsm_dhcps_lookup_api(argv[0]);
		if(dhcps)
		{
			vty->index = dhcps;
			vty->node = DHCPS_NODE;
			ret = OK;
		}
		else
		{
			vty_out (vty, "%% Can not lookup dhcp pool %s", VTY_NEWLINE);
			return CMD_WARNING;
		}
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_dhcp_pool,
		no_ip_dhcp_pool_cmd,
		"no ip dhcp pool NAME",
		NO_STR
		"Interface Internet Protocol config commands\n"
		"DHCP configure\n"
		"POOL configure\n"
		"name\n")
{
	int ret = ERROR;
	nsm_dhcps_t *dhcps = NULL;
	dhcps = nsm_dhcps_lookup_api(argv[0]);
	if(!dhcps)
	{
		vty_out (vty, "%% Can not lookup dhcp pool %s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	else
	{
		if(nsm_dhcps_del_api(argv[0]) != OK)
		{
			vty_out (vty, "%% Can not del dhcp pool %s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		dhcps = nsm_dhcps_lookup_api(argv[0]);
		if(!dhcps)
		{
			ret = OK;
		}
		else
		{
			vty_out (vty, "%% Can not del dhcp pool %s", VTY_NEWLINE);
			return CMD_WARNING;
		}
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (dhcp_network,
		dhcp_network_cmd,
		"network "CMD_KEY_IPV4_PREFIX,
		"network configure\n"
		CMD_KEY_IPV4_HELP)
{
	int ret = ERROR;
	struct prefix address;
	nsm_dhcps_t *dhcps = vty->index;
	prefix_zero(&address);
	if(dhcps)
	{
		if(argc == 1)
		{
			ret = str2prefix_ipv4 (argv[0], (struct prefix_ipv4 *)&address);
			if (ret <= 0)
			{
				vty_out (vty, "%% Malformed address %s", VTY_NEWLINE);
				return CMD_WARNING;
			}
			ret = nsm_dhcps_set_api(dhcps, DHCPS_CMD_NETWORK, &address);
		}
		else
		{

			char prefix[64];
			memset(prefix, 0, sizeof(prefix));
			netmask_str2prefix_str(argv[0], argv[2], prefix);
			ret = str2prefix_ipv4 (prefix, (struct prefix_ipv4 *)&address);
			if (ret <= 0)
			{
				vty_out (vty, "%% Malformed address %s", VTY_NEWLINE);
				return CMD_WARNING;
			}
			ret = nsm_dhcps_set_api(dhcps, DHCPS_CMD_NETWORK_START, &address);
			if(ret == OK)
			{
				ret = str2prefix_ipv4 (argv[1], (struct prefix_ipv4 *)&address);
				if (ret <= 0)
				{
					vty_out (vty, "%% Malformed address %s", VTY_NEWLINE);
					return CMD_WARNING;
				}
				ret = nsm_dhcps_set_api(dhcps, DHCPS_CMD_NETWORK_END, &address);
			}
		}
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

/*
ALIAS(dhcp_network,
		dhcp_network_range_cmd,
		"network " CMD_KEY_IPV4 " " CMD_KEY_IPV4 " " CMD_KEY_IPV4,
		"network configure\n"
		"Specify IPv4 address of start in pool(e.g. 0.0.0.0)\n"
		"Specify IPv4 address of end in pool(e.g. 0.0.0.0)\n"
		"Specify IPv4 address of netmask in pool(e.g. 0.0.0.0)\n");
*/

DEFUN (dhcp_address_range,
		dhcp_address_range_cmd,
		"address range " CMD_KEY_IPV4 " " CMD_KEY_IPV4,
		"ip address configure\n"
		"range configure\n"
		"Specify IPv4 address of start in pool(e.g. 0.0.0.0)\n"
		"Specify IPv4 address of end in pool(e.g. 0.0.0.0)\n")
{
	int ret = ERROR;
	struct prefix address;
	nsm_dhcps_t *dhcps = vty->index;
	prefix_zero(&address);
	if(dhcps)
	{
		if(argc == 2)
		{
			ret = str2prefix_ipv4 (argv[0], (struct prefix_ipv4 *)&address);
			if (ret <= 0)
			{
				vty_out (vty, "%% Malformed address %s", VTY_NEWLINE);
				return CMD_WARNING;
			}
			ret = nsm_dhcps_set_api(dhcps, DHCPS_CMD_NETWORK_START, &address);
			if(ret == OK)
			{
				prefix_zero(&address);
				ret = str2prefix_ipv4 (argv[1], (struct prefix_ipv4 *)&address);
				if (ret <= 0)
				{
					vty_out (vty, "%% Malformed address %s", VTY_NEWLINE);
					return CMD_WARNING;
				}
				ret = nsm_dhcps_set_api(dhcps, DHCPS_CMD_NETWORK_END, &address);
			}
		}
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_dhcp_address_range,
		no_dhcp_address_range_cmd,
		"no address range",
		NO_STR
		"ip address configure\n"
		"range configure\n")
{
	int ret = ERROR;
	nsm_dhcps_t *dhcps = vty->index;
	if(dhcps)
	{
		ret = nsm_dhcps_unset_api(dhcps, DHCPS_CMD_NETWORK);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (no_dhcp_network,
		no_dhcp_network_cmd,
		"no network",
		NO_STR
		"network configure\n")
{
	int ret = ERROR;
	nsm_dhcps_t *dhcps = vty->index;
	if(dhcps)
	{
		ret = nsm_dhcps_unset_api(dhcps, DHCPS_CMD_NETWORK);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (dhcp_default_router,
		dhcp_default_router_cmd,
		"default-router "CMD_KEY_IPV4,
		"default router configure\n"
		CMD_KEY_IPV4_HELP)
{
	int ret = ERROR;
	struct prefix address;
	nsm_dhcps_t *dhcps = vty->index;
	prefix_zero(&address);
	if(dhcps)
	{
		ret = str2prefix_ipv4 (argv[0], (struct prefix_ipv4 *)&address);
		if (ret <= 0)
		{
			vty_out (vty, "%% Malformed address %s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		if(argc == 1)
			ret = nsm_dhcps_set_api(dhcps, DHCPS_CMD_GATEWAY, &address);
		else
			ret = nsm_dhcps_set_api(dhcps, DHCPS_CMD_GATEWAY_SECONDARY, &address);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(dhcp_default_router,
		dhcp_default_router_sec_cmd,
		"default-router "CMD_KEY_IPV4 "(secondary|)",
		"default router configure\n"
		CMD_KEY_IPV4_HELP
		"secondary default router\n");


DEFUN (no_dhcp_default_router,
		no_dhcp_default_router_cmd,
		"no default-router",
		NO_STR
		"default router configure\n")
{
	int ret = 0;
	nsm_dhcps_t *dhcps = vty->index;
	if(dhcps)
	{
		if(argc == 1)
			ret = nsm_dhcps_unset_api(dhcps, DHCPS_CMD_GATEWAY);
		else
			ret = nsm_dhcps_unset_api(dhcps, DHCPS_CMD_GATEWAY_SECONDARY);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(no_dhcp_default_router,
		no_dhcp_default_router_sec_cmd,
		"no default-router (secondary|)",
		NO_STR
		"default router configure\n"
		"secondary default router\n");



DEFUN (dhcp_netbios,
		dhcp_netbios_cmd,
		"netbios-name-server "CMD_KEY_IPV4,
		"netbios name server configure\n"
		CMD_KEY_IPV4_HELP)
{
	int ret = ERROR;
	struct prefix address;
	nsm_dhcps_t *dhcps = vty->index;
	prefix_zero(&address);
	if(dhcps)
	{
		ret = str2prefix_ipv4 (argv[0], (struct prefix_ipv4 *)&address);
		if (ret <= 0)
		{
			vty_out (vty, "%% Malformed address %s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		if(argc == 1)
			ret = nsm_dhcps_set_api(dhcps, DHCPS_CMD_NETBIOS, &address);
		else
			ret = nsm_dhcps_set_api(dhcps, DHCPS_CMD_NETBIOS_SECONDARY, &address);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(dhcp_netbios,
		dhcp_netbios_sec_cmd,
		"netbios-name-server "CMD_KEY_IPV4 "(secondary|)",
		"netbios name server configure\n"
		CMD_KEY_IPV4_HELP
		"secondary netbios name server\n");


DEFUN (no_dhcp_netbios,
		no_dhcp_netbios_cmd,
		"no netbios-name-server",
		NO_STR
		"netbios name server configure\n")
{
	int ret = 0;
	nsm_dhcps_t *dhcps = vty->index;
	if(dhcps)
	{
		if(argc == 1)
			ret = nsm_dhcps_unset_api(dhcps, DHCPS_CMD_NETBIOS);
		else
			ret = nsm_dhcps_unset_api(dhcps, DHCPS_CMD_NETBIOS_SECONDARY);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(no_dhcp_netbios,
		no_dhcp_netbios_sec_cmd,
		"no netbios-name-server (secondary|)",
		NO_STR
		"netbios name server configure\n"
		"secondary netbios name server\n");


DEFUN (dhcp_dns_server,
		dhcp_dns_server_cmd,
		"dns-server "CMD_KEY_IPV4,
		"domain name server configure\n"
		CMD_KEY_IPV4_HELP)
{
	int ret = ERROR;
	struct prefix address;
	nsm_dhcps_t *dhcps = vty->index;
	prefix_zero(&address);
	if(dhcps)
	{
		ret = str2prefix_ipv4 (argv[0], (struct prefix_ipv4 *)&address);
		if (ret <= 0)
		{
			vty_out (vty, "%% Malformed address %s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		if(argc == 1)
			ret = nsm_dhcps_set_api(dhcps, DHCPS_CMD_DNS, &address);
		else
			ret = nsm_dhcps_set_api(dhcps, DHCPS_CMD_DNS_SECONDARY, &address);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(dhcp_dns_server,
		dhcp_dns_server_sec_cmd,
		"dns-server "CMD_KEY_IPV4 "(secondary|)",
		"domain name server configure\n"
		CMD_KEY_IPV4_HELP
		"secondary domain name server\n");


DEFUN (no_dhcp_dns_server,
		no_dhcp_dns_server_cmd,
		"no dns-server",
		NO_STR
		"domain name server configure\n")
{
	int ret = 0;
	nsm_dhcps_t *dhcps = vty->index;
	if(dhcps)
	{
		if(argc == 1)
			ret = nsm_dhcps_unset_api(dhcps, DHCPS_CMD_DNS);
		else
			ret = nsm_dhcps_unset_api(dhcps, DHCPS_CMD_DNS_SECONDARY);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(no_dhcp_dns_server,
		no_dhcp_dns_server_sec_cmd,
		"no dns-server (secondary|)",
		NO_STR
		"domain name server configure\n"
		"secondary domain name server\n");




DEFUN (dhcp_tftp_server,
		dhcp_tftp_server_cmd,
		"tftp-server "CMD_KEY_IPV4,
		"TFTP server configure\n"
		CMD_KEY_IPV4_HELP)
{
	int ret = ERROR;
	struct prefix address;
	nsm_dhcps_t *dhcps = vty->index;
	prefix_zero(&address);
	if(dhcps)
	{
		ret = str2prefix_ipv4 (argv[0], (struct prefix_ipv4 *)&address);
		if (ret <= 0)
		{
			vty_out (vty, "%% Malformed address %s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		ret = nsm_dhcps_set_api(dhcps, DHCPS_CMD_TFTP, &address);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_dhcp_tftp_server,
		no_dhcp_tftp_server_cmd,
		"no tftp-server",
		NO_STR
		"TFTP server configure\n")
{
	int ret = 0;
	nsm_dhcps_t *dhcps = vty->index;
	if(dhcps)
	{
		ret = nsm_dhcps_unset_api(dhcps, DHCPS_CMD_TFTP);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (dhcp_domain_name,
		dhcp_domain_name_cmd,
		"domain-name NAME",
		"domain name configure\n"
		"name of domain name\n")
{
	int ret = ERROR;
	nsm_dhcps_t *dhcps = vty->index;
	if(dhcps)
	{
		ret = nsm_dhcps_set_api(dhcps, DHCPS_CMD_DOMAIN_NAME, argv[0]);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_dhcp_domain_name,
		no_dhcp_domain_name_cmd,
		"no domain-name",
		NO_STR
		"domain name configure\n")
{
	int ret = 0;
	nsm_dhcps_t *dhcps = vty->index;
	if(dhcps)
	{
		ret = nsm_dhcps_unset_api(dhcps, DHCPS_CMD_DOMAIN_NAME);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (dhcp_lease_time,
		dhcp_lease_time_cmd,
		"lease <30-10080>",
		"dhcp lease configure\n"
		"lease time value of minute(default:120)\n")
{
	int ret = ERROR;
	nsm_dhcps_t *dhcps = vty->index;
	if(dhcps)
	{
		int value = atoi(argv[0]);
		ret = nsm_dhcps_set_api(dhcps, DHCPS_CMD_LEASE, &value);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_dhcp_lease_time,
		no_dhcp_lease_time_cmd,
		"no lease",
		NO_STR
		"dhcp lease configure\n")
{
	int ret = 0;
	nsm_dhcps_t *dhcps = vty->index;
	if(dhcps)
	{
		ret = nsm_dhcps_unset_api(dhcps, DHCPS_CMD_LEASE);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (nsm_interface_ip_dhcp_server,
		nsm_interface_ip_dhcp_server_cmd,
		"dhcp select server",
		"dhcp config commands\n"
		"dhcp select configure\n"
		"DHCP server\n")
{
	int ret = 0;
	BOOL mode = FALSE;
	struct interface *ifp = (struct interface *) vty->index;
	if(ifp)
	{
		//if(nsm_interface_dhcp_is_enable(ifp))
		{
			if(nsm_interface_dhcp_mode_get_api(ifp) == DHCP_SERVER)
				return CMD_SUCCESS;

			if(nsm_interface_dhcp_mode_get_api(ifp) != DHCP_NONE)
			{
				vty_out (vty, "%% This interface is already enable dhcp client/relay %s", VTY_NEWLINE);
				return CMD_WARNING;
			}
		}
/*
		if(nsm_interface_dhcp_enable(ifp, TRUE) != OK)
		{
			vty_out (vty, "%% Can not enable dhcp on this interface%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
*/
		ret = nsm_interface_dhcp_mode_set_api(ifp, DHCP_SERVER);
		if(ret == ERROR)
			vty_out (vty, "%% Can not enable dhcp server on this interface%s",VTY_NEWLINE);
		return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	return CMD_WARNING;
}

DEFUN (no_nsm_interface_ip_dhcp_server,
		no_nsm_interface_ip_dhcp_server_cmd,
		"no dhcp select server",
		NO_STR
		"dhcp config commands\n"
		"dhcp select configure\n"
		"DHCP server\n")
{
	int ret = 0;
	BOOL mode = FALSE;
	struct interface *ifp = (struct interface *) vty->index;
	if(ifp)
	{
		if(nsm_interface_dhcp_mode_get_api(ifp) == DHCP_NONE)
			return CMD_SUCCESS;

		if(nsm_interface_dhcp_mode_get_api(ifp) != DHCP_SERVER)
		{
			vty_out (vty, "%% This interface is already enable dhcp client/relay %s", VTY_NEWLINE);
			return CMD_WARNING;
		}

		ret = nsm_interface_dhcp_mode_set_api(ifp, DHCP_NONE);
		if(ret == ERROR)
			vty_out (vty, "%% Can not disable dhcp server on this interface%s",VTY_NEWLINE);
		return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	return CMD_WARNING;
}


static void cmd_base_interface_dhcpd_init(int node)
{
	install_element(node, &nsm_interface_ip_dhcp_server_cmd);
	install_element(node, &no_nsm_interface_ip_dhcp_server_cmd);
}


static void cmd_base_dhcps_init(int node)
{
	install_element(node, &dhcp_network_cmd);
	//install_element(node, &dhcp_network_range_cmd);
	install_element(node, &no_dhcp_network_cmd);

	install_element(node, &dhcp_address_range_cmd);
	install_element(node, &no_dhcp_address_range_cmd);

	install_element(node, &dhcp_default_router_cmd);
	install_element(node, &dhcp_default_router_sec_cmd);
	install_element(node, &no_dhcp_default_router_cmd);
	install_element(node, &no_dhcp_default_router_sec_cmd);


	install_element(node, &dhcp_netbios_cmd);
	install_element(node, &dhcp_netbios_sec_cmd);
	install_element(node, &no_dhcp_netbios_cmd);
	install_element(node, &no_dhcp_netbios_sec_cmd);

	install_element(node, &dhcp_dns_server_cmd);
	install_element(node, &dhcp_dns_server_sec_cmd);
	install_element(node, &no_dhcp_dns_server_cmd);
	install_element(node, &no_dhcp_dns_server_sec_cmd);


	install_element(node, &dhcp_tftp_server_cmd);
	install_element(node, &no_dhcp_tftp_server_cmd);

	install_element(node, &dhcp_domain_name_cmd);
	install_element(node, &no_dhcp_domain_name_cmd);

	install_element(node, &dhcp_lease_time_cmd);
	install_element(node, &no_dhcp_lease_time_cmd);
}

static void cmd_show_dhcps_init(int node)
{
/*	install_element(node, &nsm_show_dhcp_client_cmd);
	install_element(node, &nsm_show_dhcp_client_interface_cmd);

	install_element(node, &nsm_show_dhcp_client_detail_cmd);
	install_element(node, &nsm_show_dhcp_client_detail_interface_cmd);*/
}

void cmd_dhcps_init(void)
{

	install_default(DHCPS_NODE);
	install_default_basic(DHCPS_NODE);
	reinstall_node(DHCPS_NODE, nsm_dhcps_write_config);

	install_element(CONFIG_NODE, &ip_dhcp_pool_cmd);
	install_element(CONFIG_NODE, &no_ip_dhcp_pool_cmd);

	cmd_base_dhcps_init(DHCPS_NODE);

	cmd_show_dhcps_init(ENABLE_NODE);
	cmd_show_dhcps_init(CONFIG_NODE);

	cmd_base_interface_dhcpd_init(INTERFACE_L3_NODE);
	cmd_base_interface_dhcpd_init(LAG_INTERFACE_L3_NODE);
	cmd_base_interface_dhcpd_init(WIRELESS_INTERFACE_NODE);
}
