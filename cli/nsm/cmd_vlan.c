/*
 * nsm_vlan.c
 *
 *  Created on: Jan 11, 2018
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
#include "vrf.h"
#include "interface.h"
#include "if_name.h"

#include "nsm_vlan.h"

struct vlan_user
{
	struct vty 		*vty;
	vlan_t			vlan;
	vlan_mode_t		mode;
	ifindex_t		ifindex;
	int				itag;
	int				iuntag;
	int				total;
	BOOL			all;

	int 			frist;
	char			cli_str[256];
};


//static int show_nsm_vlan_database(struct vty *vty, const char *type);
static int show_nsm_vlan_database_one(l2vlan_t *node, struct vlan_user *pVoid);


DEFUN (vlan_database,
		vlan_database_cmd,
		CMD_VLAN_DATABASE_STR,
		CMD_VLAN_STR_DATABASE_HELP)
{
	if(nsm_vlan_is_enable())
		vty->node = VLAN_DATABASE_NODE;
	else
	{
		if(nsm_vlan_enable() == OK)
			vty->node = VLAN_DATABASE_NODE;
	}
	return CMD_SUCCESS;
}

DEFUN (vlan_add,
	vlan_add_cmd,
	CMD_VLAN_STR,
	CMD_VLAN_STR_HELP)
{
	int ret = 0;
	vlan_t vlan = 0;
	if(strstr(argv[0], "-") || strstr(argv[0], ","))
	{
#if 1
		ret = nsm_vlan_list_create_api(argv[0]);
#else
		int i = 0, num = 0;
		vlan_t value[VLAN_TABLE_MAX];
		vlan_t base = 0, end = 0;
		num = vlan_string_explain(argv[0], value, VLAN_TABLE_MAX, &base, &end);
		if(num >= 0)
		{
			if(base && end)
				ret = nsm_vlan_batch_create_api(base, end);
			if(num)
			{
				for(i = 0; i < num; i++)
				{
					if(value[i])
					{
						ret = nsm_vlan_create_api(value[i]);
						if(ret != OK)
							break;
					}
				}
			}
		}
#endif
	}
	else
	{
		VTY_GET_INTEGER ("vlan ID", vlan, argv[0]);
		ret = nsm_vlan_create_api(vlan);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(vlan_add,
	vlan_add_list_cmd,
	"vlan VLANLIST",
	"Vlan information\n"
	"VLANLIST string(eg:2,4,5,7,9-100)\n");


DEFUN (no_vlan_add,
	no_vlan_add_cmd,
	"no "CMD_VLAN_STR,
	NO_STR
	CMD_VLAN_STR_HELP)
{
	int ret = 0;
	vlan_t vlan = 0;
	if(strstr(argv[0], "-") || strstr(argv[0], ","))
	{
#if 1
		ret = nsm_vlan_list_destroy_api(argv[0]);
#else
		int i = 0, num = 0;
		vlan_t value[VLAN_TABLE_MAX];
		vlan_t base = 0, end = 0;
		num = vlan_string_explain(argv[0], value, VLAN_TABLE_MAX, &base, &end);
		if(num >= 0)
		{
			if(base && end)
				ret = nsm_vlan_batch_destroy_api(base, end);
			if(num)
			{
				for(i = 0; i < num; i++)
				{
					if(value[i])
					{
						ret = nsm_vlan_destroy_api(value[i]);
						if(ret != OK)
							break;
					}
				}
			}
		}
#endif
	}
	else
	{
		VTY_GET_INTEGER ("vlan ID", vlan, argv[0]);
		ret = nsm_vlan_destroy_api(vlan);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


ALIAS(no_vlan_add,
	no_vlan_add_list_cmd,
	"no vlan VLANLIST",
	NO_STR
	"Vlan information\n"
	"VLANLIST string(eg:2,4,5,7,9-100)\n");



DEFUN (vlan_id,
	vlan_id_cmd,
	CMD_VLAN_STR,
	CMD_VLAN_STR_HELP)
{
	int ret = 0;
	vlan_t vlan = 0;
	l2vlan_t *l2vlan = NULL;
	{
		VTY_GET_INTEGER ("vlan ID", vlan, argv[0]);
		l2vlan = nsm_vlan_lookup_api( vlan);
		if(l2vlan)
		{
			vty->node = VLAN_NODE;
			vty->index_value = vlan;
			ret = OK;
		}
		else
		{
			ret = nsm_vlan_create_api(vlan);
			if(ret == OK)
			{
				vty->node = VLAN_NODE;
				vty->index_value = vlan;
			}
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_vlan_id,
	no_vlan_id_cmd,
	"no "CMD_VLAN_STR,
	NO_STR
	CMD_VLAN_STR_HELP)
{
	int ret = 0;
	vlan_t vlan = 0;
	l2vlan_t *l2vlan = NULL;
	{
		VTY_GET_INTEGER ("vlan ID", vlan, argv[0]);
		l2vlan = nsm_vlan_lookup_api( vlan);
		if(!l2vlan)
		{
			vty_out(vty, "Error:can not find this vlan %d %s", vlan, VTY_NEWLINE);
			return CMD_WARNING;
		}
		ret = nsm_vlan_destroy_api(vlan);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (vlan_name,
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
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_vlan_name,
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
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (vlan_add_interface,
		vlan_add_interface_cmd,
		"interface " CMD_IF_USPV_STR " "CMD_USP_STR,
		"Select an interface to configure\n"
		CMD_IF_USPV_STR_HELP
		CMD_USP_STR_HELP)
{
	int ret = 0;
	struct interface *ifp = NULL;
	ifp = if_lookup_by_name(if_ifname_format(argv[0], argv[1]));
	if(!ifp)
	{
		vty_out(vty, "Can not find this interface.%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(argc == 3)
	{
		if(nsm_interface_lookup_tag_vlan_api(vty->index_value, ifp) == OK)
		{
			vty_out(vty, "this interface is already below this vlan in tag mode.%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		ret = nsm_interface_add_tag_vlan_api(vty->index_value, ifp);
	}
	else
	{
		if(nsm_interface_lookup_untag_vlan_api(vty->index_value, ifp) == OK)
		{
			vty_out(vty, "this interface is already below this vlan. in untag mode.%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		ret = nsm_interface_add_untag_vlan_api(vty->index_value, ifp);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(vlan_add_interface,
	vlan_add_interface_trunk_cmd,
	"interface " CMD_IF_USPV_STR " "CMD_USP_STR " (trunk|)",
	"Select an interface to configure\n"
	CMD_IF_USPV_STR_HELP
	CMD_USP_STR_HELP
	"trunk mode\n");


DEFUN (no_vlan_add_interface,
		no_vlan_add_interface_cmd,
		"no interface " CMD_IF_USPV_STR " "CMD_USP_STR,
		NO_STR
		"Select an interface to configure\n"
		CMD_IF_USPV_STR_HELP
		CMD_USP_STR_HELP)
{
	int ret = 0;
	struct interface *ifp = NULL;
	ifp = if_lookup_by_name(if_ifname_format(argv[0], argv[1]));
	if(!ifp)
	{
		vty_out(vty, "Can not find this interface.%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(argc == 3)
	{
		if(nsm_interface_lookup_tag_vlan_api(vty->index_value, ifp) != OK)
		{
			vty_out(vty, "this interface is not below this vlan in tag mode.%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		ret = nsm_interface_del_tag_vlan_api(vty->index_value, ifp);
	}
	else
	{
		if(nsm_interface_lookup_untag_vlan_api(vty->index_value, ifp) != OK)
		{
			vty_out(vty, "this interface is not below this vlan. in untag mode.%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		ret = nsm_interface_del_untag_vlan_api(vty->index_value, ifp);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(no_vlan_add_interface,
	no_vlan_add_interface_trunk_cmd,
	"no interface " CMD_IF_USPV_STR " "CMD_USP_STR " (trunk|)",
	NO_STR
	"Select an interface to configure\n"
	CMD_IF_USPV_STR_HELP
	CMD_USP_STR_HELP
	"trunk mode\n");



DEFUN (switchport_access_vlan,
		switchport_access_vlan_cmd,
		"switchport access "CMD_VLAN_STR,
		"Switchport interface\n"
		"access port\n"
		CMD_VLAN_STR_HELP)
{
	int ret = 0;
	int mode = 0;
	vlan_t vlan = 0, oldvlan= 0;
	struct interface *ifp = vty->index;
	VTY_GET_INTEGER ("vlan ID", vlan, argv[0]);
	if(ifp)
	{
		if(nsm_interface_mode_get_api(ifp, &mode) == OK && mode == IF_MODE_ACCESS_L2)
		{
			if(nsm_interface_access_vlan_get_api(ifp, &oldvlan) == OK)
			{
				if(oldvlan != vlan)
					ret = nsm_interface_access_vlan_set_api(ifp, vlan);
				return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
			}
		}
	}
	return CMD_WARNING;
}


DEFUN (no_switchport_access_vlan,
		no_switchport_access_vlan_cmd,
		"no switchport access vlan",
		NO_STR
		"Switchport interface\n"
		"access port\n"
		"Vlan information\n")
{
	int ret = 0;
	int mode = 0;
	vlan_t vlan = 0, oldvlan= 0;
	struct interface *ifp = vty->index;
	//VTY_GET_INTEGER ("vlan ID", vlan, argv[0]);
	if(ifp)
	{
		if(nsm_interface_mode_get_api(ifp, &mode) == OK && mode == IF_MODE_ACCESS_L2)
		{
			if(nsm_interface_access_vlan_get_api(ifp, &oldvlan) == OK)
			{
				if(oldvlan != vlan)
					ret = nsm_interface_access_vlan_set_api(ifp, vlan);
				return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
			}
		}
	}
	return CMD_WARNING;
}




DEFUN (switchport_trunk_vlan,
		switchport_trunk_vlan_cmd,
		"switchport trunk native "CMD_VLAN_STR,
		"Switchport interface\n"
		"trunk port\n"
		CMD_VLAN_STR_HELP)
{
	int ret = 0;
	int mode = 0;
	vlan_t vlan = 0;
	struct interface *ifp = vty->index;
	VTY_GET_INTEGER ("vlan ID", vlan, argv[0]);
	if(ifp)
	{
		if(nsm_interface_mode_get_api(ifp, &mode) == OK && mode == IF_MODE_TRUNK_L2)
		{
			ret = nsm_interface_native_vlan_set_api(ifp, vlan);
			return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
		}
	}
	return CMD_WARNING;
}


DEFUN (no_switchport_trunk_vlan,
		no_switchport_trunk_vlan_cmd,
		"no switchport trunk native "CMD_VLAN_STR,
		NO_STR
		"Switchport interface\n"
		"trunk port\n"
		CMD_VLAN_STR_HELP)
{
	int ret = 0;
	int mode = 0;
	struct interface *ifp = vty->index;
	if(ifp)
	{
		if(nsm_interface_mode_get_api(ifp, &mode) == OK && mode == IF_MODE_TRUNK_L2)
		{
			ret = nsm_interface_native_vlan_set_api(ifp, 0);
			return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
		}
	}
	return CMD_WARNING;
}


DEFUN (switchport_trunk_allow_vlan,
		switchport_trunk_allow_vlan_cmd,
		"switchport trunk allowed (add|remove) "CMD_VLAN_STR,
		"Switchport interface\n"
		"trunk port\n"
		"allowed\n"
		"add\n"
		"remove\n"
		CMD_VLAN_STR_HELP)
{
	int ret = 0;
	vlan_t vlan = 0;
	struct interface *ifp = vty->index;
	int mode = 0;
	if(nsm_interface_mode_get_api(ifp, &mode) != OK || mode != IF_MODE_TRUNK_L2)
		return CMD_WARNING;
	if(argc == 1)
	{
		if(ifp)
		{
			if(os_memcmp(argv[0], "add", 3) == 0)
				ret = nsm_interface_trunk_add_allowed_vlan_api(ifp, 0);
			else
				ret = nsm_interface_trunk_del_allowed_vlan_api(ifp, 0);
			return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
		}
	}
	else if(strstr(argv[1], "-") || strstr(argv[1], ","))
	{
		if(ifp)
		{
			if(os_memcmp(argv[0], "add", 3) == 0)
				ret = nsm_interface_trunk_allowed_vlan_list_api(1, ifp, argv[1]);
			else
				ret = nsm_interface_trunk_allowed_vlan_list_api(0, ifp, argv[1]);


			return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
		}
	}
	else
	{
		VTY_GET_INTEGER ("vlan ID", vlan, argv[1]);
		if(ifp)
		{
			if(os_memcmp(argv[0], "add", 3) == 0)
				ret = nsm_interface_trunk_add_allowed_vlan_api(ifp, vlan);
			else
				ret = nsm_interface_trunk_del_allowed_vlan_api(ifp, vlan);
			return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
		}
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



DEFUN (show_vlan_brief,
	show_vlan_brief_cmd,
	"show vlan brief",
	SHOW_STR
	"vlan-database\n"
	"brief\n")
{
	struct vlan_user vuser;
	os_memset(&vuser, 0, sizeof(vuser));
	vuser.vty = vty;
	vty_out(vty, "%-6s %-16s %-8s %-8s %-10s %-10s%s", "VLANID", "NAME", "STATE", "STP", "Untagport","Tagport",VTY_NEWLINE);
	vty_out(vty, "%-6s %-16s %-8s %-8s %-10s %-10s%s", "------", "----------------", "--------",
			"--------", "----------","----------",VTY_NEWLINE);
	nsm_vlan_callback_api((l2vlan_cb)show_nsm_vlan_database_one, &vuser);
	return CMD_SUCCESS;
}


DEFUN (show_vlan_id,
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
	if(all_digit(argv[0]))
	{
		vlan = atoi(argv[0]);
		node = nsm_vlan_lookup_api(vlan);
	}
	else
	{
		node = nsm_vlan_lookup_by_name_api(argv[0]);
	}
	if(node)
	{
		vty_out(vty, "%-6s %-16s %-8s %-8s %-10s %-10s%s", "VLANID", "NAME", "STATE", "STP", "Untagport","Tagport",VTY_NEWLINE);
		vty_out(vty, "%-6s %-16s %-8s %-8s %-10s %-10s%s", "------", "----------------", "--------",
				"--------", "----------","----------",VTY_NEWLINE);
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
	int i = 0;
	int frist = 0;
	char vlan[16], name[32], state[16], stp[16], member1[64], member2[64], empty[64];
	struct vty *vty = pVoid->vty;
	memset(vlan, 0, sizeof(vlan));
	memset(name, 0, sizeof(name));
	memset(state, 0, sizeof(state));
	memset(stp, 0, sizeof(stp));
	memset(member1, 0, sizeof(member1));
	memset(member2, 0, sizeof(member2));
	memset(empty, 0, sizeof(empty));
	sprintf(empty, "%s", "                                                                ");
	sprintf(vlan, "%d", node->vlan);
	if(node->vlan_name)
		sprintf(name, "%s", node->vlan_name);
	else
		sprintf(name, "VLAN%04d", node->vlan);
	sprintf(state, "%s", "ACTIVE");
	sprintf(stp, "%d", node->stp);

	if(node->vlan==1)
	{
		vty_out(vty, "%-6s %-16s %-8s %-8s ", vlan, name, state, stp);
		for(i = 0; i < PHY_PORT_MAX; i++)
		{
			if(node->untagport[i] || node->tagport[i])
			{
				//sprintf(member1, "%s", ifindex2ifname(node->untagport[i]));
				//vty_iusp_format()
				if(frist)
				{
					vty_out(vty, "                                          %-10s %-10s%s",
							node->untagport[i] ? if_ifname_abstract_make(node->untagport[i]):empty,
									node->tagport[i] ? if_ifname_abstract_make(node->tagport[i]):empty, VTY_NEWLINE);
				}
				else
				{
					vty_out(vty, "%-10s %-10s%s", node->untagport[i] ? if_ifname_abstract_make(node->untagport[i]):empty,
							node->tagport[i] ? if_ifname_abstract_make(node->tagport[i]):empty, VTY_NEWLINE);
					frist = 1;
				}
			}
		}
	}
	else
	{
		vty_out(vty, "%-6s %-16s %-8s %-8s ", vlan, name, state, stp);
		for(i = 0; i < PHY_PORT_MAX; i++)
		{
			if(node->untagport[i] || node->tagport[i])
			{
				//sprintf(member1, "%s", ifindex2ifname(node->untagport[i]));
				if(frist)
				{
					vty_out(vty, "                                          %-10s %-10s%s",
							node->untagport[i] ? if_ifname_abstract_make(node->untagport[i]):empty,
									node->tagport[i] ? if_ifname_abstract_make(node->tagport[i]):empty, VTY_NEWLINE);
				}
				else
				{
					vty_out(vty, "%-10s %-10s%s", node->untagport[i] ? if_ifname_abstract_make(node->untagport[i]):empty,
							node->tagport[i] ? if_ifname_abstract_make(node->tagport[i]):empty, VTY_NEWLINE);
					frist = 1;
				}
			}
		}
	}
	vty_out(vty, "%s", VTY_NEWLINE);
	return OK;
}

#ifdef PL_CLI_DEBUG
static int vlan_database_show_node(l2vlan_t *node, struct vlan_user *pVoid)
{
	struct vty *vty = pVoid->vty;

	if(node->maxvlan)
	{
		vty_out(vty, "vlan %d -> %d%s", node->vlan, node->maxvlan, VTY_NEWLINE);
	}
	else
		vty_out(vty, "vlan %d%s", node->vlan, VTY_NEWLINE);
	return OK;
}

DEFUN_HIDDEN (show_vlan_database_node,
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
	if(node->vlan == 1)
		return OK;
	//2,4,6,7,8-12,14-33,36,78
	//2,4,6,7,8-12,14,15,56,36-78
	//8-12,14,15,56,36-78
	memset(tmp, 0, sizeof(tmp));
	if(node->minvlan && node->maxvlan)
	{
		if(node->vlan == node->minvlan)
		{
			sprintf(tmp, "%d-%d", node->vlan, node->maxvlan);
			if(pVoid->frist)
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
		if(pVoid->frist)
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
	if(vuser.total)
	{
		vty_out(vty, "vlan-database%s", VTY_NEWLINE);
		vty_out(vty, " vlan %s%s",vuser.cli_str, VTY_NEWLINE);
		vty_out(vty, "!%s", VTY_NEWLINE);
	}
	return 1;
}

static int nsm_vlan_config_one(l2vlan_t *node, struct vlan_user *pVoid)
{
	struct vty *vty = pVoid->vty;
	if(node->vlan_name)
	{
		vty_out(vty, "vlan %d%s",node->vlan, VTY_NEWLINE);
		vty_out(vty, "name %s%s",node->vlan_name, VTY_NEWLINE);
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
	if(vuser.total)
		vty_out(vty, "%s", VTY_NEWLINE);
	return 1;
}

static struct cmd_node vlan_database_node =
{
	VLAN_DATABASE_NODE,
	"%s(config-vlan-database)# ",
	1
};

static struct cmd_node vlan_node =
{
	VLAN_NODE,
	"%s(config-vlan)# ",
	1
};

void cmd_vlan_init (void)
{
	install_node(&vlan_database_node, nsm_vlan_database_config);
	install_node(&vlan_node, nsm_vlan_config);

	install_default(VLAN_DATABASE_NODE);
	install_default_basic(VLAN_DATABASE_NODE);
	install_default(VLAN_NODE);
	install_default_basic(VLAN_NODE);

#ifdef PL_CLI_DEBUG
	install_element(ENABLE_NODE, &show_vlan_database_node_cmd);
	install_element(CONFIG_NODE, &show_vlan_database_node_cmd);
#endif

	install_element(ENABLE_NODE, &show_vlan_brief_cmd);
	install_element(CONFIG_NODE, &show_vlan_brief_cmd);
	install_element(VLAN_DATABASE_NODE, &show_vlan_brief_cmd);

	install_element(CONFIG_NODE, &vlan_database_cmd);

	install_element(VLAN_DATABASE_NODE, &vlan_add_cmd);
	install_element(VLAN_DATABASE_NODE, &vlan_add_list_cmd);

	install_element(VLAN_DATABASE_NODE, &no_vlan_add_cmd);
	install_element(VLAN_DATABASE_NODE, &no_vlan_add_list_cmd);


	install_element(CONFIG_NODE, &vlan_id_cmd);
	install_element(CONFIG_NODE, &no_vlan_id_cmd);

	install_element(VLAN_NODE, &vlan_name_cmd);
	install_element(VLAN_NODE, &no_vlan_name_cmd);

	install_element(VLAN_NODE, &vlan_add_interface_cmd);
	install_element(VLAN_NODE, &vlan_add_interface_trunk_cmd);

	install_element(VLAN_NODE, &no_vlan_add_interface_cmd);
	install_element(VLAN_NODE, &no_vlan_add_interface_trunk_cmd);

	install_element(INTERFACE_NODE, &switchport_access_vlan_cmd);
	install_element(INTERFACE_NODE, &no_switchport_access_vlan_cmd);

	//install_element(INTERFACE_NODE, &switchport_access_allow_vlan_cmd);
	install_element(INTERFACE_NODE, &switchport_trunk_vlan_cmd);
	install_element(INTERFACE_NODE, &no_switchport_trunk_vlan_cmd);

	install_element(INTERFACE_NODE, &switchport_trunk_allow_vlan_cmd);
	install_element(INTERFACE_NODE, &switchport_trunk_allow_vlan_all_cmd);
	install_element(INTERFACE_NODE, &switchport_trunk_allow_vlan_list_cmd);

}
