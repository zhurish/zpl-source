/*
 * cmd_mac.c
 *
 *  Created on: Jan 9, 2018
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

#include "nsm_mac.h"


struct mac_user
{
	struct vty 		*vty;
	mac_type_t 		type;
	mac_action_t	action;
	mac_class_t		class;
	int				istatic;
	int				dynamic;
	int				unicast;
	int				multcast;
	int				broadcast;
	int				discard;
	int				forward;
	BOOL			all;
};

static int show_nsm_mac_address_table(struct vty *vty, const char *type);

DEFUN (mac_address_table_ageing_time,
		mac_address_table_ageing_time_cmd,
		CMD_MAC_ADDRESS_STR " " CMD_AGEING_TIME_STR " <10-1000000>",
		CMD_MAC_ADDRESS_STR_HELP
		CMD_AGEING_TIME_STR_HELP)
{
	int ret = ERROR;
	int ageing;
	if (argc == 0)
		return CMD_SUCCESS;
	VTY_GET_INTEGER ("vlan ID", ageing, argv[0]);

	ret = nsm_mac_ageing_time_set_api(ageing);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_mac_address_table_ageing_time,
		no_mac_address_table_ageing_time_cmd,
		"no "CMD_MAC_ADDRESS_STR " " CMD_AGEING_TIME_STR,
		CMD_MAC_ADDRESS_STR_HELP
		CMD_AGEING_TIME_STR_HELP)
{
	int ret = ERROR;
	ret = nsm_mac_ageing_time_set_api(0);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (mac_address_table,
		mac_address_table_cmd,
		CMD_MAC_ADDRESS_STR " " CMD_MAC_STR " " CMD_FORWARD_STR " "
			CMD_INTERFACE_STR " " CMD_IF_USPV_STR " " CMD_USP_STR " " CMD_VLAN_STR,
		CMD_MAC_ADDRESS_STR_HELP
		CMD_MAC_STR_HELP
		CMD_FORWARD_STR_HELP
		CMD_INTERFACE_STR_HELP
		CMD_IF_USPV_STR_HELP
		CMD_USP_STR_HELP
		CMD_VLAN_STR_HELP)
{
	int ret = ERROR;
	l2mac_t mac;
	memset(&mac, 0, sizeof(mac));
	VTY_IMAC_GET(argv[0], mac.mac);

	mac.ifindex = if_ifindex_make(argv[1], argv[2]);

	VTY_GET_INTEGER ("vlan ID", mac.vlan, argv[3]);
	mac.action = MAC_FORWARD;
	mac.type = MAC_STATIC;
	if(nsm_mac_lookup_api(mac.mac, mac.vlan) == OK)
	{
		vty_out(vty, "Error: This mac address is already exist.%s",VTY_NEWLINE);
		return CMD_WARNING;
	}
	ret = nsm_mac_add_api(&mac);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_mac_address_table,
		no_mac_address_table_cmd,
		"no "CMD_MAC_ADDRESS_STR " " CMD_MAC_STR " " CMD_FORWARD_STR " "
			CMD_INTERFACE_STR " " CMD_IF_USPV_STR " " CMD_USP_STR " " CMD_VLAN_STR,
		CMD_MAC_ADDRESS_STR_HELP
		CMD_MAC_STR_HELP
		CMD_FORWARD_STR_HELP
		CMD_INTERFACE_STR_HELP
		CMD_IF_USPV_STR_HELP
		CMD_USP_STR_HELP
		CMD_VLAN_STR_HELP)
{
	int ret = ERROR;
	l2mac_t mac;
	memset(&mac, 0, sizeof(mac));
	VTY_IMAC_GET(argv[0], mac.mac);

	mac.ifindex = if_ifindex_make(argv[1], argv[2]);

	VTY_GET_INTEGER ("vlan ID", mac.vlan, argv[3]);
	mac.action = MAC_FORWARD;
	mac.type = MAC_STATIC;
	if(nsm_mac_lookup_api(mac.mac, mac.vlan) == OK)
	{
		vty_out(vty, "Error: This mac address is already exist.%s",VTY_NEWLINE);
		return CMD_WARNING;
	}
	ret = nsm_mac_del_api(&mac);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}



DEFUN (mac_address_table_discard,
		mac_address_table_discard_cmd,
		CMD_MAC_ADDRESS_STR " " CMD_MAC_STR " " CMD_DISCARD_STR,
		CMD_MAC_ADDRESS_STR_HELP
		CMD_MAC_STR_HELP
		CMD_DISCARD_STR_HELP)
{
	int ret = ERROR;
	l2mac_t mac;
	memset(&mac, 0, sizeof(mac));
	VTY_IMAC_GET(argv[0], mac.mac);
	mac.action = MAC_DISCARDED;
	mac.type = MAC_STATIC;
	if(nsm_mac_lookup_api(mac.mac, 0) == OK)
	{
		vty_out(vty, "Error: This mac address is already exist.%s",VTY_NEWLINE);
		return CMD_WARNING;
	}
	ret = nsm_mac_add_api(&mac);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_mac_address_table_discard,
		no_mac_address_table_discard_cmd,
		"no "CMD_MAC_ADDRESS_STR " " CMD_MAC_STR " " CMD_DISCARD_STR,
		CMD_MAC_ADDRESS_STR_HELP
		CMD_MAC_STR_HELP
		CMD_DISCARD_STR_HELP)
{
	int ret = ERROR;
	l2mac_t mac;
	memset(&mac, 0, sizeof(mac));
	VTY_IMAC_GET(argv[0], mac.mac);

	mac.action = MAC_DISCARDED;
	mac.type = MAC_STATIC;
	if(nsm_mac_lookup_api(mac.mac, 0) == OK)
	{
		vty_out(vty, "Error: This mac address is already exist.%s",VTY_NEWLINE);
		return CMD_WARNING;
	}
	ret = nsm_mac_del_api(&mac);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (clear_mac_address_table,
		clear_mac_address_table_cmd,
		"clear "CMD_MAC_ADDRESS_STR " (static|dynamic) " CMD_VLAN_STR,
		CMD_MAC_ADDRESS_STR_HELP
		"Static\n"
		"Dynamic\n"
		"Multicast\n"
		CMD_VLAN_STR_HELP)
{
	int ret = ERROR;
	vlan_t vlan;
	if(!os_strstr(argv[1], "-"))
	{
		VTY_GET_INTEGER ("vlan ID", vlan, argv[1]);
		if(memcmp(argv[0], "static", 4) == 0)
			ret = nsm_mac_clean_vlan_api(MAC_STATIC, vlan);
		if(memcmp(argv[0], "dynamic", 4) == 0)
			ret = nsm_mac_clean_vlan_api(MAC_DYNAMIC, vlan);
	}
	else
	{
		l2mac_t mac;
		memset(&mac, 0, sizeof(mac));
		VTY_IMAC_GET(argv[0], mac.mac);
		if(nsm_mac_lookup_api(mac.mac, 0) == OK)
		{
			vty_out(vty, "Error: This mac address is already exist.%s",VTY_NEWLINE);
			return CMD_WARNING;
		}
		ret = nsm_mac_del_api(&mac);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(clear_mac_address_table,
		clear_mac_address_table_address_cmd,
		"clear "CMD_MAC_ADDRESS_STR " (static|dynamic) address " CMD_MAC_STR,
		CMD_MAC_ADDRESS_STR_HELP
		"Static\n"
		"Dynamic\n"
		"Multicast\n"
		"Mac address"
		CMD_MAC_STR_HELP);


DEFUN (clear_mac_address_table_interface,
		clear_mac_address_table_interface_cmd,
		"clear "CMD_MAC_ADDRESS_STR " (static|dynamic) " CMD_INTERFACE_STR " " CMD_IF_USPV_STR " " CMD_USP_STR ,
		CMD_MAC_ADDRESS_STR_HELP
		"Static\n"
		"Dynamic\n"
		"Multicast\n"
		CMD_INTERFACE_STR_HELP
		CMD_IF_USPV_STR_HELP
		CMD_USP_STR_HELP)
{
	int ret = ERROR;
	ifindex_t ifindex = if_ifindex_make(argv[1], argv[2]);
	if(memcmp(argv[0], "static", 4) == 0)
		ret = nsm_mac_clean_ifindex_api(MAC_STATIC, ifindex);
	if(memcmp(argv[0], "dynamic", 4) == 0)
		ret = nsm_mac_clean_ifindex_api(MAC_DYNAMIC, ifindex);

	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (show_mac_address_table,
		show_mac_address_table_cmd,
		"show "CMD_MAC_ADDRESS_STR,
		SHOW_STR
		CMD_MAC_ADDRESS_STR_HELP)
{
	int ret = ERROR;
	if(argc == 0)
		ret = show_nsm_mac_address_table(vty, NULL);
	else if(argc == 1)
	{
		if(memcpy(argv[0], "ageing", 4)==0)
		{
			int ageing = 0;
			ret = nsm_mac_ageing_time_get_api(&ageing);
			if(ret == OK)
				vty_out(vty, "MAC address table ageing time is %d seconds%s", ageing, VTY_NEWLINE);
		}
		else
		{
			ret = show_nsm_mac_address_table(vty, argv[0]);
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(show_mac_address_table,
		show_mac_address_table_value_cmd,
		"show "CMD_MAC_ADDRESS_STR " ("CMD_AGEING_TIME_STR"|count|mac-filter|static|dynamic)",
		SHOW_STR
		CMD_MAC_ADDRESS_STR_HELP
		CMD_AGEING_TIME_STR_HELP
		"MAC Count information\n"
		"Filter MAC information\n");


static int nsm_mac_address_table_summary(l2mac_t *node, struct mac_user *user)
{
	if(node->action == MAC_BROADCAST)
		user->broadcast++;
	else if(node->action == MAC_MULTICAST)
		user->multcast++;
	else
		user->unicast++;
	if(node->type == MAC_STATIC)
		user->istatic++;
	else
		user->dynamic++;

	if(node->action == MAC_DISCARDED)
		user->discard++;
	else
		user->forward++;
	return OK;
}

static int show_nsm_mac_address_table_head(struct mac_user *user)
{
	struct vty *vty = user->vty;
	vty_out(vty, "  MAC Address Table: %s", VTY_NEWLINE);
	vty_out(vty, "   Total			: %d%s", (user->istatic+user->dynamic), VTY_NEWLINE);
	vty_out(vty, "   Static		: %d%s", user->istatic, VTY_NEWLINE);
	vty_out(vty, "   Dynamic		: %d%s", user->dynamic, VTY_NEWLINE);
	vty_out(vty, "   Forward		: %d%s", user->forward, VTY_NEWLINE);
	vty_out(vty, "   Discard		: %d%s", user->discard, VTY_NEWLINE);
	vty_out(vty, "   Unicast		: %d%s", user->unicast, VTY_NEWLINE);
	vty_out(vty, "   Multcast		: %d%s", user->multcast, VTY_NEWLINE);
	vty_out(vty, "   Broadcast		: %d%s", user->broadcast, VTY_NEWLINE);
	vty_out(vty, " ---------------------------------------------------------%s", VTY_NEWLINE);
	vty_out(vty, " %-6s %-24s %-8s %-16s %s", "Vlan", "MAC Address", "Type", "Interface", VTY_NEWLINE);
	vty_out(vty, " ------ ------------------------ -------- ---------------- %s", VTY_NEWLINE);
	return OK;
}

static int show_nsm_mac_address_table_detail(l2mac_t *node, struct mac_user *user)
{
	char mac[32], vlan[16], type[16], ifname[32];
	struct vty *vty = user->vty;
	os_memset(mac, 0, sizeof(mac));
	os_memset(vlan, 0, sizeof(vlan));
	os_memset(type, 0, sizeof(type));
	os_memset(ifname, 0, sizeof(ifname));
	sprintf(mac, "%02x%02x-%02x%02x-%02x%02x",node->mac[0],node->mac[1],node->mac[2],
											 node->mac[3],node->mac[4],node->mac[5]);

	sprintf(vlan, "%d",node->vlan);
	sprintf(type, "%s",(node->type == MAC_STATIC) ? "static":"dynamic");
	sprintf(ifname, "%s",ifindex2ifname(node->ifindex));
	if(user->type && user->type == node->type)
	{
		vty_out(vty, " %-6s %-24s %-8s %-16s %s", vlan, mac, type, ifname, VTY_NEWLINE);
	}
	else if(user->action && user->action == node->action)
	{
		vty_out(vty, " %-6s %-24s %-8s %-16s %s", vlan, mac, type, ifname, VTY_NEWLINE);
	}
	else if(user->class && user->class == node->class)
	{
		vty_out(vty, " %-6s %-24s %-8s %-16s %s", vlan, mac, type, ifname, VTY_NEWLINE);
	}
	else if(user->all)
	{
		vty_out(vty, " %-6s %-24s %-8s %-16s %s", vlan, mac, type, ifname, VTY_NEWLINE);
	}
	return OK;
}

static int show_nsm_mac_address_table(struct vty *vty, const char *type)
{
	struct mac_user user;
	memset(&user, 0, sizeof(user));
	user.vty = vty;
	if(type == NULL)
	{
		user.all = TRUE;
		nsm_mac_callback_api((l2mac_cb)nsm_mac_address_table_summary, &user);
		show_nsm_mac_address_table_head(&user);
		nsm_mac_callback_api((l2mac_cb)show_nsm_mac_address_table_detail, &user);
	}
	else
	{
		if(memcpy(type, "count", 4)==0)
		{
			user.all = TRUE;
			nsm_mac_callback_api((l2mac_cb)nsm_mac_address_table_summary, &user);
			show_nsm_mac_address_table_head(&user);
		}
		else if(memcpy(type, "mac-filter", 4)==0)
		{
			user.action = MAC_DISCARDED;
			nsm_mac_callback_api((l2mac_cb)nsm_mac_address_table_summary, &user);
			show_nsm_mac_address_table_head(&user);
			nsm_mac_callback_api((l2mac_cb)show_nsm_mac_address_table_detail, &user);
		}
		else if(memcpy(type, "static", 4)==0)
		{
			user.type = MAC_STATIC;
			nsm_mac_callback_api((l2mac_cb)nsm_mac_address_table_summary, &user);
			show_nsm_mac_address_table_head(&user);
			nsm_mac_callback_api((l2mac_cb)show_nsm_mac_address_table_detail, &user);
		}
		else if(memcpy(type, "dynamic", 4)==0)
		{
			user.type = MAC_DYNAMIC;
			nsm_mac_callback_api((l2mac_cb)nsm_mac_address_table_summary, &user);
			show_nsm_mac_address_table_head(&user);
			nsm_mac_callback_api((l2mac_cb)show_nsm_mac_address_table_detail, &user);
		}
	}
	return OK;
}


static int _nsm_mac_address_table_config(l2mac_t *node, struct mac_user *user)
{
	char mac[32], vlan[16],ifname[32];
	struct vty *vty = user->vty;
	if(node->type  != MAC_STATIC)
		return 0;
	os_memset(mac, 0, sizeof(mac));
	os_memset(vlan, 0, sizeof(vlan));
	os_memset(ifname, 0, sizeof(ifname));
	sprintf(mac, "%02x%02x-%02x%02x-%02x%02x",node->mac[0],node->mac[1],node->mac[2],
											 node->mac[3],node->mac[4],node->mac[5]);

	sprintf(ifname, "%s",ifindex2ifname(node->ifindex));
	//mac-address-table 0000-1111-2222 forward interface gigabitethernet 0/1/1 vlan 11
	//mac-address-table 0000-1111-2222 discard
	//mac-address-table ageing-time 33
	if(node->action == MAC_DISCARDED)
	{
		vty_out(vty, "mac-address-table %s discard %s", mac, VTY_NEWLINE);
	}
	if(node->action == MAC_FORWARD)
	{
		vty_out(vty, "mac-address-table %s forward interface %s %s %s", mac, ifname, vlan, VTY_NEWLINE);
	}
	return OK;
}

int nsm_mac_address_table_config(struct vty *vty)
{
	struct mac_user user;
	memset(&user, 0, sizeof(user));
	user.vty = vty;
	nsm_mac_callback_api((l2mac_cb)_nsm_mac_address_table_config, &user);
	return 1;
}

int nsm_mac_address_table_ageing_config(struct vty *vty)
{
	int agtime = 0;
	nsm_mac_ageing_time_get_api(&agtime);
	//mac-address-table ageing-time 33
	vty_out(vty, "mac-address-table ageing-time %d %s", agtime, VTY_NEWLINE);
	return 1;
}




void cmd_mac_init(void)
{
//	install_default(CONFIG_NODE);
	install_element(CONFIG_NODE, &mac_address_table_ageing_time_cmd);
	install_element(CONFIG_NODE, &no_mac_address_table_ageing_time_cmd);

	install_element(CONFIG_NODE, &mac_address_table_cmd);
	install_element(CONFIG_NODE, &no_mac_address_table_cmd);

	install_element(CONFIG_NODE, &mac_address_table_discard_cmd);
	install_element(CONFIG_NODE, &no_mac_address_table_discard_cmd);

	install_element(CONFIG_NODE, &clear_mac_address_table_cmd);
	install_element(CONFIG_NODE, &clear_mac_address_table_address_cmd);
	install_element(CONFIG_NODE, &clear_mac_address_table_interface_cmd);

	install_element(CONFIG_NODE, &mac_address_table_cmd);
	install_element(CONFIG_NODE, &mac_address_table_cmd);

	install_element(ENABLE_NODE, &show_mac_address_table_cmd);
	install_element(CONFIG_NODE, &show_mac_address_table_cmd);

	install_element(ENABLE_NODE, &show_mac_address_table_value_cmd);
	install_element(CONFIG_NODE, &show_mac_address_table_value_cmd);
}

