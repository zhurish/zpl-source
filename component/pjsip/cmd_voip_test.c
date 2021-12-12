/*
 * voip_test.c
 *
 *  Created on: 2019年1月10日
 *      Author: DELL
 */

#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"
#include "vty_include.h"

#ifdef ZPL_PJSIP_MODULE

#include "voip_app.h"


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
		//extern int voip_app_start_call_event_cli_web(app_call_source_t source, zpl_uint8 building,
		//		zpl_uint8 unit, zpl_uint16 room, char *number);

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
		"call phone NUM",
		"Call Configure\n"
		"Phone Configure\n"
		"Phone Number\n")
{
	int ret = ERROR;
	{
/*		voip_event_t ev;
		memset(&ev, 0, sizeof(voip_event_t));
		strcpy(ev.data, argv[0]);
		ev.dlen = strlen(ev.data);
		ret = voip_app_start_call_event_ui_phone(&ev);*/
		ret = voip_app_start_call_event_cli_web(APP_CALL_ID_CLI, 0, 0, 0, argv[0]);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


void cmd_voip_test_init(int node)
{
	install_element(node, CMD_CONFIG_LEVEL, &app_start_test_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &app_start_test_room_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &app_start_test_building_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &app_stop_test_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &app_stop_phone_test_cmd);
}
#endif
