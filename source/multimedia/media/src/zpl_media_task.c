/*
 * zpl_media_task.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "zpl_media.h"
#include "zpl_media_internal.h"

static int media_main_task(void *argv);
zpl_media_task_t tvideo_task;


int zpl_media_task_ready( ZPL_MEDIA_GLOBAL_E module)
{
	if(tvideo_task.t_taskid)
		tvideo_task.t_ready = 1;	
	return OK;
}

int zpl_media_task_create( ZPL_MEDIA_GLOBAL_E module, zpl_media_task_t *t_task)
{
	zpl_video_assert(t_task);
	
	if(t_task->t_taskid == 0)
    {
		t_task->t_master = thread_master_module_create(MODULE_MEDIA);
		t_task->t_taskid = os_task_create("mediaProcessTask", OS_TASK_DEFAULT_PRIORITY,
								 0, media_main_task, t_task, OS_TASK_DEFAULT_STACK);
    }
	
	if (t_task->t_taskid)
	{
		return OK;
	}
	return ERROR;
}

int zpl_media_task_destroy (zpl_media_task_t *t_task)
{
	zpl_video_assert(t_task);
	if(t_task->t_taskid)
		os_task_destroy(t_task->t_taskid);
	t_task->t_taskid = 0;
	return OK;
}


/*static int media_main_timer(struct thread *thread)
{
	zm_msg_debug(" media timer thread");
	if(tvideo_task.t_master)
		thread_add_timer(tvideo_task.t_master, media_main_timer, NULL, 5);
	return 0;	
}
*/
static int media_main_task(void *argv)
{
    zpl_media_task_t *video_task = argv;    
	zpl_video_assert(video_task);
	struct thread_master *master = (struct thread_master *)video_task->t_master;
	if(master == NULL)
	{
		video_task->t_master = thread_master_module_create(MODULE_MEDIA);
	}
	//thread_add_timer(video_task->t_master, media_main_timer, NULL, 10);
	host_waitting_loadconfig();
	while(!video_task->t_ready)
	{
		os_sleep(1);
	}
	thread_mainloop(master);
	return OK;
}


