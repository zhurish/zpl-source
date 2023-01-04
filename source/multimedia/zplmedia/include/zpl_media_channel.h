/*
 * zpl_media_channel.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_MEDIA_CHANNEL_H__
#define __ZPL_MEDIA_CHANNEL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <zpl_type.h>
#include "zpl_media.h"
#include "zpl_media_codec.h"
#include "zpl_media_buffer.h"
#include "zpl_media_area.h"
#include "zpl_media_extradata.h"


#define ZPL_MEDIA_CLIENT_MAX    32



typedef struct zpl_media_channel_s zpl_media_channel_t;

typedef int	(*zpl_media_buffer_handler)(zpl_media_channel_t *, const zpl_skbuffer_t *,  void *);

typedef struct zpl_media_client_s
{
    zpl_bool                enable;
	zpl_media_buffer_handler _pmedia_buffer_handler;
	void					*pVoidUser;
}zpl_media_client_t;

typedef struct zpl_media_video_s
{
    zpl_bool                enable;
    zpl_video_codec_t	    codec;	    //视频编码参数
    zpl_video_extradata_t   extradata;
    zpl_media_area_t        *m_areas[ZPL_MEDIA_AREA_CHANNEL_MAX];
    zpl_void                *halparam;  //通道绑定的硬件资源
}zpl_media_video_t;

typedef struct zpl_media_audio_s
{
    zpl_bool                enable;
    zpl_audio_codec_t	    codec;	    //视频编码参数
    zpl_void                *halparam;          //通道绑定的硬件资源
}zpl_media_audio_t;

typedef struct zpl_media_unit_s
{
    zpl_bool                enable;
    zpl_void                *param;      //
    zpl_uint32              cbid;
}zpl_media_unit_t;


typedef enum 
{
    ZPL_MEDIA_STATE_NONE    = 0x00000000,      //
    ZPL_MEDIA_STATE_INIT    = 0x00000001,      //初始化
    ZPL_MEDIA_STATE_ACTIVE  = 0x00000004,      //使能
    ZPL_MEDIA_STATE_INACTIVE= 0x0000008,       //去使能
} ZPL_MEDIA_STATE_E;

typedef struct zpl_media_channel_s
{
    NODE	                    node;
	zpl_int32                   channel;	        //通道号
	ZPL_MEDIA_CHANNEL_INDEX_E 	channel_index;	    //码流类型
    ZPL_MEDIA_CHANNEL_TYPE_E    channel_type;

    zpl_media_video_t           video_media;
    zpl_media_audio_t           audio_media;

    zpl_skbqueue_t              *frame_queue;      //通道对应的编码数据缓冲区

    ZPL_MEDIA_STATE_E           state;
    zpl_media_client_t          media_client[ZPL_MEDIA_CLIENT_MAX];

    zpl_uint32                  bindcount;      //绑定的数量

    zpl_media_unit_t            p_record;//通道使能录像
    zpl_media_unit_t            p_capture;//通道使能抓拍

    os_mutex_t  *_mutex;
}zpl_media_channel_t;

#define ZPL_MEDIA_CHANNEL_LOCK(m)  if(((zpl_media_channel_t*)m) && ((zpl_media_channel_t*)m)->_mutex) os_mutex_lock(((zpl_media_channel_t*)m)->_mutex, OS_WAIT_FOREVER)
#define ZPL_MEDIA_CHANNEL_UNLOCK(m)  if(((zpl_media_channel_t*)m) && ((zpl_media_channel_t*)m)->_mutex) os_mutex_unlock(((zpl_media_channel_t*)m)->_mutex)


#define zpl_media_channel_gettype(m)    (((zpl_media_channel_t*)m)->channel_type)
#define zpl_media_getptr(m)             (((zpl_media_channel_t*)m))

extern int zpl_media_channel_init(void);
extern int zpl_media_channel_exit(void);

extern int zpl_media_channel_count(void);
extern int zpl_media_channel_load_default(void);
extern int zpl_media_channel_create(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index);
extern int zpl_media_channel_hwdestroy(zpl_media_channel_t *);
extern int zpl_media_channel_destroy(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index);
extern zpl_media_channel_t * zpl_media_channel_lookup(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index);
extern zpl_media_channel_t *zpl_media_channel_lookup_sessionID(zpl_uint32 sessionID);

extern int zpl_media_channel_filecreate(zpl_char *filename, zpl_bool rd);
extern int zpl_media_channel_filedestroy(zpl_char *filename);
extern zpl_media_channel_t * zpl_media_channel_filelookup(zpl_char *filename);
extern int zpl_media_channel_filestart(zpl_media_channel_t *mchannel, bool start);
extern ZPL_MEDIA_STATE_E zpl_media_channel_state(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index);

extern int zpl_media_channel_recvcallback(zpl_media_channel_t *, int (*func)(zpl_media_channel_t*, zpl_void *, uint8_t *, uint32_t *));
extern int zpl_media_channel_sendcallback(zpl_media_channel_t *, int (*func)(zpl_media_channel_t*, zpl_void *, uint8_t *, uint32_t *));


extern int zpl_media_channel_halparam_set(zpl_int32 channel, 
    ZPL_MEDIA_CHANNEL_INDEX_E channel_index, zpl_bool video, void *halparam);

extern int zpl_media_channel_client_add(zpl_media_channel_t *mchannel, zpl_media_buffer_handler cb_handler, void *pUser);
extern int zpl_media_channel_client_del(zpl_media_channel_t *mchannel, zpl_int32 index);


/* 激活通道 -> 创建底层资源 */
extern int zpl_media_channel_active(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index);
/* 开始通道 -> 底层开始 */
extern int zpl_media_channel_start(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index);
/* 暂停通道 -> 底层结束 */
extern int zpl_media_channel_stop(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index);
/* 销毁通道 -> 销毁底层资源 */
extern int zpl_media_channel_inactive(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index);

/* active:1 使能所有 ，:2 去使能所有 :3 开始所有 :4 停止所有 :-1 销毁所有*/
extern int zpl_media_channel_handle_all(zpl_uint32 active);

#ifdef ZPL_SHELL_MODULE
int zpl_media_channel_show(void *pvoid);
#endif

extern int zpl_media_channel_foreach(int (*callback)(zpl_media_channel_t*, zpl_void *obj), zpl_void *obj);
extern int zpl_media_channel_load(zpl_void *obj);


#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_CHANNEL_H__ */
