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

#include "zpl_rtsp_def.h"
#include "zpl_rtsp_session.h"
#include "zpl_rtsp_client.h"
#include "zpl_rtsp_media.h"
#include "zpl_rtsp_adap.h"
#include "zpl_rtsp_rtp.h"
#include "zpl_rtp_h264.h"
#ifdef ZPL_LIBRTSP_MODULE
#include "zpl_media.h"
#include "zpl_media_internal.h"
#endif

static int rtsp_media_srv_setup(rtsp_media_t *media, rtsp_session_t *session);

int rtsp_media_destroy(rtsp_session_t * session, rtsp_media_t *media)
{
    if(!session->bsrv)
        return 0;
    rtsp_media_start(session, media, false);
    rtsp_media_update(session, media, false);
    if(zpl_media_channel_gettype(media) == ZPL_MEDIA_CHANNEL_FILE)
    {
        if(zpl_media_getptr(media)->bindcount < 1)
            zpl_media_channel_hwdestroy(media);
        return OK;
    }
    return 0;
}

rtsp_media_t * rtsp_media_create(rtsp_session_t * session, int channel, int level, const char *path)
{
    if(!session->bsrv)
        return NULL;
    if(path)
    {
        fprintf(stdout, " ============================rtsp_media_create path=%s\r\n", path);
        fflush(stdout);
        zpl_media_channel_t *chn = zpl_media_channel_filecreate(path, zpl_true);
        //zpl_media_channel_t *chn = zpl_media_channel_filelookup(path);
        if(chn)
        {
            //int zpl_media_file_get_frame_callback(zpl_media_file_t *file, int (*func)(zpl_media_file_t*, zpl_framedata_t *))
            if(chn->video_media.enable && chn->video_media.halparam)
                zpl_media_file_get_frame_callback(chn->video_media.halparam, _h264_file_get_frame);
            else if(chn->audio_media.enable && chn->audio_media.halparam)
                zpl_media_file_get_frame_callback(chn->audio_media.halparam, _h264_file_get_frame);
            rtsp_media_srv_setup(chn, session);
        }
        return chn;
    }
    return NULL;
}


int rtsp_media_update(rtsp_session_t * session, rtsp_media_t * channel, bool add)
{
    zpl_media_channel_t *chn = channel;
    if(!session->bsrv)
        return 0;
    if(chn)
    {
        if(add)
            chn->bindcount++;
        else
            chn->bindcount--;
    }
    return 0;
}

char *rtsp_media_name(int channel, int level)
{
    static char tmpbuf[32];
    memset(tmpbuf, 0, sizeof(tmpbuf));
    sprintf(tmpbuf, "/media/channel=%d?level=%d", channel, level);
    return tmpbuf;
}


rtsp_media_t * rtsp_media_lookup(rtsp_session_t * session, int channel, int level, const char *path)
{
    if(!session->bsrv)
        return NULL;
    zpl_media_channel_t * chn = zpl_media_channel_lookup( channel,  level);
    if(chn)
    {
        rtsp_media_srv_setup(chn, session);
        return chn;
    }
    if(path)
    {
        fprintf(stdout, " rtsp_media_lookup path=%s\r\n", path);
        fflush(stdout);
        chn = zpl_media_channel_filelookup(path);
        if(chn)
            rtsp_media_srv_setup(chn, session);
        return chn;
    }
    return NULL;
}



int rtsp_media_start(rtsp_session_t* session, rtsp_media_t *media, bool start)
{
    if(session->audio_session.b_enable &&
            session->audio_session.rtp_state != RTP_SESSION_STATE_START &&
            session->audio_session.rtp_session)
    {
        fprintf(stdout, "===============audio_session==RTP_SESSION_STATE_START\r\n");
        session->audio_session.rtp_state = start?RTP_SESSION_STATE_START:RTP_SESSION_STATE_STOP;
    }
    if(session->video_session.b_enable &&
            session->video_session.rtp_state != RTP_SESSION_STATE_START &&
            session->video_session.rtp_session)
    {
        session->video_session.rtp_state = start?RTP_SESSION_STATE_START:RTP_SESSION_STATE_STOP;
        fprintf(stdout, "===============video_session==RTP_SESSION_STATE_START\r\n");
    }
    if(session->bsrv)
    {
        if(zpl_media_channel_gettype(media) == ZPL_MEDIA_CHANNEL_FILE)
        {
            //if(start && rtsp_test_taskid == 0)
            //    pthread_create(&rtsp_test_taskid, NULL, rtsp_srvread, session);
            printf("===================rtsp_media_start===================\r\n");
            return zpl_media_channel_filestart(media, true);
        }
    }
    return 0;
}


int rtsp_media_rtp_sendto(zpl_media_channel_t *mediachn, 
                          const zpl_skbuffer_t *bufdata,  void *pVoidUser)
{
    int ret = 0;
    rtsp_session_t *session = pVoidUser;
    ret = rtsp_media_adap_rtp_sendto(bufdata->skb_header.media_header.buffer_codec, pVoidUser, bufdata->skb_header.media_header.buffer_type,
                                     ZPL_SKB_DATA(bufdata), ZPL_SKB_DATA_LEN(bufdata));
    //ret = _rtp_send_h264(pVoidUser, 1, bufdata->buffer_data, bufdata->buffer_len);

    //session->video_session.user_timestamp += 3600;
    if(bufdata->skb_header.media_header.buffer_type == ZPL_MEDIA_VIDEO)
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
    //ortp_message("==================================bufdata->buffer_len=%d", bufdata->buffer_len);

    if(bvideo && bufdata->skb_len)
    {
        bufdata->skb_header.media_header.buffer_type = ZPL_MEDIA_VIDEO;        //音频视频
        bufdata->skb_header.media_header.buffer_codec = session->video_session.payload;       //编码类型
        //bufdata->buffer_key = 0;         //帧类型
        //bufdata->buffer_priv = 0;
        //bufdata->buffer_timetick = 0;    //时间戳 毫秒
        //bufdata->buffer_seq = 0;         //序列号 底层序列号
        session->video_session.user_timestamp += session->video_session.timestamp_interval;
    }
    else if(!bvideo && bufdata->skb_len)
    {
        bufdata->skb_header.media_header.buffer_type = ZPL_MEDIA_AUDIO;        //音频视频
        bufdata->skb_header.media_header.buffer_codec = session->audio_session.payload;       //编码类型
        //bufdata->buffer_key = 0;         //帧类型
        //bufdata->buffer_priv = 0;
        //bufdata->buffer_timetick = 0;    //时间戳 毫秒
        //bufdata->buffer_seq = 0;         //序列号 底层序列号
        session->audio_session.user_timestamp += session->audio_session.timestamp_interval;
    }
    else
    {
        if(bvideo)
            session->video_session.user_timestamp += session->video_session.timestamp_interval;//rtp_session_get_current_recv_ts(session->video_session.rtp_session);
        else
            session->audio_session.user_timestamp += session->audio_session.timestamp_interval;//rtp_session_get_current_recv_ts(session->audio_session.rtp_session);
    }
    if(bufdata->skb_header.media_header.buffer_type == ZPL_MEDIA_AUDIO)
        fprintf(stdout, "===============rtsp_media_rtp_recv audio %d byte timestamp=%d(ret=%d)\n", bufdata->skb_len, session->audio_session.user_timestamp, ret);
    else if(bufdata->skb_header.media_header.buffer_type == ZPL_MEDIA_VIDEO)
        fprintf(stdout, "===============rtsp_media_rtp_recv video %d byte timestamp=%d(ret=%d)\n", bufdata->skb_len, session->video_session.user_timestamp, ret);
    fflush(stdout);
    return bufdata->skb_len;

}

static int rtsp_media_codec2pt(rtsp_session_t *session, rtsp_media_t *media)
{
    if(zpl_media_getptr(media)->video_media.enable)
    {
        zpl_video_codec_t *pcodec = &zpl_media_getptr(media)->video_media.codec;
        if(pcodec)
        {
            session->video_session.payload = pcodec->enctype;
            session->video_session.framerate = pcodec->framerate;
        }
    }
    if(zpl_media_getptr(media)->audio_media.enable)
    {
        zpl_audio_codec_t *paudiocodec = &zpl_media_getptr(media)->audio_media.codec;
        if(paudiocodec)
        {
            session->audio_session.payload = paudiocodec->enctype;
            session->audio_session.framerate = paudiocodec->framerate;
        }
    }
    return 0;
}


static int rtsp_media_srv_setup(rtsp_media_t *media, rtsp_session_t *session)
{
    int ret = 0;

    rtsp_media_codec2pt(session, media);

    if(zpl_media_getptr(media)->video_media.enable && zpl_media_getptr(media)->video_media.halparam)
    {
        session->video_session.timestamp_interval = rtp_profile_get_clock_rate(session->video_session.payload)/session->video_session.framerate;
        fprintf(stdout, " rtsp_media_srv_setup video timestamp_interval=%d\r\n", session->video_session.timestamp_interval);
    }
    else if(zpl_media_getptr(media)->audio_media.enable && zpl_media_getptr(media)->audio_media.halparam)
    {
        session->audio_session.timestamp_interval = rtp_profile_get_clock_rate(session->audio_session.payload)/session->audio_session.framerate;
        fprintf(stdout, " rtsp_media_srv_setup audio timestamp_interval=%d\r\n", session->audio_session.timestamp_interval);
    }

    if(zpl_media_channel_gettype(media) == ZPL_MEDIA_CHANNEL_FILE)
    {
        if(zpl_media_getptr(media)->video_media.enable && zpl_media_getptr(media)->video_media.halparam)
        {
            zpl_media_file_master(zpl_media_getptr(media)->video_media.halparam, eloop_master_module_lookup(MODULE_ZPLMEDIA), 1000/25);
            zpl_media_file_pdata(zpl_media_getptr(media)->video_media.halparam, session);
            zpl_media_file_get_frame_callback(zpl_media_getptr(media)->video_media.halparam, _h264_file_get_frame);
        }
        else if(zpl_media_getptr(media)->audio_media.enable && zpl_media_getptr(media)->audio_media.halparam)
        {
            zpl_media_file_master(zpl_media_getptr(media)->audio_media.halparam, eloop_master_module_lookup(MODULE_ZPLMEDIA), 1000/25);
            zpl_media_file_pdata(zpl_media_getptr(media)->audio_media.halparam, session);
            zpl_media_file_get_frame_callback(zpl_media_getptr(media)->audio_media.halparam, _h264_file_get_frame);
        }
        zpl_media_channel_client_add(media, rtsp_media_rtp_sendto, session);
    }
    return ret;
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

        RTSP_TRACE(" %s        :%d\r\n", bvideo?"video":"audio", rtpsession->b_enable);
        RTSP_TRACE(" trackid      :%d\r\n", rtpsession->i_trackid);
        RTSP_TRACE(" payload      :%d\r\n", rtpsession->payload);
        RTSP_TRACE(" interval     :%d\r\n", rtpsession->timestamp_interval);
        RTSP_TRACE(" pt           :%d\r\n", rtpmap.pt);
        RTSP_TRACE(" enc_name     :%s\r\n", rtpmap.enc_name);
        RTSP_TRACE(" clock_rate   :%d\r\n", rtpmap.clock_rate);
        if(client_media && bvideo)
            RTSP_TRACE(" size         :%dx%d\r\n", client_media->video_codec.vidsize.width, client_media->video_codec.vidsize.height);
        RTSP_TRACE(" framerate    :%d\r\n", rtpsession->framerate);
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






int rtsp_media_build_sdptext(rtsp_session_t * session, rtsp_media_t *media, uint8_t *sdp)
{
    int sdplength = 0, ret = 0;
    zpl_media_channel_t *chn = (zpl_media_channel_t*) media;
    if(chn && session->video_session.b_enable)
    {
        sdplength = sprintf((char*)sdp + sdplength, "m=video 0 RTP/AVP %d\r\n", session->video_session.payload);
        ret = rtsp_media_adap_build_sdp(session->video_session.payload, session, sdp + sdplength, 0);
        if(ret == 0)
        {
            if (rtp_profile_get_rtpmap(session->video_session.payload))
            {
                sdplength += sprintf(sdp + sdplength, "a=rtpmap:%d %s\r\n", session->video_session.payload,
                                     rtp_profile_get_rtpmap(session->video_session.payload));

                sdplength += sprintf((char*)sdp + sdplength, "a=framesize:%d %d-%d\r\n",
                                     session->video_session.payload,
                                     chn->video_media.codec.vidsize.width,
                                     chn->video_media.codec.vidsize.height);
                sdplength += sprintf((char*)sdp + sdplength, "a=framerate:%u\r\n", session->video_session.framerate);

                if(zpl_media_channel_gettype(media) == ZPL_MEDIA_CHANNEL_FILE)
                {
                    zpl_media_file_t * file = (zpl_media_file_t *)chn->video_media.halparam;
                    uint32_t npttime = file->filedesc.endtime - file->filedesc.begintime;
                    if(npttime)
                        sdplength += sprintf((char*)sdp + sdplength, "a=range:npt=0.000-%d.%d\r\n", npttime/1000, npttime%1000);
                    else
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

            sdplength += sprintf((char*)sdp + sdplength, "a=framesize:%d %d-%d\r\n",
                                 session->video_session.payload,
                                 chn->video_media.codec.vidsize.width,
                                 chn->video_media.codec.vidsize.height);
            sdplength += sprintf((char*)sdp + sdplength, "a=framerate:%u\r\n", session->video_session.framerate);
            if(zpl_media_channel_gettype(media) == ZPL_MEDIA_CHANNEL_FILE)
            {
                zpl_media_file_t * file = (zpl_media_file_t *)chn->video_media.halparam;
                uint32_t npttime = file->filedesc.endtime - file->filedesc.begintime;
                if(npttime)
                    sdplength += sprintf((char*)sdp + sdplength, "a=range:npt=0.000-%d.%d\r\n", npttime/1000, npttime%1000);
                else
                    sdplength += sprintf((char*)sdp + sdplength, "a=range:npt=now-\r\n");
            }
            else
            {
                sdplength += sprintf((char*)sdp + sdplength, "a=range:npt=now-\r\n");
            }
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
                if(zpl_media_channel_gettype(media) == ZPL_MEDIA_CHANNEL_FILE)
                {
                    zpl_media_file_t * file = (zpl_media_file_t *)chn->audio_media.halparam;
                    uint32_t npttime = file->filedesc.endtime - file->filedesc.begintime;
                    if(npttime)
                        sdplength += sprintf((char*)sdp + sdplength, "a=range:npt=0.000-%d.%d\r\n", npttime/1000, npttime%1000);
                    else
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
            sdplength += sprintf((char*)sdp + sdplength, "a=framerate:%d\r\n", session->audio_session.framerate);
            if(zpl_media_channel_gettype(media) == ZPL_MEDIA_CHANNEL_FILE)
            {
                zpl_media_file_t * file = (zpl_media_file_t *)chn->audio_media.halparam;
                uint32_t npttime = file->filedesc.endtime - file->filedesc.begintime;
                if(npttime)
                    sdplength += sprintf((char*)sdp + sdplength, "a=range:npt=0.000-%d.%d\r\n", npttime/1000, npttime%1000);
                else
                    sdplength += sprintf((char*)sdp + sdplength, "a=range:npt=now-\r\n");
            }
            else
            {
                sdplength += sprintf((char*)sdp + sdplength, "a=range:npt=now-\r\n");
            }
        }
        //sdplength += rtsp_server_build_sdp_audio(session, sdp + sdplength);
    }
    return sdplength;
}




rtsp_code rtsp_media_handle_option(rtsp_session_t * session, rtsp_media_t *media)
{
    rtsp_rtp_handle_options(session, media);
    if(session->bsrv)
        return RTSP_STATE_CODE_200;
    else
        return RTSP_STATE_CODE_200;
}


rtsp_code rtsp_media_handle_describe(rtsp_session_t * session, rtsp_media_t *media)
{
    if(rtsp_media_attr_analysis(session, media) != 0)
    {
        return RTSP_STATE_CODE_300;
    }

    rtsp_rtp_handle_describe(session, media);
    if(session->bsrv)
        return RTSP_STATE_CODE_200;
    else
        return RTSP_STATE_CODE_200;
}

rtsp_code rtsp_media_handle_setup(rtsp_session_t * session, rtsp_media_t *media)
{
    rtsp_rtp_handle_setup(session, media);
    if(session->bsrv)
        return RTSP_STATE_CODE_200;
    else
        return RTSP_STATE_CODE_200;
}

rtsp_code rtsp_media_handle_teardown(rtsp_session_t * session, rtsp_media_t *media)
{
    rtsp_rtp_handle_teardown(session, media);
    if(session->bsrv)
        return RTSP_STATE_CODE_200;
    else
        return RTSP_STATE_CODE_200;
}

rtsp_code rtsp_media_handle_play(rtsp_session_t * session, rtsp_media_t *media)
{
    rtsp_media_start(session, media, true);
    rtsp_rtp_handle_play(session, media);
    if(session->bsrv)
        return RTSP_STATE_CODE_200;
    else
        return RTSP_STATE_CODE_200;
}

rtsp_code rtsp_media_handle_pause(rtsp_session_t * session, rtsp_media_t *media)
{
    rtsp_media_start(session, media, false);
    rtsp_rtp_handle_pause(session, media);
    if(session->bsrv)
        return RTSP_STATE_CODE_200;
    else
        return RTSP_STATE_CODE_200;
}

rtsp_code rtsp_media_handle_scale(rtsp_session_t * session, rtsp_media_t *media)
{
    rtsp_rtp_handle_scale(session, media);
    if(session->bsrv)
        return RTSP_STATE_CODE_200;
    else
        return RTSP_STATE_CODE_200;
}

rtsp_code rtsp_media_handle_get_parameter(rtsp_session_t * session, rtsp_media_t *media)
{
    rtsp_rtp_handle_get_parameter(session, media);
    if(session->bsrv)
        return RTSP_STATE_CODE_200;
    else
        return RTSP_STATE_CODE_200;
}

rtsp_code rtsp_media_handle_set_parameter(rtsp_session_t * session, rtsp_media_t *media)
{
    rtsp_rtp_handle_set_parameter(session, media);
    if(session->bsrv)
        return RTSP_STATE_CODE_200;
    else
        return RTSP_STATE_CODE_200;
}
