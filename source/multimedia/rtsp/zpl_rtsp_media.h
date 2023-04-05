#ifndef __RTSP_MEDIA_H__
#define __RTSP_MEDIA_H__
#ifdef __cplusplus
extern "C" {
#endif


#ifdef ZPL_BUILD_LINUX
#ifndef RTP_MEDIA_BASE_PATH
#define     RTP_MEDIA_BASE_PATH     "/home/zhurish/workspace/qt-project/live555-test/"
#define     RTP_MEDIA_BASE_PATH     "/mnt/hgfs/ubuntu-share/qt-project/live555-test/"
#endif
#else
#ifndef RTP_MEDIA_BASE_PATH
#define     RTP_MEDIA_BASE_PATH     "D:/qt-project/live555-test/"
#endif
#endif


#define  RTP_MEDIA_FRAME_DELAY(n)   ((n)/3)

RTSP_API char *rtsp_session_media_name(int channel, int level);


RTSP_API zpl_bool rtsp_session_media_lookup(rtsp_session_t * session, int channel, int level, const char *path);
RTSP_API int rtsp_session_media_destroy(rtsp_session_t *session);


RTSP_API int rtsp_session_media_start(rtsp_session_t* session, zpl_bool start);
RTSP_API int rtsp_session_media_build_sdptext(rtsp_session_t * session, char *sdp);


RTSP_API int rtsp_session_media_tcp_forward(rtsp_session_t* session, const uint8_t *buffer, uint32_t len);



RTSP_API rtsp_code rtsp_session_media_describe(rtsp_session_t * session, void *pUser, char *src, int *len);
RTSP_API rtsp_code rtsp_session_media_setup(rtsp_session_t * session,  void *pUser);
RTSP_API rtsp_code rtsp_session_media_teardown(rtsp_session_t * session, void *pUser);
RTSP_API rtsp_code rtsp_session_media_play(rtsp_session_t * session, void *pUser);
RTSP_API rtsp_code rtsp_session_media_pause(rtsp_session_t * session, void *pUser);

int rtsp_session_media_overtcp_start(zpl_mediartp_session_t *rtpsession, zpl_bool start);


#ifdef __cplusplus
}
#endif

#endif /* __RTSP_MEDIA_H__ */
