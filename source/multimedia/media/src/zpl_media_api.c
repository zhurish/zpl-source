/*
 * zpl_media_api.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "lib_include.h"
#include "zpl_media.h"
#include "zpl_media_api.h"
#include "zpl_media_internal.h"
#include "zpl_media_event.h"



struct module_list module_list_zplmedia = {
		.module = MODULE_ZPLMEDIA,
		.name = "ZPLMEDIA\0",
		.module_init = zpl_media_module_init,
		.module_exit = zpl_media_module_exit,
		.module_task_init = zpl_media_task_init,
		.module_task_exit = zpl_media_task_exit,
		.module_cmd_init = zpl_media_cmd_init,
		.taskid = 0,
};


int zpl_media_module_init(void)
{
	zpl_media_global_init();
	zpl_media_debugmsg_init();
	zpl_media_hwres_load("./media.json");
	zpl_media_system_init();
	zpl_media_event_create("mediaEvent", 16);
	#ifdef ZPL_MEDIA_QUEUE_DISTPATH
	zpl_media_bufqueue_init();
	#endif
	zpl_media_channel_init();
	zpl_media_video_inputchn_init();
	zpl_media_video_vpsschn_init();
	zpl_media_video_encode_init();
	zpl_media_proxy_init();
	return OK;
}

int zpl_media_module_exit(void)
{
	zpl_media_channel_exit();
	zpl_media_proxy_exit();
	#ifdef ZPL_MEDIA_QUEUE_DISTPATH
	zpl_media_bufqueue_exit();
	#endif
	zpl_media_event_destroy(zpl_media_event_default());
	zpl_media_global_exit();
	return OK;
}

int zpl_media_task_init(void)
{
	zpl_media_task_create(MODULE_ZPLMEDIA, &tvideo_task);

	zpl_media_event_start(zpl_media_event_default());

	#ifdef ZPL_MEDIA_QUEUE_DISTPATH
	zpl_media_bufqueue_start();
	#endif
	return OK;
}

int zpl_media_task_exit(void)
{
	zpl_media_task_destroy( &tvideo_task);

	return OK;
}

int zpl_media_cmd_init(void)
{
	cmd_video_init();
	return OK;
}
