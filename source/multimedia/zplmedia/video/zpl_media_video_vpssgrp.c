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


int zpl_media_video_vpssgrp_count(zpl_int32 vpssgrp)
{
	int count = 0;
	NODE node;
	zpl_media_video_vpssgrp_t *t = NULL;
    LIST *_lst = NULL;
    os_mutex_t *_mutex = NULL;
    zpl_media_global_get(ZPL_MEDIA_GLOAL_VIDEO_VPSSGRP, &_lst, &_mutex);
	if(_lst == NULL)
		return 0;

	if (_mutex)
		os_mutex_lock(_mutex, OS_WAIT_FOREVER);

	for(t = (zpl_media_video_vpssgrp_t *)lstFirst(_lst); 
		t != NULL; t = (zpl_media_video_vpssgrp_t *)lstNext(&node))
	{
		node = t->node;
		if(t && t->vpss_group == vpssgrp)
		{
			count++;
		}
	}
	if(_mutex)
		os_mutex_unlock(_mutex);
	return count;
}

int zpl_media_video_vpssgrp_destroy(zpl_int32 vpssgrp)
{
	zpl_media_video_vpss_channel_t * t = (zpl_media_video_vpss_channel_t*)zpl_media_global_lookup(ZPL_MEDIA_GLOAL_VIDEO_VPSSGRP, 0, vpssgrp, 0);
	if(t)
	{
		zpl_media_global_del(ZPL_MEDIA_GLOAL_VIDEO_VPSSGRP, t);
		os_free(t);	
	}
	return OK;
}

int zpl_media_video_vpssgrp_create(zpl_int32 vpssgrp)
{
	zpl_media_video_vpssgrp_t *t = os_malloc(sizeof(zpl_media_video_vpssgrp_t));
	zpl_video_assert(t);
	if(t)
	{
		memset(t, 0, sizeof(zpl_media_video_vpssgrp_t));
		t->vpss_group = vpssgrp;
		zpl_media_global_add(ZPL_MEDIA_GLOAL_VIDEO_VPSSGRP, t);
		return OK;
	}
	return ERROR;
}



zpl_media_video_vpssgrp_t * zpl_media_video_vpssgrp_lookup(zpl_int32 vpssgrp)
{
	zpl_media_video_vpssgrp_t * t = (zpl_media_video_vpssgrp_t*)zpl_media_global_lookup(ZPL_MEDIA_GLOAL_VIDEO_VPSSGRP, 0, vpssgrp, 0);
	return t;
}

int zpl_media_video_vpssgrp_bindcount_set(zpl_media_video_vpssgrp_t *vpssgrp, zpl_bool add)
{
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSSGRP);
	if(vpssgrp)
	{
		if(add)
			vpssgrp->bindcount++;
		else
			vpssgrp->bindcount++;
	}	
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSSGRP);
	return OK;	
}

int zpl_media_video_vpssgrp_bindcount_get(zpl_media_video_vpssgrp_t *vpssgrp)
{
	int bindcount = 0;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSSGRP);
	if(vpssgrp)
	{
		bindcount = vpssgrp->bindcount;
	}	
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSSGRP);
	return bindcount;	
}

int zpl_media_video_vpssgrp_online_set(zpl_media_video_vpssgrp_t *vpssgrp, zpl_bool online)
{
	int ret = -1;
    zpl_video_assert(vpssgrp);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSSGRP);
    ret = OK;
	vpssgrp->online = online;
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSSGRP);
    return ret;
}
int zpl_media_video_vpssgrp_online_get(zpl_media_video_vpssgrp_t *vpssgrp, zpl_bool *online)
{
	int ret = -1;
    zpl_video_assert(vpssgrp);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSSGRP);
    ret = OK;
	if(online)
		*online = vpssgrp->online;
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSSGRP);
    return ret;
}




int zpl_media_video_vpssgrp_input_set(zpl_int32 vpssgrp, void *video_input)
{
	zpl_media_video_vpssgrp_t * t = zpl_media_video_vpssgrp_lookup(vpssgrp);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSSGRP);
	if(t)
	{
		t->video_input = video_input;
	}	
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSSGRP);
	return OK;	
}




int zpl_media_video_vpssgrp_sendto(zpl_media_video_vpssgrp_t *vpssgrp,  void *p, zpl_int timeout)
{
    int ret = -1;
    zpl_video_assert(vpssgrp);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSSGRP);
    if(vpssgrp->online)
        ret = zpl_vidhal_vpssgrp_frame_sendto(vpssgrp, p, timeout);
    else
        zpl_media_debugmsg_warn("Channel (%d) is offline", 
            vpssgrp->vpss_group);
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSSGRP);
    return ret;
}





int zpl_meida_video_vpssgrp_hal_create(zpl_media_video_vpssgrp_t *vpssgrp)
{
    int ret = -1;
    zpl_video_assert(vpssgrp);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSSGRP);
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
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSSGRP);
    return ret;
}

int zpl_media_video_vpssgrp_start(zpl_media_video_vpssgrp_t *vpssgrp)
{
    int ret = -1;
    zpl_video_assert(vpssgrp);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSSGRP);
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

	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSSGRP);
    return ret;
}

int zpl_media_video_vpssgrp_stop(zpl_media_video_vpssgrp_t *vpssgrp)
{
    int ret = -1;
    zpl_video_assert(vpssgrp);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSSGRP);
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
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSSGRP);
    return ret;
}

int zpl_meida_video_vpssgrp_hal_destroy(zpl_media_video_vpssgrp_t *vpssgrp)
{
    int ret = -1;
    zpl_video_assert(vpssgrp);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSSGRP);
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
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSSGRP);
    return ret;
}


#ifdef ZPL_SHELL_MODULE
int zpl_media_video_vpssgrp_show(void *pvoid)
{
	NODE *node;
	zpl_media_video_vpssgrp_t *t;	
	struct vty *vty = (struct vty *)pvoid;
    LIST *_lst = NULL;
    os_mutex_t *_mutex = NULL;
    zpl_media_global_get(ZPL_MEDIA_GLOAL_VIDEO_VPSSGRP, &_lst, &_mutex);
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
		t = (zpl_media_video_vpssgrp_t *) node;
		if (node)
		{
			vty_out(vty, "%-4d  0x%-4x %-6s  %-8d %-8d %dx%d%s", t->vpss_group, t->res_flag, t->online ? "ONLINE":"OFFLINE", 
					t->bindcount, t->video_input?((zpl_meida_video_input_channel_t*)t->video_input)->input_chn:-1, 
					t->input_size.width, t->input_size.height, VTY_NEWLINE);
		}
	}
	if (_mutex)
		os_mutex_unlock(_mutex);
	return OK;
}
#endif

#endif