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

#define OS_SIGNAL_FILE SYSLOGDIR"/signal.log"

typedef int (*os_signal_handler)(int signo, void *info);

struct os_signal_t
{
  int signal;                     /* signal number    */
  os_signal_handler signal_handler;
  void	*info;
};

extern void os_signal_default(void *abort_func, void *exit_func);
extern void os_signal_init(struct os_signal_t *tbl, int num);
extern int os_signal_add(zpl_int sig, os_signal_handler hander);

extern int os_signal_process(zpl_uint timeout);

#if 0
extern int os_signal_handler_action(zpl_int sig, void *info);
#endif
extern int os_register_signal(zpl_int sig, void (*handler)(zpl_int
#ifdef SA_SIGINFO
	     , siginfo_t *siginfo, void *context
#endif
		));



#ifdef __cplusplus
}
#endif

#endif /* __OS_SIGNAL_H__ */
