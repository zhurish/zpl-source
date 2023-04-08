/*
 * zpl_media_rtp.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_MEDIA_RTP_H__
#define __ZPL_MEDIA_RTP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <zpl_type.h>
#include "auto_include.h"
#include "zplos_include.h"
#include "lib_include.h"

#ifdef ZPL_LIBORTP_MODULE
#include <ortp/ortp.h>
#endif


#define VIDEO_RTP_PORT_DEFAULT            58964
#define VIDEO_RTCP_PORT_DEFAULT           58965

#define AUDIO_RTP_PORT_DEFAULT            58974
#define AUDIO_RTCP_PORT_DEFAULT           58975

typedef struct zpl_mediartp_session_s zpl_mediartp_session_t;
 
struct zpl_mediartp_session_s
{
    NODE            node;
    #ifdef ZPL_LIBORTP_MODULE
    RtpSession     *_session;
    int             rtpmode;
    #endif
    zpl_socket_t    tcpsock;
    zpl_socket_t    rtp_sock;           //rtp socket
    zpl_socket_t    rtcp_sock;          //rtcp socket

    zpl_bool        b_video;
    zpl_bool        overtcp;
    zpl_bool        b_start;        //rtp 传输开始
    int             payload;
    
    int             rtp_interleaved;
    int             rtcp_interleaved;

    int             i_trackid;          //视频通道
    zpl_bool        b_issetup;          //视频是否设置
    char            address[24];
    uint16_t        rtp_port;
    uint16_t        rtcp_port;  // multicast only
    uint32_t        local_ssrc;
    char            local_address[24];
    uint16_t        local_rtp_port;     //本地RTP端口
    uint16_t        local_rtcp_port;    //本地RTCP端口

    uint32_t        framerate;          //帧率
    uint16_t        video_height;       //视频的高度
    uint16_t        video_width;        //视频的宽度
    
    void            *t_master;
    void            *t_rtp_read;
    void            *t_rtcp_read;

    int32_t         mchannel;
    int32_t         mlevel;
    char            filepath[64];
    int32_t         _call_index;       //媒体回调索引, 音视频通道数据发送
    void            *media_chn;         //媒体数据结构
    uint16_t        packetization_mode; //封包解包模式
    uint32_t        user_timestamp;     //用户时间戳
    uint32_t        timestamp_interval; //用户时间戳间隔

    uint32_t        recv_user_timestamp;     //用户时间戳
    uint32_t        recv_timestamp_interval; //用户时间戳间隔
        
    void            *rtp_media_queue;      //媒体接收队列
    void            *rtsp_parent;
    void            *bind_other;


    int             (*rtp_session_send)(zpl_mediartp_session_t *);
    int             (*rtp_session_recv)(zpl_mediartp_session_t *);

    zpl_bool        b_free;
};


typedef struct 
{
    #ifdef ZPL_LIBORTP_MODULE
    SessionSet r_set;
    SessionSet w_set;
    SessionSet e_set;
    #endif
    int     count;
    void	*mutex;
    LIST	list;
    zpl_taskid_t taskid;
}zpl_mediartp_scheduler_t;

typedef struct rtp_session_adap_s
{
    char                *name;
    uint32_t            codec;
    int (*_rtp_sendto)(void *, const uint8_t *, uint32_t, int user_ts);
    int (*_rtp_recv)(void *, uint8_t *, int, uint32_t, int * );
}zpl_mediartp_session_adap_t;



int zpl_mediartp_session_scheduler_init(void);
int zpl_mediartp_session_scheduler_exit(void);
int zpl_mediartp_session_scheduler_start(void);
int zpl_mediartp_session_scheduler_stop(void);


zpl_mediartp_session_t * zpl_mediartp_session_create(int channel, int level, const char *path);
int zpl_mediartp_session_destroy(int channel, int level, const char *path);

zpl_mediartp_session_t *zpl_mediartp_session_lookup(int channel, int level, const char *path);
int zpl_mediartp_session_setup(int channel, int level, const char *path);
int zpl_mediartp_session_suspend(int channel, int level, const char *path);
int zpl_mediartp_session_resume(int channel, int level, const char *path);
int zpl_mediartp_session_start(int channel, int level, const char *path, zpl_bool start);

int zpl_mediartp_session_rtspsock(int channel, int level, const char *path, zpl_socket_t tcpsock);
int zpl_mediartp_session_rtpsock(int channel, int level, const char *path, zpl_socket_t rtpsock, zpl_socket_t rtcpsock);

int zpl_mediartp_session_tcp_interleaved(int channel, int level, const char *path, int rtp_interleaved, int rtcp_interleaved);
int zpl_mediartp_session_trackid(int channel, int level, const char *path, int i_trackid);
int zpl_mediartp_session_remoteport(int channel, int level, const char *path, char *addr, int rtp_port, int rtcp_port);
int zpl_mediartp_session_localport(int channel, int level, const char *path, char *addr, int rtp_port, int rtcp_port);
zpl_bool zpl_mediartp_session_getbind(int channel, int level, const char *path);

int zpl_mediartp_session_get_tcp_interleaved(int channel, int level, const char *path, int *rtp_interleaved, int *rtcp_interleaved);
int zpl_mediartp_session_get_trackid(int channel, int level, const char *path, int *i_trackid);
int zpl_mediartp_session_get_remoteport(int channel, int level, const char *path, char *addr, int *rtp_port, int *rtcp_port);
int zpl_mediartp_session_get_localport(int channel, int level, const char *path, char *addr, int *rtp_port, int *rtcp_port);

int zpl_mediartp_session_rtpmap_h264(int channel, int level, const char *path, char *src, uint32_t len);

int zpl_mediartp_session_overtcp_start(zpl_mediartp_session_t *rtpsession, zpl_bool start);
int zpl_mediartp_session_tcp_forward(zpl_mediartp_session_t* session, const uint8_t *buffer, uint32_t len);

int zpl_media_channel_rtp_enable(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_bool enable, char *address, int rtp_port, char *localaddr);
zpl_bool zpl_media_channel_rtp_state(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index);


int zpl_media_channel_multicast_enable(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_bool enable, char *address, int rtp_port, char *localaddr);
zpl_bool zpl_media_channel_multicast_state(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index);

void rtp_sched_test(void);


#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_RTP_H__ */
