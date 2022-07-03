/*
 * nsm_global.c
 *
 *  Created on: May 8, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include <zplos_include.h>
#include "if.h"
#include "command.h"
#include "prefix.h"
#include "nsm_global.h"
#include "vty.h"




DEFUN(global_jumboframe_size,
	  global_jumboframe_size_cmd,
	  "system jumboframe size <1518-8192>",
	  "Gloabl System\n"
	  "Jumboframe\n"
	  "Jumboframe size\n"
	  "Size Value\n")
{
	int ret = ERROR;
	zpl_uint32 value = 0;
	ret = nsm_global_jumbo_size_get(&value);
	if (ret == OK)
	{
		if (value != atoi(argv[0]))
			ret = nsm_global_jumbo_size_set(atoi(argv[0]));
		else
			ret = OK;
	}
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(no_global_jumboframe_size,
	  no_global_jumboframe_size_cmd,
	  "no system jumboframe size",
	  NO_STR
	  "Gloabl System\n"
	  "Jumboframe\n"
	  "Jumboframe size\n"
	  "Size Value\n")
{
	int ret = ERROR;
	zpl_uint32 value = 0;
	ret = nsm_global_jumbo_size_get(&value);
	if (ret == OK)
	{
		if (value != 0)
			ret = nsm_global_jumbo_size_set(value);
		else
			ret = OK;
	}
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(global_switch_forward,
	  global_switch_forward_cmd,
	  "system switch forward",
	  "Gloabl System\n"
	  "Switch\n"
	  "Forward\n")
{
	int ret = ERROR;
	zpl_bool value = 0;
	ret = nsm_global_switch_forward_get(&value);
	if (ret == OK)
	{
		if (value != zpl_true)
			ret = nsm_global_switch_forward_set(value);
		else
			ret = OK;
	}
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(no_global_switch_forward,
	  no_global_switch_forward_cmd,
	  "no system switch forward",
	  NO_STR
	  "Gloabl System\n"
	  "Switch\n"
	  "Forward\n")
{
	int ret = ERROR;
	zpl_bool value = 0;
	ret = nsm_global_switch_forward_get(&value);
	if (ret == OK)
	{
		if (value != zpl_false)
			ret = nsm_global_switch_forward_set(value);
		else
			ret = OK;
	}
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(global_unicast_flood,
	  global_unicast_flood_cmd,
	  "system unicast flood",
	  "Gloabl System\n"
	  "Unicast Pcaket\n"
	  "Flood\n")
{
	int ret = ERROR;
	zpl_bool value = 0;
	ret = nsm_global_unicast_flood_get(&value);
	if (ret == OK)
	{
		if (value != zpl_true)
			ret = nsm_global_unicast_flood_set(value);
		else
			ret = OK;
	}
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(no_global_unicast_flood,
	  no_global_unicast_flood_cmd,
	  "no system unicast flood",
	  NO_STR
	  "Gloabl System\n"
	  "Unicast Pcaket\n"
	  "Flood\n")
{
	int ret = ERROR;
	zpl_bool value = 0;
	ret = nsm_global_unicast_flood_get(&value);
	if (ret == OK)
	{
		if (value != zpl_false)
			ret = nsm_global_unicast_flood_set(value);
		else
			ret = OK;
	}
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(global_multicast_flood,
	  global_multicast_flood_cmd,
	  "system multicast flood",
	  "Gloabl System\n"
	  "Multicast Pcaket\n"
	  "Flood\n")
{
	int ret = ERROR;
	zpl_bool value = 0;
	ret = nsm_global_multicast_flood_get(&value);
	if (ret == OK)
	{
		if (value != zpl_true)
			ret = nsm_global_multicast_flood_set(value);
		else
			ret = OK;
	}
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(no_global_multicast_flood,
	  no_global_multicast_flood_cmd,
	  "no system multicast flood",
	  NO_STR
	  "Gloabl System\n"
	  "Multicast Pcaket\n"
	  "Flood\n")
{
	int ret = ERROR;
	zpl_bool value = 0;
	ret = nsm_global_multicast_flood_get(&value);
	if (ret == OK)
	{
		if (value != zpl_false)
			ret = nsm_global_multicast_flood_set(value);
		else
			ret = OK;
	}
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(global_multicast_learning,
	  global_multicast_learning_cmd,
	  "system multicast learning",
	  "Gloabl System\n"
	  "Multicast Pcaket\n"
	  "Learning\n")
{
	int ret = ERROR;
	zpl_bool value = 0;
	ret = nsm_global_multicast_learning_get(&value);
	if (ret == OK)
	{
		if (value != zpl_true)
			ret = nsm_global_multicast_learning_set(value);
		else
			ret = OK;
	}
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(no_global_multicast_learning,
	  no_global_multicast_learning_cmd,
	  "no system multicast learning",
	  NO_STR
	  "Gloabl System\n"
	  "Multicast Pcaket\n"
	  "Learning\n")
{
	int ret = ERROR;
	zpl_bool value = 0;
	ret = nsm_global_multicast_learning_get(&value);
	if (ret == OK)
	{
		if (value != zpl_false)
			ret = nsm_global_multicast_learning_set(value);
		else
			ret = OK;
	}
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(global_l2bpdu_enable,
	  global_l2bpdu_enable_cmd,
	  "system l2 bpdu enable",
	  "Gloabl System\n"
	  "L2 Pcaket\n"
	  "Bpdu Pcaket\n"
	  "Enable\n")
{
	int ret = ERROR;
	zpl_bool value = 0;
	ret = nsm_global_bpdu_get(&value);
	if (ret == OK)
	{
		if (value != zpl_true)
			ret = nsm_global_bpdu_set(value);
		else
			ret = OK;
	}
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(no_global_l2bpdu_enable,
	  no_global_l2bpdu_enable_cmd,
	  "no system l2 bpdu enable",
	  NO_STR
	  "Gloabl System\n"
	  "L2 Pcaket\n"
	  "Bpdu Pcaket\n"
	  "Enable\n")
{
	int ret = ERROR;
	zpl_bool value = 0;
	ret = nsm_global_bpdu_get(&value);
	if (ret == OK)
	{
		if (value != zpl_false)
			ret = nsm_global_bpdu_set(value);
		else
			ret = OK;
	}
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}
#ifdef ZPL_NSM_IGMP
DEFUN(global_igmp_enable,
	  global_igmp_enable_cmd,
	  "igmp (snooping|proxy) enable",
	  "IGMP Protocol\n"
	  "Snooping Pcaket\n"
	  "Proxy Pcaket\n"
	  "Enable\n")
{
	int ret = ERROR;
	zpl_bool value = zpl_false;
	enum nsm_proto_action action;
	if (os_memcmp(argv[0], "snooping", 3) == 0)
		action = NSM_PKT_SNOOPING;
	else
		action = NSM_PKT_PROXY;
	ret = nsm_snooping_proto_get(NSM_SNOOP_PROTO_IGMP, action, &value);
	if (ret == OK)
	{
		if (value != zpl_true)
		{
			if (action == NSM_PKT_PROXY)
			{
				value = zpl_false;
				ret = nsm_snooping_proto_get(NSM_SNOOP_PROTO_IGMP, NSM_PKT_SNOOPING, &value);
				if (value == zpl_true)
				{
					vty_out(vty, "Error:igmp snooping is enable, please disable it.%s", VTY_NEWLINE);
					return CMD_WARNING;
				}
			}
			else if (action == NSM_PKT_SNOOPING)
			{
				value = zpl_false;
				ret = nsm_snooping_proto_get(NSM_SNOOP_PROTO_IGMP, NSM_PKT_PROXY, &value);
				if (value == zpl_true)
				{
					vty_out(vty, "Error:igmp proxy is enable, please disable it.%s", VTY_NEWLINE);
					return CMD_WARNING;
				}
			}
			ret = nsm_snooping_proto_set(NSM_SNOOP_PROTO_IGMP, action, zpl_true);
		}
		else
			ret = OK;
	}
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(no_global_igmp_enable,
	  no_global_igmp_enable_cmd,
	  "no igmp (snooping|proxy) enable",
	  NO_STR
	  "IGMP Protocol\n"
	  "Snooping Pcaket\n"
	  "Proxy Pcaket\n"
	  "Enable\n")
{
	int ret = ERROR;
	zpl_bool value = zpl_false;
	enum nsm_proto_action action;
	if (os_memcmp(argv[0], "snooping", 3) == 0)
		action = NSM_PKT_SNOOPING;
	else
		action = NSM_PKT_PROXY;
	ret = nsm_snooping_proto_get(NSM_SNOOP_PROTO_IGMP, action, &value);
	if (ret == OK)
	{
		if (value == zpl_true)
		{
			ret = nsm_snooping_proto_set(NSM_SNOOP_PROTO_IGMP, action, zpl_false);
		}
		else
			ret = OK;
	}
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(global_igmpqry_enable,
	  global_igmpqry_enable_cmd,
	  "igmp qry (snooping|proxy) enable",
	  "IGMP Protocol\n"
	  "IGMP Qry Protocol\n"
	  "Snooping Pcaket\n"
	  "Proxy Pcaket\n"
	  "Enable\n")
{
	int ret = ERROR;
	zpl_bool value = zpl_false;
	enum nsm_proto_action action;
	if (os_memcmp(argv[0], "snooping", 3) == 0)
		action = NSM_PKT_SNOOPING;
	else
		action = NSM_PKT_PROXY;
	ret = nsm_snooping_proto_get(NSM_SNOOP_PROTO_IGMPQRY, action, &value);
	if (ret == OK)
	{
		if (value != zpl_true)
		{
			if (action == NSM_PKT_PROXY)
			{
				value = zpl_false;
				ret = nsm_snooping_proto_get(NSM_SNOOP_PROTO_IGMPQRY, NSM_PKT_SNOOPING, &value);
				if (value == zpl_true)
				{
					vty_out(vty, "Error:igmp snooping is enable, please disable it.%s", VTY_NEWLINE);
					return CMD_WARNING;
				}
			}
			else if (action == NSM_PKT_SNOOPING)
			{
				value = zpl_false;
				ret = nsm_snooping_proto_get(NSM_SNOOP_PROTO_IGMPQRY, NSM_PKT_PROXY, &value);
				if (value == zpl_true)
				{
					vty_out(vty, "Error:igmp proxy is enable, please disable it.%s", VTY_NEWLINE);
					return CMD_WARNING;
				}
			}
			ret = nsm_snooping_proto_set(NSM_SNOOP_PROTO_IGMPQRY, action, zpl_true);
		}
		else
			ret = OK;
	}
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(no_global_igmpqry_enable,
	  no_global_igmpqry_enable_cmd,
	  "no igmp qry (snooping|proxy) enable",
	  NO_STR
	  "IGMP Protocol\n"
	  "IGMP Qry Protocol\n"
	  "Snooping Pcaket\n"
	  "Proxy Pcaket\n"
	  "Enable\n")
{
	int ret = ERROR;
	zpl_bool value = zpl_false;
	enum nsm_proto_action action;
	if (os_memcmp(argv[0], "snooping", 3) == 0)
		action = NSM_PKT_SNOOPING;
	else
		action = NSM_PKT_PROXY;
	ret = nsm_snooping_proto_get(NSM_SNOOP_PROTO_IGMPQRY, action, &value);
	if (ret == OK)
	{
		if (value == zpl_true)
		{
			ret = nsm_snooping_proto_set(NSM_SNOOP_PROTO_IGMPQRY, action, zpl_false);
		}
		else
			ret = OK;
	}
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(global_igmpunknow_enable,
	  global_igmpunknow_enable_cmd,
	  "igmp unknow (snooping|proxy) enable",
	  "IGMP Protocol\n"
	  "IGMP Unknow\n"
	  "Snooping Pcaket\n"
	  "Proxy Pcaket\n"
	  "Enable\n")
{
	int ret = ERROR;
	zpl_bool value = zpl_false;
	enum nsm_proto_action action;
	if (os_memcmp(argv[0], "snooping", 3) == 0)
		action = NSM_PKT_SNOOPING;
	else
		action = NSM_PKT_PROXY;
	ret = nsm_snooping_proto_get(NSM_SNOOP_PROTO_IGMPUNKNOW, action, &value);
	if (ret == OK)
	{
		if (value != zpl_true)
		{
			if (action == NSM_PKT_PROXY)
			{
				value = zpl_false;
				ret = nsm_snooping_proto_get(NSM_SNOOP_PROTO_IGMPUNKNOW, NSM_PKT_SNOOPING, &value);
				if (value == zpl_true)
				{
					vty_out(vty, "Error:igmp snooping is enable, please disable it.%s", VTY_NEWLINE);
					return CMD_WARNING;
				}
			}
			else if (action == NSM_PKT_SNOOPING)
			{
				value = zpl_false;
				ret = nsm_snooping_proto_get(NSM_SNOOP_PROTO_IGMPUNKNOW, NSM_PKT_PROXY, &value);
				if (value == zpl_true)
				{
					vty_out(vty, "Error:igmp proxy is enable, please disable it.%s", VTY_NEWLINE);
					return CMD_WARNING;
				}
			}
			ret = nsm_snooping_proto_set(NSM_SNOOP_PROTO_IGMPUNKNOW, action, zpl_true);
		}
		else
			ret = OK;
	}
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(no_global_igmpunknow_enable,
	  no_global_igmpunknow_enable_cmd,
	  "no igmp unknow (snooping|proxy) enable",
	  NO_STR
	  "IGMP Protocol\n"
	  "IGMP Unknow\n"
	  "Snooping Pcaket\n"
	  "Proxy Pcaket\n"
	  "Enable\n")
{
	int ret = ERROR;
	zpl_bool value = zpl_false;
	enum nsm_proto_action action;
	if (os_memcmp(argv[0], "snooping", 3) == 0)
		action = NSM_PKT_SNOOPING;
	else
		action = NSM_PKT_PROXY;
	ret = nsm_snooping_proto_get(NSM_SNOOP_PROTO_IGMPUNKNOW, action, &value);
	if (ret == OK)
	{
		if (value == zpl_true)
		{
			ret = nsm_snooping_proto_set(NSM_SNOOP_PROTO_IGMPUNKNOW, action, zpl_false);
		}
		else
			ret = OK;
	}
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(global_mld_enable,
	  global_mld_enable_cmd,
	  "mld (snooping|proxy) enable",
	  "MLD Protocol\n"
	  "Snooping Pcaket\n"
	  "Proxy Pcaket\n"
	  "Enable\n")
{
	int ret = ERROR;
	zpl_bool value = zpl_false;
	enum nsm_proto_action action;
	if (os_memcmp(argv[0], "snooping", 3) == 0)
		action = NSM_PKT_SNOOPING;
	else
		action = NSM_PKT_PROXY;
	ret = nsm_snooping_proto_get(NSM_SNOOP_PROTO_MLD, action, &value);
	if (ret == OK)
	{
		if (value != zpl_true)
		{
			if (action == NSM_PKT_PROXY)
			{
				value = zpl_false;
				ret = nsm_snooping_proto_get(NSM_SNOOP_PROTO_MLD, NSM_PKT_SNOOPING, &value);
				if (value == zpl_true)
				{
					vty_out(vty, "Error:igmp snooping is enable, please disable it.%s", VTY_NEWLINE);
					return CMD_WARNING;
				}
			}
			else if (action == NSM_PKT_SNOOPING)
			{
				value = zpl_false;
				ret = nsm_snooping_proto_get(NSM_SNOOP_PROTO_MLD, NSM_PKT_PROXY, &value);
				if (value == zpl_true)
				{
					vty_out(vty, "Error:igmp proxy is enable, please disable it.%s", VTY_NEWLINE);
					return CMD_WARNING;
				}
			}
			ret = nsm_snooping_proto_set(NSM_SNOOP_PROTO_MLD, action, zpl_true);
		}
		else
			ret = OK;
	}
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(no_global_mld_enable,
	  no_global_mld_enable_cmd,
	  "no mld (snooping|proxy) enable",
	  NO_STR
	  "MLD Protocol\n"
	  "Snooping Pcaket\n"
	  "Proxy Pcaket\n"
	  "Enable\n")
{
	int ret = ERROR;
	zpl_bool value = zpl_false;
	enum nsm_proto_action action;
	if (os_memcmp(argv[0], "snooping", 3) == 0)
		action = NSM_PKT_SNOOPING;
	else
		action = NSM_PKT_PROXY;
	ret = nsm_snooping_proto_get(NSM_SNOOP_PROTO_MLD, action, &value);
	if (ret == OK)
	{
		if (value == zpl_true)
		{
			ret = nsm_snooping_proto_set(NSM_SNOOP_PROTO_MLD, action, zpl_false);
		}
		else
			ret = OK;
	}
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(global_mldqry_enable,
	  global_mldqry_enable_cmd,
	  "mld qry (snooping|proxy) enable",
	  "MLD Protocol\n"
	  "MLD Qry Protocol\n"
	  "Snooping Pcaket\n"
	  "Proxy Pcaket\n"
	  "Enable\n")
{
	int ret = ERROR;
	zpl_bool value = zpl_false;
	enum nsm_proto_action action;
	if (os_memcmp(argv[0], "snooping", 3) == 0)
		action = NSM_PKT_SNOOPING;
	else
		action = NSM_PKT_PROXY;
	ret = nsm_snooping_proto_get(NSM_SNOOP_PROTO_MLDQRY, action, &value);
	if (ret == OK)
	{
		if (value != zpl_true)
		{
			if (action == NSM_PKT_PROXY)
			{
				value = zpl_false;
				ret = nsm_snooping_proto_get(NSM_SNOOP_PROTO_MLDQRY, NSM_PKT_SNOOPING, &value);
				if (value == zpl_true)
				{
					vty_out(vty, "Error:igmp snooping is enable, please disable it.%s", VTY_NEWLINE);
					return CMD_WARNING;
				}
			}
			else if (action == NSM_PKT_SNOOPING)
			{
				value = zpl_false;
				ret = nsm_snooping_proto_get(NSM_SNOOP_PROTO_MLDQRY, NSM_PKT_PROXY, &value);
				if (value == zpl_true)
				{
					vty_out(vty, "Error:igmp proxy is enable, please disable it.%s", VTY_NEWLINE);
					return CMD_WARNING;
				}
			}
			ret = nsm_snooping_proto_set(NSM_SNOOP_PROTO_MLDQRY, action, zpl_true);
		}
		else
			ret = OK;
	}
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(no_global_mldqry_enable,
	  no_global_mldqry_enable_cmd,
	  "no mld qry (snooping|proxy) enable",
	  NO_STR
	  "MLD Protocol\n"
	  "MLD Qry Protocol\n"
	  "Snooping Pcaket\n"
	  "Proxy Pcaket\n"
	  "Enable\n")
{
	int ret = ERROR;
	zpl_bool value = zpl_false;
	enum nsm_proto_action action;
	if (os_memcmp(argv[0], "snooping", 3) == 0)
		action = NSM_PKT_SNOOPING;
	else
		action = NSM_PKT_PROXY;
	ret = nsm_snooping_proto_get(NSM_SNOOP_PROTO_MLDQRY, action, &value);
	if (ret == OK)
	{
		if (value == zpl_true)
		{
			ret = nsm_snooping_proto_set(NSM_SNOOP_PROTO_MLDQRY, action, zpl_false);
		}
		else
			ret = OK;
	}
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(global_iparpdhcp_enable,
	  global_iparpdhcp_enable_cmd,
	  "ip (arp|rarp|dhcp) snooping enable",
	  IP_STR
	  "ARP Protocol\n"
	  "RARP Protocol\n"
	  "DHCP Protocol\n"
	  "Snooping Pcaket\n"
	  "Enable\n")
{
	int ret = ERROR;
	zpl_bool value = zpl_false;
	enum nsm_snoop_proto_type type;

	if (os_memcmp(argv[0], "arp", 3) == 0)
		type = NSM_SNOOP_PROTO_ARP;
	else if (os_memcmp(argv[0], "rarp", 3) == 0)
		type = NSM_SNOOP_PROTO_RARP;
	else
		type = NSM_SNOOP_PROTO_DHCP;
	ret = nsm_snooping_proto_get(type, NSM_PKT_SNOOPING, &value);
	if (ret == OK)
	{
		if (value != zpl_true)
		{
			ret = nsm_snooping_proto_set(type, NSM_PKT_SNOOPING, zpl_true);
		}
		else
			ret = OK;
	}
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(no_global_iparpdhcp_enable,
	  no_global_iparpdhcp_enable_cmd,
	  "no ip (arp|rarp|dhcp) snooping enable",
	  NO_STR
		  IP_STR
	  "ARP Protocol\n"
	  "RARP Protocol\n"
	  "DHCP Protocol\n"
	  "Snooping Pcaket\n"
	  "Enable\n")
{
	int ret = ERROR;
	zpl_bool value = zpl_false;
	enum nsm_snoop_proto_type type;

	if (os_memcmp(argv[0], "arp", 3) == 0)
		type = NSM_SNOOP_PROTO_ARP;
	else if (os_memcmp(argv[0], "rarp", 3) == 0)
		type = NSM_SNOOP_PROTO_RARP;
	else
		type = NSM_SNOOP_PROTO_DHCP;
	ret = nsm_snooping_proto_get(type, NSM_PKT_SNOOPING, &value);
	if (ret == OK)
	{
		if (value != zpl_false)
		{
			ret = nsm_snooping_proto_set(type, NSM_PKT_SNOOPING, zpl_false);
		}
		else
			ret = OK;
	}
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}
#endif


DEFUN(global_cpu_rate,
	  global_cpu_rate_cmd,
	  "cpu rate limit <0-1388889>",
	  "Cpu System\n"
	  "Rate Configure\n"
	  "Limit Configure\n"
	  "Rate Value For 'pps'\n")
{
	int ret = ERROR;
	ret = nsm_cpu_rate_set_api(atoi(argv[0]));
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(no_global_cpu_rate,
	  no_global_cpu_rate_cmd,
	  "no cpu rate limit",
	  NO_STR
	  "Cpu System\n"
	  "Rate Configure\n"
	  "Limit Configure\n")
{
	int ret = ERROR;
	ret = nsm_cpu_rate_set_api(0);
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}


void cmd_global_init(void)
{
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &global_cpu_rate_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_global_cpu_rate_cmd);

	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &global_switch_forward_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_global_switch_forward_cmd);

	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &global_jumboframe_size_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_global_jumboframe_size_cmd);

	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &global_unicast_flood_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_global_unicast_flood_cmd);

	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &global_multicast_flood_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_global_multicast_flood_cmd);

	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &global_multicast_learning_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_global_multicast_learning_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &global_l2bpdu_enable_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_global_l2bpdu_enable_cmd);
#ifdef ZPL_NSM_IGMP
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &global_igmp_enable_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_global_igmp_enable_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &global_igmpqry_enable_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_global_igmpqry_enable_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &global_igmpunknow_enable_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_global_igmpunknow_enable_cmd);

	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &global_mld_enable_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_global_mld_enable_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &global_mldqry_enable_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_global_mldqry_enable_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &global_iparpdhcp_enable_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_global_iparpdhcp_enable_cmd);
#endif	
}
