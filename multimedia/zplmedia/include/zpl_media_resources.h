/*
 * zpl_media_resources.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_VIDHAL_RESOURCES_H__
#define __ZPL_VIDHAL_RESOURCES_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <zpl_type.h>

/* 资源索引 */
typedef enum 
{
    ZPL_VIDHAL_INDEX_DEV     = 0x00,     
    ZPL_VIDHAL_INDEX_VIPIPE  = 0x01,     
    ZPL_VIDHAL_INDEX_VICHN   = 0x02,     
    ZPL_VIDHAL_INDEX_VPSSGRP = 0x03,
    ZPL_VIDHAL_INDEX_VPSSCHN = 0x04,     
    ZPL_VIDHAL_INDEX_VENCCHN = 0x05,     
    ZPL_VIDHAL_INDEX_VODEV   = 0x06,       
    ZPL_VIDHAL_INDEX_VOCHN   = 0x07,

    ZPL_VIDHAL_INDEX_MAX,
} ZPL_VIDHAL_INDEX_E;

/* 资源标志位 */
typedef enum 
{
    ZPL_VIDEO_RES_FLAG_NONE   = 0x00000000,       //不创建
    ZPL_VIDEO_RES_FLAG_CREATE = 0x00000001,       //创建
    ZPL_VIDEO_RES_FLAG_INIT   = 0x00000004,       //已经创建
    ZPL_VIDEO_RES_FLAG_SRCBIND= 0x00000010,       //作为数据源被绑定
    ZPL_VIDEO_RES_FLAG_DSTBIND= 0x00000020,       //作为目的被绑定
    ZPL_VIDEO_RES_FLAG_START  = 0x00000040,       //开始
} ZPL_VIDEO_RES_FLAG_E;

/*  
*   -1 的资源无效
*/
typedef struct 
{
    const zpl_int32     channel;        //应用层通道号
    const zpl_int32     channel_index;   //应用层通道类型
    const zpl_uint32    width;			//宽度
	const zpl_uint32    height;			//高度
    zpl_uint32          flag[ZPL_VIDHAL_INDEX_MAX];//资源的创建销毁标志
    struct hal
    {
        const zpl_int32       input_dev;      //底层输入设备编号
        const zpl_int32       input_pipe;     //底层输入硬件pipe
        const zpl_int32       input_chn;      //底层输入通道号
        const zpl_int32       vpss_group;     //底层VPSS分组
        const zpl_int32       vpss_channel;   //底层VPS通道号
        const zpl_int32       venc_channel;   //底层编码通道号
        const zpl_int32       hdmi_dev;       //输出底层ID编号
        const zpl_int32       hdmi_chn;       //输出底层通道号
    }halres;
} zpl_video_resources_t;//资源管理表


zpl_uint32 zpl_video_resources_get_flag(zpl_int32 channel, 
    zpl_int32 channel_index, ZPL_VIDHAL_INDEX_E index);

int zpl_video_resources_get(zpl_int32 channel, 
    zpl_int32 channel_index, ZPL_VIDHAL_INDEX_E index);

//获取pipe数组
int zpl_video_resources_get_pipe(zpl_int32 *vipipe);

const char *zpl_video_resstring_get(ZPL_VIDEO_RES_FLAG_E e);

#define VIDHAL_RES_ID_LOAD(n,t,i)   zpl_video_resources_get(n, t, ZPL_VIDHAL_INDEX_ ## i)
#define VIDHAL_RES_FLAG_LOAD(n,t,i)   zpl_video_resources_get_flag(n, t, ZPL_VIDHAL_INDEX_ ## i)


#define VIDHAL_RES_FLAG_SET(n,f)        ((n) |= (ZPL_VIDEO_RES_FLAG_ ## f))
#define VIDHAL_RES_FLAG_CHECK(n,f)      ((n) & (ZPL_VIDEO_RES_FLAG_ ## f))
#define VIDHAL_RES_FLAG_UNSET(n,f)      ((n) &= ~(ZPL_VIDEO_RES_FLAG_ ## f))

int zpl_video_resources_show(void *p);


#ifdef __cplusplus
}
#endif
#endif /* __ZPL_VIDHAL_RESOURCES_H__ */
