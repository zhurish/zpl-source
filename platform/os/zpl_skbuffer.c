/*
 * zpl_skbuffer.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"




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
	//if(bufdata)
	//__sync_fetch_and_add(bufdata->atomic, 1);
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



static zpl_skb_data_t *zpl_skb_data_malloc_raw(zpl_skb_queue_t *queue, zpl_uint32 len)
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
		if (bufdata && bufdata->skb_maxsize >= (len))
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

			bufdata->skb_maxsize = (len); //buffer 的长度
			bufdata->skb_data = os_malloc(bufdata->skb_maxsize);				//buffer
			bufdata->skb_start = ZPL_SKB_START_OFFSET;
			if (bufdata->skb_data == NULL)
			{
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

zpl_skb_data_t *zpl_skb_data_malloc(zpl_skb_queue_t *queue, zpl_uint32 len)
{
	zpl_skb_data_t *bufdata = zpl_skb_data_malloc_raw(queue, ZPL_SKSIZE_ALIGN(len));
	return bufdata;
}

zpl_skb_data_t *zpl_skb_data_clone(zpl_skb_queue_t *queue, zpl_skb_data_t *data)
{
	zpl_skb_data_t *bufdata = NULL;
	bufdata = zpl_skb_data_malloc_raw(queue, data->skb_maxsize);
	if (bufdata)
	{
		memcpy(&bufdata->skb_header, &data->skb_header, sizeof(data->skb_header));
		bufdata->skb_timetick = data->skb_timetick; //时间戳 毫秒
		bufdata->skb_len = data->skb_len;			//当前缓存帧的长度
		bufdata->skb_maxsize = data->skb_maxsize; //buffer 的长度
		memset(bufdata->skb_data, 0, bufdata->skb_maxsize);
		memcpy(bufdata->skb_data, data->skb_data, data->skb_len);
		return bufdata;
	}
	return NULL;
}


int zpl_skb_data_push(zpl_skb_data_t *bufdata, uint8_t *data, uint32_t len)
{
	if(bufdata->skb_start > 0 && bufdata->skb_start >= len)
	{
		bufdata->skb_start -= len;
		bufdata->skb_len += len;
		if(data)
			memcpy(bufdata->skb_data + bufdata->skb_start, data, len);
		return  len;
	}
	return -1;
}

int zpl_skb_data_pull(zpl_skb_data_t *bufdata, uint8_t *data, uint32_t len)
{
	if(bufdata->skb_start > 0 && bufdata->skb_len >= len)
	{
		if(data)
		{
			memcpy(data, bufdata->skb_data + bufdata->skb_start, len);
		}
		bufdata->skb_start += len;
		bufdata->skb_len -= len;
		return  len;
	}
	return -1;
}



int zpl_skb_data_append(zpl_skb_data_t *bufdata, uint8_t *data, uint32_t len)
{
	if (len <= 0)
		return 0;
	if (bufdata->skb_data == NULL)
	{
		bufdata->skb_maxsize = ZPL_SKSIZE_ALIGN(len);
		bufdata->skb_data = malloc(bufdata->skb_maxsize);
		bufdata->skb_start = ZPL_SKB_START_OFFSET;
	}
	else
	{
		if (bufdata->skb_maxsize < (bufdata->skb_len + ZPL_SKBUF_ALIGN(len)))
		{
			zpl_uint32 skb_maxsize = bufdata->skb_maxsize + ZPL_SKBUF_ALIGN(len);
			bufdata->skb_data = realloc(bufdata->skb_data, skb_maxsize);
			if (bufdata->skb_data)
				bufdata->skb_maxsize = skb_maxsize;
		}
	}

	if (bufdata->skb_data != NULL && bufdata->skb_len > 0)
	{
		memcpy(bufdata->skb_data + bufdata->skb_len, data, len);
		bufdata->skb_len += len;
		return bufdata->skb_len;
	}
	return 0;
}

int zpl_skb_data_net_header(zpl_skb_data_t *bufdata, zpl_netpkt_hdr_t *data)
{
	memcpy(&bufdata->skb_header.net_header, data, sizeof(zpl_netpkt_hdr_t));
	return sizeof(zpl_netpkt_hdr_t);
}

/* net pkt */
zpl_netpkt_hdr_t * zpl_skb_netpkt_hdrget(zpl_skb_data_t *src)
{
	zpl_netpkt_hdr_t *netpkt = &src->skb_header.net_header;
	return netpkt;
}


