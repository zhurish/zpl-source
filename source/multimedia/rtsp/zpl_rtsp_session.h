#ifndef __RTSP_SESSION_H__
#define __RTSP_SESSION_H__
#ifdef __cplusplus
extern "C" {
#endif



#define VIDEO_RTP_PORT_DEFAULT            28964
#define VIDEO_RTCP_PORT_DEFAULT           28965

#define AUDIO_RTP_PORT_DEFAULT            28974
#define AUDIO_RTCP_PORT_DEFAULT           28975

typedef enum
{
    RTSP_SESSION_STATE_NONE          = 0,
    RTSP_SESSION_STATE_CLOSE,
    RTSP_SESSION_STATE_CONNECT,
    RTSP_SESSION_STATE_SETUP,
    RTSP_SESSION_STATE_READY,
} rtsp_session_state;


typedef enum
{
    RTP_SESSION_STATE_NONE          = 0,
    RTP_SESSION_STATE_CLOSE,
    RTP_SESSION_STATE_START,
    RTP_SESSION_STATE_STOP,
} rtp_session_state;


typedef int rtp_media_pt;

typedef struct rtsp_session_s rtsp_session_t;

typedef int(*rtsp_session_call) (rtsp_session_t*, void *);


typedef struct rtp_session_s
{
    bool            b_enable;           //使能
    zpl_socket_t    rtp_sock;           //rtp socket
    zpl_socket_t    rtcp_sock;          //rtcp socket
    uint8_t         rtpmode;
    void      *rtp_session;       //rtp session
    rtp_session_state rtp_state;        //RTP流状态
    int             i_trackid;          //视频通道
    bool            b_issetup;          //视频是否设置

    uint32_t        local_ssrc;
    uint16_t        local_rtp_port;     //本地RTP端口
    uint16_t        local_rtcp_port;    //本地RTCP端口

    rtsp_transport_t    transport;
    rtp_media_pt    payload;            //音视频编解码ID

    uint16_t        packetization_mode; //封包解包模式
    uint32_t        user_timestamp;     //用户时间戳
    uint32_t        timestamp_interval; //用户时间戳间隔
    uint32_t        framerate;          //帧率
    uint32_t        frame_delay_msec;             //发包时间间隔，毫秒

    void            *pdata;
}rtp_session_t;


typedef struct rtsp_callback_s
{
    rtsp_session_call _options_func;
    rtsp_session_call _describe_func;
    rtsp_session_call _setup_func;
    rtsp_session_call _teardown_func;
    rtsp_session_call _play_func;
    rtsp_session_call _pause_func;
    rtsp_session_call _scale_func;
    rtsp_session_call _set_parameter_func;
    rtsp_session_call _get_parameter_func;

    void * _options_user;
    void * _describe_user;
    void * _setup_user;
    void * _teardown_user;
    void * _play_user;
    void * _pause_user;
    void * _scale_user;
    void * _set_parameter_user;
    void * _get_parameter_user;

}rtsp_callback_t;

typedef struct rtsp_session_s {
    struct osker_list_head node;          //指向双链表前后结点的指针
    bool            bsrv;           //是否为服务端
    uint32_t        session;        //SESSION ID
    zpl_socket_t    sock;           //rtsp socket
    uint16_t        port;           //客户端端口
    char            *address;       //客户端IP地址
    bool            b_auth;         //用户认证
    char            *username;
    char            *password;
    char            *rtsp_url;      //访问流路径

    char            *mfilepath;       //没有IP地址信息的绝对路径
    int32_t         mchannel;
    int32_t         mlevel;

    rtp_session_t   video_session;
    rtp_session_t   audio_session;
    rtp_session_t   *_rtpsession;
    void      *session_set;
#ifdef ZPL_WORKQUEUE
    void            *t_master;
    void            *t_read;
#endif
    void            *parent;

    rtsp_callback_t rtsp_callback;

    int cseq;
    struct sdp_session sdptext; //RTSP 的SDP信息

    rtsp_session_state   state;     //RTSP状态

    uint32_t        rtp_payload_size;       //去掉RTP头后负载最大长度
    uint32_t        max_packet_size;        //整个RTP报文大小（函RTP头）

    //int (*_sendto)(rtsp_session_t *, uint8_t *, uint32_t );
    //int (*_recvfrom)(rtsp_session_t *, uint8_t *, uint32_t );

    //int (*_rtp_over_rtsp_send)(rtsp_session_t *, uint8_t *, uint32_t );
    //int (*_rtp_over_rtsp_recv)(rtsp_session_t *, uint8_t *, uint32_t );

    void            *mutex;
}rtsp_session_t;


//#define RTSP_SESSION_LOCK(x)    if(x && x->mutex) os_mutex_lock(x->mutex, OS_WAIT_FOREVER)
//#define RTSP_SESSION_UNLOCK(x)  if(x && x->mutex) os_mutex_unlock(x->mutex)
#define RTSP_SESSION_LOCK(x)    
#define RTSP_SESSION_UNLOCK(x)  


RTSP_API zpl_socket_t rtsp_session_listen(const char *lip, uint16_t port);
RTSP_API int rtsp_session_connect(rtsp_session_t * session, const char *ip, uint16_t port, int tomeout_ms);
RTSP_API int rtsp_session_close(rtsp_session_t * session);
RTSP_API int rtsp_session_sendto(rtsp_session_t * session, uint8_t *data, uint32_t length);
RTSP_API int rtsp_session_recvfrom(rtsp_session_t * session, uint8_t *data, uint32_t length);


RTSP_API int rtsp_session_default(rtsp_session_t * newNode, bool srv);
RTSP_API int rtsp_session_lstinit(struct osker_list_head * list);
RTSP_API int rtsp_session_lstexit(struct osker_list_head * list);
RTSP_API rtsp_session_t * rtsp_session_create(zpl_socket_t sock, const char *address, uint16_t port, void *parent);
RTSP_API int rtsp_session_destroy(rtsp_session_t *session);
RTSP_API rtsp_session_t * rtsp_session_add(struct osker_list_head * list, zpl_socket_t sock, const char *address, uint16_t port, void *parent);
RTSP_API int rtsp_session_del(struct osker_list_head * list,zpl_socket_t sock);
RTSP_API int rtsp_session_del_byid(struct osker_list_head * list, uint32_t sessionid);
RTSP_API int rtsp_session_cleancache(struct osker_list_head * list);
RTSP_API rtsp_session_t *rtsp_session_lookup(struct osker_list_head * list,zpl_socket_t sock);
RTSP_API rtsp_session_t *rtsp_session_lookup_byid(struct osker_list_head * list,uint32_t sessionid);
RTSP_API int rtsp_session_count(struct osker_list_head * list);
RTSP_API int rtsp_session_update_maxfd(struct osker_list_head * list);
RTSP_API int rtsp_session_foreach(struct osker_list_head * list, int (*calback)(rtsp_session_t *, void *), void * pVoid);

RTSP_API int rtp_over_rtsp_session_sendto(rtsp_session_t * session, uint8_t chn, uint8_t *data, uint32_t length);
RTSP_API int rtsp_session_install(rtsp_session_t * newNode, rtsp_method method, rtsp_session_call func, void *p);
RTSP_API int rtsp_session_callback(rtsp_session_t * newNode, rtsp_method method);
RTSP_API int rtsp_session_pdata(rtsp_session_t * newNode, bool bvideo, void *pdata);

#ifdef __cplusplus
}
#endif

#endif /* __RTSP_SESSION_H__ */
