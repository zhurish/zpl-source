/*
 * zpl_video_vpss.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "zpl_vidhal.h"
#include "zpl_vidhal_internal.h"

static int zpl_media_video_vpsschn_read(struct thread *t_thread);

static int zpl_media_video_vpsschn_cmp(zpl_media_video_vpsschn_t *encode, zpl_media_gkey_t *gkey)
{
	if (encode->vpss_channel == gkey->channel && encode->vpss_group == gkey->group)
		return 0;
	return 1;
}
static void zpl_media_video_vpsschn_free(void *pVoid)
{
	zpl_media_video_vpsschn_t *vpsschn = pVoid;
	if(vpsschn)
	{
		zpl_media_video_vpsschn_hal_destroy(vpsschn);
		if((vpsschn->vpssfd != ZPL_SOCKET_INVALID))
		{
			ipstack_drstroy(vpsschn->vpssfd);
			vpsschn->vpssfd = ZPL_SOCKET_INVALID;
		}
		os_free(vpsschn);
	}
	return ;
}

zpl_media_video_vpsschn_t *zpl_media_video_vpsschn_lookup(zpl_int32 vpss_group, zpl_int32 vpss_channel)
{
	zpl_media_video_vpsschn_t *vpsschn = (zpl_media_video_vpsschn_t *)zpl_media_global_lookup(ZPL_MEDIA_GLOAL_VIDEO_VPSS, vpss_channel, vpss_group, 0);
	return vpsschn;
}

int zpl_media_video_vpsschn_init(void)
{
	zpl_media_global_gkey_cmpset(ZPL_MEDIA_GLOAL_VIDEO_VPSS, zpl_media_video_vpsschn_cmp);
	zpl_media_global_freeset(ZPL_MEDIA_GLOAL_VIDEO_VPSS, zpl_media_video_vpsschn_free);
	return OK;
}

int zpl_media_video_vpssgrp_lookup(zpl_int32 vpss_group)
{
	int count = 0;
	NODE node;
	zpl_media_video_vpsschn_t *vpsschn = NULL;
	LIST *_lst = NULL;
	os_mutex_t *_mutex = NULL;
	zpl_media_global_get(ZPL_MEDIA_GLOAL_VIDEO_VPSS, &_lst, &_mutex);
	if (_lst == NULL)
		return 0;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);

	for (vpsschn = (zpl_media_video_vpsschn_t *)lstFirst(_lst);
		 vpsschn != NULL; vpsschn = (zpl_media_video_vpsschn_t *)lstNext(&node))
	{
		node = vpsschn->node;
		if (vpsschn && vpsschn->vpss_group == vpss_group)
		{
			count++;
		}
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	return count;
}

static int zpl_media_video_vpssgrp_bind_count_update(zpl_int32 vpss_group)
{
	int count = 0;
	NODE node;
	zpl_media_video_vpsschn_t *vpsschn = NULL;
	LIST *_lst = NULL;
	os_mutex_t *_mutex = NULL;
	zpl_media_global_get(ZPL_MEDIA_GLOAL_VIDEO_VPSS, &_lst, &_mutex);
	if (_lst == NULL)
		return 0;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	for (vpsschn = (zpl_media_video_vpsschn_t *)lstFirst(_lst);
		 vpsschn != NULL; vpsschn = (zpl_media_video_vpsschn_t *)lstNext(&node))
	{
		node = vpsschn->node;
		if (vpsschn && vpsschn->vpss_group == vpss_group)
		{
			count++;
		}
	}
	for (vpsschn = (zpl_media_video_vpsschn_t *)lstFirst(_lst);
		 vpsschn != NULL; vpsschn = (zpl_media_video_vpsschn_t *)lstNext(&node))
	{
		node = vpsschn->node;
		if (vpsschn && vpsschn->vpss_group == vpss_group)
		{
			vpsschn->grp_bind_count = count;
		}
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	return count;
}

zpl_media_video_vpsschn_t *zpl_media_video_vpsschn_create(zpl_int32 vpss_group, zpl_int32 vpss_channel, zpl_video_size_t output_size)
{
	zpl_media_video_vpsschn_t *vpsschn = NULL;

	if (zpl_media_video_vpsschn_lookup(vpss_group, vpss_channel) != NULL)
	{
		return NULL;
	}
	vpsschn = os_malloc(sizeof(zpl_media_video_vpsschn_t));
	zpl_video_assert(vpsschn);
	if (vpsschn)
	{
		zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
		memset(vpsschn, 0, sizeof(zpl_media_video_vpsschn_t));
		vpsschn->vpss_channel = vpss_channel;
		vpsschn->vpss_group = vpss_group;
		vpsschn->vpssfd = ipstack_create(IPSTACK_OS);
		//vpsschn->channel = channel;
		//vpsschn->input_size.width = input_size.width;	  // 宽度
		//vpsschn->input_size.height = input_size.height;	  // 高度
		vpsschn->output_size.width = output_size.width;	  // 宽度
		vpsschn->output_size.height = output_size.height; // 高度
		zm_msg_debug("vpsschn channel (%d)", vpsschn->vpss_channel);

		zpl_media_global_add(ZPL_MEDIA_GLOAL_VIDEO_VPSS, vpsschn);
		zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
		zpl_media_video_vpssgrp_bind_count_update(vpss_group);
		return vpsschn;
	}
	return NULL;
}

int zpl_media_video_vpsschn_outputsize_get(zpl_int32 vpss_group, zpl_int32 vpss_channel, zpl_video_size_t *output_size)
{
	zpl_media_video_vpsschn_t *vpsschn = zpl_media_video_vpsschn_lookup(vpss_group, vpss_channel);
	if (vpsschn != NULL)
	{
		output_size->width = vpsschn->output_size.width;   // 宽度
		output_size->height = vpsschn->output_size.height; // 高度
		return OK;
	}
	return ERROR;
}

int zpl_media_video_vpsschn_hal_create(zpl_media_video_vpsschn_t *vpsschn)
{
	int ret = ERROR;
	zpl_video_assert(vpsschn);
	if (vpsschn && !ZPL_TST_BIT(vpsschn->flags, ZPL_MEDIA_STATE_ACTIVE))
	{
		zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);

		zm_msg_debug("vpsschn channel (%d) ", vpsschn->vpss_channel);

		ret = zpl_vidhal_vpss_channel_create(vpsschn->vpss_group, vpsschn->vpss_channel, vpsschn);
		if (ret != OK)
		{
			zm_msg_error("create vpss group %d channel %d failed.", vpsschn->vpss_group, vpsschn->vpss_channel);
			zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
			return ret;
		}

		ZPL_SET_BIT(vpsschn->flags, ZPL_MEDIA_STATE_ACTIVE);

		if (ZPL_MEDIA_DEBUG(VPSS, EVENT))
		{
			zm_msg_debug("active vpsschn channel(%d) ret=%d", vpsschn->vpss_channel, ret);
		}
		zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
		return ret;
	}
	return ret;
}

int zpl_media_video_vpsschn_destroy(zpl_media_video_vpsschn_t *vpsschn)
{
	int ret = -1, vpss_group = -1;
	zpl_media_video_inputchn_t *inputchn = NULL;
	zpl_video_assert(vpsschn);
	vpss_group = vpsschn->vpss_group;
	zpl_media_video_vpsschn_hal_destroy(vpsschn);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	inputchn = vpsschn->source_input;
	if(inputchn)
		inputchn->dest_output = NULL;
	zpl_media_global_del(ZPL_MEDIA_GLOAL_VIDEO_VPSS, vpsschn);
	if((vpsschn->vpssfd != ZPL_SOCKET_INVALID))
	{
		ipstack_drstroy(vpsschn->vpssfd);
		vpsschn->vpssfd = ZPL_SOCKET_INVALID;
	}
	os_free(vpsschn);
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	zpl_media_video_vpssgrp_bind_count_update(vpss_group);
	return ret;
}

int zpl_media_video_vpsschn_hal_destroy(zpl_media_video_vpsschn_t *vpsschn)
{
	int ret = -1, vpss_group = -1;
	zpl_video_assert(vpsschn);
	vpss_group = vpsschn->vpss_group;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	if (vpsschn && vpsschn->t_read)
	{
		thread_cancel(vpsschn->t_read);
		vpsschn->t_read = NULL;
	}
	if (vpsschn->reference <= 1 && ZPL_TST_BIT(vpsschn->flags, ZPL_MEDIA_STATE_START))
	{
		ret = zpl_vidhal_vpss_channel_stop(vpsschn->vpss_group, vpsschn->vpss_channel, vpsschn);
		if (ret != OK)
		{
			zm_msg_error("stop vpss group %d channel %d failed.", vpsschn->vpss_group, vpsschn->vpss_channel);
			zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
			return ret;
		}
		vpsschn->online = zpl_false;
		ZPL_CLR_BIT(vpsschn->flags, ZPL_MEDIA_STATE_START);

		if (ZPL_MEDIA_DEBUG(VPSS, EVENT))
		{
			zm_msg_debug("stop vpsschn channel(%d) ret=%d", vpsschn->vpss_channel, ret);
		}
	}
	if (ZPL_TST_BIT(vpsschn->flags, ZPL_MEDIA_STATE_HWBIND))
	{
		zpl_media_video_inputchn_t *inputchn = vpsschn->source_input;
		if(inputchn)
		{
			if (zpl_syshal_input_unbind_vpss(inputchn->input_pipe, inputchn->input_chn, vpsschn->vpss_group) != OK)
			{
				zm_msg_error("input_pipe %d input_chn %d can not unbind to vpssgrp (%d)", inputchn->input_pipe, inputchn->input_chn, vpsschn->vpss_group);
				zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
				return ERROR;
			}
			if (ZPL_MEDIA_DEBUG(VPSS, EVENT))
			{
				zm_msg_debug("input_pipe %d input_chn %d unbind to vpssgrp (%d)", inputchn->input_pipe, inputchn->input_chn, vpsschn->vpss_group);
			}
			if(inputchn)
				inputchn->dest_output = NULL;
		}
		ZPL_CLR_BIT(vpsschn->flags, ZPL_MEDIA_STATE_HWBIND);
		vpsschn->hwbind = zpl_false;
	}
	if (ZPL_TST_BIT(vpsschn->flags, ZPL_MEDIA_STATE_ACTIVE))
	{
		ret = zpl_vidhal_vpss_channel_destroy(vpsschn->vpss_group, vpsschn->vpss_channel, vpsschn);
		if (ret != OK)
		{
			zm_msg_error("destroy vpss group %d channel %d failed.", vpsschn->vpss_group, vpsschn->vpss_channel);
			zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
			return ret;
		}
		ZPL_CLR_BIT(vpsschn->flags, ZPL_MEDIA_STATE_ACTIVE);
		if (ZPL_MEDIA_DEBUG(VPSS, EVENT))
		{
			zm_msg_debug("inactive vpsschn channel(%d) ret=%d", vpsschn->vpss_channel, ret);
		}
	}
	else
		ret = OK;
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	zpl_media_video_vpssgrp_bind_count_update(vpss_group);
	return ret;
}

int zpl_media_video_vpsschn_start(void *master, zpl_media_video_vpsschn_t *vpsschn)
{
	int ret = -1;
	zpl_video_assert(vpsschn);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);

	if (!ZPL_TST_BIT(vpsschn->flags, ZPL_MEDIA_STATE_START))
	{
		ret = zpl_vidhal_vpss_channel_start(vpsschn->vpss_group, vpsschn->vpss_channel, vpsschn);
		if (ret != OK)
		{
			zm_msg_error("start vpss group %d channel %d failed.", vpsschn->vpss_group, vpsschn->vpss_channel);
			zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
			return ret;
		}

		vpsschn->online = zpl_true;
		ZPL_SET_BIT(vpsschn->flags, ZPL_MEDIA_STATE_START);
		
		if (ZPL_MEDIA_DEBUG(VPSS, EVENT))
		{
			zm_msg_debug("start vpsschn channel(%d) ret=%d", vpsschn->vpss_channel, ret);
		}
	}
	else
		ret = OK;
	zpl_vidhal_vpss_channel_update_fd(vpsschn);
	if (master && vpsschn && !ipstack_invalid(vpsschn->vpssfd) && vpsschn->hwbind == zpl_false)
	{
		vpsschn->t_master = master;
		vpsschn->t_read = thread_add_read(master, zpl_media_video_vpsschn_read, vpsschn, vpsschn->vpssfd);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	return ret;
}

int zpl_media_video_vpsschn_stop(zpl_media_video_vpsschn_t *vpsschn)
{
	int ret = -1;
	zpl_video_assert(vpsschn);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	if (vpsschn && vpsschn->t_read)
	{
		thread_cancel(vpsschn->t_read);
		vpsschn->t_read = NULL;
	}
	if (vpsschn->reference <= 1 && ZPL_TST_BIT(vpsschn->flags, ZPL_MEDIA_STATE_START))
	{
		ret = zpl_vidhal_vpss_channel_stop(vpsschn->vpss_group, vpsschn->vpss_channel, vpsschn);
		if (ret != OK)
		{
			zm_msg_error("stop vpss group %d channel %d failed.", vpsschn->vpss_group, vpsschn->vpss_channel);
			zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
			return ret;
		}

		vpsschn->online = zpl_false;
		ZPL_CLR_BIT(vpsschn->flags, ZPL_MEDIA_STATE_START);
		if (ZPL_MEDIA_DEBUG(VPSS, EVENT))
		{
			zm_msg_debug("stop vpsschn channel(%d) ret=%d", vpsschn->vpss_channel, ret);
		}
	}
	else
		ret = OK;

	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	return ret;
}

zpl_bool zpl_media_video_vpsschn_state_check(zpl_media_video_vpsschn_t *vpsschn, int bit)
{
	zpl_bool ret = zpl_false;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	if (vpsschn)
	{
		if (ZPL_TST_BIT(vpsschn->flags, bit))
			ret = zpl_true;
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	return ret;
}

int zpl_media_video_vpsschn_thread(zpl_media_video_vpsschn_t *vpsschn, zpl_bool start)
{
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	if (start == zpl_false && vpsschn && vpsschn->t_read)
	{
		thread_cancel(vpsschn->t_read);
		vpsschn->t_read = NULL;
		zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
		return OK;
	}
	zpl_vidhal_vpss_channel_update_fd(vpsschn);
	if (start && vpsschn && !ipstack_invalid(vpsschn->vpssfd))
	{
		vpsschn->t_read = thread_add_read(vpsschn->t_master, zpl_media_video_vpsschn_read, vpsschn, vpsschn->vpssfd);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	return OK;
}

int zpl_media_video_vpsschn_connect(zpl_int32 vpss_group, zpl_int32 vpss_channel, zpl_int32 venc_channel, zpl_bool hwbind)
{
	int ret = ERROR;
	zpl_media_video_vpsschn_t *vpsschn = (zpl_media_video_vpsschn_t *)zpl_media_global_lookup(ZPL_MEDIA_GLOAL_VIDEO_VPSS, vpss_channel, vpss_group, 0);
	zpl_media_video_encode_t *encode = (zpl_media_video_encode_t *)zpl_media_global_lookup(ZPL_MEDIA_GLOAL_VIDEO_ENCODE, venc_channel, 0, 0);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	if (vpsschn && encode && vpsschn->dest_output == encode)
	{
		int vpssgrp = 0;
		if(vpsschn)
		{
			vpssgrp = vpsschn->vpss_group; // 发送目的ID
			if (hwbind && !ZPL_TST_BIT(vpsschn->flags, ZPL_MEDIA_STATE_HWBIND))
			{
				if (vpsschn->t_read)
				{
					thread_cancel(vpsschn->t_read);
					vpsschn->t_read = NULL;
				}
				if (zpl_syshal_vpss_bind_venc(vpssgrp, vpsschn->vpss_channel, encode->venc_channel) != OK)
				{
					zm_msg_error("vpssgrp %d vpsschn %d can not bind to encode channel(%d)", vpssgrp, vpsschn->vpss_channel, encode->venc_channel);
					zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
					return ERROR;
				}
				if (ZPL_MEDIA_DEBUG(ENCODE, EVENT))
				{
					zm_msg_debug("vpssgrp %d vpsschn %d bind to encode channel(%d)", vpssgrp, vpsschn->vpss_channel, encode->venc_channel);
				}
				ZPL_SET_BIT(vpsschn->flags, ZPL_MEDIA_STATE_HWBIND);
				vpsschn->hwbind = zpl_true;
				ret = OK;
			}
			else if (hwbind == zpl_false && ZPL_TST_BIT(vpsschn->flags, ZPL_MEDIA_STATE_HWBIND))
			{
				if (vpsschn->t_read)
				{
					thread_cancel(vpsschn->t_read);
					vpsschn->t_read = NULL;
				}
				if (zpl_syshal_vpss_unbind_venc(vpssgrp, vpsschn->vpss_channel, encode->venc_channel) != OK)
				{
					zm_msg_error("vpssgrp %d vpsschn %d can not unbind to encode channel(%d)", vpssgrp, vpsschn->vpss_channel, encode->venc_channel);
					zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
					return ERROR;
				}
				if (ZPL_MEDIA_DEBUG(ENCODE, EVENT))
				{
					zm_msg_debug("vpssgrp %d vpsschn %d unbind to encode channel(%d)", vpssgrp, vpsschn->vpss_channel, encode->venc_channel);
				}
				ZPL_CLR_BIT(vpsschn->flags, ZPL_MEDIA_STATE_HWBIND);
				vpsschn->hwbind = zpl_false;
				zpl_vidhal_vpss_channel_update_fd(vpsschn);
				if (!ipstack_invalid(vpsschn->vpssfd) && vpsschn->t_master)
				{
					if (ZPL_TST_BIT(vpsschn->flags, ZPL_MEDIA_STATE_START))
						vpsschn->t_read = thread_add_read(vpsschn->t_master, zpl_media_video_vpsschn_read, vpsschn, vpsschn->vpssfd);
				}
				ret = OK;
			}
			else
			{
				ret = OK;
			}
		}
		else
			zm_msg_error("can not get source stream of vpssgrp %d vpsschn %d on encode channel(%d)", vpssgrp, vpsschn->vpss_channel, encode->venc_channel);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	return ret;
}

int zpl_media_video_vpsschn_source_set(zpl_int32 vpss_group, zpl_int32 vpss_channel, void *video_input)
{
	int ret = ERROR;
	zpl_media_video_vpsschn_t *vpsschn = (zpl_media_video_vpsschn_t *)zpl_media_global_lookup(ZPL_MEDIA_GLOAL_VIDEO_VPSS, vpss_channel, vpss_group, 0);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	if (vpsschn)
	{
		zpl_media_video_inputchn_t *inputchn = video_input;
		if(inputchn)
		{
			vpsschn->input_size.width = inputchn->output_size.width;
        	vpsschn->input_size.height = inputchn->output_size.height;
			//vpsschn->output_size.width = inputchn->input_size.width;
        	//vpsschn->output_size.height = inputchn->input_size.height;
			vpsschn->source_input = video_input;
			inputchn->dest_output = vpsschn;
			ret = OK;
			zpl_media_video_inputchn_addref(inputchn);
		}
		else
		{
			if(vpsschn->source_input)
			{
				inputchn = vpsschn->source_input;
				zpl_media_video_inputchn_delref(vpsschn->source_input);
				if (vpsschn->t_read)
				{
					thread_cancel(vpsschn->t_read);
					vpsschn->t_read = NULL;
				}
				inputchn->dest_output = NULL;
				vpsschn->source_input = NULL;
				ret = OK;
			}
		}
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	return ret;
}

static int zpl_media_video_vpsschn_reference_set(zpl_media_video_vpsschn_t *vpsschn, zpl_bool add)
{
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	if (vpsschn)
	{
		if (add)
			vpsschn->reference++;
		else
			vpsschn->reference++;
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	return OK;
}

int zpl_media_video_vpsschn_addref(zpl_media_video_vpsschn_t *vpsschn)
{
	return zpl_media_video_vpsschn_reference_set(vpsschn, zpl_true);
}
int zpl_media_video_vpsschn_delref(zpl_media_video_vpsschn_t *vpsschn)
{
	return zpl_media_video_vpsschn_reference_set(vpsschn, zpl_false);
}

int zpl_media_video_vpsschn_reference(zpl_media_video_vpsschn_t *vpsschn)
{
	int reference = 0;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	if (vpsschn)
	{
		reference = vpsschn->reference;
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	return reference;
}

static int zpl_media_video_vpsschn_read(struct thread *t_thread)
{
	zpl_media_video_vpsschn_t *vpsschn = THREAD_ARG(t_thread);

	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	if (vpsschn)
	{
		vpsschn->t_read = NULL;

		zpl_vidhal_vpss_channel_frame_recvfrom(vpsschn);

		if (!ipstack_invalid(vpsschn->vpssfd))
			vpsschn->t_read = thread_add_read(vpsschn->t_master, zpl_media_video_vpsschn_read, vpsschn, vpsschn->vpssfd);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	return OK;
}

int zpl_media_video_vpsschn_sendto(zpl_media_video_vpsschn_t *vpsschn, void *p, zpl_int timeout)
{
	int ret = -1;
	zpl_video_assert(vpsschn);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	if (vpsschn->online)
		ret = zpl_vidhal_vpss_channel_frame_sendto(vpsschn, p, timeout);
	else
		zm_msg_warn("Channel (%d) is offline",
								vpsschn->vpss_group);
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	return ret;
}

#ifdef ZPL_SHELL_MODULE
int zpl_media_video_vpsschn_show(void *pvoid)
{
	NODE *node;
	zpl_media_video_vpsschn_t *vpsschn = NULL;
	struct vty *vty = (struct vty *)pvoid;
	LIST *_lst = NULL;
	os_mutex_t *_mutex = NULL;
	zpl_media_video_inputchn_t *inputchn;
	zpl_media_global_get(ZPL_MEDIA_GLOAL_VIDEO_VPSS, &_lst, &_mutex);
	if (_lst == NULL)
		return ERROR;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);

	if (lstCount(_lst))
	{
		vty_out(vty, "-----------------------------------------%s", VTY_NEWLINE);
	}
	for (node = lstFirst(_lst); node != NULL; node = lstNext(node))
	{
		vpsschn = (zpl_media_video_vpsschn_t *)node;
		if (node)
		{
			vty_out(vty, "group               : %d%s", vpsschn->vpss_group, VTY_NEWLINE);
			vty_out(vty, " channel            : %d%s", vpsschn->vpss_channel, VTY_NEWLINE);
			vty_out(vty, "  input size        : %dx%d%s", vpsschn->input_size.width, vpsschn->input_size.height, VTY_NEWLINE);
			vty_out(vty, "  output size       : %dx%d%s", vpsschn->output_size.width, vpsschn->output_size.height, VTY_NEWLINE);
			vty_out(vty, "  reference         : %d%s", vpsschn->reference, VTY_NEWLINE);
			vty_out(vty, "  grp chn count     : %d%s", vpsschn->grp_bind_count, VTY_NEWLINE);
			vty_out(vty, "  hwbind            : %d%s", vpsschn->hwbind, VTY_NEWLINE);
			vty_out(vty, "  flags             : 0x%x%s", vpsschn->flags, VTY_NEWLINE);
			vty_out(vty, "  online            : %s%s", vpsschn->online ? "ONLINE" : "OFFLINE", VTY_NEWLINE);
			vty_out(vty, "  master            : %p%s", vpsschn->t_master, VTY_NEWLINE);
			if(vpsschn->vpssfd != ZPL_SOCKET_INVALID)
			{
				vty_out(vty, "  fd                : %d%s", ipstack_fd(vpsschn->vpssfd), VTY_NEWLINE);
			}
			inputchn = (zpl_media_video_inputchn_t *)vpsschn->source_input;
			vty_out(vty, "  source_input      : pipe %d channel %d extchannel %d%s", inputchn ? inputchn->input_pipe:-1, 
				inputchn ? inputchn->input_chn : -1, inputchn ? inputchn->input_extchn : -1, VTY_NEWLINE);
		}
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_VPSS);
	return OK;
}
#endif