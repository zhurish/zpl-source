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
#include "voip_app.h"



static voip_task_t	*voip_task = NULL;




static int voip_main_task(voip_task_t *task)
{
	zassert(task != NULL);
	//int n = 0;
	while(!os_load_config_done())
	{
		os_sleep(1);
	}
	task->enable = TRUE;
	//os_task_priority_set(task->taskid, 2);
	while(!task->enable)
	{
		os_sleep(1);
	}
	//VOIP_TASK_DEBUG( "===================%s:1", __func__);
	while(task->enable)
	{
		if(task->sem)
			os_sem_take(task->sem, OS_WAIT_FOREVER);
		//VOIP_TASK_DEBUG( "===================%s:", __func__);
		if(task->active)
		{
			if(task->stream)
			{
				voip_volume_open_api(VOIP_VOLUME_ALL);
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
					if(!task->stream)
						voip_volume_close_api(VOIP_VOLUME_PLAYBACK);
				}
			}
		}
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
	zassert(voip_app != NULL);
	voip_task = voip_app->voip_task;
	zassert(voip_task != NULL);
	voip_task->enable = TRUE;
	if(voip_task->sem == NULL)
		voip_task->sem = os_sem_init();
	if(voip_task->taskid)
		return OK;

/*
	voip_task->taskid = os_task_create("voipTask", OS_TASK_DEFAULT_PRIORITY,
			OS_TASK_TIME_SLICED, voip_main_task, voip_task, OS_TASK_DEFAULT_STACK);
*/
	voip_task->taskid = os_task_create("voipTask", OS_TASK_DEFAULT_PRIORITY,
			0, voip_main_task, voip_task, OS_TASK_DEFAULT_STACK);

	if(voip_task->taskid)
		return OK;
	return ERROR;
}


int voip_task_module_exit()
{
	zassert(voip_task != NULL);
	voip_task->enable = FALSE;
	if(voip_task->sem)
	{
		if(os_sem_exit(voip_task->sem)==OK)
			voip_task->sem = NULL;
	}
	if(voip_task->taskid)
	{
		if(os_task_destroy(voip_task->taskid)==OK)
			voip_task->taskid = 0;
	}
	return OK;
}
