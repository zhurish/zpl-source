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
#ifdef PL_VOIP_MODULE
#include "voip_app.h"
#include "voip_api.h"


/*
 * SIP Module
 */


/*
 * SIP Global
 */
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
		if(!voip_sip_isenable())
			ret = voip_sip_enable(TRUE);
		else
			ret = OK;
	}
	else
	{
		if(voip_sip_isenable())
			ret = voip_sip_enable(FALSE);
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
	if(!voip_sip_isenable())
		ret = voip_sip_enable(TRUE);
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
	if(voip_sip_isenable())
		ret = voip_sip_enable(FALSE);
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
	ret = voip_sip_local_address_set_api(ntohl(address.u.prefix4.s_addr));
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
	ret = voip_sip_local_address_set_api(0);
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
			ret = voip_sip_source_interface_set_api(ifp->ifindex);
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
	ret = voip_sip_source_interface_set_api(0);
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
	ret = voip_sip_local_port_set_api(value);
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
	ret = voip_sip_local_port_set_api(value);
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
		ret = voip_sip_server_set_api(ntohl(address.u.prefix4.s_addr), sip_config->sip_port_sec, TRUE);
	else
		ret = voip_sip_server_set_api(ntohl(address.u.prefix4.s_addr), sip_config->sip_port, FALSE);
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
		ret = voip_sip_server_set_api(0, sip_config->sip_port_sec, TRUE);
	else
		ret = voip_sip_server_set_api(0, sip_config->sip_port, FALSE);
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
	value = atoi (argv[0]);
	if (value <= 0)
	{
		vty_out (vty, "%% Malformed port %s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(argc == 2)
		ret = voip_sip_server_set_api(sip_config->sip_server_sec, value, TRUE);
	else
		ret = voip_sip_server_set_api(sip_config->sip_server, value, FALSE);
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
	if(argc == 1)
		ret = voip_sip_server_set_api(sip_config->sip_server_sec, 0, TRUE);
	else
		ret = voip_sip_server_set_api(sip_config->sip_server, 0, FALSE);
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
		ret = voip_sip_proxy_server_set_api(ntohl(address.u.prefix4.s_addr), sip_config->sip_port_sec, TRUE);
	else
		ret = voip_sip_proxy_server_set_api(ntohl(address.u.prefix4.s_addr), sip_config->sip_port, FALSE);
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
		ret = voip_sip_proxy_server_set_api(0, sip_config->sip_port_sec, TRUE);
	else
		ret = voip_sip_proxy_server_set_api(0, sip_config->sip_port, FALSE);
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
	value = atoi (argv[0]);
	if (value <= 0)
	{
		vty_out (vty, "%% Malformed port %s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(argc == 2)
		ret = voip_sip_proxy_server_set_api(sip_config->sip_proxy_server_sec, value, TRUE);
	else
		ret = voip_sip_proxy_server_set_api(sip_config->sip_proxy_server, value, FALSE);
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
	if(argc == 1)
		ret = voip_sip_proxy_server_set_api(sip_config->sip_proxy_server_sec, 0, TRUE);
	else
		ret = voip_sip_proxy_server_set_api(sip_config->sip_proxy_server, 0, FALSE);
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
	sip_transport_t proto = SIP_PROTO_UDP;
	if(strstr(argv[0], "udp"))
	{
		proto = SIP_PROTO_UDP;
	}
	else if(strstr(argv[0], "tcp"))
	{
		proto = SIP_PROTO_TCP;
	}
	else if(strstr(argv[0], "dtls"))
	{
		proto = SIP_PROTO_DTLS;
	}
	else if(strstr(argv[0], "tls"))
	{
		proto = SIP_PROTO_TLS;
	}
	ret = voip_sip_transport_proto_set_api(proto);
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
	ret = voip_sip_transport_proto_set_api(SIP_PROTO_UDP);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ip_sip_payload,
		ip_sip_payload_cmd,
		"ip sip payload "VOIP_PAYLOAD_STR,
		IP_STR
		"SIP Protocol\n"
		"Payload configure(default:pcmu)\n"
		VOIP_PAYLOAD_STR_HELP)
{
	int ret = ERROR;
	ret = voip_sip_payload_name_set_api( argv[0]);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_payload,
		no_ip_sip_payload_cmd,
		"no ip sip payload",
		NO_STR
		IP_STR
		"SIP Protocol\n"
		"Payload configure\n")
{
	int ret = ERROR;
	ret = voip_sip_payload_name_set_api("PCMU");
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
	sip_dtmf_t dtmf = VOIP_SIP_INFO;
	if(strstr(argv[0], "info"))
	{
		dtmf = VOIP_SIP_INFO;
	}
	else if(strstr(argv[0], "rfc2833"))
	{
		dtmf = VOIP_SIP_RFC2833;
	}
	else if(strstr(argv[0], "inband"))
	{
		dtmf = VOIP_SIP_INBAND;
	}
	ret = voip_sip_dtmf_set_api(dtmf);
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
	ret = voip_sip_dtmf_set_api(VOIP_SIP_INFO);
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



/*
 * SIP User
 */

DEFUN (ip_sip_multiuser,
		ip_sip_multiuser_cmd,
		"ip sip multiuser (enable|disable)",
		IP_STR
		"SIP configure\n"
		"Multiuser configure\n"
		"Enable\n"
		"Disnable\n")
{
	int ret = ERROR;
	if(strstr(argv[0], "enable"))
		ret = voip_sip_multiuser_set_api(TRUE);
	else if(strstr(argv[0], "disable"))
		ret = voip_sip_multiuser_set_api(FALSE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_multiuser,
		no_ip_sip_multiuser_cmd,
		"no ip sip multiuser enable",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Multiuser configure\n"
		"Enable\n")
{
	int ret = ERROR;
	ret = voip_sip_multiuser_set_api(FALSE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_active_standby,
		ip_sip_active_standby_cmd,
		"ip sip active-standby (enable|disable)",
		IP_STR
		"SIP configure\n"
		"Active/Standby configure\n"
		"Enable\n"
		"Disnable\n")
{
	int ret = ERROR;
	if(strstr(argv[0], "enable"))
		ret = voip_sip_active_standby_set_api(TRUE);
	else if(strstr(argv[0], "disable"))
		ret = voip_sip_active_standby_set_api(FALSE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_active_standby,
		no_ip_sip_active_standby_cmd,
		"no ip sip active-standby enable",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Active/Standby configure\n"
		"Enable\n")
{
	int ret = ERROR;
	ret = voip_sip_active_standby_set_api(FALSE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_proxy,
		ip_sip_proxy_cmd,
		"ip sip proxy (enable|disable)",
		IP_STR
		"SIP configure\n"
		"Proxy configure\n"
		"Enable\n"
		"Disnable\n")
{
	int ret = ERROR;
	if(strstr(argv[0], "enable"))
		ret = voip_sip_proxy_enable_set_api(TRUE);
	else if(strstr(argv[0], "disable"))
		ret = voip_sip_proxy_enable_set_api(FALSE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_proxy,
		no_ip_sip_proxy_cmd,
		"no ip sip proxy enable",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Proxy configure\n"
		"Enable\n")
{
	int ret = ERROR;
	ret = voip_sip_proxy_enable_set_api(FALSE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_keepalive,
		ip_sip_keepalive_cmd,
		"ip sip keepalive (enable|disable)",
		IP_STR
		"SIP configure\n"
		"Keepalive configure\n"
		"Enable\n"
		"Disnable\n")
{
	int ret = ERROR;
	if(strstr(argv[0], "enable"))
		ret = voip_sip_keepalive_set_api(TRUE);
	else if(strstr(argv[0], "disable"))
		ret = voip_sip_keepalive_set_api(FALSE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_keepalive,
		no_ip_sip_keepalive_cmd,
		"no ip sip keepalive enable",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Keepalive configure\n"
		"Enable\n")
{
	int ret = ERROR;
	ret = voip_sip_keepalive_set_api(FALSE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ip_sip_keepalive_interval,
		ip_sip_keepalive_interval_cmd,
		"ip sip keepalive-interval <1-60>",
		IP_STR
		"SIP configure\n"
		"Keepalive Interval configure\n"
		"Interval Value\n")
{
	int ret = ERROR;
	ret = voip_sip_keepalive_interval_set_api(atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ip_sip_keepalive_interval,
		no_ip_sip_keepalive_interval_cmd,
		"no ip sip keepalive-interval",
		NO_STR
		IP_STR
		"SIP configure\n"
		"Keepalive Interval configure\n")
{
	int ret = ERROR;
	ret = voip_sip_keepalive_interval_set_api(0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

/*
 * SIP User
 */
DEFUN (ip_sip_phone,
		ip_sip_phone_cmd,
		"ip sip local-phone STRING",
		IP_STR
		"SIP configure\n"
		"local-phone configure\n"
		"phone number\n")
{
	int ret = ERROR;
	if(argc == 2)
		ret = voip_sip_local_number_set_api(argv[0], TRUE);
	else
		ret = voip_sip_local_number_set_api(argv[0], FALSE);
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
	if(argc == 1)
		ret = voip_sip_local_number_set_api(NULL, TRUE);
	else
		ret = voip_sip_local_number_set_api(NULL, FALSE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(no_ip_sip_phone,
		no_ip_sip_phone_sec_cmd,
		"no ip sip local-phone (secondary|)",
		NO_STR
		IP_STR
		"SIP configure\n"
		"local-phone configure\n"
		"Secondary\n");


DEFUN (ip_sip_username,
		ip_sip_username_cmd,
		"ip sip username STRING",
		IP_STR
		"SIP configure\n"
		"username configure\n"
		"username\n")
{
	int ret = ERROR;
	if(argc == 2)
		ret = voip_sip_user_set_api(argv[0], TRUE);
	else
		ret = voip_sip_user_set_api(argv[0], FALSE);
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
	if(argc == 1)
		ret = voip_sip_user_set_api(NULL, TRUE);
	else
		ret = voip_sip_user_set_api(NULL, FALSE);
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
	if(argc == 2)
		ret = voip_sip_password_set_api(argv[0], TRUE);
	else
		ret = voip_sip_password_set_api(argv[0], FALSE);
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
	if(argc == 1)
		ret = voip_sip_password_set_api(NULL, TRUE);
	else
		ret = voip_sip_password_set_api(NULL, FALSE);
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
	if(argc == 2)
		ret = voip_sip_realm_set_api(argv[0], TRUE);
	else
		ret = voip_sip_realm_set_api(argv[0], FALSE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(ip_sip_realm,
		ip_sip_realm_sec_cmd,
		"ip sip realm STRING (secondary|)",
		IP_STR
		"SIP configure\n"
		"realm configure\n"
		"STRING\n"
		"Secondary\n");


DEFUN (no_ip_sip_realm,
		no_ip_sip_realm_cmd,
		"no ip sip realm",
		NO_STR
		IP_STR
		"SIP configure\n"
		"realm configure\n")
{
	int ret = ERROR;
	if(argc == 1)
		ret = voip_sip_realm_set_api(NULL, TRUE);
	else
		ret = voip_sip_realm_set_api(NULL, FALSE);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(no_ip_sip_realm,
		no_ip_sip_realm_sec_cmd,
		"no ip sip realm (secondary|)",
		IP_STR
		"SIP configure\n"
		"realm configure\n"
		"Secondary\n");


#if 0

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

#endif




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
	ret = voip_osip_set_log_level(level, 1);
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
	ret = voip_osip_set_log_level(LOG_ERR, 0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (show_debugging_sip,
       show_debugging_sip_cmd,
       "show debugging sip",
       SHOW_STR
       "Debugging information\n"
	   "SIP Configure\n")
{
	int levelmask = 0;
	voip_osip_get_log_level(&levelmask, NULL);

	vty_out (vty, "Sip debugging status:%s", VTY_NEWLINE);
	if (levelmask & (1<<LOG_ALERT))
		vty_out (vty, "  Sip alerts debugging is on%s", VTY_NEWLINE);

	if (levelmask & (1<<LOG_EMERG))
		vty_out (vty, "  Sip emergencies debugging is on%s", VTY_NEWLINE);

	if (levelmask & (1<<LOG_ERR))
		vty_out (vty, "  Sip errors debugging is on%s", VTY_NEWLINE);

	if (levelmask & (1<<LOG_WARNING))
		vty_out (vty, "  Sip emergencies debugging is on%s", VTY_NEWLINE);

	if (levelmask & (1<<LOG_NOTICE))
		vty_out (vty, "  Sip notifications debugging is on%s", VTY_NEWLINE);

	if (levelmask & (1<<LOG_DEBUG))
		vty_out (vty, "  Sip debugging debugging is on%s", VTY_NEWLINE);

	return CMD_SUCCESS;
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
	voip_sip_show_config(vty, FALSE);
	return CMD_SUCCESS;
}


DEFUN (sip_reg_cli,
		sip_reg_cli_cmd,
		"sip-register (start|stop)",
		"SIP Register\n"
		"Start\n"
		"Stop\n")
{
	if(strstr(argv[0], "start"))
		voip_app_sip_register_start(TRUE);
	else if(strstr(argv[0], "stop"))
		voip_app_sip_register_start(FALSE);
	return  CMD_SUCCESS;
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

	install_element(node, &ip_sip_phone_cmd);
	install_element(node, &no_ip_sip_phone_cmd);
	install_element(node, &ip_sip_phone_sec_cmd);
	install_element(node, &no_ip_sip_phone_sec_cmd);

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

	install_element(node, &ip_sip_payload_cmd);
	install_element(node, &no_ip_sip_payload_cmd);

	install_element(node, &ip_sip_register_interval_cmd);
	install_element(node, &no_ip_sip_register_interval_cmd);

	install_element(node, &ip_sip_realm_cmd);
	install_element(node, &no_ip_sip_realm_cmd);
	install_element(node, &ip_sip_realm_sec_cmd);
	install_element(node, &no_ip_sip_realm_sec_cmd);

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
	install_element(node, &sip_reg_cli_cmd);
	install_element(node, &show_debugging_sip_cmd);
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

	install_element(VIEW_NODE, &debug_ip_sip_cmd);
	install_element(VIEW_NODE, &no_debug_ip_sip_cmd);
}
#endif
