/*
 * Copyright © 2001-2013 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <ortp/ortp.h>

#include "zpl_rtsp.h"
#include "zpl_rtsp_util.h"
#include "zpl_rtsp_transport.h"
#include "zpl_rtsp_sdp.h"
#include "zpl_rtsp_sdpfmtp.h"
#include "zpl_rtsp_base64.h"
#include "zpl_rtsp_auth.h"
#include "zpl_rtsp_session.h"
#include "zpl_rtsp_client.h"
#include "zpl_rtsp_media.h"
#include "zpl_rtsp_adap.h"
#include "zpl_rtsp_rtp.h"
#include "zpl_rtsp_server.h"

#define rtsp_srv_getptr(m)             (((rtsp_srv_t*)m))

static int rtsp_media_srv_setup(rtsp_session_t *session);

int rtsp_media_destroy(rtsp_session_t *session)
{
    if(!session->bsrv)
        return 0;
    rtsp_media_start(session, false);
    rtsp_media_update(session, false);
    if(session->parent && RTSP_DEBUG_FLAG(rtsp_srv_getptr(session->parent)->debug, EVENT))
    {
        rtsp_log_debug("RTSP Stop Media for %d/%d or %s", session->mchannel, session->mlevel, session->mfilepath?session->mfilepath:"nil");
    }
    if(zpl_media_channel_islocalfile(session->mchannel, session->mlevel, session->mfilepath))
    {
        if(zpl_media_channel_bindcount_get(session->mchannel, session->mlevel, session->mfilepath) < 1)
            zpl_media_channel_destroy(session->mchannel, session->mlevel, session->mfilepath);
        return OK;
    }
    return 0;
}


int rtsp_media_update(rtsp_session_t * session, bool add)
{
    if(!session->bsrv)
        return 0;
    zpl_media_channel_bindcount_set(session->mchannel, session->mlevel, session->mfilepath, add);
    return 0;
}

char *rtsp_media_name(int channel, int level)
{
    static char tmpbuf[32];
    memset(tmpbuf, 0, sizeof(tmpbuf));
    sprintf(tmpbuf, "/media/channel=%d?level=%d", channel, level);
    return tmpbuf;
}


int rtsp_media_lookup(rtsp_session_t * session, int channel, int level, const char *path)
{
    if(!session->bsrv)
        return 0;
    zpl_media_channel_t * chn = zpl_media_channel_lookup( channel,  level, path);
    if(chn == NULL)
    {
        if(zpl_media_channel_create(channel, level, path) == OK)
        {
            chn = zpl_media_channel_lookup(channel, level, path);
        }
    }    
    if(chn)
    {
            if(zpl_media_channel_isvideo(session->mchannel, session->mlevel, session->mfilepath))
            {
                //zpl_media_file_get_frame_callback(chn->video_media.halparam, zpl_media_file_get_frame_h264);
                if(session->parent && RTSP_DEBUG_FLAG(rtsp_srv_getptr(session->parent)->debug, EVENT))
                {
                    rtsp_log_debug("RTSP Media is Video for %d/%d or %s", session->mchannel, session->mlevel, session->mfilepath?session->mfilepath:"nil");
                }
            }
            else if(zpl_media_channel_isaudio(session->mchannel, session->mlevel, session->mfilepath))
            {
                //zpl_media_file_get_frame_callback(chn->audio_media.halparam, zpl_media_file_get_frame_h264);
                if(session->parent && RTSP_DEBUG_FLAG(rtsp_srv_getptr(session->parent)->debug, EVENT))
                {
                    rtsp_log_debug("RTSP Media is Audio for %d/%d or %s", session->mchannel, session->mlevel, session->mfilepath?session->mfilepath:"nil");
                }
            }
        rtsp_media_srv_setup(session);
        return 1;
    }
    if(session->parent && RTSP_DEBUG_FLAG(rtsp_srv_getptr(session->parent)->debug, EVENT))
    {
        rtsp_log_debug("RTSP Can not lookup Media for %d/%d or %s", session->mchannel, session->mlevel, session->mfilepath?session->mfilepath:"nil");
    }   
    return 0;
}

static int rtsp_media_codec2pt(rtsp_session_t *session)
{
    if(zpl_media_channel_isvideo(session->mchannel, session->mlevel, session->mfilepath))
    {
        zpl_video_codec_t pcodec;
        if (zpl_media_channel_video_codec_get(session->mchannel, session->mlevel, session->mfilepath, &pcodec) == OK)
        {
            session->video_session.payload = pcodec.enctype;
            /* 更新帧率和发包时间间隔 */
            session->video_session.framerate = pcodec.framerate;
            session->video_session.frame_delay_msec = 1000/session->video_session.framerate;
            session->video_session.timestamp_interval = rtp_profile_get_clock_rate(session->video_session.payload)/session->video_session.framerate;
            if(session->parent && RTSP_DEBUG_FLAG(rtsp_srv_getptr(session->parent)->debug, EVENT))
            {
                rtsp_log_debug("RTSP Media get Video timestamp=%d for %d/%d or %s", 
                    session->video_session.timestamp_interval, session->mchannel, session->mlevel, session->mfilepath?session->mfilepath:"nil");
            } 
        }
    }
    if(zpl_media_channel_isaudio(session->mchannel, session->mlevel, session->mfilepath))
    {
        zpl_audio_codec_t paudiocodec;
        if (zpl_media_channel_audio_codec_get(session->mchannel, session->mlevel, session->mfilepath, &paudiocodec) == OK)
        {
            session->audio_session.payload = paudiocodec.enctype;
            /* 更新帧率和发包时间间隔 */
            session->audio_session.framerate = paudiocodec.framerate;
            session->audio_session.frame_delay_msec = 1000/session->audio_session.framerate;
            session->audio_session.timestamp_interval = rtp_profile_get_clock_rate(session->audio_session.payload)/session->audio_session.framerate;
            if(session->parent && RTSP_DEBUG_FLAG(rtsp_srv_getptr(session->parent)->debug, EVENT))
            {
                rtsp_log_debug("RTSP Media get Audio timestamp=%d for %d/%d or %s", 
                    session->video_session.timestamp_interval, session->mchannel, session->mlevel, session->mfilepath?session->mfilepath:"nil");
            } 
        }
    }
    return 0;
}


static int rtsp_media_srv_setup(rtsp_session_t *session)
{
    int ret = 0;

    rtsp_media_codec2pt(session);

    if(zpl_media_channel_islocalfile(session->mchannel, session->mlevel, session->mfilepath))
    {
        if(zpl_media_channel_isvideo(session->mchannel, session->mlevel, session->mfilepath))
        {
            zpl_media_channel_update_interval(session->mchannel, session->mlevel, session->mfilepath, session->video_session.frame_delay_msec);
        }
        else if(zpl_media_channel_isaudio(session->mchannel, session->mlevel, session->mfilepath))
        {
            zpl_media_channel_update_interval(session->mchannel, session->mlevel, session->mfilepath, session->audio_session.frame_delay_msec);
        }
        zpl_media_channel_client_add(session->mchannel, session->mlevel, session->mfilepath, rtsp_media_rtp_sendto, session);
    }
    return ret;
}


int rtsp_media_extradata_get(rtsp_session_t * session, int channel, int level, const char *path, void *p)
{
    return 0;
}

int rtsp_media_start(rtsp_session_t* session, bool start)
{
    if(session->audio_session.b_enable &&
            session->audio_session.rtp_state != RTP_SESSION_STATE_START &&
            session->audio_session.rtp_session)
    {
        if(session->parent && RTSP_DEBUG_FLAG(rtsp_srv_getptr(session->parent)->debug, EVENT))
        {
            rtsp_log_debug("RTSP Media Start Audio for %d/%d or %s", session->mchannel, session->mlevel, session->mfilepath?session->mfilepath:"nil");
        } 
        session->audio_session.rtp_state = start?RTP_SESSION_STATE_START:RTP_SESSION_STATE_STOP;
    }
    if(session->video_session.b_enable &&
            session->video_session.rtp_state != RTP_SESSION_STATE_START &&
            session->video_session.rtp_session)
    {
        session->video_session.rtp_state = start?RTP_SESSION_STATE_START:RTP_SESSION_STATE_STOP;
        if(session->parent && RTSP_DEBUG_FLAG(rtsp_srv_getptr(session->parent)->debug, EVENT))
        {
            rtsp_log_debug("RTSP Media Start Video for %d/%d or %s", session->mchannel, session->mlevel, session->mfilepath?session->mfilepath:"nil");
        } 
    }
    if(session->bsrv)
    {
        if(zpl_media_channel_islocalfile(session->mchannel, session->mlevel, session->mfilepath))
        {
            return zpl_media_channel_start(session->mchannel, session->mlevel, session->mfilepath);
        }
    }
    return 0;
}


int rtsp_media_rtp_sendto(zpl_media_channel_t *mediachn, 
                          const zpl_skbuffer_t *bufdata,  void *pVoidUser)
{
    int ret = 0;
    rtsp_session_t *session = pVoidUser;
    zpl_media_hdr_t *media_header = bufdata->skb_hdr.other_hdr;
    if(session->parent && RTSP_DEBUG_FLAG(rtsp_srv_getptr(session->parent)->debug, RTPSEND) && 
        RTSP_DEBUG_FLAG(rtsp_srv_getptr(session->parent)->debug, DETAIL))
    {
        //rtsp_log_debug("RTSP Media Rtp Sendto for %d/%d or %s", session->mchannel, session->mlevel, session->mfilepath?session->mfilepath:"nil");
    } 
    ret = rtsp_media_adap_rtp_sendto(media_header->frame_codec, pVoidUser, media_header->frame_type,
                                     ZPL_SKB_DATA(bufdata), ZPL_SKB_DATA_LEN(bufdata));
    //ret = _rtp_send_h264(pVoidUser, 1, bufdata->frame_data, bufdata->frame_len);

    //session->video_session.user_timestamp += 3600;
    if(media_header->frame_type == ZPL_MEDIA_VIDEO)
        session->video_session.user_timestamp += session->video_session.timestamp_interval;
    else
        session->audio_session.user_timestamp += session->audio_session.timestamp_interval;
    return ret;
}

int rtsp_media_tcp_forward(rtsp_session_t* session, const uint8_t *buffer, uint32_t len)
{
    return rtsp_rtp_tcp_forward(session, buffer,  len);
}

int rtsp_media_rtp_recv(rtsp_session_t* session, bool bvideo, zpl_skbuffer_t *bufdata)
{
    int havemore = 1 , ret = 0;
    rtp_media_pt pt = 0;
    uint8_t pbuffer[MAX_RTP_PAYLOAD_LENGTH + 8];
    zpl_media_hdr_t *media_header = bufdata->skb_hdr.other_hdr;
    if(bvideo)
        pt = session->video_session.payload;
    else
        pt = session->audio_session.payload;
    while (havemore)
    {
        ret = rtsp_rtp_recv(session, pbuffer + 4, MAX_RTP_PAYLOAD_LENGTH, bvideo, &havemore);

        if (ret>0){
            rtsp_media_adap_rtp_unpacket(pt, bufdata, pbuffer + 4, ret);
        }
        if (havemore)
        {
            //fprintf(stdout, " ======================= rtsp_rtp_recv: havemore=%i\n",havemore);
            //fflush(stdout);
        }
    }
    //ortp_message("==================================bufdata->frame_len=%d", bufdata->frame_len);

    if(bvideo && bufdata->skb_len)
    {
        media_header->frame_type = ZPL_MEDIA_VIDEO;        //音频视频
        media_header->frame_codec = session->video_session.payload;       //编码类型
        //bufdata->frame_key = 0;         //帧类型
        //bufdata->frame_priv = 0;
        //bufdata->frame_timetick = 0;    //时间戳 毫秒
        //bufdata->frame_seq = 0;         //序列号 底层序列号
        session->video_session.user_timestamp += session->video_session.timestamp_interval;
    }
    else if(!bvideo && bufdata->skb_len)
    {
        media_header->frame_type = ZPL_MEDIA_AUDIO;        //音频视频
        media_header->frame_codec = session->audio_session.payload;       //编码类型
        //bufdata->frame_key = 0;         //帧类型
        //bufdata->frame_priv = 0;
        //bufdata->frame_timetick = 0;    //时间戳 毫秒
        //bufdata->frame_seq = 0;         //序列号 底层序列号
        session->audio_session.user_timestamp += session->audio_session.timestamp_interval;
    }
    else
    {
        if(bvideo)
            session->video_session.user_timestamp += session->video_session.timestamp_interval;//rtp_session_get_current_recv_ts(session->video_session.rtp_session);
        else
            session->audio_session.user_timestamp += session->audio_session.timestamp_interval;//rtp_session_get_current_recv_ts(session->audio_session.rtp_session);
    }
    if(media_header->frame_type == ZPL_MEDIA_AUDIO)
    {
        if(session->parent && RTSP_DEBUG_FLAG(rtsp_srv_getptr(session->parent)->debug, RTPRECV) && 
            RTSP_DEBUG_FLAG(rtsp_srv_getptr(session->parent)->debug, DETAIL))
        {
            rtsp_log_debug("RTSP Media Rtp Recv Audio %d byte timestamp=%d for %d/%d or %s", bufdata->skb_len, 
                session->audio_session.user_timestamp, session->mchannel, session->mlevel, session->mfilepath?session->mfilepath:"nil");
        } 
    }
    else if(media_header->frame_type == ZPL_MEDIA_VIDEO)
    {
        if(session->parent && RTSP_DEBUG_FLAG(rtsp_srv_getptr(session->parent)->debug, RTPRECV) && 
            RTSP_DEBUG_FLAG(rtsp_srv_getptr(session->parent)->debug, DETAIL))
        {
            rtsp_log_debug("RTSP Media Rtp Recv Video %d byte timestamp=%d for %d/%d or %s", bufdata->skb_len, 
                session->video_session.user_timestamp, session->mchannel, session->mlevel, session->mfilepath?session->mfilepath:"nil");
        } 
    }
    return bufdata->skb_len;

}



static int _rtsp_media_fmtp_attr_analysis(struct sdp_media *m, rtsp_session_t *session, void *pUser, bool bvideo)
{
    uint32_t step = 0;
    char *rtpmapstr = NULL;
    rtp_session_t *rtpsession = session->_rtpsession;
    struct sdp_rtpmap rtpmap;
    if(m)
    {
        zpl_client_media_t *client_media = pUser;

        char *attrval = NULL;
        memset(&rtpmap, 0, sizeof(struct sdp_rtpmap));

        attrval = sdp_attr_find(m->attr, m->attr_count, "control", NULL);
        if(attrval)
        {
            if(strstr(attrval, "trackID="))
                rtpsession->i_trackid = sdp_get_intcode(attrval,"trackID=");
        }
        attrval = sdp_attr_find(m->attr, m->attr_count, "framerate", NULL);
        if(attrval)
        {
            rtpsession->framerate = atoi(attrval);
            if(client_media && bvideo)
                client_media->video_codec.framerate = rtpsession->framerate;
            else if(client_media && !bvideo)
                client_media->audio_codec.framerate = rtpsession->framerate;
        }
        if(bvideo)
        {
            //framesize:96 640-480
            attrval = sdp_attr_find(m->attr, m->attr_count, "framesize", NULL);
            if(attrval)
            {
                if(client_media && bvideo)
                    sscanf(attrval, "%d %d-%d", &step, &client_media->video_codec.vidsize.width,
                           &client_media->video_codec.vidsize.height);
            }
        }

        if(m->desc.fmt[0] && m->desc.fmt_count)
            rtpsession->payload = atoi(m->desc.fmt[0]);

        attrval = sdp_attr_find(m->attr, m->attr_count, "recvonly", NULL);
        if(attrval)
            rtpsession->rtpmode = RTP_SESSION_SENDONLY;
        else
        {
            attrval = sdp_attr_find(m->attr, m->attr_count, "sendonly", NULL);
            if(attrval)
                rtpsession->rtpmode = RTP_SESSION_RECVONLY;
            else
            {
                rtpsession->rtpmode = RTP_SESSION_SENDRECV;
            }
        }
        step = 0;
        while(1)
        {
            rtpmapstr = sdp_attr_find_value(m->attr, m->attr_count, "rtpmap", &step);
            if(rtpmapstr)
            {
                step++;
                memset(&rtpmap, 0, sizeof(struct sdp_rtpmap));
                sdp_attr_rtpmap_get(rtpmapstr, &rtpmap);
                if(rtpsession->payload == rtpmap.pt)
                {
                    if(strstr(rtpmap.enc_name, "H264"))
                    {
                        uint32_t astep = 0;
                        while(1)
                        {
                            attrval = sdp_attr_find_value(m->attr, m->attr_count, "fmtp", &astep);
                            if(attrval)
                            {
                                int format = atoi(attrval);
                                if(format == rtpmap.pt)
                                {
                                    if(rtpmap.pt != RTP_MEDIA_PAYLOAD_H264)
                                        rtp_profile_payload_update(rtpmap.pt, &payload_type_h264);

                                    rtsp_media_adap_parse_sdp(format, session, (uint8_t*)attrval, strlen(attrval));
                                    break;
                                }
                            }
                            else
                            {
                                return ERROR;
                            }
                        }
                    }

                    if(rtpmap.clock_rate)
                    {
                        //rtpsession->payload = atoi(attrval);
                        if(client_media && bvideo)
                            client_media->video_codec.enctype = rtpsession->payload;
                        else if(client_media && !bvideo)
                            client_media->audio_codec.enctype = rtpsession->payload;

                        if(rtpsession->framerate)
                            rtpsession->timestamp_interval = rtpmap.clock_rate/rtpsession->framerate;
                    }
                    break;
                }
                else
                {
                    return ERROR;
                }
            }
            else
            {
                return ERROR;
            }
        }
        if(rtpsession->timestamp_interval == 0 && rtpsession->framerate)
            rtpsession->timestamp_interval = rtp_profile_get_clock_rate(rtpsession->payload)/rtpsession->framerate;

        RTSP_DEBUG_TRACE(" %s        :%d\r\n", bvideo?"video":"audio", rtpsession->b_enable);
        RTSP_DEBUG_TRACE(" trackid      :%d\r\n", rtpsession->i_trackid);
        RTSP_DEBUG_TRACE(" payload      :%d\r\n", rtpsession->payload);
        RTSP_DEBUG_TRACE(" interval     :%d\r\n", rtpsession->timestamp_interval);
        RTSP_DEBUG_TRACE(" pt           :%d\r\n", rtpmap.pt);
        RTSP_DEBUG_TRACE(" enc_name     :%s\r\n", rtpmap.enc_name);
        RTSP_DEBUG_TRACE(" clock_rate   :%d\r\n", rtpmap.clock_rate);
        if(client_media && bvideo)
            RTSP_DEBUG_TRACE(" size         :%dx%d\r\n", client_media->video_codec.vidsize.width, client_media->video_codec.vidsize.height);
        RTSP_DEBUG_TRACE(" framerate    :%d\r\n", rtpsession->framerate);
    }
    return 0;
}


static int rtsp_media_attr_analysis(rtsp_session_t* session, void *pUser)
{
    int ret = 0;
    if(!session->bsrv)
    {
        session->video_session.rtpmode = RTP_SESSION_SENDRECV;
        session->audio_session.rtpmode = RTP_SESSION_SENDRECV;

        struct sdp_media *m = sdp_media_find(session->sdptext.media,
                                             session->sdptext.media_count, "video", NULL);
        if(m)
        {
            session->_rtpsession = &session->video_session;
            session->video_session.b_enable = true;
            ret = _rtsp_media_fmtp_attr_analysis(m, session, pUser, true);
        }

        m = sdp_media_find(session->sdptext.media,
                           session->sdptext.media_count, "audio", NULL);
        if(m)
        {
            session->_rtpsession = &session->audio_session;
            session->audio_session.b_enable = true;
            ret = _rtsp_media_fmtp_attr_analysis(m, session, pUser, false);
        }
    }
    else
    {
        session->video_session.rtpmode = RTP_SESSION_SENDRECV;
        session->audio_session.rtpmode = RTP_SESSION_SENDRECV;
    }
    return ret;
}






int rtsp_media_build_sdptext(rtsp_session_t * session, uint8_t *sdp)
{
    int sdplength = 0, ret = 0;
    zpl_video_codec_t pcodec;
    if(session->video_session.b_enable)
    {
        sdplength = sprintf((char*)sdp + sdplength, "m=video 0 RTP/AVP %d\r\n", session->video_session.payload);
        ret = rtsp_media_adap_build_sdp(session->video_session.payload, session, sdp + sdplength, 0);
        if(ret == 0)
        {
            
            if (rtp_profile_get_rtpmap(session->video_session.payload))
            {
                sdplength += sprintf(sdp + sdplength, "a=rtpmap:%d %s\r\n", session->video_session.payload,
                                     rtp_profile_get_rtpmap(session->video_session.payload));
                
                if (zpl_media_channel_video_codec_get(session->mchannel, session->mlevel, session->mfilepath, &pcodec) == OK)
                {
                    sdplength += sprintf((char*)sdp + sdplength, "a=framesize:%d %d-%d\r\n",
                                     session->video_session.payload,
                                     pcodec.vidsize.width,
                                     pcodec.vidsize.height);
                }
                sdplength += sprintf((char*)sdp + sdplength, "a=framerate:%u\r\n", session->video_session.framerate);

                if(zpl_media_channel_islocalfile(session->mchannel, session->mlevel, session->mfilepath))
                {
                    zpl_media_channel_update_interval(session->mchannel, session->mlevel, session->mfilepath, 1000/session->video_session.framerate);

                    sdplength += sprintf((char*)sdp + sdplength, "a=range:npt=now-\r\n");
                }
                else
                {
                    sdplength += sprintf((char*)sdp + sdplength, "a=range:npt=now-\r\n");
                }
            }
        }
        else
        {
            sdplength += ret;
            if (zpl_media_channel_video_codec_get(session->mchannel, session->mlevel, session->mfilepath, &pcodec) == OK)
            {
                sdplength += sprintf((char*)sdp + sdplength, "a=framesize:%d %d-%d\r\n",
                                     session->video_session.payload,
                                     pcodec.vidsize.width,
                                     pcodec.vidsize.height);
            }
            sdplength += sprintf((char*)sdp + sdplength, "a=framerate:%u\r\n", session->video_session.framerate);
            sdplength += sprintf((char*)sdp + sdplength, "a=range:npt=now-\r\n");
        }
        //sdplength += rtsp_server_build_sdp_video(session, sdp + sdplength);
    }
    if(session->audio_session.b_enable)
    {
        sdplength = sprintf((char*)sdp + sdplength, "m=audio 0 RTP/AVP %d\r\n", session->audio_session.payload);
        ret = rtsp_media_adap_build_sdp(session->audio_session.payload, session, sdp + sdplength, 0);
        if(ret == 0)
        {
            if (rtp_profile_get_rtpmap(session->audio_session.payload))
            {
                sdplength += sprintf(sdp + sdplength, "a=rtpmap:%d %s\r\n", session->audio_session.payload,
                                     rtp_profile_get_rtpmap(session->audio_session.payload));
                sdplength += sprintf((char*)sdp + sdplength, "a=framerate:%d\r\n", session->audio_session.framerate);
                sdplength += sprintf((char*)sdp + sdplength, "a=range:npt=now-\r\n");
            }
        }
        else
        {
            sdplength += ret;
            sdplength += sprintf((char*)sdp + sdplength, "a=framerate:%d\r\n", session->audio_session.framerate);
            sdplength += sprintf((char*)sdp + sdplength, "a=range:npt=now-\r\n");
        }
        //sdplength += rtsp_server_build_sdp_audio(session, sdp + sdplength);
    }
    return sdplength;
}




rtsp_code rtsp_media_handle_option(rtsp_session_t * session, void *pUser)
{
    rtsp_rtp_handle_options(session);
    if(session->bsrv)
        return RTSP_STATE_CODE_200;
    else
        return RTSP_STATE_CODE_200;
}


rtsp_code rtsp_media_handle_describe(rtsp_session_t * session, void *pUser)
{
    if(rtsp_media_attr_analysis(session, pUser) != 0)
    {
        return RTSP_STATE_CODE_300;
    }

    rtsp_rtp_handle_describe(session);
    if(session->bsrv)
        return RTSP_STATE_CODE_200;
    else
        return RTSP_STATE_CODE_200;
}

rtsp_code rtsp_media_handle_setup(rtsp_session_t * session, void *pUser)
{
    rtsp_rtp_handle_setup(session);
    if(session->bsrv)
        return RTSP_STATE_CODE_200;
    else
        return RTSP_STATE_CODE_200;
}

rtsp_code rtsp_media_handle_teardown(rtsp_session_t * session, void *pUser)
{
    rtsp_rtp_handle_teardown(session);
    if(session->bsrv)
        return RTSP_STATE_CODE_200;
    else
        return RTSP_STATE_CODE_200;
}

rtsp_code rtsp_media_handle_play(rtsp_session_t * session, void *pUser)
{
    rtsp_media_start(session, true);
    rtsp_rtp_handle_play(session);
    if(session->bsrv)
        return RTSP_STATE_CODE_200;
    else
        return RTSP_STATE_CODE_200;
}

rtsp_code rtsp_media_handle_pause(rtsp_session_t * session, void *pUser)
{
    rtsp_media_start(session, false);
    rtsp_rtp_handle_pause(session);
    if(session->bsrv)
        return RTSP_STATE_CODE_200;
    else
        return RTSP_STATE_CODE_200;
}

rtsp_code rtsp_media_handle_scale(rtsp_session_t * session, void *pUser)
{
    rtsp_rtp_handle_scale(session);
    if(session->bsrv)
        return RTSP_STATE_CODE_200;
    else
        return RTSP_STATE_CODE_200;
}

rtsp_code rtsp_media_handle_get_parameter(rtsp_session_t * session, void *pUser)
{
    rtsp_rtp_handle_get_parameter(session);
    if(session->bsrv)
        return RTSP_STATE_CODE_200;
    else
        return RTSP_STATE_CODE_200;
}

rtsp_code rtsp_media_handle_set_parameter(rtsp_session_t * session, void *pUser)
{
    rtsp_rtp_handle_set_parameter(session);
    if(session->bsrv)
        return RTSP_STATE_CODE_200;
    else
        return RTSP_STATE_CODE_200;
}
