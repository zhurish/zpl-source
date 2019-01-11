/*
 * voip_test.c
 *
 *  Created on: 2019年1月10日
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

#include "voip_def.h"
#include "voip_api.h"
#include "voip_sip.h"
#include "voip_ring.h"
#include "voip_statistics.h"
#include "voip_volume.h"
#include "voip_stream.h"



#ifdef VOIP_STREAM_DEBUG_TEST
/*
 * Stream Module test
 */
DEFUN (voip_start_stream_test,
		voip_start_stream_test_cmd,
		"voip stream-test start "CMD_KEY_IPV4,
		"VOIP Configure\n"
		"Start Configure\n"
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
	if(argc == 1)
	{
		ret = _voip_stream_start_api_shell(argv[0], 0, 0);
	}
	else if(argc == 2)
	{
		ret = _voip_stream_start_api_shell(argv[0], 0, atoi(argv[1]));
	}
	else if(argc == 3)
	{
		ret = _voip_stream_start_api_shell(argv[0], atoi(argv[2]), atoi(argv[1]));
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


ALIAS(voip_start_stream_test,
		voip_start_stream_test_port_cmd,
		"voip stream-test start "CMD_KEY_IPV4 " port <1-65535>",
		"VOIP Configure\n"
		"Start Configure\n"
		CMD_KEY_IPV4_HELP
		"Port configure\n"
		"port value\n");

ALIAS(voip_start_stream_test,
		voip_start_stream_test_port_local_cmd,
		"voip stream-test start "CMD_KEY_IPV4 " port <1-65535> localport <1-65535>",
		"VOIP Configure\n"
		"Start Configure\n"
		CMD_KEY_IPV4_HELP
		"Port configure\n"
		"port value\n"
		"Local Port configure\n"
		"port value\n");

DEFUN (voip_stop_stream_test,
		voip_stop_stream_test_cmd,
		"voip stream-test stop "CMD_KEY_IPV4,
		"VOIP Configure\n"
		"Stop Configure\n"
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
	if(argc == 1)
	{
		ret = voip_stream_stop_api();
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
#endif

#ifdef VOIP_STREAM_API_DEBUG_TEST
DEFUN (voip_start_test,
		voip_start_test_cmd,
		"voip start "CMD_KEY_IPV4,
		"VOIP Configure\n"
		"Start Configure\n"
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
	if(argc == 1)
	{
		ret = _voip_start_api_shell(argv[0], 0, 0);
	}
	else if(argc == 2)
	{
		ret = _voip_start_api_shell(argv[0], 0, atoi(argv[1]));
	}
	else if(argc == 3)
	{
		ret = _voip_start_api_shell(argv[0], atoi(argv[2]), atoi(argv[1]));
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


ALIAS(voip_start_test,
		voip_start_test_port_cmd,
		"voip start "CMD_KEY_IPV4 " port <1-65535>",
		"VOIP Configure\n"
		"Start Configure\n"
		CMD_KEY_IPV4_HELP
		"Port configure\n"
		"port value\n");

ALIAS(voip_start_test,
		voip_start_test_port_local_cmd,
		"voip start "CMD_KEY_IPV4 " port <1-65535> localport <1-65535>",
		"VOIP Configure\n"
		"Start Configure\n"
		CMD_KEY_IPV4_HELP
		"Port configure\n"
		"port value\n"
		"Local Port configure\n"
		"port value\n");

DEFUN (voip_stop_test,
		voip_stop_test_cmd,
		"voip stop "CMD_KEY_IPV4,
		"VOIP Configure\n"
		"Stop Configure\n"
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
	if(argc == 1)
	{
		ret = voip_stream_stop_api();
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
#endif

#ifdef VOIP_STARTUP_TEST
DEFUN (voip_startup_test,
		voip_startup_test_cmd,
		"voip startup test",
		"VOIP Configure\n"
		"Stop Configure\n")
{
	int ret = ERROR;
	{
		ret = _voip_startup_test(NULL);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
#endif


#ifdef VOIP_APP_DEBUG
DEFUN (voip_call_test,
		voip_call_test_cmd,
		"voip call (start|stop) "CMD_KEY_IPV4,
		"VOIP Configure\n"
		"Call Configure\n"
		"Start Configure\n"
		"Stop Configure\n"
		CMD_KEY_IPV4_HELP)
{
	int ret = ERROR;
	BOOL enable = TRUE;
	struct prefix address;
	prefix_zero(&address);
	ret = str2prefix_ipv4 (argv[1], (struct prefix_ipv4 *)&address);
	if (ret <= 0)
	{
		vty_out (vty, "%% Malformed address %s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(strstr(argv[0], "start"))
		enable = TRUE;
	else
		enable = FALSE;
	if(argc == 2)
	{
		ret = voip_app_call_test(enable, argv[1], 0, 0);
	}
	else if(argc == 3)
	{
		ret = voip_app_call_test(enable, argv[1], atoi(argv[2]), 0);
	}
	else if(argc == 4)
	{
		ret = voip_app_call_test(enable, argv[1], atoi(argv[2]), atoi(argv[3]));
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(voip_call_test,
		voip_call_test_port_cmd,
		"voip call (start|stop) "CMD_KEY_IPV4 " port <1-65535>",
		"VOIP Configure\n"
		"Call Configure\n"
		"Start Configure\n"
		"Stop Configure\n"
		CMD_KEY_IPV4_HELP
		"Port configure\n"
		"port value\n");

ALIAS(voip_call_test,
		voip_call_test_port_local_cmd,
		"voip call (start|stop) "CMD_KEY_IPV4 " port <1-65535> localport <1-65535>",
		"VOIP Configure\n"
		"Call Configure\n"
		"Start Configure\n"
		"Stop Configure\n"
		CMD_KEY_IPV4_HELP
		"Port configure\n"
		"port value\n"
		"Local Port configure\n"
		"port value\n");

#endif



void cmd_voip_test_init(int node)
{
#ifdef VOIP_STREAM_DEBUG_TEST
	install_element(node, &voip_start_stream_test_cmd);
	install_element(node, &voip_start_stream_test_port_cmd);
	install_element(node, &voip_start_stream_test_port_local_cmd);
	install_element(node, &voip_stop_stream_test_cmd);
#endif
#ifdef VOIP_STREAM_API_DEBUG_TEST
	install_element(node, &voip_start_test_cmd);
	install_element(node, &voip_start_test_port_cmd);
	install_element(node, &voip_start_test_port_local_cmd);
	install_element(node, &voip_stop_test_cmd);
#endif

#ifdef VOIP_APP_DEBUG
	install_element(node, &voip_call_test_cmd);
	install_element(node, &voip_call_test_port_cmd);
	install_element(node, &voip_call_test_port_local_cmd);
#endif

#ifdef VOIP_STARTUP_TEST
	install_element(node, &voip_startup_test_cmd);
#endif
}
