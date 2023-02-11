/*
 * Copyright © 2001-2013 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */


#include <ortp/ortp.h>

#include "zpl_rtsp.h"
#include "zpl_rtsp_util.h"
#include "zpl_rtsp_transport.h"
#include "zpl_rtsp_sdp.h"
#include "zpl_rtsp_sdpfmtp.h"
#include "zpl_rtsp_auth.h"
#include "zpl_rtsp_session.h"
#include "zpl_rtsp_client.h"
#include "zpl_rtsp_media.h"
#include "zpl_rtsp_adap.h"
#include "zpl_rtsp_rtp.h"
#include "zpl_rtsp_server.h"

#define rtsp_srv_getptr(m)             (((rtsp_srv_t*)m))
#define ortp_socketpair ipstack_socketpair

extern void _rtp_session_apply_socket_sizes(RtpSession * session);

int rtsp_session_rtp_init(void)
{
    ortp_init();

    return OK;
}

int rtsp_session_rtp_start(void)
{
    ortp_scheduler_init();
    return OK;
}


static void rtsp_session_rtp_timestamp_jump(RtpSession *session)
{
    rtsp_log_debug("======================rtp_session_timestamp_jump !\n");
}

int rtsp_session_rtp_setup(rtsp_session_t *session, int isvideo)
{
    int ret = 0;
    bool bcreate = false;
    rtp_session_t *_rtpsession = isvideo ? &session->video_session : &session->audio_session;
    if (_rtpsession == NULL)
        return ERROR;
    if (_rtpsession->rtp_session)
    {
        bcreate = false;
    }
    else
    {
        if (session->bsrv)
            _rtpsession->rtpmode = RTP_SESSION_SENDRECV; // RTP_SESSION_SENDONLY;// RTP_SESSION_SENDRECV
        else
            _rtpsession->rtpmode = RTP_SESSION_RECVONLY;
        _rtpsession->rtp_session = rtp_session_new(_rtpsession->rtpmode);
        bcreate = true;
    }
    if (_rtpsession->rtp_session)
    {
        if (_rtpsession->local_ssrc == 0)
            _rtpsession->local_ssrc = (uint32_t)_rtpsession->rtp_session;
        rtp_session_set_recv_buf_size(_rtpsession->rtp_session, 165530);
        rtp_session_set_send_buf_size(_rtpsession->rtp_session, 65530);

        rtp_session_set_seq_number(_rtpsession->rtp_session, 0);

        rtp_session_set_ssrc(_rtpsession->rtp_session, _rtpsession->local_ssrc);

        rtp_session_set_payload_type(_rtpsession->rtp_session, _rtpsession->payload);
        /*rtp_session_set_symmetric_rtp(_rtpsession->rtp_session, true);
        rtp_session_enable_adaptive_jitter_compensation(_rtpsession->rtp_session, true);
        rtp_session_set_jitter_compensation(_rtpsession->rtp_session, 40);
*/
        rtp_session_enable_rtcp(_rtpsession->rtp_session, true);

        rtp_session_signal_connect(_rtpsession->rtp_session, "timestamp_jump", (RtpCallback)rtsp_session_rtp_timestamp_jump, 0);
        rtp_session_signal_connect(_rtpsession->rtp_session, "ssrc_changed", (RtpCallback)rtp_session_reset, 0);
        rtp_session_set_scheduling_mode(_rtpsession->rtp_session, true);
        rtp_session_set_blocking_mode(_rtpsession->rtp_session, false);
        if (session->bsrv)
        {
            ret = rtp_session_set_remote_addr_and_port(_rtpsession->rtp_session,
                                                 session->address,
                                                 _rtpsession->transport.rtp.unicast.rtp_port,
                                                 _rtpsession->transport.rtp.unicast.rtcp_port);
            if(ret != 0)
                return ERROR;                                     
        }
        if (bcreate == false)
        {
            ret = rtp_session_add_aux_remote_addr_full(_rtpsession->rtp_session,
                                                 session->address,
                                                 _rtpsession->transport.rtp.unicast.rtp_port,
                                                 session->address,
                                                 _rtpsession->transport.rtp.unicast.rtcp_port);
            // ORTP_PUBLIC int rtp_session_del_aux_remote_addr_full(RtpSession * session, const char * rtp_addr, int rtp_port, const char * rtcp_addr, int rtcp_port);
            if (session->parent && RTSP_DEBUG_FLAG(rtsp_srv_getptr(session->parent)->debug, EVENT))
            {
                rtsp_log_debug("RTSP Media set remote rtp-port %d rtcp-port %d local rtp-port %d rtcp-port %d payload %d for %d/%d or %s",
                               _rtpsession->transport.rtp.unicast.rtp_port, _rtpsession->transport.rtp.unicast.rtcp_port,
                               _rtpsession->transport.rtp.unicast.local_rtp_port, _rtpsession->transport.rtp.unicast.local_rtcp_port,
                               _rtpsession->payload, session->mchannel, session->mlevel, session->mfilepath ? session->mfilepath : "nil");
            }
            if(ret != 0)
                return ERROR; 
            return OK;    
        }
        ret = rtp_session_set_local_addr(_rtpsession->rtp_session, NULL,
                                   _rtpsession->transport.rtp.unicast.local_rtp_port,
                                   _rtpsession->transport.rtp.unicast.local_rtcp_port);
        if (session->parent && RTSP_DEBUG_FLAG(rtsp_srv_getptr(session->parent)->debug, EVENT))
        {
            rtsp_log_debug("RTSP Media set local rtp-port %d rtcp-port %d for %d/%d or %s",
                           _rtpsession->transport.rtp.unicast.local_rtp_port, _rtpsession->transport.rtp.unicast.local_rtcp_port,
                           session->mchannel, session->mlevel, session->mfilepath ? session->mfilepath : "nil");
        }
        if(ret != 0)
            return ERROR; 
        if (_rtpsession->transport.proto == RTSP_TRANSPORT_RTP_RTPOVERRTSP ||
            _rtpsession->transport.proto == RTSP_TRANSPORT_RTP_TCP)
        {
            zpl_socket_t rtp_recv[2] = {0, 0};
            zpl_socket_t rtcp_recv[2] = {0, 0};
            if (ipstack_invalid(_rtpsession->rtp_sock))
            {
                ortp_socketpair(IPSTACK_OS, AF_UNIX, SOCK_STREAM, 0, rtp_recv);
                if (!ipstack_invalid(rtp_recv[0]) && !ipstack_invalid(rtp_recv[1]))
                {
                    ipstack_set_nonblocking(rtp_recv[0]);
                    ipstack_set_nonblocking(rtp_recv[1]);
                    _rtpsession->rtp_sock = rtp_recv[0];
                }
            }
            if (ipstack_invalid(_rtpsession->rtcp_sock))
            {
                ortp_socketpair(IPSTACK_OS, AF_UNIX, SOCK_STREAM, 0, rtcp_recv);
                if (!ipstack_invalid(rtcp_recv[0]) && !ipstack_invalid(rtcp_recv[1]))
                {
                    _rtpsession->rtcp_sock = rtcp_recv[0];
                    ipstack_set_nonblocking(rtcp_recv[0]);
                    ipstack_set_nonblocking(rtcp_recv[1]);
                }
            }
            if (!ipstack_invalid(rtp_recv[1]) && !ipstack_invalid(rtcp_recv[1]))
            {
                rtp_session_set_sockets(_rtpsession->rtp_session,
                                        ipstack_fd(rtp_recv[1]), ipstack_fd(rtcp_recv[1]));
                rtp_session_set_overtcp(_rtpsession->rtp_session, true,
                                        _rtpsession->transport.rtp_interleaved,
                                        _rtpsession->transport.rtcp_interleaved);
                _rtp_session_apply_socket_sizes(_rtpsession->rtp_session);
            }
        }
    }
    return ret;
}

int rtsp_session_rtp_teardown(rtsp_session_t* session)
{
    if(session->audio_session.rtp_session)
    {
        session->audio_session.rtp_state = RTP_SESSION_STATE_CLOSE;
        rtp_session_destroy(session->audio_session.rtp_session);
        session->audio_session.rtp_session = NULL;
    }
    if(session->video_session.rtp_session)
    {
        session->video_session.rtp_state = RTP_SESSION_STATE_CLOSE;
        rtp_session_destroy(session->video_session.rtp_session);
        session->video_session.rtp_session = NULL;
    }
	ortp_global_stats_display();
    return OK;
}




int rtsp_session_rtp_tcp_forward(rtsp_session_t* session, const uint8_t *buffer, uint32_t len)
{
    zpl_socket_t sock = ZPL_SOCKET_INVALID;
    uint8_t  channel = buffer[1];
    if(session->video_session.transport.rtp_interleaved == channel)
        sock = session->video_session.rtp_sock;
    else if(session->video_session.transport.rtcp_interleaved == channel)
        sock = session->video_session.rtcp_sock;
    else if(session->audio_session.transport.rtp_interleaved == channel)
        sock = session->audio_session.rtp_sock;
    else if(session->audio_session.transport.rtcp_interleaved == channel)
        sock = session->audio_session.rtcp_sock;
    if(!ipstack_invalid(sock))
        return rtp_session_tcp_forward(ipstack_fd(sock), buffer + 4, (int)len - 4);
    return OK;
}




