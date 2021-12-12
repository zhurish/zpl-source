/*
 * zpl_media_event.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */


#include "zpl_media.h"
#include "zpl_media_event.h"
#include "zpl_media_internal.h"

static zpl_media_event_queue_t *_media_event_queue_default = NULL;

static int zpl_media_event_task(void *p);


static int zpl_media_eventcb_free(zpl_media_event_t *data)
{
    //zpl_video_assert(data);
	if(data)
	{		
        free(data);
	}	
	return OK;
}

zpl_media_event_queue_t *zpl_media_event_default()
{
    return _media_event_queue_default;
}

zpl_media_event_queue_t *zpl_media_event_create(const char *name, zpl_uint32 maxsize)
{
    zpl_media_event_queue_t *queue = malloc(sizeof(zpl_media_event_queue_t));
	if(queue)
	{
        memset(queue, 0, sizeof(zpl_media_event_queue_t));
        queue->mutex = os_mutex_init();
        queue->sem = os_sem_init();
        queue->maxsize = maxsize;
        if(name)
            queue->name = strdup(name);
        lstInitFree (&queue->list, zpl_media_eventcb_free);
        if(_media_event_queue_default == NULL)
            _media_event_queue_default = queue;
		return queue;
	}
	return NULL;
}

int zpl_media_event_destroy(zpl_media_event_queue_t *queue)
{
	if(!queue)
		return ERROR;
	if(queue->taskid)
	{
		if(os_task_destroy(queue->taskid)==OK)
			queue->taskid = 0;
	}
    if(queue->mutex)
    	os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
    if(queue->name)
        free(queue->name);
	lstFree(&queue->list);
    if(queue->mutex)
    	os_mutex_exit(queue->mutex);
    if(queue->sem)
    	os_sem_exit(queue->sem);
    free(queue);
	return OK;
}

int zpl_media_event_start(zpl_media_event_queue_t *queue)
{
    if(queue && queue->taskid == 0)
        ;//queue->taskid = os_task_create(queue->name, OS_TASK_DEFAULT_PRIORITY,
	    //           0, zpl_media_event_task, queue, OS_TASK_DEFAULT_STACK);
    return OK;               
}

int zpl_media_event_push(zpl_media_event_queue_t *queue, zpl_media_event_t *data)
{
    zpl_video_assert(queue);
    zpl_video_assert(data);
    if(queue->mutex)
    	os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	lstAdd (&queue->list, data);
    data->state |= ZPL_MEDIA_EVENT_STATE_ACTIVE;
    if(queue->mutex)
    	os_mutex_unlock(queue->mutex);
    if(queue->sem)
        os_sem_give(queue->sem);
	return OK;
}


zpl_uint32 zpl_media_event_register_entry(zpl_media_event_queue_t *queue, zpl_uint32 module, zpl_uint32 event, 
        eventcb_handler evcb, zpl_void *p, zpl_uint32 flag)
{
    zpl_media_event_t *data = NULL;
    if(queue->mutex)
    	os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
    if(lstCount (&queue->ulist))    
	    data = lstFirst (&queue->ulist);
    if(queue->mutex)
    	os_mutex_unlock(queue->mutex);
    if(data == NULL)    
        data = malloc(sizeof(zpl_media_event_t));

    if(data)
    {
        memset(data, 0, sizeof(zpl_media_event_t));
        data->module = module;             //模块
        data->ev_type = event;            //
        data->ev_flags = flag;
        data->evcb_handler = evcb;
        data->event_data = p;       //buffer
        data->evid = (zpl_uint32)data;
        zpl_media_event_push(queue, data);
        return (zpl_uint32)data->evid;
    }
    return 0;
}

int zpl_media_event_unregister_entry(zpl_media_event_queue_t *queue, zpl_uint32 id)
{
	NODE node;
    zpl_media_event_t *eventcb = NULL;
    zpl_video_assert(queue);
    if(queue->mutex)
    	os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	
    for(eventcb = (zpl_media_event_t *)lstFirst(&queue->list); eventcb != NULL;
        eventcb = (zpl_media_event_t *)lstNext(&node))
	{
        node = eventcb->node;
        if(eventcb && id == (zpl_uint32)eventcb->evid) 
		{
            lstDelete(&queue->list, eventcb);
            eventcb->state = ZPL_MEDIA_EVENT_STATE_NONE;
            lstAdd (&queue->ulist, eventcb);  
            break;
		}
	}
    if(queue->mutex)
    	os_mutex_unlock(queue->mutex);
	return OK;    
}

int zpl_media_event_distribute(zpl_media_event_queue_t *queue)
{
	NODE node;
    zpl_media_event_t *eventcb = NULL;
    zpl_video_assert(queue);
    if(queue->mutex)
    	os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	
    for(eventcb = (zpl_media_event_t *)lstFirst(&queue->list); eventcb != NULL;
        eventcb = (zpl_media_event_t *)lstNext(&node))
	{
        node = eventcb->node;
        if(eventcb && eventcb->module && (eventcb->state & ZPL_MEDIA_EVENT_STATE_ACTIVE))
		{
            if(eventcb->evcb_handler)
                (eventcb->evcb_handler)(eventcb);  
            if(eventcb->ev_flags & ZPL_MEDIA_EVENT_FLAG_ONCE)  
            {
                eventcb->state |= ZPL_MEDIA_EVENT_STATE_INACTIVE;
                eventcb->state &= ~ZPL_MEDIA_EVENT_STATE_ACTIVE;
            }  
		}
	}
	
    for(eventcb = (zpl_media_event_t *)lstFirst(&queue->list); eventcb != NULL;
        eventcb = (zpl_media_event_t *)lstNext(&node))
	{
        node = eventcb->node;
        if(eventcb && eventcb->module && (eventcb->state & ZPL_MEDIA_EVENT_STATE_INACTIVE)) 
		{
            lstDelete(&queue->list, eventcb);
            eventcb->state = ZPL_MEDIA_EVENT_STATE_NONE;
            lstAdd (&queue->ulist, eventcb);  
		}
	}
    if(queue->mutex)
    	os_mutex_unlock(queue->mutex);
	return OK;
}

int zpl_media_event_dispatch(zpl_media_event_queue_t *queue)
{
    return zpl_media_event_distribute(queue);
}

int zpl_media_event_scheduler(zpl_media_event_queue_t *queue)
{
    return zpl_media_event_distribute(queue);
}

static int zpl_media_event_task(void *p)
{
    zpl_media_event_queue_t *queue = p;
	host_config_load_waitting();
	while(queue)
	{
        if(queue->sem)
		    os_sem_take(queue->sem, OS_WAIT_FOREVER);
        zpl_media_event_dispatch(queue);
	}
	return OK;
}