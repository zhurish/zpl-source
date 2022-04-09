/*
 * cmd_sntp.c
 *
 *  Created on: Jan 2, 2018
 *      Author: zhurish
 */


#include "auto_include.h"
#include <zplos_include.h>
#include "zmemory.h"
#include "vty.h"
#include "command.h"


#ifdef ZPL_SERVICE_SNTPS
#include "sntpsLib.h"
#ifndef SNTPS_CLI_ENABLE

DEFUN (sntps_server_enable,
		sntps_server_enable_cmd,
	    "sntp server enable",
		"sntp protocol configure\n"
		"sntp server configure\n"
		"enable sntp server\n")
{
	int enable = 1;
	return sntp_server_set_api(vty, API_SNTPS_SET_ENABLE, &enable);
}
DEFUN (no_sntps_server_enable,
		no_sntps_server_enable_cmd,
	    "no sntp server enable",
		NO_STR
		"sntp protocol configure\n"
		"sntp server configure\n"
		"enable sntp server\n")
{
	return sntp_server_set_api(vty, API_SNTPS_SET_ENABLE, NULL);
}

DEFUN (sntps_server_mode,
		sntps_server_mode_cmd,
	    "sntp server mode (broadcast|unicast|multicast)",
		"sntp protocol configure\n"
		"sntp server configure\n"
		"sntp server mode configure\n"
		"sntp server broadcast mode\n"
		"sntp server unicast mode\n"
		"sntp server multicast mode\n")

{
	return sntp_server_set_api(vty, API_SNTPS_SET_MODE, argv[0]);
}

DEFUN (no_sntps_server_mode,
		no_sntps_server_mode_cmd,
	    "no sntp server mode",
		NO_STR
		"sntp protocol configure\n"
		"sntp server configure\n"
		"enable sntp server\n")
{
	return sntp_server_set_api(vty, API_SNTPS_SET_MODE, NULL);
}
DEFUN (sntps_server_interval,
		sntps_server_interval_cmd,
	    "sntp server interval <60-3600>",
		"sntp protocol configure\n"
		"sntp server configure\n"
		"sntp server poll-interval configure\n"
		"poll-interval value in sec\n")
{
	return sntp_server_set_api(vty, API_SNTPS_SET_INTERVAL, argv[0]);
}
DEFUN (no_sntps_server_interval,
		no_sntps_server_interval_cmd,
	    "no sntp server interval",
		NO_STR
		"sntp protocol configure\n"
		"sntp server configure\n"
		"poll-interval\n")
{
	return sntp_server_set_api(vty, API_SNTPS_SET_INTERVAL, NULL);
}
DEFUN (sntps_server_listen_port,
		sntps_server_listen_port_cmd,
	    "sntp server ipstack_listen-port <100-65536>",
		"sntp protocol configure\n"
		"sntp server configure\n"
		"sntp server ipstack_listen port configure\n"
		"sntp server local UDP port\n")
{
	return sntp_server_set_api(vty, API_SNTPS_SET_LISTEN, argv[0]);
}
DEFUN (no_sntps_server_listen_port,
		no_sntps_server_listen_port_cmd,
	    "no sntp server ipstack_listen-port",
		NO_STR
		"sntp protocol configure\n"
		"sntp server configure\n"
		"sntp server ipstack_listen port configure\n")
{
	return sntp_server_set_api(vty, API_SNTPS_SET_LISTEN, NULL);
}
DEFUN (sntps_version,
		sntps_version_cmd,
	    "sntp server version (1|2|3|4)",
		"sntp protocol configure\n"
		"sntp server configure\n"
		"sntp protocol version information\n"
		"sntp protocol version 1\n"
		"sntp protocol version 2\n"
		"sntp protocol version 3\n"
		"sntp protocol version 4\n")
{
	return sntp_server_set_api(vty, API_SNTPS_SET_VERSION, argv[0]);
}

DEFUN (no_sntps_version,
		no_sntps_version_cmd,
	    "no sntp server version",
		NO_STR
		"sntp protocol configure\n"
		"sntp server configure\n"
		"sntp protocol version information\n")
{
	return sntp_server_set_api(vty, API_SNTPS_SET_VERSION, NULL);
}


DEFUN (sntps_debug,
		sntps_debug_cmd,
	    "debug sntp server",
		DEBUG_STR
		"sntp protocol configure\n"
		"SNTP Server timespec infomation\n")
{
	int debug = 1;
	return sntp_server_set_api(vty, API_SNTPS_SET_DEBUG, &debug);
}
DEFUN (no_sntps_debug,
		no_sntps_debug_cmd,
	    "no debug sntp server",
		NO_STR
		DEBUG_STR
		"sntp protocol configure\n"
		"SNTP Server timespec infomation\n")
{
	return sntp_server_set_api(vty, API_SNTPS_SET_DEBUG, NULL);
}

DEFUN (sntps_mutilcast_ttl,
		sntps_mutilcast_ttl_cmd,
	    "sntp server mutilcast ttl <2-255>",
		DEBUG_STR
		"sntp protocol configure\n"
		"sntp server configure\n"
		"sntp mutilcast information\n"
		"mutilcast IPSTACK_TTL configure\n"
		"mutilcast IPSTACK_TTL value\n")
{
	return sntp_server_set_api(vty, API_SNTPS_SET_TTL, argv[0]);
}

DEFUN (no_sntps_mutilcast_ttl,
		no_sntps_mutilcast_ttl_cmd,
	    "sntp server mutilcast ttl",
		DEBUG_STR
		"sntp protocol configure\n"
		"sntp server configure\n"
		"sntp mutilcast information\n"
		"mutilcast IPSTACK_TTL configure\n")
{
	return sntp_server_set_api(vty, API_SNTPS_SET_TTL, NULL);
}

DEFUN (show_sntps_server,
		show_sntps_server_cmd,
	    "show sntp server",
		SHOW_STR
		"sntp protocol configure\n"
		"sntp server configure\n")
{
	return vty_show_sntps_server(vty);
}

#endif
#endif

#ifdef ZPL_SERVICE_SNTPC
#include "sntpcLib.h"
#ifndef SNTPC_CLI_ENABLE
DEFUN (sntp_enable,
		sntp_enable_cmd,
	    "sntp server (A.B.C.D|dynamics)",
		"sntp protocol configure\n"
		"sntp server configure\n"
		"sntp server ip address format A.B.C.D \n"
		"sntp server dynamics\n")
{
	int ret = 0;
	int port = SNTPC_SERVER_PORT;
	int time_interval = LOCAL_UPDATE_GMT;

	if(argc >= 2 && argv[1])
	{
		port = atoi(argv[1]);
		ret |= sntpc_client_set_api(vty, API_SNTPC_SET_PORT, &port);
	}

	if(argc == 3 && argv[2])
	{
		time_interval = atoi(argv[2]);
		ret |= sntpc_client_set_api(vty, API_SNTPC_SET_INTERVAL, &time_interval);
	}
	if(ret == 0)
	{
		int enable = 1;
		if(strstr(argv[0], "."))
		{
			if(sntpc_is_dynamics())
				sntpc_dynamics_disable();
			ret = sntpc_client_set_api(vty, API_SNTPC_SET_ADDRESS, argv[0]);
			if(ret == OK)
				return sntpc_client_set_api(vty, API_SNTPC_SET_ENABLE, &enable);
		}
		else
		{
			sntpc_dynamics_enable();
			return sntpc_client_set_api(vty, API_SNTPC_SET_ENABLE, &enable);
		}
	}
	return CMD_WARNING;
}

ALIAS(sntp_enable,
		sntp_enable_port_cmd,
		"sntp server (A.B.C.D|dynamics) port <100-65536>",
		"sntp protocol configure\n"
		"sntp server configure\n"
		"sntp server ip address format A.B.C.D \n"
		"sntp server dynamics\n"
		"sntp server port configure\n"
		"udp port of sntp server\n")

ALIAS(sntp_enable,
		sntp_enable_port_interval_cmd,
	    "sntp server (A.B.C.D|dynamics) port <100-65536> interval <30-3600>",
		"sntp protocol configure\n"
		"sntp server configure\n"
		"sntp server ip address format A.B.C.D \n"
		"sntp server dynamics\n"
		"sntp server port configure\n"
		"udp port of sntp server\n"
		"sntp server ipstack_send request interval\n"
		"time interval of sec\n")

DEFUN (no_sntp_enable,
		no_sntp_enable_cmd,
		"no sntp server",
		NO_STR
		"sntp protocol configure\n"
		"sntp server configure\n")
{
	return sntpc_client_set_api(vty, API_SNTPC_SET_ENABLE, NULL);
}

DEFUN (sntp_version,
		sntp_version_cmd,
	    "sntp client version (1|2|3|4)",
		"sntp protocol configure\n"
		"sntp client configure\n"
		"sntp protocol version information\n"
		"sntp protocol version 1\n"
		"sntp protocol version 2\n"
		"sntp protocol version 3\n"
		"sntp protocol version 4\n")
{
	return sntpc_client_set_api(vty, API_SNTPC_SET_VERSION, argv[0]);
}

DEFUN (no_sntp_version,
		no_sntp_version_cmd,
	    "no sntp client version",
		NO_STR
		"sntp protocol configure\n"
		"sntp client configure\n"
		"sntp protocol version information\n")

{
	return sntpc_client_set_api(vty, API_SNTPC_SET_VERSION, NULL);
}

DEFUN (clock_timezone,
		clock_timezone_cmd,
	    "clock timezone <0-24>",
		"clock timezone configure\n"
		"local clock timezone configure\n"
		"local clock timezone value(default 8).\n")
{
	return sntpc_client_set_api(vty, API_SNTPC_SET_TIMEZONE, argv[0]);
}

DEFUN (no_clock_timezone,
		no_clock_timezone_cmd,
	    "no clock timezone",
		NO_STR
		"clock timezone configure\n"
		"local clock timezone configure(default 8).\n")
{
	return sntpc_client_set_api(vty, API_SNTPC_SET_TIMEZONE, NULL);
}

DEFUN (sntp_passive,
		sntp_passive_cmd,
	    "sntp client passive",
		"sntp protocol configure\n"
		"sntp server configure\n"
		"sntp server passive type\n")

{
	int passive = 1;
	return sntpc_client_set_api(vty, API_SNTPC_SET_PASSIVE, &passive);
}

DEFUN (no_sntp_passive,
		no_sntp_passive_cmd,
	    "no sntp client passive",
		NO_STR
		"sntp protocol configure\n"
		"sntp server configure\n"
		"sntp server passive type\n")
{
	return sntpc_client_set_api(vty, API_SNTPC_SET_PASSIVE, NULL);
}


DEFUN (sntp_debug,
		sntp_debug_cmd,
	    "debug sntp client",
		DEBUG_STR
		"sntp protocol configure\n"
		"SNTP Client timespec infomation\n")
{
	int debug = 1;
	return sntpc_client_set_api(vty, API_SNTPC_SET_DEBUG, &debug);
}

DEFUN (no_sntp_debug,
		no_sntp_debug_cmd,
	    "no debug sntp client",
		NO_STR
		DEBUG_STR
		"sntp protocol configure\n"
		"SNTP Client timespec infomation\n")
{
	return sntpc_client_set_api(vty, API_SNTPC_SET_DEBUG, NULL);
}

DEFUN (show_sntp_client,
		show_sntp_client_cmd,
	    "show sntp client",
		SHOW_STR
		"sntp protocol configure\n"
		"sntp server configure\n")
{
	return vty_show_sntpc_client(vty);
}

#endif


int cmd_sntpc_init(void)
{
#ifndef SNTPC_CLI_ENABLE
	install_element (ENABLE_NODE, CMD_VIEW_LEVEL, &show_sntp_client_cmd);

	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &sntp_enable_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &sntp_enable_port_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &sntp_enable_port_interval_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_sntp_enable_cmd);

	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &sntp_version_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_sntp_version_cmd);

	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &clock_timezone_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_clock_timezone_cmd);

	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &sntp_passive_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_sntp_passive_cmd);

	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &sntp_debug_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_sntp_debug_cmd);
#endif
	return 0;
}
#endif

#ifdef ZPL_SERVICE_SNTPS
int cmd_sntps_init(void)
{
#ifndef SNTPS_CLI_ENABLE
	install_element (ENABLE_NODE, CMD_VIEW_LEVEL, &show_sntps_server_cmd);

	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &sntps_server_enable_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_sntps_server_enable_cmd);

	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &sntps_server_listen_port_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_sntps_server_listen_port_cmd);


	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &sntps_version_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_sntps_version_cmd);

	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &sntps_server_interval_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_sntps_server_interval_cmd);

	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &sntps_server_mode_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_sntps_server_mode_cmd);

	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &sntps_mutilcast_ttl_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_sntps_mutilcast_ttl_cmd);

	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &sntps_debug_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_sntps_debug_cmd);
#endif
	return 0;
}
#endif
