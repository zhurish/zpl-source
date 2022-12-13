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

#ifndef STR_UTILS_H
#define STR_UTILS_H


#include <ortp/port.h>
#include <ortp/rtp_queue.h>

#ifdef __cplusplus
extern "C"{
#endif

ORTP_PUBLIC void ortp_recvaddr_to_sockaddr(ortp_recv_addr_t *recvaddr, struct ipstack_sockaddr *addr, socklen_t *socklen);
ORTP_PUBLIC void ortp_sockaddr_to_recvaddr(const struct ipstack_sockaddr * addr, ortp_recv_addr_t * recvaddr);

/*
ORTP_PUBLIC char * ortp_strdup_vprintf(const char *fmt, va_list ap);
ORTP_PUBLIC char *ortp_strdup_printf(const char *fmt,...);
ORTP_PUBLIC char * ortp_strcat_vprintf(char* dst, const char *fmt, va_list ap);
ORTP_PUBLIC char *ortp_strcat_printf(char* dst, const char *fmt,...);
*/
#ifdef __cplusplus
}
#endif

#endif
