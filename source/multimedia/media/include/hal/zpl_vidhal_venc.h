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



extern int zpl_vidhal_venc_create(zpl_media_video_encode_t *venc);
extern int zpl_vidhal_venc_reset(zpl_media_video_encode_t *venc);
extern int zpl_vidhal_venc_start(zpl_media_video_encode_t *venc);
extern int zpl_vidhal_venc_stop(zpl_media_video_encode_t *venc);
extern int zpl_vidhal_venc_destroy(zpl_media_video_encode_t *venc);

extern int zpl_vidhal_venc_update_fd(zpl_media_video_encode_t *venc);

extern int zpl_vidhal_venc_snap_channel_create(zpl_int32 vencchn, zpl_video_size_t* pstSize, zpl_bool bSupportDCF);
extern int zpl_vidhal_venc_snap_channel_destroy(zpl_int32 vencchn);
extern int zpl_vidhal_venc_snap_channel_start(zpl_int32 vencchn, zpl_int32 SnapCnt, void *);


extern int zpl_vidhal_venc_request_IDR(zpl_media_video_encode_t *venc);
extern int zpl_vidhal_venc_enable_IDR(zpl_media_video_encode_t *venc, zpl_bool bEnableIDR);
extern int zpl_vidhal_venc_frame_recvfrom(zpl_media_video_encode_t *venc);
extern int zpl_vidhal_venc_frame_sendto(zpl_media_video_encode_t *venc, zpl_int32 id, zpl_int32 vencchn, void *p, zpl_int32 s32MilliSec);

#ifdef __cplusplus
}
#endif
#endif /* __ZPL_VIDHAL_VENC_H__ */
