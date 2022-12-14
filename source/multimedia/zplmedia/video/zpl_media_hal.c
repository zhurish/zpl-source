/*
 * zpl_media_hal.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"



/* hal init */
static int zpl_video_vpss_hal_create(zpl_media_channel_t *chn, zpl_int32 vpss_group, zpl_int32 vpss_channel)
{
    int createflag = 0;
    zpl_video_vpss_channel_t *video_vpss = NULL;
#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
    zpl_video_vpssgrp_t *video_vpssgrp = NULL;
#endif

#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
    if (zpl_video_vpssgrp_lookup(vpss_group) == NULL)
    {
        if(zpl_video_vpssgrp_create(vpss_group)!= OK)
        {
            if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
            {
                zpl_media_debugmsg_error("create vpssgrp channel(%d)", vpss_group);
            } 
            return ERROR;
        }
        createflag = 1;
    }
    video_vpssgrp = zpl_video_vpssgrp_lookup(vpss_group);
    if (video_vpssgrp == NULL)
    {
        zpl_video_vpssgrp_destroy(vpss_group);
        {
            if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
            {
                zpl_media_debugmsg_error("lookup vpssgrp channel(%d)", vpss_group);
            } 
            return ERROR;
        }
        return ERROR;
    }
    if(createflag)
        video_vpssgrp->res_flag = VIDHAL_RES_FLAG_LOAD(chn->channel, chn->channel_index, VPSSGRP);
    zpl_video_vpssgrp_bindcount_set(video_vpssgrp, zpl_true);
    zpl_media_debugmsg_debug("channel(%d/%d) VPSSGRP Flags (0x%x)", chn->channel, chn->channel_index, video_vpssgrp->res_flag);

#endif
    createflag = 0;
    if (zpl_video_vpss_channel_lookup(vpss_channel) == NULL)
    {
        if(zpl_video_vpss_channel_create(vpss_channel)!= OK)
        {
            if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
            {
                zpl_media_debugmsg_error("create vpss channel(%d)", vpss_channel);
            } 
            return ERROR;
        }
        createflag = 1;
    }
    video_vpss = zpl_video_vpss_channel_lookup(vpss_channel);
    if (video_vpss == NULL)
    {
        if (zpl_video_vpssgrp_bindcount_get(video_vpssgrp) == 0)
            zpl_video_vpssgrp_destroy(vpss_group);
        if (zpl_video_vpss_channel_bindcount_get(video_vpss) == 0)    
            zpl_video_vpss_channel_destroy(vpss_channel);
        {
            if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
            {
                zpl_media_debugmsg_error("lookup vpss channel(%d)", vpss_channel);
            } 
            return ERROR;
        }
    }
    zpl_video_vpss_channel_bindcount_set(video_vpss, zpl_true);
    zpl_media_hardadap_init(&video_vpss->callback);
    if(createflag)
        video_vpss->res_flag = VIDHAL_RES_FLAG_LOAD(chn->channel, chn->channel_index, VPSSCHN);
    zpl_media_debugmsg_debug("channel(%d/%d) VPSS Flags (0x%x)", chn->channel, chn->channel_index, video_vpss->res_flag);

    return OK;
}

static int zpl_video_input_hal_create(zpl_media_channel_t *chn, zpl_int32 input_chn, zpl_int32 input_pipe, zpl_int32 input_dev)
{
    int createflag = 0;
    zpl_video_input_channel_t *video_input = NULL;
    zpl_video_input_pipe_t *video_input_pipe = NULL;
    if (zpl_video_input_channel_lookup(input_chn) == NULL)
    {
        if (zpl_video_input_channel_create(input_chn) != OK)
        {
            if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
            {
                zpl_media_debugmsg_error("create input channel(%d)", input_chn);
            } 
            return ERROR;
        }  
        createflag = 1;    
    }
    video_input = zpl_video_input_channel_lookup(input_chn);

    if (video_input == NULL)
    {
        zpl_media_debugmsg_error("can not get input hal for channel(%d)", input_chn);
        if (zpl_video_input_channel_bindcount_get(video_input) == 0)
            zpl_video_input_channel_destroy( input_chn);
        return ERROR;
    }
    zpl_video_input_channel_bindcount_set(video_input, zpl_true);
    zpl_media_hardadap_init(&video_input->callback);
    if(createflag)
        video_input->res_flag = VIDHAL_RES_FLAG_LOAD(chn->channel, chn->channel_index, VICHN);

    zpl_media_debugmsg_debug("channel(%d/%d) VICHN Flags (0x%x)", chn->channel, chn->channel_index, video_input->res_flag);
    
    createflag = 0;
    if (zpl_video_input_pipe_lookup(input_dev, input_pipe) == NULL)
    {
        if (zpl_video_input_pipe_create(input_dev, input_pipe) != OK)
        {
            if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
            {
                zpl_media_debugmsg_error("create input pipe(%d)", input_pipe);
            } 
            return ERROR;
        }  
        createflag = 1;   
    }
    video_input_pipe = zpl_video_input_pipe_lookup(input_dev, input_pipe);

    if (video_input_pipe == NULL)
    {
        zpl_media_debugmsg_error("can not get input pipe hal for channel(%d)", input_chn);
        if (zpl_video_input_channel_bindcount_get(video_input) == 0)
            zpl_video_input_channel_destroy(input_chn);
        if (zpl_video_input_pipe_bindcount_get(video_input_pipe) == 0)
        zpl_video_input_pipe_destroy( input_dev, input_pipe);
        return ERROR;
    }
    zpl_video_input_pipe_bindcount_set(video_input_pipe, zpl_true);
    zpl_media_hardadap_init(&video_input_pipe->callback);
    if(createflag)
        video_input_pipe->res_flag = VIDHAL_RES_FLAG_LOAD(chn->channel, chn->channel_index, VIPIPE);
    zpl_media_debugmsg_debug("channel(%d/%d) VIPIPE Flags (0x%x)", chn->channel, chn->channel_index, video_input_pipe->res_flag);

    return OK;
}

int zpl_media_hal_create(zpl_media_channel_t *chn, void *buffer_queue)
{
    zpl_int32 venc_channel = VIDHAL_RES_ID_LOAD(chn->channel, chn->channel_index, VENCCHN);
    //zpl_media_debugmsg_debug("zpl_media_hal_create channel(%d/%d) -> venc:%d", chn->channel, chn->channel_index, venc_channel);
    if (venc_channel >= 0)
    {
        zpl_int32 input_pipe = -1, input_chn = -1, input_dev = -1;
        zpl_int32 vpss_channel = -1, vpss_group = -1; //底层通道号
        zpl_video_encode_t *video_encode = NULL;
        zpl_video_vpss_channel_t *video_vpss = NULL;
        zpl_video_input_channel_t *video_input = NULL;
        zpl_video_input_pipe_t *video_input_pipe = NULL;
#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
        zpl_video_vpssgrp_t *video_vpssgrp = NULL;
#endif

        vpss_channel = VIDHAL_RES_ID_LOAD(chn->channel, chn->channel_index, VPSSCHN);
        vpss_group = VIDHAL_RES_ID_LOAD(chn->channel, chn->channel_index, VPSSGRP);

        input_chn = VIDHAL_RES_ID_LOAD(chn->channel, chn->channel_index, VICHN);
        input_pipe = VIDHAL_RES_ID_LOAD(chn->channel, chn->channel_index, VIPIPE);
        input_dev = VIDHAL_RES_ID_LOAD(chn->channel, chn->channel_index, DEV);
zpl_media_hal_sys_init();
        if(zpl_video_encode_create(venc_channel) != OK)
        {
            if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
            {
                zpl_media_debugmsg_error("create video_encode channel(%d)", venc_channel);
            } 
            return ERROR;
        } 
        video_encode = chn->video_media.halparam = zpl_video_encode_lookup(venc_channel);
        if (chn->video_media.halparam == NULL)
        {
            zpl_media_debugmsg_error("can not lookup video_encode channel(%d)", venc_channel);
            zpl_video_encode_destroy(venc_channel);
            return ERROR;
        }
        if (zpl_video_encode_buffer_queue_set(venc_channel, buffer_queue) != OK)
        {
            zpl_media_debugmsg_error("can not set buffer queue for video_encode channel(%d)", venc_channel);
            zpl_video_encode_destroy(venc_channel);
            return ERROR;
        }
        video_encode->media_channel = chn;
        video_encode->res_flag = VIDHAL_RES_FLAG_LOAD(chn->channel, chn->channel_index, VENCCHN);
        zpl_media_debugmsg_debug("channel(%d/%d) VENC Flags (0x%x)", chn->channel, chn->channel_index, video_encode->res_flag);

        if(zpl_video_vpss_hal_create(chn, vpss_group, vpss_channel) != OK)
        {
            zpl_media_debugmsg_error("can not create vpss hal for video_encode channel(%d)", venc_channel);
            zpl_video_encode_destroy(venc_channel);
            return ERROR;
        }

        if(zpl_video_input_hal_create(chn, input_chn, input_pipe, input_dev) != OK)
        {
             zpl_media_debugmsg_error("can not create input hal for video_encode channel(%d)", venc_channel);
            zpl_video_encode_destroy(venc_channel);
            return ERROR;
        }
        video_encode->pCodec = &chn->video_media.codec;
        video_vpss = zpl_video_vpss_channel_lookup(vpss_channel);
        video_encode->video_vpss = video_vpss;
        video_vpssgrp = zpl_video_vpssgrp_lookup(vpss_group);
        if(video_vpss)
		{
            video_vpss->vpssgrp = video_vpssgrp;
			if(video_vpss->input_size.width == 0 || video_vpss->input_size.width == 0)
			{
				video_vpss->input_size.width = chn->video_media.codec.vidsize.width;
				video_vpss->input_size.height = chn->video_media.codec.vidsize.height;
			}
		}
        video_input = zpl_video_input_channel_lookup(input_chn);       
        video_input_pipe = zpl_video_input_pipe_lookup(input_dev, input_pipe);

        if(video_vpssgrp)
		{
            video_vpssgrp->video_input = video_input;
			if(video_vpssgrp->input_size.width == 0 || video_vpssgrp->input_size.width == 0)
			{
				video_vpssgrp->input_size.width = chn->video_media.codec.vidsize.width;
				video_vpssgrp->input_size.height = chn->video_media.codec.vidsize.height;
			}
		}
        if(video_input)
            video_input->inputpipe = video_input_pipe;

        zpl_video_vpss_channel_bindcount_set(video_vpss, zpl_true);
        zpl_video_vpssgrp_bindcount_set(video_vpssgrp, zpl_true);
        zpl_video_input_channel_bindcount_set(video_input, zpl_true);
        zpl_video_input_pipe_bindcount_set(video_input_pipe, zpl_true);

		if(ZPL_MEDIA_DEBUG(CHANNEL, EVENT) && ZPL_MEDIA_DEBUG(CHANNEL, DETAIL))
		{
			zpl_media_debugmsg_debug("media video_encode(%d) bind to vpss(%d) vpssgrp(%d) input channel(%d) input pipe(%d) input dev(%d)", 
                video_encode?video_encode->venc_channel:-1, video_vpss?video_vpss->vpss_channel:-1,
                video_vpssgrp?video_vpssgrp->vpss_group:-1, video_input?video_input->input_chn:-1,
                video_input_pipe?video_input_pipe->input_pipe:-1,video_input_pipe?video_input_pipe->input_dev:-1);
		} 
        return OK;
    }
    return ERROR;
}

int zpl_media_hal_destroy(zpl_media_channel_t *chn)
{
    zpl_video_encode_t *video_encode = NULL;
    zpl_video_vpss_channel_t *video_vpss = NULL;
    zpl_video_input_channel_t *video_input = NULL;
    zpl_video_input_pipe_t *video_input_pipe = NULL;
#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
    zpl_video_vpssgrp_t *video_vpssgrp = NULL;
#endif
    video_encode = chn->video_media.halparam;
    if (video_encode == NULL || video_encode->video_vpss == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("video_encode channel or vpss NULL");
        } 
        return ERROR;
    } 
    video_vpss = video_encode->video_vpss;
#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
    if (video_vpss == NULL || video_vpss->vpssgrp == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("vpss or vpssgrp NULL");
        } 
        return ERROR;
    } 
    video_vpssgrp = video_vpss->vpssgrp;
    if (video_vpssgrp == NULL || video_vpssgrp->video_input == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("vpssgrp or input channel NULL");
        } 
        return ERROR;
    } 
    video_input = video_vpssgrp->video_input;
#else
    if (video_vpss == NULL || video_vpss->video_input == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("vpssgrp or input channel NULL");
        } 
        return ERROR;
    } 
    video_input = video_vpss->video_input;
#endif
    video_input_pipe = video_input->inputpipe;
    if (video_input_pipe == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("input pipe NULL");
        } 
        return ERROR;
    } 
    if (zpl_video_vpss_channel_bindcount_get(video_vpss) == 1)
    {
        zpl_video_vpss_channel_destroy(video_vpss->vpss_channel);
    }
    else
    {
        zpl_video_vpss_channel_bindcount_set(video_vpss, zpl_false);
    }
#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
    if (zpl_video_vpssgrp_bindcount_get(video_vpssgrp) == 1)
    {
        zpl_video_vpssgrp_destroy(video_vpssgrp->vpss_group);
    }
    else
    {
        zpl_video_vpssgrp_bindcount_set(video_vpssgrp, zpl_false);
    }
#endif
    if (zpl_video_input_channel_bindcount_get(video_input) == 1)
    {
        zpl_video_input_channel_destroy(video_input->input_chn);
    }
    else
    {
        zpl_video_input_channel_bindcount_set(video_vpss, zpl_false);
    }

    if (zpl_video_input_pipe_bindcount_get(video_input_pipe) == 1)
    {
        zpl_video_input_pipe_destroy(video_input_pipe->input_pipe, video_input_pipe->input_dev);
    }
    else
    {
        zpl_video_input_pipe_bindcount_set(video_input_pipe, zpl_false);
    }
    zpl_video_encode_destroy(video_encode->venc_channel);
    return OK;
}

int zpl_media_hal_active(zpl_media_channel_t *chn)
{
    zpl_video_encode_t *video_encode = NULL;
    zpl_video_vpss_channel_t *video_vpss = NULL;
    zpl_video_input_channel_t *video_input = NULL;
    zpl_video_input_pipe_t *video_input_pipe = NULL;
    zpl_media_hardadap_t cbnode;
#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
    zpl_video_vpssgrp_t *video_vpssgrp = NULL;
#endif
	if(ZPL_MEDIA_DEBUG(CHANNEL, EVENT) && ZPL_MEDIA_DEBUG(CHANNEL, DETAIL))
	{
			zpl_media_debugmsg_debug("media channel(%d/%d)", chn->channel, chn->channel_index);
	}
    video_encode = chn->video_media.halparam;
    if (video_encode == NULL || video_encode->video_vpss == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("video_encode or vpss NULL");
        } 
        return ERROR;
    }
    video_vpss = video_encode->video_vpss;
#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
    if (video_vpss == NULL || video_vpss->vpssgrp == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("vpss or vpssgrp NULL");
        } 
        return ERROR;
    }
    video_vpssgrp = video_vpss->vpssgrp;
    if (video_vpssgrp == NULL || video_vpssgrp->video_input == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("vpssgrp or input channel NULL");
        } 
        return ERROR;
    }
    video_input = video_vpssgrp->video_input;
#else
    if (video_vpss == NULL || video_vpss->video_input == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("vpssgrp or input channel NULL");
        } 
        return ERROR;
    }
    video_input = video_vpss->video_input;
#endif
    
#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
    video_vpssgrp = video_vpss->vpssgrp;
    video_input = video_vpssgrp->video_input;
    cbnode.module = ZPL_MEDIA_HARDADAP_VPSS;   // 模块ID
    cbnode.dst_device = video_vpssgrp->vpss_group;    // 发送目的ID
    cbnode.dst_channel = -1; // 发送目的通道
    cbnode.online = &video_vpssgrp->online;
    cbnode.dst_private = video_vpssgrp;
    cbnode.hardadap_sendto = zpl_media_hal_input_sendto_vpss_default; // 发送回调
    zpl_media_hardadap_install(&video_input->callback, &cbnode);

#else
    video_input = video_vpss->video_input;
    cbnode.module = ZPL_MEDIA_HARDADAP_VPSS;   // 模块ID
    cbnode.dst_device = video_vpss->vpss_group;    // 发送目的ID
    cbnode.dst_channel = video_vpss->vpss_channel; // 发送目的通道
    cbnode.online = &video_vpss->online;
    cbnode.dst_private = video_vpss;
    cbnode.hardadap_sendto = zpl_media_hal_input_sendto_vpss_default; // 发送回调
    zpl_media_hardadap_install(&video_input->callback, &cbnode);

#endif
    video_input_pipe = video_input->inputpipe; 
    if (video_input_pipe == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("input pipe NULL");
        } 
        return ERROR;
    }
    cbnode.module = ZPL_MEDIA_HARDADAP_ENCODE; // 模块ID
    cbnode.dst_device = 0;                         // 发送目的ID
    cbnode.dst_channel = video_encode->venc_channel;     // 发送目的通道
    cbnode.online = &video_encode->online;
    cbnode.dst_private = video_encode;
    cbnode.hardadap_sendto = zpl_media_hal_vpss_sendto_encode_default; // 发送回调
    zpl_media_hardadap_install(&video_vpss->callback, &cbnode);

    if (zpl_video_input_pipe_active(video_input_pipe) != OK)
    {
        return ERROR;
    } 
    if (zpl_video_input_channel_active(video_input) != OK)
    {
        return ERROR;
    } 
#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
    if (zpl_video_vpssgrp_active(video_vpssgrp) != OK)
    {
        return ERROR;
    }
#endif
    if (zpl_video_vpss_channel_active(video_vpss) != OK)
    {
        return ERROR;
    }
    if (zpl_video_encode_active(video_encode) != OK)
    {
        return ERROR;
    }

    return OK;
}

int zpl_media_hal_start(zpl_media_channel_t *chn)
{
    zpl_video_encode_t *video_encode = NULL;
    zpl_video_vpss_channel_t *video_vpss = NULL;
    zpl_video_input_channel_t *video_input = NULL;
    zpl_video_input_pipe_t *video_input_pipe = NULL;
#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
    zpl_video_vpssgrp_t *video_vpssgrp = NULL;
#endif
    video_encode = chn->video_media.halparam;
    if (video_encode == NULL || video_encode->video_vpss == NULL)
    {
        if (ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("video_encode channel or vpss NULL");
        }
        return ERROR;
    }
    video_vpss = video_encode->video_vpss;
#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
    if (video_vpss == NULL || video_vpss->vpssgrp == NULL)
    {
        if (ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("vpss or vpssgrp NULL");
        }
        return ERROR;
    }
    video_vpssgrp = video_vpss->vpssgrp;
    if (video_vpssgrp == NULL || video_vpssgrp->video_input == NULL)
    {
        if (ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("vpssgrp or input channel NULL");
        }
        return ERROR;
    }
    video_input = video_vpssgrp->video_input;
#else
    if (video_vpss == NULL || video_vpss->video_input == NULL)
    {
        if (ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("vpssgrp or input channel NULL");
        }
        return ERROR;
    }
    video_input = video_vpss->video_input;
#endif
    video_input_pipe = video_input->inputpipe;
    if (video_input_pipe == NULL)
    {
        if (ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("input pipe NULL");
        }
        return ERROR;
    }
    if (zpl_video_input_pipe_start(video_input_pipe) != OK)
    {
        return ERROR;
    }
    if (zpl_video_input_channel_start(video_input) != OK)
    {
        return ERROR;
    }
#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
    if (zpl_video_vpssgrp_start(video_vpssgrp) != OK)
    {
        return ERROR;
    }
#endif
    if (zpl_video_vpss_channel_start(video_vpss) != OK)
    {
        return ERROR;
    }
    if (zpl_video_encode_start(video_encode) != OK)
    {
        return ERROR;
    }
    if (VIDHAL_RES_FLAG_CHECK(video_vpssgrp->res_flag, DSTBIND) && !video_vpssgrp->hwbind)
    {
        zpl_media_debugmsg_debug("====input -> vpss bind ===%d->%d->%d", video_input_pipe->input_pipe, video_input->input_chn, video_vpssgrp->vpss_group);
        if (zpl_syshal_input_bind_vpss(video_input_pipe->input_pipe, video_input->input_chn, video_vpssgrp->vpss_group) == OK)
        {
            zpl_video_input_pipe_read_stop(video_input_pipe);
            zpl_video_input_channel_read_stop(video_input);
        }
        video_vpssgrp->hwbind = zpl_true;
        video_input->hwbind = zpl_true;
    }
    if (VIDHAL_RES_FLAG_CHECK(video_encode->res_flag, DSTBIND) && !video_encode->hwbind)
    {
        zpl_media_debugmsg_debug("====vpss -> venc bind ===%d->%d->%d", video_vpssgrp->vpss_group, video_vpss->vpss_channel, video_encode->venc_channel);
        if (zpl_syshal_vpss_bind_venc(video_vpssgrp->vpss_group, video_vpss->vpss_channel, video_encode->venc_channel) == OK)
            zpl_video_vpss_channel_read_stop(video_vpss);
        video_vpss->hwbind = zpl_true;
        video_encode->hwbind = zpl_true;
    }
    return OK;
}

int zpl_media_hal_stop(zpl_media_channel_t *chn)
{
    zpl_video_encode_t *video_encode = NULL;
    zpl_video_vpss_channel_t *video_vpss = NULL;
    zpl_video_input_channel_t *video_input = NULL;
    zpl_video_input_pipe_t *video_input_pipe = NULL;
#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
    zpl_video_vpssgrp_t *video_vpssgrp = NULL;
#endif
    video_encode = chn->video_media.halparam;
    if (video_encode == NULL || video_encode->video_vpss == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("video_encode channel or vpss NULL");
        } 
        return ERROR;
    } 
    video_vpss = video_encode->video_vpss;
#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
    if (video_vpss == NULL || video_vpss->vpssgrp == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("vpss or vpssgrp NULL");
        } 
        return ERROR;
    } 
    video_vpssgrp = video_vpss->vpssgrp;
    if (video_vpssgrp == NULL || video_vpssgrp->video_input == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("vpssgrp or input channel NULL");
        } 
        return ERROR;
    } 
    video_input = video_vpssgrp->video_input;
#else
    if (video_vpss == NULL || video_vpss->video_input == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("vpssgrp or input channel NULL");
        } 
        return ERROR;
    } 
    video_input = video_vpss->video_input;
#endif
    video_input_pipe = video_input->inputpipe;
    if (video_input_pipe == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("input pipe NULL");
        } 
        return ERROR;
    } 
    if (zpl_video_encode_stop(video_encode) != OK)
    {
        return ERROR;
    }
    if (zpl_video_vpss_channel_stop(video_vpss) != OK)
    {
        return ERROR;
    }
#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
    if (zpl_video_vpssgrp_stop(video_vpssgrp) != OK)
    {
        return ERROR;
    }
#endif
    if (zpl_video_input_channel_stop(video_input) != OK)
    {
        return ERROR;
    }
    if (zpl_video_input_pipe_stop(video_input_pipe) != OK)
    {
        return ERROR;
    }    
    return OK;
}

int zpl_media_hal_inactive(zpl_media_channel_t *chn)
{
    zpl_video_encode_t *video_encode = NULL;
    zpl_video_vpss_channel_t *video_vpss = NULL;
    zpl_video_input_channel_t *video_input = NULL;
    zpl_video_input_pipe_t *video_input_pipe = NULL;
#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
    zpl_video_vpssgrp_t *video_vpssgrp = NULL;
#endif
    video_encode = chn->video_media.halparam;
    if (video_encode == NULL || video_encode->video_vpss == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("video_encode channel or vpss NULL");
        } 
        return ERROR;
    } 
    video_vpss = video_encode->video_vpss;
#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
    if (video_vpss == NULL || video_vpss->vpssgrp == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("vpss or vpssgrp NULL");
        } 
        return ERROR;
    } 
    video_vpssgrp = video_vpss->vpssgrp;
    if (video_vpssgrp == NULL || video_vpssgrp->video_input == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("vpssgrp or input channel NULL");
        } 
        return ERROR;
    } 
    video_input = video_vpssgrp->video_input;
#else
    if (video_vpss == NULL || video_vpss->video_input == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("vpssgrp or input channel NULL");
        } 
        return ERROR;
    } 
    video_input = video_vpss->video_input;
#endif
    video_input_pipe = video_input->inputpipe;
    if (video_input_pipe == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("input pipe NULL");
        } 
        return ERROR;
    } 
    if (zpl_video_encode_inactive(video_encode) != OK)
    {
        return ERROR;
    }

    if (zpl_video_vpss_channel_inactive(video_vpss) != OK)
    {
        return ERROR;
    }
#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
    if (zpl_video_vpssgrp_inactive(video_vpssgrp) != OK)
    {
        return ERROR;
    }
#endif
    if (zpl_video_input_channel_inactive(video_input) != OK)
    {
        return ERROR;
    }
    if (zpl_video_input_pipe_inactive(video_input_pipe) != OK)
    {
        return ERROR;
    }
    return OK;
}

int zpl_media_hal_encode_hwbind(zpl_media_channel_t *chn, zpl_bool bind)
{
    zpl_video_encode_t *video_encode = NULL;
    zpl_video_vpss_channel_t *video_vpss = NULL;
    zpl_video_input_channel_t *video_input = NULL;
    zpl_video_input_pipe_t *video_input_pipe = NULL;
#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
    zpl_video_vpssgrp_t *video_vpssgrp = NULL;
#endif
    video_encode = chn->video_media.halparam;
    if (video_encode == NULL || video_encode->video_vpss == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("video_encode channel or vpss NULL");
        } 
        return ERROR;
    } 
    video_vpss = video_encode->video_vpss;
#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
    if (video_vpss == NULL || video_vpss->vpssgrp == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("vpss or vpssgrp NULL");
        } 
        return ERROR;
    } 
    video_vpssgrp = video_vpss->vpssgrp;
    if (video_vpssgrp == NULL || video_vpssgrp->video_input == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("vpssgrp or input channel NULL");
        } 
        return ERROR;
    } 
    video_input = video_vpssgrp->video_input;
#else
    if (video_vpss == NULL || video_vpss->video_input == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("vpssgrp or input channel NULL");
        } 
        return ERROR;
    } 
    video_input = video_vpss->video_input;
#endif
    video_input_pipe = video_input->inputpipe;
    if (video_input_pipe == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("input pipe NULL");
        } 
        return ERROR;
    }     
    if(bind)
    {
        if(VIDHAL_RES_FLAG_CHECK(video_encode->res_flag, DSTBIND) && !video_encode->hwbind)
        {
            zpl_media_debugmsg_debug("====vpss -> venc bind ===%d->%d->%d", video_vpssgrp->vpss_group, video_vpss->vpss_channel, video_encode->venc_channel);
            if(zpl_syshal_vpss_bind_venc(video_vpssgrp->vpss_group, video_vpss->vpss_channel, video_encode->venc_channel) == OK)
                zpl_video_vpss_channel_read_stop(video_vpss);
            video_vpss->hwbind = zpl_true;    
            //video_encode->hwbind = zpl_true;
        }
    }
    else
    {
        if(VIDHAL_RES_FLAG_CHECK(video_encode->res_flag, DSTBIND) && video_encode->hwbind)
        {
            zpl_media_debugmsg_debug("====vpss -> venc unbind ===%d->%d->%d", video_vpssgrp->vpss_group, video_vpss->vpss_channel, video_encode->venc_channel);
            if(zpl_syshal_vpss_unbind_venc(video_vpssgrp->vpss_group, video_vpss->vpss_channel, video_encode->venc_channel) == OK)
            {
                if(video_vpss->t_master)
                    zpl_video_vpss_channel_read_start(video_vpss->t_master, video_vpss);
            }
            if(video_vpss->bindcount == 1)
                video_vpss->hwbind = zpl_false;
            //video_encode->hwbind = zpl_false;
        } 
    }
    return OK;
}

int zpl_media_hal_vpss_hwbind(zpl_media_channel_t *chn, zpl_bool bind)
{
    zpl_video_encode_t *video_encode = NULL;
    zpl_video_vpss_channel_t *video_vpss = NULL;
    zpl_video_input_channel_t *video_input = NULL;
    zpl_video_input_pipe_t *video_input_pipe = NULL;
#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
    zpl_video_vpssgrp_t *video_vpssgrp = NULL;
#endif
    video_encode = chn->video_media.halparam;
    if (video_encode == NULL || video_encode->video_vpss == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("video_encode channel or vpss NULL");
        } 
        return ERROR;
    } 
    video_vpss = video_encode->video_vpss;
#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
    if (video_vpss == NULL || video_vpss->vpssgrp == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("vpss or vpssgrp NULL");
        } 
        return ERROR;
    } 
    video_vpssgrp = video_vpss->vpssgrp;
    if (video_vpssgrp == NULL || video_vpssgrp->video_input == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("vpssgrp or input channel NULL");
        } 
        return ERROR;
    } 
    video_input = video_vpssgrp->video_input;
#else
    if (video_vpss == NULL || video_vpss->video_input == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("vpssgrp or input channel NULL");
        } 
        return ERROR;
    } 
    video_input = video_vpss->video_input;
#endif
    video_input_pipe = video_input->inputpipe;
    if (video_input_pipe == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("input pipe NULL");
        } 
        return ERROR;
    }     
    if(bind)
    {
        if(VIDHAL_RES_FLAG_CHECK(video_vpssgrp->res_flag, DSTBIND) && !video_vpssgrp->hwbind)
        {
            zpl_media_debugmsg_debug("====input -> vpss bind ===%d->%d->%d", video_input_pipe->input_pipe, video_input->input_chn, video_vpssgrp->vpss_group);
            if(zpl_syshal_input_bind_vpss(video_input_pipe->input_pipe, video_input->input_chn, video_vpssgrp->vpss_group) == OK)
            {
                zpl_video_input_pipe_read_stop(video_input_pipe);
                zpl_video_input_channel_read_stop(video_input);
            }
            //video_vpssgrp->hwbind = zpl_true;
            video_input->hwbind = zpl_true;
        }
    }
    else
    {
        if(VIDHAL_RES_FLAG_CHECK(video_vpssgrp->res_flag, DSTBIND) && video_vpssgrp->hwbind)
        {
            zpl_media_debugmsg_debug("====input -> vpss unbind ===%d->%d->%d", video_input_pipe->input_pipe, video_input->input_chn, video_vpssgrp->vpss_group);
            if(zpl_syshal_input_unbind_vpss(video_input_pipe->input_pipe, video_input->input_chn, video_vpssgrp->vpss_group) == OK)
            {
                if(video_input_pipe->t_master)
                    zpl_video_input_pipe_read_start(video_input_pipe->t_master, video_input_pipe);
                if(video_input->t_master)
                    zpl_video_input_channel_read_start(video_input->t_master, video_input); 
            }
            if(video_input->bindcount == 1)
                video_input->hwbind = zpl_false;
            //video_vpssgrp->hwbind = zpl_false;
        } 
    }
    return OK;
}

int zpl_media_hal_read_stop(ZPL_MEDIA_NODE_E module, zpl_media_channel_t *chn)
{
    int ret = -1;
    zpl_video_encode_t *video_encode = NULL;
    zpl_video_vpss_channel_t *video_vpss = NULL;
    zpl_video_input_channel_t *video_input = NULL;
    zpl_video_input_pipe_t *video_input_pipe = NULL;
#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
    zpl_video_vpssgrp_t *video_vpssgrp = NULL;
#endif
    video_encode = chn->video_media.halparam;
    if (video_encode == NULL || video_encode->video_vpss == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("video_encode channel or vpss NULL");
        } 
        return ERROR;
    } 
    video_vpss = video_encode->video_vpss;
#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
    if (video_vpss == NULL || video_vpss->vpssgrp == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("vpss or vpssgrp NULL");
        } 
        return ERROR;
    } 
    video_vpssgrp = video_vpss->vpssgrp;
    if (video_vpssgrp == NULL || video_vpssgrp->video_input == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("vpssgrp or input channel NULL");
        } 
        return ERROR;
    } 
    video_input = video_vpssgrp->video_input;
#else
    if (video_vpss == NULL || video_vpss->video_input == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("vpssgrp or input channel NULL");
        } 
        return ERROR;
    } 
    video_input = video_vpss->video_input;
#endif
    video_input_pipe = video_input->inputpipe;
    if (video_input_pipe == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("input pipe NULL");
        } 
        return ERROR;
    } 
    switch(module)
    {
    case ZPL_MEDIA_NODE_PIPE:
    ret = zpl_video_input_pipe_read_stop(video_input_pipe);
    break;
    case ZPL_MEDIA_NODE_INPUT:
    ret = zpl_video_input_channel_read_stop(video_input);
    break;
    //case ZPL_MEDIA_NODE_PROCESS:  
    //ret = zpl_video_vpss_channel_read_stop(video_vpss);
    //break;  
    case ZPL_MEDIA_NODE_PROCESS: 
    ret = zpl_video_vpss_channel_read_stop(video_vpss);
    break;
    case ZPL_MEDIA_NODE_ENCODE:
    ret = zpl_video_encode_read_stop(video_encode);
    break;
    default:
    break;
    }
    return ret;
}

int zpl_media_hal_read_start(ZPL_MEDIA_NODE_E module, zpl_void *master, zpl_media_channel_t *chn)
{
    int ret = -1;
    zpl_video_encode_t *video_encode = NULL;
    zpl_video_vpss_channel_t *video_vpss = NULL;
    zpl_video_input_channel_t *video_input = NULL;
    zpl_video_input_pipe_t *video_input_pipe = NULL;
#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
    zpl_video_vpssgrp_t *video_vpssgrp = NULL;
#endif
    video_encode = chn->video_media.halparam;
    if (video_encode == NULL || video_encode->video_vpss == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("video_encode channel or vpss NULL");
        } 
        return ERROR;
    } 
    video_vpss = video_encode->video_vpss;
#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
    if (video_vpss == NULL || video_vpss->vpssgrp == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("vpss or vpssgrp NULL");
        } 
        return ERROR;
    } 
    video_vpssgrp = video_vpss->vpssgrp;
    if (video_vpssgrp == NULL || video_vpssgrp->video_input == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("vpssgrp or input channel NULL");
        } 
        return ERROR;
    } 
    video_input = video_vpssgrp->video_input;
#else
    if (video_vpss == NULL || video_vpss->video_input == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("vpssgrp or input channel NULL");
        } 
        return ERROR;
    } 
    video_input = video_vpss->video_input;
#endif
    video_input_pipe = video_input->inputpipe;
    if (video_input_pipe == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zpl_media_debugmsg_error("input pipe NULL");
        } 
        return ERROR;
    } 
    switch(module)
    {
    case ZPL_MEDIA_NODE_PIPE:
    if(video_input_pipe->hwbind == zpl_false)
    //if(!VIDHAL_RES_FLAG_CHECK(video_input_pipe->res_flag, SRCBIND))
        ret = zpl_video_input_pipe_read_start(master, video_input_pipe);
    break;
    case ZPL_MEDIA_NODE_INPUT:
    if(video_input->hwbind == zpl_false)
    //if(!VIDHAL_RES_FLAG_CHECK(video_input->res_flag, SRCBIND))
        ret = zpl_video_input_channel_read_start(master, video_input);
    break;
    //case ZPL_MEDIA_HARDADAP_VPSSGRP:  
    //ret = zpl_video_vpss_channel_read_start(master, video_vpss);
    //break;  
    case ZPL_MEDIA_NODE_PROCESS: 
    if(video_vpss->hwbind == zpl_false)
    //if(!VIDHAL_RES_FLAG_CHECK(video_vpss->res_flag, DSTBIND))
        ret = zpl_video_vpss_channel_read_start(master, video_vpss);
    break;
    case ZPL_MEDIA_NODE_ENCODE:
    //if(video_encode->hwbind == zpl_false)
        ret = zpl_video_encode_read_start(master, video_encode);
    break;
    default:
    break;
    }
    return ret;
}

int zpl_media_hal_sys_init()
{
    zpl_syshal_vbmem_init();
    /*
    zpl_syshal_vivpss_mode_set(0, VI_OFFLINE_VPSS_OFFLINE);
    int zpl_vidhal_mipi_start(zpl_int32 snsdev, zpl_int32 mipmdev);
    int zpl_vidhal_isp_start(zpl_vidhal_isp_sensor_t *sensor);
    */
    return OK;
}
