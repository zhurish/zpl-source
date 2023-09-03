/*
 * zpl_video_encode.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "zpl_vidhal.h"
#include "zpl_vidhal_internal.h"

static int zpl_media_audio_read_thread(struct thread *audio);


zpl_bool zpl_media_audio_state_check(zpl_media_audio_channel_t *audio, int bit)
{
	zpl_bool ret = zpl_false;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio)
	{
		if (ZPL_TST_BIT(audio->flags, bit))
			ret = zpl_true;
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}


zpl_media_audio_channel_t *zpl_media_audio_create(zpl_int32 channel, zpl_uint32 clock_rate, zpl_uint8 channel_cnt, zpl_uint8 bits_per_sample, zpl_uint32 framerate)
{
	zpl_media_audio_channel_t *audio = NULL;
	if (zpl_media_audio_lookup(channel) != NULL)
	{
		return NULL;
	}
	audio = os_malloc(sizeof(zpl_media_audio_channel_t));
	if (audio)
	{
		memset(audio, 0, sizeof(zpl_media_audio_channel_t));
		audio->channel = channel;

		audio->input.clock_rate = clock_rate;     /**< Sampling rate.                 */
		audio->input.channel_cnt = channel_cnt;
		audio->input.bits_per_sample = bits_per_sample; /*每帧的采样点个数  （1/framerate）* clock_rate */
		audio->input.framerate = framerate;

		audio->input.fd = ipstack_create(IPSTACK_OS);
		audio->encode.fd = ipstack_create(IPSTACK_OS);

		zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
		zm_msg_debug("audio channel (%d) ", audio->channel);
		zpl_media_global_add(ZPL_MEDIA_GLOAL_AUDIO, audio);
		zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
		return audio;
	}
	return NULL;
}

int zpl_media_audio_destroy(zpl_media_audio_channel_t *audio)
{
	int ret = -1;
	zpl_video_assert(audio);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio)
	{
		if((audio->input.fd != ZPL_SOCKET_INVALID))
		{
			ipstack_drstroy(audio->input.fd);
			audio->input.fd = ZPL_SOCKET_INVALID;
		}
		if((audio->encode.fd != ZPL_SOCKET_INVALID))
		{
			ipstack_drstroy(audio->encode.fd);
			audio->encode.fd = ZPL_SOCKET_INVALID;
		}
	}
	zpl_media_global_del(ZPL_MEDIA_GLOAL_AUDIO, audio);
	free(audio);
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}


int zpl_media_audio_param_default(zpl_media_audio_channel_t *audio)
{
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio)
	{
		zpl_audio_encode_decode_t *encode = &audio->encode;
		zpl_audio_encode_decode_t *decode = &audio->decode;

		encode->bEnable = zpl_false;
		encode->devid = 0;
		encode->channel = 0;

		audio->input.micgain = 12;
		audio->input.boost = 1;
		audio->input.in_volume = 30;

		audio->b_codec_active = zpl_false;
		audio->input.bEnable = zpl_true;
		audio->input.devid = 0;         //绑定的设备号
		audio->input.channel = 0;       //底层通道号
		audio->input.clock_rate = encode->codec.clock_rate;     /**< Sampling rate.                 */
		audio->input.channel_cnt = encode->codec.channel_cnt;
		audio->input.bits_per_sample = encode->codec.bits_per_sample; /*每帧的采样点个数  （1/framerate）* clock_rate */
		audio->input.max_frame_size = encode->codec.max_frame_size;
		//audio->input.hwbind = ZPL_MEDIA_CONNECT_SW;
		//audio->input.hw_connect_out = ZPL_MEDIA_CONNECT_NONE;//ZPL_MEDIA_CONNECT_SW;
		audio->input.input_frame_handle = zpl_audhal_audio_frame_forward_hander;


		decode->bEnable = zpl_false;
		decode->devid = 0;
		decode->channel = 0;

		audio->output.out_volume = 80;
		audio->output.bEnable = zpl_true;
		audio->output.devid = 0;         //绑定的设备号
		audio->output.channel = 0;       //底层通道号
		audio->output.clock_rate = decode->codec.clock_rate;     /**< Sampling rate.                 */
		audio->output.channel_cnt = decode->codec.channel_cnt;
		audio->output.bits_per_sample = decode->codec.bits_per_sample; /*每帧的采样点个数  （1/framerate）* clock_rate */
		audio->output.max_frame_size = decode->codec.max_frame_size;
		//audio->output.hwbind = ZPL_MEDIA_CONNECT_HW; 
		//audio->output.hwbind = ZPL_MEDIA_CONNECT_NONE;
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return OK;
}

int zpl_media_audio_input_set(zpl_media_audio_channel_t *audio, int clock_rate, int channel_cnt, int bits_per_sample, int framerate)
{
	int ret = 0;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio)
	{
		if(clock_rate != audio->input.clock_rate)
			audio->b_codec_active = zpl_false;
		audio->input.clock_rate = clock_rate;     /**< Sampling rate.                 */
		audio->input.channel_cnt = channel_cnt;
		audio->input.bits_per_sample = bits_per_sample; /*每帧的采样点个数  （1/framerate）* clock_rate */
		audio->input.framerate = framerate;
		if(clock_rate)
			audio->input.bEnable = zpl_true;
		else
			audio->input.bEnable = zpl_false;
		if(audio->b_codec_active == zpl_false)	
		{
			ret = zpl_audhal_audio_codec_clock_rate(audio, audio->input.clock_rate);
			if(ret == OK)
				audio->b_codec_active = zpl_true;
		}
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;	
}

int zpl_media_audio_encode_set(zpl_media_audio_channel_t *audio, int clock_rate, int channel_cnt, int bits_per_sample, int framerate)
{
	int ret = 0;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio)
	{
		audio->encode.codec.clock_rate = clock_rate;     /**< Sampling rate.                 */
		audio->encode.codec.channel_cnt = channel_cnt;
		audio->encode.codec.bits_per_sample = bits_per_sample; /*每帧的采样点个数  （1/framerate）* clock_rate */
		audio->encode.codec.framerate = framerate;
		if(clock_rate)
			audio->encode.bEnable = zpl_true;
		else
			audio->encode.bEnable = zpl_false;	
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;	
}

int zpl_media_audio_decode_set(zpl_media_audio_channel_t *audio, int clock_rate, int channel_cnt, int bits_per_sample, int framerate)
{
	int ret = 0;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio)
	{
		audio->decode.codec.clock_rate = clock_rate;     /**< Sampling rate.                 */
		audio->decode.codec.channel_cnt = channel_cnt;
		audio->decode.codec.bits_per_sample = bits_per_sample; /*每帧的采样点个数  （1/framerate）* clock_rate */
		audio->decode.codec.framerate = framerate;
		if(clock_rate)
			audio->decode.bEnable = zpl_true;
		else
			audio->decode.bEnable = zpl_false;	
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;	
}

int zpl_media_audio_output_set(zpl_media_audio_channel_t *audio, int clock_rate, int channel_cnt, int bits_per_sample, int framerate)
{
	int ret = 0;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio)
	{
		audio->output.clock_rate = clock_rate;     /**< Sampling rate.                 */
		audio->output.channel_cnt = channel_cnt;
		audio->output.bits_per_sample = bits_per_sample; /*每帧的采样点个数  （1/framerate）* clock_rate */
		//audio->output.framerate = framerate;
		if(clock_rate)
			audio->output.bEnable = zpl_true;
		else
			audio->output.bEnable = zpl_false;	
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;	
}


int zpl_media_audio_input_start(zpl_void *master, zpl_media_audio_channel_t *audio)
{
	int ret = ERROR;
	zpl_video_assert(audio);
	zpl_video_assert(audio->media_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio->t_master == NULL)
		audio->t_master = master;
	if ((audio && audio->input.bEnable) && !ZPL_TST_BIT(audio->flags, ZPL_MEDIA_STATE_START))
	{
		ret = zpl_audhal_audio_input_start(&audio->input);
		if (ret != OK)
		{
			zm_msg_error("start audio input channel(%d) failed", audio->channel);
			zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
			return ret;
		}
		if(audio->b_codec_active == zpl_false)	
		{
			ret = zpl_audhal_audio_codec_clock_rate(audio, audio->input.clock_rate);
			if(ret == OK)
				audio->b_codec_active = zpl_true;
		}
		if(audio->b_codec_active == zpl_true)
		{
			zpl_audhal_audio_codec_input_volume(audio, audio->input.in_volume);
			zpl_audhal_audio_codec_mic_gain_val(audio, audio->input.micgain);
			zpl_audhal_audio_codec_boost_val(audio, audio->input.boost);
		}
	}
	else
	{
		ret = OK;
	}

	if (ZPL_MEDIA_DEBUG(ENCODE, EVENT))
	{
		zm_msg_debug("\r\naudio channel(%d) start", audio->channel);
	}
	if (audio && audio->input.bEnable)
	{
		zpl_audhal_audio_input_update_fd(&audio->input);

		if (master && audio &&
			audio->input.t_read == NULL && !ipstack_invalid(audio->input.fd))
		{
			if(audio->input.get_input_frame == NULL)
				audio->input.get_input_frame = zpl_audhal_audio_get_input_frame;
			audio->input.t_read = thread_add_read(audio->t_master, zpl_media_audio_read_thread, audio, audio->input.fd);
		}
	}
	ZPL_SET_BIT(audio->flags, ZPL_MEDIA_STATE_START);
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}
int zpl_media_audio_encode_start(zpl_void *master, zpl_media_audio_channel_t *audio)
{
	int ret = ERROR;
	zpl_video_assert(audio);
	zpl_video_assert(audio->media_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio->t_master == NULL)
		audio->t_master = master;

	if ((audio && audio->encode.bEnable))
	{
		ret = zpl_audhal_audio_encode_start(&audio->encode);
		if (ret != OK)
		{
			zm_msg_error("start audio encode channel(%d) failed", audio->channel);
			zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
			return ret;
		}
		zpl_audhal_audio_encode_update_fd(&audio->encode);
		if (master && audio &&
			audio->encode.t_read == NULL && !ipstack_invalid(audio->encode.fd))
		{
			if(audio->encode.get_encode_frame == NULL)
				audio->encode.get_encode_frame = zpl_audhal_audio_get_encode_frame;
			audio->encode.t_read = thread_add_read(audio->t_master, zpl_media_audio_read_thread, audio, audio->encode.fd);
		}
	}
	else
	{
		ret = OK;
	}
	if (ZPL_MEDIA_DEBUG(ENCODE, EVENT))
	{
		zm_msg_debug("\r\naudio encode channel(%d) start", audio->channel);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}
int zpl_media_audio_decode_start(zpl_void *master, zpl_media_audio_channel_t *audio)
{
	int ret = ERROR;
	zpl_video_assert(audio);
	zpl_video_assert(audio->media_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio->t_master == NULL)
		audio->t_master = master;

	if ((audio && audio->decode.bEnable))
	{
		ret = zpl_audhal_audio_decode_start(&audio->decode);
		if (ret != OK)
		{
			zm_msg_error("start audio decode channel(%d) failed", audio->channel);
			zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
			return ret;
		}
	}
	else
	{
		ret = OK;
	}
	if (ZPL_MEDIA_DEBUG(ENCODE, EVENT))
	{
		zm_msg_debug("\r\naudio decode channel(%d) start", audio->channel);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}
int zpl_media_audio_output_start(zpl_void *master, zpl_media_audio_channel_t *audio)
{
	int ret = ERROR;
	zpl_video_assert(audio);
	zpl_video_assert(audio->media_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio->t_master == NULL)
		audio->t_master = master;

	if ((audio && audio->output.bEnable))
	{
		ret = zpl_audhal_audio_output_start(&audio->output);
		if (ret != OK)
		{
			zm_msg_error("start audio output channel(%d) failed", audio->channel);
			zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
			return ret;
		}
		zpl_audhal_audio_codec_output_volume(audio, audio->output.out_volume);
	}
	else
	{
		ret = OK;
	}
	if (ZPL_MEDIA_DEBUG(ENCODE, EVENT))
	{
		zm_msg_debug("\r\naudio channel(%d) start", audio->channel);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}

int zpl_media_audio_input_stop(zpl_media_audio_channel_t *audio)
{
	int ret = ERROR;
	zpl_video_assert(audio);
	zpl_video_assert(audio->media_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio && audio->input.bEnable)
	{
		if(audio->input.t_read)
		{
			thread_cancel(audio->input.t_read);
			audio->input.t_read = NULL;
		}
		ret = zpl_audhal_audio_input_stop(&audio->input);
		if (ret != OK)
		{
			zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
			zm_msg_error("stop audio input channel(%d) failed.", audio->channel);
			return ERROR;
		}
	}
	else
		ret = OK;
	if (ZPL_MEDIA_DEBUG(ENCODE, EVENT))
	{
		zm_msg_debug("audio channel(%d) is stop", audio->channel);
	}
	ZPL_CLR_BIT(audio->flags, ZPL_MEDIA_STATE_START);
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}
int zpl_media_audio_encode_stop(zpl_media_audio_channel_t *audio)
{
	int ret = ERROR;
	zpl_video_assert(audio);
	zpl_video_assert(audio->media_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio && audio->encode.bEnable)
	{
		if(audio->encode.t_read)
		{
			thread_cancel(audio->encode.t_read);
			audio->encode.t_read = NULL;
		}
		ret = zpl_audhal_audio_encode_stop(&audio->encode);
		if (ret != OK)
		{
			zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
			zm_msg_error("stop audio encode channel(%d) failed.", audio->channel);
			return ERROR;
		}
	}
	else
		ret = OK;
	if (ZPL_MEDIA_DEBUG(ENCODE, EVENT))
	{
		zm_msg_debug("audio encode channel(%d) is stop", audio->channel);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}
int zpl_media_audio_decode_stop(zpl_media_audio_channel_t *audio)
{
	int ret = ERROR;
	zpl_video_assert(audio);
	zpl_video_assert(audio->media_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio && audio->decode.bEnable)
	{
		ret = zpl_audhal_audio_decode_stop(&audio->decode);
		if (ret != OK)
		{
			zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
			zm_msg_error("stop audio decode channel(%d) failed.", audio->channel);
			return ERROR;
		}
	}
	else
		ret = OK;
	if (ZPL_MEDIA_DEBUG(ENCODE, EVENT))
	{
		zm_msg_debug("audio decode channel(%d) is stop", audio->channel);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}
int zpl_media_audio_output_stop(zpl_media_audio_channel_t *audio)
{
	int ret = ERROR;
	zpl_video_assert(audio);
	zpl_video_assert(audio->media_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);

	if ((audio && audio->output.bEnable))
	{
		ret = zpl_audhal_audio_output_stop(&audio->output);
		if (ret != OK)
		{
			zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
			zm_msg_error("stop audio output channel(%d) failed.", audio->channel);
			return ERROR;
		}
	}
	else
		ret = OK;
	if (ZPL_MEDIA_DEBUG(ENCODE, EVENT))
	{
		zm_msg_debug("audio output channel(%d) is stop", audio->channel);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}

int zpl_media_audio_input_connect_encode(zpl_media_audio_channel_t *audio, ZPL_MEDIA_CONNECT_TYPE_E b_enable)
{
	int ret = 0;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio)
	{
		if(b_enable)
		{		
			audio->input.b_connect_encode = b_enable;
			audio->input.encode = &audio->encode;
		}
		else
		{
			audio->input.b_connect_encode = b_enable;
		}
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}

int zpl_media_audio_output_connect_decode(zpl_media_audio_channel_t *audio, ZPL_MEDIA_CONNECT_TYPE_E b_enable)
{
	int ret = 0;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio)
	{
		if(b_enable)
		{
			audio->output.b_connect_decode = b_enable;
			audio->output.decode = &audio->decode;
		}
		else
		{
			audio->output.b_connect_decode = b_enable;
		}
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}

int zpl_media_audio_input_connect_output(zpl_media_audio_channel_t *audio, ZPL_MEDIA_CONNECT_TYPE_E b_enable)
{
	int ret = 0;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio)
	{
		if(b_enable)
		{
			audio->input.b_connect_output = b_enable;
			audio->input.output = &audio->output;
		}
		else
		{
			audio->input.b_connect_output = b_enable;
		}
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}



static int zpl_media_audio_cmp(zpl_media_audio_channel_t *audio, zpl_media_gkey_t *gkey)
{
	if (audio->channel == gkey->channel)
		return 0;
	return 1;
}

static void zpl_media_audio_free(void *pVoid)
{
	zpl_media_audio_channel_t *audio = pVoid;
	if(audio)
	{
		if((audio->input.fd != ZPL_SOCKET_INVALID))
		{
			ipstack_drstroy(audio->input.fd);
			audio->input.fd = ZPL_SOCKET_INVALID;
		}
		if((audio->encode.fd != ZPL_SOCKET_INVALID))
		{
			ipstack_drstroy(audio->encode.fd);
			audio->encode.fd = ZPL_SOCKET_INVALID;
		}
		os_free(audio);
	}
	return;
}

int zpl_media_audio_init(void)
{
	zpl_media_global_gkey_cmpset(ZPL_MEDIA_GLOAL_AUDIO, zpl_media_audio_cmp);
	zpl_media_global_freeset(ZPL_MEDIA_GLOAL_AUDIO, zpl_media_audio_free);
	return OK;
}

zpl_media_audio_channel_t *zpl_media_audio_lookup(zpl_int32 channel)
{
	zpl_media_audio_channel_t *audio = (zpl_media_audio_channel_t *)zpl_media_global_lookup(ZPL_MEDIA_GLOAL_AUDIO, channel, 0, 0);
	return audio;
}


/* 编码模块接收到数据后完成编码，编码输出数据有这里接收，接收到编码后的数据发送到通道的消息队列，*/
static int zpl_media_audio_read_thread(struct thread *thread)
{
	int ret = 0;
	zpl_media_audio_channel_t *audio = THREAD_ARG(thread);
	zpl_socket_t _sock = THREAD_FD(thread);
	if (audio && audio->media_channel)
	{
		zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);

		if(ipstack_same(_sock, audio->input.fd))
			audio->input.t_read = NULL;
		if(ipstack_same(_sock, audio->encode.fd))
			audio->encode.t_read = NULL;

		if (audio)
		{
			if(ipstack_same(_sock, audio->encode.fd) && audio->encode.get_encode_frame != NULL)
			{
				ret = (audio->encode.get_encode_frame)(audio->media_channel, &audio->input, NULL);
			}
			else if(ipstack_same(_sock, audio->input.fd) && audio->input.get_input_frame != NULL)
			{
				ret = (audio->input.get_input_frame)(audio->media_channel, &audio->input, NULL);
			}
		}
		//编码后的数据才会到用户的消息队列
		if (ipstack_same(_sock, audio->encode.fd) && audio->frame_queue && ret > 0)
		{
			zpl_media_event_dispatch_signal(audio->media_channel, ZPL_MEDIA_GLOAL_AUDIO);
		}
		if (audio)
		{
			if(ipstack_same(_sock, audio->input.fd))
				audio->input.t_read = thread_add_read(audio->t_master, zpl_media_audio_read_thread, audio, audio->input.fd);
			if(ipstack_same(_sock, audio->encode.fd))
				audio->encode.t_read = thread_add_read(audio->t_master, zpl_media_audio_read_thread, audio, audio->encode.fd);
		}
		zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	}
	return OK;
}



int zpl_media_audio_output_frame_write(zpl_media_audio_channel_t *audio,  zpl_audio_frame_t *frame)
{
	int ret = -1;
	zpl_video_assert(audio);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio && audio->output.bEnable)
	{
		ret = zpl_audhal_audio_output_frame_sendto(audio->media_channel, &audio->output, frame);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}

int zpl_media_audio_input_frame_read(zpl_media_audio_channel_t *audio,  zpl_audio_frame_t *frame)
{
	int ret = -1;
	zpl_video_assert(audio);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio && audio->input.bEnable)
	{
		ret = zpl_audhal_audio_get_input_frame(audio->media_channel, &audio->input, frame);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}

int zpl_media_audio_encode_frame_write(zpl_media_audio_channel_t *audio,  zpl_audio_frame_t *frame)
{
	int ret = -1;
	zpl_video_assert(audio);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio && audio->encode.bEnable)
	{
		ret = zpl_audhal_audio_encode_frame_sendto(audio->media_channel, &audio->encode, frame);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}

int zpl_media_audio_encode_frame_read(zpl_media_audio_channel_t *audio,  zpl_audio_frame_t *frame)
{
	int ret = -1;
	zpl_video_assert(audio);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio && audio->encode.bEnable)
	{
		ret = zpl_audhal_audio_get_encode_frame(audio->media_channel, &audio->encode, frame);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}

int zpl_media_audio_decode_frame_write(zpl_media_audio_channel_t *audio,  zpl_audio_frame_t *frame)
{
	int ret = -1;
	zpl_video_assert(audio);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio && audio->decode.bEnable)
	{
		ret = zpl_audhal_audio_decode_frame_sendto(audio->media_channel, &audio->decode, frame);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}

int zpl_media_audio_decode_frame_read(zpl_media_audio_channel_t *audio,  zpl_audio_frame_t *frame)
{
	int ret = -1;
	zpl_video_assert(audio);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio && audio->decode.bEnable)
	{
		ret = zpl_audhal_audio_get_decode_frame(audio->media_channel, &audio->decode, frame);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}


int zpl_media_audio_input_micgain(zpl_media_audio_channel_t *audio, int val)
{
	int ret = -1;
	zpl_video_assert(audio);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	ret = zpl_audhal_audio_codec_mic_gain_val(audio, val);
	if(ret == 0)
	{
		audio->input.micgain = val;
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}

int zpl_media_audio_input_boost(zpl_media_audio_channel_t *audio, zpl_bool val)
{
	int ret = -1;
	zpl_video_assert(audio);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	ret = zpl_audhal_audio_codec_boost_val(audio, val);
	if(ret == 0)
	{
		audio->input.boost = val;
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}
int zpl_media_audio_input_volume(zpl_media_audio_channel_t *audio, int val)
{
	int ret = -1;
	zpl_video_assert(audio);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	ret = zpl_audhal_audio_codec_input_volume(audio, val);
	if(ret == 0)
	{
		audio->input.in_volume = val;
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}
int zpl_media_audio_output_volume(zpl_media_audio_channel_t *audio, int val)
{
	int ret = -1;
	zpl_video_assert(audio);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	ret = zpl_audhal_audio_codec_output_volume(audio, val);
	if(ret == 0)
	{
		audio->output.out_volume = val;
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}

#ifdef ZPL_SHELL_MODULE
int zpl_media_audio_show(void *pvoid)
{
	int flag = 0;
	NODE *node;
	zpl_media_audio_channel_t *audio;
	struct vty *vty = (struct vty *)pvoid;
	LIST *_lst = NULL;
	os_mutex_t *_mutex = NULL;
	char *connect_typestr[] = {"none", "hwbind", "sfconnect"};
	char *bitrate_typestr[] = {"none", "CBR", "VBR", "ABR"};
	int bits_per_sample[4] = {8, 16, 24, 32};
	zpl_media_global_get(ZPL_MEDIA_GLOAL_AUDIO, &_lst, &_mutex);

	if (_lst == NULL)
		return ERROR;
	if (_mutex)
		os_mutex_lock(_mutex, OS_WAIT_FOREVER);

	if (lstCount(_lst))
	{
		vty_out(vty, " -----------------------------------------------%s", VTY_NEWLINE);
	}
	for (node = lstFirst(_lst); node != NULL; node = lstNext(node))
	{
		audio = (zpl_media_audio_channel_t *)node;
		if (node)
		{
			vty_out(vty, "channel            : %d%s", audio->channel, VTY_NEWLINE);
			if (audio->input.bEnable)
			{
				vty_out(vty, " -----------------------------------------------%s", VTY_NEWLINE);
				vty_out(vty, " dir               : input%s", VTY_NEWLINE);
				vty_out(vty, " devid             : %d%s", audio->input.devid, VTY_NEWLINE);
				vty_out(vty, " channel           : %d%s", audio->input.channel, VTY_NEWLINE);
				vty_out(vty, " volume            : %d%s", audio->input.in_volume, VTY_NEWLINE);
				vty_out(vty, " clock rate        : %d%s", audio->input.clock_rate, VTY_NEWLINE);
				vty_out(vty, " bits per sample   : %d%s", bits_per_sample[audio->input.bits_per_sample], VTY_NEWLINE);

				vty_out(vty, " encode connect    : %s%s", connect_typestr[audio->input.b_connect_encode], VTY_NEWLINE);
				vty_out(vty, " output connect : %s(%p)%s", connect_typestr[audio->input.b_connect_output], audio->input.output, VTY_NEWLINE);
				flag++;
			}
			if (audio->encode.bEnable)
			{
				vty_out(vty, " Encode            : %s%s", audio->encode.bEnable ? "enable" : "disable", VTY_NEWLINE);
				vty_out(vty, "  devid            : %d%s", audio->encode.devid, VTY_NEWLINE);
				vty_out(vty, "  channel          : %d%s", audio->encode.channel, VTY_NEWLINE);
				vty_out(vty, "  bitrate          : %d%s", audio->encode.codec.bitrate, VTY_NEWLINE);
				vty_out(vty, "  bitrate type     : %s%s", bitrate_typestr[audio->encode.codec.bitrate_type], VTY_NEWLINE);
				vty_out(vty, "  channel_cnt      : %d%s", audio->encode.codec.channel_cnt, VTY_NEWLINE);
				vty_out(vty, "  clock rate       : %d%s", audio->encode.codec.clock_rate, VTY_NEWLINE);
				vty_out(vty, "  bits per sample  : %d%s", bits_per_sample[audio->encode.codec.bits_per_sample], VTY_NEWLINE);
				vty_out(vty, "  frame size       : %d%s", audio->encode.codec.max_frame_size, VTY_NEWLINE);
			}
			if (audio->output.bEnable)
			{
				vty_out(vty, " -----------------------------------------------%s", VTY_NEWLINE);
				vty_out(vty, " dir               : output%s", VTY_NEWLINE);
				vty_out(vty, " devid             : %d%s", audio->output.devid, VTY_NEWLINE);
				vty_out(vty, " channel           : %d%s", audio->output.channel, VTY_NEWLINE);
				vty_out(vty, " volume            : %d%s", audio->output.out_volume, VTY_NEWLINE);
				vty_out(vty, " clock rate        : %d%s", audio->output.clock_rate, VTY_NEWLINE);
				vty_out(vty, " bits per sample   : %d%s", bits_per_sample[audio->output.bits_per_sample], VTY_NEWLINE);

				flag++;
			}
			if (audio->decode.bEnable)
			{
				vty_out(vty, " Decode            : %s%s", audio->decode.bEnable ? "enable" : "disable", VTY_NEWLINE);
				vty_out(vty, "  devid            : %d%s", audio->decode.devid, VTY_NEWLINE);
				vty_out(vty, "  channel          : %d%s", audio->decode.channel, VTY_NEWLINE);
				vty_out(vty, "  bitrate          : %d%s", audio->decode.codec.bitrate, VTY_NEWLINE);
				vty_out(vty, "  bitrate type     : %s%s", bitrate_typestr[audio->decode.codec.bitrate_type], VTY_NEWLINE);
				vty_out(vty, "  channel_cnt      : %d%s", audio->decode.codec.channel_cnt, VTY_NEWLINE);
				vty_out(vty, "  clock rate       : %d%s", audio->decode.codec.clock_rate, VTY_NEWLINE);
				vty_out(vty, "  bits per sample  : %d%s", bits_per_sample[audio->decode.codec.bits_per_sample], VTY_NEWLINE);
				vty_out(vty, "  frame size       : %d%s", audio->encode.codec.max_frame_size, VTY_NEWLINE);
			}
		}
	}
	if (flag)
		vty_out(vty, " -----------------------------------------------%s", VTY_NEWLINE);
	if (_mutex)
		os_mutex_unlock(_mutex);
	return OK;
}
#endif
