
/*
 * zpl_video_encode.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_VIDEO_ENCODE_H__
#define __ZPL_VIDEO_ENCODE_H__

#ifdef __cplusplus
extern "C" {
#endif



//#define ZPL_VENC_READ_DEBUG		0x200000
#define ZPL_VIDHAL_DEFULT_FRAMERATE     30      //默认帧率
#define ZPL_VIDHAL_DEFULT_BITRATE       ZPL_BIT_RATE_CBR//默认码率类型
#define ZPL_VIDHAL_DEFULT_IKEY_INTERVAL 30      //  默认I帧间隔
#define ZPL_VIDHAL_DEFULT_FORMAT_MAIN   ZPL_VIDEO_FORMAT_1080P     //  默认主码流分辨率
#define ZPL_VIDHAL_DEFULT_FORMAT_SUB    ZPL_VIDEO_FORMAT_720P      //  默认次码流分辨率
#define ZPL_VIDHAL_DEFULT_FORMAT_SUB1   ZPL_VIDEO_FORMAT_D1_NTSC  //  默认三码流分辨率



typedef struct 
{
    NODE                node;
	
    zpl_int32           venc_channel;       //底层通道号
    zpl_socket_t        vencfd;             //

	zpl_video_codec_t   *pCodec;			//指向上层编码参数
    zpl_bool            b_capture;
    zpl_void            *t_read;
	zpl_void            *t_master;
    zpl_void            *frame_queue;      //编码后数据队列

	zpl_void			*source_input;		//绑定的VPSS
	zpl_void			*media_channel;		//指向父级

    zpl_bool            online;         //资源状态，在线不在线
    zpl_uint32          flags;

#ifdef ZPL_HISIMPP_HWDEBUG
    zpl_uint32              dbg_send_count;
    zpl_uint32              dbg_recv_count;    
#endif
    int (*get_encode_frame)(void *);
}zpl_media_video_encode_t;


int zpl_media_video_encode_init(void);
zpl_media_video_encode_t * zpl_media_video_encode_lookup(zpl_int32 venc_channel, zpl_bool capture);
zpl_media_video_encode_t * zpl_media_video_encode_create(zpl_int32 venc_channel, zpl_bool capture);
int zpl_media_video_encode_hal_create(zpl_media_video_encode_t *encode);
int zpl_media_video_encode_destroy(zpl_media_video_encode_t *chn);
int zpl_media_video_encode_hal_destroy(zpl_media_video_encode_t *chn);
int zpl_media_video_encode_start(zpl_void *master, zpl_media_video_encode_t *encode);
int zpl_media_video_encode_stop(zpl_media_video_encode_t *encode);
zpl_bool zpl_media_video_encode_state_check(zpl_media_video_encode_t *encode, int bit);
int zpl_media_video_encode_thread(zpl_media_video_encode_t *encode, zpl_bool start);

int zpl_media_video_encode_source_set(zpl_int32 venc_channel, void *halparam);

int zpl_media_video_encode_frame_queue_set(zpl_int32 venc_channel, void *frame_queue);

int zpl_media_video_encode_sendto(zpl_media_video_encode_t *encode,  void *p, zpl_int timeout);


int zpl_media_video_encode_request_IDR(zpl_media_video_encode_t *encode);
int zpl_media_video_encode_enable_IDR(zpl_media_video_encode_t *encode, zpl_bool bEnableIDR);
int zpl_media_video_encode_encode_reset(zpl_media_video_encode_t *encode);

#ifdef ZPL_SHELL_MODULE
int zpl_media_video_encode_show(void *pvoid);
#endif

#ifdef __cplusplus
}
#endif
#endif /* __ZPL_VIDEO_ENCODE_H__ */
