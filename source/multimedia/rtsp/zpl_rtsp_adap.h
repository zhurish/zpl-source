#ifndef __RTSP_MEDIA_ADAP_H__
#define __RTSP_MEDIA_ADAP_H__

#ifdef __cplusplus
extern "C" {
#endif




typedef struct rtsp_media_adap_s
{
    char                *name;
    uint32_t            ptid;
    int (*_build_sdp)(rtsp_session_t*, uint8_t *, uint32_t);
    int (*_parse_sdp)(rtsp_session_t*, const uint8_t *, uint32_t);
    int (*_rtp_packet)(void *, const uint8_t *, uint32_t);
    int (*_rtp_unpacket)(void *, uint8_t *, uint32_t);
    int (*_rtp_sendto)(rtsp_session_t*, int, const uint8_t *, uint32_t);
    int (*_rtp_recv)(rtsp_session_t*, int, uint8_t *, uint32_t);
}rtsp_media_adap_t;

RTSP_API int rtsp_media_adap_build_sdp(uint32_t ptid, rtsp_session_t *session, uint8_t *buf, uint32_t len);
RTSP_API int rtsp_media_adap_parse_sdp(uint32_t ptid, rtsp_session_t *session, const uint8_t *buf, uint32_t len);

RTSP_API int rtsp_media_adap_rtp_packet(uint32_t ptid, void *packet, const uint8_t *buf, uint32_t len);
RTSP_API int rtsp_media_adap_rtp_unpacket(uint32_t ptid, void *packet,  uint8_t *buf, uint32_t len);

RTSP_API int rtsp_media_adap_rtp_sendto(uint32_t ptid, rtsp_session_t *session, int type, const uint8_t *buf, uint32_t len);
RTSP_API int rtsp_media_adap_rtp_recv(uint32_t ptid, rtsp_session_t *session, int type, uint8_t *buf, uint32_t len);


#ifdef __cplusplus
}
#endif

#endif /* __RTSP_MEDIA_ADAP_H__ */
