/*
 * zpl_media_buffer.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "zpl_media.h"
#include "zpl_media_internal.h"

#ifdef ZPL_MEDIA_QUEUE_DISTPATH
static zpl_media_bufqueue_t *_media_bufqueue = NULL;
#endif

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

int zpl_media_buffer_header_channel(zpl_skbuffer_t * bufdata, void *channel)
{
	zpl_media_channel_t *media_channel = channel;
    zpl_media_hdr_t *media_header = bufdata->skb_hdr.other_hdr;
    media_header->ID = ZPL_MEDIA_CHANNEL_SET(media_channel->channel, media_channel->channel_index, media_channel->media_type);
	if (media_header->type == ZPL_MEDIA_VIDEO)
	{
		media_header->codectype = media_channel->media_param.video_media.codec.codectype;
	}
	else if (media_header->type == ZPL_MEDIA_AUDIO)
	{
		media_header->codectype = media_channel->media_param.audio_media.codec.codectype;
	}
	return OK;	
}

int zpl_media_buffer_header(void *channel, zpl_skbuffer_t * bufdata, ZPL_MEDIA_E type, int timetick, int datalen)
{
    zpl_media_hdr_t *media_header = bufdata->skb_hdr.other_hdr;
	zpl_media_channel_t *media_channel = channel;
	if(media_channel)
	{
		media_header->ID = ZPL_MEDIA_CHANNEL_SET(media_channel->channel, media_channel->channel_index, media_channel->media_type);
		if (media_header->type == ZPL_MEDIA_VIDEO)
		{
			media_header->codectype = media_channel->media_param.video_media.codec.codectype;
		}
		else if (media_header->type == ZPL_MEDIA_AUDIO)
		{
			media_header->codectype = media_channel->media_param.audio_media.codec.codectype;
		}
	}
    media_header->type = type;     //音频视频
    media_header->timetick = timetick;    //时间戳 毫秒
	media_header->length = datalen;
    bufdata->skb_len = datalen;             //当前缓存帧的长度
	return OK;
}

int zpl_media_buffer_header_framedatatype(zpl_skbuffer_t * bufdata, ZPL_MEDIA_FRAME_DATA_E buffertype)
{
    zpl_media_hdr_t *media_header = bufdata->skb_hdr.other_hdr;
    media_header->buffertype = buffertype;    //时间戳 毫秒
	return OK;
}

int zpl_media_buffer_header_frame_type(zpl_skbuffer_t * bufdata, ZPL_VIDEO_FRAME_TYPE_E key)
{
    zpl_media_hdr_t *media_header = bufdata->skb_hdr.other_hdr;
    media_header->frame_type = key;    //时间戳 毫秒
	return OK;
}

//#define SKBUFF_SPLIT_DEBUG
#ifdef SKBUFF_SPLIT_DEBUG
static int zpl_media_channel_skbuffer_putone(void *mchannel, zpl_skbuffer_t * bufdata, 
	ZPL_VIDEO_FRAME_TYPE_E key, char *framedata, int datalen)
{
	zpl_media_hdr_t *media_header = bufdata->skb_hdr.other_hdr;
    zpl_skbuffer_t * skbtmp = zpl_skbuffer_create(ZPL_SKBUF_TYPE_MEDIA, zpl_media_getptr(mchannel)->frame_queue, datalen);
    if(bufdata && skbtmp && skbtmp->skb_data && skbtmp->skb_maxsize >= datalen)
    {
        zpl_media_buffer_header(mchannel, skbtmp, media_header->type, media_header->timetick, datalen);
        zpl_media_buffer_header_framedatatype(skbtmp, media_header->buffertype);
        memcpy(ZPL_SKB_DATA(skbtmp), framedata, datalen);
		zpl_media_buffer_header_frame_type(skbtmp, key);

		#ifdef ZPL_MEDIA_QUEUE_DISTPATH
		return zpl_skbqueue_async_enqueue(zpl_media_getptr(mchannel)->frame_queue, skbtmp);
		#else
		return zpl_skbqueue_async_enqueue(zpl_media_getptr(mchannel)->frame_queue, skbtmp);
		#endif
	}
	return ERROR;
}

static int zpl_media_channel_skbuff_splitbuf(void *p, zpl_skbuffer_t * bufdata, zpl_uint8 *Buf, int len)
{
    H264_NALU_T nalu;
    zpl_media_channel_t *chn = (zpl_media_channel_t *)p;
    zpl_uint8 *tmpbuf = Buf;
    int data_offset=0, pos = 0;
	ZPL_VIDEO_FRAME_TYPE_E key;
    memset(&nalu, 0, sizeof(H264_NALU_T));
    while(data_offset <(len-4))
    {
        if(zpl_media_channel_isnaluhdr(tmpbuf, &nalu))
        {
            tmpbuf += nalu.hdr_len;
            data_offset += nalu.hdr_len;
        }
        if(nalu.hdr_len)
        {
            pos = zpl_media_channel_get_nextnalu(tmpbuf, (len-4)-data_offset);
            if(pos)
            {
                nalu.len += pos;
				switch (nalu.nal_unit_type)
				{
					case NALU_TYPE_SEI:
					key = ZPL_VIDEO_FRAME_TYPE_SEI;//ZPL_VIDEO_FRAME_TYPE_VPS
					break;
					case NALU_TYPE_SPS:
					key = ZPL_VIDEO_FRAME_TYPE_SPS;
					break;
					case NALU_TYPE_PPS:
					key = ZPL_VIDEO_FRAME_TYPE_PPS;
					break;
					case NALU_TYPE_IDR:
					key = ZPL_VIDEO_FRAME_TYPE_IDRSLICE;
					break;
					default:
					key = 0;
					break;
				}
				zpl_media_channel_skbuffer_putone(chn, bufdata, key, nalu.buf, nalu.len);

                memset(&nalu, 0, sizeof(H264_NALU_T));
                tmpbuf += pos;
                data_offset += pos;
                continue;
            }
            else
            {
                nalu.len += ((len)-data_offset);

				switch (nalu.nal_unit_type)
				{
					case NALU_TYPE_SEI:
					key = ZPL_VIDEO_FRAME_TYPE_SEI;//ZPL_VIDEO_FRAME_TYPE_VPS
					break;
					case NALU_TYPE_SPS:
					key = ZPL_VIDEO_FRAME_TYPE_SPS;
					break;
					case NALU_TYPE_PPS:
					key = ZPL_VIDEO_FRAME_TYPE_PPS;
					break;
					case NALU_TYPE_IDR:
					key = ZPL_VIDEO_FRAME_TYPE_IDRSLICE;
					break;
					default:
					key = 0;
					break;
				}
				zpl_media_channel_skbuffer_putone(chn, bufdata, key, nalu.buf, nalu.len);

                memset(&nalu, 0, sizeof(H264_NALU_T));
                return 0;
            }
        }
        tmpbuf++;
        data_offset++;
    }
    return 0;
}
#endif

int zpl_media_channel_skbuffer_frame_put(void *mchannel, ZPL_MEDIA_E type, ZPL_MEDIA_FRAME_DATA_E buffertype, 
	ZPL_VIDEO_FRAME_TYPE_E key, int hwtimetick, char *framedata, int datalen)
{
    zpl_skbuffer_t * bufdata = zpl_skbuffer_create(ZPL_SKBUF_TYPE_MEDIA, zpl_media_getptr(mchannel)->frame_queue, datalen);
    if(bufdata && bufdata->skb_data && bufdata->skb_maxsize >= datalen)
    {
        zpl_media_buffer_header(mchannel, bufdata, type, hwtimetick, datalen);
        zpl_media_buffer_header_framedatatype(bufdata, buffertype);
		zpl_media_buffer_header_frame_type(bufdata, key);
		#ifdef SKBUFF_SPLIT_DEBUG
		switch(key)
		{
			case ZPL_VIDEO_FRAME_TYPE_SEI:                         /* H264/H265 SEI types */
			case ZPL_VIDEO_FRAME_TYPE_SPS:                         /* H264/H265 SPS types */
			case ZPL_VIDEO_FRAME_TYPE_PPS:                         /* H264/H265 PPS types */
			case ZPL_VIDEO_FRAME_TYPE_VPS:                        /* H265 VPS types */
			//case ZPL_VIDEO_FRAME_TYPE_ISLICE:
			case ZPL_VIDEO_FRAME_TYPE_IDRSLICE:
			zpl_media_channel_skbuff_splitbuf(mchannel, bufdata, framedata, datalen);
			return 0;
			break;
			default:
			memcpy(ZPL_SKB_DATA(bufdata), framedata, datalen);
			break;
		}
		#else
		memcpy(ZPL_SKB_DATA(bufdata), framedata, datalen);
		#endif
		#ifdef ZPL_MEDIA_QUEUE_DISTPATH
		return zpl_skbqueue_async_enqueue(zpl_media_getptr(mchannel)->frame_queue, bufdata);
		#else
		return zpl_skbqueue_async_enqueue(zpl_media_getptr(mchannel)->frame_queue, bufdata);
		#endif
	}
	return ERROR;
}

int zpl_media_channel_extradata_skbuffer_repush(void *mchannel, ZPL_VIDEO_FRAME_TYPE_E key, char *framedata, int datalen)
{
	zpl_media_channel_t *media_channel = mchannel;
    zpl_skbuffer_t * bufdata = NULL;
	if(framedata && datalen)
		bufdata = zpl_skbuffer_create(ZPL_SKBUF_TYPE_MEDIA, zpl_media_getptr(media_channel)->frame_queue, datalen);
    if(bufdata && bufdata->skb_data && bufdata->skb_maxsize >= datalen)
    {
        zpl_media_buffer_header(media_channel, bufdata, ZPL_MEDIA_VIDEO, 0, datalen);
        zpl_media_buffer_header_framedatatype(bufdata, ZPL_MEDIA_FRAME_DATA_ENCODE);
        memcpy(ZPL_SKB_DATA(bufdata), framedata, datalen);
		zpl_media_buffer_header_frame_type(bufdata, key);
		#ifdef ZPL_MEDIA_QUEUE_DISTPATH
		return zpl_skbqueue_async_enqueue(zpl_media_getptr(media_channel)->frame_queue, bufdata);
		#else
		return zpl_skbqueue_async_enqueue(zpl_media_getptr(media_channel)->frame_queue, bufdata);
		#endif
	}
	return ERROR;
}

int zpl_media_channel_extradata_update(zpl_skbuffer_t * bufdata, void *channel)
{
	zpl_media_channel_t *media_channel = channel;
    zpl_media_hdr_t *media_header = bufdata->skb_hdr.other_hdr;
	if (media_header->type == ZPL_MEDIA_VIDEO)
	{
		switch(media_header->frame_type)
		{
    		case ZPL_VIDEO_FRAME_TYPE_SEI:                         /* H264/H265 SEI types */
    		case ZPL_VIDEO_FRAME_TYPE_SPS:                         /* H264/H265 SPS types */
    		case ZPL_VIDEO_FRAME_TYPE_PPS:                         /* H264/H265 PPS types */
    		case ZPL_VIDEO_FRAME_TYPE_VPS:                        /* H265 VPS types */
			case ZPL_VIDEO_FRAME_TYPE_ISLICE:
			case ZPL_VIDEO_FRAME_TYPE_IDRSLICE:
			zpl_media_channel_extradata_import(media_channel, ZPL_SKB_DATA(bufdata), ZPL_SKB_DATA_LEN(bufdata));
			break;
			default:
			break;
		}
	}
	return OK;
}

#ifdef ZPL_MEDIA_QUEUE_DISTPATH
zpl_skbqueue_t * zpl_media_bufqueue_get(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index)
{
	if(_media_bufqueue)
		return _media_bufqueue->media_queue[channel][channel_index];
	return NULL;	
}

int zpl_media_bufqueue_signal(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index)
{
	if(_media_bufqueue == NULL)
		return ERROR;
    zpl_media_bufqueue_event_t *data = NULL;
    if(_media_bufqueue->mutex)
    	os_mutex_lock(_media_bufqueue->mutex, OS_WAIT_FOREVER);
    if(lstCount (&_media_bufqueue->ulist))    
	    data = lstFirst (&_media_bufqueue->ulist);
    if(data == NULL)    
        data = malloc(sizeof(zpl_media_bufqueue_event_t));

    if(data)
    {
        memset(data, 0, sizeof(zpl_media_bufqueue_event_t));
        data->channel = channel;             //模块
        data->channel_index = channel_index;            //
        lstAdd (&_media_bufqueue->list, data);
    	if(_media_bufqueue->mutex)
    		os_mutex_unlock(_media_bufqueue->mutex);
		if (_media_bufqueue->sem)
			os_sem_give(_media_bufqueue->sem);
        return OK;
    }
	return ERROR;	
}

static int zpl_media_bufqueue_free(zpl_media_bufqueue_event_t *data)
{
	if(data)
	{		
        free(data);
	}	
	return OK;
}
int zpl_media_bufqueue_init(void)
{
	int ch = 0, chtype = 0;
	if(_media_bufqueue == NULL)
		_media_bufqueue = malloc(sizeof(zpl_media_bufqueue_t));

	if(_media_bufqueue == NULL)
		return ERROR;	
	memset(_media_bufqueue, 0, sizeof(zpl_media_bufqueue_t));
    _media_bufqueue->mutex = os_mutex_name_create("mediaBufQueue-mutex");	
	_media_bufqueue->sem = os_sem_name_create("mediaBufQueue-sem");
    lstInitFree (&_media_bufqueue->list, zpl_media_bufqueue_free);
	lstInitFree (&_media_bufqueue->ulist, zpl_media_bufqueue_free);
	for(ch = ZPL_MEDIA_CHANNEL_0; ch < ZPL_MEDIA_CHANNEL_MAX; ch++)
	{
		for(chtype = ZPL_MEDIA_CHANNEL_TYPE_MAIN; chtype < ZPL_MEDIA_CHANNEL_TYPE_MAX; chtype++)
		{
			_media_bufqueue->media_queue[ch][chtype] = zpl_skbqueue_create(os_name_format("mediaBufQueue-%d/%d", ch, chtype), ZPL_MEDIA_BUFQUEUE_SIZE, zpl_true);
			if(_media_bufqueue->media_queue[ch][chtype])
				zpl_skbqueue_attribute_set(_media_bufqueue->media_queue[ch][chtype], ZPL_SKBQUEUE_FLAGS_LIMIT_MAX);
		}
	}
	return OK;
}

int zpl_media_bufqueue_exit(void)
{
	int ch = 0, chtype = 0;	
	if(_media_bufqueue == NULL)
		return OK;
    if(_media_bufqueue->mutex)
    	os_mutex_lock(_media_bufqueue->mutex, OS_WAIT_FOREVER);
	if(_media_bufqueue->taskid)
	{
		if(os_task_destroy(_media_bufqueue->taskid)==OK)
			_media_bufqueue->taskid = 0;
	}
	for(ch = ZPL_MEDIA_CHANNEL_0; ch < ZPL_MEDIA_CHANNEL_MAX; ch++)
	{
		for(chtype = ZPL_MEDIA_CHANNEL_TYPE_MAIN; chtype < ZPL_MEDIA_CHANNEL_TYPE_MAX; chtype++)
		{
			if(_media_bufqueue->media_queue[ch][chtype])
				zpl_skbqueue_destroy(_media_bufqueue->media_queue[ch][chtype]);
		}
	}
	lstFree(&_media_bufqueue->list);
    lstFree(&_media_bufqueue->ulist);
    if(_media_bufqueue->mutex)
    	os_mutex_destroy(_media_bufqueue->mutex);
	if (_media_bufqueue->sem)
		os_sem_destroy(_media_bufqueue->sem);
	_media_bufqueue->sem = NULL;	
	free(_media_bufqueue);
	_media_bufqueue = NULL;
	return OK;
}

static int media_queue_task(void *p)
{
	NODE node;
    zpl_media_bufqueue_event_t *eventcb = NULL;
	host_waitting_loadconfig();

	while(_media_bufqueue && OS_TASK_TRUE())
	{
        if(_media_bufqueue->sem)
		    os_sem_take(_media_bufqueue->sem, OS_WAIT_FOREVER);

		if(_media_bufqueue->mutex)
			os_mutex_lock(_media_bufqueue->mutex, OS_WAIT_FOREVER);

		for(eventcb = (zpl_media_bufqueue_event_t *)lstFirst(&_media_bufqueue->list); eventcb != NULL;
			eventcb = (zpl_media_bufqueue_event_t *)lstNext(&node))
		{
			node = eventcb->node;
			if(eventcb) 
			{
				lstDelete(&_media_bufqueue->list, eventcb);
				lstAdd (&_media_bufqueue->ulist, eventcb); 
	
				if(_media_bufqueue->media_queue[eventcb->channel][eventcb->channel_index])
					zpl_skbqueue_async_wait_distribute(_media_bufqueue->media_queue[eventcb->channel][eventcb->channel_index], 
						5,	zpl_media_client_foreach, NULL);
			}
		}
		if(_media_bufqueue->mutex)
			os_mutex_unlock(_media_bufqueue->mutex);		
	}
	return OK;
}

int zpl_media_bufqueue_start(void)
{
    if(_media_bufqueue->taskid == 0)
        _media_bufqueue->taskid = os_task_create("mediaQuaueTask", OS_TASK_DEFAULT_PRIORITY,
	               0, media_queue_task, NULL, OS_TASK_DEFAULT_STACK);
    return OK;               
}
#endif