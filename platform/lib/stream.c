  /*
 * Packet interface
 * Copyright (C) 1999 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"

#include "stream.h"
#include "zmemory.h"
#include "network.h"
#include "prefix.h"
#include "log.h"

//#pragma  GCC diagnostic ignored error "-Werror=pointer-arith"

/* Tests whether a position is valid */ 
#define GETP_VALID(S,G) \
  ((G) <= (S)->endp)
#define PUT_AT_VALID(S,G) GETP_VALID(S,G)
#define ENDP_VALID(S,E) \
  ((E) <= (S)->size)

/* asserting sanity checks. Following must be true before
 * stream functions are called:
 *
 * Following must always be true of stream elements
 * before and after calls to stream functions:
 *
 * getp <= endp <= size
 *
 * Note that after a stream function is called following may be true:
 * if (getp == endp) then stream is no longer readable
 * if (endp == size) then stream is no longer writeable
 *
 * It is valid to put to anywhere within the size of the stream, but only
 * using stream_put..._at() functions.
 */
#define STREAM_WARN_OFFSETS(S) \
  zlog_warn (MODULE_DEFAULT, "&(struct stream): %p, size: %lu, getp: %lu, endp: %lu\n", \
             (void *)(S), \
             (zpl_ulong) (S)->size, \
             (zpl_ulong) (S)->getp, \
             (zpl_ulong) (S)->endp)\

#define STREAM_VERIFY_SANE(S) \
  do { \
    if ( !(GETP_VALID(S, (S)->getp) && ENDP_VALID(S, (S)->endp)) ) \
      STREAM_WARN_OFFSETS(S); \
    assert ( GETP_VALID(S, (S)->getp) ); \
    assert ( ENDP_VALID(S, (S)->endp) ); \
  } while (0)

#define STREAM_BOUND_WARN(S, WHAT) \
  do { \
    zlog_warn (MODULE_DEFAULT, "%s: Attempt to %s out of bounds", __func__, (WHAT)); \
    STREAM_WARN_OFFSETS(S); \
    assert (0); \
  } while (0)

/* XXX: Deprecated macro: do not use */
#define CHECK_SIZE(S, Z) \
  do { \
    if (((S)->endp + (Z)) > (S)->size) \
      { \
        zlog_warn (MODULE_DEFAULT, "CHECK_SIZE: truncating requested size %lu\n", \
                   (zpl_ulong) (Z)); \
        STREAM_WARN_OFFSETS(S); \
        (Z) = (S)->size - (S)->endp; \
      } \
  } while (0);

/* Make stream buffer. */
struct stream *
stream_new (zpl_size_t size)
{
  struct stream *s;

  assert (size > 0);
  
  if (size == 0)
    {
      zlog_warn (MODULE_DEFAULT, "stream_new(): called with 0 size!");
      return NULL;
    }
  
  s = XCALLOC (MTYPE_STREAM, sizeof (struct stream));

  if (s == NULL)
    return s;
  
  if ( (s->data = XMALLOC (MTYPE_STREAM_DATA, size)) == NULL)
    {
      XFREE (MTYPE_STREAM, s);
      return NULL;
    }
  
  s->size = size;
  return s;
}

/* Free it now. */
void
stream_free (struct stream *s)
{
  if (!s)
    return;
  
  XFREE (MTYPE_STREAM_DATA, s->data);
  XFREE (MTYPE_STREAM, s);
}

struct stream *
stream_copy (struct stream *new, struct stream *src)
{
  STREAM_VERIFY_SANE (src);
  
  assert (new != NULL);
  assert (STREAM_SIZE(new) >= src->endp);

  new->endp = src->endp;
  new->getp = src->getp;
  
  memcpy (new->data, src->data, src->endp);
  
  return new;
}

struct stream *
stream_dup (struct stream *s)
{
  struct stream *new;

  STREAM_VERIFY_SANE (s);

  if ( (new = stream_new (s->endp)) == NULL)
    return NULL;

  return (stream_copy (new, s));
}

struct stream *
stream_dupcat (struct stream *s1, struct stream *s2, zpl_size_t offset)
{
  struct stream *new;

  STREAM_VERIFY_SANE (s1);
  STREAM_VERIFY_SANE (s2);

  if ( (new = stream_new (s1->endp + s2->endp)) == NULL)
    return NULL;

  memcpy (new->data, s1->data, offset);
  memcpy (new->data + offset, s2->data, s2->endp);
  memcpy (new->data + offset + s2->endp, s1->data + offset,
	  (s1->endp - offset));
  new->endp = s1->endp + s2->endp;
  return new;
}

zpl_size_t
stream_resize (struct stream *s, zpl_size_t newsize)
{
  zpl_uchar *newdata;
  STREAM_VERIFY_SANE (s);
  
  newdata = XREALLOC (MTYPE_STREAM_DATA, s->data, newsize);
  
  if (newdata == NULL)
    return s->size;
  
  s->data = newdata;
  s->size = newsize;
  
  if (s->endp > s->size)
    s->endp = s->size;
  if (s->getp > s->endp)
    s->getp = s->endp;
  
  STREAM_VERIFY_SANE (s);
  
  return s->size;
}

zpl_size_t
stream_get_getp (struct stream *s)
{
  STREAM_VERIFY_SANE(s);
  return s->getp;
}

zpl_size_t
stream_get_endp (struct stream *s)
{
  STREAM_VERIFY_SANE(s);
  return s->endp;
}

zpl_size_t
stream_get_size (struct stream *s)
{
  STREAM_VERIFY_SANE(s);
  return s->size;
}

/* Stream structre' stream pointer related functions.  */
void
stream_set_getp (struct stream *s, zpl_size_t pos)
{
  STREAM_VERIFY_SANE(s);
  
  if (!GETP_VALID (s, pos))
    {
      STREAM_BOUND_WARN (s, "set getp");
      pos = s->endp;
    }

  s->getp = pos;
}

void
stream_set_endp (struct stream *s, zpl_size_t pos)
{
  STREAM_VERIFY_SANE(s);

  if (!ENDP_VALID(s, pos))
    {
      STREAM_BOUND_WARN (s, "set endp");
      return;
    }

  /*
   * Make sure the current read pointer is not beyond the new endp.
   */
  if (s->getp > pos)
    {
      STREAM_BOUND_WARN(s, "set endp");
      return;
    }

  s->endp = pos;
  STREAM_VERIFY_SANE(s);
}

/* Forward pointer. */
void
stream_forward_getp (struct stream *s, zpl_size_t size)
{
  STREAM_VERIFY_SANE(s);
  
  if (!GETP_VALID (s, s->getp + size))
    {
      STREAM_BOUND_WARN (s, "seek getp");
      return;
    }
  
  s->getp += size;
}

void
stream_forward_endp (struct stream *s, zpl_size_t size)
{
  STREAM_VERIFY_SANE(s);
  
  if (!ENDP_VALID (s, s->endp + size))
    {
      STREAM_BOUND_WARN (s, "seek endp");
      return;
    }
  
  s->endp += size;
}

/* Copy from stream to destination. */
void
stream_get (void *dst, struct stream *s, zpl_size_t size)
{
  STREAM_VERIFY_SANE(s);
  
  if (STREAM_READABLE(s) < size)
    {
      STREAM_BOUND_WARN (s, "get");
      return;
    }
  
  memcpy (dst, s->data + s->getp, size);
  s->getp += size;
}

/* Get next character from the stream. */
zpl_uchar
stream_getc (struct stream *s)
{
  zpl_uchar c;
  
  STREAM_VERIFY_SANE (s);

  if (STREAM_READABLE(s) < sizeof (zpl_uchar))
    {
      STREAM_BOUND_WARN (s, "get zpl_char");
      return 0;
    }
  c = s->data[s->getp++];
  
  return c;
}

/* Get next character from the stream. */
zpl_uchar
stream_getc_from (struct stream *s, zpl_size_t from)
{
  zpl_uchar c;

  STREAM_VERIFY_SANE(s);
  
  if (!GETP_VALID (s, from + sizeof (zpl_uchar)))
    {
      STREAM_BOUND_WARN (s, "get zpl_char");
      return 0;
    }
  
  c = s->data[from];
  
  return c;
}

/* Get next word from the stream. */
zpl_uint16
stream_getw (struct stream *s)
{
  zpl_uint16 w;

  STREAM_VERIFY_SANE (s);

  if (STREAM_READABLE (s) < sizeof (zpl_uint16))
    {
      STREAM_BOUND_WARN (s, "get ");
      return 0;
    }
  
  w = s->data[s->getp++] << 8;
  w |= s->data[s->getp++];
  
  return w;
}

/* Get next word from the stream. */
zpl_uint16
stream_getw_from (struct stream *s, zpl_size_t from)
{
  zpl_uint16 w;

  STREAM_VERIFY_SANE(s);
  
  if (!GETP_VALID (s, from + sizeof (zpl_uint16)))
    {
      STREAM_BOUND_WARN (s, "get ");
      return 0;
    }
  
  w = s->data[from++] << 8;
  w |= s->data[from];
  
  return w;
}

/* Get next long word from the stream. */
zpl_uint32
stream_getl_from (struct stream *s, zpl_size_t from)
{
  zpl_uint32 l;

  STREAM_VERIFY_SANE(s);
  
  if (!GETP_VALID (s, from + sizeof (zpl_uint32)))
    {
      STREAM_BOUND_WARN (s, "get long");
      return 0;
    }
  
  l  = s->data[from++] << 24;
  l |= s->data[from++] << 16;
  l |= s->data[from++] << 8;
  l |= s->data[from];
  
  return l;
}

zpl_uint32
stream_getl (struct stream *s)
{
  zpl_uint32 l;

  STREAM_VERIFY_SANE(s);
  
  if (STREAM_READABLE (s) < sizeof (zpl_uint32))
    {
      STREAM_BOUND_WARN (s, "get long");
      return 0;
    }
  
  l  = s->data[s->getp++] << 24;
  l |= s->data[s->getp++] << 16;
  l |= s->data[s->getp++] << 8;
  l |= s->data[s->getp++];
  
  return l;
}

/* Get next quad word from the stream. */
uint64_t
stream_getq_from (struct stream *s, zpl_size_t from)
{
  uint64_t q;

  STREAM_VERIFY_SANE(s);
  
  if (!GETP_VALID (s, from + sizeof (uint64_t)))
    {
      STREAM_BOUND_WARN (s, "get quad");
      return 0;
    }
  
  q  = ((uint64_t) s->data[from++]) << 56;
  q |= ((uint64_t) s->data[from++]) << 48;
  q |= ((uint64_t) s->data[from++]) << 40;
  q |= ((uint64_t) s->data[from++]) << 32;  
  q |= ((uint64_t) s->data[from++]) << 24;
  q |= ((uint64_t) s->data[from++]) << 16;
  q |= ((uint64_t) s->data[from++]) << 8;
  q |= ((uint64_t) s->data[from++]);
  
  return q;
}

uint64_t
stream_getq (struct stream *s)
{
  uint64_t q;

  STREAM_VERIFY_SANE(s);
  
  if (STREAM_READABLE (s) < sizeof (uint64_t))
    {
      STREAM_BOUND_WARN (s, "get quad");
      return 0;
    }
  
  q  = ((uint64_t) s->data[s->getp++]) << 56;
  q |= ((uint64_t) s->data[s->getp++]) << 48;
  q |= ((uint64_t) s->data[s->getp++]) << 40;
  q |= ((uint64_t) s->data[s->getp++]) << 32;  
  q |= ((uint64_t) s->data[s->getp++]) << 24;
  q |= ((uint64_t) s->data[s->getp++]) << 16;
  q |= ((uint64_t) s->data[s->getp++]) << 8;
  q |= ((uint64_t) s->data[s->getp++]);
  
  return q;
}

/* Get next long word from the stream. */
zpl_uint32
stream_get_ipv4 (struct stream *s)
{
  zpl_uint32 l;

  STREAM_VERIFY_SANE(s);
  
  if (STREAM_READABLE (s) < sizeof(zpl_uint32))
    {
      STREAM_BOUND_WARN (s, "get ipv4");
      return 0;
    }
  
  memcpy (&l, s->data + s->getp, sizeof(zpl_uint32));
  s->getp += sizeof(zpl_uint32);

  return l;
}

zpl_float
stream_getf (struct stream *s)
{
#if !defined(__STDC_IEC_559__) && __GCC_IEC_559 < 1
#warning "Unknown floating-point format, __func__ may be wrong"
#endif
/* we assume 'zpl_float' is in the single precision IEC 60559 binary
   format, in host byte order */
  union {
    zpl_float r;
    zpl_uint32  d;
  } u;
  u.d = stream_getl (s);
  return u.r;
}

zpl_double
stream_getd (struct stream *s)
{
#if !defined(__STDC_IEC_559__) && __GCC_IEC_559 < 1
#warning "Unknown floating-point format, __func__ may be wrong"
#endif
  union {
    zpl_double r;
    uint64_t d;
  } u;
  u.d = stream_getq (s);
  return u.r;
}

/* Copy to source to stream.
 *
 * XXX: This uses CHECK_SIZE and hence has funny semantics -> Size will wrap
 * around. This should be fixed once the stream updates are working.
 *
 * stream_write() is saner
 */
void
stream_put (struct stream *s, const void *src, zpl_size_t size)
{

  /* XXX: CHECK_SIZE has strange semantics. It should be deprecated */
  CHECK_SIZE(s, size);
  
  STREAM_VERIFY_SANE(s);
  
  if (STREAM_WRITEABLE (s) < size)
    {
      STREAM_BOUND_WARN (s, "put");
      return;
    }
  
  if (src)
    memcpy (s->data + s->endp, src, size);
  else
    memset (s->data + s->endp, 0, size);

  s->endp += size;
}

/* Put character to the stream. */
int
stream_putc (struct stream *s, zpl_uchar c)
{
  STREAM_VERIFY_SANE(s);
  
  if (STREAM_WRITEABLE (s) < sizeof(zpl_uchar))
    {
      STREAM_BOUND_WARN (s, "put");
      return 0;
    }
  
  s->data[s->endp++] = c;
  return sizeof (zpl_uchar);
}

/* Put word to the stream. */
int
stream_putw (struct stream *s, zpl_uint16 w)
{
  STREAM_VERIFY_SANE (s);

  if (STREAM_WRITEABLE (s) < sizeof (zpl_uint16))
    {
      STREAM_BOUND_WARN (s, "put");
      return 0;
    }
  
  s->data[s->endp++] = (zpl_uchar)(w >>  8);
  s->data[s->endp++] = (zpl_uchar) w;

  return 2;
}

/* Put long word to the stream. */
int
stream_putl (struct stream *s, zpl_uint32 l)
{
  STREAM_VERIFY_SANE (s);

  if (STREAM_WRITEABLE (s) < sizeof (zpl_uint32))
    {
      STREAM_BOUND_WARN (s, "put");
      return 0;
    }
  
  s->data[s->endp++] = (zpl_uchar)(l >> 24);
  s->data[s->endp++] = (zpl_uchar)(l >> 16);
  s->data[s->endp++] = (zpl_uchar)(l >>  8);
  s->data[s->endp++] = (zpl_uchar)l;

  return 4;
}

/* Put quad word to the stream. */
int
stream_putq (struct stream *s, uint64_t q)
{
  STREAM_VERIFY_SANE (s);

  if (STREAM_WRITEABLE (s) < sizeof (uint64_t))
    {
      STREAM_BOUND_WARN (s, "put quad");
      return 0;
    }
  
  s->data[s->endp++] = (zpl_uchar)(q >> 56);
  s->data[s->endp++] = (zpl_uchar)(q >> 48);
  s->data[s->endp++] = (zpl_uchar)(q >> 40);
  s->data[s->endp++] = (zpl_uchar)(q >> 32);
  s->data[s->endp++] = (zpl_uchar)(q >> 24);
  s->data[s->endp++] = (zpl_uchar)(q >> 16);
  s->data[s->endp++] = (zpl_uchar)(q >>  8);
  s->data[s->endp++] = (zpl_uchar)q;

  return 8;
}

int
stream_putf (struct stream *s, zpl_float f)
{
#if !defined(__STDC_IEC_559__) && __GCC_IEC_559 < 1
#warning "Unknown floating-point format, __func__ may be wrong"
#endif

/* we can safely assume 'zpl_float' is in the single precision
   IEC 60559 binary format in host order */
  union {
    zpl_float i;
    zpl_uint32  o;
  } u;
  u.i = f;
  return stream_putl (s, u.o);
}

int
stream_putd (struct stream *s, zpl_double d)
{
#if !defined(__STDC_IEC_559__) && __GCC_IEC_559 < 1
#warning "Unknown floating-point format, __func__ may be wrong"
#endif
  union {
    zpl_double i;
    uint64_t o;
  } u;
  u.i = d;
  return stream_putq (s, u.o);
}

int
stream_putc_at (struct stream *s, zpl_size_t putp, zpl_uchar c)
{
  STREAM_VERIFY_SANE(s);
  
  if (!PUT_AT_VALID (s, putp + sizeof (zpl_uchar)))
    {
      STREAM_BOUND_WARN (s, "put");
      return 0;
    }
  
  s->data[putp] = c;
  
  return 1;
}

int
stream_putw_at (struct stream *s, zpl_size_t putp, zpl_uint16 w)
{
  STREAM_VERIFY_SANE(s);
  
  if (!PUT_AT_VALID (s, putp + sizeof (zpl_uint16)))
    {
      STREAM_BOUND_WARN (s, "put");
      return 0;
    }
  
  s->data[putp] = (zpl_uchar)(w >>  8);
  s->data[putp + 1] = (zpl_uchar) w;
  
  return 2;
}

int
stream_putl_at (struct stream *s, zpl_size_t putp, zpl_uint32 l)
{
  STREAM_VERIFY_SANE(s);
  
  if (!PUT_AT_VALID (s, putp + sizeof (zpl_uint32)))
    {
      STREAM_BOUND_WARN (s, "put");
      return 0;
    }
  s->data[putp] = (zpl_uchar)(l >> 24);
  s->data[putp + 1] = (zpl_uchar)(l >> 16);
  s->data[putp + 2] = (zpl_uchar)(l >>  8);
  s->data[putp + 3] = (zpl_uchar)l;
  
  return 4;
}

int
stream_putq_at (struct stream *s, zpl_size_t putp, uint64_t q)
{
  STREAM_VERIFY_SANE(s);
  
  if (!PUT_AT_VALID (s, putp + sizeof (uint64_t)))
    {
      STREAM_BOUND_WARN (s, "put");
      return 0;
    }
  s->data[putp] =     (zpl_uchar)(q >> 56);
  s->data[putp + 1] = (zpl_uchar)(q >> 48);
  s->data[putp + 2] = (zpl_uchar)(q >> 40);
  s->data[putp + 3] = (zpl_uchar)(q >> 32);
  s->data[putp + 4] = (zpl_uchar)(q >> 24);
  s->data[putp + 5] = (zpl_uchar)(q >> 16);
  s->data[putp + 6] = (zpl_uchar)(q >>  8);
  s->data[putp + 7] = (zpl_uchar)q;
  
  return 8;
}

/* Put long word to the stream. */
int
stream_put_ipv4 (struct stream *s, zpl_uint32 l)
{
  STREAM_VERIFY_SANE(s);
  
  if (STREAM_WRITEABLE (s) < sizeof (zpl_uint32))
    {
      STREAM_BOUND_WARN (s, "put");
      return 0;
    }
  memcpy (s->data + s->endp, &l, sizeof (zpl_uint32));
  s->endp += sizeof (zpl_uint32);

  return sizeof (zpl_uint32);
}

/* Put long word to the stream. */
int
stream_put_in_addr (struct stream *s, struct ipstack_in_addr *addr)
{
  STREAM_VERIFY_SANE(s);
  
  if (STREAM_WRITEABLE (s) < sizeof (zpl_uint32))
    {
      STREAM_BOUND_WARN (s, "put");
      return 0;
    }

  memcpy (s->data + s->endp, addr, sizeof (zpl_uint32));
  s->endp += sizeof (zpl_uint32);

  return sizeof (zpl_uint32);
}

/* Put prefix by nlri type format. */
int
stream_put_prefix (struct stream *s, struct prefix *p)
{
  zpl_size_t psize;
  
  STREAM_VERIFY_SANE(s);
  
  psize = PSIZE (p->prefixlen);
  
  if (STREAM_WRITEABLE (s) < (psize + sizeof (zpl_uchar)))
    {
      STREAM_BOUND_WARN (s, "put");
      return 0;
    }
  
  s->data[s->endp++] = p->prefixlen;
  memcpy (s->data + s->endp, &p->u.prefix, psize);
  s->endp += psize;
  
  return psize;
}

/* Read size from fd. */
int
stream_read (struct stream *s, zpl_socket_t fd, zpl_size_t size)
{
  zpl_uint32 nbytes;

  STREAM_VERIFY_SANE(s);
  
  if (STREAM_WRITEABLE (s) < size)
    {
      STREAM_BOUND_WARN (s, "put");
      return 0;
    }
  
  nbytes = readn (fd, s->data + s->endp, size);

  if (nbytes > 0)
    s->endp += nbytes;
  
  return nbytes;
}

ssize_t
stream_read_try(struct stream *s, zpl_socket_t fd, zpl_size_t size)
{
  ssize_t nbytes;

  STREAM_VERIFY_SANE(s);
  
  if (STREAM_WRITEABLE(s) < size)
    {
      STREAM_BOUND_WARN (s, "put");
      /* Fatal (not transient) error, since retrying will not help
         (stream is too small to contain the desired data). */
      return -1;
    }
	  nbytes = ipstack_read(fd, s->data + s->endp, size);
  if (nbytes >= 0)
    {
      s->endp += nbytes;
      return nbytes;
    }
  /* Error: was it transient (return -2) or fatal (return -1)? */
  if (IPSTACK_ERRNO_RETRY(ipstack_errno))
    return -2;
  zlog_warn(MODULE_DEFAULT, "%s: read failed on fd %d: %s", __func__, fd, ipstack_strerror(ipstack_errno));
  return -1;
}

/* Read up to size bytes into the stream from the fd, using recvmsgfrom
 * whose arguments match the remaining arguments to this function
 */
ssize_t 
stream_recvfrom (struct stream *s, zpl_socket_t fd, zpl_size_t size, zpl_uint32 flags,
                 struct ipstack_sockaddr *from, socklen_t *fromlen)
{
  ssize_t nbytes;

  STREAM_VERIFY_SANE(s);
  
  if (STREAM_WRITEABLE(s) < size)
    {
      STREAM_BOUND_WARN (s, "put");
      /* Fatal (not transient) error, since retrying will not help
         (stream is too small to contain the desired data). */
      return -1;
    }
	  nbytes = ipstack_recvfrom (fd, s->data + s->endp, size,
	                            flags, from, fromlen);
  if (nbytes >= 0)
    {
      s->endp += nbytes;
      return nbytes;
    }
  /* Error: was it transient (return -2) or fatal (return -1)? */
  if (IPSTACK_ERRNO_RETRY(ipstack_errno))
    return -2;
  zlog_warn(MODULE_DEFAULT, "%s: read failed on fd %d: %s", __func__, fd, ipstack_strerror(ipstack_errno));
  return -1;
}

/* Read up to smaller of size or SIZE_REMAIN() bytes to the stream, starting
 * from endp.
 * First ipstack_iovec will be used to receive the data.
 * Stream need not be empty.
 */
ssize_t
stream_recvmsg (struct stream *s, zpl_socket_t fd, struct ipstack_msghdr *msgh, zpl_uint32 flags, 
                zpl_size_t size)
{
  zpl_uint32 nbytes;
  struct ipstack_iovec *iov;
  
  STREAM_VERIFY_SANE(s);
  assert (msgh->msg_iovlen > 0);  
  
  if (STREAM_WRITEABLE (s) < size)
    {
      STREAM_BOUND_WARN (s, "put");
      /* This is a logic error in the calling code: the stream is too small
         to hold the desired data! */
      return -1;
    }
  
  iov = &(msgh->msg_iov[0]);
  iov->iov_base = (s->data + s->endp);
  iov->iov_len = size;
	  nbytes = ipstack_recvmsg (fd, msgh, flags);
  if (nbytes > 0)
    s->endp += nbytes;
  
  return nbytes;
}

int
stream_writefd (struct stream *s, zpl_socket_t fd, zpl_size_t size)
{
  zpl_uint32 nbytes;
  nbytes = writen (fd, s->data + s->getp, size);
  return nbytes;
}

ssize_t
stream_write_try(struct stream *s, zpl_socket_t fd, zpl_size_t size)
{
  ssize_t nbytes;

	  nbytes = ipstack_write(fd, s->data + s->getp, size);
      return nbytes;
}

/* Read up to size bytes into the stream from the fd, using recvmsgfrom
 * whose arguments match the remaining arguments to this function
 */
ssize_t 
stream_sendto (struct stream *s, zpl_socket_t fd, zpl_size_t size, zpl_uint32 flags,
                 struct ipstack_sockaddr *to, socklen_t *tolen)
{
  ssize_t nbytes;

	  nbytes = ipstack_sendto (fd, s->data + s->getp, size,
	                            flags, to, *tolen);
      return nbytes;
}

/* Read up to smaller of size or SIZE_REMAIN() bytes to the stream, starting
 * from endp.
 * First ipstack_iovec will be used to receive the data.
 * Stream need not be empty.
 */
ssize_t
stream_sendmsg (struct stream *s, zpl_socket_t fd, struct ipstack_msghdr *msgh, zpl_uint32 flags, 
                zpl_size_t size)
{
  zpl_uint32 nbytes;
  struct ipstack_iovec *iov;
  
  iov = &(msgh->msg_iov[0]);
  iov->iov_base = (s->data + s->getp);
  iov->iov_len = size;
	nbytes = ipstack_sendmsg (fd, msgh, flags);
  return nbytes;
}


/* Write data to buffer. */
zpl_size_t
stream_write (struct stream *s, const void *ptr, zpl_size_t size)
{

  CHECK_SIZE(s, size);

  STREAM_VERIFY_SANE(s);
  
  if (STREAM_WRITEABLE (s) < size)
    {
      STREAM_BOUND_WARN (s, "put");
      return 0;
    }
  
  memcpy (s->data + s->endp, ptr, size);
  s->endp += size;

  return size;
}

/* Return current read pointer. 
 * DEPRECATED!
 * Use stream_get_pnt_to if you must, but decoding streams properly
 * is preferred
 */
zpl_uchar *
stream_pnt (struct stream *s)
{
  STREAM_VERIFY_SANE(s);
  return s->data + s->getp;
}

/* Check does this stream empty? */
int
stream_empty (struct stream *s)
{
  STREAM_VERIFY_SANE(s);

  return (s->endp == 0);
}

/* Reset stream. */
void
stream_reset (struct stream *s)
{
  STREAM_VERIFY_SANE (s);

  s->getp = s->endp = 0;
}

/* Discard read data (prior to the getp), and move the unread data
 * to the beginning of the stream.
 *
 * See also stream_fifo_* functions, for another approach to managing
 * streams.
 */
void
stream_discard (struct stream *s)
{
  STREAM_VERIFY_SANE (s);
  
  if (s->getp == 0)
    return;
  
  if (s->getp == s->endp)
    {
      stream_reset (s);
      return;
    }
  
  s->data = memmove (s->data, s->data + s->getp, s->endp - s->getp);
  s->endp -= s->getp;
  s->getp = 0;
}

/* Write stream contens to the file discriptor. */
int
stream_flush (struct stream *s, zpl_socket_t fd)
{
  zpl_uint32 nbytes;
  
  STREAM_VERIFY_SANE(s);
	  nbytes = ipstack_write (fd, s->data + s->getp, s->endp - s->getp);
  return nbytes;
}

/* Stream first in first out queue. */

struct stream_fifo *
stream_fifo_new (void)
{
  struct stream_fifo *new;
 
  new = XCALLOC (MTYPE_STREAM_FIFO, sizeof (struct stream_fifo));
  return new;
}

/* Add new stream to fifo. */
void
stream_fifo_push (struct stream_fifo *fifo, struct stream *s)
{
  if (fifo->tail)
    fifo->tail->next = s;
  else
    fifo->head = s;
     
  fifo->tail = s;

  fifo->count++;
}

/* Delete first stream from fifo. */
struct stream *
stream_fifo_pop (struct stream_fifo *fifo)
{
  struct stream *s;
  
  s = fifo->head; 

  if (s)
    { 
      fifo->head = s->next;

      if (fifo->head == NULL)
	fifo->tail = NULL;

      fifo->count--;
    }

  return s; 
}

/* Return first fifo entry. */
struct stream *
stream_fifo_head (struct stream_fifo *fifo)
{
  return fifo->head;
}

void
stream_fifo_clean (struct stream_fifo *fifo)
{
  struct stream *s;
  struct stream *next;

  for (s = fifo->head; s; s = next)
    {
      next = s->next;
      stream_free (s);
    }
  fifo->head = fifo->tail = NULL;
  fifo->count = 0;
}

void
stream_fifo_free (struct stream_fifo *fifo)
{
  stream_fifo_clean (fifo);
  XFREE (MTYPE_STREAM_FIFO, fifo);
}
