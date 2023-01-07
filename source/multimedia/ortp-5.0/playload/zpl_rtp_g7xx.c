#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


#ifdef ZPL_LIBRTSP_MODULE


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
#include "zpl_rtp_g7xx.h"

/*
int sdp_g711u(uint8_t *data, int bytes, const char* proto, unsigned short port)
{
    static const char* pattern = "m=audio %hu %s 0\n";
    return snprintf((char*)data, bytes, pattern, port, proto && *proto ? proto : "RTP/AVP");
}

int sdp_g711a(uint8_t *data, int bytes, const char* proto, unsigned short port)
{
    static const char* pattern = "m=audio %hu %s 8\n";
    return snprintf((char*)data, bytes, pattern, port, proto && *proto ? proto : "RTP/AVP");
}
*/
/*
 * 封装g7xx媒体的SDP
 */
int _rtsp_build_sdp_g7xx(rtsp_session_t *session, uint8_t *src, uint32_t len)
{
    int sdplength = 0;
    switch (session->audio_session.payload)
    {
    case RTP_MEDIA_PAYLOAD_G722:
        if (rtp_profile_get_rtpmap(RTP_MEDIA_PAYLOAD_G722))
            sdplength += sprintf(src + sdplength, "a=rtpmap:%d %s\r\n", RTP_MEDIA_PAYLOAD_G722, rtp_profile_get_rtpmap(RTP_MEDIA_PAYLOAD_G722));
        else
            sdplength += sprintf(src + sdplength, "a=rtpmap:%d G722/8000\r\n", RTP_MEDIA_PAYLOAD_G722);
        break;

    case RTP_MEDIA_PAYLOAD_G711A:
        if (rtp_profile_get_rtpmap(RTP_MEDIA_PAYLOAD_G711A))
            sdplength += sprintf(src + sdplength, "a=rtpmap:%d %s\r\n", RTP_MEDIA_PAYLOAD_G711A, rtp_profile_get_rtpmap(RTP_MEDIA_PAYLOAD_G711A));
        else
            sdplength += sprintf(src + sdplength, "a=rtpmap:%d PCMA/8000\r\n", RTP_MEDIA_PAYLOAD_G711A);
        break;
    case RTP_MEDIA_PAYLOAD_G711U:
        if (rtp_profile_get_rtpmap(RTP_MEDIA_PAYLOAD_G711U))
            sdplength += sprintf(src + sdplength, "a=rtpmap:%d %s\r\n", RTP_MEDIA_PAYLOAD_G711U, rtp_profile_get_rtpmap(RTP_MEDIA_PAYLOAD_G711U));
        else
            sdplength += sprintf(src + sdplength, "a=rtpmap:%d PCMU/8000\r\n", RTP_MEDIA_PAYLOAD_G711U);
        break;

    }
    return sdplength;
}

/*
 * 读取一帧数据
 */
int _g7xx_get_frame(void *file, void *p)
{
    return OK;
}
/*
 * 多个nalu组成一个包
 */
int _g7xx_data_packet(zpl_skbuffer_t *nalu, uint8_t *out)
{
    return OK;
}
/*
 * 发送一帧数据
 */
int _rtp_send_g7xx(rtsp_session_t *session, int type, const uint8_t *buffer, uint32_t len)
{
    int ret = rtsp_rtp_send(session, false, buffer, len, 1);
    return ret;
}

int _rtp_unpacket_g7xx(zpl_skbuffer_t *packet, uint8_t *payload, size_t payload_len)
{
    return zpl_skbuffer_put(packet, payload, payload_len);
}


/*
 * 解封装g7xx特殊的SDP
 */
int _rtsp_parse_sdp_g7xx(rtsp_session_t *session, uint8_t *src, uint32_t len)
{
    return OK;
}
#endif