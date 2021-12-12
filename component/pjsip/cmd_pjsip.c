/*
 * cmd_pjsip.c
 *
 *  Created on: 2019年6月20日
 *      Author: DELL
 */



#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"
#include "vty_include.h"

#ifdef ZPL_PJSIP_MODULE
#include "voip_app.h"
#include "pjsua_app_common.h"
#include "pjsua_app_config.h"
#include "pjsip_app_api.h"


/*
 * SIP Global
 */
//int pl_pjsip_global_set_api(zpl_bool enable);
//int pl_pjsip_global_get_api(zpl_bool *enable);
static int pjsip_write_config(struct vty *vty, void *pVoid);


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
			ret = pl_pjsip_global_set_api(zpl_true);
		else
			ret = OK;
	}
	else
	{
		if(pl_pjsip_global_isenable())
			ret = pl_pjsip_global_set_api(zpl_false);
		else
			ret = OK;
	}
	if(ret == OK)
		vty->node = ALL_SERVICE_NODE;
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_service,
		ip_sip_service_cmd,
		"service sip",
		"Service configure\n"
		"SIP Protocol\n")
{
	template_t * temp = nsm_template_lookup_name (zpl_true, "service pjsip");
	if(temp)
	{
		if(!pl_pjsip_global_isenable())
			pl_pjsip_global_set_api(zpl_true);
		vty->node = ALL_SERVICE_NODE;
		memset(vty->prompt, 0, sizeof(vty->prompt));
		sprintf(vty->prompt, "%s", temp->prompt);
		return CMD_SUCCESS;
	}
	else
	{
		temp = nsm_template_new (zpl_true);
		if(temp)
		{
			temp->module = 0;
			strcpy(temp->name, "service pjsip");
			strcpy(temp->prompt, "service-sip"); /* (config-app-esp)# */
			temp->write_template = pjsip_write_config;
			//temp->pVoid = v9_video_app_tmp();
			nsm_template_install(temp, 0);
			if(!pl_pjsip_global_isenable())
				pl_pjsip_global_set_api(zpl_true);
			vty->node = ALL_SERVICE_NODE;
			memset(vty->prompt, 0, sizeof(vty->prompt));
			sprintf(vty->prompt, "%s", temp->prompt);
			return CMD_SUCCESS;
		}
	}
	return CMD_WARNING;
/*
	int ret = ERROR;
	if(!pl_pjsip_global_isenable())
		ret = pl_pjsip_global_set_api(zpl_true);
	else
		ret = OK;
	if(ret == OK)
		vty->node = SIP_SERVICE_NODE;
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;*/
}

DEFUN (no_ip_sip_service,
		no_ip_sip_service_cmd,
		"no service sip",
		NO_STR
		"Service configure\n"
		"SIP Protocol\n")
{
	template_t * temp = nsm_template_lookup_name (zpl_true, "service pjsip");
	if(temp)
	{
		if(pl_pjsip_global_isenable())
			pl_pjsip_global_set_api(zpl_false);
		nsm_template_free(temp);
		return CMD_SUCCESS;
	}
	return CMD_WARNING;

/*	int ret = ERROR;
	if(pl_pjsip_global_isenable())
		ret = pl_pjsip_global_set_api(zpl_false);
	else
		ret = OK;
	if(ret == OK)
		vty->node = SIP_SERVICE_NODE;
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;*/
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
		ret = pl_pjsip_server_set_api(argv[0], pl_pjsip->sip_server_sec.sip_port, zpl_true);
	else
		ret = pl_pjsip_server_set_api(argv[0], pl_pjsip->sip_server.sip_port, zpl_false);
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
		ret = pl_pjsip_server_set_api(NULL, pl_pjsip->sip_server_sec.sip_port, zpl_true);
	else
		ret = pl_pjsip_server_set_api(NULL, pl_pjsip->sip_server.sip_port, zpl_false);
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
	zpl_int8		sip_address[PJSIP_ADDRESS_MAX];
	value = atoi (argv[0]);
	if (value <= 0)
	{
		vty_out (vty, "%% Malformed port %s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(argc == 2)
	{
		memset(sip_address, 0, sizeof(sip_address));
		pl_pjsip_server_get_api(sip_address, NULL, zpl_true);
		ret = pl_pjsip_server_set_api(sip_address, value, zpl_true);
	}
	else
	{
		memset(sip_address, 0, sizeof(sip_address));
		pl_pjsip_server_get_api(sip_address, NULL, zpl_false);
		ret = pl_pjsip_server_set_api(sip_address, value, zpl_false);
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
	zpl_int8		sip_address[PJSIP_ADDRESS_MAX];
	if(argc == 1)
	{
		memset(sip_address, 0, sizeof(sip_address));
		pl_pjsip_server_get_api(sip_address, NULL, zpl_true);
		ret = pl_pjsip_server_set_api(sip_address, 0, zpl_true);
	}
	else
	{
		memset(sip_address, 0, sizeof(sip_address));
		pl_pjsip_server_get_api(sip_address, NULL, zpl_false);
		ret = pl_pjsip_server_set_api(sip_address, 0, zpl_false);
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
		ret = pl_pjsip_proxy_set_api(argv[0], pl_pjsip->sip_proxy_sec.sip_port, zpl_true);
	else
		ret = pl_pjsip_proxy_set_api(argv[0], pl_pjsip->sip_proxy.sip_port, zpl_false);
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
		ret = pl_pjsip_proxy_set_api(NULL, pl_pjsip->sip_proxy_sec.sip_port, zpl_true);
	else
		ret = pl_pjsip_proxy_set_api(NULL, pl_pjsip->sip_proxy.sip_port, zpl_false);
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
	zpl_int8		sip_address[PJSIP_ADDRESS_MAX];
	value = atoi (argv[0]);
	if (value <= 0)
	{
		vty_out (vty, "%% Malformed port %s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(argc == 2)
	{
		memset(sip_address, 0, sizeof(sip_address));
		pl_pjsip_proxy_get_api(sip_address, NULL, zpl_true);
		ret = pl_pjsip_proxy_set_api(sip_address, value, zpl_true);
	}
	else
	{
		memset(sip_address, 0, sizeof(sip_address));
		pl_pjsip_proxy_get_api(sip_address, NULL, zpl_false);
		ret = pl_pjsip_proxy_set_api(sip_address, value, zpl_false);
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
	zpl_int8		sip_address[PJSIP_ADDRESS_MAX];
	if(argc == 1)
	{
		memset(sip_address, 0, sizeof(sip_address));
		pl_pjsip_proxy_get_api(sip_address, NULL, zpl_true);
		ret = pl_pjsip_proxy_set_api(sip_address, 0, zpl_true);
	}
	else
	{
		memset(sip_address, 0, sizeof(sip_address));
		pl_pjsip_proxy_get_api(sip_address, NULL, zpl_false);
		ret = pl_pjsip_proxy_set_api(sip_address, 0, zpl_false);
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
	ret = pl_pjsip_100rel_set_api(zpl_true);
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
	ret = pl_pjsip_100rel_set_api(zpl_false);
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
		pl_pjsip_username_get_api(NULL, sip_password, zpl_true);
		ret = pl_pjsip_username_set_api(argv[0], sip_password, zpl_true);
	}
	else
	{
		memset(sip_password, 0, sizeof(sip_password));
		pl_pjsip_username_get_api(NULL, sip_password, zpl_false);
		ret = pl_pjsip_username_set_api(argv[0], sip_password, zpl_false);
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
		pl_pjsip_username_get_api(NULL, sip_password, zpl_true);
		ret = pl_pjsip_username_set_api(NULL, sip_password, zpl_true);
	}
	else
	{
		memset(sip_password, 0, sizeof(sip_password));
		pl_pjsip_username_get_api(NULL, sip_password, zpl_false);
		ret = pl_pjsip_username_set_api(NULL, sip_password, zpl_false);
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
		pl_pjsip_username_get_api(NULL, sip_password, zpl_true);
		ret = pl_pjsip_username_set_api(argv[0], sip_password, zpl_true);
	}
	else
	{
		memset(sip_password, 0, sizeof(sip_password));
		pl_pjsip_username_get_api(NULL, sip_password, zpl_false);
		ret = pl_pjsip_username_set_api(argv[0], sip_password, zpl_false);
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
		pl_pjsip_username_get_api(NULL, sip_password, zpl_true);
		ret = pl_pjsip_username_set_api(NULL, sip_password, zpl_true);
	}
	else
	{
		memset(sip_password, 0, sizeof(sip_password));
		pl_pjsip_username_get_api(NULL, sip_password, zpl_false);
		ret = pl_pjsip_username_set_api(NULL, sip_password, zpl_false);
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
		pl_pjsip_username_get_api(sip_phone, NULL, zpl_true);
		ret = pl_pjsip_username_set_api(sip_phone, argv[0], zpl_true);
	}
	else
	{
		memset(sip_phone, 0, sizeof(sip_phone));
		pl_pjsip_username_get_api(sip_phone, NULL, zpl_false);
		ret = pl_pjsip_username_set_api(sip_phone, argv[0], zpl_false);
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
		pl_pjsip_username_get_api(sip_phone, NULL, zpl_true);
		ret = pl_pjsip_username_set_api(sip_phone, NULL, zpl_true);
	}
	else
	{
		memset(sip_phone, 0, sizeof(sip_phone));
		pl_pjsip_username_get_api(sip_phone, NULL, zpl_false);
		ret = pl_pjsip_username_set_api(sip_phone, NULL, zpl_false);
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
		ret = voip_sip_realm_set_api(NULL, zpl_true);
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

DEFUN (ip_sip_register_delay,
		ip_sip_register_delay_cmd,
		"ip sip rereg-delay <15-3600>",
		IP_STR
		"SIP configure\n"
		"reregister delay\n"
		"interval value\n")
{
	int ret = ERROR, value = 0;
	value = atoi(argv[0]);
	ret = pl_pjsip_reregist_delay_set_api(value);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_register_delay,
		no_ip_sip_register_delay_cmd,
		"no ip sip rereg-delay",
		NO_STR
		IP_STR
		"SIP configure\n"
		"reregister delay configure\n")
{
	int ret = ERROR, value = 0;
	ret = pl_pjsip_reregist_delay_set_api(value);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_register_proxy,
		ip_sip_register_proxy_cmd,
		"ip sip reg-proxy (disable|none|outbound|acc|all)",
		IP_STR
		"SIP configure\n"
		"register proxy\n"
		"interval value\n")
{
	int ret = ERROR;
	pjsip_reg_proxy_t value = PJSIP_REGISTER_NONE;
	if(strstr(argv[0], "disable"))
		value = PJSIP_REGISTER_NONE;
	else if(strstr(argv[0], "none"))
		value = PJSIP_REGISTER_NO_PROXY;
	else if(strstr(argv[0], "outbound"))
		value = PJSIP_REGISTER_OUTBOUND_PROXY;
	else if(strstr(argv[0], "acc"))
		value = PJSIP_REGISTER_ACC_ONLY;
	else if(strstr(argv[0], "all"))
		value = PJSIP_REGISTER_ALL;
	ret = pl_pjsip_reregister_proxy_set_api(value);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_register_proxy,
		no_ip_sip_register_proxy_cmd,
		"no ip sip reg-proxy",
		NO_STR
		IP_STR
		"SIP configure\n"
		"register proxy configure\n")
{
	int ret = ERROR, value = PJSIP_REGISTER_NONE;
	ret = pl_pjsip_reregister_proxy_set_api(value);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_sip_option_set,
		ip_sip_option_set_cmd,
		"ip sip enable (publish|mwi|ims)",
		IP_STR
		"SIP configure\n"
		"Enable Configure\n"
		"Publish option\n"
		"Mwi option\n"
		"Ims option\n")
{
	int ret = ERROR;
	if(strstr(argv[0], "publish"))
		ret = pl_pjsip_publish_set_api(zpl_true);
	else if(strstr(argv[0], "mwi"))
		ret = pl_pjsip_mwi_set_api(zpl_true);
	else if(strstr(argv[0], "ims"))
		ret = pl_pjsip_ims_set_api(zpl_true);

	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_option_set,
		no_ip_sip_option_set_cmd,
		"no ip sip enable (publish|mwi|ims)",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Enable Configure\n"
		"Publish option\n"
		"Mwi option\n"
		"Ims option\n")
{
	int ret = ERROR;
	if(strstr(argv[0], "publish"))
		ret = pl_pjsip_publish_set_api(zpl_false);
	else if(strstr(argv[0], "mwi"))
		ret = pl_pjsip_mwi_set_api(zpl_false);
	else if(strstr(argv[0], "ims"))
		ret = pl_pjsip_ims_set_api(zpl_false);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_sip_srtp_mode_set,
		ip_sip_srtp_mode_set_cmd,
		"ip sip srtp mode (disabled|optional|mandatory|optional-duplicating)",
		IP_STR
		"SIP configure\n"
		"Srtp Configure\n"
		"Mode option\n"
		"Disabled mode\n"
		"Optional mode\n"
		"Mandatory mode\n"
		"Optional-duplicating mode\n")
{
	int ret = ERROR;
	pjsip_srtp_t val = PJSIP_SRTP_DISABLE;
	if(strstr(argv[0], "disabled"))
		val = PJSIP_SRTP_DISABLE;
	else if(strstr(argv[0], "optional-duplicating"))
		val = PJSIP_SRTP_OPTIONAL_DUP;
	else if(strstr(argv[0], "optional"))
		val = PJSIP_SRTP_OPTIONAL;
	else if(strstr(argv[0], "mandatory"))
		val = PJSIP_SRTP_MANDATORY;

	ret = pl_pjsip_srtp_mode_set_api(val);

	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_srtp_mode_set,
		no_ip_sip_srtp_mode_set_cmd,
		"no ip sip srtp mode",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Srtp Configure\n"
		"Mode option\n")
{
	int ret = ERROR;
	ret = pl_pjsip_srtp_mode_set_api(PJSIP_SRTP_DISABLE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_sip_srtp_secure_set,
		ip_sip_srtp_secure_set_cmd,
		"ip sip srtp secure (tls|sips)",
		IP_STR
		"SIP configure\n"
		"Srtp Configure\n"
		"Secure option\n"
		"Tls mode\n"
		"Sips mode\n")
{
	int ret = ERROR;
	pjsip_srtp_sec_t val = PJSIP_SRTP_SEC_NO;
	if(strstr(argv[0], "tls"))
		val = PJSIP_SRTP_SEC_TLS;
	else if(strstr(argv[0], "sips"))
		val = PJSIP_SRTP_SEC_SIPS;

	ret = pl_pjsip_srtp_secure_set_api(val);

	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_srtp_secure_set,
		no_ip_sip_srtp_secure_set_cmd,
		"no ip sip srtp secure",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Srtp Configure\n"
		"Mode option\n")
{
	int ret = ERROR;
	ret = pl_pjsip_srtp_secure_set_api(PJSIP_SRTP_SEC_NO);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_srtp_keying_set,
		ip_sip_srtp_keying_set_cmd,
		"ip sip srtp keying method (sdes|dtls)",
		IP_STR
		"SIP configure\n"
		"Srtp Configure\n"
		"Keying option\n"
		"Method option\n"
		"Sdes mode\n"
		"Dtls mode\n")
{
	int ret = ERROR;
	pjsip_srtp_keying_t val = PJSIP_SRTP_KEYING_SDES;
	if(strstr(argv[0], "sdes"))
		val = PJSIP_SRTP_KEYING_SDES;
	else if(strstr(argv[0], "dtls"))
		val = PJSIP_SRTP_KEYING_DTLS;

	ret = pl_pjsip_srtp_keying_set_api(val);

	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_srtp_keying_set,
		no_ip_sip_srtp_keying_set_cmd,
		"no ip sip srtp keying method",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Srtp Configure\n"
		"Keying option\n"
		"Method option\n")
{
	int ret = ERROR;
	ret = pl_pjsip_srtp_keying_set_api(PJSIP_SRTP_KEYING_SDES);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_sip_session_timers_set,
		ip_sip_session_timers_set_cmd,
		"ip sip session-timers (inactive|optional|mandatory|always)",
		IP_STR
		"SIP configure\n"
		"Session-timers Configure\n"
		"Inactive mode\n"
		"Optional mode\n"
		"Mandatory mode\n"
		"Always mode\n")
{
	int ret = ERROR;
	pjsip_timer_t val = PJSIP_TIMER_INACTIVE;
	if(strstr(argv[0], "inactive"))
		val = PJSIP_TIMER_INACTIVE;
	else if(strstr(argv[0], "optional"))
		val = PJSIP_TIMER_OPTIONAL;
	else if(strstr(argv[0], "mandatory"))
		val = PJSIP_TIMER_MANDATORY;
	else if(strstr(argv[0], "always"))
		val = PJSIP_TIMER_ALWAYS;

	ret = pl_pjsip_timer_set_api(val);

	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_session_timers_set,
		no_ip_sip_session_timers_set_cmd,
		"no ip sip session-timers",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Session-timers Configure\n")
{
	int ret = ERROR;
	ret = pl_pjsip_timer_set_api(PJSIP_TIMER_INACTIVE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_session_timers_sec_set,
		ip_sip_session_timers_sec_set_cmd,
		"ip sip session-timers expiration-period <1-3600>",
		IP_STR
		"SIP configure\n"
		"Session-timers Configure\n"
		"Expiration Period\n"
		"value\n")
{
	int ret = ERROR;
	ret = pl_pjsip_timer_sec_set_api(atoi(argv[0]));

	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_session_timers_sec_set,
		no_ip_sip_session_timers_sec_set_cmd,
		"no ip sip session-timers expiration-period",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Session-timers Configure\n"
		"Expiration Period\n")
{
	int ret = ERROR;
	ret = pl_pjsip_timer_sec_set_api(0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_auto_update_nat_set,
		ip_sip_auto_update_nat_set_cmd,
		"ip sip traversal-behind symmetric nat",
		IP_STR
		"SIP configure\n"
		"Traversal behind Configure\n"
		"symmetric\n"
		"nat\n")
{
	int ret = ERROR;
	ret = pl_pjsip_auto_update_nat_set_api(zpl_true);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_auto_update_nat_set,
		no_ip_sip_auto_update_nat_set_cmd,
		"no ip sip traversal-behind symmetric nat",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Traversal behind Configure\n"
		"symmetric\n"
		"nat\n")
{
	int ret = ERROR;
	ret = pl_pjsip_auto_update_nat_set_api(zpl_false);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_stun_enable_set,
		ip_sip_stun_enable_set_cmd,
		"ip sip stun (disabled|enabled)",
		IP_STR
		"SIP configure\n"
		"Stun Configure\n"
		"disabled\n"
		"enabled\n")
{
	int ret = ERROR;
	if(strstr(argv[0], "disable"))
		ret = pl_pjsip_stun_set_api(zpl_false);
	else
		ret = pl_pjsip_stun_set_api(zpl_true);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_stun_enable_set,
		no_ip_sip_stun_enable_set_cmd,
		"no ip sip stun",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Stun Configure\n")
{
	int ret = ERROR;
	ret = pl_pjsip_stun_set_api(zpl_false);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

//Transport Options:
DEFUN (ip_sip_ipv6_enable_set,
		ip_sip_ipv6_enable_set_cmd,
		"ip sip ipv6 (disabled|enabled)",
		IP_STR
		"SIP configure\n"
		"ipv6 Configure\n"
		"disabled\n"
		"enabled\n")
{
	int ret = ERROR;
	if(strstr(argv[0], "disable"))
		ret = pl_pjsip_ipv6_set_api(zpl_false);
	else
		ret = pl_pjsip_ipv6_set_api(zpl_true);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_ipv6_enable_set,
		no_ip_sip_ipv6_enable_set_cmd,
		"no ip sip ipv6",
		NO_STR
		IP_STR
		"SIP configure\n"
		"ipv6 Configure\n")
{
	int ret = ERROR;
	ret = pl_pjsip_ipv6_set_api(zpl_false);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_tagging_qos_set,
		ip_sip_tagging_qos_set_cmd,
		"ip sip tagging-qos (disabled|enabled)",
		IP_STR
		"SIP configure\n"
		"Tagging Qos Configure\n"
		"disabled\n"
		"enabled\n")
{
	int ret = ERROR;
	if(strstr(argv[0], "disable"))
		ret = pl_pjsip_qos_set_api(zpl_false);
	else
		ret = pl_pjsip_qos_set_api(zpl_true);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_tagging_qos_set,
		no_ip_sip_tagging_qos_set_cmd,
		"no ip sip tagging-qos",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Tagging Qos Configure\n")
{
	int ret = ERROR;
	ret = pl_pjsip_qos_set_api(zpl_false);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_transport_tcpudp_set,
		ip_sip_transport_tcpudp_set_cmd,
		"ip sip transport (udp|tcp) (disabled|enabled)",
		IP_STR
		"SIP configure\n"
		"Tagging Qos Configure\n"
		"Udp Configure\n"
		"Tcp Configure\n"
		"disabled\n"
		"enabled\n")
{
	int ret = ERROR;
	if(strstr(argv[0], "udp"))
	{
		if(strstr(argv[1], "disable"))
			ret = pl_pjsip_noudp_set_api(zpl_false);
		else
			ret = pl_pjsip_noudp_set_api(zpl_true);
	}
	else
	{
		if(strstr(argv[1], "disable"))
			ret = pl_pjsip_notcp_set_api(zpl_false);
		else
			ret = pl_pjsip_notcp_set_api(zpl_true);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_transport_tcpudp_set,
		no_ip_sip_transport_tcpudp_set_cmd,
		"no ip sip transport (udp|tcp)",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Tagging Qos Configure\n")
{
	int ret = ERROR;
	if(strstr(argv[0], "udp"))
		ret = pl_pjsip_noudp_set_api(zpl_false);
	else
		ret = pl_pjsip_notcp_set_api(zpl_false);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_sip_nameserver,
		ip_sip_nameserver_cmd,
		"ip sip nameserver "CMD_KEY_IPV4,
		IP_STR
		"SIP Configure\n"
		"Name Server configure\n"
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
		ret = pl_pjsip_nameserver_set_api(argv[0], pl_pjsip->sip_nameserver.sip_port);
	else
		ret = pl_pjsip_nameserver_set_api(argv[0], pl_pjsip->sip_nameserver.sip_port);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_nameserver,
		no_ip_sip_nameserver_cmd,
		"no ip sip nameserver",
		NO_STR
		IP_STR
		"SIP Configure\n"
		"Name Server configure\n")
{
	int ret = 0;
	if(argc == 1)
		ret = pl_pjsip_nameserver_set_api(NULL, pl_pjsip->sip_nameserver.sip_port);
	else
		ret = pl_pjsip_nameserver_set_api(NULL, pl_pjsip->sip_nameserver.sip_port);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_sip_nameserver_port,
		ip_sip_nameserver_port_cmd,
		"ip sip nameserver port <256-65535>",
		IP_STR
		"SIP Configure\n"
		"Name Server configure\n"
		"Name Server port configure\n"
		"Port Value\n")
{
	int ret = ERROR;
	int value = 0;
	zpl_int8		sip_address[PJSIP_ADDRESS_MAX];
	value = atoi (argv[0]);
	if (value <= 0)
	{
		vty_out (vty, "%% Malformed port %s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(argc == 2)
	{
		memset(sip_address, 0, sizeof(sip_address));
		pl_pjsip_nameserver_get_api(sip_address, NULL);
		ret = pl_pjsip_nameserver_set_api(sip_address, value);
	}
	else
	{
		memset(sip_address, 0, sizeof(sip_address));
		pl_pjsip_nameserver_get_api(sip_address, NULL);
		ret = pl_pjsip_nameserver_set_api(sip_address, value);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_nameserver_port,
		no_ip_sip_nameserver_port_cmd,
		"no ip sip nameserver port",
		NO_STR
		IP_STR
		"SIP Configure\n"
		"Name Server configure\n"
		"Name Server port configure\n")
{
	int ret = 0;
	if(argc == 1)
		ret = pl_pjsip_nameserver_set_api(NULL, pl_pjsip->sip_nameserver.sip_port);
	else
		ret = pl_pjsip_nameserver_set_api(NULL, pl_pjsip->sip_nameserver.sip_port);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_sip_outbound,
		ip_sip_outbound_cmd,
		"ip sip outbound "CMD_KEY_IPV4,
		IP_STR
		"SIP Configure\n"
		"Outbound configure\n"
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
		ret = pl_pjsip_outbound_set_api(argv[0], pl_pjsip->sip_outbound.sip_port);
	else
		ret = pl_pjsip_outbound_set_api(argv[0], pl_pjsip->sip_outbound.sip_port);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_outbound,
		no_ip_sip_outbound_cmd,
		"no ip sip outbound",
		NO_STR
		IP_STR
		"SIP Configure\n"
		"Outbound configure\n")
{
	int ret = 0;
	if(argc == 1)
		ret = pl_pjsip_outbound_set_api(NULL, pl_pjsip->sip_outbound.sip_port);
	else
		ret = pl_pjsip_outbound_set_api(NULL, pl_pjsip->sip_outbound.sip_port);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_outbound_port,
		ip_sip_outbound_port_cmd,
		"ip sip outbound port <256-65535>",
		IP_STR
		"SIP Configure\n"
		"Outbound configure\n"
		"Outbound port configure\n"
		"Port Value\n")
{
	int ret = ERROR;
	int value = 0;
	zpl_int8		sip_address[PJSIP_ADDRESS_MAX];
	value = atoi (argv[0]);
	if (value <= 0)
	{
		vty_out (vty, "%% Malformed port %s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(argc == 2)
	{
		memset(sip_address, 0, sizeof(sip_address));
		pl_pjsip_outbound_get_api(sip_address, NULL);
		ret = pl_pjsip_outbound_set_api(sip_address, value);
	}
	else
	{
		memset(sip_address, 0, sizeof(sip_address));
		pl_pjsip_outbound_get_api(sip_address, NULL);
		ret = pl_pjsip_outbound_set_api(sip_address, value);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_outbound_port,
		no_ip_sip_outbound_port_cmd,
		"no ip sip outbound port",
		NO_STR
		IP_STR
		"SIP Configure\n"
		"Outbound configure\n"
		"Outbound port configure\n")
{
	int ret = 0;
	if(argc == 1)
		ret = pl_pjsip_outbound_set_api(NULL, pl_pjsip->sip_outbound.sip_port);
	else
		ret = pl_pjsip_outbound_set_api(NULL, pl_pjsip->sip_outbound.sip_port);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_sip_stun_server,
		ip_sip_stun_server_cmd,
		"ip sip stun-server "CMD_KEY_IPV4,
		IP_STR
		"SIP Configure\n"
		"Stun Server configure\n"
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
		ret = pl_pjsip_stun_server_set_api(argv[0], pl_pjsip->sip_stun_server.sip_port);
	else
		ret = pl_pjsip_stun_server_set_api(argv[0], pl_pjsip->sip_stun_server.sip_port);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_stun_server,
		no_ip_sip_stun_server_cmd,
		"no ip sip stun-server",
		NO_STR
		IP_STR
		"SIP Configure\n"
		"Stun Server configure\n")
{
	int ret = 0;
	if(argc == 1)
		ret = pl_pjsip_stun_server_set_api(NULL, pl_pjsip->sip_stun_server.sip_port);
	else
		ret = pl_pjsip_stun_server_set_api(NULL, pl_pjsip->sip_stun_server.sip_port);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_stun_server_port,
		ip_sip_stun_server_port_cmd,
		"ip sip stun-server port <256-65535>",
		IP_STR
		"SIP Configure\n"
		"Stun Server configure\n"
		"Stun Server port configure\n"
		"Port Value\n")
{
	int ret = ERROR;
	int value = 0;
	zpl_int8		sip_address[PJSIP_ADDRESS_MAX];
	value = atoi (argv[0]);
	if (value <= 0)
	{
		vty_out (vty, "%% Malformed port %s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(argc == 2)
	{
		memset(sip_address, 0, sizeof(sip_address));
		pl_pjsip_stun_server_get_api(sip_address, NULL);
		ret = pl_pjsip_stun_server_set_api(sip_address, value);
	}
	else
	{
		memset(sip_address, 0, sizeof(sip_address));
		pl_pjsip_stun_server_get_api(sip_address, NULL);
		ret = pl_pjsip_stun_server_set_api(sip_address, value);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_stun_server_port,
		no_ip_sip_stun_server_port_cmd,
		"no ip sip stun-server port",
		NO_STR
		IP_STR
		"SIP Configure\n"
		"Stun Server configure\n"
		"Stun Server port configure\n")
{
	int ret = 0;
	if(argc == 1)
		ret = pl_pjsip_stun_server_set_api(NULL, pl_pjsip->sip_stun_server.sip_port);
	else
		ret = pl_pjsip_stun_server_set_api(NULL, pl_pjsip->sip_stun_server.sip_port);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}



//TLS Options:
#if defined(PJSIP_HAS_TLS_TRANSPORT) && (PJSIP_HAS_TLS_TRANSPORT != 0)
DEFUN (ip_sip_tls_enable_set,
		ip_sip_tls_enable_set_cmd,
		"ip sip tls enable",
		IP_STR
		"SIP configure\n"
		"Tls Configure\n"
		"enabled\n")
{
	int ret = ERROR;
	ret = pl_pjsip_tls_set_api(zpl_true);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_tls_enable_set,
		no_ip_sip_tls_enable_set_cmd,
		"no ip sip tls enable",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Tls Configure\n")
{
	int ret = ERROR;
	ret = pl_pjsip_tls_set_api(zpl_false);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_tls_ca_file,
		ip_sip_tls_ca_file_cmd,
		"ip sip tls-ca-file FILENAME",
		IP_STR
		"SIP configure\n"
		"Tls ca-file Configure\n"
		"filename\n")
{
	int ret = ERROR;
	ret = pl_pjsip_tls_ca_set_api(argv[0]);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_tls_ca_file,
		no_ip_sip_tls_ca_file_cmd,
		"no ip sip tls-ca-file",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Tls ca-file Configure\n")
{
	int ret = ERROR;
	ret = pl_pjsip_tls_ca_set_api(NULL);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_tls_cert_file,
		ip_sip_tls_cert_file_cmd,
		"ip sip tls-cert-file FILENAME",
		IP_STR
		"SIP configure\n"
		"Tls cert-file Configure\n"
		"filename\n")
{
	int ret = ERROR;
	ret = pl_pjsip_tls_cert_set_api(argv[0]);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_tls_cert_file,
		no_ip_sip_tls_cert_file_cmd,
		"no ip sip tls-cert-file",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Tls cert-file Configure\n")
{
	int ret = ERROR;
	ret = pl_pjsip_tls_cert_set_api(NULL);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_tls_private_file,
		ip_sip_tls_private_file_cmd,
		"ip sip tls-private-file FILENAME",
		IP_STR
		"SIP configure\n"
		"Tls private-file Configure\n"
		"filename\n")
{
	int ret = ERROR;
	ret = pl_pjsip_tls_privkey_set_api(argv[0]);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_tls_private_file,
		no_ip_sip_tls_private_file_cmd,
		"no ip sip tls-private-file",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Tls private-file Configure\n")
{
	int ret = ERROR;
	ret = pl_pjsip_tls_privkey_set_api(NULL);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_tls_password,
		ip_sip_tls_password_cmd,
		"ip sip tls-password PASSWORD",
		IP_STR
		"SIP configure\n"
		"Tls password Configure\n"
		"password\n")
{
	int ret = ERROR;
	ret = pl_pjsip_tls_password_set_api(argv[0]);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_tls_password,
		no_ip_sip_tls_password_cmd,
		"no ip sip tls-password",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Tls password Configure\n")
{
	int ret = ERROR;
	ret = pl_pjsip_tls_password_set_api(NULL);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_sip_verify_server,
		ip_sip_verify_server_cmd,
		"ip sip verify-server "CMD_KEY_IPV4,
		IP_STR
		"SIP Configure\n"
		"verify Server configure\n"
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
		ret = pl_pjsip_tls_verify_server_set_api(argv[0], pl_pjsip->sip_tls_verify_server.sip_port);
	else
		ret = pl_pjsip_tls_verify_server_set_api(argv[0], pl_pjsip->sip_tls_verify_server.sip_port);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_verify_server,
		no_ip_sip_verify_server_cmd,
		"no ip sip verify-server",
		NO_STR
		IP_STR
		"SIP Configure\n"
		"verify Server configure\n")
{
	int ret = 0;
	if(argc == 1)
		ret = pl_pjsip_tls_verify_server_set_api(NULL, pl_pjsip->sip_tls_verify_server.sip_port);
	else
		ret = pl_pjsip_tls_verify_server_set_api(NULL, pl_pjsip->sip_tls_verify_server.sip_port);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_verify_server_port,
		ip_sip_verify_server_port_cmd,
		"ip sip verify-server port <256-65535>",
		IP_STR
		"SIP Configure\n"
		"verify Server configure\n"
		"verify Server port configure\n"
		"Port Value\n")
{
	int ret = ERROR;
	int value = 0;
	zpl_int8		sip_address[PJSIP_ADDRESS_MAX];
	value = atoi (argv[0]);
	if (value <= 0)
	{
		vty_out (vty, "%% Malformed port %s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(argc == 2)
	{
		memset(sip_address, 0, sizeof(sip_address));
		pl_pjsip_tls_verify_server_get_api(sip_address, NULL);
		ret = pl_pjsip_tls_verify_server_set_api(sip_address, value);
	}
	else
	{
		memset(sip_address, 0, sizeof(sip_address));
		pl_pjsip_tls_verify_server_get_api(sip_address, NULL);
		ret = pl_pjsip_tls_verify_server_set_api(sip_address, value);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_verify_server_port,
		no_ip_sip_verify_server_port_cmd,
		"no ip sip verify-server port",
		NO_STR
		IP_STR
		"SIP Configure\n"
		"verify Server configure\n"
		"verify Server port configure\n")
{
	int ret = 0;
	if(argc == 1)
		ret = pl_pjsip_tls_verify_server_set_api(NULL, pl_pjsip->sip_tls_verify_server.sip_port);
	else
		ret = pl_pjsip_tls_verify_server_set_api(NULL, pl_pjsip->sip_tls_verify_server.sip_port);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_sip_verify_client,
		ip_sip_verify_client_cmd,
		"ip sip verify-client "CMD_KEY_IPV4,
		IP_STR
		"SIP Configure\n"
		"verify client configure\n"
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
		ret = pl_pjsip_tls_verify_client_set_api(argv[0], pl_pjsip->sip_tls_verify_client.sip_port);
	else
		ret = pl_pjsip_tls_verify_client_set_api(argv[0], pl_pjsip->sip_tls_verify_client.sip_port);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_verify_client,
		no_ip_sip_verify_client_cmd,
		"no ip sip verify-client",
		NO_STR
		IP_STR
		"SIP Configure\n"
		"verify client configure\n")
{
	int ret = 0;
	if(argc == 1)
		ret = pl_pjsip_tls_verify_client_set_api(NULL, pl_pjsip->sip_tls_verify_client.sip_port);
	else
		ret = pl_pjsip_tls_verify_client_set_api(NULL, pl_pjsip->sip_tls_verify_client.sip_port);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_verify_client_port,
		ip_sip_verify_client_port_cmd,
		"ip sip verify-client port <256-65535>",
		IP_STR
		"SIP Configure\n"
		"verify client configure\n"
		"verify client port configure\n"
		"Port Value\n")
{
	int ret = ERROR;
	int value = 0;
	zpl_int8		sip_address[PJSIP_ADDRESS_MAX];
	value = atoi (argv[0]);
	if (value <= 0)
	{
		vty_out (vty, "%% Malformed port %s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(argc == 2)
	{
		memset(sip_address, 0, sizeof(sip_address));
		pl_pjsip_tls_verify_client_get_api(sip_address, NULL);
		ret = pl_pjsip_tls_verify_client_set_api(sip_address, value);
	}
	else
	{
		memset(sip_address, 0, sizeof(sip_address));
		pl_pjsip_tls_verify_client_get_api(sip_address, NULL);
		ret = pl_pjsip_tls_verify_client_set_api(sip_address, value);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_verify_client_port,
		no_ip_sip_verify_client_port_cmd,
		"no ip sip verify-client port",
		NO_STR
		IP_STR
		"SIP Configure\n"
		"verify client configure\n"
		"verify client port configure\n")
{
	int ret = 0;
	if(argc == 1)
		ret = pl_pjsip_tls_verify_client_set_api(NULL, pl_pjsip->sip_tls_verify_client.sip_port);
	else
		ret = pl_pjsip_tls_verify_client_set_api(NULL, pl_pjsip->sip_tls_verify_client.sip_port);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_sip_tls_cipher,
		ip_sip_tls_cipher_cmd,
		"ip sip tls-cipher PASSWORD",
		IP_STR
		"SIP configure\n"
		"Tls cipher Configure\n"
		"password\n")
{
	int ret = ERROR;
	ret = pl_pjsip_tls_cipher_set_api(argv[0]);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_tls_cipher,
		no_ip_sip_tls_cipher_cmd,
		"no ip sip tls-password",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Tls cipher Configure\n")
{
	int ret = ERROR;
	ret = pl_pjsip_tls_cipher_set_api(NULL);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
#endif

DEFUN (ip_sip_negotiation_timeout,
		ip_sip_negotiation_timeout_cmd,
		"ip sip negotiation timeout <1-3600>",
		IP_STR
		"SIP configure\n"
		"Negotiation Configure\n"
		"Timeout Configure\n"
		"Timeout value\n")
{
	int ret = ERROR;
	ret = pl_pjsip_neg_timeout_set_api(atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_negotiation_timeout,
		no_ip_sip_negotiation_timeout_cmd,
		"no ip sip negotiation timeout",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Negotiation Configure\n"
		"Timeout Configure\n")
{
	int ret = ERROR;
	ret = pl_pjsip_neg_timeout_set_api(0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


//Audio Options:
DEFUN (ip_sip_default_codec_add,
		ip_sip_default_codec_add_cmd,
		"ip sip default codec (pcmu|pcma|g722|gsm|ilbc|speex-nb)",
		IP_STR
		"SIP configure\n"
		"codec Configure\n"
		"PCMU/8000\n"
		"PCMA/8000\n"
		"G722\n"
		"GSM\n"
		"iLBC/8000\n"
		"SPEEX/8000\n")
{
	int ret = ERROR;

	ret = pl_pjsip_codec_default_set_api(argv[0]);

	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_default_codec_add,
		no_ip_sip_default_codec_add_cmd,
		"no ip sip default codec",
		NO_STR
		IP_STR
		"SIP configure\n"
		"codec Configure\n")
{
	int ret = ERROR;

	ret = pl_pjsip_codec_default_set_api(NULL);

	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
DEFUN (ip_sip_codec_add,
		ip_sip_codec_add_cmd,
		"ip sip codec (pcmu|pcma|g722|gsm|ilbc|speex-nb)",
		IP_STR
		"SIP configure\n"
		"codec Configure\n"
		"PCMU/8000\n"
		"PCMA/8000\n"
		"G722\n"
		"GSM\n"
		"iLBC/8000\n"
		"SPEEX/8000\n")
{
	int ret = ERROR;
	pl_pjsip_discodec_del_api(argv[0]);
	ret = pl_pjsip_codec_add_api(argv[0]);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_codec_add,
		no_ip_sip_codec_add_cmd,
		"no ip sip codec (pcmu|pcma|g722|gsm|ilbc|speex-nb)",
		NO_STR
		IP_STR
		"SIP configure\n"
		"codec Configure\n"
		"PCMU/8000\n"
		"PCMA/8000\n"
		"G722\n"
		"GSM\n"
		"iLBC/8000\n"
		"SPEEX/8000\n")
{
	int ret = ERROR;
	pl_pjsip_codec_del_api(argv[0]);
	ret = pl_pjsip_discodec_add_api(argv[0]);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}



DEFUN (ip_sip_snd_clock_rate,
		ip_sip_snd_clock_rate_cmd,
		"ip sip snd-clock-rate <1-3600>",
		IP_STR
		"SIP configure\n"
		"Sound Clock Rate Configure\n"
		"Rate value\n")
{
	int ret = ERROR;
	ret = pl_pjsip_snd_clock_rate_set_api(atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_snd_clock_rate,
		no_ip_sip_snd_clock_rate_cmd,
		"no ip sip snd-clock-rate",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Sound Clock Rate Configure\n")
{
	int ret = ERROR;
	ret = pl_pjsip_snd_clock_rate_set_api(PJSIP_DEFAULT_CLOCK_RATE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}



DEFUN (ip_sip_auto_play_file,
		ip_sip_auto_play_file_cmd,
		"ip sip auto-play-file FILENAME",
		IP_STR
		"SIP configure\n"
		"Auto play file Configure\n"
		"filename\n")
{
	int ret = ERROR;
	ret = pl_pjsip_play_file_set_api(argv[0]);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_auto_play_file,
		no_ip_sip_auto_play_file_cmd,
		"no ip sip auto-play-file",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Auto play file Configure\n")
{
	int ret = ERROR;
	ret = pl_pjsip_play_file_set_api(NULL);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_auto_tone_file,
		ip_sip_auto_tone_file_cmd,
		"ip sip auto-tone-file FILENAME",
		IP_STR
		"SIP configure\n"
		"Auto play file Configure\n"
		"filename\n")
{
	int ret = ERROR;
	ret = pl_pjsip_play_tone_set_api(argv[0]);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_auto_tone_file,
		no_ip_sip_auto_tone_file_cmd,
		"no ip sip auto-tone-file",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Auto play file Configure\n")
{
	int ret = ERROR;
	ret = pl_pjsip_play_tone_set_api(NULL);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_sip_rec_file,
		ip_sip_rec_file_cmd,
		"ip sip rec-file FILENAME",
		IP_STR
		"SIP configure\n"
		"Auto rec file Configure\n"
		"filename\n")
{
	int ret = ERROR;
	ret = pl_pjsip_rec_file_set_api(argv[0]);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_rec_file,
		no_ip_sip_rec_file_cmd,
		"no ip sip rec-file",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Auto rec file Configure\n")
{
	int ret = ERROR;
	ret = pl_pjsip_rec_file_set_api(NULL);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}



DEFUN (ip_sip_auto_option_set,
		ip_sip_auto_option_set_cmd,
		"ip sip (auto-play|auto-loop|auto-confured) enable",
		IP_STR
		"SIP configure\n"
		"Tagging Qos Configure\n"
		"Udp Configure\n"
		"Tcp Configure\n"
		"disabled\n"
		"enabled\n")
{
	int ret = ERROR;
	if(strstr(argv[0], "play"))
	{
		ret = pl_pjsip_auto_play_set_api(zpl_true);
	}
	else if(strstr(argv[0], "loop"))
	{
		ret = pl_pjsip_auto_loop_set_api(zpl_true);
	}
	else if(strstr(argv[0], "confured"))
	{
		ret = pl_pjsip_auto_conf_set_api(zpl_true);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_auto_option_set,
		no_ip_sip_auto_option_set_cmd,
		"no ip sip (auto-play|auto-loop|auto-confured) enable",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Tagging Qos Configure\n")
{
	int ret = ERROR;
	if(strstr(argv[0], "play"))
	{
		ret = pl_pjsip_auto_play_set_api(zpl_false);
	}
	else if(strstr(argv[0], "loop"))
	{
		ret = pl_pjsip_auto_loop_set_api(zpl_false);
	}
	else if(strstr(argv[0], "confured"))
	{
		ret = pl_pjsip_auto_conf_set_api(zpl_false);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}



DEFUN (ip_sip_media_quality,
		ip_sip_media_quality_cmd,
		"ip sip media-quality <1-3600>",
		IP_STR
		"SIP configure\n"
		"Media quality Configure\n"
		"value\n")
{
	int ret = ERROR;
	ret = pl_pjsip_quality_set_api(atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_media_quality,
		no_ip_sip_media_quality_cmd,
		"no ip sip media-quality",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Media quality Configure\n")
{
	int ret = ERROR;
	ret = pl_pjsip_quality_set_api(0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_codec_ptime,
		ip_sip_codec_ptime_cmd,
		"ip sip codec-ptime <1-3600>",
		IP_STR
		"SIP configure\n"
		"Codec Ptime  Configure\n"
		"value\n")
{
	int ret = ERROR;
	ret = pl_pjsip_ptime_set_api(atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_codec_ptime,
		no_ip_sip_codec_ptime_cmd,
		"no ip sip codec-ptime",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Codec Ptime Configure\n")
{
	int ret = ERROR;
	ret = pl_pjsip_ptime_set_api(0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_sip_vad_silence,
		ip_sip_vad_silence_cmd,
		"ip sip vad-silence disabled",
		IP_STR
		"SIP configure\n"
		"vad-silence  Configure\n"
		"disabled\n")
{
	int ret = ERROR;
	ret = pl_pjsip_no_vad_set_api(zpl_true);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_vad_silence,
		no_ip_sip_vad_silence_cmd,
		"no ip sip vad-silence disabled",
		NO_STR
		IP_STR
		"SIP configure\n"
		"vad-silence  Configure\n"
		"disabled\n")
{
	int ret = ERROR;
	ret = pl_pjsip_no_vad_set_api(zpl_false);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_sip_echo_tail_len,
		ip_sip_echo_tail_len_cmd,
		"ip sip echo canceller tail <1-3600>",
		IP_STR
		"SIP configure\n"
		"Echo Configure\n"
		"Canceller Configure\n"
		"Tail Configure\n"
		"value\n")
{
	int ret = ERROR;
	ret = pl_pjsip_echo_tail_set_api(atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_echo_tail_len,
		no_ip_sip_echo_tail_len_cmd,
		"no ip sip echo canceller tail",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Echo Configure\n"
		"Canceller Configure\n"
		"Tail Configure\n")
{
	int ret = ERROR;
	ret = pl_pjsip_echo_tail_set_api(0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_echo_mode,
		ip_sip_echo_mode_cmd,
		"ip sip echo canceller algorithm (default|speex|suppressor|webrtc)",
		IP_STR
		"SIP configure\n"
		"Echo Configure\n"
		"Canceller Configure\n"
		"Algorithm Configure\n"
		"Default mode\n"
		"Speex mode\n"
		"Suppressor mode\n"
		"webrtc mode\n")
{
	int ret = ERROR;
	if(strstr(argv[0],"default"))
		ret = pl_pjsip_echo_mode_set_api(PJSIP_ECHO_DEFAULT);
	else if(strstr(argv[0],"speex"))
			ret = pl_pjsip_echo_mode_set_api(PJSIP_ECHO_SPEEX);
	else if(strstr(argv[0],"suppressor"))
			ret = pl_pjsip_echo_mode_set_api(PJSIP_ECHO_SUPPRESSER);
	else if(strstr(argv[0],"webrtc"))
			ret = pl_pjsip_echo_mode_set_api(PJSIP_ECHO_WEBRTXC);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_echo_mode,
		no_ip_sip_echo_mode_cmd,
		"no ip sip echo canceller algorithm",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Echo Tail Configure\n")
{
	int ret = ERROR;
	ret = pl_pjsip_echo_mode_set_api(PJSIP_ECHO_DISABLE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_ilbc_fps,
		ip_sip_ilbc_fps_cmd,
		"ip sip ilbc fps (20|30)",
		IP_STR
		"SIP configure\n"
		"iLBC Configure\n"
		"fps Configure\n"
		"value\n")
{
	int ret = ERROR;
	ret = pl_pjsip_ilbc_mode_set_api(atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_ilbc_fps,
		no_ip_sip_ilbc_fps_cmd,
		"no ip sip ilbc fps",
		NO_STR
		IP_STR
		"SIP configure\n"
		"iLBC Configure\n"
		"fps Configure\n")
{
	int ret = ERROR;
	ret = pl_pjsip_ilbc_mode_set_api(PJSUA_DEFAULT_ILBC_MODE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}



//Media Transport Options:

DEFUN (ip_sip_ice_enable,
		ip_sip_ice_enable_cmd,
		"ip sip ice enable",
		IP_STR
		"SIP configure\n"
		"ICE Configure\n"
		"Enable\n")
{
	int ret = ERROR;
	ret = pl_pjsip_ice_enable_set_api(zpl_true);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_ice_enable,
		no_ip_sip_ice_enable_cmd,
		"no ip sip ice enable",
		NO_STR
		IP_STR
		"SIP configure\n"
		"ICE Configure\n"
		"Enable\n")
{
	int ret = ERROR;
	ret = pl_pjsip_ice_enable_set_api(zpl_false);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_ice_regular,
		ip_sip_ice_regular_cmd,
		"ip sip ice regular <1-3600>",
		IP_STR
		"SIP configure\n"
		"ICE Configure\n"
		"regular Configure\n"
		"value\n")
{
	int ret = ERROR;
	ret = pl_pjsip_ice_regular_set_api(atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_ice_regular,
		no_ip_sip_ice_regular_cmd,
		"no ip sip ice regular",
		NO_STR
		IP_STR
		"SIP configure\n"
		"ICE Configure\n"
		"regular Configure\n")
{
	int ret = ERROR;
	ret = pl_pjsip_ice_regular_set_api(0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_ice_max_host,
		ip_sip_ice_max_host_cmd,
		"ip sip ice max-host <1-255>",
		IP_STR
		"SIP configure\n"
		"ICE Configure\n"
		"max-host Configure\n"
		"value\n")
{
	int ret = ERROR;
	ret = pl_pjsip_ice_max_host_set_api(atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_ice_max_host,
		no_ip_sip_ice_max_host_cmd,
		"no ip sip ice max-host",
		NO_STR
		IP_STR
		"SIP configure\n"
		"ICE Configure\n"
		"max-host Configure\n")
{
	int ret = ERROR;
	ret = pl_pjsip_ice_max_host_set_api(0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_sip_ice_notcp,
		ip_sip_ice_notcp_cmd,
		"ip sip ice notcp",
		IP_STR
		"SIP configure\n"
		"ICE Configure\n"
		"notcp\n")
{
	int ret = ERROR;
	ret = pl_pjsip_ice_nortcp_set_api(zpl_true);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_ice_notcp,
		no_ip_sip_ice_notcp_cmd,
		"no ip sip ice notcp",
		NO_STR
		IP_STR
		"SIP configure\n"
		"ICE Configure\n"
		"notcp\n")
{
	int ret = ERROR;
	ret = pl_pjsip_ice_nortcp_set_api(zpl_false);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_sip_rtp_port,
		ip_sip_rtp_port_cmd,
		"ip sip rtp port <1-65535>",
		IP_STR
		"SIP configure\n"
		"RTP Configure\n"
		"Rtp port Configure\n"
		"value\n")
{
	int ret = ERROR;
	ret = pl_pjsip_rtp_port_set_api(atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_rtp_port,
		no_ip_sip_rtp_port_cmd,
		"no ip sip rtp port",
		NO_STR
		IP_STR
		"SIP configure\n"
		"RTP Configure\n"
		"Rtp port Configure\n")
{
	int ret = ERROR;
	ret = pl_pjsip_rtp_port_set_api(PJSIP_RTP_PORT_DEFAULT);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_sip_drop_pct,
		ip_sip_drop_pct_cmd,
		"ip sip drop (rx-rtp|tx-rtp) <0-3600>",
		IP_STR
		"SIP configure\n"
		"Drop Configure\n"
		"RX rtp Configure\n"
		"TX rtp Configure\n"
		"value\n")
{
	int ret = ERROR;
	if(strstr(argv[0],"rx"))
		ret = pl_pjsip_rx_drop_pct_set_api(atoi(argv[0]));
	else
		ret = pl_pjsip_tx_drop_pct_set_api(atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_drop_pct,
		no_ip_sip_drop_pct_cmd,
		"no ip sip drop (rx-rtp|tx-rtp)",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Drop Configure\n"
		"RX rtp Configure\n"
		"TX rtp Configure\n")
{
	int ret = ERROR;
	if(strstr(argv[0],"rx"))
		ret = pl_pjsip_rx_drop_pct_set_api(0);
	else
		ret = pl_pjsip_tx_drop_pct_set_api(0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_sip_turn_enable,
		ip_sip_turn_enable_cmd,
		"ip sip turn enable",
		IP_STR
		"SIP configure\n"
		"Turn Configure\n"
		"Enable\n")
{
	int ret = ERROR;
	ret = pl_pjsip_turn_enable_set_api(zpl_true);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_turn_enable,
		no_ip_sip_turn_enable_cmd,
		"no ip sip turn enable",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Turn Configure\n"
		"Enable\n")
{
	int ret = ERROR;
	ret = pl_pjsip_turn_enable_set_api(zpl_false);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_sip_turn_tcp_enable,
		ip_sip_turn_tcp_enable_cmd,
		"ip sip turn-tcp enable",
		IP_STR
		"SIP configure\n"
		"Turn Configure\n"
		"Enable\n")
{
	int ret = ERROR;
	ret = pl_pjsip_turn_tcp_set_api(zpl_true);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_turn_tcp_enable,
		no_ip_sip_turn_tcp_enable_cmd,
		"no ip sip turn-tcp enable",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Turn Configure\n"
		"Enable\n")
{
	int ret = ERROR;
	ret = pl_pjsip_turn_tcp_set_api(zpl_false);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}




DEFUN (ip_sip_turn_server,
		ip_sip_turn_server_cmd,
		"ip sip turn-server "CMD_KEY_IPV4,
		IP_STR
		"SIP Configure\n"
		"verify client configure\n"
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
		ret = pl_pjsip_turn_server_set_api(argv[0], pl_pjsip->sip_turn_srv.sip_port);
	else
		ret = pl_pjsip_turn_server_set_api(argv[0], pl_pjsip->sip_turn_srv.sip_port);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_turn_server,
		no_ip_sip_turn_server_cmd,
		"no ip sip turn-server",
		NO_STR
		IP_STR
		"SIP Configure\n"
		"verify client configure\n")
{
	int ret = 0;
	if(argc == 1)
		ret = pl_pjsip_turn_server_set_api(NULL, pl_pjsip->sip_turn_srv.sip_port);
	else
		ret = pl_pjsip_turn_server_set_api(NULL, pl_pjsip->sip_turn_srv.sip_port);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_turn_server_port,
		ip_sip_turn_server_port_cmd,
		"ip sip turn-server port <256-65535>",
		IP_STR
		"SIP Configure\n"
		"verify client configure\n"
		"verify client port configure\n"
		"Port Value\n")
{
	int ret = ERROR;
	int value = 0;
	zpl_int8		sip_address[PJSIP_ADDRESS_MAX];
	value = atoi (argv[0]);
	if (value <= 0)
	{
		vty_out (vty, "%% Malformed port %s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(argc == 2)
	{
		memset(sip_address, 0, sizeof(sip_address));
		pl_pjsip_turn_server_get_api(sip_address, NULL);
		ret = pl_pjsip_turn_server_set_api(sip_address, value);
	}
	else
	{
		memset(sip_address, 0, sizeof(sip_address));
		pl_pjsip_turn_server_get_api(sip_address, NULL);
		ret = pl_pjsip_turn_server_set_api(sip_address, value);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_turn_server_port,
		no_ip_sip_turn_server_port_cmd,
		"no ip sip turn-server port",
		NO_STR
		IP_STR
		"SIP Configure\n"
		"verify client configure\n"
		"verify client port configure\n")
{
	int ret = 0;
	if(argc == 1)
		ret = pl_pjsip_turn_server_set_api(NULL, pl_pjsip->sip_turn_srv.sip_port);
	else
		ret = pl_pjsip_turn_server_set_api(NULL, pl_pjsip->sip_turn_srv.sip_port);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_sip_turn_username,
		ip_sip_turn_username_cmd,
		"ip sip turn username USERNAME",
		IP_STR
		"SIP configure\n"
		"Turn Configure\n"
		"Username Configure\n"
		"USERNAME\n")
{
	int ret = ERROR;
	ret = pl_pjsip_turn_username_set_api(argv[0]);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_turn_username,
		no_ip_sip_turn_username_cmd,
		"no ip sip turn username",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Turn Configure\n"
		"Username Configure\n")
{
	int ret = ERROR;
	ret = pl_pjsip_turn_username_set_api(NULL);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_sip_turn_password,
		ip_sip_turn_password_cmd,
		"ip sip turn password PASSWORD",
		IP_STR
		"SIP configure\n"
		"Turn Configure\n"
		"Password Configure\n"
		"password\n")
{
	int ret = ERROR;
	ret = pl_pjsip_turn_password_set_api(argv[0]);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_turn_password,
		no_ip_sip_turn_password_cmd,
		"no ip sip turn password",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Turn Configure\n"
		"Password Configure\n")
{
	int ret = ERROR;
	ret = pl_pjsip_turn_password_set_api(NULL);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}



//User Agent options:

DEFUN (ip_sip_auto_answer_code,
		ip_sip_auto_answer_code_cmd,
		"ip sip auto-answer-code <1-65535>",
		IP_STR
		"SIP configure\n"
		"auto answer code Configure\n"
		"value\n")
{
	int ret = ERROR;
	ret = pl_pjsip_auto_answer_code_set_api(atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_auto_answer_code,
		no_ip_sip_auto_answer_code_cmd,
		"no ip sip auto-answer-code",
		NO_STR
		IP_STR
		"SIP configure\n"
		"auto answer code Configure\n")
{
	int ret = ERROR;
	ret = pl_pjsip_auto_answer_code_set_api(0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_max_calls,
		ip_sip_max_calls_cmd,
		"ip sip max calls <1-65535>",
		IP_STR
		"SIP configure\n"
		"max calls Configure\n"
		"value\n")
{
	int ret = ERROR;
	ret = pl_pjsip_max_calls_set_api(atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_max_calls,
		no_ip_sip_max_calls_cmd,
		"no ip sip max calls",
		NO_STR
		IP_STR
		"SIP configure\n"
		"max calls Configure\n")
{
	int ret = ERROR;
	ret = pl_pjsip_max_calls_set_api(PJSUA_MAX_CALLS);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_sip_max_call_duration,
		ip_sip_max_call_duration_cmd,
		"ip sip max call duration (<1-65535>|no-limit)",
		IP_STR
		"SIP configure\n"
		"max call Configure\n"
		"duration Configure\n"
		"value\n"
		"no-limit\n")
{
	int ret = ERROR;
	if(strstr(argv[0],"no-limit"))
		ret = pl_pjsip_duration_set_api(PJSUA_APP_NO_LIMIT_DURATION);
	else
		ret = pl_pjsip_duration_set_api(atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_max_call_duration,
		no_ip_sip_max_call_duration_cmd,
		"no ip sip max call duration",
		NO_STR
		IP_STR
		"SIP configure\n"
		"max call Configure\n"
		"duration Configure\n")
{
	int ret = ERROR;
	ret = pl_pjsip_duration_set_api(PJSUA_APP_NO_LIMIT_DURATION);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_sip_norefersub_enable,
		ip_sip_norefersub_enable_cmd,
		"ip sip refer subscription disabled",
		IP_STR
		"SIP configure\n"
		"Turn Configure\n"
		"Enable\n")
{
	int ret = ERROR;
	ret = pl_pjsip_norefersub_set_api(zpl_true);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_norefersub_enable,
		no_ip_sip_norefersub_enable_cmd,
		"no ip sip refer subscription disabled",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Turn Configure\n"
		"Enable\n")
{
	int ret = ERROR;
	ret = pl_pjsip_norefersub_set_api(zpl_false);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_redirect_method,
		ip_sip_redirect_method_cmd,
		"ip sip redirect method (redirect|follow|follow-replace|ask)",
		IP_STR
		"SIP configure\n"
		"Redirect Configure\n"
		"Method Configure\n")
{
	int ret = ERROR;
	if(strstr(argv[0], "redirect"))
		ret = pl_pjsip_accept_redirect_set_api(PJSIP_ACCEPT_REDIRECT_REJECT);
	else if(strstr(argv[0], "follow"))
			ret = pl_pjsip_accept_redirect_set_api(PJSIP_ACCEPT_REDIRECT_FOLLOW);
	else if(strstr(argv[0], "replace"))
			ret = pl_pjsip_accept_redirect_set_api(PJSIP_ACCEPT_REDIRECT_FOLLOW_REPLACE);
	else if(strstr(argv[0], "ask"))
			ret = pl_pjsip_accept_redirect_set_api(PJSIP_ACCEPT_REDIRECT_ASK);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_redirect_method,
		no_ip_sip_redirect_method_cmd,
		"no ip sip redirect method",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Redirect Configure\n"
		"Method Configure\n")
{
	int ret = ERROR;
	ret = pl_pjsip_accept_redirect_set_api(PJSIP_REDIRECT_ACCEPT_REPLACE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

/*
 * Sound Module
 */

/*
 * Playback
 */
DEFUN (voip_playback_volume,
		voip_playback_volume_cmd,
		"voip playback volume <0-100>",
		"VOIP Configure\n"
		"Playback configure\n"
		"Volume Configure\n"
		"volume value in percent\n")
{
	int ret = ERROR;
	if(argc == 1)
	{
		ret = voip_playback_volume_out_set_api(atoi(argv[0]));
	}
	else
	{
		if(strstr(argv[0], "stereo"))
			ret = voip_playback_volume_dac_set_api(atoi(argv[1]));
		else
			ret = voip_playback_volume_mono_set_api(atoi(argv[1]));
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(voip_playback_volume,
		voip_playback_mono_volume_cmd,
		"voip playback volume (stereo|mono) <0-100>",
		"VOIP Configure\n"
		"Playback configure\n"
		"Volume Configure\n"
		"Stereo Configure\n"
		"Mono Configure\n"
		"volume value in percent\n");

DEFUN (no_voip_playback_volume,
		no_voip_playback_volume_cmd,
		"no voip playback volume",
		NO_STR
		"VOIP Configure\n"
		"Playback configure\n"
		"Volume Configure\n")
{
	int ret = ERROR;
	if(argc == 1)
	{
		ret = voip_playback_volume_out_set_api(0);
	}
	else
	{
		if(strstr(argv[0], "stereo"))
			ret = voip_playback_volume_dac_set_api(0);
		else
			ret = voip_playback_volume_mono_set_api(0);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(no_voip_playback_volume,
		no_voip_playback_mono_volume_cmd,
		"no voip playback volume (stereo|mono)",
		NO_STR
		"VOIP Configure\n"
		"Playback configure\n"
		"Volume Configure\n"
		"Stereo Configure\n"
		"Mono Configure\n");

/*
 * Capture
 */
DEFUN (voip_capture_volume,
		voip_capture_volume_cmd,
		"voip capture volume <0-100>",
		"VOIP Configure\n"
		"Capture configure\n"
		"Volume Configure\n"
		"volume value in percent\n")
{
	int ret = ERROR;
	if(argc == 1)
	{
		ret = voip_capture_volume_in_set_api(atoi(argv[0]));
	}
	else
	{
		if(strstr(argv[0], "stereo"))
			ret = voip_capture_volume_adc_set_api(atoi(argv[1]));
		else
			ret = voip_capture_volume_mono_set_api(atoi(argv[1]));
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(voip_capture_volume,
		voip_capture_mono_volume_cmd,
		"voip capture volume (stereo|mono) <0-100>",
		"VOIP Configure\n"
		"Capture configure\n"
		"Volume Configure\n"
		"Stereo Configure\n"
		"Mono Configure\n"
		"volume value in percent\n");

DEFUN (no_voip_capture_volume,
		no_voip_capture_volume_cmd,
		"no voip capture volume",
		NO_STR
		"VOIP Configure\n"
		"Capture configure\n"
		"Volume Configure\n")
{
	int ret = ERROR;
	if(argc == 1)
	{
		ret = voip_capture_volume_in_set_api(0);
	}
	else
	{
		if(strstr(argv[0], "stereo"))
			ret = voip_capture_volume_adc_set_api(0);
		else
			ret = voip_capture_volume_mono_set_api(0);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(no_voip_capture_volume,
		no_voip_capture_mono_volume_cmd,
		"no voip capture volume (stereo|mono)",
		NO_STR
		"VOIP Configure\n"
		"Capture configure\n"
		"Volume Configure\n"
		"Stereo Configure\n"
		"Mono Configure\n");


DEFUN (voip_capture_boost,
		voip_capture_boost_cmd,
		"voip capture boost <0-8>",
		"VOIP Configure\n"
		"Capture configure\n"
		"boost configure\n"
		"boost value\n")
{
	int ret = ERROR;
	ret = voip_volume_boost_set_api(atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_voip_capture_boost,
		no_voip_capture_boost_cmd,
		"no voip capture boost",
		"VOIP Configure\n"
		"Capture configure\n"
		"boost configure\n")
{
	int ret = ERROR;
	ret = voip_volume_boost_set_api(0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (voip_capture_boost_gain,
		voip_capture_boost_gain_cmd,
		"voip capture boost-gain <0-3>",
		"VOIP Configure\n"
		"Capture configure\n"
		"boost gain configure\n"
		"boost value\n")
{
	int ret = ERROR;
	ret = voip_volume_boost_gain_set_api(atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_voip_capture_boost_gain,
		no_voip_capture_boost_gain_cmd,
		"no voip capture boost-cain",
		"VOIP Configure\n"
		"Capture configure\n"
		"boost gain configure\n")
{
	int ret = ERROR;
	ret = voip_volume_boost_gain_set_api(0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


/*
 * call
 */
#if 0
DEFUN (pjsip_call_stop_cli,
		pjsip_call_stop_cli_cmd,
		"pjsip-call stop STRING",
		"SIP Register\n"
		"Stop\n"
		"PhoneNumber\n")
{
	pl_pjsip_app_stop_call(app_config.current_call, zpl_false);
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
#endif

DEFUN (pjsip_restart_cli,
	   pjsip_restart_cli_cmd,
		"pjsip restart",
		"SIP Register\n"
		"Start\n"
		"PhoneNumber\n")
{
	pjsua_app_restart();
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
	pl_pjsip_show_config(vty, zpl_false);
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

#ifdef X5B_APP_DATABASE
/************************************ debug ************************************/
/*
 * Dbtest Module
 */
DEFUN (show_voip_dbase,
		show_voip_dbase_cmd,
		"show voip dbase",
		SHOW_STR
		"Voip Configure\n"
		"Data Base information\n")
{
	int ret = ERROR;
	ret = voip_dbase_show_room_phone(vty, 0, 0, 0, NULL, NULL);
	return  CMD_SUCCESS;
}

DEFUN (show_voip_card_dbase,
		show_voip_card_dbase_cmd,
		"show voip card-dbase",
		SHOW_STR
		"Voip Configure\n"
		"Card Data Base information\n")
{
	int ret = ERROR;
	ret = voip_card_cli_show_all(vty);
	return  CMD_SUCCESS;
}

DEFUN (show_voip_card_dbase_info,
		show_voip_card_dbase_info_cmd,
		"show voip card-dbase info",
		SHOW_STR
		"Voip Configure\n"
		"Card Data Base information\n")
{
	int ret = ERROR;
	ret = show_voip_card_info(vty);
	return  CMD_SUCCESS;
}

DEFUN (show_voip_facecard,
	   show_voip_facecard_cmd,
		"show voip facecard",
		SHOW_STR
		"Voip Configure\n"
		"Face Card Data Base information\n")
{
	int ret = ERROR;
	if(argc == 1)
		ret = voip_facecard_cli_show_all(vty, zpl_true);
	else
		ret = voip_facecard_cli_show_all(vty, zpl_false);
	return  CMD_SUCCESS;
}

ALIAS (show_voip_facecard,
	   show_voip_facecard_info_cmd,
		"show voip facecard (detail|)",
		SHOW_STR
		"Voip Configure\n"
		"Face Card Data Base information\n");
#endif
/************************************ debug ************************************/
DEFUN (debug_voip_app,
		debug_voip_app_cmd,
		"debug voip app",
		DEBUG_STR
		"VOIP Configure\n"
		"APP configure\n")
{
	int ret = ERROR;
	int level = VOIP_APP_DEBUG_EVENT;
	ret = voip_app_debug_set_api(level);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_debug_voip_app,
		no_debug_voip_app_cmd,
		"no debug voip app",
		NO_STR
		DEBUG_STR
		"VOIP Configure\n"
		"APP configure\n")
{
	int ret = ERROR;
	ret = voip_app_debug_set_api(0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (debug_ip_sip,
		debug_ip_sip_cmd,
		"debug ip sip "LOG_LEVELS,
		DEBUG_STR
		IP_STR
		"SIP configure\n"
		LOG_LEVEL_DESC)
{
	int ret = ERROR;
	int level = zlog_priority_match(argv[0]);
	ret = pl_pjsip_debug_level_set_api(level);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_debug_ip_sip,
		no_debug_ip_sip_cmd,
		"no debug ip sip",
		NO_STR
		DEBUG_STR
		IP_STR
		"SIP configure\n"
		LOG_LEVEL_DESC)
{
	int ret = ERROR;
	ret = pl_pjsip_debug_level_set_api(LOG_ERR);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (debug_ip_sip_detail,
		debug_ip_sip_detail_cmd,
		"debug ip sip msg-detail",
		DEBUG_STR
		IP_STR
		"SIP configure\n"
		LOG_LEVEL_DESC)
{
	int ret = ERROR;
	ret = pl_pjsip_debug_detail_set_api(zpl_true);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_debug_ip_sip_detail,
		no_debug_ip_sip_detail_cmd,
		"no debug ip sip",
		NO_STR
		DEBUG_STR
		IP_STR
		"SIP configure\n"
		LOG_LEVEL_DESC)
{
	int ret = ERROR;
	ret = pl_pjsip_debug_detail_set_api(zpl_false);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (show_debugging_voip,
       show_debugging_voip_cmd,
       "show debugging voip",
       SHOW_STR
       "Debugging information\n"
	   "VOIP Configure\n")
{
	int debug_level = 0;
	zpl_bool debug_enable = zpl_false;
	vty_out (vty, "Voip debugging status:%s", VTY_NEWLINE);
	if (VOIP_APP_DEBUG(EVENT))
		vty_out (vty, "  Voip event debugging is on%s", VTY_NEWLINE);

	pl_pjsip_debug_level_get_api(&debug_level);
	if(debug_level)
		vty_out (vty, "  Sip %s debugging is on%s", zlog_priority_name(debug_level), VTY_NEWLINE);

	pl_pjsip_debug_detail_get_api(&debug_enable);
	if(debug_enable)
		vty_out (vty, "  Sip debug msg-detail debugging is on%s", VTY_NEWLINE);
/*
	if (VOIP_STREAM_IS_DEBUG(EVENT))
		vty_out (vty, "  Voip Stream event debugging is on%s", VTY_NEWLINE);

	if (VOIP_STREAM_IS_DEBUG(INFO))
		vty_out (vty, "  Voip Stream info debugging is on%s", VTY_NEWLINE);

	if (VOIP_STREAM_IS_DEBUG(MSG))
		vty_out (vty, "  Voip Stream msg debugging is on%s", VTY_NEWLINE);

	if (VOIP_STREAM_IS_DEBUG(DETAIL))
		vty_out (vty, "  Voip Stream detail debugging is on%s", VTY_NEWLINE);
*/

	//voip_stream_debug_get_api(vty);
	return CMD_SUCCESS;
}

static int pjsip_write_config(struct vty *vty, void *pVoid)
{
	if(pVoid)
	{
		vty_out(vty, "service pjsip%s",VTY_NEWLINE);
		pl_pjsip_write_config(vty);
		return 1;
	}
	return 0;
}

static void cmd_base_sip_init(int node)
{
	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_local_address_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_local_address_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_local_interface_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_local_interface_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_port_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_port_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_server_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_server_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_server_sec_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_server_sec_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_server_port_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_server_port_sec_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_server_port_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_server_port_sec_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_proxy_server_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_proxy_server_sec_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_proxy_server_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_proxy_server_sec_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_proxy_server_port_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_proxy_server_port_sec_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_proxy_server_port_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_proxy_server_port_sec_cmd);


/*
	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_multiuser_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_multiuser_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_active_standby_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_active_standby_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_proxy_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_proxy_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_keepalive_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_keepalive_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_keepalive_interval_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_keepalive_interval_cmd);
*/

/*	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_phone_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_phone_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_phone_sec_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_phone_sec_cmd);*/


	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_username_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_username_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_username_sec_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_username_sec_cmd);


	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_password_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_password_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_password_sec_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_password_sec_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_dtmf_type_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_dtmf_type_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_transport_type_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_transport_type_cmd);

/*
	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_payload_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_payload_cmd);
*/

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_register_interval_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_register_interval_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_realm_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_realm_cmd);
	//install_element(node, CMD_CONFIG_LEVEL, &ip_sip_realm_sec_cmd);
	//install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_realm_sec_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_100_rel_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_100_rel_cmd);


	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_register_delay_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_register_delay_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_register_proxy_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_register_proxy_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_option_set_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_option_set_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_srtp_mode_set_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_srtp_mode_set_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_srtp_secure_set_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_srtp_secure_set_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_srtp_keying_set_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_srtp_keying_set_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_session_timers_set_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_session_timers_set_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_session_timers_sec_set_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_session_timers_sec_set_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_auto_update_nat_set_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_auto_update_nat_set_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_stun_enable_set_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_stun_enable_set_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_ipv6_enable_set_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_ipv6_enable_set_cmd);


	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_tagging_qos_set_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_tagging_qos_set_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_transport_tcpudp_set_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_transport_tcpudp_set_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_nameserver_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_nameserver_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_nameserver_port_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_nameserver_port_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_outbound_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_outbound_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_outbound_port_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_outbound_port_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_stun_server_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_stun_server_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_stun_server_port_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_stun_server_port_cmd);
#if defined(PJSIP_HAS_TLS_TRANSPORT) && (PJSIP_HAS_TLS_TRANSPORT != 0)
	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_tls_enable_set_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_tls_enable_set_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_tls_ca_file_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_tls_ca_file_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_tls_cert_file_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_tls_cert_file_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_tls_private_file_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_tls_private_file_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_tls_password_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_tls_password_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_verify_server_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_verify_server_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_verify_server_port_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_verify_server_port_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_verify_client_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_verify_client_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_verify_client_port_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_verify_client_port_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_tls_cipher_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_tls_cipher_cmd);

#endif
	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_negotiation_timeout_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_negotiation_timeout_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_default_codec_add_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_default_codec_add_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_codec_add_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_codec_add_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_snd_clock_rate_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_snd_clock_rate_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_auto_play_file_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_auto_play_file_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_auto_tone_file_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_auto_tone_file_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_rec_file_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_rec_file_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_auto_option_set_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_auto_option_set_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_media_quality_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_media_quality_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_codec_ptime_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_codec_ptime_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_vad_silence_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_vad_silence_cmd);


	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_echo_tail_len_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_echo_tail_len_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_echo_mode_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_echo_mode_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_ilbc_fps_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_ilbc_fps_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_ice_enable_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_ice_enable_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_ice_regular_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_ice_regular_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_ice_max_host_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_ice_max_host_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_ice_notcp_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_ice_notcp_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_rtp_port_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_rtp_port_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_drop_pct_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_drop_pct_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_turn_enable_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_turn_enable_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_turn_tcp_enable_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_turn_tcp_enable_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_turn_server_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_turn_server_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_turn_server_port_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_turn_server_port_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_turn_username_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_turn_username_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_turn_password_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_turn_password_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_auto_answer_code_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_auto_answer_code_cmd);






	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_max_calls_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_max_calls_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_max_call_duration_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_max_call_duration_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_norefersub_enable_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_norefersub_enable_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &ip_sip_redirect_method_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_ip_sip_redirect_method_cmd);


	/*
	 * sound
	 */
	install_element(node, CMD_CONFIG_LEVEL, &voip_playback_volume_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &voip_playback_mono_volume_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &no_voip_playback_volume_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_voip_playback_mono_volume_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &voip_capture_volume_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &voip_capture_mono_volume_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &no_voip_capture_volume_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_voip_capture_mono_volume_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &voip_capture_boost_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_voip_capture_boost_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &voip_capture_boost_gain_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_voip_capture_boost_gain_cmd);
}

static void cmd_voip_other_init(int node)
{
	install_element(node, CMD_ENABLE_LEVEL, &debug_voip_app_cmd);
	install_element(node, CMD_ENABLE_LEVEL, &no_debug_voip_app_cmd);

	install_element(node, CMD_ENABLE_LEVEL, &debug_ip_sip_cmd);
	install_element(node, CMD_ENABLE_LEVEL, &no_debug_ip_sip_cmd);

	install_element(node, CMD_ENABLE_LEVEL, &debug_ip_sip_detail_cmd);
	install_element(node, CMD_ENABLE_LEVEL, &no_debug_ip_sip_detail_cmd);

	//install_element(node, CMD_CONFIG_LEVEL, &pjsip_call_start_cli_cmd);
	//install_element(node, CMD_CONFIG_LEVEL, &pjsip_call_stop_cli_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &pjsip_restart_cli_cmd);
}

static void cmd_show_sip_init(int node)
{
	install_element(node, CMD_VIEW_LEVEL, &show_ip_sip_server_cmd);
	install_element(node, CMD_VIEW_LEVEL, &show_ip_sip_state_cmd);
#ifdef X5B_APP_DATABASE
	install_element(node, CMD_VIEW_LEVEL, &show_voip_dbase_cmd);
	install_element(node, CMD_VIEW_LEVEL, &show_voip_card_dbase_cmd);
	install_element(node, CMD_VIEW_LEVEL, &show_voip_card_dbase_info_cmd);
#endif
	install_element(node, CMD_VIEW_LEVEL, &show_debugging_voip_cmd);
#ifdef X5B_APP_DATABASE
	install_element(node, CMD_VIEW_LEVEL, &show_voip_facecard_cmd);
	install_element(node, CMD_VIEW_LEVEL, &show_voip_facecard_info_cmd);
#endif
}

void cmd_voip_init(void)
{
	//install_default(SIP_SERVICE_NODE);
	//install_default_basic(SIP_SERVICE_NODE);
	//reinstall_node(SIP_SERVICE_NODE, pl_pjsip_write_config);

	template_t * temp = nsm_template_new (zpl_true);
	if(temp)
	{
		temp->module = 0;
		strcpy(temp->name, "service pjsip");
		strcpy(temp->prompt, "service-sip"); /* (config-app-esp)# */
		//temp->prompt[64];
		//temp->id;
		//temp->pVoid;
		temp->pVoid = NULL;
		temp->write_template = pjsip_write_config;
		nsm_template_install(temp, 0);

		install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &ip_sip_enable_cmd);
		install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &ip_sip_service_cmd);
		install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_ip_sip_service_cmd);

		cmd_base_sip_init(ALL_SERVICE_NODE);

		cmd_show_sip_init(ENABLE_NODE);
		cmd_show_sip_init(CONFIG_NODE);
		cmd_show_sip_init(ALL_SERVICE_NODE);

		cmd_voip_test_init(ENABLE_NODE);

		cmd_voip_other_init(ENABLE_NODE);
	}
}
#endif/* ZPL_PJSIP_MODULE */
