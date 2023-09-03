/**
 * @file     zpl_audhal_audio.h
 * @brief     : Description
 * @author   zhurish (zhurish@163.com)
 * @version  1.0
 * @date     2023-08-13
 * 
 * @copyright   Copyright (c) 2023 {author}({email}).Co.Ltd. All rights reserved.
 * 
 */
#ifndef __ZPL_VIDHAL_AUDIO_H__
#define __ZPL_VIDHAL_AUDIO_H__

#ifdef __cplusplus
extern "C" {
#endif



extern int zpl_audhal_audio_input_start(zpl_audio_input_t *audio);
extern int zpl_audhal_audio_input_stop(zpl_audio_input_t *audio);
extern int zpl_audhal_audio_input_update_fd(zpl_audio_input_t *audio);
extern int zpl_audhal_audio_get_input_frame(void *media_channel, zpl_audio_input_t *audio, zpl_audio_frame_t *frame);


extern int zpl_audhal_audio_encode_start(zpl_audio_encode_decode_t *audio);
extern int zpl_audhal_audio_encode_stop(zpl_audio_encode_decode_t *audio);
extern int zpl_audhal_audio_encode_update_fd(zpl_audio_encode_decode_t *audio);
extern int zpl_audhal_audio_get_encode_frame(void *media_channel, zpl_audio_encode_decode_t *audio, zpl_audio_frame_t *);
extern int zpl_audhal_audio_encode_frame_sendto(void *media_channel, zpl_audio_encode_decode_t *audio, zpl_audio_frame_t *frame);/*向编码单元发送数据*/


extern int zpl_audhal_audio_output_start(zpl_audio_output_t *audio);
extern int zpl_audhal_audio_output_stop(zpl_audio_output_t *audio);
extern int zpl_audhal_audio_output_frame_sendto(void *media_channel, zpl_audio_output_t *audio, zpl_audio_frame_t *frame);


extern int zpl_audhal_audio_decode_start(zpl_audio_encode_decode_t *audio);
extern int zpl_audhal_audio_decode_stop(zpl_audio_encode_decode_t *audio);
extern int zpl_audhal_audio_decode_finsh(zpl_audio_encode_decode_t *audio);
extern int zpl_audhal_audio_decode_frame_sendto(void *media_channel, zpl_audio_encode_decode_t *audio, zpl_audio_frame_t *frame);/*向解码单元发送数据*/
extern int zpl_audhal_audio_get_decode_frame(void *media_channel, zpl_audio_encode_decode_t *audio, zpl_audio_frame_t *frame);/*向解码单元读取数据*/


extern int zpl_audhal_audio_frame_forward_hander(zpl_audio_input_t *audio, char *p, char *p2, int ti);


extern int zpl_audhal_audio_codec_clock_rate(zpl_media_audio_channel_t *, int enSample);
extern int zpl_audhal_audio_codec_input_volume(zpl_media_audio_channel_t *, int val);
extern int zpl_audhal_audio_codec_output_volume(zpl_media_audio_channel_t *, int val);
extern int zpl_audhal_audio_codec_mic_gain_val(zpl_media_audio_channel_t *, int val);
extern int zpl_audhal_audio_codec_boost_val(zpl_media_audio_channel_t *, int val);


#ifdef __cplusplus
}
#endif
#endif /* __ZPL_VIDHAL_AUDIO_CODEC_H__ */
