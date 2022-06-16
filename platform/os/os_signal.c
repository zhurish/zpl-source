/*
 * os_util.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"

#if 1

#ifdef SA_SIGINFO
#ifdef HAVE_UCONTEXT_H
#ifdef GNU_LINUX
/* get REG_EIP from ucontext.h */
#ifndef __USE_GNU
#define __USE_GNU
#endif /* __USE_GNU */
#endif /* GNU_LINUX */
#include <ucontext.h>
#endif /* HAVE_UCONTEXT_H */
#endif /* SA_SIGINFO */

struct os_signal_process_t
{
  int signal_rfd;
  int signal_wfd;
#ifdef SA_SIGINFO
  void (*os_core_abort)(zpl_int signo, const char *action,
    siginfo_t *siginfo, void *context);
  void (*os_core_exit)(zpl_int signo, const char *action,
    siginfo_t *siginfo, void *context);
#else
  void (*os_core_abort)(zpl_int signo, const char *action);
  void (*os_core_exit)(zpl_int signo, const char *action);
#endif  
  struct os_signal_t os_signal_tbl[__SIGRTMAX];
};

struct sigmap_tbl
{
  zpl_uint32 signo;
#ifdef SA_SIGINFO
  void (*handler)(int signo, siginfo_t *info, void *context);
#else
  void (*handler)(int signo);
#endif
};

struct os_signal_process_t os_sigproc_tbl;

static void os_signal_handler_action(zpl_int sig, void *info);


#ifdef SA_SIGINFO
static void os_signal_default_interrupt(zpl_int signo, siginfo_t *siginfo, void *context);
#else
static void os_signal_default_interrupt(zpl_int signo);
#endif


#ifndef ZPL_COREDUMP_ENABLE

#ifdef SA_SIGINFO
static void os_core_exit_handler(int signo, siginfo_t *siginfo, void *context);
static void os_core_abort_handler(int signo, siginfo_t *siginfo, void *context);
#else
static void os_core_exit_handler(int signo);
static void os_core_abort_handler(int signo);
#endif
#endif


static const struct sigmap_tbl default_signals[] = {
#ifndef ZPL_COREDUMP_ENABLE
    { SIGQUIT,    os_core_abort_handler},
    { SIGILL,    os_core_abort_handler},
#ifdef SIGEMT
    { SIGEMT,    os_core_abort_handler},
#endif
    { SIGFPE,    os_core_abort_handler},
    { SIGBUS,    os_core_abort_handler},
    { SIGSEGV,    os_core_abort_handler},
#ifdef SIGSYS
    { SIGSYS,    os_core_abort_handler},
#endif
#ifdef SIGXCPU
    { SIGXCPU,    os_core_abort_handler},
#endif
#ifdef SIGXFSZ
    { SIGXFSZ,    os_core_abort_handler},
#endif

    { SIGHUP,     os_core_exit_handler},//exit_signals
    { SIGINT,     os_core_exit_handler},
    { SIGALRM,     os_core_exit_handler},
    { SIGTERM,     os_core_exit_handler},
    { SIGUSR1,     os_core_exit_handler},
// { SIGUSR2,     os_core_exit_handler},
#ifdef SIGPOLL
    { SIGPOLL,     os_core_exit_handler},
#endif
#ifdef SIGVTALRM
    { SIGVTALRM,     os_core_exit_handler},
#endif
#ifdef SIGSTKFLT
    { SIGSTKFLT,     os_core_exit_handler},
#endif
#endif
    { SIGPIPE,  NULL},//ignore_signals
    {0,  NULL},
};


int os_signal_add(zpl_int sig, os_signal_handler hander)
{
  os_sigproc_tbl.os_signal_tbl[sig].signal = sig;
  os_sigproc_tbl.os_signal_tbl[sig].signal_handler = hander;
  os_register_signal(sig, os_signal_default_interrupt);
  return OK;
}

void os_signal_init(struct os_signal_t *tbl, int num)
{
  uint32_t i = 0;
  for (i = 0; i < num; i++)
  {
    os_signal_add(tbl[i].signal, tbl[i].signal_handler);
  }
#ifdef ZPL_COREDUMP_ENABLE
  super_system("ulimit -c unlimited");
  super_system("echo 1 > /proc/sys/kernel/core_uses_pid");
  // core.filename.pid格式的core dump文件：
  super_system("echo \""SYSTMPDIR"/coredump.%e.%p\" > /proc/sys/kernel/core_pattern");
  // super_system("ulimit -c unlimited");
  // super_system("ulimit -c unlimited");
  // super_system("ulimit -c unlimited");
#endif
}

static void os_signal_handler_action(zpl_int sig, void *info)
{
  int i = 0;
  for (i = 0; i < __SIGRTMAX; i++)
  {
    if (os_sigproc_tbl.os_signal_tbl[i].signal == sig &&
        os_sigproc_tbl.os_signal_tbl[i].signal_handler)
      (os_sigproc_tbl.os_signal_tbl[sig].signal_handler)(sig, os_sigproc_tbl.os_signal_tbl[i].info ? os_sigproc_tbl.os_signal_tbl[i].info : info);
  }
  return;
}

#ifdef SA_SIGINFO
static void os_signal_default_interrupt(zpl_int signo, siginfo_t *siginfo, void *context)
#else
static void os_signal_default_interrupt(zpl_int signo)                                     
#endif
{
  fprintf(stdout, "======================os_signal_default_interrupt %d\r\n", signo);
  fflush(stdout);
  os_log(OS_SIGNAL_FILE, "%s:==========+++++++++++++++++++++++++=========signo=%d\r\n", __func__, signo);
#ifdef OS_SIGNAL_PIPE  
  zpl_int wsigno = signo;
  if (os_sigproc_tbl.signal_wfd)
    write(os_sigproc_tbl.signal_wfd, &wsigno, 4);
#else
  os_signal_handler_action(signo, NULL);
#endif
  return;
}

int os_signal_send(zpl_uint signo)
{
#ifdef OS_SIGNAL_PIPE  
  zpl_int wsigno = signo;
  if (os_sigproc_tbl.signal_wfd)
    write(os_sigproc_tbl.signal_wfd, &wsigno, 4);
#else
  kill(getpid(), signo);    
#endif
  return 0;
}

int os_signal_process(zpl_uint timeout)
{
#ifdef OS_SIGNAL_PIPE 
  zpl_int rsigno = 0;
  if (os_read_timeout(os_sigproc_tbl.signal_rfd, &rsigno, 4, timeout) == 4)
  {
    os_signal_handler_action(rsigno, NULL);
  }
#else
  os_msleep(timeout);
#endif  
  return OK;
}

static void os_trap_default_signals(void)
{
  zpl_uint32 i = 0;

  for (i = 0; i < array_size(default_signals); i++)
  {
    if(default_signals[i].signo)
    {
      os_register_signal(default_signals[i].signo, default_signals[i].handler);
    }
  }
}

void os_signal_default(os_signal_abort_cb abort_func, os_signal_exit_cb exit_func)
{
  memset(os_sigproc_tbl.os_signal_tbl, 0, sizeof(os_sigproc_tbl.os_signal_tbl));
  os_sigproc_tbl.os_core_abort = abort_func;
  os_sigproc_tbl.os_core_exit = exit_func;
#ifdef OS_SIGNAL_PIPE 
  os_unix_sockpair_create(zpl_false, &os_sigproc_tbl.signal_rfd, &os_sigproc_tbl.signal_wfd);
  if (os_sigproc_tbl.signal_wfd)
    os_set_nonblocking(os_sigproc_tbl.signal_wfd);
  if (os_sigproc_tbl.signal_rfd)
    os_set_nonblocking(os_sigproc_tbl.signal_rfd);
#endif
  os_trap_default_signals();
}

#ifdef SA_SIGINFO
int os_register_signal(zpl_int sig, void (*handler)(zpl_int, siginfo_t *siginfo, void *context))
#else
int os_register_signal(zpl_int sig, void (*handler)(zpl_int))                                                 
#endif
{
  int ret = 0;
  struct sigaction sa, osa;

  memset(&sa, 0, sizeof(sa));
  memset(&osa, 0, sizeof(osa));

	ret = sigaction(sig, NULL, &osa);
  if(ret == 0)
  {
    /*if(osa.sa_handler != SIG_DFL)
    {
      return (0);
    }*/
  }
  else
  {
    return -1;
  }

  if (handler == NULL)
  {
    sa.sa_handler = SIG_DFL; // SIG_IGN;
    sa.sa_flags = 0;
  }
  else
  {
#ifdef SA_SIGINFO
    sa.sa_sigaction = handler;
    sa.sa_flags = SA_SIGINFO;
#else
    sa.sa_handler = handler;
    sa.sa_flags = 0;
#endif
  }
  if (sig == SIGALRM)
  {
#ifdef SA_INTERRUPT
    sa.sa_flags |= SA_INTERRUPT; /* SunOS */
#endif
  }
  else
  {
#ifdef SA_RESTART
    sa.sa_flags |= SA_RESTART;
#endif /* SA_RESTART */
  }
  sigfillset(&sa.sa_mask);
  if (sigaction(sig, &sa, NULL) < 0)
  {
    fprintf(stdout, "Unable to set up signal handler for %d", sig);
    return (-1);
  }
  return (0);
}

#ifndef ZPL_COREDUMP_ENABLE
#ifdef SA_SIGINFO

/* XXX This function should be enhanced to support more platforms
       (it currently works only on Linux/x86). */
static void *
os_signal_program_counter(void *context)
{
#ifdef HAVE_UCONTEXT_H
#ifdef GNU_LINUX
  /* these are from GNU libc, rather than Linux, strictly speaking */
#if defined(REG_EIP)
#define REG_INDEX REG_EIP
#elif defined(REG_RIP)
#define REG_INDEX REG_RIP
#elif defined(__powerpc__)
#define REG_INDEX 32
#endif
#elif defined(SUNOS_5) /* !GNU_LINUX */
#define REG_INDEX REG_PC
#endif /* GNU_LINUX */

#ifdef REG_INDEX
#ifdef HAVE_UCONTEXT_T_UC_MCONTEXT_GREGS
#define REGS gregs[REG_INDEX]
#elif defined(HAVE_UCONTEXT_T_UC_MCONTEXT_UC_REGS)
#define REGS uc_regs->gregs[REG_INDEX]
#endif /* HAVE_UCONTEXT_T_UC_MCONTEXT_GREGS */
#endif /* REG_INDEX */

#ifdef REGS
  if (context)
    return (void *)(((ucontext_t *)context)->uc_mcontext.REGS);
#elif defined(HAVE_UCONTEXT_T_UC_MCONTEXT_REGS__NIP)
  /* older Linux / struct pt_regs ? */
  if (context)
    return (void *)(((ucontext_t *)context)->uc_mcontext.regs->nip);
#endif /* REGS */

#endif /* HAVE_UCONTEXT_H */
  return NULL;
}
#endif /* SA_SIGINFO */

#ifdef SA_SIGINFO
static void __attribute__((noreturn)) os_core_exit_handler(int signo, siginfo_t *siginfo, void *context)
#else
static void __attribute__((noreturn)) os_core_exit_handler(int signo)        
#endif
{
  fprintf(stdout,"%s:============+++++++++++++++++++++===========signo=%d\r\n", __func__, signo);
  fflush(stdout);
  os_log(OS_SIGNAL_FILE, "%s:==========+++++++++++++++++++++++++=========signo=%d\r\n", __func__, signo);
  if (os_sigproc_tbl.os_core_exit)
    (os_sigproc_tbl.os_core_exit)(signo, "exiting..."
#ifdef SA_SIGINFO
                                  ,
                                  siginfo, os_signal_program_counter(context)
#endif
    );
  _exit(128 + signo);
}

#ifdef SA_SIGINFO
static void __attribute__((noreturn)) os_core_abort_handler(int signo, siginfo_t *siginfo, void *context)
#else
static void __attribute__((noreturn)) os_core_abort_handler(int signo)        
#endif
{
  fprintf(stdout,"%s:==========+++++++++++++++++++++++++=========signo=%d\r\n", __func__, signo);
  fflush(stdout);
  os_log(OS_SIGNAL_FILE, "%s:==========+++++++++++++++++++++++++=========signo=%d\r\n", __func__, signo);
  if (os_sigproc_tbl.os_core_abort)
    (os_sigproc_tbl.os_core_abort)(signo, "aborting..."
#ifdef SA_SIGINFO
                                   ,
                                   siginfo, os_signal_program_counter(context)
#endif
    );
  abort();
}

#endif

int os_signal_reload_test(void)
{
  #ifndef ZPL_COREDUMP_ENABLE
  os_register_signal(SIGSEGV, os_core_abort_handler);
  os_register_signal(SIGBUS, os_core_abort_handler);
  #endif
  return OK;
}

#endif 