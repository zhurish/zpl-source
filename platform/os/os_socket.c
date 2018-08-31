/*
 * os_socket.c
 *
 *  Created on: Aug 25, 2018
 *      Author: zhurish
 */

#include "zebra.h"
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
//#include <string.h>
#include <log.h>
#include <os_socket.h>
#include <sys/un.h>



int sock_server_create(BOOL tcp, char *ipaddress, int port, int listennum)
{
	int rc, ret;
	struct sockaddr_in serv;
	int flag = 1;

	/* socket creation */
	rc = socket(AF_INET, tcp ? SOCK_STREAM : SOCK_DGRAM, 0);
	if (rc < 0)
	{
		fprintf(stderr, "cannot open socket\n");
		return ERROR;
	}

	if (setsockopt(rc, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) < 0)
	{
		fprintf(stderr, "cannot SO_REUSEADDR socket\n");
		close(rc);
		return ERROR;
	}
	/* bind local server port */
	serv.sin_family = AF_INET;
	if (ipaddress)
		serv.sin_addr.s_addr = inet_addr(ipaddress);
	else
		serv.sin_addr.s_addr = htonl(INADDR_ANY);
	serv.sin_port = htons(port);

	ret = bind(rc, (struct sockaddr *) &serv, sizeof(serv));
	if (ret < 0)
	{
		fprintf(stderr, "cannot bind port number %d(%s) \n", port,
				strerror(errno));
		close(rc);
		return ERROR;;
	}
	if (!tcp)
	{
		ret = listen(rc, listennum);
		if (ret < 0)
		{
			fprintf(stderr, "cannot listen port number %d(%s) \n", port,
					strerror(errno));
			close(rc); /* Avoid sd leak. */
			return ERROR;;
		}
	}
	return rc;
}

int sock_accept (int accept_sock, void *p)
{
	int sock;
	int client_len;
	struct sockaddr_in client;

	memset (&client, 0, sizeof (struct sockaddr_in));
	client_len = sizeof (struct sockaddr_in);

	sock = accept (accept_sock, (struct sockaddr *) &client,
			(socklen_t *) &client_len);
	if((char *)p)
	{
		os_memcpy((char*)p, &client, sizeof(client));
	}
	return sock;
}

int tcp_sock_state (int sock)
{
	int client_len;
	struct tcp_info client;

	memset (&client, 0, sizeof (struct tcp_info));
	client_len = sizeof (struct tcp_info);

	sock = getsockopt (sock, IPPROTO_TCP, TCP_INFO,  &client,
			(socklen_t *) &client_len);

	return client.tcpi_state;
}

int sock_client_create(BOOL tcp, char *ipaddress, int port)
{
	int rc, ret;
	int flag = 1;

	/* socket creation */
	rc = socket(AF_INET, tcp ? SOCK_STREAM : SOCK_DGRAM, 0);
	if (rc < 0)
	{
		fprintf(stderr, "cannot open socket\n");
		return ERROR;
	}

	/*	if (setsockopt(rc, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) < 0)
	 {
	 fprintf(stderr,"cannot SO_REUSEADDR socket\n");
	 close(rc);
	 return ERROR;
	 }*/
	if (tcp || ipaddress)
	{
		struct sockaddr_in serv;
		/* bind local server port */
		serv.sin_family = AF_INET;
		serv.sin_addr.s_addr = inet_addr(ipaddress);
		serv.sin_port = htons(port);

		ret = connect(rc, (struct sockaddr *) &serv, sizeof(serv));
		if (ret < 0)
		{
			fprintf(stderr, "cannot connect to %s:%d(%s) \n", ipaddress, port,
					strerror(errno));
			close(rc);
			return ERROR;;
		}
	}
	return rc;
}

int sock_client_write(int fd, char *ipaddress, int port, char *buf, int len)
{
	int ret = ERROR;
	if (buf && ipaddress)
	{
		struct sockaddr_in serv;
		/* bind local server port */
		serv.sin_family = AF_INET;
		serv.sin_addr.s_addr = inet_addr(ipaddress);
		serv.sin_port = htons(port);

		ret = sendto(fd, buf, len, 0, (struct sockaddr *) &serv, sizeof(serv));
		if (ret < 0)
		{
			fprintf(stderr, "cannot sendto to %s:%d(%s) \n", ipaddress, port,
					strerror(errno));
			return ERROR;;
		}
		return ret;
	}
	return ret;
}

int unix_sock_server_create(BOOL tcp, const char *name)
{
	int ret;
	int sock, len;
	char path[128];
	mode_t old_mask;
	struct sockaddr_un serv;
	/* First of all, unlink existing socket */

	os_memset(path, 0, sizeof(path));
	os_snprintf(path, sizeof(path), "%s/%s.sock", OS_SOCKET_BASE, name);
	/* Set umask */
	old_mask = umask (0007);
	unlink(path);
	/* Make UNIX domain socket. */
	sock = socket(AF_UNIX, tcp ? SOCK_STREAM : SOCK_DGRAM, 0);
	if (sock < 0)
	{
		fprintf(stderr, "Cannot create unix stream socket: %s",
				strerror(errno));
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

	ret = bind(sock, (struct sockaddr *) &serv, len);
	if (ret < 0)
	{
		fprintf(stderr, "Cannot bind path %s: %s", path, strerror(errno));
		close(sock); /* Avoid sd leak. */
		return ERROR;
	}
	if (tcp)
	{
		ret = listen(sock, 5);
		if (ret < 0)
		{
			fprintf(stderr, "listen(fd %d) failed: %s", sock, strerror(errno));
			close(sock); /* Avoid sd leak. */
			return ERROR;
		}
	}
	umask (old_mask);

	return sock;
}

int unix_sock_accept (int accept_sock, void *p)
{
	int sock;
	int client_len;
	struct sockaddr_un client;

	memset (&client, 0, sizeof (struct sockaddr_un));
	client_len = sizeof (struct sockaddr_un);

	sock = accept (accept_sock, (struct sockaddr *) &client,
			(socklen_t *) &client_len);
	if(sock && (char *)p)
	{
		os_memcpy((char*)p, &client, sizeof(client));
	}
	return sock;
}


int unix_sock_client_create (BOOL tcp, const char *name)
{
	int ret;
	int sock, len;
	struct sockaddr_un addr;
	struct stat s_stat;
	char path[128];
	/* First of all, unlink existing socket */

	os_memset(path, 0, sizeof(path));
	os_snprintf(path, sizeof(path), "%s/%s.sock", OS_SOCKET_BASE, name);

	/* Stat socket to see if we have permission to access it. */
	ret = stat(path, &s_stat);
	if (ret < 0 && errno != ENOENT)
	{
		fprintf(stderr, "vtysh_connect(%s): stat = %s\n", path,
				strerror(errno));
		return ERROR;
	}

	if (ret >= 0)
	{
		if (!S_ISSOCK(s_stat.st_mode))
		{
			fprintf(stderr, "vtysh_connect(%s): Not a socket\n", path);
			return ERROR;
		}
	}

	sock = socket(AF_UNIX, tcp ? SOCK_STREAM : SOCK_DGRAM, 0);
	if (sock < 0)
	{
		fprintf(stderr, "vtysh_connect(%s): socket = %s\n", path,
				strerror(errno));
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

	ret = connect(sock, (struct sockaddr *) &addr, len);
	if (ret < 0)
	{
		fprintf(stderr, "vtysh_connect(%s): connect = %s\n", path,
				strerror(errno));
		close(sock);
		return ERROR;
	}
	return sock;
}

/*
 * just for unix UDP
 */
int unix_sock_client_write(int fd, char *name, char *buf, int len)
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

		ret = sendto(fd, buf, len, 0, (struct sockaddr *) &serv, sizeof(serv));
		if (ret < 0)
		{
			fprintf(stderr, "cannot sendto to %s(%s) \n", path,
					strerror(errno));
			return ERROR;;
		}
		return ret;
	}
	return ret;
}
