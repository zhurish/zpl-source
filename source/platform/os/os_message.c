/*
 * os_message.c
 *
 *  Created on: Jun 9, 2018
 *      Author: zhurish
 */
#include "auto_include.h"
#include "zpl_type.h"
#include "os_message.h"

#define IPSTACK_ERRNO_RETRY(EN) \
	(((EN) == IPSTACK_ERRNO_EAGAIN) || ((EN) == IPSTACK_ERRNO_EWOULDBLOCK) || ((EN) == IPSTACK_ERRNO_EINTR))

/* Utility macros. */
#define ZPL_OSMSG_SIZE(S) ((S)->size)
/* number of bytes which can still be written */
#define ZPL_OSMSG_WRITEABLE(S) ((S)->size - (S)->endp)
/* number of bytes still to be read */
#define ZPL_OSMSG_READABLE(S) ((S)->endp - (S)->getp)

#define ZPL_OSMSG_CONCAT_REMAIN(S1, S2, size) \
	((size) - (S1)->endp - (S2)->endp)

/* deprecated macros - do not use in dst code */
#define ZPL_OSMSG_PNT(S) zpl_osmsg_pnt((S))
#define ZPL_OSMSG_DATA(S) ((S)->data)
#define ZPL_OSMSG_REMAIN(S) ZPL_OSMSG_WRITEABLE((S))

/* Tests whether a position is valid */
#define GETP_VALID(S, G) \
	((G) <= (S)->endp)
#define PUT_AT_VALID(S, G) GETP_VALID(S, G)
#define ENDP_VALID(S, E) \
	((E) <= (S)->size)

/* asserting sanity checks. Following must be true before
 * zpl_osmsg functions are called:
 *
 * Following must always be true of zpl_osmsg elements
 * before and after calls to zpl_osmsg functions:
 *
 * getp <= endp <= size
 *
 * Note that after a zpl_osmsg function is called following may be true:
 * if (getp == endp) then zpl_osmsg is no longer readable
 * if (endp == size) then zpl_osmsg is no longer writeable
 *
 * It is valid to put to anywhere within the size of the zpl_osmsg, but only
 * using zpl_osmsg_put..._at() functions.
 */
#define ZPL_OSMSG_WARN_OFFSETS(S)                                           \
	_OS_ERROR("&(struct zpl_osmsg): %p, size: %lu, getp: %lu, endp: %lu\n", \
			  (void *)(S),                                                  \
			  (zpl_uint32)(S)->size,                                        \
			  (zpl_uint32)(S)->getp,                                        \
			  (zpl_uint32)(S)->endp)

#define ZPL_OSMSG_VERIFY_SANE(S)                                     \
	do                                                               \
	{                                                                \
		if (!(GETP_VALID(S, (S)->getp) && ENDP_VALID(S, (S)->endp))) \
			ZPL_OSMSG_WARN_OFFSETS(S);                               \
		assert(GETP_VALID(S, (S)->getp));                            \
		assert(ENDP_VALID(S, (S)->endp));                            \
	} while (0)

#define ZPL_OSMSG_BOUND_WARN(S, WHAT)                                  \
	do                                                                 \
	{                                                                  \
		_OS_WARN("%s: Attempt to %s out of bounds", __func__, (WHAT)); \
		ZPL_OSMSG_WARN_OFFSETS(S);                                     \
		assert(0);                                                     \
	} while (0)

/* XXX: Deprecated macro: do not use */
#define CHECK_SIZE(S, Z)                                            \
	do                                                              \
	{                                                               \
		if (((S)->endp + (Z)) > (S)->size)                          \
		{                                                           \
			_OS_WARN("CHECK_SIZE: truncating requested size %lu\n", \
					 (zpl_uint32)(Z));                              \
			ZPL_OSMSG_WARN_OFFSETS(S);                              \
			(Z) = (S)->size - (S)->endp;                            \
		}                                                           \
	} while (0);

/* Make zpl_osmsg buffer. */
struct zpl_osmsg *
zpl_osmsg_new(zpl_uint32 size)
{
	struct zpl_osmsg *s;

	assert(size > 0);

	if (size == 0)
	{
		_OS_WARN("zpl_osmsg_new(): called with 0 size!");
		return NULL;
	}

	s = os_malloc(sizeof(struct zpl_osmsg));

	if (s == NULL)
		return s;

	if ((s->data = os_malloc(size)) == NULL)
	{
		os_free(s);
		return NULL;
	}

	s->size = size;
	return s;
}

/* Free it now. */
void zpl_osmsg_free(struct zpl_osmsg *s)
{
	if (!s)
		return;

	os_free(s->data);
	os_free(s);
}

struct zpl_osmsg *
zpl_osmsg_copy(struct zpl_osmsg *dst, struct zpl_osmsg *src)
{
	ZPL_OSMSG_VERIFY_SANE(src);

	assert(dst != NULL);
	assert(ZPL_OSMSG_SIZE(dst) >= src->endp);

	dst->endp = src->endp;
	dst->getp = src->getp;

	memcpy(dst->data, src->data, src->endp);

	return dst;
}

struct zpl_osmsg *
zpl_osmsg_dup(struct zpl_osmsg *s)
{
	struct zpl_osmsg *dst;

	ZPL_OSMSG_VERIFY_SANE(s);

	if ((dst = zpl_osmsg_new(s->endp)) == NULL)
		return NULL;

	return (zpl_osmsg_copy(dst, s));
}

struct zpl_osmsg *
zpl_osmsg_dupcat(struct zpl_osmsg *s1, struct zpl_osmsg *s2, zpl_uint32 offset)
{
	struct zpl_osmsg *dst;

	ZPL_OSMSG_VERIFY_SANE(s1);
	ZPL_OSMSG_VERIFY_SANE(s2);

	if ((dst = zpl_osmsg_new(s1->endp + s2->endp)) == NULL)
		return NULL;

	memcpy(dst->data, s1->data, offset);
	memcpy(dst->data + offset, s2->data, s2->endp);
	memcpy(dst->data + offset + s2->endp, s1->data + offset,
		   (s1->endp - offset));
	dst->endp = s1->endp + s2->endp;
	return dst;
}

zpl_uint32
zpl_osmsg_resize(struct zpl_osmsg *s, zpl_uint32 newsize)
{
	zpl_uchar *newdata;
	ZPL_OSMSG_VERIFY_SANE(s);

	newdata = os_realloc(s->data, newsize);

	if (newdata == NULL)
		return s->size;

	s->data = newdata;
	s->size = newsize;

	if (s->endp > s->size)
		s->endp = s->size;
	if (s->getp > s->endp)
		s->getp = s->endp;

	ZPL_OSMSG_VERIFY_SANE(s);

	return s->size;
}

zpl_uint32
zpl_osmsg_get_getp(struct zpl_osmsg *s)
{
	ZPL_OSMSG_VERIFY_SANE(s);
	return s->getp;
}

zpl_uint32
zpl_osmsg_get_endp(struct zpl_osmsg *s)
{
	ZPL_OSMSG_VERIFY_SANE(s);
	return s->endp;
}

zpl_uint32
zpl_osmsg_get_size(struct zpl_osmsg *s)
{
	ZPL_OSMSG_VERIFY_SANE(s);
	return s->size;
}

/* Stream structre' zpl_osmsg pointer related functions.  */
void zpl_osmsg_set_getp(struct zpl_osmsg *s, zpl_uint32 pos)
{
	ZPL_OSMSG_VERIFY_SANE(s);

	if (!GETP_VALID(s, pos))
	{
		ZPL_OSMSG_BOUND_WARN(s, "set getp");
		pos = s->endp;
	}

	s->getp = pos;
}

void zpl_osmsg_set_endp(struct zpl_osmsg *s, zpl_uint32 pos)
{
	ZPL_OSMSG_VERIFY_SANE(s);

	if (!ENDP_VALID(s, pos))
	{
		ZPL_OSMSG_BOUND_WARN(s, "set endp");
		return;
	}

	/*
	 * Make sure the current read pointer is not beyond the dst endp.
	 */
	if (s->getp > pos)
	{
		ZPL_OSMSG_BOUND_WARN(s, "set endp");
		return;
	}

	s->endp = pos;
	ZPL_OSMSG_VERIFY_SANE(s);
}

/* Forward pointer. */
void zpl_osmsg_forward_getp(struct zpl_osmsg *s, zpl_uint32 size)
{
	ZPL_OSMSG_VERIFY_SANE(s);

	if (!GETP_VALID(s, s->getp + size))
	{
		ZPL_OSMSG_BOUND_WARN(s, "seek getp");
		return;
	}

	s->getp += size;
}

void zpl_osmsg_forward_endp(struct zpl_osmsg *s, zpl_uint32 size)
{
	ZPL_OSMSG_VERIFY_SANE(s);

	if (!ENDP_VALID(s, s->endp + size))
	{
		ZPL_OSMSG_BOUND_WARN(s, "seek endp");
		return;
	}

	s->endp += size;
}

/* Copy from zpl_osmsg to destination. */
void zpl_osmsg_get(void *dst, struct zpl_osmsg *s, zpl_uint32 size)
{
	ZPL_OSMSG_VERIFY_SANE(s);

	if (ZPL_OSMSG_READABLE(s) < size)
	{
		ZPL_OSMSG_BOUND_WARN(s, "get");
		return;
	}

	memcpy(dst, s->data + s->getp, size);
	s->getp += size;
}

/* Get next character from the zpl_osmsg. */
zpl_uchar
zpl_osmsg_getc(struct zpl_osmsg *s)
{
	zpl_uchar c;

	ZPL_OSMSG_VERIFY_SANE(s);

	if (ZPL_OSMSG_READABLE(s) < sizeof(zpl_uchar))
	{
		ZPL_OSMSG_BOUND_WARN(s, "get zpl_char");
		return 0;
	}
	c = s->data[s->getp++];

	return c;
}

/* Get next character from the zpl_osmsg. */
zpl_uchar
zpl_osmsg_getc_from(struct zpl_osmsg *s, zpl_uint32 from)
{
	zpl_uchar c;

	ZPL_OSMSG_VERIFY_SANE(s);

	if (!GETP_VALID(s, from + sizeof(zpl_uchar)))
	{
		ZPL_OSMSG_BOUND_WARN(s, "get zpl_char");
		return 0;
	}

	c = s->data[from];

	return c;
}

/* Get next word from the zpl_osmsg. */
zpl_uint16
zpl_osmsg_getw(struct zpl_osmsg *s)
{
	zpl_uint16 w;

	ZPL_OSMSG_VERIFY_SANE(s);

	if (ZPL_OSMSG_READABLE(s) < sizeof(zpl_uint16))
	{
		ZPL_OSMSG_BOUND_WARN(s, "get ");
		return 0;
	}

	w = s->data[s->getp++] << 8;
	w |= s->data[s->getp++];

	return w;
}

/* Get next word from the zpl_osmsg. */
zpl_uint16
zpl_osmsg_getw_from(struct zpl_osmsg *s, zpl_uint32 from)
{
	zpl_uint16 w;

	ZPL_OSMSG_VERIFY_SANE(s);

	if (!GETP_VALID(s, from + sizeof(zpl_uint16)))
	{
		ZPL_OSMSG_BOUND_WARN(s, "get ");
		return 0;
	}

	w = s->data[from++] << 8;
	w |= s->data[from];

	return w;
}

/* Get next long word from the zpl_osmsg. */
zpl_uint32
zpl_osmsg_getl_from(struct zpl_osmsg *s, zpl_uint32 from)
{
	zpl_uint32 l;

	ZPL_OSMSG_VERIFY_SANE(s);

	if (!GETP_VALID(s, from + sizeof(zpl_uint32)))
	{
		ZPL_OSMSG_BOUND_WARN(s, "get long");
		return 0;
	}

	l = s->data[from++] << 24;
	l |= s->data[from++] << 16;
	l |= s->data[from++] << 8;
	l |= s->data[from];

	return l;
}

zpl_uint32
zpl_osmsg_getl(struct zpl_osmsg *s)
{
	zpl_uint32 l;

	ZPL_OSMSG_VERIFY_SANE(s);

	if (ZPL_OSMSG_READABLE(s) < sizeof(zpl_uint32))
	{
		ZPL_OSMSG_BOUND_WARN(s, "get long");
		return 0;
	}

	l = s->data[s->getp++] << 24;
	l |= s->data[s->getp++] << 16;
	l |= s->data[s->getp++] << 8;
	l |= s->data[s->getp++];

	return l;
}

/* Get next quad word from the zpl_osmsg. */
zpl_uint64
zpl_osmsg_getq_from(struct zpl_osmsg *s, zpl_uint32 from)
{
	zpl_uint64 q;

	ZPL_OSMSG_VERIFY_SANE(s);

	if (!GETP_VALID(s, from + sizeof(zpl_uint64)))
	{
		ZPL_OSMSG_BOUND_WARN(s, "get quad");
		return 0;
	}

	q = ((zpl_uint64)s->data[from++]) << 56;
	q |= ((zpl_uint64)s->data[from++]) << 48;
	q |= ((zpl_uint64)s->data[from++]) << 40;
	q |= ((zpl_uint64)s->data[from++]) << 32;
	q |= ((zpl_uint64)s->data[from++]) << 24;
	q |= ((zpl_uint64)s->data[from++]) << 16;
	q |= ((zpl_uint64)s->data[from++]) << 8;
	q |= ((zpl_uint64)s->data[from++]);

	return q;
}

zpl_uint64
zpl_osmsg_getq(struct zpl_osmsg *s)
{
	zpl_uint64 q;

	ZPL_OSMSG_VERIFY_SANE(s);

	if (ZPL_OSMSG_READABLE(s) < sizeof(zpl_uint64))
	{
		ZPL_OSMSG_BOUND_WARN(s, "get quad");
		return 0;
	}

	q = ((zpl_uint64)s->data[s->getp++]) << 56;
	q |= ((zpl_uint64)s->data[s->getp++]) << 48;
	q |= ((zpl_uint64)s->data[s->getp++]) << 40;
	q |= ((zpl_uint64)s->data[s->getp++]) << 32;
	q |= ((zpl_uint64)s->data[s->getp++]) << 24;
	q |= ((zpl_uint64)s->data[s->getp++]) << 16;
	q |= ((zpl_uint64)s->data[s->getp++]) << 8;
	q |= ((zpl_uint64)s->data[s->getp++]);

	return q;
}

/* Get next long word from the zpl_osmsg. */
zpl_uint32
zpl_osmsg_get_ipv4(struct zpl_osmsg *s)
{
	zpl_uint32 l;

	ZPL_OSMSG_VERIFY_SANE(s);

	if (ZPL_OSMSG_READABLE(s) < sizeof(zpl_uint32))
	{
		ZPL_OSMSG_BOUND_WARN(s, "get ipv4");
		return 0;
	}

	memcpy(&l, s->data + s->getp, sizeof(zpl_uint32));
	s->getp += sizeof(zpl_uint32);

	return l;
}

zpl_float
zpl_osmsg_getf(struct zpl_osmsg *s)
{
#if !defined(__STDC_IEC_559__) && __GCC_IEC_559 < 1
#warning "Unknown floating-point format, __func__ may be wrong"
#endif
	/* we assume 'zpl_float' is in the single precision IEC 60559 binary
	   format, in host byte order */
	union
	{
		zpl_float r;
		zpl_uint32 d;
	} u;
	u.d = zpl_osmsg_getl(s);
	return u.r;
}

zpl_double
zpl_osmsg_getd(struct zpl_osmsg *s)
{
#if !defined(__STDC_IEC_559__) && __GCC_IEC_559 < 1
#warning "Unknown floating-point format, __func__ may be wrong"
#endif
	union
	{
		zpl_double r;
		zpl_uint64 d;
	} u;
	u.d = zpl_osmsg_getq(s);
	return u.r;
}

/* Copy to source to zpl_osmsg.
 *
 * XXX: This uses CHECK_SIZE and hence has funny semantics -> Size will wrap
 * around. This should be fixed once the zpl_osmsg updates are working.
 *
 * zpl_osmsg_write() is saner
 */
void zpl_osmsg_put(struct zpl_osmsg *s, const void *src, zpl_uint32 size)
{

	/* XXX: CHECK_SIZE has strange semantics. It should be deprecated */
	CHECK_SIZE(s, size);

	ZPL_OSMSG_VERIFY_SANE(s);

	if (ZPL_OSMSG_WRITEABLE(s) < size)
	{
		ZPL_OSMSG_BOUND_WARN(s, "put");
		return;
	}

	if (src)
		memcpy(s->data + s->endp, src, size);
	else
		memset(s->data + s->endp, 0, size);

	s->endp += size;
}

/* Put character to the zpl_osmsg. */
int zpl_osmsg_putc(struct zpl_osmsg *s, zpl_uchar c)
{
	ZPL_OSMSG_VERIFY_SANE(s);

	if (ZPL_OSMSG_WRITEABLE(s) < sizeof(zpl_uchar))
	{
		ZPL_OSMSG_BOUND_WARN(s, "put");
		return 0;
	}

	s->data[s->endp++] = c;
	return sizeof(zpl_uchar);
}

/* Put word to the zpl_osmsg. */
int zpl_osmsg_putw(struct zpl_osmsg *s, zpl_uint16 w)
{
	ZPL_OSMSG_VERIFY_SANE(s);

	if (ZPL_OSMSG_WRITEABLE(s) < sizeof(zpl_uint16))
	{
		ZPL_OSMSG_BOUND_WARN(s, "put");
		return 0;
	}

	s->data[s->endp++] = (zpl_uchar)(w >> 8);
	s->data[s->endp++] = (zpl_uchar)w;

	return 2;
}

/* Put long word to the zpl_osmsg. */
int zpl_osmsg_putl(struct zpl_osmsg *s, zpl_uint32 l)
{
	ZPL_OSMSG_VERIFY_SANE(s);

	if (ZPL_OSMSG_WRITEABLE(s) < sizeof(zpl_uint32))
	{
		ZPL_OSMSG_BOUND_WARN(s, "put");
		return 0;
	}

	s->data[s->endp++] = (zpl_uchar)(l >> 24);
	s->data[s->endp++] = (zpl_uchar)(l >> 16);
	s->data[s->endp++] = (zpl_uchar)(l >> 8);
	s->data[s->endp++] = (zpl_uchar)l;

	return 4;
}

/* Put quad word to the zpl_osmsg. */
int zpl_osmsg_putq(struct zpl_osmsg *s, zpl_uint64 q)
{
	ZPL_OSMSG_VERIFY_SANE(s);

	if (ZPL_OSMSG_WRITEABLE(s) < sizeof(zpl_uint64))
	{
		ZPL_OSMSG_BOUND_WARN(s, "put quad");
		return 0;
	}

	s->data[s->endp++] = (zpl_uchar)(q >> 56);
	s->data[s->endp++] = (zpl_uchar)(q >> 48);
	s->data[s->endp++] = (zpl_uchar)(q >> 40);
	s->data[s->endp++] = (zpl_uchar)(q >> 32);
	s->data[s->endp++] = (zpl_uchar)(q >> 24);
	s->data[s->endp++] = (zpl_uchar)(q >> 16);
	s->data[s->endp++] = (zpl_uchar)(q >> 8);
	s->data[s->endp++] = (zpl_uchar)q;

	return 8;
}

int zpl_osmsg_putf(struct zpl_osmsg *s, zpl_float f)
{
#if !defined(__STDC_IEC_559__) && __GCC_IEC_559 < 1
#warning "Unknown floating-point format, __func__ may be wrong"
#endif

	/* we can safely assume 'zpl_float' is in the single precision
	   IEC 60559 binary format in host order */
	union
	{
		zpl_float i;
		zpl_uint32 o;
	} u;
	u.i = f;
	return zpl_osmsg_putl(s, u.o);
}

int zpl_osmsg_putd(struct zpl_osmsg *s, zpl_double d)
{
#if !defined(__STDC_IEC_559__) && __GCC_IEC_559 < 1
#warning "Unknown floating-point format, __func__ may be wrong"
#endif
	union
	{
		zpl_double i;
		zpl_uint64 o;
	} u;
	u.i = d;
	return zpl_osmsg_putq(s, u.o);
}

int zpl_osmsg_putc_at(struct zpl_osmsg *s, zpl_uint32 putp, zpl_uchar c)
{
	ZPL_OSMSG_VERIFY_SANE(s);

	if (!PUT_AT_VALID(s, putp + sizeof(zpl_uchar)))
	{
		ZPL_OSMSG_BOUND_WARN(s, "put");
		return 0;
	}

	s->data[putp] = c;

	return 1;
}

int zpl_osmsg_putw_at(struct zpl_osmsg *s, zpl_uint32 putp, zpl_uint16 w)
{
	ZPL_OSMSG_VERIFY_SANE(s);

	if (!PUT_AT_VALID(s, putp + sizeof(zpl_uint16)))
	{
		ZPL_OSMSG_BOUND_WARN(s, "put");
		return 0;
	}

	s->data[putp] = (zpl_uchar)(w >> 8);
	s->data[putp + 1] = (zpl_uchar)w;

	return 2;
}

int zpl_osmsg_putl_at(struct zpl_osmsg *s, zpl_uint32 putp, zpl_uint32 l)
{
	ZPL_OSMSG_VERIFY_SANE(s);

	if (!PUT_AT_VALID(s, putp + sizeof(zpl_uint32)))
	{
		ZPL_OSMSG_BOUND_WARN(s, "put");
		return 0;
	}
	s->data[putp] = (zpl_uchar)(l >> 24);
	s->data[putp + 1] = (zpl_uchar)(l >> 16);
	s->data[putp + 2] = (zpl_uchar)(l >> 8);
	s->data[putp + 3] = (zpl_uchar)l;

	return 4;
}

int zpl_osmsg_putq_at(struct zpl_osmsg *s, zpl_uint32 putp, zpl_uint64 q)
{
	ZPL_OSMSG_VERIFY_SANE(s);

	if (!PUT_AT_VALID(s, putp + sizeof(zpl_uint64)))
	{
		ZPL_OSMSG_BOUND_WARN(s, "put");
		return 0;
	}
	s->data[putp] = (zpl_uchar)(q >> 56);
	s->data[putp + 1] = (zpl_uchar)(q >> 48);
	s->data[putp + 2] = (zpl_uchar)(q >> 40);
	s->data[putp + 3] = (zpl_uchar)(q >> 32);
	s->data[putp + 4] = (zpl_uchar)(q >> 24);
	s->data[putp + 5] = (zpl_uchar)(q >> 16);
	s->data[putp + 6] = (zpl_uchar)(q >> 8);
	s->data[putp + 7] = (zpl_uchar)q;

	return 8;
}

/* Put long word to the zpl_osmsg. */
int zpl_osmsg_put_ipv4(struct zpl_osmsg *s, zpl_uint32 l)
{
	ZPL_OSMSG_VERIFY_SANE(s);

	if (ZPL_OSMSG_WRITEABLE(s) < sizeof(zpl_uint32))
	{
		ZPL_OSMSG_BOUND_WARN(s, "put");
		return 0;
	}
	memcpy(s->data + s->endp, &l, sizeof(zpl_uint32));
	s->endp += sizeof(zpl_uint32);

	return sizeof(zpl_uint32);
}


/* Return current read pointer.
 * DEPRECATED!
 * Use zpl_osmsg_get_pnt_to if you must, but decoding zpl_osmsgs properly
 * is preferred
 */
zpl_uchar *
zpl_osmsg_pnt(struct zpl_osmsg *s)
{
	ZPL_OSMSG_VERIFY_SANE(s);
	return s->data + s->getp;
}

/* Check does this zpl_osmsg empty? */
int zpl_osmsg_empty(struct zpl_osmsg *s)
{
	ZPL_OSMSG_VERIFY_SANE(s);

	return (s->endp == 0);
}

/* Reset zpl_osmsg. */
void zpl_osmsg_reset(struct zpl_osmsg *s)
{
	ZPL_OSMSG_VERIFY_SANE(s);

	s->getp = s->endp = 0;
}

/* Discard read data (prior to the getp), and move the unread data
 * to the beginning of the zpl_osmsg.
 *
 * See also zpl_osmsg_fifo_* functions, for another approach to managing
 * zpl_osmsgs.
 */
void zpl_osmsg_discard(struct zpl_osmsg *s)
{
	ZPL_OSMSG_VERIFY_SANE(s);

	if (s->getp == 0)
		return;

	if (s->getp == s->endp)
	{
		zpl_osmsg_reset(s);
		return;
	}

	s->data = memmove(s->data, s->data + s->getp, s->endp - s->getp);
	s->endp -= s->getp;
	s->getp = 0;
}

/************************************************************************/
/************************************************************************/
/************************************************************************/


/* Read size from fd. */
int zpl_osmsg_readfd(struct zpl_osmsg *s, zpl_socket_t fd, zpl_uint32 size)
{
	zpl_uint32 nbytes = 0, alreadbytes = 0;

	ZPL_OSMSG_VERIFY_SANE(s);

	if (ZPL_OSMSG_WRITEABLE(s) < size)
	{
		ZPL_OSMSG_BOUND_WARN(s, "put");
		return 0;
	}
	while(1)
	{
		nbytes = ipstack_read(fd, s->data + s->endp + alreadbytes, size - alreadbytes);
		if(nbytes <= 0)
		{
			if (!IPSTACK_ERRNO_RETRY(ipstack_errno))
				return -1;
		}
		alreadbytes += nbytes;
		if(alreadbytes == size)
			break;
	}
	if (alreadbytes > 0)
		s->endp += alreadbytes;
	return alreadbytes;
}

ssize_t
zpl_osmsg_read_try(struct zpl_osmsg *s, zpl_socket_t fd, zpl_uint32 size)
{
	ssize_t nbytes;

	ZPL_OSMSG_VERIFY_SANE(s);

	if (ZPL_OSMSG_WRITEABLE(s) < size)
	{
		ZPL_OSMSG_BOUND_WARN(s, "put");
		/* Fatal (not transient) error, since retrying will not help
		   (zpl_osmsg is too small to contain the desired data). */
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
	_OS_WARN( "%s: read failed on fd %d: %s", __func__, fd, ipstack_strerror(ipstack_errno));
	return -1;
}

/* Read up to size bytes into the zpl_osmsg from the fd, using recvmsgfrom
 * whose arguments match the remaining arguments to this function
 */
ssize_t
zpl_osmsg_recvfrom(struct zpl_osmsg *s, zpl_socket_t fd, zpl_uint32 size, zpl_uint32 flags,
				   struct ipstack_sockaddr *from, socklen_t *fromlen)
{
	ssize_t nbytes;

	ZPL_OSMSG_VERIFY_SANE(s);

	if (ZPL_OSMSG_WRITEABLE(s) < size)
	{
		ZPL_OSMSG_BOUND_WARN(s, "put");
		/* Fatal (not transient) error, since retrying will not help
		   (zpl_osmsg is too small to contain the desired data). */
		return -1;
	}
	nbytes = ipstack_recvfrom(fd, s->data + s->endp, size,
							  flags, from, fromlen);
	if (nbytes >= 0)
	{
		s->endp += nbytes;
		return nbytes;
	}
	/* Error: was it transient (return -2) or fatal (return -1)? */
	if (IPSTACK_ERRNO_RETRY(ipstack_errno))
		return -2;
	_OS_WARN( "%s: read failed on fd %d: %s", __func__, fd, ipstack_strerror(ipstack_errno));
	return -1;
}

/* Read up to smaller of size or SIZE_REMAIN() bytes to the zpl_osmsg, starting
 * from endp.
 * First ipstack_iovec will be used to receive the data.
 * Stream need not be empty.
 */
ssize_t
zpl_osmsg_recvmsg(struct zpl_osmsg *s, zpl_socket_t fd, struct ipstack_msghdr *msgh, zpl_uint32 flags,
				  zpl_uint32 size)
{
	zpl_uint32 nbytes;
	struct ipstack_iovec *iov;

	ZPL_OSMSG_VERIFY_SANE(s);
	assert(msgh->msg_iovlen > 0);

	if (ZPL_OSMSG_WRITEABLE(s) < size)
	{
		ZPL_OSMSG_BOUND_WARN(s, "put");
		/* This is a logic error in the calling code: the zpl_osmsg is too small
		   to hold the desired data! */
		return -1;
	}

	iov = &(msgh->msg_iov[0]);
	iov->iov_base = (s->data + s->endp);
	iov->iov_len = size;
	nbytes = ipstack_recvmsg(fd, msgh, flags);
	if (nbytes > 0)
		s->endp += nbytes;

	return nbytes;
}

int zpl_osmsg_writefd(struct zpl_osmsg *s, zpl_socket_t fd, zpl_uint32 size)
{
	zpl_uint32 nbytes = 0, alreadbytes = 0;
	while(1)
	{
		nbytes = ipstack_write(fd, s->data + s->getp + alreadbytes, size - alreadbytes);
		if(nbytes <= 0)
		{
			if (!IPSTACK_ERRNO_RETRY(ipstack_errno))
				return -1;
		}
		alreadbytes += nbytes;
	}
	return alreadbytes;
}

ssize_t
zpl_osmsg_write_try(struct zpl_osmsg *s, zpl_socket_t fd, zpl_uint32 size)
{
	ssize_t nbytes = 0;
	nbytes = ipstack_write(fd, s->data + s->getp, size);
	return nbytes;
}

/* Read up to size bytes into the zpl_osmsg from the fd, using recvmsgfrom
 * whose arguments match the remaining arguments to this function
 */
ssize_t
zpl_osmsg_sendto(struct zpl_osmsg *s, zpl_socket_t fd, zpl_uint32 size, zpl_uint32 flags,
				 struct ipstack_sockaddr *to, socklen_t *tolen)
{
	ssize_t nbytes = 0;

	nbytes = ipstack_sendto(fd, s->data + s->getp, size,
							flags, to, *tolen);
	return nbytes;
}

/* Read up to smaller of size or SIZE_REMAIN() bytes to the zpl_osmsg, starting
 * from endp.
 * First ipstack_iovec will be used to receive the data.
 * Stream need not be empty.
 */
ssize_t
zpl_osmsg_sendmsg(struct zpl_osmsg *s, zpl_socket_t fd, struct ipstack_msghdr *msgh, zpl_uint32 flags,
				  zpl_uint32 size)
{
	zpl_uint32 nbytes;
	struct ipstack_iovec *iov;

	iov = &(msgh->msg_iov[0]);
	iov->iov_base = (s->data + s->getp);
	iov->iov_len = size;
	nbytes = ipstack_sendmsg(fd, msgh, flags);
	return nbytes;
}


/* Write zpl_osmsg contens to the file discriptor. */
int zpl_osmsg_flush(struct zpl_osmsg *s, zpl_socket_t fd)
{
	zpl_uint32 nbytes = 0, alreadbytes = 0;
	ZPL_OSMSG_VERIFY_SANE(s);
	while(1)
	{
		nbytes = ipstack_write(fd, s->data + s->getp + alreadbytes, (s->endp - s->getp) - alreadbytes);
		if(nbytes <= 0)
		{
			if (!IPSTACK_ERRNO_RETRY(ipstack_errno))
				return -1;
		}
		alreadbytes += nbytes;
	}
	return alreadbytes;
}