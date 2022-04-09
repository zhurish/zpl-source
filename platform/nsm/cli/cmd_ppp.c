/*
 * cmd_ppp.c
 *
 *  Created on: Aug 12, 2018
 *      Author: zhurish
 */



#include "auto_include.h"
#include <zplos_include.h>
#include "if.h"
#include "command.h"
#include "prefix.h"
#include "nsm_ppp.h"
#include "vty.h"

DEFUN (ppp_authentication,
		ppp_authentication_cmd,
		"ppp authentication (pap|chap|ms-chap|ms-chap-v2|eap)",
		"ppp options\n"
		"authentication configure\n"
		"pap authentication configure\n"
		"chap authentication configure\n")
{
	int ret = ERROR;
	int ageing;
	if (argc == 0)
		return CMD_SUCCESS;
	VTY_GET_INTEGER ("vlan ID", ageing, argv[0]);

	//ret = nsm_mac_ageing_time_set_api(ageing);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ppp_authentication,
		no_ppp_authentication_cmd,
		"ppp authentication (pap|chap|ms-chap|ms-chap-v2|eap)",
		NO_STR
		"ppp options\n"
		"authentication configure\n"
		"pap authentication configure\n"
		"chap authentication configure\n")
{
	int ret = ERROR;
	//ret = nsm_mac_ageing_time_set_api(0);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


//chap
DEFUN (ppp_chap_username,
		ppp_chap_username_cmd,
		"ppp chap username NAME",
		"ppp options\n"
		"chap configure\n"
		"username configure\n"
		"username\n")
{
	int ret = ERROR;
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ppp_chap_username,
		no_ppp_chap_username_cmd,
		"no ppp chap username",
		NO_STR
		"ppp options\n"
		"chap configure\n"
		"username configure\n")
{
	int ret = ERROR;
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ppp_chap_password,
		ppp_chap_password_cmd,
		"ppp chap password PASSWD",
		"ppp options\n"
		"chap configure\n"
		"password configure\n"
		"password\n")
{
	int ret = ERROR;
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ppp_chap_password,
		no_ppp_chap_password_cmd,
		"no ppp chap password",
		NO_STR
		"ppp options\n"
		"chap configure\n"
		"password configure\n")
{
	int ret = ERROR;
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ppp_chap_direction,
		ppp_chap_direction_cmd,
		"ppp direction callin",
		"ppp options\n"
		"chap direction configure\n"
		"callin\n")
{
	int ret = ERROR;
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ppp_chap_direction,
		no_ppp_chap_direction_cmd,
		"no ppp direction callin",
		NO_STR
		"ppp options\n"
		"chap direction configure\n"
		"callin\n")
{
	int ret = ERROR;
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ppp_chap_refuse,
		ppp_chap_refuse_cmd,
		"ppp chap refuse",
		"ppp options\n"
		"chap configure\n"
		"refuse\n")
{
	int ret = ERROR;
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(ppp_chap_refuse,
		ppp_chap_refuse_callin_cmd,
		"ppp chap refuse (callin|)",
		"ppp options\n"
		"chap configure\n"
		"refuse\n"
		"callin\n");

DEFUN (no_ppp_chap_refuse,
		no_ppp_chap_refuse_cmd,
		"no ppp chap refuse",
		NO_STR
		"ppp options\n"
		"chap configure\n"
		"refuse\n")
{
	int ret = ERROR;
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ppp_chap_wait,
		ppp_chap_wait_cmd,
		"ppp chap wait",
		"ppp options\n"
		"chap configure\n"
		"wait options\n")
{
	int ret = ERROR;
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ppp_chap_wait,
		no_ppp_chap_wait_cmd,
		"no ppp chap wait",
		NO_STR
		"ppp options\n"
		"chap configure\n"
		"wait options\n")
{
	int ret = ERROR;
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ppp_max_bad_auth,
		ppp_max_bad_auth_cmd,
		"ppp max-bad-auth <0-16>",
		"ppp options\n"
		"Max bad auth configure\n"
		"value\n")
{
	int ret = ERROR;
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ppp_max_bad_auth,
		no_ppp_max_bad_auth_cmd,
		"no ppp max-bad-auth",
		NO_STR
		"ppp options\n"
		"Max bad auth configure\n")
{
	int ret = ERROR;
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ppp_chap_splitnames,
		ppp_chap_splitnames_cmd,
		"ppp chap splitnames",
		"ppp options\n"
		"chap options\n"
		"splitnames configure\n")
{
	int ret = ERROR;
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ppp_chap_splitnames,
		no_ppp_chap_splitnames_cmd,
		"no ppp chap splitnames",
		NO_STR
		"ppp options\n"
		"chap options\n"
		"splitnames configure\n")
{
	int ret = ERROR;
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ppp_chap_ignoreus,
		ppp_chap_ignoreus_cmd,
		"ppp chap ignoreus",
		"ppp options\n"
		"chap options\n"
		"ignoreus configure\n")
{
	int ret = ERROR;
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ppp_chap_ignoreus,
		no_ppp_chap_ignoreus_cmd,
		"no ppp chap ignoreus",
		NO_STR
		"ppp options\n"
		"chap options\n"
		"ignoreus configure\n")
{
	int ret = ERROR;
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}



/*
 * pap
 */
DEFUN (ppp_pap_username,
		ppp_pap_username_cmd,
		"ppp pap sent-username NAME password PASSWD",
		"ppp options\n"
		"pap configure\n"
		"sent username\n"
		"username\n"
		"sent username password\n"
		"username password\n")
{
	int ret = ERROR;
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ppp_pap_username,
		no_ppp_pap_username_cmd,
		"no ppp pap sent-username",
		NO_STR
		"ppp options\n"
		"pap configure\n"
		"sent username\n")
{
	int ret = ERROR;
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ppp_negotiation_timeout,
		ppp_negotiation_timeout_cmd,
		"ppp negotiation timeout <1-3600>",
		"ppp options\n"
		"negotiation options\n"
		"timeout options for LCP\n"
		"timeout value\n")
{
	int ret = ERROR;
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ppp_negotiation_timeout,
		no_ppp_negotiation_timeout_cmd,
		"no ppp negotiation timeout",
		NO_STR
		"ppp options\n"
		"negotiation options\n"
		"timeout options for LCP\n")
{
	int ret = ERROR;
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}




//
DEFUN (peer_default_address,
		peer_default_address_cmd,
		"peer default ip address "CMD_KEY_IPV4,
		"peer options\n"
		"default configure\n"
		IP_STR
		"Set the IP address of an interface\n"
		CMD_KEY_IPV4_HELP)
{
	int ret = ERROR;
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_peer_default_address,
		no_peer_default_address_cmd,
		"no peer default ip address",
		NO_STR
		"peer options\n"
		"default configure\n"
		IP_STR
		"Set the IP address of an interface\n")
{
	int ret = ERROR;
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


//
DEFUN (ppp_compress,
		ppp_compress_cmd,
		"compress (predictor|stac)",
		"ppp compress options\n"
		"predictor compress configure\n"
		"stac compress configure\n")
{
	int ret = ERROR;
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ppp_compress,
		no_ppp_compress_cmd,
		"no compress",
		NO_STR
		"ppp compress options\n")
{
	int ret = ERROR;
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ppp_tcp_compress,
		ppp_tcp_compress_cmd,
		"ip tcp header-compress",
		"ppp compress options\n"
		"predictor compress configure\n"
		"stac compress configure\n")
{
	int ret = ERROR;
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ppp_tcp_compress,
		no_ppp_tcp_compress_cmd,
		"ip tcp header-compress",
		NO_STR
		"ppp compress options\n"
		"predictor compress configure\n"
		"stac compress configure\n")
{
	int ret = ERROR;
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


static void cmd_base_ppp_init(int node)
{
	install_element(node, CMD_CONFIG_LEVEL, &ppp_authentication_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ppp_authentication_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ppp_chap_username_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ppp_chap_username_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ppp_chap_password_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ppp_chap_password_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ppp_chap_direction_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ppp_chap_direction_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ppp_chap_refuse_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &ppp_chap_refuse_callin_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ppp_chap_refuse_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ppp_chap_wait_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ppp_chap_wait_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ppp_max_bad_auth_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ppp_max_bad_auth_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ppp_chap_splitnames_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ppp_chap_splitnames_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ppp_chap_ignoreus_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ppp_chap_ignoreus_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ppp_pap_username_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ppp_pap_username_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ppp_negotiation_timeout_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ppp_negotiation_timeout_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &peer_default_address_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_peer_default_address_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ppp_compress_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ppp_compress_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ppp_tcp_compress_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ppp_tcp_compress_cmd);
}

void cmd_ppp_init(void)
{
	cmd_base_ppp_init(SERIAL_INTERFACE_NODE);
}
