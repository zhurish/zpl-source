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

#define ZPL_INVALID_VAL	-1


#define ZPL_MEDIA_PROCESS_ONE


#define ZPL_MEDIA_ISNORMAL_CHANNEL(n)	((n) >= 0)&&((n)<8)
#define ZPL_MEDIA_ISCAPTURE_CHANNEL(n)	((n) >= 16)&&((n)<24)
#define ZPL_MEDIA_ISRECORED_CHANNEL(n)	((n) >= 24)&&((n)<32)
#define ZPL_MEDIA_ISFILE_CHANNEL(n)	    ((n) >= 32)&&((n)<40)

#define ZPL_MEDIA_BUF_ALIGN(n)	(((n)+3)/4)*4

#define ZPL_MEDIA_CHANNEL_SET(c,i,t)	    ((c) << 8)|((i)<<4)|(t)
#define ZPL_MEDIA_CHANNEL_GET_C(n)	        (((n) >> 8) & 0xFF)
#define ZPL_MEDIA_CHANNEL_GET_I(n)	        (((n) >> 4) & 0x0F)
#define ZPL_MEDIA_CHANNEL_GET_T(n)	        ((n) & 0x0F)

typedef enum
{
    ZPL_MEDIA_CHANNEL_NORMAL	= 0x00,				//预览默认通道类型<0-8>
    ZPL_MEDIA_CHANNEL_CAPTURE	= 0x01,				//抓拍通道<16-23>
    ZPL_MEDIA_CHANNEL_RECORED	= 0x02,				//录像通道<24-32>
    ZPL_MEDIA_CHANNEL_FILE  	= 0x04,				//录像通道<32-40>
} ZPL_MEDIA_CHANNEL_TYPE_E;

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
    ZPL_MEDIA_NODE_INPUT = ZPL_SUB_MODULE_ID(MODULE_ZPLMEDIA,1),			//输入
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
}zpl_media_head_t __attribute__ ((aligned (1)));



#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_H__ */
