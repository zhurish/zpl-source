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
static int zpl_media_audio_param_reactive(zpl_media_audio_channel_t *audio);

int zpl_media_audio_start(zpl_void *master, zpl_media_audio_channel_t *audio)
{
	int ret = ERROR;
	zpl_video_assert(audio);
	zpl_video_assert(audio->media_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio->t_master == NULL)
		audio->t_master = master;
	if ((audio && audio->b_input) && !ZPL_TST_BIT(audio->flags, ZPL_MEDIA_STATE_START))
	{
		ret = zpl_audhal_audio_input_start(&audio->audio_param.input);
		if (ret != OK)
		{
			zm_msg_error("start audio input channel(%d) failed", audio->channel);
			zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
			return ret;
		}
	}
	if ((audio && !audio->b_input) && !ZPL_TST_BIT(audio->flags, ZPL_MEDIA_STATE_START))
	{
		ret = zpl_audhal_audio_output_start(&audio->audio_param.output);
		if (ret != OK)
		{
			zm_msg_error("start audio output channel(%d) failed", audio->channel);
			zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
			return ret;
		}
	}
	else
	{
		ret = OK;
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	zpl_media_audio_param_reactive(audio);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (ZPL_MEDIA_DEBUG(ENCODE, EVENT))
	{
		zm_msg_debug("\r\naudio channel(%d) start", audio->channel);
	}
	if (audio && audio->b_input)
	{
		zpl_audhal_audio_input_update_fd(&audio->audio_param.input);
		if (master && audio && audio->audio_param.input.hwbind != ZPL_MEDIA_CONNECT_HW && 
			audio->audio_param.input.encode.t_read == NULL && !ipstack_invalid(audio->audio_param.input.encode.fd))
		{
			//zm_msg_debug("==============add zpl_audhal_audio_encode_frame_recvfrom");
			if(audio->audio_param.input.encode.get_encode_frame == NULL)
				audio->audio_param.input.encode.get_encode_frame = zpl_audhal_audio_encode_frame_recvfrom;
			audio->audio_param.input.encode.t_read = thread_add_read(audio->t_master, zpl_media_audio_read_thread, audio, audio->audio_param.input.encode.fd);
		}
		if (master && audio && audio->audio_param.input.hwbind != ZPL_MEDIA_CONNECT_HW && 
			audio->audio_param.input.t_read == NULL && !ipstack_invalid(audio->audio_param.input.fd))
		{
			//zm_msg_debug("==============add zpl_audhal_audio_input_frame_recvfrom");
			if(audio->audio_param.input.get_input_frame == NULL)
				audio->audio_param.input.get_input_frame = zpl_audhal_audio_input_frame_recvfrom;
			audio->audio_param.input.t_read = thread_add_read(audio->t_master, zpl_media_audio_read_thread, audio, audio->audio_param.input.fd);
		}
	}
	ZPL_SET_BIT(audio->flags, ZPL_MEDIA_STATE_START);
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}

int zpl_media_audio_stop(zpl_media_audio_channel_t *audio)
{
	int ret = ERROR;
	zpl_video_assert(audio);
	zpl_video_assert(audio->media_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio && audio->b_input)
	{
		if(audio->audio_param.input.t_read)
		{
			thread_cancel(audio->audio_param.input.t_read);
			audio->audio_param.input.t_read = NULL;
		}
		if(audio->audio_param.input.encode.t_read)
		{
			thread_cancel(audio->audio_param.input.encode.t_read);
			audio->audio_param.input.encode.t_read = NULL;
		}
	}
	if ((audio && audio->b_input) && ZPL_TST_BIT(audio->flags, ZPL_MEDIA_STATE_START))
	{
		ret = zpl_audhal_audio_input_stop(&audio->audio_param.input);
		if (ret != OK)
		{
			zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
			zm_msg_error("stop audio input channel(%d) failed.", audio->channel);
			return ERROR;
		}
	}
	if ((audio && !audio->b_input) && ZPL_TST_BIT(audio->flags, ZPL_MEDIA_STATE_START))
	{
		ret = zpl_audhal_audio_output_stop(&audio->audio_param.output);
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
		zm_msg_debug("audio channel(%d) is stop", audio->channel);
	}
	ZPL_CLR_BIT(audio->flags, ZPL_MEDIA_STATE_START);
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}

zpl_media_audio_channel_t *zpl_media_audio_create(zpl_int32 channel, zpl_bool b_input)
{
	zpl_media_audio_channel_t *audio = NULL;
	if (zpl_media_audio_lookup(channel, b_input) != NULL)
	{
		return NULL;
	}
	audio = os_malloc(sizeof(zpl_media_audio_channel_t));
	if (audio)
	{
		memset(audio, 0, sizeof(zpl_media_audio_channel_t));
		audio->channel = channel;
		audio->b_input = b_input;
		if (audio && audio->b_input)
		{
			audio->audio_param.input.fd = ipstack_create(IPSTACK_OS);
			audio->audio_param.input.encode.fd = ipstack_create(IPSTACK_OS);
		}

		zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
		zm_msg_debug("audio channel (%d) ", audio->channel);
		zpl_media_global_add(ZPL_MEDIA_GLOAL_AUDIO, audio);
		zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
		return audio;
	}
	return NULL;
}

int zpl_media_audio_hal_create(zpl_media_audio_channel_t *audio)
{
	int ret = ERROR;
	if (!ZPL_TST_BIT(audio->flags, ZPL_MEDIA_STATE_ACTIVE))
	{
		zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
		if((audio && audio->b_input))
		{
			ret = zpl_audhal_audio_input_create(&audio->audio_param.input);
			if(ret == OK)
			{
				if(audio->audio_param.input.hwbind == ZPL_MEDIA_CONNECT_HW)
				{
					ret = zpl_syshal_aenc_bind_ai(audio->audio_param.input.devid, audio->audio_param.input.channel, audio->audio_param.input.encode.channel);
					if(ret != OK)
					{
						zm_msg_error("audio input channel %d/%d band to aenc %d failed.", audio->audio_param.input.devid, audio->audio_param.input.channel, audio->audio_param.input.encode.channel);
						zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
						return ret;
					}
				}
			}
			else
			{
				zm_msg_error("create audio input channel(%d) failed.", audio->channel);
				zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
				return ret;
			}
			if(audio->b_inner_codec_enable)
			{
				ret = zpl_audhal_audio_codec_clock_rate(audio, audio->audio_param.input.clock_rate);
				if(ret != OK)
				{
					zm_msg_error("audio codec init failed.");
					zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
					return ret;
				}
				ret = zpl_media_audio_codec_micgain(audio, audio->micgain); // 0-16
				ret = zpl_media_audio_codec_boost(audio, audio->boost ? 1 : 0);
				ret = zpl_media_audio_codec_input_volume(audio, audio->in_volume); //[19,50]
				ret = zpl_media_audio_codec_output_volume(audio, audio->out_volume - 94); //[-121, 6]
			}
		}
		else	
		{
			ret = zpl_audhal_audio_output_create(&audio->audio_param.output);
			if(ret == OK)
			{
				if(audio->audio_param.output.hwbind == ZPL_MEDIA_CONNECT_HW)
				{
					ret = zpl_syshal_ao_bind_adec(audio->audio_param.output.devid, audio->audio_param.output.channel, audio->audio_param.output.decode.channel);
					if(ret != OK)
					{
						zm_msg_error("audio output channel %d/%d band to adec %d failed.", audio->audio_param.output.devid, audio->audio_param.output.channel, audio->audio_param.output.decode.channel);
						zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
						return ret;
					}
				}
			}
			else
			{
				zm_msg_error("create audio output channel(%d) failed.", audio->channel);
				zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
				return ret;
			}
		}
		ZPL_SET_BIT(audio->flags, ZPL_MEDIA_STATE_ACTIVE);
		if (ZPL_MEDIA_DEBUG(ENCODE, EVENT))
		{
			zm_msg_debug("active audio channel(%d) ret=%d", audio->channel, ret);
		}
		zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
		return ret;
	}
	return ret;
}

int zpl_media_audio_destroy(zpl_media_audio_channel_t *audio)
{
	int ret = -1;
	zpl_video_assert(audio);
	zpl_media_audio_hal_destroy(audio);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio && audio->b_input)
	{
		if((audio->audio_param.input.fd != ZPL_SOCKET_INVALID))
		{
			ipstack_drstroy(audio->audio_param.input.fd);
			audio->audio_param.input.fd = ZPL_SOCKET_INVALID;
		}
		if((audio->audio_param.input.encode.fd != ZPL_SOCKET_INVALID))
		{
			ipstack_drstroy(audio->audio_param.input.encode.fd);
			audio->audio_param.input.encode.fd = ZPL_SOCKET_INVALID;
		}
	}
	zpl_media_global_del(ZPL_MEDIA_GLOAL_AUDIO, audio);
	free(audio);
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}

int zpl_media_audio_hal_destroy(zpl_media_audio_channel_t *audio)
{
	int ret = -1;
	
	zpl_video_assert(audio);
	zpl_video_assert(audio->media_channel);
	
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio && audio->b_input)
	{
		if(audio->audio_param.input.t_read)
		{
			thread_cancel(audio->audio_param.input.t_read);
			audio->audio_param.input.t_read = NULL;
		}
		if(audio->audio_param.input.encode.t_read)
		{
			thread_cancel(audio->audio_param.input.encode.t_read);
			audio->audio_param.input.encode.t_read = NULL;
		}
	}
	if (ZPL_TST_BIT(audio->flags, ZPL_MEDIA_STATE_START))
	{
		if (audio && audio->b_input)
		{
			if(audio->audio_param.input.hwbind == ZPL_MEDIA_CONNECT_HW)
				ret = zpl_syshal_aenc_unbind_ai(audio->audio_param.input.devid, audio->audio_param.input.channel, audio->audio_param.input.encode.channel);
			ret = zpl_audhal_audio_input_stop(&audio->audio_param.input);
		}
		else if (audio && !audio->b_input)
		{
			if(audio->audio_param.output.hwbind == ZPL_MEDIA_CONNECT_HW)
				ret = zpl_syshal_ao_unbind_adec(audio->audio_param.output.devid, audio->audio_param.output.channel, audio->audio_param.output.decode.channel);
			ret = zpl_audhal_audio_output_stop(&audio->audio_param.output);
		}
		if (ret != OK)
		{
			zm_msg_error("stop audio channel(%d) failed.", audio->channel);
			zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
			return ret;
		}
		if (ZPL_MEDIA_DEBUG(ENCODE, EVENT))
		{
			zm_msg_debug("stop audio channel(%d) ret=%d", audio->channel, ret);
		}
		ZPL_CLR_BIT(audio->flags, ZPL_MEDIA_STATE_START);
	}
	if (ZPL_TST_BIT(audio->flags, ZPL_MEDIA_STATE_ACTIVE))
	{
		if (audio && audio->b_input)
			ret = zpl_audhal_audio_input_destroy(&audio->audio_param.input);
		else if (audio && !audio->b_input)
			ret = zpl_audhal_audio_output_destroy(&audio->audio_param.output);		
		if (ret != OK)
		{
			zm_msg_error("destroy audio channel(%d) failed.", audio->channel);
			zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
			return ret;
		}
		ZPL_CLR_BIT(audio->flags, ZPL_MEDIA_STATE_ACTIVE);
		if (ZPL_MEDIA_DEBUG(ENCODE, EVENT))
		{
			zm_msg_debug("inactive audio channel(%d) ret=%d", audio->channel, ret);
		}
	}
	else
	{
		ret = OK;
		if (ZPL_MEDIA_DEBUG(ENCODE, EVENT))
		{
			zm_msg_debug("audio channel(%d) is already inactive", audio->channel);
		}
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}

int zpl_media_audio_param_default(zpl_media_audio_channel_t *audio)
{
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio && audio->b_input)
	{
		zpl_audio_encode_decodec_t *encode = &audio->audio_param.input.encode;
		encode->bEnable = zpl_true;
		encode->devid = 0;
		encode->channel = 0;

		audio->micgain = 12;
		audio->boost = 1;
		audio->in_volume = 30;

		audio->b_inner_codec_enable = zpl_true;
		audio->audio_param.input.bEnable = zpl_true;
		audio->audio_param.input.devid = 0;         //绑定的设备号
		audio->audio_param.input.channel = 0;       //底层通道号
		audio->audio_param.input.bResample = zpl_false;
		audio->audio_param.input.volume = 0;
		audio->audio_param.input.clock_rate = encode->codec.clock_rate;     /**< Sampling rate.                 */
		audio->audio_param.input.channel_cnt = encode->codec.channel_cnt;
		audio->audio_param.input.bits_per_sample = encode->codec.bits_per_sample; /*每帧的采样点个数  （1/framerate）* clock_rate */
		audio->audio_param.input.max_frame_size = encode->codec.max_frame_size;
		audio->audio_param.input.hwbind = ZPL_MEDIA_CONNECT_SW;
		audio->audio_param.input.hw_connect_out = ZPL_MEDIA_CONNECT_SW;
		audio->audio_param.input.input_frame_handle = zpl_audhal_audio_frame_forward_hander;
	}
	else if (audio && !audio->b_input)
	{
		zpl_audio_encode_decodec_t *decode = &audio->audio_param.output.decode;
		decode->bEnable = zpl_true;
		decode->devid = 0;
		decode->channel = 0;
		audio->out_volume = 80;
		audio->b_inner_codec_enable = zpl_true;
		audio->audio_param.output.bEnable = zpl_true;
		audio->audio_param.output.devid = 0;         //绑定的设备号
		audio->audio_param.output.channel = 0;       //底层通道号
		audio->audio_param.output.bResample = zpl_false;
		audio->audio_param.output.volume = 80;
		audio->audio_param.output.clock_rate = decode->codec.clock_rate;     /**< Sampling rate.                 */
		audio->audio_param.output.channel_cnt = decode->codec.channel_cnt;
		audio->audio_param.output.bits_per_sample = decode->codec.bits_per_sample; /*每帧的采样点个数  （1/framerate）* clock_rate */
		audio->audio_param.output.max_frame_size = decode->codec.max_frame_size;
		audio->audio_param.output.hwbind = ZPL_MEDIA_CONNECT_HW; 
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return OK;
}

static int zpl_media_audio_param_reactive(zpl_media_audio_channel_t *audio)
{
	int ret = 0;
	if (audio->b_input && audio->audio_param.input.hw_connect_out)
	{
		if (audio->audio_param.input.output == NULL)
		{
			zpl_media_audio_channel_t *audio_out = zpl_media_audio_lookup(audio->channel, zpl_false);
			if (audio_out)
				audio->audio_param.input.output = &audio_out->audio_param.output;
		}
	}
	if (audio->b_inner_codec_enable)
	{
		ret = zpl_audhal_audio_codec_clock_rate(audio, audio->audio_param.input.clock_rate);
		if (audio->b_input)
		{
			ret = zpl_media_audio_codec_micgain(audio, audio->micgain); // 0-16
			ret = zpl_media_audio_codec_boost(audio, audio->boost ? 1 : 0);
			ret = zpl_media_audio_codec_input_volume(audio, audio->in_volume); //[19,50]
		}
		if (!audio->b_input)
		{
			ret = zpl_media_audio_codec_output_volume(audio, audio->out_volume - 94); //[-121, 6]
			zpl_media_audio_volume(audio, audio->audio_param.output.volume);
		}
	}
	return 0;
}

int zpl_media_audio_connect_encode(zpl_media_audio_channel_t *audio, ZPL_MEDIA_CONNECT_TYPE_E b_enable)
{
	int ret = 0;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio && audio->b_input)
	{
		if(b_enable)
		{		
			if(b_enable == ZPL_MEDIA_CONNECT_HW)
			{
				if(audio->audio_param.input.t_read)
				{
					thread_cancel(audio->audio_param.input.t_read);
					audio->audio_param.input.t_read = NULL;
				}
				ret = zpl_syshal_aenc_bind_ai(audio->audio_param.input.devid, audio->audio_param.input.channel, audio->audio_param.input.encode.channel);
			}
			if(ret == OK)
				audio->audio_param.input.hwbind = b_enable;
		}
		else
		{
			if(audio->audio_param.input.hwbind == ZPL_MEDIA_CONNECT_HW)
				ret = zpl_syshal_aenc_unbind_ai(audio->audio_param.input.devid, audio->audio_param.input.channel, audio->audio_param.input.encode.channel);
			if(ret == OK)
				audio->audio_param.input.hwbind = b_enable;
		}
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}
int zpl_media_audio_connect_decode(zpl_media_audio_channel_t *audio, ZPL_MEDIA_CONNECT_TYPE_E b_enable)
{
	int ret = 0;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio && !audio->b_input)
	{
		if(b_enable)
		{		
			if(b_enable == ZPL_MEDIA_CONNECT_HW)
			{
				ret = zpl_syshal_ao_bind_adec(audio->audio_param.output.devid, audio->audio_param.output.channel, audio->audio_param.output.decode.channel);
			}
			if(ret == OK)
				audio->audio_param.output.hwbind = b_enable;
		}
		else
		{
			if(audio->audio_param.output.hwbind == ZPL_MEDIA_CONNECT_HW)
				ret = zpl_syshal_ao_unbind_adec(audio->audio_param.output.devid, audio->audio_param.output.channel, audio->audio_param.output.decode.channel);
			if(ret == OK)
				audio->audio_param.output.hwbind = b_enable;
		}
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}

int zpl_media_audio_connect_local_output(zpl_media_audio_channel_t *audio, ZPL_MEDIA_CONNECT_TYPE_E b_enable, int chan)
{
	int ret = 0;
	zpl_media_audio_channel_t *audio_out = zpl_media_audio_lookup(chan, zpl_false);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio && audio->b_input && audio_out)
	{
		if(b_enable)
		{
			audio->audio_param.input.output = &audio_out->audio_param.output;
			if(b_enable == ZPL_MEDIA_CONNECT_HW)
				ret = zpl_syshal_ao_bind_ai(audio_out->audio_param.output.devid, audio_out->audio_param.output.channel, audio->audio_param.input.devid, audio->audio_param.input.channel);
			if(ret == OK)
				audio->audio_param.input.hw_connect_out = b_enable;
		}
		else
		{
			if(audio->audio_param.input.hw_connect_out == ZPL_MEDIA_CONNECT_HW)
				ret = zpl_syshal_ao_unbind_ai(audio_out->audio_param.output.devid, audio_out->audio_param.output.channel, audio->audio_param.input.devid, audio->audio_param.input.channel);
			if(ret == OK)
				audio->audio_param.input.hw_connect_out = b_enable;
			audio->audio_param.input.output = NULL;	
		}
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}

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

int zpl_media_audio_thread(zpl_media_audio_channel_t *audio, zpl_bool start)
{
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (start == zpl_false && audio)
	{
		if (audio && audio->b_input)
		{
			if(audio->audio_param.input.t_read)
			{
				thread_cancel(audio->audio_param.input.t_read);
				audio->audio_param.input.t_read = NULL;
			}
			if(audio->audio_param.input.encode.t_read)
			{
				thread_cancel(audio->audio_param.input.encode.t_read);
				audio->audio_param.input.encode.t_read = NULL;
			}
		}
		zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
		return OK;
	}
	if (audio && audio->b_input)
	{
		zpl_audhal_audio_input_update_fd(&audio->audio_param.input);
		if (audio->t_master && audio && audio->audio_param.input.hwbind != ZPL_MEDIA_CONNECT_HW && 
			audio->audio_param.input.encode.t_read == NULL && !ipstack_invalid(audio->audio_param.input.encode.fd))
		{
			if(audio->audio_param.input.encode.get_encode_frame == NULL)
				audio->audio_param.input.encode.get_encode_frame = zpl_audhal_audio_encode_frame_recvfrom;
			audio->audio_param.input.encode.t_read = thread_add_read(audio->t_master, zpl_media_audio_read_thread, audio, audio->audio_param.input.encode.fd);
		}
		if (audio->t_master && audio && audio->audio_param.input.hwbind != ZPL_MEDIA_CONNECT_HW && 
			audio->audio_param.input.t_read == NULL && !ipstack_invalid(audio->audio_param.input.fd))
		{
			if(audio->audio_param.input.get_input_frame == NULL)
				audio->audio_param.input.get_input_frame = zpl_audhal_audio_input_frame_recvfrom;
			audio->audio_param.input.t_read = thread_add_read(audio->t_master, zpl_media_audio_read_thread, audio, audio->audio_param.input.fd);
		}
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return OK;
}

static int zpl_media_audio_cmp(zpl_media_audio_channel_t *audio, zpl_media_gkey_t *gkey)
{
	if (audio->channel == gkey->channel && audio->b_input == gkey->group)
		return 0;
	return 1;
}

static void zpl_media_audio_free(void *pVoid)
{
	zpl_media_audio_channel_t *audio = pVoid;
	if(audio)
	{
		zpl_media_audio_hal_destroy(audio);
		if (audio && audio->b_input)
		{
			if((audio->audio_param.input.fd != ZPL_SOCKET_INVALID))
			{
				ipstack_drstroy(audio->audio_param.input.fd);
				audio->audio_param.input.fd = ZPL_SOCKET_INVALID;
			}
			if((audio->audio_param.input.encode.fd != ZPL_SOCKET_INVALID))
			{
				ipstack_drstroy(audio->audio_param.input.encode.fd);
				audio->audio_param.input.encode.fd = ZPL_SOCKET_INVALID;
			}
		}
		os_free(audio);
	}
	return ;
}
int zpl_media_audio_init(void)
{
	zpl_media_global_gkey_cmpset(ZPL_MEDIA_GLOAL_AUDIO, zpl_media_audio_cmp);
	zpl_media_global_freeset(ZPL_MEDIA_GLOAL_AUDIO, zpl_media_audio_free);
	return OK;
}

zpl_media_audio_channel_t *zpl_media_audio_lookup(zpl_int32 channel, zpl_bool b_input)
{
	zpl_media_audio_channel_t *audio = (zpl_media_audio_channel_t *)zpl_media_global_lookup(ZPL_MEDIA_GLOAL_AUDIO, channel, b_input, 0);
	return audio;
}




int zpl_media_audio_frame_queue_set(zpl_int32 channel, void *frame_queue)
{
	zpl_media_audio_channel_t *audio = (zpl_media_audio_channel_t *)zpl_media_global_lookup(ZPL_MEDIA_GLOAL_AUDIO, channel, 0, 0);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio)
	{
		audio->frame_queue = frame_queue;
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return OK;
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

		if(ipstack_same(_sock, audio->audio_param.input.fd))
			audio->audio_param.input.t_read = NULL;
		if(ipstack_same(_sock, audio->audio_param.input.encode.fd))
			audio->audio_param.input.encode.t_read = NULL;

		if (audio && audio->b_input)
		{
			if(ipstack_same(_sock, audio->audio_param.input.encode.fd) && audio->audio_param.input.encode.get_encode_frame != NULL)
			{
				ret = (audio->audio_param.input.encode.get_encode_frame)(audio->media_channel, &audio->audio_param.input);
			}
			else if(ipstack_same(_sock, audio->audio_param.input.fd) && audio->audio_param.input.get_input_frame != NULL)
			{
				ret = (audio->audio_param.input.get_input_frame)(audio->media_channel, &audio->audio_param.input);
			}
		}

		if (audio->frame_queue && ret > 0)
		{
			zpl_media_event_dispatch_signal(audio->media_channel);
		}
		if (audio && audio->b_input)
		{
			if(ipstack_same(_sock, audio->audio_param.input.fd))
				audio->audio_param.input.t_read = thread_add_read(audio->t_master, zpl_media_audio_read_thread, audio, audio->audio_param.input.fd);
			if(ipstack_same(_sock, audio->audio_param.input.encode.fd))
				audio->audio_param.input.encode.t_read = thread_add_read(audio->t_master, zpl_media_audio_read_thread, audio, audio->audio_param.input.encode.fd);
		}
		zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	}
	return OK;
}

int zpl_media_audio_recv(zpl_media_audio_channel_t *audio,  void *p)
{
	int ret = -1;
	zpl_video_assert(audio);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio && !audio->b_input)
	{
		ret = zpl_audhal_audio_decode_frame_recv(audio->media_channel, &audio->audio_param.output, p);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}

int zpl_media_audio_sendto(zpl_media_audio_channel_t *audio, void *p)
{
	int ret = -1;
	zpl_video_assert(audio);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio && audio->b_input)
	{
		ret = zpl_audhal_audio_encode_frame_sendto(audio->media_channel, &audio->audio_param.input, p);
	}
	else if (audio && !audio->b_input)
	{
		ret = zpl_audhal_audio_output_frame_sendto(audio->media_channel, &audio->audio_param.output, p);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}
int zpl_media_audio_volume(zpl_media_audio_channel_t *audio, int val)
{
	int ret = -1;
	zpl_video_assert(audio);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	ret = zpl_audhal_audio_output_volume(audio, val);
	if(ret == 0)
	{
		if(!audio->b_input)
			audio->audio_param.output.volume = val;
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}
int zpl_media_audio_codec_enable(zpl_media_audio_channel_t *audio, zpl_bool b_enable)
{
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	if (audio && audio->b_input)
	{
		audio->b_inner_codec_enable = b_enable;
		zpl_audhal_audio_codec_clock_rate(audio, audio->audio_param.input.clock_rate);
	}
	else if (audio && !audio->b_input)
	{
		audio->b_inner_codec_enable = b_enable;
		zpl_audhal_audio_codec_clock_rate(audio, audio->audio_param.input.clock_rate);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return OK;
}
int zpl_media_audio_codec_micgain(zpl_media_audio_channel_t *audio, int val)
{
	int ret = -1;
	zpl_video_assert(audio);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	ret = zpl_audhal_audio_codec_mic_gain_val(audio, val);
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}

int zpl_media_audio_codec_boost(zpl_media_audio_channel_t *audio, int val)
{
	int ret = -1;
	zpl_video_assert(audio);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	ret = zpl_audhal_audio_codec_boost_val(audio, val);
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}
int zpl_media_audio_codec_input_volume(zpl_media_audio_channel_t *audio, int val)
{
	int ret = -1;
	zpl_video_assert(audio);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	ret = zpl_audhal_audio_codec_input_volume(audio, val);
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_AUDIO);
	return ret;
}
int zpl_media_audio_codec_output_volume(zpl_media_audio_channel_t *audio, int val)
{
	int ret = -1;
	zpl_video_assert(audio);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_AUDIO);
	ret = zpl_audhal_audio_codec_output_volume(audio, val);
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
			if(audio->b_input)
			{
				vty_out(vty, " -----------------------------------------------%s", VTY_NEWLINE);
				vty_out(vty, " dir               : input%s", VTY_NEWLINE);
				vty_out(vty, " devid             : %d%s", audio->audio_param.input.devid, VTY_NEWLINE);
				vty_out(vty, " channel           : %d%s", audio->audio_param.input.channel, VTY_NEWLINE);
				if(audio->audio_param.input.bResample)
				{
					vty_out(vty, " resample          : %s%s", audio->audio_param.input.bResample?"enable":"disable", VTY_NEWLINE);
					vty_out(vty, " resample rate     : %d%s", audio->audio_param.input.resample_rate, VTY_NEWLINE);
				}
				//vty_out(vty, " Echo              : %s%s", audio->audio_param.input.bEcho?"enable":"disable", VTY_NEWLINE);
				//vty_out(vty, " Noise             : %s%s", audio->audio_param.input.bNoise?"enable":"disable", VTY_NEWLINE);
				vty_out(vty, " volume            : %d%s", audio->audio_param.input.volume, VTY_NEWLINE);
				vty_out(vty, " clock rate        : %d%s", audio->audio_param.input.clock_rate, VTY_NEWLINE);
				vty_out(vty, " bits per sample   : %d%s", bits_per_sample[audio->audio_param.input.bits_per_sample], VTY_NEWLINE);

				vty_out(vty, " encode connect    : %s%s", connect_typestr[audio->audio_param.input.hwbind], VTY_NEWLINE);
				vty_out(vty, " local out connect : %s(%p)%s", connect_typestr[audio->audio_param.input.hw_connect_out], audio->audio_param.input.output, VTY_NEWLINE);

				if(audio->audio_param.input.encode.bEnable)
				{
					vty_out(vty, " Encode            : %s%s", audio->audio_param.input.encode.bEnable?"enable":"disable", VTY_NEWLINE);
					vty_out(vty, "  devid            : %d%s", audio->audio_param.input.encode.devid, VTY_NEWLINE);
					vty_out(vty, "  channel          : %d%s", audio->audio_param.input.encode.channel, VTY_NEWLINE);
					vty_out(vty, "  bitrate          : %d%s", audio->audio_param.input.encode.codec.bitrate, VTY_NEWLINE);
					vty_out(vty, "  bitrate type     : %s%s", bitrate_typestr[audio->audio_param.input.encode.codec.bitrate_type], VTY_NEWLINE);
					vty_out(vty, "  channel_cnt      : %d%s", audio->audio_param.input.encode.codec.channel_cnt, VTY_NEWLINE);
					vty_out(vty, "  clock rate       : %d%s", audio->audio_param.input.encode.codec.clock_rate, VTY_NEWLINE);
					vty_out(vty, "  bits per sample  : %d%s", bits_per_sample[audio->audio_param.input.encode.codec.bits_per_sample], VTY_NEWLINE);
					vty_out(vty, "  frame size       : %d%s", audio->audio_param.input.encode.codec.max_frame_size, VTY_NEWLINE);
				}
				flag++;
			}
			else if(!audio->b_input)
			{
				vty_out(vty, " -----------------------------------------------%s", VTY_NEWLINE);
				vty_out(vty, " dir               : output%s", VTY_NEWLINE);
				vty_out(vty, " devid             : %d%s", audio->audio_param.output.devid, VTY_NEWLINE);
				vty_out(vty, " channel           : %d%s", audio->audio_param.output.channel, VTY_NEWLINE);
				if(audio->audio_param.output.bResample)
				{
					vty_out(vty, " resample          : %s%s", audio->audio_param.output.bResample?"enable":"disable", VTY_NEWLINE);
					vty_out(vty, " resample rate     : %d%s", audio->audio_param.output.resample_rate, VTY_NEWLINE);
				}
				//vty_out(vty, " Echo              : %s%s", audio->audio_param.output.bEcho?"enable":"disable", VTY_NEWLINE);
				//vty_out(vty, " Noise             : %s%s", audio->audio_param.output.bNoise?"enable":"disable", VTY_NEWLINE);
				vty_out(vty, " volume            : %d%s", audio->audio_param.output.volume, VTY_NEWLINE);
				vty_out(vty, " clock rate        : %d%s", audio->audio_param.output.clock_rate, VTY_NEWLINE);
				vty_out(vty, " bits per sample   : %d%s", bits_per_sample[audio->audio_param.output.bits_per_sample], VTY_NEWLINE);

				if(audio->audio_param.output.decode.bEnable)
				{
					vty_out(vty, " Encode            : %s%s", audio->audio_param.output.decode.bEnable?"enable":"disable", VTY_NEWLINE);
					vty_out(vty, "  devid            : %d%s", audio->audio_param.output.decode.devid, VTY_NEWLINE);
					vty_out(vty, "  channel          : %d%s", audio->audio_param.output.decode.channel, VTY_NEWLINE);
					vty_out(vty, "  bitrate          : %d%s", audio->audio_param.output.decode.codec.bitrate, VTY_NEWLINE);
					vty_out(vty, "  bitrate type     : %s%s", bitrate_typestr[audio->audio_param.output.decode.codec.bitrate_type], VTY_NEWLINE);
					vty_out(vty, "  channel_cnt      : %d%s", audio->audio_param.output.decode.codec.channel_cnt, VTY_NEWLINE);
					vty_out(vty, "  clock rate       : %d%s", audio->audio_param.output.decode.codec.clock_rate, VTY_NEWLINE);
					vty_out(vty, "  bits per sample  : %d%s", bits_per_sample[audio->audio_param.output.decode.codec.bits_per_sample], VTY_NEWLINE);
					vty_out(vty, "  frame size       : %d%s", audio->audio_param.input.encode.codec.max_frame_size, VTY_NEWLINE);
				}
				flag++;
			}
		}
	}
	if(flag)
		vty_out(vty, " -----------------------------------------------%s", VTY_NEWLINE);
	if (_mutex)
		os_mutex_unlock(_mutex);
	return OK;
}
#endif
