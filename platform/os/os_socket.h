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

#define OS_SOCKET_BASE	DAEMON_VTY_DIR


extern int sock_create(ospl_bool tcp);
extern int sock_bind(int sock, char *ipaddress, ospl_uint16 port);
extern int sock_listen(int sock, ospl_uint32 listennum);
extern int sock_accept (int accept_sock, void *p);
extern int sock_connect(int sock, char *ipaddress, ospl_uint16 port);
extern int sock_connect_timeout(int sock, char *ipaddress, ospl_uint16 port, ospl_uint32 timeout_ms);


extern int sock_client_write(int fd, char *ipaddress, ospl_uint16 port, char *buf, ospl_uint32 len);

extern int raw_sock_create(ospl_int style, ospl_uint16 protocol);
extern int raw_sock_bind(int fd, ospl_int family, ospl_uint16 protocol, ospl_int ifindex);
extern int raw_sock_sendto(int fd, ospl_int family, ospl_uint16 protocol, ospl_int ifindex,
		ospl_uint8 *dstmac, const char *data, ospl_uint32 len);

extern int unix_sock_server_create(ospl_bool tcp, const char *name);
extern int unix_sock_accept (int accept_sock, void *p);
extern int unix_sock_client_create (ospl_bool tcp, const char *name);
extern int unix_sock_client_write(int fd, char *name, char *buf, ospl_uint32 len);

extern int tcp_sock_state (int sock);


extern int unix_sockpair_create(ospl_bool tcp, int *rfd, int *wfd);

#ifdef __cplusplus
}
#endif

#endif /* __OS_SOCKET_H__ */
