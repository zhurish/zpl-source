#ifndef ZPL_RTP_G7XX_H
#define ZPL_RTP_G7XX_H
#ifdef __cplusplus
extern "C" {
#endif



/*
 * 读取一帧数据
 */
RTSP_API int _g7xx_get_frame(void *file, void *p);
/*
 * 多个nalu组成一个包
 */
RTSP_API int _g7xx_data_packet(zpl_skbuffer_t *nalu, uint8_t *out);
/*
 * 发送一帧数据
 */
RTSP_API int _rtp_send_g7xx(rtsp_session_t *session, int type, const uint8_t *buffer, uint32_t len);
RTSP_API int _rtp_unpacket_g7xx(zpl_skbuffer_t *packet, uint8_t *payload, size_t payload_len);
/*
 * 封装g7xx媒体的SDP
 */
RTSP_API int _rtsp_build_sdp_g7xx(rtsp_session_t *session, uint8_t *src, uint32_t len);
/*
 * 解封装g7xx特殊的SDP
 */
RTSP_API int _rtsp_parse_sdp_g7xx(rtsp_session_t *session, uint8_t *src, uint32_t len);


#ifdef __cplusplus
}
#endif


#endif // ZPL_RTP_G7XX_H
