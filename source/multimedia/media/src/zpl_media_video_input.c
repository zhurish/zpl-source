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

static int zpl_media_video_inputchn_read(struct thread *t_thread);

static int zpl_media_video_inputchn_cmp(zpl_media_video_inputchn_t *inputchn, zpl_media_gkey_t *gkey)
{
	if (inputchn->input_chn == gkey->channel && 
		inputchn->input_pipe == gkey->group && 
		inputchn->devnum == gkey->ID)
		return 0;
	return 1;
}

static void zpl_media_video_inputchn_free(void *pVoid)
{
	zpl_media_video_inputchn_t *inputchn = pVoid;
	if(inputchn)
	{
		zpl_media_video_inputchn_hal_destroy(inputchn);
		if((inputchn->pipefd != ZPL_SOCKET_INVALID))
		{
			ipstack_drstroy(inputchn->pipefd);
			inputchn->pipefd = ZPL_SOCKET_INVALID;
		}
		if((inputchn->chnfd != ZPL_SOCKET_INVALID))
		{
			ipstack_drstroy(inputchn->chnfd);
			inputchn->chnfd = ZPL_SOCKET_INVALID;
		}
		os_free(inputchn);
	}
	return ;
}
int zpl_media_video_inputchn_init(void)
{
	zpl_media_global_gkey_cmpset(ZPL_MEDIA_GLOAL_VIDEO_INPUT, zpl_media_video_inputchn_cmp);
	zpl_media_global_freeset(ZPL_MEDIA_GLOAL_VIDEO_INPUT, zpl_media_video_inputchn_free);
	return OK;
}

int zpl_media_video_input_pipe_lookup(zpl_int32 input_pipe)
{
	int count = 0;
	NODE node;
	zpl_media_video_inputchn_t *inputchn = NULL;
	LIST *_lst = NULL;
	os_mutex_t *_mutex = NULL;
	zpl_media_global_get(ZPL_MEDIA_GLOAL_VIDEO_INPUT, &_lst, &_mutex);
	if (_lst == NULL)
		return 0;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);

	for (inputchn = (zpl_media_video_inputchn_t *)lstFirst(_lst);
		 inputchn != NULL; inputchn = (zpl_media_video_inputchn_t *)lstNext(&node))
	{
		node = inputchn->node;
		if (inputchn && inputchn->input_pipe == input_pipe)
		{
			count++;
		}
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	return count;
}

static int zpl_media_video_input_pipe_bind_count_update(zpl_int32 input_pipe)
{
	int count = 0;
	NODE node;
	zpl_media_video_inputchn_t *inputchn = NULL;
	LIST *_lst = NULL;
	os_mutex_t *_mutex = NULL;
	zpl_media_global_get(ZPL_MEDIA_GLOAL_VIDEO_INPUT, &_lst, &_mutex);
	if (_lst == NULL)
		return 0;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	for (inputchn = (zpl_media_video_inputchn_t *)lstFirst(_lst);
		 inputchn != NULL; inputchn = (zpl_media_video_inputchn_t *)lstNext(&node))
	{
		node = inputchn->node;
		if (inputchn && inputchn->input_pipe == input_pipe)
		{
			count++;
		}
	}
	for (inputchn = (zpl_media_video_inputchn_t *)lstFirst(_lst);
		 inputchn != NULL; inputchn = (zpl_media_video_inputchn_t *)lstNext(&node))
	{
		node = inputchn->node;
		if (inputchn && inputchn->input_pipe == input_pipe)
		{
			inputchn->input_pipe_bind_chn = count;
		}
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	return count;
}

zpl_media_video_inputchn_t *zpl_media_video_inputchn_create(zpl_int32 devnum, zpl_int32 input_pipe, zpl_int32 input_channel, zpl_video_size_t output_size)
{
	zpl_media_video_inputchn_t *inputchn = NULL;
	if (zpl_media_video_inputchn_lookup(devnum, input_pipe, input_channel) != NULL)
	{
		return NULL;
	}
	inputchn = os_malloc(sizeof(zpl_media_video_inputchn_t));
	zpl_video_assert(inputchn);
	if (inputchn)
	{
		zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
		memset(inputchn, 0, sizeof(zpl_media_video_inputchn_t));
		inputchn->pipefd = ipstack_create(IPSTACK_OS);
		inputchn->chnfd = ipstack_create(IPSTACK_OS);

		inputchn->input_chn = input_channel; // 底层通道号
		inputchn->input_pipe = input_pipe;
		inputchn->inputdev.snsdev = ZPL_INVALID_VAL;
		inputchn->inputdev.mipidev = ZPL_INVALID_VAL;
		inputchn->inputdev.snstype = ZPL_INVALID_VAL;
		inputchn->devnum = devnum;
		inputchn->output_size.width = output_size.width;   // 宽度
		inputchn->output_size.height = output_size.height; // 高度
		zm_msg_debug("input_channel(%d) ", inputchn->input_chn);

		zpl_media_global_add(ZPL_MEDIA_GLOAL_VIDEO_INPUT, inputchn);
		zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
		zpl_media_video_input_pipe_bind_count_update(input_pipe);
		return inputchn;
	}
	return NULL;
}

int zpl_media_video_inputchn_hal_param(zpl_media_video_inputchn_t *inputchn, int snstype, int mipidev, int snsdev)
{
	int ret = ERROR;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	if (inputchn && inputchn->inputdev.snstype == ZPL_INVALID_VAL)
	{
		inputchn->inputdev.snsdev = snsdev;
		inputchn->inputdev.mipidev = mipidev;
		inputchn->inputdev.snstype = snstype;
		ret = OK;
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	return ret;
}

int zpl_media_video_inputchn_outputsize_get(zpl_int32 devnum, zpl_int32 input_pipe, zpl_int32 input_channel, zpl_video_size_t *output_size)
{
	zpl_media_video_inputchn_t *inputchn = zpl_media_video_inputchn_lookup(devnum, input_pipe, input_channel);
	if (inputchn != NULL)
	{
		output_size->width = inputchn->output_size.width;   // 宽度
		output_size->height = inputchn->output_size.height; // 高度
		return OK;
	}
	return ERROR;
}

int zpl_media_video_inputchn_hal_create(zpl_media_video_inputchn_t *inputchn)
{
	int ret = 0;
	zpl_video_assert(inputchn);
	if (inputchn && !ZPL_TST_BIT(inputchn->flags, ZPL_MEDIA_STATE_ACTIVE))
	{
		zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);

		zm_msg_debug("input_channel(%d)", inputchn->input_chn);

		ret = zpl_vidhal_inputchn_create(inputchn->input_pipe, inputchn->input_chn, inputchn);
		if (ret != OK)
		{
			zm_msg_error("create input channel pipe %d channel %d failed.", inputchn->input_pipe, inputchn->input_chn);
			zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
			return ret;
		}
		ZPL_SET_BIT(inputchn->flags, ZPL_MEDIA_STATE_ACTIVE);
	
		if (ZPL_MEDIA_DEBUG(INPUT, EVENT))
		{
			zm_msg_debug("active inputchn channel(%d) ret=%d", inputchn->input_pipe, ret);
		}

		zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
		zpl_media_video_input_pipe_bind_count_update(inputchn->input_pipe);
		return ret;
	}
	return ret;
}

int zpl_media_video_inputchn_hal_destroy(zpl_media_video_inputchn_t *inputchn)
{
	int ret = 0, input_pipe = 0;
	if (inputchn)
	{
		zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
		input_pipe = inputchn->input_pipe;
		if (inputchn && inputchn->t_read)
		{
			thread_cancel(inputchn->t_read);
			inputchn->t_read = NULL;
		}
		if (ZPL_TST_BIT(inputchn->flags, ZPL_MEDIA_STATE_START))
		{
			ret = zpl_vidhal_inputchn_stop(inputchn->input_pipe, inputchn->input_chn, inputchn);
			if (ret != OK)
			{
				zm_msg_error("stop input channel pipe %d channel %d failed.", inputchn->input_pipe, inputchn->input_chn);
				zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
				return ret;
			}
	
			ZPL_CLR_BIT(inputchn->flags, ZPL_MEDIA_STATE_START);
			if (ZPL_MEDIA_DEBUG(INPUT, EVENT))
			{
				zm_msg_debug("stop inputchn channel(%d) ret=%d", inputchn->input_chn, ret);
			}
		}
		if (ZPL_TST_BIT(inputchn->flags, ZPL_MEDIA_STATE_ACTIVE))
		{
			ret = zpl_vidhal_inputchn_destroy(inputchn->input_pipe, inputchn->input_chn, inputchn);
			if (ret != OK)
			{
				zm_msg_error("destroy input channel pipe %d channel %d failed.", inputchn->input_pipe, inputchn->input_chn);
				zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
				return ret;
			}

			ZPL_CLR_BIT(inputchn->flags, ZPL_MEDIA_STATE_ACTIVE);

			if (ZPL_MEDIA_DEBUG(INPUT, EVENT))
			{
				zm_msg_debug("inactive inputchn channel(%d) ret=%d", inputchn->input_chn, ret);
			}
		}
		zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
		zpl_media_video_input_pipe_bind_count_update(input_pipe);
	}
	return ret;
}

int zpl_media_video_inputchn_destroy(zpl_media_video_inputchn_t *inputchn)
{
	int ret = 0, input_pipe = 0;
	if (inputchn)
	{
		zpl_media_video_inputchn_hal_destroy(inputchn);

		zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
		input_pipe = inputchn->input_pipe;
		
		if((inputchn->pipefd != ZPL_SOCKET_INVALID))
		{
			ipstack_drstroy(inputchn->pipefd);
			inputchn->pipefd = ZPL_SOCKET_INVALID;
		}
		if((inputchn->chnfd != ZPL_SOCKET_INVALID))
		{
			ipstack_drstroy(inputchn->chnfd);
			inputchn->chnfd = ZPL_SOCKET_INVALID;
		}
		zpl_media_global_del(ZPL_MEDIA_GLOAL_VIDEO_INPUT, inputchn);
		os_free(inputchn);
		zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
		zpl_media_video_input_pipe_bind_count_update(input_pipe);
	}
	return ret;
}

int zpl_media_video_inputchn_connect(zpl_media_video_inputchn_t *inputchn, zpl_int32 vpss_group, zpl_int32 vpss_channel, zpl_bool hwbind)
{
	int ret = ERROR;
	zpl_media_video_vpsschn_t *vpsschn = (zpl_media_video_vpsschn_t *)zpl_media_global_lookup(ZPL_MEDIA_GLOAL_VIDEO_VPSS, vpss_channel, vpss_group, 0);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	if (vpsschn && inputchn->dest_output == vpsschn)
	{
		int vpssgrp = vpss_group;
		if(inputchn)
		{
			if (hwbind && !ZPL_TST_BIT(inputchn->flags, ZPL_MEDIA_STATE_HWBIND))
			{
				if (inputchn->t_read)
				{
					thread_cancel(inputchn->t_read);
					inputchn->t_read = NULL;
				}
				if (zpl_syshal_input_bind_vpss(inputchn->input_pipe, inputchn->input_chn, vpssgrp) != OK)
				{
					zm_msg_error("input_pipe %d input_chn %d can not connect to vpssgrp (%d)", inputchn->input_pipe, inputchn->input_chn, vpssgrp);
					zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
					return ERROR;
				}
				if (ZPL_MEDIA_DEBUG(INPUT, EVENT))
				{
					zm_msg_debug("input_pipe %d input_chn %d connect to vpssgrp (%d)", inputchn->input_pipe, inputchn->input_chn, vpssgrp);
				}
				ZPL_SET_BIT(inputchn->flags, ZPL_MEDIA_STATE_HWBIND);
				inputchn->hwbind = zpl_true;
				ret = OK;
			}
			else if (hwbind == zpl_false && ZPL_TST_BIT(inputchn->flags, ZPL_MEDIA_STATE_HWBIND))
			{
				if (inputchn->t_read)
				{
					thread_cancel(inputchn->t_read);
					inputchn->t_read = NULL;
				}
				if (zpl_syshal_input_unbind_vpss(inputchn->input_pipe, inputchn->input_chn, vpssgrp) != OK)
				{
					zm_msg_error("input_pipe %d input_chn %d can not unbind to vpssgrp (%d)", inputchn->input_pipe, inputchn->input_chn, vpssgrp);
					zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
					return ERROR;
				}
				if (ZPL_MEDIA_DEBUG(INPUT, EVENT))
				{
					zm_msg_debug("input_pipe %d input_chn %d unbind to vpssgrp (%d)", inputchn->input_pipe, inputchn->input_chn, vpssgrp);
				}
				inputchn->hwbind = zpl_false;
				ZPL_CLR_BIT(inputchn->flags, ZPL_MEDIA_STATE_HWBIND);
				zpl_vidhal_inputchn_update_fd(inputchn->input_pipe, inputchn->input_chn, inputchn);
				if (!ipstack_invalid(inputchn->chnfd) && inputchn->t_master)
				{
					if (ZPL_TST_BIT(inputchn->flags, ZPL_MEDIA_STATE_START))
					inputchn->t_read = thread_add_read(inputchn->t_master, zpl_media_video_inputchn_read, inputchn, inputchn->chnfd);
				}

				ret = OK;
			}
			else
			{
				ret = OK;
			}
		}
		else
			zm_msg_error("can not get source stream of input pipe %d input channel %d on vpss group(%d)", inputchn->input_pipe, inputchn->input_chn, vpssgrp);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	return ret;
}

zpl_bool zpl_media_video_inputchn_state_check(zpl_media_video_inputchn_t *inputchn, int bit)
{
	zpl_bool ret = zpl_false;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	if (inputchn)
	{
		if (ZPL_TST_BIT(inputchn->flags, bit))
			ret = zpl_true;
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	return ret;
}

int zpl_media_video_inputchn_start(zpl_void *master, zpl_media_video_inputchn_t *inputchn)
{
	int ret = -1;
	zpl_video_assert(inputchn);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	if (!ZPL_TST_BIT(inputchn->flags, ZPL_MEDIA_STATE_START))
	{
		ret = zpl_vidhal_inputchn_start(inputchn->input_pipe, inputchn->input_chn, inputchn);
		if (ret != OK)
		{
			zm_msg_error("start input channel pipe %d channel %d failed.", inputchn->input_pipe, inputchn->input_chn);
			zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
			return ret;
		}
		ZPL_SET_BIT(inputchn->flags, ZPL_MEDIA_STATE_START);

		if (ZPL_MEDIA_DEBUG(INPUT, EVENT))
		{
			zm_msg_debug("start inputchn channel(%d) ret=%d", inputchn->input_chn, ret);
		}
	}
	else
	{
		ret = OK;
		if (ZPL_MEDIA_DEBUG(INPUT, EVENT))
		{
			zm_msg_debug("inputchn channel(%d) is already start", inputchn->input_chn);
		}
	}
	zpl_vidhal_inputchn_update_fd(inputchn->input_pipe, inputchn->input_chn, inputchn);
	if (inputchn->hwbind == zpl_false && master && inputchn && !ipstack_invalid(inputchn->chnfd))
	{
		inputchn->t_master = master;
		inputchn->t_read = thread_add_read(master, zpl_media_video_inputchn_read, inputchn, inputchn->chnfd);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	return ret;
}

int zpl_media_video_inputchn_stop(zpl_media_video_inputchn_t *inputchn)
{
	int ret = -1;
	zpl_video_assert(inputchn);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	if (inputchn && inputchn->t_read)
	{
		thread_cancel(inputchn->t_read);
		inputchn->t_read = NULL;
	}
	if (ZPL_TST_BIT(inputchn->flags, ZPL_MEDIA_STATE_START))
	{
		ret = zpl_vidhal_inputchn_stop(inputchn->input_pipe, inputchn->input_chn, inputchn);
		if (ret != OK)
		{
			zm_msg_error("stop input channel pipe %d channel %d failed.", inputchn->input_pipe, inputchn->input_chn);
			zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
			return ret;
		}
		ZPL_CLR_BIT(inputchn->flags, ZPL_MEDIA_STATE_START);
		if (ZPL_MEDIA_DEBUG(INPUT, EVENT))
		{
			zm_msg_debug("stop inputchn channel(%d) ret=%d", inputchn->input_chn, ret);
		}
	}
	else
	{
		ret = OK;
		if (ZPL_MEDIA_DEBUG(INPUT, EVENT))
		{
			zm_msg_debug("inputchn channel(%d) is already stop", inputchn->input_chn);
		}
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	return ret;
}

int zpl_media_video_inputchn_thread(zpl_media_video_inputchn_t *inputchn, zpl_bool start)
{
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	if (start == zpl_false && inputchn && inputchn->t_read)
	{
		thread_cancel(inputchn->t_read);
		inputchn->t_read = NULL;
		zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
		return OK;
	}
	zpl_vidhal_inputchn_update_fd(inputchn->input_pipe, inputchn->input_chn, inputchn);
	if (start && inputchn && !ipstack_invalid(inputchn->chnfd))
	{
		inputchn->t_read = thread_add_read(inputchn->t_master, zpl_media_video_inputchn_read, inputchn, inputchn->chnfd);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	return OK;
}

zpl_media_video_inputchn_t *zpl_media_video_inputchn_lookup(zpl_int32 devnum, zpl_int32 input_pipe, zpl_int32 input_channel)
{
	zpl_media_video_inputchn_t *inputchn = (zpl_media_video_inputchn_t *)zpl_media_global_lookup(ZPL_MEDIA_GLOAL_VIDEO_INPUT, input_channel, input_pipe, devnum);
	return inputchn;
}

static int zpl_media_video_inputchn_read(struct thread *t_thread)
{
	zpl_media_video_inputchn_t *inputchn = THREAD_ARG(t_thread);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	if (inputchn)
	{
		inputchn->t_read = NULL;
		zpl_vidhal_inputchn_frame_recvfrom(inputchn->input_pipe, inputchn->input_chn, inputchn);

		if (!ipstack_invalid(inputchn->chnfd))
			inputchn->t_read = thread_add_read(inputchn->t_master, zpl_media_video_inputchn_read, inputchn, inputchn->chnfd);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	return OK;
}

int zpl_media_video_inputchn_sendto(zpl_media_video_inputchn_t *inputchn, void *p, zpl_int timeout)
{
	int ret = -1;
	zpl_video_assert(inputchn);

	return ret;
}

static int zpl_media_video_inputchn_reference_set(zpl_media_video_inputchn_t *inputchn, zpl_bool add)
{
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	if (inputchn)
	{
		if (add)
			inputchn->reference++;
		else
			inputchn->reference++;
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	return OK;
}
int zpl_media_video_inputchn_addref(zpl_media_video_inputchn_t *inputchn)
{
	return zpl_media_video_inputchn_reference_set(inputchn, zpl_true);
}
int zpl_media_video_inputchn_delref(zpl_media_video_inputchn_t *inputchn)
{
	return zpl_media_video_inputchn_reference_set(inputchn, zpl_false);
}

int zpl_media_video_inputchn_reference(zpl_media_video_inputchn_t *inputchn)
{
	int reference = 0;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	if (inputchn)
	{
		reference = inputchn->reference;
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	return reference;
}

#ifdef ZPL_SHELL_MODULE
int zpl_media_video_inputchn_show(void *pvoid)
{
	NODE *node;
	zpl_media_video_inputchn_t *inputchn;
	struct vty *vty = (struct vty *)pvoid;
	LIST *_lst = NULL;
	os_mutex_t *_mutex = NULL;
	//zpl_media_video_dev_t *inputdev;
	zpl_media_global_get(ZPL_MEDIA_GLOAL_VIDEO_INPUT, &_lst, &_mutex);
	if (_lst == NULL)
		return ERROR;
	if (_mutex)
		os_mutex_lock(_mutex, OS_WAIT_FOREVER);

	if (lstCount(_lst))
	{
		vty_out(vty, "-----------------------------------------%s", VTY_NEWLINE);
	}
	for (node = lstFirst(_lst); node != NULL; node = lstNext(node))
	{
		inputchn = (zpl_media_video_inputchn_t *)node;
		if (node)
		{
			vty_out(vty, "pipe                : %d%s", inputchn->input_pipe, VTY_NEWLINE);
			vty_out(vty, " channel            : %d%s", inputchn->input_chn, VTY_NEWLINE);
			vty_out(vty, " extchannel         : %d%s", inputchn->input_extchn, VTY_NEWLINE);
			vty_out(vty, "  input size        : %dx%d%s", inputchn->input_size.width, inputchn->input_size.height, VTY_NEWLINE);
			vty_out(vty, "  output size       : %dx%d%s", inputchn->output_size.width, inputchn->output_size.height, VTY_NEWLINE);
			vty_out(vty, "  reference         : %d%s", inputchn->reference, VTY_NEWLINE);
			vty_out(vty, "  grp chn count     : %d%s", inputchn->input_pipe_bind_chn, VTY_NEWLINE);
			vty_out(vty, "  hwbind            : %d%s", inputchn->hwbind, VTY_NEWLINE);
			vty_out(vty, "  flags             : 0x%x%s", inputchn->flags, VTY_NEWLINE);
			vty_out(vty, "  sns_type          : 0x%x%s", inputchn->inputdev.snstype, VTY_NEWLINE);
			vty_out(vty, "  online            : %s%s", inputchn->online ? "ONLINE" : "OFFLINE", VTY_NEWLINE);
			vty_out(vty, "  master            : %p%s", inputchn->t_master, VTY_NEWLINE);

			if(inputchn->pipefd != ZPL_SOCKET_INVALID)
			{
				vty_out(vty, "  pipe fd           : %d%s", ipstack_fd(inputchn->pipefd), VTY_NEWLINE);
			}
			if(inputchn->chnfd != ZPL_SOCKET_INVALID)
			{
				vty_out(vty, "  channel fd        : %d%s", ipstack_fd(inputchn->chnfd), VTY_NEWLINE);
			}			
		}
	}
	if (_mutex)
		os_mutex_unlock(_mutex);
	return OK;
}
#endif

int zpl_media_video_inputchn_crop(zpl_media_video_inputchn_t *inputchn, zpl_bool out, zpl_video_size_t cropsize)
{
	int ret = -1;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	if (inputchn)
	{
		ret = zpl_vidhal_inputchn_crop(inputchn->input_pipe, inputchn->input_chn, out, cropsize);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	return ret;
}

int zpl_media_video_inputchn_mirror_flip(zpl_media_video_inputchn_t *inputchn, zpl_bool mirror, zpl_bool flip)
{
	int ret = -1;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	if (inputchn)
	{
		ret = zpl_vidhal_inputchn_mirror_flip(inputchn->input_pipe, inputchn->input_chn, mirror, flip);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	return ret;
}

int zpl_media_video_inputchn_ldc(zpl_media_video_inputchn_t *inputchn, zpl_video_size_t cropsize) // 畸形矫正
{
	int ret = -1;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	if (inputchn)
	{
		ret = zpl_vidhal_inputchn_ldc(inputchn->input_pipe, inputchn->input_chn, cropsize);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	return ret;
}

int zpl_media_video_inputchn_fish_eye(zpl_media_video_inputchn_t *inputchn, void *LMF) // 鱼眼校正
{
	int ret = -1;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	if (inputchn)
	{
		ret = zpl_vidhal_inputchn_fish_eye(inputchn->input_pipe, inputchn->input_chn, LMF);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	return ret;
}

int zpl_media_video_inputchn_rotation(zpl_media_video_inputchn_t *inputchn, zpl_uint32 rotation)
{
	int ret = -1;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	if (inputchn)
	{
		ret = zpl_vidhal_inputchn_rotation(inputchn->input_pipe, inputchn->input_chn, rotation);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	return ret;
}

int zpl_media_video_inputchn_rotation_angle(zpl_media_video_inputchn_t *inputchn, void *p)
{
	int ret = -1;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	if (inputchn)
	{
		ret = zpl_vidhal_inputchn_rotation_angle(inputchn->input_pipe, inputchn->input_chn, p);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	return ret;
}

int zpl_media_video_inputchn_spread(zpl_media_video_inputchn_t *inputchn, void *p)
{
	int ret = -1;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	if (inputchn)
	{
		ret = zpl_vidhal_inputchn_spread(inputchn->input_pipe, inputchn->input_chn, p);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_INPUT);
	return ret;
}