/*
 * cmd_nvram_env.c
 *
 *  Created on: 2019年3月5日
 *      Author: DELL
 */



#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
//#include "nsm_include.h"
#include "vty_include.h"


DEFUN (show_process,
		show_process_cmd,
		"show process cpu [NAME]",
		SHOW_STR
		"system process information\n"
		"process CPU usage\n"
		"system process name\n")
{
	os_task_cli_hook_set(vty_out);
	if (argc == 1)
		os_task_show(vty, argv[0], 0);
	else
		os_task_show(vty, NULL, 0);
	return CMD_SUCCESS;
}


DEFUN (show_process_detail,
		show_process_detail_cmd,
		"show process cpu detail [NAME]",
		SHOW_STR
		"system process information\n"
		"process CPU usage\n"
		"system process name\n")
{
	os_task_cli_hook_set(vty_out);
	if (argc == 1)
		os_task_show(vty, argv[0], 1);
	else
		os_task_show(vty, NULL, 1);
	return CMD_SUCCESS;
}

#ifdef ZPL_IPCOM_STACK_MODULE
DEFUN (show_ipcom_process,
		show_ipcom_process_cmd,
		"show ipcom process",
		SHOW_STR
		"ipcom process information\n"
		"process CPU usage\n")
{
	os_task_cli_hook_set(vty_out);
	ipcom_process_show(vty_out, vty);
	return CMD_SUCCESS;
}
#endif

int cmd_os_init(void)
{
	install_element(ENABLE_NODE,  CMD_VIEW_LEVEL,  &show_process_cmd);
	install_element(ENABLE_NODE,  CMD_VIEW_LEVEL,  &show_process_detail_cmd);
#ifdef ZPL_IPCOM_STACK_MODULE
	install_element(ENABLE_NODE,  CMD_VIEW_LEVEL,  &show_ipcom_process_cmd);
#endif
	return 0;
}
