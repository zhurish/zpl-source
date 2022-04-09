/*
 * zpl_video_input.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"

static LIST *video_input_list = NULL;
static os_mutex_t *video_input_mutex = NULL;


int zpl_video_input_init()
{
	if(video_input_list == NULL)
	{
		video_input_list = os_malloc(sizeof(LIST));
		if(video_input_list)
		{
			lstInit(video_input_list);
		}
		else
			return ERROR;
	}
	if(video_input_mutex == NULL)
	{
		video_input_mutex = os_mutex_init();
	}
	zpl_video_input_pipe_init();
	return ERROR;
}

int zpl_video_input_exit()
{
	if(video_input_mutex)
	{
		if(os_mutex_exit(video_input_mutex)==OK)
			video_input_mutex = NULL;
	}
	if(video_input_list)
	{
		lstFree(video_input_list);
		video_input_list = NULL;
	}
	zpl_video_input_pipe_exit();
	return OK;
}


int zpl_video_input_channel_destroy(zpl_int32 input_channel)
{
	NODE node;
	zpl_video_input_channel_t *t = NULL;
	if (video_input_mutex)
		os_mutex_lock(video_input_mutex, OS_WAIT_FOREVER);

	for(t = (zpl_video_input_channel_t *)lstFirst(video_input_list); 
		t != NULL; t = (zpl_video_input_channel_t *)lstNext(&node))
	{
		node = t->node;
		if(t && t->input_chn == input_channel)
		{
			lstDelete (video_input_list, (NODE *)t);
			os_free(t);	
			break;
		}
	}
	if(video_input_mutex)
		os_mutex_unlock(video_input_mutex);
	return OK;
}



int zpl_video_input_channel_create(zpl_int32 input_channel)
{
	zpl_video_input_channel_t *t = os_malloc(sizeof(zpl_video_input_channel_t));
	zpl_video_assert(t);
	if(t)
	{
		memset(t, 0, sizeof(zpl_video_input_channel_t));
    	t->input_chn = input_channel;                     //底层通道号
		if(video_input_mutex)
			os_mutex_lock(video_input_mutex, OS_WAIT_FOREVER);
		if(video_input_list)
			lstAdd(video_input_list, (NODE *)t);
		if(video_input_mutex)
			os_mutex_unlock(video_input_mutex);
		return OK;
	}
	return ERROR;
}



zpl_video_input_channel_t * zpl_video_input_channel_lookup(zpl_int32 input_channel)
{
	NODE node;
	zpl_video_input_channel_t *t = NULL;
	if (video_input_mutex)
		os_mutex_lock(video_input_mutex, OS_WAIT_FOREVER);

	for(t = (zpl_video_input_channel_t *)lstFirst(video_input_list); 
		t != NULL; t = (zpl_video_input_channel_t *)lstNext(&node))
	{
		node = t->node;
		if(t && t->input_chn == input_channel)
		{
			break;
		}
	}
	if(video_input_mutex)
		os_mutex_unlock(video_input_mutex);
	return t;
}



static int zpl_video_input_channel_read(struct thread *t)
{
	zpl_video_input_channel_t *input = THREAD_ARG(t);
	
	if (video_input_mutex)
		os_mutex_lock(video_input_mutex, OS_WAIT_FOREVER);
    if(input)
    {
        input->t_read = NULL;
        zpl_vidhal_input_channel_frame_recvfrom(input);

        if(!ipstack_invalid(input->chnfd))
            input->t_read = thread_add_read(t->master, zpl_video_input_channel_read, input, input->chnfd);
    }
	if(video_input_mutex)
		os_mutex_unlock(video_input_mutex);
	return OK;
}

int zpl_video_input_channel_read_start(zpl_void *master, zpl_video_input_channel_t *input)
{
    zpl_video_assert(master);
    zpl_video_assert(input);
	if(ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, DETAIL))
        zpl_media_debugmsg_debug(" start video input channel (%d/%d) read thread\n", 
			input->inputpipe?input->inputpipe->input_pipe:-1, input->input_chn);
	if (video_input_mutex)
		os_mutex_lock(video_input_mutex, OS_WAIT_FOREVER);
	zpl_vidhal_input_channel_update_fd(input);
    if(master && input && !ipstack_invalid(input->chnfd))
	{
		input->t_master = master;
		//input->master = master;
        input->t_read = thread_add_read(master, zpl_video_input_channel_read, input, input->chnfd);
	}
	if(video_input_mutex)
		os_mutex_unlock(video_input_mutex);
	return OK;    
}

int zpl_video_input_channel_read_stop(zpl_video_input_channel_t *input)
{
    zpl_video_assert(input);  
	if(ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, DETAIL))
        zpl_media_debugmsg_debug(" stop video input channel (%d/%d) read thread\n", 
			input->inputpipe?input->inputpipe->input_pipe:-1, input->input_chn);
	if (video_input_mutex)
		os_mutex_lock(video_input_mutex, OS_WAIT_FOREVER);
    if(input && input->t_read)
    {
        thread_cancel(input->t_read);
        input->t_read = NULL;
    }
	if(video_input_mutex)
		os_mutex_unlock(video_input_mutex);
    return OK; 
}




int zpl_video_input_channel_sendto(zpl_video_input_channel_t *input,  void *p, zpl_int timeout)
{
    int ret = -1;
    zpl_video_assert(input);

    return ret;
}





int zpl_video_input_channel_active(zpl_video_input_channel_t *input)
{
    int ret = -1;
    zpl_video_assert(input);
	if (video_input_mutex)
		os_mutex_lock(video_input_mutex, OS_WAIT_FOREVER);
	zpl_media_debugmsg_debug("input_channel(%d)  Flags (0x%x)", input->input_chn, input->res_flag);
	if(VIDHAL_RES_FLAG_CHECK(input->res_flag, CREATE) && !VIDHAL_RES_FLAG_CHECK(input->res_flag, INIT))
	{
    	ret = zpl_vidhal_input_channel_create(input);
		if(ret == OK)	
			VIDHAL_RES_FLAG_SET(input->res_flag, INIT);
		if(ZPL_MEDIA_DEBUG(INPUT, EVENT))
		{
			zpl_media_debugmsg_debug("active input channel(%d) ret=%d", input->input_chn, ret);
		} 
	}
	else
	{
		ret = OK;
		if(ZPL_MEDIA_DEBUG(INPUT, EVENT))
		{
			zpl_media_debugmsg_debug("input channel(%d) is already active", input->input_chn);
		} 
	}
	if(video_input_mutex)
		os_mutex_unlock(video_input_mutex);
    return ret;
}

int zpl_video_input_channel_start(zpl_video_input_channel_t *input)
{
    int ret = -1;
    zpl_video_assert(input);
	if (video_input_mutex)
		os_mutex_lock(video_input_mutex, OS_WAIT_FOREVER);
	if(!VIDHAL_RES_FLAG_CHECK(input->res_flag, START))	
	{
    	ret = zpl_vidhal_input_channel_start(input);
		if(ret == OK)	
		{
			VIDHAL_RES_FLAG_SET(input->res_flag, START);
			input->online = zpl_true;
		}
			if(ZPL_MEDIA_DEBUG(INPUT, EVENT))
		{
			zpl_media_debugmsg_debug("start input channel(%d) ret=%d", input->input_chn, ret);
		} 
	}
	else
	{
		ret = OK;
		if(ZPL_MEDIA_DEBUG(INPUT, EVENT))
		{
			zpl_media_debugmsg_debug("input channel(%d) is already start", input->input_chn);
		} 
	}
	if(video_input_mutex)
		os_mutex_unlock(video_input_mutex);
    return ret;
}

int zpl_video_input_channel_stop(zpl_video_input_channel_t *input)
{
    int ret = -1;
    zpl_video_assert(input);
	if (video_input_mutex)
		os_mutex_lock(video_input_mutex, OS_WAIT_FOREVER);
	if(input->bindcount <= 1 && VIDHAL_RES_FLAG_CHECK(input->res_flag, START))	
	{
    	ret = zpl_vidhal_input_channel_stop(input);
		if(ret == OK)	
		{
			VIDHAL_RES_FLAG_UNSET(input->res_flag, START);
			input->online = zpl_false;
		}
		if(ZPL_MEDIA_DEBUG(INPUT, EVENT))
		{
			zpl_media_debugmsg_debug("stop input channel(%d) ret=%d", input->input_chn, ret);
		} 
	}
	else
	{
		ret = OK;
		if(ZPL_MEDIA_DEBUG(INPUT, EVENT))
		{
			zpl_media_debugmsg_debug("input channel(%d) is already stop", input->input_chn);
		} 
	}
    
	if(video_input_mutex)
		os_mutex_unlock(video_input_mutex);
    return ret;
}

int zpl_video_input_channel_inactive(zpl_video_input_channel_t *input)
{
    int ret = -1;
    zpl_video_assert(input);
	if (video_input_mutex)
		os_mutex_lock(video_input_mutex, OS_WAIT_FOREVER);
	if(VIDHAL_RES_FLAG_CHECK(input->res_flag, CREATE) && VIDHAL_RES_FLAG_CHECK(input->res_flag, INIT))
	{	
    	ret = zpl_vidhal_input_channel_destroy(input);
		if(ret == OK)
			VIDHAL_RES_FLAG_UNSET(input->res_flag, INIT);
		if(ZPL_MEDIA_DEBUG(INPUT, EVENT))
		{
			zpl_media_debugmsg_debug("inactive input channel(%d) ret=%d", input->input_chn, ret);
		} 
	}
	else
	{
		ret = OK;
		if(ZPL_MEDIA_DEBUG(INPUT, EVENT))
		{
			zpl_media_debugmsg_debug("input channel(%d) is already inactive", input->input_chn);
		} 
	}

	if(video_input_mutex)
		os_mutex_unlock(video_input_mutex);
    return ret;
}

int zpl_video_input_channel_online_set(zpl_video_input_channel_t *input, zpl_bool online)
{
    int ret = -1;
    zpl_video_assert(input);
	if (video_input_mutex)
		os_mutex_lock(video_input_mutex, OS_WAIT_FOREVER);
    ret = OK;
	input->online = online;
	if(video_input_mutex)
		os_mutex_unlock(video_input_mutex);
    return ret;
}
int zpl_video_input_channel_online_get(zpl_video_input_channel_t *input, zpl_bool *online)
{
    int ret = -1;
    zpl_video_assert(input);
	if (video_input_mutex)
		os_mutex_lock(video_input_mutex, OS_WAIT_FOREVER);
    ret = OK;
	if(online)
		*online = input->online;
	if(video_input_mutex)
		os_mutex_unlock(video_input_mutex);
    return ret;
}

int zpl_video_input_channel_bindcount_set(zpl_video_input_channel_t *input, zpl_bool add)
{
	if (video_input_mutex)
		os_mutex_lock(video_input_mutex, OS_WAIT_FOREVER);
	if(input)
	{
		if(add)
			input->bindcount++;
		else
			input->bindcount++;
	}	
	if(video_input_mutex)
		os_mutex_unlock(video_input_mutex);
	return OK;	
}

int zpl_video_input_channel_bindcount_get(zpl_video_input_channel_t *input)
{
	int bindcount = 0;
	if (video_input_mutex)
		os_mutex_lock(video_input_mutex, OS_WAIT_FOREVER);
	if(input)
	{
		bindcount = input->bindcount;
	}	
	if(video_input_mutex)
		os_mutex_unlock(video_input_mutex);
	return bindcount;	
}

#ifdef ZPL_SHELL_MODULE
int zpl_video_input_show(void *pvoid)
{
	NODE *node;
	zpl_video_input_channel_t *t;	
	struct vty *vty = (struct vty *)pvoid;

	if (video_input_mutex)
		os_mutex_lock(video_input_mutex, OS_WAIT_FOREVER);

	if(lstCount(video_input_list))
	{
		vty_out(vty, "%-4s %-4s %-6s %-8s %-8s %-16s %s", "----", "----", "------", "--------", "--------", "----------------", VTY_NEWLINE);
		vty_out(vty, "%-4s %-4s %-6s %-8s %-8s %-16s %s", "ID", "FLAG", "ONLINE", "BINDCNT", "BINDID",  "SIZE", VTY_NEWLINE);
		vty_out(vty, "%-4s %-4s %-6s %-8s %-8s %-16s %s", "----", "----", "------", "--------", "--------", "----------------", VTY_NEWLINE);
	}
	for (node = lstFirst(video_input_list); node != NULL; node = lstNext(node))
	{
		t = (zpl_video_input_channel_t *) node;
		if (node)
		{
			vty_out(vty, "%-4d 0x%-4x %-6s  %-8d %-8d %dx%d%s", t->input_chn, t->res_flag, t->online ? "ONLINE":"OFFLINE", 
					t->bindcount, t->inputpipe?((zpl_video_input_pipe_t*)t->inputpipe)->input_pipe:-1, 
					t->input_size.width, t->input_size.height, VTY_NEWLINE);
		}
	}
	if (video_input_mutex)
		os_mutex_unlock(video_input_mutex);
	return OK;
}
#endif




int zpl_video_input_channel_crop(zpl_video_input_channel_t *input, zpl_bool out, zpl_video_size_t cropsize)
{
	int ret = -1;
	if (video_input_mutex)
		os_mutex_lock(video_input_mutex, OS_WAIT_FOREVER);
	if(input)
	{
		ret = zpl_vidhal_input_crop(input->inputpipe->input_pipe, input->input_chn, out, cropsize);
	}	
	if(video_input_mutex)
		os_mutex_unlock(video_input_mutex);
	return ret;		
}

int zpl_video_input_channel_mirror_flip(zpl_video_input_channel_t *input, zpl_bool mirror, zpl_bool flip)
{
	int ret = -1;
	if (video_input_mutex)
		os_mutex_lock(video_input_mutex, OS_WAIT_FOREVER);
	if(input)
	{
		ret = zpl_vidhal_input_mirror_flip(input->inputpipe->input_pipe, input->input_chn, mirror, flip);
	}	
	if(video_input_mutex)
		os_mutex_unlock(video_input_mutex);
	return ret;		
}

int zpl_video_input_channel_ldc(zpl_video_input_channel_t *input, zpl_video_size_t cropsize)//畸形矫正
{
	int ret = -1;
	if (video_input_mutex)
		os_mutex_lock(video_input_mutex, OS_WAIT_FOREVER);
	if(input)
	{
		ret = zpl_vidhal_input_ldc(input->inputpipe->input_pipe, input->input_chn, cropsize);
	}	
	if(video_input_mutex)
		os_mutex_unlock(video_input_mutex);
	return ret;		
}

int zpl_video_input_channel_fish_eye(zpl_video_input_channel_t *input, void * LMF)//鱼眼校正
{
	int ret = -1;
	if (video_input_mutex)
		os_mutex_lock(video_input_mutex, OS_WAIT_FOREVER);
	if(input)
	{
		ret = zpl_vidhal_input_fish_eye(input->inputpipe->input_pipe, input->input_chn,LMF);
	}	
	if(video_input_mutex)
		os_mutex_unlock(video_input_mutex);
	return ret;		
}

int zpl_video_input_channel_rotation(zpl_video_input_channel_t *input, zpl_uint32 rotation)
{
	int ret = -1;
	if (video_input_mutex)
		os_mutex_lock(video_input_mutex, OS_WAIT_FOREVER);
	if(input)
	{
		ret = zpl_vidhal_input_rotation(input->inputpipe->input_pipe, input->input_chn, rotation);
	}	
	if(video_input_mutex)
		os_mutex_unlock(video_input_mutex);
	return ret;		
}

int zpl_video_input_channel_rotation_angle(zpl_video_input_channel_t *input, void *p)
{
	int ret = -1;
	if (video_input_mutex)
		os_mutex_lock(video_input_mutex, OS_WAIT_FOREVER);
	if(input)
	{
		ret = zpl_vidhal_input_rotation_angle(input->inputpipe->input_pipe, input->input_chn, p);
	}	
	if(video_input_mutex)
		os_mutex_unlock(video_input_mutex);
	return ret;		
}

int zpl_video_input_channel_spread(zpl_video_input_channel_t *input, void *p)
{
	int ret = -1;
	if (video_input_mutex)
		os_mutex_lock(video_input_mutex, OS_WAIT_FOREVER);
	if(input)
	{
		ret = zpl_vidhal_input_spread(input->inputpipe->input_pipe, input->input_chn, p);
	}	
	if(video_input_mutex)
		os_mutex_unlock(video_input_mutex);
	return ret;	
}