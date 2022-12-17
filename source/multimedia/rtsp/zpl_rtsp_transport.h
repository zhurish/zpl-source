/*
 * Copyright © 2001-2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __RTSP_TRANSPORT_H__
#define __RTSP_TRANSPORT_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "zpl_rtsp_def.h"
#include "zpl_rtsp.h"


// Transport: RTP/AVP/TCP;interleaved=0-1
// Transport: RTP/AVP;unicast;client_port=4588-4589;server_port=6256-6257
// Transport: RTP/AVP;multicast;destination=224.2.0.1;port=3456-3457;ttl=16
typedef struct rtsp_transport_s
{
    rtsp_transport_rtp_t     proto;      // RTSP_TRANSPORT_xxx
    rtsp_transport_type_t        type;     // 0-unicast/1-multicast, default multicast
    char        *destination;   // IPv4/IPv6
    char        *source;        // IPv4/IPv6
    uint8_t     layer;          // rtsp setup response only
    uint8_t     mode;           // PLAY/RECORD, default PLAY, rtsp setup response only
    uint8_t     append;         // use with RECORD mode only, rtsp setup response only
    uint8_t     rtp_interleaved;
    uint8_t     rtcp_interleaved;   // rtsp setup response only
    union
    {
        struct rtsp_multicast_t
        {
            uint8_t     ttl;        // multicast only
            uint16_t    rtp_port;
            uint16_t    rtcp_port;  // multicast only
        } multicast;

        struct rtsp_unicast_t
        {
            uint16_t    rtp_port;
            uint16_t    rtcp_port;  // unicast RTP/RTCP port pair, RTP only
            uint16_t    local_rtp_port;
            uint16_t    local_rtcp_port; // unicast RTP/RTCP port pair, RTP only
            uint32_t    ssrc;       // RTP only(synchronization source (SSRC) identifier) 4-bytes
        } unicast;
    } rtp;
}rtsp_transport_t;

/// parse RTSP Transport header
/// @return 0-ok, other-error
/// usage 1:
/// struct rtsp_header_transport_t transport;
/// const char* header = "Transport: RTP/AVP;unicast;client_port=4588-4589;server_port=6256-6257";
/// r = rtsp_header_transport("RTP/AVP;unicast;client_port=4588-4589;server_port=6256-6257", &transport);
/// check(r)
///
/// usage 2:
/// const char* header = "Transport: RTP/AVP;unicast;client_port=4588-4589;server_port=6256-6257,RTP/AVP;unicast;client_port=5000-5001;server_port=6000-6001";
/// split(header, ',');
/// r1 = rtsp_header_transport("RTP/AVP;unicast;client_port=4588-4589;server_port=6256-6257", &transport);
/// r2 = rtsp_header_transport("RTP/AVP;unicast;client_port=5000-5001;server_port=6000-6001", &transport);
/// check(r1, r2)
RTSP_API int rtsp_header_transport(zpl_bool srv, const char* field, rtsp_transport_t* transport);
RTSP_API int rtsp_header_transport_destroy(rtsp_transport_t* t);

#ifdef __cplusplus
}
#endif

#endif /* __RTSP_TRANSPORT_H__ */
