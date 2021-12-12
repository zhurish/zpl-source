/*
 * os_signal.h
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#ifndef __OS_SIGNAL_H__
#define __OS_SIGNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*os_signal_handler)(int signo, void *info);

extern void os_signal_default();
extern int os_signal_add(zpl_int sig, os_signal_handler hander);
extern int os_signal_handler_action(zpl_int sig, void *info);
extern int os_signal_process(void);

extern int os_register_signal(zpl_int sig, void (*handler)(zpl_int
#ifdef SA_SIGINFO
	     , siginfo_t *siginfo, void *context
#endif
		));



#ifdef __cplusplus
}
#endif

#endif /* __OS_SIGNAL_H__ */
