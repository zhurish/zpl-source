/*
 * cmd_dns.c
 *
 *  Created on: Oct 13, 2018
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
#include "nsm_dns.h"


DEFUN (ip_dns_server_add,
		ip_dns_server_add_cmd,
		"dns server " CMD_KEY_IPV4,
		"DNS Information\n"
		"Server Information\n"
		CMD_KEY_IPV4_HELP)
{
	int ret = ERROR;
	struct prefix address;
	ip_dns_t *dns = NULL;
	if (argc < 1)
		return CMD_WARNING;

	ret = str2prefix (argv[0], &address);
	if (ret <= 0)
	{
		vty_out (vty, "%% Malformed address%s", VTY_NEWLINE);
		return CMD_WARNING;
	}

	dns = nsm_ip_dns_lookup_api( &address, IP_DNS_STATIC);
	if (dns)
	{
		if(argc == 1)
		{
			if(!dns->_dns_secondly)
			{
				vty_out (vty, "%% This DNS Server is already exist.%s", VTY_NEWLINE);
				return CMD_WARNING;
			}
			ret = nsm_ip_dns_add_api(&address, FALSE);
		}
		else if(argc == 2)
		{
			if(dns->_dns_secondly)
			{
				vty_out (vty, "%% This DNS Server is already exist.%s", VTY_NEWLINE);
				return CMD_WARNING;
			}
			ret = nsm_ip_dns_add_api(&address, TRUE);
		}
	}
	else
	{
		if(argc == 1)
		{
			ret = nsm_ip_dns_add_api(&address, FALSE);
		}
		else if(argc == 2)
		{
			ret = nsm_ip_dns_add_api(&address, TRUE);
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


ALIAS (ip_dns_server_add,
		ip_dns_server_add_secondary_cmd,
		"dns server " CMD_KEY_IPV4 " (secondary|)",
		"DNS Information\n"
		"Server Information\n"
		CMD_KEY_IPV4_HELP
		"Secondary\n");



DEFUN (no_ip_dns_server_add,
		no_ip_dns_server_add_cmd,
		"no dns server " CMD_KEY_IPV4,
		NO_STR
		"DNS Information\n"
		"Server Information\n"
		CMD_KEY_IPV4_HELP)
{
	int ret = ERROR;
	struct prefix address;
	ip_dns_t *dns = NULL;
	if (argc < 1)
		return CMD_WARNING;

	ret = str2prefix (argv[0], &address);
	if (ret <= 0)
	{
		vty_out (vty, "%% Malformed address%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	dns = nsm_ip_dns_lookup_api( &address, IP_DNS_STATIC);
	if (dns)
	{
		ret = nsm_ip_dns_del_api(&address);
	}
	else
	{
		vty_out (vty, "%% This DNS Server is not exist.%s", VTY_NEWLINE);
		return CMD_WARNING;
	}

	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


/*
 * ip host
 */

DEFUN (ip_host_add,
		ip_host_add_cmd,
		"ip host " CMD_KEY_IPV4 " HOSTNAME",
		IP_STR
		"Host Information\n"
		CMD_KEY_IPV4_HELP
		"Host name\n")
{
	int ret = ERROR;
	struct prefix address;
	ip_host_t *dns = NULL;
	if (argc < 1)
		return CMD_WARNING;

	ret = str2prefix (argv[0], &address);
	if (ret <= 0)
	{
		vty_out (vty, "%% Malformed address%s", VTY_NEWLINE);
		return CMD_WARNING;
	}

	dns = nsm_ip_host_lookup_api( &address, IP_HOST_STATIC);
	if (dns)
	{
		vty_out (vty, "%% This host is already exist.%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	else
	{
		ret = nsm_ip_host_add_api(&address, argv[1]);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_host_add,
		no_ip_host_add_cmd,
		"no ip host " CMD_KEY_IPV4,
		NO_STR
		IP_STR
		"Host Information\n"
		CMD_KEY_IPV4_HELP)
{
	int ret = ERROR;
	struct prefix address;
	ip_host_t *dns = NULL;
	if (argc < 1)
		return CMD_WARNING;

	ret = str2prefix (argv[0], &address);
	if (ret <= 0)
	{
		vty_out (vty, "%% Malformed address%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	dns = nsm_ip_host_lookup_api( &address, IP_HOST_STATIC);
	if (!dns)
	{
		vty_out (vty, "%% This host is not exist.%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	else
	{
		ret = nsm_ip_host_del_api(&address);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (debug_dns_set,
		debug_dns_set_cmd,
		"debug dns (db|event)",
		DEBUG_STR
		"DNS Information\n"
		"DNS DB information\n"
		"DNS Event information\n")
{
	if(strncmp(argv[0], "db", 2) == 2)
		NSM_DNS_DEBUG_ON(IP_DNS_DEBUG);
	else
		NSM_DNS_DEBUG_ON(IP_DNS_EVENT_DEBUG);
	return CMD_SUCCESS;
}

DEFUN (no_debug_dns_set,
		no_debug_dns_set_cmd,
		"no debug dns (db|event)",
		NO_STR
		DEBUG_STR
		"DNS Information\n"
		"DNS DB information\n"
		"DNS Event information\n")
{
	if(strncmp(argv[0], "db", 2) == 2)
		NSM_DNS_DEBUG_OFF(IP_DNS_DEBUG);
	else
		NSM_DNS_DEBUG_OFF(IP_DNS_EVENT_DEBUG);
	return CMD_SUCCESS;
}


int nsm_dns_debug_write(struct vty *vty)
{
	if(NSM_DNS_IS_DEBUG(IP_DNS_DEBUG))
		vty_out(vty, "debug dns db%s", VTY_NEWLINE);
	if(NSM_DNS_DEBUG_OFF(IP_DNS_EVENT_DEBUG))
		vty_out(vty, "debug dns event%s", VTY_NEWLINE);
	return CMD_SUCCESS;
}


DEFUN (show_dns_server,
		show_dns_server_cmd,
		"show dns server",
		SHOW_STR
		"DNS Information\n"
		"Server Information\n")
{
	return nsm_ip_dns_host_show(vty);
}



void cmd_dns_init(void)
{
	install_element(VIEW_NODE, &show_dns_server_cmd);
	install_element(ENABLE_NODE, &show_dns_server_cmd);
	install_element(CONFIG_NODE, &show_dns_server_cmd);

	install_element(ENABLE_NODE, &debug_dns_set_cmd);
	install_element(CONFIG_NODE, &debug_dns_set_cmd);
	install_element(ENABLE_NODE, &no_debug_dns_set_cmd);
	install_element(CONFIG_NODE, &no_debug_dns_set_cmd);


	install_element(CONFIG_NODE, &ip_dns_server_add_cmd);
	install_element(CONFIG_NODE, &ip_dns_server_add_secondary_cmd);
	install_element(CONFIG_NODE, &no_ip_dns_server_add_cmd);

	install_element(CONFIG_NODE, &ip_host_add_cmd);
	install_element(CONFIG_NODE, &no_ip_host_add_cmd);
}
