/*
 * Copyright © 2001-2013 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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
#include "zpl_rtsp_rtp.h"

int rtsp_session_recvfrom(rtsp_session_t *session, uint8_t *req, uint32_t req_length)
{
    if (!ipstack_invalid(session->sock))
        return ipstack_recv(session->sock, req, req_length, 0);
    return -1;
}

int rtsp_session_sendto(rtsp_session_t *session, uint8_t *req, uint32_t req_length)
{
    if (!ipstack_invalid(session->sock))
        return ipstack_send(session->sock, req, req_length, 0);
    return -1;
}

int rtsp_session_close(rtsp_session_t *session)
{
    if (!ipstack_invalid(session->sock))
    {
        ipstack_close(session->sock);
        session->sock = ZPL_SOCKET_INVALID;
    }
    return OK;
}

zpl_socket_t rtsp_session_listen(const char *lip, uint16_t port)
{
    zpl_socket_t sock = ipstack_sock_create(IPSTACK_OS, zpl_true);
    if (!ipstack_invalid(sock))
    {
        int enable = 1;
        if (ipstack_sock_bind(sock, lip, port) != OK)
        {
            ipstack_close(sock);
            rtsp_log_error("Can not bind to %s:%d rtsp server sock, error:%s", lip ? lip : "127.0.0.1", port, ipstack_strerror(ipstack_errno));
            return ZPL_SOCKET_INVALID;
        }
        if (ipstack_sock_listen(sock, 5) != OK)
        {
            ipstack_close(sock);
            rtsp_log_error( "Can not bind to %s:%d rtsp server sock, error:%s", lip ? lip : "127.0.0.1", port, ipstack_strerror(ipstack_errno));
            return ZPL_SOCKET_INVALID;
        }
        ipstack_set_nonblocking(sock);
        sockopt_reuseaddr(sock);
        ipstack_setsockopt(sock, IPSTACK_SOL_SOCKET, IPSTACK_SO_REUSEADDR,
                           (void *)&enable, sizeof(enable));
        return sock;
    }
    rtsp_log_error( "Can not create rtsp server sock, error:%s", ipstack_strerror(ipstack_errno));
    return ZPL_SOCKET_INVALID;
}

int rtsp_session_connect(rtsp_session_t *session, const char *ip, uint16_t port, int timeout_ms)
{
    int ret = 0;
    session->sock = ipstack_sock_create(IPSTACK_OS, zpl_true);
    if (!ipstack_invalid(session->sock))
    {
        ret = ipstack_sock_connect_nonblock(session->sock, ip, port);
        if (ret == OK)
        {
            int cnt = timeout_ms / 500;
            if (cnt <= 0)
                cnt = 1;
            while (cnt)
            {
                os_msleep(500);
                if (ipstack_sock_connect_check(session->sock) == OK)
                {
                    // ipstack_set_nonblocking(session->sock);
                    ret = sockopt_reuseaddr(session->sock);
                    // ipstack_tcp_nodelay(session->sock, 1);
                    sockopt_keepalive(session->sock);
                    return OK;
                }
                cnt--;
            }
        }
        ipstack_close(session->sock);
        session->sock = ZPL_SOCKET_INVALID;
    }
    rtsp_log_error("Can not create rtsp client sock to %s:%d, error:%s", ip ? ip : "127.0.0.1", port, ipstack_strerror(ipstack_errno));
    return ERROR;
}

int rtp_over_rtsp_session_sendto(rtsp_session_t *session, uint8_t chn, uint8_t *data, uint32_t length)
{
    uint16_t *dlen = (uint16_t *)(data + 2);
    data[0] = '$';
    data[1] = chn;
    *dlen = htons(length);
    return rtsp_session_sendto(session, data, length + 4);
    /*
    | magic number | channel number | embedded data length | data |
    magic number - 1 byte value of hex 0x24
    RTP数据标识符，"$"
    channel number - 1 byte value to denote the channel
    信道数字 - 1个字节，用来指示信道
    embedded data length - 2 bytes to denote the embedded data length
    数据长度 - 2个字节，用来指示插入数据长度
    data - data packet, ie RTP packet, with the total length of the embedded data length
    数据 - 数据包，比如说RTP包，总长度与上面的数据长度相同
    Below is a full example of the communication exchanged
    */
}

int rtsp_session_lstinit(struct osker_list_head *list)
{
    INIT_OSKER_LIST_HEAD(list);
    return 0;
}

int rtsp_session_lstexit(struct osker_list_head *list)
{
    struct osker_list_head *pos = NULL, *n = NULL;
    rtsp_session_t *p = NULL;
    osker_list_for_each_safe(pos, n, list)
    {
        p = (rtsp_session_t *)osker_list_entry(pos, rtsp_session_t, node);
        if (p)
        {
            osker_list_del(pos);
            rtsp_session_destroy(p);
            p = NULL;
        }
    }
    return 0;
}

int rtsp_session_destroy(rtsp_session_t *session)
{
    if (session)
    {
        RTSP_SESSION_LOCK(session);
#ifdef ZPL_WORKQUEUE
        if (session->t_read)
            eloop_cancel(session->t_read);
#endif
        if (session && !ipstack_invalid(session->sock))
        {
            ipstack_close(session->sock);
            session->sock = ZPL_SOCKET_INVALID;
        }
        if (rtsp_media_lookup(session, session->mchannel, session->mlevel, session->mfilepath))
        {
            rtsp_media_destroy(session);
        }
        rtsp_header_transport_destroy(&session->video_session.transport);
        rtsp_header_transport_destroy(&session->audio_session.transport);

        sdp_text_free(&session->sdptext);

        if (session->username)
        {
            free(session->username);
            session->username = NULL;
        }
        if (session->password)
        {
            free(session->password);
            session->password = NULL;
        }
        if (session->rtsp_url)
        {
            free(session->rtsp_url);
            session->rtsp_url = NULL;
        }
        if (session->mfilepath)
        {
            free(session->mfilepath);
            session->mfilepath = NULL;
        }
        RTSP_SESSION_UNLOCK(session);
        if (session->mutex)
        {
            os_mutex_exit(session->mutex);
            session->mutex = NULL;
        }

        free(session);
        // session = NULL;
    }
    return 0;
}

int rtsp_session_install(rtsp_session_t *newNode, rtsp_method method, rtsp_session_call func, void *p)
{
    // RTSP_SESSION_LOCK(newNode);
    switch (method)
    {
    case RTSP_METHOD_OPTIONS:
        newNode->rtsp_callback._options_func = func;
        newNode->rtsp_callback._options_user = p;
        break;
    case RTSP_METHOD_DESCRIBE:
        newNode->rtsp_callback._describe_func = func;
        newNode->rtsp_callback._describe_user = p;
        break;
    case RTSP_METHOD_SETUP:
        newNode->rtsp_callback._setup_func = func;
        newNode->rtsp_callback._setup_user = p;
        break;
    case RTSP_METHOD_TEARDOWN:
        newNode->rtsp_callback._teardown_func = func;
        newNode->rtsp_callback._teardown_user = p;
        break;
    case RTSP_METHOD_PLAY:
        newNode->rtsp_callback._play_func = func;
        newNode->rtsp_callback._play_user = p;
        break;
    case RTSP_METHOD_PAUSE:
        newNode->rtsp_callback._pause_func = func;
        newNode->rtsp_callback._pause_user = p;
        break;
    case RTSP_METHOD_SCALE:
        newNode->rtsp_callback._scale_func = func;
        newNode->rtsp_callback._scale_user = p;
        break;
    case RTSP_METHOD_GET_PARAMETER:
        newNode->rtsp_callback._get_parameter_func = func;
        newNode->rtsp_callback._get_parameter_user = p;
        break;
    case RTSP_METHOD_SET_PARAMETER:
        newNode->rtsp_callback._set_parameter_func = func;
        newNode->rtsp_callback._set_parameter_user = p;
        break;
    default:
        break;
    }
    // RTSP_SESSION_UNLOCK(newNode);
    return 0;
}

int rtsp_session_callback(rtsp_session_t *newNode, rtsp_method method)
{
    int ret = 0;
    // RTSP_SESSION_LOCK(newNode);
    switch (method)
    {
    case RTSP_METHOD_OPTIONS:
        if (newNode->rtsp_callback._options_func)
            ret = (newNode->rtsp_callback._options_func)(newNode, newNode->rtsp_callback._options_user);
        break;
    case RTSP_METHOD_DESCRIBE:
        if (newNode->rtsp_callback._describe_func)
            ret = (newNode->rtsp_callback._describe_func)(newNode, newNode->rtsp_callback._describe_user);
        break;
    case RTSP_METHOD_SETUP:
        if (newNode->rtsp_callback._setup_func)
            ret = (newNode->rtsp_callback._setup_func)(newNode, newNode->rtsp_callback._setup_user);
        break;
    case RTSP_METHOD_TEARDOWN:
        if (newNode->rtsp_callback._teardown_func)
            ret = (newNode->rtsp_callback._teardown_func)(newNode, newNode->rtsp_callback._teardown_user);
        break;
    case RTSP_METHOD_PLAY:
        if (newNode->rtsp_callback._play_func)
            ret = (newNode->rtsp_callback._play_func)(newNode, newNode->rtsp_callback._play_user);
        break;
    case RTSP_METHOD_PAUSE:
        if (newNode->rtsp_callback._pause_func)
            ret = (newNode->rtsp_callback._pause_func)(newNode, newNode->rtsp_callback._pause_user);
        break;
    case RTSP_METHOD_SCALE:
        if (newNode->rtsp_callback._scale_func)
            ret = (newNode->rtsp_callback._scale_func)(newNode, newNode->rtsp_callback._scale_user);
        break;
    case RTSP_METHOD_GET_PARAMETER:
        if (newNode->rtsp_callback._get_parameter_func)
            ret = (newNode->rtsp_callback._get_parameter_func)(newNode, newNode->rtsp_callback._get_parameter_user);
        break;
    case RTSP_METHOD_SET_PARAMETER:
        if (newNode->rtsp_callback._set_parameter_func)
            ret = (newNode->rtsp_callback._set_parameter_func)(newNode, newNode->rtsp_callback._set_parameter_user);
        break;
    default:
        break;
    }
    // RTSP_SESSION_UNLOCK(newNode);
    return ret;
}

int rtsp_session_pdata(rtsp_session_t *newNode, bool bvideo, void *pdata)
{
    // RTSP_SESSION_LOCK(newNode);
    if (bvideo && newNode)
    {
        newNode->video_session.pdata = pdata;
    }
    if (!bvideo && newNode)
    {
        newNode->audio_session.pdata = pdata;
    }
    // RTSP_SESSION_UNLOCK(newNode);
    return 0;
}

int rtsp_session_default(rtsp_session_t *newNode, bool srv)
{
    RTSP_SESSION_LOCK(newNode);
    newNode->bsrv = srv;
    newNode->session = (int)newNode;
    newNode->_rtpsession = NULL;
    newNode->state = RTSP_SESSION_STATE_CONNECT;
    newNode->rtsp_callback._options_func = NULL;       // rtsp_rtp_after_options;
    newNode->rtsp_callback._describe_func = NULL;      // rtsp_rtp_after_describe;
    newNode->rtsp_callback._setup_func = NULL;         // rtsp_rtp_after_setup;
    newNode->rtsp_callback._teardown_func = NULL;      // rtsp_rtp_after_teardown;
    newNode->rtsp_callback._play_func = NULL;          // rtsp_rtp_after_play;
    newNode->rtsp_callback._pause_func = NULL;         // rtsp_rtp_after_pause;
    newNode->rtsp_callback._scale_func = NULL;         // rtsp_rtp_after_scale;
    newNode->rtsp_callback._set_parameter_func = NULL; // rtsp_rtp_after_set_parameter;
    newNode->rtsp_callback._get_parameter_func = NULL; // rtsp_rtp_after_get_parameter;

    newNode->audio_session.local_rtp_port = AUDIO_RTP_PORT_DEFAULT;
    newNode->audio_session.local_rtcp_port = AUDIO_RTCP_PORT_DEFAULT;

    newNode->audio_session.b_enable = false; // 使能
    // newNode->audio_session.rtp_sock = -1;
    // newNode->audio_session.rtcp_sock = -1;
    newNode->audio_session.i_trackid = -1;                          // 视频通道
    newNode->audio_session.b_issetup = false;                       // 视频是否设置
    newNode->audio_session.transport.type = RTSP_TRANSPORT_UNICAST; // 对端期待的传输模式
    newNode->audio_session.pdata = NULL;
    newNode->audio_session.packetization_mode = 1;
    newNode->audio_session.payload = RTP_MEDIA_PAYLOAD_G711U;
    newNode->audio_session.frame_delay_msec = 0;
    newNode->audio_session.framerate = ZPL_AUDIO_FRAMERATE_DEFAULT;
    newNode->audio_session.rtpmode = 2;
    newNode->audio_session.user_timestamp = 0;
    newNode->audio_session.frame_delay_msec = 1000/newNode->audio_session.framerate;
    if (!srv)
    {
        newNode->session = 0;
        newNode->audio_session.transport.rtp.unicast.local_rtp_port = newNode->audio_session.local_rtp_port;
        newNode->audio_session.transport.rtp.unicast.local_rtcp_port = newNode->audio_session.local_rtcp_port;
        newNode->audio_session.transport.proto = RTSP_TRANSPORT_RTP_UDP; // RTSP_TRANSPORT_xxx
        newNode->audio_session.transport.type = RTSP_TRANSPORT_UNICAST;  // 0-unicast/1-multicast, default multicast
        // newNode->audio_session.transport.layer;          // rtsp setup response only
        newNode->audio_session.transport.mode = RTSP_TRANSPORT_PLAY; // PLAY/RECORD, default PLAY, rtsp setup response only
        // newNode->audio_session.transport. append;         // use with RECORD mode only, rtsp setup response only
        // newNode->audio_session.transport.rtp_interleaved;
        // newNode->audio_session.transport.rtcp_interleaved;   // rtsp setup response only
    }

    newNode->video_session.local_rtp_port = VIDEO_RTP_PORT_DEFAULT;
    newNode->video_session.local_rtcp_port = VIDEO_RTCP_PORT_DEFAULT;

    newNode->video_session.b_enable = true; // 使能
    // newNode->video_session.rtp_sock = -1;
    // newNode->video_session.rtcp_sock = -1;
    newNode->video_session.i_trackid = 0;                           // 视频通道
    newNode->video_session.b_issetup = false;                       // 视频是否设置
    newNode->video_session.transport.type = RTSP_TRANSPORT_UNICAST; // 对端期待的传输模式
    
    newNode->video_session.pdata = NULL;
    newNode->video_session.packetization_mode = 1;
    newNode->video_session.frame_delay_msec = 0;
    newNode->video_session.payload = RTP_MEDIA_PAYLOAD_H264;
    newNode->video_session.framerate = ZPL_VIDEO_FRAMERATE_DEFAULT;
    newNode->video_session.rtpmode = 2; // RTP_SESSION_SENDRECV;
    newNode->video_session.user_timestamp = 0;
    newNode->video_session.frame_delay_msec = 1000/newNode->video_session.framerate;
    if (!srv)
    {
        newNode->video_session.transport.rtp.unicast.local_rtp_port = newNode->video_session.local_rtp_port;
        newNode->video_session.transport.rtp.unicast.local_rtcp_port = newNode->video_session.local_rtcp_port;
        newNode->video_session.transport.proto = RTSP_TRANSPORT_RTP_UDP; // RTSP_TRANSPORT_xxx
        newNode->video_session.transport.type = RTSP_TRANSPORT_UNICAST;  // 0-unicast/1-multicast, default multicast
        // newNode->video_session.transport.layer;          // rtsp setup response only
        newNode->video_session.transport.mode = RTSP_TRANSPORT_PLAY; // PLAY/RECORD, default PLAY, rtsp setup response only
        // newNode->video_session.transport. append;         // use with RECORD mode only, rtsp setup response only
        // newNode->video_session.transport.rtp_interleaved;
        // newNode->video_session.transport.rtcp_interleaved;   // rtsp setup response only
    }

    newNode->mfilepath = NULL;
    newNode->mchannel = newNode->mlevel = -1;
    // newNode->_sendto = rtsp_session_sendto;
    // newNode->_recvfrom = rtsp_session_recvfrom;
    RTSP_SESSION_UNLOCK(newNode);
    return 0;
}

rtsp_session_t *rtsp_session_create(zpl_socket_t sock, const char *address, uint16_t port, void *parent)
{
    rtsp_session_t *newNode = NULL; // 每次申请链表结点时所使用的指针
    newNode = (rtsp_session_t *)malloc(sizeof(rtsp_session_t));
    if (newNode)
    {
        memset(newNode, 0, sizeof(rtsp_session_t));
        newNode->mutex = os_mutex_name_init("rtspsession");
        if (address)
            newNode->address = strdup(address);
        newNode->sock = sock;
        newNode->port = port;
        newNode->parent = parent;
        return newNode;
    }
    return newNode;
}
#if 1
rtsp_session_t *rtsp_session_add(struct osker_list_head *list, zpl_socket_t sock, const char *address, uint16_t port, void *parent)
{
    rtsp_session_t *newNode = rtsp_session_create(sock, address, port, parent);
    if (newNode)
    {
        newNode->bsrv = true;

        rtsp_session_default(newNode, true);

        osker_list_add_tail(&newNode->node, list); // 调用list.h中的添加节点的函数osker_list_add_tail

        return newNode;
    }
    return newNode;
}
#else
rtsp_session_t *rtsp_session_add(struct osker_list_head *list, zpl_socket_t sock, const char *address, uint16_t port, void *parent)
{
    rtsp_session_t *newNode = NULL; // 每次申请链表结点时所使用的指针
    newNode = (rtsp_session_t *)malloc(sizeof(rtsp_session_t));
    if (newNode)
    {
        memset(newNode, 0, sizeof(rtsp_session_t));
        newNode->mutex = os_mutex_name_init("rtspsession");
        if (address)
            newNode->address = strdup(address);
        newNode->sock = sock;
        newNode->port = port;
        newNode->bsrv = true;
        newNode->parent = parent;

        rtsp_session_default(newNode, true);
        // RTSP_SESSION_LOCK(newNode);
        newNode->session = (int)newNode;
        osker_list_add_tail(&newNode->node, list); // 调用list.h中的添加节点的函数osker_list_add_tail
        // RTSP_SESSION_UNLOCK(newNode);
        return newNode;
    }
    return newNode;
}
#endif

int rtsp_session_del(struct osker_list_head *list, zpl_socket_t sock)
{
    struct osker_list_head *pos = NULL, *n = NULL;
    rtsp_session_t *p = NULL;
    int judge = 0;
    osker_list_for_each_safe(pos, n, list)
    {
        p = (rtsp_session_t *)osker_list_entry(pos, rtsp_session_t, node);
        if (p && p->sock == sock)
        {
            judge = 1;
            osker_list_del(pos);
            rtsp_session_destroy(p);
            p = NULL;
            // printf("this node %d has removed from the doublelist...\n",i);
        }
    }
    if (judge == 0)
    {
        // printf("NO FOUND!\n");
    }
    else
        return 0;
    return -1;
}
int rtsp_session_del_byid(struct osker_list_head *list, uint32_t id)
{
    struct osker_list_head *pos = NULL, *n = NULL;
    rtsp_session_t *p = NULL;
    int judge = 0;
    osker_list_for_each_safe(pos, n, list)
    {
        p = (rtsp_session_t *)osker_list_entry(pos, rtsp_session_t, node);
        if (p && p->session == id)
        {
            judge = 1;
            osker_list_del(pos);
            rtsp_session_destroy(p);
            p = NULL;
            // printf("this node %d has removed from the doublelist...\n",i);
        }
    }
    if (judge == 0)
    {
        // printf("NO FOUND!\n");
    }
    else
        return 0;
    return -1;
}

int rtsp_session_cleancache(struct osker_list_head *list)
{
    struct osker_list_head *pos = NULL, *n = NULL;
    rtsp_session_t *p = NULL;
    osker_list_for_each_safe(pos, n, list)
    {
        p = (rtsp_session_t *)osker_list_entry(pos, rtsp_session_t, node);
        if (p && p->sock && p->state == RTSP_SESSION_STATE_CLOSE)
        {
            osker_list_del(pos);
            rtsp_session_destroy(p);
            p = NULL;
        }
    }
    return 0;
}

rtsp_session_t *rtsp_session_lookup(struct osker_list_head *list, zpl_socket_t sock)
{
    struct osker_list_head *pos = NULL, *n = NULL;
    rtsp_session_t *p = NULL;
    osker_list_for_each_safe(pos, n, list)
    {
        p = (rtsp_session_t *)osker_list_entry(pos, rtsp_session_t, node);
        if (p && p->sock == sock)
        {
            return p;
        }
    }
    return NULL;
}

rtsp_session_t *rtsp_session_lookup_byid(struct osker_list_head *list, uint32_t sessionid)
{
    struct osker_list_head *pos = NULL, *n = NULL;
    rtsp_session_t *p = NULL;
    osker_list_for_each_safe(pos, n, list)
    {
        p = (rtsp_session_t *)osker_list_entry(pos, rtsp_session_t, node);
        if (p && p->session == sessionid)
        {
            return p;
        }
    }
    return NULL;
}

int rtsp_session_foreach(struct osker_list_head *list, int (*calback)(rtsp_session_t *, void *), void *pVoid)
{
    struct osker_list_head *pos = NULL, *n = NULL;
    rtsp_session_t *p = NULL;
    osker_list_for_each_safe(pos, n, list)
    {
        p = (rtsp_session_t *)osker_list_entry(pos, rtsp_session_t, node);
        if (p && p->sock && calback)
        {
            (calback)(p, pVoid);
        }
    }
    return 0;
}

int rtsp_session_update_maxfd(struct osker_list_head *list)
{
    struct osker_list_head *pos = NULL, *n = NULL;
    rtsp_session_t *p = NULL;
    int maxfd = 0;
    osker_list_for_each_safe(pos, n, list)
    {
        p = (rtsp_session_t *)osker_list_entry(pos, rtsp_session_t, node);
        if (p && p->sock)
        {
            maxfd = MAX(maxfd, ipstack_fd(p->sock));
        }
    }
    return maxfd;
}

/*
int rtsp_session_update_maxfd(rtsp_session_t * head, fd_set *rset, fd_set *wset)
{
    struct osker_list_head *pos = NULL, *n = NULL;
    rtsp_session_t *p = NULL;
    int maxfd = 0;
    osker_list_for_each_safe(pos,n,&head->list)
    {
        p = (rtsp_session_t*)osker_list_entry(pos, rtsp_session_t, list);
        if(p->sock)
        {
            if(rset)
                FD_SET(p->sock, rset);
            if(wset)
                FD_SET(p->sock, wset);
            maxfd = max(maxfd, p->sock);
        }
    }
    return maxfd;
}

int rtsp_session_read(rtsp_session_t * head, fd_set *rset, char *req, int req_length)
{
    struct osker_list_head *pos = NULL, *n = NULL;
    rtsp_session_t *p = NULL;
    osker_list_for_each_safe(pos,n,&head->list)
    {
        p = (rtsp_session_t*)osker_list_entry(pos, rtsp_session_t, list);
        if(p->sock)
        {
            if(FD_ISSET(p->sock, rset))
            {
                if(p->_recvfrom)
                    (p->_recvfrom)(p, req, req_length);
            }
        }
    }
    return 0;
}

int rtsp_session_write(rtsp_session_t * head, fd_set *wset, char *req, int req_length)
{
    struct osker_list_head *pos = NULL, *n = NULL;
    rtsp_session_t *p = NULL;
    int maxfd = 0;
    osker_list_for_each_safe(pos,n,&head->list)
    {
        p = (rtsp_session_t*)osker_list_entry(pos, rtsp_session_t, list);
        if(p->sock)
        {
            if(FD_ISSET(p->sock, wset))
            {
                if(p->_sendto)
                    (p->_sendto)(p, req, req_length);
            }
        }
    }
    return 0;
}
*/

int rtsp_session_count(struct osker_list_head *list)
{
    struct osker_list_head *pos = NULL, *n = NULL;
    int count = 0;
    osker_list_for_each_safe(pos, n, list)
    {
        count++;
    }
    return count;
}
