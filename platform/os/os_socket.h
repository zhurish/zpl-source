/*
 * os_socket.h
 *
 *  Created on: Aug 25, 2018
 *      Author: zhurish
 */

#ifndef __OS_SOCKET_H__
#define __OS_SOCKET_H__

#define OS_SOCKET_BASE	DAEMON_VTY_DIR


extern int sock_create(BOOL tcp);
extern int sock_bind(int sock, char *ipaddress, int port);
extern int sock_listen(int sock, int listennum);
extern int sock_accept (int accept_sock, void *p);
extern int sock_connect(int sock, char *ipaddress, int port);
extern int sock_connect_timeout(int sock, char *ipaddress, int port, int timeout_ms);


extern int sock_client_write(int fd, char *ipaddress, int port, char *buf, int len);

extern int raw_sock_create(int style, int protocol);
extern int raw_sock_bind(int fd, int family, int protocol, int ifindex);
extern int raw_sock_sendto(int fd, int family, int protocol, int ifindex,
		u_int8 *dstmac, const char *data, int len);

extern int unix_sock_server_create(BOOL tcp, const char *name);
extern int unix_sock_accept (int accept_sock, void *p);
extern int unix_sock_client_create (BOOL tcp, const char *name);
extern int unix_sock_client_write(int fd, char *name, char *buf, int len);

extern int tcp_sock_state (int sock);


extern int unix_sockpair_create(BOOL tcp, int *rfd, int *wfd);


#endif /* __OS_SOCKET_H__ */
