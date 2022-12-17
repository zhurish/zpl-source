#ifndef __RTSP_CLIENT_H__
#define __RTSP_CLIENT_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "zpl_rtsp_def.h"
#include "osker_list.h"
#include "zpl_rtsp_sdp.h"
#include "zpl_rtsp_media.h"
#include "zpl_rtsp_session.h"




#define RTSP_REQUEST_TIMEOUT    2

typedef struct zpl_client_media_s
{
    zpl_bool                video_enable;
    zpl_video_codec_t	    video_codec;	    //视频编码参数
    zpl_video_extradata_t   extradata;
    zpl_bool                audio_enable;
    zpl_audio_codec_t	    audio_codec;	    //音频编码参数
}zpl_client_media_t;


typedef struct rtsp_client_s {

    char            *clientname;
    char            *url;
    char            *proto;
    char            *authorization;
    char            *hostname;
    char            *path;

    int             timeout;
    rtsp_method     method;
    int             code;
    uint8_t         _recv_buf[2048];
    uint8_t         _recv_sdpbuf[2048];
    uint8_t         _send_buf[2048];
    uint8_t         *_send_build;

    uint32_t        _recv_offset;
    uint32_t        _send_offset;
    uint32_t        _recv_length;
    uint32_t        _send_length;

    uint32_t        flags;

    rtsp_session_t  *rtsp_session;
    zpl_socket_t    wait_sock;           //rtsp wait socket
    bool            istcp;
    int             (*_rtsp_client_rtpdata)(void *,  zpl_skbuffer_t *);

    zpl_skbuffer_t video_bufdata;
    zpl_skbuffer_t audio_bufdata;

    zpl_client_media_t client_media;

}rtsp_client_t;


RTSP_API rtsp_client_t * rtsp_client_create(const char *name, const char *url);
RTSP_API int rtsp_client_destroy(rtsp_client_t * client);
RTSP_API int rtsp_client_connect(rtsp_client_t * client, int timeout);

RTSP_API int rtsp_client_request(rtsp_client_t * client, rtsp_method method);
RTSP_API int rtsp_client_open(rtsp_client_t * client);
RTSP_API int rtsp_client_close(rtsp_client_t * client);

RTSP_API int rtsp_client_thread(rtsp_client_t * client);


RTSP_API int rtsp_client_rtpread(rtsp_client_t * client);
RTSP_API int rtsp_client_task_init(void* argv);

#ifdef __cplusplus
}
#endif

#endif /* __RTSP_CLIENT_H__ */
