/**
 * @file      : rtmp-client-task.c
 * @brief     : Description
 * @author    : zhurish (zhurish@163.com)
 * @version   : 1.0
 * @date      : 2024-03-15
 * 
 * @copyright : Copyright (c) - 2024 zhurish(zhurish@163.com).Co.Ltd. All rights reserved.
 * 
 */
#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "log.h"
#include "rtmpsys.h"
#include "rtmplog.h"
#include "rtmp-client-api.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"

static int zpl_mediartmp_event_dispatch_signal(zpl_media_channel_t *chm);

static int zpl_mediartmp_session_dispatch_all(zpl_media_channel_t *chm)
{
    int ret = 0;
    if (chm->rtp_param.param)
    {
        zpl_skbuffer_t *skb = zpl_skbqueue_get(chm->rtp_param.param);
        if (skb)
        {
            zpl_media_hdr_t *media_header = skb->skb_hdr.other_hdr;

            ret = zpl_mediartp_session_adap_rtp_sendto(chm, media_header->codectype,
                                                       ZPL_SKB_DATA(skb), ZPL_SKB_DATA_LEN(skb));

            zpl_skbqueue_finsh(chm->rtp_param.param, skb);
        }
    }
    return ret;
}

static int zpl_mediartmp_session_rtp_clone(zpl_media_channel_t *mediachn,
                                          const zpl_skbuffer_t *bufdata, void *pVoidUser)
{
    int ret = 0;
    zpl_skbuffer_t *skb = NULL;
    if (mediachn->rtp_param.param)
    {
        skb = zpl_skbuffer_clone(mediachn->rtp_param.param, bufdata);
        if (skb)
        {
            ret = zpl_skbqueue_add(mediachn->rtp_param.param, skb);
            zpl_mediartmp_event_dispatch_signal(mediachn);
        }
        else
        {
            zpl_mediartmp_event_dispatch_signal(mediachn);
            zm_msg_error("media channel %d/%d can not clone skb", mediachn->channel, mediachn->channel_index);
        }
    }
    return ret;
}

static int zpl_mediartp_event_handle(zpl_media_event_t *event)
{
    zpl_media_channel_t *chm = event->event_data;
    if (chm)
    {
        zpl_mediartmp_session_dispatch_all(chm);
    }
    return OK;
}

static int zpl_mediartmp_event_dispatch_signal(zpl_media_channel_t *chm)
{
    int ret = 0;
    if (_mediaRtpSched.evtqueue)
    {
        ret = zpl_media_event_register(_mediaRtpSched.evtqueue, ZPL_MEDIA_GLOAL_VIDEO_ENCODE, ZPL_MEDIA_EVENT_DISTPATCH,
                                 zpl_mediartp_event_handle, chm);
        if(ret == 0)
        {
            zm_msg_error("media channel %d/%d can push event", chm->channel, chm->channel_index);
        }                         
    }
    return OK;
}

int zpl_mediartp_scheduler_init(void)
{
    memset(&_mediaRtpSched, 0, sizeof(zpl_mediartp_scheduler_t));
    _mediaRtpSched.mutex = os_mutex_name_create("mrtp-mutex");
    lstInitFree(&_mediaRtpSched.list, zpl_mediartp_session_cbfree);
#ifdef ZPL_JRTPLIB_MODULE
    _mediaRtpSched.evtqueue = zpl_media_event_create("mediaRtpSched", 32);
    jrtp_av_profile_init(&jrtp_av_profile);
#endif
    _mediaRtpSched._debug = 0x00ffffff;
    //memset(&my_h26x_pktz, 0, sizeof(my_h26x_pktz));
    return OK;
}

int zpl_mediartp_scheduler_exit(void)
{
    if (_mediaRtpSched.mutex)
        os_mutex_lock(_mediaRtpSched.mutex, OS_WAIT_FOREVER);
    lstFree(&_mediaRtpSched.list);
    if (_mediaRtpSched.mutex)
        os_mutex_unlock(_mediaRtpSched.mutex);
    #ifdef ZPL_JRTPLIB_MODULE
    zpl_media_event_destroy(_mediaRtpSched.evtqueue);
    #endif
    if (_mediaRtpSched.mutex)
        os_mutex_destroy(_mediaRtpSched.mutex);
    _mediaRtpSched.mutex = NULL;
    return OK;
}

int zpl_mediartp_scheduler_start(void)
{
    #ifdef ZPL_JRTPLIB_MODULE
    zpl_media_event_start(_mediaRtpSched.evtqueue, 60, OS_TASK_DEFAULT_STACK*2);
    #endif                                          
    return OK;
}

int zpl_mediartp_scheduler_stop(void)
{
    if (_mediaRtpSched.taskid)
    {
        if (os_task_destroy(_mediaRtpSched.taskid) == OK)
            _mediaRtpSched.taskid = 0;
    }
    return OK;
}
