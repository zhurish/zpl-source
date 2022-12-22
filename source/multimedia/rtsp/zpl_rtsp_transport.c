/*
 * Copyright © 2001-2013 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "zpl_rtsp.h"
#include "zpl_rtsp_sdp.h"
#include "zpl_rtsp_transport.h"


#if defined(_WIN32) || defined(_WIN64) || defined(OS_WINDOWS)
#define strcasecmp _stricmp
#define strncasecmp	_strnicmp
#endif


#define TRANSPORT_SPECIAL ",;\r\n"

int rtsp_header_transport(zpl_bool srv, const char* field, rtsp_transport_t* t)
{
    const char* p1 = NULL;
    const char* p = field;
    int n = 0;

    uint16_t    s_port = 0;
    uint16_t    s_cport = 0;  // unicast RTP/RTCP port pair, RTP only
    uint16_t    c_port = 0;
    uint16_t    c_cport = 0; // unicast RTP/RTCP port pair, RTP only
    int tmpval[2] = {0,0};

    t->type = RTSP_TRANSPORT_UNICAST; // default unicast
    t->proto = RTSP_TRANSPORT_RTP_UDP;

    while(p && *p)
    {
        p1 = strpbrk(p, TRANSPORT_SPECIAL);
        n = p1 ? (int)(p1 - p) : strlen(p); // ptrdiff_t -> size_t

        switch(*p)
        {
        case 'r':
        case 'R':
            if(11 == n && 0 == strncasecmp("RTP/AVP/UDP", p, 11))
            {
                t->proto = RTSP_TRANSPORT_RTP_UDP;
            }
            else if(11 == n && 0 == strncasecmp("RTP/AVP/TCP", p, 11))
            {
                t->proto = RTSP_TRANSPORT_RTP_TCP;
            }
            else if(11 == n && 0 == strncasecmp("RAW/RAW/UDP", p, 11))
            {
                t->proto = RTSP_TRANSPORT_RTP_RAW;
            }
            else if(7 == n && 0 == strncasecmp("RTP/AVP", p, 7))
            {
                t->proto = RTSP_TRANSPORT_RTP_UDP;
            }
            break;

        case 'u':
        case 'U':
            if(7 == n && 0 == strncasecmp("unicast", p, 7))
            {
                t->type = RTSP_TRANSPORT_UNICAST;
            }
            break;

        case 'm':
        case 'M':
            if(9 == n && 0 == strncasecmp("multicast", p, 9))
            {
                t->type = RTSP_TRANSPORT_MULTICAST;
            }
            else if(n > 5 && 0 == strncasecmp("mode=", p, 5))
            {
                if( (11==n && 0 == strcasecmp("\"PLAY\"", p+5)) || (9==n && 0 == strcasecmp("PLAY", p+5)) )
                    t->mode = RTSP_TRANSPORT_PLAY;
                else if( (13==n && 0 == strcasecmp("\"RECORD\"", p+5)) || (11==n && 0 == strcasecmp("RECORD", p+5)) )
                    t->mode = RTSP_TRANSPORT_RECORD;
            }
            break;

        case 'd':
        case 'D':
            if(n >= 12 && 0 == strncasecmp("destination=", p, 12))
            {
                t->destination = sdp_scopy( (char*)(p+12), n - 12);
            }
            break;

        case 's':
        case 'S':
            if(n >= 7 && 0 == strncasecmp("source=", p, 7))
            {
                t->source = sdp_scopy( p+7, n - 7);
            }
            else if(13 == n && 0 == strncasecmp("ssrc=", p, 5))
            {
                // unicast only
                t->rtp.unicast.ssrc = (int)strtol(p+5, NULL, 16);
            }
            else if(2 == sscanf(p, "server_port=%hu-%hu", &s_port, &s_cport))
            {
            }
            else if(1 == sscanf(p, "server_port=%hu", &s_port))
            {
                s_port = s_port / 2 * 2; // RFC 3550 (p56)
                s_cport = s_port + 1;
            }
            break;

        case 'a':
            if(6 == n && 0 == strcasecmp("append", p))
            {
                t->append = 1;
            }
            break;

        case 'p':
            if(2 == sscanf(p, "port=%hu-%hu", &t->rtp.multicast.rtp_port, &t->rtp.multicast.rtcp_port))
            {
            }
            else if(1 == sscanf(p, "port=%hu", &t->rtp.multicast.rtp_port))
            {
                t->rtp.multicast.rtp_port = t->rtp.multicast.rtp_port / 2 * 2; // RFC 3550 (p56)
                t->rtp.multicast.rtcp_port = t->rtp.multicast.rtp_port + 1;
            }
            break;

        case 'c':
            if(2 == sscanf(p, "client_port=%hu-%hu", &c_port, &c_cport))
            {
                t->rtp.unicast.rtp_port = tmpval[0];
                t->rtp.unicast.rtcp_port = tmpval[1];
            }
            else if(1 == sscanf(p, "client_port=%hu", &c_port))
            {
                c_port = c_port / 2 * 2; // RFC 3550 (p56)
                c_cport = c_port + 1;
            }
            break;

        case 'i':
            if(2 == sscanf(p, "interleaved=%d-%d", &tmpval[0], &tmpval[1]))
            {
                t->rtp_interleaved = tmpval[0];
                t->rtcp_interleaved = tmpval[1];
            }
            else if(1 == sscanf(p, "interleaved=%d", &tmpval[0]))
            {
                t->rtp_interleaved = tmpval[0];
                t->rtcp_interleaved = t->rtp_interleaved + 1;
            }
            break;

        case 't':
            if(1 == sscanf(p, "ttl=%d", &tmpval[0]))
            {
                t->rtp.multicast.ttl = tmpval[0];
            }
            break;

        case 'l':
            if(1 == sscanf(p, "layers=%d", &tmpval[0]))
            {
                t->layer = tmpval[0];
            }
            break;
        }

        if(NULL == p1 || '\r' == *p1 || '\n' == *p1 || '\0' == *p1 || ',' == *p1)
            break;
        p = p1 + 1;
    }
    if(t->type != RTSP_TRANSPORT_MULTICAST)
    {
        if(srv)
        {
            if(s_port)
                t->rtp.unicast.local_rtp_port = s_port;
            if(s_cport)
                t->rtp.unicast.local_rtcp_port = s_cport;
            if(c_port)
                t->rtp.unicast.rtp_port = c_port;
            if(c_cport)
                t->rtp.unicast.rtcp_port = c_cport;
        }
        else
        {
            if(c_port)
                t->rtp.unicast.local_rtp_port = c_port;
            if(c_cport)
                t->rtp.unicast.local_rtcp_port = c_cport;
            if(s_port)
                t->rtp.unicast.rtp_port = s_port;
            if(s_cport)
                t->rtp.unicast.rtcp_port = s_cport;
        }
        fprintf(stdout,"==================================rtsp_header_transport========rtp-port=%d rtcp-port=%d\n",t->rtp.unicast.rtp_port, t->rtp.unicast.rtcp_port);
        fprintf(stdout,"==================================rtsp_header_transport===local=====rtp-port=%d rtcp-port=%d\n",t->rtp.unicast.local_rtp_port, t->rtp.unicast.local_rtcp_port);
        //fprintf(stdout,"==================================rtsp_header_transport========payload=%d\n",_rtpsession->payload);
        fflush(stdout);
    }
    return 0;
}



int rtsp_header_transport_destroy(rtsp_transport_t* t)
{
    if(t->destination)   // IPv4/IPv6
    {
        free(t->destination);
        t->destination = NULL;
    }
    if(t->source)   // IPv4/IPv6
    {
        free(t->source);
        t->source = NULL;
    }
    return 0;
}
