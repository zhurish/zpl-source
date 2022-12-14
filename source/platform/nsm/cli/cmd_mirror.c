/*
 * cmd_mirror.c
 *
 *  Created on: May 11, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include <zplos_include.h>
#include "if.h"
#include "command.h"
#include "prefix.h"
#include "nsm_mirror.h"
#include "nsm_vlan.h"
#include "vty.h"


/*
 * monitor session 1 source interface gigabitEthernet 1/0/1 （in|out|both）
 * monitor session 1 source interface gigabitEthernet 1/0/1 （in|out|both）(src-mac|dst-mac) XXXX.XXXX.XXXX
 * monitor session 1 source interface gigabitEthernet 1/0/1 （in|out) (src-mac|dst-mac) XXXX.XXXX.XXXX
 * monitor session 1 destination interface gigabitEthernet 1/0/6
 */

/*
 * global
 */
DEFUN (monitor_session_source,
		monitor_session_source_cmd,
		"monitor session <1-2> source interface (ethernet|gigabitethernet) <unit/slot/port>",
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
	mirror_mode_t mode = MIRROR_SOURCE_PORT;
	mirror_filter_t filter = MIRROR_FILTER_ALL;
	zpl_uint32 index = 0, subindex = 0;
	if (argv[0] && argv[1] && argv[2])
	{
		ifindex = if_ifindex_make(argv[1], argv[2]);
		if(ifindex)
		{
			if(nsm_mirror_is_destination_api( ifindex, &index))
			{
				vty_out(vty, "ERROR: this interface is enable by destination interface on session %d.%s", index, VTY_NEWLINE);
				return CMD_WARNING;
			}
			if(nsm_mirror_is_source_api(ifindex, &index, &mode, &dir, &filter, &subindex))
			{
				if(index != atoi(argv[0]))
				{
					vty_out(vty, "ERROR: this interface is already enable souce interface on session %d.%s", index, VTY_NEWLINE);
					return CMD_WARNING;
				}
			}
			if(argc == 4)
			{
				dir = MIRROR_NONE;
				if(os_memcmp(argv[3], "ingress", 2) == 0)
					dir = MIRROR_INGRESS;
				else if(os_memcmp(argv[3], "egress", 2) == 0)
					dir = MIRROR_EGRESS;
				else if(os_memcmp(argv[3], "both", 2) == 0)
					dir = MIRROR_BOTH;
			}
			index = atoi(argv[0]);
			if(dir)
				ret = nsm_mirror_source_set_api(index, ifindex, zpl_true, dir);
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


ALIAS(monitor_session_source,
		monitor_session_source_both_cmd,
		"monitor session <1-2> source interface (ethernet|gigabitethernet) <unit/slot/port> (ingress|egress|both)",
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
		"no monitor session <1-2> source interface (ethernet|gigabitethernet) <unit/slot/port>",
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
	mirror_mode_t mode = MIRROR_SOURCE_PORT;
	mirror_filter_t filter = MIRROR_FILTER_ALL;
	zpl_uint32 index = 0, subindex = 0;

	if (argv[0] && argv[1])
	{
		ifindex = if_ifindex_make(argv[0], argv[1]);
		if(ifindex)
		{
			if(!nsm_mirror_is_source_api(ifindex, &index, &mode, &dir, &filter, &subindex))
			{
				vty_out(vty, "ERROR: this interface is not enable souce interface.%s", VTY_NEWLINE);
				return CMD_WARNING;
			}
			if(index != atoi(argv[0]))
			{
				vty_out(vty, "ERROR: this interface is already enable souce interface on session %d.%s", index, VTY_NEWLINE);
				return CMD_WARNING;
			}	
			index = atoi(argv[0]);
			ret = nsm_mirror_source_set_api(index, ifindex, zpl_false, dir);
			if(ret != OK)
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
		"monitor session <1-2> source interface (ethernet|gigabitethernet) <unit/slot/port> (ingress|egress) (source|destination) "CMD_MAC_STR,
		"Monitor configure\n"
		"Session configure\n"
		"Session source\n"
		INTERFACE_STR
		"Ethernet interface\n"
		"GigabitEthernet interface\n"
		CMD_USP_STR_HELP
		"Ingress\n"
		"Egress\n"
		"Source\n"
		"Destination\n"
		CMD_MAC_STR_HELP)
{
	int ret = ERROR;
	ifindex_t	ifindex = 0;
	mirror_dir_en dir = MIRROR_BOTH;
	mirror_mode_t mode = MIRROR_SOURCE_PORT;
	mirror_filter_t filter = MIRROR_FILTER_ALL;
	zpl_uint32 index = 0, subindex = 0;
	zpl_uchar		mac[NSM_MAC_MAX];
	if (argv[0] && argv[1] && argv[2])
	{
		ifindex = if_ifindex_make(argv[1], argv[2]);
		if(ifindex)
		{
			if(nsm_mirror_is_destination_api( ifindex, &index))
			{
				vty_out(vty, "ERROR: this interface is enable by destination interface on session %d.%s", index, VTY_NEWLINE);
				return CMD_WARNING;
			}
			if(nsm_mirror_is_source_api(ifindex, &index, &mode, &dir, &filter, &subindex))
			{
				if(index != atoi(argv[0]))
				{
					vty_out(vty, "ERROR: this interface is already enable souce interface on session %d.%s", index, VTY_NEWLINE);
					return CMD_WARNING;
				}
			}
			if(argc >= 4)
			{
				dir = MIRROR_NONE;
				if(os_memcmp(argv[3], "ingress", 2) == 0)
					dir = MIRROR_INGRESS;
				else if(os_memcmp(argv[3], "egress", 2) == 0)
					dir = MIRROR_EGRESS;
				else if(os_memcmp(argv[3], "both", 2) == 0)
					dir = MIRROR_BOTH;
			}
			if(argc >= 5)
			{
				if(os_memcmp(argv[4], "source", 2) == 0)
					filter = MIRROR_FILTER_SA;
				else if(os_memcmp(argv[4], "destination", 2) == 0)
					filter = MIRROR_FILTER_DA;
			}
			vty_mac_get (argv[5], mac);
			if(NSM_MAC_IS_BROADCAST(mac))
			{
				vty_out(vty, "Error: This is Broadcast mac address.%s",VTY_NEWLINE);
				return CMD_WARNING;
			}
			if(NSM_MAC_IS_MULTICAST(mac))
			{
				vty_out(vty, "Error: This is Multicast mac address.%s",VTY_NEWLINE);
				return CMD_WARNING;
			}
			index = atoi(argv[0]);
			if(dir)
				ret = nsm_mirror_source_mac_filter_set_api(index, ifindex, zpl_true, dir, filter, mac);
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

/***************************************/
DEFUN (monitor_session_destination,
		monitor_session_destination_cmd,
		"monitor session <1-2> destination interface (ethernet|gigabitethernet) <unit/slot/port>",
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
	zpl_uint32	index = 0;
	if (argv[0] && argv[1] && argv[2])
	{
		ifindex = if_ifindex_make(argv[1], argv[2]);
		if(ifindex)
		{
			if(nsm_mirror_is_destination_api( ifindex, &index))
			{
				vty_out(vty, "ERROR: this mirror destination interface is already enable.%s", VTY_NEWLINE);
				return CMD_WARNING;
			}
			if(index == atoi(argv[0]))
			{
				return CMD_SUCCESS;
			}
			else if(index > 0)
			{
				vty_out(vty, "ERROR: this mirror destination interface is enable in anther session.%s", VTY_NEWLINE);
				return CMD_WARNING;
			}
			index = atoi(argv[0]);
			ret = nsm_mirror_destination_set_api(index, ifindex, zpl_true);
			if(ret != OK)
			{
				vty_out(vty, "ERROR: this mirror destination interface is not configure.%s", VTY_NEWLINE);
				return CMD_WARNING;
			}
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_monitor_session_destination,
		no_monitor_session_destination_cmd,
		"no monitor session <1-2> destination interface (ethernet|gigabitethernet) <unit/slot/port>",
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
	zpl_uint32	index = 0;
	if (argv[0] && argv[1] && argv[2])
	{
		ifindex = if_ifindex_make(argv[1], argv[2]);
		if(ifindex)
		{
			if(!nsm_mirror_is_destination_api( ifindex, &index))
			{
				vty_out(vty, "ERROR: this mirror destination interface is not enable.%s", VTY_NEWLINE);
				return CMD_WARNING;
			}
			if(index && (index != atoi(argv[0])))
			{
				vty_out(vty, "ERROR: this mirror destination interface is not in this session.%s", VTY_NEWLINE);
				return CMD_WARNING;
			}
			index = atoi(argv[0]);
			ret = nsm_mirror_destination_set_api(index, ifindex, zpl_false);
			if(ret != OK)
			{
				vty_out(vty, "ERROR: this mirror destination interface is not configure.%s", VTY_NEWLINE);
				return CMD_WARNING;
			}
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN(show_monitor_session,
      show_monitor_session_cmd,
    "show monitor session",
    SHOW_STR
	"Monitor configure\n"
	"Session configure\n")
{
	bulid_mirror_show(vty);
    return CMD_SUCCESS;
}


void cmd_mirror_init(void)
{
	//reinstall_node(INTERFACE_NODE, nsm_interface_config_write);
	//install_default(INTERFACE_NODE);
	//install_default(INTERFACE_L3_NODE);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &monitor_session_source_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &monitor_session_source_both_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_monitor_session_source_cmd);

	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &monitor_session_source_filter_cmd);


	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &monitor_session_destination_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_monitor_session_destination_cmd);

	install_element(VIEW_NODE, CMD_VIEW_LEVEL, &show_monitor_session_cmd);
	install_element(CONFIG_NODE, CMD_VIEW_LEVEL, &show_monitor_session_cmd);
}
