/*
 * cmd_arp.c
 *
 *  Created on: Apr 14, 2018
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
#include "nsm_vrf.h"
#include "nsm_interface.h"
#include "nsm_mac.h"
#include "nsm_arp.h"


struct arp_user
{
	struct vty 		*vty;
	arp_class_t		class;
	ifindex_t		ifindex;
	unsigned short	vlan;
	vrf_id_t		vrfid;
	BOOL			all;
	BOOL			summary;

	//summary
	int				iStatic;
	int				dynamic;
	int				total;
};

static int show_nsm_ip_arp_config(struct vty *vty, struct arp_user *user);

DEFUN (ip_arp_retry_interval,
		ip_arp_retry_interval_cmd,
		CMD_ARP_STR " retry-interval <0-3>" ,
		CMD_ARP_STR_HELP
		"retry interval to send arp request"
		"<0-3> Seconds\n")
{
	int ret = ERROR;
	ret = nsm_ip_arp_retry_interval_set_api(atoi(argv[0]));
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_arp_retry_interval,
		no_ip_arp_retry_interval_cmd,
		"no "CMD_ARP_STR " retry-interval" ,
		NO_STR
		CMD_ARP_STR_HELP
		"retry interval to send arp request")
{
	int ret = ERROR;
	ret = nsm_ip_arp_retry_interval_set_api(0);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_arp_timeout,
		ip_arp_timeout_cmd,
		CMD_ARP_STR " timeout <1-65536>" ,
		CMD_ARP_STR_HELP
		"Arp Table TTL\n"
		SECONDS_STR)
{
	int ret = ERROR;
	ret = nsm_ip_arp_timeout_set_api(atoi(argv[0]));
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_arp_timeout,
		no_ip_arp_timeout_cmd,
		"no "CMD_ARP_STR " timeout" ,
		NO_STR
		CMD_ARP_STR_HELP
		"Arp Table TTL\n")
{
	int ret = ERROR;
	ret = nsm_ip_arp_timeout_set_api(0);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_arp_proxy,
		ip_arp_proxy_cmd,
		CMD_ARP_STR " proxy-arp enable",
		CMD_ARP_STR_HELP
		"Arp Proxy\n"
		"Enable\n")
{
	int ret = ERROR;
	ret = nsm_ip_arp_proxy_set_api(atoi(argv[0]));
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_arp_proxy,
		no_ip_arp_proxy_cmd,
		"no "CMD_ARP_STR " proxy-arp enable",
		CMD_ARP_STR_HELP
		"Arp Proxy\n"
		"Enable\n")
{
	int ret = ERROR;
	ret = nsm_ip_arp_proxy_set_api(0);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_arp_proxy_local,
		ip_arp_proxy_local_cmd,
		CMD_ARP_STR " local-proxy-arp enable",
		CMD_ARP_STR_HELP
		"Local Arp Proxy\n"
		"Enable\n")
{
	int ret = ERROR;
	ret = nsm_ip_arp_proxy_local_set_api(atoi(argv[0]));
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_arp_proxy_local,
		no_ip_arp_proxy_local_cmd,
		"no "CMD_ARP_STR " local-proxy-arp enable",
		CMD_ARP_STR_HELP
		"Local Arp Proxy\n"
		"Enable\n")
{
	int ret = ERROR;
	ret = nsm_ip_arp_proxy_local_set_api(0);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_arp_add,
		ip_arp_add_cmd,
		CMD_ARP_STR " " CMD_KEY_IPV4 " " CMD_MAC_STR,
		CMD_ARP_STR_HELP
		CMD_KEY_IPV4_HELP
		CMD_MAC_STR_HELP)
{
	int ret = ERROR;
	struct prefix address;
	unsigned char mac[8];
	struct interface *ifp;
	if (argc < 2)
		return CMD_WARNING;

	ret = str2prefix (argv[0], &address);
	if (ret <= 0)
	{
		vty_out (vty, "%% Malformed address%s", VTY_NEWLINE);
		return CMD_WARNING;
	}


	memset(mac, 0, sizeof(mac));
	VTY_IMAC_GET(argv[1], mac);

	if (argc > 2)
	{
		ifp = if_lookup_by_index(if_ifindex_make(argv[2], argv[3]));
	}
	else
	{
		ifp = if_lookup_address(address.u.prefix4);
	}
	if(ifp)
		ret = nsm_ip_arp_add_api(ifp, &address, mac);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS (ip_arp_add,
		ip_arp_add_interface_cmd,
		CMD_ARP_STR " " CMD_KEY_IPV4 " " CMD_MAC_STR " "
			CMD_INTERFACE_STR " (ethernet|gigabitethernet) "CMD_USP_STR,
		CMD_ARP_STR_HELP
		CMD_KEY_IPV4_HELP
		CMD_MAC_STR_HELP
		CMD_INTERFACE_STR_HELP
		"Ethernet interface\n"
		"GigabitEthernet interface\n"
		CMD_USP_STR_HELP);


DEFUN (no_ip_arp_add,
		no_ip_arp_add_cmd,
		"no " CMD_ARP_STR " " CMD_KEY_IPV4,
		NO_STR
		CMD_ARP_STR_HELP
		CMD_KEY_IPV4_HELP)
{
	int ret = ERROR;
	struct prefix address;
	struct interface *ifp;
	if (argc < 1)
		return CMD_WARNING;

	ret = str2prefix (argv[0], &address);
	if (ret <= 0)
	{
		vty_out (vty, "%% Malformed address%s", VTY_NEWLINE);
		return CMD_WARNING;
	}

	if (argc > 1)
	{
		ifp = if_lookup_by_index(if_ifindex_make(argv[1], argv[2]));
	}
	else
	{
		ifp = if_lookup_address(address.u.prefix4);
	}
	if(ifp)
		ret = nsm_ip_arp_del_api(ifp, &address);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS (no_ip_arp_add,
		no_ip_arp_add_interface_cmd,
		"no " CMD_ARP_STR " " CMD_KEY_IPV4 " "
			CMD_INTERFACE_STR " (ethernet|gigabitethernet) "CMD_USP_STR,
		NO_STR
		CMD_ARP_STR_HELP
		CMD_KEY_IPV4_HELP
		CMD_INTERFACE_STR_HELP
		"Ethernet interface\n"
		"GigabitEthernet interface\n"
		CMD_USP_STR_HELP);



DEFUN (clear_ip_arp_add,
		clear_ip_arp_cmd,
		"clear ip " CMD_ARP_STR,
		CLEAR_STR
		IP_STR
		CMD_ARP_STR_HELP)
{
	int ret = ERROR;
	if(argc == 0)
	{
		ret = ip_arp_cleanup_api(ARP_DYNAMIC, FALSE, 0);
	}
	else if(argc == 1)
	{
		//ret = ip_arp_cleanup_api(ARP_DYNAMIC, FALSE, 0);
		vty_out(vty, "not implementation now%s",VTY_NEWLINE);
	}
	else if(argc > 2)
	{
		if(if_lookup_by_index(if_ifindex_make(argv[1], argv[2])))
			ret = ip_arp_cleanup_api(ARP_DYNAMIC, FALSE, if_ifindex_make(argv[1], argv[2]));
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS (clear_ip_arp_add,
		clear_ip_arp_address_cmd,
		"clear ip " CMD_ARP_STR " " CMD_KEY_IPV4,
		CLEAR_STR
		IP_STR
		CMD_ARP_STR_HELP
		CMD_KEY_IPV4_HELP);

ALIAS (no_ip_arp_add,
		clear_ip_arp_interface_cmd,
		"clear ip " CMD_ARP_STR " " CMD_INTERFACE_STR " (ethernet|gigabitethernet) "CMD_USP_STR,
		CLEAR_STR
		IP_STR
		CMD_ARP_STR_HELP
		CMD_INTERFACE_STR_HELP
		"Ethernet interface\n"
		"GigabitEthernet interface\n"
		CMD_USP_STR_HELP);



DEFUN (show_ip_arp,
		show_ip_arp_cmd,
		"show ip " CMD_ARP_STR,
		SHOW_STR
		IP_STR
		CMD_ARP_STR_HELP)
{
	int ret = ERROR;
	struct interface *ifp;
	struct arp_user user;
	os_memset(&user, 0, sizeof(user));
	if(argc == 0)
		ret = show_nsm_ip_arp_config(vty,  &user);
	else
	{
		if (argc == 1)
		{
			//dynamic|static|summary
			if(os_memcmp(argv[0], "summary", 3) == 0)
				user.summary = TRUE;
			if(os_memcmp(argv[0], "static", 3) == 0)
				user.class = ARP_STATIC;
			if(os_memcmp(argv[0], "dynamic", 3) == 0)
				user.class = ARP_DYNAMIC;
			if(user.class || user.summary)
				ret = show_nsm_ip_arp_config(vty,  &user);
		}
		if (argc == 2)
		{
			ifp = if_lookup_by_index(if_ifindex_make(argv[1], argv[2]));
			if(ifp)
			{
				user.ifindex = ifp->ifindex;
				ret = show_nsm_ip_arp_config(vty,  &user);
			}
		}
	}
	if(ret != OK)
		ret = OK;
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS (show_ip_arp,
		show_ip_arp_detail_cmd,
		"show ip " CMD_ARP_STR " (dynamic|static|summary)",
		SHOW_STR
		IP_STR
		CMD_ARP_STR_HELP
		CMD_INTERFACE_STR_HELP
		"Ethernet interface\n"
		"GigabitEthernet interface\n"
		CMD_USP_STR_HELP);

ALIAS (show_ip_arp,
		show_ip_arp_interface_cmd,
		"show ip " CMD_ARP_STR " "
			CMD_INTERFACE_STR " (ethernet|gigabitethernet) "CMD_USP_STR,
		SHOW_STR
		IP_STR
		CMD_ARP_STR_HELP
		CMD_INTERFACE_STR_HELP
		"Ethernet interface\n"
		"GigabitEthernet interface\n"
		CMD_USP_STR_HELP);

static int nsm_ip_arp_table_summary(ip_arp_t *node, struct arp_user *user)
{
	if(node->class == ARP_DYNAMIC)
		user->dynamic++;
	else
		user->iStatic++;
	user->total++;
	return OK;
}

static int show_nsm_ip_arp_table_head(struct vty *vty, struct arp_user *user)
{
	vty_out(vty, "    ARP Table Information%s",VTY_NEWLINE);
	vty_out(vty, " Total	:%d%s",user->total, VTY_NEWLINE);
	vty_out(vty, " Static 	:%d%s",user->iStatic, VTY_NEWLINE);
	vty_out(vty, " Dynamic	:%d%s",user->dynamic, VTY_NEWLINE);
	vty_out(vty, "%-8s %-16s %-8s %-16s %-16s %-8s %-16s %s",
			"Protocol", "Address", "TTL/Age(min)", "Hardware Addr", "Interface", "VLAN", "VRF",VTY_NEWLINE);
	return OK;
}

static int show_nsm_ip_arp_table_one(ip_arp_t *node, struct arp_user *user)
{
	char mac[32], vlan[16], ifname[32],vrf[16],timeout[16];
	struct vty *vty = user->vty;
	os_memset(mac, 0, sizeof(mac));
	os_memset(vlan, 0, sizeof(vlan));
	os_memset(vrf, 0, sizeof(vrf));
	os_memset(timeout, 0, sizeof(timeout));
	os_memset(ifname, 0, sizeof(ifname));
/*	sprintf(mac, "%02x%02x-%02x%02x-%02x%02x",node->mac[0],node->mac[1],node->mac[2],
											 node->mac[3],node->mac[4],node->mac[5]);*/
	sprintf(mac, "%s", if_mac_out_format(node->mac));

	sprintf(ifname, "%s",ifindex2ifname(node->ifindex));
	//Protocol	Address	Age (min)	Hardware Addr    Interface
	//Internet	1.1.1.1	-	        7cb5.0157.0c00   eth-0-1

	sprintf(vlan, "%d",1);
	sprintf(vrf, "%s","default");


	if(node->class == ARP_DYNAMIC)
	{
		int age = 0;
		nsm_ip_arp_ageing_time_get_api(age);
		sprintf(timeout, "%d/%d",node->ttl, age);
	}
	else
	{
		sprintf(timeout, "%s","--/--");
	}

	vty_out(vty, "%-8s %-16s %-8s %-16s %-16s %-8s %-16s %s",
			"Internet", inet_ntoa(node->address.u.prefix4), timeout, mac, ifname, vlan, vrf, VTY_NEWLINE);
	return OK;
}

static int show_nsm_ip_arp_table_detail(ip_arp_t *node, struct arp_user *user)
{
	//static/dynamic
	if(user->class && (node->class == user->class))
	{
		return show_nsm_ip_arp_table_one(node, user);
	}
	//interface
	if(user->ifindex && (node->ifindex == user->ifindex))
	{
		return show_nsm_ip_arp_table_one(node, user);
	}
	//vrf
	if(user->vrfid && (node->vrfid == user->vrfid))
	{
		return show_nsm_ip_arp_table_one(node, user);
	}
	//
	if(user->all)
		return show_nsm_ip_arp_table_one(node, user);
	return OK;
}

static int show_nsm_ip_arp_config(struct vty *vty, struct arp_user *user)
{
	user->vty = vty;
	if(user->summary)
	{
		nsm_ip_arp_callback_api((ip_arp_cb)nsm_ip_arp_table_summary, &user);
		vty_out(vty, " %d IP ARP Entries%s",user->total, VTY_NEWLINE);
		vty_out(vty, " Static 	:%d%s",user->iStatic, VTY_NEWLINE);
		vty_out(vty, " Dynamic	:%d%s",user->dynamic, VTY_NEWLINE);
		vty_out(vty, "%s",VTY_NEWLINE);
		return OK;
	}
	nsm_ip_arp_callback_api((ip_arp_cb)nsm_ip_arp_table_summary, &user);
	if(user->total)
	{
		show_nsm_ip_arp_table_head(vty, user);
		nsm_ip_arp_callback_api((ip_arp_cb)show_nsm_ip_arp_table_detail, &user);
	}
	return OK;
}



void cmd_arp_init(void)
{
//	install_default(CONFIG_NODE);
	//reinstall_node(SERVICE_NODE, nsm_ip_arp_config);

	install_element(CONFIG_NODE, &ip_arp_add_cmd);
	install_element(CONFIG_NODE, &ip_arp_add_interface_cmd);
	install_element(CONFIG_NODE, &no_ip_arp_add_cmd);
	install_element(CONFIG_NODE, &no_ip_arp_add_interface_cmd);

	install_element(CONFIG_NODE, &clear_ip_arp_cmd);
	install_element(CONFIG_NODE, &clear_ip_arp_address_cmd);
	install_element(CONFIG_NODE, &clear_ip_arp_interface_cmd);

	install_element(CONFIG_NODE, &ip_arp_retry_interval_cmd);
	install_element(CONFIG_NODE, &no_ip_arp_retry_interval_cmd);

	install_element(CONFIG_NODE, &ip_arp_timeout_cmd);
	install_element(CONFIG_NODE, &no_ip_arp_timeout_cmd);

	install_element(CONFIG_NODE, &ip_arp_proxy_cmd);
	install_element(CONFIG_NODE, &no_ip_arp_proxy_cmd);

	install_element(CONFIG_NODE, &ip_arp_proxy_local_cmd);
	install_element(CONFIG_NODE, &no_ip_arp_proxy_local_cmd);


	install_element(ENABLE_NODE, &show_ip_arp_cmd);
	install_element(ENABLE_NODE, &show_ip_arp_detail_cmd);
	install_element(ENABLE_NODE, &show_ip_arp_interface_cmd);
	install_element(CONFIG_NODE, &show_ip_arp_cmd);
	install_element(CONFIG_NODE, &show_ip_arp_detail_cmd);
	install_element(CONFIG_NODE, &show_ip_arp_interface_cmd);
}
