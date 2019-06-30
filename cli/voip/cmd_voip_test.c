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

#ifdef PL_VOIP_MODULE
#ifdef PL_PJSIP_MODULE
#include "voip_app.h"
#endif
#ifdef PL_OSIP_MODULE
#include "voip_app.h"
#include "voip_api.h"




#ifdef VOIP_STREAM_RAW
/*
 * Stream Module test
 */
DEFUN (media_start_test,
		media_start_test_cmd,
		"media start "CMD_KEY_IPV4,
		"Media Configure\n"
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


ALIAS(media_start_test,
		media_start_test_port_cmd,
		"media start "CMD_KEY_IPV4 " port <1-65535>",
		"Media Configure\n"
		"Start Configure\n"
		CMD_KEY_IPV4_HELP
		"Port configure\n"
		"port value\n");

ALIAS(media_start_test,
		media_start_test_port_local_cmd,
		"media start "CMD_KEY_IPV4 " port <1-65535> localport <1-65535>",
		"Media Configure\n"
		"Start Configure\n"
		CMD_KEY_IPV4_HELP
		"Port configure\n"
		"port value\n"
		"Local Port configure\n"
		"port value\n");

DEFUN (media_stop_test,
		media_stop_test_cmd,
		"media stop",
		"Media Configure\n"
		"Stop Configure\n")
{
	int ret = ERROR;
	{
		ret = voip_stream_stop_api();
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

#endif
#endif

DEFUN (app_start_test,
		app_start_test_cmd,
		"call start NUMBER",
		"Call Configure\n"
		"Start Configure\n"
		"Phone Number\n")
{
	int ret = ERROR;
	if(argc == 1)
	{
		//extern int voip_app_start_call_event_cli_web(app_call_source_t source, u_int8 building,
		//		u_int8 unit, u_int16 room, char *number);

		ret = voip_app_start_call_event_cli_web(APP_CALL_ID_CLI, 0, 0, 0, argv[0]);
	}
	else if(argc == 2)
	{
		ret = voip_app_start_call_event_cli_web(APP_CALL_ID_CLI, 0, 0, atoi(argv[0]), argv[1]);
	}
	else if(argc == 3)
	{
		ret = voip_app_start_call_event_cli_web(APP_CALL_ID_CLI, atoi(argv[0]), 0, atoi(argv[1]), argv[2]);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS(app_start_test,
		app_start_test_room_cmd,
		"call start room <1-65530> NUMBER",
		"Call Configure\n"
		"Start Configure\n"
		"Room Number\n"
		"Phone Number\n");

ALIAS(app_start_test,
		app_start_test_building_cmd,
		"call start building <1-256> room <1-65530> NUMBER",
		"Call Configure\n"
		"Start Configure\n"
		"Building Number\n"
		"Room Number\n"
		"Phone Number\n");

DEFUN (app_stop_test,
		app_stop_test_cmd,
		"call stop",
		"Call Configure\n"
		"Stop Configure\n")
{
	int ret = ERROR;
	{
		ret = voip_app_stop_call_event_cli_web(NULL);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (app_stop_phone_test,
		app_stop_phone_test_cmd,
		"call-phone NUM",
		"Call Configure\n"
		"Stop Configure\n")
{
	int ret = ERROR;
	{
		voip_event_t ev;
		memset(&ev, 0, sizeof(voip_event_t));
		strcpy(ev.data, argv[0]);
		ev.dlen = strlen(ev.data);
		ret = voip_app_start_call_event_ui_phone(&ev);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


void cmd_voip_test_init(int node)
{
#ifdef PL_OSIP_MODULE
#ifdef VOIP_STREAM_RAW
	install_element(node, &media_start_test_cmd);
	install_element(node, &media_start_test_port_cmd);
	install_element(node, &media_start_test_port_local_cmd);
	install_element(node, &media_stop_test_cmd);
#endif
#endif
	install_element(node, &app_start_test_cmd);
	install_element(node, &app_start_test_room_cmd);
	install_element(node, &app_start_test_building_cmd);
	install_element(node, &app_stop_test_cmd);
	install_element(node, &app_stop_phone_test_cmd);
}
#endif
