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

typedef struct pjmedia_file_data
{
    pjmedia_clock *play_clock;
    const char *file_name;
    pjmedia_port *play_port;
    pjmedia_port *stream_port;
    void *read_buf;
    pj_size_t read_buf_size;
} pjmedia_file_data;

typedef struct pj_factory
{
    pj_caching_pool cach_pool;
    pj_pool_t *pool;
}pj_factory;

typedef struct
{
    pjmedia_endpt *med_endpt;
#if defined(PJMEDIA_HAS_VIDEO) && (PJMEDIA_HAS_VIDEO != 0)
    pjmedia_vid_codec_info *video_codec_info;
    pjmedia_vid_codec_param *video_codec_param;
    pjmedia_vid_codec_param vidcodec_param;
#endif
    pjmedia_codec_info *audio_codec_info;
    pjmedia_type type;
    pjmedia_dir dir;
    pj_int8_t rx_pt;
    pj_int8_t tx_pt;
    pj_uint16_t local_port;
    pj_bool_t mcast;
    pj_sockaddr_in *mcast_addr;
    pj_sockaddr_in *rem_addr;
#if defined(PJMEDIA_HAS_SRTP) && (PJMEDIA_HAS_SRTP != 0)
    pj_bool_t use_srtp;
    pj_str_t *crypto_suite;
    pj_str_t *srtp_tx_key;
    pj_str_t *srtp_rx_key;
    pj_bool_t is_dtls_client;
    pj_bool_t is_dtls_server;
#endif
#if defined(PJMEDIA_HAS_VIDEO) && (PJMEDIA_HAS_VIDEO != 0)
    pjmedia_vid_stream **video_stream;
#endif
    pjmedia_stream **audio_stream;
} pjapp_media_stream_cfg_t;



pj_status_t pjapp_media_stream_create(pj_pool_t *pool, pjapp_media_stream_cfg_t *);

/* Util to display the error message for the specified error code  */
static int app_perror(const char *sender, const char *title,
                      pj_status_t status)
{
    char errmsg[PJ_ERR_MSG_SIZE];

    pj_strerror(status, errmsg, sizeof(errmsg));

    PJ_LOG(3, (sender, "%s: %s [code=%d]", title, errmsg, status));
    return 1;
}

#endif /* __PJSUA_MEDIA_STREAM_H__ */
