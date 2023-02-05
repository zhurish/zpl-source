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
#include "zpl_rtsp_base64.h"
#include "zpl_rtsp_auth.h"
#include "zpl_rtsp_session.h"
#include "zpl_rtsp_client.h"
#include "zpl_rtsp_media.h"
#include "zpl_rtsp_adap.h"
#include "zpl_rtsp_server.h"


static int rtsp_srv_session_event_handle(rtsp_srv_t *ctx, rtsp_session_t *session);
#ifdef ZPL_WORKQUEUE
static int rtsp_srv_accept_eloop(struct eloop *ctx);
static int rtsp_srv_session_read_eloop(struct eloop *ctx);
#endif

void rtsp_srv_destroy(rtsp_srv_t *ctx)
{
    RTSP_SRV_LOCK(ctx);

#ifdef ZPL_WORKQUEUE

    if (ctx->t_accept)
        eloop_cancel(ctx->t_accept);
    if (ctx->t_read)
        eloop_cancel(ctx->t_read);
    if (ctx->t_master)
        eloop_master_free(ctx->t_master);
#endif
    rtsp_session_exit();
    if (!ipstack_invalid(ctx->audio_sock._rtp_sock))
    {
        ipstack_close(ctx->audio_sock._rtp_sock);
        ctx->audio_sock._rtp_sock = ZPL_SOCKET_INVALID;
    }
    if (!ipstack_invalid(ctx->audio_sock._rtcp_sock))
    {
        ipstack_close(ctx->audio_sock._rtcp_sock);
        ctx->audio_sock._rtcp_sock = ZPL_SOCKET_INVALID;
    }
    if (!ipstack_invalid(ctx->video_sock._rtp_sock))
    {
        ipstack_close(ctx->video_sock._rtp_sock);
        ctx->video_sock._rtp_sock = ZPL_SOCKET_INVALID;
    }
    if (!ipstack_invalid(ctx->video_sock._rtcp_sock))
    {
        ipstack_close(ctx->video_sock._rtcp_sock);
        ctx->video_sock._rtcp_sock = ZPL_SOCKET_INVALID;
    }

    if (!ipstack_invalid(ctx->listen_sock))
    {
        ipstack_close(ctx->listen_sock);
        ctx->listen_sock = ZPL_SOCKET_INVALID;
    }
    if (ctx->srvname)
    {
        free(ctx->srvname);
        ctx->srvname = NULL;
    }
    if (ctx->username)
    {
        free(ctx->username);
        ctx->username = NULL;
    }
    if (ctx->password)
    {
        free(ctx->password);
        ctx->password = NULL;
    }
    if (ctx->realm)
    {
        free(ctx->realm);
        ctx->realm = NULL;
    }
    if (ctx->listen_address)
    {
        free(ctx->listen_address);
        ctx->listen_address = NULL;
    }
    RTSP_SRV_UNLOCK(ctx);

    free(ctx);
}
#ifdef ZPL_WORKQUEUE
rtsp_srv_t *rtsp_srv_create(const char *ip, uint16_t port, int pro)
#else
rtsp_srv_t *rtsp_srv_create(const char *ip, uint16_t port)
#endif
{
    zpl_socket_t listen_sock;
    rtsp_srv_t *ctx = NULL;
    listen_sock = rtsp_session_listen(ip, port);
    if (ipstack_invalid(listen_sock))
    {
        return NULL;
    }

    ctx = (rtsp_srv_t *)malloc(sizeof(rtsp_srv_t));
    if (ctx == NULL)
    {
        return NULL;
    }
    memset(ctx, 0, sizeof(rtsp_srv_t));
    ctx->listen_sock = listen_sock;
    if (ip)
        ctx->listen_address = strdup(ip);
    else
        ctx->listen_address = NULL;
    ctx->srvname = strdup(RTSP_SRV_NAME);
    ctx->listen_port = port;
    ctx->_recv_offset = 4;
    ctx->_send_offset = 4;
    ctx->_recv_length = 0;
    ctx->_send_length = 0;

    ctx->audio_sock.local_rtp_port = AUDIO_RTP_PORT_DEFAULT;
    ctx->audio_sock.local_rtcp_port = AUDIO_RTCP_PORT_DEFAULT;
    ctx->video_sock.local_rtp_port = VIDEO_RTP_PORT_DEFAULT;
    ctx->video_sock.local_rtcp_port = VIDEO_RTCP_PORT_DEFAULT;
    ctx->debug = 0xffff;
    RTSP_SRV_LOCK(ctx);
#ifdef ZPL_WORKQUEUE
    ctx->t_master = eloop_master_module_create(pro);
    if (ctx->t_master)
        ctx->t_accept = eloop_add_read(ctx->t_master, rtsp_srv_accept_eloop, ctx, ctx->listen_sock);
#endif
    rtsp_session_init();
    RTSP_SRV_UNLOCK(ctx);
    return ctx;
}

static int rtsp_srv_accept(rtsp_srv_t *ctx)
{
    char address[128];
    zpl_socket_t sock = 0;
    struct ipstack_sockaddr_in client;
    if (!ipstack_invalid(ctx->listen_sock))
    {
        memset(address, 0, sizeof(address));
        sock = ipstack_sock_accept(ctx->listen_sock, &client);
        if (ipstack_invalid(sock))
        {
            if(RTSP_DEBUG_FLAG(ctx->debug, ERROR))
                rtsp_log_error("Can not accept rtsp client error:%s", ipstack_strerror(ipstack_errno));
            return -1;
        }
        ipstack_inet_ntop(IPSTACK_AF_INET, &client.sin_addr, address, sizeof(address));
        if(RTSP_DEBUG_FLAG(ctx->debug, EVENT))
        {
            rtsp_log_debug("RTSP Media Server accept remote for %s:%d", address, ntohs(client.sin_port));
        }         
        ipstack_set_nonblocking(sock);
        sockopt_reuseaddr(sock);
        ipstack_tcp_nodelay(sock, 1);
        //sockopt_keepalive(sock);

        return rtsp_srv_session_create(ctx, sock, address, ntohs(client.sin_port));
    }
    return -1;
}

static int rtsp_srv_accept_eloop(struct eloop *ctx)
{
    rtsp_srv_t *pctx = ELOOP_ARG(ctx);
    pctx->t_accept = NULL;
    rtsp_srv_accept(pctx);
    pctx->t_accept = eloop_add_read(pctx->t_master, rtsp_srv_accept_eloop, pctx, pctx->listen_sock);
    return 0;
}

static int rtsp_srv_session_rtsp_sdp_parse(rtsp_srv_t *ctx, int *find)
{
    char *p = ctx->_recv_buf;
    int len = 0, maxlen = ctx->_recv_length;
    while(p && len < maxlen)
    {
        if(p[len] == '\r' && p[len+1] == '\n' && p[len+2] == '\r' && p[len+3] == '\n')
        {
            if(find)
                *find = 1; 
            break;
        }
        len++;
    }
    return len;
}   

static int rtsp_srv_session_read_handler(rtsp_srv_t *ctx, rtsp_session_t *session)
{
    int ret = 0;
    int findsdp = 0, sdplen;
    uint8_t tmpbuf[RTSP_PACKET_MAX];
    if (!ipstack_invalid(session->sock))
    {
        RTSP_SRV_LOCK(ctx);
        if(ctx->_recv_length == 0)
            memset(ctx->_recv_buf, 0, sizeof(ctx->_recv_buf));
        ret = rtsp_session_recvfrom(session, ctx->_recv_buf + ctx->_recv_length, sizeof(ctx->_recv_buf)-4-ctx->_recv_length);
        if (ret <= 0)
        {
            if(RTSP_DEBUG_FLAG(ctx->debug, ERROR))
                rtsp_log_error("rtsp server read msg error for %s:%d [%d], and close it, error:%s", session->address, session->port, 
                    ipstack_fd(session->sock), ipstack_strerror(errno));
            rtsp_session_close(session);
        }
        else
        {
            ctx->_recv_length += ret;
            if (ctx->_recv_buf[0] == '$')
            {
                int need_len = 0;
                rtp_tcp_header_t *hdr = (rtp_tcp_header_t *)ctx->_recv_buf;
                ctx->_recv_offset = 4;
                need_len = sizeof(rtp_tcp_header_t) + ntohs(hdr->length);
                if(ctx->_recv_length < 4)
                {
                    RTSP_SRV_UNLOCK(ctx);
                    return 0;
                }
                if(need_len > ctx->_recv_length)
                {
                    ret = rtsp_session_recvfrom(session, ctx->_recv_buf + ctx->_recv_length, need_len - ctx->_recv_length);
                    if(ret < 0)
                    {
                        rtsp_session_close(session);
                        RTSP_SRV_UNLOCK(ctx);
                        return 0;
                    }
                    ctx->_recv_length += ret;
                    if(need_len == ctx->_recv_length)
                        rtsp_session_media_tcp_forward(session, ctx->_recv_buf, ctx->_recv_length);
                }  
                else
                {
                    ret = ctx->_recv_length - need_len;
                    if(ret)
                        memcpy(tmpbuf, ctx->_recv_buf + need_len, ret);
                    ctx->_recv_length = need_len;
                    rtsp_session_media_tcp_forward(session, ctx->_recv_buf, ctx->_recv_length);
                    if(ret)
                    {
                        memcpy(ctx->_recv_buf, tmpbuf, ret);
                        ctx->_recv_length = ret;
                    }
                }  
            }
            else
            {
                ret = rtsp_srv_session_rtsp_sdp_parse(ctx, &findsdp);
                if(findsdp && ret)
                {
                    sdplen = (ret + 4);
                    if(ctx->_recv_length - sdplen)
                        memcpy(tmpbuf, ctx->_recv_buf + sdplen, ctx->_recv_length - sdplen);
                    ctx->_recv_offset = 0;
                    memset(ctx->_recv_buf + sdplen, 0, sizeof(ctx->_recv_buf) - sdplen);
                    ret = ctx->_recv_length - sdplen;
                    ctx->_recv_length = sdplen;
                    rtsp_srv_session_event_handle(ctx, session);
                    ctx->_recv_length = ret;
                    if(ret)
                        memcpy(ctx->_recv_buf, tmpbuf, ret);
                }
            }
        }
        RTSP_SRV_UNLOCK(ctx);
    }
    return 0;
}

static int rtsp_srv_session_read_eloop(struct eloop *ctx)
{
    rtsp_session_t *session = ELOOP_ARG(ctx);
    rtsp_srv_t *srv = (rtsp_srv_t *)session->parent;
    if (session && srv)
    {
        RTSP_SESSION_LOCK(session);
        session->t_read = NULL;
        rtsp_srv_session_read_handler(srv, session);
        if (session->state != RTSP_SESSION_STATE_CLOSE)
            session->t_read = eloop_add_read(session->t_master, rtsp_srv_session_read_eloop, session, session->sock);
        RTSP_SESSION_UNLOCK(session);
        RTSP_SRV_LOCK(srv);
        if (session->state == RTSP_SESSION_STATE_CLOSE)
            rtsp_session_del(session->sock);
        rtsp_session_cleancache();
        RTSP_SRV_UNLOCK(srv);
    }
    return 0;
}


int rtsp_srv_session_create(rtsp_srv_t *ctx, zpl_socket_t sock, const char *ip_address, uint16_t port)
{
#ifdef ZPL_WORKQUEUE
    RTSP_SRV_LOCK(ctx);
    rtsp_session_t *session = rtsp_session_add(sock, ip_address, port, ctx);
    if (session)
    {
        session->t_master = ctx->t_master;
        session->t_read = eloop_add_read(ctx->t_master, rtsp_srv_session_read_eloop, session, sock);
        RTSP_SRV_UNLOCK(ctx);
        return 0;
    }
    RTSP_SRV_UNLOCK(ctx);
    return -1;
#else
    if (rtsp_session_add(sock, ip_address, port, ctx))
        return 0;
    return -1;
#endif
}

int rtsp_srv_session_destroy(rtsp_srv_t *ctx, zpl_socket_t sock)
{
    int ret = 0;
    RTSP_SRV_LOCK(ctx);
    ret = rtsp_session_del(sock);
    RTSP_SRV_UNLOCK(ctx);
    return ret;
}

rtsp_session_t *rtsp_srv_session_lookup(rtsp_srv_t *ctx, zpl_socket_t sock)
{
    rtsp_session_t *ret = NULL;
    RTSP_SRV_LOCK(ctx);
    ret = rtsp_session_lookup(sock);
    RTSP_SRV_UNLOCK(ctx);
    return ret;
}

static int rtsp_srv_session_url_split(rtsp_srv_t *ctx, rtsp_session_t *session)
{
    if (session->rtsp_url == NULL && session->sdptext.misc.url)
    {
        char miscurl[1024];
        os_url_t urlpath;
        memset(miscurl, 0, sizeof(miscurl));
        memset(&urlpath, 0, sizeof(os_url_t));
        strcpy(miscurl, session->sdptext.misc.url);
        rtsp_url_stream_path(miscurl, &urlpath);

        if (strlen(urlpath.url))
            session->rtsp_url = strdup(urlpath.url);

        RTSP_DEBUG_TRACE("=======================%s====================\r\n", miscurl);
        RTSP_DEBUG_TRACE("===============username   :%s\r\n", urlpath.user ? urlpath.user : "null");
        RTSP_DEBUG_TRACE("===============password   :%s\r\n", urlpath.pass ? urlpath.pass : "null");
        RTSP_DEBUG_TRACE("===============hostname   :%s\r\n", urlpath.host ? urlpath.host : "null");
        RTSP_DEBUG_TRACE("===============port       :%d\r\n", urlpath.port);
        RTSP_DEBUG_TRACE("===============channel    :%d\r\n", urlpath.channel);
        RTSP_DEBUG_TRACE("===============level      :%d\r\n", urlpath.level);
        RTSP_DEBUG_TRACE("===============path       :%s\r\n", urlpath.filename ? urlpath.filename : "null");
        RTSP_DEBUG_TRACE("===============url        :%s\r\n", urlpath.url);
        RTSP_DEBUG_TRACE("===============mode       :%d\r\n", urlpath.mode);
        RTSP_DEBUG_TRACE("===========================================\r\n");

        // 解析URL，确定访问的码流还是文件
        if (urlpath.filename && strlen(urlpath.filename))
        {
            if (urlpath.path)
            {
                char fullpath[256];
                memset(fullpath, 0, sizeof(fullpath));
                sprintf(fullpath, "%s/%s", urlpath.path, urlpath.filename);
                session->mfilepath = strdup(fullpath);
            }
            else
                session->mfilepath = strdup(urlpath.filename);
        }

        session->mchannel = urlpath.channel;
        session->mlevel = urlpath.level;

        os_url_free(&urlpath);

        if ((session->mchannel != -1 || (session->mfilepath)))
            return 0;
        return -1;
    }
    if ((session->mchannel != -1 || (session->mfilepath)))
        return 0;
    return -1;
}

static int rtsp_srv_session_handle_option(rtsp_srv_t *ctx, rtsp_session_t *session)
{
    int length = 0;

    if (rtsp_srv_session_url_split(ctx, session) == 0)
    {
        if (rtsp_session_media_lookup(session, session->mchannel, session->mlevel, session->mfilepath))
        {
            length = sdp_build_respone_header(ctx->_send_build, ctx->srvname, NULL, RTSP_STATE_CODE_200, session->cseq, session->session);

            length += sprintf(ctx->_send_build + length, "Public: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE\r\n");
        }
    }
    if (length == 0)
    {
        length = sdp_build_respone_header(ctx->_send_build, ctx->srvname, NULL, RTSP_STATE_CODE_404, session->cseq, session->session);
    }
    if (length)
    {
        length += sprintf(ctx->_send_build + length, "\r\n");
        return rtsp_session_sendto(session, ctx->_send_build, length);
    }
    return 0;
}

static int rtsp_srv_session_handle_describe(rtsp_srv_t *ctx, rtsp_session_t *session)
{
    int length = 0;
    uint8_t buftmp[1024];
    int code = RTSP_STATE_CODE_200;
    if (rtsp_srv_session_url_split(ctx, session) == 0)
    {
        if (!rtsp_session_media_lookup(session, session->mchannel, session->mlevel, session->mfilepath))
        {
            length = sdp_build_respone_header(ctx->_send_build, ctx->srvname, NULL, RTSP_STATE_CODE_404, session->cseq, session->session);
            ;
            if (length)
            {
                length += sprintf(ctx->_send_build + length, "\r\n");
                return rtsp_session_sendto(session, ctx->_send_build, length);
            }
            return 0;
        }
    }
    if ((session->video_session.b_enable && session->video_session.payload == RTP_MEDIA_PAYLOAD_NONE) ||
        (session->audio_session.b_enable && session->audio_session.payload == RTP_MEDIA_PAYLOAD_NONE))
    {
        length = sdp_build_respone_header(ctx->_send_build, ctx->srvname, NULL, RTSP_STATE_CODE_415, session->cseq, session->session);

        if (length)
        {
            length += sprintf(ctx->_send_build + length, "\r\n");
            return rtsp_session_sendto(session, ctx->_send_build, length);
        }
        return 0;
    }
    code = RTSP_STATE_CODE_200;
    memset(buftmp, 0, sizeof(buftmp));

    length = sdp_build_respone_header(ctx->_send_build, ctx->srvname, session->rtsp_url,
                                      code, session->cseq, session->session);

    code = rtsp_session_media_handle_describe(session, NULL);

    if (code == RTSP_STATE_CODE_200)
    {
        int sdplength = 0;
        memset(buftmp, 0, sizeof(buftmp));
        sdplength += sprintf(buftmp + sdplength, "v=0\r\n");

        sdplength += sprintf(buftmp + sdplength, "o=%s %u %u IN IPV4 0.0.0.0\r\n", ctx->srvname, time(NULL), rand());
        sdplength += sprintf(buftmp + sdplength, "c=IN IPV4 %s\r\n", ctx->listen_address ? ctx->listen_address : "0.0.0.0");
        //sdplength += sprintf(buftmp + sdplength, "a=control:%s\r\n", session->rtsp_url);
        sdplength += sprintf(buftmp + sdplength, "a=control:*\r\n");
        sdplength += sprintf(buftmp + sdplength, "t=0 0\r\n");
        /*
        if(session->video_session.b_enable)
        {
            sdplength += rtsp_server_build_sdp_video(ctx, session, buftmp + sdplength);
            if(session->video_session.i_trackid >= 0)
                sdplength += sprintf(buftmp + sdplength, "a=control:trackID=%d\r\n", session->video_session.i_trackid);
        }
        if(session->audio_session.b_enable)
        {
            sdplength += rtsp_server_build_sdp_audio(ctx, session, buftmp + sdplength);
            if(session->audio_session.i_trackid >= 0)
                sdplength += sprintf(buftmp + sdplength, "a=control:trackID=%d\r\n", session->audio_session.i_trackid);
        }
        */
        sdplength += rtsp_session_media_build_sdptext(session, buftmp + sdplength);

        length += sprintf(ctx->_send_build + length, "Content-Type: application/sdp\r\n");
        length += sprintf(ctx->_send_build + length, "Content-Length: %d\r\n", sdplength);
        length += sprintf(ctx->_send_build + length, "\r\n");
        length += sprintf(ctx->_send_build + length, "%s", buftmp);
    }
    if (length)
    {
        length += sprintf(ctx->_send_build + length, "\r\n");
        return rtsp_session_sendto(session, ctx->_send_build, length);
    }
    return 0;
}

static int rtsp_srv_session_handle_setup(rtsp_srv_t *ctx, rtsp_session_t *session)
{
    int length = 0;
    int code = RTSP_STATE_CODE_200;
    int isvideo = 0;
    rtp_session_t *_rtpsession = NULL;
    if (!rtsp_session_media_lookup(session, session->mchannel, session->mlevel, session->mfilepath))
    {
        length = sdp_build_respone_header(ctx->_send_build, ctx->srvname, NULL, RTSP_STATE_CODE_404, session->cseq, session->session);
        if (length)
        {
            length += sprintf(ctx->_send_build + length, "\r\n");
            return rtsp_session_sendto(session, ctx->_send_build, length);
        }
        return 0;
    }

    if (code == RTSP_STATE_CODE_200)
    {
        if (session->sdptext.misc.url)
        {
            int trackID = -1;
            if (strstr(session->sdptext.misc.url, "trackID="))
            {
                trackID = sdp_get_intcode(session->sdptext.misc.url, "trackID=");
            }
            if (trackID == session->video_session.i_trackid && session->video_session.b_enable)
            {
                _rtpsession = &session->video_session;
                isvideo = 1;
            }
            else if (trackID == session->audio_session.i_trackid && session->audio_session.b_enable)
            {
                _rtpsession = &session->audio_session;
            }
            else if (session->video_session.b_enable && !session->audio_session.b_enable)
            {
                _rtpsession = &session->video_session;
                isvideo = 1;
            }
            else if (!session->video_session.b_enable && session->audio_session.b_enable)
            {
                 _rtpsession = &session->audio_session;
            }

            // memset(&_rtpsession->transport, 0, sizeof(rtsp_transport_t));

            rtsp_header_transport(zpl_true, session->sdptext.header.Transport, &_rtpsession->transport);

            if (_rtpsession->transport.proto != RTSP_TRANSPORT_RTP_MULTCAST)
            {
                if (!_rtpsession->transport.rtp.unicast.local_rtp_port)
                    _rtpsession->transport.rtp.unicast.local_rtp_port = _rtpsession->local_rtp_port;
                if (!_rtpsession->transport.rtp.unicast.local_rtcp_port)
                    _rtpsession->transport.rtp.unicast.local_rtcp_port = _rtpsession->local_rtcp_port;
            }

            session->sdptext.origin.id = session->session;

            length = sdp_build_respone_header(ctx->_send_build, ctx->srvname, NULL, code, session->cseq, session->session);

            if (_rtpsession->transport.proto == RTSP_TRANSPORT_RTP_UDP)
            {
                length += sprintf(ctx->_send_build + length, "Transport: %s;server_port=%d-%d\r\n",
                                  session->sdptext.header.Transport,
                                  _rtpsession->transport.rtp.unicast.local_rtp_port,
                                  _rtpsession->transport.rtp.unicast.local_rtcp_port);
            }
            else if (_rtpsession->transport.proto == RTSP_TRANSPORT_RTP_TCP)
            {
                char tmp[64];
                memset(tmp, 0, sizeof(tmp));

                if (trackID >= 0 && trackID == session->video_session.i_trackid)
                {
                    _rtpsession->transport.rtp_interleaved = 0;
                    _rtpsession->transport.rtcp_interleaved = 1;
                }
                else if (trackID >= 0 && trackID == session->audio_session.i_trackid)
                {
                    _rtpsession->transport.rtp_interleaved = 2;
                    _rtpsession->transport.rtcp_interleaved = 3;
                }
                if(trackID >= 0)
                sprintf(tmp, ";interleaved=%d-%d", _rtpsession->transport.rtp_interleaved,
                        _rtpsession->transport.rtcp_interleaved);

                if (strlen(tmp))
                    length += sprintf(ctx->_send_build + length, "Transport: %s%s;\r\n",
                                      session->sdptext.header.Transport, tmp);
                else
                    length += sprintf(ctx->_send_build + length, "Transport: %s;\r\n",
                                      session->sdptext.header.Transport);
            }
            else if (_rtpsession->transport.proto == RTSP_TRANSPORT_RTP_TLS)
            {
                length += sprintf(ctx->_send_build + length, "Transport: %s;destination=239.0.0.0\r\n",
                                  session->sdptext.header.Transport);
            }
            else if (_rtpsession->transport.proto == RTSP_TRANSPORT_RTP_RTPOVERRTSP)
            {
                length += sprintf(ctx->_send_build + length, "Transport: %s;destination=239.0.0.0\r\n",
                                  session->sdptext.header.Transport);
            }
            else if (_rtpsession->transport.proto == RTSP_TRANSPORT_RTP_MULTCAST)
            {
                length += sprintf(ctx->_send_build + length, "Transport: %s;destination=239.0.0.0\r\n",
                                  session->sdptext.header.Transport);
            }
        }
        else
            length = sdp_build_respone_header(ctx->_send_build, ctx->srvname, NULL, RTSP_STATE_CODE_404, session->cseq, session->session);

        code = rtsp_session_media_handle_setup(session, isvideo, NULL);
        if (code != RTSP_STATE_CODE_200)
        {
            length = sdp_build_respone_header(ctx->_send_build, ctx->srvname, NULL, code, session->cseq, session->session);
            if (length)
            {
                length += sprintf(ctx->_send_build + length, "\r\n");
                return rtsp_session_sendto(session, ctx->_send_build, length);
            }
            return 0;
        }
    }
    else
        length = sdp_build_respone_header(ctx->_send_build, ctx->srvname, NULL, code, session->cseq, session->session);
    if (length)
    {
        length += sprintf(ctx->_send_build + length, "\r\n");
        return rtsp_session_sendto(session, ctx->_send_build, length);
    }
    return 0;
}

static int rtsp_srv_session_handle_teardown(rtsp_srv_t *ctx, rtsp_session_t *session)
{
    int length = 0;
    int code = RTSP_STATE_CODE_200;
    if (rtsp_session_media_lookup(session, session->mchannel, session->mlevel, session->mfilepath))
    {
        code = RTSP_STATE_CODE_200;
    }
    else
       code = RTSP_STATE_CODE_404;

    code = rtsp_session_media_handle_teardown(session, NULL);

    if (code != RTSP_STATE_CODE_200)
    {
        length = sdp_build_respone_header(ctx->_send_build, ctx->srvname, NULL, code, session->cseq, session->session);
        length += sprintf(ctx->_send_build + length, "Connection: close\r\n");
    }
    else
    {
        length = sdp_build_respone_header(ctx->_send_build, ctx->srvname, NULL, code, session->cseq, session->session);
    }
    if (length)
    {
        int ret = 0;
        length += sprintf(ctx->_send_build + length, "\r\n");
        rtsp_session_sendto(session, ctx->_send_build, length);
        session->state = RTSP_SESSION_STATE_CLOSE;
        return ret;
    }
    
    session->state = RTSP_SESSION_STATE_CLOSE;
    return 0;
}

static int rtsp_srv_session_handle_play(rtsp_srv_t *ctx, rtsp_session_t *session)
{
    int length = 0;
    int code = RTSP_STATE_CODE_200;
    if (rtsp_session_media_lookup(session, session->mchannel, session->mlevel, session->mfilepath))
    {
        code = RTSP_STATE_CODE_200;
    }
    else
       code = RTSP_STATE_CODE_404;

    code = rtsp_session_media_handle_play(session, NULL);

    if (code != RTSP_STATE_CODE_200)
    {
        length = sdp_build_respone_header(ctx->_send_build, ctx->srvname, NULL, code, session->cseq, session->session);

        if (session->video_session.i_trackid >= 0 && session->audio_session.i_trackid >= 0)
        {
            length += sprintf(ctx->_send_build + length, "RTP-Info: url=trackID=%d;seq=0;rtptime=0;url=trackID=%d;seq=0;rtptime=0\r\n",
                              session->video_session.i_trackid, session->audio_session.i_trackid);
        }
        else if (session->video_session.i_trackid >= 0 && session->audio_session.i_trackid == -1)
        {
            length += sprintf(ctx->_send_build + length, "RTP-Info: url=trackID=%d;seq=0;rtptime=0\r\n",
                              session->video_session.i_trackid);
        }
        else if (session->audio_session.i_trackid >= 0 && session->video_session.i_trackid == -1)
        {
            length += sprintf(ctx->_send_build + length, "RTP-Info: url=trackID=%d;seq=0;rtptime=0\r\n",
                              session->audio_session.i_trackid);
        }
    }
    else
    {
        length = sdp_build_respone_header(ctx->_send_build, ctx->srvname, NULL, code, session->cseq, session->session);
    }
    if (length)
    {
        length += sprintf(ctx->_send_build + length, "\r\n");
        return rtsp_session_sendto(session, ctx->_send_build, length);
    }
    return 0;
}

static int rtsp_srv_session_handle_pause(rtsp_srv_t *ctx, rtsp_session_t *session)
{
    int length = 0;
    int code = RTSP_STATE_CODE_200;
    if (rtsp_session_media_lookup(session, session->mchannel, session->mlevel, session->mfilepath))
    {
        code = RTSP_STATE_CODE_200;
    }
    else
       code = RTSP_STATE_CODE_404;

    code = rtsp_session_media_handle_pause(session, NULL);

    if (code != RTSP_STATE_CODE_200)
        length = sdp_build_respone_header(ctx->_send_build, ctx->srvname, NULL, code, session->cseq, session->session);
    else
    {
        length = sdp_build_respone_header(ctx->_send_build, ctx->srvname, NULL, code, session->cseq, session->session);
    }
    if (length)
    {
        length += sprintf(ctx->_send_build + length, "\r\n");
        return rtsp_session_sendto(session, ctx->_send_build, length);
    }
    return 0;
}

static int rtsp_srv_session_handle_scale(rtsp_srv_t *ctx, rtsp_session_t *session)
{
    int length = 0;
    if (length)
    {
        length += sprintf(ctx->_send_build + length, "\r\n");
        return rtsp_session_sendto(session, ctx->_send_build, length);
    }
    return 0;
}

static int rtsp_srv_session_handle_set_parameter(rtsp_srv_t *ctx, rtsp_session_t *session)
{
    int length = 0;
    if (length)
    {
        length += sprintf(ctx->_send_build + length, "\r\n");
        return rtsp_session_sendto(session, ctx->_send_build, length);
    }
    return 0;
}

static int rtsp_srv_session_handle_get_parameter(rtsp_srv_t *ctx, rtsp_session_t *session)
{
    int length = 0;
    if (length)
    {
        length += sprintf(ctx->_send_build + length, "\r\n");
        return rtsp_session_sendto(session, ctx->_send_build, length);
    }
    return 0;
}

static int rtsp_srv_session_socket_load(rtsp_srv_t *ctx, rtsp_session_t *session)
{
    if (session->video_session.rtcp_sock <= 0 && ctx->video_sock._rtcp_sock)
    {
        session->video_session.rtcp_sock = ctx->video_sock._rtcp_sock;
    }
    if (session->video_session.transport.rtp.unicast.local_rtcp_port == 0)
        session->video_session.transport.rtp.unicast.local_rtcp_port = ctx->video_sock.local_rtcp_port;

    if (session->video_session.rtp_sock <= 0 && ctx->video_sock._rtp_sock)
    {
        session->video_session.rtp_sock = ctx->video_sock._rtp_sock;
    }
    if (session->video_session.transport.rtp.unicast.local_rtp_port == 0)
        session->video_session.transport.rtp.unicast.local_rtp_port = ctx->video_sock.local_rtp_port;

    if (session->audio_session.rtcp_sock <= 0 && ctx->audio_sock._rtcp_sock)
    {
        session->audio_session.rtcp_sock = ctx->audio_sock._rtcp_sock;
    }
    if (session->audio_session.transport.rtp.unicast.local_rtcp_port == 0)
        session->audio_session.transport.rtp.unicast.local_rtcp_port = ctx->audio_sock.local_rtcp_port;

    if (session->audio_session.rtp_sock <= 0 && ctx->audio_sock._rtp_sock)
    {
        session->audio_session.rtp_sock = ctx->audio_sock._rtp_sock;
    }
    if (session->audio_session.transport.rtp.unicast.local_rtp_port == 0)
        session->audio_session.transport.rtp.unicast.local_rtp_port = ctx->audio_sock.local_rtp_port;
    return 0;
}

static int rtsp_srv_session_event_handle(rtsp_srv_t *ctx, rtsp_session_t *session)
{
    int ret = 0;
    char *recv_optstr[] = {"NONE", "OPTIONS", "DESCRIBE", "SETUP", "TEARDOWN", "PLAY", "PAUSE", "SCALE", "GET PARAMETER", "SET PARAMETER", "MAX METHOD"};

    RTSP_DEBUG_TRACE("===========================================\r\n");
    RTSP_DEBUG_TRACE("Client from %s:%d socket=%d session=%u\r\n", session->address ? session->address : " ",
               session->port, ipstack_fd(session->sock), session->session);
    RTSP_DEBUG_TRACE("===========================================\r\n");

    sdp_text_init(&session->sdptext, (const char *)(ctx->_recv_buf + ctx->_recv_offset),
                  ctx->_recv_length - ctx->_recv_offset);

    sdp_text_prase(true, &session->sdptext);

    if (session->sdptext.header.CSeq > -1)
    {
        session->cseq = (session->sdptext.header.CSeq);
    }
    memset(ctx->_send_buf, 0, sizeof(ctx->_send_buf));

    rtsp_srv_session_socket_load(ctx, session);

    ctx->_send_build = ctx->_send_buf + ctx->_send_offset;


    if(RTSP_DEBUG_FLAG(ctx->debug, EVENT) && session->sdptext.method >= 0 && session->sdptext.method <= RTSP_METHOD_MAX)
        rtsp_log_debug( "RTSP Server Secv '%s' Request", recv_optstr[session->sdptext.method]);

    if(RTSP_DEBUG_FLAG(ctx->debug, DEBUG) && RTSP_DEBUG_FLAG(ctx->debug, DETAIL))
        rtsp_log_debug("\r\nC -> S:\r\n%s\r\n", ctx->_recv_buf + ctx->_recv_offset);

    switch (session->sdptext.method)
    {
    case RTSP_METHOD_OPTIONS:

        ret = rtsp_srv_session_handle_option(ctx, session);
        break;
    case RTSP_METHOD_DESCRIBE:

        ret = rtsp_srv_session_handle_describe(ctx, session);
        break;
    case RTSP_METHOD_SETUP:
        ret = rtsp_srv_session_handle_setup(ctx, session);
        break;
    case RTSP_METHOD_TEARDOWN:
        ret = rtsp_srv_session_handle_teardown(ctx, session);
        break;
    case RTSP_METHOD_PLAY:
        ret = rtsp_srv_session_handle_play(ctx, session);
        break;
    case RTSP_METHOD_PAUSE:
        ret = rtsp_srv_session_handle_pause(ctx, session);
        break;
    case RTSP_METHOD_SCALE:
        ret = rtsp_srv_session_handle_scale(ctx, session);
        break;
    case RTSP_METHOD_GET_PARAMETER:
        ret = rtsp_srv_session_handle_get_parameter(ctx, session);
        break;
    case RTSP_METHOD_SET_PARAMETER:
        ret = rtsp_srv_session_handle_set_parameter(ctx, session);
        break;
    default:
    {
        static int adsdadsada = 0;
        adsdadsada++;
        RTSP_DEBUG_TRACE( "===============:%s -> default\r\n", __func__);
        int length = sdp_build_respone_header(ctx->_send_build, ctx->srvname, NULL,
                                              RTSP_STATE_CODE_405, session->cseq, session->session);
        if (length)
        {
            length += sprintf(ctx->_send_build + length, "\r\n");
            return rtsp_session_sendto(session, ctx->_send_build, length);
        }
        if (adsdadsada == 10)
        {
            rtsp_session_close(session);
        }
    }
    break;
    }
    if (ret)
        rtsp_session_callback(session, session->sdptext.method);
    if (ret)
    {
        if(RTSP_DEBUG_FLAG(ctx->debug, DEBUG) && RTSP_DEBUG_FLAG(ctx->debug, DETAIL))
            rtsp_log_debug("\r\nS -> C(ret=%d):\r\n%s\r\n", ret, ctx->_send_build);
    }

    sdp_text_free(&session->sdptext);
    return 0;
}

/*
OPTIONS rtsp://192.168.1.211:554/h264/ch1/main/av_stream RTSP/1.0
CSeq: 2
User-Agent: LibVLC/2.2.4 (LIVE555 Streaming Media v2016.02.22)
# OPTIONS 和前面的一样

RTSP/1.0 200 OK
CSeq: 2
Public: DESCRIBE, PLAY, PAUSE, SETUP, OPTIONS, TEARDOWN, SET_PARAMETER
# 和前面的一样，服务器回复你的能力： DESCRIBE, PLAY, PAUSE, SETUP, OPTIONS, TEARDOWN, SET_PARAMETER
# 和之前比，少了个GET_PARAMETER

DESCRIBE rtsp://192.168.1.211:554/h264/ch1/main/av_stream RTSP/1.0
CSeq: 3
User-Agent: LibVLC/2.2.4 (LIVE555 Streaming Media v2016.02.22)
Accept: application/sdp
# DESCRIBE 和前面的一样

RTSP/1.0 401 Unauthorized
CSeq: 3
WWW-Authenticate: Basic realm="/"
# 和前面的一样；
# 服务器回答，你没有认证（用户密码），所以给你401吧
# 区别在于比我的少了WWW-Authenticate: Digest realm="1868cb21d4df", nonce="cfbaf30c677edba80dbd7f0eb1df5db6", stale="FALSE"

DESCRIBE rtsp://192.168.1.211:554/h264/ch1/main/av_stream RTSP/1.0
CSeq: 4
Authorization: Basic YWRtaW46MTIzNDU=
User-Agent: LibVLC/2.2.4 (LIVE555 Streaming Media v2016.02.22)
Accept: application/sdp
# 和前面一样： 我表示不服，再上诉：让服务器描述一下流的情况，你用sdp的格式告诉我吧
# 区别在于比我的少了WWW-Authenticate: Digest realm="1868cb21d4df", nonce="cfbaf30c677edba80dbd7f0eb1df5db6", stale="FALSE"

RTSP/1.0 200 OK
CSeq: 4
Content-Type: application/sdp
Content-Length: 499

v=0
o=- 1109162014219182 1109162014219192 IN IP4 x.y.z.w
s=Media Presentation
e=NONE
c=IN IP4 0.0.0.0
t=0 0
a=control:*
m=video 0 RTP/AVP 96
a=rtpmap:96 H264/90000
a=control:trackID=1
a=fmtp:96 profile-level-id=4D0014;packetization-mode=0;sprop-parameter-sets=Z0KAKIiLQCgC3QgAABdwAAr8gCA=,aM44gA==
m=audio 0 RTP/AVP 0
a=rtpmap:0 PCMU/8000
a=control:trackID=2
a=Media_header:MEDIAINFO=494D4B48010100000400000110710110401F000000FA000000000000000000000000000000000000;
a=appversion:1.0
# 服务器答应了，然后把SDP发给我了
# 里面有各种信息：我看到只有一路视频，1080P的，H264编码；
# 96是视频流ID, 这符合规范，mediainfo可能是sps pps的加密信息
# trackID=1 是视频； trackID=2 是音频，音频格式是PCMU 8K的，监控常用

# 问题来了： m=audio 0 RTP/AVP 0  音频的ID按协议应该是97（m=audio 0 RTP/AVP 97）
# 根据sdp草案，写0可能也不算bug，具体再细看
# sdp: https://wenku.baidu.com/view/f5e3838102d276a200292ed5.html

SETUP rtsp://192.168.1.211:554/h264/ch1/main/av_stream/trackID=1 RTSP/1.0
CSeq: 5
Authorization: Basic YWRtaW46MTIzNDU=
User-Agent: LibVLC/2.2.4 (LIVE555 Streaming Media v2016.02.22)
Transport: RTP/AVP;unicast;client_port=63234-63235
# SETUP ：为了透过防火墙，我想指定传输机制RTP/AVP/TCP告诉服务器
# interleaved变成了client_port； 我那边是：Transport: RTP/AVP/TCP;unicast;interleaved=0-1，这个不清楚有无影响
# trackID=1 这路是音频

RTSP/1.0 200 OK
CSeq: 5
Session:       1095657681
Transport: RTP/AVP;unicast;client_port=63234-63235;server_port=8813-8814;ssrc=55667788
# 服务器准了这路音频，不过回复不一样，也没告诉你可以play了
# 我的是这样的：Transport: RTP/AVP/TCP;unicast;interleaved=0-1;ssrc=72a9bdb8;mode="play"

SETUP rtsp://192.168.1.211:554/h264/ch1/main/av_stream/trackID=2 RTSP/1.0
CSeq: 6
Authorization: Basic YWRtaW46MTIzNDU=
User-Agent: LibVLC/2.2.4 (LIVE555 Streaming Media v2016.02.22)
Transport: RTP/AVP;unicast;client_port=63236-63237
Session: 1095657681
# trackID=2 这路是视频

RTSP/1.0 200 OK
CSeq: 6
Session:       1095657681
Transport: RTP/AVP;unicast;client_port=63236-63237;server_port=8813-8814;ssrc=55667788
# 准了这路视频

PLAY rtsp://192.168.1.211:554/h264/ch1/main/av_stream RTSP/1.0
CSeq: 7
Authorization: Basic YWRtaW46MTIzNDU=
User-Agent: LibVLC/2.2.4 (LIVE555 Streaming Media v2016.02.22)
Session: 1095657681
Range: npt=0.000-
# 和之前一样，PLAY ：我想告诉服务器以SETUP指定的机制开始发送数据；
# 还可以用关键字Range指定play的范围

RTSP/1.0 200 OK
CSeq: 7
Session:       1095657681
Range: npt=now-
RTP-Info: url=trackID=1;seq=6575,url=trackID=2;seq=43949

RTSP/1.0 200 OK
CSeq: 7
Session:       1095657681
Range: npt=now-
RTP-Info: url=trackID=1;seq=6575,url=trackID=2;seq=43949
# 为啥发两次一模一样的？

TEARDOWN rtsp://192.168.1.211:554/h264/ch1/main/av_stream RTSP/1.0
CSeq: 8
Authorization: Basic YWRtaW46MTIzNDU=
User-Agent: LibVLC/2.2.4 (LIVE555 Streaming Media v2016.02.22)
Session: 1095657681
# 和之前一样TEARDOWN ：不玩了，就停止给定URL流发送数据，并释放相关资源

RTSP/1.0 200 OK
CSeq: 8
Session:       1095657681
Connection: close
# 服务器关闭了会话，告诉你一下，后会无期~~
*/
