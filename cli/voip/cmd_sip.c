/*
 * cmd_sip.c
 *
 *  Created on: 2018��12��18��
 *      Author: DELL
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
#include "vrf.h"
#include "interface.h"

#include "voip_api.h"
#include "voip_sip.h"


/*
 * SIP Module
 */
//server ip
DEFUN (ip_sip_server,
		ip_sip_server_cmd,
		"ip sip-server "CMD_KEY_IPV4,
		IP_STR
		"SIP server configure\n"
		CMD_KEY_IPV4_HELP)
{
	int ret = ERROR;
	struct prefix address;
	prefix_zero(&address);
	ret = str2prefix_ipv4 (argv[0], (struct prefix_ipv4 *)&address);
	if (ret <= 0)
	{
		vty_out (vty, "%% Malformed address %s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(argc == 2)
		ret = voip_sip_server_set_api(ntohl(address.u.prefix4.s_addr), voip_sip_config.sip_port_sec, TRUE);
	else
		ret = voip_sip_server_set_api(ntohl(address.u.prefix4.s_addr), voip_sip_config.sip_port, FALSE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(ip_sip_server,
		ip_sip_server_sec_cmd,
		"ip sip-server "CMD_KEY_IPV4 "(secondary|)",
		IP_STR
		"SIP server configure\n"
		CMD_KEY_IPV4_HELP
		"secondary server\n");

DEFUN (no_ip_sip_server,
		no_ip_sip_server_cmd,
		"no ip sip-server",
		NO_STR
		IP_STR
		"SIP server configure\n")
{
	int ret = 0;
	if(argc == 1)
		ret = voip_sip_server_set_api(0, voip_sip_config.sip_port_sec, TRUE);
	else
		ret = voip_sip_server_set_api(0, voip_sip_config.sip_port, FALSE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(no_ip_sip_server,
		no_ip_sip_server_sec_cmd,
		"no ip sip-server (secondary|)",
		NO_STR
		IP_STR
		"SIP server configure\n"
		"secondary  server\n");


//server port
DEFUN (ip_sip_server_port,
		ip_sip_server_port_cmd,
		"ip sip-server port <256-65535>",
		IP_STR
		"SIP server configure\n"
		"SIP server port configure\n"
		CMD_KEY_IPV4_HELP)
{
	int ret = ERROR;
	int value = 0;
	value = atoi (argv[0]);
	if (value <= 0)
	{
		vty_out (vty, "%% Malformed port %s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(argc == 2)
		ret = voip_sip_server_set_api(voip_sip_config.sip_server_sec, value, TRUE);
	else
		ret = voip_sip_server_set_api(voip_sip_config.sip_server, value, FALSE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(ip_sip_server_port,
		ip_sip_server_port_sec_cmd,
		"ip sip-server port (secondary|)",
		IP_STR
		"SIP server configure\n"
		"SIP server port configure\n"
		CMD_KEY_IPV4_HELP
		"secondary server\n");

DEFUN (no_ip_sip_server_port,
		no_ip_sip_server_port_cmd,
		"no ip sip-server port",
		NO_STR
		IP_STR
		"SIP server configure\n"
		"SIP server port configure\n")
{
	int ret = 0;
	if(argc == 1)
		ret = voip_sip_server_set_api(voip_sip_config.sip_server_sec, 0, TRUE);
	else
		ret = voip_sip_server_set_api(voip_sip_config.sip_server, 0, FALSE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(no_ip_sip_server_port,
		no_ip_sip_server_port_sec_cmd,
		"no ip sip-server port (secondary|)",
		NO_STR
		IP_STR
		"SIP server configure\n"
		"SIP server port configure\n"
		"secondary server\n");


//proxy server ip
DEFUN (ip_sip_proxy_server,
		ip_sip_proxy_server_cmd,
		"ip sip-proxy-server "CMD_KEY_IPV4,
		IP_STR
		"SIP proxy server configure\n"
		CMD_KEY_IPV4_HELP)
{
	int ret = ERROR;
	struct prefix address;
	prefix_zero(&address);
	ret = str2prefix_ipv4 (argv[0], (struct prefix_ipv4 *)&address);
	if (ret <= 0)
	{
		vty_out (vty, "%% Malformed address %s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(argc == 2)
		ret = voip_sip_proxy_server_set_api(ntohl(address.u.prefix4.s_addr), voip_sip_config.sip_port_sec, TRUE);
	else
		ret = voip_sip_proxy_server_set_api(ntohl(address.u.prefix4.s_addr), voip_sip_config.sip_port, FALSE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(ip_sip_proxy_server,
		ip_sip_proxy_server_sec_cmd,
		"ip sip-proxy-server "CMD_KEY_IPV4 "(secondary|)",
		IP_STR
		"SIP proxy server configure\n"
		CMD_KEY_IPV4_HELP
		"secondary server\n");

DEFUN (no_ip_sip_proxy_server,
		no_ip_sip_proxy_server_cmd,
		"no ip sip-proxy-server",
		NO_STR
		IP_STR
		"SIP proxy server configure\n")
{
	int ret = 0;
	if(argc == 1)
		ret = voip_sip_proxy_server_set_api(0, voip_sip_config.sip_port_sec, TRUE);
	else
		ret = voip_sip_proxy_server_set_api(0, voip_sip_config.sip_port, FALSE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(no_ip_sip_proxy_server,
		no_ip_sip_proxy_server_sec_cmd,
		"no ip sip-proxy-server (secondary|)",
		NO_STR
		IP_STR
		"SIP proxy server configure\n"
		"secondary  server\n");


//proxy server port
DEFUN (ip_sip_proxy_server_port,
		ip_sip_proxy_server_port_cmd,
		"ip sip-proxy-server port <256-65535>",
		IP_STR
		"SIP proxy server configure\n"
		"SIP proxy server port configure\n"
		CMD_KEY_IPV4_HELP)
{
	int ret = ERROR;
	int value = 0;
	value = atoi (argv[0]);
	if (value <= 0)
	{
		vty_out (vty, "%% Malformed port %s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(argc == 2)
		ret = voip_sip_proxy_server_set_api(voip_sip_config.sip_proxy_server_sec, value, TRUE);
	else
		ret = voip_sip_proxy_server_set_api(voip_sip_config.sip_proxy_server, value, FALSE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(ip_sip_proxy_server_port,
		ip_sip_proxy_server_port_sec_cmd,
		"ip sip-proxy-server port (secondary|)",
		IP_STR
		"SIP proxy server configure\n"
		"SIP proxy server port configure\n"
		CMD_KEY_IPV4_HELP
		"secondary server\n");

DEFUN (no_ip_sip_proxy_server_port,
		no_ip_sip_proxy_server_port_cmd,
		"no ip sip-proxy-server port",
		NO_STR
		IP_STR
		"SIP proxy server configure\n"
		"SIP proxy server port configure\n")
{
	int ret = 0;
	if(argc == 1)
		ret = voip_sip_proxy_server_set_api(voip_sip_config.sip_proxy_server_sec, 0, TRUE);
	else
		ret = voip_sip_proxy_server_set_api(voip_sip_config.sip_proxy_server, 0, FALSE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(no_ip_sip_proxy_server_port,
		no_ip_sip_proxy_server_port_sec_cmd,
		"no ip sip-proxy-server port (secondary|)",
		NO_STR
		IP_STR
		"SIP proxy server configure\n"
		"SIP proxy server port configure\n"
		"secondary server\n");




DEFUN (ip_sip_enable,
		ip_sip_enable_cmd,
		"ip sip-server (enable|disable)",
		IP_STR
		"SIP server configure\n"
		"Enable\n"
		"Disable\n")
{
	int ret = ERROR;
	if(strstr(argv[0], "enable"))
	{
		if(!voip_sip_config.sip_enable)
			ret = voip_sip_enable(TRUE, voip_sip_config.sip_local_port);
		else
			ret = OK;
	}
	else
	{
		if(voip_sip_config.sip_enable)
			ret = voip_sip_enable(FALSE, voip_sip_config.sip_local_port);
		else
			ret = OK;
	}
	if(ret == OK)
		vty->node = SIP_SERVICE_NODE;
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_service,
		ip_sip_service_cmd,
		"service sip",
		"Service configure\n"
		"SIP Protocol\n")
{
	int ret = ERROR;
	if(!voip_sip_config.sip_enable)
		ret = voip_sip_enable(TRUE, voip_sip_config.sip_local_port);
	else
		ret = OK;
	if(ret == OK)
		vty->node = SIP_SERVICE_NODE;
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_service,
		no_ip_sip_service_cmd,
		"no service sip",
		NO_STR
		"Service configure\n"
		"SIP Protocol\n")
{
	int ret = ERROR;
	if(voip_sip_config.sip_enable)
		ret = voip_sip_enable(FALSE, voip_sip_config.sip_local_port);
	else
		ret = OK;
	if(ret == OK)
		vty->node = SIP_SERVICE_NODE;
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_sip_port,
		ip_sip_port_cmd,
		"ip sip local-port <256-65535>",
		IP_STR
		"SIP configure\n"
		"Local port\n"
		"port number\n")
{
	int ret = ERROR, value = 0;
	value = atoi(argv[0]);
	ret = voip_sip_enable(voip_sip_config.sip_enable, value);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_port,
		no_ip_sip_port_cmd,
		"no ip sip local-port",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Local port\n")
{
	int ret = ERROR, value = 0;
	ret = voip_sip_enable(voip_sip_config.sip_enable, value);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}



DEFUN (ip_sip_time_sync,
		ip_sip_time_sync_cmd,
		"ip sip time-sync",
		IP_STR
		"SIP configure\n"
		"time-sync\n")
{
	int ret = ERROR;
	ret = voip_sip_time_syne_set_api(TRUE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_time_sync,
		no_ip_sip_time_sync_cmd,
		"no ip sip time-sync",
		NO_STR
		IP_STR
		"SIP configure\n"
		"time-sync\n")
{
	int ret = ERROR;
	ret = voip_sip_time_syne_set_api(FALSE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_sip_ring,
		ip_sip_ring_cmd,
		"ip sip ring <0-16>",
		IP_STR
		"SIP configure\n"
		"Ring Confiure\n"
		"ring number\n")
{
	int ret = ERROR, value = 0;
	value = atoi(argv[0]);
	ret = voip_sip_ring_set_api(value);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_ring,
		no_ip_sip_ring_cmd,
		"no ip sip ring",
		NO_STR
		IP_STR
		"SIP configure\n"
		"ring configure\n")
{
	int ret = ERROR, value = 0;
	ret = voip_sip_ring_set_api(value);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_sip_100_rel,
		ip_sip_100_rel_cmd,
		"ip sip rel-100",
		IP_STR
		"SIP configure\n"
		"rel-100\n")
{
	int ret = ERROR;
	ret = voip_sip_100_rel_set_api(TRUE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_100_rel,
		no_ip_sip_100_rel_cmd,
		"no ip sip rel-100",
		NO_STR
		IP_STR
		"SIP configure\n"
		"rel-100\n")
{
	int ret = ERROR;
	ret = voip_sip_100_rel_set_api(FALSE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_display_name,
		ip_sip_display_name_cmd,
		"ip sip display-name",
		IP_STR
		"SIP configure\n"
		"display name\n")
{
	int ret = ERROR;
	ret = voip_sip_display_name_set_api(TRUE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_display_name,
		no_ip_sip_display_name_cmd,
		"no ip sip display-name",
		NO_STR
		IP_STR
		"SIP configure\n"
		"display name\n")
{
	int ret = ERROR;
	ret = voip_sip_display_name_set_api(FALSE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_sip_register_interval,
		ip_sip_register_interval_cmd,
		"ip sip register-interval <15-3600>",
		IP_STR
		"SIP configure\n"
		"register interval\n"
		"interval value\n")
{
	int ret = ERROR, value = 0;
	value = atoi(argv[0]);
	ret = voip_sip_register_interval_set_api(value);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_register_interval,
		no_ip_sip_register_interval_cmd,
		"no ip sip register-interval",
		NO_STR
		IP_STR
		"SIP configure\n"
		"register interval configure\n")
{
	int ret = ERROR, value = 0;
	ret = voip_sip_register_interval_set_api(value);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_sip_keep_interval,
		ip_sip_keep_interval_cmd,
		"ip sip keep-interval <15-3600>",
		IP_STR
		"SIP configure\n"
		"keep interval\n"
		"interval value\n")
{
	int ret = ERROR, value = 0;
	value = atoi(argv[0]);
	ret = voip_sip_interval_set_api(value);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_keep_interval,
		no_ip_sip_keep_interval_cmd,
		"no ip sip keep-interval",
		NO_STR
		IP_STR
		"SIP configure\n"
		"keep interval configure\n")
{
	int ret = ERROR, value = 0;
	ret = voip_sip_interval_set_api(value);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_hostpart,
		ip_sip_hostpart_cmd,
		"ip sip hostpart STRING",
		IP_STR
		"SIP configure\n"
		"hostpart\n"
		"hostpart value\n")
{
	int ret = ERROR;
	ret = voip_sip_hostpart_set_api(argv[0]);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_hostpart,
		no_ip_sip_hostpart_cmd,
		"no ip sip hostpart",
		NO_STR
		IP_STR
		"SIP configure\n"
		"hostpart configure\n")
{
	int ret = ERROR;
	ret = voip_sip_hostpart_set_api(NULL);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_dialplan,
		ip_sip_dialplan_cmd,
		"ip sip dialplan STRING",
		IP_STR
		"SIP configure\n"
		"dialplan\n"
		"value\n")
{
	int ret = ERROR;
	ret = voip_sip_dialplan_set_api(argv[0]);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_dialplan,
		no_ip_sip_dialplan_cmd,
		"no ip sip dialplan",
		NO_STR
		IP_STR
		"SIP configure\n"
		"dialplan configure\n")
{
	int ret = ERROR;
	ret = voip_sip_dialplan_set_api(NULL);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_phone,
		ip_sip_phone_cmd,
		"ip sip local-phone STRING",
		IP_STR
		"SIP configure\n"
		"local-phone configure\n"
		"phone number\n")
{
	int ret = ERROR;
	ret = voip_sip_local_number_set_api(argv[0]);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_phone,
		no_ip_sip_phone_cmd,
		"no ip sip local-phone",
		NO_STR
		IP_STR
		"SIP configure\n"
		"local-phone configure\n")
{
	int ret = ERROR;
	ret = voip_sip_local_number_set_api(NULL);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_username,
		ip_sip_username_cmd,
		"ip sip username STRING",
		IP_STR
		"SIP configure\n"
		"username configure\n"
		"username\n")
{
	int ret = ERROR;
	ret = voip_sip_user_set_api(argv[0]);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_username,
		no_ip_sip_username_cmd,
		"no ip sip username",
		NO_STR
		IP_STR
		"SIP configure\n"
		"username configure\n")
{
	int ret = ERROR;
	ret = voip_sip_user_set_api(NULL);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_password,
		ip_sip_password_cmd,
		"ip sip password STRING",
		IP_STR
		"SIP configure\n"
		"password configure\n"
		"password\n")
{
	int ret = ERROR;
	ret = voip_sip_password_set_api(argv[0]);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_password,
		no_ip_sip_password_cmd,
		"no ip sip password",
		NO_STR
		IP_STR
		"SIP configure\n"
		"password configure\n")
{
	int ret = ERROR;
	ret = voip_sip_password_set_api(NULL);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_realm,
		ip_sip_realm_cmd,
		"ip sip realm STRING",
		IP_STR
		"SIP configure\n"
		"realm configure\n"
		"STRING\n")
{
	int ret = ERROR;
	ret = voip_sip_realm_set_api(argv[0]);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_realm,
		no_ip_sip_realm_cmd,
		"no ip sip realm",
		NO_STR
		IP_STR
		"SIP configure\n"
		"realm configure\n")
{
	int ret = ERROR;
	ret = voip_sip_realm_set_api(NULL);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_encrypt,
		ip_sip_encrypt_cmd,
		"ip sip encrypt",
		IP_STR
		"SIP configure\n"
		"encrypt\n")
{
	int ret = ERROR;
	ret = voip_sip_encrypt_set_api(TRUE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_encrypt,
		no_ip_sip_encrypt_cmd,
		"no ip sip encrypt",
		NO_STR
		IP_STR
		"SIP configure\n"
		"encrypt\n")
{
	int ret = ERROR;
	ret = voip_sip_encrypt_set_api(FALSE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (show_ip_sip_server,
		show_ip_sip_server_cmd,
		"show sip service",
		SHOW_STR
		"SIP configure\n"
		"Service\n")
{
	voip_sip_show_config(vty, FALSE);
	return CMD_SUCCESS;
}


DEFUN (sip_test_cmd,
		sip_test_cmd_cmd,
		"sip-test (register-start|register-stop|call-stop|STRING)",
		"SIP configure\n"
		"encrypt\n")
{
	//int ret = ERROR;
	//if(argc == 1)
	//{
		if(strstr(argv[0], "retister-start"))
			voip_sip_register_start(TRUE);
		else if(strstr(argv[0], "retister-start"))
			voip_sip_register_start(FALSE);
		else if(strstr(argv[0], "call-stop"))
			voip_sip_call_stop(FALSE);
	//}
	//else
	//{
		else
			voip_sip_call_start(argv[0]);
	//}
	return  CMD_SUCCESS;
}



static void cmd_base_sip_init(int node)
{
	install_element(node, &ip_sip_server_cmd);
	install_element(node, &no_ip_sip_server_cmd);
	install_element(node, &ip_sip_server_sec_cmd);
	install_element(node, &no_ip_sip_server_sec_cmd);

	install_element(node, &ip_sip_server_port_cmd);
	install_element(node, &ip_sip_server_port_sec_cmd);
	install_element(node, &no_ip_sip_server_port_cmd);
	install_element(node, &no_ip_sip_server_port_sec_cmd);

	install_element(node, &ip_sip_proxy_server_cmd);
	install_element(node, &ip_sip_proxy_server_sec_cmd);
	install_element(node, &no_ip_sip_proxy_server_cmd);
	install_element(node, &no_ip_sip_proxy_server_sec_cmd);

	install_element(node, &ip_sip_proxy_server_port_cmd);
	install_element(node, &ip_sip_proxy_server_port_sec_cmd);
	install_element(node, &no_ip_sip_proxy_server_port_cmd);
	install_element(node, &no_ip_sip_proxy_server_port_sec_cmd);


	install_element(node, &ip_sip_port_cmd);
	install_element(node, &no_ip_sip_port_cmd);

	install_element(node, &ip_sip_time_sync_cmd);
	install_element(node, &no_ip_sip_time_sync_cmd);

	install_element(node, &ip_sip_ring_cmd);
	install_element(node, &no_ip_sip_ring_cmd);

	install_element(node, &ip_sip_100_rel_cmd);
	install_element(node, &no_ip_sip_100_rel_cmd);

	install_element(node, &ip_sip_display_name_cmd);
	install_element(node, &no_ip_sip_display_name_cmd);
	install_element(node, &ip_sip_register_interval_cmd);
	install_element(node, &no_ip_sip_register_interval_cmd);


	install_element(node, &ip_sip_keep_interval_cmd);
	install_element(node, &no_ip_sip_keep_interval_cmd);
	install_element(node, &ip_sip_hostpart_cmd);
	install_element(node, &no_ip_sip_hostpart_cmd);

	install_element(node, &ip_sip_dialplan_cmd);
	install_element(node, &no_ip_sip_dialplan_cmd);
	install_element(node, &ip_sip_phone_cmd);
	install_element(node, &no_ip_sip_phone_cmd);

	install_element(node, &ip_sip_username_cmd);
	install_element(node, &no_ip_sip_username_cmd);

	install_element(node, &ip_sip_password_cmd);
	install_element(node, &no_ip_sip_password_cmd);

	install_element(node, &ip_sip_realm_cmd);
	install_element(node, &no_ip_sip_realm_cmd);

	install_element(node, &ip_sip_encrypt_cmd);
	install_element(node, &no_ip_sip_encrypt_cmd);
}

static void cmd_show_sip_init(int node)
{
	install_element(node, &show_ip_sip_server_cmd);
	install_element(node, &sip_test_cmd_cmd);
}

void cmd_sip_init(void)
{
	install_default(SIP_SERVICE_NODE);
	install_default_basic(SIP_SERVICE_NODE);

	reinstall_node(SIP_SERVICE_NODE, voip_sip_write_config);

	install_element(CONFIG_NODE, &ip_sip_enable_cmd);
	install_element(CONFIG_NODE, &ip_sip_service_cmd);
	install_element(CONFIG_NODE, &no_ip_sip_service_cmd);

	cmd_base_sip_init(SIP_SERVICE_NODE);

	cmd_show_sip_init(ENABLE_NODE);
	cmd_show_sip_init(CONFIG_NODE);
	cmd_show_sip_init(SIP_SERVICE_NODE);
}
