/*
 * zpl_vidhal_vpss.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_VIDHAL_VPSS_H__
#define __ZPL_VIDHAL_VPSS_H__

#ifdef __cplusplus
extern "C" {
#endif



extern int zpl_vidhal_vpss_crop(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_video_size_t cropsize);//裁剪
extern int zpl_vidhal_vpss_frc(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_uint32 srcframerate, zpl_uint32 dstframerate);   //帧率控制
extern int zpl_vidhal_vpss_sharpen(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_video_size_t cropsize);//锐化
extern int zpl_vidhal_vpss_3DNR(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_video_size_t cropsize);
extern int zpl_vidhal_vpss_scale(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_video_size_t cropsize);//缩放
extern int zpl_vidhal_vpss_ldc(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_video_size_t cropsize);//畸形矫正
extern int zpl_vidhal_vpss_spread(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_video_size_t cropsize);//伸开
extern int zpl_vidhal_vpss_cover(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_video_size_t cropsize);//封面/封面
extern int zpl_vidhal_vpss_overlayex(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_video_size_t cropsize);//覆盖层
extern int zpl_vidhal_vpss_mosaic(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_video_size_t cropsize);//马赛克
extern int zpl_vidhal_vpss_mirror_flip(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_bool mirror, zpl_bool flip);//镜像/翻转
extern int zpl_vidhal_vpss_aspect_ratio(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_video_size_t cropsize);//纵横比
extern int zpl_vidhal_vpss_rotation(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_uint32 rotation);//固定角度旋转、任意角度旋转
extern int zpl_vidhal_vpss_fish_eye(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_video_size_t cropsize);//鱼眼校正
extern int zpl_vidhal_vpss_compression(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_video_size_t cropsize);//压缩解压 


extern int zpl_vidhal_vpss_channel_frame_sendto(zpl_media_video_vpsschn_t *, void *p, zpl_int32 s32MilliSec);
extern int zpl_vidhal_vpss_channel_create(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_media_video_vpsschn_t *);
extern int zpl_vidhal_vpss_channel_destroy(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_media_video_vpsschn_t *);
extern int zpl_vidhal_vpss_channel_start(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_media_video_vpsschn_t *);
extern int zpl_vidhal_vpss_channel_stop(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_media_video_vpsschn_t *);
extern int zpl_vidhal_vpss_channel_update_fd(zpl_media_video_vpsschn_t *);
extern int zpl_vidhal_vpss_channel_frame_recvfrom(zpl_media_video_vpsschn_t *vpss);






#endif /* __ZPL_VIDHAL_VPSS_H__ */
