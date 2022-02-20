/*
 * zpl_media_task.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_VIDEO_TASK_H__
#define __ZPL_VIDEO_TASK_H__

#ifdef __cplusplus
extern "C" {
#endif


typedef struct 
{
	zpl_uint32		t_taskid;
	zpl_uint32		t_ready;
	zpl_void*		t_master;
}zpl_media_task_t;

extern zpl_media_task_t tvideo_task;


int zpl_media_task_create(ZPL_MEDIA_NODE_E module, zpl_media_task_t *t_task);
int zpl_media_task_destroy (zpl_media_task_t *t_task);
int zpl_media_task_ready(ZPL_MEDIA_NODE_E module);


#ifdef __cplusplus
}
#endif
#endif /* __ZPL_VIDEO_TASK_H__ */
