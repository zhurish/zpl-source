/*
 * cmd_mirror.c
 *
 *  Created on: May 11, 2018
 *      Author: zhurish
 */
#include "zebra.h"
#include "vty.h"
#include "if.h"

#include "buffer.h"
#include "command.h"

#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "nsm_vrf.h"
#include "nsm_interface.h"
#include "if_name.h"
#include "nsm_mac.h"
#include "nsm_mirror.h"


/*
 * monitor session 1 source interface gigabitEthernet 1/0/1 （in|out|both）
 * monitor session 1 destination interface gigabitEthernet 1/0/6
 */

/*
 * global
 */
DEFUN (monitor_session_source,
		monitor_session_source_cmd,
		"monitor session source interface (ethernet|gigabitethernet) <unit/slot/port>",
		"Monitor configure\n"
		"Session configure\n"
		"Session source\n"
		INTERFACE_STR
		"Ethernet interface\n"
		"GigabitEthernet interface\n"
		CMD_USP_STR_HELP)
{
	int ret = ERROR;
	ifindex_t	ifindex = 0;
	mirror_dir_en dir = MIRROR_BOTH;
	if(!nsm_mirror_global_is_enable())
		ret = nsm_mirror_global_enable(TRUE);

	if (argv[0] && argv[1])
	{
		ifindex = if_ifindex_make(argv[0], argv[1]);
		if(ifindex)
		{
			if(nsm_mirror_is_destination_api(ifindex))
			{
				vty_out(vty, "ERROR: this mirror source interface is same to destination interface.%s", VTY_NEWLINE);
				return CMD_WARNING;
			}
			if(argc == 3)
			{
				dir = MIRROR_NONE;
				if(os_memcmp(argv[2], "ingress", 2) == 0)
					dir = MIRROR_INGRESS;
				else if(os_memcmp(argv[2], "egress", 2) == 0)
					dir = MIRROR_EGRESS;
				else if(os_memcmp(argv[2], "both", 2) == 0)
					dir = MIRROR_BOTH;
			}
			if(dir)
				ret = nsm_mirror_source_set_api(ifindex, TRUE, dir);
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


ALIAS(monitor_session_source,
		monitor_session_source_both_cmd,
		"monitor session source interface (ethernet|gigabitethernet) <unit/slot/port> (ingress|egress|both)",
		"Monitor configure\n"
		"Session configure\n"
		"Session source\n"
		INTERFACE_STR
		"Ethernet interface\n"
		"GigabitEthernet interface\n"
		CMD_USP_STR_HELP
		"Ingress stream\n"
		"Egress stream\n"
		"Ingress and Egress stream\n");



DEFUN (no_monitor_session_source,
		no_monitor_session_source_cmd,
		"no monitor session source interface (ethernet|gigabitethernet) <unit/slot/port>",
		NO_STR
		"Monitor configure\n"
		"Session configure\n"
		"Session source\n"
		INTERFACE_STR
		"Ethernet interface\n"
		"GigabitEthernet interface\n"
		CMD_USP_STR_HELP)
{
	int ret = ERROR;
	ifindex_t	ifindex = 0;
	mirror_dir_en dir = MIRROR_BOTH;
	if(!nsm_mirror_global_is_enable())
		return CMD_WARNING;
	if (argv[0] && argv[1])
	{
		ifindex = if_ifindex_make(argv[0], argv[1]);
		if(ifindex)
		{
			BOOL enable = FALSE;
			//mirror_dir_en odir= MIRROR_NONE;
			ret = nsm_mirror_source_get_api(ifindex, &enable, &dir);
			if(ret == OK && enable)
			{
				ret = nsm_mirror_source_set_api(ifindex, FALSE, dir);
			}
			else
			{
				vty_out(vty, "ERROR: this mirror source interface is not configure.%s", VTY_NEWLINE);
				ret = ERROR;
			}
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (monitor_session_source_filter,
		monitor_session_source_filter_cmd,
		"monitor session source filter (ingress|egress) (source|destination) mac "CMD_MAC_STR,
		"Monitor configure\n"
		"Session configure\n"
		"Session source\n"
		"Filter\n"
		"Ingress\n"
		"Egress\n"
		"Source\n"
		"Destination\n"
		"specify MAC address\n"
		CMD_MAC_STR_HELP)
{
	int ret = ERROR;
	mirror_dir_en	dir = MIRROR_NONE;
	BOOL	dst = FALSE;
	u_char	mac[NSM_MAC_MAX];
	if(!nsm_mirror_global_is_enable())
		ret = nsm_mirror_global_enable(TRUE);

	if(os_memcmp(argv[0], "ingress", 3) == 0)
		dir = MIRROR_INGRESS;
	else if(os_memcmp(argv[0], "ingress", 3) == 0)
		dir = MIRROR_EGRESS;

	if(os_memcmp(argv[1], "source", 3) == 0)
		dst = FALSE;
	else if(os_memcmp(argv[1], "destination", 3) == 0)
		dst = TRUE;

	os_memset(mac, 0, sizeof(mac));
	VTY_IMAC_GET(argv[0], mac);
	ret = nsm_mirror_source_mac_filter_set_api(TRUE, mac,  dst,  dir);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_monitor_session_source_filter,
		no_monitor_session_source_filter_cmd,
		"no monitor session source filter (ingress|egress) (source|destination) mac",
		NO_STR
		"Monitor configure\n"
		"Session configure\n"
		"Session source\n"
		"Filter\n"
		"Ingress\n"
		"Egress\n"
		"Source\n"
		"Destination\n"
		"specify MAC address\n")
{
	int ret = ERROR;
	mirror_dir_en	dir = MIRROR_NONE;
	BOOL	dst = FALSE;

	if(!nsm_mirror_global_is_enable())
		return CMD_WARNING;

	if(os_memcmp(argv[0], "ingress", 3) == 0)
		dir = MIRROR_INGRESS;
	else if(os_memcmp(argv[0], "ingress", 3) == 0)
		dir = MIRROR_EGRESS;

	if(os_memcmp(argv[1], "source", 3) == 0)
		dst = FALSE;
	else if(os_memcmp(argv[1], "destination", 3) == 0)
		dst = TRUE;

	ret = nsm_mirror_source_mac_filter_set_api(FALSE, NULL,  dst,  dir);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
/*

DEFUN (monitor_session_source_mac,
		monitor_session_source_mac_cmd,
		"monitor session source mac " CMD_MAC_STR,
		"Monitor configure\n"
		"Session configure\n"
		"Session source\n"
		"specify MAC address\n"
		CMD_MAC_STR_HELP)
{
	int ret = ERROR;
	u_char	mac[NSM_MAC_MAX];
	BOOL	bMac = FALSE;
	mirror_dir_en dir = MIRROR_BOTH;
	if(!nsm_mirror_global_is_enable())
		ret = nsm_mirror_global_enable(TRUE);

	if(nsm_mirror_is_source_api())
	{
		vty_out(vty, "ERROR: this mirror session base interface mode is already configure.%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	ret = nsm_mirror_mode_set_api(TRUE);
	VTY_IMAC_GET(argv[0], mac);

	if (ret == OK)
	{
		if(argc == 2)
		{
			dir = MIRROR_NONE;
			if(os_memcmp(argv[1], "ingress", 2) == 0)
				dir = MIRROR_INGRESS;
			else if(os_memcmp(argv[1], "egress", 2) == 0)
				dir = MIRROR_EGRESS;
			else if(os_memcmp(argv[1], "both", 2) == 0)
				dir = MIRROR_BOTH;
		}
		if(dir)
			ret = nsm_mirror_source_mac_set_api(TRUE, mac, dir);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(monitor_session_source_mac,
		monitor_session_source_mac_both_cmd,
		"monitor session source mac " CMD_MAC_STR " (ingress|egress|both)",
		"Monitor configure\n"
		"Session configure\n"
		"Session source\n"
		"specify MAC address\n"
		CMD_MAC_STR_HELP
		"Ingress stream\n"
		"Egress stream\n"
		"Ingress and Egress stream\n");

DEFUN (no_monitor_session_source_mac,
		no_monitor_session_source_mac_cmd,
		"no monitor session source mac ",
		NO_STR
		"Monitor configure\n"
		"Session configure\n"
		"Session source\n"
		"specify MAC address\n")
{
	int ret = ERROR;
	BOOL bMac = FALSE;
	if(!nsm_mirror_global_is_enable())
		ret = nsm_mirror_global_enable(TRUE);

	if(nsm_mirror_is_source_api())
	{
		vty_out(vty, "ERROR: this mirror session base interface mode is already configure.%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(nsm_mirror_mode_get_api(&bMac) == OK)
	{
		if(!bMac)
		{
			vty_out(vty, "ERROR: this mirror session base MAC mode is not configure.%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
	}
	ret = nsm_mirror_mode_set_api(FALSE);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
*/



DEFUN (monitor_session_destination,
		monitor_session_destination_cmd,
		"monitor session destination interface (ethernet|gigabitethernet) <unit/slot/port>",
		"Monitor configure\n"
		"Session configure\n"
		"Session destination\n"
		INTERFACE_STR
		"Ethernet interface\n"
		"GigabitEthernet interface\n"
		CMD_USP_STR_HELP)
{
	int ret = ERROR;
	ifindex_t	ifindex = 0;
	if(!nsm_mirror_global_is_enable())
		ret = nsm_mirror_global_enable(TRUE);

	if (argv[0] && argv[1])
	{
		ifindex = if_ifindex_make(argv[0], argv[1]);
		if(ifindex)
		{
			if(nsm_mirror_is_enable_api(ifindex) && !nsm_mirror_is_destination_api(ifindex))
			{
				vty_out(vty, "ERROR: this mirror destination interface is same to source interface.%s", VTY_NEWLINE);
				return CMD_WARNING;
			}
			if(!nsm_mirror_is_enable_api(ifindex))
				ret = nsm_mirror_destination_set_api(ifindex, TRUE);
			else
			{
				vty_out(vty, "ERROR: this mirror destination interface is already configure.%s", VTY_NEWLINE);
				return CMD_WARNING;
			}
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_monitor_session_destination,
		no_monitor_session_destination_cmd,
		"no monitor session destination interface (ethernet|gigabitethernet) <unit/slot/port>",
		NO_STR
		"Monitor configure\n"
		"Session configure\n"
		"Session destination\n"
		INTERFACE_STR
		"Ethernet interface\n"
		"GigabitEthernet interface\n"
		CMD_USP_STR_HELP)
{
	int ret = ERROR;
	ifindex_t	ifindex = 0;
	if(!nsm_mirror_global_is_enable())
		ret = nsm_mirror_global_enable(TRUE);

	if (argv[0] && argv[1])
	{
		ifindex = if_ifindex_make(argv[0], argv[1]);
		if(ifindex)
		{
			if(nsm_mirror_is_enable_api(ifindex) && !nsm_mirror_is_destination_api(ifindex))
			{
				vty_out(vty, "ERROR: this mirror destination interface is same to source interface.%s", VTY_NEWLINE);
				return CMD_WARNING;
			}
			if(nsm_mirror_is_enable_api(ifindex))
				ret = nsm_mirror_destination_set_api(ifindex, FALSE);
			else
			{
				vty_out(vty, "ERROR: this mirror destination interface is not configure.%s", VTY_NEWLINE);
				return CMD_WARNING;
			}
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}




static int bulid_mirror_one(nsm_mirror_t *node, struct vty *vty)
{
	if(node && node->enable)
	{
		if(node->mirror_dst)
		{
			vty_out(vty, "monitor session destination interface %s%s",
					if_ifname_make(node->ifindex), VTY_NEWLINE);
			//vty_out(vty, "monitor session destination interface %s%s", if_ifname_make(node->ifindex), VTY_NEWLINE);
		}
		else
		{
			char mac_str[64];
			char *both[] = {"none", "ingress", "egress", "both"};
			if(node->dir != MIRROR_BOTH)
				vty_out(vty, "monitor session source interface %s %s%s",
						if_ifname_make(node->ifindex), both[node->dir], VTY_NEWLINE);
			else
				vty_out(vty, "monitor session source interface %s%s",
						if_ifname_make(node->ifindex), VTY_NEWLINE);

			if(node->global)
			{
				if(node->global->in_enable)
				{
					os_memset(mac_str, 0, sizeof(mac_str));
					sprintf(mac_str, "%02x%02x-%02x%02x-%02x%02x", node->global->ingress_mac[0],
							node->global->ingress_mac[1], node->global->ingress_mac[2],
							node->global->ingress_mac[3], node->global->ingress_mac[4],
							node->global->ingress_mac[5]);
					vty_out(vty, "monitor session source filter ingress %s mac %s%s",
							node->global->ingress_dst ? "destination":"source", mac_str, VTY_NEWLINE);
				}
				if(node->global->out_enable)
				{
					os_memset(mac_str, 0, sizeof(mac_str));
					sprintf(mac_str, "%02x%02x-%02x%02x-%02x%02x", node->global->egress_mac[0],
							node->global->egress_mac[1], node->global->egress_mac[2],
							node->global->egress_mac[3], node->global->egress_mac[4],
							node->global->egress_mac[5]);
					vty_out(vty, "monitor session source filter egress %s mac %s%s",
							node->global->egress_dst ? "destination":"source", mac_str, VTY_NEWLINE);
				}
			}
		}
	}
	return OK;
}

static int bulid_mirror_config(struct vty *vty)
{
	//int ret = 0;
	//BOOL enable = FALSE;
	mirror_callback_api((mirror_cb)bulid_mirror_one, vty);
/*	ret = nsm_mirror_mode_get_api(&enable);
	if(enable)
	{
		u_char	mac[NSM_MAC_MAX];
		mirror_dir_en dir = MIRROR_BOTH;
		os_memset(mac, 0, sizeof(mac));
		if(nsm_mirror_source_mac_get_api(&enable, mac, &dir) == OK)
		{
			char *both[] = {"none", "ingress", "egress", "both"};
			char mac_str[32];
			os_memset(mac_str, 0, sizeof(mac_str));
			sprintf(mac_str, "%02x%02x-%02x%02x-%02x%02x",mac[0],mac[1],mac[2],
													 mac[3],mac[4],mac[5]);
			if(dir != MIRROR_BOTH)
				vty_out(vty, "monitor session source mac %s %s%s",
						mac_str, both[dir], VTY_NEWLINE);
			else
				vty_out(vty, "monitor session source mac %s%s",
						mac_str, VTY_NEWLINE);
		}
	}*/
	return 1;
}






void cmd_mirror_init(void)
{
	//reinstall_node(INTERFACE_NODE, nsm_interface_config_write);
	//install_default(INTERFACE_NODE);
	//install_default(INTERFACE_L3_NODE);
	struct nsm_client *nsm = nsm_client_new ();
	nsm->write_config_cb = bulid_mirror_config;
/*	nsm->interface_add_cb = nsm_vlan_add_interface;
	nsm->interface_delete_cb = nsm_vlan_del_interface;
	nsm->interface_write_config_cb = nsm_vlan_interface_config;*/
	nsm_client_install (nsm, NSM_MIRROR);

	install_element(CONFIG_NODE, &monitor_session_source_cmd);
	install_element(CONFIG_NODE, &monitor_session_source_both_cmd);
	install_element(CONFIG_NODE, &no_monitor_session_source_cmd);

	install_element(CONFIG_NODE, &monitor_session_source_filter_cmd);
	install_element(CONFIG_NODE, &no_monitor_session_source_filter_cmd);

/*
	install_element(CONFIG_NODE, &monitor_session_source_mac_cmd);
	install_element(CONFIG_NODE, &monitor_session_source_mac_both_cmd);
	install_element(CONFIG_NODE, &no_monitor_session_source_mac_cmd);
*/

	install_element(CONFIG_NODE, &monitor_session_destination_cmd);
	install_element(CONFIG_NODE, &no_monitor_session_destination_cmd);
}
