/*
 * voip_task.c
 *
 *  Created on: 2018年12月27日
 *      Author: DELL
 */


#include "zebra.h"
#include "memory.h"
#include "log.h"
#include "memory.h"
#include "str.h"
#include "linklist.h"
#include "prefix.h"
#include "table.h"
#include "vector.h"

#include "voip_def.h"
#include "voip_task.h"
#include "voip_event.h"
#include "voip_stream.h"
#include "voip_volume.h"


voip_task_t	voip_task;




static int voip_main_task(voip_task_t *task)
{
	//int n = 0;
	while(!os_load_config_done())
	{
		os_sleep(1);
	}

	while(!task->enable)
	{
		os_sleep(1);
	}
	while(task->enable)
	{
#if 0
		if(task->active)
		{
			if(access("/app/etc/volume_setup.sh", F_OK) == 0)
			{
				super_system("cd /app/etc/; chmod +x volume_setup.sh");
				super_system("cd /app/etc/;./volume_setup.sh");
			}
			if(task->stream) // voip stream
			{
#ifdef PL_VOIP_MEDIASTREAM_TEST
				setup_media_streams(task->pVoid);
#else
				voip_stream_setup_api(task->pVoid);
#endif
				voip_stream_running_api(task->pVoid);
				if(task->pVoid)
				{
#ifdef PL_VOIP_MEDIASTREAM_TEST
					clear_mediastreams(task->pVoid);
#else
					voip_stream_clear_api(task->pVoid);
#endif
				}
			}
			else	//ring
			{
				if(voip_call_ring_active_api())
				{
					voip_call_ring_running(task->pVoid);
				}
			}
		}
#else
		if(task->active)
		{
			if(task->stream) // voip stream
			{
				if(access("/app/etc/volume_setup.sh", F_OK) == 0)
				{
					super_system("cd /app/etc/; chmod +x volume_setup.sh");
					super_system("cd /app/etc/;./volume_setup.sh");
				}
				//voip_volume_open_api(VOIP_VOLUME_ALL);
#ifdef PL_VOIP_MEDIASTREAM_TEST
				setup_media_streams(task->pVoid);
#else
				voip_stream_setup_api(task->pVoid);
#endif
				voip_stream_running_api(task->pVoid);
				if(task->pVoid)
				{
#ifdef PL_VOIP_MEDIASTREAM_TEST
					clear_mediastreams(task->pVoid);
#else
					voip_stream_clear_api(task->pVoid);
#endif
				}
				voip_volume_close_api(VOIP_VOLUME_ALL);
			}
			else	//ring
			{
				if(voip_call_ring_active_api())
				{
					voip_volume_open_api(VOIP_VOLUME_PLAYBACK);
					voip_call_ring_running(task->pVoid);
					voip_volume_close_api(VOIP_VOLUME_PLAYBACK);
				}
			}
		}
#endif
		else
		{
			os_sleep(1);
		}
	}
	if(task->pVoid)
	{
#ifdef PL_VOIP_MEDIASTREAM_TEST
		clear_mediastreams(task->pVoid);
#else
		voip_stream_clear_api(task->pVoid);
#endif
		voip_stream_module_exit();
		free(task->pVoid);
		task->pVoid = NULL;
	}
	return OK;
}


int voip_task_module_init()
{
	if(voip_task.taskid)
		return OK;
	voip_task.taskid = os_task_create("voipTask", OS_TASK_DEFAULT_PRIORITY,
	               0, voip_main_task, &voip_task, OS_TASK_DEFAULT_STACK);
	if(voip_task.taskid)
		return OK;
	return ERROR;
}


int voip_task_module_exit()
{
	if(voip_task.taskid)
	{
		if(os_task_destroy(voip_task.taskid)==OK)
			voip_task.taskid = 0;
	}
	return OK;
}
