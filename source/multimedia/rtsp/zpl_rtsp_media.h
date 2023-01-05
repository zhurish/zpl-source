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



//rtp_h264_cfg h264;
//rtp_h264_cfg h265;

#ifndef ZPL_LIBRTSP_MODULE
#pragma pack(1)
typedef struct
{
    uint8_t     video;
    uint8_t     audio;
    uint32_t    video_len;  //缓冲区大小
    uint32_t    audio_len;  //缓冲区大小
}packet_head_t;
#pragma pack(0)

typedef struct node		/* Node of a linked list. */
    {
    struct node *next;		/* Points at the next node in the list */
    struct node *previous;	/* Points at the previous node in the list */
    } NODE;

typedef struct
{
    NODE	node;
    zpl_uint8 	ID;				//ID 通道号
    zpl_uint8 	buffer_type;		//音频视频
    zpl_uint8 	buffer_codec;	//帧类型
    zpl_uint8 	buffer_key;		//帧类型

    zpl_uint32 	buffer_timetick;		//时间戳
    zpl_uint32 	buffer_seq;		//序列号
    zpl_uint32	buffer_len;		//当前缓存帧的长度
    zpl_uint32	buffer_maxsize;		//buffer 的长度
    zpl_void	*buffer_data;	//buffer
}zpl_skbuffer_t, zpl_buffer_data_t;

typedef struct
{
    packet_head_t head;
    zpl_buffer_data_t   data;
}zpl_framedata_t;


typedef struct zpl_media_file_s zpl_media_file_t;
 
struct zpl_media_file_s
{
    uint8_t     filename[128];
    FILE        *fp;
    int         file_size;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
    int         msec;           //定时间隔
    void        *t_master;
    void        *t_read;

    zpl_framedata_t packet;
    bool        bvideo;           //音频视频
    int 	    enctype;		//编码类型
    uint32_t    flags;

    uint32_t    cnt;
    void        *pdata;                     //稀有数据
    void    *buffer_queue;              //消息队列
    void    *parent;                    //父节点
    zpl_framedata_t tmppacket;

    int         (*get_frame)(zpl_media_file_t*, zpl_framedata_t *);//读取一帧数据回调函数
    int         (*get_extradata)(zpl_media_file_t*, zpl_video_extradata_t *);//读取额外数据回调函数
    int         (*put_frame)(zpl_media_file_t*, zpl_framedata_t *);
    int         (*put_extradata)(zpl_media_file_t*, zpl_video_extradata_t *);
};
#endif




RTSP_API char *rtsp_media_name(int channel, int level);


RTSP_API rtsp_media_t* rtsp_media_lookup(rtsp_session_t * session, int channel, int level, const char *path);
RTSP_API rtsp_media_t* rtsp_media_create(rtsp_session_t * session, int channel, int level, const char *path);
RTSP_API int rtsp_media_destroy(rtsp_session_t * session, rtsp_media_t *media);

RTSP_API int rtsp_media_extradata_get(rtsp_session_t * session, int channel, int level, const char *path, void *p);
RTSP_API int rtsp_media_update(rtsp_session_t * session, rtsp_media_t * channel, bool add);
RTSP_API int rtsp_media_start(rtsp_session_t* session, rtsp_media_t *media, bool start);

RTSP_API int rtsp_media_build_sdptext(rtsp_session_t * session, rtsp_media_t *media, uint8_t *sdp);

RTSP_API int rtsp_media_rtp_sendto(zpl_media_channel_t *mediachn,
        const zpl_skbuffer_t *bufdata,  void *pVoidUser);
RTSP_API int rtsp_media_tcp_forward(rtsp_session_t* session, const uint8_t *buffer, uint32_t len);
RTSP_API int rtsp_media_rtp_recv(rtsp_session_t* session, bool bvideo, zpl_skbuffer_t *bufdata);



RTSP_API rtsp_code rtsp_media_handle_option(rtsp_session_t * session, rtsp_media_t *media);
RTSP_API rtsp_code rtsp_media_handle_describe(rtsp_session_t * session, rtsp_media_t *media);
RTSP_API rtsp_code rtsp_media_handle_setup(rtsp_session_t * session, rtsp_media_t *media);
RTSP_API rtsp_code rtsp_media_handle_teardown(rtsp_session_t * session, rtsp_media_t *media);
RTSP_API rtsp_code rtsp_media_handle_play(rtsp_session_t * session, rtsp_media_t *media);
RTSP_API rtsp_code rtsp_media_handle_pause(rtsp_session_t * session, rtsp_media_t *media);
RTSP_API rtsp_code rtsp_media_handle_scale(rtsp_session_t * session, rtsp_media_t *media);
RTSP_API rtsp_code rtsp_media_handle_get_parameter(rtsp_session_t * session, rtsp_media_t *media);
RTSP_API rtsp_code rtsp_media_handle_set_parameter(rtsp_session_t * session, rtsp_media_t *media);






#ifdef __cplusplus
}
#endif

#endif /* __RTSP_MEDIA_H__ */
