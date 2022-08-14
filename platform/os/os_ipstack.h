/*
 * os_ipstack.h
 *
 *  Created on: Aug 25, 2018
 *      Author: zhurish
 */

#ifndef __OS_IPSTACK_H__
#define __OS_IPSTACK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zpl_type.h"
#include "auto_include.h"

/* Create a new socket of type TYPE in domain DOMAIN, using
   protocol PROTOCOL.  If PROTOCOL is zero, one is chosen automatically.
   Returns a file descriptor for the new socket, or -1 for errors.  */
extern zpl_socket_t ipstack_socket (zpl_ipstack stack, int __domain, int __type, int __protocol) ;
extern zpl_socket_t ipstack_socket_vrf(zpl_ipstack stack, int domain, zpl_uint32 type, zpl_uint16 protocol, vrf_id_t vrf_id);
/* Create two new sockets, of type TYPE in domain DOMAIN and using
   protocol PROTOCOL, which are connected to each other, and put file
   descriptors for them in FDS[0] and FDS[1].  If PROTOCOL is zero,
   one will be chosen automatically.  Returns 0 on success, -1 for errors.  */
extern int ipstack_socketpair (zpl_ipstack stack, int __domain, int __type, int __protocol,
		       zpl_socket_t _socks[2]) ;

/* Give the socket FD the local address ADDR (which is LEN bytes long).  */
extern int ipstack_bind (zpl_socket_t _sock, void * __addr, socklen_t __len);

/* Put the local address of FD into *ADDR and its length in *LEN.  */
extern int ipstack_getsockname (zpl_socket_t _sock, void * __addr,
			socklen_t * __len) ;

/* Open a connection on socket FD to peer at ADDR (which LEN bytes long).
   For connectionless socket types, just set the default address to send to
   and the only address from which to accept transmissions.
   Return 0 on success, -1 for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern int ipstack_connect (zpl_socket_t _sock, void * __addr, socklen_t __len);

/* Put the address of the peer connected to socket FD into *ADDR
   (which is *LEN bytes long), and its actual length into *LEN.  */
extern int ipstack_getpeername (zpl_socket_t _sock, void * __addr,
			socklen_t * __len) ;


/* Send N bytes of BUF to socket FD.  Returns the number sent or -1.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern ssize_t ipstack_send (zpl_socket_t _sock, const void *__buf, size_t __n, int __flags);

/* Read N bytes into BUF from socket FD.
   Returns the number read or -1 for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern ssize_t ipstack_recv (zpl_socket_t _sock, void *__buf, size_t __n, int __flags);

/* Send N bytes of BUF on socket FD to peer at address ADDR (which is
   ADDR_LEN bytes long).  Returns the number sent, or -1 for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern ssize_t ipstack_sendto (zpl_socket_t _sock, const void *__buf, size_t __n,
		       int __flags, void * __addr,
		       socklen_t __addr_len);

/* Read N bytes into BUF through socket FD.
   If ADDR is not NULL, fill in *ADDR_LEN bytes of it with tha address of
   the sender, and store the actual size of the address in *ADDR_LEN.
   Returns the number of bytes read or -1 for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern ssize_t ipstack_recvfrom (zpl_socket_t _sock, void * __buf, size_t __n,
			 int __flags, void * __addr,
			 socklen_t * __addr_len);


/* Send a message described MESSAGE on socket FD.
   Returns the number of bytes sent, or -1 for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern ssize_t ipstack_sendmsg (zpl_socket_t _sock, const struct ipstack_msghdr *__message,
			int __flags);

/* Receive a message as described by MESSAGE from socket FD.
   Returns the number of bytes read or -1 for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern ssize_t ipstack_recvmsg (zpl_socket_t _sock, struct ipstack_msghdr *__message, int __flags);



/* Put the current value for socket FD's option OPTNAME at protocol level LEVEL
   into OPTVAL (which is *OPTLEN bytes long), and set *OPTLEN to the value's
   actual length.  Returns 0 on success, -1 for errors.  */
extern int ipstack_getsockopt (zpl_socket_t _sock, int __level, int __optname,
		       void * __optval,
		       socklen_t * __optlen);

/* Set socket FD's option OPTNAME at protocol level LEVEL
   to *OPTVAL (which is OPTLEN bytes long).
   Returns 0 on success, -1 for errors.  */
extern int ipstack_setsockopt (zpl_socket_t _sock, int __level, int __optname,
		       const void *__optval, socklen_t __optlen);


/* Prepare to accept connections on socket FD.
   N connection requests will be queued before further requests are refused.
   Returns 0 on success, -1 for errors.  */
extern int ipstack_listen (zpl_socket_t _sock, int __n) ;

/* Await a connection on socket FD.
   When a connection arrives, open a new socket to communicate with it,
   set *ADDR (which is *ADDR_LEN bytes long) to the address of the connecting
   peer and *ADDR_LEN to the address's actual length, and return the
   new socket's descriptor, or -1 for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern zpl_socket_t ipstack_accept (zpl_socket_t _sock, void * __addr,
		   socklen_t * __addr_len);


/* Shut down all or part of the connection open on socket FD.
   HOW determines what to shut down:
     SHUT_RD   = No more receptions;
     SHUT_WR   = No more transmissions;
     SHUT_RDWR = No more receptions or transmissions.
   Returns 0 on success, -1 for errors.  */
extern int ipstack_shutdown (zpl_socket_t _sock, int __how);
#ifdef ZPL_SOCKET_T_POINT
extern int ipstack_close (zpl_socket_t _sock);
#else
extern int ipstack_close_pst (zpl_socket_t *_sock);
#define ipstack_close(n)   ipstack_close_pst(&(n))
#endif
extern int ipstack_ioctl (zpl_socket_t _sock, unsigned long request, void *argp);
extern int ipstack_write (zpl_socket_t _sock, void *buf, int nbytes);
extern int ipstack_writev (zpl_socket_t _sock, struct ipstack_iovec *iov, int iovlen);
extern int ipstack_read (zpl_socket_t _sock, void *buf, int nbytes);
extern int ipstack_socketselect (zpl_ipstack stack, int width, ipstack_fd_set *rfds, ipstack_fd_set *wfds, ipstack_fd_set *exfds, struct zpl_timeval *tmo);

extern zpl_socket_t ipstack_open (zpl_ipstack stack, const char *__path, int __oflag);

extern zpl_bool ipstack_invalid(zpl_socket_t _sock);

extern zpl_socket_t ipstack_create(zpl_ipstack stack);
extern int ipstack_drstroy(zpl_socket_t _sock);
extern zpl_socket_t ipstack_init(zpl_ipstack stack, int fd);
extern zpl_bool ipstack_same(zpl_socket_t src, zpl_socket_t dst);
extern zpl_socket_t ipstack_max(zpl_socket_t src, zpl_socket_t dst);
#ifdef ZPL_SOCKET_T_POINT
extern int ipstack_bzero(zpl_socket_t _sock);
extern int ipstack_copy(zpl_socket_t src, zpl_socket_t dst);
#else
extern int ipstack_bzero_pst(zpl_socket_t *_sock);
extern int ipstack_copy_pst(zpl_socket_t *src, zpl_socket_t *dst);
#define ipstack_bzero(n)   ipstack_bzero_pst(&(n))
#define ipstack_copy(s,d)   ipstack_copy_pst(&(s), &(d))
#endif

extern int ipstack_get_blocking(zpl_socket_t _sock);
extern int ipstack_set_nonblocking(zpl_socket_t _sock);
extern int ipstack_set_blocking(zpl_socket_t _sock);

extern char * ipstack_sockstr(zpl_socket_t _sock);



#ifdef ZPL_SOCKET_T_POINT
#define ipstack_fd(n)         (((n)->_fd))
#define ipstack_type(n)       (((n)->stack))
#define is_os_stack(n)	      ((n)->stack == OS_STACK)
#define is_ipcom_stack(n)	   ((n)->stack == IPCOM_STACK)
#else
#define ipstack_fd(n)         ((n)._fd)
#define ipstack_type(n)       ((n).stack)
#define is_os_stack(n)	      ((n).stack == OS_STACK)
#define is_ipcom_stack(n)	   ((n).stack == IPCOM_STACK)
#endif
#define ipstack_closesocket	ipstack_close
#define ipstack_select	ipstack_socketselect


#ifdef GNU_LINUX
#define WANT_OSPF_WRITE_FRAGMENT
#endif

/* Does the I/O error indicate that the operation should be retried later? */
#define IPSTACK_ERRNO_RETRY(EN) \
	(((EN) == IPSTACK_ERRNO_EAGAIN) || ((EN) == IPSTACK_ERRNO_EWOULDBLOCK) || ((EN) == IPSTACK_ERRNO_EINTR))



#ifdef HAVE_BROKEN_CMSG_FIRSTHDR
/* This bug is present in Solaris 8 and pre-patch Solaris 9 <sys/socket.h>;
   please refer to http://bugzilla.quagga.net/show_bug.cgi?id=142 */

/* Check that msg_controllen is large enough. */
#define ZCMSG_FIRSTHDR(mhdr) \
  (((size_t)((mhdr)->msg_controllen) >= sizeof(struct ipstack_cmsghdr)) ? \
   IPSTACK_CMSG_FIRSTHDR(mhdr) : (struct ipstack_cmsghdr *)NULL)

#warning "IPSTACK_CMSG_FIRSTHDR is broken on this platform, using a workaround"

#else /* HAVE_BROKEN_CMSG_FIRSTHDR */
#define ZCMSG_FIRSTHDR(M) IPSTACK_CMSG_FIRSTHDR(M)
#endif /* HAVE_BROKEN_CMSG_FIRSTHDR */



/* 
 * RFC 3542 defines several macros for using struct ipstack_cmsghdr.
 * Here, we define those that are not present
 */

/*
 * Internal defines, for use only in this file.
 * These are likely wrong on other than ILP32 machines, so warn.
 */
#ifndef _CMSG_DATA_ALIGN
#define _CMSG_DATA_ALIGN(n)           (((n) + 3) & ~3)
#endif /* _CMSG_DATA_ALIGN */

#ifndef _CMSG_HDR_ALIGN
#define _CMSG_HDR_ALIGN(n)            (((n) + 3) & ~3)
#endif /* _CMSG_HDR_ALIGN */

/*
 * IPSTACK_CMSG_SPACE and IPSTACK_CMSG_LEN are required in RFC3542, but were new in that
 * version.
 */
#ifndef IPSTACK_CMSG_SPACE
#define IPSTACK_CMSG_SPACE(l)       (_CMSG_DATA_ALIGN(sizeof(struct ipstack_cmsghdr)) + \
                              _CMSG_HDR_ALIGN(l))
#warning "assuming 4-byte alignment for IPSTACK_CMSG_SPACE"
#endif  /* IPSTACK_CMSG_SPACE */


#ifndef IPSTACK_CMSG_LEN
#define IPSTACK_CMSG_LEN(l)         (_CMSG_DATA_ALIGN(sizeof(struct ipstack_cmsghdr)) + (l))
#warning "assuming 4-byte alignment for IPSTACK_CMSG_LEN"
#endif /* IPSTACK_CMSG_LEN */




/* misc include group */
#if !(defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
/* Not C99; do we need to define va_copy? */
#ifndef va_copy
#ifdef __va_copy
#define va_copy(DST,SRC) __va_copy(DST,SRC)
#else
/* Now we are desperate; this should work on many typical platforms. 
   But this is slightly dangerous, because the standard does not require
   va_copy to be a macro. */
#define va_copy(DST,SRC) memcpy(&(DST), &(SRC), sizeof(va_list))
#warning "Not C99 and no va_copy macro available, falling back to memcpy"
#endif /* __va_copy */
#endif /* !va_copy */
#endif /* !C99 */


/* 
 * IP_HDRINCL / struct ip byte order
 *
 * Linux: network byte order
 * *BSD: network, except for length and offset. (cf Stevens)
 * SunOS: nominally as per BSD. but bug: network order on LE.
 * OpenBSD: network byte order, apart from older versions which are as per 
 *          *BSD
 */
#if defined(__NetBSD__) \
   || (defined(__FreeBSD__) && (__FreeBSD_version < 1100030)) \
   || (defined(__OpenBSD__) && (OpenBSD < 200311)) \
   || (defined(__APPLE__)) \
   || (defined(SUNOS_5) && defined(WORDS_BIGENDIAN))
#define HAVE_IP_HDRINCL_BSD_ORDER
#endif


/*  The definition of struct ipstack_in_pktinfo is missing in old version of
    GLIBC 2.1 (Redhat 6.1).  */
#if defined (GNU_LINUX) && (!defined (__USE_MISC) && !defined(__UAPI_DEF_IN_PKTINFO))
struct ipstack_in_pktinfo
{
  int ipi_ifindex;
  struct ipstack_in_addr ipi_spec_dst;
  struct ipstack_in_addr ipi_addr;
};
#endif
#if defined (GNU_LINUX) && (!defined (__USE_MISC) && !defined(__UAPI_DEF_IN6_PKTINFO))
struct ipstack_in6_pktinfo
  {
    struct ipstack_in6_addr ipi6_addr;
    zpl_uint32 ipi6_ifindex;
  };
#endif



#ifdef __cplusplus
}
#endif

#endif /* __OS_IPSTACK_H__ */
