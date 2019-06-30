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
#include "vty.h"
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

static voip_event_ctx_t *voip_event = NULL;



int voip_event_module_init(void)
{
	if(voip_event == NULL)
	{
		voip_event = XMALLOC(MTYPE_VOIP_TOP, sizeof(voip_event_ctx_t));
	}
	if(!voip_event)
		return ERROR;
	memset(voip_event, 0, sizeof(voip_event_ctx_t));
	lstInit(&voip_event->execute_lst);
	lstInit(&voip_event->high_lst);			//
	lstInit(&voip_event->timer_lst);
	lstInit(&voip_event->ready_lst);
	lstInit(&voip_event->event_unlst);
	voip_event->mutex = os_mutex_init();
	voip_event->sem = os_sem_init();
	return OK;
}

int voip_event_module_exit(void)
{
	zassert(voip_event != NULL);
	if(voip_event->mutex)
	{
		if(os_mutex_exit(voip_event->mutex)==OK)
			voip_event->mutex = NULL;
	}
	if(voip_event->sem)
	{
		if(os_sem_exit(voip_event->sem)==OK)
			voip_event->sem = NULL;
	}
	lstFree(&voip_event->execute_lst);
	lstFree(&voip_event->high_lst);
	lstFree(&voip_event->timer_lst);
	lstFree(&voip_event->ready_lst);
	lstFree(&voip_event->event_unlst);
	memset(voip_event, 0, sizeof(voip_event_ctx_t));
	XFREE(MTYPE_VOIP_TOP, voip_event);
	voip_event = NULL;
	return OK;
}


/*
 * VOIP Event List
 */
static int _voip_event_list_lookup(voip_event_ctx_t *event, voip_event_type_t type, voip_event_t *lknode)
{
	NODE node;
	LIST *lst = NULL;
	voip_event_t *lookup = NULL;
	zassert(event != NULL);
	switch(type)
	{
	case VOIP_EVENT_EXECUTE:
		lst = &event->execute_lst;
		break;
	case VOIP_EVENT_HIGH:
		lst = &event->high_lst;
		break;
	case VOIP_EVENT_TIMER:
		lst = &event->timer_lst;
		break;
	case VOIP_EVENT_READY:
		lst = &event->ready_lst;
		break;
	case VOIP_EVENT_UNUSE:
/*		lst = &event->event_unlst;
		break;*/
	default:
		break;
	}
	if(lst)
	{
		for (lookup = (voip_event_t *) lstFirst(lst);
				lookup != NULL; lookup = (voip_event_t *) lstNext(&node))
		{
			node = lookup->node;
			if (lookup && lknode && lookup == lknode)
			{
				return ERROR;
			}
		}
	}
	return OK;
}

static voip_event_t * voip_event_list_get_empty(voip_event_ctx_t *event)
{
	voip_event_t *node = NULL;
	zassert(event != NULL);
	if(lstCount(&event->event_unlst) > 0)
	{
		node = lstFirst(&event->event_unlst);
		if(node == NULL)
		{
			node = XMALLOC(MTYPE_VOIP_EVENT, sizeof(voip_event_t));
			if(node == NULL)
				return NULL;
		}
		else
			lstDelete(&event->event_unlst, (NODE *) node);
	}
	else
	{
		node = XMALLOC(MTYPE_VOIP_EVENT, sizeof(voip_event_t));
		if(node == NULL)
			return NULL;
	}
	if(node)
		memset(node, 0, sizeof(voip_event_t));
	return node;
}


static int _voip_event_list_add(voip_event_ctx_t *event, voip_event_type_t type,
		voip_event_t *node)
{
	LIST *lst = NULL;
	zassert(event != NULL);
	zassert(node != NULL);
	switch(type)
	{
	case VOIP_EVENT_EXECUTE:
		lst = &event->execute_lst;
		break;
	case VOIP_EVENT_HIGH:
		lst = &event->high_lst;
		break;
	case VOIP_EVENT_TIMER:
		lst = &event->timer_lst;
		break;
	case VOIP_EVENT_READY:
		lst = &event->ready_lst;
		break;
	case VOIP_EVENT_UNUSE:
		lst = &event->event_unlst;
		break;
	default:
		break;
	}
	if(lst)
		lstAdd (lst, (NODE *)node);
	return OK;
}


static int _voip_event_list_del(voip_event_ctx_t *event, voip_event_type_t type,
		voip_event_t *node)
{
	LIST *lst = NULL;
	zassert(event != NULL);
	zassert(node != NULL);
	switch(type)
	{
	case VOIP_EVENT_EXECUTE:
		lst = &event->execute_lst;
		break;
	case VOIP_EVENT_READY:
		lst = &event->ready_lst;
		break;
	case VOIP_EVENT_TIMER:
		lst = &event->timer_lst;
		break;
	case VOIP_EVENT_HIGH:
		lst = &event->high_lst;
		break;
	case VOIP_EVENT_UNUSE:
		lst = &event->event_unlst;
		break;
	default:
		break;
	}
	if(lst)
	{
		lstDelete (lst, (NODE *)node);
		node->type = VOIP_EVENT_UNUSE;
		lstAdd (&event->event_unlst, (NODE *)node);
	}
	return OK;
}

static int _voip_event_list_call(voip_event_ctx_t *event, LIST *lst)
{
	int ret = 0, brk = 0;
	NODE node;
	voip_event_t *lookup = NULL;
	zassert(event != NULL);
	zassert(lst != NULL);
	for (lookup = (voip_event_t *) lstFirst(lst);
			lookup != NULL; lookup = (voip_event_t *) lstNext(&node))
	{
		node = lookup->node;
		if (lookup && lookup->ev_cb)
		{
			if(lookup->type == VOIP_EVENT_EXECUTE)
			{
				if(event->mutex)
					os_mutex_unlock(event->mutex);
				lstDelete (lst, (NODE *)lookup);
/*				if(event->mutex)
					os_mutex_unlock(event->mutex);*/
				ret = (lookup->ev_cb)(lookup);
				VOIP_EV_DEBUG( "============%s:EXECUTE:%s", __func__, lookup->entry_name);
				lookup->type = VOIP_EVENT_UNUSE;
				lstAdd (&event->event_unlst, (NODE *)lookup);
				if (event->mutex)
					os_mutex_lock(event->mutex, OS_WAIT_FOREVER);
			}
			else if(lookup->type == VOIP_EVENT_HIGH)
			{
				VOIP_EV_DEBUG( "============%s:HIGH:%s", __func__, lookup->entry_name);
				lstDelete (lst, (NODE *)lookup);
				lookup->type = VOIP_EVENT_EXECUTE;
				lstAdd (&event->execute_lst, (NODE *)lookup);
				brk++;
			}
			else if(lookup->type == VOIP_EVENT_READY)
			{
				VOIP_EV_DEBUG( "============%s:READY:%s", __func__, lookup->entry_name);
				lstDelete (lst, (NODE *)lookup);
				lookup->type = VOIP_EVENT_EXECUTE;
				lstAdd (&event->execute_lst, (NODE *)lookup);
				brk++;
			}
			else if(lookup->type == VOIP_EVENT_TIMER)
			{
				if(os_monotonic_time() >= lookup->timer)
				{
					//VOIP_EV_DEBUG( "============%s:TIMER:%s", __func__, lookup->entry_name);
					lstDelete (lst, (NODE *)lookup);
					lookup->type = VOIP_EVENT_EXECUTE;
					lstAdd (&event->execute_lst, (NODE *)lookup);
					brk++;
				}
			}
		}
	}
	return brk;
}

static int voip_event_type_call(voip_event_ctx_t *event, voip_event_type_t type)
{
	LIST *lst = NULL;
	zassert(event != NULL);
	switch(type)
	{
	case VOIP_EVENT_EXECUTE:
		lst = &event->execute_lst;
		break;
	case VOIP_EVENT_READY:
		lst = &event->ready_lst;
		break;
	case VOIP_EVENT_TIMER:
		lst = &event->timer_lst;
		break;
	case VOIP_EVENT_HIGH:
		lst = &event->high_lst;
		break;
	case VOIP_EVENT_UNUSE:
		lst = &event->event_unlst;
		break;
	default:
		break;
	}
	if(lst != NULL && lstCount(lst))
		return _voip_event_list_call(event, lst);
	return OK;
}


static int voip_event_sync_read(int fd)
{
	int ret = 0;
	char buf[1024];
	memset(buf, 0, sizeof(buf));
	if(fd > 0)
		ret = read(fd, buf, sizeof(buf));
	if(ret < 0)
		zlog_debug(ZLOG_VOIP, "============%s:%s", __func__, strerror(errno));
	return OK;
}
/*
 * VOIP Event Task
 */
static int voip_event_task(voip_event_ctx_t *event)
{
	int num = 0, max_fd = 0, fd = 0;
	fd_set rfdset;
	zassert(event != NULL);
	while(!os_load_config_done())
	{
		os_sleep(1);
	}
	while(!voip_socket_get_readfd())
	{
		os_sleep(1);
	}
	//max_fd = fd = voip_socket_get_readfd();
	event->enable = TRUE;
	while(!event->enable)
	{
		os_sleep(1);
	}
	max_fd = fd = voip_socket_get_readfd();
	while(event->enable)
	{
		if (event->mutex)
			os_mutex_lock(event->mutex, OS_WAIT_FOREVER);
		if(lstCount(&event->execute_lst) > 0)
		{
			voip_event_type_call(event, VOIP_EVENT_EXECUTE);
			if(event->mutex)
				os_mutex_unlock(event->mutex);
			continue;
		}
		if(event->mutex)
			os_mutex_unlock(event->mutex);

		max_fd = fd = voip_socket_get_readfd();
/*		if(event->sem)
			os_sem_take(event->sem, OS_WAIT_FOREVER);*/
		FD_ZERO(&rfdset);
		FD_SET(fd, &rfdset);

		if (event->mutex)
			os_mutex_lock(event->mutex, OS_WAIT_FOREVER);

		if(voip_event_type_call(event, VOIP_EVENT_HIGH) > 0)
		{
			if(event->mutex)
				os_mutex_unlock(event->mutex);
			continue;
		}

		if(voip_event_type_call(event, VOIP_EVENT_READY) > 0)
		{
			if(event->mutex)
				os_mutex_unlock(event->mutex);
			continue;
		}

		if(event->mutex)
			os_mutex_unlock(event->mutex);

		num = os_select_wait(max_fd + 1, &rfdset, NULL, 1000);

		if (event->mutex)
			os_mutex_lock(event->mutex, OS_WAIT_FOREVER);

		if(num > 0)
		{
			if(FD_ISSET(fd, &rfdset))
			{
				FD_CLR(fd, &rfdset);
				voip_event_sync_read(fd);
				if(voip_event_type_call(event, VOIP_EVENT_HIGH) > 0)
				{
					if(event->mutex)
						os_mutex_unlock(event->mutex);
					continue;
				}
				//voip_event_type_call(event, VOIP_EVENT_HIGH);
			}
		}
		//if(num == OS_TIMEOUT)
			voip_event_type_call(event, VOIP_EVENT_TIMER);

		if(event->mutex)
			os_mutex_unlock(event->mutex);
	}
	return OK;
}



int voip_event_task_init()
{
	zassert(voip_event != NULL);
	if(voip_event->taskid)
		return OK;

	voip_event->taskid = os_task_create("voipEvent", OS_TASK_DEFAULT_PRIORITY,
	               0, voip_event_task, voip_event, OS_TASK_DEFAULT_STACK);
	if(voip_event->taskid)
		return OK;
	return ERROR;
}


int voip_event_task_exit()
{
	zassert(voip_event != NULL);
	if(voip_event->taskid)
	{
		if(os_task_destroy(voip_event->taskid)==OK)
			voip_event->taskid = 0;
	}
	return OK;
}


voip_event_t * voip_event_add_one(voip_event_type_t type, voip_event_t *node)
{
	voip_event_t * pnode = NULL;
	zassert(voip_event != NULL);
	zassert(node != NULL);
	if (voip_event->mutex)
		os_mutex_lock(voip_event->mutex, OS_WAIT_FOREVER);

	if(_voip_event_list_lookup(voip_event, type, node) == ERROR)
	{
		if(voip_event->mutex)
			os_mutex_unlock(voip_event->mutex);
		return NULL;
	}

	pnode = voip_event_list_get_empty(voip_event);
	if(node && pnode)
	{
		memcpy(pnode, node, sizeof(voip_event_t));
		_voip_event_list_add(voip_event, type, pnode);

/*		os_sem_give(voip_event->sem);*/

		if(voip_event->mutex)
			os_mutex_unlock(voip_event->mutex);
		return pnode;
	}
	if(voip_event->mutex)
		os_mutex_unlock(voip_event->mutex);
	return NULL;
}

int voip_event_del_one(voip_event_t *node)
{
	zassert(node != NULL);
	zassert(voip_event != NULL);
	if (voip_event->mutex)
		os_mutex_lock(voip_event->mutex, OS_WAIT_FOREVER);

	if(node->type == VOIP_EVENT_UNUSE)
	{
		if(voip_event->mutex)
			os_mutex_unlock(voip_event->mutex);
		return OK;
	}
	if(_voip_event_list_lookup(voip_event, VOIP_EVENT_EXECUTE, node) == ERROR)
	{
		if(node)
		{
			int ret = _voip_event_list_del(voip_event, VOIP_EVENT_EXECUTE, node);
			if(voip_event->mutex)
				os_mutex_unlock(voip_event->mutex);
			//zlog_debug(ZLOG_VOIP, "===================%s: delete %s on EXECUTE", __func__, node->entry_name);
			return ret;
		}
	}
	else if(_voip_event_list_lookup(voip_event, VOIP_EVENT_HIGH, node) == ERROR)
	{
		if(node)
		{
			int ret = _voip_event_list_del(voip_event, VOIP_EVENT_HIGH, node);
			if(voip_event->mutex)
				os_mutex_unlock(voip_event->mutex);
			//zlog_debug(ZLOG_VOIP, "===================%s: delete %s on HIGH", __func__, node->entry_name);
			return ret;
		}
	}
	else if(_voip_event_list_lookup(voip_event, VOIP_EVENT_READY, node) == ERROR)
	{
		if(node)
		{
			int ret = _voip_event_list_del(voip_event, VOIP_EVENT_READY, node);
			if(voip_event->mutex)
				os_mutex_unlock(voip_event->mutex);
			//zlog_debug(ZLOG_VOIP, "===================%s: delete %s on READY", __func__, node->entry_name);
			return ret;
		}
	}
	else if(_voip_event_list_lookup(voip_event, VOIP_EVENT_TIMER, node) == ERROR)
	{
		if(node)
		{
			int ret = _voip_event_list_del(voip_event, VOIP_EVENT_TIMER, node);
			if(voip_event->mutex)
				os_mutex_unlock(voip_event->mutex);
			//zlog_debug(ZLOG_VOIP, "===================%s: delete %s on TIMER", __func__, node->entry_name);
			return ret;
		}
	}
	if(voip_event->mutex)
		os_mutex_unlock(voip_event->mutex);
	//zlog_err(ZLOG_VOIP, "===================%s: %d %s", __func__, node->type, node->entry_name);
	return ERROR;
}

voip_event_t * _voip_event_add_raw_one(voip_event_type_t type, int (*cb)(voip_event_t *),
		void *pVoid, char *buf, int len, int value, char *funcname)
{
	voip_event_t * pnode = NULL;
	zassert(cb != NULL);
	zassert(voip_event != NULL);
	if (voip_event->mutex)
		os_mutex_lock(voip_event->mutex, OS_WAIT_FOREVER);

	pnode = voip_event_list_get_empty(voip_event);
	if(pnode)
	{
		pnode->ev_cb = cb;
		pnode->pVoid = pVoid;
		if(funcname)
			strncpy(pnode->entry_name, funcname, MIN(strlen(funcname), VOIP_EVENT_NAME_MAX));
		if(buf && len)
		{
			memcpy(pnode->data, buf, MIN(len, VOIP_EVENT_DATA_MAX));
			pnode->dlen = len;
		}
		switch(type)
		{
		case VOIP_EVENT_READY:
			//pnode->fd = value;
			break;
		case VOIP_EVENT_TIMER:
			pnode->interval = value;
			pnode->timer = os_monotonic_time() + value;
			break;
		case VOIP_EVENT_HIGH:
			break;
		default:
			break;
		}
		pnode->type = type;

		if(_voip_event_list_lookup(voip_event, type, pnode) == ERROR)
		{
			lstAdd (&voip_event->event_unlst, (NODE *)pnode);
			if(voip_event->mutex)
				os_mutex_unlock(voip_event->mutex);
			return ERROR;
		}
		_voip_event_list_add(voip_event, pnode->type, pnode);
		//os_sem_give(voip_event->sem);

		if(voip_event->mutex)
			os_mutex_unlock(voip_event->mutex);
		if(type == VOIP_EVENT_HIGH)
		{
			voip_socket_sync_cmd();
		}
		return pnode;
	}

	if(voip_event->mutex)
		os_mutex_unlock(voip_event->mutex);
	return pnode;
}



static int _voip_event_list_debug_one(voip_event_ctx_t *event, LIST *lst, struct vty *vty, char *lsthdr)
{
	NODE node;
	voip_event_t *lookup = NULL;
	zassert(event != NULL);
	zassert(lst != NULL);
	for (lookup = (voip_event_t *) lstFirst(lst);
			lookup != NULL; lookup = (voip_event_t *) lstNext(&node))
	{
		node = lookup->node;
		if (lookup && lookup->ev_cb)
		{
			if(lookup->type == VOIP_EVENT_EXECUTE)
			{
				if(vty)
					vty_out(vty, "%s EXECUTE      : %s%s", lsthdr, lookup->entry_name, VTY_NEWLINE);
				else
					zlog_debug(ZLOG_VOIP, "============%s:EXECUTE:%s", __func__, lookup->entry_name);
			}
			else if(lookup->type == VOIP_EVENT_HIGH)
			{
				if(vty)
					vty_out(vty, "%s HIGH         : %s%s", lsthdr, lookup->entry_name, VTY_NEWLINE);
				else
					zlog_debug(ZLOG_VOIP, "============%s:HIGH:%s", __func__, lookup->entry_name);
			}
			else if(lookup->type == VOIP_EVENT_READY)
			{
				if(vty)
					vty_out(vty, "%s READY        : %s%s", lsthdr, lookup->entry_name, VTY_NEWLINE);
				else
					zlog_debug(ZLOG_VOIP, "============%s:READY:%s", __func__, lookup->entry_name);
			}
			else if(lookup->type == VOIP_EVENT_TIMER)
			{
				if(vty)
					vty_out(vty, "%s TIMER        : %s%s", lsthdr, lookup->entry_name, VTY_NEWLINE);
				else
					zlog_debug(ZLOG_VOIP, "============%s:TIMER:%s", __func__, lookup->entry_name);
			}
			else if(lookup->type == VOIP_EVENT_UNUSE)
			{
				if(vty)
					vty_out(vty, "%s UNUSE        : %s%s", lsthdr, lookup->entry_name, VTY_NEWLINE);
				else
					zlog_debug(ZLOG_VOIP, "============%s:UNUSE:%s", __func__, lookup->entry_name);
			}
		}
	}
	return OK;
}

int _voip_event_list_debug(struct vty *vty)
{
	if(lstCount(&voip_event->execute_lst))
		_voip_event_list_debug_one(voip_event, &voip_event->execute_lst, vty, "execute lst");
	if(lstCount(&voip_event->ready_lst))
		_voip_event_list_debug_one(voip_event, &voip_event->ready_lst, vty, "ready lst  ");
	if(lstCount(&voip_event->timer_lst))
		_voip_event_list_debug_one(voip_event, &voip_event->timer_lst, vty, "timer lst  ");
	if(lstCount(&voip_event->high_lst))
		_voip_event_list_debug_one(voip_event, &voip_event->high_lst, vty, "high lst   ");
	if(lstCount(&voip_event->event_unlst))
		_voip_event_list_debug_one(voip_event, &voip_event->event_unlst, vty, "unuse lst  ");
	return OK;
}
