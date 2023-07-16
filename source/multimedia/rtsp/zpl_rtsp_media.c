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

//#include <ortp/ortp.h>

#include "zpl_rtsp.h"
#include "zpl_rtsp_util.h"
#include "zpl_rtsp_transport.h"
#include "zpl_rtsp_sdp.h"
#include "zpl_rtsp_sdpfmtp.h"
#include "zpl_rtsp_auth.h"
#include "zpl_rtsp_server.h"
#include "zpl_rtsp_session.h"
#include "zpl_rtsp_client.h"
#include "zpl_rtsp_media.h"


#if 0
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
            out_size = os_base64_decode(tmp, (const char*)start, sizeof(tmp));
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
    session->mediartp_session.packetization_mode = h264.packetization_mode;


    rtsp_session_media_sprop_parameterset_parse_h264(h264.sprop_parameter_sets,
                                   h264.sprop_parameter_sets_len, &extradata);
 
    zpl_media_channel_decode_sps(extradata.fSPS, extradata.fSPSSize, &extradata.h264spspps);

    client->client_media.video_codec.vidsize.width = extradata.h264spspps.vidsize.width;
    client->client_media.video_codec.vidsize.height = extradata.h264spspps.vidsize.height;
    client->client_media.video_codec.framerate = extradata.h264spspps.fps;
    client->client_media.video_codec.profile = extradata.h264spspps.profile; 
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
    zpl_mediartp_session_t *rtpsession = bvideo?&session->mediartp_session:&session->bind_other_session;
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
                            client_media->video_codec.codectype = rtpsession->payload;
                        else if(client_media && !bvideo)
                            client_media->audio_codec.codectype = rtpsession->payload;

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
        session->mediartp_session.rtpmode = RTP_SESSION_SENDRECV;
        session->bind_other_session.rtpmode = RTP_SESSION_SENDRECV;

        struct sdp_media *m = sdp_media_find(session->sdptext.media,
                                             session->sdptext.media_count, "video", NULL);
        if(m)
        {
            session->mediartp_session.b_enable = true;
            ret = rtsp_session_media_fmtp_attr_parse(m, session, pUser, true);
        }

        m = sdp_media_find(session->sdptext.media,
                           session->sdptext.media_count, "audio", NULL);
        if(m)
        {
            session->bind_other_session.b_enable = true;
            ret = rtsp_session_media_fmtp_attr_parse(m, session, pUser, false);
        }
    }
    else
    {
        session->mediartp_session.rtpmode = RTP_SESSION_SENDRECV;
        session->bind_other_session.rtpmode = RTP_SESSION_SENDRECV;
    }
    return ret;
}
#endif




#if 0
int rtsp_session_media_tcp_forward(rtsp_session_t* session, const uint8_t *buffer, uint32_t len)
{
    zpl_socket_t sock = ZPL_SOCKET_INVALID;
    uint8_t  channel = buffer[1];
    if(session->mediartp_session.media_chn)
    {
        if(session->mediartp_session.rtp_interleaved == channel)
            sock = session->mediartp_session.rtp_sock;
        else if(session->mediartp_session.rtcp_interleaved == channel)
            sock = session->mediartp_session.rtcp_sock;
    }
    if(session->bind_other_session.media_chn)
    {
        if(session->bind_other_session.rtp_interleaved == channel)
            sock = session->bind_other_session.rtp_sock;
        else if(session->bind_other_session.rtcp_interleaved == channel)
            sock = session->bind_other_session.rtcp_sock;
    }
    if(!ipstack_invalid(sock))
        return rtp_session_tcp_forward(ipstack_fd(sock), buffer + 4, (int)len - 4);
    return OK;
}
#endif



       