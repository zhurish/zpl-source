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
    H264_PACKETIZER_MODE_SINGLE_NAL,
    
    /**
     * Non-interleaved packetization mode will generate payloads with the
     * following possible formats:
     * - single NAL unit packets,
     * - NAL units aggregation STAP-A packets,
     * - fragmented NAL unit FU-A packets.
     */
    H264_PACKETIZER_MODE_NON_INTERLEAVED,

    /**
     * Interleaved packetization mode will generate payloads with the
     * following possible formats:
     * - single NAL unit packets,
     * - NAL units aggregation STAP-A & STAP-B packets,
     * - fragmented NAL unit FU-A & FU-B packets.
     * This packetization mode is currently unsupported.
     */
    H264_PACKETIZER_MODE_INTERLEAVED,

    /**
     * H.263 RTP packetization using RFC 4629.
     */
    H263_PACKETIZER_MODE_RFC4629 = 0,

    /**
     * H.263 RTP packetization using legacy RFC 2190.
     * This is currently not supported.
     */
    H263_PACKETIZER_MODE_RFC2190,

} h264_packetizer_mode;


/**
 * H.264 packetizer setting.
 */
typedef struct h264_packetizer_cfg
{
    /**
     * Maximum payload length.
     * Default: PJMEDIA_MAX_MTU
     */
    int mtu;

    /**
     * Packetization mode.
     * Default: PJMEDIA_H264_PACKETIZER_MODE_NON_INTERLEAVED
     */
    h264_packetizer_mode mode;

    /**
     * NAL start code size used for unpacketizing.
     * Valid options are 3 (0, 0, 1) or 4 (0, 0, 0, 1).
     * Default: 3 (0, 0, 1)
     */
    unsigned unpack_nal_start;
}h264_packetizer_cfg;

/* H.264 packetizer definition */
typedef struct h264_packetizer
{
    /* Current settings */
    h264_packetizer_cfg cfg;
    
    /* Unpacketizer state */
    unsigned        unpack_last_sync_pos;
    int             unpack_prev_lost;
    u_int8_t        *enc_frame_whole;
    unsigned        enc_frame_size;
    unsigned        enc_processed;    
}h264_packetizer;


/**
 * Create H.264 packetizer.
 *
 * @param pool          The memory pool.
 * @param cfg           Packetizer settings, if NULL, default setting
 *                      will be used.
 * @param p_pktz        Pointer to receive the packetizer.
 *
 * @return              PJ_SUCCESS on success.
 */
int rtp_h264_packetizer_create(h264_packetizer_cfg *cfg,
                                    h264_packetizer **p_pktz);


/**
 * Generate an RTP payload from a H.264 picture bitstream. Note that this
 * function will apply in-place processing, so the bitstream may be modified
 * during the packetization.
 *
 * @param pktz          The packetizer.
 * @param bits          The picture bitstream to be packetized.
 * @param bits_len      The length of the bitstream.
 * @param bits_pos      The bitstream offset to be packetized.
 * @param payload       The output payload.
 * @param payload_len   The output payload length.
 *
 * @return              PJ_SUCCESS on success.
 */
int rtp_h264_packetize(h264_packetizer *pktz,
                                            u_int8_t *bits,
                                            int bits_len,
                                            unsigned *bits_pos,
                                            const u_int8_t **payload,
                                            int *payload_len);


/**
 * Append an RTP payload to an H.264 picture bitstream. Note that in case of
 * noticing packet lost, application should keep calling this function with
 * payload pointer set to NULL, as the packetizer need to update its internal
 * state.
 *
 * @param pktz          The packetizer.
 * @param payload       The payload to be unpacketized.
 * @param payload_len   The payload length.
 * @param bits          The bitstream buffer.
 * @param bits_len      The bitstream buffer size.
 * @param bits_pos      The bitstream offset to put the unpacketized payload
 *                      in the bitstream, upon return, this will be updated
 *                      to the latest offset as a result of the unpacketized
 *                      payload.
 *
 * @return              PJ_SUCCESS on success.
 */
int rtp_h264_unpacketize(h264_packetizer *pktz,
                                              const u_int8_t *payload,
                                              int   payload_len,
                                              u_int8_t *bits,
                                              int   bits_len,
                                              unsigned   *bits_pos);



/**
 * Create H.263 packetizer.
 *
 * @param pool          The memory pool.
 * @param cfg           Packetizer settings, if NULL, default setting
 *                      will be used.
 * @param p_pktz        Pointer to receive the packetizer.
 *
 * @return              PJ_SUCCESS on success.
 */
int rtp_h263_packetizer_create(const h264_packetizer_cfg *cfg,
                                    h264_packetizer **p_pktz);


/**
 * Generate an RTP payload from a H.263 picture bitstream. Note that this
 * function will apply in-place processing, so the bitstream may be modified
 * during the packetization.
 *
 * @param pktz          The packetizer.
 * @param bits          The picture bitstream to be packetized.
 * @param bits_len      The length of the bitstream.
 * @param bits_pos      The bitstream offset to be packetized.
 * @param payload       The output payload.
 * @param payload_len   The output payload length.
 *
 * @return              PJ_SUCCESS on success.
 */
int rtp_h263_packetize(h264_packetizer *pktz,
                                            u_int8_t *bits,
                                            int bits_len,
                                            unsigned *bits_pos,
                                            const u_int8_t **payload,
                                            int *payload_len);


/**
 * Append an RTP payload to an H.263 picture bitstream. Note that in case of
 * noticing packet lost, application should keep calling this function with
 * payload pointer set to NULL, as the packetizer need to update its internal
 * state.
 *
 * @param pktz          The packetizer.
 * @param payload       The payload to be unpacketized.
 * @param payload_len   The payload length.
 * @param bits          The bitstream buffer.
 * @param bits_size     The bitstream buffer size.
 * @param bits_pos      The bitstream offset to put the unpacketized payload
 *                      in the bitstream, upon return, this will be updated
 *                      to the latest offset as a result of the unpacketized
 *                      payload.
 *
 * @return              PJ_SUCCESS on success.
 */
int rtp_h263_unpacketize(h264_packetizer *pktz,
                                              const u_int8_t *payload,
                                              int payload_len,
                                              u_int8_t *bits,
                                              int bits_size,
                                              unsigned *bits_pos);
/*
 * 发送一帧数据
 */
int rtp_payload_send_h264(void *session, const u_int8_t *buffer, u_int32_t len, int user_ts);


#ifdef __cplusplus
}
#endif


#endif // __RTP_H264_H__
