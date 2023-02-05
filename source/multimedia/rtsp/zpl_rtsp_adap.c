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
#include "ortp/ortp.h"
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

#include "rtp_payload.h"


static rtsp_session_media_adap_t _rtp_media_adap_tbl[] =
{
    {"H264", RTP_MEDIA_PAYLOAD_H264, rtp_payload_send_h264, NULL},
    {"G711A", RTP_MEDIA_PAYLOAD_G711A, rtp_payload_send_g7xx, NULL},
    {"G711U", RTP_MEDIA_PAYLOAD_G711U, rtp_payload_send_g7xx, NULL},

    {"G722", RTP_MEDIA_PAYLOAD_G722, rtp_payload_send_g7xx, NULL},
    {"G726", RTP_MEDIA_PAYLOAD_G726, rtp_payload_send_g7xx, NULL},

    {"G728", RTP_MEDIA_PAYLOAD_G728, rtp_payload_send_g7xx, NULL},
    {"G729", RTP_MEDIA_PAYLOAD_G729, rtp_payload_send_g7xx, NULL},
    {"G729A", RTP_MEDIA_PAYLOAD_G729A, rtp_payload_send_g7xx, NULL},

    {"PCMA", RTP_MEDIA_PAYLOAD_PCMA, rtp_payload_send_g7xx, NULL},
    {"PCMU", RTP_MEDIA_PAYLOAD_PCMU, rtp_payload_send_g7xx, NULL},
};


int rtsp_session_media_adap_rtp_sendto(uint32_t ptid, rtsp_session_t *session, int type, const uint8_t *buf, uint32_t len)
{
    uint32_t i = 0;
    rtp_session_t   *myrtp_session = NULL;
    if(type == ZPL_MEDIA_VIDEO && session->video_session.b_enable && session->video_session.rtp_session && session->video_session.rtp_state==RTP_SESSION_STATE_START)
    {
        myrtp_session = &session->video_session;                               
    }
    if(type == ZPL_MEDIA_AUDIO && session->audio_session.b_enable && session->audio_session.rtp_session && session->audio_session.rtp_state==RTP_SESSION_STATE_START)
    {
        myrtp_session = &session->audio_session;                               
    }    
    if(myrtp_session && myrtp_session->rtp_session)
    {
        for(i = 0; i < sizeof(_rtp_media_adap_tbl)/sizeof(_rtp_media_adap_tbl[0]); i++)
        {
            if(_rtp_media_adap_tbl[i].ptid == ptid)
            {
                if(_rtp_media_adap_tbl[i]._rtp_sendto)
                return (_rtp_media_adap_tbl[i]._rtp_sendto)(myrtp_session->rtp_session, buf, len, myrtp_session->user_timestamp);
            }
        }
    }
    return -1;
}

int rtsp_session_media_adap_rtp_recv(uint32_t ptid, rtsp_session_t *session, int type, uint8_t *buf, uint32_t len, int *havemore)
{
    uint32_t i = 0;
    rtp_session_t   *myrtp_session = NULL;
    if(type == ZPL_MEDIA_VIDEO && session->video_session.b_enable && session->video_session.rtp_session)
    {
        myrtp_session = &session->video_session;                               
    }
    if(type == ZPL_MEDIA_AUDIO && session->audio_session.b_enable && session->audio_session.rtp_session)
    {
        myrtp_session = &session->audio_session;                               
    }  
    for(i = 0; i < sizeof(_rtp_media_adap_tbl)/sizeof(_rtp_media_adap_tbl[0]); i++)
    {
        if(_rtp_media_adap_tbl[i].ptid == ptid && myrtp_session->rtp_session)
        {
            if(_rtp_media_adap_tbl[i]._rtp_recv)
               return (_rtp_media_adap_tbl[i]._rtp_recv)(myrtp_session->rtp_session, buf, len, myrtp_session->user_timestamp, havemore);
        }
    }
    return -1;
}
