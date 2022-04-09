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

#ifndef _ZEBRA_THREAD_H
#define _ZEBRA_THREAD_H


#ifdef __cplusplus
extern "C" {
#endif

#include "auto_include.h"
#include "zplos_include.h"

#define THREAD_MASTER_LIST

/* Linked list of thread. */
struct thread_list
{
  struct thread *head;
  struct thread *tail;
  zpl_uint32 count;
};

struct pqueue;

/*
 * Abstract it so we can use different methodologies to
 * select on data.
 */
typedef fd_set thread_fd_set;


#define OS_THREAD_CPU_MAX	128
struct thread_cpu
{
	zpl_uint32 key;
	void *data;
};

enum thread_mode
{
	OS_MODE = 1,
	IP_MODE = 2,
};

/* Master of the theads. */
struct thread_master
{
  #ifdef THREAD_MASTER_LIST
  struct thread_master *next;		/* next pointer of the thread */   
  struct thread_master *prev;		/* previous pointer of the thread */
  #endif
  enum thread_mode workmode;

  struct thread_list read;
  struct thread_list write;
  struct pqueue *timer;
  struct thread_list event;
  struct thread_list ready;
  struct thread_list unuse;
  struct pqueue *background;
  int fd_limit;

  thread_fd_set readfd;
  thread_fd_set writefd;
  thread_fd_set exceptfd;

  zpl_ulong alloc;

  zpl_uint32 module;

  int max_fd;
  struct thread_cpu cpu_record[OS_THREAD_CPU_MAX];
  struct timeval relative_time;
  struct thread *thread_current;
  void *mutex;
  zpl_bool	bquit;
  zpl_uint32 debug;
};

typedef zpl_uchar thread_type;

/* Thread itself. */
struct thread
{
  thread_type type;		/* thread type */
  thread_type add_type;		/* thread type */
  struct thread *next;		/* next pointer of the thread */   
  struct thread *prev;		/* previous pointer of the thread */
  struct thread_master *master;	/* pointer to the struct thread_master. */
  int (*func) (struct thread *); /* event function */
  void *arg;			/* event argument */
  union {
    zpl_uint32 val;			/* second argument of the event. */
    zpl_socket_t fd;			/* file descriptor in case of read/write. */
    struct timeval sands;	/* rest of time sands value. */
  } u;
  zpl_uint32 index;			/* used for timers to store position in queue */
  struct timeval real;
  struct cpu_thread_history *hist; /* cache pointer to cpu_history */
  const char *funcname;
  const char *schedfrom;
  zpl_uint32 schedfrom_line;
};

struct cpu_thread_history 
{
  int (*func)(struct thread *);
  zpl_uint32  total_calls;
  struct os_time_stats real;
  thread_type types;
  const char *funcname;
};

/* Thread types. */
#define THREAD_READ           0
#define THREAD_WRITE          1
#define THREAD_TIMER          2
#define THREAD_EVENT          3
#define THREAD_READY          4
#define THREAD_BACKGROUND     5
#define THREAD_UNUSED         6
#define THREAD_EXECUTE        7

/* Thread yield time.  */
#define THREAD_YIELD_TIME_SLOT     10 * 1000L /* 10ms */

/* Macros. */
#define THREAD_ARG(X) ((X)->arg)
#define THREAD_FD(X)  ((X)->u.fd)
#define THREAD_VAL(X) ((X)->u.val)

#define THREAD_READ_ON(master,thread,func,arg,sock) \
  do { \
    if (! thread) \
      thread = thread_add_read (master, func, arg, sock); \
  } while (0)

#define THREAD_WRITE_ON(master,thread,func,arg,sock) \
  do { \
    if (! thread) \
      thread = thread_add_write (master, func, arg, sock); \
  } while (0)

#define THREAD_TIMER_ON(master,thread,func,arg,time) \
  do { \
    if (! thread) \
      thread = thread_add_timer (master, func, arg, time); \
  } while (0)

#define THREAD_TIMER_MSEC_ON(master,thread,func,arg,time) \
  do { \
    if (! thread) \
      thread = thread_add_timer_msec (master, func, arg, time); \
  } while (0)

#define THREAD_OFF(thread) \
  do { \
    if (thread) \
      { \
        thread_cancel (thread); \
        thread = NULL; \
      } \
  } while (0)

#define THREAD_READ_OFF(thread)  THREAD_OFF(thread)
#define THREAD_WRITE_OFF(thread)  THREAD_OFF(thread)
#define THREAD_TIMER_OFF(thread)  THREAD_OFF(thread)

#define debugargdef  const char *funcname, const char *schedfrom, zpl_uint32 fromln

#define thread_add_read(m,f,a,v) funcname_thread_add_read(m,f,a,v,#f,__FILE__,__LINE__)
#define thread_add_write(m,f,a,v) funcname_thread_add_write(m,f,a,v,#f,__FILE__,__LINE__)
#define thread_add_timer(m,f,a,v) funcname_thread_add_timer(m,f,a,v,#f,__FILE__,__LINE__)
#define thread_add_timer_msec(m,f,a,v) funcname_thread_add_timer_msec(m,f,a,v,#f,__FILE__,__LINE__)
#define thread_add_timer_tv(m,f,a,v) funcname_thread_add_timer_tv(m,f,a,v,#f,__FILE__,__LINE__)
#define thread_add_event(m,f,a,v) funcname_thread_add_event(m,f,a,v,#f,__FILE__,__LINE__)
#define thread_execute(m,f,a,v) funcname_thread_execute(m,f,a,v,#f,__FILE__,__LINE__)
#define thread_add_ready(m,f,a,v) funcname_thread_ready(m,f,a,v,#f,__FILE__,__LINE__)

/* The 4th arg to thread_add_background is the # of milliseconds to delay. */
#define thread_add_background(m,f,a,v) funcname_thread_add_background(m,f,a,v,#f,__FILE__,__LINE__)

/* Prototypes. */
extern struct thread_master *thread_master_create (void);

extern struct thread_master *thread_master_module_create (zpl_uint32 );
extern struct thread_master *thread_master_module_lookup (zpl_uint32 module);
extern void thread_master_free (struct thread_master *);

extern struct thread *funcname_thread_add_read (struct thread_master *, 
				                int (*)(struct thread *),
				                void *, zpl_socket_t, debugargdef);
extern struct thread *funcname_thread_add_write (struct thread_master *,
				                 int (*)(struct thread *),
				                 void *, zpl_socket_t, debugargdef);
extern struct thread *funcname_thread_add_timer (struct thread_master *,
				                 int (*)(struct thread *),
				                 void *, long, debugargdef);
extern struct thread *funcname_thread_add_timer_msec (struct thread_master *,
				                      int (*)(struct thread *),
				                      void *, long, debugargdef);
extern struct thread *funcname_thread_add_timer_tv (struct thread_master *,
				                    int (*)(struct thread *),
				                    void *, struct timeval *,
						    debugargdef);
extern struct thread *funcname_thread_add_event (struct thread_master *,
				                 int (*)(struct thread *),
				                 void *, int, debugargdef);
extern struct thread *funcname_thread_add_background (struct thread_master *,
                                               int (*func)(struct thread *),
				               void *arg,
				               long milliseconds_to_delay,
					       debugargdef);
extern struct thread *funcname_thread_execute (struct thread_master *,
                                               int (*)(struct thread *),
                                               void *, int, debugargdef);
extern struct thread *funcname_thread_ready (struct thread_master *,
                                               int (*)(struct thread *),
                                               void *, int, debugargdef);
#undef debugargdef

extern void thread_cancel (struct thread *);
extern zpl_uint32  thread_cancel_event (struct thread_master *, void *);
extern struct thread *thread_fetch (struct thread_master *);
extern struct thread *thread_mainloop(struct thread_master *);
extern void thread_call (struct thread *);
extern zpl_ulong thread_timer_remain_second (struct thread *);
extern struct timeval thread_timer_remain(struct thread*);
extern int thread_should_yield (struct thread *);
extern int thread_fetch_quit (struct thread_master *);
extern int thread_wait_quit (struct thread_master *);

extern void thread_getrusage (struct timeval *real);
/* Returns elapsed real (wall clock) time. */
extern zpl_ulong thread_consumed_time(struct timeval *after, struct timeval *before,
					  zpl_ulong *cpu_time_elapsed);
#ifndef THREAD_MASTER_LIST
extern struct thread_master * master_thread[];
#endif
//extern struct timeval recent_relative_time (void);

//extern int cpu_thread_show(struct thread_master *m, struct vty *vty);
#ifdef ZPL_SHELL_MODULE
extern int cmd_os_thread_init(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _ZEBRA_THREAD_H */
