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

static LIST *video_encode_list = NULL;
static os_mutex_t *video_encode_mutex = NULL;



int zpl_video_encode_init(void)
{
	if(video_encode_list == NULL)
	{
		video_encode_list = os_malloc(sizeof(LIST));
		if(video_encode_list)
		{
			lstInit(video_encode_list);
		}
		else
			return ERROR;
	}
	if(video_encode_mutex == NULL)
	{
		video_encode_mutex = os_mutex_name_init("video_encode_mutex");
	}
	return ERROR;
}

int zpl_video_encode_exit(void)
{
	if(video_encode_mutex)
	{
		if(os_mutex_exit(video_encode_mutex)==OK)
			video_encode_mutex = NULL;
	}
	if(video_encode_list)
	{
		lstFree(video_encode_list);
		video_encode_list = NULL;
	}

	return OK;
}


int zpl_video_encode_destroy(zpl_int32 venc_channel)
{
	NODE node;
	zpl_video_encode_t *t = NULL;
	if (video_encode_mutex)
		os_mutex_lock(video_encode_mutex, OS_WAIT_FOREVER);

	for(t = (zpl_video_encode_t *)lstFirst(video_encode_list); 
		t != NULL; t = (zpl_video_encode_t *)lstNext(&node))
	{
		node = t->node;
		if(t && t->venc_channel == venc_channel)
		{
			lstDelete (video_encode_list, (NODE *)t);
#ifdef ZPL_VENC_READ_DEBUG
		if(t->mem_buf)       
    		os_free(t->mem_buf);       
#endif
			os_free(t);	
			break;
		}
	}
	if(video_encode_mutex)
		os_mutex_unlock(video_encode_mutex);
	return OK;
}

int zpl_video_encode_create(zpl_int32 venc_channel)
{
	zpl_video_encode_t *t = os_malloc(sizeof(zpl_video_encode_t));
	zpl_video_assert(t);
	if(t)
	{
		memset(t, 0, sizeof(zpl_video_encode_t));
		t->venc_channel = venc_channel;
#ifdef ZPL_VENC_READ_DEBUG
		t->mem_buf = os_malloc(ZPL_VENC_READ_DEBUG); 
		if(t->mem_buf)       
    		t->mem_size = ZPL_VENC_READ_DEBUG;       
#endif

		if(video_encode_mutex)
			os_mutex_lock(video_encode_mutex, OS_WAIT_FOREVER);
		if(video_encode_list)
			lstAdd(video_encode_list, (NODE *)t);
		if(video_encode_mutex)
			os_mutex_unlock(video_encode_mutex);
		return OK;
	}
	return ERROR;
}

zpl_video_encode_t * zpl_video_encode_lookup(zpl_int32 venc_channel)
{
	NODE node;
	zpl_video_encode_t *t = NULL;
	if (video_encode_mutex)
		os_mutex_lock(video_encode_mutex, OS_WAIT_FOREVER);

	for(t = (zpl_video_encode_t *)lstFirst(video_encode_list); 
		t != NULL; t = (zpl_video_encode_t *)lstNext(&node))
	{
		node = t->node;
		if(t && t->venc_channel == venc_channel)
		{
			break;
		}
	}
	if(video_encode_mutex)
		os_mutex_unlock(video_encode_mutex);
	return t;
}

int zpl_video_encode_vpss_set(zpl_int32 venc_channel, void *halparam)
{
	zpl_video_encode_t * t = zpl_video_encode_lookup( venc_channel);
	if (video_encode_mutex)
		os_mutex_lock(video_encode_mutex, OS_WAIT_FOREVER);
	if(t)
	{
		t->video_vpss = halparam;
	}	
	if(video_encode_mutex)
		os_mutex_unlock(video_encode_mutex);
	return OK;
}

int zpl_video_encode_buffer_queue_set(zpl_int32 venc_channel, void *buffer_queue)
{
	zpl_video_encode_t * t = zpl_video_encode_lookup( venc_channel);
	if (video_encode_mutex)
		os_mutex_lock(video_encode_mutex, OS_WAIT_FOREVER);
	if(t)
	{
		t->buffer_queue = buffer_queue;
	}	
	if(video_encode_mutex)
		os_mutex_unlock(video_encode_mutex);
	return OK;	
}



#ifdef ZPL_SHELL_MODULE
int zpl_video_encode_show(void *pvoid)
{
	NODE *node;
	zpl_video_encode_t *t;	
	struct vty *vty = (struct vty *)pvoid;
	if(video_encode_list == NULL)
		return ERROR;
	if (video_encode_mutex)
		os_mutex_lock(video_encode_mutex, OS_WAIT_FOREVER);

	if(lstCount(video_encode_list))
	{
		vty_out(vty, "%-4s %-4s %-6s %-8s %-8s %-16s %s", "----", "----", "------", "--------", "--------", "----------------", VTY_NEWLINE);
		vty_out(vty, "%-4s %-4s %-6s %-8s %-8s %-16s %s", "ID", "FLAG", "ONLINE", "BINDCNT", "BINDID",  "SIZE", VTY_NEWLINE);
		vty_out(vty, "%-4s %-4s %-6s %-8s %-8s %-16s %s", "----", "----", "------", "--------", "--------", "----------------", VTY_NEWLINE);
	}
	for (node = lstFirst(video_encode_list); node != NULL; node = lstNext(node))
	{
		t = (zpl_video_encode_t *) node;
		if (node)
		{
			vty_out(vty, "%-4d 0x%-4x %-6s  %-8d %-8d %dx%d%s", t->venc_channel, t->res_flag, t->online ? "ONLINE":"OFFLINE", 
					1, t->video_vpss?((zpl_video_vpss_channel_t*)t->video_vpss)->vpss_channel:-1, 
					t->pCodec?t->pCodec->vidsize.width:-1, t->pCodec?t->pCodec->vidsize.height:-1, VTY_NEWLINE);
		}
	}
	if (video_encode_mutex)
		os_mutex_unlock(video_encode_mutex);
	return OK;
}
#endif









/* 编码模块接收到数据后完成编码，编码输出数据有这里接收，接收到编码后的数据发送到通道的消息队列，*/
static int zpl_video_encode_read(struct thread *t)
{
	zpl_video_encode_t *encode = THREAD_ARG(t);
    if(encode && encode->media_channel)
    {
		if (video_encode_mutex)
			os_mutex_lock(video_encode_mutex, OS_WAIT_FOREVER);
        encode->t_read = NULL;

        //zpl_vidhal_venc_frame_recvfrom(((zpl_media_channel_t*)encode->media_channel)->channel, 
		//	((zpl_media_channel_t*)encode->media_channel)->channel_index, encode);
		zpl_vidhal_venc_frame_recvfrom_one(((zpl_media_channel_t*)encode->media_channel)->channel, 
			((zpl_media_channel_t*)encode->media_channel)->channel_index, encode);
#ifdef ZPL_VENC_READ_DEBUG
#else /* ZPL_VENC_READ_DEBUG */
		if(encode->buffer_queue)
		{
			//zpl_media_debugmsg_warn(" =====================zpl_video_encode_read:zpl_media_buffer_distribute");
			zpl_media_buffer_distribute(encode->buffer_queue);
		}
#endif /* ZPL_VENC_READ_DEBUG */
        if(!ipstack_invalid(encode->vencfd))
           encode->t_read = thread_add_read(t->master, zpl_video_encode_read, encode, encode->vencfd);
		if (video_encode_mutex)
			os_mutex_unlock(video_encode_mutex);   
    }
	return OK;
}


int zpl_video_encode_read_start(zpl_void *master, zpl_video_encode_t *encode)
{
    zpl_video_assert(master);
    zpl_video_assert(encode);
    zpl_video_assert(encode->media_channel);
	if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, DETAIL))
		zpl_media_debugmsg_debug(" start video encode channel %d read thread\n", encode->venc_channel);
	if (video_encode_mutex)
		os_mutex_lock(video_encode_mutex, OS_WAIT_FOREVER);
	zpl_vidhal_venc_update_fd(((zpl_media_channel_t*)encode->media_channel)->channel, 
			((zpl_media_channel_t*)encode->media_channel)->channel_index, encode);
    if(master && encode && !ipstack_invalid(encode->vencfd))
	{
		encode->t_master = master;
		//zpl_media_debugmsg_debug(" =============== encode channel %d read thread  thread_add_read(%d)\n", encode->venc_channel, encode->vencfd);
        encode->t_read = thread_add_read(master, zpl_video_encode_read, encode, encode->vencfd);
	}
	if (video_encode_mutex)
		os_mutex_unlock(video_encode_mutex);
    return OK;    
}

int zpl_video_encode_read_stop(zpl_video_encode_t *encode)
{
    zpl_video_assert(encode);
    zpl_video_assert(encode->media_channel);  
	if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, DETAIL))
		zpl_media_debugmsg_debug(" stop video encode channel %d read thread\n", encode->venc_channel);
	if (video_encode_mutex)
		os_mutex_lock(video_encode_mutex, OS_WAIT_FOREVER);
    if(encode && encode->t_read)
    {
        thread_cancel(encode->t_read);
        encode->t_read = NULL;
    }
	if (video_encode_mutex)
		os_mutex_unlock(video_encode_mutex);
    return OK; 
}




int zpl_video_encode_sendto(zpl_video_encode_t *encode,  void *p, zpl_int timeout)
{
    int ret = -1;
    zpl_video_assert(encode);
    zpl_video_assert(encode->media_channel);
	if (video_encode_mutex)
		os_mutex_lock(video_encode_mutex, OS_WAIT_FOREVER);
    if(encode->online)
        ret = zpl_vidhal_venc_frame_sendto(encode, 0, encode->venc_channel, p, timeout);
    else
        zpl_media_debugmsg_warn("Channel (%d %d) is offline", 0, 
            encode->venc_channel);
	if (video_encode_mutex)
		os_mutex_unlock(video_encode_mutex);
    return ret;
}





int zpl_video_encode_active(zpl_video_encode_t *encode)
{
    int ret = -1;
    zpl_video_assert(encode);
	zpl_video_assert(encode->media_channel);
	if (video_encode_mutex)
		os_mutex_lock(video_encode_mutex, OS_WAIT_FOREVER);
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
	if (video_encode_mutex)
		os_mutex_unlock(video_encode_mutex);
    return ret;
}

int zpl_video_encode_start(zpl_video_encode_t *encode)
{
    int ret = -1;
    zpl_video_assert(encode);
	zpl_video_assert(encode->media_channel);
	if (video_encode_mutex)
		os_mutex_lock(video_encode_mutex, OS_WAIT_FOREVER);
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
	if (video_encode_mutex)
		os_mutex_unlock(video_encode_mutex);
	zpl_video_encode_enable_IDR(encode, zpl_true);
	//zpl_video_encode_read_start(zpl_void *master, zpl_video_encode_t *encode)
    return ret;
}

int zpl_video_encode_stop(zpl_video_encode_t *encode)
{
    int ret = -1;
    zpl_video_assert(encode);
	zpl_video_assert(encode->media_channel);
	if (video_encode_mutex)
		os_mutex_lock(video_encode_mutex, OS_WAIT_FOREVER);
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
	if (video_encode_mutex)
		os_mutex_unlock(video_encode_mutex);
    return ret;
}

int zpl_video_encode_inactive(zpl_video_encode_t *encode)
{
    int ret = -1;
    zpl_video_assert(encode);
	zpl_video_assert(encode->media_channel);
	if (video_encode_mutex)
		os_mutex_lock(video_encode_mutex, OS_WAIT_FOREVER);
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
	if (video_encode_mutex)
		os_mutex_unlock(video_encode_mutex);
    return ret;
}

int zpl_video_encode_online_set(zpl_video_encode_t *encode, zpl_bool online)
{
    int ret = -1;
    zpl_video_assert(encode);
	zpl_video_assert(encode->media_channel);
	if (video_encode_mutex)
		os_mutex_lock(video_encode_mutex, OS_WAIT_FOREVER);
    ret = OK;
	encode->online = online;
	if (video_encode_mutex)
		os_mutex_unlock(video_encode_mutex);
    return ret;
}

int zpl_video_encode_online_get(zpl_video_encode_t *encode, zpl_bool *online)
{
    int ret = -1;
    zpl_video_assert(encode);
	zpl_video_assert(encode->media_channel);
	if (video_encode_mutex)
		os_mutex_lock(video_encode_mutex, OS_WAIT_FOREVER);
    ret = OK;
	if(online)
		*online = encode->online;
	if (video_encode_mutex)
		os_mutex_unlock(video_encode_mutex);
    return ret;
}


int zpl_video_encode_request_IDR(zpl_video_encode_t *encode)
{
	int ret = -1;
    zpl_video_assert(encode);
    zpl_video_assert(encode->media_channel);
	//if (video_encode_mutex)
	//	os_mutex_lock(video_encode_mutex, OS_WAIT_FOREVER);
    ret = zpl_vidhal_venc_request_IDR(((zpl_media_channel_t*)encode->media_channel)->channel, 
		((zpl_media_channel_t*)encode->media_channel)->channel_index, encode);
	if(ZPL_MEDIA_DEBUG(ENCODE, EVENT))
	{
		zpl_media_debugmsg_debug("request IDR encode channel(%d) ret=%d", encode->venc_channel, ret);
	} 
	//if (video_encode_mutex)
	//	os_mutex_unlock(video_encode_mutex);
	return ret;	
}

int zpl_video_encode_enable_IDR(zpl_video_encode_t *encode, zpl_bool bEnableIDR)
{
	int ret = -1;
    zpl_video_assert(encode);
    zpl_video_assert(encode->media_channel);
	if (video_encode_mutex)
		os_mutex_lock(video_encode_mutex, OS_WAIT_FOREVER);
    ret = zpl_vidhal_venc_enable_IDR(((zpl_media_channel_t*)encode->media_channel)->channel, 
		((zpl_media_channel_t*)encode->media_channel)->channel_index, encode, bEnableIDR);
	if(ZPL_MEDIA_DEBUG(ENCODE, EVENT))
	{
		zpl_media_debugmsg_debug("enable IDR encode channel(%d) ret=%d", encode->venc_channel, ret);
	} 
	if (video_encode_mutex)
		os_mutex_unlock(video_encode_mutex);
	return ret;	
}

int zpl_video_encode_encode_reset(zpl_video_encode_t *encode)
{
	int ret = -1;
    zpl_video_assert(encode);
    zpl_video_assert(encode->media_channel);
	if (video_encode_mutex)
		os_mutex_lock(video_encode_mutex, OS_WAIT_FOREVER);
    ret = zpl_vidhal_venc_reset(((zpl_media_channel_t*)encode->media_channel)->channel, 
		((zpl_media_channel_t*)encode->media_channel)->channel_index, encode);
	if(ZPL_MEDIA_DEBUG(ENCODE, EVENT))
	{
		zpl_media_debugmsg_debug("reset IDR encode channel(%d) ret=%d", encode->venc_channel, ret);
	} 
	if (video_encode_mutex)
		os_mutex_unlock(video_encode_mutex);
	return ret;	
}



