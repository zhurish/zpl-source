/*
 * zpl_media_buffer.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_MEDIA_BUFFER_H__
#define __ZPL_MEDIA_BUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif



#define ZPL_MEDIA_BUFFER_DEBUG_ONFILE
//epoll_dispatch
//dispatch
//Scheduler
#define ZPL_MEDIA_BUFFER_FRAME_MAXSIZE		(1920*1080*3)//(BMP位图)
#define ZPL_MEDIA_BUFFER_FRAME_CACHESIZE	(60)//(临时缓存16帧)
#define ZPL_MEDIA_BUFQUEUE_SIZE	            (60)//(临时缓存16帧)


typedef struct zpl_media_bufcache_s
{
    uint8_t     *data;
    int32_t    len;
    uint32_t    maxsize;
}zpl_media_bufcache_t, zpl_media_frame_t;


int zpl_media_bufcache_resize(zpl_media_bufcache_t * bufdata, int datalen);
int zpl_media_bufcache_add(zpl_media_bufcache_t * bufdata, char *data, int datalen);
int zpl_media_bufcache_create(zpl_media_bufcache_t * bufdata, int maxlen);
int zpl_media_bufcache_destroy(zpl_media_bufcache_t * bufdata);

int zpl_media_buffer_header(void *channel, zpl_skbuffer_t * bufdata, ZPL_MEDIA_E type, int timetick, int datalen);

int zpl_media_buffer_header_frame_type(zpl_skbuffer_t * bufdata, ZPL_VIDEO_FRAME_TYPE_E frame_type);
int zpl_media_buffer_header_framedatatype(zpl_skbuffer_t * bufdata, ZPL_MEDIA_FRAME_DATA_E buffertype);
int zpl_media_buffer_header_channel(zpl_skbuffer_t * bufdata, void *channel);

int zpl_media_channel_extradata_update(zpl_skbuffer_t * bufdata, void *channel);

int zpl_media_channel_skbuffer_frame_put(void *mchannel, ZPL_MEDIA_E type, ZPL_MEDIA_FRAME_DATA_E buffertype, 
	ZPL_VIDEO_FRAME_TYPE_E key, int hwtimetick, char *framedata, int datalen);
    
int zpl_media_channel_extradata_skbuffer_repush(void *mchannel, ZPL_VIDEO_FRAME_TYPE_E key, char *framedata, int datalen);


extern char *zpl_media_timerstring(void);
extern zpl_uint32 zpl_media_timerstamp(void);
extern void zpl_media_msleep(zpl_uint32 msec);



#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_BUFFER_H__ */
