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



RTSP_API char *rtsp_media_name(int channel, int level);


RTSP_API int rtsp_media_lookup(rtsp_session_t * session, int channel, int level, const char *path);
RTSP_API int rtsp_media_start(rtsp_session_t* session, bool start);
RTSP_API int rtsp_media_update(rtsp_session_t * session, bool add);
RTSP_API int rtsp_media_destroy(rtsp_session_t *session);

RTSP_API int rtsp_media_extradata_get(rtsp_session_t *session, int channel, int level, const char *path, void *p);

RTSP_API int rtsp_media_build_sdptext(rtsp_session_t * session, uint8_t *sdp);

RTSP_API int rtsp_media_rtp_sendto(zpl_media_channel_t *mediachn,
        const zpl_skbuffer_t *bufdata,  void *pVoidUser);
RTSP_API int rtsp_media_tcp_forward(rtsp_session_t* session, const uint8_t *buffer, uint32_t len);
RTSP_API int rtsp_media_rtp_recv(rtsp_session_t* session, bool bvideo, zpl_skbuffer_t *bufdata);



RTSP_API rtsp_code rtsp_media_handle_option(rtsp_session_t * session, void *pUser);
RTSP_API rtsp_code rtsp_media_handle_describe(rtsp_session_t * session, void *pUser);
RTSP_API rtsp_code rtsp_media_handle_setup(rtsp_session_t * session, void *pUser);
RTSP_API rtsp_code rtsp_media_handle_teardown(rtsp_session_t * session, void *pUser);
RTSP_API rtsp_code rtsp_media_handle_play(rtsp_session_t * session, void *pUser);
RTSP_API rtsp_code rtsp_media_handle_pause(rtsp_session_t * session, void *pUser);
RTSP_API rtsp_code rtsp_media_handle_scale(rtsp_session_t * session, void *pUser);
RTSP_API rtsp_code rtsp_media_handle_get_parameter(rtsp_session_t * session, void *pUser);
RTSP_API rtsp_code rtsp_media_handle_set_parameter(rtsp_session_t * session, void *pUser);






#ifdef __cplusplus
}
#endif

#endif /* __RTSP_MEDIA_H__ */
