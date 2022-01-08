/*
 * os_util.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "os_include.h"
#include "zpl_include.h"

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
	void (*os_core_abort)(zpl_int signo, const char *action
#ifdef SA_SIGINFO
	     , siginfo_t *siginfo, void *context
#endif
	);
	void (*os_core_exit)(zpl_int signo, const char *action
#ifdef SA_SIGINFO
	     , siginfo_t *siginfo, void *context
#endif
	);
	struct os_signal_t os_signal_tbl[__SIGRTMAX];
};

struct os_signal_process_t os_sigproc_tbl;
//static int signal_rfd = 0, signal_wfd = 0;

//static struct os_signal_t os_sigproc_tbl.os_signal_tbl[__SIGRTMAX];
static int os_signal_handler_action(zpl_int sig, void *info);

static void os_signal_default_interrupt(zpl_int signo
#ifdef SA_SIGINFO
	     , siginfo_t *siginfo, void *context
#endif
);

static void os_exit_handler(int signo
#ifdef SA_SIGINFO
             ,
             siginfo_t *siginfo, void *context
#endif
);

static void os_core_handler(int signo
#ifdef SA_SIGINFO
             ,
             siginfo_t *siginfo, void *context
#endif
);

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
	for(i = 0; i < num; i++)
	{
		os_signal_add(tbl[i].signal, tbl[i].signal_handler);
	}
}

static int os_signal_handler_action(zpl_int sig, void *info)
{
	int i = 0;
	for(i = 0; i < __SIGRTMAX; i++)
	{
		if(os_sigproc_tbl.os_signal_tbl[i].signal == sig &&
		 os_sigproc_tbl.os_signal_tbl[i].signal_handler)
		 return (os_sigproc_tbl.os_signal_tbl[sig].signal_handler)(sig, os_sigproc_tbl.os_signal_tbl[i].info?os_sigproc_tbl.os_signal_tbl[i].info:info);
	}
	return OK;
}

static void os_signal_default_interrupt(zpl_int signo
#ifdef SA_SIGINFO
	     , siginfo_t *siginfo, void *context
#endif
)
{
	zpl_int wsigno = signo;
	if(os_sigproc_tbl.signal_wfd)
		write(os_sigproc_tbl.signal_wfd, &wsigno, 4);
	return;
}

int os_signal_process(zpl_uint timeout)
{
	//extern int os_select_wait(int maxfd, fd_set *rfdset, fd_set *wfdset, zpl_uint32 timeout_ms);
	zpl_int rsigno = 0;
	if(os_read_timeout(os_sigproc_tbl.signal_rfd, &rsigno, 4, timeout) == 4)
	{
		os_signal_handler_action(rsigno, NULL);
	}
	return OK;
}


static void os_trap_default_signals(void)
{
  static const struct
  {
    const int *sigs;
    zpl_uint32 nsigs;
    void (*handler)(int signo
#ifdef SA_SIGINFO
		, siginfo_t *info, void *context
#endif
    );
  } sigmap[] = {
      {core_signals, array_size(core_signals), os_core_handler},
      {exit_signals, array_size(exit_signals), os_exit_handler},
      {ignore_signals, array_size(ignore_signals), NULL},
  };
  zpl_uint32 i = 0;

  for (i = 0; i < array_size(sigmap); i++)
  {
    zpl_uint32 j = 0;

    for (j = 0; j < sigmap[i].nsigs; j++)
    {
		os_register_signal(sigmap[i].sigs[j], sigmap[i].handler);
    }
  }
}

void os_signal_default(void *abort_func, void *exit_func)
{
	memset(os_sigproc_tbl.os_signal_tbl, 0, sizeof(os_sigproc_tbl.os_signal_tbl));
	os_sigproc_tbl.os_core_abort = abort_func;
	os_sigproc_tbl.os_core_exit = exit_func;
	os_unix_sockpair_create(zpl_false, &os_sigproc_tbl.signal_rfd, &os_sigproc_tbl.signal_wfd);
	if(os_sigproc_tbl.signal_wfd)
		os_set_nonblocking(os_sigproc_tbl.signal_wfd);
	if(os_sigproc_tbl.signal_rfd)	
		os_set_nonblocking(os_sigproc_tbl.signal_rfd);

	os_trap_default_signals();
}



int os_register_signal(zpl_int sig, void (*handler)(zpl_int
#ifdef SA_SIGINFO
	     , siginfo_t *siginfo, void *context
#endif
		))
{
	struct sigaction sa;//, osa;

	memset(&sa, 0, sizeof(sa));
	//sa.sa_handler = handler;
	//sigaction(sig, NULL, &osa);
    if (handler == NULL)
    {
        sa.sa_handler = SIG_DFL;//SIG_IGN;
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
	if (sigfillset(&sa.sa_mask) != 0 ||
	    sigaction(sig, &sa, NULL) < 0)
	{
		fprintf(stdout, "Unable to set up signal handler for %d", sig);
		return (-1);
	}
	return (0);
}


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

static void __attribute__((noreturn))
os_exit_handler(int signo
#ifdef SA_SIGINFO
             ,
             siginfo_t *siginfo, void *context
#endif
)
{
  os_log(OS_SIGNAL_FILE,"%s:============+++++++++++++++++++++===========signo=%d\r\n", __func__, signo);
  if(os_sigproc_tbl.os_core_exit)
  	(os_sigproc_tbl.os_core_exit)( signo, "exiting..."
#ifdef SA_SIGINFO
              ,
              siginfo, os_signal_program_counter(context)
#endif
  );
  _exit(128 + signo);
}

static void __attribute__((noreturn))
os_core_handler(int signo
#ifdef SA_SIGINFO
             ,
             siginfo_t *siginfo, void *context
#endif
)
{
  os_log(OS_SIGNAL_FILE,"%s:==========+++++++++++++++++++++++++=========signo=%d\r\n", __func__, signo);
  if(os_sigproc_tbl.os_core_abort)
  	(os_sigproc_tbl.os_core_abort)( signo, "aborting..."
#ifdef SA_SIGINFO
              ,
              siginfo, os_signal_program_counter(context)
#endif
  );
  abort();
}




