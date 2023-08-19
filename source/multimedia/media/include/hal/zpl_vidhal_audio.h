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



extern int zpl_audhal_audio_input_create(zpl_audio_input_t *audio);
extern int zpl_audhal_audio_input_destroy(zpl_audio_input_t *audio);
extern int zpl_audhal_audio_input_start(zpl_audio_input_t *audio);
extern int zpl_audhal_audio_input_stop(zpl_audio_input_t *audio);
extern int zpl_audhal_audio_input_update_fd(zpl_audio_input_t *audio);
extern int zpl_audhal_audio_input_frame_recvfrom(void *media_channel, zpl_audio_input_t *audio);
extern int zpl_audhal_audio_encode_frame_recvfrom(void *media_channel, zpl_audio_input_t *audio);
extern int zpl_audhal_audio_encode_frame_sendto(void *media_channel, zpl_audio_input_t *audio, void *p);/*向编码单元发送数据*/

extern int zpl_audhal_audio_output_create(zpl_audio_output_t *audio);
extern int zpl_audhal_audio_output_destroy(zpl_audio_output_t *audio);
extern int zpl_audhal_audio_output_start(zpl_audio_output_t *audio);
extern int zpl_audhal_audio_output_stop(zpl_audio_output_t *audio);
extern int zpl_audhal_audio_decode_finsh(zpl_audio_output_t *audio);
extern int zpl_audhal_audio_output_frame_sendto(void *media_channel, zpl_audio_output_t *audio, void *p);
extern int zpl_audhal_audio_decode_frame_sendto(void *media_channel, zpl_audio_output_t *audio, void *p);/*向解码单元发送数据*/
extern int zpl_audhal_audio_decode_frame_recv(void *media_channel, zpl_audio_output_t *audio, void *p);/*向解码单元读取数据*/
extern int zpl_audhal_audio_output_volume(zpl_media_audio_channel_t *audio, int val);

extern int zpl_audhal_audio_codec_clock_rate(zpl_media_audio_channel_t *, int enSample);
extern int zpl_audhal_audio_codec_input_volume(zpl_media_audio_channel_t *, int val);
extern int zpl_audhal_audio_codec_output_volume(zpl_media_audio_channel_t *, int val);
extern int zpl_audhal_audio_codec_mic_gain_val(zpl_media_audio_channel_t *, int val);
extern int zpl_audhal_audio_codec_boost_val(zpl_media_audio_channel_t *, int val);


#ifdef __cplusplus
}
#endif
#endif /* __ZPL_VIDHAL_AUDIO_CODEC_H__ */
