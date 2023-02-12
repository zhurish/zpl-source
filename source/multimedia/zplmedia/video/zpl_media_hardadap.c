/*
 * zpl_media_callback.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */


#include "zpl_media.h"
#include "zpl_media_internal.h"


/* callback */
int zpl_media_hardadap_init(zpl_media_hardadap_lst_t *lst)
{
    int j = 0;
    //zpl_video_assert(callback);
    for (j = 0; j < ZPL_MEDIA_HARDADAP_MAX; j++)
    {
        lst->callback[j].hardadap_sendto = NULL;
        lst->callback[j].dst_device = ZPL_INVALID_VAL;            // 发送目的ID
        lst->callback[j].dst_channel = ZPL_INVALID_VAL;            // 发送目的通道
        lst->callback[j].src_device = ZPL_INVALID_VAL;            // 发送目的ID
        lst->callback[j].src_channel = ZPL_INVALID_VAL;            // 发送目的通道
        lst->callback[j].module = ZPL_INVALID_VAL;
    }
    return OK;
}

static int zpl_media_hardadap_lookup(zpl_media_hardadap_lst_t *lst,
                                         zpl_int32 module, zpl_int32 dst_device, zpl_int32 dst_channel)
{
    //zpl_video_assert(callback);
    int ret = ERROR, j = 0;
    for (j = 0; j < ZPL_MEDIA_HARDADAP_MAX; j++)
    {
        if (lst->callback[j].module == module && lst->callback[j].dst_device == dst_device &&
            lst->callback[j].dst_channel == dst_channel && lst->callback[j].module != ZPL_INVALID_VAL) // 发送目的通道
        {
            return OK;
        }
    }
    return ret;
}

int zpl_media_hardadap_install(zpl_media_hardadap_lst_t *lst, zpl_media_hardadap_t *cbnode)
{


    //zpl_video_assert(callback);
    //zpl_video_assert(cbnode);
    int ret = -1, j = 0;
    if (zpl_media_hardadap_lookup(lst, cbnode->module, cbnode->dst_device, cbnode->dst_channel) == OK)
    {
        printf("Channel (%d %d %d) callback is already exist\r\n", cbnode->module, cbnode->dst_device, cbnode->dst_channel);
        return ERROR;
    }
    for (j = 0; j < ZPL_MEDIA_HARDADAP_MAX; j++)
    {
        if (lst->callback[j].module == ZPL_INVALID_VAL) // 发送目的通道
        {
            lst->callback[j].hardadap_sendto = cbnode->hardadap_sendto;
            lst->callback[j].dst_device = cbnode->dst_device;            // 发送目的ID
            lst->callback[j].dst_channel = cbnode->dst_channel;            // 发送目的通道
            lst->callback[j].src_device = cbnode->src_device;            // 发送目的ID
            lst->callback[j].src_channel = cbnode->src_channel;            // 发送目的通道
            lst->callback[j].module = cbnode->module;
            lst->callback[j].online = cbnode->online;
            lst->callback[j].dst_private = cbnode->dst_private;
            return OK;
        }
    }
    return ret;
}

int zpl_media_hardadap_uninstall(zpl_media_hardadap_lst_t *lst,
                                      zpl_int32 module, zpl_int32 dst_device, zpl_int32 dst_channel)
{
    //zpl_video_assert(callback);
    int ret = -1, j = 0;
    if (zpl_media_hardadap_lookup(lst, module, dst_device, dst_channel) != OK)
    {
        printf("Channel (%d %d %d) callback is already exist\r\n", module, dst_device, dst_channel);
        return ERROR;
    }
    for (j = 0; j < ZPL_MEDIA_HARDADAP_MAX; j++)
    {
        if (lst->callback[j].module == module && lst->callback[j].dst_device == dst_device &&
            lst->callback[j].dst_channel == dst_channel && lst->callback[j].module != ZPL_INVALID_VAL) // 发送目的通道
        {
            lst->callback[j].hardadap_sendto = NULL;
            lst->callback[j].dst_device = ZPL_INVALID_VAL;            // 发送目的ID
            lst->callback[j].dst_channel = ZPL_INVALID_VAL;            // 发送目的通道
            lst->callback[j].src_device = ZPL_INVALID_VAL;            // 发送目的ID
            lst->callback[j].src_channel = ZPL_INVALID_VAL;            // 发送目的通道
            lst->callback[j].module = ZPL_INVALID_VAL;
            lst->callback[j].online = NULL;
            lst->callback[j].dst_private = NULL;
            return OK;
        }
    }
    return ret;
}

int zpl_media_hardadap_handle(zpl_media_hardadap_lst_t *lst, void *p, zpl_int timeout)
{
    zpl_uint32 j = 0;

    for (j = 0; j < ZPL_MEDIA_HARDADAP_MAX; j++)
    {
        if (lst->callback[j].hardadap_sendto != NULL)
        {
            if (lst->callback[j].dst_device >= 0 || lst->callback[j].dst_channel >= 0) // 发送目的通道
            {
                //if (callback[j].online && *callback[j].online == zpl_true)
                    (lst->callback[j].hardadap_sendto)(lst->callback[j].dst_private, lst->callback[j].dst_device, lst->callback[j].dst_channel, p, timeout);
            }
        }
    }
    return OK;
}




int zpl_media_hal_input_sendto_vpss_default(zpl_void *dst, zpl_int32 vpssgrp, zpl_int32 vpsschn,
                                            void *p, zpl_int32 timeout)
{
    if (((zpl_media_video_vpssgrp_t *)dst)->vpss_group != vpssgrp)
	{
		if(ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE) && ZPL_MEDIA_DEBUG(INPUT, SEND))
		{
			//zpl_media_debugmsg_warn(" =====================vpss_group(%d) vpssgrp(%d) ", ((zpl_media_video_vpssgrp_t *)dst)->vpss_group, vpssgrp);
		}
        return ERROR;
	}
    return zpl_media_video_vpssgrp_sendto((zpl_media_video_vpssgrp_t *)dst, p, timeout);
}

int zpl_media_hal_vpss_sendto_encode_default(zpl_void *dst, zpl_int32 vencgrp, zpl_int32 vencchn,
                                             void *p, zpl_int32 timeout)
{
    if (((zpl_media_video_encode_t *)dst)->venc_channel != vencchn)
	{
		if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE) && ZPL_MEDIA_DEBUG(VPSS, SEND))
		{
			//zpl_media_debugmsg_warn(" ====================venc_channel(%d) vencchn(%d) ", ((zpl_media_video_encode_t *)dst)->venc_channel, vencchn);
		}
        return ERROR;
	}
    return zpl_media_video_encode_sendto((zpl_media_video_encode_t *)dst, p, timeout);
}