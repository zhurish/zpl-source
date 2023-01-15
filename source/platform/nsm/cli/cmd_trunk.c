/*
 * cmd_trunk.c
 *
 *  Created on: May 8, 2018
 *      Author: zhurish
 */


#include "auto_include.h"
#include <zplos_include.h>
#include "route_types.h"
#include "nsm_event.h"
#include "zmemory.h"
#include "if.h"

#include "prefix.h"
#include "command.h"
#include "nsm_trunk.h"

struct trunk_user
{
	struct vty *vty;
	ifindex_t	ifindex;
};

DEFUN (channel_group,
		channel_group_cmd,
		"channel-group <1-2> mode (active|passive)",
		"Channel Group\n"
		"Channel number\n"
		"Channel Group mode\n"
		"Active mode\n"
		"Passive mode\n")
{
	int ret = ERROR;
	zpl_uint32 trunkid;
	struct interface *ifp = vty->index;
	trunk_type_t type = TRUNK_DYNAMIC;
	trunk_mode_t mode = TRUNK_ACTIVE;
	if (argc < 1)
		return CMD_WARNING;

	if(!nsm_trunk_is_enable())
		nsm_trunk_enable();
	trunkid = atoi(argv[0]);
	if(os_memcmp(argv[1], "active", 3) == 0)
		mode = TRUNK_ACTIVE;
	else if(os_memcmp(argv[1], "passive", 3) == 0)
		mode = TRUNK_PASSIVE;

	if(!l2trunk_lookup_api(trunkid))
	{
		vty_out(vty, "interface port-channel%d is not exist.%s",trunkid, VTY_NEWLINE);
		return CMD_WARNING;
	}
	else
		ret = OK;
	if(ret == OK)
	{
		if(!l2trunk_lookup_interface_api(ifp->ifindex))
		{
			ret = nsm_trunk_add_interface_api(trunkid, type, mode, ifp);
			return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
		}
		else
		{
			zpl_uint32 trunkIdOld = 0;
			if(nsm_trunk_get_ID_interface_api(ifp->ifindex, &trunkIdOld) == OK)
			{
				if(trunkIdOld != trunkid)
				{
					return CMD_WARNING;
				}
				else
					ret = OK;
			}
			else
				return CMD_WARNING;
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (channel_group_static,
		channel_group_static_cmd,
		"static-channel-group <1-2>",
		"Static Channel Group\n"
		"Channel number\n")
{
	int ret = ERROR;
	zpl_uint32 trunkid;
	struct interface *ifp = vty->index;
	trunk_type_t type = TRUNK_STATIC;
	trunk_mode_t mode = TRUNK_ACTIVE;
	if (argc < 1)
		return CMD_WARNING;

	if(!nsm_trunk_is_enable())
		nsm_trunk_enable();
	trunkid = atoi(argv[0]);

	if(l2trunk_lookup_api(trunkid))
	{
		if(!l2trunk_lookup_interface_api(ifp->ifindex))
		{
			ret = nsm_trunk_add_interface_api(trunkid, type, mode, ifp);
			return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
		}
		else
		{
			zpl_uint32 trunkIdOld = 0;
			if(nsm_trunk_get_ID_interface_api(ifp->ifindex, &trunkIdOld) == OK)
			{
				if(trunkIdOld != trunkid)
				{
					return CMD_WARNING;
				}
				else
					ret = OK;
			}
			else
				return CMD_WARNING;
		}
	}
	else
	{
		vty_out(vty, "interface port-channel%d is not exist.%s",trunkid, VTY_NEWLINE);
		return CMD_WARNING;
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_channel_group_static,
		no_channel_group_static_cmd,
		"no static-channel-group",
		NO_STR
		"Static Channel Group\n")
{
	int ret = ERROR;
	zpl_uint32 trunkid;
	struct interface *ifp = vty->index;
	if(nsm_trunk_get_ID_interface_api(ifp->ifindex, &trunkid) != OK)
	{
		vty_out(vty, "this interface is not member of trunk.%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	ret = nsm_trunk_del_interface_api(trunkid, ifp);
	if(ret == OK)
	{
/*		if(l2trunk_lookup_interface_count_api(trunkid) <= 0)
		{
			ret = nsm_trunk_destroy_api(trunkid);
		}*/
		
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS (no_channel_group_static,
		no_channel_group_cmd,
		"no channel-group",
		NO_STR
		"Channel Group\n");



DEFUN (lacp_port_priority,
		lacp_port_priority_cmd,
		"lacp port-priority <1-65535>",
		"Lacp"
		"port-priority\n"
		"priority value\n")
{
	int ret = ERROR;
	zpl_uint32  pri;
	struct interface *ifp = vty->index;
	pri = atoi(argv[1]);
	if(!l2trunk_lookup_interface_api(ifp->ifindex))
	{
		vty_out(vty, "this interface is not member of trunk.%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	/*
	if(nsm_trunk_get_ID_interface_api(ifp->ifindex, &trunkid) != OK)
	{
		vty_out(vty, "this interface is not member of trunk.%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	*/
	ret = nsm_trunk_lacp_port_priority_api(ifp->ifindex, pri);

	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_lacp_port_priority,
		no_lacp_port_priority_cmd,
		"no lacp port-priority",
		NO_STR
		"Lacp"
		"port-priority\n"
		"system-priority\n")
{
	int ret = ERROR;
	zpl_uint32  pri;
	//trunkid = vty->index_value;
	pri = 0;
	struct interface *ifp = vty->index;
	if(!l2trunk_lookup_interface_api(ifp->ifindex))
	{
		vty_out(vty, "this interface is not member of trunk.%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	ret = nsm_trunk_lacp_port_priority_api(ifp->ifindex, pri);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (lacp_system_priority,
		lacp_system_priority_cmd,
		"lacp system-priority <1-65535>",
		"Lacp"
		"system-priority\n"
		"priority value\n")
{
	int ret = ERROR;
	zpl_uint32  pri;
	zpl_uint32 trunkid = vty->index_value;
	//struct interface *ifp = vty->index;
	pri = atoi(argv[1]);

	ret = nsm_trunk_lacp_system_priority_api(trunkid, pri);

	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_lacp_system_priority,
		no_lacp_system_priority_cmd,
		"no lacp system-priority",
		NO_STR
		"Lacp"
		"system-priority\n")
{
	int ret = ERROR;
	zpl_uint32  pri;
	pri = 0;
	zpl_uint32 trunkid = vty->index_value;

	ret = nsm_trunk_lacp_system_priority_api(trunkid, pri);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (lacp_timeout,
		lacp_timeout_cmd,
		"lacp timeout <1-65535>",
		"Lacp"
		"timeout\n"
		"timeout value\n")
{
	int ret = ERROR;
	zpl_uint32 value;
	struct interface *ifp = vty->index;
	if(!l2trunk_lookup_interface_api(ifp->ifindex))
	{
		vty_out(vty, "this interface is not member of trunk.%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	value = atoi(argv[1]);
	ret = nsm_trunk_lacp_timeout_api(ifp->ifindex, value);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_lacp_timeout,
		no_lacp_timeout_cmd,
		"no lacp timeout",
		NO_STR
		"Lacp"
		"timeout\n")
{
	int ret = ERROR;
	zpl_uint32 value;
	struct interface *ifp = vty->index;
	if(!l2trunk_lookup_interface_api(ifp->ifindex))
	{
		vty_out(vty, "this interface is not member of trunk.%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	value = 1;
	ret = nsm_trunk_lacp_timeout_api(ifp->ifindex, value);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}



DEFUN (port_channel_load_balance,
		port_channel_load_balance_cmd,
		"port-channel load-balance (dst-mac|src-mac|dst-src-mac)",
		"port-channel\n"
		"load-balance\n"
		"dst mac load-balance mode\n"
		"src mac load-balance mode\n"
		"dst src mac load-balance mode\n")
{
	int ret = ERROR;
	zpl_uint32 trunkid = 0;
	load_balance_t value = TRUNK_LOAD_BALANCE_NONE;
	trunkid = vty->index_value;
	if(!nsm_trunk_is_enable())
		nsm_trunk_enable();
	if(!l2trunk_lookup_api(trunkid))
	{
		vty_out(vty, "interface port-channel%d is not exist.%s",trunkid, VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(os_memcmp(argv[0], "dst-mac", 5) == 0)
	{
		value = TRUNK_LOAD_BALANCE_DSTMAC;
	}
	else if(os_memcmp(argv[0], "src-mac", 5) == 0)
	{
		value = TRUNK_LOAD_BALANCE_SRCMAC;
	}
	else if(os_memcmp(argv[0], "dst-src-mac", 5) == 0)
	{
		value = TRUNK_LOAD_BALANCE_DSTSRCMAC;
	}
	ret = nsm_trunk_load_balance_api(trunkid, value);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_port_channel_load_balance,
		no_port_channel_load_balance_cmd,
		"no port-channel load-balance",
		NO_STR
		"port-channel\n"
		"load-balance\n")
{
	int ret = ERROR;
	zpl_uint32 trunkid, value;
	trunkid = vty->index_value;
	if(!nsm_trunk_is_enable())
		nsm_trunk_enable();

	value = TRUNK_LOAD_BALANCE_NONE;
	ret = nsm_trunk_load_balance_api(trunkid, value);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}



static int _trunk_group_show_one(l2trunk_group_t *group, struct vty *vty)
{
	char *load[] = {"NONE", "dst-mac", "src-mac", "dst-src-mac", "dst-ip"};
	if(group)
	{
		vty_out(vty, " trunkId              : %d%s", group->trunkId, VTY_NEWLINE);
		vty_out(vty, " lacp_system_priority : %d%s", group->lacp_system_priority, VTY_NEWLINE);
		vty_out(vty, " load_balance         : %s%s", load[group->load_balance], VTY_NEWLINE);
		vty_out(vty, " type                 : %d%s", group->type, VTY_NEWLINE);
	}
	return 0;
}


DEFUN (show_port_channel_all,
		show_port_channel_all_cmd,
		"show port-channel all",
		SHOW_STR
		"port-channel\n"
		"all\n")
{
	nsm_trunk_group_callback_api((l2trunk_group_cb)_trunk_group_show_one, vty);
	return CMD_SUCCESS;
}
/*
int build_trunk_interface_config(struct vty *vty, struct interface *ifp)
{
	struct trunk_user user;
	user.vty = vty;
	user.ifindex = ifp->ifindex;
	if(nsm_trunk_is_enable())
		nsm_trunk_callback_api((l2trunk_cb)_trunk_interface_show_one, &user);
	return 1;
}

*/



static int cmd_trunk_interface_init(int node)
{
	install_element(node, CMD_CONFIG_LEVEL, &channel_group_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &channel_group_static_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &no_channel_group_static_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_channel_group_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &lacp_port_priority_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_lacp_port_priority_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &lacp_timeout_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_lacp_timeout_cmd);
	return OK;
}

static int cmd_trunk_base_init(int node)
{
	install_element(node, CMD_CONFIG_LEVEL, &port_channel_load_balance_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_port_channel_load_balance_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &lacp_system_priority_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_lacp_system_priority_cmd);

	return OK;
}

void cmd_trunk_init(void)
{
	cmd_trunk_base_init(LAG_INTERFACE_NODE);
	cmd_trunk_base_init(LAG_INTERFACE_L3_NODE);

	cmd_trunk_interface_init(INTERFACE_NODE);
	cmd_trunk_interface_init(INTERFACE_L3_NODE);
/*
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &port_channel_load_balance_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_port_channel_load_balance_cmd);

	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &lacp_system_priority_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_lacp_system_priority_cmd);

	install_element(INTERFACE_NODE, CMD_CONFIG_LEVEL, &channel_group_cmd);
	install_element(INTERFACE_NODE, CMD_CONFIG_LEVEL, &channel_group_static_cmd);

	install_element(INTERFACE_NODE, CMD_CONFIG_LEVEL, &no_channel_group_static_cmd);
	install_element(INTERFACE_NODE, CMD_CONFIG_LEVEL, &no_channel_group_cmd);

	install_element(INTERFACE_NODE, CMD_CONFIG_LEVEL, &lacp_port_priority_cmd);
	install_element(INTERFACE_NODE, CMD_CONFIG_LEVEL, &no_lacp_port_priority_cmd);
*/

/*	install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_ip_arp_cmd);
	install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_ip_arp_detail_cmd);
	install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_ip_arp_interface_cmd);
	install_element(CONFIG_NODE, CMD_VIEW_LEVEL, &show_ip_arp_cmd);
	install_element(CONFIG_NODE, CMD_VIEW_LEVEL, &show_ip_arp_detail_cmd);
	install_element(CONFIG_NODE, CMD_VIEW_LEVEL, &show_ip_arp_interface_cmd);*/
	install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_port_channel_all_cmd);
	install_element(CONFIG_NODE, CMD_VIEW_LEVEL, &show_port_channel_all_cmd);
	
}


