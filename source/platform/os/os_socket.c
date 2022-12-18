/*
 * os_socket.c
 *
 *  Created on: Aug 25, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include <log.h>
#include <sys/un.h>

int os_sock_create(zpl_bool tcp)
{
	int rc = 0; //, ret;
	/* socket creation */
	rc = socket(AF_INET, tcp ? SOCK_STREAM : SOCK_DGRAM, tcp ? IPPROTO_TCP : IPPROTO_UDP);
	if (rc < 0)
	{
		_OS_ERROR( "cannot open socket\n");
		return ERROR;
	}

	return rc;
}

int os_sock_bind(int sock, char *ipaddress, zpl_uint16 port)
{
	struct sockaddr_in serv;
	int ret = 0; //, flag = 1;
	/* bind local server port */
	serv.sin_family = AF_INET;
	if (ipaddress)
		serv.sin_addr.s_addr = ipstack_inet_addr(ipaddress);
	else
		serv.sin_addr.s_addr = htonl(INADDR_ANY);
	serv.sin_port = htons(port);

	ret = bind(sock, (struct sockaddr *)&serv, sizeof(serv));
	if (ret < 0)
	{
		_OS_ERROR( "cannot bind(%d) port number %d(%s) \n", sock, port,strerror(ipstack_errno));
		return ERROR;
		;
	}
	return OK;
}

int os_sock_listen(int sock, zpl_uint32 listennum)
{
	int ret = 0;
	ret = listen(sock, listennum);
	if (ret < 0)
	{
		_OS_ERROR( "cannot listen(%d) %s \n",sock,strerror(ipstack_errno));
		return ERROR;
	}
	return OK;
}

int os_sock_accept(int accept_sock, void *p)
{
	int sock;
	zpl_uint32 client_len;
	struct sockaddr_in client;

	memset(&client, 0, sizeof(struct sockaddr_in));
	client_len = sizeof(struct sockaddr_in);

	sock = accept(accept_sock, (struct sockaddr *)&client,
				  (socklen_t *)&client_len);
	if ((char *)p)
	{
		os_memcpy((char *)p, &client, sizeof(client));
	}
	return sock;
}

int os_tcp_sock_state(int sock)
{
	zpl_uint32 client_len;
	struct tcp_info client;

	memset(&client, 0, sizeof(struct tcp_info));
	client_len = sizeof(struct tcp_info);

	sock = getsockopt(sock, IPPROTO_TCP, TCP_INFO, &client,
					  (socklen_t *)&client_len);

	return client.tcpi_state;
}
int os_sock_connect(int sock, char *ipaddress, zpl_uint16 port)
{
	int ret = 0;
	if (ipaddress)
	{
		struct sockaddr_in serv;
		/* bind local server port */
		serv.sin_family = AF_INET;
		serv.sin_addr.s_addr = ipstack_inet_addr(ipaddress);
		serv.sin_port = htons(port);

		ret = connect(sock, (struct sockaddr *)&serv, sizeof(serv));
		if (ret < 0)
		{
			_OS_ERROR("unblock connect(%d) failed!\n", sock);
			return ERROR;
		}
		return OK;
	}
	return ERROR;
}
int os_sock_connect_nonblock(int sock, char *ipaddress, zpl_uint16 port)
{
	int ret = 0;
	if (ipaddress)
	{
		struct sockaddr_in serv;
		/* bind local server port */
		serv.sin_family = AF_INET;
		serv.sin_addr.s_addr = ipstack_inet_addr(ipaddress);
		serv.sin_port = htons(port);
		if (os_get_blocking(sock) == 1)
			os_set_nonblocking(sock);
		ret = connect(sock, (struct sockaddr *)&serv, sizeof(serv));
		if (ret < 0)
		{
			int sockerror = 0;
			socklen_t length = sizeof(sockerror);
			if (ipstack_errno != EINPROGRESS)
			{
				_OS_ERROR("unblock connect(%d) failed!\n", sock);
				return ERROR;
			}
			if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &sockerror, &length) < 0)
			{
				_OS_ERROR("get socket(%d) option failed\n",sock);
				return ERROR;
			}
			if (sockerror != 0)
			{
				_OS_ERROR("connection failed (%d) with the error: %d \n", sock, sockerror);
				return ERROR;
			}
		}
		return OK;
	}
	return ERROR;
}

int os_sock_connect_timeout(int sock, char *ipaddress, zpl_uint16 port, zpl_uint32 timeout_ms)
{
	int ret = 0;

	if (ipaddress && sock > 0)
	{
		fd_set writefds;
		int sockerror = 0;
		socklen_t length = sizeof(sockerror);
		struct sockaddr_in serv;
		/* bind local server port */
		serv.sin_family = AF_INET;
		serv.sin_addr.s_addr = ipstack_inet_addr(ipaddress);
		serv.sin_port = htons(port);
		if (os_get_blocking(sock) == 1)
			os_set_nonblocking(sock);
		ret = connect(sock, (struct sockaddr *)&serv, sizeof(serv));
		if (ret < 0)
		{
			// unblock mode --> connect return immediately! ret = -1 & ipstack_errno=EINPROGRESS
			if (ipstack_errno != EINPROGRESS)
			{
				_OS_ERROR("unblock connect(%d) failed!\n", sock);
				return ERROR;
			}
			if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &sockerror, &length) < 0)
			{
				_OS_ERROR("get socket(%d) option failed\n",sock);
				return ERROR;
			}

			if (sockerror != 0)
			{
				_OS_ERROR("connection failed after select(%d) with the error: %d \n", sock, sockerror);
				return ERROR;
			}
		}
		FD_ZERO(&writefds);
		FD_SET(sock, &writefds);
		ret = os_select_wait(sock + 1, NULL, &writefds, timeout_ms);
		// use select to check write event, if the socket is writable, then
		// connect is complete successfully!
		if (ret == OS_TIMEOUT)
		{
			_OS_WARN("connect(%d) timeout:%s\n", sock, ipaddress);
			return OS_TIMEOUT;
		}
		if (ret < 0)
		{
			_OS_ERROR("connect(%d) %s error %s\n", sock, ipaddress, strerror(ipstack_errno));
			return ERROR;
		}
		if (!FD_ISSET(sock, &writefds))
		{
			_OS_ERROR("no events on sockfd(%d) found\n", sock);
			return ERROR;
		}
		if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &sockerror, &length) < 0)
		{
			_OS_ERROR("get socket(%d) option failed\n",sock);
			return ERROR;
		}

		if (sockerror != 0)
		{
			_OS_ERROR("connection failed after select(%d) with the error: %d \n", sock, sockerror);
			return ERROR;
		}
		return OK;
	}
	return ERROR;
}

int os_sock_client_write(int fd, char *ipaddress, zpl_uint16 port, char *buf, zpl_uint32 len)
{
	int ret = ERROR;
	if (buf && ipaddress)
	{
		struct sockaddr_in serv;
		/* bind local server port */
		serv.sin_family = AF_INET;
		serv.sin_addr.s_addr = ipstack_inet_addr(ipaddress);
		serv.sin_port = htons(port);

		ret = sendto(fd, buf, len, 0, (struct sockaddr *)&serv, sizeof(serv));
		if (ret < 0)
		{
			_OS_ERROR( "cannot sendto(%d) to %s:%d(%s) \n", fd, ipaddress, port,
					strerror(ipstack_errno));
			return ERROR;
			;
		}
		return ret;
	}
	return ret;
}

int os_sock_raw_create(zpl_int style, zpl_uint16 protocol)
{
	int fd = 0;
	if ((fd = socket(AF_PACKET, style, htons(protocol))) < 0)
	{
		_OS_ERROR( "failed to open raw socket (%s)", strerror(ipstack_errno));
		return (ERROR);
	}
	return fd;
}

int os_sock_raw_bind(int fd, zpl_int family, zpl_uint16 protocol, zpl_int ifindex)
{
	int ret = 0;
	struct sockaddr_ll sock;
	memset(&sock, 0, sizeof(sock)); /* let's be deterministic */
	sock.sll_family = family;
	sock.sll_protocol = htons(protocol);
	sock.sll_ifindex = ifindex;
	// zlog_debug(MODULE_DHCP, "Can not bind raw socket(%s)", strerror(ipstack_errno));
	/*sock.sll_hatype = ARPHRD_???;*/
	/*sock.sll_pkttype = PACKET_???;*/
	/*sock.sll_halen = ???;*/
	/*sock.sll_addr[8] = ???;*/
	ret = bind(fd, (struct sockaddr *)&sock, sizeof(sock));
	if (ret == 0)
		return OK;
	return ERROR;
}

int os_sock_raw_sendto(int fd, zpl_int family, zpl_uint16 protocol, zpl_int ifindex,
					   zpl_uint8 *dstmac, const char *data, zpl_uint32 len)
{
	struct sockaddr_ll dest_sll;

	int result = ERROR;

	memset(&dest_sll, 0, sizeof(dest_sll));

	dest_sll.sll_family = family;
	dest_sll.sll_protocol = htons(protocol);
	dest_sll.sll_ifindex = (ifindex);
	/*dest_sll.sll_hatype = ARPHRD_???;*/
	/*dest_sll.sll_pkttype = PACKET_???;*/
	dest_sll.sll_halen = 6;
	memcpy(dest_sll.sll_addr, dstmac, 6);

	result = sendto(fd, data, len, /*flags:*/ 0,
					(struct sockaddr *)&dest_sll, sizeof(dest_sll));
	return result;
}

int os_unix_sockpair_create(zpl_bool tcp, int *rfd, int *wfd)
{
	int fd[2];
	if (socketpair(AF_UNIX, tcp ? SOCK_STREAM : SOCK_DGRAM, 0, fd) == 0)
	{
		if (rfd)
			*rfd = fd[0];
		if (wfd)
			*wfd = fd[1];
		return OK;
	}
	return ERROR;
}

int os_sock_unix_server_create(zpl_bool tcp, const char *name)
{
	int ret = 0;
	int sock = 0;
	zpl_uint32 len = 0;
	char path[128];
	mode_t old_mask;
	struct sockaddr_un serv;
	/* First of all, unlink existing socket */

	os_memset(path, 0, sizeof(path));
	os_snprintf(path, sizeof(path), "%s/%s.sock", OS_SOCKET_BASE, name);
	/* Set umask */
	old_mask = umask(0007);
	unlink(path);
	/* Make UNIX domain socket. */
	sock = socket(AF_UNIX, tcp ? SOCK_STREAM : SOCK_DGRAM, 0 /*tcp ? IPPROTO_TCP:IPPROTO_UDP*/);
	if (sock < 0)
	{
		_OS_ERROR( "Cannot create unix stream socket: %s",
				strerror(ipstack_errno));
		return ERROR;
	}

	/* Make server socket. */
	memset(&serv, 0, sizeof(struct sockaddr_un));
	serv.sun_family = AF_UNIX;
	strncpy(serv.sun_path, path, strlen(path));
#ifdef HAVE_STRUCT_SOCKADDR_UN_SUN_LEN
	len = serv.sun_len = SUN_LEN(&serv);
#else
	len = sizeof(serv.sun_family) + strlen(serv.sun_path);
#endif /* HAVE_STRUCT_SOCKADDR_UN_SUN_LEN */

	ret = bind(sock, (struct sockaddr *)&serv, len);
	if (ret < 0)
	{
		_OS_ERROR( "Cannot bind(%d) path %s: %s", sock, path, strerror(ipstack_errno));
		close(sock); /* Avoid sd leak. */
		return ERROR;
	}
	if (tcp)
	{
		ret = listen(sock, 5);
		if (ret < 0)
		{
			_OS_ERROR( "listen(fd %d) failed: %s", sock, strerror(ipstack_errno));
			close(sock); /* Avoid sd leak. */
			return ERROR;
		}
	}
	umask(old_mask);

	return sock;
}

int os_sock_unix_accept(int accept_sock, void *p)
{
	int sock = 0;
	zpl_uint32 client_len = 0;
	struct sockaddr_un client;

	memset(&client, 0, sizeof(struct sockaddr_un));
	client_len = sizeof(struct sockaddr_un);

	sock = accept(accept_sock, (struct sockaddr *)&client,
				  (socklen_t *)&client_len);
	if (sock && (char *)p)
	{
		os_memcpy((char *)p, &client, sizeof(client));
	}
	return sock;
}

int os_sock_unix_client_create(zpl_bool tcp, const char *name)
{
	int ret;
	int sock;
	zpl_uint32 len;
	struct sockaddr_un addr;
	struct stat s_stat;
	char path[128];
	/* First of all, unlink existing socket */

	os_memset(path, 0, sizeof(path));
	os_snprintf(path, sizeof(path), "%s/%s.sock", OS_SOCKET_BASE, name);

	/* Stat socket to see if we have permission to access it. */
	ret = stat(path, &s_stat);
	if (ret < 0 && ipstack_errno != ENOENT)
	{
		_OS_ERROR( "sock(%s): stat = %s\n", path,
				strerror(ipstack_errno));
		return ERROR;
	}

	if (ret >= 0)
	{
		if (!S_ISSOCK(s_stat.st_mode))
		{
			_OS_ERROR( "sock(%s): Not a socket\n", path);
			return ERROR;
		}
	}

	sock = socket(AF_UNIX, tcp ? SOCK_STREAM : SOCK_DGRAM, 0 /*tcp ? IPPROTO_TCP:IPPROTO_UDP*/);
	if (sock < 0)
	{
		_OS_ERROR( "sock(%s): socket = %s\n", path,
				strerror(ipstack_errno));
		return ERROR;
	}

	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, path, strlen(path));
#ifdef HAVE_STRUCT_SOCKADDR_UN_SUN_LEN
	len = addr.sun_len = SUN_LEN(&addr);
#else
	len = sizeof(addr.sun_family) + strlen(addr.sun_path);
#endif /* HAVE_STRUCT_SOCKADDR_UN_SUN_LEN */

	ret = connect(sock, (struct sockaddr *)&addr, len);
	if (ret < 0)
	{
		_OS_ERROR( "ipstack sock(%d) (%s): connect = %s\n", sock, path,
				strerror(ipstack_errno));
		close(sock);
		return ERROR;
	}
	return sock;
}

/*
 * just for unix UDP
 */
int os_sock_unix_client_write(int fd, char *name, char *buf, zpl_uint32 len)
{
	int ret = ERROR;
	if (buf && name)
	{
		struct sockaddr_un serv;
		char path[128];
		/* First of all, unlink existing socket */

		os_memset(path, 0, sizeof(path));
		os_snprintf(path, sizeof(path), "%s/%s.sock", OS_SOCKET_BASE, name);

		memset(&serv, 0, sizeof(struct sockaddr_un));
		serv.sun_family = AF_UNIX;
		strncpy(serv.sun_path, path, strlen(path));
#ifdef HAVE_STRUCT_SOCKADDR_UN_SUN_LEN
		len = serv.sun_len = SUN_LEN(&serv);
#else
		len = sizeof(serv.sun_family) + strlen(serv.sun_path);
#endif /* HAVE_STRUCT_SOCKADDR_UN_SUN_LEN */

		ret = sendto(fd, buf, len, 0, (struct sockaddr *)&serv, sizeof(serv));
		if (ret < 0)
		{
			_OS_ERROR( "cannot sendto(%d) to %s(%s) \n", fd, path,
					strerror(ipstack_errno));
			return ERROR;
			;
		}
		return ret;
	}
	return ret;
}

int os_select_wait(int maxfd, fd_set *rfdset, fd_set *wfdset, zpl_uint32 timeout_ms)
{
	zpl_int32 num = 0;
	struct timeval timer_tmp1,timer_tmp2;
	struct timeval timer_wait = {.tv_sec = 1, .tv_usec = 0};
	timer_wait.tv_sec = timeout_ms / 1000;
	timer_wait.tv_usec = (timeout_ms % 1000) * 1000;
	os_gettime (OS_CLK_REALTIME, &timer_tmp1);
	while (1)
	{
		num = select(maxfd, rfdset, wfdset, NULL, timeout_ms ? &timer_wait : NULL);
		if (num < 0)
		{
			if (ipstack_errno == EINTR || ipstack_errno == EAGAIN)
			{
				os_gettime (OS_CLK_REALTIME, &timer_tmp2);
				timer_tmp1 = os_timeval_subtract(timer_tmp2, timer_tmp1);
				timer_wait = os_timeval_subtract(timer_wait, timer_tmp1);
				fprintf(stdout, "%s (ipstack_errno=%d -> %s)", __func__, ipstack_errno, strerror(ipstack_errno));
				continue;
			}
			zlog_err(MODULE_LIB, "===========os_select_wait is ERROR:%d:%s", ipstack_errno, ipstack_strerror(ipstack_errno));
			return ERROR;
		}
		else if (num == 0)
		{
			return OS_TIMEOUT;
		}
		return num;
	}
	return ERROR;
}

int os_write_timeout(int fd, zpl_char *buf, zpl_uint32 len, zpl_uint32 timeout_ms)
{
	int ret = 0;
	fd_set writefds;
	FD_ZERO(&writefds);
	FD_SET(fd, &writefds);
	ret = os_select_wait(fd + 1, NULL, &writefds, timeout_ms);
	if (ret == OS_TIMEOUT)
	{
		_OS_WARN("os_select_wait (%d) timeout on write\n",fd);
		return OS_TIMEOUT;
	}
	if (ret < 0)
	{
		_OS_ERROR("os_select_wait to write(%d) %s\n", fd, strerror(ipstack_errno));
		return ERROR;
	}
	if (!FD_ISSET(fd, &writefds))
	{
		_OS_ERROR("no events on sockfd(%d) found\n",fd);
		return ERROR;
	}
	return write(fd, buf, len);
}

int os_read_timeout(int fd, zpl_char *buf, zpl_uint32 len, zpl_uint32 timeout_ms)
{
	int ret = 0;
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);
	ret = os_select_wait(fd + 1, &readfds, NULL, timeout_ms);
	if (ret == OS_TIMEOUT)
	{
		return OS_TIMEOUT;
	}
	if (ret == ERROR)
	{
		_OS_ERROR("os_select_wait to read(%d) %s\n", fd, strerror(ipstack_errno));
		return ERROR;
	}
	if (!FD_ISSET(fd, &readfds))
	{
		_OS_ERROR("no events on sockfd(%d) found\n", fd);
		return ERROR;
	}
	return read(fd, buf, len);
}

int os_write_iov(int fd, int type, struct iovec *iov, int iovcnt)
{
	int written = 0;

		written = writev(fd, iov, iovcnt);

	/* only place where written should be sign compared */
	if (written < 0)
	{
		if (!((((ipstack_errno) == EAGAIN) || ((ipstack_errno) == EWOULDBLOCK) || ((ipstack_errno) == EINTR))))
			return ERROR;
	}
	return written;
}



/*************************************************************************/

zpl_socket_t ipstack_sock_create(zpl_ipstack stack, zpl_bool tcp)
{
	zpl_socket_t rc; //, ret;

	/* socket creation */
	rc = ipstack_socket(stack, AF_INET, tcp ? SOCK_STREAM : SOCK_DGRAM, tcp ? IPPROTO_TCP : IPPROTO_UDP);
	if (ipstack_invalid(rc))
	{
		_OS_ERROR( "cannot open ipstack socket\n");
		return rc;
	}
	return rc;
}

int ipstack_sock_bind(zpl_socket_t sock, char *ipaddress, zpl_uint16 port)
{
	struct ipstack_sockaddr_in serv;
	int ret = 0; //, flag = 1;
	/* bind local server port */
	serv.sin_family = AF_INET;
	if (ipaddress)
		serv.sin_addr.s_addr = ipstack_inet_addr(ipaddress);
	else
		serv.sin_addr.s_addr = htonl(INADDR_ANY);
	serv.sin_port = htons(port);

	ret = ipstack_bind(sock, (struct ipstack_sockaddr *)&serv, sizeof(serv));
	if (ret < 0)
	{
		_OS_ERROR( "ipstack sock(%d) cannot bind port number %d(%s) \n",ipstack_fd(sock), port,
				strerror(ipstack_errno));
		return ERROR;
		;
	}
	return OK;
}

int ipstack_sock_listen(zpl_socket_t sock, zpl_uint32 listennum)
{
	int ret = 0;
	ret = ipstack_listen(sock, listennum);
	if (ret < 0)
	{
		_OS_ERROR( "ipstack sock(%d) cannot listen %s \n",ipstack_fd(sock),
				strerror(ipstack_errno));
		return ERROR;
		;
	}
	return OK;
}

zpl_socket_t ipstack_sock_accept(zpl_socket_t accept_sock, void *p)
{
	zpl_socket_t sock;
	zpl_uint32 client_len;
	struct ipstack_sockaddr_in client;

	memset(&client, 0, sizeof(struct ipstack_sockaddr_in));
	client_len = sizeof(struct ipstack_sockaddr_in);

	sock = ipstack_accept(accept_sock, (struct ipstack_sockaddr *)&client,
				  (socklen_t *)&client_len);
	if ((char *)p)
	{
		os_memcpy((char *)p, &client, sizeof(client));
	}
	return sock;
}

int ipstack_tcp_sock_state(zpl_socket_t sock)
{
	zpl_uint32 client_len;
	struct tcp_info client;

	memset(&client, 0, sizeof(struct tcp_info));
	client_len = sizeof(struct tcp_info);

	ipstack_getsockopt(sock, IPPROTO_TCP, TCP_INFO, &client,
					  (socklen_t *)&client_len);

	return client.tcpi_state;
}

int ipstack_sock_connect(zpl_socket_t sock, char *ipaddress, zpl_uint16 port)
{
	int ret = 0;
	if (ipaddress)
	{
		struct ipstack_sockaddr_in serv;
		/* bind local server port */
		serv.sin_family = AF_INET;
		serv.sin_addr.s_addr = ipstack_inet_addr(ipaddress);
		serv.sin_port = htons(port);

		ret = ipstack_connect(sock, (struct ipstack_sockaddr *)&serv, sizeof(serv));
		if (ret < 0)
		{
			_OS_ERROR( "ipstack sock(%d) cannot connect to %s:%d(%s) \n",ipstack_fd(sock), ipaddress, port,
					strerror(ipstack_errno));
			return ERROR;
		}
		return OK;
	}
	return ERROR;
}

int ipstack_sock_connect_nonblock(zpl_socket_t sock, char *ipaddress, zpl_uint16 port)
{
	int ret = 0;
	if (ipaddress)
	{
		struct ipstack_sockaddr_in serv;
		/* bind local server port */
		serv.sin_family = AF_INET;
		serv.sin_addr.s_addr = ipstack_inet_addr(ipaddress);
		serv.sin_port = htons(port);
		if (ipstack_get_blocking(sock) == 1)
			ipstack_set_nonblocking(sock);
		ret = ipstack_connect(sock, (struct ipstack_sockaddr *)&serv, sizeof(serv));
		if (ret < 0)
		{
			int sockerror = 0;
			socklen_t length = sizeof(sockerror);
			if (ipstack_errno != EINPROGRESS)
			{
				_OS_ERROR("ipstack sock(%d) unblock connect failed!\n",ipstack_fd(sock));
				return ERROR;
			}
			if (ipstack_getsockopt(sock, IPSTACK_SOL_SOCKET, IPSTACK_SO_ERROR, &sockerror, &length) < 0)
			{
				_OS_ERROR("ipstack sock(%d) get socket option failed\n",ipstack_fd(sock));
				return ERROR;
			}
			if (sockerror != 0)
			{
				_OS_ERROR("ipstack sock(%d) connection failed %d with the error: %d \n",ipstack_fd(sock), sockerror);
				return ERROR;
			}
		}
		return OK;
	}
	return ERROR;
}

int ipstack_sock_connect_timeout(zpl_socket_t sock, char *ipaddress, zpl_uint16 port, zpl_uint32 timeout_ms)
{
	int ret = 0;

	if (ipaddress && !ipstack_invalid(sock))
	{
		int sockerror = 0;
		socklen_t length = sizeof(sockerror);
		struct ipstack_sockaddr_in serv;
		/* bind local server port */
		serv.sin_family = AF_INET;
		serv.sin_addr.s_addr = ipstack_inet_addr(ipaddress);
		serv.sin_port = htons(port);
		if (ipstack_get_blocking(sock) == 1)
			ipstack_set_nonblocking(sock);
		ret = ipstack_connect(sock, (struct ipstack_sockaddr *)&serv, sizeof(serv));
		if (ret < 0)
		{
			// unblock mode --> connect return immediately! ret = -1 & ipstack_errno=EINPROGRESS
			if (ipstack_errno != EINPROGRESS)
			{
				_OS_ERROR("ipstack sock(%d) unblock connect failed!\n",ipstack_fd(sock));
				return ERROR;
			}
			if (ipstack_getsockopt(sock, IPSTACK_SOL_SOCKET, IPSTACK_SO_ERROR, &sockerror, &length) < 0)
			{
				_OS_ERROR("ipstack sock(%d) get socket option failed\n",ipstack_fd(sock));
				return ERROR;
			}
			if (sockerror != 0)
			{
				_OS_ERROR("ipstack sock(%d) connection failed after select with the error: %d \n",ipstack_fd(sock), sockerror);
				return ERROR;
			}
		}
		if(is_os_stack(sock))
		{
			fd_set writefds;
			FD_ZERO(&writefds);
			FD_SET(ipstack_fd(sock), &writefds);
			ret = ipstack_select_wait(ipstack_type(sock), ipstack_fd(sock) + 1, NULL, &writefds, timeout_ms);
			// use select to check write event, if the socket is writable, then
			// connect is complete successfully!
			if (ret == OS_TIMEOUT)
			{
				_OS_WARN("ipstack sock(%d) connect timeout:%s\n",ipstack_fd(sock), ipaddress);
				return OS_TIMEOUT;
			}
			if (ret == ERROR)
			{
				_OS_ERROR("ipstack sock(%d) connect %s error %s\n",ipstack_fd(sock), ipaddress, strerror(ipstack_errno));
				return ERROR;
			}
			if (!FD_ISSET(ipstack_fd(sock), &writefds))
			{
				_OS_ERROR("no events on sockfd(%d) found\n",ipstack_fd(sock));
				return ERROR;
			}
			if (ipstack_getsockopt(sock, IPSTACK_SOL_SOCKET, IPSTACK_SO_ERROR, &sockerror, &length) < 0)
			{
				_OS_ERROR("ipstack sock(%d) get socket option failed\n",ipstack_fd(sock));
				return ERROR;
			}
			if (sockerror != 0)
			{
				_OS_ERROR("ipstack sock(%d) connection failed after select with the error: %d \n",ipstack_fd(sock), sockerror);
				return ERROR;
			}
		}
		else if(is_ipcom_stack(sock))
		{
			ipstack_fd_set writefds;
			IPSTACK_FD_ZERO(&writefds);
			IPSTACK_FD_SET(ipstack_fd(sock), &writefds);
			ret = ipstack_select_wait(ipstack_type(sock), ipstack_fd(sock) + 1, NULL, &writefds, timeout_ms);
			// use select to check write event, if the socket is writable, then
			// connect is complete successfully!
			if (ret == OS_TIMEOUT)
			{
				_OS_WARN("ipstack sock(%d) connect timeout:%s\n",ipstack_fd(sock), ipaddress);
				return OS_TIMEOUT;
			}
			if (ret == ERROR)
			{
				_OS_ERROR("ipstack sock(%d) connect %s error %s\n",ipstack_fd(sock), ipaddress, strerror(ipstack_errno));
				return ERROR;
			}
			if (!IPSTACK_FD_ISSET(ipstack_fd(sock), &writefds))
			{
				_OS_ERROR("no events on sockfd(%d) found\n",ipstack_fd(sock));
				return ERROR;
			}
			if (ipstack_getsockopt(sock, IPSTACK_SOL_SOCKET, IPSTACK_SO_ERROR, &sockerror, &length) < 0)
			{
				_OS_ERROR("ipstack sock(%d) get socket option failed\n",ipstack_fd(sock));
				return ERROR;
			}
			if (sockerror != 0)
			{
				_OS_ERROR("ipstack sock(%d) connection failed after select with the error: %d \n",ipstack_fd(sock), sockerror);
				return ERROR;
			}
		}
		return OK;
	}
	return ERROR;
}

int ipstack_sock_client_write(zpl_socket_t fd, char *ipaddress, zpl_uint16 port, char *buf, zpl_uint32 len)
{
	int ret = ERROR;
	if (buf && ipaddress)
	{
		struct ipstack_sockaddr_in serv;
		/* bind local server port */
		serv.sin_family = AF_INET;
		serv.sin_addr.s_addr = ipstack_inet_addr(ipaddress);
		serv.sin_port = htons(port);

		ret = ipstack_sendto(fd, buf, len, 0, (struct ipstack_sockaddr *)&serv, sizeof(serv));
		if (ret < 0)
		{
			_OS_ERROR( "ipstack sock(%d) cannot sendto to %s:%d(%s) \n",ipstack_fd(fd), ipaddress, port,
					strerror(ipstack_errno));
			return ERROR;
			;
		}
		return ret;
	}
	return ret;
}

zpl_socket_t ipstack_sock_raw_create(zpl_ipstack stack, zpl_int style, zpl_uint16 protocol)
{
	zpl_socket_t fd;
	fd = ipstack_socket(stack, AF_PACKET, style, htons(protocol));
	if (ipstack_invalid(fd))
	{
		_OS_ERROR( "ipstack sock failed to open raw socket(%d) (%s)",ipstack_fd(fd), strerror(ipstack_errno));
		return (fd);
	}
	return fd;
}

int ipstack_sock_raw_bind(zpl_socket_t fd, zpl_int family, zpl_uint16 protocol, zpl_int ifindex)
{
	int ret = 0;
	struct ipstack_sockaddr_ll sock;
	memset(&sock, 0, sizeof(sock)); /* let's be deterministic */
	sock.sll_family = family;
	sock.sll_protocol = htons(protocol);
	sock.sll_ifindex = ifindex;
	// zlog_debug(MODULE_DHCP, "Can not bind raw socket(%s)", strerror(ipstack_errno));
	/*sock.sll_hatype = ARPHRD_???;*/
	/*sock.sll_pkttype = PACKET_???;*/
	/*sock.sll_halen = ???;*/
	/*sock.sll_addr[8] = ???;*/
	ret = ipstack_bind(fd, (struct ipstack_sockaddr *)&sock, sizeof(sock));
	if (ret == 0)
		return OK;
	return ERROR;
}

int ipstack_sock_raw_sendto(zpl_socket_t fd, zpl_int family, zpl_uint16 protocol, zpl_int ifindex,
					   zpl_uint8 *dstmac, const char *data, zpl_uint32 len)
{
	struct ipstack_sockaddr_ll dest_sll;

	int result = ERROR;

	memset(&dest_sll, 0, sizeof(dest_sll));

	dest_sll.sll_family = family;
	dest_sll.sll_protocol = htons(protocol);
	dest_sll.sll_ifindex = (ifindex);
	/*dest_sll.sll_hatype = ARPHRD_???;*/
	/*dest_sll.sll_pkttype = PACKET_???;*/
	dest_sll.sll_halen = 6;
	memcpy(dest_sll.sll_addr, dstmac, 6);

	result = ipstack_sendto(fd, data, len, /*flags:*/ 0,
					(struct ipstack_sockaddr *)&dest_sll, sizeof(dest_sll));
	return result;
}

int ipstack_unix_sockpair_create(zpl_ipstack stack, zpl_bool tcp, zpl_socket_t *rfd, zpl_socket_t *wfd)
{
	zpl_socket_t tpmd[2];
	if (ipstack_socketpair(stack, AF_UNIX, tcp ? SOCK_STREAM : SOCK_DGRAM, tcp ? IPPROTO_TCP : IPPROTO_UDP, tpmd) == 0)
	{
		if(ipstack_invalid(tpmd[0]) || ipstack_invalid(tpmd[1]))
		{
			return ERROR;
		}
		if (rfd)
			*rfd = tpmd[0];
		if (wfd)
			*wfd = tpmd[1];
		return OK;
	}
	return ERROR;
}

zpl_socket_t ipstack_sock_unix_server_create(zpl_ipstack stack, zpl_bool tcp, const char *name)
{
	int ret = 0;
	zpl_socket_t sock;
	zpl_uint32 len = 0;
	char path[128];
	mode_t old_mask;
	struct ipstack_sockaddr_un serv;
	/* First of all, unlink existing socket */

	os_memset(path, 0, sizeof(path));
	os_snprintf(path, sizeof(path), "%s/%s.sock", OS_SOCKET_BASE, name);
	/* Set umask */
	old_mask = umask(0007);
	unlink(path);
	/* Make UNIX domain socket. */
	sock = ipstack_socket(stack, AF_UNIX, tcp ? SOCK_STREAM : SOCK_DGRAM, 0 /*tcp ? IPPROTO_TCP:IPPROTO_UDP*/);
	if (ipstack_invalid(sock))
	{
		_OS_ERROR( "ipstack sock Cannot create unix stream socket: %s",
				strerror(ipstack_errno));
		return sock;
	}

	/* Make server socket. */
	memset(&serv, 0, sizeof(struct ipstack_sockaddr_un));
	serv.sun_family = AF_UNIX;
	strncpy(serv.sun_path, path, strlen(path));
#ifdef HAVE_STRUCT_SOCKADDR_UN_SUN_LEN
	len = serv.sun_len = SUN_LEN(&serv);
#else
	len = sizeof(serv.sun_family) + strlen(serv.sun_path);
#endif /* HAVE_STRUCT_SOCKADDR_UN_SUN_LEN */

	ret = ipstack_bind(sock, (struct ipstack_sockaddr *)&serv, len);
	if (ret < 0)
	{
		_OS_ERROR( "ipstack sock Cannot bind path %s: %s", path, strerror(ipstack_errno));
		ipstack_close(sock); /* Avoid sd leak. */
		return sock;
	}
	if (tcp)
	{
		ret = ipstack_listen(sock, 5);
		if (ret < 0)
		{
			_OS_ERROR( "ipstack sock listen(fd %d) failed: %s", ipstack_fd(sock), strerror(ipstack_errno));
			ipstack_close(sock); /* Avoid sd leak. */
			return sock;
		}
	}
	umask(old_mask);

	return sock;
}

zpl_socket_t ipstack_sock_unix_accept(zpl_socket_t accept_sock, void *p)
{
	zpl_socket_t sock;
	zpl_uint32 client_len = 0;
	struct ipstack_sockaddr_un client;

	memset(&client, 0, sizeof(struct ipstack_sockaddr_un));
	client_len = sizeof(struct ipstack_sockaddr_un);

	sock = ipstack_accept(accept_sock, (struct ipstack_sockaddr *)&client,
				  (socklen_t *)&client_len);
	if (!ipstack_invalid(sock) && (char *)p)
	{
		os_memcpy((char *)p, &client, sizeof(client));
	}
	return sock;
}

zpl_socket_t ipstack_sock_unix_client_create(zpl_ipstack stack, zpl_bool tcp, const char *name)
{
	int ret;
	ZPL_SOCKET_T(sock);
	zpl_uint32 len;
	struct ipstack_sockaddr_un addr;
	struct stat s_stat;
	char path[128];
	/* First of all, unlink existing socket */

	os_memset(path, 0, sizeof(path));
	os_snprintf(path, sizeof(path), "%s/%s.sock", OS_SOCKET_BASE, name);

	/* Stat socket to see if we have permission to access it. */
	ret = stat(path, &s_stat);
	if (ret < 0 && ipstack_errno != ENOENT)
	{
		_OS_ERROR( "ipstack sock (%s): stat = %s\n", path,
				strerror(ipstack_errno));
		return sock;
	}

	if (ret >= 0)
	{
		if (!S_ISSOCK(s_stat.st_mode))
		{
			_OS_ERROR( "ipstack sock(%s): Not a socket\n", path);
			return sock;
		}
	}

	sock = ipstack_socket(stack, AF_UNIX, tcp ? SOCK_STREAM : SOCK_DGRAM, 0 /*tcp ? IPPROTO_TCP:IPPROTO_UDP*/);
	if (ipstack_invalid(sock))
	{
		_OS_ERROR( "ipstack sock(%d) (%s): socket = %s\n",ipstack_fd(sock), path, strerror(ipstack_errno));
		return sock;
	}

	memset(&addr, 0, sizeof(struct ipstack_sockaddr_un));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, path, strlen(path));
#ifdef HAVE_STRUCT_SOCKADDR_UN_SUN_LEN
	len = addr.sun_len = SUN_LEN(&addr);
#else
	len = sizeof(addr.sun_family) + strlen(addr.sun_path);
#endif /* HAVE_STRUCT_SOCKADDR_UN_SUN_LEN */

	ret = ipstack_connect(sock, (struct ipstack_sockaddr *)&addr, len);
	if (ret < 0)
	{
		_OS_ERROR( "ipstack sock(%d) (%s): connect = %s\n",ipstack_fd(sock), path,
				strerror(ipstack_errno));
		ipstack_close(sock);
		return sock;
	}
	return sock;
}

/*
 * just for unix UDP
 */
int ipstack_sock_unix_client_write(zpl_socket_t fd, char *name, char *buf, zpl_uint32 len)
{
	int ret = ERROR;
	if (buf && name)
	{
		struct ipstack_sockaddr_un serv;
		char path[128];
		/* First of all, unlink existing socket */

		os_memset(path, 0, sizeof(path));
		os_snprintf(path, sizeof(path), "%s/%s.sock", OS_SOCKET_BASE, name);

		memset(&serv, 0, sizeof(struct ipstack_sockaddr_un));
		serv.sun_family = AF_UNIX;
		strncpy(serv.sun_path, path, strlen(path));
#ifdef HAVE_STRUCT_SOCKADDR_UN_SUN_LEN
		len = serv.sun_len = SUN_LEN(&serv);
#else
		len = sizeof(serv.sun_family) + strlen(serv.sun_path);
#endif /* HAVE_STRUCT_SOCKADDR_UN_SUN_LEN */

		ret = ipstack_sendto(fd, buf, len, 0, (struct sockaddr *)&serv, sizeof(serv));
		if (ret < 0)
		{
			_OS_ERROR( "ipstack sock cannot(%d) sendto to %s(%s) \n",ipstack_fd(fd), path,
					strerror(ipstack_errno));
			return ERROR;
		}
		return ret;
	}
	return ret;
}



int ipstack_select_wait(int type, int maxfd, ipstack_fd_set *rfdset, ipstack_fd_set *wfdset, zpl_uint32 timeout_ms)
{
	zpl_int32 num = 0;
	struct ipstack_timeval timer_tmp1,timer_tmp2;
	struct ipstack_timeval timer_wait = {.tv_sec = 1, .tv_usec = 0};
	timer_wait.tv_sec = timeout_ms / 1000;
	timer_wait.tv_usec = (timeout_ms % 1000) * 1000;
	os_gettime (OS_CLK_REALTIME, &timer_tmp1);
	while (1)
	{
		num = ipstack_select(type, maxfd, rfdset, wfdset, NULL, timeout_ms ? &timer_wait : NULL);
		if (num < 0)
		{
			if (ipstack_errno == EINTR || ipstack_errno == EAGAIN)
			{
				os_gettime (OS_CLK_REALTIME, &timer_tmp2);
				timer_tmp1 = os_timeval_subtract(timer_tmp2, timer_tmp1);
				timer_wait = os_timeval_subtract(timer_wait, timer_tmp1);
				continue;
			}
			zlog_err(MODULE_LIB, "===========ipstack_select_wait is ERROR:%d:%s", ipstack_errno, ipstack_strerror(ipstack_errno));			
			return ERROR;
		}
		else if (num == 0)
		{
			return OS_TIMEOUT;
		}
		return num;
	}
	return ERROR;
}

int ipstack_write_timeout(zpl_socket_t fd, zpl_char *buf, zpl_uint32 len, zpl_uint32 timeout_ms)
{
	int ret = 0;
	if(is_os_stack(fd))
	{
		fd_set writefds;
		FD_ZERO(&writefds);
		FD_SET(ipstack_fd(fd), &writefds);
		ret = os_select_wait(ipstack_fd(fd) + 1, NULL, &writefds, timeout_ms);
		if (ret == OS_TIMEOUT)
		{
			_OS_ERROR("os_select_wait timeout on write(%d)\n",ipstack_fd(fd));
			return OS_TIMEOUT;
		}
		if (ret == ERROR)
		{
			_OS_ERROR("os_select_wait to write(%d) error %s\n", ipstack_fd(fd),strerror(ipstack_errno));
			return ERROR;
		}
		if (!FD_ISSET(ipstack_fd(fd), &writefds))
		{
			_OS_ERROR("no events on sockfd(%d) found\n",ipstack_fd(fd));
			return ERROR;
		}
	}
	else if(is_ipcom_stack(fd))
	{
		ipstack_fd_set writefds;
		IPSTACK_FD_ZERO(&writefds);
		IPSTACK_FD_SET(ipstack_fd(fd), &writefds);
		ret = ipstack_select_wait(ipstack_type(fd), ipstack_fd(fd) + 1, NULL, &writefds, timeout_ms);
		if (ret == OS_TIMEOUT)
		{
			_OS_ERROR("ipstack_select_wait timeout on write(%d)\n",ipstack_fd(fd));
			return OS_TIMEOUT;
		}
		if (ret == ERROR)
		{
			_OS_ERROR("ipstack_select_wait write(%d) error %s\n",ipstack_fd(fd), strerror(ipstack_errno));
			return ERROR;
		}
		if (!IPSTACK_FD_ISSET(ipstack_fd(fd), &writefds))
		{
			_OS_ERROR("no events on sockfd(%d) found\n",ipstack_fd(fd));
			return ERROR;
		}
	}
	return ipstack_write(fd, buf, len);
}

int ipstack_read_timeout(zpl_socket_t fd, zpl_char *buf, zpl_uint32 len, zpl_uint32 timeout_ms)
{
	int ret = 0;
	if(is_os_stack(fd))
	{
		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(ipstack_fd(fd), &readfds);
		ret = os_select_wait(ipstack_fd(fd) + 1, &readfds, NULL, timeout_ms);
		if (ret == OS_TIMEOUT)
		{
			return OS_TIMEOUT;
		}
		if (ret == ERROR)
		{
			_OS_ERROR("os_select_wait read(%d) error %s\n",ipstack_fd(fd), strerror(ipstack_errno));
			return ERROR;
		}
		if (!FD_ISSET(ipstack_fd(fd), &readfds))
		{
			_OS_ERROR("no events on sockfd(%d) found\n",ipstack_fd(fd));
			return ERROR;
		}
	}
	else if(is_ipcom_stack(fd))
	{
		ipstack_fd_set readfds;
		IPSTACK_FD_ZERO(&readfds);
		IPSTACK_FD_SET(ipstack_fd(fd), &readfds);
		ret = ipstack_select_wait(ipstack_type(fd), ipstack_fd(fd) + 1, &readfds, NULL, timeout_ms);
		if (ret == OS_TIMEOUT)
		{
			return OS_TIMEOUT;
		}
		if (ret == ERROR)
		{
			_OS_ERROR("ipstack_select_wait read(%d) error %s\n",ipstack_fd(fd), strerror(ipstack_errno));
			return ERROR;
		}
		if (!IPSTACK_FD_ISSET(ipstack_fd(fd), &readfds))
		{
			_OS_ERROR("no events on sockfd(%d) found\n",ipstack_fd(fd));
			return ERROR;
		}
	}
	return ipstack_read(fd, buf, len);
}

int ipstack_write_iov(zpl_socket_t fd, int type, struct iovec *iov, int iovcnt)
{
	int written = 0;

		written = ipstack_writev(fd, iov, iovcnt);

	/* only place where written should be sign compared */
	if (written < 0)
	{
		if (!((((ipstack_errno) == EAGAIN) || ((ipstack_errno) == EWOULDBLOCK) || ((ipstack_errno) == EINTR))))
			return ERROR;
	}
	return written;
}










