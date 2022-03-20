/*
 * nsm_vlan.c
 *
 *  Created on: Jan 11, 2018
 *      Author: zhurish
 */

#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"
#include "vty_include.h"

struct vlan_user
{
	struct vty *vty;
	vlan_t vlan;
	vlan_mode_t mode;
	ifindex_t ifindex;
	int itag;
	int iuntag;
	int total;
	zpl_bool all;

	int frist;
	char cli_str[256];
};

// static int show_nsm_vlan_database(struct vty *vty, const char *type);
static int show_nsm_vlan_database_one(l2vlan_t *node, struct vlan_user *pVoid);

DEFUN(vlan_database,
	  vlan_database_cmd,
	  CMD_VLAN_DATABASE_STR,
	  CMD_VLAN_STR_DATABASE_HELP)
{
	if (nsm_vlan_is_enable())
		vty->node = VLAN_DATABASE_NODE;
	else
	{
		if (nsm_vlan_enable() == OK)
			vty->node = VLAN_DATABASE_NODE;
		else
		{
			vty_out(vty, "Error:Can not Enable vlan %s", VTY_NEWLINE);
			return CMD_WARNING;
		}
	}
	return CMD_SUCCESS;
}

DEFUN(vlan_add,
	  vlan_add_cmd,
	  CMD_VLAN_STR,
	  CMD_VLAN_STR_HELP)
{
	int ret = 0;
	vlan_t vlan = 0;
	if (strstr(argv[0], "-") || strstr(argv[0], ","))
	{
		vlan_t vlanlist[VLAN_TABLE_MAX];
		zpl_int32 num = 0;
		memset(vlanlist, 0, sizeof(vlanlist));
		num = nsm_vlan_list_split_api(argv[0], &vlanlist);
		if (num == ERROR)
		{
			vty_out(vty, "Error:Can not split vlan list table.%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		if (nsm_vlan_list_lookup_api(vlanlist, num))
		{
			vty_out(vty, "Error:some vlan is already exist.%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
#if 1
		ret = nsm_vlan_list_create_api(argv[0]);
#else

#endif
	}
	else
	{
		VTY_GET_INTEGER("vlan ID", vlan, argv[0]);
		if (!nsm_vlan_lookup_api(vlan))
			ret = nsm_vlan_create_api(vlan, (argc == 2) ? argv[1] : NULL);
		else
			ret = OK;
	}
	if (ret != OK)
		vty_out(vty, "Error:Can not Create vlan %s", VTY_NEWLINE);
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

ALIAS(vlan_add,
	  vlan_add_name_cmd,
	  CMD_VLAN_STR " name NAME",
	  CMD_VLAN_STR_HELP
	  "Name information\n"
	  "Name string\n")

ALIAS(vlan_add,
	  vlan_add_list_cmd,
	  "vlan VLANLIST",
	  "Vlan information\n"
	  "VLANLIST string(eg:2,4,5,7,9-100)\n");

DEFUN(no_vlan_add,
	  no_vlan_add_cmd,
	  "no " CMD_VLAN_STR,
	  NO_STR
		  CMD_VLAN_STR_HELP)
{
	int ret = 0;
	vlan_t vlan = 0;
	if (strstr(argv[0], "-") || strstr(argv[0], ","))
	{
		vlan_t vlanlist[VLAN_TABLE_MAX];
		zpl_int32 num = 0;
		memset(vlanlist, 0, sizeof(vlanlist));
		num = nsm_vlan_list_split_api(argv[0], &vlanlist);
		if (num == ERROR)
		{
			vty_out(vty, "Error:Can not split vlan list table.%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		if (!nsm_vlan_list_lookup_api(vlanlist, num))
		{
			vty_out(vty, "Error:some vlan is not exist.%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
#if 1
		ret = nsm_vlan_list_destroy_api(argv[0]);
#else

#endif
	}
	else
	{
		VTY_GET_INTEGER("vlan ID", vlan, argv[0]);
		if (nsm_vlan_lookup_api(vlan))
			ret = nsm_vlan_destroy_api(vlan);
		else
		{
			vty_out(vty, "Error:Can not Find This vlan %d%s", vlan, VTY_NEWLINE);
			return CMD_WARNING;
		}
	}
	if (ret != OK)
		vty_out(vty, "Error:Can not Destroy vlan %s", VTY_NEWLINE);
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

ALIAS(no_vlan_add,
	  no_vlan_add_list_cmd,
	  "no vlan VLANLIST",
	  NO_STR
	  "Vlan information\n"
	  "VLANLIST string(eg:2,4,5,7,9-100)\n");

DEFUN(vlan_id,
	  vlan_id_cmd,
	  CMD_VLAN_STR,
	  CMD_VLAN_STR_HELP)
{
	int ret = 0;
	vlan_t vlan = 0;
	l2vlan_t *l2vlan = NULL;
	{
		VTY_GET_INTEGER("vlan ID", vlan, argv[0]);
		l2vlan = nsm_vlan_lookup_api(vlan);
		if (l2vlan)
		{
			vty->node = VLAN_NODE;
			vty->index_value = vlan;
			ret = OK;
		}
		else
		{
			ret = nsm_vlan_create_api(vlan, NULL);
			if (ret == OK)
			{
				vty->node = VLAN_NODE;
				vty->index_value = vlan;
			}
			if (ret != OK)
				vty_out(vty, "Error:Can not Create vlan %s", VTY_NEWLINE);
		}
	}
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(no_vlan_id,
	  no_vlan_id_cmd,
	  "no " CMD_VLAN_STR,
	  NO_STR
		  CMD_VLAN_STR_HELP)
{
	int ret = 0;
	vlan_t vlan = 0;
	l2vlan_t *l2vlan = NULL;
	{
		VTY_GET_INTEGER("vlan ID", vlan, argv[0]);
		l2vlan = nsm_vlan_lookup_api(vlan);
		if (!l2vlan)
		{
			vty_out(vty, "Error:can not find this vlan %d %s", vlan, VTY_NEWLINE);
			return CMD_WARNING;
		}
		ret = nsm_vlan_destroy_api(vlan);
		if (ret != OK)
			vty_out(vty, "Error:Can not Destroy vlan %s", VTY_NEWLINE);
	}
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(vlan_name,
	  vlan_name_cmd,
	  "vlan name NAME",
	  "Vlan information\n"
	  "Name information\n"
	  "Name string\n")
{
	int ret = 0;
	{
		ret = nsm_vlan_name_api(vty->index_value, argv[0]);
	}
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(no_vlan_name,
	  no_vlan_name_cmd,
	  "no vlan name",
	  NO_STR
	  "Vlan information\n"
	  "Name information\n")
{
	int ret = 0;
	{
		ret = nsm_vlan_name_api(vty->index_value, NULL);
	}
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(switchport_access_vlan,
	  switchport_access_vlan_cmd,
	  "switchport access " CMD_VLAN_STR,
	  "Switchport interface\n"
	  "access port\n" CMD_VLAN_STR_HELP)
{
	int ret = 0;
	int mode = 0;
	vlan_t vlan = 0, oldvlan = 0;
	struct interface *ifp = vty->index;
	VTY_GET_INTEGER("vlan ID", vlan, argv[0]);
	if (!nsm_vlan_lookup_api(vlan))
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
	int mode = 0;
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
	int mode = 0;
	vlan_t vlan = 0;
	struct interface *ifp = vty->index;
	VTY_GET_INTEGER("vlan ID", vlan, argv[0]);
	if (!nsm_vlan_lookup_api(vlan))
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
	int mode = 0;
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
	int mode = 0;
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
			num = nsm_vlan_list_split_api(argv[1], &vlanlist);
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
			if (!nsm_vlan_lookup_api(vlan))
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
					num = nsm_vlan_list_split_api(argv[1], &vlanlist);
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
					if (!nsm_vlan_lookup_api(vlan))
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

DEFUN(show_vlan_brief,
	  show_vlan_brief_cmd,
	  "show vlan brief",
	  SHOW_STR
	  "vlan-database\n"
	  "brief\n")
{
	struct vlan_user vuser;
	os_memset(&vuser, 0, sizeof(vuser));
	vuser.vty = vty;
	vty_out(vty, "%-6s %-16s %-8s %-8s %-16s %-16s%s", "VLANID", "NAME", "STATE", "MSTP", "Untagport", "Tagport", VTY_NEWLINE);
	vty_out(vty, "%-6s %-16s %-8s %-8s %-16s %-16s%s", "------", "----------------", "--------",
			"--------", "----------------", "----------------", VTY_NEWLINE);
	nsm_vlan_callback_api((l2vlan_cb)show_nsm_vlan_database_one, &vuser);
	return CMD_SUCCESS;
}

DEFUN(show_vlan_id,
	  show_vlan_id_cmd,
	  "show vlan id <1-4094>",
	  SHOW_STR
	  "vlan-database\n"
	  "Vlan id\n"
	  "vlan id\n")
{
	vlan_t vlan = 0;
	l2vlan_t *node = NULL;
	struct vlan_user vuser;
	os_memset(&vuser, 0, sizeof(vuser));
	vuser.vty = vty;
	if (all_digit(argv[0]))
	{
		vlan = atoi(argv[0]);
		node = nsm_vlan_lookup_api(vlan);
	}
	else
	{
		node = nsm_vlan_lookup_by_name_api(argv[0]);
	}
	if (node)
	{
		vty_out(vty, "%-6s %-16s %-8s %-8s %-16s %-16s%s", "VLANID", "NAME", "STATE", "MSTP", "Untagport", "Tagport", VTY_NEWLINE);
		vty_out(vty, "%-6s %-16s %-8s %-8s %-16s %-16s%s", "------", "----------------", "--------",
				"--------", "----------------", "----------------", VTY_NEWLINE);
		show_nsm_vlan_database_one(node, &vuser);
	}
	return CMD_SUCCESS;
}

ALIAS(show_vlan_id,
	  show_vlan_name_cmd,
	  "show vlan name NAME",
	  SHOW_STR
	  "vlan-database\n"
	  "Vlan name\n"
	  "vlan name\n");

static int show_nsm_vlan_database_one(l2vlan_t *node, struct vlan_user *pVoid)
{
	zpl_uint32 i = 0;
	int frist = 0;
	char vlan[16], name[32], state[16], mstp[16], member1[64], member2[64], empty[128];
	struct vty *vty = pVoid->vty;
	memset(vlan, 0, sizeof(vlan));
	memset(name, 0, sizeof(name));
	memset(state, 0, sizeof(state));
	memset(mstp, 0, sizeof(mstp));
	memset(member1, 0, sizeof(member1));
	memset(member2, 0, sizeof(member2));
	memset(empty, 0, sizeof(empty));
	sprintf(empty, "%s", "----------------");
	sprintf(vlan, "%d", node->vlan);
	if (node->vlan_name)
		sprintf(name, "%s", node->vlan_name);
	else
		sprintf(name, "VLAN%04d", node->vlan);
	sprintf(state, "%s", "ACTIVE");
	sprintf(mstp, "%d", node->mstp);

	if (node->vlan == 1)
	{
		vty_out(vty, "%-6s %-16s %-8s %-8s ", vlan, name, state, mstp);
		for (i = 0; i < PHY_PORT_MAX; i++)
		{
			if (node->untagport[i] || node->tagport[i])
			{
				if (frist)
				{
					vty_out(vty, "                                          %-16s ",
							node->untagport[i] ? if_ifname_abstract_make(node->untagport[i]) : empty);
					vty_out(vty, "%-16s%s",
							node->tagport[i] ? if_ifname_abstract_make(node->tagport[i]) : empty, VTY_NEWLINE);
				}
				else
				{
					vty_out(vty, "%-16s ", node->untagport[i] ? if_ifname_abstract_make(node->untagport[i]) : empty);
					vty_out(vty, "%-16s%s", node->tagport[i] ? if_ifname_abstract_make(node->tagport[i]) : empty, VTY_NEWLINE);
					frist = 1;
				}
			}
		}
	}
	else
	{
		frist = 0;
		vty_out(vty, "%-6s %-16s %-8s %-8s ", vlan, name, state, mstp);
		for (i = 0; i < PHY_PORT_MAX; i++)
		{
			if (node->untagport[i] || node->tagport[i])
			{
				if (frist)
				{
					vty_out(vty, "                                          %-16s ",
							node->untagport[i] ? if_ifname_abstract_make(node->untagport[i]) : empty);
					vty_out(vty, "%-16s%s",
							node->tagport[i] ? if_ifname_abstract_make(node->tagport[i]) : empty, VTY_NEWLINE);
				}
				else
				{
					vty_out(vty, "%-16s ", node->untagport[i] ? if_ifname_abstract_make(node->untagport[i]) : empty);
					vty_out(vty, "%-16s%s", node->tagport[i] ? if_ifname_abstract_make(node->tagport[i]) : empty, VTY_NEWLINE);
					frist = 1;
				}
			}
		}
	}
	if (frist == 0)
		vty_out(vty, "%s", VTY_NEWLINE);
	return OK;
}

#ifdef ZPL_CLI_DEBUG
static int vlan_database_show_node(l2vlan_t *node, struct vlan_user *pVoid)
{
	struct vty *vty = pVoid->vty;

	if (node->maxvlan)
	{
		vty_out(vty, "vlan %d -> %d%s", node->vlan, node->maxvlan, VTY_NEWLINE);
	}
	else
		vty_out(vty, "vlan %d%s", node->vlan, VTY_NEWLINE);
	return OK;
}

DEFUN_HIDDEN(show_vlan_database_node,
			 show_vlan_database_node_cmd,
			 "show vlan node",
			 SHOW_STR
			 "vlan-database\n"
			 "node information\n")
{
	struct vlan_user vuser;
	os_memset(&vuser, 0, sizeof(vuser));
	vuser.vty = vty;
	nsm_vlan_callback_api((l2vlan_cb)vlan_database_show_node, &vuser);
	return CMD_SUCCESS;
}
#endif

static int nsm_vlan_database_config_one(l2vlan_t *node, struct vlan_user *pVoid)
{
	char tmp[32];
	if (node->vlan == 1)
		return OK;
	// 2,4,6,7,8-12,14-33,36,78
	// 2,4,6,7,8-12,14,15,56,36-78
	// 8-12,14,15,56,36-78
	memset(tmp, 0, sizeof(tmp));
	if (node->minvlan && node->maxvlan)
	{
		if (node->vlan == node->minvlan)
		{
			sprintf(tmp, "%d-%d", node->vlan, node->maxvlan);
			if (pVoid->frist)
			{
				strcat(pVoid->cli_str, ",");
			}
			strcat(pVoid->cli_str, tmp);
			pVoid->frist = 1;
		}
	}
	else
	{
		sprintf(tmp, "%d", node->vlan);
		if (pVoid->frist)
		{
			strcat(pVoid->cli_str, ",");
		}
		strcat(pVoid->cli_str, tmp);
		pVoid->frist = 1;
	}
	pVoid->total++;
	return OK;
}

static int nsm_vlan_database_config(struct vty *vty)
{
	struct vlan_user vuser;
	os_memset(&vuser, 0, sizeof(vuser));
	vuser.vty = vty;
	nsm_vlan_callback_api((l2vlan_cb)nsm_vlan_database_config_one, &vuser);
	if (vuser.total)
	{
		vty_out(vty, "vlan-database%s", VTY_NEWLINE);
		vty_out(vty, " vlan %s%s", vuser.cli_str, VTY_NEWLINE);
		vty_out(vty, "!%s", VTY_NEWLINE);
	}
	return 1;
}

static int nsm_vlan_config_one(l2vlan_t *node, struct vlan_user *pVoid)
{
	struct vty *vty = pVoid->vty;
	if (node->vlan_name)
	{
		vty_out(vty, "vlan %d%s", node->vlan, VTY_NEWLINE);
		vty_out(vty, "name %s%s", node->vlan_name, VTY_NEWLINE);
		vty_out(vty, "!%s", VTY_NEWLINE);
		pVoid->total++;
	}
	return OK;
}

static int nsm_vlan_config(struct vty *vty)
{
	struct vlan_user vuser;
	os_memset(&vuser, 0, sizeof(vuser));
	vuser.vty = vty;
	nsm_vlan_callback_api((l2vlan_cb)nsm_vlan_config_one, &vuser);
	if (vuser.total)
		vty_out(vty, "%s", VTY_NEWLINE);
	return 1;
}

static struct cmd_node vlan_database_node =
	{
		VLAN_DATABASE_NODE,
		"%s(config-vlan-database)# ",
		1};

static struct cmd_node vlan_node =
	{
		VLAN_NODE,
		"%s(config-vlan)# ",
		1};

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
}

static void cmd_vlan_show_init(int node)
{

	install_element(node, CMD_VIEW_LEVEL, &show_vlan_brief_cmd);
	install_element(node, CMD_VIEW_LEVEL, &show_vlan_id_cmd);
	install_element(node, CMD_VIEW_LEVEL, &show_vlan_name_cmd);

#ifdef ZPL_CLI_DEBUG
	install_element(node, CMD_VIEW_LEVEL, &show_vlan_database_node_cmd);
#endif
}

void cmd_vlan_init(void)
{
	install_node(&vlan_database_node, nsm_vlan_database_config);
	install_node(&vlan_node, nsm_vlan_config);

	install_default(VLAN_DATABASE_NODE);
	install_default_basic(VLAN_DATABASE_NODE);
	install_default(VLAN_NODE);
	install_default_basic(VLAN_NODE);

	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &vlan_database_cmd);

	install_element(VLAN_DATABASE_NODE, CMD_CONFIG_LEVEL, &vlan_add_cmd);
	install_element(VLAN_DATABASE_NODE, CMD_CONFIG_LEVEL, &vlan_add_name_cmd);
	install_element(VLAN_DATABASE_NODE, CMD_CONFIG_LEVEL, &vlan_add_list_cmd);

	install_element(VLAN_DATABASE_NODE, CMD_CONFIG_LEVEL, &no_vlan_add_cmd);
	install_element(VLAN_DATABASE_NODE, CMD_CONFIG_LEVEL, &no_vlan_add_list_cmd);

	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &vlan_id_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_vlan_id_cmd);

	install_element(VLAN_NODE, CMD_CONFIG_LEVEL, &vlan_name_cmd);
	install_element(VLAN_NODE, CMD_CONFIG_LEVEL, &no_vlan_name_cmd);

	// install_element(VLAN_NODE,CMD_CONFIG_LEVEL, &vlan_add_interface_cmd);
	// install_element(VLAN_NODE,CMD_CONFIG_LEVEL, &vlan_add_interface_trunk_cmd);

	// install_element(VLAN_NODE,CMD_CONFIG_LEVEL, &no_vlan_add_interface_cmd);
	// install_element(VLAN_NODE,CMD_CONFIG_LEVEL, &no_vlan_add_interface_trunk_cmd);

	cmd_vlan_interface_attr_init(INTERFACE_NODE);
	cmd_vlan_interface_attr_init(INTERFACE_RANGE_NODE);
	cmd_vlan_interface_attr_init(LAG_INTERFACE_NODE);
	// cmd_vlan_attr_init(EPON_INTERFACE_NODE);
	// cmd_vlan_attr_init(EPON_INTERFACE_RANGE_NODE);

	cmd_vlan_show_init(VLAN_NODE);
	cmd_vlan_show_init(VLAN_DATABASE_NODE);
	cmd_vlan_show_init(CONFIG_NODE);
	cmd_vlan_show_init(ENABLE_NODE);
}
