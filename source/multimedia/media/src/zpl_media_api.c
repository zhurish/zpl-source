/*
 * zpl_media_api.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "zpl_media.h"
#include "zpl_media_api.h"
#include "zpl_media_internal.h"

static int zpl_media_module_init(void);
static int zpl_media_module_exit(void);
static int zpl_media_module_task_init(void);
static int zpl_media_module_task_exit(void);

struct module_list module_list_zplmedia = {
		.module = MODULE_MEDIA,
		.name = "MEDIA\0",
		.module_init = zpl_media_module_init,
		.module_exit = zpl_media_module_exit,
		.module_task_init = zpl_media_module_task_init,
		.module_task_exit = zpl_media_module_task_exit,
		.module_cmd_init = zpl_media_cmd_init,
		.taskid = 0,
};


static int zpl_media_module_init(void)
{
	zpl_media_global_init();
	zpl_media_debugmsg_init();
	zpl_media_hwres_load(ZPL_MEDIA_HALRES_PATH);
	zpl_media_system_init();

	zpl_media_channel_init();
	zpl_media_audio_init();
	zpl_media_video_inputchn_init();
	zpl_media_video_vpsschn_init();
	zpl_media_video_encode_init();
	zpl_media_proxy_init();
	zpl_mediartp_scheduler_init();
	return OK;
}

static int zpl_media_module_exit(void)
{
	zpl_mediartp_scheduler_exit();
	zpl_media_channel_exit();
	zpl_media_proxy_exit();
	zpl_media_global_exit();
	return OK;
}

int zpl_media_global_ready(int channel, zpl_bool enable)
{
	if(_media_global.mthreadpool[channel].t_taskid)
	{
		_media_global.mthreadpool[channel].t_ready = enable;	
		if(enable == zpl_false)
		{
			if(_media_global.mthreadpool[channel].t_master)
				thread_fetch_quit(_media_global.mthreadpool[channel].t_master);
		}
	}
	return OK;
}

static int media_process_task(void *argv)
{
	zpl_mthread_pool_t *mthreadpool = (zpl_mthread_pool_t *)argv;
	if(mthreadpool)
	{
		host_waitting_loadconfig();
		while(!mthreadpool->t_ready)
		{
			os_sleep(1);
		}
		thread_mainloop(mthreadpool->t_master);
	}
	return OK;
}

static int zpl_media_module_task_init(void)
{
	int i = 0;
	for(i = 0; i < ZPL_MEDIA_HWCHANNEL_MAX; i++)
	{
		if(_media_global.mthreadpool[i].t_taskid == 0)
		{
			_media_global.mthreadpool[i].t_master = thread_master_name_create(os_name_format("mediaProcess%d", i));
			_media_global.mthreadpool[i].t_taskid = os_task_create(os_name_format("mediaProcess%d", i), 50,
									0, media_process_task, &_media_global.mthreadpool[i], OS_TASK_DEFAULT_STACK*2);
		}
	}
	if (_media_global.mthreadpool[0].t_taskid)
	{
		zpl_mediartp_scheduler_start();
		return OK;
	}
	return ERROR;
}

static int zpl_media_module_task_exit(void)
{
	int i = 0;
	zpl_mediartp_scheduler_stop();
	for(i = 0; i < ZPL_MEDIA_HWCHANNEL_MAX; i++)
	{
		if(_media_global.mthreadpool[i].t_taskid)
			os_task_destroy(_media_global.mthreadpool[i].t_taskid);
		_media_global.mthreadpool[i].t_taskid = 0;
	}
	return OK;
}


