/*
 * voip_event.c
 *
 *  Created on: 2018年12月28日
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
#include "eloop.h"
#include "network.h"
#include "os_util.h"
#include "os_socket.h"

#include "voip_def.h"
#include "voip_task.h"
#include "voip_event.h"
#include "voip_app.h"
#include "voip_stream.h"

voip_event_t voip_event;



int voip_event_module_init(void)
{
	memset(&voip_event, 0, sizeof(voip_event));
	lstInit(&voip_event.event_lst);			//
	lstInit(&voip_event.event_unlst);
	voip_event.mutex = os_mutex_init();
	voip_event.sem = os_sem_init();
	return OK;
}

int voip_event_module_exit(void)
{
	if(voip_event.mutex)
	{
		if(os_mutex_exit(voip_event.mutex)==OK)
			voip_event.mutex = NULL;
	}
	if(voip_event.sem)
	{
		if(os_sem_exit(voip_event.sem)==OK)
			voip_event.sem = NULL;
	}
	lstFree(&voip_event.event_lst);
	lstFree(&voip_event.event_unlst);
	memset(&voip_event, 0, sizeof(voip_event));
	return OK;
}


/*
 * VOIP Event List
 */
static event_node_t * voip_event_node_get_empty(voip_event_t *event)
{
	event_node_t *node = NULL;
	if (event->mutex)
		os_mutex_lock(event->mutex, OS_WAIT_FOREVER);
	node = lstFirst(&event->event_unlst);
	if(!node)
	{
		node = malloc(sizeof(event_node_t));
		if(node)
			memset(node, 0, sizeof(event_node_t));
	}
	else
		lstDelete(&event->event_unlst, (NODE *) node);
	if(event->mutex)
		os_mutex_unlock(event->mutex);
	return node;
}

static int _voip_event_node_add(voip_event_t *event, event_node_t *node)
{
	if (event->mutex)
		os_mutex_lock(event->mutex, OS_WAIT_FOREVER);
	lstAdd (&event->event_lst, (NODE *)node);
	//V_APP_DEBUG("-------------%s", __func__);
	if(event->mutex)
		os_mutex_unlock(event->mutex);
	return OK;
}


static int _voip_event_node_del(voip_event_t *event, event_node_t *node)
{
	if (event->mutex)
		os_mutex_lock(event->mutex, OS_WAIT_FOREVER);
	lstDelete (&event->event_lst, (NODE *)node);
	lstAdd (&event->event_unlst, (NODE *)node);
	//V_APP_DEBUG("-------------%s", __func__);
	if(event->mutex)
		os_mutex_unlock(event->mutex);
	return OK;
}

int voip_event_node_add(event_node_t *node)
{
	event_node_t * pnode = voip_event_node_get_empty(&voip_event);
	if(node && pnode)
	{
		memcpy(pnode, node, sizeof(event_node_t));
		_voip_event_node_add(&voip_event, pnode);
		os_sem_give(voip_event.sem);
		return OK;
	}
	return ERROR;
}

int _voip_event_node_register(int (*cb)(event_node_t *), void *pVoid, char *buf, int len, char *funcname)
{
	event_node_t * pnode = voip_event_node_get_empty(&voip_event);
	if(cb && pnode)
	{
		//memcpy(pnode, node, sizeof(event_node_t));
		pnode->ev_cb = cb;
		pnode->pVoid = pVoid;
		if(funcname)
			strncpy(pnode->entry_name, funcname, MIN(strlen(funcname), 128));
		if(buf)
			memcpy(pnode->data, buf, len);
		//V_APP_DEBUG("-------------%s", __func__);
		_voip_event_node_add(&voip_event, pnode);
		os_sem_give(voip_event.sem);
		return OK;
	}
	return ERROR;
}

int voip_event_node_del(event_node_t *node)
{
	return _voip_event_node_del(&voip_event, node);
}

int voip_event_node_unregister(int (*cb)(event_node_t *), void *pVoid)
{
	NODE node;
	event_node_t *lookup;
	if (voip_event.mutex)
		os_mutex_lock(voip_event.mutex, OS_WAIT_FOREVER);
	LIST *lst = &voip_event.event_lst;
	for (lookup = (event_node_t *) lstFirst(lst);
			lookup != NULL; lookup = (event_node_t *) lstNext(&node))
	{
		node = lookup->node;
		if (lookup && lookup->ev_cb == cb && lookup->pVoid == pVoid)
		{
			lstDelete (&voip_event.event_lst, (NODE *)lookup);
			lstAdd (&voip_event.event_unlst, (NODE *)lookup);
			//V_APP_DEBUG("-------------%s", __func__);
			break;
		}
	}
	if(voip_event.mutex)
		os_mutex_unlock(voip_event.mutex);
	return OK;
}

static int _voip_event_node_process(voip_event_t *event, event_node_t *pnode)
{
	NODE node;
	event_node_t *lookup;
	if (event->mutex)
		os_mutex_lock(event->mutex, OS_WAIT_FOREVER);
	LIST *lst = &event->event_lst;
	for (lookup = (event_node_t *) lstFirst(lst);
			lookup != NULL; lookup = (event_node_t *) lstNext(&node))
	{
		node = lookup->node;
		if (lookup && lookup->ev_cb)
		{
			(lookup->ev_cb)(lookup);
			V_APP_DEBUG("-------------%s:%s", __func__, lookup->entry_name);
			lstDelete (&event->event_lst, (NODE *)lookup);
			lstAdd (&event->event_unlst, (NODE *)lookup);
		}
	}
	if(event->mutex)
		os_mutex_unlock(event->mutex);
	return OK;
}

/*
 * VOIP Event Task
 */
static int voip_event_task(voip_event_t *event)
{
	//int n = 0;
	while(!os_load_config_done())
	{
		os_sleep(1);
	}
	event->enable = TRUE;
	while(!event->enable)
	{
		os_sleep(1);
/*		n++;
		if(n == 8)
			voip_test();*/
	}
	while(event->enable)
	{
		//os_sem_give(voip_event.sem);
		if(event->sem)
			os_sem_take(event->sem, OS_WAIT_FOREVER);
		zlog_debug(ZLOG_VOIP, "-------------%s", __func__);
		if(lstCount(&event->event_lst))
			_voip_event_node_process(event, NULL);
	}
	return OK;
}



int voip_event_task_init()
{
	if(voip_event.taskid)
		return OK;

	voip_event.taskid = os_task_create("voipEvent", OS_TASK_DEFAULT_PRIORITY,
	               0, voip_event_task, &voip_event, OS_TASK_DEFAULT_STACK);
	if(voip_event.taskid)
		return OK;
	return ERROR;
}


int voip_event_task_exit()
{
	if(voip_event.taskid)
	{
		if(os_task_destroy(voip_event.taskid)==OK)
			voip_event.taskid = 0;
	}
	return OK;
}





