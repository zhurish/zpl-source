/*
 * zpl_media.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_MEDIA_H__
#define __ZPL_MEDIA_H__

#ifdef __cplusplus
extern "C" {
#endif



#include "auto_include.h"
#include "zplos_include.h"
#include "lib_include.h" 
#include "vty_include.h" 

#ifdef ZPL_LIBMEDIA_MODULE

#define ZPL_INVALID_VAL	-1


#ifdef ZPL_BUILD_LINUX
#ifdef ZPL_HISIMPP_MODULE
#define     ZPL_MEDIA_BASE_PATH     "/nfsroot"
#else
#define     ZPL_MEDIA_BASE_PATH     "/home/zhurish/workspace/working/zpl-source/source/multimedia/media"
#endif
#else
#define     ZPL_MEDIA_BASE_PATH     "D:/qt-project/live555-test"
#endif

#define ZPL_VIDEO_VPSSGRP_ENABLE

#define ZPL_MEDIA_BUF_ALIGN(n)	(((n)+3)/4)*4

#define ZPL_MEDIA_CHANNEL_SET(c,i,t)	    ((c) << 8)|((i)<<4)|(t)
#define ZPL_MEDIA_CHANNEL_GET_C(n)	        (((n) >> 8) & 0xFF)
#define ZPL_MEDIA_CHANNEL_GET_I(n)	        (((n) >> 4) & 0x0F)
#define ZPL_MEDIA_CHANNEL_GET_T(n)	        ((n) & 0x0F)

#define ZPL_MEDIA_CHANNEL_AUDIO(n)	        ((n) | 0x30)

typedef enum
{
    ZPL_MEDIA_CHANNEL_NONE = -1,
    ZPL_MEDIA_CHANNEL_0 = 0,			//通道0
    ZPL_MEDIA_CHANNEL_1 = 1,			//通道1
    ZPL_MEDIA_CHANNEL_2 = 2,			//通道2
    ZPL_MEDIA_CHANNEL_3 = 3,            //通道3

    ZPL_MEDIA_CHANNEL_AUDIO_0 = 0x30,			//通道0
    ZPL_MEDIA_CHANNEL_AUDIO_1 = 0x31,			//通道1
    ZPL_MEDIA_CHANNEL_AUDIO_2 = 0x32,			//通道2
    ZPL_MEDIA_CHANNEL_AUDIO_3 = 0x33,            //通道3
    ZPL_MEDIA_CHANNEL_MAX,
} ZPL_MEDIA_CHANNEL_E; /* 通道 */

typedef enum
{
    ZPL_MEDIA_CHANNEL_TYPE_NONE = -1,
    ZPL_MEDIA_CHANNEL_TYPE_MAIN = 0,	//主码流
    ZPL_MEDIA_CHANNEL_TYPE_SUB,			//次码流
    ZPL_MEDIA_CHANNEL_TYPE_SUB1,		//三码流
    ZPL_MEDIA_CHANNEL_TYPE_INPUT,       //音频输入 AI -> AENC
    ZPL_MEDIA_CHANNEL_TYPE_OUTPUT,      //音频输出 ADEV -> AO
    ZPL_MEDIA_CHANNEL_TYPE_MAX,
} ZPL_MEDIA_CHANNEL_TYPE_E;/* 码流 */

typedef enum
{
    ZPL_MEDIA_CONNECT_NONE = 0,			//
    ZPL_MEDIA_CONNECT_HW = 1,			//
    ZPL_MEDIA_CONNECT_SW = 2,			//
} ZPL_MEDIA_CONNECT_TYPE_E;

typedef enum
{
    ZPL_MEDIA_VIDEO = 1,			//
    ZPL_MEDIA_AUDIO = 2,			//
} ZPL_MEDIA_E;

typedef struct zpl_point_s
{
	zpl_uint32 x;			//x轴坐标
	zpl_uint32 y;			//y轴坐标
}zpl_point_t __attribute__ ((aligned (4)));

typedef struct
{
	zpl_uint32 width;			//宽度
	zpl_uint32 height;			//高度
}zpl_video_size_t __attribute__ ((aligned (4)));

typedef struct zpl_rect_s
{
	zpl_uint32 x;		//x轴坐标
	zpl_uint32 y;		//y轴坐标
	zpl_uint32 width;	//宽度
	zpl_uint32 height;	//高度
}zpl_rect_t __attribute__ ((aligned (4)));

#define ZPL_MEDIA_AREA_MAX  6

typedef struct zpl_multarea_s
{
	zpl_uint8 num;		//
	zpl_point_t pt[ZPL_MEDIA_AREA_MAX];
}zpl_multarea_t;


typedef struct  {
    zpl_int32      modId;
    zpl_int32      devId;
    zpl_int32      chnId;
} zpl_media_syschn_t;

typedef enum
{
    ZPL_MEDIA_FRAME_DATA_INPUT       = 0x00,
    ZPL_MEDIA_FRAME_DATA_ENCODE      = 0x01,           //编码后
    ZPL_MEDIA_FRAME_DATA_RECORD      = 0x02,			  //录像
    ZPL_MEDIA_FRAME_DATA_CAPTURE     = 0x04,           //抓拍
    ZPL_MEDIA_FRAME_DATA_YUV420      = 0x08,		      //YUV输入
    ZPL_MEDIA_FRAME_DATA_YUV422      = 0x10,		      //
    ZPL_MEDIA_FRAME_DATA_BMP         = 0x20,           //
} ZPL_MEDIA_FRAME_DATA_E;


typedef struct
{
    zpl_uint16 	ID;          //ID 通道号
    zpl_uint8 	type;        //音频/视频 ZPL_MEDIA_E
    zpl_uint8 	codectype;   //编码类型 ZPL_VIDEO_CODEC_E
    zpl_uint8 	frame_type;  //帧类型  ZPL_VIDEO_FRAME_TYPE_E
    zpl_uint8 	buffertype;    //ZPL_MEDIA_FRAME_DATA_E
    zpl_uint32 	timetick;    //时间戳毫秒
    zpl_uint32 	seqnum;      //序列号底层序列号
    zpl_int32	length;      //帧长度  
}__attribute__ ((packed)) zpl_media_hdr_t ;

typedef enum 
{
    ZPL_MEDIA_GLOAL_VIDEO_DEV  = 0x00,
    ZPL_MEDIA_GLOAL_VIDEO_INPUT      = 0x02,    //输入
    ZPL_MEDIA_GLOAL_VIDEO_VPSS       = 0x04,    //处理
    ZPL_MEDIA_GLOAL_VIDEO_ENCODE     = 0x05,    //编码
    ZPL_MEDIA_GLOAL_VIDEO_DECODE     = 0x06,    //解码
    ZPL_MEDIA_GLOAL_VIDEO_OUTPUT     = 0x07,    //输出

    ZPL_MEDIA_GLOAL_AUDIO      = 0x11,

    ZPL_MEDIA_GLOAL_MAX       ,
} ZPL_MEDIA_GLOBAL_E;

typedef struct
{
    zpl_int32 	channel;
    zpl_int32 	group;   
    zpl_int32   ID;
}zpl_media_gkey_t ;


typedef struct zpl_media_channel_s zpl_media_channel_t;


typedef struct
{
    LIST video_encode_list;
    os_mutex_t video_encode_mutex;

    LIST video_decode_list;
    os_mutex_t video_decode_mutex;

    LIST video_output_list;
    os_mutex_t video_output_mutex;

    LIST vpss_channel_list;
    os_mutex_t vpss_channel_mutex;

    LIST video_input_list;
    os_mutex_t video_input_mutex;

    LIST input_dev_list;
    os_mutex_t input_dev_mutex;

    LIST audio_list;
    os_mutex_t audio_mutex;

}zpl_media_global_t ;

extern int zpl_media_global_init(void);
extern int zpl_media_global_exit(void);
extern int zpl_media_global_lock(ZPL_MEDIA_GLOBAL_E type);
extern int zpl_media_global_unlock(ZPL_MEDIA_GLOBAL_E type);
extern int zpl_media_global_gkey_cmpset(ZPL_MEDIA_GLOBAL_E type, int (*cmpfunc)(void*, void*));
extern int zpl_media_global_freeset(ZPL_MEDIA_GLOBAL_E type, void (*cmpfunc)(void*));
extern int zpl_media_global_del(ZPL_MEDIA_GLOBAL_E type, void *node);
extern int zpl_media_global_add(ZPL_MEDIA_GLOBAL_E type, void *node);
extern void * zpl_media_global_lookup(ZPL_MEDIA_GLOBAL_E type, int channel, int group, int ID);
extern int zpl_media_global_get(ZPL_MEDIA_GLOBAL_E type, LIST **lst, os_mutex_t **mutex);
extern int zpl_media_global_foreach(ZPL_MEDIA_GLOBAL_E type, int (*func)(void*, void*), void *p);

extern int zpl_media_system_bind(zpl_media_syschn_t src, zpl_media_syschn_t dst);
extern int zpl_media_system_unbind(zpl_media_syschn_t src, zpl_media_syschn_t dst);

int zpl_media_system_init(void);

#endif

#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_H__ */
