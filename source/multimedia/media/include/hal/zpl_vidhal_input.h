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



extern int zpl_vidhal_inputchn_crop(zpl_int32 input_pipe, zpl_int32 input_chn, zpl_bool out, zpl_video_size_t cropsize);
extern int zpl_vidhal_inputchn_mirror_flip(zpl_int32 input_pipe, zpl_int32 input_chn, zpl_bool mirror, zpl_bool flip);
extern int zpl_vidhal_inputchn_ldc(zpl_int32 input_pipe, zpl_int32 input_chn, zpl_video_size_t cropsize);//畸形矫正
extern int zpl_vidhal_inputchn_fish_eye(zpl_int32 input_pipe, zpl_int32 input_chn, void * LMF);//鱼眼校正
extern int zpl_vidhal_inputchn_rotation(zpl_int32 input_pipe, zpl_int32 input_chn, zpl_uint32 rotation);
extern int zpl_vidhal_inputchn_rotation_angle(zpl_int32 input_pipe, zpl_int32 input_chn, void *p);
extern int zpl_vidhal_inputchn_spread(zpl_int32 input_pipe, zpl_int32 input_chn, void *p);

extern int zpl_vidhal_inputchn_pipe_frame_recvfrom(zpl_int32 input_pipe, zpl_int32 input_channel, zpl_media_video_inputchn_t *input);
extern int zpl_vidhal_inputchn_pipe_update_fd(zpl_int32 input_pipe, zpl_int32 input_channel, zpl_media_video_inputchn_t *input);


extern int zpl_vidhal_inputchn_create(zpl_int32 input_pipe, zpl_int32 input_channel, zpl_media_video_inputchn_t *);
extern int zpl_vidhal_inputchn_start(zpl_int32 input_pipe, zpl_int32 input_channel, zpl_media_video_inputchn_t *);
extern int zpl_vidhal_inputchn_stop(zpl_int32 input_pipe, zpl_int32 input_channel, zpl_media_video_inputchn_t *);
extern int zpl_vidhal_inputchn_destroy(zpl_int32 input_pipe, zpl_int32 input_channel, zpl_media_video_inputchn_t *);
extern int zpl_vidhal_inputchn_frame_recvfrom(zpl_int32 input_pipe, zpl_int32 input_channel, zpl_media_video_inputchn_t *input);
extern int zpl_vidhal_inputchn_update_fd(zpl_int32 input_pipe, zpl_int32 input_channel, zpl_media_video_inputchn_t *);


#ifdef __cplusplus
}
#endif
#endif /* __ZPL_VIDHAL_INPUT_H__ */
