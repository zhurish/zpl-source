/*
 * Network library.
 * Copyright (C) 1997 Kunihiro Ishiguro
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

#include "log.h"
#include "network.h"

/* Read nbytes from fd and store into ptr. */
int
readn (zpl_socket_t fd, zpl_uchar *ptr, zpl_uint32 nbytes)
{
  zpl_uint32 nleft;
  zpl_uint32 nread;

  nleft = nbytes;

  while (nleft > 0) 
    {
		  nread = ipstack_read (fd, ptr, nleft);
      if (nread < 0) 
	return (nread);
      else
	if (nread == 0) 
	  break;

      nleft -= nread;
      ptr += nread;
    }

  return nbytes - nleft;
}  

/* Write nbytes from ptr to fd. */
int
writen(zpl_socket_t fd, const zpl_uchar *ptr, zpl_uint32 nbytes)
{
  zpl_uint32 nleft;
  zpl_uint32 nwritten;

  nleft = nbytes;

  while (nleft > 0) 
    {
		  nwritten = ipstack_write(fd, ptr, nleft);
      if (nwritten <= 0) 
	return (nwritten);

      nleft -= nwritten;
      ptr += nwritten;
    }
  return nbytes - nleft;
}


zpl_float
htonf (zpl_float host)
{
#if !defined(__STDC_IEC_559__) && __GCC_IEC_559 < 0
#warning "Unknown floating-point format on platform, htonf may break"
#endif
  zpl_uint32 lu1, lu2;
  zpl_float convert;
  
  memcpy (&lu1, &host, sizeof (zpl_uint32));
  lu2 = htonl (lu1);
  memcpy (&convert, &lu2, sizeof (zpl_uint32));
  return convert;
}

zpl_float
ntohf (zpl_float net)
{
  return htonf (net);
}
