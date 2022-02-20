/* Quagga signal handling functions.
 * Copyright (C) 2004 Paul Jakma,
 *
 * This file is part of Quagga.
 *
 * Quagga is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * Quagga is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Quagga; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include "os_include.h"
#include "zpl_include.h"
#include "lib_include.h"

#include "sigevent.h"

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

static const int core_signals[] = {
    SIGQUIT,
    SIGILL,
#ifdef SIGEMT
    SIGEMT,
#endif
    SIGFPE,
    SIGBUS,
    SIGSEGV,
#ifdef SIGSYS
    SIGSYS,
#endif
#ifdef SIGXCPU
    SIGXCPU,
#endif
#ifdef SIGXFSZ
    SIGXFSZ,
#endif
};

static const int exit_signals[] = {
    SIGHUP,
    SIGINT,
    SIGALRM,
    SIGTERM,
    SIGUSR1,
// SIGUSR2,
#ifdef SIGPOLL
    SIGPOLL,
#endif
#ifdef SIGVTALRM
    SIGVTALRM,
#endif
#ifdef SIGSTKFLT
    SIGSTKFLT,
#endif
};
static const int ignore_signals[] = {
    SIGPIPE,
};

/* master signals descriptor struct */
struct quagga_sigevent_master_t
{
  pthread_t tid;

  struct quagga_signal_t *signals;
  int sigc;

  volatile sig_atomic_t caught;
} sigmaster;

/* Generic signal handler
 * Schedules signal event thread
 */
static void
quagga_signal_handler(int signo)
{
  zpl_uint32 i;
  struct quagga_signal_t *sig;
  sigmaster.tid = pthread_self();
  for (i = 0; i < sigmaster.sigc; i++)
  {
    sig = &(sigmaster.signals[i]);

    if (sig->signal == signo)
      sig->caught = 1;
    // quagga_sigevent_process ();
  }
  sigmaster.caught = 1;
}

/* check if signals have been caught and run appropriate handlers */
#ifdef QUAGGA_SIGNAL_SIGWAIT
int quagga_sigevent_process(void)
{
  int err = 0, signo = 0;
  sigset_t set;
  sigfillset(&set);         //把所有信号加入到集合中，信号集中将包含Linux支持的64种信号
  sigdelset(&set, SIGUSR2); //将指定信号从信号集中删去。
  for (;;)
  {
    err = sigwait(&set, &signo);
    if (err != 0)
    {
      err_exit("sigwait failed\n");
    }

    switch (signo)
    {
    case SIGTERM:
      pr_info("Catch SIGTERM; exiting\n");
      break;
    case SIGQUIT:
      pr_info("Catch SIGQUIT; exiting\n");
      exit(0);
      break;
    case SIGINT:
      pr_info("Catch SIGINT; exiting\n");
      break;

    default:
      pr_info("Unexpected signal %d\n", signo);
    }
  }
}
#else
int quagga_sigevent_process(void)
{
  struct quagga_signal_t *sig;
  zpl_uint32 i;
#ifdef SIGEVENT_BLOCK_SIGNALS
  /* shouldnt need to block signals, but potentially may be needed */
  sigset_t newmask, oldmask;

  /*
   * Block most signals, but be careful not to defer SIGTRAP because
   * doing so breaks gdb, at least on NetBSD 2.0.  Avoid asking to
   * block SIGKILL, just because we shouldn't be able to do so.
   */
  sigfillset(&newmask);
  sigdelset(&newmask, SIGTRAP);
  sigdelset(&newmask, SIGKILL);
  //设置信号屏蔽位函数
  if ((sigprocmask(SIG_BLOCK, &newmask, &oldmask)) < 0)
  {
    zlog_err(MODULE_DEFAULT, "quagga_signal_timer: couldnt block signals!");
    return -1;
  }
#endif /* SIGEVENT_BLOCK_SIGNALS */

  /*
    if(sigmaster.tid != pthread_self())
      return 0;
  */

  if (sigmaster.caught > 0)
  {
    sigmaster.caught = 0;
    /* must not read or set sigmaster.caught after here,
     * race condition with per-sig caught flags if one does
     */

    for (i = 0; i < sigmaster.sigc; i++)
    {
      sig = &(sigmaster.signals[i]);

      if (sig->caught > 0)
      {
        sig->caught = 0;
        os_signal_handler_action(sig->signal, NULL);
        if (sig->handler)
          sig->handler();
        if (sig->signal_handler)
          sig->signal_handler(sig->signal);
      }
    }
  }

#ifdef SIGEVENT_BLOCK_SIGNALS
  if (sigprocmask(SIG_UNBLOCK, &oldmask, NULL) < 0)
    ;
  return -1;
#endif /* SIGEVENT_BLOCK_SIGNALS */
  sigmaster.tid = 0;

  return 0;
}
#endif
#ifdef SIGEVENT_SCHEDULE_THREAD
/* timer thread to check signals. Shouldnt be needed */
int quagga_signal_timer(struct thread *t)
{
  struct quagga_sigevent_master_t *sigm;
  struct quagga_signal_t *sig;
  int i;

  sigm = THREAD_ARG(t);
  sigm->t = thread_add_timer(sigm->t->master, quagga_signal_timer, &sigmaster,
                             QUAGGA_SIGNAL_TIMER_INTERVAL);
  return quagga_sigevent_process();
}
#endif /* SIGEVENT_SCHEDULE_THREAD */

/* Initialization of signal handles. */
/* Signal wrapper. */
static int
signal_set(int signo)
{
  int ret;
  struct sigaction sig;
  struct sigaction osig;

  sig.sa_handler = &quagga_signal_handler;
  sigfillset(&sig.sa_mask); //把所有信号加入到集合中，信号集中将包含Linux支持的64种信号
  sig.sa_flags = 0;

  if (signo == SIGALRM)
  {
#ifdef SA_INTERRUPT
    sig.sa_flags |= SA_INTERRUPT; /* SunOS */
#endif
  }
  else
  {
#ifdef SA_RESTART
    sig.sa_flags |= SA_RESTART;
#endif /* SA_RESTART */
  }

  ret = sigaction(signo, &sig, &osig);
  if (ret < 0)
    return ret;
  else
    return 0;
}

#ifdef SA_SIGINFO

/* XXX This function should be enhanced to support more platforms
       (it currently works only on Linux/x86). */
static void *
program_counter(void *context)
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

static void __attribute__((noreturn))
exit_handler(int signo
#ifdef SA_SIGINFO
             ,
             siginfo_t *siginfo, void *context
#endif
)
{
  //printf("%s:============+++++++++++++++++++++===========signo=%d\r\n", __func__, signo);
  zlog_signal(signo, "exiting..."
#ifdef SA_SIGINFO
              ,
              siginfo, program_counter(context)
#endif
  );
  _exit(128 + signo);
}

static void __attribute__((noreturn))
core_handler(int signo
#ifdef SA_SIGINFO
             ,
             siginfo_t *siginfo, void *context
#endif
)
{
  //printf("%s:==========+++++++++++++++++++++++++=========signo=%d\r\n", __func__, signo);
  zlog_signal(signo, "aborting..."
#ifdef SA_SIGINFO
              ,
              siginfo, program_counter(context)
#endif
  );
  /* dump memory stats on core */
  log_memstats_stderr("core_handler");
  abort();
}

static void
trap_default_signals(void)
{
  static const struct
  {
    const int *sigs;
    zpl_uint32 nsigs;
    void (*handler)(int signo
#ifdef SA_SIGINFO
                    ,
                    siginfo_t *info, void *context
#endif
    );
  } sigmap[] = {
      {core_signals, array_size(core_signals), core_handler},
      {exit_signals, array_size(exit_signals), exit_handler},
      {ignore_signals, array_size(ignore_signals), NULL},
  };
  zpl_uint32 i;

  for (i = 0; i < array_size(sigmap); i++)
  {
    zpl_uint32 j;

    for (j = 0; j < sigmap[i].nsigs; j++)
    {
      struct sigaction oact;
      if ((sigaction(sigmap[i].sigs[j], NULL, &oact) == 0) &&
          (oact.sa_handler == SIG_DFL))
      {
        struct sigaction act;
        sigfillset(&act.sa_mask);
        if (sigmap[i].handler == NULL)
        {
          act.sa_handler = SIG_IGN;
          act.sa_flags = 0;
        }
        else
        {
#ifdef SA_SIGINFO
          /* Request extra arguments to signal handler. */
          act.sa_sigaction = sigmap[i].handler;
          act.sa_flags = SA_SIGINFO;
#else
          act.sa_handler = sigmap[i].handler;
          act.sa_flags = 0;
#endif
        }
        if (sigaction(sigmap[i].sigs[j], &act, NULL) < 0)
          zlog_warn(MODULE_DEFAULT, "Unable to set signal handler for signal %d: %s",
                    sigmap[i].sigs[j], ipstack_strerror(ipstack_errno));
      }
    }
  }
}

void signal_init(void *m, int sigc,
                 struct quagga_signal_t signals[])
{
  zpl_uint32 i = 0;
  struct quagga_signal_t *sig;

  /* First establish some default handlers that can be overridden by
     the application. */
  trap_default_signals();

  while (i < sigc)
  {
    sig = &signals[i];
    if (sig->signal != SIGKILL)
      if (signal_set(sig->signal) < 0)
      {
        fprintf(stdout, "Unable to set signal handler for signal %d: %s(sdddddd)\r\n",
                sig->signal, ipstack_strerror(ipstack_errno));
        // exit (-1);
      }
    i++;
  }

  sigmaster.sigc = sigc;
  sigmaster.signals = signals;
#ifdef SIGEVENT_SCHEDULE_THREAD
  sigmaster.t =
      thread_add_timer(m, quagga_signal_timer, &sigmaster,
                       QUAGGA_SIGNAL_TIMER_INTERVAL);
#endif /* SIGEVENT_SCHEDULE_THREAD */
}

/*void signal_sigmask()
{
  zpl_uint32 i = 0;
  sigset_t	mask;
  sigfillset(&mask);
  for(i = 0; i < array_size(core_signals); i++)
  {
    sigdelset(&mask, core_signals[i]);
  }
  for(i = 0; i < array_size(exit_handler); i++)
  {
    sigdelset(&mask, exit_handler[i]);
  }
  for(i = 0; i < array_size(ignore_signals); i++)
  {
    sigdelset(&mask, ignore_signals[i]);
  }
}*/
