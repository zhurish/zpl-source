/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
 *
 * This file is part of oRTP.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include "ortp-config.h"
#endif

#include <ortp/port.h>
#include <ortp/logging.h>
#include <ortp/ortp_list.h>
#include <ortp/extremum.h>
#include <ortp/rtp_queue.h>
#include <ortp/rtp.h>
#include <ortp/rtcp.h>
#include <ortp/sessionset.h>
#include <ortp/payloadtype.h>
#include <ortp/rtpprofile.h>

#include <ortp/rtpsession_priv.h>
#include <ortp/rtpsession.h>
#include <ortp/event.h>
#include "ortp/str_utils.h"
#include "utils.h"




void ortp_recvaddr_to_sockaddr(ortp_recv_addr_t *recvaddr, struct sockaddr *addr, socklen_t *socklen) {
	if (recvaddr->family == AF_INET) {
		struct sockaddr_in *addr_in = (struct sockaddr_in *)addr;
		addr_in->sin_family = AF_INET;
		addr_in->sin_addr = recvaddr->addr.ipi_addr;
		addr_in->sin_port = recvaddr->port;
		*socklen = sizeof(struct sockaddr_in);
	} else if (recvaddr->family == AF_INET6) {
		struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)addr;
		addr_in6->sin6_family = AF_INET6;
		addr_in6->sin6_port = recvaddr->port;
		memcpy(&addr_in6->sin6_addr, &recvaddr->addr.ipi6_addr, sizeof(recvaddr->addr.ipi6_addr));
		*socklen = sizeof(struct sockaddr_in6);
	}else{
		*socklen = 0;
	}
}
void ortp_sockaddr_to_recvaddr(const struct sockaddr * addr, ortp_recv_addr_t * recvaddr) {
	if( addr->sa_family == AF_INET) {
		struct sockaddr_in * addr_in = (struct sockaddr_in *)addr;
		recvaddr->family = AF_INET;
		recvaddr->port = addr_in->sin_port;
		recvaddr->addr.ipi_addr = addr_in->sin_addr;
	}else if( addr->sa_family == AF_INET6) {
		struct sockaddr_in6 * addr_in6 = (struct sockaddr_in6 *)addr;
		recvaddr->family = AF_INET6;
		recvaddr->port = addr_in6->sin6_port;
		memcpy(&recvaddr->addr.ipi6_addr, &addr_in6->sin6_addr, sizeof(addr_in6->sin6_addr));
	}
}



char * ortp_strdup_vprintf(const char *fmt, va_list ap)
{
/* Guess we need no more than 100 bytes. */
	int n, size = 200;
	char *p,*np;
#ifndef _WIN32
	va_list cap;/*copy of our argument list: a va_list cannot be re-used (SIGSEGV on linux 64 bits)*/
#endif
	if ((p = (char *) ortp_malloc (size)) == NULL)
		return NULL;
	while (1){
/* Try to print in the allocated space. */
#ifndef _WIN32
		va_copy(cap,ap);
		n = vsnprintf (p, size, fmt, cap);
		va_end(cap);
#else
		/*this works on 32 bits, luckily*/
		n = vsnprintf (p, size, fmt, ap);
#endif
		/* If that worked, return the string. */
		if (n > -1 && n < size)
			return p;
//printf("Reallocing space.\n");
		/* Else try again with more space. */
		if (n > -1)	/* glibc 2.1 */
			size = n + 1;	/* precisely what is needed */
		else		/* glibc 2.0 */
			size *= 2;	/* twice the old size */
		if ((np = (char *) ortp_realloc (p, size)) == NULL)
		{
			free(p);
			return NULL;
		} else {
			p = np;
		}
	}
}

char *ortp_strdup_printf(const char *fmt,...){
	char *ret;
	va_list args;
	va_start (args, fmt);
	ret=ortp_strdup_vprintf(fmt, args);
	va_end (args);
	return ret;
}

char * ortp_strcat_vprintf(char* dst, const char *fmt, va_list ap){
	char *ret;
	size_t dstlen, retlen;

	ret=ortp_strdup_vprintf(fmt, ap);
	if (!dst) return ret;

	dstlen = strlen(dst);
	retlen = strlen(ret);

	if ((dst = ortp_realloc(dst, dstlen+retlen+1)) != NULL){
		strcat(dst,ret);
		ortp_free(ret);
		return dst;
	} else {
		ortp_free(ret);
		return NULL;
	}
}

char *ortp_strcat_printf(char* dst, const char *fmt,...){
	char *ret;
	va_list args;
	va_start (args, fmt);
	ret=ortp_strcat_vprintf(dst, fmt, args);
	va_end (args);
	return ret;
}