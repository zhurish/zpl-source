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

#define ZPL_MEDIARTP_MULTICAST_KEY        0xABCDED00

typedef struct zpl_mediartp_session_s zpl_mediartp_session_t;
 
struct zpl_mediartp_session_s
{
    NODE            node;
    uint32_t        keyval;
    int             channel;
    int             level;
    char            *filename;
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
    
    void            *media_chn;         //媒体数据结构
    uint16_t        packetization_mode; //封包解包模式
    uint32_t        user_timestamp;     //用户时间戳
    uint32_t        timestamp_interval; //用户时间戳间隔
    os_mutex_t      *_mutex;
};

#define ZPL_MEDIARTP_CHANNEL_LOCK(m)  if(((zpl_mediartp_session_t*)m) && ((zpl_mediartp_session_t*)m)->_mutex) os_mutex_lock(((zpl_mediartp_session_t*)m)->_mutex, OS_WAIT_FOREVER)
#define ZPL_MEDIARTP_CHANNEL_UNLOCK(m)  if(((zpl_mediartp_session_t*)m) && ((zpl_mediartp_session_t*)m)->_mutex) os_mutex_unlock(((zpl_mediartp_session_t*)m)->_mutex)

typedef struct 
{
    int     count;
    void	*mutex;
    LIST	list;
    #ifdef ZPL_JRTPLIB_MODULE
    zpl_media_event_queue_t *evtqueue;
    #endif
    zpl_taskid_t taskid;
    int     _debug;
}zpl_mediartp_scheduler_t;




int zpl_mediartp_scheduler_init(void);
int zpl_mediartp_scheduler_exit(void);
int zpl_mediartp_scheduler_start(void);
int zpl_mediartp_scheduler_stop(void);


zpl_mediartp_session_t * zpl_mediartp_session_create(int keyval, int channel, int level, char *filename);
int zpl_mediartp_session_destroy(int keyval);

int zpl_mediartp_session_rtp_sendto(zpl_media_channel_t *chm, const void *data, size_t len,
	                u_int8_t pt, bool mark, u_int32_t next_ts);

int zpl_mediartp_session_suspend(int keyval);
int zpl_mediartp_session_resume(int keyval);
int zpl_mediartp_session_start(int keyval, zpl_bool start);

zpl_mediartp_session_t *zpl_mediartp_session_lookup(int keyval, int channel, int level, char *filename);
int zpl_mediartp_session_remoteport(int keyval, char *address, u_int16_t rtpport, u_int16_t rtcpport);
int zpl_mediartp_session_get_localport(int keyval, char *address, u_int16_t *rtpport, u_int16_t *rtcpport);
int zpl_mediartp_session_localport(int keyval, char *address, u_int16_t rtpport, u_int16_t rtcpport);
int zpl_mediartp_session_tcp_interleaved(int keyval, int rtp, int rtcp);
int zpl_mediartp_session_get_tcp_interleaved(int keyval, int *rtp, int *rtcp);
int zpl_mediartp_session_get_trackid(int keyval, int *trackid);

int zpl_mediartp_session_describe(int keyval, void *pUser, char *src, int *len);
int zpl_mediartp_session_setup(int keyval, int channel, int level, char *filename, void *pUser);

int zpl_mediartp_session_add_dest(zpl_mediartp_session_t *my_session, char *address, u_int16_t rtpport, u_int16_t rtcpport);
int zpl_mediartp_session_del_dest(zpl_mediartp_session_t *my_session, char *address, u_int16_t rtpport, u_int16_t rtcpport);

int zpl_mediartp_session_rtpmap_h264(int keyval, char *src, uint32_t len);

int zpl_media_channel_multicast_enable(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_bool enable, char *address, int rtp_port, char *localaddr);
zpl_bool zpl_media_channel_multicast_state(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index);

int zpl_mediartp_session_show(struct vty *vty);

#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_RTP_H__ */
