/*
 * zpl_media_channel.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "zpl_media_file.h"
#include "zpl_media_area.h"

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
		chn->media_param.audio_media.codec.framerate = ZPL_AUDIO_FRAMERATE_DEFAULT; // 帧率
		chn->media_param.audio_media.codec.codectype = ZPL_AUDIO_CODEC_PCMU;          // 编码类型
		chn->media_param.audio_media.codec.bitrate = 64;                            // 码率
		chn->media_param.audio_media.codec.bitrate_type = 0;                        // 码率类型
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
		zpl_media_channel_create(0, ZPL_MEDIA_CHANNEL_TYPE_MAIN);
		zpl_media_channel_create(0, ZPL_MEDIA_CHANNEL_TYPE_SUB);
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
		if(ZPL_TST_BIT(chn->flags, ZPL_MEDIA_STATE_START))
		{
			if (zpl_media_video_encode_stop(chn->media_param.video_media.halparam) != OK)
			{
				zm_msg_error("can not stop media channel(%d/%d)", channel,channel_index);
				if (media_channel_mutex)
					os_mutex_unlock(media_channel_mutex);
				return ERROR;
			}
			ZPL_CLR_BIT(chn->flags, ZPL_MEDIA_STATE_START);
		}
		if(ZPL_TST_BIT(chn->flags, ZPL_MEDIA_STATE_ACTIVE))
		{
			if (zpl_media_video_encode_destroy(chn->media_param.video_media.halparam) != OK)
			{
				zm_msg_error("can not destroy media channel(%d/%d)", channel,channel_index);
				if (media_channel_mutex)
					os_mutex_unlock(media_channel_mutex);
				return ERROR;
			}
			ZPL_CLR_BIT(chn->flags, ZPL_MEDIA_STATE_ACTIVE);
		}
        lstDelete(media_channel_list, (NODE *)chn);
        if (chn->frame_queue)
            chn->frame_queue = NULL;
        if (os_mutex_destroy(chn->_mutex) == OK)
			chn->_mutex = NULL;
        free(chn);
	}
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return OK;
}

int zpl_media_channel_create(ZPL_MEDIA_CHANNEL_E channel,
							 ZPL_MEDIA_CHANNEL_TYPE_E channel_index)
{
    zpl_media_channel_t *chn = NULL;
	zpl_media_video_encode_t *encode = NULL;
    chn = os_malloc(sizeof(zpl_media_channel_t));
	zpl_video_assert(chn);
    if (chn)
    {
		int vencchn = 0;
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
		zpl_media_channel_encode_default(chn);
		chn->frame_queue = zpl_media_bufqueue_get(channel, channel_index);	
		if (chn->frame_queue == NULL)
		{
			zm_msg_error("can not create buffer for media channel(%d/%d)", channel, channel_index);
			os_free(chn);
			return ERROR;
		}
		vencchn = 0;//VIDHAL_RES_ID_LOAD(chn->channel, chn->channel_index, VENCCHN);
		if(channel >= ZPL_MEDIA_CHANNEL_AUDIO_0)
		{

		}
		else
		{
			encode = zpl_media_video_encode_create( vencchn, zpl_false);
			if (encode == NULL)
			{
				zm_msg_error("can not create hal for media channel(%d/%d)", channel, channel_index);
				chn->frame_queue = NULL;
				os_free(chn);
				return ERROR;
			}
			chn->media_param.video_media.halparam = encode;
		}
		encode->frame_queue = chn->frame_queue;
		encode->media_channel = chn;
		encode->t_master = chn->t_master;
		if(channel >= ZPL_MEDIA_CHANNEL_AUDIO_0)
		{

		}
		else
		{
			encode->pCodec = &chn->media_param.video_media.codec;
			
			if (ZPL_MEDIA_DEBUG(CHANNEL, EVENT))
			{
				zm_msg_debug("create media channel(%d/%d) bind to encode(%d)", channel, channel_index, encode ? encode->venc_channel : -1);
				zm_msg_debug("  format:%d %dx%d", chn->media_param.video_media.codec.format, chn->media_param.video_media.codec.vidsize.width, chn->media_param.video_media.codec.vidsize.height);
				zm_msg_debug("    codectype:%d framerate:%d bitrate:%d profile:%d", chn->media_param.video_media.codec.codectype, chn->media_param.video_media.codec.framerate,
											chn->media_param.video_media.codec.bitrate, chn->media_param.video_media.codec.profile);
				zm_msg_debug("    bitrate_type:%d ikey_rate:%d enRcMode:%d gopmode:%d", chn->media_param.video_media.codec.bitrate_type, chn->media_param.video_media.codec.ikey_rate,
											chn->media_param.video_media.codec.enRcMode, chn->media_param.video_media.codec.gopmode);
			}
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
		return OK;
	}
	return ERROR;
}

int zpl_media_channel_bind_encode_set(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, void *halparam)
{
	int ret = ERROR;
	zpl_media_channel_t *chn = (zpl_media_channel_t *)zpl_media_channel_lookup(channel, channel_index);
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
	if (chn)
	{
		zpl_media_video_encode_t *encode = halparam;
		if(encode)
		{
			if(chn->media_type == ZPL_MEDIA_VIDEO)
				chn->media_param.video_media.halparam = halparam;
			else
				chn->media_param.audio_media.halparam = halparam;

			encode->frame_queue = chn->frame_queue;
			encode->media_channel = chn;
			encode->t_master = chn->t_master;
			if(chn->media_type == ZPL_MEDIA_VIDEO)
				encode->pCodec = &chn->media_param.video_media.codec;
			else
				encode->pCodec = &chn->media_param.audio_media.codec;	
			ret = OK;
		}
		else
		{
			if(chn->media_type == ZPL_MEDIA_VIDEO && chn->media_param.video_media.halparam)
			{
				encode = chn->media_param.video_media.halparam;
				encode->media_channel = NULL;
				encode->frame_queue = NULL;
				encode->t_master = NULL;
				encode->pCodec = NULL;
				chn->media_param.video_media.halparam = NULL;
				ret = OK;
			}
			else if(chn->media_type == ZPL_MEDIA_AUDIO && chn->media_param.audio_media.halparam)
			{
				encode = chn->media_param.audio_media.halparam;
				encode->media_channel = NULL;
				encode->frame_queue = NULL;
				encode->t_master = NULL;
				encode->pCodec = NULL;
				chn->media_param.audio_media.halparam = NULL;
				ret = OK;
			}
		}
	}
    if (media_channel_mutex)
        os_mutex_unlock(media_channel_mutex);
	return ret;
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

zpl_media_channel_t * zpl_media_channel_lookup_bind(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index)
{
	zpl_media_channel_t *chn = NULL;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
    chn = zpl_media_channel_lookup_entry( channel, channel_index, 0);
    if (media_channel_mutex)
        os_mutex_unlock(media_channel_mutex);
	return chn?chn->bind_other:NULL;
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
    zpl_int32 venc_channel = 0;//VIDHAL_RES_ID_LOAD(chn->channel, chn->channel_index, VENCCHN);
    if (venc_channel >= 0)
    {
        zpl_media_video_encode_t *video_encode = zpl_media_video_encode_lookup(venc_channel, 0);
        if (video_encode)
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

int zpl_media_channel_load(zpl_void *obj)
{
	zpl_media_channel_t *chn = obj;
	zpl_video_assert(chn);
	if (chn)
	{
		chn->frame_queue = zpl_media_bufqueue_get(chn->channel, chn->channel_index);
		if (chn->frame_queue == NULL)
		{
			os_free(chn);
			return ERROR;
		}
		//if (zpl_media_channel_hal_create(chn, chn->frame_queue) != OK)
		{
            zm_msg_error("can not create hal media channel(%d/%d)", chn->channel, chn->channel_index);
			chn->frame_queue = NULL;
			os_free(chn);
			return ERROR;
		}
		if (media_channel_mutex)
			os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
		if (media_channel_list)
			lstAdd(media_channel_list, (NODE *)chn);
		if (media_channel_mutex)
			os_mutex_unlock(media_channel_mutex);
		return OK;
	}
	return ERROR;
}

int zpl_media_channel_halparam_set(ZPL_MEDIA_CHANNEL_E channel,
								   ZPL_MEDIA_CHANNEL_TYPE_E channel_index, void *halparam)
{
	zpl_media_channel_t *chn = zpl_media_channel_lookup(channel, channel_index);
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
	if (chn)
	{
		if(chn->media_type == ZPL_MEDIA_VIDEO)
			chn->media_param.video_media.halparam = halparam;
		else
			chn->media_param.audio_media.halparam = halparam;	
	}
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return OK;
}



int zpl_media_channel_active(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index)
{
	zpl_media_channel_t *chn = zpl_media_channel_lookup(channel, channel_index);
	if (chn)
	{
		if (media_channel_mutex)
			os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
		if(!ZPL_SET_BIT(chn->flags, ZPL_MEDIA_STATE_ACTIVE))	
		{
			if (zpl_media_video_encode_hal_create(chn->media_param.video_media.halparam) != OK)
			{
				zm_msg_error("can not active media channel(%d/%d)", channel,channel_index);
				if (media_channel_mutex)
					os_mutex_unlock(media_channel_mutex);
				return ERROR;
			}
			if (ZPL_MEDIA_DEBUG(CHANNEL, EVENT))
			{
				zm_msg_debug("active media channel(%d/%d)", channel, channel_index);
			}
			ZPL_SET_BIT(chn->flags, ZPL_MEDIA_STATE_ACTIVE);
		}
		if (media_channel_mutex)
			os_mutex_unlock(media_channel_mutex);
		return OK;
	}
	return ERROR;
}

int zpl_media_channel_start(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index)
{
    int ret = ERROR;
    zpl_media_channel_t *chn = zpl_media_channel_lookup(channel, channel_index);
    if (chn)
	{
		if (media_channel_mutex)
			os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
		if(!ZPL_SET_BIT(chn->flags, ZPL_MEDIA_STATE_START))
		{
			if (zpl_media_video_encode_start(chn->t_master, chn->media_param.video_media.halparam) != OK)
			{
				zm_msg_error("can not start media channel(%d/%d)", channel,channel_index);
				if (media_channel_mutex)
					os_mutex_unlock(media_channel_mutex);
				return ERROR;
			}
			ZPL_SET_BIT(chn->flags, ZPL_MEDIA_STATE_START);
			ret = OK;
			
			if (ZPL_MEDIA_DEBUG(CHANNEL, EVENT))
			{
				zm_msg_debug("start media channel(%d/%d)", channel, channel_index);
			}
		}
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
		if(ZPL_SET_BIT(chn->flags, ZPL_MEDIA_STATE_START))
		{	
			if (zpl_media_video_encode_stop(chn->media_param.video_media.halparam) != OK)
			{
				zm_msg_error("can not stop media channel(%d/%d)", channel,channel_index);
				if (media_channel_mutex)
					os_mutex_unlock(media_channel_mutex);
				return ERROR;
			}
			ret = OK;
			ZPL_CLR_BIT(chn->flags, ZPL_MEDIA_STATE_START);
			if (ZPL_MEDIA_DEBUG(CHANNEL, EVENT))
			{
				zm_msg_debug("stop media channel(%d/%d)", channel, channel_index);
			}
		}
		if (media_channel_mutex)
			os_mutex_unlock(media_channel_mutex);
		return ret;
	}
	return ERROR;
}

int zpl_media_channel_inactive(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index)
{
	zpl_media_channel_t *chn = zpl_media_channel_lookup(channel, channel_index);
	if (chn)
	{
		if (media_channel_mutex)
			os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
		if(ZPL_SET_BIT(chn->flags, ZPL_MEDIA_STATE_ACTIVE))
		{
			if (zpl_media_video_encode_hal_destroy(chn->media_param.video_media.halparam) != OK)
			{
				zm_msg_error("can not inactive media channel(%d/%d)", channel,channel_index);
				if (media_channel_mutex)
					os_mutex_unlock(media_channel_mutex);
				return ERROR;
			}
			ZPL_CLR_BIT(chn->flags, ZPL_MEDIA_STATE_ACTIVE);
			if (ZPL_MEDIA_DEBUG(CHANNEL, EVENT))
			{
				zm_msg_debug("inactive media channel(%d/%d)", channel, channel_index);
			}
		}
		if (media_channel_mutex)
			os_mutex_unlock(media_channel_mutex);
		return OK;
	}
	return ERROR;
}

int zpl_media_channel_handle_all(zpl_int32 active)
{
	return OK;
}

#ifdef ZPL_SHELL_MODULE
int zpl_media_channel_show(void *pvoid)
{
	NODE *node;
	zpl_media_channel_t *chn;
	zpl_media_video_encode_t *encode;
	char *media_typestr[] = {"unknow", "video", "audio"};
	char *bitrate_typestr[] = {"none", "CBR", "VBR", "ABR"};
	char *enRcMode_typestr[] = {"RC CBR", "RC VBR", "RC AVBR", "RC QVBR", "RC CVBR", "RC QPMAP","RC FIXQP"};
	char *gopmode_typestr[] = {"NORMALP", "DUALP", "SMARTP","ADVSMARTP", "BIPREDB", "LOWDELAYB","BUTT"};
	struct vty *vty = (struct vty *)pvoid;

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
			vty_out(vty, "channel            : %d/%d%s", chn->channel, chn->channel_index, VTY_NEWLINE);
			vty_out(vty, " type              : %s%s", media_typestr[chn->media_type], VTY_NEWLINE);
			vty_out(vty, " bindcount         : %d%s", chn->bindcount, VTY_NEWLINE);
			vty_out(vty, " flags             : 0x%x%s", chn->flags, VTY_NEWLINE);
			vty_out(vty, " size              : %dx%d%s", chn->media_param.video_media.codec.vidsize.width, chn->media_param.video_media.codec.vidsize.height, VTY_NEWLINE);
			vty_out(vty, " format            : %s%s", zpl_media_format_name(chn->media_param.video_media.codec.format), VTY_NEWLINE);
			vty_out(vty, " codectype         : %s%s", zpl_media_codec_name(chn->media_param.video_media.codec.codectype), VTY_NEWLINE);
			vty_out(vty, " framerate         : %d fps%s", chn->media_param.video_media.codec.framerate, VTY_NEWLINE);
			vty_out(vty, " bitrate           : %d%s", chn->media_param.video_media.codec.bitrate, VTY_NEWLINE);
			vty_out(vty, " bitrate_type      : %s%s", bitrate_typestr[chn->media_param.video_media.codec.bitrate_type], VTY_NEWLINE);
			vty_out(vty, " profile           : %d%s", chn->media_param.video_media.codec.profile, VTY_NEWLINE);
			vty_out(vty, " ikey_rate         : %d%s", chn->media_param.video_media.codec.ikey_rate, VTY_NEWLINE);
			vty_out(vty, " enRcMode          : %s%s", enRcMode_typestr[chn->media_param.video_media.codec.enRcMode], VTY_NEWLINE);
			vty_out(vty, " gopmode           : %s%s", gopmode_typestr[chn->media_param.video_media.codec.gopmode], VTY_NEWLINE);
			vty_out(vty, " packetization_mode: %d%s", chn->media_param.video_media.codec.packetization_mode, VTY_NEWLINE);
		}
		else if(chn->media_type == ZPL_MEDIA_AUDIO)
		{
			//vty_out(vty, "%-4d  %-4d %-6s %-8d %dx%d%s", chn->channel, chn->channel_index, media_typestr[chn->media_type],
			//		chn->media_param.video_media.halparam ? ((zpl_audio_encode_t *)chn->media_param.audio_media.halparam)->venc_channel : -1,
			//		chn->media_param.video_media.codec.vidsize.width, chn->media_param.video_media.codec.vidsize.height, VTY_NEWLINE);
		}	
	}
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return OK;
}
#endif
