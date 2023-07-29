/*
 * zpl_video_region.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_VIDEO_REGION_H__
#define __ZPL_VIDEO_REGION_H__

#ifdef __cplusplus
extern "C" {
#endif




/* type of video regions */
typedef enum  {
    ZPL_OVERLAY_RGN = 0, /* video overlay region */
    ZPL_COVER_RGN,
    ZPL_COVEREX_RGN,
    ZPL_OVERLAYEX_RGN,
    ZPL_MOSAIC_RGN,
    ZPL_RGN_BUTT
} ZPL_HWRGN_TYPE_E;


typedef struct 
{
    zpl_void			   *area;		    //
    ZPL_HWRGN_TYPE_E        rgn_type;       //
    zpl_int32               rgn_handle;     //描述符
    zpl_media_syschn_t      rgn_chn;        //通道号
    zpl_uint32		        bg_color;		//区域背景颜色 
    zpl_int32               fg_alpha;       //透明度
    zpl_int32               bg_alpha;       //背景透明度
    zpl_int32               rgn_layer;      //层次[0-7]
    zpl_rect_t              m_rect; 
}zpl_media_video_hwregion_t;


extern int zpl_media_video_hwregion_destroy(zpl_media_video_hwregion_t *);
extern zpl_media_video_hwregion_t * zpl_media_video_hwregion_create(zpl_media_area_t *area, zpl_rect_t m_rect);

extern int zpl_media_video_hwregion_attachtochannel(zpl_media_video_hwregion_t *, zpl_media_channel_t *chn, zpl_rect_t m_rect, zpl_bool attach);
int zpl_media_video_hwregion_show(zpl_media_video_hwregion_t *, zpl_rect_t m_rect, zpl_bool show);
int zpl_media_video_hwregion_set_bitmap(zpl_media_video_hwregion_t *, zpl_rect_t m_rect, zpl_media_bitmap_t *pstBitmap);
int zpl_media_video_hwregion_update_canvas(zpl_media_video_hwregion_t *, zpl_rect_t m_rect);


#ifdef __cplusplus
}
#endif
#endif /* __ZPL_VIDEO_REGION_H__ */
