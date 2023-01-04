/*
 * Buffering of output and input. 
 * Copyright (C) 1998 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 * 
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA. 
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"

#include "zmemory.h"
#include "buffer.h"
#include "log.h"
#include "network.h"


/* It should always be true that: 0 <= sp <= cp <= size */

/* Default buffer size (used if none specified).  It is rounded up to the
 next page boundery. */

#define BUFFER_DATA_FREE(D) XFREE(MTYPE_BUFFER_DATA, (D))

/* Make new buffer. */
struct buffer *
buffer_new(zpl_size_t size)
{
	struct buffer *b;

	b = XCALLOC(MTYPE_BUFFER, sizeof(struct buffer));

	if (size)
		b->size = size;
	else
	{
		static zpl_size_t default_size;
		if (!default_size)
		{
			long pgsz = sysconf(_SC_PAGESIZE);
			default_size = ((((BUFFER_SIZE_DEFAULT - 1) / pgsz) + 1) * pgsz);
		}
		b->size = default_size;
	}

	return b;
}

/* Free buffer. */
void buffer_free(struct buffer *b)
{
	buffer_reset(b);
	XFREE(MTYPE_BUFFER, b);
}

/* Make string clone. */
zpl_char *
buffer_getstr(struct buffer *b)
{
	zpl_size_t totlen = 0;
	struct buffer_data *data;
	zpl_char *s;
	zpl_char *p;

	for (data = b->head; data; data = data->next)
		totlen += data->cp - data->sp;
	if (!(s = XMALLOC(MTYPE_TMP, totlen + 1)))
		return NULL;
	p = s;
	for (data = b->head; data; data = data->next)
	{
		memcpy(p, data->data + data->sp, data->cp - data->sp);
		p += data->cp - data->sp;
	}
	*p = '\0';
	return s;
}

/* Return 1 if buffer is empty. */
zpl_bool buffer_empty(struct buffer *b)
{
	return (b->head == NULL);
}

/* Clear and free all allocated data. */
void buffer_reset(struct buffer *b)
{
	struct buffer_data *data;
	struct buffer_data *next;

	for (data = b->head; data; data = next)
	{
		next = data->next;
		BUFFER_DATA_FREE(data);
	}
	b->head = b->tail = NULL;
}

/* Add buffer_data to the end of buffer. */
static struct buffer_data *
buffer_add(struct buffer *b)
{
	struct buffer_data *d;

	d = XMALLOC(MTYPE_BUFFER_DATA, offsetof(struct buffer_data, data) + b->size);
	d->cp = d->sp = 0;
	d->next = NULL;

	if (b->tail)
		b->tail->next = d;
	else
		b->head = d;
	b->tail = d;

	return d;
}

/* Write data to buffer. */
void buffer_put(struct buffer *b, const void *p, zpl_size_t size)
{
	struct buffer_data *data = b->tail;
	const char *ptr = p;

	/* We use even last one byte of data buffer. */
	while (size)
	{
		zpl_size_t chunk;

		/* If there is no data buffer add it. */
		if (data == NULL || data->cp == b->size)
			data = buffer_add(b);

		chunk = ((size <= (b->size - data->cp)) ? size : (b->size - data->cp));
		memcpy((data->data + data->cp), ptr, chunk);
		size -= chunk;
		ptr += chunk;
		data->cp += chunk;
	}
}

void *buffer_dataptr(struct buffer *b)
{
	struct buffer_data *data = b->tail;
	if(data == NULL)
		data = buffer_add(b);
	return (data->data + data->cp);
}

void buffer_putemptyeize(struct buffer *b, zpl_size_t len)
{
	struct buffer_data *data = b->tail;
	//const char *ptr = p;

	/* We use even last one byte of data buffer. */
	while (len)
	{
		zpl_size_t chunk;

		/* If there is no data buffer add it. */
		if (data == NULL || data->cp == b->size)
			data = buffer_add(b);

		chunk = ((len <= (b->size - data->cp)) ? len : (b->size - data->cp));
		//memcpy((data->data + data->cp), ptr, chunk);
		len -= chunk;
		//ptr += chunk;
		data->cp += chunk;
	}
}
/* Insert character into the buffer. */
void buffer_putc(struct buffer *b, zpl_uchar c)
{
	buffer_put(b, &c, 1);
}

/* Put string to the buffer. */
void buffer_putstr(struct buffer *b, const char *c)
{
	buffer_put(b, c, strlen(c));
}

zpl_uint32 buffer_size(struct buffer *b)
{
	struct buffer_data *d = NULL;
	zpl_size_t nbyte = 0;

	for (d = b->head; d; d = d->next)
	{
		nbyte += (d->cp - d->sp);
	}
	return nbyte;
}
/* Keep flushing data to the fd until the buffer is empty or an error is
 encountered or the operation would block. */
buffer_status_t buffer_flush_all(struct buffer *b, zpl_socket_t fd)
{
	buffer_status_t ret;
	struct buffer_data *head;
	zpl_size_t head_sp;

	if (!b->head)
		return BUFFER_EMPTY;
	head_sp = (head = b->head)->sp;
	/* Flush all data. */
	while ((ret = buffer_flush_available(b, fd)) == BUFFER_PENDING)
	{
		if ((b->head == head) && (head_sp == head->sp) && (ipstack_errno != IPSTACK_ERRNO_EINTR))
			/* No data was flushed, so kernel buffer must be full. */
			return ret;
		head_sp = (head = b->head)->sp;
	}

	return ret;
}

/* Flush enough data to fill a terminal window of the given scene (used only
 by vty telnet interface). */
buffer_status_t buffer_flush_window(struct buffer *b, zpl_socket_t fd, zpl_uint32 width,
		zpl_uint32 height, zpl_uint32 erase_flag, zpl_uint32 no_more_flag)
{
	zpl_int32 nbytes;
	zpl_uint32 iov_alloc;
	zpl_uint32 iov_index;
	struct ipstack_iovec *iov;
	struct ipstack_iovec small_iov[3];
	zpl_char more[] = " --More-- ";
	zpl_char erase[] =
	{ 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, ' ', ' ', ' ',
			' ', ' ', ' ', ' ', ' ', ' ', ' ', 0x08, 0x08, 0x08, 0x08, 0x08,
			0x08, 0x08, 0x08, 0x08, 0x08 };
	struct buffer_data *data;
	zpl_uint32 column;

	if (!b->head)
		return BUFFER_EMPTY;

	if (height < 1)
	{
		zlog_warn(MODULE_DEFAULT,
				"%s called with non-positive window height %d, forcing to 1",
				__func__, height);
		height = 1;
	}
	else if (height >= 2)
		height--;
	if (width < 1)
	{
		zlog_warn(MODULE_DEFAULT,
				"%s called with non-positive window width %d, forcing to 1",
				__func__, width);
		width = 1;
	}

	/* For erase and more data add two to b's buffer_data count.*/
	if (b->head->next == NULL)
	{
		iov_alloc = array_size(small_iov);
		iov = small_iov;
	}
	else
	{
		iov_alloc = ((height * (width + 2)) / b->size) + 10;
		iov = XMALLOC(MTYPE_TMP, iov_alloc * sizeof(*iov));
	}
	iov_index = 0;

	/* Previously print out is performed. */
	if (erase_flag)
	{
		iov[iov_index].iov_base = erase;
		iov[iov_index].iov_len = sizeof erase;
		iov_index++;
	}

	/* Output data. */
	column = 1; /* Column position of next character displayed. */
	for (data = b->head; data && (height > 0); data = data->next)
	{
		zpl_size_t cp;

		cp = data->sp;
		while ((cp < data->cp) && (height > 0))
		{
			/* Calculate lines remaining and column position after displaying
			 this character. */
			if (data->data[cp] == '\r')
				column = 1;
			else if ((data->data[cp] == '\n') || (column == width))
			{
				column = 1;
				height--;
			}
			else
				column++;
			cp++;
		}
		iov[iov_index].iov_base = (zpl_char *) (data->data + data->sp);
		iov[iov_index++].iov_len = cp - data->sp;
		data->sp = cp;

		if (iov_index == iov_alloc)
		/* This should not ordinarily happen. */
		{
			iov_alloc *= 2;
			if (iov != small_iov)
			{
				zlog_warn(MODULE_DEFAULT, "%s: growing iov array to %d; "
						"width %d, height %d, size %lu", __func__, iov_alloc,
						width, height, (u_long) b->size);
				iov = XREALLOC(MTYPE_TMP, iov, iov_alloc * sizeof(*iov));
			}
			else
			{
				/* This should absolutely never occur. */
				zlog_err(MODULE_DEFAULT,
						"%s: corruption detected: iov_small overflowed; "
								"head %p, tail %p, head->next %p", __func__,
						(void *) b->head, (void *) b->tail,
						(void *) b->head->next);
				iov = XMALLOC(MTYPE_TMP, iov_alloc * sizeof(*iov));
				memcpy(iov, small_iov, sizeof(small_iov));
			}
		}
	}

	/* In case of `more' display need. */
	if (b->tail && (b->tail->sp < b->tail->cp) && !no_more_flag)
	{
		iov[iov_index].iov_base = more;
		iov[iov_index].iov_len = sizeof more;
		iov_index++;
	}

#ifdef IOV_MAX
	/* IOV_MAX are normally defined in <sys/uio.h> , Posix.1g.
	 example: Solaris2.6 are defined IOV_MAX size at 16.     */
	{
		struct ipstack_iovec *c_iov = iov;
		nbytes = 0; /* Make sure it's initialized. */

		while (iov_index > 0)
		{
			zpl_uint32 iov_size;

			iov_size = ((iov_index > IOV_MAX) ? IOV_MAX : iov_index);
			nbytes = ipstack_writev(fd, c_iov, iov_size);
			if (nbytes < 0)
			{
				zlog_warn(MODULE_DEFAULT, "%s: writev to fd %d failed: %s",
						__func__, ipstack_fd(fd), ipstack_strerror(ipstack_errno));
				break;
			}

			/* move pointer io-vector */
			c_iov += iov_size;
			iov_index -= iov_size;
		}
	}
#else  /* IOV_MAX */
		nbytes = ipstack_writev(fd, iov, iov_index);
	if (nbytes < 0)
		zlog_warn(MODULE_DEFAULT, "%s: writev to fd %d failed: %s", __func__, ipstack_fd(fd),
				ipstack_strerror(ipstack_errno));
#endif /* IOV_MAX */

	/* Free printed buffer data. */
	while (b->head && (b->head->sp == b->head->cp))
	{
		struct buffer_data *del;
		if (!(b->head = (del = b->head)->next))
			b->tail = NULL;
		BUFFER_DATA_FREE(del);
	}

	if (iov != small_iov)
		XFREE(MTYPE_TMP, iov);

	return (nbytes < 0) ?
			BUFFER_ERROR : (b->head ? BUFFER_PENDING : BUFFER_EMPTY);
}

/* This function (unlike other buffer_flush* functions above) is designed
 to work with non-blocking sockets.  It does not attempt to write out
 all of the queued data, just a "big" chunk.  It returns 0 if it was
 able to empty out the buffers completely, 1 if more flushing is
 required later, or -1 on a fatal write error. */
buffer_status_t buffer_flush_available(struct buffer *b, zpl_socket_t fd)
{

	/* These are just reasonable values to make sure a significant amount of
	 data is written.  There's no need to go crazy and try to write it all
	 in one shot. */
#ifdef IOV_MAX
#define MAX_CHUNKS ((IOV_MAX >= 16) ? 16 : IOV_MAX)
#else
#define MAX_CHUNKS 16
#endif
#define MAX_FLUSH 131072

	struct buffer_data *d = NULL;
	zpl_int32 written = 0;
	struct ipstack_iovec iov[MAX_CHUNKS];
	zpl_size_t iovcnt = 0;
	zpl_size_t nbyte = 0;

	for (d = b->head; d && (iovcnt < MAX_CHUNKS) && (nbyte < MAX_FLUSH);
			d = d->next, iovcnt++)
	{
		iov[iovcnt].iov_base = d->data + d->sp;
		nbyte += (iov[iovcnt].iov_len = d->cp - d->sp);
	}

	if (!nbyte)
		/* No data to flush: should we issue a warning message? */
		return BUFFER_EMPTY;

		written = ipstack_writev(fd, iov, iovcnt);
	/* only place where written should be sign compared */
	if (written < 0)
	{
		if (IPSTACK_ERRNO_RETRY(ipstack_errno))
			/* Calling code should try again later. */
			return BUFFER_PENDING;
		zlog_warn(MODULE_DEFAULT, "%s: write error on fd %d: %s", __func__, ipstack_fd(fd),
				ipstack_strerror(ipstack_errno));
		return BUFFER_ERROR;
	}
	if(written > nbyte || written > 0xffff)
	{
		buffer_reset(b);
		return BUFFER_EMPTY;
	}
	/* Free printed buffer data. */
	while (written > 0)
	{
		//struct buffer_data *d = NULL;
		if (!(d = b->head))
		{
			zlog_err(MODULE_DEFAULT,
					"%s: corruption detected: buffer queue empty, "
							"but written is %lu", __func__, (u_long) written);
			break;
		}
		if (written < d->cp - d->sp)
		{
			d->sp += written;
			return BUFFER_PENDING;
		}

		written -= (d->cp - d->sp);
		if (!(b->head = d->next))
			b->tail = NULL;
		BUFFER_DATA_FREE(d);
	}

	return b->head ? BUFFER_PENDING : BUFFER_EMPTY;

#undef MAX_CHUNKS
#undef MAX_FLUSH
}

buffer_status_t buffer_write(struct buffer *b, zpl_socket_t fd, const void *p,
		zpl_size_t size)
{
	ssize_t nbytes;

#if 0
	/* Should we attempt to drain any previously buffered data?  This could help
	 reduce latency in pushing out the data if we are stuck in a long-running
	 thread that is preventing the main select loop from calling the flush
	 thread... */
	if (b->head && (buffer_flush_available(b, fd, type) == BUFFER_ERROR))
	return BUFFER_ERROR;
#endif
	if (b->head)
		/* Buffer is not empty, so do not attempt to write the new data. */
		nbytes = 0;
	else
	{
		nbytes = ipstack_writev(fd, p, size);
		if (nbytes < 0)
		{
			if (IPSTACK_ERRNO_RETRY(ipstack_errno))
				nbytes = 0;
			else
			{
				zlog_warn(MODULE_DEFAULT, "%s: write error on fd %d: %s",
						__func__, ipstack_fd(fd), ipstack_strerror(ipstack_errno));
				return BUFFER_ERROR;
			}
		}
	}
	/* Add any remaining data to the buffer. */
	{
		zpl_size_t written = nbytes;
		if (written < size)
			buffer_put(b, ((const char *) p) + written, size - written);
	}
	return b->head ? BUFFER_PENDING : BUFFER_EMPTY;
}