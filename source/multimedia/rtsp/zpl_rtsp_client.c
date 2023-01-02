/*
 * Copyright © 2001-2013 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <ortp/port.h>
#include <ortp/logging.h>
#include <ortp/ortp_list.h>
#include <ortp/extremum.h>
#include <ortp/rtp_queue.h>
#include <ortp/rtp.h>
#include <ortp/rtcp.h>
#include <ortp/sessionset.h>
#include <ortp/payloadtype.h>
#include <ortp/rtpprofile.h>
#include <ortp/rtpsession_priv.h>
#include <ortp/rtpsession.h>
#include <ortp/telephonyevents.h>
#include <ortp/rtpsignaltable.h>
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
#include "zpl_rtsp_rtp.h"

static int rtsp_client_event_handle(rtsp_client_t *client);

rtsp_client_t *rtsp_client_create(const char *name, const char *url)
{
    rtsp_client_t *client = NULL; // 每次申请链表结点时所使用的指针
    client = (rtsp_client_t *)malloc(sizeof(rtsp_client_t));
    if (client)
    {
        os_url_t urlpath;
        memset(client, 0, sizeof(rtsp_client_t));

        memset(&urlpath, 0, sizeof(os_url_t));
        rtsp_url_stream_path(url, &urlpath);

        if (urlpath.host && strstr(urlpath.host, ":"))
        {
            char *p = strstr(urlpath.host, ":");
            *p = '\0';
        }
        // if(strlen(proto))
        //     client->proto = strdup(proto);
        if (urlpath.user && strlen(urlpath.user))
            client->authorization = strdup(urlpath.user);
        if (urlpath.host && strlen(urlpath.host))
            client->hostname = strdup(urlpath.host);
        if (urlpath.path && strlen(urlpath.path))
            client->path = strdup(urlpath.path);

        client->url = strdup(url);
        client->clientname = strdup(name);

        client->rtsp_session = rtsp_session_create(NULL, urlpath.host, (urlpath.port == 0) ? 554 : urlpath.port, client);
        if (client->rtsp_session)
        {
            rtsp_session_default(client->rtsp_session, false);

            sdp_text_init(&client->rtsp_session->sdptext, (const char *)(client->_recv_sdpbuf + client->_recv_offset),
                          client->_recv_length - client->_recv_offset);

            client->_send_offset = 4;
            client->_send_build = client->_send_buf + client->_send_offset;
            client->timeout = RTSP_REQUEST_TIMEOUT;

            client->istcp = false;
            fprintf(stdout, "%s [%d]  rtsp client connect:%s:%d\r\n", __func__, __LINE__,
                    client->rtsp_session->address, client->rtsp_session->port);
            fflush(stdout);
        }
        else
        {
            free(client);
            client = NULL;
        }
        os_url_free(&urlpath);
        return client;
    }
    return NULL;
}

int rtsp_client_destroy(rtsp_client_t *client)
{
    if (client == NULL)
        return -1;
    // if(client->rtsp_session->state != RTSP_SESSION_STATE_NONE)

    if (client->rtsp_session)
    {
        rtsp_session_destroy(client->rtsp_session);
        client->rtsp_session = NULL;
        // client->rtsp_session->state = RTSP_SESSION_STATE_NONE;
    }
    if (client->clientname)
    {
        free(client->clientname);
        client->clientname = NULL;
    }
    if (client->url)
    {
        free(client->url);
        client->url = NULL;
    }
    if (client->proto)
    {
        free(client->proto);
        client->proto = NULL;
    }
    if (client->authorization)
    {
        free(client->authorization);
        client->authorization = NULL;
    }
    if (client->hostname)
    {
        free(client->hostname);
        client->hostname = NULL;
    }
    if (client->path)
    {
        free(client->path);
        client->path = NULL;
    }
    free(client);
    client = NULL;
    return 0;
}

int rtsp_client_connect(rtsp_client_t *client, int timeout)
{
    int i = 5;
    while (i)
    {
        if (rtsp_session_connect(client->rtsp_session, client->rtsp_session->address, client->rtsp_session->port, timeout) != OK)
        {
            continue;
        }
        i--;
    }
    return -1;
}

static int rtsp_client_tcpthread(rtsp_client_t *client)
{
    int ret = 0;
    ipstack_fd_set rset;
    if (client->rtsp_session && !ipstack_invalid(client->rtsp_session->sock))
    {
        IPSTACK_FD_ZERO(&rset);
        IPSTACK_FD_SET(ipstack_fd(client->rtsp_session->sock), &rset);

        ret = ipstack_select_wait(ipstack_type(client->rtsp_session->sock), ipstack_fd(client->rtsp_session->sock) + 1, &rset, NULL, 1);
        if (ret > 0)
        {
            if (IPSTACK_FD_ISSET(ipstack_fd(client->rtsp_session->sock), &rset))
            {
                IPSTACK_FD_CLR(ipstack_fd(client->rtsp_session->sock), &rset);
                {
                    ret = rtsp_session_recvfrom(client->rtsp_session, client->_recv_buf, sizeof(client->_recv_buf));
                    if (ret)
                    {
                        if (client->_recv_buf[0] == '$')
                        {
                            // client->_recv_offset = 4;
                            return rtsp_media_tcp_forward(client->rtsp_session, client->_recv_buf, ret);
                        }
                        else
                        {
                            if (!ipstack_invalid(client->rtsp_session->sock))
                            {
                                return ipstack_send(client->rtsp_session->sock, client->_recv_buf, ret, 0);
                            }
                        }
                    }
                }
                fprintf(stdout, "%s [%d]  rtsp client select\r\n", __func__, __LINE__);
                fflush(stdout);
                return -1;
            }
        }
        else if (ret == 0)
        {
            fprintf(stdout, "%s [%d]  rtsp client select timeout\r\n", __func__, __LINE__);
            fflush(stdout);
            return 1;
        }
        else
        {
            fprintf(stdout, "%s [%d]  rtsp client select %s\r\n", __func__, __LINE__, strerror(errno));
            fflush(stdout);
            return -1;
        }
    }
    return -1;
}

static int rtsp_client_wait_respone(rtsp_client_t *client, int timeout)
{
    int ret = 0;
    ipstack_fd_set rset;
    if (client->rtsp_session && !ipstack_invalid(client->rtsp_session->sock))
    {
        IPSTACK_FD_ZERO(&rset);
        IPSTACK_FD_SET(ipstack_fd(client->rtsp_session->sock), &rset);

        ret = ipstack_select_wait(ipstack_type(client->rtsp_session->sock), ipstack_fd(client->rtsp_session->sock) + 1, &rset, NULL, timeout);
        if (ret > 0)
        {
            if (IPSTACK_FD_ISSET(ipstack_fd(client->rtsp_session->sock), &rset))
            {
                IPSTACK_FD_CLR(ipstack_fd(client->rtsp_session->sock), &rset);
                fprintf(stdout, "%s [%d]  rtsp client select\r\n", __func__, __LINE__);
                fflush(stdout);
                return 0;
            }
        }
        else if (ret == 0)
        {
            fprintf(stdout, "%s [%d]  rtsp client select timeout\r\n", __func__, __LINE__);
            fflush(stdout);
            return 1;
        }
        else
        {
            fprintf(stdout, "%s [%d]  rtsp client select %s\r\n", __func__, __LINE__, strerror(errno));
            fflush(stdout);
            return -1;
        }
    }
    return -1;
}

static int rtsp_client_read_respone(rtsp_client_t *client)
{
    if (client->rtsp_session && !ipstack_invalid(client->rtsp_session->sock))
    {
        memset(client->_recv_sdpbuf, 0, sizeof(client->_recv_sdpbuf));
        client->_recv_length = ipstack_recv(client->rtsp_session->sock, client->_recv_sdpbuf, sizeof(client->_recv_sdpbuf), 0);
        if (client->_recv_length <= 0)
        {
            client->rtsp_session->state = RTSP_SESSION_STATE_CLOSE;
            fprintf(stdout, "%s [%d]  rtsp client recvfrom %s\r\n", __func__, __LINE__, strerror(errno));
            fflush(stdout);
        }
        else
        {
            client->_recv_offset = 0;
            fprintf(stdout, "%s [%d]  rtsp client recvfrom %d byte\r\n", __func__, __LINE__, client->_recv_length);
            fflush(stdout);
            return rtsp_client_event_handle(client);
        }
    }
    return -1;
}

static int rtsp_client_request_and_wait_respone(rtsp_client_t *client, rtsp_session_t *session, int timeout)
{
    int ret = 0;
    client->_send_length += sprintf(client->_send_build + client->_send_length, "\r\n");
    ret = rtsp_session_sendto(session, client->_send_buf + client->_send_offset,
                              client->_send_length);
    if (ret)
    {
        fprintf(stdout, "\r\n");
        fprintf(stdout, "C -> S(ret=%d):\r\n%s", ret, client->_send_buf + client->_send_offset);
        fprintf(stdout, "\r\n");
        fflush(stdout);
    }
    if (ret)
    {
        ret = rtsp_client_wait_respone(client, timeout);
        if (ret == 0)
        {
            return rtsp_client_read_respone(client);
        }
        else if (ret == 1)
        {
            // timeout;
        }
        else
        {
            // error
        }
    }
    return -1;
}

static int rtsp_client_event_handle(rtsp_client_t *client)
{
    int ret = -1;
    fprintf(stdout, "\r\n");
    fprintf(stdout, "S -> C:\r\n%s", client->_recv_sdpbuf + client->_recv_offset);
    fprintf(stdout, "\r\n");
    fflush(stdout);

    // client->method = client->method;
    /*    sdp_text_init(&client->rtsp_session->sdptext, (const char *)(client->_recv_sdpbuf+client->_recv_offset),
                     client->_recv_length-client->_recv_offset);
*/
    // client->rtsp_session->sdptext.media_index = -1;
    // client->rtsp_session->sdptext.header.CSeq = -1;
    client->rtsp_session->sdptext.misc.sdp_start = client->rtsp_session->sdptext.misc.sdp_data;
    client->rtsp_session->sdptext.misc.sdp_offset = 0;
    client->rtsp_session->sdptext.misc.sdp_len = client->_recv_length - client->_recv_offset;

    sdp_text_prase(false, &client->rtsp_session->sdptext);

    client->code = client->rtsp_session->sdptext.code;

    if (client->rtsp_session->session == 0 && client->rtsp_session->sdptext.header.Session)
        client->rtsp_session->session = atoi(client->rtsp_session->sdptext.header.Session);

    if (client->code != RTSP_STATE_CODE_200)
    {
        fprintf(stdout, "code:%s\r\n", client->rtsp_session->sdptext.misc.result_string);
        fflush(stdout);
        client->method = RTSP_METHOD_NONE;
        sdp_text_free(&client->rtsp_session->sdptext);
        return client->code;
    }

    switch (client->method)
    {
    case RTSP_METHOD_OPTIONS:
        ret = rtsp_media_handle_option(client->rtsp_session, &client->client_media);
        fprintf(stdout, "===============:%s <- RTSP_METHOD_OPTIONS(%d)\r\n", __func__, ret);
        break;
    case RTSP_METHOD_DESCRIBE:
        ret = rtsp_media_handle_describe(client->rtsp_session, &client->client_media);
        fprintf(stdout, "===============:%s <- RTSP_METHOD_DESCRIBE(%d)\r\n", __func__, ret);
        break;
    case RTSP_METHOD_SETUP:
        if (!client->rtsp_session->bsrv)
        {
            if (client->rtsp_session->_rtpsession)
            {
                rtsp_header_transport(zpl_false, client->rtsp_session->sdptext.header.Transport,
                                      &client->rtsp_session->_rtpsession->transport);
            }
        }
        ret = rtsp_media_handle_setup(client->rtsp_session, &client->client_media);
        fprintf(stdout, "===============:%s <- RTSP_METHOD_SETUP(%d)\r\n", __func__, ret);
        break;
    case RTSP_METHOD_TEARDOWN:
        ret = rtsp_media_handle_teardown(client->rtsp_session, &client->client_media);
        fprintf(stdout, "===============:%s <- RTSP_METHOD_TEARDOWN(%d)\r\n", __func__, ret);
        break;
    case RTSP_METHOD_PLAY:
        ret = rtsp_media_handle_play(client->rtsp_session, &client->client_media);
        fprintf(stdout, "===============:%s <- RTSP_METHOD_PLAY(%d)\r\n", __func__, ret);
        break;
    case RTSP_METHOD_PAUSE:
        ret = rtsp_media_handle_pause(client->rtsp_session, &client->client_media);
        fprintf(stdout, "===============:%s <- RTSP_METHOD_PAUSE(%d)\r\n", __func__, ret);
        break;
    case RTSP_METHOD_SCALE:
        ret = rtsp_media_handle_scale(client->rtsp_session, &client->client_media);
        fprintf(stdout, "===============:%s <- RTSP_METHOD_SCALE(%d)\r\n", __func__, ret);
        break;
    case RTSP_METHOD_GET_PARAMETER:
        ret = rtsp_media_handle_get_parameter(client->rtsp_session, &client->client_media);
        fprintf(stdout, "===============:%s <- RTSP_METHOD_GET_PARAMETER(%d)\r\n", __func__, ret);
        break;
    case RTSP_METHOD_SET_PARAMETER:
        ret = rtsp_media_handle_set_parameter(client->rtsp_session, &client->client_media);
        fprintf(stdout, "===============:%s <- RTSP_METHOD_SET_PARAMETER(%d)\r\n", __func__, ret);
        break;
    default:
        break;
    }

    if (ret == 0)
        rtsp_session_callback(client->rtsp_session, client->method);

    sdp_text_free(&client->rtsp_session->sdptext);
    return ret;
}

/* Client request */
static int rtsp_client_request_option(rtsp_client_t *client, rtsp_session_t *session)
{
    int length = 0;
    length = sdp_build_request_header(&session->sdptext,
                                      client->_send_build, client->clientname, RTSP_METHOD_OPTIONS,
                                      client->url, session->cseq);
    if (length)
    {
        client->method = RTSP_METHOD_OPTIONS;
        client->_send_length = length;
        return rtsp_client_request_and_wait_respone(client, session, client->timeout);
    }
    return 0;
}

static int rtsp_client_request_describe(rtsp_client_t *client, rtsp_session_t *session)
{
    int length = 0;
    length = sdp_build_request_header(&session->sdptext,
                                      client->_send_build, client->clientname,
                                      RTSP_METHOD_DESCRIBE, client->url, session->cseq);

    if (length)
    {
        client->method = RTSP_METHOD_DESCRIBE;
        client->_send_length = length;
        return rtsp_client_request_and_wait_respone(client, session, client->timeout);
    }
    return 0;
}

static int rtsp_client_request_setup(rtsp_client_t *client, rtsp_session_t *session)
{
    int length = 0, ret = -1;

    if (session->video_session.b_enable)
    {
        char urltmp[256];
        memset(urltmp, 0, sizeof(urltmp));
        if (session->video_session.i_trackid >= 0)
            sprintf(urltmp, "%s/trackID=%d", client->url, session->video_session.i_trackid);
        length = sdp_build_request_header(&session->sdptext,
                                          client->_send_build, client->clientname,
                                          RTSP_METHOD_SETUP, urltmp, session->cseq);
        if (session->session)
            length += sdp_build_sessionID(&session->sdptext,
                                          client->_send_build + length, session->session);

        if (length && session->video_session.transport.rtp.unicast.local_rtp_port &&
            session->video_session.transport.rtp.unicast.local_rtcp_port)
            length += sprintf(client->_send_build + length, "Transport: RTP/AVP;unicast;client_port=%d-%d\r\n",
                              session->video_session.transport.rtp.unicast.local_rtp_port,
                              session->video_session.transport.rtp.unicast.local_rtcp_port);
        if (length)
        {
            client->rtsp_session->_rtpsession = &client->rtsp_session->video_session;
            client->method = RTSP_METHOD_SETUP;
            client->_send_length = length;
            ret = rtsp_client_request_and_wait_respone(client, session, client->timeout);
            session->cseq++;
        }
        if (ret != RTSP_STATE_CODE_200)
        {
            client->rtsp_session->_rtpsession = NULL;
            return -1;
        }
    }
    if (session->audio_session.b_enable)
    {
        char urltmp[256];
        memset(urltmp, 0, sizeof(urltmp));
        if (session->audio_session.i_trackid >= 0)
            sprintf(urltmp, "%s/trackID=%d", client->url, session->audio_session.i_trackid);
        length = sdp_build_request_header(&session->sdptext,
                                          client->_send_build, client->clientname,
                                          RTSP_METHOD_SETUP, urltmp, session->cseq);
        if (session->session)
            length += sdp_build_sessionID(&session->sdptext,
                                          client->_send_build + length, session->session);

        if (length && session->audio_session.transport.rtp.unicast.local_rtp_port &&
            session->audio_session.transport.rtp.unicast.local_rtcp_port)
            length += sprintf(client->_send_build + length, "Transport: RTP/AVP;unicast;client_port=%d-%d\r\n",
                              session->audio_session.transport.rtp.unicast.local_rtp_port,
                              session->audio_session.transport.rtp.unicast.local_rtcp_port);
        if (length)
        {
            fprintf(stdout, "=====:%s", client->_send_build);
            fflush(stdout);
            client->rtsp_session->_rtpsession = &client->rtsp_session->audio_session;
            client->method = RTSP_METHOD_SETUP;
            client->_send_length = length;
            ret = rtsp_client_request_and_wait_respone(client, session, client->timeout);
        }
    }
    if (ret != RTSP_STATE_CODE_200)
    {
        client->rtsp_session->_rtpsession = NULL;
        return -1;
    }
    return ret;
}

static int rtsp_client_request_teardown(rtsp_client_t *client, rtsp_session_t *session)
{
    int length = 0;
    length = sdp_build_request_header(&session->sdptext,
                                      client->_send_build, client->clientname, RTSP_METHOD_TEARDOWN, client->url, session->cseq);
    if (session->session)
        length += sdp_build_sessionID(&session->sdptext,
                                      client->_send_build + length, session->session);
    if (length)
    {
        client->method = RTSP_METHOD_TEARDOWN;
        client->_send_length = length;
        return rtsp_client_request_and_wait_respone(client, session, client->timeout);
    }
    return 0;
}

static int rtsp_client_request_play(rtsp_client_t *client, rtsp_session_t *session)
{
    int length = 0;
    length = sdp_build_request_header(&session->sdptext,
                                      client->_send_build, client->clientname, RTSP_METHOD_PLAY, client->url, session->cseq);
    if (session->session)
        length += sdp_build_sessionID(&session->sdptext,
                                      client->_send_build + length, session->session);
    if (length)
    {
        client->method = RTSP_METHOD_PLAY;
        client->_send_length = length;
        return rtsp_client_request_and_wait_respone(client, session, client->timeout);
    }
    return 0;
}

static int rtsp_client_request_pause(rtsp_client_t *client, rtsp_session_t *session)
{
    int length = 0;
    length = sdp_build_request_header(&session->sdptext,
                                      client->_send_build, client->clientname, RTSP_METHOD_PAUSE, client->url, session->cseq);
    if (session->session)
        length += sdp_build_sessionID(&session->sdptext,
                                      client->_send_build + length, session->session);
    if (length)
    {
        client->method = RTSP_METHOD_PAUSE;
        client->_send_length = length;
        return rtsp_client_request_and_wait_respone(client, session, client->timeout);
    }
    return 0;
}

static int rtsp_client_request_scale(rtsp_client_t *client, rtsp_session_t *session)
{
    int length = 0;
    length = sdp_build_request_header(&session->sdptext,
                                      client->_send_build, client->clientname, RTSP_METHOD_SCALE, client->url, session->cseq);
    if (session->session)
        length += sdp_build_sessionID(&session->sdptext,
                                      client->_send_build + length, session->session);
    if (length)
    {
        client->method = RTSP_METHOD_SCALE;
        client->_send_length = length;
        return rtsp_client_request_and_wait_respone(client, session, client->timeout);
    }
    return 0;
}

static int rtsp_client_request_set_parameter(rtsp_client_t *client, rtsp_session_t *session)
{
    int length = 0;
    length = sdp_build_request_header(&session->sdptext,
                                      client->_send_build, client->clientname, RTSP_METHOD_SET_PARAMETER, client->url, session->cseq);
    if (session->session)
        length += sdp_build_sessionID(&session->sdptext,
                                      client->_send_build + length, session->session);
    if (length)
    {
        client->method = RTSP_METHOD_SET_PARAMETER;
        client->_send_length = length;
        return rtsp_client_request_and_wait_respone(client, session, client->timeout);
    }
    return 0;
}

static int rtsp_client_request_get_parameter(rtsp_client_t *client, rtsp_session_t *session)
{
    int length = 0;
    length = sdp_build_request_header(&session->sdptext,
                                      client->_send_build, client->clientname, RTSP_METHOD_GET_PARAMETER, client->url, session->cseq);
    if (session->session)
        length += sdp_build_sessionID(&session->sdptext,
                                      client->_send_build + length, session->session);
    if (length)
    {
        client->method = RTSP_METHOD_GET_PARAMETER;
        client->_send_length = length;
        return rtsp_client_request_and_wait_respone(client, session, client->timeout);
    }
    return 0;
}

int rtsp_client_request(rtsp_client_t *client, rtsp_method method)
{
    int ret = 0;
    memset(client->_send_buf, 0, sizeof(client->_send_buf));
    client->method = method;
    switch (client->method)
    {
    case RTSP_METHOD_OPTIONS:
        ret = rtsp_client_request_option(client, client->rtsp_session);
        fprintf(stdout, "===============:%s -> RTSP_METHOD_OPTIONS(%d)\r\n", __func__, ret);
        break;
    case RTSP_METHOD_DESCRIBE:
        ret = rtsp_client_request_describe(client, client->rtsp_session);
        fprintf(stdout, "===============:%s -> RTSP_METHOD_DESCRIBE(%d)\r\n", __func__, ret);
        break;
    case RTSP_METHOD_SETUP:
        ret = rtsp_client_request_setup(client, client->rtsp_session);
        fprintf(stdout, "===============:%s -> RTSP_METHOD_SETUP(%d)\r\n", __func__, ret);
        break;
    case RTSP_METHOD_TEARDOWN:
        ret = rtsp_client_request_teardown(client, client->rtsp_session);
        fprintf(stdout, "===============:%s -> RTSP_METHOD_TEARDOWN(%d)\r\n", __func__, ret);
        break;
    case RTSP_METHOD_PLAY:
        ret = rtsp_client_request_play(client, client->rtsp_session);
        fprintf(stdout, "===============:%s -> RTSP_METHOD_PLAY(%d)\r\n", __func__, ret);
        break;
    case RTSP_METHOD_PAUSE:
        ret = rtsp_client_request_pause(client, client->rtsp_session);
        fprintf(stdout, "===============:%s -> RTSP_METHOD_PAUSE(%d)\r\n", __func__, ret);
        break;
    case RTSP_METHOD_SCALE:
        ret = rtsp_client_request_scale(client, client->rtsp_session);
        fprintf(stdout, "===============:%s -> RTSP_METHOD_SCALE(%d)\r\n", __func__, ret);
        break;
    case RTSP_METHOD_GET_PARAMETER:
        ret = rtsp_client_request_get_parameter(client, client->rtsp_session);
        fprintf(stdout, "===============:%s -> RTSP_METHOD_GET_PARAMETER(%d)\r\n", __func__, ret);
        break;
    case RTSP_METHOD_SET_PARAMETER:
        ret = rtsp_client_request_set_parameter(client, client->rtsp_session);
        fprintf(stdout, "===============:%s -> RTSP_METHOD_SET_PARAMETER(%d)\r\n", __func__, ret);
        break;
    default:
    {
        fprintf(stdout, "===============:%s -> default\r\n", __func__);
    }
    break;
    }
    if (ret != RTSP_STATE_CODE_200)
        client->rtsp_session->state = RTSP_SESSION_STATE_CLOSE;
    else
        client->rtsp_session->cseq++;
    return ret;
}

int rtsp_client_open(rtsp_client_t *client, zpl_client_media_data_callback cb)
{
    int ret = 0;
    client->client_rtpdata_callback = cb;
    ret = rtsp_client_request(client, RTSP_METHOD_OPTIONS);
    if (ret != RTSP_STATE_CODE_200)
    {
        client->rtsp_session->state = RTSP_SESSION_STATE_CLOSE;
        rtsp_client_destroy(client);
        return -1;
    }
    ret = rtsp_client_request(client, RTSP_METHOD_DESCRIBE);
    if (ret != RTSP_STATE_CODE_200)
    {
        client->rtsp_session->state = RTSP_SESSION_STATE_CLOSE;
        rtsp_client_destroy(client);
        return -1;
    }
    ret = rtsp_client_request(client, RTSP_METHOD_SETUP);
    if (ret != RTSP_STATE_CODE_200)
    {
        client->rtsp_session->state = RTSP_SESSION_STATE_CLOSE;
        rtsp_client_destroy(client);
        return -1;
    }

    ret = rtsp_client_request(client, RTSP_METHOD_PLAY);
    if (ret != RTSP_STATE_CODE_200)
    {
        client->rtsp_session->state = RTSP_SESSION_STATE_CLOSE;
        rtsp_client_destroy(client);
        return -1;
    }
    return ret;
}

int rtsp_client_close(rtsp_client_t *client)
{
    int ret = rtsp_client_request(client, RTSP_METHOD_TEARDOWN);
    if (ret != 0)
    {
        return -1;
    }
    return ret;
}

static int rtsp_client_rtpread(rtsp_client_t *client)
{
    int flag = 0;
    rtsp_session_t *rtsp_session = client->rtsp_session;
    if (rtsp_session && rtsp_rtp_select(rtsp_session))
    {
        if (rtsp_session->video_session.rtp_session && rtsp_session->video_session.rtp_state == RTP_SESSION_STATE_START)
        {
            flag = 1;
            if (rtsp_session->session_set &&
                session_set_is_set(((SessionSet *)rtsp_session->session_set), (RtpSession *)rtsp_session->video_session.rtp_session))
            {
                client->video_bufdata.skb_len = 0;
                rtsp_media_rtp_recv(rtsp_session, true, &client->video_bufdata);
                session_set_clr(((SessionSet *)rtsp_session->session_set), (RtpSession *)rtsp_session->video_session.rtp_session);
            }
        }

        if (rtsp_session->audio_session.rtp_session && rtsp_session->audio_session.rtp_state == RTP_SESSION_STATE_START)
        {
            flag = 1;
            if (rtsp_session->session_set &&
                session_set_is_set(((SessionSet *)rtsp_session->session_set), (RtpSession *)rtsp_session->audio_session.rtp_session))
            {
                client->audio_bufdata.skb_len = 0;
                session_set_clr(((SessionSet *)rtsp_session->session_set), (RtpSession *)rtsp_session->video_session.rtp_session);
                rtsp_media_rtp_recv(rtsp_session, false, &client->audio_bufdata);
            }
        }
        if (flag == 0)
        {
#if defined(_WIN32)
            Sleep(10);
#else
            usleep(10000);
#endif
        }
    }
    return -1;
}

int rtsp_client_thread(rtsp_client_t *client)
{
    if (client->istcp)
        rtsp_client_tcpthread(client);
    else
    {
        zpl_skbuffer_t *skbdata = NULL;
        int ret = rtsp_client_rtpread(client);
        if (ret && client->client_rtpdata_callback)
        {
            skbdata = &client->video_bufdata;
            if (ZPL_SKB_DATA_LEN(skbdata))
                (client->client_rtpdata_callback)(client, ZPL_SKB_DATA(skbdata), ZPL_SKB_DATA_LEN(skbdata));

            skbdata = &client->audio_bufdata;
            if (ZPL_SKB_DATA_LEN(skbdata))
                (client->client_rtpdata_callback)(client, ZPL_SKB_DATA(skbdata), ZPL_SKB_DATA_LEN(skbdata));
        }
    }
    return 1;
}
