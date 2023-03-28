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
    zpl_bool                is_use;
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
    zpl_uint32              cbid;       //通道客户端 media_client 回调
    zpl_void                *param;     //子单元私有数据
}zpl_media_unit_t;


typedef enum 
{
    ZPL_MEDIA_STATE_NONE    = 0,      //
    ZPL_MEDIA_STATE_ACTIVE  = 1,      //硬件通道创建并使能
    ZPL_MEDIA_STATE_START   = 2,      //硬件通道创建并开始
    ZPL_MEDIA_STATE_HWBIND   = 3,      //硬件通道创建并开始    
} ZPL_MEDIA_STATE_E;

typedef struct zpl_media_channel_s
{
    NODE	                    node;
	ZPL_MEDIA_CHANNEL_E         channel;	        //通道号
	ZPL_MEDIA_CHANNEL_TYPE_E 	channel_index;	    //码流类型

    ZPL_MEDIA_E                 media_type;
    union
    {
        zpl_media_video_t           video_media;
        zpl_media_audio_t           audio_media;
    }media_param;
    
    zpl_skbqueue_t              *frame_queue;      //通道对应的编码数据缓冲区

    zpl_uint32                  flags;
    zpl_media_client_t          media_client[ZPL_MEDIA_CLIENT_MAX];

    zpl_uint32                  bindcount;      //绑定的数量
    zpl_media_channel_t         *bind_other;    //视频通道绑定的音频通道

    zpl_media_unit_t            p_capture;      //通道使能抓拍
    zpl_media_unit_t            p_record;       //通道使能录像
    zpl_media_unit_t            p_mucast;       //通道多播发送

    zpl_void                    *t_master;
    os_mutex_t                  *_mutex;
}zpl_media_channel_t;

#define ZPL_MEDIA_CHANNEL_LOCK(m)  if(((zpl_media_channel_t*)m) && ((zpl_media_channel_t*)m)->_mutex) os_mutex_lock(((zpl_media_channel_t*)m)->_mutex, OS_WAIT_FOREVER)
#define ZPL_MEDIA_CHANNEL_UNLOCK(m)  if(((zpl_media_channel_t*)m) && ((zpl_media_channel_t*)m)->_mutex) os_mutex_unlock(((zpl_media_channel_t*)m)->_mutex)

#define zm_get_video_encode(m)             (((zpl_media_channel_t*)m)->media_param.video_media.halparam)
#define zm_get_audio_encode(m)             (((zpl_media_channel_t*)m)->media_param.audio_media.halparam)
#define zpl_media_gettype(m)    (((zpl_media_channel_t*)m)->media_type)
#define zpl_media_getptr(m)             (((zpl_media_channel_t*)m))

/*
*
*       hal_input -----------> hal_vpss -----------> hal_venc -----------> hal_hdmi
*                       |
*                       -----> hal_vpss -----------> hal_venc -----------> hal_hdmi
*                       |
*                       -----> hal_vpss -----------> hal_venc 
*                                             |                    
*                                             -----> hal_venc 
*
*
*
*/

extern int zpl_media_channel_init(void);
extern int zpl_media_channel_exit(void);

extern int zpl_media_channel_count(void);
extern int zpl_media_channel_load_default(void);
extern int zpl_media_channel_create(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index);
extern int zpl_media_channel_hal_create(zpl_media_channel_t *chn);
extern int zpl_media_channel_destroy(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index);
extern int zpl_media_channel_hal_destroy(zpl_media_channel_t *chn);


/* 开始通道 -> 底层开始 */
extern int zpl_media_channel_start(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index);
/* 暂停通道 -> 底层结束 */
extern int zpl_media_channel_stop(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index);


extern int zpl_media_channel_halparam_set(ZPL_MEDIA_CHANNEL_E channel, 
    ZPL_MEDIA_CHANNEL_TYPE_E channel_index, void *halparam);
extern int zpl_media_channel_bind_encode_set(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, void *halparam);


extern zpl_media_channel_t * zpl_media_channel_lookup(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index);
extern zpl_media_channel_t *zpl_media_channel_lookup_sessionID(zpl_uint32 sessionID);

extern zpl_media_channel_t * zpl_media_channel_lookup_bind(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index);

extern ZPL_MEDIA_STATE_E zpl_media_channel_state(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index);

extern zpl_bool zpl_media_channel_isvideo(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index);
extern zpl_bool zpl_media_channel_isaudio(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index);
extern int zpl_media_channel_video_codec_get(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_video_codec_t *);
extern int zpl_media_channel_audio_codec_get(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_audio_codec_t *);
extern int zpl_media_channel_bindcount_get(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index);
extern int zpl_media_channel_bindcount_set(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, int addsub);

extern zpl_media_area_t * zpl_media_channel_area_lookup(zpl_media_channel_t *chn, ZPL_MEDIA_AREA_E type, ZPL_MEDIA_OSD_TYPE_E osd_type);
extern int zpl_media_channel_area_add(zpl_media_channel_t *chn, zpl_media_area_t *);
extern int zpl_media_channel_area_del(zpl_media_channel_t *chn, ZPL_MEDIA_AREA_E type, ZPL_MEDIA_OSD_TYPE_E osd_type);

extern int zpl_media_channel_client_add(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_media_buffer_handler cb_handler, void *pUser);
extern int zpl_media_channel_client_del(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_int32 index);
extern int zpl_media_channel_client_start(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_int32 index, zpl_bool start);


extern int zpl_media_channel_codec_set(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, ZPL_MEDIA_CODEC_E codec);
extern int zpl_media_channel_codec_get(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, ZPL_MEDIA_CODEC_E *codec);
extern int zpl_media_channel_hal_request_IDR(zpl_media_channel_t *chn);
/*分辨率*/
extern int zpl_media_channel_video_resolving_set(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, ZPL_VIDEO_FORMAT_E val);
extern int zpl_media_channel_video_resolving_get(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, ZPL_VIDEO_FORMAT_E *val);

/*帧率*/
extern int zpl_media_channel_framerate_set(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_uint32 framerate);
extern int zpl_media_channel_framerate_get(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_uint32 *framerate);

/*码率*/
extern int zpl_media_channel_bitrate_set(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, ZPL_BIT_RATE_E type, zpl_uint32 bitrate);
extern int zpl_media_channel_bitrate_get(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, ZPL_BIT_RATE_E *type, zpl_uint32 *bitrate);

/*编码等级*/
extern int zpl_media_channel_video_profile_set(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_uint32	profile);
extern int zpl_media_channel_video_profile_get(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_uint32	*profile);

/*I帧间隔*/
extern int zpl_media_channel_video_ikey_rate_set(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_uint32 ikey_rate);
extern int zpl_media_channel_video_ikey_rate_get(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_uint32 *ikey_rate);


extern int zpl_media_channel_video_enRcMode_set(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, ZPL_VENC_RC_E val);
extern int zpl_media_channel_video_enRcMode_get(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, ZPL_VENC_RC_E *val);

extern int zpl_media_channel_video_gopmode_set(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, ZPL_VENC_GOP_MODE_E val);
extern int zpl_media_channel_video_gopmode_get(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, ZPL_VENC_GOP_MODE_E *val);



#ifdef ZPL_SHELL_MODULE
int zpl_media_channel_show(void *pvoid);
#endif

extern int zpl_media_channel_foreach(int (*callback)(zpl_media_channel_t*, zpl_void *obj), zpl_void *obj);


#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_CHANNEL_H__ */
