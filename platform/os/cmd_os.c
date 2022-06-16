/*
 * cmd_nvram_env.c
 *
 *  Created on: 2019年3月5日
 *      Author: DELL
 */



#include "auto_include.h"
#include <zplos_include.h>
#include "module.h"
#include "vty.h"
#include "command.h"

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

#ifdef ZPL_IPCOM_MODULE
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


#ifdef OS_ANSYNC_GLOBAL_LIST

static int os_ansync_show_func(os_ansync_lst *lst, void *pVoid)
{
	return os_ansync_show(lst, vty_out, pVoid);
}
DEFUN (show_ansyc_thread,
		show_ansyc_thread_cmd,
		"show ansync thread dump",
		SHOW_STR
		"ansync thread information\n"
		"thread information\n"
		"dump information\n")
{
	os_ansync_global_foreach(os_ansync_show_func, vty);
	return CMD_SUCCESS;
}
#endif

int cmd_os_init(void)
{
	install_element(ENABLE_NODE,  CMD_VIEW_LEVEL,  &show_process_cmd);
	install_element(ENABLE_NODE,  CMD_VIEW_LEVEL,  &show_process_detail_cmd);
#ifdef ZPL_IPCOM_MODULE
	install_element(ENABLE_NODE,  CMD_VIEW_LEVEL,  &show_ipcom_process_cmd);
#endif
#ifdef OS_ANSYNC_GLOBAL_LIST
	install_element(ENABLE_NODE,  CMD_VIEW_LEVEL,  &show_ansyc_thread_cmd);
#endif
	return 0;
}
