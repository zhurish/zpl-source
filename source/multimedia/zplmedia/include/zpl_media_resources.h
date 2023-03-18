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

#include <zpl_media.h>
#include <zpl_vidhal.h>

#ifndef VI_MAX_DEV_NUM
#define VI_MAX_DEV_NUM 1
#endif
#ifndef VI_MAX_PIPE_NUM
#define VI_MAX_PIPE_NUM 1
#endif
#ifndef VI_MAX_CHN_NUM
#define VI_MAX_CHN_NUM 4
#endif
#ifndef VPSS_MAX_GRP_NUM
#define VPSS_MAX_GRP_NUM 8
#endif
#ifndef VPSS_MAX_CHN_NUM
#define VPSS_MAX_CHN_NUM 3
#endif
#ifndef VENC_MAX_CHN_NUM
#define VENC_MAX_CHN_NUM 16
#endif

/* 资源索引 */
typedef enum 
{
    ZPL_VIDHAL_INDEX_DEV     = 0x00,     
    ZPL_VIDHAL_INDEX_VIPIPE  = 0x01,     
    ZPL_VIDHAL_INDEX_INPUTCHN   = 0x02,     
    ZPL_VIDHAL_INDEX_VPSSGRP = 0x03,
    ZPL_VIDHAL_INDEX_VPSSCHN = 0x04,     
    ZPL_VIDHAL_INDEX_VENCCHN = 0x05,     
    ZPL_VIDHAL_INDEX_VODEV   = 0x06,       
    ZPL_VIDHAL_INDEX_VOCHN   = 0x07,
    ZPL_VIDHAL_INDEX_CAPTURE_VENCCHN = 0x08, 
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

typedef struct 
{
    zpl_int32           snsdev;
    zpl_int32           mipmdev;
    zpl_int32           snstype;
    zpl_int32           enWDRMode;
    zpl_int32           bMultiPipe;
    zpl_int32           SnapPipe;
    zpl_int32           bDoublePipe;
    zpl_int32           s32BusId;
}zpl_video_devres_t;

typedef struct 
{
    zpl_int32           id;
    zpl_uint32          flag;
}zpl_resources_t;

typedef struct 
{
    zpl_resources_t vdev_halres[VI_MAX_DEV_NUM];
    zpl_resources_t vpipe_halres[VI_MAX_PIPE_NUM];
    zpl_resources_t vchn_halres[VI_MAX_CHN_NUM];
    zpl_resources_t vpssgrp_halres[VPSS_MAX_GRP_NUM];
    zpl_resources_t vpsschn_halres[VPSS_MAX_GRP_NUM][VPSS_MAX_CHN_NUM];
    zpl_resources_t venc_halres[VENC_MAX_CHN_NUM];
}zpl_media_halres_t;

extern zpl_media_halres_t _halres;

zpl_uint32 zpl_video_halres_get_flag(int grp, int id, int type);
zpl_uint32 zpl_video_halres_set_flag(int grp, int id, int flag, int type);
#define ZPL_MEDIA_HALRES_GET(g,i,t)   zpl_video_halres_get_flag(g, i, ZPL_VIDHAL_INDEX_ ## t)
#define ZPL_MEDIA_HALRES_SET(g,i,f,t)   zpl_video_halres_set_flag(g, i, f, ZPL_VIDHAL_INDEX_ ## t)


zpl_uint32 zpl_video_devres_get(ZPL_MEDIA_CHANNEL_E channel, zpl_video_devres_t *info);

//获取pipe数组
int zpl_video_resources_get_pipe(zpl_int32 *vipipe);


#define VIDHAL_RES_ID_LOAD(n,t,i)   zpl_video_resources_get(n, t, ZPL_VIDHAL_INDEX_ ## i)


#ifdef __cplusplus
}
#endif
#endif /* __ZPL_VIDHAL_RESOURCES_H__ */
