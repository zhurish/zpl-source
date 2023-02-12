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

#include <zpl_type.h>
#include <thread.h>
#include <module.h>
#include "auto_include.h"
#include "zplos_include.h"

#ifdef ZPL_ZPLMEDIA_MODULE

#define ZPL_INVALID_VAL	-1


#ifdef ZPL_BUILD_LINUX
#define     ZPL_MEDIA_BASE_PATH     "/home/zhurish/workspace/working/zpl-source/source/multimedia/zplmedia/"
#else
#define     ZPL_MEDIA_BASE_PATH     "D:/qt-project/live555-test/"
#endif

#define ZPL_MEDIA_BUF_ALIGN(n)	(((n)+3)/4)*4

#define ZPL_MEDIA_CHANNEL_SET(c,i,t)	    ((c) << 8)|((i)<<4)|(t)
#define ZPL_MEDIA_CHANNEL_GET_C(n)	        (((n) >> 8) & 0xFF)
#define ZPL_MEDIA_CHANNEL_GET_I(n)	        (((n) >> 4) & 0x0F)
#define ZPL_MEDIA_CHANNEL_GET_T(n)	        ((n) & 0x0F)

typedef enum
{
    ZPL_MEDIA_CHANNEL_NONE = -1,
    ZPL_MEDIA_CHANNEL_0 = 0,			//主码流
    ZPL_MEDIA_CHANNEL_1 = 1,			//次码流
    ZPL_MEDIA_CHANNEL_2 = 2,			//三码流
    ZPL_MEDIA_CHANNEL_3 = 3,
    ZPL_MEDIA_CHANNEL_MAX,
} ZPL_MEDIA_CHANNEL_E;

typedef enum
{
    ZPL_MEDIA_CHANNEL_TYPE_NONE = -1,
    ZPL_MEDIA_CHANNEL_TYPE_MAIN = 0,			//主码流
    ZPL_MEDIA_CHANNEL_TYPE_SUB,			//次码流
    ZPL_MEDIA_CHANNEL_TYPE_SUB1,			//三码流
    ZPL_MEDIA_CHANNEL_TYPE_MAX,
} ZPL_MEDIA_CHANNEL_TYPE_E;


typedef enum
{
    ZPL_MEDIA_VIDEO = 1,			//
    ZPL_MEDIA_AUDIO = 2,			//
} ZPL_MEDIA_E;

typedef enum
{
    ZPL_MEDIA_NODE_PIPE = ZPL_SUB_MODULE_ID(MODULE_ZPLMEDIA,1),
    ZPL_MEDIA_NODE_INPUT,			//输入
    ZPL_MEDIA_NODE_PROCESS,			//处理
    ZPL_MEDIA_NODE_ENCODE,            //编码
    ZPL_MEDIA_NODE_OUTPUT,            //输出
    ZPL_MEDIA_NODE_RECORD,            //录制
    ZPL_MEDIA_NODE_CAPTURE,           //抓拍
} ZPL_MEDIA_NODE_E;



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
    zpl_uint32     enModId;
    zpl_int32      s32DevId;
    zpl_int32      s32ChnId;
} zpl_media_syschs_t;

typedef enum
{
    ZPL_MEDIA_FRAME_DATA_ENCODE      = 0x00,           //编码后
    ZPL_MEDIA_FRAME_DATA_RECORD      = 0x01,			  //录像
    ZPL_MEDIA_FRAME_DATA_CAPTURE     = 0x02,           //抓拍
    ZPL_MEDIA_FRAME_DATA_YUV420      = 0x04,		      //YUV输入
    ZPL_MEDIA_FRAME_DATA_YUV422      = 0x08,		      //
    ZPL_MEDIA_FRAME_DATA_BMP         = 0x10,           //
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
    ZPL_MEDIA_GLOAL_VIDEO_INPUTPIPE  = 0x01,
    ZPL_MEDIA_GLOAL_VIDEO_INPUT      = 0x02,
    ZPL_MEDIA_GLOAL_VIDEO_VPSSGRP    = 0x03,
    ZPL_MEDIA_GLOAL_VIDEO_VPSS       = 0x04,
    ZPL_MEDIA_GLOAL_VIDEO_ENCODE     = 0x05,
    ZPL_MEDIA_GLOAL_VIDEO_DECODE     = 0x06,
    ZPL_MEDIA_GLOAL_VIDEO_OUTPUT     = 0x07,

    ZPL_MEDIA_GLOAL_AUDIO_INPUT      = 0x11,
    ZPL_MEDIA_GLOAL_AUDIO_ENCODE     = 0x12,
    ZPL_MEDIA_GLOAL_AUDIO_DECODE     = 0x13,
    ZPL_MEDIA_GLOAL_AUDIO_OUTPUT     = 0x14,

    ZPL_MEDIA_GLOAL_MAX        = 0x10,
} ZPL_MEDIA_GLOBAL_E;

typedef struct
{
    zpl_int32 	channel;
    zpl_int32 	group;   
    zpl_int32   ID;
}zpl_media_gkey_t ;

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

    LIST vpss_group_list;
    os_mutex_t vpss_group_mutex;

    LIST video_input_list;
    os_mutex_t video_input_mutex;

    LIST input_pipe_list;
    os_mutex_t input_pipe_mutex;

    LIST audio_output_list;
    os_mutex_t audio_output_mutex;

    LIST audio_input_list;
    os_mutex_t audio_input_mutex;   

}zpl_media_global_t ;

extern int zpl_media_global_init(void);
extern int zpl_media_global_exit(void);
extern int zpl_media_global_lock(ZPL_MEDIA_GLOBAL_E type);
extern int zpl_media_global_unlock(ZPL_MEDIA_GLOBAL_E type);
extern int zpl_media_global_gkey_cmpset(ZPL_MEDIA_GLOBAL_E type, int (*cmpfunc)(void*, void*));
extern int zpl_media_global_del(ZPL_MEDIA_GLOBAL_E type, void *node);
extern int zpl_media_global_add(ZPL_MEDIA_GLOBAL_E type, void *node);
extern void * zpl_media_global_lookup(ZPL_MEDIA_GLOBAL_E type, int channel, int group, int ID);
extern int zpl_media_global_get(ZPL_MEDIA_GLOBAL_E type, LIST **lst, os_mutex_t **mutex);
extern int zpl_media_global_foreach(ZPL_MEDIA_GLOBAL_E type, int (*func)(void*, void*), void *p);

#endif

#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_H__ */
