/*
 * Copyright © 2001-2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __RTSP_SOCKET_H__
#define __RTSP_SOCKET_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "zpl_rtsp_def.h"



typedef struct rtsp_socket_s {
    zpl_socket_t (*_socket)(zpl_ipstack, int, int, int );
    int (*_bind)(zpl_socket_t, char *, uint16_t);

    zpl_socket_t (*_accept)(zpl_socket_t,  char *, uint16_t *);
    int (*_listen)(zpl_socket_t, int );

    int (*_connect) (zpl_socket_t, char *addr, uint16_t port);
    int (*_close) (zpl_socket_t);
    int (*_select) (zpl_ipstack, int , ipstack_fd_set *, ipstack_fd_set *, struct timeval *);

    int (*_sendto)(zpl_socket_t, uint8_t *, uint32_t );
    int (*_recvfrom)(zpl_socket_t, uint8_t *, uint32_t );
    int (*_sendmsg)(zpl_socket_t, uint8_t *, uint32_t );
    int (*_recvmsg)(zpl_socket_t, uint8_t *, uint32_t );
}rtsp_socket_t;


extern rtsp_socket_t  rtsp_socket;

#ifdef __cplusplus
}
#endif

#endif /* __RTSP_SOCKET_H__ */
