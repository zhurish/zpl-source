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

typedef struct
{
    SessionSet *r_session_set;
    SessionSet *w_session_set;
    int count;
} rtsp_session_rtp_scheduler;



RTSP_API int rtsp_session_rtp_init(void);
RTSP_API int rtsp_session_rtp_start(void);

RTSP_API int rtsp_session_rtp_setup(rtsp_session_t* session, int isvideo);
RTSP_API int rtsp_session_rtp_teardown(rtsp_session_t*);

RTSP_API int rtsp_session_rtp_tcp_forward(rtsp_session_t* session,  const uint8_t *buffer, uint32_t len);


#ifdef __cplusplus
}
#endif

#endif /* __RTSP_RTP_H__ */
