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

#include <zpl_type.h>
#include <zpl_media.h>


#define ZPL_MEDIA_BUFFER_DEBUG_ONFILE
//epoll_dispatch
//dispatch
//Scheduler
#define ZPL_MEDIA_BUFFER_FRAME_MAXSIZE		(1920*1080*3)//(BMP位图)
#define ZPL_MEDIA_BUFFER_FRAME_CACHESIZE	(16)//(临时缓存2帧)
#define ZPL_MEDIA_BUFQUEUE_SIZE	(2)//(临时缓存2帧)
typedef enum
{
    ZPL_BUFFER_DATA_ENCODE      = 0x00,           //编码后
    ZPL_BUFFER_DATA_RECORD      = 0x01,			  //录像
    ZPL_BUFFER_DATA_CAPTURE     = 0x02,           //抓拍
    ZPL_BUFFER_DATA_YUV420      = 0x04,		      //YUV输入
    ZPL_BUFFER_DATA_YUV422      = 0x08,		      //
    ZPL_BUFFER_DATA_BMP         = 0x10,           //
} ZPL_BUFFER_DATA_E;


typedef struct zpl_media_bufcache_s
{
    uint8_t     *data;
    int32_t    len;
    uint32_t    maxsize;
}zpl_media_bufcache_t, zpl_media_frame_t;

typedef struct 
{
    zpl_skbqueue_t  *media_queue;
    zpl_taskid_t taskid;
}zpl_media_bufqueue_t;

int zpl_media_bufcache_resize(zpl_media_bufcache_t * bufdata, int datalen);
int zpl_media_bufcache_add(zpl_media_bufcache_t * bufdata, char *data, int datalen);
int zpl_media_bufcache_create(zpl_media_bufcache_t * bufdata, int maxlen);
int zpl_media_bufcache_destroy(zpl_media_bufcache_t * bufdata);

int zpl_media_buffer_header(zpl_skbuffer_t * bufdata, int type,int flag, int timetick, int datalen);
int zpl_media_buffer_header_channel_key(zpl_skbuffer_t * bufdata, void *channel, int key);

int zpl_media_channel_extradata_update(zpl_skbuffer_t * bufdata, void *channel);


extern char *zpl_media_timerstring(void);
extern zpl_uint32 zpl_media_timerstamp(void);
extern void zpl_media_msleep(zpl_uint32 msec);

zpl_skbqueue_t * zpl_media_bufqueue_get(void);
int zpl_media_bufqueue_init(void);
int zpl_media_bufqueue_exit(void);
int zpl_media_bufqueue_start(void);

#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_BUFFER_H__ */
