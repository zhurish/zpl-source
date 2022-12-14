#ifndef ZPL_RTP_H265_H
#define ZPL_RTP_H265_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "zpl_rtsp_def.h"
#include "zpl_rtsp_session.h"
#ifdef ZPL_LIBRTSP_MODULE
#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "zpl_media_extradata.h"
#endif
#include "zpl_rtsp_media.h"


RTSP_BEGIN_DECLS

enum
{
    NAL5_TYPE_STAP_A = 48,//单一时间的组合包
    NAL5_TYPE_FU_A = 49,//分片的单元
};

#pragma pack(1)
typedef struct
{
#ifdef ORTP_BIGENDIAN
    uint16_t FU_F:1;
    uint16_t Type:6;
    uint16_t LayerId:6;
    uint16_t TID:3;

    uint8_t FU_HDR_S:1;
    uint8_t FU_HDR_E:1;
    uint8_t FU_Type:6;
#else
    uint16_t TID:3;
    uint16_t LayerId:6;
    uint16_t Type:6;
    uint16_t FU_F:1;

    uint8_t FU_Type:6;
    uint8_t FU_HDR_E:1;
    uint8_t FU_HDR_S:1;
#endif
} H265_NALU_FUHDR;
#pragma pack(0)

#define H265_NALU_TYPE(n)       ((((n)>>1) & 0x3F))
#define H265_FUHDR_TYPE         49
#define H265_HDR_LEN         sizeof(H265_NALU_FUHDR)

/*
 * 读取一帧数据
 */
RTSP_API int _h265_file_get_frame(void *file, void *p);
/*
 * 多个nalu组成一个包
 */
RTSP_API int _rtp_packet_h265(zpl_skbuffer_t *nalu, uint8_t *out);
/*
 * 发送一帧数据
 */
RTSP_API int _rtp_send_h265(rtsp_session_t *session, int type, const uint8_t *buffer, uint32_t len);
RTSP_API int _rtp_unpacket_h265(zpl_skbuffer_t *packet, uint8_t *payload, size_t payload_len);
/*
 * 封装H265媒体的SDP
 */
RTSP_API int _rtsp_build_sdp_h265(rtsp_session_t *session, uint8_t *src, uint32_t len);
/*
 * 解封装H265特殊的SDP
 */
RTSP_API int _rtsp_parse_sdp_h265(rtsp_session_t *session, uint8_t *src, uint32_t len);




RTSP_END_DECLS


#endif // ZPL_RTP_H265_H
