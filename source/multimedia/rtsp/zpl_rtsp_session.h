#ifndef __RTSP_SESSION_H__
#define __RTSP_SESSION_H__
#ifdef __cplusplus
extern "C" {
#endif


#define RTSP_PACKET_MAX     2048


typedef enum
{
    RTSP_SESSION_STATE_NONE          = 0,
    RTSP_SESSION_STATE_CLOSE,
    RTSP_SESSION_STATE_CONNECT,
    RTSP_SESSION_STATE_SETUP,
    RTSP_SESSION_STATE_READY,
} rtsp_session_state;


typedef int rtp_media_pt;

typedef struct rtsp_session_s rtsp_session_t;

typedef int(*rtsp_session_call) (rtsp_session_t*, void *);


typedef struct rtsp_rtp_session_s
{
    zpl_bool b_enable;
    zpl_bool b_video;
    int32_t         i_trackid;
    rtsp_transport_t    transport;
}rtsp_rtp_session_t;

typedef struct rtsp_session_s {
    NODE node;          //指向双链表前后结点的指针
    bool            bsrv;           //是否为服务端
    uint32_t        sesid;        //SESSION ID
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

    rtsp_rtp_session_t  mrtp_session[2];
    //zpl_mediartp_session_t   *mediartp_session;

    void            *t_master;
    void            *t_read;
    void            *parent;
    char            *srvname;
    char            *listen_address;    
    int             cseq;
    struct sdp_session sdptext; //RTSP 的SDP信息

    rtsp_session_state   state;     //RTSP状态
    rtsp_transport_t    transport;

    uint8_t         _recv_buf[RTSP_PACKET_MAX];
    uint8_t         _send_buf[RTSP_PACKET_MAX];
    uint8_t         *_send_build;

    uint32_t        _recv_offset;
    uint32_t        _send_offset;
    int32_t         _recv_length;
    int32_t         _send_length;

    //void     *mutex;
}rtsp_session_t;

typedef struct
{
    LIST     _list_head;
    void     *mutex;
    
} rtsp_session_list;

extern int _rtsp_session_debug;

//#define RTSP_SESSION_LOCK(x)    if(x && x->mutex) os_mutex_lock(x->mutex, OS_WAIT_FOREVER)
//#define RTSP_SESSION_UNLOCK(x)  if(x && x->mutex) os_mutex_unlock(x->mutex)
//#define RTSP_SESSION_LOCK(x)    
//#define RTSP_SESSION_UNLOCK(x)  

RTSP_API int rtsp_session_init(void);
RTSP_API int rtsp_session_exit(void);

RTSP_API int rtsp_session_connect(rtsp_session_t * session, const char *ip, uint16_t port, int tomeout_ms);
RTSP_API int rtsp_session_close(rtsp_session_t * session);
RTSP_API int rtsp_session_sendto(rtsp_session_t * session, uint8_t *data, uint32_t length);


RTSP_API int rtsp_session_default(rtsp_session_t * newNode, bool srv);

RTSP_API rtsp_session_t * rtsp_session_create(zpl_socket_t sock, const char *address, uint16_t port, void *ctx, void *master, char *localip);

RTSP_API rtsp_session_t *rtsp_session_lookup(zpl_socket_t sock);
RTSP_API int rtsp_session_count(void);
RTSP_API int rtsp_session_update_maxfd(void);
RTSP_API int rtsp_session_foreach(int (*calback)(rtsp_session_t *, void *), void * pVoid);


#ifdef __cplusplus
}
#endif

#endif /* __RTSP_SESSION_H__ */
