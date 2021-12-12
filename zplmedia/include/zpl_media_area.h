/*
 * zpl_media_area.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_MEDIA_AREA_H__
#define __ZPL_MEDIA_AREA_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "zpl_media.h"
#include "zpl_media_text.h"

#define ZPL_MEDIA_AREA_OVERLAY_MAX      6 //叠加区域最大数量
#define ZPL_MEDIA_AREA_MOSAIC_MAX       6 //打马赛克区域最大数量
#define ZPL_MEDIA_AREA_INTERESTED_MAX   6 //感兴趣区域最大数量

#define ZPL_MEDIA_AREA_CHANNEL_MAX (ZPL_MEDIA_AREA_OVERLAY_MAX + ZPL_MEDIA_AREA_MOSAIC_MAX + ZPL_MEDIA_AREA_INTERESTED_MAX + 1)

typedef enum
{
    ZPL_MEDIA_AREA_NONE = 0,
    ZPL_MEDIA_AREA_MOSAIC,     //屏蔽区域
    ZPL_MEDIA_AREA_INTERESTED, //感兴趣区域
    ZPL_MEDIA_AREA_OVERLAY,    //叠加区域 打马赛克区域
    ZPL_MEDIA_AREA_OSD,        //OSD区域
} ZPL_MEDIA_AREA_E;

typedef enum
{
    ZPL_MEDIA_OSD_NONE = 0,
    ZPL_MEDIA_OSD_CHANNAL,  //通道信息
    ZPL_MEDIA_OSD_DATETIME, //时间信息
    ZPL_MEDIA_OSD_RECT,     //画框
    ZPL_MEDIA_OSD_BITRATE,  //码率信息
    ZPL_MEDIA_OSD_LABEL,    //标签信息
    ZPL_MEDIA_OSD_OTHER,    //其他信息
} ZPL_MEDIA_OSD_TYPE_E;


typedef struct
{
    zpl_uint32 areaid;
    ZPL_MEDIA_AREA_E areatype;
    zpl_bool b_rect;

    ZPL_MEDIA_OSD_TYPE_E osd_type;
    union
    {
        zpl_rect_t m_rect;         //矩形区域
        zpl_multarea_t m_multarea; //多边形区域
    } media_area;

    zpl_media_text_t *m_text; //OSD字符
    zpl_void *m_hwregion;     //区域硬件资源

    zpl_bool        bactive;
} zpl_media_area_t;



extern zpl_media_area_t *zpl_media_area_create(ZPL_MEDIA_AREA_E type);
extern int zpl_media_area_destroy(zpl_media_area_t * area);
extern int zpl_media_area_rectsize(zpl_media_area_t * area, zpl_rect_t rect);
extern int zpl_media_area_multrectsize(zpl_media_area_t * area, zpl_multarea_t *rect);
extern int zpl_media_area_osd_attr(zpl_media_area_t * area, ZPL_MEDIA_OSD_TYPE_E osd, zpl_uint32 bgcolor);
extern int zpl_media_area_osd_show(zpl_media_area_t * area, zpl_bool bshow, zpl_char *osdstring, zpl_uint32 pixel,
                               zpl_uint32 color, zpl_bool bold);
extern int zpl_media_area_active(zpl_media_area_t * area, zpl_bool bactive);
extern int zpl_media_area_destroy_all(void *chn);
extern int zpl_media_area_channel_default(void *);

#ifdef __cplusplus
}
#endif

#endif /* __ZPL_MEDIA_AREA_H__ */
