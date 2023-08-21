/**
 * @file     zpl_media_audio.h
 * @brief     : Description
 * @author   zhurish (zhurish@163.com)
 * @version  1.0
 * @date     2023-08-13
 * 
 * @copyright   Copyright (c) 2023 {author}({email}).Co.Ltd. All rights reserved.
 * 
 */
#ifndef __ZPL_AUDIO_AUDIO_H__
#define __ZPL_AUDIO_AUDIO_H__

#ifdef __cplusplus
extern "C" {
#endif


typedef struct 
{
    zpl_bool            bEnable;
    zpl_int32           devid;         //绑定的设备号
    zpl_int32           channel;       //底层通道号
    zpl_socket_t        fd;             //
    zpl_void            *t_read;
    int (*get_encode_frame)(void *, void *);
    zpl_audio_codec_t   codec;
}zpl_audio_encode_decodec_t;


typedef struct 
{
    zpl_bool            bEnable;
    zpl_int32           devid;         //绑定的设备号
    zpl_int32           channel;       //底层通道号
    zpl_bool            bResample;
    zpl_uint32          resample_rate;
    zpl_int32           volume;
    zpl_uint32          clock_rate;     /**< Sampling rate.                 */
    zpl_uint8           channel_cnt;
    zpl_uint8           bits_per_sample; /**< Bits/sample in the PCM side    */
    zpl_uint32          max_frame_size;

    int (*put_decode_frame)(void *, void *);

    ZPL_MEDIA_CONNECT_TYPE_E            hwbind;
    zpl_audio_encode_decodec_t          decode;    
}zpl_audio_output_t;

typedef struct 
{
    zpl_bool            bEnable;
    zpl_int32           devid;         //绑定的设备号
    zpl_int32           channel;       //底层通道号
    zpl_socket_t        fd;             //
    zpl_void            *t_read;
    zpl_bool            bResample;
    zpl_uint32          resample_rate;
    zpl_int32           volume;
    zpl_uint32          clock_rate;     /**< Sampling rate.                 */
    zpl_uint8           channel_cnt;
    zpl_uint8           bits_per_sample; /*每帧的采样点个数  （1/framerate）* clock_rate */
    zpl_uint32          max_frame_size;
    int (*input_frame_handle)(void *, char *, char *, zpl_uint32);
    /*
每帧的采样点个数。
取值范围：G711、G726、ADPCM_DVI4 编码时取值为 80、
160、240、320、480；ADPCM_IMA 编码时取值为 81、161、
241、321、481。
AI 取值范围为：[80, 2048]，AO 取值范围为：[80, 4096]。
静态属性*/
    int (*get_input_frame)(void *, void *);

    ZPL_MEDIA_CONNECT_TYPE_E            hwbind;
    zpl_audio_encode_decodec_t          encode;
    ZPL_MEDIA_CONNECT_TYPE_E            hw_connect_out;
    zpl_audio_output_t                  *output;
}zpl_audio_input_t;

typedef struct 
{
#if BYTE_ORDER == LITTLE_ENDIAN
    zpl_uint8   rev;
    zpl_uint8   frame_type;
    zpl_uint8   len;
    zpl_uint8   seq;
#else    
    zpl_uint8   frame_type;
    zpl_uint8   rev;
    zpl_uint8   seq;
    zpl_uint8   len;
#endif 
}__attribute__ ((packed)) zpl_audio_frame_hdr_t ;

typedef struct 
{
    NODE                node;
	int                 channel;
    zpl_bool            b_input;

    union
    {
        zpl_audio_input_t    input;
        zpl_audio_output_t   output;
    }audio_param;
    zpl_bool            b_inner_codec_enable;
    zpl_uint32          flags;
	zpl_void            *t_master;
    zpl_void            *frame_queue;      //编码后数据队列

	zpl_void			*media_channel;		//指向父级

    zpl_int32           micgain;
    zpl_int32           boost;
    zpl_int32           in_volume;
    zpl_int32           out_volume;
    //media channel audio codec (micgain|boost|in-volume|out-volume) value <0-100>
}zpl_media_audio_channel_t;


int zpl_media_audio_init(void);
zpl_media_audio_channel_t * zpl_media_audio_lookup(zpl_int32 channel, zpl_bool b_input);
zpl_media_audio_channel_t * zpl_media_audio_create(zpl_int32 channel, zpl_bool b_input);

int zpl_media_audio_param_default(zpl_media_audio_channel_t *audio);


int zpl_media_audio_hal_create(zpl_media_audio_channel_t *audio);
int zpl_media_audio_destroy(zpl_media_audio_channel_t *audio);
int zpl_media_audio_hal_destroy(zpl_media_audio_channel_t *audio);
int zpl_media_audio_start(zpl_void *master, zpl_media_audio_channel_t *audio);
int zpl_media_audio_stop(zpl_media_audio_channel_t *audio);
zpl_bool zpl_media_audio_state_check(zpl_media_audio_channel_t *audio, int bit);
int zpl_media_audio_thread(zpl_media_audio_channel_t *audio, zpl_bool start);

int zpl_media_audio_connect_encode(zpl_media_audio_channel_t *audio, ZPL_MEDIA_CONNECT_TYPE_E b_enable);
int zpl_media_audio_connect_decode(zpl_media_audio_channel_t *audio, ZPL_MEDIA_CONNECT_TYPE_E b_enable);
int zpl_media_audio_connect_local_output(zpl_media_audio_channel_t *audio, ZPL_MEDIA_CONNECT_TYPE_E b_enable, int chan);

int zpl_media_audio_volume(zpl_media_audio_channel_t *audio, int val);


int zpl_media_audio_codec_enable(zpl_media_audio_channel_t *audio, zpl_bool b_enable);
int zpl_media_audio_codec_micgain(zpl_media_audio_channel_t *audio, int val);//[0,16]
int zpl_media_audio_codec_boost(zpl_media_audio_channel_t *audio, int val);//[0,1]
int zpl_media_audio_codec_input_volume(zpl_media_audio_channel_t *audio, int val);//[19,50]
int zpl_media_audio_codec_output_volume(zpl_media_audio_channel_t *audio, int val);//[-121, 6]

int zpl_media_audio_frame_queue_set(zpl_int32 channel, void *frame_queue);

int zpl_media_audio_sendto(zpl_media_audio_channel_t *audio,  void *p);
int zpl_media_audio_recv(zpl_media_audio_channel_t *audio,  void *p);

#ifdef ZPL_SHELL_MODULE
int zpl_media_audio_show(void *pvoid);
#endif


#ifdef __cplusplus
}
#endif
#endif /* __ZPL_AUDIO_AUDIO_H__ */
