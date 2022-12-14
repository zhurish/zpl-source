/*
 * cmd_bridge.c
 *
 *  Created on: Oct 17, 2018
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
#include "tty_com.h"
#include "nsm_bridge.h"


DEFUN (bridge_add_interface,
		bridge_add_interface_cmd,
		"bridge-interface (ethernet|gigabitethernet|wireless) "CMD_USP_STR,
		"bridge-interface configure\n"
		"Ethernet interface\n"
		"GigabitEthernet interface\n"
		"Wireless interface\n"
		CMD_USP_STR_HELP)
{
	int ret = ERROR;
	struct interface *ifp = NULL;
	ifp = if_lookup_by_name (if_ifname_format(argv[0], argv[1]));
	if(ifp)
		ret = nsm_bridge_add_interface_api(vty->index, ifp);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_bridge_add_interface,
		no_bridge_add_interface_cmd,
		"no bridge-interface (ethernet|gigabitethernet|wireless) "CMD_USP_STR,
		NO_STR
		"bridge-interface configure\n"
		"Ethernet interface\n"
		"GigabitEthernet interface\n"
		"Wireless interface\n"
		CMD_USP_STR_HELP)
{
	int ret = ERROR;
	struct interface *ifp = NULL;
	ifp = if_lookup_by_name (if_ifname_format(argv[0], argv[1]));
	if(ifp)
		ret = nsm_bridge_del_interface_api(vty->index, ifp);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}



DEFUN (bridge_protocol_ieee,
		bridge_protocol_ieee_cmd,
		"bridge-protocol ieee",
		"bridge-protocol configure\n"
		"IEEE spanning-tree\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(ifp)
		ret = nsm_bridge_interface_stp_set_api(ifp, TRUE);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_bridge_protocol_ieee,
		no_bridge_protocol_ieee_cmd,
		"no bridge-protocol ieee",
		NO_STR
		"bridge-protocol configure\n"
		"IEEE spanning-tree\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(ifp)
		ret = nsm_bridge_interface_stp_set_api(ifp, FALSE);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ieee_age_time,
		ieee_age_time_cmd,
		"ieee age <10-65536>",
		"IEEE spanning-tree\n"
		"Aging time\n"
		"time value\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	int value = atoi(argv[0]);
	if(ifp)
		ret = nsm_bridge_interface_max_age_set_api(ifp, value);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ieee_age_time,
		no_ieee_age_time_cmd,
		"ieee age",
		NO_STR
		"IEEE spanning-tree\n"
		"Aging time\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(ifp)
		ret = nsm_bridge_interface_max_age_set_api(ifp, 0);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}



DEFUN (ieee_hello_time,
		ieee_hello_time_cmd,
		"ieee hello-time <10-65536>",
		"IEEE spanning-tree\n"
		"Hello time\n"
		"time value\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	int value = atoi(argv[0]);
	if(ifp)
		ret = nsm_bridge_interface_hello_time_set_api(ifp, value);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ieee_hello_time,
		no_ieee_hello_time_cmd,
		"ieee hello-time",
		NO_STR
		"IEEE spanning-tree\n"
		"Hello time\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(ifp)
		ret = nsm_bridge_interface_hello_time_set_api(ifp, 0);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ieee_forward_time,
		ieee_forward_time_cmd,
		"ieee forward-delay <10-65536>",
		"IEEE spanning-tree\n"
		"Forward-delay time\n"
		"time value\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	int value = atoi(argv[0]);
	if(ifp)
		ret = nsm_bridge_interface_forward_delay_set_api(ifp, value);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ieee_forward_time,
		no_ieee_forward_time_cmd,
		"ieee forward-delay",
		NO_STR
		"IEEE spanning-tree\n"
		"Forward-delay time\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(ifp)
		ret = nsm_bridge_interface_forward_delay_set_api(ifp, 0);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


static void cmd_base_bridge_init(int node)
{
	install_element(node, &bridge_add_interface_cmd);
	install_element(node, &no_bridge_add_interface_cmd);

	install_element(node, &bridge_protocol_ieee_cmd);
	install_element(node, &no_bridge_protocol_ieee_cmd);

	install_element(node, &ieee_age_time_cmd);
	install_element(node, &no_ieee_age_time_cmd);

	install_element(node, &ieee_hello_time_cmd);
	install_element(node, &no_ieee_hello_time_cmd);

	install_element(node, &ieee_forward_time_cmd);
	install_element(node, &no_ieee_forward_time_cmd);
}

void cmd_bridge_init(void)
{
	cmd_base_bridge_init(BRIGDE_INTERFACE_NODE);
}
