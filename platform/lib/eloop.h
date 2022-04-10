/* Thread management routine header.
 * Copyright (C) 1998 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef _ELOOP_H
#define _ELOOP_H

#ifdef __cplusplus
extern "C" {
#endif


#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"

#ifndef ZPL_IPCOM_MODULE
#include "thread.h"
#define eloop_list thread_list
#define eloop_master thread_master
#define eloop thread
#define ELOOP_ARG THREAD_ARG
#define ELOOP_FD  THREAD_FD
#define ELOOP_VAL THREAD_VAL

#define ELOOP_READ_ON THREAD_READ_ON
#define ELOOP_WRITE_ON THREAD_WRITE_ON
#define ELOOP_TIMER_ON THREAD_TIMER_ON
#define ELOOP_TIMER_MSEC_ON THREAD_TIMER_MSEC_ON
#define ELOOP_OFF THREAD_OFF
#define ELOOP_READ_OFF(eloop)  THREAD_OFF(eloop)
#define ELOOP_WRITE_OFF(eloop)  THREAD_OFF(eloop)
#define ELOOP_TIMER_OFF(eloop)  THREAD_OFF(eloop)

#define eloop_add_read  thread_add_read
#define eloop_add_write thread_add_write
#define eloop_add_timer thread_add_timer
#define eloop_add_timer_msec  thread_add_timer_msec
#define eloop_add_timer_tv  thread_add_timer_tv
#define eloop_add_event thread_add_event
#define eloop_execute thread_execute
#define eloop_add_ready thread_add_ready
#define eloop_add_background  thread_add_background

#define eloop_master_create  thread_master_create
#define eloop_master_module_create  thread_master_module_create
#define eloop_master_module_lookup  thread_master_module_lookup
#define eloop_master_free  thread_master_free
#define eloop_cancel  thread_cancel

#define eloop_cancel_event  thread_cancel_event
#define eloop_fetch  thread_fetch
#define eloop_mainloop  thread_mainloop
#define eloop_call  thread_call
#define eloop_timer_remain_second  thread_timer_remain_second

#define eloop_timer_remain  thread_timer_remain
#define eloop_should_yield  thread_should_yield
#define eloop_fetch_quit  thread_fetch_quit
#define eloop_wait_quit  thread_wait_quit
#define eloop_getrusage  thread_getrusage

#define eloop_consumed_time  thread_consumed_time

#else
#define ELOOP_MASTER_LIST


/* Linked list of eloop. */
struct eloop_list
{
  struct eloop *head;
  struct eloop *tail;
  zpl_uint32 count;
};

struct pqueue;

/*
 * Abstract it so we can use different methodologies to
 * select on data.
 */
typedef ipstack_fd_set eloop_fd_set;

#define OS_ELOOP_CPU_MAX	128
struct eloop_cpu
{
	zpl_uint32 key;
	void *data;
};


/* Master of the theads. */
struct eloop_master
{
  #ifdef ELOOP_MASTER_LIST
  struct eloop_master *next;		/* next pointer of the thread */   
  struct eloop_master *prev;		/* previous pointer of the thread */
  #endif
  struct eloop_list read;
  struct eloop_list write;
  struct eloop_list timer;
  struct eloop_list event;
  struct eloop_list ready;
  struct eloop_list unuse;
  struct eloop_list background;
  int fd_limit;
  eloop_fd_set readfd;
  eloop_fd_set writefd;
  eloop_fd_set exceptfd;
  zpl_ulong alloc;

  zpl_uint32 module;

  int max_fd;
  struct eloop_cpu cpu_record[OS_ELOOP_CPU_MAX];
  struct timeval relative_time;
  struct eloop *eloop_current;
  void *mutex;
  zpl_bool bquit;
};

typedef zpl_uchar eloop_type;

/* Thread itself. */
struct eloop
{
  eloop_type type;		/* eloop type */
  eloop_type add_type;		/* eloop type */
  struct eloop *next;		/* next pointer of the eloop */
  struct eloop *prev;		/* previous pointer of the eloop */
  struct eloop_master *master;	/* pointer to the struct eloop_master. */
  int (*func) (struct eloop *); /* event function */
  void *arg;			/* event argument */
  union {
    zpl_uint32 val;			/* second argument of the event. */
    zpl_socket_t fd;			/* file descriptor in case of read/write. */
    struct timeval sands;	/* rest of time sands value. */
  } u;
  zpl_uint32 index;			/* used for timers to store position in queue */
  struct timeval real;
  struct cpu_eloop_history *hist; /* cache pointer to cpu_history */
  const char *funcname;
  const char *schedfrom;
  zpl_uint32 schedfrom_line;
};

struct cpu_eloop_history
{
  int (*func)(struct eloop *);
  zpl_uint32  total_calls;
  struct os_time_stats real;
  eloop_type types;
  const char *funcname;
};

/* Thread types. */
#define ELOOP_READ           0
#define ELOOP_WRITE          1
#define ELOOP_TIMER          2
#define ELOOP_EVENT          3
#define ELOOP_READY          4
#define ELOOP_BACKGROUND     5
#define ELOOP_UNUSED         6
#define ELOOP_EXECUTE        7

/* Thread yield time.  */
#define ELOOP_YIELD_TIME_SLOT     10 * 1000L /* 10ms */

/* Macros. */
#define ELOOP_ARG(X) ((X)->arg)
#define ELOOP_FD(X)  ((X)->u.fd)
#define ELOOP_VAL(X) ((X)->u.val)

#define ELOOP_READ_ON(master,eloop,func,arg,sock) \
  do { \
    if (! eloop) \
      eloop = eloop_add_read (master, func, arg, sock); \
  } while (0)

#define ELOOP_WRITE_ON(master,eloop,func,arg,sock) \
  do { \
    if (! eloop) \
      eloop = eloop_add_write (master, func, arg, sock); \
  } while (0)

#define ELOOP_TIMER_ON(master,eloop,func,arg,time) \
  do { \
    if (! eloop) \
      eloop = eloop_add_timer (master, func, arg, time); \
  } while (0)

#define ELOOP_TIMER_MSEC_ON(master,eloop,func,arg,time) \
  do { \
    if (! eloop) \
      eloop = eloop_add_timer_msec (master, func, arg, time); \
  } while (0)

#define ELOOP_OFF(eloop) \
  do { \
    if (eloop) \
      { \
        eloop_cancel (eloop); \
        eloop = NULL; \
      } \
  } while (0)

#define ELOOP_READ_OFF(eloop)  ELOOP_OFF(eloop)
#define ELOOP_WRITE_OFF(eloop)  ELOOP_OFF(eloop)
#define ELOOP_TIMER_OFF(eloop)  ELOOP_OFF(eloop)

#define debugargdef  const char *funcname, const char *schedfrom, zpl_uint32 fromln

#define eloop_add_read(m,f,a,v) funcname_eloop_add_read(m,f,a,v,#f,__FILE__,__LINE__)
#define eloop_add_write(m,f,a,v) funcname_eloop_add_write(m,f,a,v,#f,__FILE__,__LINE__)
#define eloop_add_timer(m,f,a,v) funcname_eloop_add_timer(m,f,a,v,#f,__FILE__,__LINE__)
#define eloop_add_timer_msec(m,f,a,v) funcname_eloop_add_timer_msec(m,f,a,v,#f,__FILE__,__LINE__)
#define eloop_add_timer_tv(m,f,a,v) funcname_eloop_add_timer_tv(m,f,a,v,#f,__FILE__,__LINE__)
#define eloop_add_event(m,f,a,v) funcname_eloop_add_event(m,f,a,v,#f,__FILE__,__LINE__)
#define eloop_execute(m,f,a,v) funcname_eloop_execute(m,f,a,v,#f,__FILE__,__LINE__)
#define eloop_add_ready(m,f,a,v) funcname_eloop_ready(m,f,a,v,#f,__FILE__,__LINE__)

/* The 4th arg to eloop_add_background is the # of milliseconds to delay. */
#define eloop_add_background(m,f,a,v) funcname_eloop_add_background(m,f,a,v,#f,__FILE__,__LINE__)

/* Prototypes. */
extern struct eloop_master *eloop_master_create (void);

extern struct eloop_master *eloop_master_module_create (zpl_uint32 );
extern struct eloop_master *eloop_master_module_lookup (zpl_uint32 module);

extern void eloop_master_free (struct eloop_master *);

extern struct eloop *funcname_eloop_add_read (struct eloop_master *,
				                int (*)(struct eloop *),
				                void *, zpl_socket_t, debugargdef);
extern struct eloop *funcname_eloop_add_write (struct eloop_master *,
				                 int (*)(struct eloop *),
				                 void *, zpl_socket_t, debugargdef);
extern struct eloop *funcname_eloop_add_timer (struct eloop_master *,
				                 int (*)(struct eloop *),
				                 void *, long, debugargdef);
extern struct eloop *funcname_eloop_add_timer_msec (struct eloop_master *,
				                      int (*)(struct eloop *),
				                      void *, long, debugargdef);
extern struct eloop *funcname_eloop_add_timer_tv (struct eloop_master *,
				                    int (*)(struct eloop *),
				                    void *, struct timeval *,
						    debugargdef);
extern struct eloop *funcname_eloop_add_event (struct eloop_master *,
				                 int (*)(struct eloop *),
				                 void *, int, debugargdef);
extern struct eloop *funcname_eloop_add_background (struct eloop_master *,
                                               int (*func)(struct eloop *),
				               void *arg,
				               long milliseconds_to_delay,
					       debugargdef);
extern struct eloop *funcname_eloop_execute (struct eloop_master *,
                                               int (*)(struct eloop *),
                                               void *, int, debugargdef);
extern struct eloop *funcname_eloop_ready (struct eloop_master *,
                                               int (*)(struct eloop *),
                                               void *, int, debugargdef);
#undef debugargdef

extern void eloop_cancel (struct eloop *);
extern zpl_uint32  eloop_cancel_event (struct eloop_master *, void *);
extern struct eloop *eloop_fetch (struct eloop_master *);
extern struct eloop *eloop_mainloop (struct eloop_master *);
extern void eloop_call (struct eloop *);
extern zpl_ulong eloop_timer_remain_second (struct eloop *);
extern struct timeval eloop_timer_remain(struct eloop*);
extern int eloop_should_yield (struct eloop *);
extern int eloop_fetch_quit (struct eloop_master *);
extern int eloop_wait_quit (struct eloop_master *);
/* Internal libzebra exports */
extern void eloop_getrusage (struct timeval *);

/* Returns elapsed real (wall clock) time. */
extern zpl_ulong eloop_consumed_time(struct timeval *after, struct timeval *before,
					  zpl_ulong *cpu_time_elapsed);
#ifndef ELOOP_MASTER_LIST
extern struct eloop_master * master_eloop[];
#endif

#ifdef ZPL_SHELL_MODULE
extern int cmd_os_eloop_init(void);
#endif
#endif 
#ifdef __cplusplus
}
#endif

#endif /* _ELOOP_H */
