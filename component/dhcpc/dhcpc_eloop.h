/* 
 * dhcpcd - DHCP client daemon
 * Copyright (c) 2006-2010 Roy Marples <roy@marples.name>
 * All rights reserved

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef DHCPC_ELOOP_H
#define DHCPC_ELOOP_H

#include <time.h>

#ifndef ELOOP_QUEUE
  #define ELOOP_QUEUE 0
#endif


#ifdef OS_ELOOP_THREAD

extern os_ansync_lst *dhcp_lstmaster;

extern int start_eloop_init(void);
extern int start_eloop_exit(void);
extern void start_eloop(void);


/*
void add_event(int fd, void (*)(void *), void *);
void delete_event(int fd);
*/

#define add_event(i, f, v) 	os_ansync_add(dhcp_lstmaster, OS_ANSYNC_INPUT, f, v, i)
#define delete_event(i)		os_ansync_del(dhcp_lstmaster, OS_ANSYNC_INPUT, NULL, NULL, i)

#define add_timeout_sec(i, f, v) 	os_ansync_add(dhcp_lstmaster, OS_ANSYNC_TIMER_ONCE, f, v, i * 1000)
#define delete_timeout(f, v)		os_ansync_del_all(dhcp_lstmaster, OS_ANSYNC_TIMER_ONCE, f, v, 0)

#define add_timeout_tv(i, f, v) 	os_ansync_add(dhcp_lstmaster, OS_ANSYNC_TIMER_ONCE, f, v, \
											(i)->tv_sec * 1000 + ((i)->tv_usec%1000)*1000)

#define delete_q_timeout(i, f, v)		os_ansync_del_all(dhcp_lstmaster, OS_ANSYNC_TIMER_ONCE, f, v, i)

/*
		tv.tv_sec = state->interval + DHCP_RAND_MIN;
		tv.tv_usec = arc4random() % (DHCP_RAND_MAX_U - DHCP_RAND_MIN_U);
#define os_ansync_register_api(l, t, c, p, v)	_os_ansync_register_api(l, t, c, p, v, #c, __FILE__, __LINE__)
#define os_ansync_unregister_api(l, t, c, p, v)	_os_ansync_unregister_api(l, t, c, p, v)


#define add_timeout_tv(a, b, c) add_q_timeout_tv(ELOOP_QUEUE, a, b, c)
#define delete_timeouts(a, ...) delete_q_timeouts(ELOOP_QUEUE, a, __VA_ARGS__)
*/

#else

#define add_timeout_tv(a, b, c) add_q_timeout_tv(ELOOP_QUEUE, a, b, c)
#define add_timeout_sec(a, b, c) add_q_timeout_sec(ELOOP_QUEUE, a, b, c)
#define delete_timeout(a, b) delete_q_timeout(ELOOP_QUEUE, a, b)
#define delete_timeouts(a, ...) delete_q_timeouts(ELOOP_QUEUE, a, __VA_ARGS__)

extern void add_event(int fd, void (*)(void *), void *);
extern void delete_event(int fd);
extern void add_q_timeout_sec(int queue, time_t, void (*)(void *), void *);
extern void add_q_timeout_tv(int queue, const struct timeval *, void (*)(void *),
    void *);
extern void delete_q_timeout(int, void (*)(void *), void *);
extern void delete_q_timeouts(int, void *, void (*)(void *), ...);
extern void start_eloop(void);
#endif


#endif
