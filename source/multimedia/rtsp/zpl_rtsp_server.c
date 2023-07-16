/*
 * Copyright © 2001-2013 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
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



static int rtsp_srv_accept_eloop(struct eloop *ctx);

void rtsp_srv_destroy(zpl_rtsp_srv_t *ctx)
{
    rtsp_session_t *p = NULL;
	NODE node;    
    if (ctx->t_accept)
        eloop_cancel(ctx->t_accept);
    if (ctx->t_read)
        eloop_cancel(ctx->t_read);
    if (ctx->t_master)
        eloop_master_free(ctx->t_master);

    for (p = (rtsp_session_t*)lstFirst( &ctx->_list_head); p != NULL; p = (rtsp_session_t*)lstNext(&node))
    {
        node = p->node;
        if (p)
        {
            rtsp_session_destroy(ctx, p);
        }
    }
    lstFree(&ctx->_list_head);
    if (ctx->mutex)
    {
        os_mutex_destroy(ctx->mutex);
        ctx->mutex = NULL;
    }
    if (!ipstack_invalid(ctx->listen_sock))
    {
        ipstack_close(ctx->listen_sock);
        ctx->listen_sock = ZPL_SOCKET_INVALID;
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
    free(ctx);
}

zpl_rtsp_srv_t *rtsp_srv_create(void *m, const char *ip, uint16_t port, int pro)
{
    zpl_rtsp_srv_t *ctx = NULL;
    zpl_socket_t listen_sock = ipstack_sock_create(IPSTACK_IPCOM, zpl_true);
    if (!ipstack_invalid(listen_sock))
    {
        int enable = 1;
        if (ipstack_sock_bind(listen_sock, ip, port) != OK)
        {
            ipstack_close(listen_sock);
            rtsp_log_error("Can not bind to %s:%d rtsp server sock, error:%s", ip ? ip : "0.0.0.0", port, ipstack_strerror(ipstack_errno));
            return NULL;
        }
        if (ipstack_sock_listen(listen_sock, 5) != OK)
        {
            ipstack_close(listen_sock);
            rtsp_log_error( "Can not bind to %s:%d rtsp server sock, error:%s", ip ? ip : "0.0.0.0", port, ipstack_strerror(ipstack_errno));
            return NULL;
        }
        ipstack_set_nonblocking(listen_sock);
        sockopt_reuseaddr(listen_sock);
        ipstack_setsockopt(listen_sock, IPSTACK_SOL_SOCKET, IPSTACK_SO_REUSEADDR,
                           (void *)&enable, sizeof(enable));

        ctx = (zpl_rtsp_srv_t *)malloc(sizeof(zpl_rtsp_srv_t));
        if (ctx == NULL)
        {
            ipstack_close(listen_sock);
            return NULL;
        }
        memset(ctx, 0, sizeof(zpl_rtsp_srv_t));
        ctx->listen_sock = listen_sock;
        if (ip)
            ctx->listen_address = strdup(ip);
        else
            ctx->listen_address = NULL;
        
        ctx->listen_port = port;
        ctx->mutex = os_mutex_name_create("rtspsession");
        lstInit(&ctx->_list_head);
        ctx->t_master = m;
        if (ctx->t_master)
            ctx->t_accept = eloop_add_read(ctx->t_master, rtsp_srv_accept_eloop, ctx, ctx->listen_sock);                      
    }
    return ctx;
}


static int rtsp_srv_accept_eloop(struct eloop *ctx)
{
    char address[128];
    zpl_socket_t sock = 0;
    struct ipstack_sockaddr_in client;

    zpl_rtsp_srv_t *pctx = ELOOP_ARG(ctx);
    pctx->t_accept = NULL;
    if (!ipstack_invalid(pctx->listen_sock))
    {
        memset(address, 0, sizeof(address));
        sock = ipstack_sock_accept(pctx->listen_sock, &client);
        if (ipstack_invalid(sock))
        {
            if(RTSP_DEBUG_FLAG(_rtsp_session_debug , ERROR))
                rtsp_log_error("Can not accept rtsp client error:%s", ipstack_strerror(ipstack_errno));
            return ERROR;
        }
        ipstack_inet_ntop(IPSTACK_AF_INET, &client.sin_addr, address, sizeof(address));
        if(RTSP_DEBUG_FLAG(_rtsp_session_debug , EVENT))
        {
            rtsp_log_debug("RTSP Media Server accept remote for %s:%d", address, ntohs(client.sin_port));
        }         
        ipstack_set_nonblocking(sock);
        sockopt_reuseaddr(sock);
        ipstack_tcp_nodelay(sock, 1);

        pctx->t_accept = eloop_add_read(pctx->t_master, rtsp_srv_accept_eloop, pctx, pctx->listen_sock);
        if(rtsp_session_create(pctx, sock, address, ntohs(client.sin_port), pctx->t_master, pctx->listen_address) == OK)
            return OK;
    }
    return ERROR;
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
