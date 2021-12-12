/*
 * zpl_media_task.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "os_include.h"
#include "zpl_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"

#ifdef ZPL_MEDIA_PROCESS_ONE
static int media_main_task(void *argv);
zpl_media_task_t tvideo_task;
#else
static int zpl_media_input_task(void *argv);
static int zpl_media_process_task(void *argv);
static int zpl_media_encode_task(void *argv);

zpl_media_task_t tvideo_task[3];
#endif

int zpl_media_task_ready( ZPL_MEDIA_NODE_E module)
{
#ifdef ZPL_MEDIA_PROCESS_ONE
	if(tvideo_task.t_taskid)
		tvideo_task.t_ready = 1;
#else
    switch(module)
    {
    case ZPL_MEDIA_NODE_INPUT:
	if(tvideo_task[0].t_taskid)
		tvideo_task[0].t_ready = 1;
        break;    
    case ZPL_MEDIA_NODE_PROCESS:
	if(tvideo_task[1].t_taskid)
		tvideo_task[1].t_ready = 1;
        break;    
    case ZPL_MEDIA_NODE_ENCODE:
	if(tvideo_task[2].t_taskid)
		tvideo_task[2].t_ready = 1;
        break; 
    default:
        break; 
    }
#endif	
	return OK;
}

int zpl_media_task_create( ZPL_MEDIA_NODE_E module, zpl_media_task_t *t_task)
{
	zpl_video_assert(t_task);
	
	if(t_task->t_taskid == 0)
    {
#ifdef ZPL_MEDIA_PROCESS_ONE
		t_task->t_master = thread_master_module_create(MODULE_ZPLMEDIA);
		t_task->t_taskid = os_task_create("vidProcessTask", OS_TASK_DEFAULT_PRIORITY,
								 0, media_main_task, t_task, OS_TASK_DEFAULT_STACK);
#else
        switch(module)
        {
        case ZPL_MEDIA_NODE_INPUT:
			t_task->t_master = thread_master_module_create(ZPL_MEDIA_NODE_INPUT);
		    t_task->t_taskid = os_task_create("vidInputTask", OS_TASK_DEFAULT_PRIORITY,
								 0, zpl_media_input_task, t_task, OS_TASK_DEFAULT_STACK);
			if (t_task->t_taskid)
				submodule_setup(MODULE_ZPLMEDIA, ZPL_MEDIA_NODE_ENCODE, "MEDIAEINPUT", t_task->t_taskid);
            break;    
        case ZPL_MEDIA_NODE_PROCESS:
			t_task->t_master = thread_master_module_create(ZPL_MEDIA_NODE_PROCESS);
		    t_task->t_taskid = os_task_create("vidProcessTask", OS_TASK_DEFAULT_PRIORITY,
								 0, zpl_media_process_task, t_task, OS_TASK_DEFAULT_STACK);
			if (t_task->t_taskid)
				submodule_setup(MODULE_ZPLMEDIA, ZPL_MEDIA_NODE_ENCODE, "MEDIAEPROCESS", t_task->t_taskid);
            break;    
        case ZPL_MEDIA_NODE_ENCODE:
			t_task->t_master = thread_master_module_create(ZPL_MEDIA_NODE_ENCODE);
		    t_task->t_taskid = os_task_create("vidEncodeTask", OS_TASK_DEFAULT_PRIORITY,
								 0, zpl_media_encode_task, t_task, OS_TASK_DEFAULT_STACK);
			if (t_task->t_taskid)
				submodule_setup(MODULE_ZPLMEDIA, ZPL_MEDIA_NODE_ENCODE, "MEDIAENCODE", t_task->t_taskid);
            break; 
        default:
            break; 
        }
#endif
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

#ifdef ZPL_MEDIA_PROCESS_ONE
static int media_main_task(void *argv)
{
    zpl_media_task_t *video_task = argv;    
	struct thread thread;
	zpl_video_assert(video_task);
	struct thread_master *master = (struct thread_master *)video_task->t_master;
	if(master == NULL)
	{
		video_task->t_master = thread_master_module_create(MODULE_ZPLMEDIA);
	}
	host_config_load_waitting();
	while(!video_task->t_ready)
	{
		os_sleep(1);
	}
	//master->debug = 1;
	//while (thread_fetch_call((struct thread_master *) master));
	while (thread_fetch((struct thread_master *) master, &thread))
		thread_call(&thread);
	return OK;
}
#else
static int zpl_media_input_task(void *argv)
{
    zpl_media_task_t *video_task = argv;    
	struct thread thread;
	zpl_video_assert(video_task);
	struct thread_master *master = (struct thread_master *)video_task->t_master;
	if(master == NULL)
	{
		video_task->t_master = thread_master_module_create(ZPL_MEDIA_NODE_INPUT);
	}
	
	host_config_load_waitting();
	while(!video_task->t_ready)
	{
		os_sleep(1);
	}
	//master->debug = 1;
	//while (thread_fetch_call((struct thread_master *) master));
	while (thread_fetch((struct thread_master *) master, &thread))
		thread_call(&thread);
	return OK;
}

static int zpl_media_process_task(void *argv)
{
    zpl_media_task_t *video_task = argv;    
	struct thread thread;
	zpl_video_assert(video_task);
	struct thread_master *master = (struct thread_master *)video_task->t_master;
	if(master == NULL)
	{
		video_task->t_master = thread_master_module_create(ZPL_MEDIA_NODE_PROCESS);
	}
	host_config_load_waitting();
	while(!video_task->t_ready)
	{
		os_sleep(1);
	}
	//master->debug = 1;
	//while (thread_fetch_call((struct thread_master *) master));
	while (thread_fetch((struct thread_master *) master, &thread))
		thread_call(&thread);
	return OK;
}

static int zpl_media_encode_task(void *argv)
{
    zpl_media_task_t *video_task = argv;    
	struct thread thread;
	zpl_video_assert(video_task);
	struct thread_master *master = (struct thread_master *)video_task->t_master;
	if(master == NULL)
	{
		video_task->t_master = thread_master_module_create(ZPL_MEDIA_NODE_ENCODE);
	}
	host_config_load_waitting();
	while(!video_task->t_ready)
	{
		os_sleep(1);
	}
	//master->debug = 1;
	//while (thread_fetch_call((struct thread_master *) master));
	while (thread_fetch((struct thread_master *) master, &thread))
		thread_call(&thread);
	return OK;
}
#endif

