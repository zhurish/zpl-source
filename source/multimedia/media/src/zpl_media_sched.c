/**
 * @file      : zpl_media_sched.c
 * @brief     : Description
 * @author    : zhurish (zhurish@163.com)
 * @version   : 1.0
 * @date      : 2024-03-15
 * 
 * @copyright : Copyright (c) - 2024 zhurish(zhurish@163.com).Co.Ltd. All rights reserved.
 * 
 */
#include "zpl_media.h"
#include "zpl_media_internal.h"



static int zpl_media_sched_dispatch_signal(zpl_media_sched_t *chm);

static int zpl_media_sched_session_callback_handler(void *session, void *pUser)
{
    //zpl_skbuffer_t *skb = pUser;
    //zpl_media_hdr_t *media_header = skb->skb_hdr.other_hdr;
    //ret = zpl_media_session_adap_rtp_sendto(chm, media_header->codectype,
    //                                           ZPL_SKB_DATA(skb), ZPL_SKB_DATA_LEN(skb));    
    return OK;
}

static int zpl_media_sched_session_dispatch_all(zpl_media_sched_t *_msched)
{
    int ret = 0;
    if (_msched && _msched->_queue)
    {
        int glen = 0;
        uint8_t skbtmp[32];
        zpl_skbuffer_t *skb = zpl_skbqueue_get(_msched->_queue);
        if (skb)
        {
            //zpl_media_hdr_t *media_header = skb->skb_hdr.other_hdr;
            /*zpl_skbuffer_t * tmpskb = zpl_skbuffer_clone(_msched->_queue, skb);
            if(tmpskb)
            {
			    os_list_for_each2(&_msched->_rtp_list, zpl_media_sched_session_callback_handler, tmpskb);
                zpl_skbqueue_finsh(_msched->_queue, tmpskb);
            }*/

            glen = zpl_skbuffer_get_startsize(skb, skbtmp, sizeof(skbtmp));

            os_list_for_each2(&_msched->_rtp_list, zpl_media_sched_session_callback_handler, skb);
            if(glen)
                zpl_skbuffer_put_startsize(skb, skbtmp, glen);
			os_list_for_each2(&_msched->_proxy_list, zpl_media_sched_session_callback_handler, skb);

            if(glen)
                zpl_skbuffer_put_startsize(skb, skbtmp, glen);
			os_list_for_each2(&_msched->_rtmp_list, zpl_media_sched_session_callback_handler, skb);

            if(glen)
                zpl_skbuffer_put_startsize(skb, skbtmp, glen);
			os_list_for_each2(&_msched->_mucast_list, zpl_media_sched_session_callback_handler, skb);

            zpl_skbqueue_finsh(_msched->_queue, skb);
        }
    }
    return ret;
}

static int zpl_media_sched_queue_clone(zpl_media_channel_t *mediachn,
                                          const zpl_skbuffer_t *bufdata, void *pVoidUser)
{
    int ret = 0;
    zpl_skbuffer_t *skb = NULL;
    if (mediachn->p_sched.param)
    {
		zpl_media_sched_t *_msched = mediachn->p_sched.param;
        if(_msched->count)
        {
            skb = zpl_skbuffer_clone(_msched->_queue, bufdata);
            if (skb)
            {
                ret = zpl_skbqueue_add(_msched->_queue, skb);
                zpl_media_sched_dispatch_signal(_msched);
            }
            else
            {
                zpl_media_sched_dispatch_signal(_msched);
                zm_msg_error("media channel %d/%d can not clone skb", mediachn->channel, mediachn->channel_index);
            }
        }
    }
    return ret;
}

static int zpl_media_sched_handler(zpl_media_event_t *event)
{
    zpl_media_sched_t *chm = event->event_data;
    if (chm)
    {
        zpl_media_sched_session_dispatch_all(chm);
    }
    return OK;
}

static int zpl_media_sched_dispatch_signal(zpl_media_sched_t *chm)
{
    int ret = 0;
    if (chm->evtqueue)
    {
        ret = zpl_media_event_register(chm->evtqueue, ZPL_MEDIA_GLOAL_VIDEO_ENCODE, ZPL_MEDIA_EVENT_DISTPATCH,
                                 zpl_media_sched_handler, chm);
        if(ret == 0)
        {
            zm_msg_error("media channel %d/%d can push event", chm->mediachn->channel, chm->mediachn->channel_index);
        }                         
    }
    return OK;
}


static void zpl_media_sched_session_cbfree(void *data)
{
    if (data)
    {
        free(data);
    }
}

static int zpl_media_sched_create(zpl_media_sched_t *sched, zpl_media_channel_t * mediachn)
{
    sched->mutex = os_mutex_name_create(os_name_format("sched-mutex-%d/%d",mediachn->channel, mediachn->channel_index));
	if(sched->mutex)
	{
        sched->_queue = zpl_skbqueue_create(os_name_format("schedQueue%d.%d", mediachn->channel, mediachn->channel_index), ZPL_MEDIA_BUFQUEUE_SIZE, zpl_false);
        if(sched->_queue == NULL)
        {
			os_mutex_destroy(sched->mutex);
            return ERROR;
        }	
		os_list_init(&sched->_rtp_list, zpl_media_sched_session_cbfree);
		os_list_init(&sched->_proxy_list, zpl_media_sched_session_cbfree);
		os_list_init(&sched->_rtmp_list, zpl_media_sched_session_cbfree);
		os_list_init(&sched->_mucast_list, zpl_media_sched_session_cbfree);

    	sched->evtqueue = zpl_media_event_create(os_name_format("mediaSched%d.%d",mediachn->channel, mediachn->channel_index), 32);
		if(sched->evtqueue  == NULL)
		{
			os_mutex_destroy(sched->mutex);
			return ERROR;
		}
	   	if(zpl_media_event_start(sched->evtqueue, 60, OS_TASK_DEFAULT_STACK*2) == OK)
		{
			return OK;
		}
        os_mutex_destroy(sched->mutex);
        zpl_media_event_destroy(sched->evtqueue);
	}
    return ERROR;
}

static int zpl_media_sched_destroy(zpl_media_sched_t *sched)
{
	zpl_media_event_destroy(sched->evtqueue);
    if (sched->mutex)
        os_mutex_lock(sched->mutex, OS_WAIT_FOREVER);
    os_list_delete_all_node_withdata(&sched->_rtp_list);
  	os_list_delete_all_node_withdata(&sched->_proxy_list);
  	os_list_delete_all_node_withdata(&sched->_rtmp_list);
  	os_list_delete_all_node_withdata(&sched->_mucast_list);
    if(sched->_queue == NULL)
    {
		zpl_skbqueue_destroy(sched->_queue);
    }	
    if (sched->mutex)
        os_mutex_unlock(sched->mutex);
    if (sched->mutex)
        os_mutex_destroy(sched->mutex);
    sched->mutex = NULL;
    return OK;
}


int zpl_media_sched_start_channel(zpl_media_channel_t * mediachn)
{   
    if (mediachn)
    {
        if(mediachn->p_sched.param == NULL)
        {
			zpl_media_sched_t *sched = malloc(sizeof(zpl_media_sched_t));
			if(sched)
			{
			    memset(sched, 0, sizeof(zpl_media_sched_t));
				if(zpl_media_sched_create(sched, mediachn) != OK)
				{
					free(sched);
					return ERROR;
				}
			}
			else
			{
				return ERROR;
			}

			sched->mediachn = mediachn;
           	mediachn->p_sched.param = sched;
        }
        if(mediachn->p_sched.cbid <= 0)
        {
            mediachn->p_sched.cbid = zpl_media_channel_client_add(mediachn->channel, mediachn->channel_index, zpl_media_sched_queue_clone, mediachn);       
            if((int)mediachn->p_sched.cbid <= 0)
            {
				zpl_media_sched_destroy(mediachn->p_sched.param);
				mediachn->p_sched.param = NULL;
                return ERROR;
            }
        }
		zpl_media_channel_client_start(mediachn->channel, mediachn->channel_index, mediachn->p_sched.cbid, zpl_true);
        return OK;
    }
    return ERROR;
}

int zpl_media_sched_stop_channel(zpl_media_channel_t * mediachn)
{   
    if (mediachn)
    {
		if(mediachn->p_sched.cbid)
		{
			zpl_media_channel_client_start(mediachn->channel, mediachn->channel_index, mediachn->p_sched.cbid, zpl_false);
			zpl_media_channel_client_del(mediachn->channel, mediachn->channel_index, mediachn->p_sched.cbid);
			mediachn->p_sched.cbid = 0;
		}
        if(mediachn->p_sched.param != NULL)
        {
			zpl_media_sched_destroy(mediachn->p_sched.param);
			mediachn->p_sched.param = NULL;
        }
        return OK;
    }
    return ERROR;
}


int zpl_media_sched_session_add(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, ZPL_MEDIA_SCHED_E type, void *session)
{
    int ret = 0;
    zpl_media_channel_t *mediachn = NULL;
    zpl_media_sched_t *sched = NULL;
    mediachn = zpl_media_channel_lookup(channel, channel_index);
    if(mediachn && mediachn->p_sched.param)
    {
        sched = mediachn->p_sched.param;

        if(sched->mutex)
    	    os_mutex_lock(sched->mutex, OS_WAIT_FOREVER);
        switch(type)
        {
            case ZPL_MEDIA_SCHED_RTP:			//
            os_listnode_add (&sched->_rtp_list, session);
            break;
            case ZPL_MEDIA_SCHED_RTMP:			//
            os_listnode_add (&sched->_rtmp_list, session);
            break;
            case ZPL_MEDIA_SCHED_MULTICAST:			//
            os_listnode_add (&sched->_mucast_list, session);
            break;
            case ZPL_MEDIA_SCHED_PROXY:
            os_listnode_add (&sched->_proxy_list, session);
            break;
            case ZPL_MEDIA_SCHED_SDK:
            os_listnode_add (&sched->_proxy_list, session);
            break;
            default:
            ret = ERROR;
            break;
        }
        sched->count = os_listcount(&sched->_rtp_list);
        sched->count += os_listcount(&sched->_rtmp_list);
        sched->count += os_listcount(&sched->_mucast_list);
        sched->count += os_listcount(&sched->_proxy_list);

        if(sched->mutex)
    	    os_mutex_unlock(sched->mutex);
        return ret;
    }
    return ERROR;
}

int zpl_media_sched_session_del(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, ZPL_MEDIA_SCHED_E type, void *session)
{
    int ret = 0;
    zpl_media_channel_t *mediachn = NULL;
    zpl_media_sched_t *sched = NULL;
    mediachn = zpl_media_channel_lookup(channel, channel_index);
    if(mediachn && mediachn->p_sched.param)
    {
        sched = mediachn->p_sched.param;

        if(sched->mutex)
    	    os_mutex_lock(sched->mutex, OS_WAIT_FOREVER);
        switch(type)
        {
            case ZPL_MEDIA_SCHED_RTP:			//
            os_listnode_delete (&sched->_rtp_list, session);
            break;
            case ZPL_MEDIA_SCHED_RTMP:			//
            os_listnode_delete (&sched->_rtmp_list, session);
            break;
            case ZPL_MEDIA_SCHED_MULTICAST:			//
            os_listnode_delete (&sched->_mucast_list, session);
            break;
            case ZPL_MEDIA_SCHED_PROXY:
            os_listnode_delete (&sched->_proxy_list, session);
            break;
            case ZPL_MEDIA_SCHED_SDK:
            os_listnode_delete (&sched->_proxy_list, session);
            break;
            default:
            ret = ERROR;
            break;
        }
        sched->count = os_listcount(&sched->_rtp_list);
        sched->count += os_listcount(&sched->_rtmp_list);
        sched->count += os_listcount(&sched->_mucast_list);
        sched->count += os_listcount(&sched->_proxy_list);

        if(sched->mutex)
    	    os_mutex_unlock(sched->mutex);
        return ret;
    }
    return ERROR;
}

int zpl_media_sched_session_lookup(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, ZPL_MEDIA_SCHED_E type, void *session)
{
    int ret = ERROR;
    os_listnode *node = NULL;
    zpl_media_channel_t *mediachn = NULL;
    zpl_media_sched_t *sched = NULL;
    mediachn = zpl_media_channel_lookup(channel, channel_index);
    if(mediachn && mediachn->p_sched.param)
    {
        sched = mediachn->p_sched.param;

        if(sched->mutex)
    	    os_mutex_lock(sched->mutex, OS_WAIT_FOREVER);
        switch(type)
        {
            case ZPL_MEDIA_SCHED_RTP:			//
            node = os_listnode_lookup (&sched->_rtp_list, session);
            break;
            case ZPL_MEDIA_SCHED_RTMP:			//
            node = os_listnode_lookup (&sched->_rtmp_list, session);
            break;
            case ZPL_MEDIA_SCHED_MULTICAST:			//
            node = os_listnode_lookup (&sched->_mucast_list, session);
            break;
            case ZPL_MEDIA_SCHED_PROXY:
            node = os_listnode_lookup (&sched->_proxy_list, session);
            break;
            case ZPL_MEDIA_SCHED_SDK:
            node = os_listnode_lookup (&sched->_proxy_list, session);
            break;
            default:
            ret = ERROR;
            break;
        }
        if(node)
            ret = OK;
        if(sched->mutex)
    	    os_mutex_unlock(sched->mutex);
        return ret;
    }
    return ERROR;
}