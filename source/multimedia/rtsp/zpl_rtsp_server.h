/*
 * Copyright © 2001-2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __RTSP_SERVER_H__
#define __RTSP_SERVER_H__
#ifdef __cplusplus
extern "C" {
#endif



#define RTSP_PACKET_MAX     2048

typedef struct rtp_socket_s
{
    uint16_t        local_rtp_port; //本地RTP端口
    uint16_t        local_rtcp_port;//本地RTCP端口
    zpl_socket_t     _rtp_sock;
    zpl_socket_t     _rtcp_sock;
}rtp_socket_t;

typedef struct rtsp_srv_s {
#ifdef ZPL_WORKQUEUE
    void            *t_master;
    void            *t_accept;
    void            *t_read;
    int             t_sock;
    zpl_taskid_t    t_taskid;
#endif
    zpl_socket_t    listen_sock;
    uint16_t        listen_port;
    char            *listen_address;

    ipstack_fd_set  rset;
    char            *username;
    char            *password;
    char            *realm;
    bool            b_auth;
    char            *srvname;
    rtsp_transport_rtp_t  transport;		/**< Transport ("RTP/AVP")	    */
    bool            unicast;
    uint16_t        rtp_port;
    uint16_t        rtcp_port;

    osker_list_head_t     session_list_head;

    rtp_socket_t    video_sock;
    rtp_socket_t    audio_sock;

    uint8_t         _recv_buf[RTSP_PACKET_MAX];
    uint8_t         _send_buf[RTSP_PACKET_MAX];
    uint8_t         *_send_build;

    uint32_t        _recv_offset;
    uint32_t        _send_offset;
    int32_t        _recv_length;
    int32_t        _send_length;

    uint32_t        debug;
    void            *mutex;
}rtsp_srv_t;

//#define RTSP_SRV_LOCK(x)    if(x && x->mutex) os_mutex_lock(x->mutex, OS_WAIT_FOREVER)
//#define RTSP_SRV_UNLOCK(x)  if(x && x->mutex) os_mutex_unlock(x->mutex)
#define RTSP_SRV_LOCK(x)    
#define RTSP_SRV_UNLOCK(x)  

RTSP_API void rtsp_srv_destroy(rtsp_srv_t *ctx);
#ifdef ZPL_WORKQUEUE
RTSP_API rtsp_srv_t* rtsp_srv_create(const char *ip_address, uint16_t port, int pro);
#else
RTSP_API rtsp_srv_t* rtsp_srv_create(const char *ip_address, uint16_t port);
RTSP_API int rtsp_srv_select(rtsp_srv_t *ctx);
#endif
RTSP_API int rtsp_srv_session_create(rtsp_srv_t *ctx, zpl_socket_t sock, const char *ip_address, uint16_t port);
RTSP_API int rtsp_srv_session_destroy(rtsp_srv_t *ctx, zpl_socket_t sock);
RTSP_API rtsp_session_t * rtsp_srv_session_lookup(rtsp_srv_t *ctx, zpl_socket_t sock);


#ifdef __cplusplus
}
#endif

#endif /* __RTSP_SERVER_H__ */
