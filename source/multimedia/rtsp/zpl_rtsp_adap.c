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

#ifdef ZPL_LIBRTSP_MODULE
#include "zpl_media.h"
#include "zpl_media_internal.h"
#endif
#include "zpl_rtsp_def.h"
#include "zpl_rtsp_session.h"
#include "zpl_rtsp_media.h"
#include "zpl_rtsp_adap.h"
#include "zpl_rtp_h264.h"
#include "zpl_rtp_g7xx.h"
#include "zpl_rtp_h265.h"

static rtsp_media_adap_t _rtp_media_adap_tbl[] =
{
    {"H264", RTP_MEDIA_PAYLOAD_H264, _rtsp_build_sdp_h264, _rtsp_parse_sdp_h264, NULL, _rtp_unpacket_h264, _rtp_send_h264, NULL},
    {"H265", RTP_MEDIA_PAYLOAD_H265, _rtsp_build_sdp_h265, _rtsp_parse_sdp_h265, NULL, _rtp_unpacket_h265, _rtp_send_h265, NULL},

    {"G711A", RTP_MEDIA_PAYLOAD_G711A, _rtsp_build_sdp_g7xx, _rtsp_parse_sdp_g7xx, NULL, _rtp_unpacket_g7xx, _rtp_send_g7xx, NULL},
    {"G711U", RTP_MEDIA_PAYLOAD_G711U, _rtsp_build_sdp_g7xx, _rtsp_parse_sdp_g7xx, NULL, _rtp_unpacket_g7xx, _rtp_send_g7xx, NULL},

    {"G722", RTP_MEDIA_PAYLOAD_G722, _rtsp_build_sdp_g7xx, _rtsp_parse_sdp_g7xx, NULL, _rtp_unpacket_g7xx, _rtp_send_g7xx, NULL},
    {"G726", RTP_MEDIA_PAYLOAD_G726, _rtsp_build_sdp_g7xx, _rtsp_parse_sdp_g7xx, NULL, _rtp_unpacket_g7xx, _rtp_send_g7xx, NULL},

    {"G728", RTP_MEDIA_PAYLOAD_G728, _rtsp_build_sdp_g7xx, _rtsp_parse_sdp_g7xx, NULL, _rtp_unpacket_g7xx, _rtp_send_g7xx, NULL},
    {"G729", RTP_MEDIA_PAYLOAD_G729, _rtsp_build_sdp_g7xx, _rtsp_parse_sdp_g7xx, NULL, _rtp_unpacket_g7xx, _rtp_send_g7xx, NULL},
    {"G729A", RTP_MEDIA_PAYLOAD_G729A, _rtsp_build_sdp_g7xx, _rtsp_parse_sdp_g7xx, NULL, _rtp_unpacket_g7xx, _rtp_send_g7xx, NULL},

    {"PCMA", RTP_MEDIA_PAYLOAD_PCMA, _rtsp_build_sdp_g7xx, _rtsp_parse_sdp_g7xx, NULL, _rtp_unpacket_g7xx, _rtp_send_g7xx, NULL},
    {"PCMU", RTP_MEDIA_PAYLOAD_PCMU, _rtsp_build_sdp_g7xx, _rtsp_parse_sdp_g7xx, NULL, _rtp_unpacket_g7xx, _rtp_send_g7xx, NULL},
};

int rtsp_media_adap_build_sdp(uint32_t ptid, rtsp_session_t *session, uint8_t *buf, uint32_t len)
{
    uint32_t i = 0;
    for(i = 0; i < sizeof(_rtp_media_adap_tbl)/sizeof(_rtp_media_adap_tbl[0]); i++)
    {
        if(_rtp_media_adap_tbl[i].ptid == ptid)
        {
            if(_rtp_media_adap_tbl[i]._build_sdp)
               return (_rtp_media_adap_tbl[i]._build_sdp)(session, buf, len);
        }
    }
    return 0;
}

int rtsp_media_adap_parse_sdp(uint32_t ptid, rtsp_session_t *session, const uint8_t *buf, uint32_t len)
{
    uint32_t i = 0;
    for(i = 0; i < sizeof(_rtp_media_adap_tbl)/sizeof(_rtp_media_adap_tbl[0]); i++)
    {
        if(_rtp_media_adap_tbl[i].ptid == ptid)
        {
            if(_rtp_media_adap_tbl[i]._parse_sdp)
               return (_rtp_media_adap_tbl[i]._parse_sdp)(session, buf, len);
        }
    }
    return -1;
}

int rtsp_media_adap_rtp_packet(uint32_t ptid, void *packet, const uint8_t *buf, uint32_t len)
{
    uint32_t i = 0;
    for(i = 0; i < sizeof(_rtp_media_adap_tbl)/sizeof(_rtp_media_adap_tbl[0]); i++)
    {
        if(_rtp_media_adap_tbl[i].ptid == ptid)
        {
            if(_rtp_media_adap_tbl[i]._rtp_packet)
               return (_rtp_media_adap_tbl[i]._rtp_packet)(packet, buf, len);
        }
    }
    return -1;
}

int rtsp_media_adap_rtp_unpacket(uint32_t ptid, void *packet,  uint8_t *buf, uint32_t len)
{
    uint32_t i = 0;
    for(i = 0; i < sizeof(_rtp_media_adap_tbl)/sizeof(_rtp_media_adap_tbl[0]); i++)
    {
        if(_rtp_media_adap_tbl[i].ptid == ptid)
        {
            if(_rtp_media_adap_tbl[i]._rtp_unpacket)
               return (_rtp_media_adap_tbl[i]._rtp_unpacket)(packet, buf, len);
        }
    }
    return -1;
}
int rtsp_media_adap_rtp_sendto(uint32_t ptid, rtsp_session_t *session, int type, const uint8_t *buf, uint32_t len)
{
    uint32_t i = 0;
    for(i = 0; i < sizeof(_rtp_media_adap_tbl)/sizeof(_rtp_media_adap_tbl[0]); i++)
    {
        if(_rtp_media_adap_tbl[i].ptid == ptid)
        {
            if(_rtp_media_adap_tbl[i]._rtp_sendto)
               return (_rtp_media_adap_tbl[i]._rtp_sendto)(session, type, buf, len);
        }
    }
    return -1;
}

int rtsp_media_adap_rtp_recv(uint32_t ptid, rtsp_session_t *session, int type, uint8_t *buf, uint32_t len)
{
    uint32_t i = 0;
    for(i = 0; i < sizeof(_rtp_media_adap_tbl)/sizeof(_rtp_media_adap_tbl[0]); i++)
    {
        if(_rtp_media_adap_tbl[i].ptid == ptid)
        {
            if(_rtp_media_adap_tbl[i]._rtp_recv)
               return (_rtp_media_adap_tbl[i]._rtp_recv)(session, type, buf, len);
        }
    }
    return -1;
}
