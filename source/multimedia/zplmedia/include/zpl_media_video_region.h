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

#include <zpl_type.h>
#include <zpl_media.h>


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
    zpl_int32               rng_id;        //ID
    zpl_void			   *area;		    //
    ZPL_HWRGN_TYPE_E        rgn_type;       //
    zpl_int32               rgn_handle;     //描述符
    zpl_media_syschs_t      rgn_chn;        //通道号
    zpl_uint32		        bg_color;		//区域背景颜色 
    zpl_int32               fg_alpha;       //透明度
    zpl_int32               bg_alpha;       //背景透明度
    zpl_int32               rgn_layer;      //层次[0-7]
    zpl_rect_t              m_rect; 
}zpl_media_video_hwregion_t;


extern int zpl_media_video_hwregion_destroy(zpl_media_video_hwregion_t *);
extern zpl_media_video_hwregion_t * zpl_media_video_hwregion_create(zpl_int32 rng_id);

#ifdef ZPL_SHELL_MODULE
extern int zpl_media_video_hwregion_show(void *pvoid);
#endif

extern int zpl_media_video_hwregion_active(zpl_media_video_hwregion_t *hwregion);
extern int zpl_media_video_hwregion_inactive(zpl_media_video_hwregion_t *hwregion);


#ifdef __cplusplus
}
#endif
#endif /* __ZPL_VIDEO_REGION_H__ */
