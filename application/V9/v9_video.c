/*
 * v9_video.c
 *
 *  Created on: 2019年11月28日
 *      Author: DELL
 */

#include "zebra.h"
#include "network.h"
#include "vty.h"
#include "if.h"
#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "eloop.h"

#include "application.h"


int v9_app_module_init()
{
	if(master_eloop[MODULE_APP_START] == NULL)
		master_eloop[MODULE_APP_START] = eloop_master_module_create(MODULE_APP_START);

	//v9_board_init();

	v9_video_board_init();

	v9_video_sdk_init();

	v9_video_user_load();

	v9_serial_init(V9_SERIAL_CTL_NAME, 115200);

	return OK;
}

int v9_app_module_exit()
{
/*	v9_board_init();

	v9_video_board_init();
	*/
	v9_serial_exit();
	return OK;
}


int v9_app_module_task_init(void)
{
	v9_video_sdk_task_init ();
	return v9_serial_task_init();
}
int v9_app_module_task_exit(void)
{
	return v9_serial_task_exit();
}
