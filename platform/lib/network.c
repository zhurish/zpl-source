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

#include <zebra.h>
#include "log.h"
#include "network.h"

/* Read nbytes from fd and store into ptr. */
int
readn (int fd, ospl_uchar *ptr, ospl_uint32 nbytes, ospl_uint32 type)
{
  ospl_uint32 nleft;
  ospl_uint32 nread;

  nleft = nbytes;

  while (nleft > 0) 
    {
	  if(type == IPCOM_STACK)
		  nread = ip_read (fd, ptr, nleft);
	  else
		  nread = read (fd, ptr, nleft);
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
writen(int fd, const ospl_uchar *ptr, ospl_uint32 nbytes, ospl_uint32 type)
{
  ospl_uint32 nleft;
  ospl_uint32 nwritten;

  nleft = nbytes;

  while (nleft > 0) 
    {
	  if(type == IPCOM_STACK)
		  nwritten = ip_write(fd, ptr, nleft);
	  else
		  nwritten = write(fd, ptr, nleft);
      if (nwritten <= 0) 
	return (nwritten);

      nleft -= nwritten;
      ptr += nwritten;
    }
  return nbytes - nleft;
}

int
set_nonblocking(int fd)
{
  ospl_uint32 flags;

  /* According to the Single UNIX Spec, the return value for F_GETFL should
     never be negative. */
  if ((flags = fcntl(fd, F_GETFL)) < 0)
    {
      zlog_warn(MODULE_DEFAULT, "fcntl(F_GETFL) failed for fd %d: %s",
      		fd, safe_strerror(errno));
      return -1;
    }
  if (fcntl(fd, F_SETFL, (flags | O_NONBLOCK)) < 0)
    {
      zlog_warn(MODULE_DEFAULT, "fcntl failed setting fd %d non-blocking: %s",
      		fd, safe_strerror(errno));
      return -1;
    }
  return 0;
}

int
set_blocking(int fd)
{
  ospl_uint32 flags;

  /* According to the Single UNIX Spec, the return value for F_GETFL should
     never be negative. */
  if ((flags = fcntl(fd, F_GETFL)) < 0)
    {
      zlog_warn(MODULE_DEFAULT, "fcntl(F_GETFL) failed for fd %d: %s",
      		fd, safe_strerror(errno));
      return -1;
    }
  flags &= ~O_NONBLOCK;
  if (fcntl(fd, F_SETFL, (flags)) < 0)
    {
      zlog_warn(MODULE_DEFAULT, "fcntl failed setting fd %d non-blocking: %s",
      		fd, safe_strerror(errno));
      return -1;
    }
  return 0;
}

ospl_float
htonf (ospl_float host)
{
#if !defined(__STDC_IEC_559__) && __GCC_IEC_559 < 0
#warning "Unknown floating-point format on platform, htonf may break"
#endif
  ospl_uint32 lu1, lu2;
  ospl_float convert;
  
  memcpy (&lu1, &host, sizeof (ospl_uint32));
  lu2 = htonl (lu1);
  memcpy (&convert, &lu2, sizeof (ospl_uint32));
  return convert;
}

ospl_float
ntohf (ospl_float net)
{
  return htonf (net);
}
