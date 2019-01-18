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



int sock_create(BOOL tcp)
{
	int rc = 0;//, ret;
	//struct sockaddr_in serv;
	//int flag = 1;

	/* socket creation */
	rc = socket(AF_INET, tcp ? SOCK_STREAM : SOCK_DGRAM, 0);
	if (rc < 0)
	{
		fprintf(stderr, "cannot open socket\n");
		return ERROR;
	}

/*	if (setsockopt(rc, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) < 0)
	{
		fprintf(stderr, "cannot SO_REUSEADDR socket\n");
		close(rc);
		return ERROR;
	}*/
	return rc;
}

int sock_bind(int sock, char *ipaddress, int port)
{
	struct sockaddr_in serv;
	int ret = 0;//, flag = 1;
	/* bind local server port */
	serv.sin_family = AF_INET;
	if (ipaddress)
		serv.sin_addr.s_addr = inet_addr(ipaddress);
	else
		serv.sin_addr.s_addr = htonl(INADDR_ANY);
	serv.sin_port = htons(port);

	ret = bind(sock, (struct sockaddr *) &serv, sizeof(serv));
	if (ret < 0)
	{
		fprintf(stderr, "cannot bind port number %d(%s) \n", port,
				strerror(errno));
		return ERROR;;
	}
	return OK;
}

int sock_listen(int sock, int listennum)
{
	//struct sockaddr_in serv;
	int ret = 0;
	ret = listen(sock, listennum);
	if (ret < 0)
	{
		fprintf(stderr, "cannot listen %s \n",
				strerror(errno));
		return ERROR;;
	}
	return OK;
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

int sock_connect(int sock, char *ipaddress, int port)
{
	int ret = 0;

	if (ipaddress)
	{
		struct sockaddr_in serv;
		/* bind local server port */
		serv.sin_family = AF_INET;
		serv.sin_addr.s_addr = inet_addr(ipaddress);
		serv.sin_port = htons(port);

		ret = connect(sock, (struct sockaddr *) &serv, sizeof(serv));
		if (ret < 0)
		{
			fprintf(stderr, "cannot connect to %s:%d(%s) \n", ipaddress, port,
					strerror(errno));
			return ERROR;;
		}
		return OK;
	}
	return ERROR;
}

int sock_connect_timeout(int sock, char *ipaddress, int port, int timeout_ms)
{
	int ret = 0;

	if (ipaddress)
	{
	    fd_set writefds;
	    int sockerror = 0;
	    socklen_t length = sizeof( sockerror );
		struct sockaddr_in serv;
		/* bind local server port */
		serv.sin_family = AF_INET;
		serv.sin_addr.s_addr = inet_addr(ipaddress);
		serv.sin_port = htons(port);
		if(os_get_blocking(sock) == 1)
			os_set_nonblocking(sock);
		ret = connect(sock, (struct sockaddr *) &serv, sizeof(serv));
		if (ret < 0)
		{
		    //unblock mode --> connect return immediately! ret = -1 & errno=EINPROGRESS
		    if ( errno != EINPROGRESS )
		    {
		        printf( "unblock connect failed!\n" );
		        return ERROR;
		    }
		    else if (errno == EINPROGRESS)
		    {
		        printf( "unblock mode socket is connecting...\n" );
		    }
			fprintf(stderr, "cannot connect to %s:%d(%s) \n", ipaddress, port,
					strerror(errno));
			return ERROR;;
		}
	    FD_ZERO( &writefds );
	    FD_SET( sock, &writefds );
		ret = os_select_wait(sock+1, NULL, &writefds, timeout_ms);
	    //use select to check write event, if the socket is writable, then
	    //connect is complete successfully!
		if(ret == 0)
		{
	        printf( "connect timeout\n" );
	        return ERROR;
		}
		if(ret < 0)
		{
	        printf( "connect error %s\n" ,strerror(errno));
	        return ERROR;
		}
	    if ( ! FD_ISSET( sock, &writefds  ) )
	    {
	        printf( "no events on sockfd found\n" );
	        return ERROR;
	    }
	    if( getsockopt( sock, SOL_SOCKET, SO_ERROR, &sockerror, &length ) < 0 )
	    {
	        printf( "get socket option failed\n" );
	        return ERROR;
	    }

	    if( sockerror != 0 )
	    {
	        printf( "connection failed after select with the error: %d \n", sockerror );
	        return ERROR;
	    }
		return OK;
	}
	return ERROR;
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


int unix_sockpair_create(BOOL tcp, int *rfd, int *wfd)
{
	int fd[2];
	if(socketpair (AF_UNIX, tcp ? SOCK_STREAM : SOCK_DGRAM, 0, fd) == 0)
	{
		if(rfd)
			*rfd = fd[0];
		if(wfd)
			*wfd = fd[1];
		return OK;
	}
	return ERROR;
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

