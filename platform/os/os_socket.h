/*
 * os_socket.h
 *
 *  Created on: Aug 25, 2018
 *      Author: zhurish
 */

#ifndef __OS_SOCKET_H__
#define __OS_SOCKET_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "os_include.h"

#define OS_SOCKET_BASE	DAEMON_VTY_DIR



extern int os_sock_create(zpl_bool tcp);
extern int os_sock_bind(int sock, char *ipaddress, zpl_uint16 port);
extern int os_sock_listen(int sock, zpl_uint32 listennum);
extern int os_sock_accept (int accept_sock, void *p);
extern int os_sock_connect(int sock, char *ipaddress, zpl_uint16 port);
extern int os_sock_connect_nonblock(int sock, char *ipaddress, zpl_uint16 port);
extern int os_sock_connect_timeout(int sock, char *ipaddress, zpl_uint16 port, zpl_uint32 timeout_ms);


extern int os_sock_client_write(int fd, char *ipaddress, zpl_uint16 port, char *buf, zpl_uint32 len);

extern int os_sock_raw_create(zpl_int style, zpl_uint16 protocol);
extern int os_sock_raw_bind(int fd, zpl_int family, zpl_uint16 protocol, zpl_int ifindex);
extern int os_sock_raw_sendto(int fd, zpl_int family, zpl_uint16 protocol, zpl_int ifindex,
		zpl_uint8 *dstmac, const char *data, zpl_uint32 len);

extern int os_sock_unix_server_create(zpl_bool tcp, const char *name);
extern int os_sock_unix_accept (int accept_sock, void *p);
extern int os_sock_unix_client_create (zpl_bool tcp, const char *name);
extern int os_sock_unix_client_write(int fd, char *name, char *buf, zpl_uint32 len);

extern int os_tcp_sock_state (int sock);


extern int os_unix_sockpair_create(zpl_bool tcp, int *rfd, int *wfd);

extern int os_select_wait(int maxfd, fd_set *rfdset, fd_set *wfdset, zpl_uint32 timeout_ms);
extern int os_write_timeout(int fd, zpl_char *buf, zpl_uint32 len, zpl_uint32 timeout_ms);
extern int os_read_timeout(int fd, zpl_char *buf, zpl_uint32 len, zpl_uint32 timeout_ms);

extern int os_writemsg(int sock, char *m, int len, char *m1, int len1);
extern int os_write_iov(int fd, int type, struct iovec *iov, int iovcnt);

/**************************************************/
extern zpl_socket_t ipstack_sock_create(zpl_ipstack, zpl_bool tcp);
extern int ipstack_sock_bind(zpl_socket_t sock, char *ipaddress, zpl_uint16 port);
extern int ipstack_sock_listen(zpl_socket_t sock, zpl_uint32 listennum);
extern zpl_socket_t ipstack_sock_accept (zpl_socket_t accept_sock, void *p);
extern int ipstack_sock_connect(zpl_socket_t sock, char *ipaddress, zpl_uint16 port);
extern int ipstack_sock_connect_nonblock(zpl_socket_t sock, char *ipaddress, zpl_uint16 port);
extern int ipstack_sock_connect_timeout(zpl_socket_t sock, char *ipaddress, zpl_uint16 port, zpl_uint32 timeout_ms);


extern int ipstack_sock_client_write(zpl_socket_t fd, char *ipaddress, zpl_uint16 port, char *buf, zpl_uint32 len);

extern zpl_socket_t ipstack_sock_raw_create(zpl_ipstack, zpl_int style, zpl_uint16 protocol);
extern int ipstack_sock_raw_bind(zpl_socket_t fd, zpl_int family, zpl_uint16 protocol, zpl_int ifindex);
extern int ipstack_sock_raw_sendto(zpl_socket_t fd, zpl_int family, zpl_uint16 protocol, zpl_int ifindex,
		zpl_uint8 *dstmac, const char *data, zpl_uint32 len);

extern zpl_socket_t ipstack_sock_unix_server_create(zpl_ipstack, zpl_bool tcp, const char *name);
extern zpl_socket_t ipstack_sock_unix_accept (zpl_socket_t accept_sock, void *p);
extern zpl_socket_t ipstack_sock_unix_client_create (zpl_ipstack, zpl_bool tcp, const char *name);
extern int ipstack_sock_unix_client_write(zpl_socket_t fd, char *name, char *buf, zpl_uint32 len);

extern int ipstack_tcp_sock_state (zpl_socket_t sock);

extern int ipstack_unix_sockpair_create(zpl_ipstack, zpl_bool tcp, zpl_socket_t *rfd, zpl_socket_t *wfd);


extern int ipstack_write_timeout(zpl_socket_t fd, zpl_char *buf, zpl_uint32 len, zpl_uint32 timeout_ms);
extern int ipstack_read_timeout(zpl_socket_t fd, zpl_char *buf, zpl_uint32 len, zpl_uint32 timeout_ms);

extern int ipstack_writemsg(zpl_socket_t sock, char *m, int len, char *m1, int len1);
extern int ipstack_write_iov(zpl_socket_t fd, int type, struct iovec *iov, int iovcnt);

extern int ipstack_select_wait(int maxfd, ipstack_fd_set *rfdset, ipstack_fd_set *wfdset, zpl_uint32 timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* __OS_SOCKET_H__ */
