/*
 * zpl_media_rtp.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "zpl_media.h"
#include "zpl_media_internal.h"

#ifdef ZPL_JRTPLIB_MODULE
#include "jrtplib_api.h"
#include "jrtp_payloadtype.h"
#include "jrtp_rtpprofile.h"
#endif

#include "rtp_payload.h"
#include "rtp_h264.h"
#ifdef ZPL_JRTPLIB_MODULE
static int zpl_mediartp_event_dispatch_signal(zpl_media_channel_t *chm);
#endif
static zpl_mediartp_scheduler_t _mediaRtpSched;

#define ZPL_MEDIARTP_DEBUG(v)	      ( _mediaRtpSched._debug& (ZPL_VIDEO_DEBUG_ ##v ) )

#if 1
typedef struct rtp_session_adap_s
{
    char                *name;
    uint32_t            codec;
    int (*_rtp_sendto)(void *, const u_int8_t *, u_int32_t );
    int (*_rtp_recv)(void *, u_int8_t *, u_int32_t );                   
}zpl_mediartp_session_adap_t;

static zpl_mediartp_session_adap_t _rtp_media_adap_tbl[] =
    {
        {"H263", RTP_MEDIA_PAYLOAD_H263, rtp_payload_send_h264, NULL},
        {"H264", RTP_MEDIA_PAYLOAD_H264, rtp_payload_send_h264, NULL},
        
        /*{"G711A", RTP_MEDIA_PAYLOAD_G711A, rtp_payload_send_h264, NULL},
        {"G711U", RTP_MEDIA_PAYLOAD_G711U, rtp_payload_send_g7xx, NULL},

        {"G722", RTP_MEDIA_PAYLOAD_G722, rtp_payload_send_g7xx, NULL},
        {"G726", RTP_MEDIA_PAYLOAD_G726, rtp_payload_send_g7xx, NULL},

        {"G728", RTP_MEDIA_PAYLOAD_G728, rtp_payload_send_g7xx, NULL},
        {"G729", RTP_MEDIA_PAYLOAD_G729, rtp_payload_send_g7xx, NULL},
        {"G729A", RTP_MEDIA_PAYLOAD_G729A, rtp_payload_send_g7xx, NULL},

        {"PCMA", RTP_MEDIA_PAYLOAD_PCMA, rtp_payload_send_g7xx, NULL},
        {"PCMU", RTP_MEDIA_PAYLOAD_PCMU, rtp_payload_send_g7xx, NULL},*/
};

static int zpl_mediartp_session_adap_rtp_sendto(zpl_media_channel_t *chm, uint32_t codec, const uint8_t *buf, uint32_t len)
{
    uint32_t i = 0;
    int ret = 0;
    if (chm)
    {
        for (i = 0; i < sizeof(_rtp_media_adap_tbl) / sizeof(_rtp_media_adap_tbl[0]); i++)
        {
            if (_rtp_media_adap_tbl[i].codec == codec)
            {
                if (_rtp_media_adap_tbl[i]._rtp_sendto)
                return (_rtp_media_adap_tbl[i]._rtp_sendto)(chm, buf, len);
            }
        }
    }
    return -1;
}
#else
typedef struct rtp_session_adap_s
{
    char                *name;
    uint32_t            codec;
    int (*_rtp_packet)(h26x_packetizer *pktz,
                                            u_int8_t *bits,
                                            int bits_len,
                                            unsigned int *bits_pos,
                                            const u_int8_t **payload,
                                            int *payload_len);
    int (*_rtp_unpacket)(h26x_packetizer *pktz,
                                              const u_int8_t *payload,
                                              int payload_len,
                                              u_int8_t *bits,
                                              int bits_size,
                                              unsigned int *pos);                   
}zpl_mediartp_session_adap_t;

static zpl_mediartp_session_adap_t _rtp_media_adap_tbl[] =
    {
        {"H263", RTP_MEDIA_PAYLOAD_H263, rtp_h26x_packetize, NULL},
        {"H264", RTP_MEDIA_PAYLOAD_H264, rtp_h26x_packetize, NULL},
};

static h26x_packetizer my_h26x_pktz;

static int zpl_mediartp_session_adap_rtp_sendto(zpl_media_channel_t *chm, uint32_t codec, const uint8_t *buf, uint32_t len)
{
    u_int8_t *payload = NULL;
    u_int32_t payload_len = 0;
    int has_more = 1;
    int ret = 0;
    uint32_t i = 0;
    if (chm)
    {
        for (i = 0; i < sizeof(_rtp_media_adap_tbl) / sizeof(_rtp_media_adap_tbl[0]); i++)
        {
            if (_rtp_media_adap_tbl[i].codec == codec)
            {
                if (_rtp_media_adap_tbl[i]._rtp_packet)
                {
                    if (my_h26x_pktz.cfg.codec == 0)
                    {
                        rtp_h26x_packetizer_init(codec, MAX_RTP_PAYLOAD_LENGTH,
                                                 H264_PACKETIZER_MODE_NON_INTERLEAVED, 4, &my_h26x_pktz);
                    }
                    my_h26x_pktz.frame_data = buf;
                    my_h26x_pktz.frame_size = len;
                    my_h26x_pktz.frame_pos = 0;
                    while (has_more)
                    {
                        ret = (_rtp_media_adap_tbl[i]._rtp_packet)(&my_h26x_pktz, my_h26x_pktz.frame_data,
                                                                   my_h26x_pktz.frame_size,
                                                                   &my_h26x_pktz.frame_pos,
                                                                   &payload, &payload_len);
                        if (ret != 0)
                            return -1;
                        has_more = (my_h26x_pktz.frame_pos < my_h26x_pktz.frame_size);
#ifdef ZPL_JRTPLIB_MODULE
                        if (payload)
                        {
                            if (len < MAX_RTP_PAYLOAD_LENGTH)
                                ret = zpl_mediartp_session_rtp_sendto(chm, payload, payload_len, 255, 1, 1);
                            else
                                ret = zpl_mediartp_session_rtp_sendto(chm, payload, payload_len, 255, has_more ? 0 : 1, has_more ? 0 : 1);
                        }
#endif
                    }
                }
            }
        }
    }
    return ret;
}
#endif

int zpl_mediartp_session_rtp_sendto(zpl_media_channel_t *chm, const void *data, size_t len,
	                u_int8_t pt, bool mark, u_int32_t next_ts)
{
    int ret = ERROR;
    NODE node;
    zpl_mediartp_session_t *my_session = NULL;
    if (_mediaRtpSched.mutex)
        os_mutex_lock(_mediaRtpSched.mutex, OS_WAIT_FOREVER);
        
    for (my_session = (zpl_mediartp_session_t *)lstFirst(&_mediaRtpSched.list);
         my_session != NULL; my_session = (zpl_mediartp_session_t *)lstNext(&node))
    {
        node = my_session->node;
        ZPL_MEDIARTP_CHANNEL_LOCK(my_session);
        if (my_session && my_session->channel == chm->channel && my_session->level == chm->channel_index)
        {
            if (my_session->b_start == zpl_false)
            {
            }
            else
            {
                //zm_msg_debug("media rtp session keyval %u sending media stream", my_session->keyval);
#ifdef ZPL_JRTPLIB_MODULE 
                ret = jrtp_session_sendto(my_session->_session, data, len, pt, mark, next_ts?my_session->timestamp_interval:0);
#endif 
            }
        }
        ZPL_MEDIARTP_CHANNEL_UNLOCK(my_session);
    }
    if (_mediaRtpSched.mutex)
        os_mutex_unlock(_mediaRtpSched.mutex);
    return ret;
}

static int zpl_mediartp_session_dispatch_all(zpl_media_channel_t *chm)
{
    int ret = 0;
    if (chm->rtp_param.param)
    {
        zpl_skbuffer_t *skb = zpl_skbqueue_get(chm->rtp_param.param);
        if (skb)
        {
            zpl_media_hdr_t *media_header = skb->skb_hdr.other_hdr;

            if (ZPL_MEDIARTP_DEBUG(EVENT) && ZPL_MEDIARTP_DEBUG(DETAIL))
            {
                //zm_msg_debug("media rtp session keyval %u is sending media stream", my_session->keyval);
            }
            ret = zpl_mediartp_session_adap_rtp_sendto(chm, media_header->codectype,
                                                       ZPL_SKB_DATA(skb), ZPL_SKB_DATA_LEN(skb));

            zpl_skbqueue_finsh(chm->rtp_param.param, skb);
        }
    }
    return ret;
}

static int zpl_mediartp_session_rtp_clone(zpl_media_channel_t *mediachn,
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
            #ifdef ZPL_JRTPLIB_MODULE
            zpl_mediartp_event_dispatch_signal(mediachn);
            #endif
        }
        else
        {
            #ifdef ZPL_JRTPLIB_MODULE
            zpl_mediartp_event_dispatch_signal(mediachn);
            #endif
            zm_msg_error("media channel %d/%d can not clone skb", mediachn->channel, mediachn->channel_index);
        }
    }
    return ret;
}


static int zpl_mediartp_connect_count(int channel, int level)
{
    int count = 0;
    NODE node;
    zpl_mediartp_session_t *my_session = NULL;
    if (_mediaRtpSched.mutex)
        os_mutex_lock(_mediaRtpSched.mutex, OS_WAIT_FOREVER);

	for (my_session = (zpl_mediartp_session_t *)lstFirst(&_mediaRtpSched.list);
		 my_session != NULL; my_session = (zpl_mediartp_session_t *)lstNext(&node))
	{
		node = my_session->node;
        if(my_session && my_session->channel == channel && my_session->level == level)
        {
            count++;
        }
	} 
    if (_mediaRtpSched.mutex)
        os_mutex_unlock(_mediaRtpSched.mutex);
    return count;    
}

static zpl_mediartp_session_t * zpl_mediartp_rtpparam(int keyval)
{
    NODE node;
    zpl_mediartp_session_t *my_session = NULL;
    if (_mediaRtpSched.mutex)
        os_mutex_lock(_mediaRtpSched.mutex, OS_WAIT_FOREVER);

	for (my_session = (zpl_mediartp_session_t *)lstFirst(&_mediaRtpSched.list);
		 my_session != NULL; my_session = (zpl_mediartp_session_t *)lstNext(&node))
	{
		node = my_session->node;
        if(my_session && my_session->keyval == keyval)
        {
            if (_mediaRtpSched.mutex)
                os_mutex_unlock(_mediaRtpSched.mutex);
            if(ZPL_MEDIARTP_DEBUG(EVENT) && ZPL_MEDIARTP_DEBUG(DETAIL))
            {
                //zm_msg_debug("media rtp session keyval %u is found", keyval);
            }  
            return my_session; 
        }
	} 
    if (_mediaRtpSched.mutex)
        os_mutex_unlock(_mediaRtpSched.mutex);
    zm_msg_error("media rtp session keyval %u is not found", keyval);   
    return NULL;    
}


int zpl_mediartp_session_add_dest(zpl_mediartp_session_t *my_session, char *address, u_int16_t rtpport, u_int16_t rtcpport)
{
#ifdef ZPL_JRTPLIB_MODULE
    if(my_session && my_session->_session)
        return jrtp_session_destination_add(my_session->_session, address,  rtpport,  rtcpport);
#endif
    return -1;
}

int zpl_mediartp_session_del_dest(zpl_mediartp_session_t *my_session, char *address, u_int16_t rtpport, u_int16_t rtcpport)
{
#ifdef ZPL_JRTPLIB_MODULE
    if(my_session && my_session->_session)
        return jrtp_session_destination_del(my_session->_session, address,  rtpport,  rtcpport);
#endif
    return -1;
}

static int zpl_mediartp_session_default(zpl_mediartp_session_t *my_session)
{
    if (my_session)
    {
        my_session->i_trackid = -1;
        my_session->rtp_interleaved = -1;
        my_session->rtcp_interleaved = -1;   // rtsp setup response only

        my_session->b_start = zpl_true;
        my_session->packetization_mode = 1; // 封包解包模式
        my_session->user_timestamp = 0;     // 用户时间戳
        my_session->timestamp_interval = 0; // 用户时间戳

        my_session->local_rtpport = VIDEO_RTP_PORT_DEFAULT + (lstCount(&_mediaRtpSched.list)) * 2;
        my_session->local_rtcpport = VIDEO_RTCP_PORT_DEFAULT + (lstCount(&_mediaRtpSched.list)) * 2;
        return OK;
    }
    return ERROR;
}

static int zpl_mediartp_session_connect(zpl_mediartp_session_t *my_session, int channel, int level)
{
    zpl_media_channel_t * mediachn = zpl_media_channel_lookup(channel, level);    
    if (my_session && mediachn)
    {
        if(mediachn->rtp_param.cbid <= 0)
        {
            mediachn->rtp_param.cbid = zpl_media_channel_client_add(mediachn->channel, mediachn->channel_index, zpl_mediartp_session_rtp_clone, mediachn);       
            if((int)mediachn->rtp_param.cbid <= 0)
            {
                return ERROR;
            }
        }
        if(mediachn->rtp_param.param == NULL)
        {
            mediachn->rtp_param.param = zpl_skbqueue_create(os_name_format("rtpSkbQueue-%d/%d", mediachn->channel, mediachn->channel_index), ZPL_MEDIA_BUFQUEUE_SIZE, zpl_false);
            if(mediachn->rtp_param.param == NULL)
            {
                return ERROR;
            }
        }
        my_session->media_chn = mediachn;

        if (mediachn->media_param.video_media.codec.framerate == 0)
            my_session->framerate = 30;
        else
            my_session->framerate = mediachn->media_param.video_media.codec.framerate;
        my_session->payload = mediachn->media_param.video_media.codec.codectype;
        if(ZPL_MEDIARTP_DEBUG(EVENT) && ZPL_MEDIARTP_DEBUG(DETAIL))
        {
            zm_msg_debug("media rtp session keyval %u is connect to %d/%d framerate %d payload %d", 
                my_session->keyval, channel, level, my_session->framerate, my_session->payload);
        } 
#ifdef ZPL_JRTPLIB_MODULE
        if(my_session->_session)
        {
            jrtp_session_framerate_set(my_session->_session, my_session->framerate);

            jrtp_session_payload_set(my_session->_session, my_session->payload, jrtp_profile_get_clock_rate(my_session->payload));
        }
        if (my_session->framerate)
        {
            my_session->timestamp_interval = jrtp_profile_get_clock_rate(my_session->payload) / my_session->framerate;
        }
        if(my_session->_session)
            jrtp_session_local_set(my_session->_session, strlen(my_session->local_address)?my_session->local_address:NULL, my_session->local_rtpport, my_session->local_rtcpport);
#endif
        return OK;
    }
    return ERROR;
}

zpl_mediartp_session_t *zpl_mediartp_session_create(int keyval, int channel, int level, char *filename)
{
    zpl_mediartp_session_t *my_session = NULL;

    my_session = zpl_mediartp_rtpparam(keyval);
    if (my_session == NULL)
    {
        my_session = malloc(sizeof(zpl_mediartp_session_t));
        if (my_session)
            memset(my_session, 0, sizeof(zpl_mediartp_session_t));
    }
    if (my_session)
    {
        if (my_session->_mutex == NULL)
        {
            my_session->_mutex = os_mutex_name_create("rtp_mutex");
        }        

#ifdef ZPL_JRTPLIB_MODULE
        if (my_session->_session == NULL)
            my_session->_session = jrtp_session_alloc();
#endif
        if (my_session->_session == NULL)
        {
            zm_msg_error("media rtp session keyval %u can not create rtp session for %d/%d", 
                    my_session->keyval, channel, level);
            os_mutex_destroy(my_session->_mutex);        
            free(my_session);
            return NULL;
        }
        zpl_mediartp_session_default(my_session);
        my_session->keyval = keyval;
        my_session->channel = channel;
        my_session->level = level;

        if(ZPL_MEDIARTP_DEBUG(EVENT) && ZPL_MEDIARTP_DEBUG(DETAIL))
        {
            zm_msg_debug("media rtp session keyval %u create and connect to %d/%d", 
                my_session->keyval, channel, level);
        } 

        if(filename)
            my_session->filename = strdup(filename);

        if(zpl_mediartp_session_connect(my_session, channel, level) == ERROR)
        {
#ifdef ZPL_JRTPLIB_MODULE
            jrtp_session_destroy(my_session->_session);
            my_session->_session = NULL;
#endif            
            os_mutex_destroy(my_session->_mutex); 
            if(my_session->filename)
            {
                free(my_session->filename);
                my_session->filename = NULL;
            }
            free(my_session);
            return NULL;
        }
        if (_mediaRtpSched.mutex)
            os_mutex_lock(_mediaRtpSched.mutex, OS_WAIT_FOREVER);
        lstAdd(&_mediaRtpSched.list, (NODE *)my_session);
        if (_mediaRtpSched.mutex)
            os_mutex_unlock(_mediaRtpSched.mutex);
        return my_session;
    }
    else
    {
    }
    return NULL;
}

int zpl_mediartp_session_destroy(int keyval)
{
    zpl_mediartp_session_t *my_session = NULL;
    zpl_media_channel_t *mchn = NULL;
    my_session = zpl_mediartp_rtpparam(keyval);
    if (!my_session || !my_session->media_chn)
        return ERROR;
    ZPL_MEDIARTP_CHANNEL_LOCK(my_session);    
    mchn = my_session->media_chn;    

    if (mchn->rtp_param.cbid > 0 && zpl_mediartp_connect_count(mchn->channel, mchn->channel_index) <= 1)
    {
        if(ZPL_MEDIARTP_DEBUG(EVENT) && ZPL_MEDIARTP_DEBUG(DETAIL))
        {
            zm_msg_debug("media rtp session keyval %u for %d/%d stop client %d", 
                my_session->keyval, mchn->channel, mchn->channel_index, mchn->rtp_param.cbid);
        }         
        zpl_media_channel_client_start(mchn->channel, mchn->channel_index, mchn->rtp_param.cbid, zpl_false);
    }
    #ifdef ZPL_JRTPLIB_MODULE
    jrtp_session_stop(my_session->_session);
    if (jrtp_session_destroy(my_session->_session) != OK)
    {
        ZPL_MEDIARTP_CHANNEL_UNLOCK(my_session); 
        return ERROR;
    }
    my_session->_session = NULL;
    #endif
    if(my_session->filename)
    {
        free(my_session->filename);
        my_session->filename = NULL;
    }
    ZPL_MEDIARTP_CHANNEL_UNLOCK(my_session); 
    if(my_session->_mutex)
        os_mutex_destroy(my_session->_mutex); 
    if (_mediaRtpSched.mutex)
        os_mutex_lock(_mediaRtpSched.mutex, OS_WAIT_FOREVER);
    lstDelete(&_mediaRtpSched.list, (NODE *)my_session);
    free(my_session);
    if (_mediaRtpSched.mutex)
        os_mutex_unlock(_mediaRtpSched.mutex);
    return OK;
}

int zpl_mediartp_session_suspend(int keyval)
{
    zpl_mediartp_session_t *my_session = NULL;
    zpl_media_channel_t *mchn = NULL;
    my_session = zpl_mediartp_rtpparam(keyval);
    if (!my_session || !my_session->media_chn)
        return ERROR;
    ZPL_MEDIARTP_CHANNEL_LOCK(my_session);    
    mchn = my_session->media_chn;  
    if (my_session && my_session->_session)
    {
        my_session->b_start = zpl_false;
        if(ZPL_MEDIARTP_DEBUG(EVENT) && ZPL_MEDIARTP_DEBUG(DETAIL))
        {
            zm_msg_debug("media rtp session keyval %u for %d/%d suspend", 
                my_session->keyval, mchn->channel, mchn->channel_index);
        } 
        ZPL_MEDIARTP_CHANNEL_UNLOCK(my_session);
        return OK;
    }
    ZPL_MEDIARTP_CHANNEL_UNLOCK(my_session);
    return ERROR;
}

int zpl_mediartp_session_resume(int keyval)
{
    zpl_mediartp_session_t *my_session = NULL;
    zpl_media_channel_t *mchn = NULL;
    my_session = zpl_mediartp_rtpparam(keyval);
    if (!my_session || !my_session->media_chn)
        return ERROR;
    ZPL_MEDIARTP_CHANNEL_LOCK(my_session);    
    mchn = my_session->media_chn;  
    if (my_session && my_session->_session)
    {
        my_session->b_start = zpl_true;
        if(ZPL_MEDIARTP_DEBUG(EVENT) && ZPL_MEDIARTP_DEBUG(DETAIL))
        {
            zm_msg_debug("media rtp session keyval %u for %d/%d resume", 
                my_session->keyval, mchn->channel, mchn->channel_index);
        } 
        ZPL_MEDIARTP_CHANNEL_UNLOCK(my_session);
        return OK;
    }
    ZPL_MEDIARTP_CHANNEL_UNLOCK(my_session);
    return ERROR;
}

int zpl_mediartp_session_start(int keyval, zpl_bool start)
{
    int ret = 0;
    zpl_mediartp_session_t *my_session = NULL;
    zpl_media_channel_t *mchn = NULL;
    my_session = zpl_mediartp_rtpparam(keyval);
    if (!my_session || !my_session->media_chn)
        return ERROR;
    ZPL_MEDIARTP_CHANNEL_LOCK(my_session);    
    mchn = my_session->media_chn; 
    if (my_session && my_session->_session)
    {
        if (start)
        {
            if(mchn->rtp_param.cbid <= 0)
            {
                mchn->rtp_param.cbid = zpl_media_channel_client_add(mchn->channel, mchn->channel_index, zpl_mediartp_session_rtp_clone, mchn);       
                if((int)mchn->rtp_param.cbid <= 0)
                {
                    ZPL_MEDIARTP_CHANNEL_UNLOCK(my_session);
                    return ERROR;
                }
            }
            if(mchn->rtp_param.param == NULL)
            {
                mchn->rtp_param.param = zpl_skbqueue_create(os_name_format("rtpSkbQueue-%d/%d", mchn->channel, mchn->channel_index), ZPL_MEDIA_BUFQUEUE_SIZE, zpl_false);
                if(mchn->rtp_param.param == NULL)
                {
                    ZPL_MEDIARTP_CHANNEL_UNLOCK(my_session);
                    return ERROR;
                }
            }

            if (mchn->rtp_param.enable == zpl_false)
            {
                if(ZPL_MEDIARTP_DEBUG(EVENT) && ZPL_MEDIARTP_DEBUG(DETAIL))
                {
                    zm_msg_debug("media rtp session keyval %u for %d/%d start client %d", 
                        my_session->keyval, mchn->channel, mchn->channel_index, mchn->rtp_param.cbid);
                } 
                ret = zpl_media_channel_client_start(mchn->channel, mchn->channel_index, mchn->rtp_param.cbid, zpl_true);
                if(ret != OK)
                {
                    ZPL_MEDIARTP_CHANNEL_UNLOCK(my_session);
                    return ERROR;
                }
                mchn->rtp_param.enable = zpl_true;
#ifdef ZPL_JRTPLIB_MODULE
                if (jrtp_session_start(my_session->_session) != OK)
                {
                    ZPL_MEDIARTP_CHANNEL_UNLOCK(my_session);
                    return ERROR;
                }
#endif
                my_session->b_start = zpl_true;
                ZPL_MEDIARTP_CHANNEL_UNLOCK(my_session);
                return ret;
            }
            else
            {
#ifdef ZPL_JRTPLIB_MODULE
                if (jrtp_session_start(my_session->_session) != OK)
                {
                    ZPL_MEDIARTP_CHANNEL_UNLOCK(my_session);
                    return ERROR;
                }
#endif
                my_session->b_start = zpl_true;
                ZPL_MEDIARTP_CHANNEL_UNLOCK(my_session);
                return OK;
            }
        }
        else
        {
            if (mchn->rtp_param.cbid > 0 && zpl_mediartp_connect_count(mchn->channel, mchn->channel_index) <= 1)
            {
                if(ZPL_MEDIARTP_DEBUG(EVENT) && ZPL_MEDIARTP_DEBUG(DETAIL))
                {
                    zm_msg_debug("media rtp session keyval %u for %d/%d stop client %d", 
                        my_session->keyval, mchn->channel, mchn->channel_index, mchn->rtp_param.cbid);
                } 
                ret = zpl_media_channel_client_start(mchn->channel, mchn->channel_index, mchn->rtp_param.cbid, zpl_false);
                if(ret != OK)
                {
                    ZPL_MEDIARTP_CHANNEL_UNLOCK(my_session);
                    return ERROR;
                }
            }
#ifdef ZPL_JRTPLIB_MODULE
            if (jrtp_session_stop(my_session->_session) != OK)
            {
                ZPL_MEDIARTP_CHANNEL_UNLOCK(my_session);
                return ERROR;
            }
#endif 
            my_session->b_start = zpl_false;
            ZPL_MEDIARTP_CHANNEL_UNLOCK(my_session);
            return ret;
        }
    }
    ZPL_MEDIARTP_CHANNEL_UNLOCK(my_session);
    return ERROR;
}


static void zpl_mediartp_session_cbfree(void *data)
{
    if (data)
    {
        free(data);
    }
}

#ifdef ZPL_JRTPLIB_MODULE
static int zpl_mediartp_event_handle(zpl_media_event_t *event)
{
    zpl_media_channel_t *chm = event->event_data;

    if(ZPL_MEDIARTP_DEBUG(EVENT) && ZPL_MEDIARTP_DEBUG(DETAIL))
    {
        //zm_msg_debug("media rtp session keyval %u shced", chm->keyval);
    } 
    if (chm)
    {
        zpl_mediartp_session_dispatch_all(chm);
    }
    return OK;
}

static int zpl_mediartp_event_dispatch_signal(zpl_media_channel_t *chm)
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
#endif

int zpl_media_channel_multicast_enable(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_bool enable, char *address, int rtp_port, char *localaddr)
{
    zpl_media_channel_t *mediachn = NULL;
    zpl_mediartp_session_t *my_session = NULL;
    mediachn = zpl_media_channel_lookup(channel, channel_index);
    if (mediachn == NULL)
        return ERROR;
    ZPL_MEDIA_CHANNEL_LOCK(mediachn);
    // 通道使能录像
    if (enable == zpl_true)
    {
        if (mediachn->p_mucast.param == NULL)
        {
            my_session = zpl_mediartp_session_create(ZPL_MEDIARTP_MULTICAST_KEY, channel, channel_index, NULL);
            if(my_session)
            {
                zpl_mediartp_session_setup(ZPL_MEDIARTP_MULTICAST_KEY, channel, channel_index, NULL, NULL);
#ifdef ZPL_JRTPLIB_MODULE
                if(!jrtp_session_isactive(my_session->_session))
                {
                    if (jrtp_session_create(my_session->_session) != OK)
                    {
                        ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
                        return ERROR; 
                    }
                }
#endif
                /*if (my_session->_call_index < 0)
                {
                    my_session->_call_index = zpl_media_channel_client_add(channel, channel_index, zpl_mediartp_session_rtp_clone, my_session);
                } 
                if(my_session->_call_index) 
                    zpl_media_channel_client_start(channel, channel_index, my_session->_call_index, zpl_true);*/
            }
        }  
        else
            my_session = mediachn->p_mucast.param;

        if (my_session == NULL)
        {
            ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
            return ERROR;
        }
        if(mediachn->p_mucast.param == NULL)
            mediachn->p_mucast.param = my_session;
        mediachn->p_mucast.enable = enable;
        #ifdef ZPL_JRTPLIB_MODULE
        jrtp_session_start(my_session->_session);
        jrtp_session_multicast_add(my_session->_session, address,  rtp_port, localaddr);
        #endif
        ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
        return OK;
    }
    else
    {
        mediachn->p_mucast.enable = enable;
        my_session = mediachn->p_mucast.param;
        if (mediachn->p_mucast.param)
        {
            //TODO delete mucase address
            #ifdef ZPL_JRTPLIB_MODULE
            jrtp_session_stop(my_session->_session);
            jrtp_session_multicast_del(my_session->_session, address,  rtp_port, localaddr);
            #endif

        }
        ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
        return OK;
    }
    ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
    return ERROR;
}

zpl_bool zpl_media_channel_multicast_state(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index)
{
    zpl_bool ret = zpl_false;
    zpl_media_channel_t *mediachn = zpl_media_channel_lookup(channel, channel_index);
    if (mediachn == NULL)
        return 0;
    ZPL_MEDIA_CHANNEL_LOCK(mediachn);
    ret = mediachn->p_mucast.enable;
    ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
    return ret;
}

static int zpl_mediartp_session_rtpmap_sdptext_h264(zpl_mediartp_session_t* my_session, char *src, uint32_t len)
{
    char base64sps[OS_BASE64_DECODE_SIZE(1024)];
    char base64pps[OS_BASE64_DECODE_SIZE(1024)];
    zpl_video_extradata_t extradata;
    int profile = 0, sdplength = 0;
    zpl_media_channel_t *mchn = NULL;

    memset(base64pps, 0, sizeof(base64pps));
    memset(base64sps, 0, sizeof(base64sps));
    memset(&extradata, 0, sizeof(zpl_video_extradata_t));
    
    mchn = my_session->media_chn; 
    if(my_session->media_chn)
    {
        zpl_media_channel_extradata_get(my_session->media_chn, &extradata);
        if(ZPL_MEDIARTP_DEBUG(EVENT) && ZPL_MEDIARTP_DEBUG(DETAIL))
        {
            zm_msg_debug("media rtp session keyval %u for %d/%d get sdptext", 
                        my_session->keyval, mchn->channel, mchn->channel_index);
        } 
    }
    profile = extradata.h264spspps.profileLevelId;

    sdplength = sprintf((char*)(src + sdplength), "m=video 0 RTP/AVP %d\r\n", my_session->payload);
#ifdef ZPL_JRTPLIB_MODULE
    if (jrtp_profile_get_rtpmap(RTP_MEDIA_PAYLOAD_H264))
        sdplength += sprintf((char*)(src + sdplength), "a=rtpmap:%d %s\r\n", RTP_MEDIA_PAYLOAD_H264, jrtp_profile_get_rtpmap(RTP_MEDIA_PAYLOAD_H264));
    else
#endif
        sdplength += sprintf((char*)(src + sdplength), "a=rtpmap:%d H264/90000\r\n", RTP_MEDIA_PAYLOAD_H264);

    if(extradata.fPPSSize)
        os_base64_encode(base64pps, sizeof(base64pps), extradata.fPPS + extradata.fPPSHdrLen, extradata.fPPSSize - extradata.fPPSHdrLen);
    if(extradata.fSPSSize)
        os_base64_encode(base64sps, sizeof(base64sps), extradata.fSPS + extradata.fSPSHdrLen, extradata.fSPSSize - extradata.fSPSHdrLen);

    if (strlen(base64sps))
    {
        if (strlen(base64pps))
        {
            sdplength += sprintf((char*)(src + sdplength), "a=fmtp:%d profile-level-id=%06x;"
                                                  "sprop-parameter-sets=%s,%s;packetization-mode=%d\r\n",
                                 RTP_MEDIA_PAYLOAD_H264, profile, base64sps, base64pps, my_session->packetization_mode);
        }
        else
        {
            sdplength += sprintf((char*)(src + sdplength), "a=fmtp:%d profile-level-id=%06x;"
                                                  "sprop-parameter-sets=%s,%s;packetization-mode=%d;bitrate=%d\r\n",
                                 RTP_MEDIA_PAYLOAD_H264, profile, base64sps, base64pps, my_session->packetization_mode, 48000 /*bitrate*/);
        }
        if(my_session->i_trackid >= 0)
            sdplength += sprintf((char*)(src + sdplength), "a=control:trackID=%d\r\n", my_session->i_trackid);

        /*sdplength += sprintf((char*)(src + sdplength), "a=framesize:%d %d-%d\r\n",
                                     my_session->payload,
                                     my_session->video_width,
                                     my_session->video_height);
        sdplength += sprintf((char*)(src + sdplength), "a=framerate:%u\r\n", my_session->framerate);*/
        //sdplength += sprintf((char*)(src + sdplength), "a=range:npt=now-\r\n"); 
    }
    else
    {
        sdplength = 0;
    }
    return sdplength;
}
/* OK
v=0 
o=- 7227499417 1 IN IP4 192.168.10.1 
s=H.264 Video, streamed by the LIVE555 Media Server 
i=0-0-1970-01-01-00-48-39-video.H264 
t=0 0 
a=tool:LIVE555 Streaming Media v2022.12.01 
a=type:broadcast 
a=control:* 
a=range:npt=now- 
a=x-qt-text-nam:H.264 Video, streamed by the LIVE555 Media Server 
a=x-qt-text-inf:0-0-1970-01-01-00-48-39-video.H264 
m=video 0 RTP/AVP 96 
c=IN IP4 0.0.0.0 
b=AS:500 
a=rtpmap:96 H264/90000 
a=fmtp:96 packetization-mode=1;profile-level-id=42002A;sprop-parameter-sets=Z0IAKpY1QPAET8s3AQEBAg==,aM4xsg== 
a=control:track1 
*/

int zpl_mediartp_session_rtpmap_h264(int keyval, char *src, uint32_t len)
{
    //a=fmtp:96 profile-level-id=000000; sprop-parameter-sets=Z0IAKpY1QPAET8s3AQEBAg==,aM4xsg==; packetization-mode=0
    int ret = 0;
    zpl_mediartp_session_t *my_session = NULL;
    my_session = zpl_mediartp_rtpparam(keyval);
    if (!my_session || !my_session->media_chn)
        return ERROR;
    if (my_session)
    {
        ZPL_MEDIARTP_CHANNEL_LOCK(my_session);
        ret = zpl_mediartp_session_rtpmap_sdptext_h264(my_session, src,  len);
        ZPL_MEDIARTP_CHANNEL_UNLOCK(my_session);
        return ret;
    }
    return ERROR;
}

zpl_mediartp_session_t *zpl_mediartp_session_lookup(int keyval, int channel, int level, char *filename)
{
    zpl_mediartp_session_t *my_session = NULL;
    my_session = zpl_mediartp_rtpparam(keyval);
    return my_session;    
}


int zpl_mediartp_session_remoteport(int keyval, char *address, u_int16_t rtpport, u_int16_t rtcpport)
{
    zpl_mediartp_session_t *my_session = NULL;
    zpl_media_channel_t *mchn = NULL;
    my_session = zpl_mediartp_rtpparam(keyval);
    if (!my_session)
        return ERROR;
    ZPL_MEDIARTP_CHANNEL_LOCK(my_session);    
    mchn = my_session->media_chn; 
    if(my_session) 
    {
        if(ZPL_MEDIARTP_DEBUG(EVENT) && ZPL_MEDIARTP_DEBUG(DETAIL))
        {
            zm_msg_debug("media rtp session keyval %u for %d/%d add remote %s:%d-%d", 
                        my_session->keyval, mchn->channel, mchn->channel_index, address?address:"null",  rtpport,  rtcpport);
        }         
#ifdef ZPL_JRTPLIB_MODULE
        jrtp_session_destination_add(my_session->_session, address,  rtpport,  rtcpport);
#endif
        if(mchn)
            zpl_media_channel_hal_request_IDR(mchn);
        ZPL_MEDIARTP_CHANNEL_UNLOCK(my_session);    
        return OK;
    }   
    ZPL_MEDIARTP_CHANNEL_UNLOCK(my_session);
    return ERROR;    
}

int zpl_mediartp_session_get_localport(int keyval, char *address, u_int16_t *rtpport, u_int16_t *rtcpport)
{
    zpl_mediartp_session_t *my_session = NULL;
    my_session = zpl_mediartp_rtpparam(keyval);
    if (!my_session)
        return ERROR;
    if(my_session) 
    {
        ZPL_MEDIARTP_CHANNEL_LOCK(my_session);
        if(address)
            strcpy(address, my_session->local_address);
        if(rtpport)
            *rtpport = my_session->local_rtpport;
        if(rtcpport)
            *rtcpport = my_session->local_rtcpport;
        ZPL_MEDIARTP_CHANNEL_UNLOCK(my_session);    
        return OK;
    }   
    return ERROR;    
}

int zpl_mediartp_session_localport(int keyval, char *address, u_int16_t rtpport, u_int16_t rtcpport)
{
    zpl_mediartp_session_t *my_session = NULL;
    zpl_media_channel_t *mchn = NULL;
    my_session = zpl_mediartp_rtpparam(keyval);
    if (!my_session)
        return ERROR;
    mchn = my_session->media_chn; 
    if(my_session) 
    {
        ZPL_MEDIARTP_CHANNEL_LOCK(my_session);
        if(ZPL_MEDIARTP_DEBUG(EVENT) && ZPL_MEDIARTP_DEBUG(DETAIL))
        {
            zm_msg_debug("media rtp session keyval %u for %d/%d add local %s:%d-%d", 
                        my_session->keyval, mchn->channel, mchn->channel_index, address?address:"null",  rtpport,  rtcpport);
        }         
        if(address)
            strcpy(my_session->local_address, address);
        if(rtpport)
            my_session->local_rtpport = rtpport;
        if(rtcpport)
            my_session->local_rtcpport = rtcpport;
        ZPL_MEDIARTP_CHANNEL_UNLOCK(my_session);    
        return OK;
    }   
    return ERROR;    
}

int zpl_mediartp_session_tcp_interleaved(int keyval, int rtp_interleaved, int rtcp_interleaved)
{
    zpl_mediartp_session_t *my_session = NULL;
    zpl_media_channel_t *mchn = NULL;
    my_session = zpl_mediartp_rtpparam(keyval);
    if (!my_session)
        return ERROR;
    mchn = my_session->media_chn; 
    if(my_session) 
    {
        ZPL_MEDIARTP_CHANNEL_LOCK(my_session);
        if(ZPL_MEDIARTP_DEBUG(EVENT) && ZPL_MEDIARTP_DEBUG(DETAIL))
        {
            zm_msg_debug("media rtp session keyval %u for %d/%d set tcp interleaved %d-%d", 
                        my_session->keyval, mchn->channel, mchn->channel_index, rtp_interleaved, rtcp_interleaved);
        } 
        my_session->rtp_interleaved = rtp_interleaved;
        my_session->rtcp_interleaved = rtcp_interleaved;
        ZPL_MEDIARTP_CHANNEL_UNLOCK(my_session);    
        return OK;
    }   
    return ERROR; 
}

int zpl_mediartp_session_get_tcp_interleaved(int keyval, int *rtp_interleaved, int *rtcp_interleaved)
{
    zpl_mediartp_session_t *my_session = NULL;
    my_session = zpl_mediartp_rtpparam(keyval);
    if (!my_session)
        return ERROR;
    if(my_session) 
    {
        ZPL_MEDIARTP_CHANNEL_LOCK(my_session);
        if(rtp_interleaved)
            *rtp_interleaved = my_session->rtp_interleaved;
        if(rtcp_interleaved)
            *rtcp_interleaved = my_session->rtcp_interleaved;
        ZPL_MEDIARTP_CHANNEL_UNLOCK(my_session);    
        return OK;
    }   
    return ERROR;    
}

int zpl_mediartp_session_get_trackid(int keyval, int *trackid)
{
    zpl_mediartp_session_t *my_session = NULL;
    my_session = zpl_mediartp_rtpparam(keyval);
    if (!my_session)
        return ERROR;
    if(my_session) 
    {
        ZPL_MEDIARTP_CHANNEL_LOCK(my_session);
        if(trackid)
            *trackid = my_session->i_trackid;
        ZPL_MEDIARTP_CHANNEL_UNLOCK(my_session);    
        return OK;
    }   
    return ERROR;    
}

int zpl_mediartp_session_describe(int keyval, void *pUser, char *src, int *len)
{
    int sdplength = 0;
    sdplength = zpl_mediartp_session_rtpmap_h264(keyval, src, *len);
    //sdplength = rtsp_session_media_build_sdptext(session, src);
    //if(session->bind_other_session.media_chn)
    //    sdplength += rtsp_session_media_build_sdptext(&session->bind_other_session, (char*)(src + sdplength));
    if(len)
        *len = sdplength;
    return OK;
}

int zpl_mediartp_session_setup(int keyval, int channel, int level, char *filename, void *pUser)
{    
    zpl_mediartp_session_t *my_session = NULL;
    zpl_media_channel_t *mchn = NULL;
    my_session = zpl_mediartp_rtpparam(keyval);
    if (!my_session)
        return ERROR;
    mchn = my_session->media_chn; 
    if(my_session) 
    {
        ZPL_MEDIARTP_CHANNEL_LOCK(my_session);
        if(ZPL_MEDIARTP_DEBUG(EVENT) && ZPL_MEDIARTP_DEBUG(DETAIL))
        {
            zm_msg_debug("media rtp session keyval %u for %d/%d setup rtp session", 
                        my_session->keyval, mchn->channel, mchn->channel_index);
        } 
        if(zpl_mediartp_session_connect(my_session, channel, level) != OK)
        {
            ZPL_MEDIARTP_CHANNEL_UNLOCK(my_session); 
            return ERROR; 
        }
    #ifdef ZPL_JRTPLIB_MODULE
        if(!jrtp_session_isactive(my_session->_session))
        {
            if (jrtp_session_create(my_session->_session) != OK)
            {
                ZPL_MEDIARTP_CHANNEL_UNLOCK(my_session); 
                return ERROR; 
            }
        }
    #endif
        ZPL_MEDIARTP_CHANNEL_UNLOCK(my_session); 
        return OK;
    }
    return ERROR;
}




int zpl_mediartp_session_show(struct vty *vty)
{
    NODE node;
    zpl_mediartp_session_t *my_session = NULL;
    if (_mediaRtpSched.mutex)
        os_mutex_lock(_mediaRtpSched.mutex, OS_WAIT_FOREVER);

	for (my_session = (zpl_mediartp_session_t *)lstFirst(&_mediaRtpSched.list);
		 my_session != NULL; my_session = (zpl_mediartp_session_t *)lstNext(&node))
	{
		node = my_session->node;
        if(my_session)
        {
            vty_out(vty, " media rtp session %d%s", lstCount(&_mediaRtpSched.list), VTY_NEWLINE);
            vty_out(vty, "      keyval             :%u%s", my_session->keyval, VTY_NEWLINE);
            vty_out(vty, "      channel            :%d/%d%s", my_session->channel, my_session->level,VTY_NEWLINE);
            vty_out(vty, "      payload            :%u%s", my_session->payload, VTY_NEWLINE);
            vty_out(vty, "      framerate          :%u%s", my_session->framerate, VTY_NEWLINE);
            vty_out(vty, "      local              :%d-%d%s", my_session->local_rtpport, my_session->local_rtcpport, VTY_NEWLINE);
            //vty_out(vty, "      client call        :%u%s", my_session->_call_index, VTY_NEWLINE);
            vty_out(vty, "      packetization mode :%u%s", my_session->packetization_mode, VTY_NEWLINE);
            vty_out(vty, "      user timestamp     :%u%s", my_session->user_timestamp, VTY_NEWLINE);
            vty_out(vty, "      timestamp interval :%u%s", my_session->timestamp_interval, VTY_NEWLINE);
        }
	} 
    if (_mediaRtpSched.mutex)
        os_mutex_unlock(_mediaRtpSched.mutex);
    return 0;    
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


// #endif /* ZPL_LIBORTP_MODULE */