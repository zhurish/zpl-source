/* $Id$ */
/*
 * Copyright (C) 2008-2013 Teluu Inc. (http://www.teluu.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __PJSUA_MEDIA_STREAM_H__
#define __PJSUA_MEDIA_STREAM_H__

#include <pjlib.h>
#include <pjlib-util.h>
#include <pjmedia.h>
#include <pjmedia-codec.h>
#include <pjmedia/transport_srtp.h>
#include "pjapp_media_file.h"

PJ_BEGIN_DECL

typedef enum pjapp_media_iotype
{
    PJAPP_MEDIA_IOTYPE_NONE  = 0,
    PJAPP_MEDIA_IOTYPE_FILE = 1,
    PJAPP_MEDIA_IOTYPE_RTP  = 2,
    PJAPP_MEDIA_IOTYPE_SRTP = 3,
}pjapp_media_iotype;

typedef struct pjapp_media_data
{
    pjapp_media_iotype type;
    pjmedia_port *_port;
} pjapp_media_data;


typedef struct
{
    pj_pool_t *pool;
    pjmedia_endpt *med_endpt;
    pjmedia_type type;
    pjmedia_dir dir;
    pj_int8_t rtp_pt;
#if defined(PJMEDIA_HAS_VIDEO) && (PJMEDIA_HAS_VIDEO != 0)
    pjmedia_vid_codec_info *video_codec_info;
    pjmedia_vid_codec_param *video_codec_param;
    pjmedia_vid_codec_param vidcodec_param;
#endif
    pjmedia_codec_info *audio_codec_info;
    pjmedia_codec_param audio_codec_param;
	pjmedia_transport *transport;
#if defined(PJMEDIA_HAS_SRTP) && (PJMEDIA_HAS_SRTP != 0)
	pjmedia_transport *srtp_tp;
#endif

#if defined(PJMEDIA_HAS_VIDEO) && (PJMEDIA_HAS_VIDEO != 0)
    pjmedia_vid_stream *video_stream;
#endif
    pjmedia_stream *audio_stream;

    pjmedia_master_port *master_port;
    pjmedia_snd_port *snd_port;
#if defined(PJMEDIA_HAS_VIDEO) && (PJMEDIA_HAS_VIDEO != 0)
    pjmedia_vid_port *vid_port;
#endif
    void *priv;
    pjapp_media_file_data play_filedata;
} pjapp_ms_cfg_t;

typedef struct
{
    pj_sockaddr_in local_addr;
    pj_bool_t mcast;
    pj_sockaddr_in mcast_addr;
    pj_sockaddr_in rem_addr;
#if defined(PJMEDIA_HAS_SRTP) && (PJMEDIA_HAS_SRTP != 0)
    pj_bool_t use_srtp;
    pj_str_t *crypto_suite;
    pj_str_t *srtp_tx_key;
    pj_str_t *srtp_rx_key;
    pj_bool_t is_dtls_client;
    pj_bool_t is_dtls_server;
#endif
} pjapp_ms_param_t;

typedef struct
{
    pjapp_ms_param_t param;
    pjapp_ms_cfg_t  cfg;
} pjapp_ms_t;



pjapp_ms_t * pjapp_media_stream_new(int type, char *local_addr, pj_uint16_t local_port, char *remote_addr, pj_uint16_t remote_port);
pj_status_t pjapp_media_stream_set_address(pj_pool_t *pool, char *local_addr, pj_uint16_t local_port, char *remote_addr, pj_uint16_t remote_port, 
    pjapp_ms_t *cfg);
pj_status_t pjapp_media_stream_set_srtp(pj_pool_t *pool, char *crypto_suite, char *srtp_tx_key, char *srtp_rx_key, 
    pj_bool_t dtls_c, pj_bool_t dtls_s, pjapp_ms_t *cfg);
pj_status_t pjapp_media_stream_codecparam(pjapp_ms_t *ms, char *codec, int w, int h, pjmedia_codec_fmtp *fmtp);
pj_status_t pjapp_media_stream_transport(pjapp_ms_t *ms);
pj_status_t pjapp_media_stream_create(pjapp_ms_t *ms);
pj_status_t pjapp_media_stream_stop(pjapp_ms_t *ms);

int pjapp_app_perror(const char *sender, const char *title,
                      pj_status_t status);
int pjapp_ms_test(void);

PJ_END_DECL

#endif /* __PJSUA_MEDIA_STREAM_H__ */
