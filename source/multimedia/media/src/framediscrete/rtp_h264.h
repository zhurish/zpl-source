#ifndef __RTP_H264_H__
#define __RTP_H264_H__
#ifdef __cplusplus
extern "C" {
#endif



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

#define NALU_START_FLAG     0X00200000
#define NALU_END_FLAG       0X00100000
#define NALU_LEN_GET(n)     0X0000ffff


#pragma pack(1)
typedef struct
{
#ifdef ORTP_BIGENDIAN
    u_int8_t FU_F:1;
    u_int8_t FU_NRI:2;
    u_int8_t FU_Type:5;

    u_int8_t FU_HDR_S:1;
    u_int8_t FU_HDR_E:1;
    u_int8_t FU_HDR_R:1;
    u_int8_t FU_HDR_Type:5;
#else
    u_int8_t FU_Type:5;
    u_int8_t FU_NRI:2;
    u_int8_t FU_F:1;

    u_int8_t FU_HDR_Type:5;
    u_int8_t FU_HDR_R:1;
    u_int8_t FU_HDR_E:1;
    u_int8_t FU_HDR_S:1;
#endif
} H264_NALU_FUHDR;
#pragma pack(0)

#define H264_NALU_F(n)          (((n) & 0x80) >> 7)
#define H264_NALU_NRI(n)        (((n) & 0x60) >> 5)
#define H264_NALU_TYPE(n)       (((n) & 0x1F))

typedef struct
{
    u_int8_t hdr_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
    u_int8_t forbidden_bit;            //! should be always FALSE
    u_int8_t nal_idc;        //! NALU_PRIORITY_xxxx
    u_int8_t nal_unit_type;            //! NALU_TYPE_xxxx                  //! contains the first byte followed by the EBSP
    u_int32_t len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
    u_int8_t *buf;
}__attribute__ ((packed)) RTP_H264_NALU_T ;

bool rtp_payload_h264_is_nalu3_start(u_int8_t *buffer);
bool rtp_payload_h264_is_nalu4_start(u_int8_t *buffer);
int rtp_payload_h264_get_nextnalu(u_int8_t *bufdata, u_int32_t len);
bool rtp_payload_h264_isnaluhdr(u_int8_t *bufdata, RTP_H264_NALU_T *nalu);
/*
 * 发送一帧数据
 */
int rtp_payload_send_h264(void *session, const u_int8_t *buffer, u_int32_t len, int user_ts);


#ifdef __cplusplus
}
#endif


#endif // __RTP_H264_H__
