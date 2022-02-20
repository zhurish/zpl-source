/*
 * zpl_media_api.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "os_include.h"
#include "zpl_include.h"
#include "lib_include.h"
#include "zpl_media.h"
#include "zpl_media_api.h"
#include "zpl_media_internal.h"
#include "zpl_media_event.h"



struct module_list module_list_zplmedia = {
		.module = MODULE_ZPLMEDIA,
		.name = "ZPLMEDIA",
		.module_init = zpl_media_module_init,
		.module_exit = zpl_media_module_exit,
		.module_task_init = zpl_media_task_init,
		.module_task_exit = zpl_media_task_exit,
		.module_cmd_init = zpl_media_cmd_init,
		.module_write_config = NULL,
		.module_show_config = NULL,
		.module_show_debug = NULL,
		.taskid = 0,
};



int zpl_media_module_init()
{
	zpl_media_debugmsg_init();
	printf( "=======zpl_media_module_init :zpl_video_resources_show\r\n");
	zpl_video_resources_show(NULL);
	printf( "=======zpl_media_module_init\r\n");
	//zpl_media_client_init(16);
	zpl_media_event_create("mediaEvent", 16);
	zpl_media_channel_init();
	zpl_video_input_init();
	zpl_video_vpss_init();
	zpl_video_encode_init();
	zpl_media_proxy_init();
	return OK;
}

int zpl_media_module_exit()
{
	printf( "=======zpl_media_module_exit\r\n");
	//zpl_media_client_exit();
	zpl_media_channel_exit();
	zpl_video_input_exit();
	zpl_video_vpss_exit();
	zpl_video_encode_exit();
	zpl_media_proxy_exit();
	return OK;
}

int zpl_media_task_init()
{
	printf( "=======zpl_media_task_init\r\n");

	zpl_media_task_create(MODULE_ZPLMEDIA, &tvideo_task);

	zpl_media_event_start(zpl_media_event_default());

	zpl_media_config_load("./media.json");
	zpl_media_channel_load_default();

	return OK;
}

int zpl_media_task_exit()
{
	printf( "=======zpl_media_task_exit\r\n");

	zpl_media_task_destroy( &tvideo_task);

	return OK;
}

int zpl_media_cmd_init()
{
	printf("=======zpl_media_cmd_init\r\n");
	cmd_video_init();
	return OK;
}
