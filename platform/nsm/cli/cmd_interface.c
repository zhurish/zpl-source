/*
 * cmd_if.c
 *
 *  Created on: Jan 2, 2018
 *      Author: zhurish
 */

#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"

static int nsm_interface_cmd_node_get(struct interface *ifp, int range)
{
#ifndef ZPL_KERNEL_STACK_MODULE
	int node = INTERFACE_L3_NODE;
#else
	int node = INTERFACE_NODE;
#endif
	switch (ifp->if_type)
	{
	case IF_NONE:
		break;
	case IF_LOOPBACK:
		node = LOOPBACK_INTERFACE_NODE;
		break;
	case IF_SERIAL:
		node = SERIAL_INTERFACE_NODE;
		break;
	case IF_ETHERNET:
	case IF_GIGABT_ETHERNET:
		if (ifp->if_mode == IF_MODE_L3)
			node = range ? INTERFACE_L3_RANGE_NODE : INTERFACE_L3_NODE;
		else
			node = range ? INTERFACE_RANGE_NODE : INTERFACE_NODE;
		break;
	case IF_WIRELESS:
		node = WIRELESS_INTERFACE_NODE;
		break;
	case IF_TUNNEL:
		node = TUNNEL_INTERFACE_NODE;
		break;
	case IF_LAG:
		if (ifp->if_mode == IF_MODE_L3)
			node = LAG_INTERFACE_L3_NODE;
		else
			node = LAG_INTERFACE_NODE;
		break;
	case IF_BRIGDE:
		node = BRIGDE_INTERFACE_NODE;
		break; // brigde interface
	case IF_VLAN:
		node = INTERFACE_L3_NODE;
		break;
	case IF_E1:
		node = range ? E1_INTERFACE_L3_RANGE_NODE : E1_INTERFACE_L3_NODE;
		break;
	case IF_EPON:
		if (ifp->if_mode == IF_MODE_L3)
			node = range ? EPON_INTERFACE_RANGE_NODE : EPON_INTERFACE_L3_NODE;
		else
			node = range ? EPON_INTERFACE_L3_RANGE_NODE : EPON_INTERFACE_NODE;
		break;
	case IF_GPON:
		if (ifp->if_mode == IF_MODE_L3)
			node = range ? EPON_INTERFACE_RANGE_NODE : EPON_INTERFACE_L3_NODE;
		else
			node = range ? EPON_INTERFACE_L3_RANGE_NODE : EPON_INTERFACE_NODE;
		break;
	default:
		break;
	}
	zlog_debug(MODULE_LIB, " nsm_interface_cmd_node_get :node = %d\r\n", node);
	return node;
}

DEFUN(nsm_interface_desc,
	  nsm_interface_desc_cmd,
	  "description .LINE",
	  "Interface specific description\n"
	  "Characters describing this interface\n")
{
	int ret = 0;
	char *desc;
	if (argc == 0)
		return CMD_SUCCESS;
	desc = argv_concat(argv, argc, 0);
	ret = nsm_interface_desc_set_api(vty->index, desc);
	if (desc)
		XFREE(MTYPE_TMP, desc);
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(no_nsm_interface_desc,
	  no_nsm_interface_desc_cmd,
	  "no description",
	  NO_STR
	  "Interface specific description\n")
{
	int ret = 0;
	ret = nsm_interface_desc_set_api(vty->index, NULL);
	return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(nsm_interface,
	  nsm_interface_cmd,
	  "interface " CMD_IF_USPV_STR " " CMD_USP_STR,
	  "Select an interface to configure\n" CMD_IF_USPV_STR_HELP
		  CMD_USP_STR_HELP)
{
	int ret = 0;
	struct interface *ifp;
	if (argv[0] && argv[1])
	{
		ifp = if_lookup_by_name(if_ifname_format(argv[0], argv[1]));
		if (!ifp)
		{
			if (nsm_interface_create_check_api(vty, argv[0], argv[1]) == zpl_false)
				return CMD_WARNING;
			ret = nsm_interface_create_api(if_ifname_format(argv[0], argv[1]));
			if (ret == OK)
			{
				ifp = if_lookup_by_name(if_ifname_format(argv[0], argv[1]));
				if (ifp)
				{
					vty->index = ifp;
					vty->node = nsm_interface_cmd_node_get(ifp, 0);
					return CMD_SUCCESS;
				}
			}
		}
		else
		{
			vty->index = ifp;
			vty->node = nsm_interface_cmd_node_get(ifp, 0);
			return CMD_SUCCESS;
		}
	}
	return CMD_WARNING;
}
ALIAS(nsm_interface,
	  nsm_interface_sub_cmd,
	  "interface " CMD_IF_SUB_USPV_STR " " CMD_USP_SUB_STR,
	  "Select an interface to configure\n" CMD_IF_SUB_USPV_STR_HELP
		  CMD_USP_SUB_STR_HELP)

static int nsm_interface_other_create(int argc, char *name, char *uspv, struct vty *vty)
{
	int ret = 0;
	struct interface *ifp;
	if (name && uspv)
	{
		ifp = if_lookup_by_name(if_ifname_format(name, uspv));
		if (!ifp)
		{
			if (nsm_interface_create_check_api(vty, name, uspv) == zpl_false)
				return CMD_WARNING;
			ret = nsm_interface_create_api(if_ifname_format(name, uspv));
			if (ret == OK)
			{
				ifp = if_lookup_by_name(if_ifname_format(name, uspv));
				if (ifp)
				{
					vty->index = ifp;
					vty->node = nsm_interface_cmd_node_get(ifp, 0);
					if(if_is_lag(ifp))
					{
						vty->index_value = IF_IFINDEX_ID_GET(ifp->ifindex);
					}
					return CMD_SUCCESS;
				}
			}
		}
		else
		{
			vty->index = ifp;
			vty->node = nsm_interface_cmd_node_get(ifp, 0);
			if(if_is_lag(ifp))
			{
				vty->index_value = IF_IFINDEX_ID_GET(ifp->ifindex);
			}
			return CMD_SUCCESS;
		}
	}
	return CMD_WARNING;
}

static int nsm_interface_other_destory(int argc, char *name, char *uspv, struct vty *vty)
{
	int ret = 0;
	struct interface *ifp;
	if (name && uspv)
	{
		ifp = if_lookup_by_name(if_ifname_format(name, uspv));
		if (ifp)
		{
			ret = nsm_interface_delete_api(ifp);
			return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
		}
	}
	return CMD_WARNING;
}

DEFUN(nsm_interface_loopback,
	  nsm_interface_loopback_cmd,
	  "interface " CMD_IF_LOOP_STR,
	  "Select an interface to configure\n" CMD_IF_LOOP_STR_HELP)
{
	return nsm_interface_other_create(2, "loopback", argv[0], vty);
}

DEFUN(no_nsm_interface_loopback,
	  no_nsm_interface_loopback_cmd,
	  "no interface " CMD_IF_LOOP_STR,
	  NO_STR
	  "Select an interface to configure\n" CMD_IF_LOOP_STR_HELP)
{
	return nsm_interface_other_destory(2, "loopback", argv[0], vty);
}
#ifdef ZPL_NSM_VLAN	
DEFUN(nsm_interface_vlan,
	  nsm_interface_vlan_cmd,
	  "interface " CMD_IF_VLAN_STR,
	  "Select an interface to configure\n" CMD_IF_VLAN_STR_HELP)
{
	return nsm_interface_other_create(2, "vlan", argv[0], vty);
}

DEFUN(no_nsm_interface_vlan,
	  no_nsm_interface_vlan_cmd,
	  "no interface " CMD_IF_VLAN_STR,
	  NO_STR
	  "Select an interface to configure\n" CMD_IF_VLAN_STR_HELP)
{
	return nsm_interface_other_destory(2, "loopback", argv[0], vty);
}
#endif
#ifdef ZPL_NSM_TRUNK
DEFUN(nsm_interface_trunk,
	  nsm_interface_trunk_cmd,
	  "interface " CMD_IF_TRUNK_STR,
	  "Select an interface to configure\n" CMD_IF_TRUNK_STR_HELP)
{
	return nsm_interface_other_create(2, "port-channel", argv[0], vty);
}

DEFUN(no_nsm_interface_trunk,
	  no_nsm_interface_trunk_cmd,
	  "no interface " CMD_IF_TRUNK_STR,
	  NO_STR
	  "Select an interface to configure\n" CMD_IF_TRUNK_STR_HELP)
{
	return nsm_interface_other_destory(2, "loopback", argv[0], vty);
}
#endif

DEFUN(no_nsm_interface,
	  no_nsm_interface_cmd,
	  "no interface " CMD_IF_USPV_STR " " CMD_USP_STR,
	  NO_STR
	  "Select an interface to configure\n" CMD_IF_USPV_STR_HELP
		  CMD_USP_STR_HELP)
{
	int ret = 0;
	struct interface *ifp;
	if (argv[0] && argv[1])
	{
		ifp = if_lookup_by_name(if_ifname_format(argv[0], argv[1]));
		if (ifp)
		{
			ret = nsm_interface_delete_api(ifp);
			return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
		}
	}
	return CMD_WARNING;
}

DEFUN_HIDDEN(hidden_nsm_interface,
			 hidden_nsm_interface_cmd,
			 "interface IFNAME",
			 "Select an interface to configure\n"
			 "Interface's name\n")
{
	struct interface *ifp = NULL;
	if (argv[0])
	{
		ifp = if_lookup_by_name(argv[0]);
		if (!ifp)
		{
			if (nsm_interface_create_check_api(vty, argv[0], NULL) == zpl_false)
				return CMD_WARNING;
			if (nsm_interface_create_api(argv[0]) != OK)
			{
				vty_out(vty, "can not create interface %s%s", argv[0], VTY_NEWLINE);
				return CMD_WARNING;
			}
			ifp = if_lookup_by_name(argv[0]);
			if (ifp)
			{
				vty->index = ifp;
				vty->node = nsm_interface_cmd_node_get(ifp, 0);
				if(if_is_lag(ifp))
				{
					vty->index_value = IF_IFINDEX_ID_GET(ifp->ifindex);
				}
				return CMD_SUCCESS;
			}
		}
		else
		{
			vty->index = ifp;
			vty->node = nsm_interface_cmd_node_get(ifp, 0);
			if(if_is_lag(ifp))
			{
				vty->index_value = IF_IFINDEX_ID_GET(ifp->ifindex);
			}
			return CMD_SUCCESS;
		}
	}
	return CMD_WARNING;
}

DEFUN_HIDDEN(hidden_no_nsm_interface,
			 hidden_no_nsm_interface_cmd,
			 "no interface IFNAME",
			 NO_STR
			 "Delete a pseudo interface's configuration\n"
			 "Interface's name\n")
{
	int ret = 0;
	struct interface *ifp = NULL;
	if (argv[0])
	{
		ifp = if_lookup_by_name(argv[0]);
		if (ifp)
		{
			ret = nsm_interface_delete_api(ifp);
			return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
		}
	}
	return CMD_WARNING;
}

DEFUN_HIDDEN(nsm_interface_range,
			 nsm_interface_range_cmd,
			 "interface " CMD_IF_SUB_USPV_STR " " CMD_USP_RANGE_STR,
			 "Select an interface to configure\n" CMD_IF_SUB_USPV_STR_HELP
				 CMD_USP_RANGE_STR_HELP)
{
	struct interface *ifp = NULL;
	zpl_uint32 uspv = 0, endport = 0, i = 0;
	ifindex_t ifindex = 0, port_index = 0, port = 0;
	if_type_t if_type = IF_NONE;
	if_mode_t if_mode = IF_MODE_NONE;
	VTY_IUSP_GET(argv[1], uspv, endport);
	if (argv[0] && argv[1])
	{
		ifindex = IF_IFINDEX_SET(name2type(argv[0]), uspv);
		port_index = IF_PORT_GET(ifindex);
		if (endport <= port_index)
		{
			return CMD_WARNING;
		}
		for (; port_index < endport; port_index++)
		{
			port = IF_USPV_SET(IF_TYPE_GET(ifindex), IF_UNIT_GET(ifindex), IF_SLOT_GET(ifindex), port_index) | IF_ID_GET(ifindex);
			ifp = if_lookup_by_index(port);
			if (!ifp)
			{
				return CMD_WARNING;
			}
			if (if_type == IF_NONE)
				if_type = ifp->if_type;
			if (if_mode == IF_MODE_NONE)
				if_mode = ifp->if_mode;

			if (if_mode == ifp->if_mode && if_type == ifp->if_type)
				vty->vty_range_index[i++] = ifp;
			else
			{
				return CMD_WARNING;
			}
		}
		port_index = IF_PORT_GET(ifindex);
		vty->index_range = endport - port_index;
		ifp = if_lookup_by_index(ifindex);
		vty->node = nsm_interface_cmd_node_get(ifp, 1);
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN_HIDDEN(nsm_interface_sub_range,
			 nsm_interface_range_sub_cmd,
			 "interface " CMD_IF_SUB_USPV_STR " " CMD_USP_SUB_RANGE_STR,
			 "Select an interface to configure\n" CMD_IF_SUB_USPV_STR_HELP
				 CMD_USP_SUB_RANGE_STR_HELP)
{
	struct interface *ifp = NULL;
	zpl_uint32 uspv = 0, endport = 0, i = 0;
	ifindex_t ifindex = 0, port_index = 0, port = 0;
	if_type_t if_type = IF_NONE;
	if_mode_t if_mode = IF_MODE_NONE;
	VTY_IUSP_GET(argv[1], uspv, endport);
	if (argv[0] && argv[1])
	{
		ifindex = IF_IFINDEX_SET(name2type(argv[0]), uspv);
		port_index = IF_PORT_GET(ifindex);
		if (endport <= port_index)
		{
			return CMD_WARNING;
		}
		for (; port_index < endport; port_index++)
		{
			port = IF_USPV_SET(IF_TYPE_GET(ifindex), IF_UNIT_GET(ifindex), IF_SLOT_GET(ifindex), port_index) | IF_ID_GET(ifindex);
			ifp = if_lookup_by_index(port);
			if (!ifp)
			{
				return CMD_WARNING;
			}
			if (if_type == IF_NONE)
				if_type = ifp->if_type;
			if (if_mode == IF_MODE_NONE)
				if_mode = ifp->if_mode;

			if (if_mode == ifp->if_mode && if_type == ifp->if_type)
				vty->vty_range_index[i++] = ifp;
			else
			{
				return CMD_WARNING;
			}
		}
		port_index = IF_PORT_GET(ifindex);
		vty->index_range = endport - port_index;
		ifp = if_lookup_by_index(ifindex);
		vty->node = nsm_interface_cmd_node_get(ifp, 1);
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}
#ifdef CUSTOM_INTERFACE
DEFUN(nsm_wifi_interface,
	  nsm_wifi_interface_cmd,
	  "interface " CMD_IF_MIP_STR " " CMD_USP_STR,
	  "Select an interface to configure\n" CMD_IF_MIP_STR_HELP
		  CMD_USP_STR_HELP)
{
	int ret = 0;
	struct interface *ifp = NULL;
	if (argv[0] && argv[1])
	{
		ifp = if_lookup_by_name(if_ifname_format(argv[0], argv[1]));
		if (!ifp)
		{
			if (nsm_interface_create_check_api(vty, argv[0], argv[1]) == zpl_false)
				return CMD_WARNING;
			ret = nsm_interface_create_api(if_ifname_format(argv[0], argv[1]));
			if (ret == OK)
			{
				ifp = if_lookup_by_name(if_ifname_format(argv[0], argv[1]));
				if (ifp)
				{
					vty->index = ifp;
					vty->node = nsm_interface_cmd_node_get(ifp, 0);
					return CMD_SUCCESS;
				}
			}
		}
		else
		{
			vty->index = ifp;
			vty->node = nsm_interface_cmd_node_get(ifp, 0);
			return CMD_SUCCESS;
		}
	}
	return CMD_WARNING;
}

DEFUN(no_nsm_wifi_interface,
	  no_nsm_wifi_interface_cmd,
	  "no interface " CMD_IF_MIP_STR " " CMD_USP_STR,
	  NO_STR
	  "Select an interface to configure\n" CMD_IF_MIP_STR_HELP
		  CMD_USP_STR_HELP)
{
	int ret = 0;
	struct interface *ifp;
	if (argv[0] && argv[1])
	{
		ifp = if_lookup_by_name(if_ifname_format(argv[0], argv[1]));
		if (!ifp)
		{
			ret = nsm_interface_delete_api(ifp);
			return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
		}
	}
	return CMD_WARNING;
}
#endif

DEFUN(nsm_interface_shutdown,
	  nsm_interface_shutdown_cmd,
	  "shutdown",
	  "Shutdown the selected interface\n")
{
	int ret = 0;
	struct interface *ifp = vty->index;
	if (ifp)
	{
		ret = nsm_interface_down_set_api(ifp);
		return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
	}
	else if (vty->index_range)
	{
		zpl_uint32 i = 0;
		for (i = 0; i < vty->index_range; i++)
		{
			if (vty->vty_range_index[i])
			{
				ret = nsm_interface_down_set_api(vty->vty_range_index[i]);
				if (ret != OK)
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN(no_nsm_interface_shutdown,
	  no_nsm_interface_shutdown_cmd,
	  "no shutdown",
	  NO_STR
	  "Shutdown the selected interface\n")
{
	int ret = 0;
	struct interface *ifp = vty->index;
	if (ifp)
	{
		ret = nsm_interface_up_set_api(ifp);
		return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
	}
	else if (vty->index_range)
	{
		zpl_uint32 i = 0;
		for (i = 0; i < vty->index_range; i++)
		{
			if (vty->vty_range_index[i])
			{
				ret = nsm_interface_up_set_api(vty->vty_range_index[i]);
				if (ret != OK)
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}


DEFUN(nsm_interface_bandwidth,
	  nsm_interface_bandwidth_cmd,
	  "bandwidth <1-10000000>",
	  "Set bandwidth informational parameter\n"
	  "Bandwidth in kilobits\n")
{
	int ret = 0;
	zpl_uint32 bandwidth;
	struct interface *ifp = (struct interface *)vty->index;
	bandwidth = strtol(argv[0], NULL, 10);

	/* bandwidth range is <1-10000000> */
	if (bandwidth < 1 || bandwidth > 10000000)
	{
		vty_out(vty, "Bandwidth is invalid%s", VTY_NEWLINE);
		return CMD_WARNING;
	}

	if (ifp)
	{
		ret = nsm_interface_bandwidth_set_api(ifp, bandwidth);
		return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
	}
	else if (vty->index_range)
	{
		zpl_uint32 i = 0;
		for (i = 0; i < vty->index_range; i++)
		{
			if (vty->vty_range_index[i])
			{
				ret = nsm_interface_bandwidth_set_api(vty->vty_range_index[i], bandwidth);
				if (ret != OK)
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN(no_nsm_interface_bandwidth,
	  no_nsm_interface_bandwidth_cmd,
	  "no bandwidth",
	  NO_STR
	  "Set bandwidth informational parameter\n")
{
	int ret = 0;
	struct interface *ifp = (struct interface *)vty->index;

	if (ifp)
	{
		ret = nsm_interface_bandwidth_set_api(ifp, 0);
		return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
	}
	else if (vty->index_range)
	{
		zpl_uint32 i = 0;
		for (i = 0; i < vty->index_range; i++)
		{
			if (vty->vty_range_index[i])
			{
				ret = nsm_interface_bandwidth_set_api(vty->vty_range_index[i], 0);
				if (ret != OK)
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN(nsm_interface_mtu,
	  nsm_interface_mtu_cmd,
	  "mtu <64-9600>",
	  "Set Interface mtu informational parameter\n"
	  "Mtu in interface\n")
{
	int ret = 0;
	zpl_uint32 mtu;
	struct interface *ifp = (struct interface *)vty->index;
	mtu = strtol(argv[0], NULL, 10);

	/* bandwidth range is <1-10000000> */
	if (mtu < 1 || mtu > 9600)
	{
		vty_out(vty, "mtu is invalid%s", VTY_NEWLINE);
		return CMD_WARNING;
	}

	if (ifp)
	{
		ret = nsm_interface_mtu_set_api(ifp, mtu);
		return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
	}
	else if (vty->index_range)
	{
		zpl_uint32 i = 0;
		for (i = 0; i < vty->index_range; i++)
		{
			if (vty->vty_range_index[i])
			{
				ret = nsm_interface_mtu_set_api(vty->vty_range_index[i], mtu);
				if (ret != OK)
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN(no_nsm_interface_mtu,
	  no_nsm_interface_mtu_cmd,
	  "no mtu",
	  NO_STR
	  "Set Interface mtu informational parameter\n")
{
	int ret = 0;
	struct interface *ifp = (struct interface *)vty->index;

	if (ifp)
	{
		ret = nsm_interface_mtu_set_api(ifp, IF_ZEBRA_MTU_DEFAULT);
		return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
	}
	else if (vty->index_range)
	{
		zpl_uint32 i = 0;
		for (i = 0; i < vty->index_range; i++)
		{
			if (vty->vty_range_index[i])
			{
				ret = nsm_interface_mtu_set_api(vty->vty_range_index[i], IF_ZEBRA_MTU_DEFAULT);
				if (ret != OK)
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN(nsm_interface_metric,
	  nsm_interface_metric_cmd,
	  "metric <1-1000>",
	  "Set Interface mtu informational parameter\n"
	  "metric in Interface\n")
{
	int ret = 0;
	zpl_uint32 metric;
	struct interface *ifp = (struct interface *)vty->index;
	metric = strtol(argv[0], NULL, 10);

	/* bandwidth range is <1-10000000> */
	if (metric < 1 || metric > 100)
	{
		vty_out(vty, "metric is invalid%s", VTY_NEWLINE);
		return CMD_WARNING;
	}

	if (ifp)
	{
		ret = nsm_interface_metric_set_api(ifp, metric);
		return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
	}
	else if (vty->index_range)
	{
		zpl_uint32 i = 0;
		for (i = 0; i < vty->index_range; i++)
		{
			if (vty->vty_range_index[i])
			{
				ret = nsm_interface_metric_set_api(vty->vty_range_index[i], metric);
				if (ret != OK)
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN(no_nsm_interface_metric,
	  no_nsm_interface_metric_cmd,
	  "no metric",
	  NO_STR
	  "Set Interface Metric informational parameter\n")
{
	int ret = 0;
	struct interface *ifp = (struct interface *)vty->index;

	if (ifp)
	{
		ret = nsm_interface_metric_set_api(ifp, 1);
		return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
	}
	else if (vty->index_range)
	{
		zpl_uint32 i = 0;
		for (i = 0; i < vty->index_range; i++)
		{
			if (vty->vty_range_index[i])
			{
				ret = nsm_interface_metric_set_api(vty->vty_range_index[i], 1);
				if (ret != OK)
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN(nsm_interface_duplex,
	  nsm_interface_duplex_cmd,
	  "duplex (auto|full|half)",
	  "Set Interface duplex informational parameter\n"
	  "Set Interface auto duplex\n"
	  "Set Interface full duplex\n"
	  "Set Interface halt duplex\n")
{
	int ret = 0;
	nsm_duplex_en duplex = 0;
	struct interface *ifp = (struct interface *)vty->index;
	if (strncmp(argv[0], "auto", 2) == 0)
		duplex = NSM_IF_DUPLEX_AUTO;
	else if (strncmp(argv[0], "full", 2) == 0)
		duplex = NSM_IF_DUPLEX_FULL;
	else if (strncmp(argv[0], "half", 2) == 0)
		duplex = NSM_IF_DUPLEX_HALF;
	if (ifp)
	{
		ret = nsm_interface_duplex_set_api(ifp, duplex);
		return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
	}
	else if (vty->index_range)
	{
		zpl_uint32 i = 0;
		for (i = 0; i < vty->index_range; i++)
		{
			if (vty->vty_range_index[i])
			{
				ret = nsm_interface_duplex_set_api(vty->vty_range_index[i], duplex);
				if (ret != OK)
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN(no_nsm_interface_duplex,
	  no_nsm_interface_duplex_cmd,
	  "no duplex",
	  NO_STR
	  "Set Interface duplex informational parameter\n")
{
	int ret = 0;
	struct interface *ifp = (struct interface *)vty->index;

	if (ifp)
	{
		ret = nsm_interface_duplex_set_api(ifp, NSM_IF_DUPLEX_AUTO);
		return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
	}
	else if (vty->index_range)
	{
		zpl_uint32 i = 0;
		for (i = 0; i < vty->index_range; i++)
		{
			if (vty->vty_range_index[i])
			{
				ret = nsm_interface_duplex_set_api(vty->vty_range_index[i], NSM_IF_DUPLEX_AUTO);
				if (ret != OK)
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN(nsm_interface_mac,
	  nsm_interface_mac_cmd,
	  "mac-address " CMD_MAC_STR,
	  "Set Interface mac-address informational parameter\n" CMD_MAC_STR_HELP)
{
	int ret = 0;
	zpl_uint8 mac[6];
	struct interface *ifp = (struct interface *)vty->index;
	VTY_IMAC_GET(argv[0], mac);
	if (ifp)
	{
		ret = nsm_interface_mac_set_api(ifp, mac, 6);
		return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
	}
	return CMD_WARNING;
}

DEFUN(no_nsm_interface_mac,
	  no_nsm_interface_mac_cmd,
	  "no mac-address",
	  NO_STR
	  "Set Interface mac-address informational parameter\n")
{
	int ret = 0;
	struct interface *ifp = (struct interface *)vty->index;

	if (ifp)
	{
		ret = nsm_interface_mac_set_api(ifp, NULL, 0);
		return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
	}
	return CMD_WARNING;
}

DEFUN(nsm_interface_speed,
	  nsm_interface_speed_cmd,
	  "speed (10|100|1000|10000|auto)",
	  "Set Interface encapsulation informational parameter\n"
	  "Set Interface auto duplex\n"
	  "Set Interface full duplex\n"
	  "Set Interface halt duplex\n")
{
	int ret = 0;
	nsm_speed_en speed = 0;
	struct interface *ifp = (struct interface *)vty->index;
	if (strncmp(argv[0], "auto", 2) == 0)
		speed = NSM_IF_SPEED_AUTO;
	else // if(strcmp(argv[0], "full", 2) == 0)
	{
		ret = atoi(argv[0]);
		switch (ret)
		{
		case 10:
			speed = NSM_IF_SPEED_10M;
			break;
		case 100:
			speed = NSM_IF_SPEED_100M;
			break;
		case 1000:
			speed = NSM_IF_SPEED_1000M;
			break;
		case 10000:
			speed = NSM_IF_SPEED_10000M;
			break;
		}
	}
	if (ifp)
	{
		ret = nsm_interface_speed_set_api(ifp, speed);
		return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
	}
	else if (vty->index_range)
	{
		zpl_uint32 i = 0;
		for (i = 0; i < vty->index_range; i++)
		{
			if (vty->vty_range_index[i])
			{
				ret = nsm_interface_speed_set_api(vty->vty_range_index[i], speed);
				if (ret != OK)
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN(no_nsm_interface_speed,
	  no_nsm_interface_speed_cmd,
	  "no speed",
	  NO_STR
	  "Set Interface duplex informational parameter\n")
{
	int ret = 0;
	struct interface *ifp = (struct interface *)vty->index;

	if (ifp)
	{
		ret = nsm_interface_speed_set_api(ifp, NSM_IF_DUPLEX_AUTO);
		return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
	}
	else if (vty->index_range)
	{
		zpl_uint32 i = 0;
		for (i = 0; i < vty->index_range; i++)
		{
			if (vty->vty_range_index[i])
			{
				ret = nsm_interface_speed_set_api(vty->vty_range_index[i], NSM_IF_DUPLEX_AUTO);
				if (ret != OK)
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

#ifdef ZPL_IPCOM_STACK_MODULE
DEFUN(nsm_interface_ip_vrf,
	  nsm_interface_ip_vrf_cmd,
	  "ip forward vrf NAME",
	  IP_STR
	  "forward\n"
	  "vrf\n"
	  "vrf id\n")
{
	int ret = 0;
	vrf_id_t id = 0;
	;
	struct interface *ifp = (struct interface *)vty->index;
	id = vrf_name2vrfid(argv[0]);
	if (ifp && id)
	{
		ret = nsm_interface_vrf_set_api(ifp, id);
		return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
	}
	else if (vty->index_range)
	{
		zpl_uint32 i = 0;
		for (i = 0; i < vty->index_range; i++)
		{
			if (vty->vty_range_index[i])
			{
				ret = nsm_interface_vrf_set_api(vty->vty_range_index[i], id);
				if (ret != OK)
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN(no_nsm_interface_ip_vrf,
	  no_nsm_interface_ip_vrf_cmd,
	  "no ip forward vrf",
	  NO_STR
		  IP_STR
	  "forward\n"
	  "vrf\n")
{
	int ret = 0;
	struct interface *ifp = (struct interface *)vty->index;

	if (ifp)
	{
		ret = nsm_interface_vrf_set_api(ifp, VRF_DEFAULT);
		return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
	}
	else if (vty->index_range)
	{
		zpl_uint32 i = 0;
		for (i = 0; i < vty->index_range; i++)
		{
			if (vty->vty_range_index[i])
			{
				ret = nsm_interface_vrf_set_api(vty->vty_range_index[i], VRF_DEFAULT);
				if (ret != OK)
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}
#endif

DEFUN(nsm_interface_enca_dot1q,
	  nsm_interface_enca_dot1q_cmd,
	  "encapsulation dot1q <2-4094>",
	  "set this interface encapsulation type\n"
	  "encapsulation dot1q VLAN\n"
	  "VLAN ID Value\n")
{
	if_enca_t enca = IF_ENCA_NONE;
	int value = 0;
	int ret = 0;
	struct interface *ifp = vty->index;
	if (ifp)
	{
		if (ifp->if_type != IF_ETHERNET && ifp->if_type != IF_GIGABT_ETHERNET)
		{
			vty_out(vty, "%% This is not ethernet interface,Can not set encapsulation info %s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		if (ifp->if_type == IF_ETHERNET || ifp->if_type == IF_GIGABT_ETHERNET)
		{
			if (!IF_IFINDEX_ID_GET(ifp->ifindex))
			{
				vty_out(vty, "%% This is not sub interface,Can not set encapsulation info %s", VTY_NEWLINE);
				return CMD_WARNING;
			}
			enca = IF_ENCA_DOT1Q;
			value = atoi(argv[0]);
			if (enca && value)
			{
				ret = nsm_interface_enca_set_api(ifp, enca, value);
				return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
			}
		}
	}
	return CMD_WARNING;
}

DEFUN(nsm_interface_enca_ppp,
	  nsm_interface_enca_ppp_cmd,
	  "encapsulation (ppp|slip|raw)",
	  "set this interface encapsulation type\n"
	  "encapsulation PPP frame\n"
	  "encapsulation slip frame\n"
	  "encapsulation raw frame，transparent transmission mode\n")
{
	if_enca_t enca = IF_ENCA_NONE;
	int value = 0;
	int ret = 0;
	struct interface *ifp = vty->index;
	if (ifp)
	{
		if (ifp->if_type != IF_SERIAL
#ifdef CUSTOM_INTERFACE
			&& ifp->if_type != IF_MODEM
#endif
		)
		{
			vty_out(vty, "%% This is not serial interface,Can not set encapsulation info %s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		if (ifp->if_type == IF_SERIAL
#ifdef CUSTOM_INTERFACE
			|| ifp->if_type == IF_MODEM
#endif
		)
		{
			if (memcmp(argv[0], "ppp", 2) == 0)
				enca = IF_ENCA_PPP;
			else if (memcmp(argv[0], "slip", 2) == 0)
				enca = IF_ENCA_SLIP;
			else if (memcmp(argv[0], "raw", 2) == 0)
				enca = IF_ENCA_RAW;
			if (enca && value)
			{
				ret = nsm_interface_enca_set_api(ifp, enca, value);
				return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
			}
		}
	}
	return CMD_WARNING;
}

DEFUN(nsm_interface_enca_pppoe,
	  nsm_interface_enca_pppoe_cmd,
	  "encapsulation pppoe",
	  "set this interface encapsulation type\n"
	  "encapsulation pppoe\n")
{
	if_enca_t enca = IF_ENCA_NONE;
	int value = 0;
	int ret = 0;
	struct interface *ifp = vty->index;
	if (ifp)
	{
		if (ifp->if_type != IF_ETHERNET && ifp->if_type != IF_GIGABT_ETHERNET
#ifdef CUSTOM_INTERFACE
			&& ifp->if_type != IF_MODEM
#endif
		)
		{
			vty_out(vty, "%% This is not ethernet interface,Can not set encapsulation info %s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		if (ifp->if_type == IF_ETHERNET || ifp->if_type == IF_GIGABT_ETHERNET)
		{
			enca = IF_ENCA_PPPOE;
			if (enca && value)
			{
				ret = nsm_interface_enca_set_api(ifp, enca, value);
				return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
			}
		}
	}
	return CMD_WARNING;
}

DEFUN(no_nsm_interface_enca,
	  no_nsm_interface_enca_cmd,
	  "no encapsulation",
	  NO_STR
	  "set this interface encapsulation type\n")
{
	int ret = 0;
	struct interface *ifp = vty->index;
	if (ifp)
	{
		// if(ifp->if_type == IF_ETHERNET || ifp->if_type == IF_GIGABT_ETHERNET)
		{
			ret = nsm_interface_enca_set_api(ifp, IF_ENCA_NONE, 0);
			return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
		}
		// else
		//	return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN(nsm_interface_ip_address,
	  nsm_interface_ip_address_cmd,
	  "ip address A.B.C.D/M",
	  "Interface Internet Protocol config commands\n"
	  "Set the IP address of an interface\n"
	  "IP address (e.g. 10.0.0.1/8)\n")
{
	int ret = 0;
	struct prefix cp;
	if_mode_t mode = IF_MODE_NONE;
	struct interface *ifp = (struct interface *)vty->index;

	if (ifp)
	{
		if (nsm_interface_mode_get_api(ifp, &mode) != OK)
		{
			vty_out(vty, "%% Can not get interface mode %s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		if (mode != IF_MODE_L3)
		{
			vty_out(vty, "%% this interface not L3 mode %s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		ret = str2prefix_ipv4(argv[0], (struct prefix_ipv4 *)&cp);
		if (ret <= 0)
		{
			vty_out(vty, "%% Malformed address %s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		ret = nsm_interface_address_set_api(ifp, &cp, zpl_false);
		if (ret == ERROR)
			vty_out(vty, "%% Can't set interface IP address.%s", VTY_NEWLINE);
		return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
	}
	return CMD_WARNING;
}

DEFUN(no_nsm_interface_ip_address,
	  no_nsm_interface_ip_address_cmd,
	  "no ip address A.B.C.D/M",
	  NO_STR
	  "Interface Internet Protocol config commands\n"
	  "Set the IP address of an interface\n"
	  "IP Address (e.g. 10.0.0.1/8)")
{
	int ret = 0;
	if_mode_t mode = IF_MODE_NONE;
	struct prefix cp;
	struct interface *ifp = (struct interface *)vty->index;

	if (ifp)
	{
		if (nsm_interface_mode_get_api(ifp, &mode) != OK)
		{
			vty_out(vty, "%% Can not get interface mode %s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		if (mode != IF_MODE_L3)
		{
			vty_out(vty, "%% this interface not L3 mode %s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		ret = str2prefix_ipv4(argv[0], (struct prefix_ipv4 *)&cp);
		if (ret <= 0)
		{
			vty_out(vty, "%% Malformed address %s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		ret = nsm_interface_address_unset_api(ifp, &cp, zpl_false);
		if (ret == ERROR)
			vty_out(vty, "%% Can't unset interface IP address.%s", VTY_NEWLINE);
		return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
	}
	return CMD_WARNING;
}

#ifdef HAVE_IPV6
DEFUN(nsm_interface_ipv6_address,
	  nsm_interface_ipv6_address_cmd,
	  "ipv6 address X:X::X:X/M",
	  "Interface IPv6 config commands\n"
	  "Set the IP address of an interface\n"
	  "IPv6 address (e.g. 3ffe:506::1/48)\n")
{
	int ret = 0;
	struct prefix cp;
	if_mode_t mode = IF_MODE_NONE;
	struct interface *ifp = (struct interface *)vty->index;

	if (ifp)
	{
		if (nsm_interface_mode_get_api(ifp, &mode) != OK)
		{
			vty_out(vty, "%% Can not get interface mode %s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		if (mode != IF_MODE_L3)
		{
			vty_out(vty, "%% this interface not L3 mode %s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		ret = str2prefix_ipv6(argv[0], (struct prefix_ipv6 *)&cp);
		if (ret <= 0)
		{
			vty_out(vty, "%% Malformed address %s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		ret = nsm_interface_address_set_api(ifp, &cp, zpl_false);
		if (ret == ERROR)
			vty_out(vty, "%% Can't set interface IP address.%s", VTY_NEWLINE);
		return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
	}
	return CMD_WARNING;
}

DEFUN(no_nsm_interface_ipv6_address,
	  no_nsm_interface_ipv6_address_cmd,
	  "no ipv6 address X:X::X:X/M",
	  NO_STR
	  "Interface IPv6 config commands\n"
	  "Set the IP address of an interface\n"
	  "IPv6 address (e.g. 3ffe:506::1/48)\n")
{
	int ret = 0;
	struct prefix cp;
	if_mode_t mode = IF_MODE_NONE;
	struct interface *ifp = (struct interface *)vty->index;

	if (ifp)
	{
		if (nsm_interface_mode_get_api(ifp, &mode) != OK)
		{
			vty_out(vty, "%% Can not get interface mode %s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		if (mode != IF_MODE_L3)
		{
			vty_out(vty, "%% this interface not L3 mode %s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		ret = str2prefix_ipv6(argv[0], (struct prefix_ipv6 *)&cp);
		if (ret <= 0)
		{
			vty_out(vty, "%% Malformed address %s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		ret = nsm_interface_address_unset_api(ifp, &cp, zpl_false);
		if (ret == ERROR)
			vty_out(vty, "%% Can't unset interface IP address.%s", VTY_NEWLINE);
		return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
	}
	return CMD_WARNING;
}
#endif /* HAVE_IPV6 */

#ifdef ZPL_KERNEL_STACK_MODULE
DEFUN_HIDDEN(nsm_interface_set_kernel,
			 nsm_interface_set_kernel_cmd,
			 "set-kernel IFNAME",
			 "Set the kernel of an interface\n"
			 "kernel interface\n")
{
	int ret = 0;
	struct interface *ifp = (struct interface *)vty->index;

	if (ifp)
	{
		/*
		if_kname_set(ifp, argv[0]);
		SET_FLAG(ifp->status, ZEBRA_INTERFACE_ATTACH);

		pal_interface_update_flag(ifp);
		ifp->k_ifindex = pal_interface_ifindex(ifp->k_name);
		pal_interface_get_lladdr(ifp);*/
		ret = nsm_interface_update_kernel(ifp, argv[0]);
		SET_FLAG(ifp->status, ZEBRA_INTERFACE_ATTACH);

		return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
	}
	return CMD_WARNING;
}
DEFUN_HIDDEN(no_nsm_interface_set_kernel,
			 no_nsm_interface_set_kernel_cmd,
			 "no set-kernel",
			 "Set the kernel of an interface\n")
{
	int ret = 0;
	struct interface *ifp = (struct interface *)vty->index;

	if (ifp)
	{

		if_kname_set(ifp, NULL);
		UNSET_FLAG(ifp->status, ZEBRA_INTERFACE_ATTACH);
		ifp->k_ifindex = 0;
		return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
	}
	return CMD_WARNING;
}

DEFUN_HIDDEN(show_interface_kernel,
			 show_interface_kernel_cmd,
			 "show interface kernel",
			 SHOW_STR
			 "Interface status and configuration\n"
			 "Kernel Interface\n")
{
	struct listnode *node = NULL;
	struct interface *ifp = NULL;
	struct list *if_list = NULL;
	vty_out(vty, " %-20s %-16s %s", "--------------------", "----------------", VTY_NEWLINE);
	vty_out(vty, " %-20s %-16s %s", "Interface", "kernel", VTY_NEWLINE);
	vty_out(vty, " %-20s %-16s %s", "--------------------", "----------------", VTY_NEWLINE);
	if (argc == 0)
	{
		if_list = if_list_get();
		if (if_list)
		{
			for (ALL_LIST_ELEMENTS_RO(if_list, node, ifp))
			{
				if (ifp)
				{
					vty_out(vty, " %-20s %-16s->%d %s", ifp->name, ifp->k_name, ifp->k_ifindex, VTY_NEWLINE);
				}
			}
		}
	}
	vty_out(vty, " %-20s %-16s %s", "--------------------", "----------------", VTY_NEWLINE);
	// if_slot_show_port_phy(vty);
	return CMD_SUCCESS;
}
#endif

/* Show all interfaces to vty. */
static int _show_interface_info(int argc, char *name, char *uspv, struct vty *vty)
{
	struct listnode *node;
	struct interface *ifp;
	struct list *if_list = NULL;
	IF_DATA_LOCK();
	if (argc == 0)
	{
		if_list = if_list_get();
		if (if_list)
		{
			for (ALL_LIST_ELEMENTS_RO(if_list, node, ifp))
			{
				if (ifp)
					nsm_interface_show_api(vty, ifp);
			}
		}
	}
	else if (argc == 1) // brief
	{
		zpl_bool brief = zpl_false, head = zpl_true;
		// nsm_interface_show_brief_api(struct vty *vty, struct interface *ifp, zpl_bool status)
		if (os_memcmp(name, "brief", 3) == 0)
			brief = zpl_false;
		else if (os_memcmp(name, "status", 3) == 0)
			brief = zpl_true;
		if_list = if_list_get();
		if (if_list)
		{
			for (ALL_LIST_ELEMENTS_RO(if_list, node, ifp))
			{
				if (ifp)
					nsm_interface_show_brief_api(vty, ifp, brief, &head);
			}
		}
	}
	else if (argc == 2)
	{
		char *ifname = NULL;
		ifname = if_ifname_format(name, uspv);
		ifp = if_lookup_by_name(ifname);
		if (ifp == NULL)
		{
			vty_out(vty, "%% Can't find interface %s %s%s", name, uspv, VTY_NEWLINE);
			return CMD_WARNING;
		}
		if (ifp)
			nsm_interface_show_api(vty, ifp);
	}
	IF_DATA_UNLOCK();
	return CMD_SUCCESS;
}

DEFUN(show_interface,
	  show_interface_one_cmd,
	  "show interface " CMD_IF_USPV_STR " " CMD_USP_STR,
	  SHOW_STR
	  "Interface status and configuration\n" CMD_IF_USPV_STR_HELP
		  CMD_USP_STR_HELP)
{
	return _show_interface_info(argc, argv[0], argv[1], vty);
}

ALIAS(show_interface,
	  show_interface_brief_cmd,
	  "show interface (brief|status)",
	  SHOW_STR
	  "Interface status and configuration\n"
	  "All interface brief information\n"
	  "All interface status information\n");

ALIAS(show_interface,
	  show_interface_cmd,
	  "show interface",
	  SHOW_STR
	  "Interface status and configuration\n");

DEFUN(show_interface_loopback,
	  show_interface_loopback_cmd,
	  "show interface " CMD_IF_LOOP_STR,
	  SHOW_STR
	  "Select an interface to configure\n" CMD_IF_LOOP_STR_HELP)
{
	return _show_interface_info(2, "loopback", argv[0], vty);
}
#ifdef ZPL_NSM_VLAN
DEFUN(show_interface_vlan,
	  show_interface_vlan_cmd,
	  "show interface " CMD_IF_VLAN_STR,
	  SHOW_STR
	  "Interface status and configuration\n" CMD_IF_VLAN_STR_HELP)
{
	return _show_interface_info(2, "vlan", argv[0], vty);
}
#endif
#ifdef ZPL_NSM_TRUNK
DEFUN(show_interface_trunk,
	  show_interface_trunk_cmd,
	  "show interface " CMD_IF_TRUNK_STR,
	  SHOW_STR
	  "Interface status and configuration\n" CMD_IF_TRUNK_STR_HELP)
{
	return _show_interface_info(2, "port-channel", argv[0], vty);
}
#endif
#ifdef CUSTOM_INTERFACE
ALIAS(show_interface,
	  show_wifi_interface_cmd,
	  "show interface " CMD_IF_MIP_STR " " CMD_USP_STR,
	  SHOW_STR
	  "Interface status and configuration\n" CMD_IF_MIP_STR_HELP
		  CMD_USP_STR_HELP)
#endif

static int nsm_interface_duplexspeed_info_write(struct interface *ifp, struct vty *vty)
{
	struct nsm_interface *if_data;
	if_data = ifp->info[MODULE_NSM];
	if (if_data)
	{
		switch (if_data->speed)
		{
		case NSM_IF_SPEED_10M:
			vty_out(vty, " speed 10%s", VTY_NEWLINE);
			break;
		case NSM_IF_SPEED_100M:
			vty_out(vty, " speed 100%s", VTY_NEWLINE);
			break;
		case NSM_IF_SPEED_1000M:
			vty_out(vty, " speed 1000%s", VTY_NEWLINE);
			break;
		case NSM_IF_SPEED_10000M:
			vty_out(vty, " speed 10000%s", VTY_NEWLINE);
			break;
		case NSM_IF_SPEED_AUTO:
			//vty_out(vty, " speed auto%s", VTY_NEWLINE);
			break;
		default:
			break;
		}
		switch (if_data->duplex)
		{
		case NSM_IF_DUPLEX_HALF:
			vty_out(vty, " duplex half%s", VTY_NEWLINE);
			break;
		case NSM_IF_DUPLEX_FULL:
			vty_out(vty, " duplex full%s", VTY_NEWLINE);
			break;
		case NSM_IF_DUPLEX_AUTO:
			//vty_out(vty, " duplex auto%s", VTY_NEWLINE);
			break;
		default:
			break;
		}
	}
	return 0;
}
static int nsm_interface_address_info_write(struct interface *ifp, struct vty *vty)
{
	struct listnode *addrnode;
	struct connected *ifc;
	struct prefix *p;
	union prefix46constptr up;
	for (ALL_LIST_ELEMENTS_RO(ifp->connected, addrnode, ifc))
	{
		if (CHECK_FLAG(ifc->conf, ZEBRA_IFC_CONFIGURED))
		{
			char buf[INET6_ADDRSTRLEN];
			p = ifc->address;
			up.p = p;
			vty_out(vty, " ip%s address %s",
					p->family == IPSTACK_AF_INET ? "" : "v6",
					prefix2str(up, buf, sizeof(buf)));

			vty_out(vty, "%s", VTY_NEWLINE);
		}
	}
	return 0;
}
static int nsm_interface_encapsulation_info_write(struct interface *ifp, struct vty *vty)
{
	if (ifp->if_type == IF_ETHERNET ||
		ifp->if_type == IF_GIGABT_ETHERNET ||
		ifp->if_type == IF_SERIAL)
	{
		if (ifp->if_enca != IF_ENCA_NONE)
			vty_out(vty, " encapsulation %s%s", if_enca_string(ifp->if_enca), VTY_NEWLINE);
	}
	return 0;
}
static int nsm_interface_bandwidth_info_write(struct interface *ifp, struct vty *vty)
{
	if (ifp->if_type == IF_ETHERNET || ifp->if_type == IF_GIGABT_ETHERNET)
	{
		if (ifp->bandwidth != 0)
			vty_out(vty, " bandwidth %u%s", ifp->bandwidth, VTY_NEWLINE);
	}
	return 0;
}

static int nsm_interface_l3vrfmtu_info_write(struct interface *ifp, struct vty *vty)
{
	if (ifp->if_mode == IF_MODE_L3)
	{
		vty_out(vty, " mac-address %s%s", if_mac_out_format(ifp->hw_addr), VTY_NEWLINE);

#ifdef ZPL_IPCOM_STACK_MODULE
		if (ifp->vrf_id != VRF_DEFAULT)
			vty_out(vty, " ip forward vrf %s%s", vrf_vrfid2name(ifp->vrf_id), VTY_NEWLINE);
#endif
		if (ifp->mtu != IF_MTU_DEFAULT)
			vty_out(vty, " mtu %d%s", ifp->mtu, VTY_NEWLINE);
		if (ifp->mtu6 != IF_MTU_DEFAULT)
			vty_out(vty, " mtu6 %d%s", ifp->mtu, VTY_NEWLINE);
	}
	return 0;
}



static int nsm_interface_loopback_config_write(struct vty *vty)
{
	struct listnode *node;
	struct interface *ifp;
	struct list *if_list = NULL;
	IF_DATA_LOCK();
	if_list = if_list_get();
	if (if_list)
	{
		for (ALL_LIST_ELEMENTS_RO(if_list, node, ifp))
		{
			struct nsm_interface *if_data;
	
			if_data = ifp->info[MODULE_NSM];
			if (ifp->if_type == IF_LOOPBACK)
			{
				vty_out(vty, "interface %s%s", ifp->name, VTY_NEWLINE);

				if (ifp->desc)
					vty_out(vty, " description %s%s", ifp->desc, VTY_NEWLINE);


				// nsm_client_interface_write_config(0, vty, ifp);
				if (ifp->if_mode == IF_MODE_L3)
				{
					nsm_interface_l3vrfmtu_info_write(ifp, vty);
					//nsm_interface_bandwidth_info_write(ifp, vty);
					nsm_interface_address_info_write(ifp, vty);
				}

				if (if_data)
				{
					if (if_data->shutdown == IF_ZEBRA_SHUTDOWN_ON)
						vty_out(vty, " shutdown%s", VTY_NEWLINE);
				}
				vty_out(vty, "!%s", VTY_NEWLINE);
			}
		}
	}
	IF_DATA_UNLOCK();
	return 0;
}

static int nsm_interface_config_write(struct vty *vty)
{
	struct listnode *node;
	struct interface *ifp;
	struct list *if_list = NULL;
	IF_DATA_LOCK();
	if_list = if_list_get();
	if (if_list)
	{
		for (ALL_LIST_ELEMENTS_RO(if_list, node, ifp))
		{
			struct nsm_interface *if_data;

			if (if_is_loop(ifp))
				continue;
			if (if_is_tunnel(ifp))
				continue;
			if_data = ifp->info[MODULE_NSM];

			vty_out(vty, "interface %s%s", ifp->name, VTY_NEWLINE);

			if (ifp->desc)
				vty_out(vty, " description %s%s", ifp->desc, VTY_NEWLINE);


			nsm_interface_write_hook_handler(NSM_INTF_VLAN, vty, ifp);

			nsm_interface_duplexspeed_info_write(ifp, vty);
			nsm_interface_bandwidth_info_write(ifp, vty);
			nsm_interface_encapsulation_info_write(ifp, vty);

			nsm_interface_write_hook_handler(NSM_INTF_PORT, vty, ifp);
			nsm_interface_write_hook_handler(NSM_INTF_MAC, vty, ifp);
			nsm_interface_write_hook_handler(NSM_INTF_TRUNK, vty, ifp);
			nsm_interface_write_hook_handler(NSM_INTF_DOT1X, vty, ifp);
			nsm_interface_write_hook_handler(NSM_INTF_SEC, vty, ifp);
			nsm_interface_write_hook_handler(NSM_INTF_ACL, vty, ifp);
			nsm_interface_write_hook_handler(NSM_INTF_QOS, vty, ifp);
			nsm_interface_write_hook_handler(NSM_INTF_TUNNEL, vty, ifp);
			nsm_interface_write_hook_handler(NSM_INTF_PPP, vty, ifp);
			nsm_interface_write_hook_handler(NSM_INTF_PPPOE, vty, ifp);
			nsm_interface_write_hook_handler(NSM_INTF_SERIAL, vty, ifp);

			nsm_interface_write_hook_handler(NSM_INTF_WIFI, vty, ifp);
			nsm_interface_write_hook_handler(NSM_INTF_VETH, vty, ifp);
			nsm_interface_write_hook_handler(NSM_INTF_BRIDGE, vty, ifp);
			
			nsm_interface_write_hook_handler(NSM_INTF_MIRROR, vty, ifp);


			if (ifp->if_mode == IF_MODE_L3)
			{
				nsm_interface_l3vrfmtu_info_write(ifp, vty);
#ifdef ZPL_DHCP_MODULE
				if (ifp->dhcp &&
					nsm_interface_dhcp_mode_get_api(ifp) != DHCP_CLIENT &&
					nsm_interface_dhcp_mode_get_api(ifp) != DHCP_NONE)
				{
					nsm_interface_dhcp_config(vty, ifp);
				}
				if (ifp->dhcp && nsm_interface_dhcp_mode_get_api(ifp) == DHCP_CLIENT)
				{
					nsm_interface_dhcp_config(vty, ifp);
				}
				else
#endif
				nsm_interface_address_info_write(ifp, vty);			
			}

			// nsm_interface_write_hook_handler(-1, vty, ifp);
			if (if_data)
			{
				if (if_data->shutdown == IF_ZEBRA_SHUTDOWN_ON)
					vty_out(vty, " shutdown%s", VTY_NEWLINE);
			}
			vty_out(vty, "!%s", VTY_NEWLINE);
		}
	}
	IF_DATA_UNLOCK();
	return 0;
}

static struct cmd_node interface_node =
	{
		INTERFACE_NODE,
		"%s(config-if)# ",
		1};
static struct cmd_node interface_l3_node =
	{
		INTERFACE_L3_NODE,
		"%s(config-if)# ",
		1};
static struct cmd_node interface_wireless_node =
	{
		WIRELESS_INTERFACE_NODE,
		"%s(config-if)# ",
		1};
static struct cmd_node tunnel_interface_node =
	{
		TUNNEL_INTERFACE_NODE,
		"%s(config-tunnel)# ",
		1};
static struct cmd_node loopback_interface_node =
	{
		LOOPBACK_INTERFACE_NODE,
		"%s(config-loopback)# ",
		1};

static struct cmd_node lag_interface_node =
	{
		LAG_INTERFACE_NODE,
		"%s(config-port-channel)# ",
		1};
static struct cmd_node lag_interface_l3_node =
	{
		LAG_INTERFACE_L3_NODE,
		"%s(config-port-channel)# ",
		1};

static struct cmd_node brigde_interface_node =
	{
		BRIGDE_INTERFACE_NODE,
		"%s(config-brigde)# ",
		1};

#ifdef CUSTOM_INTERFACE
static struct cmd_node wifi_interface_node =
	{
		WIFI_INTERFACE_NODE,
		"%s(config-wifi)# ",
		1};
static struct cmd_node modem_interface_node =
	{
		MODEM_INTERFACE_NODE,
		"%s(config-modem)# ",
		1};
#endif

static struct cmd_node serial_interface_node =
	{
		SERIAL_INTERFACE_NODE,
		"%s(config-serial)# ",
		1};
static struct cmd_node ifrange_interface_node =
	{
		INTERFACE_RANGE_NODE,
		"%s(config-if-range)# ",
		1};
static struct cmd_node ifrange_l3interface_node =
	{
		INTERFACE_L3_RANGE_NODE,
		"%s(config-if-range)# ",
		1};
static struct cmd_node e1_ifrange_l3interface_node =
	{
		E1_INTERFACE_L3_RANGE_NODE,
		"%s(config-if-range)# ",
		1};
static struct cmd_node epon_ifrange_interface_node =
	{
		EPON_INTERFACE_RANGE_NODE,
		"%s(config-if-range)# ",
		1};
static struct cmd_node epon_ifrange_l3interface_node =
	{
		EPON_INTERFACE_L3_RANGE_NODE,
		"%s(config-if-range)# ",
		1};

static struct cmd_node trunk_group_node =
	{
		TRUNK_NODE,
		"%s(config-trunk-group)# ",
		1};

static void cmd_show_interface_init(int node)
{
	install_element(node, CMD_VIEW_LEVEL, &show_interface_one_cmd);
	install_element(node, CMD_VIEW_LEVEL, &show_interface_cmd);
	install_element(node, CMD_VIEW_LEVEL, &show_interface_brief_cmd);
	install_element(node, CMD_VIEW_LEVEL, &show_interface_loopback_cmd);
#ifdef ZPL_NSM_VLAN
	install_element(node, CMD_VIEW_LEVEL, &show_interface_vlan_cmd);
#endif
#ifdef ZPL_NSM_TRUNK
	install_element(node, CMD_VIEW_LEVEL, &show_interface_trunk_cmd);
#endif	
#ifdef CUSTOM_INTERFACE
	install_element(node, CMD_VIEW_LEVEL, &show_wifi_interface_cmd);
#endif
	//install_element(node, CMD_VIEW_LEVEL, &show_interface_all_debug_cmd);

#ifdef ZPL_KERNEL_STACK_MODULE
	install_element(node, CMD_VIEW_LEVEL, &show_interface_kernel_cmd);
#endif
}

static void cmd_base_interface_init(int node)
{
	install_element(node, CMD_CONFIG_LEVEL, &nsm_interface_desc_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_nsm_interface_desc_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &nsm_interface_shutdown_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_nsm_interface_shutdown_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &nsm_interface_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &nsm_interface_sub_cmd);
#ifdef ZPL_NSM_VLAN	
	install_element(node, CMD_CONFIG_LEVEL, &nsm_interface_vlan_cmd);
#endif
	install_element(node, CMD_CONFIG_LEVEL, &nsm_interface_loopback_cmd);
#ifdef ZPL_NSM_TRUNK
	install_element(node, CMD_CONFIG_LEVEL, &nsm_interface_trunk_cmd);
#endif	
	install_element(node, CMD_CONFIG_LEVEL, &hidden_nsm_interface_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &nsm_interface_range_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &nsm_interface_range_sub_cmd);

	if (node != INTERFACE_NODE && node != LAG_INTERFACE_NODE)
	{
		install_element(node, CMD_CONFIG_LEVEL, &nsm_interface_ip_address_cmd);
		install_element(node, CMD_CONFIG_LEVEL, &no_nsm_interface_ip_address_cmd);
#ifdef HAVE_IPV6
		install_element(node, CMD_CONFIG_LEVEL, &nsm_interface_ipv6_address_cmd);
		install_element(node, CMD_CONFIG_LEVEL, &no_nsm_interface_ipv6_address_cmd);
#endif /* HAVE_IPV6 */
#ifdef ZPL_IPCOM_STACK_MODULE
		install_element(node, CMD_CONFIG_LEVEL, &nsm_interface_ip_vrf_cmd);
		install_element(node, CMD_CONFIG_LEVEL, &no_nsm_interface_ip_vrf_cmd);
#endif
		install_element(node, CMD_CONFIG_LEVEL, &nsm_interface_mtu_cmd);
		install_element(node, CMD_CONFIG_LEVEL, &no_nsm_interface_mtu_cmd);
		install_element(node, CMD_CONFIG_LEVEL, &nsm_interface_metric_cmd);
		install_element(node, CMD_CONFIG_LEVEL, &no_nsm_interface_metric_cmd);
	}
	if (node == SERIAL_INTERFACE_NODE)
	{
		install_element(node, CMD_CONFIG_LEVEL, &nsm_interface_enca_ppp_cmd);
		install_element(node, CMD_CONFIG_LEVEL, &no_nsm_interface_enca_cmd);
	}
	if (node == INTERFACE_L3_NODE)
	{
		install_element(node, CMD_CONFIG_LEVEL, &nsm_interface_enca_dot1q_cmd);
		install_element(node, CMD_CONFIG_LEVEL, &nsm_interface_enca_pppoe_cmd);
		install_element(node, CMD_CONFIG_LEVEL, &no_nsm_interface_enca_cmd);
		install_element(node, CMD_CONFIG_LEVEL, &nsm_interface_mac_cmd);
		install_element(node, CMD_CONFIG_LEVEL, &no_nsm_interface_mac_cmd);
	}

#ifdef ZPL_KERNEL_STACK_MODULE
	install_element(node, CMD_CONFIG_LEVEL, &nsm_interface_set_kernel_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_nsm_interface_set_kernel_cmd);
#endif
}

static void cmd_ethernet_interface_init(int node)
{
	install_element(node, CMD_CONFIG_LEVEL, &nsm_interface_bandwidth_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_nsm_interface_bandwidth_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &nsm_interface_duplex_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_nsm_interface_duplex_cmd);


	install_element(node, CMD_CONFIG_LEVEL, &nsm_interface_speed_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_nsm_interface_speed_cmd);
}

static void cmd_range_interface_init(int node)
{
	install_element(node, CMD_CONFIG_LEVEL, &nsm_interface_bandwidth_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_nsm_interface_bandwidth_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &nsm_interface_duplex_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_nsm_interface_duplex_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &nsm_interface_speed_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_nsm_interface_speed_cmd);
	if (node == INTERFACE_RANGE_NODE)
	{
	}
	if (node == INTERFACE_L3_RANGE_NODE)
	{
		install_element(node, CMD_CONFIG_LEVEL, &nsm_interface_mtu_cmd);
		install_element(node, CMD_CONFIG_LEVEL, &no_nsm_interface_mtu_cmd);
		install_element(node, CMD_CONFIG_LEVEL, &nsm_interface_metric_cmd);
		install_element(node, CMD_CONFIG_LEVEL, &no_nsm_interface_metric_cmd);
	}
	if (node == E1_INTERFACE_L3_RANGE_NODE)
	{
	}
	if (node == EPON_INTERFACE_RANGE_NODE)
	{
	}
	if (node == EPON_INTERFACE_L3_RANGE_NODE)
	{
	}
}

#ifdef CUSTOM_INTERFACE
static void cmd_custom_interface_init(int node)
{
}
#endif

void cmd_interface_init(void)
{
	install_node(&interface_node, nsm_interface_config_write);
	install_node(&ifrange_interface_node, NULL);
	install_node(&interface_l3_node, NULL);
	install_node(&ifrange_l3interface_node, NULL);

	install_node(&tunnel_interface_node, NULL);
	install_node(&loopback_interface_node, nsm_interface_loopback_config_write);

	install_node(&serial_interface_node, NULL);

	install_node(&brigde_interface_node, NULL);
	install_node(&interface_wireless_node, NULL);

#ifdef CUSTOM_INTERFACE
	install_node(&wifi_interface_node, NULL);
	install_node(&modem_interface_node, NULL);
#endif

	install_node(&lag_interface_node, NULL);
	install_node(&lag_interface_l3_node, NULL);

	install_node(&e1_ifrange_l3interface_node, NULL);
	install_node(&epon_ifrange_interface_node, NULL);
	install_node(&epon_ifrange_l3interface_node, NULL);

	install_node(&trunk_group_node, NULL);

	// install_node(LOOPBACK_INTERFACE_NODE, nsm_interface_loopback_config_write);
	// install_node(INTERFACE_NODE, nsm_interface_config_write);
	// reinstall_node(WIRELESS_INTERFACE_NODE, nsm_interface_config_write);

	install_default(INTERFACE_NODE);
	install_default(INTERFACE_L3_NODE);
	install_default(INTERFACE_RANGE_NODE);
	install_default(INTERFACE_L3_RANGE_NODE);
	install_default(E1_INTERFACE_L3_RANGE_NODE);
	install_default(EPON_INTERFACE_RANGE_NODE);
	install_default(EPON_INTERFACE_L3_RANGE_NODE);

	install_default(TUNNEL_INTERFACE_NODE);
	install_default(LOOPBACK_INTERFACE_NODE);
	install_default(LAG_INTERFACE_NODE);
	install_default(LAG_INTERFACE_L3_NODE);
	install_default(BRIGDE_INTERFACE_NODE);
	install_default(SERIAL_INTERFACE_NODE);
	install_default(WIRELESS_INTERFACE_NODE);

#ifdef CUSTOM_INTERFACE
	install_default(WIFI_INTERFACE_NODE);
	install_default(MODEM_INTERFACE_NODE);
#endif

	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &nsm_interface_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &nsm_interface_sub_cmd);
#ifdef ZPL_NSM_VLAN	
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &nsm_interface_vlan_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_nsm_interface_vlan_cmd);
#endif	
#ifdef ZPL_NSM_TRUNK	
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &nsm_interface_trunk_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_nsm_interface_trunk_cmd);
#endif
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &nsm_interface_loopback_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_nsm_interface_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_nsm_interface_loopback_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &hidden_nsm_interface_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &hidden_no_nsm_interface_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &nsm_interface_range_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &nsm_interface_range_sub_cmd);
#ifdef CUSTOM_INTERFACE
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &nsm_wifi_interface_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_nsm_wifi_interface_cmd);
#endif

	cmd_base_interface_init(INTERFACE_NODE);
	cmd_base_interface_init(INTERFACE_L3_NODE);
	cmd_base_interface_init(TUNNEL_INTERFACE_NODE);
	cmd_base_interface_init(LOOPBACK_INTERFACE_NODE);

	cmd_base_interface_init(LAG_INTERFACE_NODE);
	cmd_base_interface_init(LAG_INTERFACE_L3_NODE);

	cmd_base_interface_init(BRIGDE_INTERFACE_NODE);
	cmd_base_interface_init(SERIAL_INTERFACE_NODE);
	cmd_base_interface_init(WIRELESS_INTERFACE_NODE);

	cmd_ethernet_interface_init(INTERFACE_NODE);
	cmd_ethernet_interface_init(INTERFACE_L3_NODE);

	cmd_ethernet_interface_init(LAG_INTERFACE_NODE);
	cmd_ethernet_interface_init(LAG_INTERFACE_L3_NODE);

	cmd_ethernet_interface_init(BRIGDE_INTERFACE_NODE);

#ifdef CUSTOM_INTERFACE
	cmd_custom_interface_init(WIFI_INTERFACE_NODE);
	cmd_custom_interface_init(MODEM_INTERFACE_NODE);
#endif

	cmd_show_interface_init(ENABLE_NODE);
	cmd_show_interface_init(CONFIG_NODE);
	cmd_show_interface_init(INTERFACE_NODE);
	cmd_show_interface_init(INTERFACE_L3_NODE);
	cmd_show_interface_init(TUNNEL_INTERFACE_NODE);
	cmd_show_interface_init(LOOPBACK_INTERFACE_NODE);

	cmd_show_interface_init(LAG_INTERFACE_NODE);
	cmd_show_interface_init(LAG_INTERFACE_L3_NODE);

	cmd_show_interface_init(BRIGDE_INTERFACE_NODE);
	cmd_show_interface_init(SERIAL_INTERFACE_NODE);
	cmd_show_interface_init(WIRELESS_INTERFACE_NODE);

	cmd_show_interface_init(INTERFACE_RANGE_NODE);
	cmd_show_interface_init(INTERFACE_L3_RANGE_NODE);
	cmd_show_interface_init(E1_INTERFACE_L3_RANGE_NODE);
	cmd_show_interface_init(EPON_INTERFACE_RANGE_NODE);
	cmd_show_interface_init(EPON_INTERFACE_L3_RANGE_NODE);

	cmd_base_interface_init(INTERFACE_RANGE_NODE);
	cmd_base_interface_init(INTERFACE_L3_RANGE_NODE);
	cmd_base_interface_init(E1_INTERFACE_L3_RANGE_NODE);
	cmd_base_interface_init(EPON_INTERFACE_RANGE_NODE);
	cmd_base_interface_init(EPON_INTERFACE_L3_RANGE_NODE);

	cmd_range_interface_init(INTERFACE_RANGE_NODE);
	cmd_range_interface_init(INTERFACE_L3_RANGE_NODE);
}
