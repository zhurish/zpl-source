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

#include <zebra.h>
#include <sigevent.h>
#include <log.h>
#include <memory.h>

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


/* master signals descriptor struct */
struct quagga_sigevent_master_t
{
  pthread_t   tid;

  struct quagga_signal_t *signals; 
  int sigc;
  
  volatile sig_atomic_t caught;
} sigmaster;

#ifdef QUAGGA_SIGNAL_REAL_TIMER
static int signal_pipe = 0;

static void real_time_signal_handler (pthread_t pid, int sig)
{
	char buf[128];
	pthread_t *ppid = (pthread_t *)buf;
	int *psig = (int *)(buf + sizeof(pthread_t));
	*ppid = pid;
	*psig = sig;
	if (send (signal_pipe[1], buf, sizeof(pthread_t) + sizeof (sig), MSG_DONTWAIT) < 0)
		zlog_err (ZLOG_DEFAULT, "Could not send signal(%d): %s", sig, strerror (errno));
}

/* Call this before doing anything else. Sets up the socket pair
 * and installs the signal handler */
static void real_signal_setup(void)
{
	int i;
	int flags;

	socketpair (AF_UNIX, SOCK_STREAM, 0, signal_pipe);
	//os_set_nonblocking
	/* Stop any scripts from inheriting us */
/*	for (i = 0; i < 2; i++)
		if ((flags = fcntl (signal_pipe[i], F_GETFD, 0)) < 0 ||
			fcntl (signal_pipe[i], F_SETFD, flags | FD_CLOEXEC) < 0)
			zlog_err (ZLOG_DEFAULT ,"fcntl: %s", strerror (errno));*/
}
/* Read a signal from the signal pipe. Returns 0 if there is
 * no signal, -1 on error (and sets errno appropriately), and
 * your signal on success */
static int real_signal_read (int timeout)
{
	int i = 0, retval = 0;
	char buf[128];
	pthread_t *pid = (pthread_t *)buf;
	int *sig = (int *)(buf + sizeof(pthread_t));
	fd_set rfds;
	struct timeval tv;
	struct quagga_signal_t *sighandle;
	memset(buf, 0, sizeof(buf));
	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	FD_ZERO (&rfds);
	FD_SET (signal_pipe[0], &rfds);
	while (1)
	{
		retval = select(signal_pipe[0] + 1, &rfds, NULL, NULL, &tv);
		if (retval == 0)
			return 0;
		else if (retval < 0)
		{
			if (errno == EINTR)
				continue;
		}
		if (!FD_ISSET(signal_pipe[0], &rfds))
			return 0;

		if (read(signal_pipe[0], buf, sizeof(buf)) < 0)
			return -1;

		for (i = 0; i < sigmaster.sigc; i++)
		{
			sighandle = &(sigmaster.signals[i]);

			if (sighandle->signal && sighandle->signal == *sig)
			{
				if (sighandle->handler)
					sighandle->handler();
				if (sighandle->signal_handler)
					sighandle->signal_handler(sighandle->signal);
			}
		}
	}
}
#endif
/* Generic signal handler 
 * Schedules signal event thread
 */
static void
quagga_signal_handler (int signo)
{
#ifdef QUAGGA_SIGNAL_REAL_TIMER
	real_time_signal_handler (pthread_self(), signo);
#else
  int i;
  struct quagga_signal_t *sig;
  sigmaster.tid = pthread_self();
  for (i = 0; i < sigmaster.sigc; i++)
    {
      sig = &(sigmaster.signals[i]);
      
      if (sig->signal == signo)
        sig->caught = 1;
    }
  sigmaster.caught = 1;
#endif
} 

/* check if signals have been caught and run appropriate handlers */
int
quagga_sigevent_process (void)
{
#ifdef QUAGGA_SIGNAL_REAL_TIMER
#else
  struct quagga_signal_t *sig;
  int i;
#ifdef SIGEVENT_BLOCK_SIGNALS
  /* shouldnt need to block signals, but potentially may be needed */
  sigset_t newmask, oldmask;

  /*
   * Block most signals, but be careful not to defer SIGTRAP because
   * doing so breaks gdb, at least on NetBSD 2.0.  Avoid asking to
   * block SIGKILL, just because we shouldn't be able to do so.
   */
  sigfillset (&newmask);
  sigdelset (&newmask, SIGTRAP);
  sigdelset (&newmask, SIGKILL);
   
  if ( (sigprocmask (SIG_BLOCK, &newmask, &oldmask)) < 0)
    {
      zlog_err (ZLOG_DEFAULT, "quagga_signal_timer: couldnt block signals!");
      return -1;
    }
#endif /* SIGEVENT_BLOCK_SIGNALS */

  if(sigmaster.tid != pthread_self())
	  return 0;

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
              if(sig->handler)
            	  sig->handler ();
              if(sig->signal_handler)
            	  sig->signal_handler (sig->signal);
            }
        }
    }

#ifdef SIGEVENT_BLOCK_SIGNALS
  if ( sigprocmask (SIG_UNBLOCK, &oldmask, NULL) < 0 );
    return -1;
#endif /* SIGEVENT_BLOCK_SIGNALS */
  sigmaster.tid = 0;
#endif
  return 0;
}
#ifdef QUAGGA_SIGNAL_REAL_TIMER
int real_sigevent_process (int timeout)
{
	return real_signal_read (timeout);
}
#endif
/* Initialization of signal handles. */
/* Signal wrapper. */
static int
signal_set (int signo)
{
  int ret;
  struct sigaction sig;
  struct sigaction osig;

  sig.sa_handler = &quagga_signal_handler;
  sigfillset (&sig.sa_mask);
  sig.sa_flags = 0;
  if (signo == SIGALRM) {
#ifdef SA_INTERRUPT
      sig.sa_flags |= SA_INTERRUPT; /* SunOS */
#endif
  } else {
#ifdef SA_RESTART
      sig.sa_flags |= SA_RESTART;
#endif /* SA_RESTART */
  }

  ret = sigaction (signo, &sig, &osig);
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
# if defined(REG_EIP)
#  define REG_INDEX REG_EIP
# elif defined(REG_RIP)
#  define REG_INDEX REG_RIP
# elif defined(__powerpc__)
#  define REG_INDEX 32
# endif
#elif defined(SUNOS_5) /* !GNU_LINUX */
# define REG_INDEX REG_PC
#endif /* GNU_LINUX */

#ifdef REG_INDEX
# ifdef HAVE_UCONTEXT_T_UC_MCONTEXT_GREGS
#  define REGS gregs[REG_INDEX]
# elif defined(HAVE_UCONTEXT_T_UC_MCONTEXT_UC_REGS)
#  define REGS uc_regs->gregs[REG_INDEX]
# endif /* HAVE_UCONTEXT_T_UC_MCONTEXT_GREGS */
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

static void __attribute__ ((noreturn))
exit_handler(int signo
#ifdef SA_SIGINFO
	     , siginfo_t *siginfo, void *context
#endif
	    )
{
  zlog_signal(signo, "exiting..."
#ifdef SA_SIGINFO
	      , siginfo, program_counter(context)
#endif
	     );
  _exit(128+signo);
}

static void __attribute__ ((noreturn))
core_handler(int signo
#ifdef SA_SIGINFO
	     , siginfo_t *siginfo, void *context
#endif
	    )
{
  zlog_signal(signo, "aborting..."
#ifdef SA_SIGINFO
	      , siginfo, program_counter(context)
#endif
	     );
  /* dump memory stats on core */
  log_memstats_stderr ("core_handler");
  abort();
}

static void
trap_default_signals(void)
{
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
    SIGUSR2,
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
  static const struct {
    const int *sigs;
    u_int nsigs;
    void (*handler)(int signo
#ifdef SA_SIGINFO
		    , siginfo_t *info, void *context
#endif
		   );
  } sigmap[] = {
    { core_signals, array_size(core_signals), core_handler},
    { exit_signals, array_size(exit_signals), exit_handler},
    { ignore_signals, array_size(ignore_signals), NULL},
  };
  u_int i;

  for (i = 0; i < array_size(sigmap); i++)
    {
      u_int j;

      for (j = 0; j < sigmap[i].nsigs; j++)
        {
	  struct sigaction oact;
	  if ((sigaction(sigmap[i].sigs[j],NULL,&oact) == 0) &&
	      (oact.sa_handler == SIG_DFL))
	    {
	      struct sigaction act;
	      sigfillset (&act.sa_mask);
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
	      if (sigaction(sigmap[i].sigs[j],&act,NULL) < 0)
	        zlog_warn(ZLOG_DEFAULT, "Unable to set signal handler for signal %d: %s",
			  sigmap[i].sigs[j],safe_strerror(errno));

	    }
        }
    }
}

void 
signal_init (int sigc,
             struct quagga_signal_t signals[])
{
#ifdef QUAGGA_SIGNAL_REAL_TIMER
  real_signal_setup();
#endif
  int i = 0;
  struct quagga_signal_t *sig;

  /* First establish some default handlers that can be overridden by
     the application. */
  trap_default_signals();
  
  while (i < sigc)
    {
      sig = &signals[i];
      if ( signal_set (sig->signal) < 0 )
        exit (-1);
      i++;
    }

  sigmaster.sigc = sigc;
  sigmaster.signals = signals;
}
