/*
 * Copyright © 2001-2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __RTSP_RTP_H__
#define __RTSP_RTP_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "zpl_rtsp_def.h"
#include "zpl_rtsp_session.h"
#include "zpl_rtsp_media.h"
#include "zpl_rtsp_sdpfmtp.h"
#include "ortp/ortp.h"
#include "ortp/rtpsession.h"






RTSP_API int rtsp_rtp_init(void);
RTSP_API int rtsp_rtp_start(void);


RTSP_API int rtsp_rtp_handle_options(rtsp_session_t*, void *);
RTSP_API int rtsp_rtp_handle_describe(rtsp_session_t*, void *);
RTSP_API int rtsp_rtp_handle_setup(rtsp_session_t*, void *);
RTSP_API int rtsp_rtp_handle_teardown(rtsp_session_t*, void *);
RTSP_API int rtsp_rtp_handle_play(rtsp_session_t*, void *);
RTSP_API int rtsp_rtp_handle_pause(rtsp_session_t*, void *);
RTSP_API int rtsp_rtp_handle_scale(rtsp_session_t*, void *);
RTSP_API int rtsp_rtp_handle_set_parameter(rtsp_session_t*, void *);
RTSP_API int rtsp_rtp_handle_get_parameter(rtsp_session_t*, void *);



RTSP_API int rtsp_rtp_select(rtsp_session_t* session);

RTSP_API int rtsp_rtp_tcp_forward(rtsp_session_t* session,  const uint8_t *buffer, uint32_t len);
RTSP_API int rtsp_rtp_send(rtsp_session_t* session, bool bvideo, const uint8_t *buffer, uint32_t len, uint8_t flags);
RTSP_API int rtsp_rtp_recv(rtsp_session_t* session, uint8_t *buffer, uint32_t len, bool bvideo, int *more);

RTSP_API int rtsp_srvread(rtsp_session_t* rtsp_session);

typedef struct
{
    int u32X;
    int u32Y;
} ST_Point_T;

int st_app_osd_DrawText(unsigned short *yuv, int u32Width, int u32Height, ST_Point_T stPoint, const char *szString, int u32Color);

#ifdef __cplusplus
}
#endif

#endif /* __RTSP_RTP_H__ */
