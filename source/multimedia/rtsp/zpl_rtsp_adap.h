#ifndef __RTSP_MEDIA_ADAP_H__
#define __RTSP_MEDIA_ADAP_H__

#ifdef __cplusplus
extern "C" {
#endif




typedef struct rtsp_session_media_adap_s
{
    char                *name;
    uint32_t            ptid;
    int (*_rtp_sendto)(void *, const uint8_t *, uint32_t, int user_ts);
    int (*_rtp_recv)(void *, uint8_t *, int, uint32_t, int * );
}rtsp_session_media_adap_t;


RTSP_API int rtsp_session_media_adap_rtp_sendto(uint32_t ptid, rtsp_session_t *session, int type, const uint8_t *buf, uint32_t len);
RTSP_API int rtsp_session_media_adap_rtp_recv(uint32_t ptid, rtsp_session_t *session, int type, uint8_t *buf, uint32_t len, int *havemore);


#ifdef __cplusplus
}
#endif

#endif /* __RTSP_MEDIA_ADAP_H__ */
