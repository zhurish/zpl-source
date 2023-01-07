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
static zpl_media_channel_t *zpl_media_channel_lookup_entry(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index,
                                                           zpl_uint32 sseid, char *filename);

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
		media_channel_mutex = os_mutex_name_init("media_channel_mutex");
	}
	return ERROR;
}

int zpl_media_channel_exit(void)
{
	if (media_channel_mutex)
	{
		if (os_mutex_exit(media_channel_mutex) == OK)
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
    if (chn->channel_type == ZPL_MEDIA_CHANNEL_FILE)
    {
    }
    else
    {
        if (chn->channel_type == ZPL_MEDIA_CHANNEL_NORMAL) // 预览通道
        {
            if (chn->channel_index == ZPL_MEDIA_CHANNEL_INDEX_MAIN)
                chn->video_media.codec.format = ZPL_VIDHAL_DEFULT_FORMAT_MAIN;
            else if (chn->channel_index == ZPL_MEDIA_CHANNEL_INDEX_SUB)
                chn->video_media.codec.format = ZPL_VIDHAL_DEFULT_FORMAT_SUB;
            else if (chn->channel_index == ZPL_MEDIA_CHANNEL_INDEX_SUB1)
                chn->video_media.codec.format = ZPL_VIDHAL_DEFULT_FORMAT_SUB1;
        }
        else if (chn->channel_type == ZPL_MEDIA_CHANNEL_CAPTURE) // 抓拍通道使用大分辨率
        {
            chn->video_media.codec.format = ZPL_VIDHAL_DEFULT_FORMAT_MAIN;
        }
        else if (chn->channel_type == ZPL_MEDIA_CHANNEL_RECORED) // 录像通道使用小一点的分辨率
        {
            chn->video_media.codec.format = ZPL_VIDHAL_DEFULT_FORMAT_SUB;
        }
        else if (chn->channel_type == ZPL_MEDIA_CHANNEL_FILE) // 录像通道使用小一点的分辨率
        {
            chn->video_media.codec.format = ZPL_VIDHAL_DEFULT_FORMAT_SUB;
        }
        if (chn->channel_type != ZPL_MEDIA_CHANNEL_FILE)
            zpl_syshal_get_video_resolution(chn->video_media.codec.format, &chn->video_media.codec.vidsize); // 视频分辨率

        chn->video_media.codec.enctype = ZPL_VIDEO_CODEC_H264;          // 编码类型
        chn->video_media.codec.framerate = ZPL_VIDHAL_DEFULT_FRAMERATE; // 帧率

        if (chn->channel_index == ZPL_MEDIA_CHANNEL_INDEX_MAIN)
            chn->video_media.codec.bitrate = 8192; // 码率
        else if (chn->channel_index == ZPL_MEDIA_CHANNEL_INDEX_SUB)
            chn->video_media.codec.bitrate = 4096; // 码率
        else if (chn->channel_index == ZPL_MEDIA_CHANNEL_INDEX_SUB1)
            chn->video_media.codec.bitrate = 2048; // 码率
        else
            chn->video_media.codec.bitrate = 2048;
        chn->video_media.codec.bitrate_type = ZPL_VIDHAL_DEFULT_BITRATE; // 码率类型
        if (chn->video_media.codec.enctype == ZPL_VIDEO_CODEC_H264)
            chn->video_media.codec.profile = ZPL_VIDEO_CODEC_PROFILE_BASELINE;
        else if (chn->video_media.codec.enctype == ZPL_VIDEO_CODEC_H265)
            chn->video_media.codec.profile = ZPL_VIDEO_CODEC_PROFILE_MAIN;

        chn->video_media.codec.ikey_rate = ZPL_VIDHAL_DEFULT_IKEY_INTERVAL; // I帧间隔

        chn->video_media.codec.framerate = ZPL_VIDEO_FRAMERATE_DEFAULT; // 帧率
        chn->video_media.codec.enRcMode = ZPL_VENC_RC_CBR;
        chn->video_media.codec.gopmode = ZPL_VENC_GOPMODE_NORMALP;
    }
#ifndef ZPL_VIDEO_EXTRADATA_MAXSIZE
    chn->video_media.extradata.fSEI = NULL;
    chn->video_media.extradata.fSEISize = 0;
    chn->video_media.extradata.fVPS = NULL;
    chn->video_media.extradata.fVPSSize = 0;
    chn->video_media.extradata.fSPS = NULL;
    chn->video_media.extradata.fSPSSize = 0;
    chn->video_media.extradata.fPPS = NULL;
    chn->video_media.extradata.fPPSSize = 0;
    chn->video_media.extradata.profileLevelId = 0;
#endif
    chn->audio_media.codec.framerate = ZPL_AUDIO_FRAMERATE_DEFAULT; // 帧率
    chn->audio_media.codec.enctype = ZPL_AUDIO_CODEC_PCMU;          // 编码类型
    chn->audio_media.codec.bitrate = 64;                            // 码率
    chn->audio_media.codec.bitrate_type = 0;                        // 码率类型
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
		zpl_media_channel_create(0, ZPL_MEDIA_CHANNEL_INDEX_MAIN, NULL);
		zpl_media_channel_create(0, ZPL_MEDIA_CHANNEL_INDEX_SUB, NULL);
	}
	return OK;
}

static int zpl_media_channel_hwdestroy(zpl_media_channel_t *chn)
{
    if (chn)
    {
        ZPL_MEDIA_CHANNEL_LOCK(chn);
        if(chn->channel_type == ZPL_MEDIA_CHANNEL_FILE)
        {
            if(chn->video_media.enable && chn->video_media.halparam)
            {
                zpl_media_file_destroy(chn->video_media.halparam);
                zpl_media_channel_extradata_delete(&chn->video_media.extradata);
            }
            if(chn->audio_media.enable && chn->audio_media.halparam)
            {
                zpl_media_file_destroy(chn->audio_media.halparam);
            }
        }
        else
        {
            zpl_media_hal_stop(chn);
            zpl_media_hal_inactive(chn);
            if (chn->video_media.enable && chn->video_media.halparam)
                zpl_media_area_destroy_all(chn);      
        }  
        lstDelete(media_channel_list, (NODE *)chn);
        if (chn->frame_queue)
            chn->frame_queue = NULL;
        ZPL_MEDIA_CHANNEL_UNLOCK(chn);  
        if (os_mutex_exit(chn->_mutex) == OK)
			chn->_mutex = NULL;
        free(chn);
    }
    return OK;
}

int zpl_media_channel_destroy(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index, char *filename)
{
	zpl_media_channel_t *chn = NULL;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
	chn = zpl_media_channel_lookup_entry(channel, channel_index, 0, filename);
    if(chn)
        zpl_media_channel_hwdestroy(chn);
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return OK;
}

int zpl_media_channel_create(zpl_int32 channel,
							 ZPL_MEDIA_CHANNEL_INDEX_E channel_index, zpl_char *filename)
{
    zpl_media_channel_t *chn = NULL;
    chn = os_malloc(sizeof(zpl_media_channel_t));
	zpl_video_assert(chn);
    if (chn)
    {
		memset(chn, 0, sizeof(zpl_media_channel_t));
        if(filename)
        {
            memset(chn, 0, sizeof(zpl_media_channel_t));
            chn->channel = 32;
            chn->channel_index = 32;
            chn->channel_type = ZPL_MEDIA_CHANNEL_FILE;
            chn->bindcount = 0;
        }
        else
        {
            chn->channel = channel;
            chn->channel_index = channel_index;
    
            if (ZPL_MEDIA_ISNORMAL_CHANNEL(chn->channel))
                chn->channel_type = ZPL_MEDIA_CHANNEL_NORMAL;
            else if (ZPL_MEDIA_ISCAPTURE_CHANNEL(chn->channel))
                chn->channel_type = ZPL_MEDIA_CHANNEL_CAPTURE;
            else if (ZPL_MEDIA_ISRECORED_CHANNEL(chn->channel))
                chn->channel_type = ZPL_MEDIA_CHANNEL_RECORED;
            else if (ZPL_MEDIA_ISFILE_CHANNEL(chn->channel))
                chn->channel_type = ZPL_MEDIA_CHANNEL_FILE;

            chn->bindcount = 0;
            chn->video_media.enable = zpl_true;
            zpl_media_channel_encode_default(chn);
        }
		//chn->frame_queue = zpl_skbqueue_create(os_name_format("media-%d/%d", channel,channel_index),
		//	ZPL_MEDIA_BUFFER_FRAME_CACHESIZE, zpl_false);
		chn->frame_queue = zpl_media_bufqueue_get();	
		if (chn->frame_queue == NULL)
		{
			zpl_media_debugmsg_error("can not create buffer for media channel(%d/%d)", channel, channel_index);
			os_free(chn);
			return ERROR;
		}
		
		if (chn->channel_type != ZPL_MEDIA_CHANNEL_FILE)
		{
			if (zpl_media_hal_create(chn, chn->frame_queue) != OK)
			{
				zpl_media_debugmsg_error("can not create hal for media channel(%d/%d)", channel, channel_index);
				chn->frame_queue = NULL;
				os_free(chn);
				return ERROR;
			}
			if (ZPL_MEDIA_DEBUG(CHANNEL, EVENT))
			{
				zpl_video_encode_t *encode = chn->video_media.halparam;
				zpl_media_debugmsg_debug("create media channel(%d/%d) bind to encode(%d)", channel, channel_index, encode ? encode->venc_channel : -1);
				zpl_media_debugmsg_debug("  format:%d %dx%d", chn->video_media.codec.format, chn->video_media.codec.vidsize.width, chn->video_media.codec.vidsize.height);
				zpl_media_debugmsg_debug("    enctype:%d framerate:%d bitrate:%d profile:%d", chn->video_media.codec.enctype, chn->video_media.codec.framerate,
										 chn->video_media.codec.bitrate, chn->video_media.codec.profile);
				zpl_media_debugmsg_debug("    bitrate_type:%d ikey_rate:%d enRcMode:%d gopmode:%d", chn->video_media.codec.bitrate_type, chn->video_media.codec.ikey_rate,
										 chn->video_media.codec.enRcMode, chn->video_media.codec.gopmode);
			}
		}
		else
		{
            zpl_media_file_t *media_file = NULL;
            media_file = zpl_media_file_create(filename, "r");
            if (media_file == NULL)
            {
                zpl_media_debugmsg_error("can not create media file for media channel(%s)", filename);
                chn->frame_queue = NULL;
                free(chn);
                return ERROR;
            }
            media_file->frame_queue = chn->frame_queue;
            media_file->parent = chn;
            media_file->cnt = 0;

            media_file->filedesc.video.format = ZPL_VIDEO_FORMAT_720P;

            media_file->filedesc.video.vidsize.width = 1280;		//视频大小
            media_file->filedesc.video.vidsize.height = 720;
            media_file->filedesc.video.enctype = RTP_MEDIA_PAYLOAD_H264;		//编码类型
            media_file->filedesc.video.framerate = 30;		//帧率
            media_file->filedesc.video.bitrate = 16;		//码率
            media_file->filedesc.video.profile = 1;        //编码等级
            media_file->filedesc.video.bitrate_type = ZPL_BIT_RATE_CBR;	//码率类型

            if(media_file->b_video && !media_file->b_audio)
            {
                chn->video_media.enable = zpl_true;
                chn->video_media.halparam = media_file;
                zpl_media_file_extradata(media_file, &chn->video_media.extradata);
            }
            else if(!media_file->b_video && media_file->b_audio)
            {
                chn->audio_media.enable = zpl_true;
                chn->audio_media.halparam = media_file;
            }
            else if(media_file->b_video && media_file->b_audio)
            {
                chn->video_media.enable = zpl_true;
                chn->video_media.halparam = media_file;
                chn->audio_media.enable = zpl_true;
                chn->audio_media.halparam = NULL;
                zpl_media_file_extradata(media_file, &chn->video_media.extradata);
            }

            if(chn->video_media.enable)
            {
                zpl_media_file_codecdata(media_file, zpl_true, &chn->video_media.codec);
            }
            if(chn->audio_media.enable)
            {
                zpl_media_file_codecdata(media_file, zpl_false, &chn->audio_media.codec);
            }
		}

        if (chn->_mutex == NULL)
        {
            chn->_mutex = os_mutex_name_init("media_channel_mutex");
        }
		if (media_channel_mutex)
			os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
		if (media_channel_list)
			lstAdd(media_channel_list, (NODE *)chn);
		zpl_media_client_add(chn->media_client, zpl_media_proxy_buffer_data_distribute, NULL);
		if (media_channel_mutex)
			os_mutex_unlock(media_channel_mutex);
		return OK;
	}
	return ERROR;
}

static zpl_media_channel_t *zpl_media_channel_lookup_entry(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index, 
    zpl_uint32 sseid, char *filename)
{
	NODE node;
	zpl_media_channel_t *chn = NULL;
    zpl_media_file_t *media_file = NULL;
	for (chn = (zpl_media_channel_t *)lstFirst(media_channel_list);
		 chn != NULL; chn = (zpl_media_channel_t *)lstNext(&node))
	{
		node = chn->node;

        if(filename)
        {
            if (chn && chn->channel_type == ZPL_MEDIA_CHANNEL_FILE)
            {
                if(chn->video_media.enable)
                {
                    media_file = chn->video_media.halparam;
                }
                else if(chn->audio_media.enable)// && chn->video_media.halparam
                {
                    media_file = chn->audio_media.halparam;
                }
                if(media_file)
                {
                    if(filename[0] == '/')
                    {
                        if (strcmp(media_file->filename, filename) == 0)
                        {
                            break;
                        }
                    }
                    else
                    {
                        if (strcmp(media_file->filename, zpl_media_file_basename(filename)) == 0)
                        {
                            break;
                        }
                    }
                }
            }
        }
        else 
        {
            if (sseid && chn && (zpl_uint32)chn == sseid)
            {
                break;
            }
            else if (chn && chn->channel == channel && chn->channel_index == channel_index && 
                (chn->channel_type != ZPL_MEDIA_CHANNEL_FILE))
            {
                break;
            }
        }

	}
	return chn;
}

zpl_media_channel_t *zpl_media_channel_lookup(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index, zpl_char *filename)
{
	zpl_media_channel_t *chn = NULL;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
    chn = zpl_media_channel_lookup_entry( channel, channel_index, 0, filename);
    if (media_channel_mutex)
        os_mutex_unlock(media_channel_mutex);
	return chn;
}
zpl_media_channel_t *zpl_media_channel_lookup_sessionID(zpl_uint32 sessionID)
{
	zpl_media_channel_t *chn = NULL;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
	chn = zpl_media_channel_lookup_entry( -1, -1, sessionID, NULL);
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return chn;
}
ZPL_MEDIA_STATE_E zpl_media_channel_state(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index, zpl_char *filename)
{
	ZPL_MEDIA_STATE_E state = ZPL_MEDIA_STATE_NONE;
	zpl_media_channel_t *chn = NULL;
    if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
	chn = zpl_media_channel_lookup_entry(channel, channel_index, 0, filename);
    if(chn)
    {
        state = chn->state;
    }
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return state;
}

zpl_bool zpl_media_channel_isvideo(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index, zpl_char *filename)
{
    zpl_bool ret = zpl_false;
    zpl_media_channel_t *chn = NULL;
    if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
	chn = zpl_media_channel_lookup_entry(channel, channel_index, 0, filename);
    if(chn)
    {
        if(zpl_media_getptr(chn)->video_media.enable && zpl_media_getptr(chn)->video_media.halparam)
            ret = zpl_true;
    }
    if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;
}

zpl_bool zpl_media_channel_isaudio(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index, zpl_char *filename)
{
    zpl_bool ret = zpl_false;
    zpl_media_channel_t *chn = NULL;
    if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
	chn = zpl_media_channel_lookup_entry(channel, channel_index, 0, filename);
    if(chn)
    {
        if(zpl_media_getptr(chn)->audio_media.enable && zpl_media_getptr(chn)->audio_media.halparam)
            ret = zpl_true;
    }
    if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;
}

zpl_bool zpl_media_channel_islocalfile(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index, zpl_char *filename)
{
    zpl_bool ret = zpl_false;
    zpl_media_channel_t *chn = NULL;
    if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
	chn = zpl_media_channel_lookup_entry(channel, channel_index, 0, filename);
    if(chn)
    {
        if(zpl_media_channel_gettype(chn) == ZPL_MEDIA_CHANNEL_FILE)
            ret = zpl_true;
    }
    if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;
}

int zpl_media_channel_video_codec_get(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index, zpl_char *filename, zpl_video_codec_t *codec)
{
    int ret = ERROR;
    zpl_media_channel_t *chn = NULL;
    if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
	chn = zpl_media_channel_lookup_entry(channel, channel_index, 0, filename);
    if(chn)
    {
        if(zpl_media_getptr(chn)->video_media.enable && zpl_media_getptr(chn)->video_media.halparam)
        {
            if(codec)
                memcpy(codec, &chn->video_media.codec, sizeof(zpl_video_codec_t));
            ret = OK;
        }
    }
    if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;
}

int zpl_media_channel_audio_codec_get(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index, zpl_char *filename, zpl_audio_codec_t *codec)
{
    int ret = ERROR;
    zpl_media_channel_t *chn = NULL;
    if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
	chn = zpl_media_channel_lookup_entry(channel, channel_index, 0, filename);
    if(chn)
    {
        if(zpl_media_getptr(chn)->audio_media.enable && zpl_media_getptr(chn)->audio_media.halparam)
        {
            if(codec)
                memcpy(codec, &chn->audio_media.codec, sizeof(zpl_audio_codec_t));
            ret = OK;
        }
    }
    if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;
}

int zpl_media_channel_bindcount_get(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index, zpl_char *filename)
{
    int ret = 0;
    zpl_media_channel_t *chn = NULL;
    if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
	chn = zpl_media_channel_lookup_entry(channel, channel_index, 0, filename);
    if(chn)
    {
        ret = chn->bindcount;
    }
    if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;
}

int zpl_media_channel_bindcount_set(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index, zpl_char *filename, int addsub)
{
    int ret = ERROR;
    zpl_media_channel_t *chn = NULL;
    if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
	chn = zpl_media_channel_lookup_entry(channel, channel_index, 0, filename);
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


int zpl_media_channel_client_add(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index, char *filename, 
    zpl_media_buffer_handler cb_handler, void *pUser)
{
	int ret = ERROR;
    zpl_media_channel_t *mchannel = NULL;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
    mchannel = zpl_media_channel_lookup_entry( channel, channel_index, 0, filename);
    if(mchannel)
	    ret = zpl_media_client_add(mchannel->media_client, cb_handler, pUser);
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;	
}

int zpl_media_channel_client_del(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index, char *filename, zpl_int32 index)
{
	int ret = ERROR;
    zpl_media_channel_t *mchannel = NULL;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
    mchannel = zpl_media_channel_lookup_entry( channel, channel_index, 0, filename);
    if(mchannel)
	    ret = zpl_media_client_del(mchannel->media_client, index);
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return ret;	
}



int zpl_media_channel_active(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index)
{
	zpl_media_channel_t *chn = zpl_media_channel_lookup(channel, channel_index, NULL);
	if (chn)
	{
		if (media_channel_mutex)
			os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
		if (chn->channel_type != ZPL_MEDIA_CHANNEL_FILE)
		{
			if (zpl_media_hal_active(chn) != OK)
			{
                zpl_media_debugmsg_error("can not active media channel(%d/%d)", channel,channel_index);
				if (media_channel_mutex)
					os_mutex_unlock(media_channel_mutex);
				return ERROR;
			}
		}
		if (ZPL_MEDIA_DEBUG(CHANNEL, EVENT))
		{
			zpl_media_debugmsg_debug("active media channel(%d/%d)", channel, channel_index);
		}

		if (media_channel_mutex)
			os_mutex_unlock(media_channel_mutex);
		return OK;
	}
	return ERROR;
}

int zpl_media_channel_start(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index, zpl_char *filename)
{
    int ret = ERROR;
    zpl_media_channel_t *chn = zpl_media_channel_lookup(channel, channel_index, filename);
    if (chn)
	{
		if (media_channel_mutex)
			os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
		if (chn->channel_type != ZPL_MEDIA_CHANNEL_FILE)
		{
			if (zpl_media_hal_start(chn) != OK)
			{
                zpl_media_debugmsg_error("can not start media channel(%d/%d)", channel,channel_index);
				if (media_channel_mutex)
					os_mutex_unlock(media_channel_mutex);
				return ERROR;
			}
            ret = OK;
        }
        else if(chn->channel_type == ZPL_MEDIA_CHANNEL_FILE)
        {
            if(chn->video_media.enable)
                ret = zpl_media_file_play_start(chn->video_media.halparam, zpl_true);
            else if(chn->audio_media.enable)
                ret = zpl_media_file_play_start(chn->audio_media.halparam, zpl_true);
        }
		if (ZPL_MEDIA_DEBUG(CHANNEL, EVENT))
		{
			zpl_media_debugmsg_debug("start media channel(%d/%d)", channel, channel_index);
		}
		if (media_channel_mutex)
			os_mutex_unlock(media_channel_mutex);
		return ret;
	}
	return ERROR;
}

int zpl_media_channel_stop(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index, zpl_char *filename)
{
    int ret = ERROR;
    zpl_media_channel_t *chn = zpl_media_channel_lookup(channel, channel_index, filename);
    if (chn)
	{
		if (media_channel_mutex)
			os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
#ifdef ZPL_MEDIA_BUFFER_DEBUG_ONFILE
		//zpl_media_buffer_debug_onfile_close(chn->frame_queue);
#endif
		if (chn->channel_type != ZPL_MEDIA_CHANNEL_FILE)
		{
			if (zpl_media_hal_stop(chn) != OK)
			{
                zpl_media_debugmsg_error("can not stop media channel(%d/%d)", channel,channel_index);
				if (media_channel_mutex)
					os_mutex_unlock(media_channel_mutex);
				return ERROR;
			}
            ret = OK;
        }
        else if(chn->channel_type == ZPL_MEDIA_CHANNEL_FILE)
        {
            if(chn->video_media.enable)
                ret = zpl_media_file_play_start(chn->video_media.halparam, zpl_false);
            else if(chn->audio_media.enable)
                ret = zpl_media_file_play_start(chn->audio_media.halparam, zpl_false);
        }
		if (ZPL_MEDIA_DEBUG(CHANNEL, EVENT))
		{
			zpl_media_debugmsg_debug("stop media channel(%d/%d)", channel, channel_index);
		}
		if (media_channel_mutex)
			os_mutex_unlock(media_channel_mutex);
		return ret;
	}
	return ERROR;
}

int zpl_media_channel_inactive(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index)
{
	zpl_media_channel_t *chn = zpl_media_channel_lookup(channel, channel_index, NULL);
	if (chn)
	{
		if (media_channel_mutex)
			os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
#ifdef ZPL_MEDIA_BUFFER_DEBUG_ONFILE
		//zpl_media_buffer_debug_onfile_close(chn->frame_queue);
#endif
		if (chn->channel_type != ZPL_MEDIA_CHANNEL_FILE)
		{
			if (zpl_media_hal_inactive(chn) != OK)
			{
                zpl_media_debugmsg_error("can not inactive media channel(%d/%d)", channel,channel_index);
				if (media_channel_mutex)
					os_mutex_unlock(media_channel_mutex);
				return ERROR;
			}
		}
		if (ZPL_MEDIA_DEBUG(CHANNEL, EVENT))
		{
			zpl_media_debugmsg_debug("inactive media channel(%d/%d)", channel, channel_index);
		}
		if (media_channel_mutex)
			os_mutex_unlock(media_channel_mutex);
		return OK;
	}
	return ERROR;
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
		chn->frame_queue = zpl_media_bufqueue_get();
		//chn->frame_queue = zpl_skbqueue_create(os_name_format("media-%d/%d", chn->channel, chn->channel_index), 
		//	ZPL_MEDIA_BUFFER_FRAME_CACHESIZE, zpl_false);
		if (chn->frame_queue == NULL)
		{
			os_free(chn);
			return ERROR;
		}
		if (chn->channel_type != ZPL_MEDIA_CHANNEL_FILE)
		{
			if (zpl_media_hal_create(chn, chn->frame_queue) != OK)
			{
                zpl_media_debugmsg_error("can not create hal media channel(%d/%d)", chn->channel, chn->channel_index);
				chn->frame_queue = NULL;
				os_free(chn);
				return ERROR;
			}
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

int zpl_media_channel_halparam_set(zpl_int32 channel,
								   ZPL_MEDIA_CHANNEL_INDEX_E channel_index, zpl_bool video, void *halparam)
{
	zpl_media_channel_t *chn = zpl_media_channel_lookup(channel, channel_index, NULL);
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);
	if (chn)
	{
		if(video)
			chn->video_media.halparam = halparam;
		else
			chn->audio_media.halparam = halparam;	
	}
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return OK;
}

int zpl_media_channel_handle_all(zpl_uint32 active)
{
	NODE node;
	zpl_media_channel_t *chn = NULL;
	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);

	for (chn = (zpl_media_channel_t *)lstFirst(media_channel_list);
		 chn != NULL; chn = (zpl_media_channel_t *)lstNext(&node))
	{
		node = chn->node;
		if (chn)
		{
			if (chn->channel_type != ZPL_MEDIA_CHANNEL_FILE)
			{
				if (active == 1)
					zpl_media_hal_active(chn);
				else if (active == 2)
					zpl_media_hal_inactive(chn);
				else if (active == 3)
				{
					if (zpl_media_hal_start(chn) == OK)
					{
						zpl_media_hal_read_start(ZPL_MEDIA_NODE_INPUT, thread_master_module_lookup(MODULE_ZPLMEDIA), chn);
						zpl_media_hal_read_start(ZPL_MEDIA_NODE_PROCESS, thread_master_module_lookup(MODULE_ZPLMEDIA), chn);
						zpl_media_hal_read_start(ZPL_MEDIA_NODE_ENCODE, thread_master_module_lookup(MODULE_ZPLMEDIA), chn);
						zpl_media_task_ready(MODULE_ZPLMEDIA);
					}
				}
				else if (active == 4)
				{
					zpl_media_hal_read_stop(ZPL_MEDIA_NODE_INPUT, chn);
					zpl_media_hal_read_stop(ZPL_MEDIA_NODE_PROCESS, chn);
					zpl_media_hal_read_stop(ZPL_MEDIA_NODE_ENCODE, chn);
					zpl_media_hal_stop(chn);
				}
				else if (active == -1)
					zpl_media_hal_destroy(chn);
			}
			else
			{

			}
		}
	}
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return OK;
}

#ifdef ZPL_SHELL_MODULE
int zpl_media_channel_show(void *pvoid)
{
	NODE *node;
	zpl_media_channel_t *chn;
	struct vty *vty = (struct vty *)pvoid;

	if (media_channel_mutex)
		os_mutex_lock(media_channel_mutex, OS_WAIT_FOREVER);

	if (lstCount(media_channel_list))
	{
		vty_out(vty, "%-4s %-4s %-6s %-8s %-16s %s", "----", "----", "------", "--------", "----------------", VTY_NEWLINE);
		vty_out(vty, "%-4s %-4s %-6s %-8s %-16s %s", "ID", "SUB", "TYPE", "BINDID", "SIZE", VTY_NEWLINE);
		vty_out(vty, "%-4s %-4s %-6s %-8s %-16s %s", "----", "----", "------", "--------", "----------------", VTY_NEWLINE);
	}
	for (node = lstFirst(media_channel_list); node != NULL; node = lstNext(node))
	{
		chn = (zpl_media_channel_t *)node;
		if (chn && chn->channel_type != ZPL_MEDIA_CHANNEL_FILE)
		{
			vty_out(vty, "%-4d  %-4d %-6d %-8d %dx%d%s", chn->channel, chn->channel_index, chn->channel_type,
					chn->video_media.halparam ? ((zpl_video_encode_t *)chn->video_media.halparam)->venc_channel : -1,
					chn->video_media.codec.vidsize.width, chn->video_media.codec.vidsize.height, VTY_NEWLINE);
		}
		else if (chn && chn->channel_type == ZPL_MEDIA_CHANNEL_FILE)
		{
			if(chn->video_media.halparam)
			{
				zpl_media_file_t *fdesc = (zpl_media_file_t*)chn->video_media.halparam;
				/*fdesc->filedesc.video.format;	
				fdesc->filedesc.video.vidsize;		//视频大小
				fdesc->filedesc.video.enctype;		//编码类型
				fdesc->filedesc.video.framerate;		//帧率
				fdesc->filedesc.video.bitrate;		//码率
    			fdesc->filedesc.video.profile;        //编码等级
				fdesc->filedesc.video.bitrate_type;	//码率类型
				*/
				vty_out(vty, "%-4d  %-4d %-6d %-8d %dx%d%s", chn->channel, chn->channel_index, chn->channel_type, 
					chn->video_media.halparam?((zpl_media_file_t*)chn->video_media.halparam)->file_size:-1, 
					fdesc->filedesc.video.vidsize.width, fdesc->filedesc.video.vidsize.height, VTY_NEWLINE);
				vty_out(vty, "%-4d  %-4d %-6d %-8d %dx%d%s", fdesc->filedesc.video.format, 
					fdesc->filedesc.video.enctype, fdesc->filedesc.video.framerate,
					fdesc->filedesc.video.bitrate, fdesc->filedesc.video.profile, 
					fdesc->filedesc.video.bitrate_type, VTY_NEWLINE);
			}
			else
			vty_out(vty, "%-4d  %-4d %-6d %-8d %dx%d%s", chn->channel, chn->channel_index, chn->channel_type, 
					chn->video_media.halparam?((zpl_media_file_t*)chn->video_media.halparam)->file_size:-1, 
					0, 0, VTY_NEWLINE);
		}
	}
	if (media_channel_mutex)
		os_mutex_unlock(media_channel_mutex);
	return OK;
}
#endif
