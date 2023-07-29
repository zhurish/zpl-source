/*
 * zpl_media_buffer.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "zpl_media.h"
#include "zpl_media_internal.h"


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

int zpl_media_channel_skbuffer_frame_put(void *mchannel, ZPL_MEDIA_E type, ZPL_MEDIA_FRAME_DATA_E buffertype, 
	ZPL_VIDEO_FRAME_TYPE_E key, int hwtimetick, char *framedata, int datalen)
{
    zpl_skbuffer_t * bufdata = zpl_skbuffer_create(ZPL_SKBUF_TYPE_MEDIA, zpl_media_getptr(mchannel)->frame_queue, datalen);
    if(bufdata && bufdata->skb_data && bufdata->skb_maxsize >= datalen)
    {
        zpl_media_buffer_header(mchannel, bufdata, type, hwtimetick, datalen);
        zpl_media_buffer_header_framedatatype(bufdata, buffertype);
		zpl_media_buffer_header_frame_type(bufdata, key);

		memcpy(ZPL_SKB_DATA(bufdata), framedata, datalen);
		return zpl_skbqueue_async_enqueue(zpl_media_getptr(mchannel)->frame_queue, bufdata);
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
		return zpl_skbqueue_async_enqueue(zpl_media_getptr(media_channel)->frame_queue, bufdata);
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

