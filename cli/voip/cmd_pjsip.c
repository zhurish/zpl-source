/*
 * cmd_pjsip.c
 *
 *  Created on: 2019年6月20日
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

#ifdef PL_VOIP_MODULE

#include "pjsua_app_common.h"
#include "pjsua_app_config.h"
#include "pjsip_app_api.h"


/*
 * SIP Global
 */
int pl_pjsip_global_set_api(BOOL enable);
int pl_pjsip_global_get_api(BOOL *enable);

DEFUN (ip_sip_enable,
		ip_sip_enable_cmd,
		"ip sip server (enable|disable)",
		IP_STR
		"SIP Configure\n"
		"Server configure\n"
		"Enable\n"
		"Disable\n")
{
	int ret = ERROR;
	if(strstr(argv[0], "enable"))
	{
		if(!pl_pjsip_global_isenable())
			ret = pl_pjsip_global_set_api(TRUE);
		else
			ret = OK;
	}
	else
	{
		if(pl_pjsip_global_isenable())
			ret = pl_pjsip_global_set_api(FALSE);
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
	if(!pl_pjsip_global_isenable())
		ret = pl_pjsip_global_set_api(TRUE);
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
	if(pl_pjsip_global_isenable())
		ret = pl_pjsip_global_set_api(FALSE);
	else
		ret = OK;
	if(ret == OK)
		vty->node = SIP_SERVICE_NODE;
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

/*
 * SIP Local
 */
DEFUN (ip_sip_local_address,
		ip_sip_local_address_cmd,
		"ip sip local-address "CMD_KEY_IPV4,
		IP_STR
		"SIP Configure\n"
		"Local Address\n"
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
	ret = pl_pjsip_local_address_set_api(argv[0]);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_local_address,
		no_ip_sip_local_address_cmd,
		"no ip sip local-address",
		NO_STR
		IP_STR
		"SIP Configure\n"
		"Local Address\n")
{
	int ret = 0;
	ret = pl_pjsip_local_address_set_api(NULL);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_local_interface,
		ip_sip_local_interface_cmd,
		"ip sip source-interface "CMD_IF_USPV_STR " " CMD_USP_STR,
		IP_STR
		"SIP Configure\n"
		"Source Interface\n"
		CMD_IF_USPV_STR_HELP
		CMD_USP_STR_HELP)
{
	int ret = ERROR;
	struct interface *ifp;
	if (argv[0] && argv[1])
	{
		ifp = if_lookup_by_name (if_ifname_format(argv[0], argv[1]));
		if(!ifp)
		{
		}
		else
		{
			ret = pl_pjsip_source_interface_set_api(ifp->ifindex);
			return CMD_SUCCESS;
		}
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_local_interface,
		no_ip_sip_local_interface_cmd,
		"no ip sip source-interface",
		NO_STR
		IP_STR
		"SIP Configure\n"
		"Source Interface\n")
{
	int ret = 0;
	ret = pl_pjsip_source_interface_set_api(0);
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
	ret = pl_pjsip_local_port_set_api(value);
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
	ret = pl_pjsip_local_port_set_api(value);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}



/*
 * SIP Server
 */
DEFUN (ip_sip_server,
		ip_sip_server_cmd,
		"ip sip server "CMD_KEY_IPV4,
		IP_STR
		"SIP Configure\n"
		"Server configure\n"
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
		ret = pl_pjsip_server_set_api(argv[0], pl_pjsip->sip_server_sec.sip_port, TRUE);
	else
		ret = pl_pjsip_server_set_api(argv[0], pl_pjsip->sip_server.sip_port, FALSE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(ip_sip_server,
		ip_sip_server_sec_cmd,
		"ip sip server "CMD_KEY_IPV4 "(secondary|)",
		IP_STR
		"SIP Configure\n"
		"Server configure\n"
		CMD_KEY_IPV4_HELP
		"secondary server\n");

DEFUN (no_ip_sip_server,
		no_ip_sip_server_cmd,
		"no ip sip server",
		NO_STR
		IP_STR
		"SIP Configure\n"
		"Server configure\n")
{
	int ret = 0;
	if(argc == 1)
		ret = pl_pjsip_server_set_api(NULL, pl_pjsip->sip_server_sec.sip_port, TRUE);
	else
		ret = pl_pjsip_server_set_api(NULL, pl_pjsip->sip_server.sip_port, FALSE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(no_ip_sip_server,
		no_ip_sip_server_sec_cmd,
		"no ip sip server (secondary|)",
		NO_STR
		IP_STR
		"SIP Configure\n"
		"Server configure\n"
		"secondary  server\n");


DEFUN (ip_sip_server_port,
		ip_sip_server_port_cmd,
		"ip sip server port <256-65535>",
		IP_STR
		"SIP Configure\n"
		"Server configure\n"
		"SIP server port configure\n"
		"Port Value\n")
{
	int ret = ERROR;
	int value = 0;
	s_int8		sip_address[PJSIP_ADDRESS_MAX];
	value = atoi (argv[0]);
	if (value <= 0)
	{
		vty_out (vty, "%% Malformed port %s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(argc == 2)
	{
		memset(sip_address, 0, sizeof(sip_address));
		pl_pjsip_server_get_api(sip_address, NULL, TRUE);
		ret = pl_pjsip_server_set_api(sip_address, value, TRUE);
	}
	else
	{
		memset(sip_address, 0, sizeof(sip_address));
		pl_pjsip_server_get_api(sip_address, NULL, FALSE);
		ret = pl_pjsip_server_set_api(sip_address, value, FALSE);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(ip_sip_server_port,
		ip_sip_server_port_sec_cmd,
		"ip sip server port <256-65535> (secondary|)",
		IP_STR
		"SIP Configure\n"
		"Server configure\n"
		"SIP server port configure\n"
		"Port Value\n"
		"secondary server\n");

DEFUN (no_ip_sip_server_port,
		no_ip_sip_server_port_cmd,
		"no ip sip server port",
		NO_STR
		IP_STR
		"SIP Configure\n"
		"Server configure\n"
		"SIP server port configure\n")
{
	int ret = 0;
	s_int8		sip_address[PJSIP_ADDRESS_MAX];
	if(argc == 1)
	{
		memset(sip_address, 0, sizeof(sip_address));
		pl_pjsip_server_get_api(sip_address, NULL, TRUE);
		ret = pl_pjsip_server_set_api(sip_address, 0, TRUE);
	}
	else
	{
		memset(sip_address, 0, sizeof(sip_address));
		pl_pjsip_server_get_api(sip_address, NULL, FALSE);
		ret = pl_pjsip_server_set_api(sip_address, 0, FALSE);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(no_ip_sip_server_port,
		no_ip_sip_server_port_sec_cmd,
		"no ip sip server port (secondary|)",
		NO_STR
		IP_STR
		"SIP Configure\n"
		"Server configure\n"
		"SIP server port configure\n"
		"secondary server\n");


/*
 * SIP Proxy server
 */
DEFUN (ip_sip_proxy_server,
		ip_sip_proxy_server_cmd,
		"ip sip proxy-server "CMD_KEY_IPV4,
		IP_STR
		"SIP Configure\n"
		"Proxy Server configure\n"
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
		ret = pl_pjsip_proxy_set_api(argv[0], pl_pjsip->sip_proxy_sec.sip_port, TRUE);
	else
		ret = pl_pjsip_proxy_set_api(argv[0], pl_pjsip->sip_proxy.sip_port, FALSE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(ip_sip_proxy_server,
		ip_sip_proxy_server_sec_cmd,
		"ip sip proxy-server "CMD_KEY_IPV4 "(secondary|)",
		IP_STR
		"SIP Configure\n"
		"Proxy Server configure\n"
		CMD_KEY_IPV4_HELP
		"secondary server\n");

DEFUN (no_ip_sip_proxy_server,
		no_ip_sip_proxy_server_cmd,
		"no ip sip proxy-server",
		NO_STR
		IP_STR
		"SIP Configure\n"
		"Proxy Server configure\n")
{
	int ret = 0;
	if(argc == 1)
		ret = pl_pjsip_proxy_set_api(NULL, pl_pjsip->sip_proxy_sec.sip_port, TRUE);
	else
		ret = pl_pjsip_proxy_set_api(NULL, pl_pjsip->sip_proxy.sip_port, FALSE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(no_ip_sip_proxy_server,
		no_ip_sip_proxy_server_sec_cmd,
		"no ip sip proxy-server (secondary|)",
		NO_STR
		IP_STR
		"SIP Configure\n"
		"Proxy Server configure\n"
		"secondary  server\n");


DEFUN (ip_sip_proxy_server_port,
		ip_sip_proxy_server_port_cmd,
		"ip sip proxy-server port <256-65535>",
		IP_STR
		"SIP Configure\n"
		"Proxy Server configure\n"
		"SIP proxy server port configure\n"
		"Port Value\n")
{
	int ret = ERROR;
	int value = 0;
	s_int8		sip_address[PJSIP_ADDRESS_MAX];
	value = atoi (argv[0]);
	if (value <= 0)
	{
		vty_out (vty, "%% Malformed port %s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(argc == 2)
	{
		memset(sip_address, 0, sizeof(sip_address));
		pl_pjsip_proxy_get_api(sip_address, NULL, TRUE);
		ret = pl_pjsip_proxy_set_api(sip_address, value, TRUE);
	}
	else
	{
		memset(sip_address, 0, sizeof(sip_address));
		pl_pjsip_proxy_get_api(sip_address, NULL, FALSE);
		ret = pl_pjsip_proxy_set_api(sip_address, value, FALSE);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(ip_sip_proxy_server_port,
		ip_sip_proxy_server_port_sec_cmd,
		"ip sip proxy-server port <256-65535> (secondary|)",
		IP_STR
		"SIP Configure\n"
		"Proxy Server configure\n"
		"SIP proxy server port configure\n"
		"Port Value\n"
		"secondary server\n");

DEFUN (no_ip_sip_proxy_server_port,
		no_ip_sip_proxy_server_port_cmd,
		"no ip sip proxy-server port",
		NO_STR
		IP_STR
		"SIP Configure\n"
		"Proxy Server configure\n"
		"SIP proxy server port configure\n")
{
	int ret = 0;
	s_int8		sip_address[PJSIP_ADDRESS_MAX];
	if(argc == 1)
	{
		memset(sip_address, 0, sizeof(sip_address));
		pl_pjsip_proxy_get_api(sip_address, NULL, TRUE);
		ret = pl_pjsip_proxy_set_api(sip_address, 0, TRUE);
	}
	else
	{
		memset(sip_address, 0, sizeof(sip_address));
		pl_pjsip_proxy_get_api(sip_address, NULL, FALSE);
		ret = pl_pjsip_proxy_set_api(sip_address, 0, FALSE);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(no_ip_sip_proxy_server_port,
		no_ip_sip_proxy_server_port_sec_cmd,
		"no ip sip proxy-server port (secondary|)",
		NO_STR
		IP_STR
		"SIP Configure\n"
		"Proxy Server configure\n"
		"SIP proxy server port configure\n"
		"Secondary server\n");



/*
 * SIP stack
 */
DEFUN (ip_sip_transport_type,
		ip_sip_transport_type_cmd,
		"ip sip transport (udp|tcp|tls)",
		IP_STR
		"SIP configure\n"
		"Transport configure\n"
		"UDP Protocol\n"
		"TCP Protocol\n"
		"TLS Protocol\n")
{
	int ret = ERROR;
	pjsip_transport_t proto = PJSIP_PROTO_UDP;
	if(strstr(argv[0], "udp"))
	{
		proto = PJSIP_PROTO_UDP;
	}
	else if(strstr(argv[0], "tcp"))
	{
		proto = PJSIP_PROTO_TCP;
	}
	else if(strstr(argv[0], "dtls"))
	{
		proto = PJSIP_PROTO_DTLS;
	}
	else if(strstr(argv[0], "tls"))
	{
		proto = PJSIP_PROTO_TLS;
	}
	ret = pl_pjsip_transport_proto_set_api(proto);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_transport_type,
		no_ip_sip_transport_type_cmd,
		"no ip sip transport",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Transport configure\n")
{
	int ret = ERROR;
	ret = pl_pjsip_transport_proto_set_api(PJSIP_PROTO_UDP);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}




DEFUN (ip_sip_dtmf_type,
		ip_sip_dtmf_type_cmd,
		"ip sip dtmf-type (sip-info|rfc2833|inband)",
		IP_STR
		"SIP configure\n"
		"time-sync\n")
{
	int ret = ERROR;
	pjsip_dtmf_t dtmf = PJSIP_DTMF_INFO;
	if(strstr(argv[0], "info"))
	{
		dtmf = PJSIP_DTMF_INFO;
	}
	else if(strstr(argv[0], "rfc2833"))
	{
		dtmf = PJSIP_DTMF_RFC2833;
	}
	else if(strstr(argv[0], "inband"))
	{
		dtmf = PJSIP_DTMF_INBAND;
	}
	ret = pl_pjsip_dtmf_set_api(dtmf);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_dtmf_type,
		no_ip_sip_dtmf_type_cmd,
		"no ip sip dtmf-type",
		NO_STR
		IP_STR
		"SIP configure\n"
		"time-sync\n")
{
	int ret = ERROR;
	ret = pl_pjsip_dtmf_set_api(PJSIP_DTMF_INFO);
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
	ret = pl_pjsip_100rel_set_api(TRUE);
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
	ret = pl_pjsip_100rel_set_api(FALSE);
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
	ret = pl_pjsip_expires_set_api(value);
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
	ret = pl_pjsip_expires_set_api(value);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}



/*
 * SIP User
 */
/*DEFUN (ip_sip_phone,
		ip_sip_phone_cmd,
		"ip sip local-phone STRING",
		IP_STR
		"SIP configure\n"
		"local-phone configure\n"
		"phone number\n")
{
	int ret = ERROR;
	//char				sip_phone[PJSIP_NUMBER_MAX];
	//char				sip_user[PJSIP_USERNAME_MAX];
	char				sip_password[PJSIP_PASSWORD_MAX];
	if(argc == 2)
	{
		memset(sip_password, 0, sizeof(sip_password));
		pl_pjsip_username_get_api(NULL, sip_password, TRUE);
		ret = pl_pjsip_username_set_api(argv[0], sip_password, TRUE);
	}
	else
	{
		memset(sip_password, 0, sizeof(sip_password));
		pl_pjsip_username_get_api(NULL, sip_password, FALSE);
		ret = pl_pjsip_username_set_api(argv[0], sip_password, FALSE);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(ip_sip_phone,
		ip_sip_phone_sec_cmd,
		"ip sip local-phone STRING (secondary|)",
		NO_STR
		IP_STR
		"SIP configure\n"
		"local-phone configure\n"
		"phone number\n"
		"Secondary\n");

DEFUN (no_ip_sip_phone,
		no_ip_sip_phone_cmd,
		"no ip sip local-phone",
		NO_STR
		IP_STR
		"SIP configure\n"
		"local-phone configure\n")
{
	int ret = ERROR;
	char				sip_password[PJSIP_PASSWORD_MAX];
	if(argc == 1)
	{
		memset(sip_password, 0, sizeof(sip_password));
		pl_pjsip_username_get_api(NULL, sip_password, TRUE);
		ret = pl_pjsip_username_set_api(NULL, sip_password, TRUE);
	}
	else
	{
		memset(sip_password, 0, sizeof(sip_password));
		pl_pjsip_username_get_api(NULL, sip_password, FALSE);
		ret = pl_pjsip_username_set_api(NULL, sip_password, FALSE);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(no_ip_sip_phone,
		no_ip_sip_phone_sec_cmd,
		"no ip sip local-phone (secondary|)",
		NO_STR
		IP_STR
		"SIP configure\n"
		"local-phone configure\n"
		"Secondary\n");*/



DEFUN (ip_sip_username,
		ip_sip_username_cmd,
		"ip sip username STRING",
		IP_STR
		"SIP configure\n"
		"username configure\n"
		"username\n")
{
	int ret = ERROR;
	//char				sip_phone[PJSIP_NUMBER_MAX];
	//char				sip_user[PJSIP_USERNAME_MAX];
	char				sip_password[PJSIP_PASSWORD_MAX];
	if(argc == 2)
	{
		memset(sip_password, 0, sizeof(sip_password));
		pl_pjsip_username_get_api(NULL, sip_password, TRUE);
		ret = pl_pjsip_username_set_api(argv[0], sip_password, TRUE);
	}
	else
	{
		memset(sip_password, 0, sizeof(sip_password));
		pl_pjsip_username_get_api(NULL, sip_password, FALSE);
		ret = pl_pjsip_username_set_api(argv[0], sip_password, FALSE);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


ALIAS(ip_sip_username,
		ip_sip_username_sec_cmd,
		"ip sip username STRING (secondary|)",
		IP_STR
		"SIP configure\n"
		"username configure\n"
		"username\n"
		"Secondary\n");


DEFUN (no_ip_sip_username,
		no_ip_sip_username_cmd,
		"no ip sip username",
		NO_STR
		IP_STR
		"SIP configure\n"
		"username configure\n")
{
	int ret = ERROR;
	char				sip_password[PJSIP_PASSWORD_MAX];
	if(argc == 1)
	{
		memset(sip_password, 0, sizeof(sip_password));
		pl_pjsip_username_get_api(NULL, sip_password, TRUE);
		ret = pl_pjsip_username_set_api(NULL, sip_password, TRUE);
	}
	else
	{
		memset(sip_password, 0, sizeof(sip_password));
		pl_pjsip_username_get_api(NULL, sip_password, FALSE);
		ret = pl_pjsip_username_set_api(NULL, sip_password, FALSE);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(no_ip_sip_username,
		no_ip_sip_username_sec_cmd,
		"no ip sip username (secondary|)",
		NO_STR
		IP_STR
		"SIP configure\n"
		"username configure\n"
		"Secondary\n");


DEFUN (ip_sip_password,
		ip_sip_password_cmd,
		"ip sip password STRING",
		IP_STR
		"SIP configure\n"
		"password configure\n"
		"password\n")
{
	int ret = ERROR;
	char				sip_phone[PJSIP_NUMBER_MAX];
	//char				sip_user[PJSIP_USERNAME_MAX];
	//char				sip_password[PJSIP_PASSWORD_MAX];
	if(argc == 2)
	{
		memset(sip_phone, 0, sizeof(sip_phone));
		pl_pjsip_username_get_api(sip_phone, NULL, TRUE);
		ret = pl_pjsip_username_set_api(sip_phone, argv[0], TRUE);
	}
	else
	{
		memset(sip_phone, 0, sizeof(sip_phone));
		pl_pjsip_username_get_api(sip_phone, NULL, FALSE);
		ret = pl_pjsip_username_set_api(sip_phone, argv[0], FALSE);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(ip_sip_password,
		ip_sip_password_sec_cmd,
		"ip sip password STRING (secondary|)",
		IP_STR
		"SIP configure\n"
		"password configure\n"
		"password\n"
		"Secondary\n");

DEFUN (no_ip_sip_password,
		no_ip_sip_password_cmd,
		"no ip sip password",
		NO_STR
		IP_STR
		"SIP configure\n"
		"password configure\n")
{
	int ret = ERROR;
	char				sip_phone[PJSIP_NUMBER_MAX];
	//char				sip_user[PJSIP_USERNAME_MAX];
	//char				sip_password[PJSIP_PASSWORD_MAX];
	if(argc == 1)
	{
		memset(sip_phone, 0, sizeof(sip_phone));
		pl_pjsip_username_get_api(sip_phone, NULL, TRUE);
		ret = pl_pjsip_username_set_api(sip_phone, NULL, TRUE);
	}
	else
	{
		memset(sip_phone, 0, sizeof(sip_phone));
		pl_pjsip_username_get_api(sip_phone, NULL, FALSE);
		ret = pl_pjsip_username_set_api(sip_phone, NULL, FALSE);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(no_ip_sip_password,
		no_ip_sip_password_sec_cmd,
		"no ip sip password (secondary|)",
		NO_STR
		IP_STR
		"SIP configure\n"
		"password configure\n"
		"Secondary\n");



/*
 * SIP Misc
 */
DEFUN (ip_sip_realm,
		ip_sip_realm_cmd,
		"ip sip realm STRING",
		IP_STR
		"SIP configure\n"
		"realm configure\n"
		"STRING\n")
{
	int ret = ERROR;
	ret = pl_pjsip_realm_set_api(argv[0]);

	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

/*
ALIAS(ip_sip_realm,
		ip_sip_realm_sec_cmd,
		"ip sip realm STRING (secondary|)",
		IP_STR
		"SIP configure\n"
		"realm configure\n"
		"STRING\n"
		"Secondary\n");
*/


DEFUN (no_ip_sip_realm,
		no_ip_sip_realm_cmd,
		"no ip sip realm",
		NO_STR
		IP_STR
		"SIP configure\n"
		"realm configure\n")
{
	int ret = ERROR;
/*	if(argc == 1)
		ret = voip_sip_realm_set_api(NULL, TRUE);
	else*/
		ret = pl_pjsip_realm_set_api(NULL);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

/*
ALIAS(no_ip_sip_realm,
		no_ip_sip_realm_sec_cmd,
		"no ip sip realm (secondary|)",
		IP_STR
		"SIP configure\n"
		"realm configure\n"
		"Secondary\n");
*/




DEFUN (pjsip_call_stop_cli,
		pjsip_call_stop_cli_cmd,
		"pjsip-call stop STRING",
		"SIP Register\n"
		"Stop\n"
		"PhoneNumber\n")
{
	pl_pjsip_app_stop_call(app_config.current_call, FALSE);
	return  CMD_SUCCESS;
}

DEFUN (pjsip_call_start_cli,
		pjsip_call_start_cli_cmd,
		"pjsip-call start STRING",
		"SIP Register\n"
		"Start\n"
		"PhoneNumber\n")
{
	pl_pjsip_app_start_call(current_acc, argv[0], NULL);
	return  CMD_SUCCESS;
}

/*
 * SIP show
 */
DEFUN (show_ip_sip_server,
		show_ip_sip_server_cmd,
		"show sip service",
		SHOW_STR
		"SIP configure\n"
		"Service\n")
{
	pl_pjsip_show_config(vty, FALSE);
	return CMD_SUCCESS;
}

DEFUN (show_ip_sip_state,
		show_ip_sip_state_cmd,
		"show ip sip state",
		IP_STR
		SHOW_STR
		"SIP configure\n"
		"State\n")
{
	pl_pjsip_show_account_state(vty);
	return CMD_SUCCESS;
}


static void cmd_base_sip_init(int node)
{
	install_element(node, &ip_sip_local_address_cmd);
	install_element(node, &no_ip_sip_local_address_cmd);

	install_element(node, &ip_sip_local_interface_cmd);
	install_element(node, &no_ip_sip_local_interface_cmd);

	install_element(node, &ip_sip_port_cmd);
	install_element(node, &no_ip_sip_port_cmd);

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


/*
	install_element(node, &ip_sip_multiuser_cmd);
	install_element(node, &no_ip_sip_multiuser_cmd);
	install_element(node, &ip_sip_active_standby_cmd);
	install_element(node, &no_ip_sip_active_standby_cmd);

	install_element(node, &ip_sip_proxy_cmd);
	install_element(node, &no_ip_sip_proxy_cmd);
	install_element(node, &ip_sip_keepalive_cmd);
	install_element(node, &no_ip_sip_keepalive_cmd);

	install_element(node, &ip_sip_keepalive_interval_cmd);
	install_element(node, &no_ip_sip_keepalive_interval_cmd);
*/

/*	install_element(node, &ip_sip_phone_cmd);
	install_element(node, &no_ip_sip_phone_cmd);
	install_element(node, &ip_sip_phone_sec_cmd);
	install_element(node, &no_ip_sip_phone_sec_cmd);*/


	install_element(node, &ip_sip_username_cmd);
	install_element(node, &no_ip_sip_username_cmd);
	install_element(node, &ip_sip_username_sec_cmd);
	install_element(node, &no_ip_sip_username_sec_cmd);


	install_element(node, &ip_sip_password_cmd);
	install_element(node, &no_ip_sip_password_cmd);
	install_element(node, &ip_sip_password_sec_cmd);
	install_element(node, &no_ip_sip_password_sec_cmd);

	install_element(node, &ip_sip_dtmf_type_cmd);
	install_element(node, &no_ip_sip_dtmf_type_cmd);

	install_element(node, &ip_sip_transport_type_cmd);
	install_element(node, &no_ip_sip_transport_type_cmd);

/*
	install_element(node, &ip_sip_payload_cmd);
	install_element(node, &no_ip_sip_payload_cmd);
*/

	install_element(node, &ip_sip_register_interval_cmd);
	install_element(node, &no_ip_sip_register_interval_cmd);

	install_element(node, &ip_sip_realm_cmd);
	install_element(node, &no_ip_sip_realm_cmd);
	//install_element(node, &ip_sip_realm_sec_cmd);
	//install_element(node, &no_ip_sip_realm_sec_cmd);

	install_element(node, &ip_sip_100_rel_cmd);
	install_element(node, &no_ip_sip_100_rel_cmd);

#if 0
	install_element(node, &ip_sip_keep_interval_cmd);
	install_element(node, &no_ip_sip_keep_interval_cmd);

	install_element(node, &ip_sip_display_name_cmd);
	install_element(node, &no_ip_sip_display_name_cmd);

	install_element(node, &ip_sip_time_sync_cmd);
	install_element(node, &no_ip_sip_time_sync_cmd);

	install_element(node, &ip_sip_ring_cmd);
	install_element(node, &no_ip_sip_ring_cmd);

	install_element(node, &ip_sip_hostpart_cmd);
	install_element(node, &no_ip_sip_hostpart_cmd);

	install_element(node, &ip_sip_dialplan_cmd);
	install_element(node, &no_ip_sip_dialplan_cmd);

	install_element(node, &ip_sip_encrypt_cmd);
	install_element(node, &no_ip_sip_encrypt_cmd);
#endif
}

static void cmd_show_sip_init(int node)
{
	install_element(node, &show_ip_sip_server_cmd);
	install_element(node, &show_ip_sip_state_cmd);

	install_element(node, &pjsip_call_start_cli_cmd);
	install_element(node, &pjsip_call_stop_cli_cmd);
	//install_element(node, &show_debugging_sip_cmd);
}

void cmd_voip_init(void)
{
	install_default(SIP_SERVICE_NODE);
	install_default_basic(SIP_SERVICE_NODE);

	reinstall_node(SIP_SERVICE_NODE, pl_pjsip_write_config);

	install_element(CONFIG_NODE, &ip_sip_enable_cmd);
	install_element(CONFIG_NODE, &ip_sip_service_cmd);
	install_element(CONFIG_NODE, &no_ip_sip_service_cmd);

	cmd_base_sip_init(SIP_SERVICE_NODE);

	cmd_show_sip_init(ENABLE_NODE);
	cmd_show_sip_init(CONFIG_NODE);
	cmd_show_sip_init(SIP_SERVICE_NODE);

	cmd_voip_test_init(ENABLE_NODE);
	//install_element(VIEW_NODE, &debug_ip_sip_cmd);
	//install_element(VIEW_NODE, &no_debug_ip_sip_cmd);
}
#endif/* PL_VOIP_MODULE */
