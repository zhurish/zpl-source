/*
 * zpl_skbuffer.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "os_include.h"
#include "zpl_include.h"



zpl_uint32 zpl_skb_timerstamp(void)
{
	zpl_uint32 pts = 0u;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	pts = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	return pts;
}


int zpl_skb_data_free(zpl_skb_data_t *data)
{
	assert(data);
	if (data)
	{
		if (data->skb_data)
		{
			os_free(data->skb_data);
		}
		os_free(data);
	}
	return OK;
}

zpl_skb_queue_t *zpl_skb_queue_create(zpl_uint32 maxsize, zpl_bool sem)
{
	zpl_skb_queue_t *queue = os_malloc(sizeof(zpl_skb_queue_t));
	if (queue)
	{
		os_memset(queue, 0, sizeof(zpl_skb_queue_t));
		queue->maxsize = maxsize;
		queue->mutex = os_mutex_init();
		if (sem)
			queue->sem = os_sem_init();
		else
			queue->sem = NULL;

		lstInitFree(&queue->ulist, zpl_skb_data_free);
		lstInitFree(&queue->list, zpl_skb_data_free);
		return queue;
	}
	return NULL;
}

int zpl_skb_queue_destroy(zpl_skb_queue_t *queue)
{
	assert(queue);
	if (!queue)
		return ERROR;
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);

	lstFree(&queue->ulist);
	lstFree(&queue->list);
	if (queue->mutex)
		os_mutex_exit(queue->mutex);
	if (queue->sem)
		os_sem_exit(queue->sem);
	os_free(queue);
	return OK;
}

int zpl_skb_queue_finsh(zpl_skb_queue_t *queue, zpl_skb_data_t *data)
{
	assert(queue);
	assert(data);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	lstAdd(&queue->ulist, data);
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	return OK;
}

int zpl_skb_queue_enqueue(zpl_skb_queue_t *queue, zpl_skb_data_t *data)
{
	assert(queue);
	assert(data);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);

	lstAdd(&queue->list, data);
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	if (queue->sem)
		os_sem_give(queue->sem);
	return OK;
}

zpl_skb_data_t *zpl_skb_queue_dequeue(zpl_skb_queue_t *queue)
{
	zpl_skb_data_t *bufdata = NULL;
	assert(queue);
	if (queue->sem)
		os_sem_take(queue->sem, OS_WAIT_FOREVER);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	if (lstCount(&queue->list))
	{
		bufdata = lstFirst(&queue->list);
		if (bufdata)
			lstDelete(&queue->list, (NODE *)bufdata);
	}
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	return bufdata;
}

int zpl_skb_queue_add(zpl_skb_queue_t *queue, zpl_skb_data_t *data)
{
	assert(queue);
	assert(data);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);

	lstAdd(&queue->list, data);
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	return OK;
}

int zpl_skb_queue_put(zpl_skb_queue_t *queue, zpl_skb_data_t *data)
{
	return zpl_skb_queue_add(queue, data);
}

zpl_skb_data_t *zpl_skb_queue_get(zpl_skb_queue_t *queue)
{
	zpl_skb_data_t *bufdata = NULL;
	assert(queue);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	if (lstCount(&queue->list))
	{
		bufdata = lstFirst(&queue->list);
		if (bufdata)
			lstDelete(&queue->list, (NODE *)bufdata);
	}
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	return bufdata;
}

int zpl_skb_queue_distribute(zpl_skb_queue_t *queue, int(*func)(zpl_skb_data_t*, void *), void *p)
{
	NODE node;
	zpl_skb_data_t *bufdata = NULL;
	assert(queue);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);

	for (bufdata = (zpl_skb_data_t *)lstFirst(&queue->list); bufdata != NULL;
		 bufdata = (zpl_skb_data_t *)lstNext(&node))
	{
		node = bufdata->node;
		if (bufdata && bufdata->skb_len)
		{
			lstDelete(&queue->list, (NODE *)bufdata);
			(func)(bufdata, p);
			lstAdd(&queue->ulist, bufdata);
		}
	}
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	return OK;
}



zpl_skb_data_t *zpl_skb_data_malloc(zpl_skb_queue_t *queue, zpl_uint32 len)
{
	NODE node;
	zpl_skb_data_t *bufdata = NULL;
	assert(queue);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	if (lstCount(&queue->list) >= queue->maxsize)
	{
		if (queue->mutex)
			os_mutex_unlock(queue->mutex);
		return NULL;
	}
	for (bufdata = (zpl_skb_data_t *)lstFirst(&queue->ulist); bufdata != NULL;
		 bufdata = (zpl_skb_data_t *)lstNext(&node))
	{
		node = bufdata->node;
		if (bufdata && bufdata->skb_maxsize >= len)
		{
			lstDelete(&queue->ulist, (NODE *)bufdata);
			break;
		}
	}
	if (bufdata == NULL)
	{
		bufdata = os_malloc(sizeof(zpl_skb_data_t));
		if (bufdata)
		{
			memset(bufdata, 0, sizeof(zpl_skb_data_t));

			bufdata->skb_maxsize = ZPL_MEDIA_BUF_ALIGN(len); //buffer 的长度
			bufdata->skb_data = os_malloc(len);				//buffer
			bufdata->skb_hdr_len = 0;
			if (bufdata->skb_data == NULL)
			{
				memset(bufdata->skb_data, 0, len);
				bufdata->skb_maxsize = 0;
				free(bufdata);
				bufdata = NULL;
			}
		}
	}
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	return bufdata;
}

zpl_skb_data_t *zpl_skb_data_clone(zpl_skb_queue_t *queue, zpl_skb_data_t *data)
{
	zpl_skb_data_t *bufdata = NULL;
	bufdata = zpl_skb_data_malloc(queue, data->skb_maxsize);
	if (bufdata)
	{
		bufdata->skb_timetick = data->skb_timetick; //时间戳 毫秒
		//bufdata->skb_seq     = data->skb_seq;         //序列号 底层序列号
		bufdata->skb_len = data->skb_len;			//当前缓存帧的长度
		bufdata->skb_maxsize = data->skb_maxsize; //buffer 的长度
		bufdata->skb_hdr_len = data->skb_hdr_len;
		memset(bufdata->skb_data, 0, bufdata->skb_maxsize);
		memcpy(bufdata->skb_data, data->skb_data, data->skb_len);
		return bufdata;
	}
	return NULL;
}


int zpl_skb_data_copy(zpl_skb_data_t *dst, zpl_skb_data_t *src)
{
	if (dst && src && src->skb_data && src->skb_len && (src->skb_len <= src->skb_maxsize))
	{
		if (dst->skb_data)
		{
			if (src->skb_maxsize > dst->skb_maxsize)
			{
				dst->skb_data = realloc(dst->skb_data, src->skb_maxsize);
				dst->skb_maxsize = src->skb_maxsize; //buffer 的长度
			}
		}
		else
		{
			dst->skb_data = malloc(src->skb_maxsize); //buffer
			dst->skb_maxsize = src->skb_maxsize;		//buffer 的长度
		}

		if (dst->skb_data == NULL)
		{
			return 0;
		}

		dst->skb_timetick = src->skb_timetick; //时间戳 毫秒
		//dst->skb_seq     = src->skb_seq;         //序列号 底层序列号
		dst->skb_len = src->skb_len; //当前缓存帧的长度
		dst->skb_hdr_len = src->skb_hdr_len;
		memset(dst->skb_data, 0, dst->skb_maxsize);
		memcpy(dst->skb_data, src->skb_data, src->skb_len);
		return dst->skb_len;
	}
	return 0;
}

int zpl_skb_data_append(zpl_skb_data_t *bufdata, uint8_t *data, uint32_t len)
{
	if (len <= 0)
		return 0;
	if (bufdata->skb_data == NULL)
	{
		bufdata->skb_data = malloc(bufdata->skb_hdr_len + len);
	}
	else
	{
		if (bufdata->skb_maxsize < (bufdata->skb_len + bufdata->skb_hdr_len + len))
		{
			bufdata->skb_data = realloc(bufdata->skb_data, bufdata->skb_len + bufdata->skb_hdr_len + len);
			if (bufdata->skb_data)
				bufdata->skb_maxsize = bufdata->skb_len + bufdata->skb_hdr_len + len;
		}
	}

	if (bufdata->skb_data != NULL && bufdata->skb_len >= 0)
	{
		memcpy(bufdata->skb_data + bufdata->skb_len, data, len);
		bufdata->skb_len += len;
		return bufdata->skb_len;
	}
	return 0;
}

int zpl_skb_data_head(zpl_skb_data_t *bufdata, uint8_t *data, uint32_t len)
{
	if (bufdata->skb_data != NULL && bufdata->skb_len >= len)
	{
		bufdata->skb_hdr_len = len;
		memcpy(bufdata->skb_data, data, len);
		bufdata->skb_len += len;
		return bufdata->skb_hdr_len;
	}
	return 0;
}


