/*
 * os_ipstack.c
 *
 *  Created on: Aug 25, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include <log.h>
#include <sys/un.h>



char * ipstack_sockstr(zpl_socket_t _sock)
{
	static char buf[64];
	os_bzero(buf, sizeof(buf));
	char *bustr[] = {"UN","OS", "IPNET"};
	if(_sock.stack > IPCOM_STACK)
		_sock.stack = 0;
	snprintf(buf, sizeof(buf), "%s-%d", bustr[_sock.stack], _sock._fd);
	return buf;
}

int ipstack_invalid(zpl_socket_t _sock)
{
#ifdef ZPL_IPCOM_MODULE
	if(_sock.stack == OS_STACK)
	{
		if(_sock._fd >= 3)
			return 0;
		return 1;	
	}
	else if(_sock.stack == IPCOM_STACK)
	{
		if(_sock._fd >= 3)
			return 0;
		return 1;	
	}
	return 1;
#else
	if(_sock._fd >= 3 && _sock.stack == OS_STACK && _sock._fd != ERROR)
		return 0;
	return 1;	
#endif
}

int ipstack_bzero(zpl_socket_t _sock)
{
	_sock.stack = 0;
	_sock._fd = ERROR;
	return OK;
}
int ipstack_init(zpl_ipstack stack, zpl_socket_t _sock)
{
	_sock.stack = stack;
	_sock._fd = ERROR;
	return OK;
}

int ipstack_get_blocking(zpl_socket_t fd)
{
	if(ipstack_invalid(fd))
		return ERROR;
#ifdef ZPL_IPCOM_MODULE
	if(_sock.stack == OS_STACK)
	{
		return os_get_blocking(fd._fd);
	}
	else
	{
		int a = 0;
		if (ipcom_socketioctl(fd._fd, IP_X_FIOGNBIO, (char *)&a) != 0)
		{
			return ERROR;
		}
		return a;
	}
	return OK;
#else
	return os_get_blocking(fd._fd);	
#endif
}


int ipstack_set_nonblocking(zpl_socket_t fd)
{
	if(ipstack_invalid(fd))
		return ERROR;
#ifdef ZPL_IPCOM_MODULE
	if(_sock.stack == OS_STACK)
	{
		return os_set_nonblocking(fd._fd);
	}
	else
	{
		int a = 0;
		if (ipcom_socketioctl(fd._fd, IP_FIONBIO, (char *)&a) != 0)
		{
			return ERROR;
		}
		return OK;
	}
	return ERROR;
#else
	return os_set_nonblocking(fd._fd);	
#endif
}

int ipstack_set_blocking(zpl_socket_t fd)
{
	if(ipstack_invalid(fd))
		return ERROR;
#ifdef ZPL_IPCOM_MODULE
	if(_sock.stack == OS_STACK)
	{
		return os_set_blocking(fd._fd);
	}
	else
	{
		int a = 1;
		if (ipcom_socketioctl(fd._fd, IPSTACK_FIONBIO, (char *)&a) != 0)
		{
			return ERROR;
		}
		return OK;
	}
	return ERROR;
#else
	return os_set_blocking(fd._fd);	
#endif
}

int ipstack_same(zpl_socket_t src, zpl_socket_t dst)
{
	if((src.stack==OS_STACK || src.stack == IPCOM_STACK) && 
		src.stack == dst.stack && src._fd > 0 && src._fd == dst._fd)
		return 1;
	return 0;	
}

int ipstack_copy(zpl_socket_t src, zpl_socket_t dst)
{
	dst._fd = src._fd;
	dst.stack = src.stack;
	return OK;
}

zpl_socket_t ipstack_max(zpl_socket_t src, zpl_socket_t dst)
{
	if(dst._fd > src._fd)
		return dst;
	else
		return src;	
}

zpl_socket_t ipstack_open (zpl_ipstack stack, const char *__path, int __oflag)
{
	zpl_socket_t tmp;
	tmp.stack = stack;
	tmp._fd = open(__path, __oflag);
	if(tmp._fd <= 0)
		tmp.stack = 0;
	return tmp;
}
/* Create a new socket of type TYPE in domain DOMAIN, using
   protocol PROTOCOL.  If PROTOCOL is zero, one is chosen automatically.
   Returns a file descriptor for the new socket, or -1 for errors.  */
zpl_socket_t ipstack_socket (zpl_ipstack stack, int __domain, int __type, int __protocol) 
{
#ifdef ZPL_IPCOM_MODULE
	zpl_socket_t socktmp;
	socktmp.stack = stack;
	if(stack == OS_STACK)
	{
		socktmp._fd = socket(__domain, __type, __protocol);
	}
	else
	{
		socktmp._fd = ipcom_socket(__domain, __type, __protocol);
	}
	_OS_DEBUG_DETAIL("socket create: %s" ,ipstack_sockstr(socktmp));
	return socktmp;
#else
	zpl_socket_t socktmp;
	socktmp.stack = stack = OS_STACK;
	socktmp._fd = socket(__domain, __type, __protocol);
	_OS_DEBUG_DETAIL("socket create: %s" ,ipstack_sockstr(socktmp));
	return socktmp;
#endif
}
/* Create a ipstack_socket for the VRF. */
zpl_socket_t ipstack_socket_vrf(zpl_ipstack stack, int domain, zpl_uint32 type, zpl_uint16 protocol, vrf_id_t vrf_id)
{
	int ret = 0;
	vrf_id_t vrf = vrf_id;
	zpl_socket_t socktmp;
	socktmp = ipstack_socket (stack, domain, type, protocol);
	if(ipstack_invalid(socktmp))
		return socktmp;
#ifdef ZPL_IPCOM_MODULE
	if(stack == OS_STACK)
	{
		ret = setsockopt (socktmp._fd, IPSTACK_SOL_SOCKET, IPSTACK_SO_MARK, &vrf, sizeof (vrf));
		if(ret != 0)
		{
			ipstack_close(socktmp);
		}
	}
	else
	{
		ret = ipcom_setsockopt (socktmp._fd, IPSTACK_SOL_SOCKET, IP_SO_X_VR, &vrf, sizeof (vrf));
		if(ret != 0)
		{
			ipstack_close(socktmp);
		}
	}
#else
	ret = setsockopt (socktmp._fd, IPSTACK_SOL_SOCKET, IPSTACK_SO_MARK, &vrf, sizeof (vrf));
#endif
	if(ret == 0)
	{
		return socktmp;
	}
	ipstack_close(socktmp);
	return socktmp;
}

/* Create two new sockets, of type TYPE in domain DOMAIN and using
   protocol PROTOCOL, which are connected to each other, and put file
   descriptors for them in FDS[0] and FDS[1].  If PROTOCOL is zero,
   one will be chosen automatically.  Returns 0 on success, -1 for errors.  */
int ipstack_socketpair (zpl_ipstack stack, int __domain, int __type, int __protocol,
		       zpl_socket_t _socks[2]) 
{
#ifdef ZPL_IPCOM_MODULE
	int ret = 0, mfds[2];
	_socks[0].stack = _socks[1].stack = stack;
	if(stack == OS_STACK)
	{
		ret = socketpair(__domain, __type, __protocol, mfds);
		_socks[0]._fd = mfds[0];
		_socks[1]._fd = mfds[1];
	}
	else
	{
		ret = ipcom_socketpair(__domain, __type, __protocol, mfds);
		_socks[0]._fd = mfds[0];
		_socks[1]._fd = mfds[1];
	}
	_OS_DEBUG_DETAIL("socket socketpair: %s" ,ipstack_sockstr(_socks[0]));
	return ret;
#else
	int ret = 0, mfds[2];
	_socks[0].stack = _socks[1].stack = stack = OS_STACK;
	ret = socketpair(__domain, __type, __protocol, mfds);
	_socks[0]._fd = mfds[0];
	_socks[1]._fd = mfds[1];
	_OS_DEBUG_DETAIL("socket socketpair: %s" ,ipstack_sockstr(_socks[0]));
	return ret;
#endif
}
/* Give the socket FD the local address ADDR (which is LEN bytes long).  */
int ipstack_bind (zpl_socket_t _sock, void * __addr, socklen_t __len)
{
	if(ipstack_invalid(_sock))
		return ERROR;
#ifdef ZPL_IPCOM_MODULE
	int ret = 0;
	if(_sock.stack == OS_STACK)
	{
		ret = bind(_sock._fd, __addr, __len);
	}
	else
	{
		ret = ipcom_bind(_sock._fd, __addr, __len);
	}
	_OS_DEBUG_DETAIL("socket bind: %s" ,ipstack_sockstr(_socks));
	return ret;
#else
	int ret = bind(_sock._fd, __addr, __len);
	_OS_DEBUG_DETAIL("socket bind: %s" ,ipstack_sockstr(_sock));
	return ret;
#endif
}

/* Put the local address of FD into *ADDR and its length in *LEN.  */
int ipstack_getsockname (zpl_socket_t _sock, void * __addr,
			socklen_t * __len) 
{
	if(ipstack_invalid(_sock))
		return ERROR;
#ifdef ZPL_IPCOM_MODULE
	int ret = 0;
	if(_sock.stack == OS_STACK)
	{
		ret = getsockname(_sock._fd, __addr, __len);
	}
	else
	{
		ret = ipcom_getsockname(_sock._fd, __addr, __len);
	}
	_OS_DEBUG_DETAIL("socket getsockname: %s" ,ipstack_sockstr(_sock));
	return ret;
#else
	int ret = getsockname(_sock._fd, __addr, __len);
	_OS_DEBUG_DETAIL("socket getsockname: %s" ,ipstack_sockstr(_sock));
	return ret;
#endif
}
/* Open a connection on socket FD to peer at ADDR (which LEN bytes long).
   For connectionless socket types, just set the default address to send to
   and the only address from which to accept transmissions.
   Return 0 on success, -1 for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
int ipstack_connect (zpl_socket_t _sock, void * __addr, socklen_t __len)
{
	if(ipstack_invalid(_sock))
		return ERROR;
#ifdef ZPL_IPCOM_MODULE
	int ret = 0;
	if(_sock.stack == OS_STACK)
	{
		ret = connect(_sock._fd, __addr, __len);
	}
	else
	{
		ret = ipcom_connect(_sock._fd, __addr, __len);
	}
	_OS_DEBUG_DETAIL("socket connect: %s" ,ipstack_sockstr(_sock));
	return ret;
#else
	int ret = connect(_sock._fd, __addr, __len);
	_OS_DEBUG_DETAIL("socket connect: %s" ,ipstack_sockstr(_sock));
	return ret;
#endif
}
/* Put the address of the peer connected to socket FD into *ADDR
   (which is *LEN bytes long), and its actual length into *LEN.  */
int ipstack_getpeername (zpl_socket_t _sock, void * __addr,
			socklen_t * __len) 
{
	if(ipstack_invalid(_sock))
		return ERROR;
#ifdef ZPL_IPCOM_MODULE
	int ret = 0;
	if(_sock.stack == OS_STACK)
	{
		ret = getpeername(_sock._fd, __addr, __len);
	}
	else
	{
		ret = ipcom_getpeername(_sock._fd, __addr, __len);
	}
	_OS_DEBUG_DETAIL("socket getpeername: %s" ,ipstack_sockstr(_sock));
	return ret;
#else
	int ret = getpeername(_sock._fd, __addr, __len);
	_OS_DEBUG_DETAIL("socket getpeername: %s" ,ipstack_sockstr(_sock));
	return ret;
#endif
}

/* Send N bytes of BUF to socket FD.  Returns the number sent or -1.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
ssize_t ipstack_send (zpl_socket_t _sock, const void *__buf, size_t __n, int __flags)
{
	if(ipstack_invalid(_sock))
		return ERROR;
#ifdef ZPL_IPCOM_MODULE
	int ret = 0;
	if(_sock.stack == OS_STACK)
	{
		ret = send(_sock._fd, __buf, __n, __flags);
	}
	else
	{
		ret = ipcom_send(_sock._fd, __buf, __n, __flags);
	}
	_OS_DEBUG_DETAIL("socket send: %s" ,ipstack_sockstr(_sock));
	return ret;
#else
	int ret = send(_sock._fd, __buf, __n, __flags);
	_OS_DEBUG_DETAIL("socket send: %s" ,ipstack_sockstr(_sock));
	return ret;
#endif
}
/* Read N bytes into BUF from socket FD.
   Returns the number read or -1 for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
ssize_t ipstack_recv (zpl_socket_t _sock, void *__buf, size_t __n, int __flags)
{
	if(ipstack_invalid(_sock))
		return ERROR;
#ifdef ZPL_IPCOM_MODULE
	int ret = 0;
	if(_sock.stack == OS_STACK)
	{
		ret = recv(_sock._fd, __buf, __n, __flags);
	}
	else
	{
		ret = ipcom_recv(_sock._fd, __buf, __n, __flags);
	}
	_OS_DEBUG_DETAIL("socket recv: %s" ,ipstack_sockstr(_sock));
	return ret;
#else
	int ret = recv(_sock._fd, __buf, __n, __flags);
	_OS_DEBUG_DETAIL("socket recv: %s" ,ipstack_sockstr(_sock));
	return ret;
#endif
}
/* Send N bytes of BUF on socket FD to peer at address ADDR (which is
   ADDR_LEN bytes long).  Returns the number sent, or -1 for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
ssize_t ipstack_sendto (zpl_socket_t _sock, const void *__buf, size_t __n,
		       int __flags, void * __addr,
		       socklen_t __addr_len)
{
	if(ipstack_invalid(_sock))
		return ERROR;
#ifdef ZPL_IPCOM_MODULE
	int ret = 0;
	if(_sock.stack == OS_STACK)
	{
		ret = sendto(_sock._fd, __buf, __n, __flags, __addr, __addr_len);
	}
	else
	{
		ret = ipcom_sendto(_sock._fd, __buf, __n, __flags, __addr, __addr_len);
	}
	_OS_DEBUG_DETAIL("socket sendto: %s" ,ipstack_sockstr(_sock));
	return ret;
#else
	int ret = sendto(_sock._fd, __buf, __n, __flags, __addr, __addr_len);
	_OS_DEBUG_DETAIL("socket sendto: %s" ,ipstack_sockstr(_sock));
	return ret;
#endif
}
/* Read N bytes into BUF through socket FD.
   If ADDR is not NULL, fill in *ADDR_LEN bytes of it with tha address of
   the sender, and store the actual size of the address in *ADDR_LEN.
   Returns the number of bytes read or -1 for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
ssize_t ipstack_recvfrom (zpl_socket_t _sock, void * __buf, size_t __n,
			 int __flags, void * __addr,
			 socklen_t * __addr_len)
{
	if(ipstack_invalid(_sock))
		return ERROR;
#ifdef ZPL_IPCOM_MODULE
	int ret = 0;
	if(_sock.stack == OS_STACK)
	{
		ret = recvfrom(_sock._fd, __buf, __n, __flags, __addr, __addr_len);
	}
	else
	{
		ret = ipcom_recvfrom(_sock._fd, __buf, __n, __flags, __addr, __addr_len);
	}
	_OS_DEBUG_DETAIL("socket recvfrom: %s" ,ipstack_sockstr(_sock));
	return ret;
#else
	int ret = recvfrom(_sock._fd, __buf, __n, __flags, __addr, __addr_len);
	_OS_DEBUG_DETAIL("socket recvfrom: %s" ,ipstack_sockstr(_sock));
	return ret;
#endif
}

/* Send a message described MESSAGE on socket FD.
   Returns the number of bytes sent, or -1 for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
ssize_t ipstack_sendmsg (zpl_socket_t _sock, const struct ipstack_msghdr *__message,
			int __flags)
{
	if(ipstack_invalid(_sock))
		return ERROR;
#ifdef ZPL_IPCOM_MODULE
	int ret = 0;
	if(_sock.stack == OS_STACK)
	{
		ret = sendmsg(_sock._fd, __message, __flags);
	}
	else
	{
		ret = ipcom_sendmsg(_sock._fd, __message, __flags);
	}
	_OS_DEBUG_DETAIL("socket sendmsg: %s" ,ipstack_sockstr(_sock));
	return ret;
#else
	int ret = sendmsg(_sock._fd, __message, __flags);
	_OS_DEBUG_DETAIL("socket sendmsg: %s" ,ipstack_sockstr(_sock));
	return ret;
#endif
}
/* Receive a message as described by MESSAGE from socket FD.
   Returns the number of bytes read or -1 for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
ssize_t ipstack_recvmsg (zpl_socket_t _sock, struct ipstack_msghdr *__message, int __flags)
{
	if(ipstack_invalid(_sock))
		return ERROR;
#ifdef ZPL_IPCOM_MODULE
	int ret = 0;
	if(_sock.stack == OS_STACK)
	{
		ret = recvmsg(_sock._fd, __message, __flags);
	}
	else
	{
		ret = ipcom_recvmsg(_sock._fd, __message, __flags);
	}
	_OS_DEBUG_DETAIL("socket recvmsg: %s" ,ipstack_sockstr(_sock));
	return ret;
#else
	int ret = recvmsg(_sock._fd, __message, __flags);
	_OS_DEBUG_DETAIL("socket recvmsg: %s" ,ipstack_sockstr(_sock));
	return ret;
#endif
}


/* Put the current value for socket FD's option OPTNAME at protocol level LEVEL
   into OPTVAL (which is *OPTLEN bytes long), and set *OPTLEN to the value's
   actual length.  Returns 0 on success, -1 for errors.  */
int ipstack_getsockopt (zpl_socket_t _sock, int __level, int __optname,
		       void * __optval,
		       socklen_t * __optlen)
{
	if(ipstack_invalid(_sock))
		return ERROR;
#ifdef ZPL_IPCOM_MODULE
	int ret = 0;
	if(_sock.stack == OS_STACK)
	{
		ret = getsockopt(_sock._fd, __level, __optname, __optval, __optlen);
	}
	else
	{
		ret = ipcom_getsockopt(_sock._fd, __level, __optname, __optval, __optlen);
	}
	_OS_DEBUG_DETAIL("socket getsockopt: %s" ,ipstack_sockstr(_sock));
	return ret;
#else
	int ret = getsockopt(_sock._fd, __level, __optname, __optval, __optlen);
	_OS_DEBUG_DETAIL("socket getsockopt: %s" ,ipstack_sockstr(_sock));
	return ret;
#endif
}
/* Set socket FD's option OPTNAME at protocol level LEVEL
   to *OPTVAL (which is OPTLEN bytes long).
   Returns 0 on success, -1 for errors.  */
int ipstack_setsockopt (zpl_socket_t _sock, int __level, int __optname,
		       const void *__optval, socklen_t __optlen)
{
	if(ipstack_invalid(_sock))
		return ERROR;
#ifdef ZPL_IPCOM_MODULE
	int ret = 0;
	if(_sock.stack == OS_STACK)
	{
		ret = setsockopt(_sock._fd, __level, __optname, __optval, __optlen);
	}
	else
	{
		ret = ipcom_setsockopt(_sock._fd, __level, __optname, __optval, __optlen);
	}
	_OS_DEBUG_DETAIL("socket setsockopt: %s" ,ipstack_sockstr(_sock));
	return ret;
#else
	int ret = setsockopt(_sock._fd, __level, __optname, __optval, __optlen);
	_OS_DEBUG_DETAIL("socket setsockopt: %s" ,ipstack_sockstr(_sock));
	return ret;
#endif
}

/* Prepare to accept connections on socket FD.
   N connection requests will be queued before further requests are refused.
   Returns 0 on success, -1 for errors.  */
int ipstack_listen (zpl_socket_t _sock, int __n) 
{
	if(ipstack_invalid(_sock))
		return ERROR;
#ifdef ZPL_IPCOM_MODULE
	int ret = 0;
	if(_sock.stack == OS_STACK)
	{
		ret = listen(_sock._fd, __n);
	}
	else
	{
		ret = ipcom_listen(_sock._fd, __n);
	}
	_OS_DEBUG_DETAIL("socket listen: %s" ,ipstack_sockstr(_sock));
	return ret;
#else
	int ret = listen(_sock._fd, __n);
	_OS_DEBUG_DETAIL("socket listen: %s" ,ipstack_sockstr(_sock));
	return ret;
#endif
}
/* Await a connection on socket FD.
   When a connection arrives, open a new socket to communicate with it,
   set *ADDR (which is *ADDR_LEN bytes long) to the address of the connecting
   peer and *ADDR_LEN to the address's actual length, and return the
   new socket's descriptor, or -1 for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
zpl_socket_t ipstack_accept (zpl_socket_t _sock, void * __addr,
		   socklen_t * __addr_len)
{
	if(ipstack_invalid(_sock))
	{
		zpl_socket_t retaa;
		return retaa;
	}
#ifdef ZPL_IPCOM_MODULE
	zpl_socket_t ret;
	ipstack_bzero(ret);
	ret.stack = _sock.stack;
	if(_sock.stack == OS_STACK)
	{
		ret._fd = accept(_sock._fd, __addr, __addr_len);
	}
	else
	{
		ret._fd = ipcom_accept(_sock._fd, __addr, __addr_len);
	}
	_OS_DEBUG_DETAIL("socket accept: %s" ,ipstack_sockstr(_sock));
	_OS_DEBUG_DETAIL("socket accept return:%s" ,ipstack_sockstr(ret));
	return ret;
#else
	zpl_socket_t ret;
	ipstack_bzero(ret);
	ret.stack = _sock.stack;
	ret._fd = accept(_sock._fd, __addr, __addr_len);
	//os_vslog("DETAIL", __func__, __LINE__, "socket accept: %s" ,ipstack_sockstr(_sock));
	//os_vslog("DETAIL", __func__, __LINE__, "socket accept return:%s" ,ipstack_sockstr(ret));
	_OS_DEBUG_DETAIL("socket accept: %s" ,ipstack_sockstr(_sock));
	_OS_DEBUG_DETAIL("socket accept return:%s" ,ipstack_sockstr(ret));
	return ret;
#endif
}

/* Shut down all or part of the connection open on socket FD.
   HOW determines what to shut down:
     SHUT_RD   = No more receptions;
     SHUT_WR   = No more transmissions;
     SHUT_RDWR = No more receptions or transmissions.
   Returns 0 on success, -1 for errors.  */
int ipstack_shutdown (zpl_socket_t _sock, int __how)
{
	if(ipstack_invalid(_sock))
	{
		return ERROR;
	}
#ifdef ZPL_IPCOM_MODULE
	int ret = 0;
	if(_sock.stack == OS_STACK)
	{
		ret = shutdown(_sock._fd, __how);
	}
	else
	{
		ret = ipcom_shutdown(_sock._fd, __how);
	}
	_OS_DEBUG_DETAIL("socket shutdown: %s" ,ipstack_sockstr(_sock));
	return ret;
#else
	int ret = shutdown(_sock._fd, __how);
	_OS_DEBUG_DETAIL("socket shutdown: %s" ,ipstack_sockstr(_sock));
	return ret;
#endif
}

int ipstack_close (zpl_socket_t _sock)
{
#ifdef ZPL_IPCOM_MODULE
	int ret = -1;
	if(_sock.stack == OS_STACK)
	{
		if(_sock._fd)
			ret = close(_sock._fd);
	}
	else
	{
		if(_sock._fd)
			ret = ipcom_socketclose(_sock._fd);
	}
	_OS_DEBUG_DETAIL("socket close: %s" ,ipstack_sockstr(_sock));
	if(ret >= 0)
		_sock._fd = -1;
	return ret;
#else
	int ret = -1;
	_OS_DEBUG_DETAIL("socket close: %s" ,ipstack_sockstr(_sock));
	if(_sock._fd)	
		ret = close(_sock._fd);
	if(ret >= 0)
		_sock._fd = -1;
	return ret;
#endif
}

int ipstack_ioctl (zpl_socket_t _sock, unsigned long request, void *argp)
{
	if(ipstack_invalid(_sock))
	{
		return ERROR;
	}
#ifdef ZPL_IPCOM_MODULE
	int ret = 0;
	if(_sock.stack == OS_STACK)
	{
		ret = ioctl(_sock._fd, request, argp);
	}
	else
	{
		if(ipstack_invalid(_sock))
		{
			return ERROR;
		}
		ret = ipcom_socketioctl(_sock._fd, request, argp);
	}
	_OS_DEBUG_DETAIL("socket ioctl: %s" ,ipstack_sockstr(_sock));
	return ret;
#else
	int ret = ioctl(_sock._fd, request, argp);
	_OS_DEBUG_DETAIL("socket ioctl: %s" ,ipstack_sockstr(_sock));
	return ret;
#endif
}

int ipstack_write (zpl_socket_t _sock, void *buf, int nbytes)
{
	if(_sock.stack == OS_STACK)
		return write(_sock._fd, buf, nbytes);
	return ipstack_send(_sock, buf, nbytes, 0);
}

/*
int ip_stack_writev (int fd, void *buf, int nbytes)
{
#ifdef ZPL_IPCOM_MODULE
	return ipcom_socketwritev(fd, buf, nbytes);
#else
	return writev(fd, buf, nbytes);
#endif
}
*/
int ipstack_writev (zpl_socket_t _sock, struct ipstack_iovec *iov, int iovlen)
{
#ifdef ZPL_IPCOM_MODULE
	int ret = 0;
	if(_sock.stack == OS_STACK)
	{
		ret = writev(_sock._fd, iov, iovlen);
	}
	else
	{
		if(ipstack_invalid(_sock))
		{
			return ERROR;
		}
		ret = ipcom_socketwritev(_sock._fd, iov, iovlen);
	}
	_OS_DEBUG_DETAIL("socket writev: %s" ,ipstack_sockstr(_sock));
	return ret;
#else
	int ret = writev(_sock._fd, iov, iovlen);
	_OS_DEBUG_DETAIL("socket writev: %s" ,ipstack_sockstr(_sock));
	return ret;
#endif
}

int ipstack_read (zpl_socket_t _sock, void *buf, int nbytes)
{
	if(_sock.stack == OS_STACK)
		return read(_sock._fd, buf, nbytes);
	return ipstack_recv(_sock, buf, nbytes, 0);
}

/*
int ip_stack_read (int fd, void *buf, int nbytes)
{
#ifdef ZPL_IPCOM_MODULE
	return ipcom_recv(fd, buf, nbytes, 0);
#else
	return read(fd, buf, nbytes);
#endif
}
*/

int ipstack_socketselect (zpl_ipstack stack, int width, ipstack_fd_set *rfds, ipstack_fd_set *wfds, ipstack_fd_set *exfds, struct zpl_timeval *tmo)
{
#ifdef ZPL_IPCOM_MODULE
	int ret = 0;
	if(stack == OS_STACK)
	{
		ret = select(width, rfds, wfds, exfds, tmo);
	}
	else
	{
		if(ipstack_invalid(_sock))
		{
			return ERROR;
		}
		ret = ipcom_socketselect(width, rfds, wfds, exfds, tmo);
	}
	return ret;
#else
	return select(width, rfds, wfds, exfds, tmo);
#endif
}


