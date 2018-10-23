/*
 * os_socket.h
 *
 *  Created on: Aug 25, 2018
 *      Author: zhurish
 */

#ifndef __OS_SOCKET_H__
#define __OS_SOCKET_H__

#define OS_SOCKET_BASE	DAEMON_VTY_DIR


extern int sock_server_create(BOOL tcp, char *ipaddress, int port, int listennum);
extern int sock_accept (int accept_sock, void *p);
extern int sock_client_create(BOOL tcp, char *ipaddress, int port);
extern int sock_client_write(int fd, char *ipaddress, int port, char *buf, int len);

extern int unix_sock_server_create(BOOL tcp, const char *name);
extern int unix_sock_accept (int accept_sock, void *p);
extern int unix_sock_client_create (BOOL tcp, const char *name);
extern int unix_sock_client_write(int fd, char *name, char *buf, int len);

extern int tcp_sock_state (int sock);

#endif /* __OS_SOCKET_H__ */
