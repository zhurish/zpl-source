/*
 * os_ansync.c
 *
 *  Created on: Sep 3, 2018
 *      Author: zhurish
 */
#include "os_include.h"
#include "zpl_include.h"
#include "sys/epoll.h"


#ifdef OS_ANSYNC_GLOBAL_LIST
static LIST	*ansyncList = NULL;
static zpl_uint8	g_ansync_init = 0;
static os_ansync_t *_m_os_ansync_current = NULL;

static int os_ansync_global_init(void)
{
	if(g_ansync_init == 0)
	{
		ansyncList = os_malloc(sizeof(LIST));
		if(ansyncList)
		{
			lstInit(ansyncList);
			g_ansync_init = 1;
			return OK;
		}
	}
	return ERROR;
}

os_ansync_lst * os_ansync_global_lookup(zpl_uint32 taskid, zpl_uint32 module)
{
	NODE index;
	os_ansync_lst *pstNode = NULL;
	for(pstNode = (os_ansync_lst *)lstFirst(ansyncList);
			pstNode != NULL;  pstNode = (os_ansync_lst *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(taskid && (taskid == pstNode->taskid) )
			return pstNode;
		else if(module && (pstNode->module == module))
		{
			return pstNode;
		}
	}
	return NULL;
}
#endif

static os_ansync_t * os_ansync_lookup_node(os_ansync_lst *lst, os_ansync_t *value);


os_ansync_lst *os_ansync_lst_create(zpl_uint32 module, int maxfd)
{
	os_ansync_lst *lst = os_malloc(sizeof(os_ansync_lst));
#ifdef OS_ANSYNC_GLOBAL_LIST
	os_ansync_global_init();
#endif
	if(lst)
	{
		os_memset(lst, 0, sizeof(os_ansync_lst));
		lst->mutex = os_mutex_init();
		if(!lst->mutex)
		{
			os_free(lst);
			return NULL;
		}
		lst->ansync_mutex = os_mutex_init();
		if(!lst->ansync_mutex)
		{
			os_mutex_exit(lst->mutex);
			os_free(lst);
			return NULL;
		}

		lst->list = os_malloc(sizeof(LIST));
		lst->unuselist = os_malloc(sizeof(LIST));
		lst->events = os_malloc(sizeof(struct epoll_event) * maxfd);
		lst->epoll_fd = epoll_create(maxfd);
		if(!lst->list || !lst->unuselist || !lst->events || !lst->epoll_fd)
		{
			if(lst->list)
			{
				os_free(lst->list);
			}
			if(lst->unuselist)
			{
				os_free(lst->unuselist);
			}
			if(lst->events)
			{
				os_free(lst->events);
			}
			if(lst->epoll_fd)
			{
				close(lst->epoll_fd);
			}
			if(lst->mutex)
				os_mutex_exit(lst->mutex);
			os_free(lst);
			return NULL;
		}
		lst->module = module;
		lst->taskid = os_task_id_self();
		os_memset(lst->events, 0, sizeof(struct epoll_event) * maxfd);
		lst->max_fd = maxfd;
		lstInit(lst->list);
		lstInit(lst->unuselist);
		lst->bquit = zpl_false;
#ifdef OS_ANSYNC_GLOBAL_LIST
		lstAdd(ansyncList, (NODE *)lst);
#endif
	}
	return lst;
}

int os_ansync_lst_destroy(os_ansync_lst *lst)
{
	if(lst)
	{
#ifdef OS_ANSYNC_GLOBAL_LIST
		lstDelete(ansyncList, (NODE *)lst);
#endif
		if(lst->list && lstCount(lst->list))
			lstFree(lst->list);
		if(lst->unuselist && lstCount(lst->unuselist))
			lstFree(lst->unuselist);

		if(lst->mutex)
			os_mutex_exit(lst->mutex);
		if(lst->ansync_mutex)
			os_mutex_exit(lst->ansync_mutex);

		if(lst->list)
		{
			os_free(lst->list);
		}
		if(lst->unuselist)
		{
			os_free(lst->unuselist);
		}
		if(lst->events)
		{
			os_free(lst->events);
		}
		if(lst->epoll_fd)
		{
			close(lst->epoll_fd);
		}
		os_free(lst);
	}
	return OK;
}

static zpl_bool os_ansync_lst_chk(os_ansync_lst *lst)
{
	if (!lst)
		return zpl_false;
	if (!lst->mutex)
		return zpl_false;
	if (!lst->list)
		return zpl_false;
	if (!lst->unuselist)
		return zpl_false;
	return zpl_true;
}

static int os_ansync_epoll_event_add(os_ansync_lst *lst, os_ansync_t *value)
{
	os_ansync_t *elem = NULL;
	struct epoll_event ev;
	elem = os_ansync_lookup_node(lst, value);
    if (elem)
    {
        os_memset(&ev, 0, sizeof(struct epoll_event));
        ev.data.fd = value->fd;
        if(value->type == OS_ANSYNC_INPUT)
        	ev.events = EPOLLIN;
        else if(value->type == OS_ANSYNC_OUTPUT)
        	ev.events = EPOLLOUT;
        return epoll_ctl(lst->epoll_fd, EPOLL_CTL_MOD, value->fd, &ev);
    }
    else
    {
        os_memset(&ev, 0, sizeof(struct epoll_event));
        ev.data.fd = value->fd;
        if(value->type == OS_ANSYNC_INPUT)
        	ev.events = EPOLLIN;
        else if(value->type == OS_ANSYNC_OUTPUT)
        	ev.events = EPOLLOUT;
        return epoll_ctl(lst->epoll_fd, EPOLL_CTL_ADD, value->fd, &ev);
    }
    return ERROR;
}

static int os_ansync_epoll_event_del(os_ansync_lst *lst, os_ansync_t *value)
{
	return epoll_ctl(lst->epoll_fd, EPOLL_CTL_DEL, value->fd, NULL);
}

static int os_ansync_add_node_sort(os_ansync_lst *lst, os_ansync_t *value)
{
	NODE index;
	os_ansync_t *pstNode = NULL;
	os_ansync_t *pPrev = NULL;
	for(pstNode = (os_ansync_t *)lstFirst(lst->list);
			pstNode != NULL;  pstNode = (os_ansync_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if((pstNode->type == OS_ANSYNC_TIMER || pstNode->type == OS_ANSYNC_TIMER_ONCE) &&
				pstNode->state == OS_ANSYNC_STATE_NONE)
		{
			if(os_timeval_cmp(pstNode->timeout, value->timeout) > 0)
			{
				pPrev = (os_ansync_t *)pstNode->node.previous;
				break;
			}
		}
	}
	if(pPrev)
		lstInsert(lst->list, (NODE *)pPrev, (NODE *)value);
	else
		lstAdd(lst->list, (NODE *)value);
	return OK;
}

static int os_ansync_add_node(os_ansync_lst *lst, os_ansync_t *value)
{
	int ret = ERROR;
	if(!os_ansync_lst_chk(lst))
		return ERROR;
	if(value)
	{
		value->master = lst;
		if(value->fd && (value->type == OS_ANSYNC_INPUT || value->type == OS_ANSYNC_OUTPUT))
		{
/*
			if(value->type == OS_ANSYNC_INPUT)
				FD_SET(value->fd, &lst->rfdset);
			else if(value->type == OS_ANSYNC_OUTPUT)
				FD_SET(value->fd, &lst->wfdset);
			lst->maxfd = MAX(lst->maxfd, value->fd);
*/
			ret = os_ansync_epoll_event_add(lst, value);
			OS_ANSYNC_DEBUG("os ansync add %d(%s)ret = %d: %s:%d", value->fd,
					(value->type == OS_ANSYNC_INPUT) ? "input":"output",ret, value->entryname, value->line);
		}
		else if(value->type == OS_ANSYNC_TIMER || value->type == OS_ANSYNC_TIMER_ONCE)
		{
			//获取定时链表最小时间轴的时间
			//os_timeval_cmp(value->timeout, )
/*			if(lst->timeout.tv_sec == 0 && lst->timeout.tv_usec == 0)
				lst->timeout = os_time_max(lst->timeout, value->timeout);
			else
				lst->timeout = os_time_min(lst->timeout, value->timeout);*/
/*			if(lst->timeout && value->interval)
				lst->timeout = MIN(lst->timeout, value->interval);*/
//			OS_ANSYNC_DEBUG("os ansync add %d msec(timer):%s:%d", value->interval, value->entryname, value->line);
			ret = OK;
		}
		else if(value->type == OS_ANSYNC_EVENT)
		{
			value->state = OS_ANSYNC_STATE_READY;
			ret = OK;
		}
		if(ret == OK)
		{
			if(value->type == OS_ANSYNC_TIMER || value->type == OS_ANSYNC_TIMER_ONCE)
				os_ansync_add_node_sort(lst, value);
			else
				lstAdd(lst->list, (NODE *)value);
		}
		return OK;
	}
	return ERROR;
}


static int os_ansync_del_node(os_ansync_lst *lst, os_ansync_t *value)
{
	int ret = ERROR;
	if(!os_ansync_lst_chk(lst))
		return ERROR;
	if(value)
	{
		if(value->fd && (value->type == OS_ANSYNC_INPUT || value->type == OS_ANSYNC_OUTPUT))
		{
/*			if(value->type == OS_ANSYNC_INPUT)
				FD_CLR(value->fd, &lst->rfdset);
			else if(value->type == OS_ANSYNC_OUTPUT)
				FD_CLR(value->fd, &lst->wfdset);
			lst->maxfd = MIN(lst->maxfd, value->fd);*/
			//epoll_ctl(lst->epoll_fd, EPOLL_CTL_DEL, value->fd, NULL);
			ret = os_ansync_epoll_event_del(lst, value);
		}
		else if(value->type == OS_ANSYNC_TIMER || value->type == OS_ANSYNC_TIMER_ONCE)
		{
/*			if(lst->timeout && value->interval)
				lst->timeout = MIN(lst->timeout, value->interval);
*/
/*			if(os_timeval_cmp(value->interval, lst->timeout))
			if(lst->timeout.tv_sec == 0 && lst->timeout.tv_usec == 0)
				lst->timeout = os_time_max(lst->timeout, value->interval);
			else
				lst->timeout = os_time_min(lst->timeout, value->interval);
*/
/*			struct timeval	interval;
			os_gettime (OS_CLK_MONOTONIC, &interval);
			//删除定时事件，更新最小时间轴
			if(os_timeval_cmp(lst->timeout, interval) > 0)
			{
				lst->timeout = os_timeval_subtract(lst->timeout, interval);
				lst->timeout.tv_sec += interval.tv_sec;
				lst->timeout.tv_usec += interval.tv_usec;
			}
			OS_ANSYNC_DEBUG("os ansync del min timer msec %d :now:%d", OS_ANSYNC_SEC(lst->timeout.tv_sec),
					OS_ANSYNC_SEC(interval.tv_sec));*/
			ret = OK;
		}
		else if(value->type == OS_ANSYNC_EVENT)
		{
			ret = OK;
		}
		if(ret == OK)
		{
			lstDelete(lst->list, (NODE *)value);
			os_memset(value, 0, sizeof(os_ansync_t));
			lstAdd(lst->unuselist, (NODE *)value);
		}

		return OK;
	}
	return ERROR;
}

static os_ansync_t * os_ansync_lookup_node(os_ansync_lst *lst, os_ansync_t *value)
{
	os_ansync_t *pstNode = NULL;
	NODE index;
	if(!os_ansync_lst_chk(lst))
		return NULL;
	for(pstNode = (os_ansync_t *)lstFirst(lst->list);
			pstNode != NULL;  pstNode = (os_ansync_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(value && value->fd && (value->fd == pstNode->fd) &&
				(pstNode->type == value->type) )
			return pstNode;
		else if(value && value->ansync_cb &&
				(value->ansync_cb== pstNode->ansync_cb) && (pstNode->type == value->type))
		{
			if(value->pVoid && value->pVoid== pstNode->pVoid)
				return pstNode;
			else
				return pstNode;
		}
	}
	return NULL;
}

static int os_ansync_min_timer_refresh(os_ansync_lst *lst)
{
	os_ansync_t *pstNode = NULL;
	NODE index;
	if(!os_ansync_lst_chk(lst))
		return ERROR;
	if(lst->mutex)
		os_mutex_lock(lst->mutex, OS_WAIT_FOREVER);
	for(pstNode = (os_ansync_t *)lstFirst(lst->list);
			pstNode != NULL;  pstNode = (os_ansync_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if((pstNode->type == OS_ANSYNC_TIMER || pstNode->type == OS_ANSYNC_TIMER_ONCE) &&
				pstNode->state == OS_ANSYNC_STATE_NONE)
		{
			lst->timeout.tv_sec = pstNode->timeout.tv_sec;
			lst->timeout.tv_usec = pstNode->timeout.tv_usec;
			os_timeval_adjust(lst->timeout);
			break;
		}
	}
	if(lst->mutex)
		os_mutex_unlock(lst->mutex);
	return OK;
}

os_ansync_t * os_ansync_lookup_api(os_ansync_lst *lst, os_ansync_t *value)
{
	if(!lst)
		return NULL;
	if(!os_ansync_lst_chk(lst))
		return NULL;
	if(lst->mutex)
		os_mutex_lock(lst->mutex, OS_WAIT_FOREVER);
	os_ansync_t *node = os_ansync_lookup_node(lst, value);
	if(lst->mutex)
		os_mutex_unlock(lst->mutex);
	return node;
}



int os_ansync_add_api(os_ansync_lst *lst, os_ansync_t *value)
{
	int ret = ERROR;
	if(!lst)
		return ERROR;
	if(!os_ansync_lst_chk(lst))
		return ERROR;
	if(lst->mutex)
		os_mutex_lock(lst->mutex, OS_WAIT_FOREVER);
	if(!os_ansync_lookup_node(lst, value))
	{
		ret = os_ansync_add_node(lst, value);
	}
	else
	{
		OS_ANSYNC_DEBUG("os ansync %s is already here", value->entryname);
	}
	if(lst->mutex)
		os_mutex_unlock(lst->mutex);
	return ret;
}

int os_ansync_timeout_api(os_ansync_lst *lst, zpl_uint32 value)
{
	int ret = OK;
	if(!lst)
		return ERROR;
	if(!os_ansync_lst_chk(lst))
		return ERROR;
	if(lst->mutex)
		os_mutex_lock(lst->mutex, OS_WAIT_FOREVER);
	lst->interval = value;
	if(lst->mutex)
		os_mutex_unlock(lst->mutex);
	return ret;
}

int os_ansync_del_api(os_ansync_lst *lst, os_ansync_t *value)
{
	os_ansync_t *node;
	int ret = ERROR;
	if(!lst)
		return ERROR;
	if(!os_ansync_lst_chk(lst))
		return ERROR;
	if(lst->mutex)
		os_mutex_lock(lst->mutex, OS_WAIT_FOREVER);
	node = os_ansync_lookup_node(lst, value);
	if(node)
	{
		ret = os_ansync_del_node(lst, value);
	}
	if(lst->mutex)
		os_mutex_unlock(lst->mutex);
	return ret;
}



static os_ansync_t * os_ansync_get(os_ansync_lst *lst)
{
	NODE index;
	os_ansync_t *pstNode = NULL;
	if(!os_ansync_lst_chk(lst))
		return NULL;
	if(lst->mutex)
		os_mutex_lock(lst->mutex, OS_WAIT_FOREVER);
	for(pstNode = (os_ansync_t *)lstFirst(lst->unuselist);
			pstNode != NULL;  pstNode = (os_ansync_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode->state == OS_ANSYNC_STATE_NONE)
		{
			lstDelete(lst->unuselist, (NODE *)pstNode);
			break;
		}
	}
	if(lst->mutex)
		os_mutex_unlock(lst->mutex);
	return pstNode;
}

os_ansync_t * _os_ansync_register_api(os_ansync_lst *lst, os_ansync_type type, os_ansync_cb cb,
		void *pVoid, int value, zpl_char *func_name, zpl_char *file, zpl_uint32 line)
{
	//int ret = 0;
	os_ansync_t *node = NULL;
	if(!lst)
		return NULL;
	if(!os_ansync_lst_chk(lst))
		return NULL;
	//OS_ANSYNC_DEBUG("%s", func_name);
	if(lstCount(lst->unuselist))
	{
		node = os_ansync_get(lst);
		//lstDelete(lst->unuselist, (NODE *)node);
	}
	if(!node)
	{
		node = os_malloc(sizeof(os_ansync_t));
	}
	if(node)
	{
		//OS_ANSYNC_DEBUG("%s", func_name);
		os_memset(node, 0, sizeof(os_ansync_t));
		if(type == OS_ANSYNC_INPUT || type == OS_ANSYNC_OUTPUT)
			node->fd = value;
		else if(type == OS_ANSYNC_TIMER || type == OS_ANSYNC_TIMER_ONCE)
		{
			os_gettime (OS_CLK_MONOTONIC, &node->timeout);
			//设置定时时间轴
			node->timeout.tv_sec += value/OS_ANSYNC_MSEC_MICRO;
			node->timeout.tv_usec += (value%OS_ANSYNC_MSEC_MICRO)*OS_ANSYNC_MSEC_MICRO;
			os_timeval_adjust(node->timeout);
			//设置定时间隔
			node->interval = value;
			OS_ANSYNC_DEBUG("os ansync register %d msec(timer):%s:%d", node->interval, func_name, line);
			//zlog_debug(6, "os ansync register %d (%d)msec(timer):%s:%d", node->interval, node->timeout.tv_sec, func_name, line);
		}
		node->type 		= type;
		node->ansync_cb = cb;
		node->pVoid 	= pVoid;
		os_strcpy(node->entryname, func_name);
		os_strcpy(node->filename, file);
		node->line = line;
		if(os_ansync_add_api(lst, node) == OK)
			return node;
	}
	return NULL;
}


int _os_ansync_unregister_api(os_ansync_lst *lst, os_ansync_type type, os_ansync_cb cb,
		void *pVoid, int value)
{
	os_ansync_t find;
	os_ansync_t *node = NULL;
	int ret = ERROR;
	if(!lst)
		return ERROR;
	if(!os_ansync_lst_chk(lst))
		return ERROR;


	os_memset(&find, 0, sizeof(os_ansync_t));
	if(type == OS_ANSYNC_INPUT || type == OS_ANSYNC_OUTPUT)
		find.fd = value;
	else
		find.interval = value;
	find.type 		= type;
	find.ansync_cb = cb;
	find.pVoid 	= pVoid;

	if(lst->mutex)
		os_mutex_lock(lst->mutex, OS_WAIT_FOREVER);
	node = os_ansync_lookup_node(lst, &find);
	if(node)
	{
		ret = os_ansync_del_node(lst, node);
	}
	if(lst->mutex)
		os_mutex_unlock(lst->mutex);
	return ret;
}

int _os_ansync_unregister_all_api(os_ansync_lst *lst, os_ansync_type type, os_ansync_cb cb,
		void *pVoid, int value)
{
	NODE index;
	os_ansync_t *pstNode = NULL;
	//zpl_time_t cut = os_time(NULL);
	if(!os_ansync_lst_chk(lst))
		return ERROR;
	if(lst->mutex)
		os_mutex_lock(lst->mutex, OS_WAIT_FOREVER);
	for(pstNode = (os_ansync_t *)lstFirst(lst->list);
			pstNode != NULL;  pstNode = (os_ansync_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(type == OS_ANSYNC_INPUT || type == OS_ANSYNC_OUTPUT)
		{
			if(value && (value == pstNode->fd) &&
					(pstNode->type == type) )
				os_ansync_del_node(lst, pstNode);
		}
		else
		{
			if((pstNode->type == type))
			{
				if(cb)
				{
					if((cb == pstNode->ansync_cb))
					{
						if(pVoid && pstNode->pVoid == pVoid)
						{
							os_ansync_del_node(lst, pstNode);
						}
						if(!pVoid)
						{
							os_ansync_del_node(lst, pstNode);
						}
					}
				}
				if(pVoid)
				{
					if(pVoid && pstNode->pVoid == pVoid)
					{
						if(cb && pstNode->ansync_cb == cb)
						{
							os_ansync_del_node(lst, pstNode);
						}
						if(!cb)
						{
							os_ansync_del_node(lst, pstNode);
						}
					}
				}
			}
		}
	}
	if(lst->mutex)
		os_mutex_unlock(lst->mutex);
	return OK;
}

os_ansync_t * _os_ansync_register_event_api(os_ansync_lst *lst, os_ansync_type type, os_ansync_cb cb,
		void *pVoid, int value, zpl_char *func_name, zpl_char *file, zpl_uint32 line)
{
	os_ansync_t * ret = 0;
	if(lst->ansync_mutex)
		os_mutex_lock(lst->ansync_mutex, OS_WAIT_FOREVER);

	ret = _os_ansync_register_api(lst,  type,  cb,
			pVoid,  value, func_name, file,  line);
	if(lst->ansync_mutex)
		os_mutex_unlock(lst->ansync_mutex);
	return ret;
}

static int os_ansync_io_helper(os_ansync_lst *lst, zpl_uint32 num)
{
	zpl_uint32 i = 0;
	//NODE index;
	os_ansync_t value;
	os_ansync_t *node = NULL;
	if(!os_ansync_lst_chk(lst))
		return ERROR;
	if(lst->mutex)
		os_mutex_lock(lst->mutex, OS_WAIT_FOREVER);

	for(i = 0; i < num; i++)
	{
		if ((lst->events[i].events & EPOLLIN) || (lst->events[i].events & EPOLLPRI))
		{
			os_memset(&value, 0, sizeof(os_ansync_t));
			value.fd = lst->events[i].data.fd;
			value.type = OS_ANSYNC_INPUT;
			node = os_ansync_lookup_node(lst, &value);
			if(node)
			{
				//OS_ANSYNC_DEBUG("io(%d)  input %s", node->fd, node->entryname);
				node->state = OS_ANSYNC_STATE_READY;
				//lstDelete(lst->list, (NODE *)node);
				//lstAdd(lst->unuselist, (NODE *)node);
			}
			lst->events[i].events &= ~(EPOLLIN|EPOLLPRI);
			if(!(lst->events[i].events & EPOLLOUT))
				lst->events[i].data.fd = 0;
		}
		if ((lst->events[i].events & EPOLLOUT))
		{
			os_memset(&value, 0, sizeof(os_ansync_t));
			value.fd = lst->events[i].data.fd;
			value.type = OS_ANSYNC_OUTPUT;
			node = os_ansync_lookup_node(lst, &value);
			if(node)
			{
				//OS_ANSYNC_DEBUG("io(%d)  input %s", node->fd, node->entryname);
				node->state = OS_ANSYNC_STATE_READY;
				//lstDelete(lst->list, (NODE *)node);
				//lstAdd(lst->unuselist, (NODE *)node);
			}
			lst->events[i].events &= ~(EPOLLOUT);
			lst->events[i].data.fd = 0;
		}
	}
	if(lst->mutex)
		os_mutex_unlock(lst->mutex);
	return OK;
}


static int os_ansync_timer_helper(os_ansync_lst *lst, struct timeval *wait_tv)
{
	NODE index;
	os_ansync_t *pstNode = NULL;
	//zpl_time_t cut = os_time(NULL);
	if(!os_ansync_lst_chk(lst))
		return ERROR;
	if(lst->mutex)
		os_mutex_lock(lst->mutex, OS_WAIT_FOREVER);
	for(pstNode = (os_ansync_t *)lstFirst(lst->list);
			pstNode != NULL;  pstNode = (os_ansync_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode->type == OS_ANSYNC_TIMER || pstNode->type == OS_ANSYNC_TIMER_ONCE)
		{
			//检测是否有定时事件需要执行
			if(os_timeval_cmp(*wait_tv, pstNode->timeout) >= 0)
			{
				//if(lst->taskid == os_task_lookup_by_name("dhcpcTask"))
				//	zlog_debug(6, "os_ansync_timer_helper : %s now=%d msec :%d", pstNode->entryname, wait_tv->tv_sec, pstNode->timeout.tv_sec);
				//OS_ANSYNC_DEBUG("io timer %s", pstNode->entryname);
				pstNode->state = OS_ANSYNC_STATE_READY;
				//lstDelete(lst->list, (NODE *)pstNode);
				//lstAdd(lst->unuselist, (NODE *)pstNode);
				if(pstNode->type == OS_ANSYNC_TIMER)
				{
					os_gettime (OS_CLK_MONOTONIC, &pstNode->timeout);
					//更新定时期下次定时时间
					pstNode->timeout.tv_sec += pstNode->interval/OS_ANSYNC_MSEC_MICRO;
					pstNode->timeout.tv_usec += (pstNode->interval%OS_ANSYNC_MSEC_MICRO)*OS_ANSYNC_MSEC_MICRO;
					os_timeval_adjust(pstNode->timeout);
					OS_ANSYNC_DEBUG("os ansync update nest %d msec(timer):%s:%d", OS_ANSYNC_TVTOMSEC(pstNode->timeout), pstNode->entryname, pstNode->line);
				}
				else if(pstNode->type == OS_ANSYNC_TIMER_ONCE)
				{
					pstNode->timeout.tv_sec = OS_EVENTS_TIMER_INTERVAL_MAX;
					pstNode->timeout.tv_usec = OS_EVENTS_TIMER_INTERVAL_MAX;
					os_timeval_adjust(pstNode->timeout);
				}
			}
/*			//更新下一个定时时间
			if(os_timeval_cmp(*wait_tv, lst->timeout) >= 0)
			{
				lst->timeout = os_time_max(lst->timeout, pstNode->timeout);
			}
			else
			{
				lst->timeout = os_time_min(lst->timeout, pstNode->timeout);
			}*/
		}
	}
	if(lst->mutex)
		os_mutex_unlock(lst->mutex);
	return OK;
}


static os_ansync_t *
os_ansync_run(os_ansync_lst *lst)
{
	NODE index;
	os_ansync_t *pstNode = NULL;
	if(!os_ansync_lst_chk(lst))
		return NULL;
	if(lst->mutex)
		os_mutex_lock(lst->mutex, OS_WAIT_FOREVER);
	for(pstNode = (os_ansync_t *)lstFirst(lst->list);
			pstNode != NULL;  pstNode = (os_ansync_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode->state == OS_ANSYNC_STATE_READY)
		{
			//OS_ANSYNC_DEBUG("io run %s", pstNode->entryname);
			break;
		}
	}
	if(pstNode)
	{
		if(pstNode->type == OS_ANSYNC_TIMER ||
				pstNode->type == OS_ANSYNC_TIMER_ONCE ||
				pstNode->type == OS_ANSYNC_EVENT)
		{
			lstDelete(lst->list, (NODE *)pstNode);
		}
	}
	if (lst->mutex)
		os_mutex_unlock(lst->mutex);
	return pstNode;
}
int os_ansync_fetch_quit (os_ansync_lst *lst)
{
	if(lst)
	{
		lst->bquit = zpl_true;
	}
	return OK;
}

int os_ansync_fetch_wait (os_ansync_lst *lst)
{
	if(lst)
	{
		while(lst->bquit)
		{
			os_msleep(50);
		}
	}
	return OK;
}


os_ansync_t *os_ansync_fetch(os_ansync_lst *lst)
{
	int num = 0;
	struct timeval wait_tv;
	os_ansync_t *node = NULL;
	if(!os_ansync_lst_chk(lst))
		return NULL;

	if(lst->taskid <= 0)
		lst->taskid = os_task_id_self();
	while(1)
	{
		node = os_ansync_run(lst);
		if(node)
			return node;
		if(lst->bquit)
		{
			//fetch = NULL;
			lst->bquit = zpl_false;
			return NULL;
		}
		os_gettime (OS_CLK_MONOTONIC, &wait_tv);
		if(os_ansync_min_timer_refresh(lst) == OK)
		{
			if(os_timeval_cmp(wait_tv, lst->timeout) >= 0)
			{
				lst->timeout.tv_sec = wait_tv.tv_sec + OS_EVENTS_TIMER_DEFAULT/OS_ANSYNC_MSEC_MICRO;
				lst->timeout.tv_usec = wait_tv.tv_usec;
				os_timeval_adjust(lst->timeout);
			}
		}
		else
		{
			lst->timeout.tv_sec = wait_tv.tv_sec + 2;
			lst->timeout.tv_usec = wait_tv.tv_usec;
			os_timeval_adjust(lst->timeout);
		}
		lst->timeout = os_timeval_subtract(lst->timeout, wait_tv);
		lst->interval = OS_ANSYNC_TVTOMSEC(lst->timeout);

		OS_ANSYNC_DEBUG("timeout = %d(msec)", lst->interval);
		//fprintf(stderr, "timeout = %d(msec)\r\n", lst->interval);
		num = epoll_wait(lst->epoll_fd, lst->events, lst->max_fd, lst->interval);
		if (num < 0)
		{
			if (ipstack_errno == EINTR || ipstack_errno == EAGAIN || ipstack_errno == EWOULDBLOCK)
				continue;
			if (ipstack_errno == EBADF || ipstack_errno == EFAULT || ipstack_errno == EINVAL)
				return NULL;
			return NULL;
		}
		else if (num == 0)
		{
			os_gettime (OS_CLK_MONOTONIC, &wait_tv);
			os_ansync_timer_helper(lst, &wait_tv);
			node = os_ansync_run(lst);
			if(node)
				return node;
		}
		else
		{
			os_ansync_io_helper(lst, num);
			node = os_ansync_run(lst);
			if(node)
				return node;
		}
	}
	return NULL;
}

int os_ansync_execute(os_ansync_lst *lst, os_ansync_t *value, os_ansync_exe exe)
{
	int ret = ERROR;
	if(!os_ansync_lst_chk(lst))
		return ERROR;
	if(!value)
		return ERROR;
/*
	if(lst->mutex)
		os_mutex_lock(lst->mutex, OS_WAIT_FOREVER);
*/
	if(lst->ansync_mutex)
		os_mutex_lock(lst->ansync_mutex, OS_WAIT_FOREVER);

	lst->os_ansync = value;

	if(value->type == OS_ANSYNC_INPUT || value->type == OS_ANSYNC_OUTPUT)
	{
		if(value->ansync_cb)
		{
			//OS_ANSYNC_DEBUG("io (%d) %s (%s)",value->fd,  value->entryname, (value->type == OS_ANSYNC_INPUT) ? "input":"output");
			if(exe == OS_ANSYNC_EXECUTE_NONE)
				ret = (value->ansync_cb)(value);
			else
				ret = (value->ansync_cb)(value->pVoid);
		}
		value->state = OS_ANSYNC_STATE_NONE;
	}
	else if(value->type == OS_ANSYNC_TIMER || value->type == OS_ANSYNC_TIMER_ONCE)
	{
		if(value->ansync_cb)
		{
			//OS_ANSYNC_DEBUG("%s (%s)", value->entryname,  "timer");
			if(exe == OS_ANSYNC_EXECUTE_NONE)
				ret = (value->ansync_cb)(value);
			else
				ret = (value->ansync_cb)(value->pVoid);
		}
		value->state = OS_ANSYNC_STATE_NONE;

		if(value->type == OS_ANSYNC_TIMER_ONCE)
		{
			if(lst->mutex)
				os_mutex_lock(lst->mutex, OS_WAIT_FOREVER);
			//lstDelete(lst->list, (NODE *)value);
			lstAdd(lst->unuselist, (NODE *)value);
			if(lst->mutex)
				os_mutex_unlock(lst->mutex);
		}
		else
		{
			if(lst->mutex)
				os_mutex_lock(lst->mutex, OS_WAIT_FOREVER);

			os_gettime (OS_CLK_MONOTONIC, &value->timeout);
			//更新定时期下次定时时间
			value->timeout.tv_sec += value->interval/OS_ANSYNC_MSEC_MICRO;
			value->timeout.tv_usec += (value->interval%OS_ANSYNC_MSEC_MICRO)*OS_ANSYNC_MSEC_MICRO;
			os_timeval_adjust(value->timeout);
			os_ansync_add_node_sort(lst, value);
			if(lst->mutex)
				os_mutex_unlock(lst->mutex);
		}
	}
	else if(value->type == OS_ANSYNC_EVENT)
	{
		if(value->ansync_cb)
		{
			if(lst->mutex)
				os_mutex_lock(lst->mutex, OS_WAIT_FOREVER);
			//lstDelete(lst->list, (NODE *)value);
			lstAdd(lst->unuselist, (NODE *)value);
			_m_os_ansync_current = value;
			if(lst->mutex)
				os_mutex_unlock(lst->mutex);

			zpl_backtrace_symb_set(value->filename, value->entryname, value->line);
			//OS_ANSYNC_DEBUG("io (%d) %s (%s)",value->fd,  value->entryname, (value->type == OS_ANSYNC_INPUT) ? "input":"output");
			if(exe == OS_ANSYNC_EXECUTE_NONE)
				ret = (value->ansync_cb)(value);
			else
				ret = (value->ansync_cb)(value->pVoid);
			zpl_backtrace_symb_set(NULL, NULL, 0);
			_m_os_ansync_current = NULL;
		}
		value->state = OS_ANSYNC_STATE_NONE;
	}

	lst->os_ansync = NULL;
/*	if(lst->mutex)
		os_mutex_unlock(lst->mutex);*/
	if(lst->ansync_mutex)
		os_mutex_unlock(lst->ansync_mutex);
	return ret;
}

os_ansync_t *os_ansync_current_get(void)
{
	return _m_os_ansync_current;
}

int os_ansync_lock(os_ansync_lst *lst)
{
	if(lst->ansync_mutex)
		os_mutex_lock(lst->ansync_mutex, OS_WAIT_FOREVER);
	if(lst->mutex)
		os_mutex_lock(lst->mutex, OS_WAIT_FOREVER);
	return OK;
}

int os_ansync_unlock(os_ansync_lst *lst)
{
	if(lst->mutex)
		os_mutex_unlock(lst->mutex);
	if(lst->ansync_mutex)
		os_mutex_unlock(lst->ansync_mutex);
	return OK;
}

int os_ansync_show(os_ansync_lst *lst, int (*show)(void *, zpl_char *fmt,...), void *pVoid)
{
	int head = 0;
	zpl_char type[16], value[8], state[8];
	NODE index;
	os_ansync_t *pstNode = NULL;
	if(!os_ansync_lst_chk(lst))
		return ERROR;
	if(lst->mutex)
		os_mutex_lock(lst->mutex, OS_WAIT_FOREVER);
	for(pstNode = (os_ansync_t *)lstFirst(lst->list);
			pstNode != NULL;  pstNode = (os_ansync_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(show)
		{
			if(head == 0)
			{
				head = 1;
				(show)(pVoid, "%-12s %-20s %-8s %-8s\r\n", "------------", "--------------------", "--------", "--------");
				(show)(pVoid, "%-12s %-20s %-8s %-8s\r\n", "TYPE", "ENTRY POINT", "STATE", "FD/TIME");
				(show)(pVoid, "%-12s %-20s %-8s %-8s\r\n", "------------", "--------------------", "--------", "--------");
			}
			os_memset(type, 0, sizeof(type));
			os_memset(value, 0, sizeof(value));
			os_memset(state, 0, sizeof(state));
			switch(pstNode->type)
			{
			//case OS_ANSYNC_NONE:
			//	break;
			case OS_ANSYNC_INPUT:
				os_snprintf(type, sizeof(type), "%s", "READ");
				os_snprintf(value, sizeof(value), "%d", pstNode->fd);
				break;
			case OS_ANSYNC_OUTPUT:
				os_snprintf(type, sizeof(type), "%s", "WRITE");
				os_snprintf(value, sizeof(value), "%d", pstNode->fd);
				break;
			case OS_ANSYNC_TIMER:
				os_snprintf(type, sizeof(type), "%s", "TIMER");
				os_snprintf(value, sizeof(value), "%d", pstNode->interval);
				break;
			case OS_ANSYNC_TIMER_ONCE:
				os_snprintf(type, sizeof(type), "%s", "TIMER ONCE");
				os_snprintf(value, sizeof(value), "%d", pstNode->interval);
				break;
			case OS_ANSYNC_EVENT:
				os_snprintf(type, sizeof(type), "%s", "EVENT");
				os_snprintf(value, sizeof(value), "%s", " ");
				break;
			default:
				os_snprintf(type, sizeof(type), "%s", "UNKNOWN");
				os_snprintf(value, sizeof(value), "%s", " ");
				break;
			}
			if(pstNode->state == OS_ANSYNC_STATE_READY)
				os_snprintf(state, sizeof(state), "%s", "READY");
			else
				os_snprintf(state, sizeof(state), "%s", "WAIT");

			(show)(pVoid, "%-12s %-20s %-8s %-8s\r\n", type, pstNode->entryname, state, value);
		}
	}
	if (lst->mutex)
		os_mutex_unlock(lst->mutex);
	return OK;
}


int os_ansync_main(os_ansync_lst *lst, os_ansync_exe exe)
{
	os_ansync_t *node = NULL;
	//host_waitting_loadconfig();
	while(lst)
	{
		while((node = os_ansync_fetch(lst)))
			os_ansync_execute(lst, node, exe);
	}
	return 0;
}



/*

int os_ansync_test()
{
	int ret = 0;
	os_ansync_lst *lst;
	os_ansync_t *node;
	while(1)
	{
		ret = os_ansync_wait(lst);
		if(ret == 0 || ret > 0)
		{
			while(1)
			{
				node = os_ansync_fetch(lst);
				if(node)
					os_ansync_execute(lst, node);
				else
					break;
			}
		}
	}
}
*/

#ifdef __OS_ANSYNC_DEBUG
void os_ansync_debug_printf(void *fp, zpl_char *func, zpl_uint32 line,  const char *format, ...)
{
	va_list args;
	va_start(args, format);
	fprintf(fp, "%s(%d)",func,line);
	vfprintf(fp, format, args);
	fprintf(fp, "\n");
	fflush(fp);
	va_end(args);
}
#endif
