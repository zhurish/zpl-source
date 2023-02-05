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

#define ZPL_RTP_SESSION(n)  ((RtpSession*)(n))

static rtsp_session_media_scheduler _media_scheduler;

#define rtsp_srv_getptr(m)             (((rtsp_srv_t*)m))
static int rtsp_session_media_option_load(rtsp_session_t *session);

static int rtsp_session_media_proxy_eloop(struct eloop *rtsp_media)
{
    rtp_session_t *session = ELOOP_ARG(rtsp_media);
    zpl_socket_t *sock = ELOOP_FD(rtsp_media);
    rtsp_session_t *rtsp_session = NULL;
    char buftmp[1600];
    int ret = 0, already = 0, need_len = 0;
    rtp_tcp_header_t *hdr = (rtp_tcp_header_t *)buftmp;
    if (session && session->rtsp_parent && sock && !ipstack_invalid(sock))
    {
        rtsp_session = (rtsp_session_t *)session->rtsp_parent;
        if(ipstack_same(sock, session->rtp_sock) || ipstack_same(sock, session->rtcp_sock))
        {
            if(ipstack_same(sock, session->rtp_sock))
                session->t_rtp_read = NULL;
            if(ipstack_same(sock, session->rtcp_sock))
                session->t_rtcp_read = NULL;
            while(1)
            {
                if(already < sizeof(rtp_tcp_header_t))  
                {
                    ret = ipstack_recv(sock, buftmp+already, sizeof(rtp_tcp_header_t)-already, 0);
                    if(ret < 0)
                        return ERROR;
                    if((sizeof(rtp_tcp_header_t)-already) == ret)
                    {
                        already += ret;
                    }
                }
                need_len = sizeof(rtp_tcp_header_t) + ntohs(hdr->length);
                if(already < need_len)
                {
                    ret = ipstack_recv(sock, buftmp + already, need_len - already, 0);
                    if(ret)
                        already += ret;
                    if(ret < 0)
                        return ERROR;
                }
                if(need_len == already)
                    break;
            }    

            if(already > 0)
            {
                if (!ipstack_invalid(rtsp_session->sock))
                    ipstack_send(rtsp_session->sock, buftmp, already, 0);
            }  
            if(ipstack_same(sock, session->rtp_sock))  
                session->t_rtp_read = eloop_add_read(rtsp_session->t_master, rtsp_session_media_proxy_eloop, session, sock);
            if(ipstack_same(sock, session->rtcp_sock))  
                session->t_rtcp_read = eloop_add_read(rtsp_session->t_master, rtsp_session_media_proxy_eloop, session, sock);
        }
    }
    return OK;
}
static int rtsp_session_media_overtcp_start(rtsp_session_t *session, zpl_bool bvideo, zpl_bool start)
{
    if(session && session->video_session.b_enable && bvideo)
    {
        if(session->video_session.transport.proto == RTSP_TRANSPORT_RTP_RTPOVERRTSP ||
            session->video_session.transport.proto == RTSP_TRANSPORT_RTP_TCP)
        {
            if (start == zpl_false)
            {
                if(session->video_session.t_rtp_read)
                {
                    ELOOP_OFF(session->video_session.t_rtp_read);
                    session->video_session.t_rtp_read = NULL;
                }
                if(session->video_session.t_rtcp_read)
                {
                    ELOOP_OFF(session->video_session.t_rtcp_read);
                    session->video_session.t_rtcp_read = NULL;
                }
                if(!ipstack_invalid(session->video_session.rtp_sock))
                {
                    ipstack_close(session->video_session.rtp_sock);
                    session->video_session.rtp_sock = ZPL_SOCKET_INVALID;
                }
                if(!ipstack_invalid(session->video_session.rtcp_sock))
                {
                    ipstack_close(session->video_session.rtcp_sock);
                    session->video_session.rtcp_sock = ZPL_SOCKET_INVALID;
                }
            }
            else
            {
                if(!ipstack_invalid(session->video_session.rtp_sock) && session->video_session.t_rtp_read == NULL)
                {
                    session->video_session.t_rtp_read = eloop_add_read(session->t_master, rtsp_session_media_proxy_eloop, &session->video_session, session->video_session.rtp_sock);
                }
                if(!ipstack_invalid(session->video_session.rtcp_sock) && session->video_session.t_rtcp_read == NULL)
                {
                    session->video_session.t_rtcp_read = eloop_add_read(session->t_master, rtsp_session_media_proxy_eloop, &session->video_session, session->video_session.rtcp_sock);
                }
            }
        }
    }
    if(session && session->audio_session.b_enable && bvideo == zpl_false)
    {
        if(session->audio_session.transport.proto == RTSP_TRANSPORT_RTP_RTPOVERRTSP ||
            session->audio_session.transport.proto == RTSP_TRANSPORT_RTP_TCP)
        {
            if (start == zpl_false)
            {
                if(session->audio_session.t_rtp_read)
                {
                    ELOOP_OFF(session->audio_session.t_rtp_read);
                    session->audio_session.t_rtp_read = NULL;
                }
                if(session->audio_session.t_rtcp_read)
                {
                    ELOOP_OFF(session->audio_session.t_rtcp_read);
                    session->audio_session.t_rtcp_read = NULL;
                }
                if(!ipstack_invalid(session->audio_session.rtp_sock))
                {
                    ipstack_close(session->audio_session.rtp_sock);
                    session->audio_session.rtp_sock = ZPL_SOCKET_INVALID;
                }
                if(!ipstack_invalid(session->audio_session.rtcp_sock))
                {
                    ipstack_close(session->audio_session.rtcp_sock);
                    session->audio_session.rtcp_sock = ZPL_SOCKET_INVALID;
                }
            }
            else
            {
                if(!ipstack_invalid(session->audio_session.rtp_sock) && session->audio_session.t_rtp_read == NULL)
                {
                    session->audio_session.t_rtp_read = eloop_add_read(session->t_master, rtsp_session_media_proxy_eloop, &session->audio_session, session->audio_session.rtp_sock);
                }
                if(!ipstack_invalid(session->audio_session.rtcp_sock) && session->audio_session.t_rtcp_read == NULL)
                {
                    session->audio_session.t_rtcp_read = eloop_add_read(session->t_master, rtsp_session_media_proxy_eloop, &session->audio_session, session->audio_session.rtcp_sock);
                }
            }
        }
    }    
    return OK;
}

int rtsp_session_media_start(rtsp_session_t* session, zpl_bool bvideo, zpl_bool start)
{
    if(session->bsrv && session->audio_session.b_enable && bvideo == zpl_false && 
            session->audio_session.rtp_state != RTP_SESSION_STATE_START &&
            session->audio_session.rtp_session)
    {
        if(session->parent && RTSP_DEBUG_FLAG(rtsp_srv_getptr(session->parent)->debug, EVENT))
        {
            rtsp_log_debug("RTSP Media Start Audio for %d/%d or %s", session->audio_session.mchannel, session->audio_session.mlevel, session->mfilepath?session->mfilepath:"nil");
        } 
        session->audio_session.rtp_state = start?RTP_SESSION_STATE_START:RTP_SESSION_STATE_STOP;
        rtsp_session_media_overtcp_start(session, zpl_false, start);
        if(start)
            rtsp_session_media_scheduler_add(session->audio_session.rtp_session);
        else 
            rtsp_session_media_scheduler_del(session->audio_session.rtp_session);

        if((session->audio_session.mchannel != -1 && session->audio_session.mlevel != -1))
        {
            if(start)
            {
                session->audio_session._call_index = zpl_media_channel_client_add(session->audio_session.mchannel, session->audio_session.mlevel, rtsp_session_media_rtp_proxy, &session->audio_session);
                return zpl_media_channel_start(session->audio_session.mchannel, session->audio_session.mlevel);
            }
            else
            {
                if(session->audio_session._call_index)
                    zpl_media_channel_client_del(session->audio_session.mchannel, session->audio_session.mlevel, session->audio_session._call_index);
                session->audio_session._call_index = 0;    
                return zpl_media_channel_stop(session->audio_session.mchannel, session->audio_session.mlevel);      
            }  
        }
        else if(session->audio_session.rtsp_media)
        {
            if(start)
                return zpl_media_file_reopen(session->audio_session.rtsp_media);
            else
            {
                int ret = zpl_media_file_destroy(session->audio_session.rtsp_media);
                session->audio_session.rtsp_media = NULL;
                return ret;    
            }
        }
    }
    if(session->bsrv && session->video_session.b_enable && bvideo == zpl_true && 
            session->video_session.rtp_state != RTP_SESSION_STATE_START &&
            session->video_session.rtp_session)
    {
        session->video_session.rtp_state = start?RTP_SESSION_STATE_START:RTP_SESSION_STATE_STOP;
        if(session->parent && RTSP_DEBUG_FLAG(rtsp_srv_getptr(session->parent)->debug, EVENT))
        {
            rtsp_log_debug("RTSP Media Start Video for %d/%d or %s", session->video_session.mchannel, session->video_session.mlevel, session->mfilepath?session->mfilepath:"nil");
        } 
        rtsp_session_media_overtcp_start(session, zpl_true, start);
        if(start)
            rtsp_session_media_scheduler_add(session->video_session.rtp_session);
        else 
            rtsp_session_media_scheduler_del(session->video_session.rtp_session);
        if((session->video_session.mchannel != -1 && session->video_session.mlevel != -1))
        {
            if(start)
            {
                session->video_session._call_index = zpl_media_channel_client_add(session->video_session.mchannel, session->video_session.mlevel, rtsp_session_media_rtp_proxy, &session->video_session);
                return zpl_media_channel_start(session->video_session.mchannel, session->video_session.mlevel);
            }
            else
            {
                if(session->video_session._call_index)
                    zpl_media_channel_client_del(session->video_session.mchannel, session->video_session.mlevel, session->video_session._call_index);
                session->video_session._call_index = 0;    
                return zpl_media_channel_stop(session->video_session.mchannel, session->video_session.mlevel);      
            }  
        }
        else if(session->video_session.rtsp_media)
        {
            if(start)
                return zpl_media_file_reopen(session->video_session.rtsp_media);
            else
            {
                int ret = zpl_media_file_destroy(session->video_session.rtsp_media);
                session->video_session.rtsp_media = NULL;
                return ret;    
            }
        }
    }
    return ERROR;
}


int rtsp_session_media_destroy(rtsp_session_t *session)
{
    if(!session->bsrv)
        return OK;
    rtsp_session_media_start(session, zpl_true, zpl_false);
    rtsp_session_media_start(session, zpl_false, zpl_false);
   
    return ERROR;
}

char *rtsp_session_media_name(int channel, int level)
{
    static char tmpbuf[32];
    memset(tmpbuf, 0, sizeof(tmpbuf));
    sprintf(tmpbuf, "/media/channel=%d?level=%d", channel, level);
    return tmpbuf;
}


int rtsp_session_media_lookup(rtsp_session_t * session, int channel, int level, const char *path)
{
    if(!session->bsrv)
        return OK;
    if((channel != -1 && level != -1) && zpl_media_channel_lookup( channel,  level))
    {
        zpl_media_channel_t * other_chn = NULL;
        if(zpl_media_channel_isvideo(channel, level))
        {
            if(session->parent && RTSP_DEBUG_FLAG(rtsp_srv_getptr(session->parent)->debug, EVENT))
            {
                rtsp_log_debug("RTSP Media is Video for %d/%d or %s", channel, level, path?path:"nil");
            }
            session->video_session.mchannel = channel;
            session->video_session.mlevel = level;
            session->video_session.b_enable = true;
            other_chn = zpl_media_channel_lookup_bind( channel, level);
            if(other_chn)
            {
                session->audio_session.mchannel = other_chn->channel;
                session->audio_session.mlevel = other_chn->channel_index;
                session->audio_session.b_enable = true;
                session->video_session.i_trackid = 1;
                session->audio_session.i_trackid = 2;
            }
        }
        else if(zpl_media_channel_isaudio(channel, level))
        {
            if(session->parent && RTSP_DEBUG_FLAG(rtsp_srv_getptr(session->parent)->debug, EVENT))
            {
                rtsp_log_debug("RTSP Media is Audio for %d/%d or %s", channel, level, path?path:"nil");
            }
            session->audio_session.mchannel = channel;
            session->audio_session.mlevel = level;
            session->audio_session.b_enable = true;
            other_chn = zpl_media_channel_lookup_bind( channel, level);
            if(other_chn)
            {
                session->video_session.mchannel = other_chn->channel;
                session->video_session.mlevel = other_chn->channel_index;
                session->video_session.b_enable = true;
                session->video_session.i_trackid = 1;
                session->audio_session.i_trackid = 2;
            }
        }
        rtsp_session_media_option_load(session);
        return 1;
    }
    else
    {
        if(path && zpl_media_file_lookup(path))
        {
            int acnt = 0;
            loop_againt:
            if(session->audio_session.b_enable == zpl_false && session->video_session.b_enable == zpl_false)
            {
                zpl_media_file_t *mfile = zpl_media_file_open(path);
                if(mfile)
                {
                    session->audio_session.b_enable = zpl_media_file_getptr(mfile)->b_audio;
                    session->video_session.b_enable = zpl_media_file_getptr(mfile)->b_video;
                    if(session->video_session.b_enable)
                        session->video_session.rtsp_media = mfile;
                    if(session->audio_session.b_enable)
                        session->audio_session.rtsp_media = mfile;
                    rtsp_log_debug("=====================RTSP Media file %p", session->audio_session.rtsp_media);    
                }
            }

            if(session->video_session.b_enable && session->video_session.rtsp_media && zpl_media_file_check(session->video_session.rtsp_media, path))
            {
                rtsp_session_media_option_load(session);
                return 1; 
            }
            if(session->audio_session.b_enable && session->audio_session.rtsp_media && zpl_media_file_check(session->audio_session.rtsp_media, path))
            {
                rtsp_session_media_option_load(session);
                return 1; 
            }
            if(session->video_session.b_enable && session->video_session.rtsp_media)
            {
                zpl_media_file_destroy(session->video_session.rtsp_media);
                session->video_session.b_enable = zpl_false;
            }
            if(session->audio_session.b_enable && session->audio_session.rtsp_media)
            {
                zpl_media_file_destroy(session->audio_session.rtsp_media);
                session->audio_session.b_enable = zpl_false;
            }
            acnt++;
            if(acnt != 2)
                goto loop_againt;
            else
                return 0;    
        }
    }
    return 0;
}

static int rtsp_session_media_option_load(rtsp_session_t *session)
{
    zpl_video_codec_t pcodec;
    zpl_audio_codec_t paudiocodec;
    int is_video = 0;
    if (session->video_session.b_enable)
    {
        if (session->video_session.rtsp_media)
        {
            if (zpl_media_file_codecdata(session->video_session.rtsp_media, zpl_true, &pcodec) != OK)
            {
                return ERROR;
            }
        }
        else
        {
            if (zpl_media_channel_video_codec_get(session->video_session.mchannel, session->video_session.mlevel, &pcodec) != OK)
            {
                return ERROR;
            }
        }
        is_video = 1;
    }
    if (session->audio_session.b_enable)
    {
        if (session->audio_session.rtsp_media)
        {
            if (zpl_media_file_codecdata(session->audio_session.rtsp_media, zpl_false, &pcodec) != OK)
            {
                return ERROR;
            }
        }
        else
        {
            if (zpl_media_channel_video_codec_get(session->audio_session.mchannel, session->audio_session.mlevel, &paudiocodec) != OK)
            {
                return ERROR;
            }
        }
        is_video |= 2;
    }
    if (is_video & 1)
    {
        session->video_session.payload = pcodec.enctype;
        /* 更新帧率和发包时间间隔 */
        session->video_session.framerate = pcodec.framerate;
        session->video_session.video_height = pcodec.vidsize.height; // 视频的高度
        session->video_session.video_width = pcodec.vidsize.width;   // 视频的宽度
        if(session->video_session.framerate)
        {
            session->video_session.frame_delay_msec = RTP_MEDIA_FRAME_DELAY(1000 / session->video_session.framerate);
            session->video_session.timestamp_interval = rtp_profile_get_clock_rate(session->video_session.payload) / session->video_session.framerate;
        }
        if (session->parent && RTSP_DEBUG_FLAG(rtsp_srv_getptr(session->parent)->debug, EVENT))
        {
            rtsp_log_debug("RTSP Media get Video timestamp=%d for %d/%d or %s",
                           session->video_session.timestamp_interval, session->video_session.mchannel, session->video_session.mlevel, session->mfilepath ? session->mfilepath : "nil");
        }
    }
    if (is_video & 2)
    {
        session->audio_session.payload = paudiocodec.enctype;
        /* 更新帧率和发包时间间隔 */
        session->audio_session.framerate = paudiocodec.framerate;
        if(session->audio_session.framerate)
        {
            session->audio_session.frame_delay_msec = RTP_MEDIA_FRAME_DELAY(1000 / session->audio_session.framerate);
            session->audio_session.timestamp_interval = rtp_profile_get_clock_rate(session->audio_session.payload) / session->audio_session.framerate;
        }
        if (session->parent && RTSP_DEBUG_FLAG(rtsp_srv_getptr(session->parent)->debug, EVENT))
        {
            rtsp_log_debug("RTSP Media get Audio timestamp=%d for %d/%d or %s",
                           session->audio_session.timestamp_interval, session->audio_session.mchannel, session->audio_session.mlevel, session->mfilepath ? session->mfilepath : "nil");
        }
    }
    return OK;
}

static int rtsp_session_media_build_sdptext_h264(rtsp_session_t *session, uint8_t *src, uint32_t len)
{
    uint8_t base64sps[AV_BASE64_DECODE_SIZE(1024)];
    uint8_t base64pps[AV_BASE64_DECODE_SIZE(1024)];

    zpl_video_extradata_t extradata;

    int profile = 0, sdplength = 0;
 
    if((session->video_session.mchannel != -1 && session->video_session.mlevel != -1))
    {
         zpl_media_channel_t *chn = zpl_media_channel_lookup(session->video_session.mchannel, session->video_session.mlevel);
        if(chn)
             zpl_media_channel_extradata_get(chn, &extradata);
    }
    else
    {
        if(session->video_session.rtsp_media)
            zpl_media_file_extradata(session->video_session.rtsp_media, &extradata);
    }
    profile = extradata.profileLevelId;


    if (rtp_profile_get_rtpmap(RTP_MEDIA_PAYLOAD_H264))
        sdplength += sprintf(src + sdplength, "a=rtpmap:%d %s\r\n", RTP_MEDIA_PAYLOAD_H264, rtp_profile_get_rtpmap(RTP_MEDIA_PAYLOAD_H264));
    else
        sdplength += sprintf(src + sdplength, "a=rtpmap:%d H264/90000\r\n", RTP_MEDIA_PAYLOAD_H264);

    memset(base64pps, 0, sizeof(base64pps));
    memset(base64sps, 0, sizeof(base64sps));
    if(extradata.fPPSSize)
        av_base64_encode(base64pps, sizeof(base64pps), extradata.fPPS, extradata.fPPSSize);
    if(extradata.fSPSSize)
        av_base64_encode(base64sps, sizeof(base64sps), extradata.fSPS, extradata.fSPSSize);

    if (strlen(base64sps))
    {
        if (strlen(base64pps))
        {
            sdplength += sprintf(src + sdplength, "a=fmtp:%d profile-level-id=%06x;"
                                                  " sprop-parameter-sets=%s,%s; packetization-mode=%d\r\n",
                                 RTP_MEDIA_PAYLOAD_H264, profile, base64sps, base64pps, session->video_session.packetization_mode);
        }
        else
        {
            sdplength += sprintf(src + sdplength, "a=fmtp:%d profile-level-id=%06x;"
                                                  " sprop-parameter-sets=%s,%s; packetization-mode=%d;bitrate=%d\r\n",
                                 RTP_MEDIA_PAYLOAD_H264, profile, base64sps, base64pps, session->video_session.packetization_mode, 48000 /*bitrate*/);
        }
    }
    else
    {
        sdplength += sprintf(src + sdplength, "a=fmtp:%d profile-level-id=42E00D; "
                                              "sprop-parameter-sets=Z0LgDdqFAlE=,aM48gA==,aFOPoA==; packetization-mode=%d\r\n",
                             RTP_MEDIA_PAYLOAD_H264, session->video_session.packetization_mode);
    }
    return sdplength;
}


int rtsp_session_media_build_sdptext(rtsp_session_t * session, uint8_t *sdp)
{
    int sdplength = 0, ret = 0;
 
    if(session->video_session.b_enable)
    {
        sdplength = sprintf((char*)sdp + sdplength, "m=video 0 RTP/AVP %d\r\n", session->video_session.payload);
        if(session->video_session.payload == RTP_MEDIA_PAYLOAD_H264)
            ret = rtsp_session_media_build_sdptext_h264(session, sdp + sdplength, 0);
        //ret = rtsp_session_media_adap_build_sdp(session->video_session.payload, session, sdp + sdplength, 0);
        if(ret == 0)
        {
            if (rtp_profile_get_rtpmap(session->video_session.payload))
            {
                sdplength += sprintf(sdp + sdplength, "a=rtpmap:%d %s\r\n", session->video_session.payload,
                                     rtp_profile_get_rtpmap(session->video_session.payload));
                if(session->video_session.b_enable)
                {
                    if(session->video_session.i_trackid >= 0)
                        sdplength += sprintf(sdp + sdplength, "a=control:trackID=%d\r\n", session->video_session.i_trackid);
                }
                sdplength += sprintf((char*)sdp + sdplength, "a=framesize:%d %d-%d\r\n",
                                     session->video_session.payload,
                                     session->video_session.video_width,
                                     session->video_session.video_height);
             
                sdplength += sprintf((char*)sdp + sdplength, "a=framerate:%u\r\n", session->video_session.framerate);

                sdplength += sprintf((char*)sdp + sdplength, "a=range:npt=now-\r\n");
            }
        }
        else
        {
            sdplength += ret;
            sdplength += sprintf((char*)sdp + sdplength, "a=framesize:%d %d-%d\r\n",
                                     session->video_session.payload,
                                     session->video_session.video_width,
                                     session->video_session.video_height);
            sdplength += sprintf((char*)sdp + sdplength, "a=framerate:%u\r\n", session->video_session.framerate);
            sdplength += sprintf((char*)sdp + sdplength, "a=range:npt=now-\r\n");
        }
        //sdplength += rtsp_server_build_sdp_video(session, sdp + sdplength);
    }
    if(session->audio_session.b_enable)
    {
        sdplength = sprintf((char*)sdp + sdplength, "m=audio 0 RTP/AVP %d\r\n", session->audio_session.payload);
        if(ret == 0)
        {
            if (rtp_profile_get_rtpmap(session->audio_session.payload))
            {
                sdplength += sprintf(sdp + sdplength, "a=rtpmap:%d %s\r\n", session->audio_session.payload,
                                     rtp_profile_get_rtpmap(session->audio_session.payload));
                if(session->audio_session.b_enable)
                {
                    if(session->audio_session.i_trackid >= 0)
                        sdplength += sprintf(sdp + sdplength, "a=control:trackID=%d\r\n", session->audio_session.i_trackid);
                }
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


static int rtsp_session_media_sprop_parameterset_parse_h264(uint8_t *buffer, uint32_t len, zpl_video_extradata_t *extradata)
{
    uint32_t i = 0;
    uint8_t *p = NULL;
    uint8_t *start = buffer;
    uint8_t tmp[4096];
    size_t out_size = 0;
    for (p = buffer; *p != '\0' && i < len; ++p,i++)
    {
        if (*p == ',')
        {
            *p = '\0';
            memset(tmp, 0, sizeof(tmp));
            out_size = av_base64_decode(tmp, (const char*)start, sizeof(tmp));
            if(out_size)
            {
                uint8_t nal_unit_type = (tmp[0])&0x1F;
                if (nal_unit_type == 7/*SPS*/)
                {
                    #ifdef ZPL_VIDEO_EXTRADATA_MAXSIZE
                    if(out_size <= ZPL_VIDEO_EXTRADATA_MAXSIZE)
                    #else 
                    extradata->fSPS = malloc(out_size);
                    if(extradata->fSPS)
                    #endif
                    {
                        memset(extradata->fSPS, 0, out_size);
                        memcpy(extradata->fSPS, tmp, out_size);
                        extradata->fSPSSize = out_size;
                    }
                }
                else if (nal_unit_type == 8/*PPS*/)
                {
                    #ifdef ZPL_VIDEO_EXTRADATA_MAXSIZE
                    if(out_size <= ZPL_VIDEO_EXTRADATA_MAXSIZE)
                    #else 
                    extradata->fPPS = malloc(out_size);
                    if(extradata->fPPS)
                    #endif
                    {
                        memset(extradata->fPPS, 0, out_size);
                        memcpy(extradata->fPPS, tmp, out_size);
                        extradata->fPPSSize = out_size;
                    }
                }
            }
            start = p + 1;
        }
    }
    return OK;
}

static int rtsp_session_media_parse_sdptext_h264(rtsp_session_t *session, uint8_t *attrval, uint32_t len)
{
    int format  = 0;
    zpl_video_extradata_t extradata;
    struct sdp_attr_fmtp_h264_t h264;
    rtsp_client_t *client = session->parent;

    memset(&h264, 0, sizeof(struct sdp_attr_fmtp_h264_t));
    sdp_attr_fmtp_h264(attrval, &format, &h264);
    session->video_session.packetization_mode = h264.packetization_mode;


    rtsp_session_media_sprop_parameterset_parse_h264(h264.sprop_parameter_sets,
                                   h264.sprop_parameter_sets_len, &extradata);
    zpl_media_channel_decode_sps(extradata.fSPS, extradata.fSPSSize,
                                    &client->client_media.video_codec.vidsize.width,
                                    &client->client_media.video_codec.vidsize.height,
                                    &client->client_media.video_codec.framerate);
#ifndef ZPL_VIDEO_EXTRADATA_MAXSIZE
    if(extradata.fPPS)
        free(extradata.fPPS);
    if(extradata.fSPS)
        free(extradata.fSPS);
#endif
    return OK;
}
static int rtsp_session_media_fmtp_attr_parse(struct sdp_media *m, rtsp_session_t *session, void *pUser, bool bvideo)
{
    uint32_t step = 0;
    char *rtpmapstr = NULL;
    rtp_session_t *rtpsession = bvideo?&session->video_session:&session->audio_session;
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

                                    rtsp_session_media_parse_sdptext_h264(session, (uint8_t*)attrval, strlen(attrval));
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
    return OK;
}


static int rtsp_session_media_parse_sdptext(rtsp_session_t* session, void *pUser)
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
            session->video_session.b_enable = true;
            ret = rtsp_session_media_fmtp_attr_parse(m, session, pUser, true);
        }

        m = sdp_media_find(session->sdptext.media,
                           session->sdptext.media_count, "audio", NULL);
        if(m)
        {
            session->audio_session.b_enable = true;
            ret = rtsp_session_media_fmtp_attr_parse(m, session, pUser, false);
        }
    }
    else
    {
        session->video_session.rtpmode = RTP_SESSION_SENDRECV;
        session->audio_session.rtpmode = RTP_SESSION_SENDRECV;
    }
    return ret;
}




rtsp_code rtsp_session_media_handle_option(rtsp_session_t * session, void *pUser)
{
    if(session->bsrv)
        return RTSP_STATE_CODE_200;
    else
        return RTSP_STATE_CODE_200;
}


rtsp_code rtsp_session_media_handle_describe(rtsp_session_t * session, void *pUser)
{
    if(session->bsrv)
        return RTSP_STATE_CODE_200;
    else
    {
        rtsp_session_media_parse_sdptext(session, pUser);
        return RTSP_STATE_CODE_200;
    }
}

rtsp_code rtsp_session_media_handle_setup(rtsp_session_t * session, int isvideo, void *pUser)
{
    rtsp_session_rtp_setup(session, isvideo);
    if(session->bsrv)
        return RTSP_STATE_CODE_200;
    else
        return RTSP_STATE_CODE_200;
}

rtsp_code rtsp_session_media_handle_teardown(rtsp_session_t * session, void *pUser)
{
    rtsp_session_media_start(session, zpl_true, zpl_false);
    rtsp_session_media_start(session, zpl_false, zpl_false);
    rtsp_session_rtp_teardown(session);
    if(session->bsrv)
        return RTSP_STATE_CODE_200;
    else
        return RTSP_STATE_CODE_200;
}

rtsp_code rtsp_session_media_handle_play(rtsp_session_t * session, void *pUser)
{
    rtsp_session_media_start(session, zpl_true, zpl_true);
    rtsp_session_media_start(session, zpl_false, zpl_true);
    if(session->bsrv)
        return RTSP_STATE_CODE_200;
    else
        return RTSP_STATE_CODE_200;
}

rtsp_code rtsp_session_media_handle_pause(rtsp_session_t * session, void *pUser)
{
    if(session->bsrv)
    {
        if(session->audio_session.b_enable && session->audio_session.rtp_session)
        {
            session->audio_session.rtp_state = RTP_SESSION_STATE_STOP;
            rtsp_session_media_start(session, zpl_false, zpl_false);
        }
        if(session->video_session.b_enable && session->video_session.rtp_session)
        {
            session->video_session.rtp_state = RTP_SESSION_STATE_STOP;
            rtsp_session_media_start(session, zpl_true, zpl_false);
        }
        return RTSP_STATE_CODE_200;
    }
    else
        return RTSP_STATE_CODE_200;
}

rtsp_code rtsp_session_media_handle_scale(rtsp_session_t * session, void *pUser)
{
    if(session->bsrv)
        return RTSP_STATE_CODE_200;
    else
        return RTSP_STATE_CODE_200;
}

rtsp_code rtsp_session_media_handle_get_parameter(rtsp_session_t * session, void *pUser)
{
    if(session->bsrv)
        return RTSP_STATE_CODE_200;
    else
        return RTSP_STATE_CODE_200;
}

rtsp_code rtsp_session_media_handle_set_parameter(rtsp_session_t * session, void *pUser)
{
    if(session->bsrv)
        return RTSP_STATE_CODE_200;
    else
        return RTSP_STATE_CODE_200;
}



int rtsp_session_media_rtp_proxy(zpl_media_channel_t *mediachn, 
                          const zpl_skbuffer_t *bufdata,  void *pVoidUser)
{
    int ret = 0;
    rtp_session_t *session = pVoidUser;
    zpl_media_hdr_t *media_header = bufdata->skb_hdr.other_hdr;
    int channel = -1, level = -1, mtype = -1;
    zpl_skbuffer_t *skb = NULL;
    channel = ZPL_MEDIA_CHANNEL_GET_C(media_header->ID);
    level = ZPL_MEDIA_CHANNEL_GET_I(media_header->ID);
    mtype = ZPL_MEDIA_CHANNEL_GET_T(media_header->ID);   
    if(session->rtsp_media_queue)
    {
        skb = zpl_skbuffer_clone(session->rtsp_media_queue, bufdata); 
        if(skb)
            ret = zpl_skbqueue_add(session->rtsp_media_queue, skb);
    }
    return ret;
}

int rtsp_session_media_tcp_forward(rtsp_session_t* session, const uint8_t *buffer, uint32_t len)
{
    return rtsp_session_rtp_tcp_forward(session, buffer,  len);
}

#if 0
int rtsp_session_media_rtp_sendto(zpl_media_channel_t *mediachn, 
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
    ret = rtsp_session_media_adap_rtp_sendto(media_header->frame_codec, pVoidUser, media_header->frame_type,
                                     ZPL_SKB_DATA(bufdata), ZPL_SKB_DATA_LEN(bufdata));
 
    if(media_header->frame_type == ZPL_MEDIA_VIDEO)
        session->video_session.user_timestamp += session->video_session.timestamp_interval;
    else
        session->audio_session.user_timestamp += session->audio_session.timestamp_interval;
    return ret;
}


int rtsp_session_media_rtp_recv(rtsp_session_t* session, bool bvideo, zpl_skbuffer_t *bufdata)
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
        ret = rtsp_session_rtp_recv(session, pbuffer + 4, MAX_RTP_PAYLOAD_LENGTH, bvideo, &havemore);

        if (havemore)
        {
            //fprintf(stdout, " ======================= rtsp_session_rtp_recv: havemore=%i\n",havemore);
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
#endif


static int rtsp_session_media_rtp_sendto(rtp_session_t *rtp_session)
{
    int ret = 0;
    if(rtp_session->rtsp_media_queue)
    {
        zpl_skbuffer_t * skb = zpl_skbqueue_get(rtp_session->rtsp_media_queue);
        if(skb)
        {
            zpl_media_hdr_t *media_header = skb->skb_hdr.other_hdr;
  
            ret = rtsp_session_media_adap_rtp_sendto(media_header->frame_codec, rtp_session->rtsp_parent, media_header->frame_type,
                                            ZPL_SKB_DATA(skb), ZPL_SKB_DATA_LEN(skb));
            rtp_session->user_timestamp += rtp_session->timestamp_interval;
            zpl_skbqueue_finsh(rtp_session->rtsp_media_queue, skb);
        }
    }
    else if(rtp_session->rtsp_media)
    {
        zpl_media_bufcache_t *bufcache = &zpl_media_file_getptr(rtp_session->rtsp_media)->bufcache;
        //rtsp_log_debug("=====================RTSP Media read file %p", rtp_session->rtsp_media); 
        if(zpl_media_file_read(rtp_session->rtsp_media, bufcache) > 0)
        {
            int type = zpl_media_file_getptr(rtp_session->rtsp_media)->b_audio?ZPL_MEDIA_AUDIO:ZPL_MEDIA_VIDEO;
            int frame_codec = 0;
            //rtsp_log_debug("============RTSP Media Rtp read and Sendto %d byte", bufcache->len);
            if(type == ZPL_MEDIA_VIDEO)
                frame_codec = zpl_media_file_getptr(rtp_session->rtsp_media)->filedesc.video.enctype;
            else
                frame_codec = zpl_media_file_getptr(rtp_session->rtsp_media)->filedesc.audio.enctype;
            if(bufcache->len) 
            {       
                ret = rtsp_session_media_adap_rtp_sendto(frame_codec, rtp_session->rtsp_parent, type,
                                                bufcache->data, bufcache->len);
                rtp_session->user_timestamp += rtp_session->timestamp_interval;
            }
        }
    }
    return ret;
}
static int rtsp_session_media_rtp_recv(rtp_session_t *myrtp_session)
{
    int havemore = 1 , ret = 0, tlen = 0;
    uint8_t pbuffer[MAX_RTP_PAYLOAD_LENGTH + 128];
    while (havemore && myrtp_session->rtp_session)
    {
        ret = rtp_session_recv_with_ts(myrtp_session->rtp_session, pbuffer + 4, MAX_RTP_PAYLOAD_LENGTH, myrtp_session->user_timestamp, &havemore);
        //ret = rtsp_session_media_adap_rtp_recv(myrtp_session->payload, myrtp_session->rtsp_parent, ZPL_MEDIA_VIDEO, pbuffer + 4, MAX_RTP_PAYLOAD_LENGTH, &havemore)
        if (havemore)
        {
            fprintf(stdout, " ======================= rtsp_session_rtp_recv: havemore=%i\n",havemore);
            fflush(stdout);
        }
        if(ret)
            tlen += ret;
    }
    ortp_message("==================================bufdata->frame_len=%d", tlen);
    myrtp_session->user_timestamp += myrtp_session->timestamp_interval;
    return OK;
}


int rtsp_session_media_scheduler_add(void *rtp_session)
{
    if (_media_scheduler.r_session_set == NULL)
    {
        _media_scheduler.r_session_set = session_set_new();
        if (_media_scheduler.r_session_set)
            session_set_init(_media_scheduler.r_session_set);
    }
    if (_media_scheduler.w_session_set == NULL)
    {
        _media_scheduler.w_session_set = session_set_new();
        if (_media_scheduler.w_session_set)
            session_set_init(_media_scheduler.w_session_set);
        else
        {
            session_set_destroy(_media_scheduler.r_session_set);
            _media_scheduler.r_session_set = NULL;
            return ERROR;
        }
    }
    if (_media_scheduler.e_session_set == NULL)
    {
        _media_scheduler.e_session_set = session_set_new();
        if (_media_scheduler.e_session_set)
            session_set_init(_media_scheduler.e_session_set);
        else
        {
            session_set_destroy(_media_scheduler.r_session_set);
            _media_scheduler.r_session_set = NULL;
            session_set_destroy(_media_scheduler.w_session_set);
            _media_scheduler.w_session_set = NULL;
            return ERROR;
        }
    }   

    if (_media_scheduler.r_session_set && _media_scheduler.w_session_set && rtp_session)
    {
        if( ZPL_RTP_SESSION(rtp_session)->mode == RTP_SESSION_RECVONLY)
        {
            session_set_set(_media_scheduler.r_session_set, ZPL_RTP_SESSION(rtp_session));
            if (_media_scheduler.e_session_set == NULL)
                session_set_set(_media_scheduler.e_session_set, ZPL_RTP_SESSION(rtp_session));
            _media_scheduler.count++;
        }
        else if( ZPL_RTP_SESSION(rtp_session)->mode == RTP_SESSION_SENDONLY)
        {
            session_set_set(_media_scheduler.w_session_set, ZPL_RTP_SESSION(rtp_session));
            if (_media_scheduler.e_session_set == NULL)
                session_set_set(_media_scheduler.e_session_set, ZPL_RTP_SESSION(rtp_session));
            _media_scheduler.count++;
        }
        else if( ZPL_RTP_SESSION(rtp_session)->mode == RTP_SESSION_SENDRECV)
        {
            session_set_set(_media_scheduler.r_session_set, ZPL_RTP_SESSION(rtp_session));
            session_set_set(_media_scheduler.w_session_set, ZPL_RTP_SESSION(rtp_session));
            if (_media_scheduler.e_session_set == NULL)
                session_set_set(_media_scheduler.e_session_set, ZPL_RTP_SESSION(rtp_session));
            _media_scheduler.count++;
            _media_scheduler.count++;
        }
    }
    session_set_copy(&_media_scheduler.all_session_set, _media_scheduler.w_session_set);
    return OK;
}
int rtsp_session_media_scheduler_del(void *rtp_session)
{
    if (_media_scheduler.r_session_set && _media_scheduler.w_session_set && rtp_session)
    {
        if( ZPL_RTP_SESSION(rtp_session)->mode == RTP_SESSION_RECVONLY)
        {
            session_set_clr(_media_scheduler.r_session_set, ZPL_RTP_SESSION(rtp_session));
            if (_media_scheduler.e_session_set == NULL)
                session_set_clr(_media_scheduler.e_session_set, ZPL_RTP_SESSION(rtp_session));
            _media_scheduler.count--;
        }
        else if( ZPL_RTP_SESSION(rtp_session)->mode == RTP_SESSION_SENDONLY)
        {
            session_set_clr(_media_scheduler.w_session_set, ZPL_RTP_SESSION(rtp_session));
            if (_media_scheduler.e_session_set == NULL)
                session_set_clr(_media_scheduler.e_session_set, ZPL_RTP_SESSION(rtp_session));
            _media_scheduler.count--;
        }
        else if( ZPL_RTP_SESSION(rtp_session)->mode == RTP_SESSION_SENDRECV)
        {
            session_set_clr(_media_scheduler.r_session_set, ZPL_RTP_SESSION(rtp_session));
            session_set_clr(_media_scheduler.w_session_set, ZPL_RTP_SESSION(rtp_session));
            if (_media_scheduler.e_session_set == NULL)
                session_set_clr(_media_scheduler.e_session_set, ZPL_RTP_SESSION(rtp_session));
            _media_scheduler.count--;
            _media_scheduler.count--;
        }
    }
    session_set_copy(&_media_scheduler.all_session_set, _media_scheduler.w_session_set);
    return OK;
}

static int rtsp_session_media_scheduler_callback(rtsp_session_t *session, void *ss)
{
    rtsp_session_media_scheduler *sche = ss;
    RtpSession* rtp_session = NULL;
    if(session->video_session.b_enable && session->video_session.rtp_session)
    {
        rtp_session = ZPL_RTP_SESSION(session->video_session.rtp_session);

        if( ZPL_RTP_SESSION(rtp_session)->mode == RTP_SESSION_RECVONLY)
        {
            if (session_set_is_set(sche->r_session_set, ZPL_RTP_SESSION(rtp_session)))
            {
                //rtsp_log_debug("RTSP Video Media Rtp read ready");
                if((session->video_session.rtp_session_recv))
                    (session->video_session.rtp_session_recv)(&session->video_session);
                else
                    rtsp_session_media_rtp_recv(&session->video_session);    
            }
        }
        else if( ZPL_RTP_SESSION(rtp_session)->mode == RTP_SESSION_SENDONLY)
        {
            if (session_set_is_set(sche->w_session_set, ZPL_RTP_SESSION(rtp_session)))
            {
                //rtsp_log_debug("RTSP Video Media Rtp Send ready");
                if((session->video_session.rtp_session_send))
                    (session->video_session.rtp_session_send)(&session->video_session);
                else 
                    rtsp_session_media_rtp_sendto(&session->video_session);
            }
        }
        else if( ZPL_RTP_SESSION(rtp_session)->mode == RTP_SESSION_SENDRECV)
        {
            if (session_set_is_set(sche->w_session_set, ZPL_RTP_SESSION(rtp_session)))
            {
                //rtsp_log_debug("RTSP Video Media Rtp Send ready 2");
                if((session->video_session.rtp_session_send))
                    (session->video_session.rtp_session_send)(&session->video_session);
                else 
                    rtsp_session_media_rtp_sendto(&session->video_session);
            }
            /*if (session_set_is_set(sche->r_session_set, ZPL_RTP_SESSION(rtp_session)))
            {
                rtsp_log_debug("RTSP Video Media Rtp read ready 1");
                if((session->video_session.rtp_session_recv))
                    (session->video_session.rtp_session_recv)(&session->video_session);
                else
                    ;//rtsp_session_media_rtp_recv(&session->video_session); 
            }*/
        }
    }
    if(session->audio_session.b_enable && session->audio_session.rtp_session)
    {
        rtp_session = ZPL_RTP_SESSION(session->audio_session.rtp_session);
        if( ZPL_RTP_SESSION(rtp_session)->mode == RTP_SESSION_RECVONLY)
        {
            if (session_set_is_set(sche->r_session_set, ZPL_RTP_SESSION(rtp_session)))
            {
                //rtsp_log_debug("RTSP Audio Media Rtp read ready");
                if((session->audio_session.rtp_session_recv))
                    (session->audio_session.rtp_session_recv)(&session->audio_session);
                else
                    rtsp_session_media_rtp_recv(&session->audio_session); 
            }
        }
        else if( ZPL_RTP_SESSION(rtp_session)->mode == RTP_SESSION_SENDONLY)
        {
            if (session_set_is_set(sche->w_session_set, ZPL_RTP_SESSION(rtp_session)))
            {
                //rtsp_log_debug("RTSP Audio Media Rtp Send ready");
                if((session->audio_session.rtp_session_send))
                    (session->audio_session.rtp_session_send)(&session->audio_session);
                else 
                    rtsp_session_media_rtp_sendto(&session->audio_session);
            }
        }
        else if( ZPL_RTP_SESSION(rtp_session)->mode == RTP_SESSION_SENDRECV)
        {
            if (session_set_is_set(sche->w_session_set, ZPL_RTP_SESSION(rtp_session)))
            {
                //rtsp_log_debug("RTSP Audio Media Rtp Send ready 2");
                if((session->audio_session.rtp_session_send))
                    (session->audio_session.rtp_session_send)(&session->audio_session);
                else 
                    rtsp_session_media_rtp_sendto(&session->audio_session);
            }
            if (session_set_is_set(sche->r_session_set, ZPL_RTP_SESSION(rtp_session)))
            {
                //rtsp_log_debug("RTSP Audio Media Rtp read ready 1");
                if((session->audio_session.rtp_session_recv))
                    (session->audio_session.rtp_session_recv)(&session->audio_session);
                 else
                    rtsp_session_media_rtp_recv(&session->audio_session); 
            }
        }
    }
    return OK;
}


int rtsp_session_media_scheduler_handle(void)
{
    int ret = 0;
    rtsp_log_debug("=====RTSP Media Rtp select befor");
    session_set_copy(_media_scheduler.w_session_set, &_media_scheduler.all_session_set);
	//ret = session_set_select(_media_scheduler.r_session_set, _media_scheduler.w_session_set, NULL);
    ret = session_set_select(NULL, _media_scheduler.w_session_set, NULL);
    rtsp_log_debug("=====RTSP Media Rtp select after %d", ret);
    if(ret)
    {
        rtsp_session_foreach(rtsp_session_media_scheduler_callback, &_media_scheduler);
    }
    return ret;
}


static int rtp_media_task(void* argv)
{   
    while(1)
    {
        if(_media_scheduler.count)
            rtsp_session_media_scheduler_handle();
        else
        {
            os_msleep(10);
        }    
    }
    return OK;
}

int rtsp_session_media_scheduler_init(void)
{
    memset(&_media_scheduler, 0, sizeof(_media_scheduler));
    if (_media_scheduler.r_session_set == NULL)
    {
        _media_scheduler.r_session_set = session_set_new();
        if (_media_scheduler.r_session_set)
            session_set_init(_media_scheduler.r_session_set);
    }
    if (_media_scheduler.w_session_set == NULL)
    {
        _media_scheduler.w_session_set = session_set_new();
        if (_media_scheduler.w_session_set)
            session_set_init(_media_scheduler.w_session_set);
        else
        {
            session_set_destroy(_media_scheduler.r_session_set);
            _media_scheduler.r_session_set = NULL;
            return ERROR;
        }
    }
    if (_media_scheduler.e_session_set == NULL)
    {
        _media_scheduler.e_session_set = session_set_new();
        if (_media_scheduler.e_session_set)
            session_set_init(_media_scheduler.e_session_set);
        else
        {
            session_set_destroy(_media_scheduler.r_session_set);
            _media_scheduler.r_session_set = NULL;
            session_set_destroy(_media_scheduler.w_session_set);
            _media_scheduler.w_session_set = NULL;
            return ERROR;
        }
    }   
    
    _media_scheduler.taskid = os_task_create("rtpMediaTask", OS_TASK_DEFAULT_PRIORITY,
                                             0, rtp_media_task, NULL, OS_TASK_DEFAULT_STACK * 8);
    return OK;
}

int rtsp_session_media_scheduler_exit(void)
{
    if(_media_scheduler.taskid)
	    os_task_destroy(_media_scheduler.taskid);
	_media_scheduler.taskid = 0;
    if(_media_scheduler.w_session_set)
    {
        session_set_destroy(_media_scheduler.w_session_set);
        _media_scheduler.w_session_set = NULL;
    }  
    if(_media_scheduler.r_session_set)
    {
        session_set_destroy(_media_scheduler.r_session_set);
        _media_scheduler.r_session_set = NULL;
    } 
    _media_scheduler.count = 0;
    return OK;
}

#if 0
int rtsp_session_rtp_scheduler_handle(rtsp_session_rtp_scheduler *scheduler)
{
	int ret = session_set_select(scheduler->r_session_set, scheduler->w_session_set, NULL);
    return ret;
}

		for (k=0;k<channels;k++){
			/* this is stupid to do this test, because all session work the same way,
			as the same user_ts is used for all sessions, here. */
			if (session_set_is_set(set,session[k])){
				rtp_session_send_with_ts(session[k],buffer,i,user_ts);
				//ortp_message("packet sended !");
			}
		}
#endif        