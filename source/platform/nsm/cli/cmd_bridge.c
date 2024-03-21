/*
 * cmd_bridge.c
 *
 *  Created on: Oct 17, 2018
 *      Author: zhurish
 */



#include "auto_include.h"
#include "zpl_type.h"
#include "module.h"
#include "prefix.h"
#include "if.h"
#include "vty_include.h"
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
	else
		vty_out (vty, "Error:Can not get this interface.%s", VTY_NEWLINE);
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
	else
		vty_out (vty, "Error:Can not get this interface.%s", VTY_NEWLINE);
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
		ret = nsm_bridge_interface_stp_set_api(ifp, zpl_true);
	else
		vty_out (vty, "Error:Can not get this interface.%s", VTY_NEWLINE);
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
		ret = nsm_bridge_interface_stp_set_api(ifp, zpl_false);
	else
		vty_out (vty, "Error:Can not get this interface.%s", VTY_NEWLINE);
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
	else
		vty_out (vty, "Error:Can not get this interface.%s", VTY_NEWLINE);
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
	else
		vty_out (vty, "Error:Can not get this interface.%s", VTY_NEWLINE);
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
	install_element(node,  CMD_CONFIG_LEVEL,&bridge_add_interface_cmd);
	install_element(node,  CMD_CONFIG_LEVEL,&no_bridge_add_interface_cmd);

	install_element(node,  CMD_CONFIG_LEVEL,&bridge_protocol_ieee_cmd);
	install_element(node,  CMD_CONFIG_LEVEL,&no_bridge_protocol_ieee_cmd);

	install_element(node,  CMD_CONFIG_LEVEL,&ieee_age_time_cmd);
	install_element(node,  CMD_CONFIG_LEVEL,&no_ieee_age_time_cmd);

	install_element(node,  CMD_CONFIG_LEVEL,&ieee_hello_time_cmd);
	install_element(node,  CMD_CONFIG_LEVEL,&no_ieee_hello_time_cmd);

	install_element(node,  CMD_CONFIG_LEVEL,&ieee_forward_time_cmd);
	install_element(node,  CMD_CONFIG_LEVEL,&no_ieee_forward_time_cmd);
}

void cmd_bridge_init(void)
{
	cmd_base_bridge_init(BRIGDE_INTERFACE_NODE);
}
