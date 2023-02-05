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
    ZPL_MEDIA_CHANNEL_INDEX_MAIN,			//主码流
    ZPL_MEDIA_CHANNEL_INDEX_SUB,			//次码流
    ZPL_MEDIA_CHANNEL_INDEX_SUB1,			//三码流
} ZPL_MEDIA_CHANNEL_INDEX_E;


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



typedef struct 
{
	zpl_uint8 	channel;
    zpl_uint8 	channel_index;		
	zpl_uint32 	timetick;		
	zpl_uint32 	frame_seq;	
	zpl_uint32	video_len;
	zpl_uint32	audio_lan;
}__attribute__ ((packed)) zpl_media_head_t ;


typedef struct
{
    zpl_uint16 	ID;                //ID 通道号
    zpl_uint8 	frame_type;        //音频/视频 ZPL_MEDIA_E
    zpl_uint8 	frame_codec;       //编码类型 ZPL_VIDEO_CODEC_E
    zpl_uint8 	frame_key;         //帧类型  ZPL_VIDEO_FRAME_TYPE_E
    zpl_uint8 	frame_rev;         //
    zpl_uint16 	frame_flags;       //ZPL_BUFFER_DATA_E
    zpl_uint32 	frame_timetick;    //时间戳毫秒
    zpl_uint32 	frame_seq;         //序列号底层序列号
    zpl_int32	frame_len;         //帧长度
    zpl_uint32  sessionID;         //just for file media    
}__attribute__ ((packed)) zpl_media_hdr_t ;


#endif

#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_H__ */
