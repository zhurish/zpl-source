/*
 * zpl_video_vpss.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"


static int zpl_media_video_vpss_channel_cmp(zpl_media_video_vpss_channel_t *encode, zpl_media_gkey_t *gkey)
{
	if(encode->vpss_channel == gkey->channel)
		return 0;
	return 1;	
}
#ifdef ZPL_VIDEO_VPSSGRP_ENABLE	
static int zpl_media_video_vpssgrp_cmp(zpl_media_video_vpssgrp_t *encode, zpl_media_gkey_t *gkey)
{
	if(encode->vpss_group == gkey->group)
		return 0;
	return 1;	
}
#endif

int zpl_media_video_vpss_init(void)
{
	zpl_media_global_gkey_cmpset(ZPL_MEDIA_GLOAL_VIDEO_VPSS, zpl_media_video_vpss_channel_cmp);
	#ifdef ZPL_VIDEO_VPSSGRP_ENABLE		
	zpl_media_global_gkey_cmpset(ZPL_MEDIA_GLOAL_VIDEO_VPSSGRP, zpl_media_video_vpssgrp_cmp);	
	#endif
	return OK;
}

int zpl_media_video_vpss_channel_count(zpl_int32 vpss_channel)
{
	int count = 0;
	NODE node;
	zpl_media_video_vpss_channel_t *t = NULL;
    LIST *_lst = NULL;
    os_mutex_t *_mutex = NULL;
    zpl_media_global_get(ZPL_MEDIA_GLOAL_VIDEO_VPSS, &_lst, &_mutex);
	if(_lst == NULL)
		return 0;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);

	for(t = (zpl_media_video_vpss_channel_t *)lstFirst(_lst); 
		t != NULL; t = (zpl_media_video_vpss_channel_t *)lstNext(&node))
	{
		node = t->node;
		if(t && t->vpss_channel == vpss_channel)
		{
			count++;
		}
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	return count;
}

int zpl_media_video_vpss_channel_delete(zpl_int32 vpss_channel)
{
	zpl_media_video_vpss_channel_t * t = (zpl_media_video_vpss_channel_t*)zpl_media_global_lookup(ZPL_MEDIA_GLOAL_VIDEO_VPSS, vpss_channel, 0, 0);
	if(t)
	{
		zpl_media_global_del(ZPL_MEDIA_GLOAL_VIDEO_VPSS, t);
		os_free(t);	
	}
	return OK;
}

int zpl_media_video_vpss_channel_new(zpl_int32 vpss_channel)
{
	zpl_media_video_vpss_channel_t *t = os_malloc(sizeof(zpl_media_video_vpss_channel_t));
	zpl_video_assert(t);
	if(t)
	{
		memset(t, 0, sizeof(zpl_media_video_vpss_channel_t));
		t->vpss_channel = vpss_channel;
		zpl_media_global_add(ZPL_MEDIA_GLOAL_VIDEO_VPSS, t);
		return OK;
	}
	return ERROR;
}



zpl_media_video_vpss_channel_t * zpl_media_video_vpss_channel_lookup(zpl_int32 vpss_channel)
{
	zpl_media_video_vpss_channel_t * t = (zpl_media_video_vpss_channel_t*)zpl_media_global_lookup(ZPL_MEDIA_GLOAL_VIDEO_VPSS, vpss_channel, 0, 0);
	return t;
}

#ifndef ZPL_VIDEO_VPSSGRP_ENABLE
int zpl_media_video_vpss_channel_input_set(zpl_int32 vpss_channel, void *video_input)
{
	zpl_media_video_vpss_channel_t * t = zpl_media_video_vpss_channel_lookup(vpss_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	if(t)
	{
		t->video_input = video_input;
	}	
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	return OK;	
}
#else
int zpl_media_video_vpss_channel_vpssgrp_set(zpl_int32 vpss_channel, void *vpssgrp)
{
	zpl_media_video_vpss_channel_t * t = zpl_media_video_vpss_channel_lookup(vpss_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	if(t)
	{
		t->vpssgrp = vpssgrp;
	}	
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	return OK;
}
#endif
int zpl_media_video_vpss_channel_bindcount_set(zpl_media_video_vpss_channel_t *vpss, zpl_bool add)
{
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	if(vpss)
	{
		if(add)
			vpss->bindcount++;
		else
			vpss->bindcount++;
	}	
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	return OK;	
}

int zpl_media_video_vpss_channel_bindcount_get(zpl_media_video_vpss_channel_t *vpss)
{
	int bindcount = 0;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	if(vpss)
	{
		bindcount = vpss->bindcount;
	}	
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	return bindcount;	
}


static int zpl_media_video_vpss_channel_read(struct thread *t)
{
	zpl_media_video_vpss_channel_t *vpss = THREAD_ARG(t);

	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
    if(vpss)
    {
        vpss->t_read = NULL;

        zpl_vidhal_vpss_channel_frame_recvfrom(vpss);

        if(!ipstack_invalid(vpss->vpssfd))
           vpss->t_read = thread_add_read(t->master, zpl_media_video_vpss_channel_read, vpss, vpss->vpssfd);
    }
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	return OK;
}

int zpl_media_video_vpss_channel_read_start(zpl_void *master, zpl_media_video_vpss_channel_t *vpss)
{
    zpl_video_assert(master);
    zpl_video_assert(vpss);
	if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, DETAIL))
		zpl_media_debugmsg_debug(" start video VPSS channel (%d/%d) read thread\n", 
			vpss->vpssgrp?vpss->vpssgrp->vpss_group:-1, vpss->vpss_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	zpl_vidhal_vpss_channel_update_fd(vpss);	
    if(master && vpss && !ipstack_invalid(vpss->vpssfd))
	{
		vpss->t_master = master;
        vpss->t_read = thread_add_read(master, zpl_media_video_vpss_channel_read, vpss, vpss->vpssfd);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
    return OK;    
}

int zpl_media_video_vpss_channel_read_stop(zpl_media_video_vpss_channel_t *vpss)
{
    zpl_video_assert(vpss);  
	if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, DETAIL))
		zpl_media_debugmsg_debug(" stop video VPSS channel (%d/%d) read thread\n", 
			vpss->vpssgrp?vpss->vpssgrp->vpss_group:-1, vpss->vpss_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
    if(vpss && vpss->t_read)
    {
        thread_cancel(vpss->t_read);
        vpss->t_read = NULL;
    }
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
    return OK; 
}





int zpl_meida_video_vpss_channel_hal_create(zpl_media_video_vpss_channel_t *vpss)
{
    int ret = -1;
    zpl_video_assert(vpss);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
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

	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
    return ret;
}

int zpl_media_video_vpss_channel_start(zpl_media_video_vpss_channel_t *vpss)
{
    int ret = -1;
    zpl_video_assert(vpss);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
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
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
    return ret;
}

int zpl_media_video_vpss_channel_stop(zpl_media_video_vpss_channel_t *vpss)
{
    int ret = -1;
    zpl_video_assert(vpss);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
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
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
    return ret;
}

int zpl_meida_video_vpss_channel_hal_destroy(zpl_media_video_vpss_channel_t *vpss)
{
    int ret = -1;
    zpl_video_assert(vpss);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
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
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
    return ret;
}

int zpl_media_video_vpss_channel_online_set(zpl_media_video_vpss_channel_t *vpss, zpl_bool online)
{
    int ret = -1;
    zpl_video_assert(vpss);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
    ret = OK;
	vpss->online = online;
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
    return ret;
}
int zpl_media_video_vpss_channel_online_get(zpl_media_video_vpss_channel_t *vpss, zpl_bool *online)
{
    int ret = -1;
    zpl_video_assert(vpss);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
    ret = OK;
	if(online)
		*online = vpss->online;
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
    return ret;
}


#ifdef ZPL_SHELL_MODULE
int zpl_media_video_vpss_channel_show(void *pvoid)
{
	NODE *node;
	zpl_media_video_vpss_channel_t *t;	
	struct vty *vty = (struct vty *)pvoid;
    LIST *_lst = NULL;
    os_mutex_t *_mutex = NULL;
    zpl_media_global_get(ZPL_MEDIA_GLOAL_VIDEO_VPSS, &_lst, &_mutex);
	if(_lst == NULL)
		return ERROR;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);

	if(lstCount(_lst))
	{
		vty_out(vty, "%-4s %-4s %-6s %-8s %-8s %-16s %s", "----", "----", "------", "--------", "--------", "----------------", VTY_NEWLINE);
		vty_out(vty, "%-4s %-4s %-6s %-8s %-8s %-16s %s", "ID", "FLAG", "ONLINE", "BINDCNT", "BINDID",  "SIZE", VTY_NEWLINE);
		vty_out(vty, "%-4s %-4s %-6s %-8s %-8s %-16s %s", "----", "----", "------", "--------", "--------", "----------------", VTY_NEWLINE);
	}
	for (node = lstFirst(_lst); node != NULL; node = lstNext(node))
	{
		t = (zpl_media_video_vpss_channel_t *) node;
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
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	return OK;
}
#endif