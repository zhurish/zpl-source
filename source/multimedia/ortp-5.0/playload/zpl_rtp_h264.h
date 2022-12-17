#ifndef ZPL_RTP_H264_H
#define ZPL_RTP_H264_H
#ifdef __cplusplus
extern "C" {
#endif
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



/**
 * Enumeration of H.264 packetization modes.
 */
typedef enum
{
    /**
     * Single NAL unit packetization mode will only generate payloads
     * containing a complete single NAL unit packet. As H.264 NAL unit
     * size can be very large, this mode is usually not applicable for
     * network environments with MTU size limitation.
     */
    RTP_H264_PACKETIZER_MODE_SINGLE_NAL,//必须使用单一 NALU 单元模式

    /**
     * Non-interleaved packetization mode will generate payloads with the
     * following possible formats:
     * - single NAL unit packets,
     * - NAL units aggregation STAP-A packets,
     * - fragmented NAL unit FU-A packets.
     */
    RTP_H264_PACKETIZER_MODE_NON_INTERLEAVED,//用非交错(non-interleaved)封包模式

    /**
     * Interleaved packetization mode will generate payloads with the
     * following possible formats:
     * - single NAL unit packets,
     * - NAL units aggregation STAP-A & STAP-B packets,
     * - fragmented NAL unit FU-A & FU-B packets.
     * This packetization mode is currently unsupported.
     */
    RTP_H264_PACKETIZER_MODE_INTERLEAVED,//用交错(interleaved)封包模式
} H264_PACKETIZER_MODE;


/* Enumeration of H.264 NAL unit types */
enum
{
    NAL_TYPE_SINGLE_NAL_MIN = 1,
    NAL_TYPE_SINGLE_NAL_MAX = 23,
    NAL_TYPE_STAP_A = 24,//单一时间的组合包
    NAL_TYPE_STAP_B = 25,//单一时间的组合包
    NAL_TYPE_MTAP_16 = 26,//多个时间的组合包
    NAL_TYPE_MTAP_24 = 27,//多个时间的组合包
    NAL_TYPE_FU_A = 28,//分片的单元
    NAL_TYPE_FU_B = 29,//分片的单元
};
enum
{
    HEADER_SIZE_FU_A = 2,
    HEADER_SIZE_STAP_A = 3,
};

#define NALU_MAX_FLAG       0X00400000
#define NALU_START_FLAG     0X00200000
#define NALU_END_FLAG       0X00100000
#define NALU_LEN_GET(n)     0X0000ffff

#ifndef ZPL_LIBRTSP_MODULE

typedef enum {
    NALU_TYPE_SLICE = 1,
    NALU_TYPE_DPA = 2,
    NALU_TYPE_DPB = 3,
    NALU_TYPE_DPC = 4,
    NALU_TYPE_IDR = 5,
    NALU_TYPE_SEI = 6,
    NALU_TYPE_SPS = 7,
    NALU_TYPE_PPS = 8,
    NALU_TYPE_AUD = 9,
    NALU_TYPE_EOSEQ = 10,
    NALU_TYPE_EOSTREAM = 11,
    NALU_TYPE_FILL = 12,
} H264_NaluType;

typedef enum {
    NALU_PRIORITY_DISPOSABLE = 0,
    NALU_PRIORITY_LOW        = 1,
    NALU_PRIORITY_HIGH       = 2,
    NALU_PRIORITY_HIGHEST    = 3
} H264_NaluPriority;

typedef struct
{
    uint8_t hdr_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
    uint8_t forbidden_bit;            //! should be always FALSE
    uint8_t nal_idc;        //! NALU_PRIORITY_xxxx
    uint8_t nal_unit_type;            //! NALU_TYPE_xxxx                  //! contains the first byte followed by the EBSP
    uint32_t len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
} H264_NALU_T;

typedef struct 
{
    uint8_t* 			fSEI;
    unsigned 			fSEISize;
    uint8_t* 			fVPS;
    unsigned 			fVPSSize;
    uint8_t* 			fSPS;
    unsigned 			fSPSSize;
    uint8_t* 			fPPS;
    unsigned 			fPPSSize;
    uint32_t 			profileLevelId;
}zpl_video_extradata_t;

#endif

#pragma pack(1)
typedef struct
{
#ifdef ORTP_BIGENDIAN
    uint8_t FU_F:1;
    uint8_t FU_NRI:2;
    uint8_t FU_Type:5;

    uint8_t FU_HDR_S:1;
    uint8_t FU_HDR_E:1;
    uint8_t FU_HDR_R:1;
    uint8_t FU_HDR_Type:5;
#else
    uint8_t FU_Type:5;
    uint8_t FU_NRI:2;
    uint8_t FU_F:1;

    uint8_t FU_HDR_Type:5;
    uint8_t FU_HDR_R:1;
    uint8_t FU_HDR_E:1;
    uint8_t FU_HDR_S:1;
#endif
} H264_NALU_FUHDR;
#pragma pack(0)

#define H264_NALU_F(n)          (((n) & 0x80) >> 7)
#define H264_NALU_NRI(n)        (((n) & 0x60) >> 5)
#define H264_NALU_TYPE(n)       (((n) & 0x1F))







/**
 * H.264 packetizer setting.
 */
typedef struct rtp_h264_cfg
{
    bool        bh264;
    H264_PACKETIZER_MODE mode;
} rtp_h264_cfg;


/*
 * 读取一帧数据
 */
RTSP_API int _h264_file_get_frame(void *file, void *p);
/*
 * 多个nalu组成一个包
 */
RTSP_API int _rtp_packet_h264(zpl_skbuffer_t *nalu, uint8_t *out);
/*
 * 发送一帧数据
 */
RTSP_API int _rtp_send_h264(rtsp_session_t *session, int type, const uint8_t *buffer, uint32_t len);
RTSP_API int _rtp_unpacket_h264(zpl_skbuffer_t *packet, uint8_t *payload, size_t payload_len);
/*
 * 封装H264媒体的SDP
 */
RTSP_API int _rtsp_build_sdp_h264(rtsp_session_t *session, uint8_t *src, uint32_t len);
/*
 * 解封装H264特殊的SDP
 */
RTSP_API int _rtsp_parse_sdp_h264(rtsp_session_t *session, uint8_t *src, uint32_t len);


//RTSP_API int rtp_build_sdp_pcm(rtsp_session_t *session, uint8_t *src, uint32_t len);

RTSP_API int rtp_send_h264_test(void);

#ifdef __cplusplus
}
#endif


#endif // ZPL_RTP_H264_H
