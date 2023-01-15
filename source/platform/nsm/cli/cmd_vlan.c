/*
 * nsm_vlan.c
 *
 *  Created on: Jan 11, 2018
 *      Author: zhurish
 */
#include "auto_include.h"
#include <zplos_include.h>
#include "route_types.h"
#include "nsm_event.h"
#include "zmemory.h"
#include "if.h"
#include "if_name.h"

#include "prefix.h"
#include "command.h"
#include "nsm_vlan.h"

#include "nsm_interface.h"



DEFUN(switchport_access_vlan,
	  switchport_access_vlan_cmd,
	  "switchport access " CMD_VLAN_STR,
	  "Switchport interface\n"
	  "access port\n" CMD_VLAN_STR_HELP)
{
	int ret = 0;
	if_mode_t mode = 0;
	vlan_t vlan = 0, oldvlan = 0;
	struct interface *ifp = vty->index;
	VTY_GET_INTEGER("vlan ID", vlan, argv[0]);
	if (!nsm_vlan_database_lookup_api(vlan))
	{
		vty_out(vty, "Error:Can not Find This vlan %d%s", vlan, VTY_NEWLINE);
		return CMD_WARNING;
	}
	if (ifp)
	{
		if (nsm_interface_mode_get_api(ifp, &mode) == OK && mode == IF_MODE_ACCESS_L2)
		{
			if (nsm_interface_access_vlan_get_api(ifp, &oldvlan) == OK)
			{
				if (oldvlan != vlan)
					ret = nsm_interface_access_vlan_set_api(ifp, vlan);
				return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
			}
			else
				vty_out(vty, "%% Can not get interface L2 Access Vlan.%s", VTY_NEWLINE);
		}
		else
			vty_out(vty, "%% Can not get interface Mode or is Not L2 Access Mode.%s", VTY_NEWLINE);
	}
	else if (vty->index_range)
	{
		zpl_uint32 i = 0;
		for (i = 0; i < vty->index_range; i++)
		{
			if (vty->vty_range_index[i])
			{
				if (nsm_interface_mode_get_api(ifp, &mode) == OK && mode == IF_MODE_ACCESS_L2)
				{
					if (nsm_interface_access_vlan_get_api(ifp, &oldvlan) == OK)
					{
						if (oldvlan != vlan)
							ret = nsm_interface_access_vlan_set_api(ifp, vlan);
						if (ret != OK)
							return CMD_WARNING;
					}
					else
						vty_out(vty, "%% Can not get interface L2 Access Vlan.%s", VTY_NEWLINE);
				}
				else
				{
					vty_out(vty, "%% Can not get interface Mode or is Not L2 Access Mode.%s", VTY_NEWLINE);
					return CMD_WARNING;
				}
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN(no_switchport_access_vlan,
	  no_switchport_access_vlan_cmd,
	  "no switchport access vlan",
	  NO_STR
	  "Switchport interface\n"
	  "access port\n"
	  "Vlan information\n")
{
	int ret = 0;
	if_mode_t mode = 0;
	vlan_t oldvlan = 0;
	struct interface *ifp = vty->index;
	if (ifp)
	{
		if (nsm_interface_mode_get_api(ifp, &mode) == OK && mode == IF_MODE_ACCESS_L2)
		{
			if (nsm_interface_access_vlan_get_api(ifp, &oldvlan) == OK)
			{
				if (oldvlan != 1)
					ret = nsm_interface_access_vlan_unset_api(ifp, oldvlan);
				return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
			}
			else
				vty_out(vty, "%% Can not get interface L2 Access Vlan.%s", VTY_NEWLINE);
		}
		else
			vty_out(vty, "%% Can not get interface Mode or is Not L2 Access Mode.%s", VTY_NEWLINE);
	}
	else if (vty->index_range)
	{
		zpl_uint32 i = 0;
		for (i = 0; i < vty->index_range; i++)
		{
			if (vty->vty_range_index[i])
			{
				if (nsm_interface_mode_get_api(ifp, &mode) == OK && mode == IF_MODE_ACCESS_L2)
				{
					if (nsm_interface_access_vlan_get_api(ifp, &oldvlan) == OK)
					{
						if (oldvlan != 1)
							ret = nsm_interface_access_vlan_unset_api(ifp, oldvlan);
						if (ret != OK)
							return CMD_WARNING;
					}
					else
						vty_out(vty, "%% Can not get interface L2 Access Vlan.%s", VTY_NEWLINE);
				}
				else
				{
					vty_out(vty, "%% Can not get interface Mode or is Not L2 Access Mode.%s", VTY_NEWLINE);
					return CMD_WARNING;
				}
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN(switchport_trunk_vlan,
	  switchport_trunk_vlan_cmd,
	  "switchport trunk native " CMD_VLAN_STR,
	  "Switchport interface\n"
	  "Trunk port\n"
	  "Native Config\n" CMD_VLAN_STR_HELP)
{
	int ret = 0;
	if_mode_t mode = 0;
	vlan_t vlan = 0;
	struct interface *ifp = vty->index;
	VTY_GET_INTEGER("vlan ID", vlan, argv[0]);
	if (!nsm_vlan_database_lookup_api(vlan))
	{
		vty_out(vty, "Error:Can not Find This vlan %d%s", vlan, VTY_NEWLINE);
		return CMD_WARNING;
	}
	if (ifp)
	{
		if (nsm_interface_mode_get_api(ifp, &mode) == OK && mode == IF_MODE_TRUNK_L2)
		{
			ret = nsm_interface_native_vlan_set_api(ifp, vlan);
			return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
		}
	}
	else if (vty->index_range)
	{
		zpl_uint32 i = 0;
		for (i = 0; i < vty->index_range; i++)
		{
			if (vty->vty_range_index[i])
			{
				if (nsm_interface_mode_get_api(ifp, &mode) == OK && mode == IF_MODE_TRUNK_L2)
				{
					ret = nsm_interface_native_vlan_set_api(ifp, vlan);
					if (ret != OK)
						return CMD_WARNING;
				}
				else
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN(no_switchport_trunk_vlan,
	  no_switchport_trunk_vlan_cmd,
	  "no switchport trunk native",
	  NO_STR
	  "Switchport interface\n"
	  "Trunk port\n"
	  "Native Config\n")
{
	int ret = 0;
	if_mode_t mode = 0;
	struct interface *ifp = vty->index;
	if (ifp)
	{
		if (nsm_interface_mode_get_api(ifp, &mode) == OK && mode == IF_MODE_TRUNK_L2)
		{
			ret = nsm_interface_native_vlan_set_api(ifp, 0);
			return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
		}
	}
	else if (vty->index_range)
	{
		zpl_uint32 i = 0;
		for (i = 0; i < vty->index_range; i++)
		{
			if (vty->vty_range_index[i])
			{
				if (nsm_interface_mode_get_api(ifp, &mode) == OK && mode == IF_MODE_TRUNK_L2)
				{
					ret = nsm_interface_native_vlan_set_api(ifp, 0);
					if (ret != OK)
						return CMD_WARNING;
				}
				else
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN(switchport_trunk_allow_vlan,
	  switchport_trunk_allow_vlan_cmd,
	  "switchport trunk allowed (add|remove) " CMD_VLAN_STR,
	  "Switchport interface\n"
	  "trunk port\n"
	  "allowed\n"
	  "add\n"
	  "remove\n" CMD_VLAN_STR_HELP)
{
	int ret = 0;
	vlan_t vlan = 0;
	struct interface *ifp = vty->index;
	if_mode_t mode = 0;
	if (ifp)
	{
		if (nsm_interface_mode_get_api(ifp, &mode) != OK || mode != IF_MODE_TRUNK_L2)
			return CMD_WARNING;
		if (argc == 1)
		{
			if (ifp)
			{
				if (os_memcmp(argv[0], "add", 3) == 0)
					ret = nsm_interface_trunk_add_allowed_vlan_api(ifp, vlan);
				else
					ret = nsm_interface_trunk_del_allowed_vlan_api(ifp, vlan);
				return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
			}
		}
		else if (strstr(argv[1], "-") || strstr(argv[1], ","))
		{
			vlan_t vlanlist[VLAN_TABLE_MAX];
			zpl_int32 num = 0;
			memset(vlanlist, 0, sizeof(vlanlist));
			num = nsm_vlan_database_list_split_api(argv[1], vlanlist);
			if (num == ERROR)
			{
				vty_out(vty, "Error:Can not split vlan list table.%s", VTY_NEWLINE);
				return CMD_WARNING;
			}

			if (os_memcmp(argv[0], "add", 3) == 0)
			{
				ret = nsm_interface_trunk_allowed_vlan_list_api(1, ifp, argv[1]);
			}
			else
			{
				if (!nsm_interface_trunk_allowed_vlan_list_lookup_api(ifp, vlanlist, num))
				{
					vty_out(vty, "Error:some vlan is not exist.%s", VTY_NEWLINE);
					return CMD_WARNING;
				}
				ret = nsm_interface_trunk_allowed_vlan_list_api(0, ifp, argv[1]);
			}
			return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
		}
		else
		{
			VTY_GET_INTEGER("vlan ID", vlan, argv[1]);
			if (!nsm_vlan_database_lookup_api(vlan))
			{
				vty_out(vty, "Error:Can not Find This vlan %d%s", vlan, VTY_NEWLINE);
				return CMD_WARNING;
			}

			if (os_memcmp(argv[0], "add", 3) == 0)
			{
				if (nsm_interface_trunk_add_allowed_vlan_lookup_api(ifp, vlan))
				{
					vty_out(vty, "Error:some vlan is already exist.%s", VTY_NEWLINE);
					return CMD_WARNING;
				}
				ret = nsm_interface_trunk_add_allowed_vlan_api(ifp, vlan);
			}
			else
			{
				if (!nsm_interface_trunk_add_allowed_vlan_lookup_api(ifp, vlan))
				{
					vty_out(vty, "Error:some vlan is not exist.%s", VTY_NEWLINE);
					return CMD_WARNING;
				}
				ret = nsm_interface_trunk_del_allowed_vlan_api(ifp, vlan);
			}
			return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
		}
	}
	else if (vty->index_range)
	{
		zpl_uint32 i = 0;
		for (i = 0; i < vty->index_range; i++)
		{
			if (vty->vty_range_index[i])
			{
				if (nsm_interface_mode_get_api(vty->vty_range_index[i], &mode) != OK || mode != IF_MODE_TRUNK_L2)
					return CMD_WARNING;
				if (argc == 1) // all
				{
					if (vty->vty_range_index[i])
					{
						if (os_memcmp(argv[0], "add", 3) == 0)
							ret = nsm_interface_trunk_add_allowed_vlan_api(vty->vty_range_index[i], 0);
						else
							ret = nsm_interface_trunk_del_allowed_vlan_api(vty->vty_range_index[i], 0);
						if (ret != OK)
							return CMD_WARNING;
					}
				}
				else if (strstr(argv[1], "-") || strstr(argv[1], ","))
				{
					vlan_t vlanlist[VLAN_TABLE_MAX];
					zpl_int32 num = 0;
					memset(vlanlist, 0, sizeof(vlanlist));
					num = nsm_vlan_database_list_split_api(argv[1], vlanlist);
					if (num == ERROR)
					{
						vty_out(vty, "Error:Can not split vlan list table.%s", VTY_NEWLINE);
						return CMD_WARNING;
					}
					if (vty->vty_range_index[i])
					{
						if (os_memcmp(argv[0], "add", 3) == 0)
						{
							ret = nsm_interface_trunk_allowed_vlan_list_api(1, vty->vty_range_index[i], argv[1]);
						}
						else
						{
							if (!nsm_interface_trunk_allowed_vlan_list_lookup_api(vty->vty_range_index[i], vlanlist, num))
							{
								vty_out(vty, "Error:some vlan is not exist.%s", VTY_NEWLINE);
								return CMD_WARNING;
							}
							ret = nsm_interface_trunk_allowed_vlan_list_api(0, vty->vty_range_index[i], argv[1]);
						}
						if (ret != OK)
							return CMD_WARNING;
					}
				}
				else
				{
					VTY_GET_INTEGER("vlan ID", vlan, argv[1]);
					if (!nsm_vlan_database_lookup_api(vlan))
					{
						vty_out(vty, "Error:Can not Find This vlan %d%s", vlan, VTY_NEWLINE);
						return CMD_WARNING;
					}
					if (vty->vty_range_index[i])
					{
						if (os_memcmp(argv[0], "add", 3) == 0)
							ret = nsm_interface_trunk_add_allowed_vlan_api(vty->vty_range_index[i], vlan);
						else
							ret = nsm_interface_trunk_del_allowed_vlan_api(vty->vty_range_index[i], vlan);
						if (ret != OK)
							return CMD_WARNING;
					}
				}
			}
		}
		return CMD_SUCCESS;
	}

	return CMD_WARNING;
}

ALIAS(switchport_trunk_allow_vlan,
	  switchport_trunk_allow_vlan_all_cmd,
	  "switchport trunk allowed (add|remove) vlan all",
	  "Switchport interface\n"
	  "trunk port\n"
	  "allowed\n"
	  "add\n"
	  "remove\n"
	  "vlan information\n"
	  "all vlan");

ALIAS(switchport_trunk_allow_vlan,
	  switchport_trunk_allow_vlan_list_cmd,
	  "switchport trunk allowed (add|remove) vlan VLANLIST",
	  "Switchport interface\n"
	  "trunk port\n"
	  "allowed\n"
	  "add\n"
	  "remove\n"
	  "Vlan information\n"
	  "VLANLIST string(eg:2,4,5,7,9-100)\n");



DEFUN(switchport_dot1q_tunnel_vlan,
	  switchport_dot1q_tunnel_cmd,
	  "switchport dot1q-tunnel vlan " CMD_VLAN_STR,
	  "Switchport interface\n"
	  "Dot1q-tunnel\n"
	  "Native Config\n" CMD_VLAN_STR_HELP)
{
	int ret = 0;
	if_mode_t mode = 0;
	vlan_t vlan = 0;
	struct interface *ifp = vty->index;
	if(!nsm_vlan_qinq_is_enable())
	{
		vty_out(vty, "%%Qinq is not enable%s",VTY_NEWLINE);
		return CMD_WARNING;
	}
	VTY_GET_INTEGER("vlan ID", vlan, argv[0]);
	if (!nsm_vlan_database_lookup_api(vlan))
	{
		vty_out(vty, "Error:Can not Find This vlan %d%s", vlan, VTY_NEWLINE);
		return CMD_WARNING;
	}
	if (ifp)
	{
		if (nsm_interface_mode_get_api(ifp, &mode) == OK && mode == IF_MODE_DOT1Q_TUNNEL)
		{
			ret = nsm_interface_qinq_vlan_set_api(ifp, vlan);
			return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
		}
	}
	else if (vty->index_range)
	{
		zpl_uint32 i = 0;
		for (i = 0; i < vty->index_range; i++)
		{
			if (vty->vty_range_index[i])
			{
				if (nsm_interface_mode_get_api(ifp, &mode) == OK && mode == IF_MODE_DOT1Q_TUNNEL)
				{
					ret = nsm_interface_qinq_vlan_set_api(ifp, vlan);
					if (ret != OK)
						return CMD_WARNING;
				}
				else
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN(no_switchport_dot1q_tunnel,
	  no_switchport_dot1q_tunnel_cmd,
	  "no switchport dot1q-tunnel vlan",
	  NO_STR
	  "Switchport interface\n"
	  "Dot1q-tunnel port\n"
	  "Native Config\n")
{
	int ret = 0;
	if_mode_t mode = 0;
	struct interface *ifp = vty->index;
	if(!nsm_vlan_qinq_is_enable())
	{
		vty_out(vty, "%%Qinq is not enable%s",VTY_NEWLINE);
		return CMD_WARNING;
	}
	if (ifp)
	{
		if (nsm_interface_mode_get_api(ifp, &mode) == OK && mode == IF_MODE_DOT1Q_TUNNEL)
		{
			ret = nsm_interface_qinq_vlan_set_api(ifp, 1);
			return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
		}
	}
	else if (vty->index_range)
	{
		zpl_uint32 i = 0;
		for (i = 0; i < vty->index_range; i++)
		{
			if (vty->vty_range_index[i])
			{
				if (nsm_interface_mode_get_api(ifp, &mode) == OK && mode == IF_MODE_DOT1Q_TUNNEL)
				{
					ret = nsm_interface_qinq_vlan_set_api(ifp, 1);
					if (ret != OK)
						return CMD_WARNING;
				}
				else
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

static void cmd_vlan_interface_attr_init(int node)
{
	install_element(node, CMD_CONFIG_LEVEL, &switchport_access_vlan_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_switchport_access_vlan_cmd);

	// install_element(node,CMD_CONFIG_LEVEL, &switchport_access_allow_vlan_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &switchport_trunk_vlan_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_switchport_trunk_vlan_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &switchport_trunk_allow_vlan_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &switchport_trunk_allow_vlan_all_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &switchport_trunk_allow_vlan_list_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &switchport_access_vlan_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_switchport_access_vlan_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &switchport_trunk_vlan_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_switchport_trunk_vlan_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &switchport_trunk_allow_vlan_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &switchport_trunk_allow_vlan_all_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &switchport_trunk_allow_vlan_list_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &switchport_dot1q_tunnel_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_switchport_dot1q_tunnel_cmd);
}



void cmd_vlan_init(void)
{
	cmd_vlan_database_init();
	cmd_vlan_interface_attr_init(INTERFACE_NODE);
	cmd_vlan_interface_attr_init(INTERFACE_RANGE_NODE);
	cmd_vlan_interface_attr_init(LAG_INTERFACE_NODE);
	// cmd_vlan_attr_init(EPON_INTERFACE_NODE);
	// cmd_vlan_attr_init(EPON_INTERFACE_RANGE_NODE);
}
