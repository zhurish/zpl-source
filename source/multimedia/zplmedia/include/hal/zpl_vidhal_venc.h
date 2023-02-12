/*
 * zpl_vidhal_venc.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_VIDHAL_VENC_H__
#define __ZPL_VIDHAL_VENC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zpl_media.h"
#include <zpl_vidhal.h>
#include "zpl_media_buffer.h"



extern int zpl_vidhal_venc_create(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_media_video_encode_t *venc);
extern int zpl_vidhal_venc_reset(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_media_video_encode_t *venc);
extern int zpl_vidhal_venc_start(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_media_video_encode_t *venc);
extern int zpl_vidhal_venc_stop(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_media_video_encode_t *venc);
extern int zpl_vidhal_venc_destroy(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_media_video_encode_t *venc);

extern int zpl_vidhal_venc_update_fd(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_media_video_encode_t *venc);

extern int zpl_vidhal_venc_snap_start(zpl_int32 vencchn, zpl_video_size_t* pstSize, zpl_bool bSupportDCF);
extern int zpl_vidhal_venc_snap_stop(zpl_int32 vencchn);
extern int zpl_vidhal_venc_snap_process(zpl_int32 vencchn, zpl_int32 SnapCnt);


extern int zpl_vidhal_venc_request_IDR(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_media_video_encode_t *venc);
extern int zpl_vidhal_venc_enable_IDR(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_media_video_encode_t *venc, zpl_bool bEnableIDR);
extern int zpl_vidhal_venc_frame_recvfrom(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_media_video_encode_t *venc);
extern int zpl_vidhal_venc_frame_recvfrom_one(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_media_video_encode_t *venc);
extern int zpl_vidhal_venc_frame_sendto(zpl_media_video_encode_t *venc, zpl_int32 id, zpl_int32 vencchn, void *p, zpl_int32 s32MilliSec);

#ifdef __cplusplus
}
#endif
#endif /* __ZPL_VIDHAL_VENC_H__ */
