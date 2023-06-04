/*
 * msg_queue.c
 *
 *  Created on: Jun 9, 2018
 *      Author: zhurish
 */
#include "auto_include.h"
#include "zplos_include.h"

#include <sys/ipc.h>
#include <sys/msg.h>

static zpl_int32 _os_msgq_id = -1;


int os_msgq_init(void)
{
	key_t key = ftok(".",100);
	_os_msgq_id = msgget(key,IPC_CREAT|0600);
	return OK;
}

int os_msgq_exit(void)
{
	if(_os_msgq_id >= 0)
		msgctl(_os_msgq_id, IPC_RMID, NULL);
	return OK;
}


int os_qdata_destroy(os_qdata_t *skbuf)
{
	assert(skbuf);
	if (skbuf)
	{
		if (skbuf->skb_data)
		{
			os_free(skbuf->skb_data);
		}	
		os_free(skbuf);
	}
	return OK;
}

int os_queue_init(os_queue_t *queue, char *name, zpl_uint32 max_num, zpl_bool sem)
{
	if (queue)
	{
		os_memset(queue, 0, sizeof(os_queue_t));
		if(name)
			queue->name = strdup(name);
		if(max_num)
			ZPL_SET_BIT(queue->queue_flag, OS_QDATA_FLAGS_LIMIT_MAX);
		queue->max_num = max_num;
		queue->mutex = os_mutex_name_create(os_name_format("%s-mutex",name));
		if (sem)
		{
			queue->sem = os_sem_name_create(os_name_format("%s-sem",name));
			ZPL_SET_BIT(queue->queue_flag, OS_QDATA_FLAGS_ASYNC);
			queue->sync_wait = OS_WAIT_FOREVER;
		}
		else
		{
			queue->sem = NULL;
			ZPL_CLR_BIT(queue->queue_flag, OS_QDATA_FLAGS_ASYNC);
		}
		lstInitFree(&queue->ulist, os_qdata_destroy);
		lstInitFree(&queue->list, os_qdata_destroy);
		return OK;
	}
	return ERROR;
}
os_queue_t *os_queue_create(char *name, zpl_uint32 max_num, zpl_bool sem)
{
	os_queue_t *queue = os_malloc(sizeof(os_queue_t));
	if (queue)
	{
		os_memset(queue, 0, sizeof(os_queue_t));
		if(name)
			queue->name = strdup(name);
		if(max_num)
			ZPL_SET_BIT(queue->queue_flag, OS_QDATA_FLAGS_LIMIT_MAX);
		queue->max_num = max_num;
		queue->mutex = os_mutex_name_create(os_name_format("%s-mutex",name));
		if (sem)
		{
			queue->sem = os_sem_name_create(os_name_format("%s-sem",name));
			ZPL_SET_BIT(queue->queue_flag, OS_QDATA_FLAGS_ASYNC);
			queue->sync_wait = OS_WAIT_FOREVER;
		}
		else
		{
			queue->sem = NULL;
			ZPL_CLR_BIT(queue->queue_flag, OS_QDATA_FLAGS_ASYNC);
		}
		ZPL_SET_BIT(queue->queue_flag, OS_QDATA_FLAGS_NEEDFREE);
		lstInitFree(&queue->ulist, os_qdata_destroy);
		lstInitFree(&queue->list, os_qdata_destroy);
		return queue;
	}
	return NULL;
}

int os_queue_destroy(os_queue_t *queue)
{
	assert(queue);
	if (!queue)
		return ERROR;
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);

	lstFree(&queue->ulist);
	lstFree(&queue->list);
	if (queue->mutex)
		os_mutex_destroy(queue->mutex);
	if (queue->sem)
		os_sem_destroy(queue->sem);
	if (queue->name)
		free(queue->name);
	if(ZPL_TST_BIT(queue->queue_flag, OS_QDATA_FLAGS_NEEDFREE))	
		os_free(queue);
	return OK;
}

int os_queue_flush(os_queue_t *queue)
{
	NODE node;
	assert(queue);
	os_qdata_t *skbuf;
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);

	for (skbuf = (os_qdata_t *)lstFirst(&queue->list); skbuf != NULL;
		 skbuf = (os_qdata_t *)lstNext(&node))
	{
		node = skbuf->node;
		if (skbuf && skbuf->skb_len)
		{
			lstDelete(&queue->list, (NODE *)skbuf);
			lstAdd(&queue->ulist, (NODE *)skbuf);
		}
	}
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	return OK;
}
int os_queue_attribute_set(os_queue_t *queue, zpl_int32 attr)
{
	assert(queue);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	if(attr == OS_QDATA_FLAGS_ASYNC)
	{	
		if(queue->sem == NULL)
			queue->sem = os_sem_name_create(os_name_format("%s-sem",queue->name));
	}
	queue->queue_flag |= attr;
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	return OK;
}

int os_queue_attribute_unset(os_queue_t *queue, zpl_int32 attr)
{
	assert(queue);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	if(attr == OS_QDATA_FLAGS_ASYNC)
	{	
		if (queue->sem)
			os_sem_destroy(queue->sem);
		queue->sem = NULL;
	}
	queue->queue_flag &= ~attr;
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	return OK;
}

int os_queue_attribute_get(os_queue_t *queue)
{
	int queue_flag = 0;
	assert(queue);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	queue_flag = queue->queue_flag;
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	return queue_flag;
}

int os_queue_set_privatedata(os_queue_t *queue, zpl_void *privatedata)
{
	assert(queue);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	queue->privatedata = privatedata;
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	return OK;
}

zpl_void *os_queue_get_privatedata(os_queue_t *queue)
{
	zpl_void *privatedata = NULL;
	assert(queue);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	privatedata = queue->privatedata;
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	return privatedata;
}



int os_queue_async_enqueue(os_queue_t *queue, os_qdata_t *skbuf)
{
	assert(queue);
	assert(skbuf);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	if(ZPL_TST_BIT(queue->queue_flag, OS_QDATA_FLAGS_LIMIT_MAX))
	{
		os_qdata_t *get_skbuf = NULL;
		if(lstCount(&queue->list) >= queue->max_num)
		{
			get_skbuf = lstFirst(&queue->list);
			if (get_skbuf)
			{
				lstDelete(&queue->list, (NODE *)get_skbuf);
				lstAdd(&queue->ulist, (NODE *)get_skbuf);
			}
		}
	}
	lstAdd(&queue->list, skbuf);
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	if (ZPL_TST_BIT(queue->queue_flag, OS_QDATA_FLAGS_ASYNC) && queue->sem)
		os_sem_give(queue->sem);
	return OK;
}

os_qdata_t *os_queue_async_wait_dequeue(os_queue_t *queue, int waitms)
{
	os_qdata_t *skbuf = NULL;
	assert(queue);
	if (ZPL_TST_BIT(queue->queue_flag, OS_QDATA_FLAGS_ASYNC) && queue->sem)
		os_sem_take(queue->sem, waitms);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	if (lstCount(&queue->list))
	{
		skbuf = lstFirst(&queue->list);
		if (skbuf)
			lstDelete(&queue->list, (NODE *)skbuf);
	}
	//if(skbuf)
	//__sync_fetch_and_add(skbuf->atomic, 1);
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	return skbuf;
}

os_qdata_t *os_queue_async_dequeue(os_queue_t *queue)
{
	return os_queue_async_wait_dequeue(queue, OS_WAIT_FOREVER);
}


int os_queue_add(os_queue_t *queue, os_qdata_t *skbuf)
{
	assert(queue);
	assert(skbuf);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	if(ZPL_TST_BIT(queue->queue_flag, OS_QDATA_FLAGS_LIMIT_MAX))
	{
		os_qdata_t *get_skbuf = NULL;
		if(lstCount(&queue->list) >= queue->max_num)
		{
			get_skbuf = lstFirst(&queue->list);
			if (get_skbuf)
			{
				lstDelete(&queue->list, (NODE *)get_skbuf);
				lstAdd(&queue->ulist, (NODE *)get_skbuf);
			}
		}
	}
	lstAdd(&queue->list, skbuf);
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	return OK;
}

int os_queue_insert(os_queue_t *queue, os_qdata_t *skbufp, os_qdata_t *skbuf)
{
	assert(queue);
	assert(skbuf);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	lstInsert(&queue->list, skbufp, skbuf);
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	return OK;
}

int os_queue_insert_befor(os_queue_t *queue, os_qdata_t *skbufp, os_qdata_t *skbuf)
{
	assert(queue);
	assert(skbuf);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	lstInsertBefore(&queue->list, skbufp, skbuf);
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	return OK;
}

os_qdata_t *os_queue_get(os_queue_t *queue)
{
	os_qdata_t *skbuf = NULL;
	assert(queue);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	if (lstCount(&queue->list))
	{
		skbuf = lstFirst(&queue->list);
		if (skbuf)
			lstDelete(&queue->list, (NODE *)skbuf);
	}
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	return skbuf;
}

int os_queue_finsh(os_queue_t *queue, os_qdata_t *skbuf)
{
	assert(queue);
	assert(skbuf);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	lstAdd(&queue->ulist, skbuf);
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	return OK;
}




int os_queue_async_wait_distribute(os_queue_t *queue, int sync_wait_ms, int(*func)(os_qdata_t*, void *), void *p)
{
	NODE node;
	os_qdata_t *skbuf = NULL;
	assert(queue);

	if(lstCount (&queue->list) == 0)
		return ERROR;
	if(ZPL_TST_BIT(queue->queue_flag, OS_QDATA_FLAGS_ASYNC))
	{
		if (queue->sem)
			os_sem_take(queue->sem, sync_wait_ms);
	}
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);

	for (skbuf = (os_qdata_t *)lstFirst(&queue->list); skbuf != NULL;
		 skbuf = (os_qdata_t *)lstNext(&node))
	{
		node = skbuf->node;
		if (skbuf && skbuf->skb_len)
		{
			lstDelete(&queue->list, (NODE *)skbuf);
			(func)(skbuf, p);
			lstAdd(&queue->ulist, (NODE *)skbuf);
		}
	}
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	return OK;
}

int os_queue_distribute(os_queue_t *queue, int(*func)(os_qdata_t*, void *), void *p)
{
	return os_queue_async_wait_distribute(queue, OS_WAIT_FOREVER, func, p);
}

int os_qdata_init_default(os_qdata_t *skbuf, int maxlen)
{
	if(maxlen)
		skbuf->skb_maxsize = (maxlen); //buffer 的长度
	skbuf->skb_start = OS_QDATA_START_OFFSET;
	skbuf->skb_len = 0;
	skbuf->res = skbuf->res1 = skbuf->res2 = 0;
	skbuf->skb_timetick = 0;
	return OK;
}

static os_qdata_t *os_qdata_create_raw(os_queue_t *queue, zpl_uint32 len)
{
	NODE node;
	os_qdata_t *skbuf = NULL;
	if(queue)
	{
		assert(queue);
		if (queue->mutex)
			os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
		if(ZPL_TST_BIT(queue->queue_flag, OS_QDATA_FLAGS_LIMIT_MAX))
		{	
			if (lstCount(&queue->list) >= queue->max_num)
			{
				if (queue->mutex)
					os_mutex_unlock(queue->mutex);
				return NULL;
			}
		}
		for (skbuf = (os_qdata_t *)lstFirst(&queue->ulist); skbuf != NULL;
			skbuf = (os_qdata_t *)lstNext(&node))
		{
			node = skbuf->node;
			if (skbuf && skbuf->skb_maxsize >= (len))
			{
				lstDelete(&queue->ulist, (NODE *)skbuf);
				os_qdata_init_default(skbuf, 0);
				break;
			}
		}
	}
	if (skbuf == NULL)
	{
		skbuf = os_malloc(sizeof(os_qdata_t));
		if (skbuf)
		{
			memset(skbuf, 0, sizeof(os_qdata_t));
			os_qdata_init_default(skbuf, len);
			skbuf->skb_data = os_malloc(skbuf->skb_maxsize);				//buffer
			if (skbuf->skb_data == NULL)
			{
				skbuf->skb_maxsize = 0;
				free(skbuf);
				skbuf = NULL;
			}
		}
	}
	if (queue && queue->mutex)
		os_mutex_unlock(queue->mutex);
	return skbuf;
}

os_qdata_t *os_qdata_create(os_queue_t *queue, zpl_uint32 len)
{
	os_qdata_t *skbuf = os_qdata_create_raw(queue, OS_QDATA_SIZE_ALIGN(len));
	return skbuf;
}

os_qdata_t *os_qdata_clone(os_queue_t *queue, os_qdata_t *skbuf)
{
	os_qdata_t *skbuftmp = NULL;
	skbuftmp = os_qdata_create_raw(queue, skbuf->skb_maxsize);
	if (skbuftmp)
	{
		skbuftmp->skb_timetick = skbuf->skb_timetick; //时间戳 毫秒
		skbuftmp->skb_len = skbuf->skb_len;			//当前缓存帧的长度
		skbuftmp->skb_maxsize = skbuf->skb_maxsize; //buffer 的长度
		skbuftmp->skb_start = skbuf->skb_start;
		skbuftmp->b_rptr = skbuf->b_rptr;
		skbuftmp->b_wptr = skbuf->b_wptr;

		memset(skbuftmp->skb_data, 0, skbuftmp->skb_maxsize);
		memcpy(skbuftmp->skb_data + skbuftmp->skb_start, skbuf->skb_data + skbuf->skb_start, skbuf->skb_len);
		return skbuftmp;
	}
	return NULL;
}

/* 报文前面添加数据 */
int os_qdata_push(os_qdata_t *skbuf, uint32_t offset, uint8_t *data, uint32_t len)
{
	if(skbuf->skb_start > 0 && skbuf->skb_start >= len)
	{
		if(offset == 0)
		{
			skbuf->skb_start -= len;
			skbuf->skb_len += len;
			skbuf->b_wptr = skbuf->skb_data + skbuf->skb_start;
			if(data)
				memcpy(skbuf->skb_data + skbuf->skb_start, data, len);
		}
		else
		{
			memmove(skbuf->skb_data + skbuf->skb_start - len, skbuf->skb_data + skbuf->skb_start, offset);
			skbuf->skb_start -= len;
			skbuf->skb_len += len;	
			skbuf->b_wptr = skbuf->skb_data + skbuf->skb_start;	
			if(data)
				memcpy(skbuf->skb_data + skbuf->skb_start, data, len);
		}
		return  len;
	}
	return ERROR;
}

/* 报文前面删除数据 */
int os_qdata_pull(os_qdata_t *skbuf, uint32_t offset, uint32_t len)
{
	uint8_t *data = NULL;
	if(skbuf->skb_start > 0 && skbuf->skb_len >= len)
	{
		data = skbuf->skb_data + (skbuf->skb_start + offset);
		if(offset)
		{
			memcpy(data, skbuf->skb_data + skbuf->skb_start, len);
		}
		skbuf->skb_start += len;
		skbuf->skb_len -= len;
		skbuf->b_wptr = skbuf->skb_data + skbuf->skb_start;
		//skbuf->b_rptr = skbuf->b_rptr;
		return  len;
	}
	return ERROR;
}


int os_qdata_put(os_qdata_t *skbuf, uint8_t *data, uint32_t len)
{
	if (len <= 0)
		return OK;
	if (skbuf->skb_data == NULL)
	{
		skbuf->skb_maxsize = OS_QDATA_SIZE_ALIGN(len);
		skbuf->skb_data = malloc(skbuf->skb_maxsize);
		skbuf->skb_start = OS_QDATA_START_OFFSET;
		skbuf->skb_len = 0;
		skbuf->res = skbuf->res1 = skbuf->res2 = 0;
		skbuf->skb_timetick = 0;
		skbuf->b_wptr = skbuf->skb_data + skbuf->skb_start;
		//skbuf->b_rptr = skbuf->skb_data + skbuf->skb_start;
		os_qdata_init_default(skbuf, OS_QDATA_SIZE_ALIGN(len));
	}
	else
	{
		if (skbuf->skb_maxsize < (skbuf->skb_len + OS_QDATA_ALIGN(len)))
		{
			zpl_uint32 skb_maxsize = skbuf->skb_maxsize + OS_QDATA_ALIGN(len);
			skbuf->skb_data = realloc(skbuf->skb_data, skb_maxsize);
			if (skbuf->skb_data)
				skbuf->skb_maxsize = skb_maxsize;
		}
	}

	if (skbuf->skb_data != NULL && (skbuf->skb_len + len) < skbuf->skb_maxsize)
	{
		if(data)
			memcpy(skbuf->skb_data + skbuf->skb_start + skbuf->skb_len, data, len);
		skbuf->b_wptr = skbuf->skb_data + skbuf->skb_start;
		//skbuf->b_rptr = skbuf->skb_data + skbuf->skb_start;	
		skbuf->skb_len += len;
		return skbuf->skb_len;
	}
	return OK;
}