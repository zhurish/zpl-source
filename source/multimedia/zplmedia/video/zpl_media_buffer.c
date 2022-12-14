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


static zpl_media_bufqueue_t _media_bufqueue;


char *zpl_media_timerstring(void)
{
	static char ttdata[64];
	struct timeval tv;
	gettimeofday(&tv, NULL);
	memset(ttdata, 0, sizeof(ttdata));
	sprintf(ttdata, "%i.%i", tv.tv_sec, tv.tv_usec / 1000);
	return ttdata;
}

zpl_uint32 zpl_media_timerstamp(void)
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

int zpl_media_bufcache_resize(zpl_media_bufcache_t * bufdata, int datalen)
{
	if (datalen <= 0)
		return OK;
	if (bufdata->data == NULL)
	{
		zpl_media_bufcache_create(bufdata, datalen);
	}
	else
	{
		if (bufdata->maxsize < (bufdata->len + ZPL_SKBUF_ALIGN(datalen)))
		{
			zpl_uint32 maxsize = bufdata->maxsize + ZPL_SKBUF_ALIGN(datalen);
			bufdata->data = realloc(bufdata->data, maxsize);
			if (bufdata->data)
				bufdata->maxsize = maxsize;
		}
	}
	if (bufdata->data != NULL && (bufdata->len + datalen) < bufdata->maxsize)
	{
		return OK;
	}
	return ERROR;
}

int zpl_media_bufcache_add(zpl_media_bufcache_t * bufdata, char *data, int datalen)
{
	if (datalen <= 0)
		return OK;
	if (bufdata->data == NULL)
	{
		zpl_media_bufcache_create(bufdata, datalen);
	}
	else
	{
		if (bufdata->maxsize < (bufdata->len + ZPL_SKBUF_ALIGN(datalen)))
		{
			zpl_uint32 maxsize = bufdata->maxsize + ZPL_SKBUF_ALIGN(datalen);
			bufdata->data = realloc(bufdata->data, maxsize);
			if (bufdata->data)
				bufdata->maxsize = maxsize;
		}
	}

	if (bufdata->data != NULL && (bufdata->len + datalen) < bufdata->maxsize)
	{
		memcpy(bufdata->data + bufdata->len, data, datalen);
		bufdata->len += datalen;
		return bufdata->len;
	}
	return OK;
}
int zpl_media_bufcache_create(zpl_media_bufcache_t * bufdata, int maxlen)
{
	if(bufdata == NULL)
		return ERROR;
	memset(bufdata, 0, sizeof(zpl_media_bufcache_t));
	bufdata->maxsize = ZPL_SKSIZE_ALIGN(maxlen);
	bufdata->data = malloc(bufdata->maxsize);
	if(bufdata->data)
	{
		memset(bufdata->data, 0, bufdata->maxsize);
		return OK;
	}
	return ERROR;
}

int zpl_media_bufcache_destroy(zpl_media_bufcache_t * bufdata)
{
	if(bufdata && bufdata->data)
	{
		free(bufdata->data);
		bufdata->data = NULL;
		return OK;
	}
	return ERROR;
}


int zpl_media_buffer_header(zpl_skbuffer_t * bufdata, int type,int flag, int timetick, int datalen)
{
    bufdata->skb_header.media_header.buffer_type = type;     //音频视频
    bufdata->skb_header.media_header.buffer_flags = flag;
    bufdata->skb_header.media_header.buffer_timetick = timetick;    //时间戳 毫秒
    //bufdata->skb_header.media_header.buffer_seq  = stStream.u32Seq;                        //序列号 底层序列号
    bufdata->skb_len = datalen;             //当前缓存帧的长度
	return OK;
}

int zpl_media_buffer_header_channel_key(zpl_skbuffer_t * bufdata, void *channel, int key)
{
	zpl_media_channel_t *media_channel = channel;
	bufdata->skb_header.media_header.ID = ZPL_MEDIA_CHANNEL_SET(media_channel->channel, media_channel->channel_index, media_channel->channel_type);
    bufdata->skb_header.media_header.buffer_key = key;    //时间戳 毫秒
	if (bufdata->skb_header.media_header.buffer_type == ZPL_MEDIA_VIDEO)
	{
		bufdata->skb_header.media_header.buffer_codec = media_channel->video_media.codec.enctype;
	}
	else if (bufdata->skb_header.media_header.buffer_type == ZPL_MEDIA_AUDIO)
	{
		bufdata->skb_header.media_header.buffer_codec = media_channel->audio_media.codec.enctype;
	}
	return OK;
}


void * zpl_media_bufqueue_get(void)
{
	return _media_bufqueue.media_queue;
}

int zpl_media_bufqueue_init(void)
{
	memset(&_media_bufqueue, 0, sizeof(zpl_media_bufqueue_t));
	_media_bufqueue.media_queue = zpl_skbqueue_create(os_name_format("mediaBufQueue"), ZPL_MEDIA_BUFQUEUE_SIZE, zpl_true);
	return OK;
}

int zpl_media_bufqueue_exit(void)
{
	if(_media_bufqueue.taskid)
	{
		if(os_task_destroy(_media_bufqueue.taskid)==OK)
			_media_bufqueue.taskid = 0;
	}
    if(_media_bufqueue.media_queue)
    	zpl_skbqueue_destroy(_media_bufqueue.media_queue);
	_media_bufqueue.media_queue = NULL;	
	return OK;
}

static int media_queue_task(void *p)
{
	host_waitting_loadconfig();
	while(OS_TASK_TRUE())
	{
        zpl_skbqueue_distribute(_media_bufqueue.media_queue, zpl_media_client_foreach, NULL);
	}
	return OK;
}

int zpl_media_bufqueue_start(void)
{
    if(_media_bufqueue.taskid == 0)
        _media_bufqueue.taskid = os_task_create("mediaQuaueTask", OS_TASK_DEFAULT_PRIORITY,
	               0, media_queue_task, NULL, OS_TASK_DEFAULT_STACK);
    return OK;               
}