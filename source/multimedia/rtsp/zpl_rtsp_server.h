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






typedef struct zpl_rtsp_srv_s {

    void            *t_master;
    void            *t_accept;
    void            *t_read;
    int             t_sock;

    zpl_socket_t    listen_sock;
    uint16_t        listen_port;
    char            *listen_address;

    ipstack_fd_set  rset;
    char            *username;
    char            *password;
    char            *realm;
    bool            b_auth;

    LIST     _list_head;
    void     *mutex;
}zpl_rtsp_srv_t;

#define RTSP_SRV_LOCK(x)    
#define RTSP_SRV_UNLOCK(x)  

RTSP_API void rtsp_srv_destroy(zpl_rtsp_srv_t *ctx);
RTSP_API zpl_rtsp_srv_t *rtsp_srv_create(void *m, const char *ip, uint16_t port, int pro);



#ifdef __cplusplus
}
#endif

#endif /* __RTSP_SERVER_H__ */
