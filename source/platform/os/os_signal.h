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

#define OS_SIGNAL_PIPE

typedef void (*os_signal_handler)(int signo, void *info);

#ifdef SA_SIGINFO
typedef void (*os_signal_abort_cb)(zpl_int signo, const char *action,
    siginfo_t *siginfo, void *context);
typedef void (*os_signal_exit_cb)(zpl_int signo, const char *action,
    siginfo_t *siginfo, void *context);
#else
typedef void (*os_signal_abort_cb)(zpl_int signo, const char *action);
typedef void (*os_signal_exit_cb)(zpl_int signo, const char *action);
#endif  

struct os_signal_t
{
  int signal;                     /* signal number    */
  os_signal_handler signal_handler;
  void	*info;
  volatile sig_atomic_t caught;   /* private member   */  
};

extern void os_signal_default(os_signal_abort_cb abort_func, os_signal_exit_cb exit_func);
extern void os_signal_init(struct os_signal_t *tbl, int num);
extern int os_signal_add(zpl_int sig, os_signal_handler hander);

extern int os_signal_send(zpl_int signo);
extern int os_signal_process(zpl_uint timeout);

#ifdef OS_SIGNAL_PIPE 
extern int os_signal_fd(void);
extern int os_signal_try_process(void);
#endif

#ifdef SA_SIGINFO
extern int os_register_signal(zpl_int , void (*handler)(zpl_int, siginfo_t *, void *));
#else
extern int os_register_signal(zpl_int , void (*handler)(zpl_int));                                            
#endif

#ifdef __cplusplus
}
#endif

#endif /* __OS_SIGNAL_H__ */
