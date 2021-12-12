/*
 * zpl_video_input_pipe.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "os_include.h"
#include "zpl_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"

static LIST *_input_pipe_list = NULL;
static os_mutex_t *_input_pipe_mutex = NULL;


int zpl_video_input_pipe_init()
{
	if(_input_pipe_list == NULL)
	{
		_input_pipe_list = os_malloc(sizeof(LIST));
		if(_input_pipe_list)
		{
			lstInit(_input_pipe_list);
		}
		else
			return ERROR;
	}
	if(_input_pipe_mutex == NULL)
	{
		_input_pipe_mutex = os_mutex_init();
	}
	return ERROR;
}

int zpl_video_input_pipe_exit()
{
	if(_input_pipe_mutex)
	{
		if(os_mutex_exit(_input_pipe_mutex)==OK)
			_input_pipe_mutex = NULL;
	}
	if(_input_pipe_list)
	{
		lstFree(_input_pipe_list);
		_input_pipe_list = NULL;
	}

	return OK;
}

int zpl_video_input_pipe_destroy(zpl_int32 dev, zpl_int32 vipipe)
{
	NODE node;
	zpl_video_input_pipe_t *t = NULL;
	if (_input_pipe_mutex)
		os_mutex_lock(_input_pipe_mutex, OS_WAIT_FOREVER);

	for(t = (zpl_video_input_pipe_t *)lstFirst(_input_pipe_list); 
		t != NULL; t = (zpl_video_input_pipe_t *)lstNext(&node))
	{
		node = t->node;
		if(t && t->input_pipe == vipipe && t->input_dev == dev)
		{
			lstDelete (_input_pipe_list, (NODE *)t);
			os_free(t);	
			break;
		}
	}
	if(_input_pipe_mutex)
		os_mutex_unlock(_input_pipe_mutex);
	return OK;
}

int zpl_video_input_pipe_channel_count(zpl_int32 vipipe)
{
	int count = 0;
	NODE node;
	zpl_video_input_pipe_t *t = NULL;
	if (_input_pipe_mutex)
		os_mutex_lock(_input_pipe_mutex, OS_WAIT_FOREVER);

	for(t = (zpl_video_input_pipe_t *)lstFirst(_input_pipe_list); 
		t != NULL; t = (zpl_video_input_pipe_t *)lstNext(&node))
	{
		node = t->node;
		if(t && t->input_pipe == vipipe && t->input_dev >= 0)
		{
			count++;	
		}
	}
	if(_input_pipe_mutex)
		os_mutex_unlock(_input_pipe_mutex);
	return count;
}

int zpl_video_input_pipe_create(zpl_int32 dev, zpl_int32 vipipe)
{
	zpl_video_input_pipe_t *t = os_malloc(sizeof(zpl_video_input_pipe_t));
	zpl_video_assert(t);
	if(t)
	{
		memset(t, 0, sizeof(zpl_video_input_pipe_t));
    	t->input_dev = dev;                     //底层设备编号
    	t->input_pipe = vipipe;                    //底层硬件pipe
		if(_input_pipe_mutex)
			os_mutex_lock(_input_pipe_mutex, OS_WAIT_FOREVER);
		if(_input_pipe_list)
			lstAdd(_input_pipe_list, (NODE *)t);
		if(_input_pipe_mutex)
			os_mutex_unlock(_input_pipe_mutex);
		return OK;
	}
	return ERROR;
}



zpl_video_input_pipe_t * zpl_video_input_pipe_lookup(zpl_int32 dev, zpl_int32 vipipe)
{
	NODE node;
	zpl_video_input_pipe_t *t = NULL;
	if (_input_pipe_mutex)
		os_mutex_lock(_input_pipe_mutex, OS_WAIT_FOREVER);

	for(t = (zpl_video_input_pipe_t *)lstFirst(_input_pipe_list); 
		t != NULL; t = (zpl_video_input_pipe_t *)lstNext(&node))
	{
		node = t->node;
		if(t && t->input_pipe == vipipe && t->input_dev == dev)
		{
			break;
		}
	}
	if(_input_pipe_mutex)
		os_mutex_unlock(_input_pipe_mutex);
	return t;
}




static int zpl_video_input_pipe_read(struct thread *t)
{
	zpl_video_input_pipe_t *input = THREAD_ARG(t);

	if (_input_pipe_mutex)
		os_mutex_lock(_input_pipe_mutex, OS_WAIT_FOREVER);
    if(input)
    {
        input->t_read = NULL;
        zpl_vidhal_input_pipe_frame_recvfrom(input);
        input->t_read = thread_add_read(t->master, zpl_video_input_pipe_read, input, input->pipefd);
    }
	if(_input_pipe_mutex)
		os_mutex_unlock(_input_pipe_mutex);
	return OK;
}

int zpl_video_input_pipe_read_start(zpl_void *master, zpl_video_input_pipe_t *input)
{
    zpl_video_assert(master);
    zpl_video_assert(input);
	if(ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT) && ZPL_MEDIA_DEBUG(INPUTPIPE, DETAIL))
		zpl_media_debugmsg_debug(" start video input pipe channel %d read thread\n", input->input_pipe);
	if (_input_pipe_mutex)
		os_mutex_lock(_input_pipe_mutex, OS_WAIT_FOREVER);
	zpl_vidhal_input_pipe_update_fd(input);
	input->t_master = master;
    if(master && input && input->pipefd)
        input->t_read = thread_add_read(master, zpl_video_input_pipe_read, input, input->pipefd);
	if(_input_pipe_mutex)
		os_mutex_unlock(_input_pipe_mutex);
	return OK;    
}

int zpl_video_input_pipe_read_stop(zpl_video_input_pipe_t *input)
{
    zpl_video_assert(input);  
	if(ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT) && ZPL_MEDIA_DEBUG(INPUTPIPE, DETAIL))
		zpl_media_debugmsg_debug(" stop video input pipe channel %d read thread\n", input->input_pipe);
	if (_input_pipe_mutex)
		os_mutex_lock(_input_pipe_mutex, OS_WAIT_FOREVER);
    if(input && input->t_read)
    {
        thread_cancel(input->t_read);
        input->t_read = NULL;
    }
	if(_input_pipe_mutex)
		os_mutex_unlock(_input_pipe_mutex);
    return OK; 
}




int zpl_video_input_pipe_sendto(zpl_video_input_pipe_t *input,  void *p, zpl_int timeout)
{
    int ret = -1;
    zpl_video_assert(input);

    return ret;
}





int zpl_video_input_pipe_active(zpl_video_input_pipe_t *input)
{
    int ret = -1;
    zpl_video_assert(input);
	if (_input_pipe_mutex)
		os_mutex_lock(_input_pipe_mutex, OS_WAIT_FOREVER);
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
	if(_input_pipe_mutex)
		os_mutex_unlock(_input_pipe_mutex);
    return ret;
}

int zpl_video_input_pipe_start(zpl_video_input_pipe_t *input)
{
    int ret = -1;
    zpl_video_assert(input);
	if (_input_pipe_mutex)
		os_mutex_lock(_input_pipe_mutex, OS_WAIT_FOREVER);
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
	if(_input_pipe_mutex)
		os_mutex_unlock(_input_pipe_mutex);
    return ret;
}

int zpl_video_input_pipe_stop(zpl_video_input_pipe_t *input)
{
    int ret = -1;
    zpl_video_assert(input);
	if (_input_pipe_mutex)
		os_mutex_lock(_input_pipe_mutex, OS_WAIT_FOREVER);
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

	if(_input_pipe_mutex)
		os_mutex_unlock(_input_pipe_mutex);
    return ret;
}

int zpl_video_input_pipe_inactive(zpl_video_input_pipe_t *input)
{
    int ret = -1;
    zpl_video_assert(input);
	if (_input_pipe_mutex)
		os_mutex_lock(_input_pipe_mutex, OS_WAIT_FOREVER);
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
	if(_input_pipe_mutex)
		os_mutex_unlock(_input_pipe_mutex);
    return ret;
}

int zpl_video_input_pipe_online_set(zpl_video_input_pipe_t *input, zpl_bool online)
{
    int ret = -1;
    zpl_video_assert(input);
	if (_input_pipe_mutex)
		os_mutex_lock(_input_pipe_mutex, OS_WAIT_FOREVER);
    ret = OK;
	input->online = online;
	if(_input_pipe_mutex)
		os_mutex_unlock(_input_pipe_mutex);
    return ret;
}
int zpl_video_input_pipe_online_get(zpl_video_input_pipe_t *input, zpl_bool *online)
{
    int ret = -1;
    zpl_video_assert(input);
	if (_input_pipe_mutex)
		os_mutex_lock(_input_pipe_mutex, OS_WAIT_FOREVER);
    ret = OK;
	if(online)
		*online = input->online;
	if(_input_pipe_mutex)
		os_mutex_unlock(_input_pipe_mutex);
    return ret;
}

int zpl_video_input_pipe_bindcount_set(zpl_video_input_pipe_t *input, zpl_bool add)
{
	if (_input_pipe_mutex)
		os_mutex_lock(_input_pipe_mutex, OS_WAIT_FOREVER);
	if(input)
	{
		if(add)
			input->bindcount++;
		else
			input->bindcount++;
	}	
	if(_input_pipe_mutex)
		os_mutex_unlock(_input_pipe_mutex);
	return OK;	
}

int zpl_video_input_pipe_bindcount_get(zpl_video_input_pipe_t *input)
{
	int bindcount = 0;
	if (_input_pipe_mutex)
		os_mutex_lock(_input_pipe_mutex, OS_WAIT_FOREVER);
	if(input)
	{
		bindcount = input->bindcount;
	}	
	if(_input_pipe_mutex)
		os_mutex_unlock(_input_pipe_mutex);
	return bindcount;	
}


#ifdef ZPL_SHELL_MODULE
int zpl_video_input_pipe_show(void *pvoid)
{
	NODE *node;
	zpl_video_input_pipe_t *t;	
	struct vty *vty = (struct vty *)pvoid;

	if (_input_pipe_mutex)
		os_mutex_lock(_input_pipe_mutex, OS_WAIT_FOREVER);

	if(lstCount(_input_pipe_list))
	{
		vty_out(vty, "%-4s %-4s %-6s %-8s %-8s %-16s %s", "----", "----", "------", "--------", "--------", "----------------", VTY_NEWLINE);
		vty_out(vty, "%-4s %-4s %-6s %-8s %-8s %-16s %s", "ID", "FLAG", "ONLINE", "BINDCNT", "BINDID",  "SIZE", VTY_NEWLINE);
		vty_out(vty, "%-4s %-4s %-6s %-8s %-8s %-16s %s", "----", "----", "------", "--------", "--------", "----------------", VTY_NEWLINE);
	}
	for (node = lstFirst(_input_pipe_list); node != NULL; node = lstNext(node))
	{
		t = (zpl_video_input_pipe_t *) node;
		if (node)
		{
			vty_out(vty, "%-4d 0x%-4x %-6s  %-8d %-8d %dx%d%s", t->input_pipe, t->res_flag, t->online ? "ONLINE":"OFFLINE", 
					t->bindcount, t->input_dev, 
					t->input_size.width, t->input_size.height, VTY_NEWLINE);
		}
	}
	if (_input_pipe_mutex)
		os_mutex_unlock(_input_pipe_mutex);
	return OK;
}
#endif