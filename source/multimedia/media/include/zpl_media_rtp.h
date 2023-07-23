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


#ifdef ZPL_JRTPLIB_MODULE
#include "jrtplib_api.h"
#endif

#define VIDEO_RTP_PORT_DEFAULT            58964
#define VIDEO_RTCP_PORT_DEFAULT           58965

#define AUDIO_RTP_PORT_DEFAULT            58974
#define AUDIO_RTCP_PORT_DEFAULT           58975

typedef struct zpl_mediartp_session_s zpl_mediartp_session_t;
 
struct zpl_mediartp_session_s
{
    NODE            node;
#ifdef ZPL_JRTPLIB_MODULE
    jrtp_session_t     *_session;
#endif
    zpl_bool b_start;
    int payload;
    int framerate;
    char local_address[64];
    uint16_t local_rtpport;
    uint16_t local_rtcpport;

    int32_t    i_trackid;
    int32_t     rtp_interleaved;
    int32_t     rtcp_interleaved;   // rtsp setup response only
    
    int32_t         _call_index;       //媒体回调索引, 音视频通道数据发送
    void            *media_chn;         //媒体数据结构
    uint16_t        packetization_mode; //封包解包模式
    uint32_t        user_timestamp;     //用户时间戳
    uint32_t        timestamp_interval; //用户时间戳间隔

    void            *rtp_media_queue;      //媒体接收队列
};


typedef struct 
{
    int     count;
    void	*mutex;
    LIST	list;
    #ifdef ZPL_JRTPLIB_MODULE
    zpl_media_event_queue_t *evtqueue;
    #endif
    zpl_taskid_t taskid;
}zpl_mediartp_scheduler_t;

typedef struct rtp_session_adap_s
{
    char                *name;
    uint32_t            codec;
    int (*_rtp_sendto)(void *, const uint8_t *, uint32_t, int user_ts);
    int (*_rtp_recv)(void *, uint8_t *, int, uint32_t, int * );
}zpl_mediartp_session_adap_t;



int zpl_mediartp_scheduler_init(void);
int zpl_mediartp_scheduler_exit(void);
int zpl_mediartp_scheduler_start(void);
int zpl_mediartp_scheduler_stop(void);


zpl_mediartp_session_t * zpl_mediartp_session_create(int channel, int level);
int zpl_mediartp_session_destroy(int channel, int level);

int zpl_mediartp_session_rtp_sendto(zpl_mediartp_session_t *rtp_session);

int zpl_mediartp_session_suspend(int channel, int level);
int zpl_mediartp_session_resume(int channel, int level);
int zpl_mediartp_session_start(int channel, int level, zpl_bool start);

zpl_mediartp_session_t *zpl_mediartp_session_lookup(int channel, int level);
zpl_bool zpl_mediartp_session_getbind(int channel, int level);
int zpl_mediartp_session_remoteport(int channel, int level, char *address, u_int16_t rtpport, u_int16_t rtcpport);
int zpl_mediartp_session_get_localport(int channel, int level, char *address, u_int16_t *rtpport, u_int16_t *rtcpport);
int zpl_mediartp_session_localport(int channel, int level, char *address, u_int16_t rtpport, u_int16_t rtcpport);
int zpl_mediartp_session_tcp_interleaved(int channel, int level, int rtp, int rtcp);
int zpl_mediartp_session_get_tcp_interleaved(int channel, int level, int *rtp, int *rtcp);
int zpl_mediartp_session_get_trackid(int channel, int level, int *trackid);

int zpl_mediartp_session_describe(int channel, int level, void *pUser, char *src, int *len);
int zpl_mediartp_session_setup(int channel, int level, void *pUser);

int zpl_mediartp_session_add_dest(zpl_mediartp_session_t *my_session, char *address, u_int16_t rtpport, u_int16_t rtcpport);
int zpl_mediartp_session_del_dest(zpl_mediartp_session_t *my_session, char *address, u_int16_t rtpport, u_int16_t rtcpport);

int zpl_mediartp_session_rtpmap_h264(int channel, int level, char *src, uint32_t len);

int zpl_media_channel_multicast_enable(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_bool enable, char *address, int rtp_port, char *localaddr);
zpl_bool zpl_media_channel_multicast_state(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index);


#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_RTP_H__ */
