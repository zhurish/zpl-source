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
#define ZPL_MEDIA_BUFFER_FRAME_CACHESIZE	(16)//(临时缓存2帧)
#define ZPL_MEDIA_BUFQUEUE_SIZE	(16)//(临时缓存16帧)


typedef struct zpl_media_bufcache_s
{
    uint8_t     *data;
    int32_t    len;
    uint32_t    maxsize;
}zpl_media_bufcache_t, zpl_media_frame_t;

#ifdef ZPL_MEDIA_QUEUE_DISTPATH
typedef struct 
{
    NODE            node;
    ZPL_MEDIA_CHANNEL_E channel;
    ZPL_MEDIA_CHANNEL_TYPE_E channel_index;
}zpl_media_bufqueue_event_t;

typedef struct 
{
    zpl_skbqueue_t  *media_queue[ZPL_MEDIA_CHANNEL_MAX][ZPL_MEDIA_CHANNEL_TYPE_MAX];
    void	*sem;
    void	*mutex;
    LIST	list;
    LIST	ulist;
    zpl_taskid_t taskid;
}zpl_media_bufqueue_t;
#endif

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

#ifdef ZPL_MEDIA_QUEUE_DISTPATH
zpl_skbqueue_t * zpl_media_bufqueue_get(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index);
int zpl_media_bufqueue_signal(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index);
int zpl_media_bufqueue_init(void);
int zpl_media_bufqueue_exit(void);
int zpl_media_bufqueue_start(void);
#endif

#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_BUFFER_H__ */
