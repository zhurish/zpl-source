/**
 * @file      : zpl_media_sched.h
 * @brief     : Description
 * @author    : zhurish (zhurish@163.com)
 * @version   : 1.0
 * @date      : 2024-03-15
 * 
 * @copyright : Copyright (c) - 2024 zhurish(zhurish@163.com).Co.Ltd. All rights reserved.
 * 
 */
#ifndef __ZPL_MEDIA_SHCED_H__
#define __ZPL_MEDIA_SHCED_H__

#ifdef __cplusplus
extern "C" {
#endif


typedef struct 
{
    zpl_media_event_queue_t *evtqueue;
	zpl_media_channel_t * mediachn;
	zpl_skbqueue_t   *_queue;
    void	*mutex;
    os_list_t	_rtp_list;
    os_list_t	_proxy_list;
    os_list_t	_rtmp_list;	
  	os_list_t	_mucast_list;	
    int     count;    

    zpl_taskid_t taskid;
    int     _debug;
}zpl_media_sched_t;

int zpl_media_sched_start_channel(zpl_media_channel_t * mediachn);
int zpl_media_sched_stop_channel(zpl_media_channel_t * mediachn);
int zpl_media_sched_session_add(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, ZPL_MEDIA_SCHED_E type, void *session);
int zpl_media_sched_session_del(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, ZPL_MEDIA_SCHED_E type, void *session);
int zpl_media_sched_session_lookup(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, ZPL_MEDIA_SCHED_E type, void *session);



#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_SHCED_H__ */
