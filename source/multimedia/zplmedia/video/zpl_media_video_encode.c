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

static int zpl_media_video_encode_read_thread(struct thread *encode);

int zpl_media_video_encode_request_IDR(zpl_media_video_encode_t *encode)
{
	int ret = -1;
	zpl_video_assert(encode);
	zpl_video_assert(encode->media_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	ret = zpl_vidhal_venc_request_IDR(encode);
	if (ZPL_MEDIA_DEBUG(ENCODE, EVENT))
	{
		zm_msg_debug("request IDR encode channel(%d) ret=%d", encode->venc_channel, ret);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	return ret;
}

int zpl_media_video_encode_enable_IDR(zpl_media_video_encode_t *encode, zpl_bool bEnableIDR)
{
	int ret = -1;
	zpl_video_assert(encode);
	zpl_video_assert(encode->media_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	ret = zpl_vidhal_venc_enable_IDR(encode, bEnableIDR);
	if (ZPL_MEDIA_DEBUG(ENCODE, EVENT))
	{
		zm_msg_debug("enable IDR encode channel(%d) ret=%d", encode->venc_channel, ret);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	return ret;
}

int zpl_media_video_encode_encode_reset(zpl_media_video_encode_t *encode)
{
	int ret = -1;
	zpl_video_assert(encode);
	zpl_video_assert(encode->media_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	if (encode && encode->t_read)
	{
		thread_cancel(encode->t_read);
		encode->t_read = NULL;
	}
	ret = zpl_vidhal_venc_reset(encode);
	if (ret != OK)
	{
		zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
		return ret;
	}
	if (ZPL_MEDIA_DEBUG(ENCODE, EVENT))
	{
		zm_msg_debug("reset IDR encode channel(%d) ret=%d", encode->venc_channel, ret);
	}
	zpl_vidhal_venc_update_fd(encode);
	if (encode->t_master && encode && encode->t_read == NULL && !ipstack_invalid(encode->vencfd))
	{
		if(encode->get_encode_frame == NULL)
			encode->get_encode_frame = zpl_vidhal_venc_frame_recvfrom;
		encode->t_read = thread_add_read(encode->t_master, zpl_media_video_encode_read_thread, encode, encode->vencfd);
	}
	zpl_vidhal_venc_request_IDR(encode);
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	return ret;
}

int zpl_media_video_encode_start(zpl_void *master, zpl_media_video_encode_t *encode)
{
	int ret = ERROR;
	zpl_video_assert(encode);
	zpl_video_assert(encode->media_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	if (encode->t_master == NULL)
		encode->t_master = master;
	if (!ZPL_TST_BIT(encode->flags, ZPL_MEDIA_STATE_START))
	{
		ret = zpl_vidhal_venc_start(encode);
		if (ret != OK)
		{
			zm_msg_error("start encode channel(%d) failed", encode->venc_channel);
			zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
			return ret;
		}
	}
	else
	{
		ret = OK;
	}
	if (ZPL_MEDIA_DEBUG(ENCODE, EVENT))
	{
		zm_msg_debug("encode channel(%d) start", encode->venc_channel);
	}
	encode->online = zpl_true;
	zpl_vidhal_venc_update_fd(encode);
	if (master && encode && encode->t_read == NULL && !ipstack_invalid(encode->vencfd))
	{
		if(encode->get_encode_frame == NULL)
			encode->get_encode_frame = zpl_vidhal_venc_frame_recvfrom;
		encode->t_read = thread_add_read(encode->t_master, zpl_media_video_encode_read_thread, encode, encode->vencfd);
	}
	ZPL_SET_BIT(encode->flags, ZPL_MEDIA_STATE_START);
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	zpl_media_video_encode_enable_IDR(encode, zpl_true);
	return ret;
}

int zpl_media_video_encode_stop(zpl_media_video_encode_t *encode)
{
	int ret = ERROR;
	zpl_video_assert(encode);
	zpl_video_assert(encode->media_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	if (encode && encode->t_read)
	{
		thread_cancel(encode->t_read);
		encode->t_read = NULL;
	}
	if (ZPL_TST_BIT(encode->flags, ZPL_MEDIA_STATE_START))
	{
		ret = zpl_vidhal_venc_stop(encode);
		if (ret != OK)
		{
			zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
			zm_msg_error("stop encode channel(%d) failed.", encode->venc_channel);
			return ERROR;
		}
	}
	else
		ret = OK;
	if (ZPL_MEDIA_DEBUG(ENCODE, EVENT))
	{
		zm_msg_debug("encode channel(%d) is stop", encode->venc_channel);
	}
	encode->online = zpl_false;
	ZPL_CLR_BIT(encode->flags, ZPL_MEDIA_STATE_START);
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	return ret;
}

zpl_media_video_encode_t *zpl_media_video_encode_create(zpl_int32 venc_channel, zpl_bool capture)
{
	zpl_media_video_encode_t *chn = NULL;
	if (zpl_media_video_encode_lookup(venc_channel, capture) != NULL)
	{
		return NULL;
	}
	chn = os_malloc(sizeof(zpl_media_video_encode_t));
	if (chn)
	{
		memset(chn, 0, sizeof(zpl_media_video_encode_t));
		chn->venc_channel = venc_channel;
		chn->b_capture = capture;
		chn->vencfd = ipstack_create(IPSTACK_OS);
		zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);

		zm_msg_debug("venc channel (%d) ", chn->venc_channel);

		zpl_media_global_add(ZPL_MEDIA_GLOAL_VIDEO_ENCODE, chn);
		zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
		return chn;
	}
	return NULL;
}

int zpl_media_video_encode_hal_create(zpl_media_video_encode_t *encode)
{
	int ret = ERROR;
	zpl_media_video_encode_t *chn = encode;
	if (chn && chn->pCodec && !ZPL_TST_BIT(encode->flags, ZPL_MEDIA_STATE_ACTIVE))
	{
		zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);

		ret = zpl_vidhal_venc_create(chn);
		if (ret != OK)
		{
			zm_msg_error("Create encode channel(%d) failed.", chn->venc_channel);
			zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
			return ret;
		}
		if(encode->get_encode_frame == NULL)
			encode->get_encode_frame = zpl_vidhal_venc_frame_recvfrom;
		ZPL_SET_BIT(chn->flags, ZPL_MEDIA_STATE_ACTIVE);

		if (ZPL_MEDIA_DEBUG(ENCODE, EVENT))
		{
			zm_msg_debug("active encode channel(%d) ret=%d", chn->venc_channel, ret);
		}
		zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
		return ret;
	}
	return ret;
}

int zpl_media_video_encode_destroy(zpl_media_video_encode_t *chn)
{
	int ret = -1;
	zpl_media_video_vpsschn_t *vpsschn = NULL;
	zpl_video_assert(chn);
	zpl_video_assert(chn->media_channel);
	zpl_media_video_encode_hal_destroy(chn);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
		
	vpsschn = chn->source_input;
	if(vpsschn)
		vpsschn->dest_output = NULL;
	if((chn->vencfd != ZPL_SOCKET_INVALID))
	{
		ipstack_drstroy(chn->vencfd);
		chn->vencfd = ZPL_SOCKET_INVALID;
	}
	zpl_media_global_del(ZPL_MEDIA_GLOAL_VIDEO_ENCODE, chn);
	free(chn);
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	return ret;
}
int zpl_media_video_encode_hal_destroy(zpl_media_video_encode_t *chn)
{
	int ret = -1;
	
	zpl_video_assert(chn);
	zpl_video_assert(chn->media_channel);
	
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	if (chn && chn->t_read)
	{
		thread_cancel(chn->t_read);
		chn->t_read = NULL;
	}
	if (ZPL_TST_BIT(chn->flags, ZPL_MEDIA_STATE_START))
	{
		ret = zpl_vidhal_venc_stop(chn);
		if (ret != OK)
		{
			zm_msg_error("stop encode channel(%d) failed.", chn->venc_channel);
			zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
			return ret;
		}
		chn->online = zpl_false;
		if (ZPL_MEDIA_DEBUG(ENCODE, EVENT))
		{
			zm_msg_debug("stop encode channel(%d) ret=%d", chn->venc_channel, ret);
		}
		ZPL_CLR_BIT(chn->flags, ZPL_MEDIA_STATE_START);
	}
	if (ZPL_TST_BIT(chn->flags, ZPL_MEDIA_STATE_HWBIND))
	{
		zpl_media_video_vpsschn_t *vpsschn = NULL;
		vpsschn = chn->source_input;
		if(vpsschn)
		{
			if (zpl_syshal_vpss_unbind_venc(vpsschn->vpss_group, vpsschn->vpss_channel, chn->venc_channel) != OK)
			{
				zm_msg_error("vpssgrp %d vpsschn %d can not unbind to encode channel(%d)", vpsschn->vpss_group, vpsschn->vpss_channel, chn->venc_channel);
				zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
				return ERROR;
			}
			if (ZPL_MEDIA_DEBUG(ENCODE, EVENT))
			{
				zm_msg_debug("vpssgrp %d vpsschn %d unbind to encode channel(%d)", vpsschn->vpss_group, vpsschn->vpss_channel, chn->venc_channel);
			}
			ZPL_CLR_BIT(chn->flags, ZPL_MEDIA_STATE_HWBIND);
			vpsschn->dest_output = NULL;
		}
	}
	if (ZPL_TST_BIT(chn->flags, ZPL_MEDIA_STATE_ACTIVE))
	{
		ret = zpl_vidhal_venc_destroy(chn);
		if (ret != OK)
		{
			zm_msg_error("destroy encode channel(%d) failed.", chn->venc_channel);
			zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
			return ret;
		}
		ZPL_CLR_BIT(chn->flags, ZPL_MEDIA_STATE_ACTIVE);
		if (ZPL_MEDIA_DEBUG(ENCODE, EVENT))
		{
			zm_msg_debug("inactive encode channel(%d) ret=%d", chn->venc_channel, ret);
		}
	}
	else
	{
		ret = OK;
		if (ZPL_MEDIA_DEBUG(ENCODE, EVENT))
		{
			zm_msg_debug("encode channel(%d) is already inactive", chn->venc_channel);
		}
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	return ret;
}

zpl_bool zpl_media_video_encode_state_check(zpl_media_video_encode_t *encode, int bit)
{
	zpl_bool ret = zpl_false;
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	if (encode)
	{
		if (ZPL_TST_BIT(encode->flags, bit))
			ret = zpl_true;
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	return ret;
}

int zpl_media_video_encode_thread(zpl_media_video_encode_t *encode, zpl_bool start)
{
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	if (start == zpl_false && encode && encode->t_read)
	{
		thread_cancel(encode->t_read);
		encode->t_read = NULL;
		zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
		return OK;
	}
	zpl_vidhal_vpss_channel_update_fd(encode);
	if (start && encode && !ipstack_invalid(encode->vencfd))
	{
		encode->t_read = thread_add_read(encode->t_master, zpl_media_video_encode_read_thread, encode, encode->vencfd);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	return OK;
}

static int zpl_media_video_encode_cmp(zpl_media_video_encode_t *encode, zpl_media_gkey_t *gkey)
{
	if (encode->venc_channel == gkey->channel && encode->b_capture == gkey->group)
		return 0;
	return 1;
}
static void zpl_media_video_encode_free(void *pVoid)
{
	zpl_media_video_encode_t *encode = pVoid;
	if(encode)
	{
		zpl_media_video_encode_hal_destroy(encode);
		if((encode->vencfd != ZPL_SOCKET_INVALID))
		{
			ipstack_drstroy(encode->vencfd);
			encode->vencfd = ZPL_SOCKET_INVALID;
		}
		os_free(encode);
	}
	return ;
}
int zpl_media_video_encode_init(void)
{
	zpl_media_global_gkey_cmpset(ZPL_MEDIA_GLOAL_VIDEO_ENCODE, zpl_media_video_encode_cmp);
	zpl_media_global_freeset(ZPL_MEDIA_GLOAL_VIDEO_ENCODE, zpl_media_video_encode_free);
	return OK;
}

zpl_media_video_encode_t *zpl_media_video_encode_lookup(zpl_int32 venc_channel, zpl_bool capture)
{
	zpl_media_video_encode_t *encode = (zpl_media_video_encode_t *)zpl_media_global_lookup(ZPL_MEDIA_GLOAL_VIDEO_ENCODE, venc_channel, capture, 0);
	return encode;
}



int zpl_media_video_encode_source_set(zpl_int32 venc_channel, void *halparam)
{
	int ret = ERROR;
	zpl_media_video_encode_t *encode = (zpl_media_video_encode_t *)zpl_media_global_lookup(ZPL_MEDIA_GLOAL_VIDEO_ENCODE, venc_channel, 0, 0);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	if (encode)
	{
		zpl_media_video_vpsschn_t *vpsschn = halparam;
		if(vpsschn)
		{
			zpl_media_video_vpsschn_addref(vpsschn);
			vpsschn->dest_output = encode;
			encode->source_input = halparam;
		}
		else
		{
			if(encode->source_input)
			{
				zpl_media_video_vpsschn_delref(encode->source_input);
				if (encode->t_read)
				{
					thread_cancel(encode->t_read);
					encode->t_read = NULL;
				}
				vpsschn->dest_output = NULL;
				encode->source_input = NULL;
				ret = OK;
			}
		}
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	return ret;
}

int zpl_media_video_encode_frame_queue_set(zpl_int32 venc_channel, void *frame_queue)
{
	zpl_media_video_encode_t *encode = (zpl_media_video_encode_t *)zpl_media_global_lookup(ZPL_MEDIA_GLOAL_VIDEO_ENCODE, venc_channel, 0, 0);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	if (encode)
	{
		encode->frame_queue = frame_queue;
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	return OK;
}

/* 编码模块接收到数据后完成编码，编码输出数据有这里接收，接收到编码后的数据发送到通道的消息队列，*/
static int zpl_media_video_encode_read_thread(struct thread *thread)
{
	int ret = 0;
	zpl_media_video_encode_t *encode = THREAD_ARG(thread);
	ZPL_MEDIA_CHANNEL_E channel;			// 通道号
	ZPL_MEDIA_CHANNEL_TYPE_E channel_index; // 码流类型
	if (encode && encode->media_channel && encode->get_encode_frame)
	{
		zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
		encode->t_read = NULL;
		channel = ((zpl_media_channel_t *)encode->media_channel)->channel;
		channel_index = ((zpl_media_channel_t *)encode->media_channel)->channel_index;

		ret = (encode->get_encode_frame)(encode);

		if (encode->frame_queue && ret > 0)
		{
			zpl_media_bufqueue_signal(channel, channel_index);
		}
		if (!ipstack_invalid(encode->vencfd))
			encode->t_read = thread_add_read(encode->t_master, zpl_media_video_encode_read_thread, encode, encode->vencfd);

		zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	}
	return OK;
}

int zpl_media_video_encode_sendto(zpl_media_video_encode_t *encode, void *p, zpl_int timeout)
{
	int ret = -1;
	zpl_video_assert(encode);
	zpl_video_assert(encode->media_channel);
	zpl_media_global_lock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	if (encode->online)
	{
		ret = zpl_vidhal_venc_frame_sendto(encode, 0, encode->venc_channel, p, timeout);
		//zm_msg_error("stop encode channel(%d) failed.", chn->venc_channel);
	}
	zpl_media_global_unlock(ZPL_MEDIA_GLOAL_VIDEO_ENCODE);
	return ret;
}

#ifdef ZPL_SHELL_MODULE
int zpl_media_video_encode_show(void *pvoid)
{
	NODE *node;
	zpl_media_video_encode_t *encode;
	struct vty *vty = (struct vty *)pvoid;
	LIST *_lst = NULL;
	os_mutex_t *_mutex = NULL;
	zpl_media_video_vpsschn_t	*vpsschn;
	char *bitrate_typestr[] = {"none", "CBR", "VBR", "ABR"};
	char *enRcMode_typestr[] = {"RC CBR", "RC VBR", "RC AVBR", "RC QVBR", "RC CVBR", "RC QPMAP","RC FIXQP"};
	char *gopmode_typestr[] = {"NORMALP", "DUALP", "SMARTP","ADVSMARTP", "BIPREDB", "LOWDELAYB","BUTT"};
	zpl_media_global_get(ZPL_MEDIA_GLOAL_VIDEO_ENCODE, &_lst, &_mutex);

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
		encode = (zpl_media_video_encode_t *)node;
		if (node)
		{
			vty_out(vty, "channel            : %d%s", encode->venc_channel, VTY_NEWLINE);
			if(encode->pCodec)
			{
				vty_out(vty, " size              : %dx%d%s", encode->pCodec->vidsize.width, encode->pCodec->vidsize.height, VTY_NEWLINE);
				vty_out(vty, " format            : %s%s", zpl_media_format_name(encode->pCodec->format), VTY_NEWLINE);
				vty_out(vty, " codectype         : %s%s", zpl_media_codec_name(encode->pCodec->codectype), VTY_NEWLINE);
				vty_out(vty, " framerate         : %d fps%s", encode->pCodec->framerate, VTY_NEWLINE);
				vty_out(vty, " bitrate           : %d%s", encode->pCodec->bitrate, VTY_NEWLINE);
				vty_out(vty, " bitrate_type      : %s%s", bitrate_typestr[encode->pCodec->bitrate_type], VTY_NEWLINE);
				vty_out(vty, " profile           : %d%s", encode->pCodec->profile, VTY_NEWLINE);
				vty_out(vty, " ikey_rate         : %d%s", encode->pCodec->ikey_rate, VTY_NEWLINE);
				vty_out(vty, " enRcMode          : %s%s", enRcMode_typestr[encode->pCodec->enRcMode], VTY_NEWLINE);
				vty_out(vty, " gopmode           : %s%s", gopmode_typestr[encode->pCodec->gopmode], VTY_NEWLINE);
				vty_out(vty, " packetization_mode: %d%s", encode->pCodec->packetization_mode, VTY_NEWLINE);
			}
			if(encode->vencfd != ZPL_SOCKET_INVALID)
			{
				vty_out(vty, " fd                : %d%s", ipstack_fd(encode->vencfd), VTY_NEWLINE);
			}
			vty_out(vty, " flags             : 0x%x%s", encode->flags, VTY_NEWLINE);
			//vty_out(vty, " res_flag          : 0x%x%s", encode->res_flag, VTY_NEWLINE);
			vty_out(vty, " online            : %s%s", encode->online ? "ONLINE" : "OFFLINE", VTY_NEWLINE);
			vty_out(vty, " master            : %p%s", encode->t_master, VTY_NEWLINE);
			vpsschn = (zpl_media_video_vpsschn_t *)encode->source_input;
			if(vpsschn)
			vty_out(vty, " source_input      : vpss group %d vpss channal %d%s", vpsschn ? vpsschn->vpss_group:-1, vpsschn ? vpsschn->vpss_channel : -1, VTY_NEWLINE);
		}
	}
	if (_mutex)
		os_mutex_unlock(_mutex);
	return OK;
}
#endif
