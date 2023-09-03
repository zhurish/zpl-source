/*
 * zpl_media_channel.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "zpl_media.h"
#include "zpl_media_internal.h"

static LIST *media_channel_list = NULL;
static os_mutex_t *media_channel_mutex = NULL;

static int zpl_media_channel_default_free(zpl_media_channel_t *chn);
static zpl_media_channel_t *zpl_media_channel_lookup_entry(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index,
                                                           zpl_uint32 sseid);

int zpl_media_channel_init(void)
{
	if (media_channel_list == NULL)
	{
		media_channel_list = os_malloc(sizeof(LIST));
		if (media_channel_list)
		{
			lstInitFree(media_channel_list, zpl_media_channel_default_free);
		}
		else
			return ERROR;
	}
	if (media_channel_mutex == NULL)
	{
		media_channel_mutex = os_mutex_name_create("media_channel_mutex");
	}
	return ERROR;
}

int zpl_media_channel_exit(void)
{
	if (media_channel_mutex)
	{
		if (os_mutex_destroy(media_channel_mutex) == OK)
			media_channel_mutex = NULL;
	}
	if (media_channel_list)
	{
		lstFree(media_channel_list);
		media_channel_list = NULL;
	}
	return OK;
}

static int zpl_media_channel_default_free(zpl_media_channel_t *chn)
{
	if (chn)
	{
		if (chn->frame_queue)
            chn->frame_queue = NULL;
	}
	return OK;
}

static int zpl_media_channel_encode_default(zpl_media_channel_t *chn)
{
    zpl_video_assert(chn);
    if(chn->media_type == ZPL_MEDIA_VIDEO)
    {
        if (chn->channel_index == ZPL_MEDIA_CHANNEL_TYPE_MAIN)
            chn->media_param.video_media.codec.format = ZPL_VIDHAL_DEFULT_FORMAT_MAIN;
        else if (chn->channel_index == ZPL_MEDIA_CHANNEL_TYPE_SUB)
            chn->media_param.video_media.codec.format = ZPL_VIDHAL_DEFULT_FORMAT_SUB;
        else if (chn->channel_index == ZPL_MEDIA_CHANNEL_TYPE_SUB1)
            chn->media_param.video_media.codec.format = ZPL_VIDHAL_DEFULT_FORMAT_SUB1;
   
        zpl_media_video_format_resolution(chn->media_param.video_media.codec.format, &chn->media_param.video_media.codec.vidsize); // 视频分辨率

        chn->media_param.video_media.codec.codectype = ZPL_VIDEO_CODEC_H264;          // 编码类型
        chn->media_param.video_media.codec.framerate = ZPL_VIDHAL_DEFULT_FRAMERATE; // 帧率

        if (chn->channel_index == ZPL_MEDIA_CHANNEL_TYPE_MAIN)
            chn->media_param.video_media.codec.bitrate = 8192; // 码率
        else if (chn->channel_index == ZPL_MEDIA_CHANNEL_TYPE_SUB)
            chn->media_param.video_media.codec.bitrate = 4096; // 码率
        else if (chn->channel_index == ZPL_MEDIA_CHANNEL_TYPE_SUB1)
            chn->media_param.video_media.codec.bitrate = 2048; // 码率
        else
            chn->media_param.video_media.codec.bitrate = 2048;
        chn->media_param.video_media.codec.bitrate_type = ZPL_VIDHAL_DEFULT_BITRATE; // 码率类型
        
		if (chn->media_param.video_media.codec.codectype == ZPL_VIDEO_CODEC_H264)
            chn->media_param.video_media.codec.profile = ZPL_VIDEO_CODEC_PROFILE_BASELINE;
        else if (chn->media_param.video_media.codec.codectype == ZPL_VIDEO_CODEC_H265)
            chn->media_param.video_media.codec.profile = ZPL_VIDEO_CODEC_PROFILE_MAIN;

        chn->media_param.video_media.codec.ikey_rate = ZPL_VIDHAL_DEFULT_IKEY_INTERVAL; // I帧间隔

        chn->media_param.video_media.codec.framerate = ZPL_VIDEO_FRAMERATE_DEFAULT; // 帧率
        chn->media_param.video_media.codec.enRcMode = ZPL_VENC_RC_CBR;
        chn->media_param.video_media.codec.gopmode = ZPL_VENC_GOPMODE_NORMALP;
#ifndef ZPL_VIDEO_EXTRADATA_MAXSIZE
		chn->media_param.video_media.extradata.fSEI = NULL;
		chn->media_param.video_media.extradata.fSEISize = 0;
		chn->media_param.video_media.extradata.fVPS = NULL;
		chn->media_param.video_media.extradata.fVPSSize = 0;
		chn->media_param.video_media.extradata.fSPS = NULL;
		chn->media_param.video_media.extradata.fSPSSize = 0;
		chn->media_param.video_media.extradata.fPPS = NULL;
		chn->media_param.video_media.extradata.fPPSSize = 0;
		chn->media_param.video_media.extradata.profileLevelId = 0;
#endif
    }
	else if(chn->media_type == ZPL_MEDIA_AUDIO)
	{
		int frame_size = 0;
    	chn->media_param.audio_media.framerate = ZPL_AUDIO_FRAMERATE_DEFAULT;		//帧率
    	chn->media_param.audio_media.clock_rate	= ZPL_AUDIO_CLOCK_RATE_DEFAULT;     /**< Sampling rate.                 */
    	chn->media_param.audio_media.channel_cnt	= 1;
    	chn->media_param.audio_media.bits_per_sample	= ZPL_AUDIO_BIT_WIDTH_DEFAULT; /*每帧的采样点个数  （1/framerate）* clock_rate */	

		chn->media_param.audio_media.codec.framerate = chn->media_param.audio_media.framerate; // 帧率
		chn->media_param.audio_media.codec.codectype = ZPL_AUDIO_CODEC_G711A;          // 编码类型
		chn->media_param.audio_media.codec.bitrate = 64;                            // 码率
		chn->media_param.audio_media.codec.bitrate_type = 0;                        // 码率类型
		//码率=采样率 x 位深度 x 声道
		//每帧PCM数据大小：PCM Buffersize=采样率*采样时间*采样位深/8*通道数（Bytes）
		/*
		假设音频采样率 = 8000，采样通道 = 2，位深度 = 16，采样间隔 = 20ms
		首先我们计算一秒钟总的数据量，采样间隔采用20ms的话，说明每秒钟需采集50次，这个计算大家应该都懂，那么总的数据量计算为
		一秒钟总的数据量 =8000 * 2*16/8 = 32000
		所以每帧音频数据大小 = 32000/50 = 640
		每个通道样本数 = 640/2 = 320
		*/
		//chn->media_param.audio_media.codec.codectype;		//编码类型
		//chn->media_param.audio_media.codec.framerate;		//帧率
		//chn->media_param.audio_media.codec.bitrate;		//码率
		//chn->media_param.audio_media.codec.bitrate_type;	//码率类型
		//chn->media_param.audio_media.codec.avg_bps;             /**< Average bandwidth in bits/sec  */
		//chn->media_param.audio_media.codec.max_bps;             /**< Maximum bandwidth in bits/sec  */

		chn->media_param.audio_media.codec.clock_rate = chn->media_param.audio_media.clock_rate;     /**< Sampling rate.                 */
		chn->media_param.audio_media.codec.channel_cnt = chn->media_param.audio_media.channel_cnt;    /**< Channel count.                 */
		chn->media_param.audio_media.codec.bits_per_sample = chn->media_param.audio_media.bits_per_sample; /**< Bits/sample in the PCM side    */

		frame_size = (chn->media_param.audio_media.codec.clock_rate * chn->media_param.audio_media.codec.channel_cnt);
		frame_size = frame_size * (8*(chn->media_param.audio_media.codec.bits_per_sample+1))/8;
		frame_size = frame_size/chn->media_param.audio_media.codec.framerate;
		chn->media_param.audio_media.codec.max_frame_size = frame_size;   /**< Maximum frame size             */
	}
    return OK;
}

int zpl_media_channel_count(void)
{
	int ret = 0;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
	ret = lstCount(media_channel_list);
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;
}

int zpl_media_channel_load_default(void)
{
	if (zpl_media_channel_count() == 0)
	{
		zpl_media_channel_create(ZPL_MEDIA_CHANNEL_0, ZPL_MEDIA_CHANNEL_TYPE_MAIN);
		zpl_media_channel_create(ZPL_MEDIA_CHANNEL_0, ZPL_MEDIA_CHANNEL_TYPE_SUB);
		zpl_media_channel_create(ZPL_MEDIA_CHANNEL_AUDIO_0, ZPL_MEDIA_CHANNEL_TYPE_INPUT);
		//zpl_media_channel_create(ZPL_MEDIA_CHANNEL_AUDIO_0, ZPL_MEDIA_CHANNEL_TYPE_OUTPUT);
	}
	return OK;
}


int zpl_media_channel_hal_create(zpl_media_channel_t *chn)
{
    if (chn)
    {
		if(chn->media_type == ZPL_MEDIA_VIDEO)
		{
			int ret = ERROR;
			ZPL_MEDIA_CHANNEL_UNLOCK(chn);
			zpl_media_video_encode_t *video_encode = NULL;
			zpl_media_video_vpsschn_t *video_vpsschn = NULL;
			zpl_media_video_inputchn_t *video_inputchn = NULL;			
			video_encode = chn->media_param.video_media.halparam;
			if(video_encode)
			{
				video_vpsschn = video_encode->source_input;
			}
			if(video_vpsschn)
			{
				video_inputchn = video_vpsschn->source_input;
			}
			zpl_video_assert(video_encode);
			zpl_video_assert(video_vpsschn);
			zpl_video_assert(video_inputchn);
			if (video_inputchn != NULL && video_vpsschn != NULL && video_encode != NULL)
			{
				if(!zpl_media_video_encode_state_check(video_encode, ZPL_MEDIA_STATE_ACTIVE))
					ret = zpl_media_video_encode_hal_create(video_encode);
				else
					ret = OK;	
				if(ret == OK && !zpl_media_video_vpsschn_state_check(video_vpsschn, ZPL_MEDIA_STATE_ACTIVE))
					ret = zpl_media_video_vpsschn_hal_create(video_vpsschn);
				else
					ret = OK;	
				if(ret == OK && !zpl_media_video_inputchn_state_check(video_inputchn, ZPL_MEDIA_STATE_ACTIVE))
					ret = zpl_media_video_inputchn_hal_create(video_inputchn);
				else
					ret = OK;	
			}
			if(ret == OK)
				ZPL_SET_BIT(chn->flags, ZPL_MEDIA_STATE_ACTIVE);
			ZPL_MEDIA_CHANNEL_UNLOCK(chn);	
			return ret;
		}
		else if(chn->media_type == ZPL_MEDIA_AUDIO)
		{
			int ret = OK;
			ZPL_MEDIA_CHANNEL_UNLOCK(chn);
			ZPL_SET_BIT(chn->flags, ZPL_MEDIA_STATE_ACTIVE);
			ZPL_MEDIA_CHANNEL_UNLOCK(chn);	
			return ret;
		}
	}	
	return ERROR;
}

static int zpl_media_channel_lstnode_create(zpl_media_channel_t *chn)
{
    if (chn)
    {
		if(chn->media_type == ZPL_MEDIA_VIDEO)
		{
			zpl_media_hwres_t *hwres = ZPL_MEDIA_HALRES_ID_LOAD(chn->channel, chn->channel_index, ALL);

			zpl_media_video_encode_t *video_encode = NULL;
			zpl_media_video_vpsschn_t *video_vpsschn = NULL;
			zpl_media_video_inputchn_t *video_inputchn = NULL;
			if(hwres == NULL)
				return ERROR;
			if(hwres && hwres->vencchn >= 0)
			{
				video_encode = zpl_media_video_encode_create( hwres->vencchn, zpl_false);
			}
			if (video_encode == NULL)
			{
				zm_msg_error("can not create hal for media channel(%d/%d)", chn->channel, chn->channel_index);
				chn->frame_queue = NULL;
				os_free(chn);
				return ERROR;
			}

			if(hwres->vpssgrp >= 0 && hwres->vpsschn >= 0)
			{
				video_vpsschn = zpl_media_video_vpsschn_lookup( hwres->vpssgrp, hwres->vpsschn);
				if(video_vpsschn == NULL)
					video_vpsschn = zpl_media_video_vpsschn_create( hwres->vpssgrp, hwres->vpsschn, chn->media_param.video_media.codec.vidsize);
			}
			if (video_vpsschn == NULL)
			{
				zm_msg_error("can not create hal for media channel(%d/%d)", chn->channel, chn->channel_index);
				zpl_media_video_encode_destroy(video_encode);
				chn->frame_queue = NULL;
				os_free(chn);
				return ERROR;
			}
			if(hwres->inputchn >= 0 && hwres->inputpipe >= 0 && hwres->devid >= 0 && hwres->mipidev >= 0 && 
				hwres->sensordev >= 0 && hwres->sensortype >= 0 && hwres->ispdev >= 0)
			{
				video_inputchn = zpl_media_video_inputchn_lookup(hwres->devid, hwres->inputpipe, hwres->inputchn);
				if(video_inputchn == NULL)
					video_inputchn = zpl_media_video_inputchn_create(hwres->devid, hwres->inputpipe, hwres->inputchn, chn->media_param.video_media.codec.vidsize);
			}
			if (video_inputchn == NULL)
			{
				zm_msg_error("can not create hal for media channel(%d/%d)", chn->channel, chn->channel_index);
				zpl_media_video_encode_destroy(video_encode);
				zpl_media_video_vpsschn_destroy(video_vpsschn);
				chn->frame_queue = NULL;
				os_free(chn);
				return ERROR;
			}
			video_inputchn->inputdev.snsdev = hwres->sensordev;
			video_inputchn->inputdev.mipidev = hwres->mipidev;
			video_inputchn->inputdev.snstype = hwres->sensortype;
			video_inputchn->t_master = chn->t_master;
			video_inputchn->dest_output = video_vpsschn;
			/* input connect vpss */
			video_inputchn->hwbind = hwres->inputchn_hwconnect;
			

			video_vpsschn->t_master = chn->t_master;
			video_vpsschn->dest_output = video_encode;
			video_vpsschn->source_input = video_inputchn;

			video_vpsschn->input_size.width = video_inputchn->output_size.width;
        	video_vpsschn->input_size.height = video_inputchn->output_size.height;
			
			/* vpss connect encode */
			video_vpsschn->hwbind = hwres->vpsschn_hwconnect;


			video_encode->frame_queue = chn->frame_queue;
			video_encode->media_channel = chn;
			video_encode->t_master = chn->t_master;
			video_encode->pCodec = &chn->media_param.video_media.codec;
			video_encode->source_input = video_vpsschn;

			chn->media_param.video_media.halparam = video_encode;
			zpl_media_video_inputchn_addref(video_inputchn);
			zpl_media_video_vpsschn_addref(video_vpsschn);

			if (ZPL_MEDIA_DEBUG(CHANNEL, EVENT))
			{
				zm_msg_debug("create media channel(%d/%d) bind to encode(%d)", chn->channel, chn->channel_index, video_encode ? video_encode->venc_channel : -1);
				zm_msg_debug("  format:%d %dx%d", chn->media_param.video_media.codec.format, chn->media_param.video_media.codec.vidsize.width, chn->media_param.video_media.codec.vidsize.height);
				zm_msg_debug("    codectype:%d framerate:%d bitrate:%d profile:%d", chn->media_param.video_media.codec.codectype, chn->media_param.video_media.codec.framerate,
											chn->media_param.video_media.codec.bitrate, chn->media_param.video_media.codec.profile);
				zm_msg_debug("    bitrate_type:%d ikey_rate:%d enRcMode:%d gopmode:%d", chn->media_param.video_media.codec.bitrate_type, chn->media_param.video_media.codec.ikey_rate,
											chn->media_param.video_media.codec.enRcMode, chn->media_param.video_media.codec.gopmode);
			}
		}
		else
		{
			zpl_media_audio_channel_t *audio_input = zpl_media_audio_create(chn->channel, chn->media_param.audio_media.clock_rate,
				chn->media_param.audio_media.channel_cnt, chn->media_param.audio_media.bits_per_sample, chn->media_param.audio_media.framerate);
			if(audio_input)
			{
				audio_input->frame_queue = chn->frame_queue;
				audio_input->media_channel = chn;
				audio_input->t_master = chn->t_master;
				memcpy(&audio_input->encode.codec, &chn->media_param.audio_media.codec, sizeof(zpl_audio_codec_t));
				chn->media_param.audio_media.halparam = audio_input;
				zpl_media_audio_param_default(audio_input);
			}
			else
				return ERROR;
		}
		return OK;
	}
	return ERROR;
}


int zpl_media_channel_create(ZPL_MEDIA_CHANNEL_E channel,
							 ZPL_MEDIA_CHANNEL_TYPE_E channel_index)
{
    zpl_media_channel_t *chn = NULL;
    chn = os_malloc(sizeof(zpl_media_channel_t));
	zpl_video_assert(chn);
    if (chn)
    {
		memset(chn, 0, sizeof(zpl_media_channel_t));
		
        chn->channel = channel;
        chn->channel_index = channel_index;
        chn->bindcount = 0;
		if(channel >= ZPL_MEDIA_CHANNEL_AUDIO_0)
		{
			chn->media_type = ZPL_MEDIA_AUDIO;
			chn->media_param.audio_media.enable = zpl_true;
		}
		else
		{
			chn->media_type = ZPL_MEDIA_VIDEO;
			chn->media_param.video_media.enable = zpl_true;
		}
		chn->t_master = tvideo_task.t_master;
		zpl_media_channel_encode_default(chn);

		chn->frame_queue = zpl_skbqueue_create(os_name_format("mediaBufQueue-%d/%d", channel, channel_index), ZPL_MEDIA_BUFQUEUE_SIZE, zpl_false);	

		if (chn->frame_queue == NULL)
		{
			zm_msg_error("can not create buffer for media channel(%d/%d)", channel, channel_index);
			os_free(chn);
			return ERROR;
		}

		zpl_skbqueue_attribute_set(chn->frame_queue, ZPL_SKBQUEUE_FLAGS_LIMIT_MAX);

		if(zpl_media_channel_lstnode_create(chn) != OK)
		{
			zpl_skbqueue_destroy(chn->frame_queue);
			os_free(chn);
			return ERROR;
		}
		if(zpl_media_channel_hal_create(chn) != OK)
		{
			zpl_skbqueue_destroy(chn->frame_queue);
			os_free(chn);
			return ERROR;
		}
		
        if (chn->_mutex == NULL)
        {
            chn->_mutex = os_mutex_name_create("media_channel_mutex");
        }
		if (media_channel_mutex)
			os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
		if (media_channel_list)
			lstAdd(media_channel_list, (NODE *)chn);

		if (media_channel_mutex)
			os_mutex_unlock(media_channel_mutex);
		zpl_media_task_ready(MODULE_MEDIA);	
		return OK;
	}
	return ERROR;
}

int zpl_media_channel_hal_destroy(zpl_media_channel_t *chn)
{
    if (chn)
    {
		if(chn->media_type == ZPL_MEDIA_VIDEO)
		{
			int ret = ERROR;
			ZPL_MEDIA_CHANNEL_LOCK(chn);
			zpl_media_video_encode_t *video_encode = NULL;
			zpl_media_video_vpsschn_t *video_vpsschn = NULL;
			zpl_media_video_inputchn_t *video_inputchn = NULL;			
			video_encode = chn->media_param.video_media.halparam;
			if(video_encode)
			{
				video_vpsschn = video_encode->source_input;
			}
			if(video_vpsschn)
			{
				video_inputchn = video_vpsschn->source_input;
			}
			zpl_video_assert(video_encode);
			zpl_video_assert(video_vpsschn);
			zpl_video_assert(video_inputchn);
			if (video_inputchn != NULL && video_vpsschn != NULL && video_encode != NULL)
			{
				if(zpl_media_video_encode_state_check(video_encode, ZPL_MEDIA_STATE_ACTIVE))
					ret = zpl_media_video_encode_hal_destroy(video_encode);
				else
					ret = OK;	
				if(zpl_media_video_vpsschn_state_check(video_vpsschn, ZPL_MEDIA_STATE_ACTIVE))
					ret = zpl_media_video_vpsschn_hal_destroy(video_vpsschn);
				else
					ret = OK;	
				if(zpl_media_video_inputchn_state_check(video_inputchn, ZPL_MEDIA_STATE_ACTIVE))
					ret = zpl_media_video_inputchn_hal_destroy(video_inputchn);
				else
					ret = OK;	
			}
			if(ret == OK)
				ZPL_CLR_BIT(chn->flags, ZPL_MEDIA_STATE_ACTIVE);
			ZPL_MEDIA_CHANNEL_UNLOCK(chn);				
			return ret;
		}
		else
		{
			int ret = OK;
			ZPL_MEDIA_CHANNEL_LOCK(chn);
			ZPL_CLR_BIT(chn->flags, ZPL_MEDIA_STATE_ACTIVE);
			ZPL_MEDIA_CHANNEL_UNLOCK(chn);				
			return ret;
		}
	}		
	return OK;
}


static int zpl_media_channel_lstnode_destroy(zpl_media_channel_t *chn)
{
	zpl_media_video_encode_t *video_encode = NULL;
	zpl_media_video_vpsschn_t *video_vpsschn = NULL;
	zpl_media_video_inputchn_t *video_inputchn = NULL;
    if (chn)
    {
		if(chn->media_type == ZPL_MEDIA_VIDEO)
		{
			video_encode = chn->media_param.video_media.halparam;
			if(video_encode)
			{
				video_vpsschn = video_encode->source_input;
			}
			if(video_vpsschn)
			{
				video_inputchn = video_vpsschn->source_input;
			}

			if (video_encode != NULL)
			{
				zpl_media_video_encode_destroy(video_encode);
				chn->media_param.video_media.halparam = NULL;
			}
			if (video_vpsschn != NULL)
			{
				zpl_media_video_vpsschn_destroy(video_vpsschn);
			}
			if (video_inputchn != NULL)
			{
				zpl_media_video_inputchn_destroy(video_inputchn);
			}
		}
		else
		{
			zpl_media_audio_channel_t *audio = NULL;	
			audio = chn->media_param.audio_media.halparam;
			if (audio != NULL)
			{
				zpl_media_audio_destroy(audio);
				chn->media_param.audio_media.halparam = NULL;
			}
		}
	}
	return OK;
}

int zpl_media_channel_destroy(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index)
{
	zpl_media_channel_t *chn = NULL;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
	chn = zpl_media_channel_lookup_entry(channel, channel_index, 0);
    if(chn)
	{
		ZPL_MEDIA_CHANNEL_LOCK(chn);
		if(zpl_media_channel_lstnode_destroy(chn) != OK)
		{
			ZPL_MEDIA_CHANNEL_UNLOCK(chn);
			if (media_channel_mutex)
				os_mutex_unlock(media_channel_mutex);
			return ERROR;
		}
		
        lstDelete(media_channel_list, (NODE *)chn);
		if(chn->frame_queue)	
		{
			zpl_skbqueue_destroy(chn->frame_queue);	
			chn->frame_queue = NULL;
		}
		ZPL_MEDIA_CHANNEL_UNLOCK(chn);
        if (os_mutex_destroy(chn->_mutex) == OK)
			chn->_mutex = NULL;
        free(chn);
	}
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return OK;
}

int zpl_media_channel_start(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index)
{
    int ret = ERROR;
    zpl_media_channel_t *chn = zpl_media_channel_lookup(channel, channel_index);
    if (chn)
	{
		if (media_channel_mutex)
			os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
		ZPL_MEDIA_CHANNEL_LOCK(chn);	
		if(!ZPL_TST_BIT(chn->flags, ZPL_MEDIA_STATE_START))
		{
			if(chn->media_type == ZPL_MEDIA_VIDEO)
			{
				zpl_media_video_encode_t *video_encode = NULL;
				zpl_media_video_vpsschn_t *video_vpsschn = NULL;
				zpl_media_video_inputchn_t *video_inputchn = NULL;			
				video_encode = chn->media_param.video_media.halparam;
				if(video_encode)
				{
					video_vpsschn = video_encode->source_input;
				}
				if(video_vpsschn)
				{
					video_inputchn = video_vpsschn->source_input;
				}
				zpl_video_assert(video_encode);
				zpl_video_assert(video_vpsschn);
				zpl_video_assert(video_inputchn);
				if (video_inputchn != NULL && video_vpsschn != NULL && video_encode != NULL)
				{
					zpl_media_video_vpsschn_connect(video_vpsschn->vpss_group, video_vpsschn->vpss_channel, video_encode->venc_channel, video_vpsschn->hwbind);
					
					zpl_media_video_inputchn_connect(video_inputchn, video_vpsschn->vpss_group, video_vpsschn->vpss_channel, video_inputchn->hwbind);

					if(zpl_media_video_encode_state_check(video_encode, ZPL_MEDIA_STATE_ACTIVE))
					{
						if(!zpl_media_video_encode_state_check(video_encode, ZPL_MEDIA_STATE_START))
							ret = zpl_media_video_encode_start(chn->t_master,  video_encode);
						else
							ret = OK;	
					}
					if(ret == OK && zpl_media_video_vpsschn_state_check(video_vpsschn, ZPL_MEDIA_STATE_ACTIVE))
					{
						if(!zpl_media_video_vpsschn_state_check(video_vpsschn, ZPL_MEDIA_STATE_START))
							ret = zpl_media_video_vpsschn_start(chn->t_master,  video_vpsschn);
						else
							ret = OK;	
					}
					if(ret == OK && zpl_media_video_inputchn_state_check(video_inputchn, ZPL_MEDIA_STATE_ACTIVE))
					{
						if(!zpl_media_video_inputchn_state_check(video_inputchn, ZPL_MEDIA_STATE_START))
							ret = zpl_media_video_inputchn_start(chn->t_master,  video_inputchn);
						else
							ret = OK;	
					}
				}
			}
			else if(chn->media_type == ZPL_MEDIA_AUDIO)
			{
				zpl_media_audio_channel_t *audio = NULL;	
				audio = chn->media_param.audio_media.halparam;
				if (audio != NULL)
				{
					if(zpl_media_audio_state_check(audio, ZPL_MEDIA_STATE_ACTIVE))
					{
						if(!zpl_media_audio_state_check(audio, ZPL_MEDIA_STATE_START))
						{
							zpl_media_audio_input_set(audio, 
								chn->media_param.audio_media.clock_rate,
								chn->media_param.audio_media.channel_cnt, 
								chn->media_param.audio_media.bits_per_sample, 
								chn->media_param.audio_media.framerate);
							ret = zpl_media_audio_input_start(chn->t_master,  audio);
						}							
						else
							ret = OK;	
					}
				}
			}
			if(ret == OK)
			{
				ZPL_SET_BIT(chn->flags, ZPL_MEDIA_STATE_START);
			}
			if (ZPL_MEDIA_DEBUG(CHANNEL, EVENT))
			{
				zm_msg_debug("start media channel(%d/%d)", channel, channel_index);
			}
		}
		else
			ret = OK;
		ZPL_MEDIA_CHANNEL_UNLOCK(chn);
		if (media_channel_mutex)
			os_mutex_unlock(media_channel_mutex);
		return ret;
	}
	return ERROR;
}

int zpl_media_channel_stop(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index)
{
    int ret = ERROR;
    zpl_media_channel_t *chn = zpl_media_channel_lookup(channel, channel_index);
    if (chn)
	{
		if (media_channel_mutex)
			os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
		ZPL_MEDIA_CHANNEL_LOCK(chn);	
		if(ZPL_TST_BIT(chn->flags, ZPL_MEDIA_STATE_START))
		{
			if(chn->media_type == ZPL_MEDIA_VIDEO)
			{
				zpl_media_video_encode_t *video_encode = NULL;
				zpl_media_video_vpsschn_t *video_vpsschn = NULL;
				zpl_media_video_inputchn_t *video_inputchn = NULL;			
				video_encode = chn->media_param.video_media.halparam;
				if(video_encode)
				{
					video_vpsschn = video_encode->source_input;
				}
				if(video_vpsschn)
				{
					video_inputchn = video_vpsschn->source_input;
				}
				zpl_video_assert(video_encode);
				zpl_video_assert(video_vpsschn);
				zpl_video_assert(video_inputchn);
				if (video_inputchn != NULL && video_vpsschn != NULL && video_encode != NULL)
				{
					if(zpl_media_video_encode_state_check(video_encode, ZPL_MEDIA_STATE_ACTIVE))
					{
						if(zpl_media_video_encode_state_check(video_encode, ZPL_MEDIA_STATE_START))
							ret = zpl_media_video_encode_stop(video_encode);
						else
							ret = OK;	
					}
					if(ret == OK && zpl_media_video_vpsschn_state_check(video_vpsschn, ZPL_MEDIA_STATE_ACTIVE))
					{
						if(zpl_media_video_vpsschn_state_check(video_vpsschn, ZPL_MEDIA_STATE_START))
							ret = zpl_media_video_vpsschn_stop(video_vpsschn);
						else
							ret = OK;	
					}
					if(ret == OK && zpl_media_video_inputchn_state_check(video_inputchn, ZPL_MEDIA_STATE_ACTIVE))
					{
						if(zpl_media_video_inputchn_state_check(video_inputchn, ZPL_MEDIA_STATE_START))
							ret = zpl_media_video_inputchn_stop(video_inputchn);
						else
							ret = OK;	
					}
				}
			}
			else if(chn->media_type == ZPL_MEDIA_AUDIO)
			{
				zpl_media_audio_channel_t *audio = NULL;	
				audio = chn->media_param.audio_media.halparam;
				if (audio != NULL)
				{
					if(zpl_media_audio_state_check(audio, ZPL_MEDIA_STATE_ACTIVE))
					{
						if(!zpl_media_audio_state_check(audio, ZPL_MEDIA_STATE_START))
							ret = zpl_media_audio_input_stop(audio);
						else
							ret = OK;	
					}
				}
			}
			if(ret == OK)
			{
				ZPL_CLR_BIT(chn->flags, ZPL_MEDIA_STATE_START);
			}
			if (ZPL_MEDIA_DEBUG(CHANNEL, EVENT))
			{
				zm_msg_debug("start media channel(%d/%d)", channel, channel_index);
			}
		}
		ZPL_MEDIA_CHANNEL_UNLOCK(chn);
		if (media_channel_mutex)
			os_mutex_unlock(media_channel_mutex);
		return ret;
	}
	return ERROR;
}

int zpl_media_channel_reset(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, int type)
{
    int ret = ERROR;
    zpl_media_channel_t *chn = zpl_media_channel_lookup(channel, channel_index);
    if (chn)
	{
		if (media_channel_mutex)
			os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
		ZPL_MEDIA_CHANNEL_LOCK(chn);	
		if(ZPL_TST_BIT(chn->flags, ZPL_MEDIA_STATE_START))
		{
			if(chn->media_type == ZPL_MEDIA_VIDEO)
			{
				zpl_media_video_encode_t *video_encode = NULL;
				zpl_media_video_vpsschn_t *video_vpsschn = NULL;
				zpl_media_video_inputchn_t *video_inputchn = NULL;			
				video_encode = chn->media_param.video_media.halparam;
				if(video_encode)
				{
					video_vpsschn = video_encode->source_input;
				}
				if(video_vpsschn)
				{
					video_inputchn = video_vpsschn->source_input;
				}
				zpl_video_assert(video_encode);
				zpl_video_assert(video_vpsschn);
				zpl_video_assert(video_inputchn);
				if (video_inputchn != NULL && video_vpsschn != NULL && video_encode != NULL)
				{
					if(zpl_media_video_encode_state_check(video_encode, ZPL_MEDIA_STATE_ACTIVE))
					{
						ret = zpl_media_video_encode_encode_reset(video_encode);
					}
					if(ret == OK && zpl_media_video_vpsschn_state_check(video_vpsschn, ZPL_MEDIA_STATE_ACTIVE))
					{
						ret = OK;	
					}
					if(ret == OK && zpl_media_video_inputchn_state_check(video_inputchn, ZPL_MEDIA_STATE_ACTIVE))
					{
						ret = OK;	
					}
				}
			}
			else
				ret = OK;
		}
		ZPL_MEDIA_CHANNEL_UNLOCK(chn);
		if (media_channel_mutex)
			os_mutex_unlock(media_channel_mutex);
		return ret;
	}
	return ERROR;
}

static zpl_media_channel_t *zpl_media_channel_lookup_entry(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, 
    zpl_uint32 sseid)
{
	NODE node;
	zpl_media_channel_t *chn = NULL;
	for (chn = (zpl_media_channel_t *)lstFirst(media_channel_list);
		 chn != NULL; chn = (zpl_media_channel_t *)lstNext(&node))
	{
		node = chn->node;
        if (sseid && chn && (zpl_uint32)chn == sseid)
        {
            break;
        }
        else if (chn && chn->channel == channel && chn->channel_index == channel_index)
        {
            break;
        }
	}
	return chn;
}

zpl_media_channel_t *zpl_media_channel_lookup(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index)
{
	zpl_media_channel_t *chn = NULL;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
    chn = zpl_media_channel_lookup_entry( channel, channel_index, 0);
    if (media_channel_mutex)
        os_mutex_unlock(media_channel_mutex);
	return chn;
}

zpl_media_channel_t *zpl_media_channel_lookup_sessionID(zpl_uint32 sessionID)
{
	zpl_media_channel_t *chn = NULL;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
	chn = zpl_media_channel_lookup_entry( -1, -1, sessionID);
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return chn;
}
ZPL_MEDIA_STATE_E zpl_media_channel_state(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index)
{
	ZPL_MEDIA_STATE_E flags = ZPL_MEDIA_STATE_NONE;
	zpl_media_channel_t *chn = NULL;
    if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
	chn = zpl_media_channel_lookup_entry(channel, channel_index, 0);
    if(chn)
    {
        flags = chn->flags;
    }
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return flags;
}

zpl_bool zpl_media_channel_isvideo(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index)
{
    zpl_bool ret = zpl_false;
    zpl_media_channel_t *chn = NULL;
    if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
	chn = zpl_media_channel_lookup_entry(channel, channel_index, 0);
    if(chn)
    {
        if(zpl_media_gettype(chn) == ZPL_MEDIA_VIDEO && zpl_media_getptr(chn)->media_param.video_media.enable && zpl_media_getptr(chn)->media_param.video_media.halparam)
            ret = zpl_true;
    }
    if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;
}

zpl_bool zpl_media_channel_isaudio(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index)
{
    zpl_bool ret = zpl_false;
    zpl_media_channel_t *chn = NULL;
    if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
	chn = zpl_media_channel_lookup_entry(channel, channel_index, 0);
    if(chn)
    {
        if(zpl_media_gettype(chn) == ZPL_MEDIA_VIDEO && zpl_media_getptr(chn)->media_param.audio_media.enable && zpl_media_getptr(chn)->media_param.audio_media.halparam)
            ret = zpl_true;
    }
    if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;
}


int zpl_media_channel_video_codec_get(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_video_codec_t *codec)
{
    int ret = ERROR;
    zpl_media_channel_t *chn = NULL;
    if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
	chn = zpl_media_channel_lookup_entry(channel, channel_index, 0);
    if(chn)
    {
        if(zpl_media_getptr(chn)->media_param.video_media.enable && zpl_media_getptr(chn)->media_param.video_media.halparam)
        {
            if(codec)
                memcpy(codec, &chn->media_param.video_media.codec, sizeof(zpl_video_codec_t));
            ret = OK;
        }
    }
    if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;
}

int zpl_media_channel_audio_codec_get(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_audio_codec_t *codec)
{
    int ret = ERROR;
    zpl_media_channel_t *chn = NULL;
    if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
	chn = zpl_media_channel_lookup_entry(channel, channel_index, 0);
    if(chn)
    {
        if(zpl_media_getptr(chn)->media_param.audio_media.enable && zpl_media_getptr(chn)->media_param.audio_media.halparam)
        {
            if(codec)
                memcpy(codec, &chn->media_param.audio_media.codec, sizeof(zpl_audio_codec_t));
            ret = OK;
        }
    }
    if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;
}

int zpl_media_channel_bindcount_get(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index)
{
    int ret = 0;
    zpl_media_channel_t *chn = NULL;
    if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
	chn = zpl_media_channel_lookup_entry(channel, channel_index, 0);
    if(chn)
    {
        ret = chn->bindcount;
    }
    if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;
}

int zpl_media_channel_bindcount_set(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, int addsub)
{
    int ret = ERROR;
    zpl_media_channel_t *chn = NULL;
    if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
	chn = zpl_media_channel_lookup_entry(channel, channel_index, 0);
    if(chn)
    {
        if(addsub)
            chn->bindcount++;
        else
            chn->bindcount--;
        ret = OK;
    }
    if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;  
}


int zpl_media_channel_client_add(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, 
    zpl_media_buffer_handler cb_handler, void *pUser)
{
	int ret = ERROR;
    zpl_media_channel_t *mchannel = NULL;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
    mchannel = zpl_media_channel_lookup_entry( channel, channel_index, 0);
    if(mchannel)
	    ret = zpl_media_client_add(mchannel->media_client, cb_handler, pUser);
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;	
}

int zpl_media_channel_client_del(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_int32 index)
{
	int ret = ERROR;
    zpl_media_channel_t *mchannel = NULL;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
    mchannel = zpl_media_channel_lookup_entry( channel, channel_index, 0);
    if(mchannel)
	    ret = zpl_media_client_del(mchannel->media_client, index);
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;	
}

int zpl_media_channel_client_start(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_int32 index, zpl_bool start)
{
	int ret = ERROR;
    zpl_media_channel_t *mchannel = NULL;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
    mchannel = zpl_media_channel_lookup_entry( channel, channel_index, 0);
    if(mchannel)
	    ret = zpl_media_client_start(mchannel->media_client, index, start);
	if(ret == OK && start)
	{
		if(mchannel->media_type == ZPL_MEDIA_VIDEO)
			zpl_media_channel_hal_request_IDR(mchannel); /* 请求关键帧 */
	}
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;	
}


int zpl_media_channel_codec_set(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, ZPL_MEDIA_CODEC_E codec)
{
	int ret = ERROR;
    zpl_media_channel_t *mchannel = NULL;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
    mchannel = zpl_media_channel_lookup_entry( channel, channel_index, 0);
    if(mchannel)
	{
		if(mchannel->media_type == ZPL_MEDIA_VIDEO)
		{
			mchannel->media_param.video_media.codec.codectype = codec;
		}
		else if(mchannel->media_type == ZPL_MEDIA_AUDIO)
		{
			mchannel->media_param.audio_media.codec.codectype = codec;
		}
	}
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;		
}

int zpl_media_channel_codec_get(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, ZPL_MEDIA_CODEC_E *codec)
{
	int ret = ERROR;
    zpl_media_channel_t *mchannel = NULL;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
    mchannel = zpl_media_channel_lookup_entry( channel, channel_index, 0);
    if(mchannel)
	{
		if(mchannel->media_type == ZPL_MEDIA_VIDEO)
		{
			if(codec)
				*codec = mchannel->media_param.video_media.codec.codectype;
			ret = OK;	
		}
		else if(mchannel->media_type == ZPL_MEDIA_AUDIO)
		{
			if(codec)
				*codec = mchannel->media_param.audio_media.codec.codectype;
			ret = OK;	
		}
	}
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;		
}

int zpl_media_channel_hal_request_IDR(zpl_media_channel_t *chn)
{
	zpl_media_video_encode_t *video_encode = (zpl_media_video_encode_t*)chn->media_param.video_media.halparam;
    if (video_encode && video_encode->venc_channel >= 0)
    {
        return zpl_media_video_encode_request_IDR(video_encode);
    }
    return ERROR;
}

/*分辨率*/
int zpl_media_channel_video_resolving_set(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, ZPL_VIDEO_FORMAT_E val)
{
	int ret = ERROR;
    zpl_media_channel_t *mchannel = NULL;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
    mchannel = zpl_media_channel_lookup_entry( channel, channel_index, 0);
    if(mchannel)
	{
		if(mchannel->media_type == ZPL_MEDIA_VIDEO)
		{
			if(ZPL_TST_BIT(mchannel->flags, ZPL_MEDIA_STATE_START))
			{
				if (zpl_media_video_encode_stop(mchannel->media_param.video_media.halparam) != OK)
				{
					if (media_channel_mutex)
						os_mutex_unlock(media_channel_mutex);
					return ret;	
				}
			}
			if(ZPL_TST_BIT(mchannel->flags, ZPL_MEDIA_STATE_ACTIVE))
			{

			}
			mchannel->media_param.video_media.codec.format = val;

			if(ZPL_TST_BIT(mchannel->flags, ZPL_MEDIA_STATE_ACTIVE))
			{

			}
			if(ZPL_TST_BIT(mchannel->flags, ZPL_MEDIA_STATE_START))
			{
				if (zpl_media_video_encode_start(mchannel->t_master, mchannel->media_param.video_media.halparam) != OK)
				{
					if (media_channel_mutex)
						os_mutex_unlock(media_channel_mutex);
					return ret;	
				}
			}
			ret = OK;
		}
	}
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;		
}
int zpl_media_channel_video_resolving_get(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, ZPL_VIDEO_FORMAT_E *val)
{
	int ret = ERROR;
    zpl_media_channel_t *mchannel = NULL;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
    mchannel = zpl_media_channel_lookup_entry( channel, channel_index, 0);
    if(mchannel)
	{
		if(mchannel->media_type == ZPL_MEDIA_VIDEO)
		{
			if(val)
				*val = mchannel->media_param.video_media.codec.format;
			ret = OK;	
		}
	}
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;		
}

/*帧率*/
int zpl_media_channel_framerate_set(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_uint32 framerate)
{
	int ret = ERROR;
    zpl_media_channel_t *mchannel = NULL;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
    mchannel = zpl_media_channel_lookup_entry( channel, channel_index, 0);
    if(mchannel)
	{
		if(mchannel->media_type == ZPL_MEDIA_VIDEO)
		{
			mchannel->media_param.video_media.codec.framerate = framerate;
		}
		else if(mchannel->media_type == ZPL_MEDIA_AUDIO)
		{
			mchannel->media_param.audio_media.codec.framerate = framerate;
		}
	}
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;		
}
int zpl_media_channel_framerate_get(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_uint32 *framerate)
{
	int ret = ERROR;
    zpl_media_channel_t *mchannel = NULL;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
    mchannel = zpl_media_channel_lookup_entry( channel, channel_index, 0);
    if(mchannel)
	{
		if(mchannel->media_type == ZPL_MEDIA_VIDEO)
		{
			if(framerate)
				*framerate = mchannel->media_param.video_media.codec.framerate;
			ret = OK;	
		}
		if(mchannel->media_type == ZPL_MEDIA_AUDIO)
		{
			if(framerate)
				*framerate = mchannel->media_param.audio_media.codec.framerate;
			ret = OK;	
		}
	}
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;		
}
/*码率*/
int zpl_media_channel_bitrate_set(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, ZPL_BIT_RATE_E type, zpl_uint32 bitrate)
{
	int ret = ERROR;
    zpl_media_channel_t *mchannel = NULL;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
    mchannel = zpl_media_channel_lookup_entry( channel, channel_index, 0);
    if(mchannel)
	{
		if(mchannel->media_type == ZPL_MEDIA_VIDEO)
		{
			mchannel->media_param.video_media.codec.bitrate = bitrate;
			mchannel->media_param.video_media.codec.bitrate_type = type;
		}
		else if(mchannel->media_type == ZPL_MEDIA_AUDIO)
		{
			mchannel->media_param.audio_media.codec.bitrate = bitrate;
			mchannel->media_param.audio_media.codec.bitrate_type = type;
		}
	}
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;		
}
int zpl_media_channel_bitrate_get(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, ZPL_BIT_RATE_E *type, zpl_uint32 *bitrate)
{
	int ret = ERROR;
    zpl_media_channel_t *mchannel = NULL;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
    mchannel = zpl_media_channel_lookup_entry( channel, channel_index, 0);
    if(mchannel)
	{
		if(mchannel->media_type == ZPL_MEDIA_VIDEO)
		{
			if(bitrate)
				*bitrate = mchannel->media_param.video_media.codec.bitrate;
			if(type)
				*type = mchannel->media_param.video_media.codec.bitrate_type;
			ret = OK;	
		}
		if(mchannel->media_type == ZPL_MEDIA_AUDIO)
		{
			if(bitrate)
				*bitrate = mchannel->media_param.audio_media.codec.bitrate;
			if(type)
				*type = mchannel->media_param.audio_media.codec.bitrate_type;
			ret = OK;	
		}
	}
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;		
}
/*编码等级*/
int zpl_media_channel_video_profile_set(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_uint32	profile)
{
	int ret = ERROR;
    zpl_media_channel_t *mchannel = NULL;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
    mchannel = zpl_media_channel_lookup_entry( channel, channel_index, 0);
    if(mchannel)
	{
		if(mchannel->media_type == ZPL_MEDIA_VIDEO)
		{
			mchannel->media_param.video_media.codec.profile = profile;
		}
	}
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;		
}
int zpl_media_channel_video_profile_get(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_uint32	*profile)
{
	int ret = ERROR;
    zpl_media_channel_t *mchannel = NULL;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
    mchannel = zpl_media_channel_lookup_entry( channel, channel_index, 0);
    if(mchannel)
	{
		if(mchannel->media_type == ZPL_MEDIA_VIDEO)
		{
			if(profile)
				*profile = mchannel->media_param.video_media.codec.profile;
			ret = OK;	
		}
	}
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;		
}
/*I帧间隔*/
int zpl_media_channel_video_ikey_rate_set(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_uint32 ikey_rate)
{
	int ret = ERROR;
    zpl_media_channel_t *mchannel = NULL;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
    mchannel = zpl_media_channel_lookup_entry( channel, channel_index, 0);
    if(mchannel)
	{
		if(mchannel->media_type == ZPL_MEDIA_VIDEO)
		{
			mchannel->media_param.video_media.codec.ikey_rate = ikey_rate;
		}
	}
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;		
}
int zpl_media_channel_video_ikey_rate_get(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_uint32 *ikey_rate)
{
	int ret = ERROR;
    zpl_media_channel_t *mchannel = NULL;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
    mchannel = zpl_media_channel_lookup_entry( channel, channel_index, 0);
    if(mchannel)
	{
		if(mchannel->media_type == ZPL_MEDIA_VIDEO)
		{
			if(ikey_rate)
				*ikey_rate = mchannel->media_param.video_media.codec.ikey_rate;
			ret = OK;	
		}
	}
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;		
}

int zpl_media_channel_video_enRcMode_set(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, ZPL_VENC_RC_E val)
{
	int ret = ERROR;
    zpl_media_channel_t *mchannel = NULL;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
    mchannel = zpl_media_channel_lookup_entry( channel, channel_index, 0);
    if(mchannel)
	{
		if(mchannel->media_type == ZPL_MEDIA_VIDEO)
		{
			mchannel->media_param.video_media.codec.enRcMode = val;
		}
	}
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;		
}
int zpl_media_channel_video_enRcMode_get(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, ZPL_VENC_RC_E *val)
{
	int ret = ERROR;
    zpl_media_channel_t *mchannel = NULL;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
    mchannel = zpl_media_channel_lookup_entry( channel, channel_index, 0);
    if(mchannel)
	{
		if(mchannel->media_type == ZPL_MEDIA_VIDEO)
		{
			if(val)
				*val = mchannel->media_param.video_media.codec.enRcMode;
			ret = OK;	
		}
	}
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;		
}
int zpl_media_channel_video_gopmode_set(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, ZPL_VENC_GOP_MODE_E val)
{
	int ret = ERROR;
    zpl_media_channel_t *mchannel = NULL;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
    mchannel = zpl_media_channel_lookup_entry( channel, channel_index, 0);
    if(mchannel)
	{
		if(mchannel->media_type == ZPL_MEDIA_VIDEO)
		{
			mchannel->media_param.video_media.codec.gopmode = val;
		}
	}
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;		
}
int zpl_media_channel_video_gopmode_get(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, ZPL_VENC_GOP_MODE_E *val)
{
	int ret = ERROR;
    zpl_media_channel_t *mchannel = NULL;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
    mchannel = zpl_media_channel_lookup_entry( channel, channel_index, 0);
    if(mchannel)
	{
		if(mchannel->media_type == ZPL_MEDIA_VIDEO)
		{
			if(val)
				*val = mchannel->media_param.video_media.codec.gopmode;
			ret = OK;	
		}
	}
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;		
}

int zpl_media_channel_foreach(int (*callback)(zpl_media_channel_t *, zpl_void *obj), zpl_void *obj)
{
	NODE node;
	zpl_media_channel_t *chn = NULL;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);

	for (chn = (zpl_media_channel_t *)lstFirst(media_channel_list);
		 chn != NULL; chn = (zpl_media_channel_t *)lstNext(&node))
	{
		node = chn->node;
		if (chn && callback)
		{
			(callback)(chn, obj);
		}
	}
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return OK;
}




#ifdef ZPL_SHELL_MODULE
int zpl_media_channel_extradata_show(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, int brief, struct vty *vty)
{
	NODE *node = NULL;
	zpl_media_channel_t *chn = NULL;
	zpl_video_extradata_t lextradata;
	zpl_char hexformat[2048];
	char *media_typestr[] = {"unknow", "video", "audio"};

	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);

	if (lstCount(media_channel_list))
	{
		vty_out(vty, "-----------------------------------------%s", VTY_NEWLINE);
	}
	for (node = lstFirst(media_channel_list); node != NULL; node = lstNext(node))
	{
		chn = (zpl_media_channel_t *)node;
		if(chn && chn->media_type == ZPL_MEDIA_VIDEO)
		{
			if(channel != ZPL_MEDIA_CHANNEL_NONE && channel_index != ZPL_MEDIA_CHANNEL_TYPE_NONE)
			{
				if(chn->channel == channel && chn->channel_index == channel_index)
				{
					memset(&lextradata, 0, sizeof(zpl_video_extradata_t));
					zpl_media_channel_extradata_get(chn, &lextradata);
					vty_out(vty, "channel            : %d/%d%s", chn->channel, chn->channel_index, VTY_NEWLINE);
					vty_out(vty, " type              : %s%s", media_typestr[chn->media_type], VTY_NEWLINE);
					if (lextradata.fPPSSize)
					{
						vty_out(vty, " PPS Len           : %d%s", lextradata.fPPSSize, VTY_NEWLINE);
						if(brief)
						{              
							memset(hexformat, 0, sizeof(hexformat));
							os_loghex(hexformat, sizeof(hexformat), lextradata.fPPS, lextradata.fPPSSize);
							vty_out(vty, "  PPS Date         : %s%s", hexformat, VTY_NEWLINE);
						}
					}
					if (lextradata.fSPSSize)
					{
						vty_out(vty, " SPS Len           : %d%s", lextradata.fSPSSize, VTY_NEWLINE);
						if(brief)
						{              
							memset(hexformat, 0, sizeof(hexformat));
							os_loghex(hexformat, sizeof(hexformat), lextradata.fSPS, lextradata.fSPSSize);
							vty_out(vty, "  SPS Date         : %s%s", hexformat, VTY_NEWLINE);
						}
					}
					if (lextradata.fVPSSize)
					{
						vty_out(vty, " VPS Len           : %d%s", lextradata.fVPSSize, VTY_NEWLINE);
						if(brief)
						{              
							memset(hexformat, 0, sizeof(hexformat));
							os_loghex(hexformat, sizeof(hexformat), lextradata.fVPS, lextradata.fVPSSize);
							vty_out(vty, "  VPS Date         : %s%s", hexformat, VTY_NEWLINE);
						}
					}
					if (lextradata.fSEISize)
					{
						vty_out(vty, " SEI Len           : %d%s", lextradata.fSEISize, VTY_NEWLINE);
						if(brief)
						{              
							memset(hexformat, 0, sizeof(hexformat));
							os_loghex(hexformat, sizeof(hexformat), lextradata.fSEI, lextradata.fSEISize);
							vty_out(vty, "  SEI Date         : %s%s", hexformat, VTY_NEWLINE);
						}
					}
					break;
				}
			}
			else
			{
				memset(&lextradata, 0, sizeof(zpl_video_extradata_t));
				zpl_media_channel_extradata_get(chn, &lextradata);
				vty_out(vty, "channel            : %d/%d%s", chn->channel, chn->channel_index, VTY_NEWLINE);
				vty_out(vty, " type              : %s%s", media_typestr[chn->media_type], VTY_NEWLINE);
				if (lextradata.fPPSSize)
				{
					vty_out(vty, " PPS Len           : %d%s", lextradata.fPPSSize, VTY_NEWLINE);
					if(brief)
					{              
						memset(hexformat, 0, sizeof(hexformat));
						os_loghex(hexformat, sizeof(hexformat), lextradata.fPPS, lextradata.fPPSSize);
						vty_out(vty, "  PPS Date         : %s%s", hexformat, VTY_NEWLINE);
					}
				}
				if (lextradata.fSPSSize)
				{
					vty_out(vty, " SPS Len           : %d%s", lextradata.fSPSSize, VTY_NEWLINE);
					if(brief)
					{              
						memset(hexformat, 0, sizeof(hexformat));
						os_loghex(hexformat, sizeof(hexformat), lextradata.fSPS, lextradata.fSPSSize);
						vty_out(vty, "  SPS Date         : %s%s", hexformat, VTY_NEWLINE);
					}
				}
				if (lextradata.fVPSSize)
				{
					vty_out(vty, " VPS Len           : %d%s", lextradata.fVPSSize, VTY_NEWLINE);
					if(brief)
					{              
						memset(hexformat, 0, sizeof(hexformat));
						os_loghex(hexformat, sizeof(hexformat), lextradata.fVPS, lextradata.fVPSSize);
						vty_out(vty, "  VPS Date         : %s%s", hexformat, VTY_NEWLINE);
					}
				}
				if (lextradata.fSEISize)
				{
					vty_out(vty, " SEI Len           : %d%s", lextradata.fSEISize, VTY_NEWLINE);
					if(brief)
					{              
						memset(hexformat, 0, sizeof(hexformat));
						os_loghex(hexformat, sizeof(hexformat), lextradata.fSEI, lextradata.fSEISize);
						vty_out(vty, "  SEI Date         : %s%s", hexformat, VTY_NEWLINE);
					}
				}
			}

		}
		else if(chn->media_type == ZPL_MEDIA_AUDIO)
		{

		}	
	}
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return OK;
}



int zpl_media_channel_show(void *pvoid)
{
	NODE *node = NULL;
	zpl_media_channel_t *chn = NULL;
	zpl_media_video_encode_t *encode = NULL;
	char *media_typestr[] = {"unknow", "video", "audio"};
	char *bitrate_typestr[] = {"none", "CBR", "VBR", "ABR"};
	char *enRcMode_typestr[] = {"RC CBR", "RC VBR", "RC AVBR", "RC QVBR", "RC CVBR", "RC QPMAP","RC FIXQP"};
	char *gopmode_typestr[] = {"NORMALP", "DUALP", "SMARTP","ADVSMARTP", "BIPREDB", "LOWDELAYB","BUTT"};
	struct vty *vty = (struct vty *)pvoid;
	int flag = 0;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);

	if (lstCount(media_channel_list))
	{
		vty_out(vty, "-----------------------------------------%s", VTY_NEWLINE);
	}
	for (node = lstFirst(media_channel_list); node != NULL; node = lstNext(node))
	{
		chn = (zpl_media_channel_t *)node;
		if(chn->media_type == ZPL_MEDIA_VIDEO)
		{
			encode = (zpl_media_video_encode_t *)chn->media_param.video_media.halparam;
			vty_out(vty, " -----------------------------------------------%s", VTY_NEWLINE);
			vty_out(vty, " channel            : %d/%d%s", chn->channel, chn->channel_index, VTY_NEWLINE);
			vty_out(vty, "  type              : %s%s", media_typestr[chn->media_type], VTY_NEWLINE);
			vty_out(vty, "  bindcount         : %d%s", chn->bindcount, VTY_NEWLINE);
			vty_out(vty, "  flags             : 0x%x%s", chn->flags, VTY_NEWLINE);
			vty_out(vty, "  size              : %dx%d%s", chn->media_param.video_media.codec.vidsize.width, chn->media_param.video_media.codec.vidsize.height, VTY_NEWLINE);
			vty_out(vty, "  format            : %s%s", zpl_media_format_name(chn->media_param.video_media.codec.format), VTY_NEWLINE);
			vty_out(vty, "  codectype         : %s%s", zpl_media_codec_name(chn->media_param.video_media.codec.codectype), VTY_NEWLINE);
			vty_out(vty, "  framerate         : %d fps%s", chn->media_param.video_media.codec.framerate, VTY_NEWLINE);
			vty_out(vty, "  bitrate           : %d kbps%s", chn->media_param.video_media.codec.bitrate, VTY_NEWLINE);
			vty_out(vty, "  bitrate_type      : %s%s", bitrate_typestr[chn->media_param.video_media.codec.bitrate_type], VTY_NEWLINE);
			vty_out(vty, "  profile           : %d%s", chn->media_param.video_media.codec.profile, VTY_NEWLINE);
			vty_out(vty, "  ikey_rate         : %d%s", chn->media_param.video_media.codec.ikey_rate, VTY_NEWLINE);
			vty_out(vty, "  enRcMode          : %s%s", enRcMode_typestr[chn->media_param.video_media.codec.enRcMode], VTY_NEWLINE);
			vty_out(vty, "  gopmode           : %s%s", gopmode_typestr[chn->media_param.video_media.codec.gopmode], VTY_NEWLINE);
			vty_out(vty, "  packetization_mode: %d%s", chn->media_param.video_media.codec.packetization_mode, VTY_NEWLINE);
			vty_out(vty, "  master            : %p%s", chn->t_master, VTY_NEWLINE);
			flag++;
		}
		else if(chn->media_type == ZPL_MEDIA_AUDIO)
		{
			char *audio_str[2] = {"input", "output"};
			int bits_per_sample[4] = {8, 16, 24, 32};
			//zpl_media_audio_channel_t *audio = (zpl_media_audio_channel_t *)chn->media_param.audio_media.halparam;
			vty_out(vty, " -----------------------------------------------%s", VTY_NEWLINE);
			vty_out(vty, " channel            : %d %s%s", chn->channel-ZPL_MEDIA_CHANNEL_AUDIO_0, audio_str[chn->channel_index-ZPL_MEDIA_CHANNEL_TYPE_INPUT], VTY_NEWLINE);
			vty_out(vty, "  type              : %s%s", media_typestr[chn->media_type], VTY_NEWLINE);
			vty_out(vty, "  bindcount         : %d%s", chn->bindcount, VTY_NEWLINE);
			vty_out(vty, "  flags             : 0x%x%s", chn->flags, VTY_NEWLINE);
			vty_out(vty, "  clock             : %d%s", chn->media_param.audio_media.codec.clock_rate, VTY_NEWLINE);
			vty_out(vty, "  codectype         : %s%s", zpl_media_codec_name(chn->media_param.audio_media.codec.codectype), VTY_NEWLINE);
			vty_out(vty, "  framerate         : %d fps%s", chn->media_param.audio_media.codec.framerate, VTY_NEWLINE);
			vty_out(vty, "  bitrate           : %d kbps%s", chn->media_param.audio_media.codec.bitrate, VTY_NEWLINE);
			vty_out(vty, "  bitrate_type      : %s%s", bitrate_typestr[chn->media_param.audio_media.codec.bitrate_type], VTY_NEWLINE);
			vty_out(vty, "  channel cnt       : %d%s", chn->media_param.audio_media.codec.channel_cnt, VTY_NEWLINE);
			vty_out(vty, "  bits per sample   : %d bit%s", bits_per_sample[chn->media_param.audio_media.codec.bits_per_sample], VTY_NEWLINE);
			vty_out(vty, "  max frame size    : %d Byte%s", chn->media_param.audio_media.codec.max_frame_size, VTY_NEWLINE);
			vty_out(vty, "  master            : %p%s", chn->t_master, VTY_NEWLINE);
			flag++;
		}	
	}
	if(flag)
		vty_out(vty, " -----------------------------------------------%s", VTY_NEWLINE);
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return OK;
}
#endif
