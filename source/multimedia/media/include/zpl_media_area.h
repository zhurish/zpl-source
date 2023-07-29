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
    ZPL_MEDIA_AREA_RECT,     //画框
} ZPL_MEDIA_AREA_E;

typedef enum
{
    ZPL_MEDIA_OSD_NONE = 0,
    ZPL_MEDIA_OSD_CHANNAL,  //通道信息
    ZPL_MEDIA_OSD_DATETIME, //时间信息
    ZPL_MEDIA_OSD_BITRATE,  //码率信息
    ZPL_MEDIA_OSD_LABEL,    //标签信息
    ZPL_MEDIA_OSD_OTHER,    //其他信息
    ZPL_MEDIA_OSD_MAX,
} ZPL_MEDIA_OSD_TYPE_E;

#define ZPL_BITMAP_ALIGN(n)      (((((n)+3)/4)*4))

#define OSD_RGB_WHITE   0xFFFF
#define OSD_RGB_BLACK   0x0000

#define OSD_FGALPHA_DEFAULT   100
#define OSD_BGALPHA_DEFAULT   40

//background
typedef struct
{
    zpl_bool            bactive;
    zpl_media_bitmap_t  m_bitmap; 
    zpl_int32           fg_alpha;       //透明度
    zpl_int32           bg_alpha;       //背景透明度   
    zpl_media_text_t    *m_text;        //字符 
    zpl_point_t          m_point;        //矩形区域
    zpl_void            *m_hwregion;     //区域硬件资源
    char                *m_keystr;
} zpl_media_area_osd_t;

typedef struct
{
    zpl_bool            bactive;
    zpl_media_bitmap_t  m_bitmap; 
    zpl_int32           fg_alpha;       //透明度
    zpl_int32           bg_alpha;       //背景透明度   
    zpl_void            *m_hwregion;     //区域硬件资源
    zpl_rect_t          m_rect;
} zpl_media_area_rect_t;

typedef struct
{
    zpl_uint32 areaid;
    ZPL_MEDIA_AREA_E areatype;
    union
    {
        zpl_media_area_osd_t    osd[ZPL_MEDIA_OSD_MAX];
        zpl_media_area_rect_t   m_rect;
        zpl_multarea_t  m_multarea; //多边形区域
    } media_area;

    zpl_media_channel_t *mchn;
    void *t_timer;
} zpl_media_area_t;



extern zpl_media_area_t *zpl_media_channel_area_create(zpl_media_channel_t *chn, ZPL_MEDIA_AREA_E type);
extern int zpl_media_channel_area_destroy(zpl_media_area_t * area);

extern int zpl_media_channel_area_osd_attr(zpl_media_channel_t *chn, ZPL_MEDIA_OSD_TYPE_E osd, zpl_uint32 pixel);
extern int zpl_media_channel_area_osd_active(zpl_media_channel_t *chn, ZPL_MEDIA_OSD_TYPE_E osd, zpl_bool bactive);

extern zpl_media_area_t * zpl_media_channel_area_lookup(zpl_media_channel_t *chn, ZPL_MEDIA_AREA_E type);
extern int zpl_media_channel_area_add(zpl_media_channel_t *chn, zpl_media_area_t *);
extern int zpl_media_channel_area_del(zpl_media_channel_t *chn, ZPL_MEDIA_AREA_E type);

extern int zpl_media_channel_areaosd_keystr(zpl_media_channel_t *chn, ZPL_MEDIA_OSD_TYPE_E type, char * keystr);
extern int zpl_media_channel_area_osd_alpha(zpl_media_channel_t *chn, ZPL_MEDIA_OSD_TYPE_E type, int alpha, int fg_alpha);
extern int zpl_media_channel_area_osd_point(zpl_media_channel_t *chn, ZPL_MEDIA_OSD_TYPE_E type, zpl_point_t m_point);
extern int zpl_media_channel_area_osd_show(zpl_media_channel_t *chn, ZPL_MEDIA_OSD_TYPE_E type, zpl_int32 x, zpl_int32 y);


extern int zpl_media_channel_area_destroy_all(void *chn);
extern int zpl_media_channel_area_default(void *);

extern int zpl_media_channel_area_config(zpl_media_channel_t *chn, struct vty *vty);

int zpl_media_text_bitmap_text(void);

#ifdef __cplusplus
}
#endif

#endif /* __ZPL_MEDIA_AREA_H__ */
