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
//#include <zpl_vidhal.h>

#define ZPL_MEDIA_HALRES_PATH   ZPL_MEDIA_BASE_PATH"/media.json"
#define ZPL_MEDIA_HALRES_NUM 8

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
    ZPL_VIDHAL_INDEX_ALL     = -1,     

    ZPL_VIDHAL_INDEX_DEV     = 0x00,     
    ZPL_VIDHAL_INDEX_VIPIPE  = 0x01,     
    ZPL_VIDHAL_INDEX_INPUTCHN   = 0x02,     
    ZPL_VIDHAL_INDEX_VPSSGRP = 0x03,
    ZPL_VIDHAL_INDEX_VPSSCHN = 0x04,     
    ZPL_VIDHAL_INDEX_VENCCHN = 0x05,     
    ZPL_VIDHAL_INDEX_VODEV   = 0x06,       
    ZPL_VIDHAL_INDEX_VOCHN   = 0x07,
    ZPL_VIDHAL_INDEX_CAPTURE_VENCCHN = 0x08, 

    ZPL_VIDHAL_INDEX_MIPIDEV   = 0x09,
    ZPL_VIDHAL_INDEX_SENSORDEV = 0x0A, 
    ZPL_VIDHAL_INDEX_SENSORTYPE   = 0x0B,
    ZPL_VIDHAL_INDEX_ISPDEV = 0x0C, 

    ZPL_VIDHAL_INDEX_AIDEV   = 0x20,       
    ZPL_VIDHAL_INDEX_AICHN   = 0x21,
    ZPL_VIDHAL_INDEX_AENCCHN = 0x22,     
    ZPL_VIDHAL_INDEX_ADECCHN = 0x23,     
    ZPL_VIDHAL_INDEX_AODEV   = 0x24,       
    ZPL_VIDHAL_INDEX_AOCHN   = 0x25,

    ZPL_VIDHAL_INDEX_MAX,
} ZPL_VIDHAL_INDEX_E;



typedef struct 
{
    zpl_int32           use;
    zpl_int32           channel;
    zpl_int32           chnindex;
    zpl_int32           vencchn;
    zpl_int32           capvencchn;
    zpl_int32           vpssgrp;
    zpl_int32           vpsschn;
    zpl_int32           inputchn;
    zpl_int32           inputpipe;
    zpl_int32           devid;
    zpl_int32           mipidev;
    zpl_int32           sensordev;
    zpl_int32           sensortype;
    zpl_int32           ispdev;
    zpl_int32           inputchn_hwconnect;
    zpl_int32           vpsschn_hwconnect;
}zpl_media_hwres_t;

typedef struct 
{
    zpl_int32           id;
    zpl_uint32          flag;
}zpl_resdev_t;

typedef struct 
{
    zpl_media_hwres_t halres[ZPL_MEDIA_HALRES_NUM];

    zpl_resdev_t vdev_halres[VI_MAX_DEV_NUM];
    zpl_resdev_t vpipe_halres[VI_MAX_PIPE_NUM];
    zpl_resdev_t vchn_halres[VI_MAX_CHN_NUM];
    zpl_resdev_t vpssgrp_halres[VPSS_MAX_GRP_NUM];
    zpl_resdev_t vpsschn_halres[VPSS_MAX_GRP_NUM][VPSS_MAX_CHN_NUM];
    zpl_resdev_t venc_halres[VENC_MAX_CHN_NUM];

}zpl_media_halres_t;

extern zpl_media_halres_t _halres;

extern zpl_uint32 zpl_video_halres_get_flag(int grp, int id, int type);
extern zpl_uint32 zpl_video_halres_set_flag(int grp, int id, int flag, int type);
#define ZPL_MEDIA_HALRES_GET(g,i,t)   zpl_video_halres_get_flag(g, i, ZPL_VIDHAL_INDEX_ ## t)
#define ZPL_MEDIA_HALRES_SET(g,i,f,t)   zpl_video_halres_set_flag(g, i, f, ZPL_VIDHAL_INDEX_ ## t)


extern zpl_media_hwres_t * zpl_video_resources_get(int chn, int indx, int type);
#define ZPL_MEDIA_HALRES_ID_LOAD(n,t,i)   zpl_video_resources_get(n, t, ZPL_VIDHAL_INDEX_ ## i)


extern int zpl_media_hwres_load(char *filename);

#ifdef __cplusplus
}
#endif
#endif /* __ZPL_VIDHAL_RESOURCES_H__ */
