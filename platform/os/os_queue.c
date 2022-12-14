/*
 * msg_queue.c
 *
 *  Created on: Jun 9, 2018
 *      Author: zhurish
 */
#include "zebra.h"
#include "os_sem.h"
#include "os_queue.h"

#include <sys/ipc.h>
#include <sys/msg.h>

static int _os_msgq_id = -1;


int os_msgq_init()
{
	key_t key = ftok(".",100);
	_os_msgq_id = msgget(key,IPC_CREAT|0600);
	return OK;
}

int os_msgq_exit()
{
	if(_os_msgq_id >= 0)
		msgctl(_os_msgq_id, IPC_RMID, NULL);
	return OK;
}

os_queue_t *os_queue_create(int max, int size)
{
	os_queue_t *queue = os_malloc(sizeof(os_queue_t));
	if(queue)
	{
		os_memset(queue, 0, sizeof(os_queue_t));
		queue->max = max;
		queue->size = size;
		queue->sem = os_sem_init();
		queue->mutex = os_mutex_init();
		lstInit (&queue->ulist);
		lstInit (&queue->list);
		return queue;
	}
	return NULL;
}

int os_queue_delete(os_queue_t *queue)
{
	if(!queue || queue->use)
		return ERROR;
	if(queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	lstFree(&queue->ulist);
	lstFree(&queue->list);
	if(queue->sem)
		os_sem_exit(queue->sem);
	if(queue->mutex)
		os_mutex_exit(queue->mutex);
	os_free(queue);
	return OK;
}

int os_queue_name(os_queue_t *queue, char *name)
{
	if(!queue)
		return ERROR;
	if(queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	os_memset(queue->name, 0, OS_QUEUE_NAME_MAX);
	os_memcpy(queue->name, name, MIN(OS_QUEUE_NAME_MAX, os_strlen(name)));
	if(queue->mutex)
		os_mutex_unlock(queue->mutex);
	return OK;
}

int os_queue_send(os_queue_t *queue, char *data, int len, int op)
{
	queue_t *queue_add = NULL;
	if(!queue || (len > queue->size) || !data)
		return ERROR;
	while(lstCount(&queue->list) == queue->max)
	{
		if(op != OS_WAIT_FOREVER)
			break;
	}
	if(queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	queue_add = lstFirst(&queue->ulist);
	if(queue_add == NULL)
	{
		queue_add = os_malloc(sizeof(queue_t));
		if(queue_add)
		{
			os_memset(queue_add, 0, sizeof(queue_t));
			queue_add->data = os_malloc(queue->size);
			if(queue_add->data)
			{
				os_memset(queue_add->data, 0, queue->size);
				os_memcpy(queue_add->data, data, len);
				queue_add->size = len;
			}
			else
			{
				if(queue->mutex)
					os_mutex_unlock(queue->mutex);
				return ERROR;
			}
		}
		else
		{
			if(queue->mutex)
				os_mutex_unlock(queue->mutex);
			return ERROR;
		}
	}
	else
	{
		lstDelete(&queue->ulist, queue_add);
		os_memset(queue_add->data, 0, queue->size);
		os_memcpy(queue_add->data, data, len);
		queue_add->size = len;
	}
	lstAdd (&queue->list, queue_add);
	if(queue->mutex)
		os_mutex_unlock(queue->mutex);
	if(queue->sem)
		os_sem_give(queue->sem);
	return OK;
}


int os_queue_recv(os_queue_t *queue, char *data, int len, int timeout)
{
	int rlen = 0;
	queue_t *queue_add = NULL;
	if(!queue || (len > queue->size) || !data)
		return ERROR;
	if(queue->sem)
		os_sem_take(queue->sem, timeout);

	if(queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	queue_add = lstFirst(&queue->list);
	if(queue_add == NULL)
	{
		if(queue->mutex)
			os_mutex_unlock(queue->mutex);
		return ERROR;
	}
	else
	{
		lstDelete(&queue->list, queue_add);
		rlen = MIN(len, queue_add->size);
		os_memcpy(data, queue_add->data, rlen);
	}
	lstAdd (&queue->ulist, queue_add);
	if(queue->mutex)
		os_mutex_unlock(queue->mutex);
	return rlen;
}

