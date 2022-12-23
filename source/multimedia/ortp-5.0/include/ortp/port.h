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
/* this file is responsible of the portability of the stack */

#ifndef ORTP_PORT_H
#define ORTP_PORT_H

#ifdef __cplusplus
extern "C"{
#endif


#include "auto_include.h"


#undef min
#undef max




typedef zpl_socket_t ortp_socket_t;

typedef pthread_t ortp_thread_t;
typedef pthread_mutex_t ortp_mutex_t;
typedef pthread_cond_t ortp_cond_t;



#define ORTP_PUBLIC        extern
#define ORTP_INLINE			inline


int __ortp_thread_join(ortp_thread_t thread, void **ptr);
int __ortp_thread_create(ortp_thread_t *thread, pthread_attr_t *attr, void * (*routine)(void*), void *arg);
unsigned long __ortp_thread_self(void);



#define ortp_thread_create	__ortp_thread_create
#define ortp_thread_join	__ortp_thread_join
#define ortp_thread_self	__ortp_thread_self
#define ortp_thread_exit	pthread_exit
#define ortp_mutex_init		pthread_mutex_init
#define ortp_mutex_lock		pthread_mutex_lock
#define ortp_mutex_unlock	pthread_mutex_unlock
#define ortp_mutex_destroy	pthread_mutex_destroy
#define ortp_cond_init		pthread_cond_init
#define ortp_cond_signal	pthread_cond_signal
#define ortp_cond_broadcast	pthread_cond_broadcast
#define ortp_cond_wait		pthread_cond_wait
#define ortp_cond_destroy	pthread_cond_destroy

#define SOCKET_OPTION_VALUE	void *
#define SOCKET_BUFFER		void *

#define getSocketError() strerror(errno)
#define getSocketErrorCode() (errno)
#define ortp_gettimeofday(tv,tz) gettimeofday(tv,tz)
#define ortp_log10f(x)	log10f(x)




#define ortp_log10f(x)	log10f(x)




#ifndef _BOOL_T_
#define _BOOL_T_
typedef unsigned char bool_t;
#endif /* _BOOL_T_ */
#undef TRUE
#undef FALSE
#define TRUE 1
#define FALSE 0

typedef struct ortp_recv_addr {
	int family;
	union {
		struct ipstack_in_addr ipi_addr;
		struct ipstack_in6_addr ipi6_addr;
	} addr;
	unsigned short port;
} ortp_recv_addr_t;

typedef struct ortpTimeSpec{
	int64_t tv_sec;
	int64_t tv_nsec;
}ortpTimeSpec;

typedef struct _OrtpAddress{
    int family;
    union
    {
        struct ipstack_sockaddr_in ipv4;
        struct ipstack_sockaddr_in6 ipv6;
    }addr;
    socklen_t len;
}OrtpAddress;




ORTP_PUBLIC void* ortp_malloc(size_t sz);
ORTP_PUBLIC void ortp_free(void *ptr);
ORTP_PUBLIC void* ortp_realloc(void *ptr, size_t sz);
ORTP_PUBLIC char * ortp_strdup(const char *tmp);

/*override the allocator with this method, to be called BEFORE ortp_init()*/
typedef struct _OrtpMemoryFunctions{
	void *(*malloc_fun)(size_t sz);
	void *(*realloc_fun)(void *ptr, size_t sz);
	void (*free_fun)(void *ptr);
}OrtpMemoryFunctions;

void ortp_set_memory_functions(OrtpMemoryFunctions *functions);

#define ortp_new(type,count)	(type*)ortp_malloc(sizeof(type)*(count))
#define ortp_new0(type,count)	(type*)ortp_malloc(sizeof(type)*(count))

//#define _USER_ORTP_ADEP
ORTP_PUBLIC int close_socket(ortp_socket_t sock);
ORTP_PUBLIC int set_non_blocking_socket(ortp_socket_t sock);
ORTP_PUBLIC int set_blocking_socket(ortp_socket_t sock);

ORTP_PUBLIC int ortp_sockaddr_to_address(const struct ipstack_sockaddr *sa, socklen_t salen, char *ip, size_t ip_size, int *port);
ORTP_PUBLIC int ortp_sockaddr_to_print_address(struct ipstack_sockaddr *sa, socklen_t salen, char *printable_ip, size_t printable_ip_size);
ORTP_PUBLIC int ortp_address_to_sockaddr(int sin_family, char *ip, int port,
                             const struct ipstack_sockaddr *sa, socklen_t *salen);
ORTP_PUBLIC bool_t ortp_sockaddr_equals(const struct ipstack_sockaddr * sa, const struct ipstack_sockaddr * sb);
ORTP_PUBLIC char *ortp_strndup(const char *str,int n);
ORTP_PUBLIC char *ortp_strdup_printf(const char *fmt,...);
ORTP_PUBLIC char *ortp_strdup_vprintf(const char *fmt, va_list ap);
ORTP_PUBLIC char *ortp_strcat_printf(char *dst, const char *fmt,...);
ORTP_PUBLIC char *ortp_strcat_vprintf(char *dst, const char *fmt, va_list ap);


ORTP_PUBLIC void ortp_get_cur_time(ortpTimeSpec *ret);
void _ortp_get_cur_time(ortpTimeSpec *ret, bool_t realtime);
ORTP_PUBLIC uint64_t ortp_get_cur_time_ms(void);
ORTP_PUBLIC void ortp_sleep_ms(int ms);
ORTP_PUBLIC void ortp_sleep_until(const ortpTimeSpec *ts);
ORTP_PUBLIC int ortp_timespec_compare(const ortpTimeSpec *s1, const ortpTimeSpec *s2);
ORTP_PUBLIC unsigned int ortp_random(void);


ORTP_PUBLIC	bool_t ortp_is_multicast_addr(const struct ipstack_sockaddr *addr);
	



   #define ORTP_VAR_PUBLIC    extern


#ifndef IN6_IS_ADDR_MULTICAST
#define IN6_IS_ADDR_MULTICAST(i)	(((uint8_t *) (i))[0] == 0xff)
#endif


	
#ifdef __cplusplus
}

#endif
#endif


