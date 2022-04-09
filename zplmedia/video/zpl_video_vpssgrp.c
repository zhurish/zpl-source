/*
 * zpl_video_vpssgrp.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"




#ifdef ZPL_VIDEO_VPSSGRP_ENABLE	
static LIST *_vpss_group_list = NULL;
static os_mutex_t *_vpss_group_mutex = NULL;

int zpl_video_vpssgrp_init()
{

	if(_vpss_group_list == NULL)
	{
		_vpss_group_list = os_malloc(sizeof(LIST));
		if(_vpss_group_list)
		{
			lstInit(_vpss_group_list);
		}
		else
			return ERROR;
	}
	if(_vpss_group_mutex == NULL)
	{
		_vpss_group_mutex = os_mutex_init();
	}
	return ERROR;
}

int zpl_video_vpssgrp_exit()
{
	if(_vpss_group_mutex)
	{
		if(os_mutex_exit(_vpss_group_mutex)==OK)
			_vpss_group_mutex = NULL;
	}
	if(_vpss_group_list)
	{
		lstFree(_vpss_group_list);
		_vpss_group_list = NULL;
	}
	return OK;
}


int zpl_video_vpssgrp_count(zpl_int32 vpssgrp)
{
	int count = 0;
	NODE node;
	zpl_video_vpssgrp_t *t = NULL;
	if (_vpss_group_mutex)
		os_mutex_lock(_vpss_group_mutex, OS_WAIT_FOREVER);

	for(t = (zpl_video_vpssgrp_t *)lstFirst(_vpss_group_list); 
		t != NULL; t = (zpl_video_vpssgrp_t *)lstNext(&node))
	{
		node = t->node;
		if(t && t->vpss_group == vpssgrp)
		{
			count++;
		}
	}
	if(_vpss_group_mutex)
		os_mutex_unlock(_vpss_group_mutex);
	return count;
}

int zpl_video_vpssgrp_destroy(zpl_int32 vpssgrp)
{
	NODE node;
	zpl_video_vpssgrp_t *t = NULL;
	if (_vpss_group_mutex)
		os_mutex_lock(_vpss_group_mutex, OS_WAIT_FOREVER);

	for(t = (zpl_video_vpssgrp_t *)lstFirst(_vpss_group_list); 
		t != NULL; t = (zpl_video_vpssgrp_t *)lstNext(&node))
	{
		node = t->node;
		if(t && t->vpss_group == vpssgrp)
		{
			lstDelete (_vpss_group_list, (NODE *)t);
			os_free(t);	
			break;
		}
	}
	if(_vpss_group_mutex)
		os_mutex_unlock(_vpss_group_mutex);
	return OK;
}

int zpl_video_vpssgrp_create(zpl_int32 vpssgrp)
{
	zpl_video_vpssgrp_t *t = os_malloc(sizeof(zpl_video_vpssgrp_t));
	zpl_video_assert(t);
	if(t)
	{
		memset(t, 0, sizeof(zpl_video_vpssgrp_t));
		t->vpss_group = vpssgrp;
		if(_vpss_group_mutex)
			os_mutex_lock(_vpss_group_mutex, OS_WAIT_FOREVER);
		if(_vpss_group_list)
			lstAdd(_vpss_group_list, (NODE *)t);
		if(_vpss_group_mutex)
			os_mutex_unlock(_vpss_group_mutex);
		return OK;
	}
	return ERROR;
}



zpl_video_vpssgrp_t * zpl_video_vpssgrp_lookup(zpl_int32 vpssgrp)
{
	NODE node;
	zpl_video_vpssgrp_t *t = NULL;
	if (_vpss_group_mutex)
		os_mutex_lock(_vpss_group_mutex, OS_WAIT_FOREVER);

	for(t = (zpl_video_vpssgrp_t *)lstFirst(_vpss_group_list); 
		t != NULL; t = (zpl_video_vpssgrp_t *)lstNext(&node))
	{
		node = t->node;
		if(t && t->vpss_group == vpssgrp)
		{
			break;
		}
	}
	if(_vpss_group_mutex)
		os_mutex_unlock(_vpss_group_mutex);
	return t;
}

int zpl_video_vpssgrp_bindcount_set(zpl_video_vpssgrp_t *vpssgrp, zpl_bool add)
{
	if (_vpss_group_mutex)
		os_mutex_lock(_vpss_group_mutex, OS_WAIT_FOREVER);
	if(vpssgrp)
	{
		if(add)
			vpssgrp->bindcount++;
		else
			vpssgrp->bindcount++;
	}	
	if(_vpss_group_mutex)
		os_mutex_unlock(_vpss_group_mutex);
	return OK;	
}

int zpl_video_vpssgrp_bindcount_get(zpl_video_vpssgrp_t *vpssgrp)
{
	int bindcount = 0;
	if (_vpss_group_mutex)
		os_mutex_lock(_vpss_group_mutex, OS_WAIT_FOREVER);
	if(vpssgrp)
	{
		bindcount = vpssgrp->bindcount;
	}	
	if(_vpss_group_mutex)
		os_mutex_unlock(_vpss_group_mutex);
	return bindcount;	
}

int zpl_video_vpssgrp_online_set(zpl_video_vpssgrp_t *vpssgrp, zpl_bool online)
{
	int ret = -1;
    zpl_video_assert(vpssgrp);
	if (_vpss_group_mutex)
		os_mutex_lock(_vpss_group_mutex, OS_WAIT_FOREVER);
    ret = OK;
	vpssgrp->online = online;
	if(_vpss_group_mutex)
		os_mutex_unlock(_vpss_group_mutex);
    return ret;
}
int zpl_video_vpssgrp_online_get(zpl_video_vpssgrp_t *vpssgrp, zpl_bool *online)
{
	int ret = -1;
    zpl_video_assert(vpssgrp);
	if (_vpss_group_mutex)
		os_mutex_lock(_vpss_group_mutex, OS_WAIT_FOREVER);
    ret = OK;
	if(online)
		*online = vpssgrp->online;
	if(_vpss_group_mutex)
		os_mutex_unlock(_vpss_group_mutex);
    return ret;
}




int zpl_video_vpssgrp_input_set(zpl_int32 vpssgrp, void *video_input)
{
	zpl_video_vpssgrp_t * t = zpl_video_vpssgrp_lookup(vpssgrp);
	if (_vpss_group_mutex)
		os_mutex_lock(_vpss_group_mutex, OS_WAIT_FOREVER);
	if(t)
	{
		t->video_input = video_input;
	}	
	if(_vpss_group_mutex)
		os_mutex_unlock(_vpss_group_mutex);
	return OK;	
}




int zpl_video_vpssgrp_sendto(zpl_video_vpssgrp_t *vpssgrp,  void *p, zpl_int timeout)
{
    int ret = -1;
    zpl_video_assert(vpssgrp);
	if (_vpss_group_mutex)
		os_mutex_lock(_vpss_group_mutex, OS_WAIT_FOREVER);
    if(vpssgrp->online)
        ret = zpl_vidhal_vpssgrp_frame_sendto(vpssgrp, p, timeout);
    else
        zpl_media_debugmsg_warn("Channel (%d) is offline", 
            vpssgrp->vpss_group);
	if(_vpss_group_mutex)
		os_mutex_unlock(_vpss_group_mutex);	
    return ret;
}





int zpl_video_vpssgrp_active(zpl_video_vpssgrp_t *vpssgrp)
{
    int ret = -1;
    zpl_video_assert(vpssgrp);
	if (_vpss_group_mutex)
		os_mutex_lock(_vpss_group_mutex, OS_WAIT_FOREVER);
	zpl_media_debugmsg_debug("vpss grp (%d)  Flags (0x%x)", vpssgrp->vpss_group, vpssgrp->res_flag);
	if(VIDHAL_RES_FLAG_CHECK(vpssgrp->res_flag, CREATE) && !VIDHAL_RES_FLAG_CHECK(vpssgrp->res_flag, INIT))
	{
    	ret = zpl_vidhal_vpssgrp_create(vpssgrp);
		if(ret == OK)
			VIDHAL_RES_FLAG_SET(vpssgrp->res_flag, INIT);
		if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT))
		{
			zpl_media_debugmsg_debug("active vpssgrp channel(%d) ret=%d", vpssgrp->vpss_group, ret);
		} 
	}
	else
	{
		ret = OK;
		if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT))
		{
			zpl_media_debugmsg_debug("vpssgrp channel(%d) is already active", vpssgrp->vpss_group);
		} 
	}
	if(_vpss_group_mutex)
		os_mutex_unlock(_vpss_group_mutex);
    return ret;
}

int zpl_video_vpssgrp_start(zpl_video_vpssgrp_t *vpssgrp)
{
    int ret = -1;
    zpl_video_assert(vpssgrp);
	if (_vpss_group_mutex)
		os_mutex_lock(_vpss_group_mutex, OS_WAIT_FOREVER);
	if(!VIDHAL_RES_FLAG_CHECK(vpssgrp->res_flag, START))
	{
    	ret = zpl_vidhal_vpssgrp_start(vpssgrp);
		if(ret == OK)	
		{
			VIDHAL_RES_FLAG_SET(vpssgrp->res_flag, START);
			vpssgrp->online = zpl_true;
		}
		if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT))
		{
			zpl_media_debugmsg_debug("start vpssgrp channel(%d) ret=%d", vpssgrp->vpss_group, ret);
		} 
	}
	else
	{
		ret = OK;
		if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT))
		{
			zpl_media_debugmsg_debug("vpssgrp channel(%d) is already start", vpssgrp->vpss_group);
		} 
	}

	if(_vpss_group_mutex)
		os_mutex_unlock(_vpss_group_mutex);
    return ret;
}

int zpl_video_vpssgrp_stop(zpl_video_vpssgrp_t *vpssgrp)
{
    int ret = -1;
    zpl_video_assert(vpssgrp);
	if (_vpss_group_mutex)
		os_mutex_lock(_vpss_group_mutex, OS_WAIT_FOREVER);
	if(vpssgrp->bindcount <= 1 && VIDHAL_RES_FLAG_CHECK(vpssgrp->res_flag, START))
	{
    	ret = zpl_vidhal_vpssgrp_stop(vpssgrp);
		if(ret == OK)	
		{
			VIDHAL_RES_FLAG_UNSET(vpssgrp->res_flag, START);
			vpssgrp->online = zpl_false;
		}
		if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT))
		{
			zpl_media_debugmsg_debug("stop vpssgrp channel(%d) ret=%d", vpssgrp->vpss_group, ret);
		} 
	}
	else
	{
		ret = OK;
		if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT))
		{
			zpl_media_debugmsg_debug("vpssgrp channel(%d) is already stop", vpssgrp->vpss_group);
		} 
	}	
	if(_vpss_group_mutex)
		os_mutex_unlock(_vpss_group_mutex);
    return ret;
}

int zpl_video_vpssgrp_inactive(zpl_video_vpssgrp_t *vpssgrp)
{
    int ret = -1;
    zpl_video_assert(vpssgrp);
	if (_vpss_group_mutex)
		os_mutex_lock(_vpss_group_mutex, OS_WAIT_FOREVER);
	if(VIDHAL_RES_FLAG_CHECK(vpssgrp->res_flag, CREATE) && VIDHAL_RES_FLAG_CHECK(vpssgrp->res_flag, INIT))
	{	
    	ret = zpl_vidhal_vpssgrp_destroy(vpssgrp);
		if(ret == OK)
			VIDHAL_RES_FLAG_UNSET(vpssgrp->res_flag, INIT);
		if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT))
		{
			zpl_media_debugmsg_debug("inactive vpssgrp channel(%d) ret=%d", vpssgrp->vpss_group, ret);
		} 
	}
	else
	{
		ret = OK;
		if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT))
		{
			zpl_media_debugmsg_debug("vpssgrp channel(%d) is already inactive", vpssgrp->vpss_group);
		} 
	}
	if(_vpss_group_mutex)
		os_mutex_unlock(_vpss_group_mutex);
    return ret;
}


#ifdef ZPL_SHELL_MODULE
int zpl_video_vpssgrp_show(void *pvoid)
{
	NODE *node;
	zpl_video_vpssgrp_t *t;	
	struct vty *vty = (struct vty *)pvoid;

	if (_vpss_group_mutex)
		os_mutex_lock(_vpss_group_mutex, OS_WAIT_FOREVER);

	if(lstCount(_vpss_group_list))
	{
		vty_out(vty, "%-4s %-4s %-6s %-8s %-8s %-16s %s", "----", "----", "------", "--------", "--------", "----------------", VTY_NEWLINE);
		vty_out(vty, "%-4s %-4s %-6s %-8s %-8s %-16s %s", "ID", "FLAG", "ONLINE", "BINDCNT", "BINDID",  "SIZE", VTY_NEWLINE);
		vty_out(vty, "%-4s %-4s %-6s %-8s %-8s %-16s %s", "----", "----", "------", "--------", "--------", "----------------", VTY_NEWLINE);
	}
	for (node = lstFirst(_vpss_group_list); node != NULL; node = lstNext(node))
	{
		t = (zpl_video_vpssgrp_t *) node;
		if (node)
		{
			vty_out(vty, "%-4d  0x%-4x %-6s  %-8d %-8d %dx%d%s", t->vpss_group, t->res_flag, t->online ? "ONLINE":"OFFLINE", 
					t->bindcount, t->video_input?((zpl_video_input_channel_t*)t->video_input)->input_chn:-1, 
					t->input_size.width, t->input_size.height, VTY_NEWLINE);
		}
	}
	if (_vpss_group_mutex)
		os_mutex_unlock(_vpss_group_mutex);
	return OK;
}
#endif

#endif