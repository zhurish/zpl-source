/*
 * Copyright © 2001-2013 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
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



static rtsp_session_list _rtsp_session_lst;
int _rtsp_session_debug = 0xffffff;

static int rtsp_session_event_handle(rtsp_session_t *session);
static int rtsp_session_read_eloop(struct eloop *ctx);

int rtsp_session_sendto(rtsp_session_t *session, uint8_t *req, uint32_t req_length)
{
    if (!ipstack_invalid(session->sock))
        return ipstack_send(session->sock, req, req_length, 0);
    return ERROR;
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

int rtsp_session_init(void)
{
    _rtsp_session_lst.mutex = os_mutex_name_create("rtspsession");
    lstInit(&_rtsp_session_lst._list_head);
    return OK;
}

int rtsp_session_exit(void)
{
    rtsp_session_t *p = NULL;
	NODE node;
    for (p = (rtsp_session_t*)lstFirst( &_rtsp_session_lst._list_head); p != NULL; p = (rtsp_session_t*)lstNext(&node))
    {
        node = p->node;
        if (p)
        {
            rtsp_session_destroy(p);
        }
    }
    lstFree(&_rtsp_session_lst._list_head);
    if (_rtsp_session_lst.mutex)
    {
        os_mutex_destroy(_rtsp_session_lst.mutex);
        _rtsp_session_lst.mutex = NULL;
    }
    return OK;
}



int rtsp_session_default(rtsp_session_t *newNode, bool srv)
{
    RTSP_SESSION_LOCK(newNode);
    newNode->bsrv = srv;
    newNode->sesid = (int)newNode;
    newNode->state = RTSP_SESSION_STATE_CONNECT;

    if(!srv)
        newNode->sesid = 0;

    newNode->mrtp_session[0].b_enable = 0;
    newNode->mrtp_session[0].b_video = 0;
    newNode->mrtp_session[0].i_trackid = -1;
    newNode->mrtp_session[0].transport.proto = RTSP_TRANSPORT_RTP_UDP;      // RTSP_TRANSPORT_xxx
    newNode->mrtp_session[0].transport.type = RTSP_TRANSPORT_UNICAST;     // 0-unicast/1-multicast, default multicast
    newNode->mrtp_session[0].transport.destination = NULL;   // IPv4/IPv6
    newNode->mrtp_session[0].transport.source = NULL;        // IPv4/IPv6
    newNode->mrtp_session[0].transport.layer = 0;          // rtsp setup response only
    newNode->mrtp_session[0].transport.mode = 0;           // PLAY/RECORD, default PLAY, rtsp setup response only
    newNode->mrtp_session[0].transport.append = 0;         // use with RECORD mode only, rtsp setup response only
    newNode->mrtp_session[0].transport.rtp_interleaved = -1;
    newNode->mrtp_session[0].transport.rtcp_interleaved = -1;   // rtsp setup response only
    memcpy(&newNode->mrtp_session[1], &newNode->mrtp_session[0], sizeof(rtsp_rtp_session_t));
    newNode->mfilepath = NULL;
    newNode->mchannel = newNode->mlevel = -1;

    RTSP_SESSION_UNLOCK(newNode);
    return OK;
}

rtsp_session_t *rtsp_session_create(zpl_socket_t sock, const char *address, uint16_t port, void *ctx, void *master, char *localip)
{
    rtsp_session_t *newNode = NULL; // 每次申请链表结点时所使用的指针
    newNode = (rtsp_session_t *)malloc(sizeof(rtsp_session_t));
    if (newNode)
    {
        memset(newNode, 0, sizeof(rtsp_session_t));

        if (address)
            newNode->address = strdup(address);
        newNode->srvname = strdup(RTSP_SRV_NAME); 
        newNode->sock = sock;
        newNode->port = port;
        //newNode->parent = ctx;
        newNode->listen_address = localip;
        rtsp_session_default(newNode, true);

        lstAdd(&_rtsp_session_lst._list_head, (NODE*)newNode); // 调用list.h中的添加节点的函数osker_list_add_tail
        newNode->t_master = master;
        newNode->t_read = eloop_add_read(newNode->t_master, rtsp_session_read_eloop, newNode, sock);
        return newNode;
    }
    return newNode;
}


int rtsp_session_destroy(rtsp_session_t *session)
{
    if (session)
    {
        RTSP_SESSION_LOCK(session);

        if (!ipstack_invalid(session->sock))
        {
            ipstack_close(session->sock);
            session->sock = ZPL_SOCKET_INVALID;
        }

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
        if (session->address)
        {
            free(session->address);
            session->address = NULL;
        }
        if (session->srvname)
        {
            free(session->srvname);
            session->srvname = NULL;
        }
        RTSP_SESSION_UNLOCK(session);

        free(session);
    }
    return OK;
}


int rtsp_session_close(rtsp_session_t *session)
{
    if (session->t_read)
        eloop_cancel(session->t_read);
    lstDelete(&_rtsp_session_lst._list_head, (NODE *)session);
    rtsp_session_destroy(session);
    return OK;
}

rtsp_session_t *rtsp_session_lookup(zpl_socket_t sock)
{
    rtsp_session_t *p = NULL;
	NODE node;
    for (p = (rtsp_session_t*)lstFirst( &_rtsp_session_lst._list_head); p != NULL; p = (rtsp_session_t*)lstNext(&node))
    {
        node = p->node;
        if (p && p->sock == sock)
        {
            return p;
        }
    }
    return NULL;
}


int rtsp_session_foreach(int (*calback)(rtsp_session_t *, void *), void *pVoid)
{
    rtsp_session_t *p = NULL;
	NODE node;
    for (p = (rtsp_session_t*)lstFirst( &_rtsp_session_lst._list_head); p != NULL; p = (rtsp_session_t*)lstNext(&node))
    {
        node = p->node;
        if (p && p->sock && calback)
        {
            (calback)(p, pVoid);
        }
    }
    return OK;
}

int rtsp_session_update_maxfd(void)
{
    rtsp_session_t *p = NULL;
	NODE node;
    int maxfd = 0;
    for (p = (rtsp_session_t*)lstFirst( &_rtsp_session_lst._list_head); p != NULL; p = (rtsp_session_t*)lstNext(&node))
    {
        node = p->node;
        if (p && p->sock)
        {
            maxfd = MAX(maxfd, ipstack_fd(p->sock));
        }
    }
    return maxfd;
}



int rtsp_session_count(void)
{
    return lstCount(&_rtsp_session_lst._list_head);
}

/******************************************************************************/

static int rtsp_session_sdp_parse(rtsp_session_t *session, int *find)
{
    char *p = (char*)session->_recv_buf;
    int len = 0, maxlen = session->_recv_length;
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

static int rtsp_session_read_eloop(struct eloop *ctx)
{
    int ret = 0;
    int findsdp = 0, sdplen;
    uint8_t tmpbuf[RTSP_PACKET_MAX];
    rtsp_session_t *session = ELOOP_ARG(ctx);
    if (session && !ipstack_invalid(session->sock))
    {
        session->t_read = NULL;
        if(session->_recv_length == 0)
            memset(session->_recv_buf, 0, sizeof(session->_recv_buf));
        ret = ipstack_recv(session->sock, session->_recv_buf + session->_recv_length, sizeof(session->_recv_buf)-4-session->_recv_length, 0);   
        if (ret <= 0)
        {
            rtsp_log_error("rtsp server read msg error for %s:%d [%d], and close it, error:%s", session->address, session->port, 
                    ipstack_fd(session->sock), ipstack_strerror(errno));
            rtsp_session_close(session);
            return OK;
        }
        else
        {
            session->_recv_length += ret;
            if (session->_recv_buf[0] == '$')
            {
                int need_len = 0;
                rtp_tcp_header_t *hdr = (rtp_tcp_header_t *)session->_recv_buf;
                session->_recv_offset = 4;
                need_len = sizeof(rtp_tcp_header_t) + ntohs(hdr->length);
                if(session->_recv_length < 4)
                {
                    session->t_read = eloop_add_read(session->t_master, rtsp_session_read_eloop, session, session->sock);
                    return OK;
                }
                if(need_len > session->_recv_length)
                {
                    ret = ipstack_recv(session->sock, session->_recv_buf + session->_recv_length, need_len - session->_recv_length, 0);  
                    if(ret < 0)
                    {
                        rtsp_session_close(session);
                        return OK;
                    }
                    session->_recv_length += ret;
                    if(need_len == session->_recv_length)
                        ;//rtsp_session_media_tcp_forward(session, session->_recv_buf, session->_recv_length);
                }  
                else
                {
                    ret = session->_recv_length - need_len;
                    if(ret)
                        memcpy(tmpbuf, session->_recv_buf + need_len, ret);
                    session->_recv_length = need_len;
                    //rtsp_session_media_tcp_forward(session, session->_recv_buf, session->_recv_length);
                    if(ret)
                    {
                        memcpy(session->_recv_buf, tmpbuf, ret);
                        session->_recv_length = ret;
                    }
                }  
            }
            else
            {
                ret = rtsp_session_sdp_parse(session, &findsdp);
                if(findsdp && ret)
                {
                    sdplen = (ret + 4);
                    if(session->_recv_length - sdplen)
                        memcpy(tmpbuf, session->_recv_buf + sdplen, session->_recv_length - sdplen);
                    session->_recv_offset = 0;
                    memset(session->_recv_buf + sdplen, 0, sizeof(session->_recv_buf) - sdplen);
                    ret = session->_recv_length - sdplen;
                    session->_recv_length = sdplen;
                    if(rtsp_session_event_handle(session) == ERROR)
                    {
                        rtsp_session_close(session);
                        return ERROR;
                    }
                    session->_recv_length = ret;
                    if(ret)
                        memcpy(session->_recv_buf, tmpbuf, ret);
                }
            }
        }
        session->t_read = eloop_add_read(session->t_master, rtsp_session_read_eloop, session, session->sock);
    }
    return OK;
}

static int rtsp_session_url_split(rtsp_session_t *session)
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
/*
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
*/
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
            return OK;
        return ERROR;
    }
    if ((session->mchannel != -1 || (session->mfilepath)))
        return OK;
    return ERROR;
}

static int rtsp_session_handle_option(rtsp_session_t *session)
{
    int length = 0;

    length = sdp_build_respone_header(session->_send_build, session->srvname, NULL, RTSP_STATE_CODE_200, session->cseq, session->sesid);

    length += sprintf((char*)(session->_send_build + length), "Public: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE\r\n");

    if (length)
    {
        length += sprintf((char*)(session->_send_build + length), "\r\n");
        return rtsp_session_sendto(session, session->_send_build, length);
    }
    return OK;
}

static int rtsp_session_handle_describe(rtsp_session_t *session)
{
    int length = 0;
    uint8_t buftmp[1024];
    int code = RTSP_STATE_CODE_200;

    code = RTSP_STATE_CODE_200;
    memset(buftmp, 0, sizeof(buftmp));

    length = sdp_build_respone_header(session->_send_build, session->srvname, session->rtsp_url,
                                      code, session->cseq, session->sesid);
    if (session->bsrv)
    {
        int sdplength = 0, desclen = 0;
        memset(buftmp, 0, sizeof(buftmp));
        sdplength += sprintf((char*)(buftmp + sdplength), "v=0\r\n");

        sdplength += sprintf((char*)(buftmp + sdplength), "o=%s %lu %u IN IPV4 0.0.0.0\r\n", session->srvname, time(NULL), rand());
        sdplength += sprintf((char*)(buftmp + sdplength), "c=IN IPV4 %s\r\n", session->listen_address ? session->listen_address : "0.0.0.0");
        sdplength += sprintf((char*)(buftmp + sdplength), "a=control:*\r\n");
        sdplength += sprintf((char*)(buftmp + sdplength), "t=0 0\r\n");
 
        //code = rtsp_session_media_describe(session, NULL);
        //sdplength += rtsp_session_media_build_sdptext(session, buftmp + sdplength);
        code = rtsp_session_media_describe(session, NULL, (char*)(buftmp + sdplength), &desclen);
        sdplength += desclen;
        length += sprintf((char*)(session->_send_build + length), "Content-Type: application/sdp\r\n");
        length += sprintf((char*)(session->_send_build + length), "Content-Length: %d\r\n", sdplength);
        length += sprintf((char*)(session->_send_build + length), "\r\n");
        length += sprintf((char*)(session->_send_build + length), "%s", buftmp);
    }
    else
    {
        //code = rtsp_session_media_describe(session, NULL);
    }
    if (length)
    {
        length += sprintf((char*)(session->_send_build + length), "\r\n");
        return rtsp_session_sendto(session, session->_send_build, length);
    }
    return OK;
}

static int rtsp_session_handle_setup(rtsp_session_t *session)
{
    int length = 0;
    int code = RTSP_STATE_CODE_200;

    if (code == RTSP_STATE_CODE_200)
    {
        if (session->sdptext.misc.url)
        {
            int trackID = -1;
            if (strstr(session->sdptext.misc.url, "trackID="))
            {
                trackID = sdp_get_intcode(session->sdptext.misc.url, "trackID=");
            }

            rtsp_header_transport(zpl_true, session->sdptext.header.Transport, &session->transport);

            session->sdptext.origin.id = session->sesid;

            length = sdp_build_respone_header(session->_send_build, session->srvname, NULL, code, session->cseq, session->sesid);

            if (session->transport.proto == RTSP_TRANSPORT_RTP_UDP)
            {
                char *addr[64];
                int rtp_port = 0, rtcp_port = 0;
                
                zpl_mediartp_session_remoteport(session->mchannel, session->mlevel, session->mfilepath, 
                    session->transport.destination, session->transport.rtp.unicast.rtp_port, session->transport.rtp.unicast.rtcp_port);

                zpl_mediartp_session_get_localport(session->mchannel, session->mlevel, session->mfilepath, addr, &rtp_port, &rtcp_port);

                length += sprintf((char*)(session->_send_build + length), "Transport: %s;server_port=%d-%d\r\n",
                                  session->sdptext.header.Transport,
                                  rtp_port, rtcp_port);
            }
            else if (session->transport.proto == RTSP_TRANSPORT_RTP_TCP)
            {
                char tmp[64];
                int rtp_interleaved = -1, rtcp_interleaved = -1;
                memset(tmp, 0, sizeof(tmp));

                if (trackID >= 0)
                {
                    zpl_mediartp_session_tcp_interleaved(session->mchannel, session->mlevel, session->mfilepath, 0, 1);
                }
                if(trackID >= 0)
                {
                    zpl_mediartp_session_get_tcp_interleaved(session->mchannel, session->mlevel, session->mfilepath, &rtp_interleaved, &rtcp_interleaved);
                    sprintf(tmp, ";interleaved=%d-%d", rtp_interleaved, rtcp_interleaved);
                }

                if (strlen(tmp))
                    length += sprintf((char*)(session->_send_build + length), "Transport: %s%s;\r\n",
                                      session->sdptext.header.Transport, tmp);
                else
                    length += sprintf((char*)(session->_send_build + length), "Transport: %s;\r\n",
                                      session->sdptext.header.Transport);
            }
            else if (session->transport.proto == RTSP_TRANSPORT_RTP_TLS)
            {
                length += sprintf((char*)(session->_send_build + length), "Transport: %s;destination=239.0.0.0\r\n",
                                  session->sdptext.header.Transport);
            }
            else if (session->transport.proto == RTSP_TRANSPORT_RTP_RTPOVERRTSP)
            {
                length += sprintf((char*)(session->_send_build + length), "Transport: %s;destination=239.0.0.0\r\n",
                                  session->sdptext.header.Transport);
            }
            else if (session->transport.proto == RTSP_TRANSPORT_RTP_MULTCAST)
            {
                length += sprintf((char*)(session->_send_build + length), "Transport: %s;destination=239.0.0.0\r\n",
                                  session->sdptext.header.Transport);
            }
        }
        else
            length = sdp_build_respone_header(session->_send_build, session->srvname, NULL, RTSP_STATE_CODE_404, session->cseq, session->sesid);

//int zpl_mediartp_session_tcp_interleaved(session->mchannel, session->mlevel, session->mfilepath, int rtp_interleaved, int rtcp_interleaved);
//int zpl_mediartp_session_trackid(session->mchannel, session->mlevel, session->mfilepath, int i_trackid);
//int zpl_mediartp_session_localport(session->mchannel, session->mlevel, session->mfilepath, char *addr, int rtp_port, int rtcp_port);

        code = rtsp_session_media_setup(session, NULL);

        if (code != RTSP_STATE_CODE_200)
        {
            length = sdp_build_respone_header(session->_send_build, session->srvname, NULL, code, session->cseq, session->sesid);
            if (length)
            {
                length += sprintf((char*)(session->_send_build + length), "\r\n");
                return rtsp_session_sendto(session, session->_send_build, length);
            }
            return OK;
        }
    }
    else
        length = sdp_build_respone_header(session->_send_build, session->srvname, NULL, code, session->cseq, session->sesid);
    if (length)
    {
        length += sprintf((char*)(session->_send_build + length), "\r\n");
        return rtsp_session_sendto(session, session->_send_build, length);
    }
    return OK;
}

static int rtsp_session_handle_teardown(rtsp_session_t *session)
{
    int length = 0;
    int code = RTSP_STATE_CODE_200;

    code = rtsp_session_media_teardown(session, NULL);

    if (code != RTSP_STATE_CODE_200)
    {
        length = sdp_build_respone_header(session->_send_build, session->srvname, NULL, code, session->cseq, session->sesid);
        length += sprintf((char*)(session->_send_build + length), "Connection: close\r\n");
    }
    else
    {
        length = sdp_build_respone_header(session->_send_build, session->srvname, NULL, code, session->cseq, session->sesid);
    }
    if (length)
    {
        int ret = 0;
        length += sprintf((char*)(session->_send_build + length), "\r\n");
        rtsp_session_sendto(session, session->_send_build, length);
        session->state = RTSP_SESSION_STATE_CLOSE;
        return ret;
    }
    
    session->state = RTSP_SESSION_STATE_CLOSE;
    return OK;
}

static int rtsp_session_handle_play(rtsp_session_t *session)
{
    int length = 0;
    int code = RTSP_STATE_CODE_200;

    code = rtsp_session_media_play(session, NULL);

    if (code != RTSP_STATE_CODE_200)
    {
        length = sdp_build_respone_header(session->_send_build, session->srvname, NULL, code, session->cseq, session->sesid);

        int i_trackid = -1;
        zpl_mediartp_session_get_trackid(session->mchannel, session->mlevel, session->mfilepath, &i_trackid);
        session->mrtp_session[0].i_trackid = i_trackid;

        if (session->mrtp_session[0].i_trackid >= 0 && session->mrtp_session[1].i_trackid >= 0)
        {
            length += sprintf((char*)(session->_send_build + length), "RTP-Info: url=trackID=%d;seq=0;rtptime=0;url=trackID=%d;seq=0;rtptime=0\r\n",
                              session->mrtp_session[0].i_trackid, session->mrtp_session[1].i_trackid);
        }
        else if (session->mrtp_session[0].i_trackid >= 0 && session->mrtp_session[1].i_trackid == -1)
        {
            length += sprintf((char*)(session->_send_build + length), "RTP-Info: url=trackID=%d;seq=0;rtptime=0\r\n",
                              session->mrtp_session[0].i_trackid);
        }
        else if (session->mrtp_session[1].i_trackid >= 0 && session->mrtp_session[0].i_trackid == -1)
        {
            length += sprintf((char*)(session->_send_build + length), "RTP-Info: url=trackID=%d;seq=0;rtptime=0\r\n",
                              session->mrtp_session[1].i_trackid);
        }
    }
    else
    {
        length = sdp_build_respone_header(session->_send_build, session->srvname, NULL, code, session->cseq, session->sesid);
    }
    if (length)
    {
        length += sprintf((char*)(session->_send_build + length), "\r\n");
        return rtsp_session_sendto(session, session->_send_build, length);
    }
    return OK;
}

static int rtsp_session_handle_pause(rtsp_session_t *session)
{
    int length = 0;
    int code = RTSP_STATE_CODE_200;

    code = rtsp_session_media_pause(session, NULL);

    if (code != RTSP_STATE_CODE_200)
        length = sdp_build_respone_header(session->_send_build, session->srvname, NULL, code, session->cseq, session->sesid);
    else
    {
        length = sdp_build_respone_header(session->_send_build, session->srvname, NULL, code, session->cseq, session->sesid);
    }
    if (length)
    {
        length += sprintf((char*)(session->_send_build + length), "\r\n");
        return rtsp_session_sendto(session, session->_send_build, length);
    }
    return OK;
}

static int rtsp_session_handle_scale(rtsp_session_t *session)
{
    int length = 0;
 
    if (length)
    {
        length += sprintf((char*)(session->_send_build + length), "\r\n");
        return rtsp_session_sendto(session, session->_send_build, length);
    }
    return OK;
}

static int rtsp_session_handle_set_parameter(rtsp_session_t *session)
{
    int length = 0;

    if (length)
    {
        length += sprintf((char*)(session->_send_build + length), "\r\n");
        return rtsp_session_sendto(session, session->_send_build, length);
    }
    return OK;
}

static int rtsp_session_handle_get_parameter(rtsp_session_t *session)
{
    int length = 0;

    if (length)
    {
        length += sprintf((char*)(session->_send_build + length), "\r\n");
        return rtsp_session_sendto(session, session->_send_build, length);
    }
    return OK;
}

static int rtsp_session_event_handle(rtsp_session_t *session)
{
    int ret = 0;
    char *recv_optstr[] = {"NONE", "OPTIONS", "DESCRIBE", "SETUP", "TEARDOWN", "PLAY", "PAUSE", "SCALE", "GET PARAMETER", "SET PARAMETER", "MAX METHOD"};

    RTSP_DEBUG_TRACE("===========================================\r\n");
    RTSP_DEBUG_TRACE("Client from %s:%d socket=%d session=%u\r\n", session->address ? session->address : " ",
               session->port, ipstack_fd(session->sock), session->sesid);
    RTSP_DEBUG_TRACE("===========================================\r\n");

    sdp_text_init(&session->sdptext, (const char *)(session->_recv_buf + session->_recv_offset),
                  session->_recv_length - session->_recv_offset);

    sdp_text_prase(true, &session->sdptext);

    if (session->sdptext.header.CSeq > -1)
    {
        session->cseq = (session->sdptext.header.CSeq);
    }
    memset(session->_send_buf, 0, sizeof(session->_send_buf));

    session->_send_build = session->_send_buf + session->_send_offset;


    if(RTSP_DEBUG_FLAG(_rtsp_session_debug , EVENT) && session->sdptext.method >= 0 && session->sdptext.method <= RTSP_METHOD_MAX)
        rtsp_log_debug( "RTSP Server Secv '%s' Request", recv_optstr[session->sdptext.method]);

    if(RTSP_DEBUG_FLAG(_rtsp_session_debug , DEBUG) && RTSP_DEBUG_FLAG(_rtsp_session_debug , DETAIL))
        rtsp_log_debug("\r\nC -> S:\r\n%s\r\n", session->_recv_buf + session->_recv_offset);

    if (rtsp_session_url_split(session) != 0)
    {
        return ERROR;
    }
    if (!rtsp_session_media_lookup(session, session->mchannel, session->mlevel, session->mfilepath))
    {
        int length = sdp_build_respone_header(session->_send_build, session->srvname, NULL, RTSP_STATE_CODE_404, session->cseq, session->sesid);
        if (length)
        {
            length += sprintf((char*)(session->_send_build + length), "\r\n");
            rtsp_session_sendto(session, session->_send_build, length);
        }
        return ERROR;
    }
    switch (session->sdptext.method)
    {
    case RTSP_METHOD_OPTIONS:
        ret = rtsp_session_handle_option(session);
        break;
    case RTSP_METHOD_DESCRIBE:
        ret = rtsp_session_handle_describe(session);
        break;
    case RTSP_METHOD_SETUP:
        ret = rtsp_session_handle_setup(session);
        break;
    case RTSP_METHOD_TEARDOWN:
        ret = rtsp_session_handle_teardown(session);
        break;
    case RTSP_METHOD_PLAY:
        ret = rtsp_session_handle_play(session);
        break;
    case RTSP_METHOD_PAUSE:
        ret = rtsp_session_handle_pause(session);
        break;
    case RTSP_METHOD_SCALE:
        ret = rtsp_session_handle_scale(session);
        break;
    case RTSP_METHOD_GET_PARAMETER:
        ret = rtsp_session_handle_get_parameter(session);
        break;
    case RTSP_METHOD_SET_PARAMETER:
        ret = rtsp_session_handle_set_parameter(session);
        break;
    default:
        {
            RTSP_DEBUG_TRACE( "===============:%s -> default\r\n", __func__);
            int length = sdp_build_respone_header(session->_send_build, session->srvname, NULL,
                                                RTSP_STATE_CODE_405, session->cseq, session->sesid);
            if (length)
            {
                length += sprintf((char*)(session->_send_build + length), "\r\n");
                return rtsp_session_sendto(session, session->_send_build, length);
            }
        }
        break;
    }
    if (ret)
    {
        if(RTSP_DEBUG_FLAG(_rtsp_session_debug , DEBUG) && RTSP_DEBUG_FLAG(_rtsp_session_debug , DETAIL))
            rtsp_log_debug("\r\nS -> C(ret=%d):\r\n%s\r\n", ret, session->_send_build);
    }

    sdp_text_free(&session->sdptext);
    return OK;
}