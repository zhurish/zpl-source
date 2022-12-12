/*
 * zpl_media_buffer.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"

char *zpl_media_timerstring()
{
	static char ttdata[64];
	struct timeval tv;
	gettimeofday(&tv, NULL);
	memset(ttdata, 0, sizeof(ttdata));
	sprintf(ttdata, "%i.%i", tv.tv_sec, tv.tv_usec / 1000);
	return ttdata;
}

zpl_uint32 zpl_media_timerstamp()
{
	zpl_uint32 pts = 0u;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	pts = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	return pts;
}

void zpl_media_msleep(zpl_uint32 msec)
{
#ifdef _WIN32
	Sleep(msec);
#else
	usleep((msec * 1000));
#endif
	return;
}

#ifdef ZPL_MEDIA_BUFFER_DEBUG_ONFILE
static int zpl_media_buffer_debug_onfile(zpl_media_buffer_t *queue, zpl_media_buffer_data_t *data);
#endif

int zpl_media_buffer_data_free(zpl_media_buffer_data_t *data)
{
	zpl_video_assert(data);
	if (data)
	{
		if (data->buffer_data)
		{
			os_free(data->buffer_data);
		}
		os_free(data);
	}
	return OK;
}

zpl_media_buffer_t *zpl_media_buffer_create(char *name, zpl_uint32 maxsize, zpl_bool sem, zpl_void *chn)
{
	zpl_media_buffer_t *queue = os_malloc(sizeof(zpl_media_buffer_t));
	if (queue)
	{
		zpl_media_buffer_data_t *dnode = NULL;
		os_memset(queue, 0, sizeof(zpl_media_buffer_t));
		queue->maxsize = maxsize;
		if(name)
			queue->name = strdup(name);
		queue->mutex = os_mutex_name_init(os_name_format("%s-mutex",name));
		if (sem)
			queue->sem = os_sem_name_init(os_name_format("%s-sem",name));
		else
			queue->sem = NULL;
		queue->media_channel = chn;
		if (chn)
		{
			queue->channel = ((zpl_media_channel_t *)chn)->channel;				//通道号
			queue->channel_index = ((zpl_media_channel_t *)chn)->channel_index; //码流类型
		}
		lstInitFree(&queue->ulist, zpl_media_buffer_data_free);
		lstInitFree(&queue->list, zpl_media_buffer_data_free);
		dnode = zpl_media_buffer_data_malloc(queue, ZPL_MEDIA_VIDEO, ZPL_BUFFER_DATA_ENCODE, ZPL_MEDIA_BUFFER_FRAME_MAXSIZE);
		if (dnode)
			lstAdd(&queue->ulist, dnode);
		dnode = zpl_media_buffer_data_malloc(queue, ZPL_MEDIA_VIDEO, ZPL_BUFFER_DATA_ENCODE, ZPL_MEDIA_BUFFER_FRAME_MAXSIZE);
		if (dnode)
			lstAdd(&queue->ulist, dnode);
		return queue;
	}
	return NULL;
}

int zpl_media_buffer_destroy(zpl_media_buffer_t *queue)
{
	zpl_video_assert(queue);
	if (!queue)
		return ERROR;
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
#ifdef ZPL_MEDIA_BUFFER_DEBUG_ONFILE
	zpl_media_buffer_debug_onfile_close(queue);
#endif
	lstFree(&queue->ulist);
	lstFree(&queue->list);
	if (queue->mutex)
		os_mutex_exit(queue->mutex);
	if (queue->sem)
		os_sem_exit(queue->sem);
	if (queue->name)
		free(queue->name);
	os_free(queue);
	return OK;
}

int zpl_media_buffer_finsh(zpl_media_buffer_t *queue, zpl_media_buffer_data_t *data)
{
	zpl_video_assert(queue);
	zpl_video_assert(data);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	lstAdd(&queue->ulist, data);
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	return OK;
}

zpl_media_buffer_data_t *zpl_media_buffer_dequeue(zpl_media_buffer_t *queue)
{
	zpl_media_buffer_data_t *bufdata = NULL;
	zpl_video_assert(queue);
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

zpl_media_buffer_data_t *zpl_media_buffer_data_malloc(zpl_media_buffer_t *queue, ZPL_MEDIA_E type, 
		ZPL_BUFFER_DATA_E flag, zpl_uint32 len)
{
	NODE node;
	zpl_media_buffer_data_t *bufdata = NULL;
	zpl_video_assert(queue);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	if (lstCount(&queue->list) >= queue->maxsize)
	{
		if (queue->mutex)
			os_mutex_unlock(queue->mutex);
		return NULL;
	}
	for (bufdata = (zpl_media_buffer_data_t *)lstFirst(&queue->ulist); bufdata != NULL;
		 bufdata = (zpl_media_buffer_data_t *)lstNext(&node))
	{
		node = bufdata->node;
		if (bufdata && bufdata->buffer_maxsize >= len)
		{
			lstDelete(&queue->ulist, (NODE *)bufdata);
			break;
		}
	}
	if (bufdata == NULL)
	{
		bufdata = os_malloc(sizeof(zpl_media_buffer_data_t));
		if (bufdata)
		{
			memset(bufdata, 0, sizeof(zpl_media_buffer_data_t));
			zpl_media_channel_t *chn = queue->media_channel;
			if (chn)
			{
				bufdata->ID = ZPL_MEDIA_CHANNEL_SET(chn->channel, chn->channel_index, chn->channel_type);
			}

			bufdata->buffer_maxsize = ZPL_MEDIA_BUF_ALIGN(len); //buffer 的长度
			bufdata->buffer_data = os_malloc(len);				//buffer
			bufdata->buffer_type = type;
			bufdata->buffer_flags = flag;
			if (bufdata->buffer_data == NULL)
			{
				memset(bufdata->buffer_data, 0, len);
				bufdata->buffer_maxsize = 0;
				free(bufdata);
				bufdata = NULL;
			}
		}
	}
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	return bufdata;
}

zpl_media_buffer_data_t *zpl_media_buffer_data_clone(zpl_media_buffer_t *queue, zpl_media_buffer_data_t *data)
{
	zpl_media_buffer_data_t *bufdata = NULL;
	bufdata = zpl_media_buffer_data_malloc(queue, data->buffer_type, data->buffer_flags, data->buffer_maxsize);
	if (bufdata)
	{
		bufdata->ID = data->ID;						//ID 通道号
		bufdata->buffer_type = data->buffer_type;	//音频视频
		bufdata->buffer_codec = data->buffer_codec; //编码类型
		bufdata->buffer_key = data->buffer_key;		//帧类型
		bufdata->buffer_flags = data->buffer_flags;

		bufdata->buffer_timetick = data->buffer_timetick; //时间戳 毫秒
		//bufdata->buffer_seq     = data->buffer_seq;         //序列号 底层序列号
		bufdata->buffer_len = data->buffer_len;			//当前缓存帧的长度
		bufdata->buffer_maxsize = data->buffer_maxsize; //buffer 的长度
		memset(bufdata->buffer_data, 0, bufdata->buffer_maxsize);
		memcpy(bufdata->buffer_data, data->buffer_data, data->buffer_len);
		return bufdata;
	}
	return NULL;
}

/*
zpl_media_buffer_data_t * zpl_media_buffer_data_memdup(zpl_media_buffer_data_t *data)
{
    zpl_media_buffer_data_t *bufdata = NULL;
    bufdata = malloc(sizeof(zpl_media_buffer_data_t));
    if(bufdata)
    {
        memset(bufdata, 0, sizeof(zpl_media_buffer_data_t));
        memcpy(bufdata, data, sizeof(zpl_media_buffer_data_t));

        bufdata->buffer_data = malloc(bufdata->buffer_maxsize);	//buffer
        if(bufdata->buffer_data == NULL)
        {
            free(bufdata);
            bufdata = NULL;
            return bufdata;
        }
        memset(bufdata->buffer_data, 0, bufdata->buffer_maxsize);
        memcpy(bufdata->buffer_data, data->buffer_data, data->buffer_len);
        return bufdata;
    }
    return NULL;
}
*/

int zpl_media_buffer_data_copy(zpl_media_buffer_data_t *dst, zpl_media_buffer_data_t *src)
{
	if (dst && src && src->buffer_data && src->buffer_len && (src->buffer_len <= src->buffer_maxsize))
	{
		if (dst->buffer_data)
		{
			if (src->buffer_maxsize > dst->buffer_maxsize)
			{
				dst->buffer_data = realloc(dst->buffer_data, src->buffer_maxsize);
				dst->buffer_maxsize = src->buffer_maxsize; //buffer 的长度
			}
		}
		else
		{
			dst->buffer_data = malloc(src->buffer_maxsize); //buffer
			dst->buffer_maxsize = src->buffer_maxsize;		//buffer 的长度
		}

		if (dst->buffer_data == NULL)
		{
			return 0;
		}
		dst->ID = src->ID;					   //ID 通道号
		dst->buffer_type = src->buffer_type;   //音频视频
		dst->buffer_codec = src->buffer_codec; //编码类型
		dst->buffer_key = src->buffer_key;	   //帧类型
		dst->buffer_flags = src->buffer_flags;

		dst->buffer_timetick = src->buffer_timetick; //时间戳 毫秒
		//dst->buffer_seq     = src->buffer_seq;         //序列号 底层序列号
		dst->buffer_len = src->buffer_len; //当前缓存帧的长度

		memset(dst->buffer_data, 0, dst->buffer_maxsize);
		memcpy(dst->buffer_data, src->buffer_data, src->buffer_len);
		return dst->buffer_len;
	}
	return 0;
}

int zpl_media_buffer_enqueue(zpl_media_buffer_t *queue, zpl_media_buffer_data_t *data)
{
	zpl_video_assert(queue);
	zpl_video_assert(data);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	if (queue->media_channel)
	{
		zpl_media_channel_t *chn = queue->media_channel;
		if (data->buffer_type == ZPL_MEDIA_VIDEO)
		{
			data->buffer_codec = chn->video_media.codec.enctype;
		}
		else if (data->buffer_type == ZPL_MEDIA_AUDIO)
		{
			data->buffer_codec = chn->audio_media.codec.enctype;
		}
	}
	lstAdd(&queue->list, data);
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	if (queue->sem)
		os_sem_give(queue->sem);
	return OK;
}

int zpl_media_buffer_data_append(zpl_media_buffer_data_t *bufdata, uint8_t *data, uint32_t len)
{
	if (len <= 0)
		return 0;
	if (bufdata->buffer_data == NULL)
	{
		bufdata->buffer_data = malloc(len);
	}
	else
	{
		if (bufdata->buffer_maxsize < (bufdata->buffer_len + len))
		{
			bufdata->buffer_data = realloc(bufdata->buffer_data, bufdata->buffer_len + len);
			if (bufdata->buffer_data)
				bufdata->buffer_maxsize = bufdata->buffer_len + len;
		}
	}

	if (bufdata->buffer_data != NULL && bufdata->buffer_len >= 0)
	{
		memcpy(bufdata->buffer_data + bufdata->buffer_len, data, len);
		bufdata->buffer_len += len;
		return bufdata->buffer_len;
	}
	return 0;
}

int zpl_media_buffer_add(zpl_media_buffer_t *queue, zpl_media_buffer_data_t *data)
{
	zpl_video_assert(queue);
	zpl_video_assert(data);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	if (queue->media_channel)
	{
		zpl_media_channel_t *chn = queue->media_channel;
		if (data->buffer_type == ZPL_MEDIA_VIDEO)
		{
			data->buffer_codec = chn->video_media.codec.enctype;
		}
		else if (data->buffer_type == ZPL_MEDIA_AUDIO)
		{
			data->buffer_codec = chn->audio_media.codec.enctype;
		}
	}
	lstAdd(&queue->list, data);
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	return OK;
}

int zpl_media_buffer_put(zpl_media_buffer_t *queue, zpl_media_buffer_data_t *data)
{
	return zpl_media_buffer_add(queue, data);
}

zpl_media_buffer_data_t *zpl_media_buffer_get(zpl_media_buffer_t *queue)
{
	zpl_media_buffer_data_t *bufdata = NULL;
	zpl_video_assert(queue);
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

int zpl_media_buffer_distribute(zpl_media_buffer_t *queue)
{
	NODE node;
	zpl_media_buffer_data_t *bufdata = NULL;
	zpl_video_assert(queue);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);

	for (bufdata = (zpl_media_buffer_data_t *)lstFirst(&queue->list); bufdata != NULL;
		 bufdata = (zpl_media_buffer_data_t *)lstNext(&node))
	{
		node = bufdata->node;
		if (bufdata && bufdata->buffer_len)
		{
			lstDelete(&queue->list, (NODE *)bufdata);
			if (queue->media_channel &&
				(bufdata->buffer_key == ZPL_VIDEO_FRAME_TYPE_SEI ||
				 bufdata->buffer_key == ZPL_VIDEO_FRAME_TYPE_SPS ||
				 bufdata->buffer_key == ZPL_VIDEO_FRAME_TYPE_PPS ||
				 bufdata->buffer_key == ZPL_VIDEO_FRAME_TYPE_VPS ||
				 bufdata->buffer_key == ZPL_VIDEO_FRAME_TYPE_ISLICE))
				zpl_media_channel_extradata_import(queue->media_channel, bufdata->buffer_data, bufdata->buffer_len);
			
			if(ZPL_MEDIA_DEBUG(ENCODE, BUFDETAIL))
			{
				;//zpl_bufdata_detail_debug(bufdata->buffer_data, MIN(bufdata->buffer_len,128));
			}
#ifdef ZPL_MEDIA_BUFFER_DEBUG_ONFILE
			if(ZPL_MEDIA_DEBUG(ENCODE, BUFFILE))
			{
				zpl_media_buffer_debug_onfile(queue, bufdata);
			}
#endif
			zpl_media_client_foreach(queue, bufdata);

			lstAdd(&queue->ulist, bufdata);
		}
	}
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	return OK;
}

int zpl_media_buffer_add_and_distribute(zpl_media_buffer_t *queue, zpl_media_buffer_data_t *data)
{
	zpl_video_assert(queue);
	zpl_video_assert(data);
	if (queue->mutex)
		os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
	if (queue->media_channel)
	{
		zpl_media_channel_t *chn = queue->media_channel;
		if (data->buffer_type == ZPL_MEDIA_VIDEO)
		{
			data->buffer_codec = chn->video_media.codec.enctype;
		}
		else if (data->buffer_type == ZPL_MEDIA_AUDIO)
		{
			data->buffer_codec = chn->audio_media.codec.enctype;
		}

		if (data->buffer_key == ZPL_VIDEO_FRAME_TYPE_SEI ||
			data->buffer_key == ZPL_VIDEO_FRAME_TYPE_SPS ||
			data->buffer_key == ZPL_VIDEO_FRAME_TYPE_PPS ||
			data->buffer_key == ZPL_VIDEO_FRAME_TYPE_VPS ||
			data->buffer_key == ZPL_VIDEO_FRAME_TYPE_ISLICE)
			zpl_media_channel_extradata_import(queue->media_channel, data->buffer_data, data->buffer_len);
	}
	if(ZPL_MEDIA_DEBUG(ENCODE, BUFDETAIL))
	{
		;//zpl_bufdata_detail_debug(data->buffer_data, MIN(data->buffer_len,128));
	}
#ifdef ZPL_MEDIA_BUFFER_DEBUG_ONFILE
	if(ZPL_MEDIA_DEBUG(ENCODE, BUFFILE))
	{
		zpl_media_buffer_debug_onfile(queue, data);
	}
#endif
	//lstAdd (&queue->list, data);
	zpl_media_client_foreach(queue, data);

	lstAdd(&queue->ulist, data);
	if (queue->mutex)
		os_mutex_unlock(queue->mutex);
	return OK;
}


#ifdef ZPL_MEDIA_BUFFER_DEBUG_ONFILE
static int zpl_media_buffer_debug_onfile(zpl_media_buffer_t *queue, zpl_media_buffer_data_t *data)
{
	if(queue->debug_fp == NULL)
	{
		queue->debug_fp = fopen("/home/debug.h264", "a+");
	}
	if(queue->debug_fp)
	{
		fwrite(data->buffer_data, data->buffer_len, 1, queue->debug_fp);
		fflush(queue->debug_fp);
	}
	return OK;
}

int zpl_media_buffer_debug_onfile_close(zpl_media_buffer_t *queue)
{
	if(queue->debug_fp)
	{
		fflush(queue->debug_fp);
		fclose(queue->debug_fp);
		queue->debug_fp = NULL;
	}
	return OK;
}
#endif