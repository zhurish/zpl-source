/*
 * cmd_dot1x.c
 *
 *  Created on: May 10, 2018
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
#include "nsm_8021x.h"



struct dot1x_user
{
	struct vty *vty;
	ifindex_t	ifindex;

};

/*
 * global
 */
DEFUN (dot1x_system_auth_ctrl,
		dot1x_system_auth_ctrl_cmd,
		"dot1x system-auth-ctrl",
		"Dot1x authentication\n"
		"system control\n")
{
	int ret = ERROR;
	if(!nsm_dot1x_global_is_enable())
		ret = nsm_dot1x_global_enable(TRUE);
	else
		ret = OK;
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_dot1x_system_auth_ctrl,
		no_dot1x_system_auth_ctrl_cmd,
		"no dot1x system-auth-ctrl",
		NO_STR
		"Dot1x authentication\n"
		"system control\n")
{
	int ret = ERROR;
	if(nsm_dot1x_global_is_enable())
		ret = nsm_dot1x_global_enable(FALSE);
	else
		ret = OK;
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


/*
 * interface
 */

/*
 * dot1x port-control (auto | force-authorized | force-unauthorized | dir ( both | in))
 * no dot1x port-control
 */
DEFUN (dot1x_port_control,
		dot1x_port_control_cmd,
		"dot1x port-control (auto|force-authorized|force-unauthorized)",
		"Dot1x authentication\n"
		"port-control\n")
{
	int ret = ERROR;
	dot1x_type_en type = 0;
	struct interface *ifp = vty->index;
	if(!nsm_dot1x_global_is_enable())
		return CMD_WARNING;

	if(argc >= 1)
	{
		if(os_memcmp(argv[0], "auto", 3) == 0)
			type = DOT1X_AUTO;
		else
		{
			if(os_memcmp(argv[0], "force-authorized", 8) == 0)
				type = DOT1X_FORCE_AUTHORIZED;
			if(os_memcmp(argv[0], "force-unauthorized", 8) == 0)
				type = DOT1X_FORCE_UNAUTHORIZED;
		}
		if(nsm_dot1x_is_enable_api(ifp->ifindex))
		{
			ret = nsm_dot1x_auth_type_set_api(ifp->ifindex, type);
		}
		else
		{
			ret = nsm_dot1x_enable_set_api(ifp->ifindex, TRUE, type);
			//ret |= nsm_dot1x_port_mode_set_api(ifp->ifindex, TRUE);
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_dot1x_port_control,
		no_dot1x_port_control_cmd,
		"no dot1x port-control",
		NO_STR
		"Dot1x authentication\n"
		"port-control\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(nsm_dot1x_global_is_enable() && nsm_dot1x_is_enable_api(ifp->ifindex))
		ret = nsm_dot1x_enable_set_api(ifp->ifindex, FALSE, DOT1X_NONE);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (dot1x_initialize,
		dot1x_initialize_cmd,
		"dot1x initialize",
		"Dot1x authentication\n"
		"Initialize\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(nsm_dot1x_global_is_enable() && nsm_dot1x_is_enable_api(ifp->ifindex))
		ret = nsm_dot1x_reset_api(ifp->ifindex);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}



DEFUN (dot1x_max_req,
		dot1x_max_req_cmd,
		"dot1x max-req <1-16>",
		"Dot1x authentication\n"
		"Max of request\n"
		"Request count")
{
	int ret = ERROR;
	u_int value = 0;
	struct interface *ifp = vty->index;
	value = atoi(argv[0]);
	if(nsm_dot1x_global_is_enable() && nsm_dot1x_is_enable_api(ifp->ifindex))
		ret = nsm_dot1x_max_req_set_api(ifp->ifindex, value);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_dot1x_max_req,
		no_dot1x_max_req_cmd,
		"no dot1x max-req",
		NO_STR
		"Dot1x authentication\n"
		"Max of request\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(nsm_dot1x_global_is_enable() && nsm_dot1x_is_enable_api(ifp->ifindex))
		ret = nsm_dot1x_max_req_set_api(ifp->ifindex, 0);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (dot1x_protocol_version,
		dot1x_protocol_version_cmd,
		"dot1x protocol-version (1|2)",
		"Dot1x authentication\n"
		"Eap Protocol version\n"
		"Version number\n")
{
	int ret = ERROR;
	u_int value = 0;
	struct interface *ifp = vty->index;
	value = atoi(argv[0]);
	if(nsm_dot1x_global_is_enable() && nsm_dot1x_is_enable_api(ifp->ifindex))
		ret = nsm_dot1x_auth_version_set_api(ifp->ifindex, value);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_dot1x_protocol_version,
		no_dot1x_protocol_version_cmd,
		"no dot1x protocol-version",
		NO_STR
		"Dot1x authentication\n"
		"Eap Protocol version\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(nsm_dot1x_global_is_enable() && nsm_dot1x_is_enable_api(ifp->ifindex))
		ret = nsm_dot1x_auth_version_set_api(ifp->ifindex, 0);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (dot1x_reauthentication,
		dot1x_reauthentication_cmd,
		"dot1x reauthentication",
		"Dot1x authentication\n"
		"Eap Reauthentication\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(nsm_dot1x_global_is_enable() && nsm_dot1x_is_enable_api(ifp->ifindex))
		ret = nsm_dot1x_reauthentication_set_api(ifp->ifindex, TRUE);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_dot1x_reauthentication,
		no_dot1x_reauthentication_cmd,
		"no dot1x reauthentication",
		NO_STR
		"Dot1x authentication\n"
		"Eap Reauthentication\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(nsm_dot1x_global_is_enable() && nsm_dot1x_is_enable_api(ifp->ifindex))
		ret = nsm_dot1x_reauthentication_set_api(ifp->ifindex, FALSE);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (dot1x_guest_vlan,
		dot1x_guest_vlan_cmd,
		"dot1x guest-vlan <2-4094>",
		"Dot1x authentication\n"
		"Eap guest-vlan\n"
		"Vlan ID\n")
{
	int ret = ERROR;
	vlan_t value = 0;
	struct interface *ifp = vty->index;
	value = atoi(argv[0]);
	if(nsm_dot1x_global_is_enable() && nsm_dot1x_is_enable_api(ifp->ifindex))
		ret = nsm_dot1x_guest_vlan_set_api(ifp->ifindex, value);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_dot1x_guest_vlan,
		no_dot1x_guest_vlan_cmd,
		"no dot1x guest-vlan",
		NO_STR
		"Dot1x authentication\n"
		"Eap guest-vlan\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(nsm_dot1x_global_is_enable() && nsm_dot1x_is_enable_api(ifp->ifindex))
		ret = nsm_dot1x_guest_vlan_set_api(ifp->ifindex, 0);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (dot1x_mac_auth_bypass,
		dot1x_mac_auth_bypass_cmd,
		"dot1x mac-auth-bypass",
		"Dot1x authentication\n"
		"Eap mac-auth-bypass\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(nsm_dot1x_global_is_enable() && nsm_dot1x_is_enable_api(ifp->ifindex))
		ret = nsm_dot1x_mac_auth_bypass_set_api(ifp->ifindex, TRUE);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_dot1x_mac_auth_bypass,
		no_dot1x_mac_auth_bypass_cmd,
		"no dot1x mac-auth-bypass",
		NO_STR
		"Dot1x authentication\n"
		"Eap mac-auth-bypass\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(nsm_dot1x_global_is_enable() && nsm_dot1x_is_enable_api(ifp->ifindex))
		ret = nsm_dot1x_mac_auth_bypass_set_api(ifp->ifindex, FALSE);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (dot1x_port_mode,
		dot1x_port_mode_cmd,
		"dot1x port-mode (port|mac)",
		"Dot1x authentication\n"
		"Eap mode\n"
		"Eap port mode\n"
		"Eap mac mode\n")
{
	int ret = ERROR;
	BOOL mode = TRUE;
	struct interface *ifp = vty->index;
	if(os_memcmp(argv[0], "port", 3) == 0)
		mode = TRUE;
	else
		mode = FALSE;
	if(nsm_dot1x_global_is_enable() && nsm_dot1x_is_enable_api(ifp->ifindex))
		ret = nsm_dot1x_port_mode_set_api(ifp->ifindex, mode);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_dot1x_port_mode,
		no_dot1x_port_mode_cmd,
		"no dot1x port-mode",
		NO_STR
		"Dot1x authentication\n"
		"Eap port mode\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(nsm_dot1x_global_is_enable() && nsm_dot1x_is_enable_api(ifp->ifindex))
		ret = nsm_dot1x_port_mode_set_api(ifp->ifindex, TRUE);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (dot1x_max_user,
		dot1x_max_user_cmd,
		"dot1x max-user <1-16>",
		"Dot1x authentication\n"
		"Eap max-user\n"
		"max user number\n")
{
	int ret = ERROR;
	vlan_t value = 0;
	struct interface *ifp = vty->index;
	value = atoi(argv[0]);
	if(nsm_dot1x_global_is_enable() && nsm_dot1x_is_enable_api(ifp->ifindex))
	{
		BOOL mode = TRUE;
		if(nsm_dot1x_port_mode_get_api(ifp->ifindex, &mode) == OK && mode == FALSE)
			ret = nsm_dot1x_max_user_set_api(ifp->ifindex, value);
		else
		{
			vty_out(vty, "ERROR: it's CMD just for MAC mode of DOT1X%s", VTY_NEWLINE);
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_dot1x_max_user,
		no_dot1x_max_user_cmd,
		"no dot1x max-user",
		NO_STR
		"Dot1x authentication\n"
		"Eap max-user\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(nsm_dot1x_global_is_enable() && nsm_dot1x_is_enable_api(ifp->ifindex))
		ret = nsm_dot1x_max_user_set_api(ifp->ifindex, 0);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

/*
dot1x timeout (re-authperiod seconds | server-timeout seconds | supp-timeout seconds |tx-period seconds | quiet-period seconds)
no dot1x timeout ( reauth-period | server-timeout | supp-timeout | tx-period | quiet-period seconds)
*/
DEFUN (dot1x_timeout_re_auth,
		dot1x_timeout_re_auth_cmd,
		"dot1x timeout re-authperiod <1-65535>",
		"Dot1x authentication\n"
		"Timeout\n"
		"re-authperiod\n"
		"value\n")
{
	int ret = ERROR;
	vlan_t value = 0;
	struct interface *ifp = vty->index;
	value = atoi(argv[0]);
	if(nsm_dot1x_global_is_enable() && nsm_dot1x_is_enable_api(ifp->ifindex))
		ret = nsm_dot1x_reauth_timeout_set_api(ifp->ifindex, value);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_dot1x_timeout_re_auth,
		no_dot1x_timeout_re_auth_cmd,
		"no dot1x re-authperiod",
		NO_STR
		"Dot1x authentication\n"
		"Timeout\n"
		"re-authperiod\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(nsm_dot1x_global_is_enable() && nsm_dot1x_is_enable_api(ifp->ifindex))
		ret = nsm_dot1x_reauth_timeout_set_api(ifp->ifindex, 0);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (dot1x_timeout_server_timeout,
		dot1x_timeout_server_timeout_cmd,
		"dot1x timeout server-timeout <1-65535>",
		"Dot1x authentication\n"
		"Timeout\n"
		"server-timeout\n"
		"value\n")
{
	int ret = ERROR;
	vlan_t value = 0;
	struct interface *ifp = vty->index;
	value = atoi(argv[0]);
	if(nsm_dot1x_global_is_enable() && nsm_dot1x_is_enable_api(ifp->ifindex))
		ret = nsm_dot1x_server_timeout_set_api(ifp->ifindex, value);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_dot1x_timeout_server_timeout,
		no_dot1x_timeout_server_timeout_cmd,
		"no dot1x server-timeout",
		NO_STR
		"Dot1x authentication\n"
		"Timeout\n"
		"server-timeout\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(nsm_dot1x_global_is_enable() && nsm_dot1x_is_enable_api(ifp->ifindex))
		ret = nsm_dot1x_server_timeout_set_api(ifp->ifindex, 0);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (dot1x_timeout_supp_timeout,
		dot1x_timeout_supp_timeout_cmd,
		"dot1x timeout supp-timeout <1-65535>",
		"Dot1x authentication\n"
		"Timeout\n"
		"supp-timeout\n"
		"value\n")
{
	int ret = ERROR;
	vlan_t value = 0;
	struct interface *ifp = vty->index;
	value = atoi(argv[0]);
	if(nsm_dot1x_global_is_enable() && nsm_dot1x_is_enable_api(ifp->ifindex))
		ret = nsm_dot1x_supp_timeout_set_api(ifp->ifindex, value);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_dot1x_timeout_supp_timeout,
		no_dot1x_timeout_supp_timeout_cmd,
		"no dot1x supp-timeout",
		NO_STR
		"Dot1x authentication\n"
		"Timeout\n"
		"supp-timeout\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(nsm_dot1x_global_is_enable() && nsm_dot1x_is_enable_api(ifp->ifindex))
		ret = nsm_dot1x_supp_timeout_set_api(ifp->ifindex, 0);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (dot1x_timeout_tx_period,
		dot1x_timeout_tx_period_cmd,
		"dot1x timeout tx-period <1-65535>",
		"Dot1x authentication\n"
		"Timeout\n"
		"tx-period\n"
		"value\n")
{
	int ret = ERROR;
	vlan_t value = 0;
	struct interface *ifp = vty->index;
	value = atoi(argv[0]);
	if(nsm_dot1x_global_is_enable() && nsm_dot1x_is_enable_api(ifp->ifindex))
		ret = nsm_dot1x_period_timeout_set_api(ifp->ifindex, value);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_dot1x_timeout_tx_period,
		no_dot1x_timeout_tx_period_cmd,
		"no dot1x tx-period",
		NO_STR
		"Dot1x authentication\n"
		"Timeout\n"
		"tx-period\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(nsm_dot1x_global_is_enable() && nsm_dot1x_is_enable_api(ifp->ifindex))
		ret = nsm_dot1x_period_timeout_set_api(ifp->ifindex, 0);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (dot1x_timeout_quiet_period,
		dot1x_timeout_quiet_period_cmd,
		"dot1x timeout quiet-period <1-65535>",
		"Dot1x authentication\n"
		"Timeout\n"
		"quiet-period\n"
		"value\n")
{
	int ret = ERROR;
	vlan_t value = 0;
	struct interface *ifp = vty->index;
	value = atoi(argv[0]);
	if(nsm_dot1x_global_is_enable() && nsm_dot1x_is_enable_api(ifp->ifindex))
		ret = nsm_dot1x_quiet_period_timeout_set_api(ifp->ifindex, value);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_dot1x_timeout_quiet_period,
		no_dot1x_timeout_quiet_period_cmd,
		"no dot1x quiet-period",
		NO_STR
		"Dot1x authentication\n"
		"Timeout\n"
		"quiet-period\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(nsm_dot1x_global_is_enable() && nsm_dot1x_is_enable_api(ifp->ifindex))
		ret = nsm_dot1x_quiet_period_timeout_set_api(ifp->ifindex, 0);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}




static int _build_dot1x_interface_one(dot1x_t *pstNode, struct dot1x_user *user)
{
	struct vty *vty = user->vty;
	if(user->ifindex == pstNode->ifindex && pstNode->enable)
	{
		const char *type_str[] = { "none", "auto", "force-authorized", "force-unauthorized"};
		//auto | force-authorized | force-unauthorized
		if(!pstNode->port_mode)
			vty_out(vty, " dot1x port-mode mac%s", VTY_NEWLINE);
		vty_out(vty, " dot1x port-control %s%s", type_str[pstNode->type], VTY_NEWLINE);
		if(pstNode->eap_version)
			vty_out(vty, " dot1x protocol-version %d%s", pstNode->eap_version, VTY_NEWLINE);

		if(pstNode->reauthentication)
			vty_out(vty, " dot1x reauthentication%s", VTY_NEWLINE);
		if(pstNode->max_req)
			vty_out(vty, " dot1x max-req %d%s", pstNode->max_req, VTY_NEWLINE);

		if(pstNode->mac_auth_bypass)
			vty_out(vty, " dot1x mac-auth-bypass%s", VTY_NEWLINE);

		if(pstNode->max_user && !pstNode->port_mode)
			vty_out(vty, " dot1x max-user %d%s", pstNode->max_user, VTY_NEWLINE);

		if(pstNode->guest_vlan)
			vty_out(vty, " dot1x guest-vlan %d%s", pstNode->guest_vlan, VTY_NEWLINE);

		if(pstNode->reauth_timeout)
			vty_out(vty, " dot1x timeout re-authperiod %d%s", pstNode->reauth_timeout, VTY_NEWLINE);
		if(pstNode->server_timeout)
			vty_out(vty, " dot1x timeout server-timeout %d%s", pstNode->server_timeout, VTY_NEWLINE);
		if(pstNode->supp_timeout)
			vty_out(vty, " dot1x timeout supp-timeout %d%s", pstNode->supp_timeout, VTY_NEWLINE);
		if(pstNode->tx_period_timeout)
			vty_out(vty, " dot1x timeout tx-period %d%s", pstNode->tx_period_timeout, VTY_NEWLINE);
		if(pstNode->quiet_period_timeout)
			vty_out(vty, " dot1x timeout quiet-period %d%s", pstNode->quiet_period_timeout, VTY_NEWLINE);
	}
	return OK;
}

static int build_dot1x_interface(struct vty *vty, struct interface *ifp)
{
	struct dot1x_user user;
	user.vty = vty;
	user.ifindex = ifp->ifindex;
	return dot1x_callback_api((dot1x_cb)_build_dot1x_interface_one, &user);
}

static int build_dot1x_config(struct vty *vty)
{
	if(nsm_dot1x_global_is_enable())
		vty_out(vty, "dot1x system-auth-ctrl%s", VTY_NEWLINE);
	return OK;
}

static void cmd_dot1x_interface_init(int node)
{
	install_element (node, &dot1x_port_control_cmd);
	install_element (node, &no_dot1x_port_control_cmd);

	install_element (node, &dot1x_initialize_cmd);

	install_element (node, &dot1x_max_req_cmd);
	install_element (node, &no_dot1x_max_req_cmd);

	install_element (node, &dot1x_protocol_version_cmd);
	install_element (node, &no_dot1x_protocol_version_cmd);

	install_element (node, &dot1x_reauthentication_cmd);
	install_element (node, &no_dot1x_reauthentication_cmd);

	install_element (node, &dot1x_guest_vlan_cmd);
	install_element (node, &no_dot1x_guest_vlan_cmd);

	install_element (node, &dot1x_mac_auth_bypass_cmd);
	install_element (node, &no_dot1x_mac_auth_bypass_cmd);

	install_element (node, &dot1x_port_mode_cmd);
	install_element (node, &no_dot1x_port_mode_cmd);

	install_element (node, &dot1x_max_user_cmd);
	install_element (node, &no_dot1x_max_user_cmd);

	install_element (node, &dot1x_timeout_re_auth_cmd);
	install_element (node, &no_dot1x_timeout_re_auth_cmd);

	install_element (node, &dot1x_timeout_server_timeout_cmd);
	install_element (node, &no_dot1x_timeout_server_timeout_cmd);

	install_element (node, &dot1x_timeout_supp_timeout_cmd);
	install_element (node, &no_dot1x_timeout_supp_timeout_cmd);

	install_element (node, &dot1x_timeout_tx_period_cmd);
	install_element (node, &no_dot1x_timeout_tx_period_cmd);

	install_element (node, &dot1x_timeout_quiet_period_cmd);
	install_element (node, &no_dot1x_timeout_quiet_period_cmd);
}


void cmd_dot1x_init()
{
	struct nsm_client *nsm = nsm_client_new ();
	//nsm->interface_add_cb = nsm_vlan_add_interface;
	//nsm->interface_delete_cb = nsm_vlan_del_interface;

	nsm->write_config_cb = build_dot1x_config;
	nsm->interface_write_config_cb = build_dot1x_interface;
	nsm_client_install (nsm, NSM_DOT1X);


	install_element (CONFIG_NODE, &dot1x_system_auth_ctrl_cmd);
	install_element (CONFIG_NODE, &no_dot1x_system_auth_ctrl_cmd);

	cmd_dot1x_interface_init(INTERFACE_NODE);
}
