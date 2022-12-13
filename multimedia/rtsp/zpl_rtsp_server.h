﻿/*
 * Copyright © 2001-2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __RTSP_SERVER_H__
#define __RTSP_SERVER_H__

#include "zpl_rtsp_def.h"
#include "osker_list.h"
#include "zpl_rtsp_session.h"
#include "zpl_rtsp_media.h"
#include "zpl_rtsp_auth.h"

RTSP_BEGIN_DECLS

#define RTSP_PACKET_MAX     2048

typedef struct rtp_socket_s
{
    uint16_t        local_rtp_port; //本地RTP端口
    uint16_t        local_rtcp_port;//本地RTCP端口
    zpl_socket_t     _rtp_sock;
    zpl_socket_t     _rtcp_sock;
}rtp_socket_t;

typedef struct rtsp_srv_s {
#ifdef ZPL_WORKQUEUE
    void            *t_master;
    void            *t_accept;
    void            *t_read;
    int             t_sock;
#endif
    zpl_socket_t             listen_sock;
    uint16_t        listen_port;
    char            *listen_address;

    fd_set          rset;

    char            *username;
    char            *password;
    char            *realm;
    bool            b_auth;
    char            *srvname;
    rtsp_transport_rtp_t  transport;		/**< Transport ("RTP/AVP")	    */
    bool            unicast;
    uint16_t        rtp_port;
    uint16_t        rtcp_port;

    osker_list_head_t     rtsp_session;

    rtp_socket_t    video_sock;
    rtp_socket_t    audio_sock;

    uint8_t         _recv_buf[RTSP_PACKET_MAX];
    uint8_t         _send_buf[RTSP_PACKET_MAX];
    uint8_t         *_send_build;

    uint32_t        _recv_offset;
    uint32_t        _send_offset;
    uint32_t        _recv_length;
    uint32_t        _send_length;

    uint32_t        debug;
}rtsp_srv_t;


RTSP_API void rtsp_srv_destroy(rtsp_srv_t *ctx);
#ifdef ZPL_WORKQUEUE
RTSP_API rtsp_srv_t* rtsp_srv_create(const char *ip_address, uint16_t port, int pro);
#else
RTSP_API rtsp_srv_t* rtsp_srv_create(const char *ip_address, uint16_t port);
RTSP_API int rtsp_srv_select(rtsp_srv_t *ctx);
#endif
RTSP_API int rtsp_srv_session_create(rtsp_srv_t *ctx, zpl_socket_t sock, const char *ip_address, uint16_t port);
RTSP_API int rtsp_srv_session_destroy(rtsp_srv_t *ctx, zpl_socket_t sock);
RTSP_API rtsp_session_t * rtsp_srv_session_lookup(rtsp_srv_t *ctx, zpl_socket_t sock);


RTSP_END_DECLS

#endif /* __RTSP_SERVER_H__ */
