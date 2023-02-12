/*
 * zpl_vidhal_region.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_VIDHAL_REGION_H__
#define __ZPL_VIDHAL_REGION_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zpl_media.h"
#include <zpl_vidhal.h>


int zpl_vidhal_region_mst_load_bmp(const char *filename, zpl_media_bitmap_t *pstBitmap, 
        zpl_bool bFil, zpl_uint32 u16FilColor, zpl_uint32 enPixelFormat);
int zpl_vidhal_region_load_canvas(const char* filename, zpl_media_bitmap_t* pstBitmap, zpl_bool bFil,
        zpl_uint32 u16FilColor, zpl_video_size_t* pstSize, zpl_uint32 u32Stride, zpl_uint32 enPixelFmt);


int zpl_vidhal_region_set_bitmap(zpl_media_video_hwregion_t *region, zpl_media_bitmap_t *pstBitmap);

int zpl_vidhal_region_update_canvas(zpl_media_video_hwregion_t *region);

int zpl_vidhal_region_attachtochannel(zpl_media_video_hwregion_t *region, zpl_uint32 modeid, zpl_uint32 devid, 
        zpl_uint32 chnid, zpl_bool attach);

int zpl_vidhal_region_update_attribute(zpl_media_video_hwregion_t *region);        
int zpl_vidhal_region_update_channel_attribute(zpl_media_video_hwregion_t *region, zpl_uint32 modeid, 
        zpl_uint32 devid, zpl_uint32 chnid);
int zpl_vidhal_region_channel_show(zpl_media_video_hwregion_t *region, zpl_uint32 modeid, 
        zpl_uint32 devid, zpl_uint32 chnid, zpl_bool show);

int zpl_vidhal_region_create(zpl_media_video_hwregion_t *);
int zpl_vidhal_region_destroy(zpl_media_video_hwregion_t *);


#ifdef __cplusplus
}
#endif
#endif /* __ZPL_VIDHAL_REGION_H__ */
