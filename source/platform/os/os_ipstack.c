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

#ifdef ZPL_SOCKET_T_POINT
#define IPSTACK_CHECK(n, v)	if(n == NULL) {return v;}
#else
#define IPSTACK_CHECK(n, v)	
#endif

#ifndef ZPL_SOCKET_T_POINT
zpl_socket_t _ipstack_invalid_sock = {0, IPSTACK_UNKNOEW};
#endif

char * ipstack_sockstr(zpl_socket_t _sock)
{
	static char buf[64];
	os_bzero(buf, sizeof(buf));
	char *bustr[] = {"UN","OS", "IPNET"};
	if(ipstack_type(_sock) > IPSTACK_IPCOM)
		ipstack_type(_sock) = 0;
	snprintf(buf, sizeof(buf), "%s-%d", bustr[ipstack_type(_sock)], ipstack_fd(_sock));
	return buf;
}

zpl_bool ipstack_invalid(zpl_socket_t _sock)
{
#ifdef ZPL_SOCKET_T_POINT	
	if(_sock == NULL)
		return zpl_true;
#endif		
#ifdef ZPL_IPCOM_MODULE
	if(ipstack_type(_sock) == IPSTACK_OS)
	{
		if(ipstack_fd(_sock) < 0)
			return zpl_true;
		return zpl_false;	
	}
	else if(ipstack_type(_sock) == IPSTACK_IPCOM)
	{
		if(ipstack_fd(_sock) < 0)
			return zpl_true;
		return zpl_false;	
	}
	return zpl_true;
#else
	/*if(ipstack_fd(_sock) >= 3 && ipstack_type(_sock) == IPSTACK_OS && ipstack_fd(_sock) != ERROR)
		return 0;
	*/
	if(ipstack_fd(_sock) < 0 || (ipstack_type(_sock) != IPSTACK_OS && ipstack_type(_sock) != IPSTACK_IPCOM))
		return zpl_true;	
	return zpl_false;	
#endif
}

#ifdef ZPL_SOCKET_T_POINT
int ipstack_bzero(zpl_socket_t _sock)
{
	IPSTACK_CHECK(_sock, OK);
	ipstack_type(_sock) = 0;
	ipstack_fd(_sock) = ERROR;
	return OK;
}
#else
int ipstack_bzero_pst(zpl_socket_t *_sock)
{
	IPSTACK_CHECK(_sock, OK);
	_sock->stack = 0;
	_sock->_fd = ERROR;
	return OK;
}
#endif

zpl_socket_t ipstack_create(zpl_ipstack stack)
{
#ifdef ZPL_SOCKET_T_POINT	
	zpl_socket_t tmp = malloc(sizeof(zpl_socket_t));
	if(tmp)
	{
		ipstack_type(tmp) = stack;
		ipstack_fd(tmp) = ERROR;
	}
#else
	zpl_socket_t tmp;
	ipstack_type(tmp) = stack;
	ipstack_fd(tmp) = ERROR;	
#endif	
	return tmp;
}

int ipstack_drstroy(zpl_socket_t _sock)
{
#ifdef ZPL_SOCKET_T_POINT	
	if(_sock)
	{
		free(_sock);
		_sock = NULL;
	}
#endif	
	return OK;
}
zpl_socket_t ipstack_init(zpl_ipstack stack, int _sock)
{
#ifdef ZPL_SOCKET_T_POINT	
	zpl_socket_t tmp = ipstack_create(stack);
	if(tmp)
	{
		ipstack_fd(tmp) = _sock;
	}
#else
	zpl_socket_t tmp;
	ipstack_type(tmp) = stack;
	ipstack_fd(tmp) = ERROR;		
#endif	
	return tmp;
}



int ipstack_get_blocking(zpl_socket_t _sock)
{
	if(ipstack_invalid(_sock))
		return ERROR;
#ifdef ZPL_IPCOM_MODULE
	if(ipstack_type(_sock) == IPSTACK_OS)
	{
		return os_get_blocking(ipstack_fd(_sock));
	}
	else
	{
		int a = 0;
		if (ipcom_socketioctl(ipstack_fd(_sock), IP_X_FIOGNBIO, (char *)&a) != 0)
		{
			return ERROR;
		}
		return a;
	}
	return OK;
#else
	return os_get_blocking(ipstack_fd(_sock));	
#endif
}


int ipstack_set_nonblocking(zpl_socket_t _sock)
{
	if(ipstack_invalid(_sock))
		return ERROR;
#ifdef ZPL_IPCOM_MODULE
	if(ipstack_type(_sock) == IPSTACK_OS)
	{
		return os_set_nonblocking(ipstack_fd(_sock));
	}
	else
	{
		int a = 0;
		if (ipcom_socketioctl(ipstack_fd(_sock), IP_FIONBIO, (char *)&a) != 0)
		{
			return ERROR;
		}
		return OK;
	}
	return ERROR;
#else
	return os_set_nonblocking(ipstack_fd(_sock));	
#endif
}

int ipstack_set_blocking(zpl_socket_t _sock)
{
	if(ipstack_invalid(_sock))
		return ERROR;
#ifdef ZPL_IPCOM_MODULE
	if(ipstack_type(_sock) == IPSTACK_OS)
	{
		return os_set_blocking(ipstack_fd(_sock));
	}
	else
	{
		int a = 1;
		if (ipcom_socketioctl(ipstack_fd(_sock), IPSTACK_FIONBIO, (char *)&a) != 0)
		{
			return ERROR;
		}
		return OK;
	}
	return ERROR;
#else
	return os_set_blocking(ipstack_fd(_sock));	
#endif
}

zpl_bool ipstack_same(zpl_socket_t src, zpl_socket_t dst)
{
	IPSTACK_CHECK(src, zpl_false);
	IPSTACK_CHECK(dst, zpl_false);
	if((ipstack_type(src)==IPSTACK_OS || ipstack_type(src) == IPSTACK_IPCOM) && 
		ipstack_type(src) == ipstack_type(dst) && ipstack_fd(src) > 0 && ipstack_fd(src) == ipstack_fd(dst))
		return zpl_true;
	return zpl_false;	
}

#ifdef ZPL_SOCKET_T_POINT
int ipstack_copy(zpl_socket_t src, zpl_socket_t dst)
{
	IPSTACK_CHECK(src, ERROR);
	IPSTACK_CHECK(dst, ERROR);
	ipstack_fd(dst) = ipstack_fd(src);
	ipstack_type(dst) = ipstack_type(src);
	return OK;
}
#else
int ipstack_copy_pst(zpl_socket_t *src, zpl_socket_t *dst)
{
	IPSTACK_CHECK(src, ERROR);
	IPSTACK_CHECK(dst, ERROR);
	(dst->_fd) = (src->_fd);
	dst->stack = src->stack;
	return OK;
}
#endif


zpl_socket_t ipstack_max(zpl_socket_t src, zpl_socket_t dst)
{
	if(ipstack_fd(dst) > ipstack_fd(src))
		return dst;
	else
		return src;	
}

zpl_socket_t ipstack_open (zpl_ipstack stack, const char *__path, int __oflag)
{
	zpl_socket_t tmp = ipstack_create(stack);
	IPSTACK_CHECK(tmp, tmp);
	ipstack_fd(tmp) = open(__path, __oflag);
	if(ipstack_fd(tmp) <= 0)
	{
		ipstack_type(tmp) = 0;
		ipstack_drstroy(tmp);
	}
	return tmp;
}
/* Create a new socket of type TYPE in domain DOMAIN, using
   protocol PROTOCOL.  If PROTOCOL is zero, one is chosen automatically.
   Returns a file descriptor for the new socket, or ERROR for errors.  */
zpl_socket_t ipstack_socket (zpl_ipstack stack, int __domain, int __type, int __protocol) 
{
#ifdef ZPL_IPCOM_MODULE
	zpl_socket_t _sock = ipstack_create(stack);
	IPSTACK_CHECK(_sock, _sock);	
	if(stack == IPSTACK_OS)
	{
		ipstack_fd(_sock) = socket(__domain, __type, __protocol);
	}
	else
	{
		ipstack_fd(_sock) = ipcom_socket(__domain, __type, __protocol);
	}
	OSSTACK_DEBUG_DETAIL("socket create: %s" ,ipstack_sockstr(_sock));
	if(ipstack_invalid(_sock))
	{
		ipstack_drstroy(_sock);
		#ifdef ZPL_SOCKET_T_POINT
		_sock = NULL;
		#endif
		return _sock;	
	}
	return _sock;
#else
	zpl_socket_t _sock = ipstack_create(IPSTACK_OS);
	IPSTACK_CHECK(_sock, _sock);	
	ipstack_fd(_sock) = socket(__domain, __type, __protocol);
	OSSTACK_DEBUG_DETAIL("socket create: %s" ,ipstack_sockstr(_sock));
	if(ipstack_invalid(_sock))
	{
		ipstack_drstroy(_sock);
		#ifdef ZPL_SOCKET_T_POINT
		_sock = NULL;
		#endif
		return _sock;	
	}
	return _sock;
#endif
}
/* Create a ipstack_socket for the VRF. */
zpl_socket_t ipstack_socket_vrf(zpl_ipstack stack, int domain, zpl_uint32 type, zpl_uint16 protocol, vrf_id_t vrf_id)
{
	int ret = 0;
	vrf_id_t vrf = vrf_id;
	zpl_socket_t _sock;
	_sock = ipstack_socket (stack, domain, type, protocol);
	IPSTACK_CHECK(_sock, _sock);
#ifdef ZPL_IPCOM_MODULE
	if(stack == IPSTACK_OS)
	{
		ret = setsockopt (ipstack_fd(_sock), IPSTACK_SOL_SOCKET, IPSTACK_SO_MARK, &vrf, sizeof (vrf));
		if(ret != 0)
		{
			ipstack_close(_sock);
			ipstack_drstroy(_sock);
			#ifdef ZPL_SOCKET_T_POINT
			_sock = NULL;
			#endif
		}
	}
	else
	{
		ret = ipcom_setsockopt (ipstack_fd(_sock), IPSTACK_SOL_SOCKET, IP_SO_X_VR, &vrf, sizeof (vrf));
		if(ret != 0)
		{
			ipstack_close(_sock);
			ipstack_drstroy(_sock);
			#ifdef ZPL_SOCKET_T_POINT
			_sock = NULL;
			#endif
		}
	}
#else
	ret = setsockopt (ipstack_fd(_sock), IPSTACK_SOL_SOCKET, IPSTACK_SO_MARK, &vrf, sizeof (vrf));
#endif
	if(ret == 0)
	{
		return _sock;
	}
	ipstack_close(_sock);
	ipstack_drstroy(_sock);
	#ifdef ZPL_SOCKET_T_POINT
	_sock = NULL;
	#endif
	return _sock;
}

/* Create two new sockets, of type TYPE in domain DOMAIN and using
   protocol PROTOCOL, which are connected to each other, and put file
   descriptors for them in FDS[0] and FDS[1].  If PROTOCOL is zero,
   one will be chosen automatically.  Returns 0 on success, ERROR for errors.  */
int ipstack_socketpair (zpl_ipstack stack, int __domain, int __type, int __protocol,
		       zpl_socket_t _socks[2]) 
{
#ifdef ZPL_IPCOM_MODULE
	int ret = 0, mfds[2];
	_socks[0] = ipstack_create(stack);
	_socks[1] = ipstack_create(stack);
#ifdef ZPL_SOCKET_T_POINT
	if(_socks[0] == NULL)
	{
		ipstack_drstroy(_socks[0]);
		ipstack_drstroy(_socks[1]);
		_socks[0] = NULL;
		_socks[1] = NULL;
		return ERROR;
	}
	if(_socks[1] == NULL)
	{
		ipstack_drstroy(_socks[0]);
		ipstack_drstroy(_socks[1]);
		_socks[0] = NULL;
		_socks[1] = NULL;
		return ERROR;
	}
#endif	
	if(stack == IPSTACK_OS)
	{
		ret = socketpair(__domain, __type, __protocol, mfds);
		ipstack_fd(_socks[0]) = mfds[0];
		ipstack_fd(_socks[1]) = mfds[1];
	}
	else
	{
		ret = ipcom_socketpair(__domain, __type, __protocol, mfds);
		ipstack_fd(_socks[0]) = mfds[0];
		ipstack_fd(_socks[1]) = mfds[1];
	}
	OSSTACK_DEBUG_DETAIL("socket socketpair: %s" ,ipstack_sockstr(_socks[0]));
	if(ipstack_invalid(_socks[0]))
	{
		ipstack_drstroy(_socks[0]);
		ipstack_drstroy(_socks[1]);
		#ifdef ZPL_SOCKET_T_POINT
		_socks[0] = NULL;
		_socks[1] = NULL;
		#endif
	}
	if(ipstack_invalid(_socks[1]))
	{
		ipstack_drstroy(_socks[0]);
		ipstack_drstroy(_socks[1]);
		#ifdef ZPL_SOCKET_T_POINT
		_socks[0] = NULL;
		_socks[1] = NULL;
		#endif
	}
	return ret;
#else
	int ret = 0, mfds[2];
	_socks[0] = ipstack_create(stack);
	_socks[1] = ipstack_create(stack);
#ifdef ZPL_SOCKET_T_POINT
	if(_socks[0] == NULL)
	{
		ipstack_drstroy(_socks[0]);
		ipstack_drstroy(_socks[1]);
		_socks[0] = NULL;
		_socks[1] = NULL;
		return ERROR;
	}
	if(_socks[1] == NULL)
	{
		ipstack_drstroy(_socks[0]);
		ipstack_drstroy(_socks[1]);
		_socks[0] = NULL;
		_socks[1] = NULL;
		return ERROR;
	}
#endif	
	ret = socketpair(__domain, __type, __protocol, mfds);
	ipstack_fd(_socks[0]) = mfds[0];
	ipstack_fd(_socks[1]) = mfds[1];
	OSSTACK_DEBUG_DETAIL("socket socketpair: %s" ,ipstack_sockstr(_socks[0]));
	if(ipstack_invalid(_socks[0]))
	{
		ipstack_drstroy(_socks[0]);
		ipstack_drstroy(_socks[1]);
		#ifdef ZPL_SOCKET_T_POINT
		_socks[0] = NULL;
		_socks[1] = NULL;
		#endif
	}
	if(ipstack_invalid(_socks[1]))
	{
		ipstack_drstroy(_socks[0]);
		ipstack_drstroy(_socks[1]);
		#ifdef ZPL_SOCKET_T_POINT
		_socks[0] = NULL;
		_socks[1] = NULL;
		#endif
	}
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
	if(ipstack_type(_sock) == IPSTACK_OS)
	{
		ret = bind(ipstack_fd(_sock), __addr, __len);
	}
	else
	{
		ret = ipcom_bind(ipstack_fd(_sock), __addr, __len);
	}
	OSSTACK_DEBUG_DETAIL("socket bind: %s" ,ipstack_sockstr(_socks));
	return ret;
#else
	int ret = bind(ipstack_fd(_sock), __addr, __len);
	OSSTACK_DEBUG_DETAIL("socket bind: %s" ,ipstack_sockstr(_sock));
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
	if(ipstack_type(_sock) == IPSTACK_OS)
	{
		ret = getsockname(ipstack_fd(_sock), __addr, __len);
	}
	else
	{
		ret = ipcom_getsockname(ipstack_fd(_sock), __addr, __len);
	}
	OSSTACK_DEBUG_DETAIL("socket getsockname: %s" ,ipstack_sockstr(_sock));
	return ret;
#else
	int ret = getsockname(ipstack_fd(_sock), __addr, __len);
	OSSTACK_DEBUG_DETAIL("socket getsockname: %s" ,ipstack_sockstr(_sock));
	return ret;
#endif
}
/* Open a connection on socket FD to peer at ADDR (which LEN bytes long).
   For connectionless socket types, just set the default address to send to
   and the only address from which to accept transmissions.
   Return 0 on success, ERROR for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
int ipstack_connect (zpl_socket_t _sock, void * __addr, socklen_t __len)
{
	if(ipstack_invalid(_sock))
		return ERROR;
#ifdef ZPL_IPCOM_MODULE
	int ret = 0;
	if(ipstack_type(_sock) == IPSTACK_OS)
	{
		ret = connect(ipstack_fd(_sock), __addr, __len);
	}
	else
	{
		ret = ipcom_connect(ipstack_fd(_sock), __addr, __len);
	}
	OSSTACK_DEBUG_DETAIL("socket connect: %s" ,ipstack_sockstr(_sock));
	return ret;
#else
	int ret = connect(ipstack_fd(_sock), __addr, __len);
	OSSTACK_DEBUG_DETAIL("socket connect: %s" ,ipstack_sockstr(_sock));
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
	if(ipstack_type(_sock) == IPSTACK_OS)
	{
		ret = getpeername(ipstack_fd(_sock), __addr, __len);
	}
	else
	{
		ret = ipcom_getpeername(ipstack_fd(_sock), __addr, __len);
	}
	OSSTACK_DEBUG_DETAIL("socket getpeername: %s" ,ipstack_sockstr(_sock));
	return ret;
#else
	int ret = getpeername(ipstack_fd(_sock), __addr, __len);
	OSSTACK_DEBUG_DETAIL("socket getpeername: %s" ,ipstack_sockstr(_sock));
	return ret;
#endif
}

/* Send N bytes of BUF to socket FD.  Returns the number sent or ERROR.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
ssize_t ipstack_send (zpl_socket_t _sock, const void *__buf, size_t __n, int __flags)
{
	if(ipstack_invalid(_sock))
		return ERROR;
#ifdef ZPL_IPCOM_MODULE
	int ret = 0;
	if(ipstack_type(_sock) == IPSTACK_OS)
	{
		ret = send(ipstack_fd(_sock), __buf, __n, __flags);
	}
	else
	{
		ret = ipcom_send(ipstack_fd(_sock), __buf, __n, __flags);
	}
	OSSTACK_DEBUG_DETAIL("socket send: %s" ,ipstack_sockstr(_sock));
	return ret;
#else
	int ret = send(ipstack_fd(_sock), __buf, __n, __flags);
	OSSTACK_DEBUG_DETAIL("socket send: %s" ,ipstack_sockstr(_sock));
	return ret;
#endif
}
/* Read N bytes into BUF from socket FD.
   Returns the number read or ERROR for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
ssize_t ipstack_recv (zpl_socket_t _sock, void *__buf, size_t __n, int __flags)
{
	if(ipstack_invalid(_sock))
		return ERROR;
#ifdef ZPL_IPCOM_MODULE
	int ret = 0;
	if(ipstack_type(_sock) == IPSTACK_OS)
	{
		ret = recv(ipstack_fd(_sock), __buf, __n, __flags);
	}
	else
	{
		ret = ipcom_recv(ipstack_fd(_sock), __buf, __n, __flags);
	}
	OSSTACK_DEBUG_DETAIL("socket recv: %s" ,ipstack_sockstr(_sock));
	return ret;
#else
	int ret = recv(ipstack_fd(_sock), __buf, __n, __flags);
	os_log_debug("socket recv: %s" ,ipstack_sockstr(_sock));
	return ret;
#endif
}
/* Send N bytes of BUF on socket FD to peer at address ADDR (which is
   ADDR_LEN bytes long).  Returns the number sent, or ERROR for errors.

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
	if(ipstack_type(_sock) == IPSTACK_OS)
	{
		ret = sendto(ipstack_fd(_sock), __buf, __n, __flags, __addr, __addr_len);
	}
	else
	{
		ret = ipcom_sendto(ipstack_fd(_sock), __buf, __n, __flags, __addr, __addr_len);
	}
	OSSTACK_DEBUG_DETAIL("socket sendto: %s" ,ipstack_sockstr(_sock));
	return ret;
#else
	int ret = sendto(ipstack_fd(_sock), __buf, __n, __flags, __addr, __addr_len);
	OSSTACK_DEBUG_DETAIL("socket sendto: %s" ,ipstack_sockstr(_sock));
	return ret;
#endif
}
/* Read N bytes into BUF through socket FD.
   If ADDR is not NULL, fill in *ADDR_LEN bytes of it with tha address of
   the sender, and store the actual size of the address in *ADDR_LEN.
   Returns the number of bytes read or ERROR for errors.

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
	if(ipstack_type(_sock) == IPSTACK_OS)
	{
		ret = recvfrom(ipstack_fd(_sock), __buf, __n, __flags, __addr, __addr_len);
	}
	else
	{
		ret = ipcom_recvfrom(ipstack_fd(_sock), __buf, __n, __flags, __addr, __addr_len);
	}
	OSSTACK_DEBUG_DETAIL("socket recvfrom: %s" ,ipstack_sockstr(_sock));
	return ret;
#else
	int ret = recvfrom(ipstack_fd(_sock), __buf, __n, __flags, __addr, __addr_len);
	OSSTACK_DEBUG_DETAIL("socket recvfrom: %s" ,ipstack_sockstr(_sock));
	return ret;
#endif
}

/* Send a message described MESSAGE on socket FD.
   Returns the number of bytes sent, or ERROR for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
ssize_t ipstack_sendmsg (zpl_socket_t _sock, const struct ipstack_msghdr *__message,
			int __flags)
{
	if(ipstack_invalid(_sock))
		return ERROR;
#ifdef ZPL_IPCOM_MODULE
	int ret = 0;
	if(ipstack_type(_sock) == IPSTACK_OS)
	{
		ret = sendmsg(ipstack_fd(_sock), __message, __flags);
	}
	else
	{
		ret = ipcom_sendmsg(ipstack_fd(_sock), __message, __flags);
	}
	OSSTACK_DEBUG_DETAIL("socket sendmsg: %s" ,ipstack_sockstr(_sock));
	return ret;
#else
	int ret = sendmsg(ipstack_fd(_sock), __message, __flags);
	OSSTACK_DEBUG_DETAIL("socket sendmsg: %s" ,ipstack_sockstr(_sock));
	return ret;
#endif
}
/* Receive a message as described by MESSAGE from socket FD.
   Returns the number of bytes read or ERROR for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
ssize_t ipstack_recvmsg (zpl_socket_t _sock, struct ipstack_msghdr *__message, int __flags)
{
	if(ipstack_invalid(_sock))
		return ERROR;
#ifdef ZPL_IPCOM_MODULE
	int ret = 0;
	if(ipstack_type(_sock) == IPSTACK_OS)
	{
		ret = recvmsg(ipstack_fd(_sock), __message, __flags);
	}
	else
	{
		ret = ipcom_recvmsg(ipstack_fd(_sock), __message, __flags);
	}
	OSSTACK_DEBUG_DETAIL("socket recvmsg: %s" ,ipstack_sockstr(_sock));
	return ret;
#else
	int ret = recvmsg(ipstack_fd(_sock), __message, __flags);
	OSSTACK_DEBUG_DETAIL("socket recvmsg: %s" ,ipstack_sockstr(_sock));
	return ret;
#endif
}


/* Put the current value for socket FD's option OPTNAME at protocol level LEVEL
   into OPTVAL (which is *OPTLEN bytes long), and set *OPTLEN to the value's
   actual length.  Returns 0 on success, ERROR for errors.  */
int ipstack_getsockopt (zpl_socket_t _sock, int __level, int __optname,
		       void * __optval,
		       socklen_t * __optlen)
{
	if(ipstack_invalid(_sock))
		return ERROR;
#ifdef ZPL_IPCOM_MODULE
	int ret = 0;
	if(ipstack_type(_sock) == IPSTACK_OS)
	{
		ret = getsockopt(ipstack_fd(_sock), __level, __optname, __optval, __optlen);
	}
	else
	{
		ret = ipcom_getsockopt(ipstack_fd(_sock), __level, __optname, __optval, __optlen);
	}
	OSSTACK_DEBUG_DETAIL("socket getsockopt: %s" ,ipstack_sockstr(_sock));
	return ret;
#else
	int ret = getsockopt(ipstack_fd(_sock), __level, __optname, __optval, __optlen);
	OSSTACK_DEBUG_DETAIL("socket getsockopt: %s" ,ipstack_sockstr(_sock));
	return ret;
#endif
}
/* Set socket FD's option OPTNAME at protocol level LEVEL
   to *OPTVAL (which is OPTLEN bytes long).
   Returns 0 on success, ERROR for errors.  */
int ipstack_setsockopt (zpl_socket_t _sock, int __level, int __optname,
		       const void *__optval, socklen_t __optlen)
{
	if(ipstack_invalid(_sock))
		return ERROR;
#ifdef ZPL_IPCOM_MODULE
	int ret = 0;
	if(ipstack_type(_sock) == IPSTACK_OS)
	{
		ret = setsockopt(ipstack_fd(_sock), __level, __optname, __optval, __optlen);
	}
	else
	{
		ret = ipcom_setsockopt(ipstack_fd(_sock), __level, __optname, __optval, __optlen);
	}
	OSSTACK_DEBUG_DETAIL("socket setsockopt: %s" ,ipstack_sockstr(_sock));
	return ret;
#else
	int ret = setsockopt(ipstack_fd(_sock), __level, __optname, __optval, __optlen);
	OSSTACK_DEBUG_DETAIL("socket setsockopt: %s" ,ipstack_sockstr(_sock));
	return ret;
#endif
}

/* Prepare to accept connections on socket FD.
   N connection requests will be queued before further requests are refused.
   Returns 0 on success, ERROR for errors.  */
int ipstack_listen (zpl_socket_t _sock, int __n) 
{
	if(ipstack_invalid(_sock))
		return ERROR;
#ifdef ZPL_IPCOM_MODULE
	int ret = 0;
	if(ipstack_type(_sock) == IPSTACK_OS)
	{
		ret = listen(ipstack_fd(_sock), __n);
	}
	else
	{
		ret = ipcom_listen(ipstack_fd(_sock), __n);
	}
	OSSTACK_DEBUG_DETAIL("socket listen: %s" ,ipstack_sockstr(_sock));
	return ret;
#else
	int ret = listen(ipstack_fd(_sock), __n);
	OSSTACK_DEBUG_DETAIL("socket listen: %s" ,ipstack_sockstr(_sock));
	return ret;
#endif
}
/* Await a connection on socket FD.
   When a connection arrives, open a new socket to communicate with it,
   set *ADDR (which is *ADDR_LEN bytes long) to the address of the connecting
   peer and *ADDR_LEN to the address's actual length, and return the
   new socket's descriptor, or ERROR for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
zpl_socket_t ipstack_accept (zpl_socket_t _sock, void * __addr,
		   socklen_t * __addr_len)
{
	if(ipstack_invalid(_sock))
	{
		zpl_socket_t retaa;
		#ifdef ZPL_SOCKET_T_POINT
		retaa = NULL;
		#endif
		return retaa;
	}
#ifdef ZPL_IPCOM_MODULE
	zpl_socket_t ret = ipstack_create(ipstack_type(_sock));
	IPSTACK_CHECK(ret, ret);

	if(ipstack_type(_sock) == IPSTACK_OS)
	{
		ipstack_fd(ret) = accept(ipstack_fd(_sock), __addr, __addr_len);
	}
	else
	{
		ipstack_fd(ret) = ipcom_accept(ipstack_fd(_sock), __addr, __addr_len);
	}
	OSSTACK_DEBUG_DETAIL("socket accept: %s" ,ipstack_sockstr(_sock));
	OSSTACK_DEBUG_DETAIL("socket accept return:%s" ,ipstack_sockstr(ret));
	if(ipstack_invalid(ret))
	{
		ipstack_drstroy(ret);
		#ifdef ZPL_SOCKET_T_POINT
		ret = NULL;
		#endif
	}
	return ret;
#else
	zpl_socket_t ret = ipstack_create(ipstack_type(_sock));
	IPSTACK_CHECK(ret, ret);
	ipstack_fd(ret) = accept(ipstack_fd(_sock), __addr, __addr_len);
	OSSTACK_DEBUG_DETAIL("socket accept: %s" ,ipstack_sockstr(_sock));
	OSSTACK_DEBUG_DETAIL("socket accept return:%s" ,ipstack_sockstr(ret));
	if(ipstack_invalid(ret))
	{
		ipstack_drstroy(ret);
		#ifdef ZPL_SOCKET_T_POINT
		ret = NULL;
		#endif
	}
	return ret;
#endif
}

/* Shut down all or part of the connection open on socket FD.
   HOW determines what to shut down:
     SHUT_RD   = No more receptions;
     SHUT_WR   = No more transmissions;
     SHUT_RDWR = No more receptions or transmissions.
   Returns 0 on success, ERROR for errors.  */
int ipstack_shutdown (zpl_socket_t _sock, int __how)
{
	if(ipstack_invalid(_sock))
	{
		return ERROR;
	}
#ifdef ZPL_IPCOM_MODULE
	int ret = 0;
	if(ipstack_type(_sock) == IPSTACK_OS)
	{
		ret = shutdown(ipstack_fd(_sock), __how);
	}
	else
	{
		ret = ipcom_shutdown(ipstack_fd(_sock), __how);
	}
	OSSTACK_DEBUG_DETAIL("socket shutdown: %s" ,ipstack_sockstr(_sock));
	return ret;
#else
	int ret = shutdown(ipstack_fd(_sock), __how);
	OSSTACK_DEBUG_DETAIL("socket shutdown: %s" ,ipstack_sockstr(_sock));
	return ret;
#endif
}


#ifdef ZPL_SOCKET_T_POINT
int ipstack_close (zpl_socket_t _sock)
{
	if(ipstack_invalid(_sock))
	{
		return ERROR;
	}
#ifdef ZPL_IPCOM_MODULE
	int ret = ERROR;
	if(ipstack_type(_sock) == IPSTACK_OS)
	{
		if(ipstack_fd(_sock)>0)
			ret = close(ipstack_fd(_sock));
	}
	else
	{
		if(ipstack_fd(_sock)>0)
			ret = ipcom_socketclose(ipstack_fd(_sock));
	}
	OSSTACK_DEBUG_DETAIL("socket close: %s" ,ipstack_sockstr(*_sock));
	if(ret >= 0)
		ipstack_fd(_sock) = ERROR;
	ipstack_drstroy(_sock);	
	#ifdef ZPL_SOCKET_T_POINT
	_sock = NULL;
	#endif
	return ret;
#else
	int ret = ERROR;
	OSSTACK_DEBUG_DETAIL("socket close: %s" ,ipstack_sockstr(_sock));
	if(ipstack_fd(_sock) > 0)	
		ret = close(ipstack_fd(_sock));
	if(ret >= 0)
	{
		ipstack_fd(_sock) = ERROR;
	}
	OSSTACK_DEBUG_DETAIL("socket == close: %s" ,ipstack_sockstr(_sock));
	ipstack_drstroy(_sock);
	#ifdef ZPL_SOCKET_T_POINT
	_sock = NULL;
	#endif
	return ret;
#endif
}
#else
int ipstack_close_pst (zpl_socket_t *_sock)
{
	if(ipstack_invalid(*_sock))
	{
		return ERROR;
	}
#ifdef ZPL_IPCOM_MODULE
	int ret = ERROR;
	if(ipstack_type(_sock) == IPSTACK_OS)
	{
		if(ipstack_fd(_sock)>0)
			ret = close(ipstack_fd(_sock));
	}
	else
	{
		if(ipstack_fd(_sock)>0)
			ret = ipcom_socketclose(ipstack_fd(_sock));
	}
	OSSTACK_DEBUG_DETAIL("socket close: %s" ,ipstack_sockstr(*_sock));
	if(ret >= 0)
		ipstack_fd(_sock) = ERROR;
	ipstack_drstroy(_sock);	
	return ret;
#else
	int ret = ERROR;
	OSSTACK_DEBUG_DETAIL("socket close: %s" ,ipstack_sockstr(*_sock));
	if((_sock->_fd) > 0)	
		ret = close((_sock->_fd));
	if(ret >= 0)
	{
		(_sock->_fd) = ERROR;
	}
	OSSTACK_DEBUG_DETAIL("socket == close: %s" ,ipstack_sockstr(*_sock));
	ipstack_drstroy(*_sock);
	return ret;
#endif
}
#endif


int ipstack_ioctl (zpl_socket_t _sock, unsigned long request, void *argp)
{
	if(ipstack_invalid(_sock))
	{
		return ERROR;
	}
#ifdef ZPL_IPCOM_MODULE
	int ret = 0;
	if(ipstack_type(_sock) == IPSTACK_OS)
	{
		ret = ioctl(ipstack_fd(_sock), request, argp);
	}
	else
	{
		if(ipstack_invalid(_sock))
		{
			return ERROR;
		}
		ret = ipcom_socketioctl(ipstack_fd(_sock), request, argp);
	}
	OSSTACK_DEBUG_DETAIL("socket ioctl: %s" ,ipstack_sockstr(_sock));
	return ret;
#else
	int ret = ioctl(ipstack_fd(_sock), request, argp);
	OSSTACK_DEBUG_DETAIL("socket ioctl: %s" ,ipstack_sockstr(_sock));
	return ret;
#endif
}

int ipstack_write (zpl_socket_t _sock, void *buf, int nbytes)
{
	if(ipstack_invalid(_sock))
	{
		return ERROR;
	}	
	if(ipstack_type(_sock) == IPSTACK_OS)
		return write(ipstack_fd(_sock), buf, nbytes);
	return ipstack_send(_sock, buf, nbytes, 0);
}

/*
int ip_stack_writev (int _sock, void *buf, int nbytes)
{
#ifdef ZPL_IPCOM_MODULE
	return ipcom_socketwritev(_sock, buf, nbytes);
#else
	return writev(_sock, buf, nbytes);
#endif
}
*/
int ipstack_writev (zpl_socket_t _sock, struct ipstack_iovec *iov, int iovlen)
{
	if(ipstack_invalid(_sock))
	{
		return ERROR;
	}	
#ifdef ZPL_IPCOM_MODULE
	int ret = 0;
	if(ipstack_type(_sock) == IPSTACK_OS)
	{
		ret = writev(ipstack_fd(_sock), iov, iovlen);
	}
	else
	{
		if(ipstack_invalid(_sock))
		{
			return ERROR;
		}
		ret = ipcom_socketwritev(ipstack_fd(_sock), iov, iovlen);
	}
	OSSTACK_DEBUG_DETAIL("socket writev: %s" ,ipstack_sockstr(_sock));
	return ret;
#else
	int ret = writev(ipstack_fd(_sock), iov, iovlen);
	OSSTACK_DEBUG_DETAIL("socket writev: %s" ,ipstack_sockstr(_sock));
	return ret;
#endif
}

int ipstack_read (zpl_socket_t _sock, void *buf, int nbytes)
{
	if(ipstack_invalid(_sock))
	{
		return ERROR;
	}	
	if(ipstack_type(_sock) == IPSTACK_OS)
		return read(ipstack_fd(_sock), buf, nbytes);
	return ipstack_recv(_sock, buf, nbytes, 0);
}

/*
int ip_stack_read (int _sock, void *buf, int nbytes)
{
#ifdef ZPL_IPCOM_MODULE
	return ipcom_recv(_sock, buf, nbytes, 0);
#else
	return read(_sock, buf, nbytes);
#endif
}
*/

int ipstack_socketselect (zpl_ipstack stack, int width, ipstack_fd_set *rfds, ipstack_fd_set *wfds, ipstack_fd_set *exfds, struct zpl_timeval *tmo)
{
#ifdef ZPL_IPCOM_MODULE
	int ret = 0;
	if(stack == IPSTACK_OS)
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


