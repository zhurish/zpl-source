
/*
 * zpl_vidhal.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_VIDHAL_H__
#define __ZPL_VIDHAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <zpl_type.h>
#include <zpl_media_format.h>
#include <zpl_media.h>
#include <zpl_video_sys.h>
#ifdef ZPL_HISIMPP_MODULE
#include "zpl_hal_hisi.h"
#endif

int zpl_video_hal_scale(void *inframe, void *outframe, zpl_video_size_t vidsize);

int zpl_video_hal_exchange(void *inframe, zpl_video_size_t vidsize);

/* 把src图片覆盖到 dst 图片的一个位置，*/
int zpl_video_hal_orign(void *srcframe, void *dstframe, zpl_point_t vidpoint);

zpl_uint32  zpl_video_hal_frame_blksize(void *inframe);
int zpl_video_hal_memblock_init(zpl_media_memblock_t *pmembuf, zpl_uint32 blocksize);
int zpl_video_hal_memblock_deinit(zpl_media_memblock_t *pmembuf);

void * zpl_video_hal_frame_clone(void *inframe, zpl_media_memblock_t *memblk);


#ifdef __cplusplus
}
#endif
#endif /* __ZPL_VIDHAL_H__ */
