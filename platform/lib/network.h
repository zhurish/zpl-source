/*
 * Network library header.
 * Copyright (C) 1998 Kunihiro Ishiguro
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

#ifndef _ZEBRA_NETWORK_H
#define _ZEBRA_NETWORK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Both readn and writen are deprecated and will be removed.  They are not
   suitable for use with non-blocking file descriptors.
 */
extern int readn (zpl_socket_t, zpl_uchar *, zpl_uint32);
extern int writen (zpl_socket_t, const zpl_uchar *, zpl_uint32);


/* Does the I/O error indicate that the operation should be retried later? */
#define IPSTACK_ERRNO_RETRY(EN) \
	(((EN) == IPSTACK_ERRNO_EAGAIN) || ((EN) == IPSTACK_ERRNO_EWOULDBLOCK) || ((EN) == IPSTACK_ERRNO_EINTR))

extern zpl_float htonf (zpl_float);
extern zpl_float ntohf (zpl_float);
 
#ifdef __cplusplus
}
#endif

#endif /* _ZEBRA_NETWORK_H */
