/*
 * zpl_video_vpss.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "os_include.h"
#include "zpl_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"




static LIST *_vpss_channel_list = NULL;
static os_mutex_t *_vpss_channel_mutex = NULL;

static int zpl_video_vpss_channel_init()
{
	if(_vpss_channel_list == NULL)
	{
		_vpss_channel_list = os_malloc(sizeof(LIST));
		if(_vpss_channel_list)
		{
			lstInit(_vpss_channel_list);
		}
		else
			return ERROR;
	}
	if(_vpss_channel_mutex == NULL)
	{
		_vpss_channel_mutex = os_mutex_init();
	}
	return ERROR;
}

static int zpl_video_vpss_channel_exit()
{	
	if(_vpss_channel_mutex)
	{
		if(os_mutex_exit(_vpss_channel_mutex)==OK)
			_vpss_channel_mutex = NULL;
	}
	if(_vpss_channel_list)
	{
		lstFree(_vpss_channel_list);
		_vpss_channel_list = NULL;
	}

	return OK;
}

int zpl_video_vpss_init()
{
	zpl_video_vpss_channel_init();
	zpl_video_vpssgrp_init();
	return OK;
}
int zpl_video_vpss_exit()
{
	zpl_video_vpss_channel_exit();
	zpl_video_vpssgrp_exit();
	return OK;
}

int zpl_video_vpss_channel_count(zpl_int32 vpss_channel)
{
	int count = 0;
	NODE node;
	zpl_video_vpss_channel_t *t = NULL;
	if (_vpss_channel_mutex)
		os_mutex_lock(_vpss_channel_mutex, OS_WAIT_FOREVER);

	for(t = (zpl_video_vpss_channel_t *)lstFirst(_vpss_channel_list); 
		t != NULL; t = (zpl_video_vpss_channel_t *)lstNext(&node))
	{
		node = t->node;
		if(t && t->vpss_channel == vpss_channel)
		{
			count++;
		}
	}
	if(_vpss_channel_mutex)
		os_mutex_unlock(_vpss_channel_mutex);
	return count;
}

int zpl_video_vpss_channel_destroy(zpl_int32 vpss_channel)
{
	NODE node;
	zpl_video_vpss_channel_t *t = NULL;
	if (_vpss_channel_mutex)
		os_mutex_lock(_vpss_channel_mutex, OS_WAIT_FOREVER);

	for(t = (zpl_video_vpss_channel_t *)lstFirst(_vpss_channel_list); 
		t != NULL; t = (zpl_video_vpss_channel_t *)lstNext(&node))
	{
		node = t->node;
		if(t && t->vpss_channel == vpss_channel)
		{
			lstDelete (_vpss_channel_list, (NODE *)t);
			os_free(t);	
			break;
		}
	}
	if(_vpss_channel_mutex)
		os_mutex_unlock(_vpss_channel_mutex);
	return OK;
}

int zpl_video_vpss_channel_create(zpl_int32 vpss_channel)
{
	zpl_video_vpss_channel_t *t = os_malloc(sizeof(zpl_video_vpss_channel_t));
	zpl_video_assert(t);
	if(t)
	{
		memset(t, 0, sizeof(zpl_video_vpss_channel_t));
		t->vpss_channel = vpss_channel;

		if(_vpss_channel_mutex)
			os_mutex_lock(_vpss_channel_mutex, OS_WAIT_FOREVER);
		if(_vpss_channel_list)
			lstAdd(_vpss_channel_list, (NODE *)t);
		if(_vpss_channel_mutex)
			os_mutex_unlock(_vpss_channel_mutex);
		return OK;
	}
	return ERROR;
}



zpl_video_vpss_channel_t * zpl_video_vpss_channel_lookup(zpl_int32 vpss_channel)
{
	NODE node;
	zpl_video_vpss_channel_t *t = NULL;
	if (_vpss_channel_mutex)
		os_mutex_lock(_vpss_channel_mutex, OS_WAIT_FOREVER);

	for(t = (zpl_video_vpss_channel_t *)lstFirst(_vpss_channel_list); 
		t != NULL; t = (zpl_video_vpss_channel_t *)lstNext(&node))
	{
		node = t->node;
		if(t && t->vpss_channel == vpss_channel)
		{
			break;
		}
	}
	if(_vpss_channel_mutex)
		os_mutex_unlock(_vpss_channel_mutex);
	return t;
}
#ifndef ZPL_VIDEO_VPSSGRP_ENABLE
int zpl_video_vpss_channel_input_set(zpl_int32 vpss_channel, void *video_input)
{
	zpl_video_vpss_channel_t * t = zpl_video_vpss_channel_lookup(vpss_channel);
	if (_vpss_channel_mutex)
		os_mutex_lock(_vpss_channel_mutex, OS_WAIT_FOREVER);
	if(t)
	{
		t->video_input = video_input;
	}	
	if(_vpss_channel_mutex)
		os_mutex_unlock(_vpss_channel_mutex);
	return OK;	
}
#else
int zpl_video_vpss_channel_vpssgrp_set(zpl_int32 vpss_channel, void *vpssgrp)
{
	zpl_video_vpss_channel_t * t = zpl_video_vpss_channel_lookup(vpss_channel);
	if (_vpss_channel_mutex)
		os_mutex_lock(_vpss_channel_mutex, OS_WAIT_FOREVER);
	if(t)
	{
		t->vpssgrp = vpssgrp;
	}	
	if(_vpss_channel_mutex)
		os_mutex_unlock(_vpss_channel_mutex);
	return OK;
}
#endif
int zpl_video_vpss_channel_bindcount_set(zpl_video_vpss_channel_t *vpss, zpl_bool add)
{
	if (_vpss_channel_mutex)
		os_mutex_lock(_vpss_channel_mutex, OS_WAIT_FOREVER);
	if(vpss)
	{
		if(add)
			vpss->bindcount++;
		else
			vpss->bindcount++;
	}	
	if(_vpss_channel_mutex)
		os_mutex_unlock(_vpss_channel_mutex);
	return OK;	
}

int zpl_video_vpss_channel_bindcount_get(zpl_video_vpss_channel_t *vpss)
{
	int bindcount = 0;
	if (_vpss_channel_mutex)
		os_mutex_lock(_vpss_channel_mutex, OS_WAIT_FOREVER);
	if(vpss)
	{
		bindcount = vpss->bindcount;
	}	
	if(_vpss_channel_mutex)
		os_mutex_unlock(_vpss_channel_mutex);
	return bindcount;	
}


static int zpl_video_vpss_channel_read(struct thread *t)
{
	zpl_video_vpss_channel_t *vpss = THREAD_ARG(t);

	if (_vpss_channel_mutex)
		os_mutex_lock(_vpss_channel_mutex, OS_WAIT_FOREVER);
    if(vpss)
    {
        vpss->t_read = NULL;

        zpl_vidhal_vpss_channel_frame_recvfrom(vpss);

        if(vpss->vpssfd)
           vpss->t_read = thread_add_read(t->master, zpl_video_vpss_channel_read, vpss, vpss->vpssfd);
    }
	if(_vpss_channel_mutex)
		os_mutex_unlock(_vpss_channel_mutex);
	return OK;
}

int zpl_video_vpss_channel_read_start(zpl_void *master, zpl_video_vpss_channel_t *vpss)
{
    zpl_video_assert(master);
    zpl_video_assert(vpss);
	if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, DETAIL))
		zpl_media_debugmsg_debug(" start video VPSS channel (%d/%d) read thread\n", 
			vpss->vpssgrp?vpss->vpssgrp->vpss_group:-1, vpss->vpss_channel);
	if (_vpss_channel_mutex)
		os_mutex_lock(_vpss_channel_mutex, OS_WAIT_FOREVER);
	zpl_vidhal_vpss_channel_update_fd(vpss);	
    if(master && vpss && vpss->vpssfd)
	{
		vpss->t_master = master;
        vpss->t_read = thread_add_read(master, zpl_video_vpss_channel_read, vpss, vpss->vpssfd);
	}
	if(_vpss_channel_mutex)
		os_mutex_unlock(_vpss_channel_mutex);
    return OK;    
}

int zpl_video_vpss_channel_read_stop(zpl_video_vpss_channel_t *vpss)
{
    zpl_video_assert(vpss);  
	if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, DETAIL))
		zpl_media_debugmsg_debug(" stop video VPSS channel (%d/%d) read thread\n", 
			vpss->vpssgrp?vpss->vpssgrp->vpss_group:-1, vpss->vpss_channel);
	if (_vpss_channel_mutex)
		os_mutex_lock(_vpss_channel_mutex, OS_WAIT_FOREVER);
    if(vpss && vpss->t_read)
    {
        thread_cancel(vpss->t_read);
        vpss->t_read = NULL;
    }
	if(_vpss_channel_mutex)
		os_mutex_unlock(_vpss_channel_mutex);
    return OK; 
}





int zpl_video_vpss_channel_active(zpl_video_vpss_channel_t *vpss)
{
    int ret = -1;
    zpl_video_assert(vpss);
	if (_vpss_channel_mutex)
		os_mutex_lock(_vpss_channel_mutex, OS_WAIT_FOREVER);
	zpl_media_debugmsg_debug("vpss channel (%d)  Flags (0x%x)", vpss->vpss_channel, vpss->res_flag);	
	if(VIDHAL_RES_FLAG_CHECK(vpss->res_flag, CREATE) && !VIDHAL_RES_FLAG_CHECK(vpss->res_flag, INIT))	
	{
    	ret = zpl_vidhal_vpss_channel_create(vpss);
		if(ret == OK)	
			VIDHAL_RES_FLAG_SET(vpss->res_flag, INIT);
		if(ZPL_MEDIA_DEBUG(VPSS, EVENT))
		{
			zpl_media_debugmsg_debug("active vpss channel(%d) ret=%d", vpss->vpss_channel, ret);
		} 
	}
	else
		ret = OK;

	if(_vpss_channel_mutex)
		os_mutex_unlock(_vpss_channel_mutex);
    return ret;
}

int zpl_video_vpss_channel_start(zpl_video_vpss_channel_t *vpss)
{
    int ret = -1;
    zpl_video_assert(vpss);
	if (_vpss_channel_mutex)
		os_mutex_lock(_vpss_channel_mutex, OS_WAIT_FOREVER);
	if(!VIDHAL_RES_FLAG_CHECK(vpss->res_flag, START))	
	{
    	ret = zpl_vidhal_vpss_channel_start(vpss);
		if(ret == OK)	
		{
			VIDHAL_RES_FLAG_SET(vpss->res_flag, START);
			vpss->online = zpl_true;
		}
		if(ZPL_MEDIA_DEBUG(VPSS, EVENT))
		{
			zpl_media_debugmsg_debug("start vpss channel(%d) ret=%d", vpss->vpss_channel, ret);
		} 
	}
	else
		ret = OK;
	if(_vpss_channel_mutex)
		os_mutex_unlock(_vpss_channel_mutex);
    return ret;
}

int zpl_video_vpss_channel_stop(zpl_video_vpss_channel_t *vpss)
{
    int ret = -1;
    zpl_video_assert(vpss);
	if (_vpss_channel_mutex)
		os_mutex_lock(_vpss_channel_mutex, OS_WAIT_FOREVER);
	if(vpss->bindcount <= 1 && VIDHAL_RES_FLAG_CHECK(vpss->res_flag, START))	
	{
    	ret = zpl_vidhal_vpss_channel_stop(vpss);
		if(ret == OK)	
		{
			VIDHAL_RES_FLAG_UNSET(vpss->res_flag, START);
			vpss->online = zpl_false;
		}
		if(ZPL_MEDIA_DEBUG(VPSS, EVENT))
		{
			zpl_media_debugmsg_debug("stop vpss channel(%d) ret=%d", vpss->vpss_channel, ret);
		} 
	}
	else
		ret = OK;	
	if(_vpss_channel_mutex)
		os_mutex_unlock(_vpss_channel_mutex);
    return ret;
}

int zpl_video_vpss_channel_inactive(zpl_video_vpss_channel_t *vpss)
{
    int ret = -1;
    zpl_video_assert(vpss);
	if (_vpss_channel_mutex)
		os_mutex_lock(_vpss_channel_mutex, OS_WAIT_FOREVER);
	if(VIDHAL_RES_FLAG_CHECK(vpss->res_flag, CREATE) && VIDHAL_RES_FLAG_CHECK(vpss->res_flag, INIT))
	{	
    	ret = zpl_vidhal_vpss_channel_destroy(vpss);
		if(ret == OK)
			VIDHAL_RES_FLAG_UNSET(vpss->res_flag, INIT);
		if(ZPL_MEDIA_DEBUG(VPSS, EVENT))
		{
			zpl_media_debugmsg_debug("inactive vpss channel(%d) ret=%d", vpss->vpss_channel, ret);
		} 
	}
	else
		ret = OK;
	if(_vpss_channel_mutex)
		os_mutex_unlock(_vpss_channel_mutex);
    return ret;
}

int zpl_video_vpss_channel_online_set(zpl_video_vpss_channel_t *vpss, zpl_bool online)
{
    int ret = -1;
    zpl_video_assert(vpss);
	if (_vpss_channel_mutex)
		os_mutex_lock(_vpss_channel_mutex, OS_WAIT_FOREVER);
    ret = OK;
	vpss->online = online;
	if(_vpss_channel_mutex)
		os_mutex_unlock(_vpss_channel_mutex);
    return ret;
}
int zpl_video_vpss_channel_online_get(zpl_video_vpss_channel_t *vpss, zpl_bool *online)
{
    int ret = -1;
    zpl_video_assert(vpss);
	if (_vpss_channel_mutex)
		os_mutex_lock(_vpss_channel_mutex, OS_WAIT_FOREVER);
    ret = OK;
	if(online)
		*online = vpss->online;
	if(_vpss_channel_mutex)
		os_mutex_unlock(_vpss_channel_mutex);
    return ret;
}


#ifdef ZPL_SHELL_MODULE
int zpl_video_vpss_channel_show(void *pvoid)
{
	NODE *node;
	zpl_video_vpss_channel_t *t;	
	struct vty *vty = (struct vty *)pvoid;

	if (_vpss_channel_mutex)
		os_mutex_lock(_vpss_channel_mutex, OS_WAIT_FOREVER);

	if(lstCount(_vpss_channel_list))
	{
		vty_out(vty, "%-4s %-4s %-6s %-8s %-8s %-16s %s", "----", "----", "------", "--------", "--------", "----------------", VTY_NEWLINE);
		vty_out(vty, "%-4s %-4s %-6s %-8s %-8s %-16s %s", "ID", "FLAG", "ONLINE", "BINDCNT", "BINDID",  "SIZE", VTY_NEWLINE);
		vty_out(vty, "%-4s %-4s %-6s %-8s %-8s %-16s %s", "----", "----", "------", "--------", "--------", "----------------", VTY_NEWLINE);
	}
	for (node = lstFirst(_vpss_channel_list); node != NULL; node = lstNext(node))
	{
		t = (zpl_video_vpss_channel_t *) node;
		if (node)
		{
#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
			vty_out(vty, "%-4d  0x%-4x %-6s  %-8d %-8d %dx%d%s", t->vpss_channel, t->res_flag, t->online ? "ONLINE":"OFFLINE", 
					t->bindcount, t->vpssgrp?t->vpssgrp->vpss_group:-1, 
					t->input_size.width, t->input_size.height, VTY_NEWLINE);
#else
			vty_out(vty, "%-4d  0x%-4x %-6s  %-8d %-8d %dx%d%s", t->vpss_channel, t->res_flag, t->online ? "ONLINE":"OFFLINE", 
					t->bindcount, t->video_input?t->video_input->vpss_group:-1, 
					t->input_size.width, t->input_size.height, VTY_NEWLINE);
#endif
		}
	}
	if (_vpss_channel_mutex)
		os_mutex_unlock(_vpss_channel_mutex);
	return OK;
}
#endif