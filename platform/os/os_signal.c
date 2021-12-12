/*
 * os_util.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "os_include.h"
#include "zpl_include.h"

struct os_signal_t
{
  int signal;                     /* signal number    */
  os_signal_handler signal_handler;
  void	*info;
};

static int signal_rfd = 0, signal_wfd = 0;

static struct os_signal_t os_signal_tbl[__SIGRTMAX];

static void os_signal_default_interrupt(zpl_int signo
#ifdef SA_SIGINFO
	     , siginfo_t *siginfo, void *context
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
	os_signal_tbl[sig].signal = sig;
	os_signal_tbl[sig].signal_handler = hander;
	os_register_signal(sig, os_signal_default_interrupt);
	return OK;
}

int os_signal_handler_action(zpl_int sig, void *info)
{
	int i = 0;
	for(i = 0; i < __SIGRTMAX; i++)
	{
		if(os_signal_tbl[i].signal == sig &&
		 os_signal_tbl[i].signal_handler)
		 return (os_signal_tbl[sig].signal_handler)(sig, os_signal_tbl[i].info?os_signal_tbl[i].info:info);
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
	if(signal_wfd)
		write(signal_wfd, &wsigno, 4);
	return;
}

int os_signal_process(void)
{
	//extern int os_select_wait(int maxfd, fd_set *rfdset, fd_set *wfdset, zpl_uint32 timeout_ms);
	zpl_int rsigno = 0;
	if(os_read_timeout(signal_rfd, &rsigno, 4, 1000) == 4)
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
      {core_signals, array_size(core_signals), NULL/*core_handler*/},
      {exit_signals, array_size(exit_signals), NULL/*exit_handler*/},
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

void os_signal_default()
{
	memset(os_signal_tbl, 0, sizeof(os_signal_tbl));
	os_unix_sockpair_create(zpl_false, &signal_rfd, &signal_wfd);
	if(signal_wfd)
		os_set_nonblocking(signal_wfd);
	if(signal_rfd)	
		os_set_nonblocking(signal_rfd);

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



/*
int os_register_signal(int sig)
{

}
*/


