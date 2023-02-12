/*
 * zpl_video_encode.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"


static int zpl_media_video_encode_cmp(zpl_media_video_encode_t *encode, zpl_media_gkey_t *gkey)
{
	if(encode->venc_channel == gkey->channel)
		return 0;
	return 1;	
}

int zpl_media_video_encode_init(void)
{
	zpl_media_global_gkey_cmpset(ZPL_MEDIA_GLOAL_VIDEO_ENCODE, zpl_media_video_encode_cmp);
	return OK;
}

int zpl_media_video_encode_new(zpl_int32 venc_channel)
{
	zpl_media_video_encode_t *t = os_malloc(sizeof(zpl_media_video_encode_t));
	zpl_video_assert(t);
	if(t)
	{
		memset(t, 0, sizeof(zpl_media_video_encode_t));
		t->venc_channel = venc_channel;
#ifdef ZPL_VENC_READ_DEBUG
		t->mem_buf = os_malloc(ZPL_VENC_READ_DEBUG); 
		if(t->mem_buf)       
    		t->mem_size = ZPL_VENC_READ_DEBUG;       
#endif
		zpl_media_global_add(ZPL_MEDIA_GLOAL_VIDEO_ENCODE, t);
		return OK;
	}
	return ERROR;
}

int zpl_media_video_encode_delete(zpl_int32 venc_channel)
{
	zpl_media_video_encode_t * t = (zpl_media_video_encode_t*)zpl_media_global_lookup(ZPL_MEDIA_GLOAL_VIDEO_ENCODE, venc_channel, 0, 0);
	if(t)
	{
		zpl_media_global_del(ZPL_MEDIA_GLOAL_VIDEO_ENCODE, t);
		os_free(t);	
	}
	return OK;
}

zpl_media_video_encode_t * zpl_media_video_encode_lookup(zpl_int32 venc_channel)
{
	zpl_media_video_encode_t * t = (zpl_media_video_encode_t*)zpl_media_global_lookup(ZPL_MEDIA_GLOAL_VIDEO_ENCODE, venc_channel, 0, 0);
	return t;
}

int zpl_media_video_encode_vpss_set(zpl_int32 venc_channel, void *halparam)
{
	zpl_media_video_encode_t * t = (zpl_media_video_encode_t*)zpl_media_global_lookup(ZPL_MEDIA_GLOAL_VIDEO_ENCODE, venc_channel, 0, 0);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	if(t)
	{
		t->video_vpss = halparam;
	}	
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	return OK;
}

int zpl_media_video_encode_frame_queue_set(zpl_int32 venc_channel, void *frame_queue)
{
	zpl_media_video_encode_t * t = (zpl_media_video_encode_t*)zpl_media_global_lookup(ZPL_MEDIA_GLOAL_VIDEO_ENCODE, venc_channel, 0, 0);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	if(t)
	{
		t->frame_queue = frame_queue;
	}	
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	return OK;	
}



#ifdef ZPL_SHELL_MODULE
int zpl_media_video_encode_show(void *pvoid)
{
	NODE *node;
	zpl_media_video_encode_t *t;	
	struct vty *vty = (struct vty *)pvoid;
    LIST *_lst = NULL;
    os_mutex_t *_mutex = NULL;
    zpl_media_global_get(ZPL_MEDIA_GLOAL_VIDEO_ENCODE, &_lst, &_mutex);

	if(_lst == NULL)
		return ERROR;
	if (_mutex)
		os_mutex_lock(_mutex, OS_WAIT_FOREVER);

	if(lstCount(_lst))
	{
		vty_out(vty, "%-4s %-4s %-6s %-8s %-8s %-16s %s", "----", "----", "------", "--------", "--------", "----------------", VTY_NEWLINE);
		vty_out(vty, "%-4s %-4s %-6s %-8s %-8s %-16s %s", "ID", "FLAG", "ONLINE", "BINDCNT", "BINDID",  "SIZE", VTY_NEWLINE);
		vty_out(vty, "%-4s %-4s %-6s %-8s %-8s %-16s %s", "----", "----", "------", "--------", "--------", "----------------", VTY_NEWLINE);
	}
	for (node = lstFirst(_lst); node != NULL; node = lstNext(node))
	{
		t = (zpl_media_video_encode_t *) node;
		if (node)
		{
			vty_out(vty, "%-4d 0x%-4x %-6s  %-8d %-8d %dx%d%s", t->venc_channel, t->res_flag, t->online ? "ONLINE":"OFFLINE", 
					1, t->video_vpss?((zpl_media_video_vpss_channel_t*)t->video_vpss)->vpss_channel:-1, 
					t->pCodec?t->pCodec->vidsize.width:-1, t->pCodec?t->pCodec->vidsize.height:-1, VTY_NEWLINE);
		}
	}
	if (_mutex)
		os_mutex_unlock(_mutex);
	return OK;
}
#endif



/* 编码模块接收到数据后完成编码，编码输出数据有这里接收，接收到编码后的数据发送到通道的消息队列，*/
static int zpl_media_video_encode_read_thread(struct thread *t)
{
	zpl_media_video_encode_t *encode = THREAD_ARG(t);
	ZPL_MEDIA_CHANNEL_E         channel;	        //通道号
	ZPL_MEDIA_CHANNEL_TYPE_E 	channel_index;	    //码流类型

    if(encode && encode->media_channel)
    {
		zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
        encode->t_read = NULL;
		channel = ((zpl_media_channel_t*)encode->media_channel)->channel;
		channel_index = ((zpl_media_channel_t*)encode->media_channel)->channel_index;
        //zpl_vidhal_venc_frame_recvfrom(((zpl_media_channel_t*)encode->media_channel)->channel, 
		//	((zpl_media_channel_t*)encode->media_channel)->channel_index, encode);
		zpl_vidhal_venc_frame_recvfrom_one(channel, channel_index, encode);
#ifdef ZPL_VENC_READ_DEBUG
#else /* ZPL_VENC_READ_DEBUG */
		if(encode->frame_queue)
		{
			zpl_media_bufqueue_signal(channel, channel_index);
			//zpl_media_debugmsg_warn(" =====================zpl_media_video_encode_read_thread:zpl_media_buffer_distribute");
			//zpl_media_buffer_distribute(encode->frame_queue);
		}
#endif /* ZPL_VENC_READ_DEBUG */
        if(!ipstack_invalid(encode->vencfd))
           encode->t_read = thread_add_read(t->master, zpl_media_video_encode_read_thread, encode, encode->vencfd);
		
		zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE); 
    }
	return OK;
}


int zpl_media_video_encode_read_start(zpl_void *master, zpl_media_video_encode_t *encode)
{
    zpl_video_assert(master);
    zpl_video_assert(encode);
    zpl_video_assert(encode->media_channel);
	if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, DETAIL))
		zpl_media_debugmsg_debug(" start video encode channel %d read thread\n", encode->venc_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	zpl_vidhal_venc_update_fd(((zpl_media_channel_t*)encode->media_channel)->channel, 
			((zpl_media_channel_t*)encode->media_channel)->channel_index, encode);
    if(master && encode && !ipstack_invalid(encode->vencfd))
	{
		encode->t_master = master;
		//zpl_media_debugmsg_debug(" =============== encode channel %d read thread  thread_add_read(%d)\n", encode->venc_channel, encode->vencfd);
        encode->t_read = thread_add_read(master, zpl_media_video_encode_read_thread, encode, encode->vencfd);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
    return OK;    
}

int zpl_media_video_encode_read_stop(zpl_media_video_encode_t *encode)
{
    zpl_video_assert(encode);
    zpl_video_assert(encode->media_channel);  
	if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, DETAIL))
		zpl_media_debugmsg_debug(" stop video encode channel %d read thread\n", encode->venc_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
    if(encode && encode->t_read)
    {
        thread_cancel(encode->t_read);
        encode->t_read = NULL;
    }
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
    return OK; 
}




int zpl_media_video_encode_sendto(zpl_media_video_encode_t *encode,  void *p, zpl_int timeout)
{
    int ret = -1;
    zpl_video_assert(encode);
    zpl_video_assert(encode->media_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
    if(encode->online)
        ret = zpl_vidhal_venc_frame_sendto(encode, 0, encode->venc_channel, p, timeout);
    else
        zpl_media_debugmsg_warn("Channel (%d %d) is offline", 0, 
            encode->venc_channel);
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
    return ret;
}

static int zpl_media_video_encode_frame_putqueue(zpl_skbuffer_t *skb, zpl_media_video_encode_t *encode)
{
	ZPL_MEDIA_CHANNEL_E         channel;	        //通道号
	ZPL_MEDIA_CHANNEL_TYPE_E 	channel_index;	    //码流类型

    if(encode && encode->media_channel)
    {
		channel = ((zpl_media_channel_t*)encode->media_channel)->channel;
		channel_index = ((zpl_media_channel_t*)encode->media_channel)->channel_index;	
		if(encode->frame_queue)
		{
			zpl_skbqueue_async_enqueue(encode->frame_queue, skb);
			zpl_media_bufqueue_signal(channel, channel_index);
		}
	}
	return OK;
}

int zpl_media_video_encode_read(zpl_media_video_encode_t *encode)
{
    if(encode && encode->media_channel && encode->get_encode_frame)
    {
		zpl_skbuffer_t *skbf = NULL;
		int ret = (encode->get_encode_frame)(encode, &skbf);
		if(ret)
		{
			zpl_skbuffer_foreach(skbf, zpl_media_video_encode_frame_putqueue, encode);
		}
		return ret;
    }
	return 0;	
}



int zpl_media_video_encode_hal_create(zpl_media_video_encode_t *encode)
{
    int ret = -1;
    zpl_video_assert(encode);
	zpl_video_assert(encode->media_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	zpl_media_debugmsg_debug("venc channel (%d)  Flags (0x%x)", encode->venc_channel, encode->res_flag);		
	if(VIDHAL_RES_FLAG_CHECK(encode->res_flag, CREATE) && !VIDHAL_RES_FLAG_CHECK(encode->res_flag, INIT))	
	{
    	ret = zpl_vidhal_venc_create(((zpl_media_channel_t*)encode->media_channel)->channel, 
			((zpl_media_channel_t*)encode->media_channel)->channel_index, encode);
		if(ret == OK)	
		{
			VIDHAL_RES_FLAG_SET(encode->res_flag, INIT);
		}
		if(ZPL_MEDIA_DEBUG(ENCODE, EVENT))
		{
			zpl_media_debugmsg_debug("active encode channel(%d) ret=%d", encode->venc_channel, ret);
		} 	
	}
	else
	{
		ret = OK;
		if(ZPL_MEDIA_DEBUG(ENCODE, EVENT))
		{
			zpl_media_debugmsg_debug("encode channel(%d) is already active", encode->venc_channel);
		} 
	}	
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
    return ret;
}

int zpl_media_video_encode_start(zpl_media_video_encode_t *encode)
{
    int ret = -1;
    zpl_video_assert(encode);
	zpl_video_assert(encode->media_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	if(!VIDHAL_RES_FLAG_CHECK(encode->res_flag, START))	
	{
    	ret = zpl_vidhal_venc_start(((zpl_media_channel_t*)encode->media_channel)->channel, 
			((zpl_media_channel_t*)encode->media_channel)->channel_index, encode);
		if(ret == OK)	
		{
			VIDHAL_RES_FLAG_SET(encode->res_flag, START);
			encode->online = zpl_true;
		}
		if(ZPL_MEDIA_DEBUG(ENCODE, EVENT))
		{
			zpl_media_debugmsg_debug("start encode channel(%d) ret=%d", encode->venc_channel, ret);
		} 
	}
	else
	{
		ret = OK;
		if(ZPL_MEDIA_DEBUG(ENCODE, EVENT))
		{
			zpl_media_debugmsg_debug("encode channel(%d) is already start", encode->venc_channel);
		} 
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	zpl_media_video_encode_enable_IDR(encode, zpl_true);
	//zpl_media_video_encode_read_start(zpl_void *master, zpl_media_video_encode_t *encode)
    return ret;
}

int zpl_media_video_encode_stop(zpl_media_video_encode_t *encode)
{
    int ret = -1;
    zpl_video_assert(encode);
	zpl_video_assert(encode->media_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	if(VIDHAL_RES_FLAG_CHECK(encode->res_flag, START))
	{	
    	ret = zpl_vidhal_venc_stop(((zpl_media_channel_t*)encode->media_channel)->channel, 
			((zpl_media_channel_t*)encode->media_channel)->channel_index, encode);
		if(ret == OK)	
		{
			VIDHAL_RES_FLAG_UNSET(encode->res_flag, START);
			encode->online = zpl_false;
		}
		if(ZPL_MEDIA_DEBUG(ENCODE, EVENT))
		{
			zpl_media_debugmsg_debug("stop encode channel(%d) ret=%d", encode->venc_channel, ret);
		} 
	}
	else
	{
		ret = OK;
		if(ZPL_MEDIA_DEBUG(ENCODE, EVENT))
		{
			zpl_media_debugmsg_debug("encode channel(%d) is already stop", encode->venc_channel);
		} 
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
    return ret;
}

int zpl_media_video_encode_hal_destroy(zpl_media_video_encode_t *encode)
{
    int ret = -1;
    zpl_video_assert(encode);
	zpl_video_assert(encode->media_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	if(VIDHAL_RES_FLAG_CHECK(encode->res_flag, CREATE) && VIDHAL_RES_FLAG_CHECK(encode->res_flag, INIT))
	{	
    	ret = zpl_vidhal_venc_destroy(((zpl_media_channel_t*)encode->media_channel)->channel, 
			((zpl_media_channel_t*)encode->media_channel)->channel_index, encode);
		if(ret == OK)
			VIDHAL_RES_FLAG_UNSET(encode->res_flag, INIT);
		if(ZPL_MEDIA_DEBUG(ENCODE, EVENT))
		{
			zpl_media_debugmsg_debug("inactive encode channel(%d) ret=%d", encode->venc_channel, ret);
		} 
	}
	else
	{
		ret = OK;
		if(ZPL_MEDIA_DEBUG(ENCODE, EVENT))
		{
			zpl_media_debugmsg_debug("encode channel(%d) is already inactive", encode->venc_channel);
		} 
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
    return ret;
}

int zpl_media_video_encode_online_set(zpl_media_video_encode_t *encode, zpl_bool online)
{
    int ret = -1;
    zpl_video_assert(encode);
	zpl_video_assert(encode->media_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
    ret = OK;
	encode->online = online;
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
    return ret;
}

int zpl_media_video_encode_online_get(zpl_media_video_encode_t *encode, zpl_bool *online)
{
    int ret = -1;
    zpl_video_assert(encode);
	zpl_video_assert(encode->media_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
    ret = OK;
	if(online)
		*online = encode->online;
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
    return ret;
}


int zpl_media_video_encode_request_IDR(zpl_media_video_encode_t *encode)
{
	int ret = -1;
    zpl_video_assert(encode);
    zpl_video_assert(encode->media_channel);

    ret = zpl_vidhal_venc_request_IDR(((zpl_media_channel_t*)encode->media_channel)->channel, 
		((zpl_media_channel_t*)encode->media_channel)->channel_index, encode);
	if(ZPL_MEDIA_DEBUG(ENCODE, EVENT))
	{
		zpl_media_debugmsg_debug("request IDR encode channel(%d) ret=%d", encode->venc_channel, ret);
	} 

	return ret;	
}

int zpl_media_video_encode_enable_IDR(zpl_media_video_encode_t *encode, zpl_bool bEnableIDR)
{
	int ret = -1;
    zpl_video_assert(encode);
    zpl_video_assert(encode->media_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
    ret = zpl_vidhal_venc_enable_IDR(((zpl_media_channel_t*)encode->media_channel)->channel, 
		((zpl_media_channel_t*)encode->media_channel)->channel_index, encode, bEnableIDR);
	if(ZPL_MEDIA_DEBUG(ENCODE, EVENT))
	{
		zpl_media_debugmsg_debug("enable IDR encode channel(%d) ret=%d", encode->venc_channel, ret);
	} 
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	return ret;	
}

int zpl_media_video_encode_encode_reset(zpl_media_video_encode_t *encode)
{
	int ret = -1;
    zpl_video_assert(encode);
    zpl_video_assert(encode->media_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
    ret = zpl_vidhal_venc_reset(((zpl_media_channel_t*)encode->media_channel)->channel, 
		((zpl_media_channel_t*)encode->media_channel)->channel_index, encode);
	if(ZPL_MEDIA_DEBUG(ENCODE, EVENT))
	{
		zpl_media_debugmsg_debug("reset IDR encode channel(%d) ret=%d", encode->venc_channel, ret);
	} 
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	return ret;	
}



