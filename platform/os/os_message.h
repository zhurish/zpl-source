/*
 * os_message.h
 *
 *  Created on: Jun 9, 2018
 *      Author: zhurish
 */

#ifndef __OS_MESSAGE_H__
#define __OS_MESSAGE_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "zpl_type.h"

/* Osmsg buffer. */
struct zpl_osmsg
{
  struct zpl_osmsg *next;
  zpl_uint32 getp; 		/* next get position */
  zpl_uint32 endp;		/* last valid data position */
  zpl_uint32 size;		/* size of data segment */
  zpl_uchar *data; /* data pointer */
};

struct zpl_osmessage
{
  zpl_socket_t	fd;
  struct zpl_osmsg	*outmsg;
  struct zpl_osmsg	*inmsg;
};

/* Osmsg prototypes. 
 * For zpl_osmsg_{put,get}S, the S suffix mean:
 *
 * c: character (unsigned byte)
 * w: word (two bytes)
 * l: long (two words)
 * q: quad (four words)
 */
extern struct zpl_osmsg *zpl_osmsg_new (zpl_uint32);
extern void zpl_osmsg_free (struct zpl_osmsg *);
extern struct zpl_osmsg * zpl_osmsg_copy (struct zpl_osmsg *, struct zpl_osmsg *src);
extern struct zpl_osmsg *zpl_osmsg_dup (struct zpl_osmsg *);
extern zpl_uint32 zpl_osmsg_resize (struct zpl_osmsg *, zpl_uint32);
extern zpl_uint32 zpl_osmsg_get_getp (struct zpl_osmsg *);
extern zpl_uint32 zpl_osmsg_get_endp (struct zpl_osmsg *);
extern zpl_uint32 zpl_osmsg_get_size (struct zpl_osmsg *);
extern zpl_uchar *zpl_osmsg_get_data (struct zpl_osmsg *);

/**
 * Create a new zpl_osmsg structure; copy offset bytes from s1 to the new
 * zpl_osmsg; copy s2 data to the new zpl_osmsg; copy rest of s1 data to the
 * new zpl_osmsg.
 */
extern struct zpl_osmsg *zpl_osmsg_dupcat(struct zpl_osmsg *s1, struct zpl_osmsg *s2,
				    zpl_uint32 offset);

extern void zpl_osmsg_set_getp (struct zpl_osmsg *, zpl_uint32);
extern void zpl_osmsg_set_endp (struct zpl_osmsg *, zpl_uint32);
extern void zpl_osmsg_forward_getp (struct zpl_osmsg *, zpl_uint32);
extern void zpl_osmsg_forward_endp (struct zpl_osmsg *, zpl_uint32);

/* steam_put: NULL source zeroes out zpl_uint32 bytes of zpl_osmsg */
extern void zpl_osmsg_put (struct zpl_osmsg *, const void *, zpl_uint32);
extern int zpl_osmsg_putc (struct zpl_osmsg *, zpl_uchar);
extern int zpl_osmsg_putc_at (struct zpl_osmsg *, zpl_uint32, zpl_uchar);
extern int zpl_osmsg_putw (struct zpl_osmsg *, zpl_uint16);
extern int zpl_osmsg_putw_at (struct zpl_osmsg *, zpl_uint32, zpl_uint16);
extern int zpl_osmsg_putl (struct zpl_osmsg *, zpl_uint32);
extern int zpl_osmsg_putl_at (struct zpl_osmsg *, zpl_uint32, zpl_uint32);
extern int zpl_osmsg_putq (struct zpl_osmsg *, zpl_uint64);
extern int zpl_osmsg_putq_at (struct zpl_osmsg *, zpl_uint32, zpl_uint64);
extern int zpl_osmsg_put_ipv4 (struct zpl_osmsg *, zpl_uint32);


extern void zpl_osmsg_get (void *, struct zpl_osmsg *, zpl_uint32);
extern zpl_uchar zpl_osmsg_getc (struct zpl_osmsg *);
extern zpl_uchar zpl_osmsg_getc_from (struct zpl_osmsg *, zpl_uint32);
extern zpl_uint16 zpl_osmsg_getw (struct zpl_osmsg *);
extern zpl_uint16 zpl_osmsg_getw_from (struct zpl_osmsg *, zpl_uint32);
extern zpl_uint32 zpl_osmsg_getl (struct zpl_osmsg *);
extern zpl_uint32 zpl_osmsg_getl_from (struct zpl_osmsg *, zpl_uint32);
extern zpl_uint64 zpl_osmsg_getq (struct zpl_osmsg *);
extern zpl_uint64 zpl_osmsg_getq_from (struct zpl_osmsg *, zpl_uint32);
extern zpl_uint32 zpl_osmsg_get_ipv4 (struct zpl_osmsg *);

/* IEEE-754 floats */
extern zpl_float zpl_osmsg_getf (struct zpl_osmsg *);
extern zpl_double zpl_osmsg_getd (struct zpl_osmsg *);
extern int zpl_osmsg_putf (struct zpl_osmsg *, zpl_float);
extern int zpl_osmsg_putd (struct zpl_osmsg *, zpl_double);


/* reset the zpl_osmsg. See Note above */
extern void zpl_osmsg_reset (struct zpl_osmsg *);
/* move unread data to start of zpl_osmsg, discarding read data */
extern void zpl_osmsg_discard (struct zpl_osmsg *);
extern int zpl_osmsg_empty (struct zpl_osmsg *); /* is the zpl_osmsg empty? */

/* deprecated */
extern zpl_uchar *zpl_osmsg_pnt (struct zpl_osmsg *);


/* Deprecated: assumes blocking I/O.  Will be removed. 
   Use zpl_osmsg_read_try instead.  */
extern int zpl_osmsg_readfd (struct zpl_osmsg *, zpl_socket_t, zpl_uint32);

/* Read up to size bytes into the zpl_osmsg.
   Return code:
     >0: number of bytes read
     0: end-of-file
     -1: fatal error
     -2: transient error, should retry later (i.e. IPSTACK_ERRNO_EAGAIN or IPSTACK_ERRNO_EINTR)
   This is suitable for use with non-blocking file descriptors.
 */
extern ssize_t zpl_osmsg_read_try(struct zpl_osmsg *s, zpl_socket_t fd, zpl_uint32 size);

extern ssize_t zpl_osmsg_recvmsg (struct zpl_osmsg *s, zpl_socket_t fd, struct ipstack_msghdr *,
                               zpl_uint32 flags, zpl_uint32 size);
extern ssize_t zpl_osmsg_recvfrom (struct zpl_osmsg *s, zpl_socket_t fd, zpl_uint32 len, 
                                zpl_uint32 flags, struct ipstack_sockaddr *from, 
                                socklen_t *fromlen);

extern int zpl_osmsg_writefd (struct zpl_osmsg *, zpl_socket_t, zpl_uint32);
extern ssize_t zpl_osmsg_write_try(struct zpl_osmsg *s, zpl_socket_t fd, zpl_uint32 size);

extern ssize_t zpl_osmsg_sendmsg (struct zpl_osmsg *s, zpl_socket_t fd, struct ipstack_msghdr *,
                               zpl_uint32 flags, zpl_uint32 size);
extern ssize_t zpl_osmsg_sendto (struct zpl_osmsg *s, zpl_socket_t fd, zpl_uint32 len, 
                                zpl_uint32 flags, struct ipstack_sockaddr *to, 
                                socklen_t *tolen);


extern int zpl_osmsg_flush (struct zpl_osmsg *, zpl_socket_t);











#ifdef __cplusplus
}
#endif

#endif /* __OS_MESSAGE_H__ */
