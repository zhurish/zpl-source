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
#ifdef ZPL_HISIMPP_MODULE
typedef struct 
{
#if BYTE_ORDER == LITTLE_ENDIAN
    zpl_uint8   rev;
    zpl_uint8   frame_type;
    zpl_uint8   len;//音频帧长度=len*2,
    zpl_uint8   seq;
#else    
    zpl_uint8   frame_type;
    zpl_uint8   rev;
    zpl_uint8   seq;
    zpl_uint8   len;//音频帧长度=len*2,
#endif 
}__attribute__ ((packed)) zpl_audio_frame_hdr_t ;
#endif

typedef struct 
{
    zpl_uint8   frame_type;
    zpl_uint8   codec;
    zpl_uint64  timeStamp;
    zpl_uint32  seqnum;
    zpl_uint16  len;
    zpl_uint16  maxlen;
    zpl_uint8   *data;
}__attribute__ ((packed)) zpl_audio_frame_t ;


typedef struct 
{
    zpl_bool            bEnable;
    zpl_int32           devid;         //绑定的设备号
    zpl_int32           channel;       //底层通道号
    zpl_socket_t        fd;             //
    zpl_void            *t_read;
    int (*get_encode_frame)(void *, void *, zpl_audio_frame_t *);
    zpl_audio_codec_t   codec;
}zpl_audio_encode_decode_t;


typedef struct 
{
    zpl_bool            bEnable;
    zpl_int32           devid;         //绑定的设备号
    zpl_int32           channel;       //底层通道号

    zpl_uint32          clock_rate;     /**< Sampling rate.                 */
    zpl_uint8           channel_cnt;
    zpl_uint8           bits_per_sample; /**< Bits/sample in the PCM side    */
    zpl_uint32          max_frame_size;
    //audio codec
    zpl_int32           out_volume;
    zpl_bool            b_connect_decode;
    zpl_audio_encode_decode_t *decode;
}zpl_audio_output_t;

typedef struct 
{
    zpl_bool            bEnable;
    zpl_int32           devid;         //绑定的设备号
    zpl_int32           channel;       //底层通道号
    zpl_socket_t        fd;             //
    zpl_void            *t_read;

    zpl_uint32			framerate;		//帧率

    zpl_uint32          clock_rate;     /**< Sampling rate.                 */
    zpl_uint8           channel_cnt;
    zpl_uint8           bits_per_sample; /*每帧的采样点个数  （1/framerate）* clock_rate */
    zpl_uint32          max_frame_size;

    //audio codec
    zpl_int32           micgain;
    zpl_bool            boost;
    zpl_int32           in_volume;

    zpl_bool            b_connect_encode;
    zpl_audio_encode_decode_t *encode;
    zpl_bool            b_connect_output;
    zpl_audio_output_t  *output;
    int (*get_input_frame)(void *, void *, zpl_audio_frame_t *);

    int (*input_frame_handle)(void *, char *, char *, zpl_uint32);

}zpl_audio_input_t;


typedef struct 
{
    NODE                node;
	int                 channel;

    zpl_bool b_codec_active;

    zpl_audio_input_t    input;
    zpl_audio_output_t   output;
    
    zpl_audio_encode_decode_t   encode;
    zpl_audio_encode_decode_t   decode;

    zpl_uint32          flags;
	zpl_void            *t_master;
    zpl_void            *frame_queue;      //编码后数据队列

	zpl_void			*media_channel;		//指向父级

    //media channel audio codec (micgain|boost|in-volume|out-volume) value <0-100>
}zpl_media_audio_channel_t;


int zpl_media_audio_init(void);

int zpl_media_audio_param_default(zpl_media_audio_channel_t *audio);


zpl_media_audio_channel_t * zpl_media_audio_lookup(zpl_int32 channel);

zpl_media_audio_channel_t * zpl_media_audio_create(zpl_int32 channel, zpl_uint32 clock_rate, zpl_uint8 channel_cnt, zpl_uint8 bits_per_sample, zpl_uint32 framerate);
int zpl_media_audio_destroy(zpl_media_audio_channel_t *audio);

zpl_bool zpl_media_audio_state_check(zpl_media_audio_channel_t *audio, int bit);

int zpl_media_audio_input_set(zpl_media_audio_channel_t *audio, int clock_rate, int channel_cnt, int bits_per_sample, int framerate);
int zpl_media_audio_encode_set(zpl_media_audio_channel_t *audio, int clock_rate, int channel_cnt, int bits_per_sample, int framerate);
int zpl_media_audio_decode_set(zpl_media_audio_channel_t *audio, int clock_rate, int channel_cnt, int bits_per_sample, int framerate);
int zpl_media_audio_output_set(zpl_media_audio_channel_t *audio, int clock_rate, int channel_cnt, int bits_per_sample, int framerate);

/*音频创建后处于关闭状态，使能录音功能后打开输入通道和编码通道*/
int zpl_media_audio_input_start(zpl_void *master, zpl_media_audio_channel_t *audio);
int zpl_media_audio_input_stop(zpl_media_audio_channel_t *audio);
int zpl_media_audio_encode_start(zpl_void *master, zpl_media_audio_channel_t *audio);
int zpl_media_audio_encode_stop(zpl_media_audio_channel_t *audio);
int zpl_media_audio_decode_start(zpl_void *master, zpl_media_audio_channel_t *audio);
int zpl_media_audio_decode_stop(zpl_media_audio_channel_t *audio);
int zpl_media_audio_output_start(zpl_void *master, zpl_media_audio_channel_t *audio);
int zpl_media_audio_output_stop(zpl_media_audio_channel_t *audio);

int zpl_media_audio_input_connect_encode(zpl_media_audio_channel_t *audio, ZPL_MEDIA_CONNECT_TYPE_E b_enable);
int zpl_media_audio_output_connect_decode(zpl_media_audio_channel_t *audio, ZPL_MEDIA_CONNECT_TYPE_E b_enable);
int zpl_media_audio_input_connect_output(zpl_media_audio_channel_t *audio, ZPL_MEDIA_CONNECT_TYPE_E b_enable);


int zpl_media_audio_input_micgain(zpl_media_audio_channel_t *audio, int val);//[0,16]
int zpl_media_audio_input_boost(zpl_media_audio_channel_t *audio, zpl_bool val);//[0,1]
int zpl_media_audio_input_volume(zpl_media_audio_channel_t *audio, int val);//[19,50]
int zpl_media_audio_output_volume(zpl_media_audio_channel_t *audio, int val);//[-121, 6]

int zpl_media_audio_output_frame_write(zpl_media_audio_channel_t *audio,  zpl_audio_frame_t *p);

int zpl_media_audio_decode_frame_write(zpl_media_audio_channel_t *audio,  zpl_audio_frame_t *p);
int zpl_media_audio_decode_frame_read(zpl_media_audio_channel_t *audio,  zpl_audio_frame_t *p);

int zpl_media_audio_encode_frame_write(zpl_media_audio_channel_t *audio,  zpl_audio_frame_t *p);
int zpl_media_audio_encode_frame_read(zpl_media_audio_channel_t *audio,  zpl_audio_frame_t *p);

int zpl_media_audio_input_frame_read(zpl_media_audio_channel_t *audio,  zpl_audio_frame_t *p);


#ifdef ZPL_SHELL_MODULE
int zpl_media_audio_show(void *pvoid);
#endif

#ifdef __cplusplus
}
#endif
#endif /* __ZPL_AUDIO_AUDIO_H__ */
