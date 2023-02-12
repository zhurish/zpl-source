/*
 * zpl_video_input_pipe.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"


static int zpl_media_video_input_pipe_cmp(zpl_meida_video_input_pipe_t *encode, zpl_media_gkey_t *gkey)
{
	if(encode->input_pipe == gkey->channel && encode->input_dev == gkey->group)
		return 0;
	return 1;	
}

int zpl_meida_video_input_pipe_init(void)
{
	zpl_media_global_gkey_cmpset(ZPL_MEDIA_GLOAL_VIDEO_INPUTPIPE, zpl_media_video_input_pipe_cmp);	
	return OK;
}


int zpl_meida_video_input_pipe_destroy(zpl_int32 dev, zpl_int32 vipipe)
{
	zpl_meida_video_input_pipe_t * t = (zpl_meida_video_input_pipe_t*)zpl_media_global_lookup(ZPL_MEDIA_GLOAL_VIDEO_INPUTPIPE, vipipe, dev, 0);
	if(t)
	{
		zpl_media_global_del(ZPL_MEDIA_GLOAL_VIDEO_INPUTPIPE, t);
		os_free(t);	
	}
	return OK;
}

int zpl_meida_video_input_pipe_channel_count(zpl_int32 vipipe)
{
	int count = 0;
	NODE node;
	zpl_meida_video_input_pipe_t *t = NULL;
    LIST *_lst = NULL;
    os_mutex_t *_mutex = NULL;
    zpl_media_global_get(ZPL_MEDIA_GLOAL_VIDEO_INPUTPIPE, &_lst, &_mutex);
	if(_lst == NULL)
		return 0;
	if (_mutex)
		os_mutex_lock(_mutex, OS_WAIT_FOREVER);

	for(t = (zpl_meida_video_input_pipe_t *)lstFirst(_lst); 
		t != NULL; t = (zpl_meida_video_input_pipe_t *)lstNext(&node))
	{
		node = t->node;
		if(t && t->input_pipe == vipipe && t->input_dev >= 0)
		{
			count++;	
		}
	}
	if(_mutex)
		os_mutex_unlock(_mutex);
	return count;
}

int zpl_meida_video_input_pipe_create(zpl_int32 dev, zpl_int32 vipipe)
{
	zpl_meida_video_input_pipe_t *t = os_malloc(sizeof(zpl_meida_video_input_pipe_t));
	zpl_video_assert(t);
	if(t)
	{
		memset(t, 0, sizeof(zpl_meida_video_input_pipe_t));
    	t->input_dev = dev;                     //底层设备编号
    	t->input_pipe = vipipe;                    //底层硬件pipe
		zpl_media_global_add(ZPL_MEDIA_GLOAL_VIDEO_INPUTPIPE, t);
		return OK;
	}
	return ERROR;
}



zpl_meida_video_input_pipe_t * zpl_meida_video_input_pipe_lookup(zpl_int32 dev, zpl_int32 vipipe)
{
	zpl_meida_video_input_pipe_t * t = (zpl_meida_video_input_pipe_t*)zpl_media_global_lookup(ZPL_MEDIA_GLOAL_VIDEO_INPUTPIPE, vipipe, dev, 0);
	return t;
}




static int zpl_meida_video_input_pipe_read(struct thread *t)
{
	zpl_meida_video_input_pipe_t *input = THREAD_ARG(t);

	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUTPIPE);
    if(input)
    {
        input->t_read = NULL;
        zpl_vidhal_input_pipe_frame_recvfrom(input);
        input->t_read = thread_add_read(t->master, zpl_meida_video_input_pipe_read, input, input->pipefd);
    }
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUTPIPE);
	return OK;
}

int zpl_meida_video_input_pipe_read_start(zpl_void *master, zpl_meida_video_input_pipe_t *input)
{
    zpl_video_assert(master);
    zpl_video_assert(input);
	if(ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT) && ZPL_MEDIA_DEBUG(INPUTPIPE, DETAIL))
		zpl_media_debugmsg_debug(" start video input pipe channel %d read thread\n", input->input_pipe);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUTPIPE);
	zpl_vidhal_input_pipe_update_fd(input);
	input->t_master = master;
    if(master && input && !ipstack_invalid(input->pipefd))
        input->t_read = thread_add_read(master, zpl_meida_video_input_pipe_read, input, input->pipefd);
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUTPIPE);
	return OK;    
}

int zpl_meida_video_input_pipe_read_stop(zpl_meida_video_input_pipe_t *input)
{
    zpl_video_assert(input);  
	if(ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT) && ZPL_MEDIA_DEBUG(INPUTPIPE, DETAIL))
		zpl_media_debugmsg_debug(" stop video input pipe channel %d read thread\n", input->input_pipe);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUTPIPE);
    if(input && input->t_read)
    {
        thread_cancel(input->t_read);
        input->t_read = NULL;
    }
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUTPIPE);
    return OK; 
}




int zpl_meida_video_input_pipe_sendto(zpl_meida_video_input_pipe_t *input,  void *p, zpl_int timeout)
{
    int ret = -1;
    zpl_video_assert(input);

    return ret;
}





int zpl_meida_video_input_pipe_hal_create(zpl_meida_video_input_pipe_t *input)
{
    int ret = -1;
    zpl_video_assert(input);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUTPIPE);
	zpl_media_debugmsg_debug("input_pipe(%d)  Flags (0x%x)", input->input_pipe, input->res_flag);

	if(VIDHAL_RES_FLAG_CHECK(input->res_flag, CREATE) && !VIDHAL_RES_FLAG_CHECK(input->res_flag, INIT))
	{
    	ret = zpl_vidhal_input_pipe_create(input);
		if(ret == OK)	
			VIDHAL_RES_FLAG_SET(input->res_flag, INIT);
		if(ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT))
		{
			zpl_media_debugmsg_debug("active input pipe(%d) ret=%d", input->input_pipe, ret);
		} 
	}
	else
	{
		ret = OK;
		if(ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT))
		{
			zpl_media_debugmsg_debug("input pipe(%d) is already active", input->input_pipe);
		} 
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUTPIPE);
    return ret;
}

int zpl_meida_video_input_pipe_start(zpl_meida_video_input_pipe_t *input)
{
    int ret = -1;
    zpl_video_assert(input);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUTPIPE);
	if(!VIDHAL_RES_FLAG_CHECK(input->res_flag, START))	
	{
    	ret = zpl_vidhal_input_pipe_start(input);
		if(ret == OK)	
		{
			VIDHAL_RES_FLAG_SET(input->res_flag, START);
			input->online = zpl_true;
		}
		if(ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT))
		{
			zpl_media_debugmsg_debug("start input pipe(%d) ret=%d", input->input_pipe, ret);
		} 
	}
	else
	{
		ret = OK;
		if(ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT))
		{
			zpl_media_debugmsg_debug("input pipe(%d) is already start", input->input_pipe);
		} 
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUTPIPE);
    return ret;
}

int zpl_meida_video_input_pipe_stop(zpl_meida_video_input_pipe_t *input)
{
    int ret = -1;
    zpl_video_assert(input);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUTPIPE);
	if(input->bindcount <= 1 && VIDHAL_RES_FLAG_CHECK(input->res_flag, START))	
	{
    	ret = zpl_vidhal_input_pipe_stop(input);
		if(ret == OK)	
		{
			VIDHAL_RES_FLAG_UNSET(input->res_flag, START);
			input->online = zpl_false;
		}
		if(ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT))
		{
			zpl_media_debugmsg_debug("stop input pipe(%d) ret=%d", input->input_pipe, ret);
		} 
	}
	else
	{
		ret = OK;
		if(ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT))
		{
			zpl_media_debugmsg_debug("input pipe(%d) is already stop", input->input_pipe);
		} 
	}

	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUTPIPE);
    return ret;
}

int zpl_meida_video_input_pipe_hal_destroy(zpl_meida_video_input_pipe_t *input)
{
    int ret = -1;
    zpl_video_assert(input);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUTPIPE);
	if(VIDHAL_RES_FLAG_CHECK(input->res_flag, CREATE) && VIDHAL_RES_FLAG_CHECK(input->res_flag, INIT))
	{	
    	ret = zpl_vidhal_input_pipe_destroy(input);
		if(ret == OK)
		{
			VIDHAL_RES_FLAG_UNSET(input->res_flag, INIT);
		}
		if(ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT))
		{
			zpl_media_debugmsg_debug("inactive input pipe(%d) ret=%d", input->input_pipe, ret);
		} 
	}
	else
	{
		ret = OK;
		if(ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT))
		{
			zpl_media_debugmsg_debug("input pipe(%d) is already inactive", input->input_pipe);
		} 
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUTPIPE);
    return ret;
}

int zpl_meida_video_input_pipe_online_set(zpl_meida_video_input_pipe_t *input, zpl_bool online)
{
    int ret = -1;
    zpl_video_assert(input);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUTPIPE);
    ret = OK;
	input->online = online;
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUTPIPE);
    return ret;
}
int zpl_meida_video_input_pipe_online_get(zpl_meida_video_input_pipe_t *input, zpl_bool *online)
{
    int ret = -1;
    zpl_video_assert(input);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUTPIPE);
    ret = OK;
	if(online)
		*online = input->online;
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUTPIPE);
    return ret;
}

int zpl_meida_video_input_pipe_bindcount_set(zpl_meida_video_input_pipe_t *input, zpl_bool add)
{
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUTPIPE);
	if(input)
	{
		if(add)
			input->bindcount++;
		else
			input->bindcount++;
	}	
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUTPIPE);
	return OK;	
}

int zpl_meida_video_input_pipe_bindcount_get(zpl_meida_video_input_pipe_t *input)
{
	int bindcount = 0;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUTPIPE);
	if(input)
	{
		bindcount = input->bindcount;
	}	
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUTPIPE);
	return bindcount;	
}


#ifdef ZPL_SHELL_MODULE
int zpl_meida_video_input_pipe_show(void *pvoid)
{
	NODE *node;
	zpl_meida_video_input_pipe_t *t;	
	struct vty *vty = (struct vty *)pvoid;
    LIST *_lst = NULL;
    os_mutex_t *_mutex = NULL;
    zpl_media_global_get(ZPL_MEDIA_GLOAL_VIDEO_INPUTPIPE, &_lst, &_mutex);
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
		t = (zpl_meida_video_input_pipe_t *) node;
		if (node)
		{
			vty_out(vty, "%-4d 0x%-4x %-6s  %-8d %-8d %dx%d%s", t->input_pipe, t->res_flag, t->online ? "ONLINE":"OFFLINE", 
					t->bindcount, t->input_dev, 
					t->input_size.width, t->input_size.height, VTY_NEWLINE);
		}
	}
	if (_mutex)
		os_mutex_unlock(_mutex);
	return OK;
}
#endif