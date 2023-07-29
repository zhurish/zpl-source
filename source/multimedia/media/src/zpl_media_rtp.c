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

#ifdef ZPL_JRTPLIB_MODULE
static int zpl_mediartp_event_dispatch_signal(zpl_mediartp_session_t *my_session);
#endif
static zpl_mediartp_scheduler_t _mediaRtpSched;

static zpl_mediartp_session_adap_t _rtp_media_adap_tbl[] =
    {
        {"H264", RTP_MEDIA_PAYLOAD_H264, rtp_payload_send_h264, NULL},
        {"G711A", RTP_MEDIA_PAYLOAD_G711A, rtp_payload_send_h264, NULL},
        {"G711U", RTP_MEDIA_PAYLOAD_G711U, rtp_payload_send_g7xx, NULL},

        {"G722", RTP_MEDIA_PAYLOAD_G722, rtp_payload_send_g7xx, NULL},
        {"G726", RTP_MEDIA_PAYLOAD_G726, rtp_payload_send_g7xx, NULL},

        {"G728", RTP_MEDIA_PAYLOAD_G728, rtp_payload_send_g7xx, NULL},
        {"G729", RTP_MEDIA_PAYLOAD_G729, rtp_payload_send_g7xx, NULL},
        {"G729A", RTP_MEDIA_PAYLOAD_G729A, rtp_payload_send_g7xx, NULL},

        {"PCMA", RTP_MEDIA_PAYLOAD_PCMA, rtp_payload_send_g7xx, NULL},
        {"PCMU", RTP_MEDIA_PAYLOAD_PCMU, rtp_payload_send_g7xx, NULL},
};

static int zpl_mediartp_session_adap_rtp_sendto(uint32_t codec, zpl_mediartp_session_t *session, const uint8_t *buf, uint32_t len)
{
    uint32_t i = 0;
    zpl_mediartp_session_t *my_session = session;

    if (my_session && my_session->_session)
    {
        for (i = 0; i < sizeof(_rtp_media_adap_tbl) / sizeof(_rtp_media_adap_tbl[0]); i++)
        {
            if (_rtp_media_adap_tbl[i].codec == codec)
            {
                if (_rtp_media_adap_tbl[i]._rtp_sendto)
                #ifdef ZPL_JRTPLIB_MODULE
                    return (_rtp_media_adap_tbl[i]._rtp_sendto)(my_session->_session, buf, len, my_session->timestamp_interval);
                #else
                    return (_rtp_media_adap_tbl[i]._rtp_sendto)(my_session->_session, buf, len, my_session->user_timestamp);
                #endif
            }
        }
    }
    return -1;
}

static int zpl_mediartp_session_rtp_clone(zpl_media_channel_t *mediachn,
                                          const zpl_skbuffer_t *bufdata, void *pVoidUser)
{
    int ret = 0;
    zpl_mediartp_session_t *session = pVoidUser;
    zpl_skbuffer_t *skb = NULL;
    if (session->rtp_media_queue)
    {
        skb = zpl_skbuffer_clone(session->rtp_media_queue, bufdata);
        if (skb)
        {
            ret = zpl_skbqueue_add(session->rtp_media_queue, skb);
            #ifdef ZPL_JRTPLIB_MODULE
            zpl_mediartp_event_dispatch_signal(session);
            #endif
        }
    }
    return ret;
}

int zpl_mediartp_session_rtp_sendto(zpl_mediartp_session_t *rtp_session)
{
    int ret = 0;
    if (rtp_session->rtp_media_queue)
    {
        if (rtp_session->b_start == zpl_false)
        {
            zpl_skbuffer_t *skb = zpl_skbqueue_get(rtp_session->rtp_media_queue);
            if(skb)
                zpl_skbqueue_finsh(rtp_session->rtp_media_queue, skb);
            rtp_session->user_timestamp += rtp_session->timestamp_interval;
        }
        else
        {
            zpl_skbuffer_t *skb = zpl_skbqueue_get(rtp_session->rtp_media_queue);
            if (skb)
            {
                zpl_media_hdr_t *media_header = skb->skb_hdr.other_hdr;
                // rtp_session->user_timestamp = (media_header->timetick/1000U)*1000U;
                rtp_session->user_timestamp += rtp_session->timestamp_interval;
                //zm_msg_debug("======== zpl_mediartp_session_rtp_sendto:timetick=%d\r\n", media_header->timetick);
                ret = zpl_mediartp_session_adap_rtp_sendto(media_header->codectype, rtp_session,
                                                           ZPL_SKB_DATA(skb), ZPL_SKB_DATA_LEN(skb));
                zpl_skbqueue_finsh(rtp_session->rtp_media_queue, skb);
            }
        }
    }
    return ret;
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
            zm_msg_debug("======== zpl_mediartp_rtpparam=%p\r\n", my_session);    
            return my_session; 
        }
	} 
    if (_mediaRtpSched.mutex)
        os_mutex_unlock(_mediaRtpSched.mutex);
    zm_msg_error("======== zpl_mediartp_rtpparam=NULL\r\n");      
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

        my_session->_call_index = -1; // 媒体回调索引, 音视频通道数据发送
        //my_session->media_chn = mchn;
        my_session->b_start = zpl_true;
        my_session->packetization_mode = 1; // 封包解包模式
        my_session->user_timestamp = 0;     // 用户时间戳
        my_session->timestamp_interval = 0; // 用户时间戳

        my_session->rtp_media_queue = NULL; // 媒体接收队列

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
        my_session->media_chn = mediachn;

        if (mediachn->media_param.video_media.codec.framerate == 0)
            my_session->framerate = 30;
        else
            my_session->framerate = mediachn->media_param.video_media.codec.framerate;
        my_session->payload = mediachn->media_param.video_media.codec.codectype;
    
        zm_msg_debug("=====================my_session->framerate=%d my_session->payload=%d", my_session->framerate, my_session->payload); 
#ifdef ZPL_JRTPLIB_MODULE
        if(my_session->_session)
        {
            jrtp_session_framerate_set(my_session->_session, my_session->framerate);

            jrtp_session_payload_set(my_session->_session, my_session->payload, jrtp_profile_get_clock_rate(my_session->payload));
        }
        if (my_session->framerate)
        {
            my_session->timestamp_interval = jrtp_profile_get_clock_rate(my_session->payload) / my_session->framerate;
            zm_msg_debug("=====================my_session->timestamp_interval=%d", my_session->timestamp_interval); 
        }
        if(my_session->_session)
            jrtp_session_local_set(my_session->_session, strlen(my_session->local_address)?my_session->local_address:NULL, my_session->local_rtpport, my_session->local_rtcpport);
#endif
        return OK;
    }
    return ERROR;
}

zpl_mediartp_session_t *zpl_mediartp_session_create(int keyval, int channel, int level)
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
        if (_mediaRtpSched.mutex)
            os_mutex_lock(_mediaRtpSched.mutex, OS_WAIT_FOREVER);
#ifdef ZPL_JRTPLIB_MODULE
        if (my_session->_session == NULL)
            my_session->_session = jrtp_session_alloc();
#endif
        if (my_session->_session == NULL)
        {
            free(my_session);
            if (_mediaRtpSched.mutex)
                os_mutex_unlock(_mediaRtpSched.mutex);
            zm_msg_error("=====================jrtp_session_alloc");
            return NULL;
        }
        zpl_mediartp_session_default(my_session);
        my_session->keyval = keyval;
        my_session->channel = channel;
        my_session->level = level;
        my_session->rtp_media_queue = zpl_skbqueue_create(os_name_format("rtpSkbQueue-%x", keyval), ZPL_MEDIA_BUFQUEUE_SIZE, zpl_false);
        if (my_session->rtp_media_queue == NULL)
        {
#ifdef ZPL_JRTPLIB_MODULE
            jrtp_session_stop(my_session->_session);
            jrtp_session_destroy(my_session->_session);
            my_session->_session = NULL;
#endif
            free(my_session);
            if (_mediaRtpSched.mutex)
                os_mutex_unlock(_mediaRtpSched.mutex);
            zm_msg_error("=====================zpl_skbqueue_create");
            return NULL;
        }
        zpl_mediartp_session_connect(my_session, channel, level);

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
    mchn = my_session->media_chn;    
    if (_mediaRtpSched.mutex)
        os_mutex_lock(_mediaRtpSched.mutex, OS_WAIT_FOREVER);

    if (my_session->_call_index > 0)
    {
        zpl_media_channel_client_start(mchn->channel, mchn->channel_index, my_session->_call_index, zpl_false);
        zpl_media_channel_client_del(mchn->channel, mchn->channel_index, my_session->_call_index);
    }
    #ifdef ZPL_JRTPLIB_MODULE
    jrtp_session_stop(my_session->_session);
    if (jrtp_session_destroy(my_session->_session) != OK)
    {
        if (_mediaRtpSched.mutex)
            os_mutex_unlock(_mediaRtpSched.mutex);
        return ERROR;
    }
    my_session->_session = NULL;
    #endif
    my_session->_call_index = -1;
    if (my_session->rtp_media_queue)
    {
        zpl_skbqueue_destroy(my_session->rtp_media_queue);
        my_session->rtp_media_queue = NULL;
    }
    lstDelete(&_mediaRtpSched.list, my_session);
    free(my_session);
    if (_mediaRtpSched.mutex)
        os_mutex_unlock(_mediaRtpSched.mutex);
    return OK;
}

int zpl_mediartp_session_suspend(int keyval)
{
    zpl_mediartp_session_t *my_session = NULL;
    my_session = zpl_mediartp_rtpparam(keyval);
    if (!my_session || !my_session->media_chn)
        return ERROR;
    if (my_session && my_session->_session)
    {
        if (_mediaRtpSched.mutex)
            os_mutex_lock(_mediaRtpSched.mutex, OS_WAIT_FOREVER);
        my_session->b_start = zpl_true;
        if (_mediaRtpSched.mutex)
            os_mutex_unlock(_mediaRtpSched.mutex);
        return OK;
    }
    return ERROR;
}

int zpl_mediartp_session_resume(int keyval)
{
    zpl_mediartp_session_t *my_session = NULL;
    my_session = zpl_mediartp_rtpparam(keyval);
    if (!my_session || !my_session->media_chn)
        return ERROR;
    if (my_session && my_session->_session)
    {
        if (_mediaRtpSched.mutex)
            os_mutex_lock(_mediaRtpSched.mutex, OS_WAIT_FOREVER);
        my_session->b_start = zpl_false;
        if (_mediaRtpSched.mutex)
            os_mutex_unlock(_mediaRtpSched.mutex);
        return OK;
    }
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
    mchn = my_session->media_chn; 
    if (my_session && my_session->_session)
    {
        if (_mediaRtpSched.mutex)
            os_mutex_lock(_mediaRtpSched.mutex, OS_WAIT_FOREVER);
        if (start)
        {
            if (my_session->_call_index < 0)
            {
                my_session->_call_index = zpl_media_channel_client_add(mchn->channel, mchn->channel_index, zpl_mediartp_session_rtp_clone, my_session);
            }            
            if (my_session->_call_index > 0)
            {
                zm_msg_debug("=====================zpl_media_channel_client_start %d", my_session->_call_index); 
                ret = zpl_media_channel_client_start(mchn->channel, mchn->channel_index, my_session->_call_index, zpl_true);
                if (_mediaRtpSched.mutex)
                    os_mutex_unlock(_mediaRtpSched.mutex);
#ifdef ZPL_JRTPLIB_MODULE
                if (jrtp_session_start(my_session->_session) != OK)
                {
                    return ERROR;
                }
#endif
                return ret;
            }
            else
            {
                if (_mediaRtpSched.mutex)
                    os_mutex_unlock(_mediaRtpSched.mutex);
                zm_msg_error("=====================zpl_media_channel_client_start %d", my_session->_call_index);  
                return ERROR;
            }
        }
        else
        {
            if (my_session->_call_index > 0)
            {
                ret = zpl_media_channel_client_start(mchn->channel, mchn->channel_index, my_session->_call_index, zpl_false);
            }
            if (_mediaRtpSched.mutex)
                os_mutex_unlock(_mediaRtpSched.mutex);
#ifdef ZPL_JRTPLIB_MODULE
            if (jrtp_session_start(my_session->_session) != OK)
            {
                return ERROR;
            }
#endif 
            return ret;
        }

        if (_mediaRtpSched.mutex)
            os_mutex_unlock(_mediaRtpSched.mutex);
    }
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
    zpl_mediartp_session_t *my_session = event->event_data;
        if (_mediaRtpSched.mutex)
        os_mutex_lock(_mediaRtpSched.mutex, OS_WAIT_FOREVER);
    //zm_msg_debug("=====================zpl_mediartp_event_handle %p", my_session);
    if (my_session)
    {
        zpl_mediartp_session_rtp_sendto(my_session);
    }
        if (_mediaRtpSched.mutex)
        os_mutex_unlock(_mediaRtpSched.mutex);
    return OK;
}

static int zpl_mediartp_event_dispatch_signal(zpl_mediartp_session_t *my_session)
{
    if (_mediaRtpSched.evtqueue)
    {
        zpl_media_event_register(_mediaRtpSched.evtqueue, ZPL_MEDIA_GLOAL_VIDEO_ENCODE, ZPL_MEDIA_EVENT_DISTPATCH,
                                 zpl_mediartp_event_handle, my_session);
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
            my_session = zpl_mediartp_session_create(ZPL_MEDIARTP_MULTICAST_KEY, channel, channel_index);
            if(my_session)
            {
                zpl_mediartp_session_setup(ZPL_MEDIARTP_MULTICAST_KEY, channel, channel_index, NULL);
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
                if (my_session->_call_index < 0)
                {
                    my_session->_call_index = zpl_media_channel_client_add(channel, channel_index, zpl_mediartp_session_rtp_clone, my_session);
                } 
                if(my_session->_call_index) 
                    zpl_media_channel_client_start(channel, channel_index, my_session->_call_index, zpl_true);
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

static int zpl_mediartp_session_rtpmap_sdptext_h264(zpl_mediartp_session_t* rtpsession, char *src, uint32_t len)
{
    char base64sps[OS_BASE64_DECODE_SIZE(1024)];
    char base64pps[OS_BASE64_DECODE_SIZE(1024)];
    zpl_video_extradata_t extradata;
    int profile = 0, sdplength = 0;
    memset(base64pps, 0, sizeof(base64pps));
    memset(base64sps, 0, sizeof(base64sps));
    memset(&extradata, 0, sizeof(zpl_video_extradata_t));

    if(rtpsession->media_chn)
        zpl_media_channel_extradata_get(rtpsession->media_chn, &extradata);

    profile = extradata.h264spspps.profileLevelId;

    sdplength = sprintf((char*)(src + sdplength), "m=video 0 RTP/AVP %d\r\n", rtpsession->payload);
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
                                 RTP_MEDIA_PAYLOAD_H264, profile, base64sps, base64pps, rtpsession->packetization_mode);
        }
        else
        {
            sdplength += sprintf((char*)(src + sdplength), "a=fmtp:%d profile-level-id=%06x;"
                                                  "sprop-parameter-sets=%s,%s;packetization-mode=%d;bitrate=%d\r\n",
                                 RTP_MEDIA_PAYLOAD_H264, profile, base64sps, base64pps, rtpsession->packetization_mode, 48000 /*bitrate*/);
        }
        if(rtpsession->i_trackid >= 0)
            sdplength += sprintf((char*)(src + sdplength), "a=control:trackID=%d\r\n", rtpsession->i_trackid);

        /*sdplength += sprintf((char*)(src + sdplength), "a=framesize:%d %d-%d\r\n",
                                     rtpsession->payload,
                                     rtpsession->video_width,
                                     rtpsession->video_height);
        sdplength += sprintf((char*)(src + sdplength), "a=framerate:%u\r\n", rtpsession->framerate);*/
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
        if (_mediaRtpSched.mutex)
            os_mutex_lock(_mediaRtpSched.mutex, OS_WAIT_FOREVER);
        ret = zpl_mediartp_session_rtpmap_sdptext_h264(my_session, src,  len);
        if (_mediaRtpSched.mutex)
            os_mutex_unlock(_mediaRtpSched.mutex);
        return ret;
    }
    return ERROR;
}

zpl_mediartp_session_t *zpl_mediartp_session_lookup(int keyval, int channel, int level)
{
    zpl_mediartp_session_t *my_session = NULL;
    my_session = zpl_mediartp_rtpparam(keyval);
    zm_msg_debug("======== zpl_mediartp_session_lookup=%p\r\n", my_session);   
    return my_session;    
}


int zpl_mediartp_session_remoteport(int keyval, char *address, u_int16_t rtpport, u_int16_t rtcpport)
{
    zpl_mediartp_session_t *my_session = NULL;
    zpl_media_channel_t *mchn = NULL;
    my_session = zpl_mediartp_rtpparam(keyval);
    if (!my_session)
        return ERROR;
    mchn = my_session->media_chn; 
    if(my_session) 
    {
#ifdef ZPL_JRTPLIB_MODULE
        jrtp_session_destination_add(my_session->_session, address,  rtpport,  rtcpport);
#endif
        if(mchn)
            zpl_media_channel_hal_request_IDR(mchn);
        return 0;
    }   
    return -1;    
}

int zpl_mediartp_session_get_localport(int keyval, char *address, u_int16_t *rtpport, u_int16_t *rtcpport)
{
    zpl_mediartp_session_t *my_session = NULL;
    my_session = zpl_mediartp_rtpparam(keyval);
    if (!my_session)
        return ERROR;
    if(my_session) 
    {
        if(address)
            strcpy(address, my_session->local_address);
        if(rtpport)
            *rtpport = my_session->local_rtpport;
        if(rtcpport)
            *rtcpport = my_session->local_rtcpport;
        return 0;
    }   
    return -1;    
}

int zpl_mediartp_session_localport(int keyval, char *address, u_int16_t rtpport, u_int16_t rtcpport)
{
    zpl_mediartp_session_t *my_session = NULL;
    my_session = zpl_mediartp_rtpparam(keyval);
    if (!my_session)
        return ERROR;
    if(my_session) 
    {
        if(address)
            strcpy(my_session->local_address, address);
        if(rtpport)
            my_session->local_rtpport = rtpport;
        if(rtcpport)
            my_session->local_rtcpport = rtcpport;
        return 0;
    }   
    return -1;    
}

int zpl_mediartp_session_tcp_interleaved(int keyval, int rtp_interleaved, int rtcp_interleaved)
{
    zpl_mediartp_session_t *my_session = NULL;
    my_session = zpl_mediartp_rtpparam(keyval);
    if (!my_session)
        return ERROR;
    if(my_session) 
    {
        my_session->rtp_interleaved = rtp_interleaved;
        my_session->rtcp_interleaved = rtcp_interleaved;
        return 0;
    }   
    return -1;    
}

int zpl_mediartp_session_get_tcp_interleaved(int keyval, int *rtp_interleaved, int *rtcp_interleaved)
{
    zpl_mediartp_session_t *my_session = NULL;
    my_session = zpl_mediartp_rtpparam(keyval);
    if (!my_session)
        return ERROR;
    if(my_session) 
    {
        if(rtp_interleaved)
            *rtp_interleaved = my_session->rtp_interleaved;
        if(rtcp_interleaved)
            *rtcp_interleaved = my_session->rtcp_interleaved;
        return 0;
    }   
    return -1;    
}

int zpl_mediartp_session_get_trackid(int keyval, int *trackid)
{
    zpl_mediartp_session_t *my_session = NULL;
    my_session = zpl_mediartp_rtpparam(keyval);
    if (!my_session)
        return ERROR;
    if(my_session) 
    {
        if(trackid)
            *trackid = my_session->i_trackid;
        return 0;
    }   
    return -1;    
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

int zpl_mediartp_session_setup(int keyval, int channel, int level, void *pUser)
{    
    zpl_mediartp_session_t *my_session = NULL;
    my_session = zpl_mediartp_rtpparam(keyval);
    if (!my_session)
        return ERROR;
    if(my_session) 
    {
        if(zpl_mediartp_session_connect(my_session, channel, level) != OK)
            return ERROR; 
    #ifdef ZPL_JRTPLIB_MODULE
        if(!jrtp_session_isactive(my_session->_session))
        {
            if (jrtp_session_create(my_session->_session) != OK)
            {
                return ERROR; 
            }
        }
    #endif
        return OK;
    }
    return ERROR;
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
    zpl_media_event_start(_mediaRtpSched.evtqueue);
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