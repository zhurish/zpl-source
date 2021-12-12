/*
 * zpl_vidhal_input.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_VIDHAL_INPUT_H__
#define __ZPL_VIDHAL_INPUT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zpl_media.h"


extern int zpl_vidhal_input_crop(zpl_int32 input_pipe, zpl_int32 input_chn, zpl_bool out, zpl_video_size_t cropsize);
extern int zpl_vidhal_input_mirror_flip(zpl_int32 input_pipe, zpl_int32 input_chn, zpl_bool mirror, zpl_bool flip);
extern int zpl_vidhal_input_ldc(zpl_int32 input_pipe, zpl_int32 input_chn, zpl_video_size_t cropsize);//畸形矫正
extern int zpl_vidhal_input_fish_eye(zpl_int32 input_pipe, zpl_int32 input_chn, void * LMF);//鱼眼校正
extern int zpl_vidhal_input_rotation(zpl_int32 input_pipe, zpl_int32 input_chn, zpl_uint32 rotation);
extern int zpl_vidhal_input_rotation_angle(zpl_int32 input_pipe, zpl_int32 input_chn, void *p);
extern int zpl_vidhal_input_spread(zpl_int32 input_pipe, zpl_int32 input_chn, void *p);




extern int zpl_vidhal_input_pipe_create(zpl_video_input_pipe_t *);
extern int zpl_vidhal_input_pipe_start(zpl_video_input_pipe_t *);
extern int zpl_vidhal_input_pipe_stop(zpl_video_input_pipe_t *);
extern int zpl_vidhal_input_pipe_destroy(zpl_video_input_pipe_t *);
extern int zpl_vidhal_input_pipe_frame_recvfrom(zpl_video_input_pipe_t *input);
extern int zpl_vidhal_input_pipe_update_fd(zpl_video_input_pipe_t *);

extern int zpl_vidhal_input_channel_create(zpl_video_input_channel_t *);
extern int zpl_vidhal_input_channel_start(zpl_video_input_channel_t *);
extern int zpl_vidhal_input_channel_stop(zpl_video_input_channel_t *);
extern int zpl_vidhal_input_channel_destroy(zpl_video_input_channel_t *);
extern int zpl_vidhal_input_channel_frame_recvfrom(zpl_video_input_channel_t *input);
extern int zpl_vidhal_input_channel_update_fd(zpl_video_input_channel_t *);

#ifdef __cplusplus
}
#endif
#endif /* __ZPL_VIDHAL_INPUT_H__ */
