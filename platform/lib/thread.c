/* Thread management routine
 * Copyright (C) 1998, 2000 Kunihiro Ishiguro <kunihiro@zebra.org>
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


#include <zebra.h>
#include "thread.h"
#include "command.h"
#include "hash.h"
#include "log.h"
#include "memory.h"
#include "pqueue.h"
#include "sigevent.h"

#include "os_list.h"
#include "os_time.h"
#include "os_sem.h"


#if defined(__APPLE__)
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif


static int os_mt_init = 0;
struct thread_master * master_thread[MODULE_MAX];

#if 0
/* Struct timeval's tv_usec one second value.  */
#define TIMER_SECOND_MICRO 1000000L

/* Adjust so that tv_usec is in the range [0,TIMER_SECOND_MICRO).
 And change negative values to 0. */
static struct timeval
timeval_adjust (struct timeval a)
{
	while (a.tv_usec >= TIMER_SECOND_MICRO)
	{
		a.tv_usec -= TIMER_SECOND_MICRO;
		a.tv_sec++;
	}

	while (a.tv_usec < 0)
	{
		a.tv_usec += TIMER_SECOND_MICRO;
		a.tv_sec--;
	}

	if (a.tv_sec < 0)
	/* Change negative timeouts to 0. */
	a.tv_sec = a.tv_usec = 0;

	return a;
}

static struct timeval
timeval_subtract (struct timeval a, struct timeval b)
{
	struct timeval ret;

	ret.tv_usec = a.tv_usec - b.tv_usec;
	ret.tv_sec = a.tv_sec - b.tv_sec;

	return timeval_adjust (ret);
}

static long
timeval_cmp (struct timeval a, struct timeval b)
{
	return (a.tv_sec == b.tv_sec
			? a.tv_usec - b.tv_usec : a.tv_sec - b.tv_sec);
}

static unsigned long
timeval_elapsed (struct timeval a, struct timeval b)
{
	return (((a.tv_sec - b.tv_sec) * TIMER_SECOND_MICRO)
			+ (a.tv_usec - b.tv_usec));
}
#endif
/* Public export of recent_relative_time by value */
/*struct timeval
 recent_relative_time (void)
 {
 int i = 0;
 static struct timeval relative_time;
 for(i = 0; i < MODULE_MAX; i++ )
 {
 if(master_thread[i] && master_thread[i]->ptid == os_task_pthread_self() )
 return master_thread[i]->relative_time;
 }
 os_gettime (OS_CLK_REALTIME, &relative_time);
 return relative_time;
 }*/

static int thread_timer_cmp(void *a, void *b)
{
	struct thread *thread_a = a;
	struct thread *thread_b = b;

	long cmp = os_timeval_cmp(thread_a->u.sands, thread_b->u.sands);

	if (cmp < 0)
		return -1;
	if (cmp > 0)
		return 1;
	return 0;
}

static void thread_timer_update(void *node, int actual_position)
{
	struct thread *thread = node;

	thread->index = actual_position;
}

/* Allocate new thread master.  */
struct thread_master *
thread_master_create()
{
	struct thread_master *rv;
	//struct rlimit limit;
	if (os_mt_init == 0)
	{
		int i = 0;
		for (i = 0; i < MODULE_MAX; i++)
		{
			master_thread[i] = NULL;
		}
		os_mt_init = 1;
	}
	//getrlimit(RLIMIT_NOFILE, &limit);

	rv = XCALLOC(MTYPE_THREAD_MASTER, sizeof(struct thread_master));
	if (rv == NULL)
	{
		return NULL;
	}
	rv->fd_limit = 0;//(int) limit.rlim_cur;
/*	rv->read = XCALLOC(MTYPE_THREAD, sizeof(struct thread *) * rv->fd_limit);
	if (rv->read == NULL)
	{
		XFREE(MTYPE_THREAD_MASTER, rv);
		return NULL;
	}

	rv->write = XCALLOC(MTYPE_THREAD, sizeof(struct thread *) * rv->fd_limit);
	if (rv->write == NULL)
	{
		XFREE(MTYPE_THREAD, rv->read);
		XFREE(MTYPE_THREAD_MASTER, rv);
		return NULL;
	}*/

	//rv->ptid = os_task_pthread_self ();

	/* Initialize the timer queues */
	rv->timer = pqueue_create();
	rv->background = pqueue_create();
	rv->timer->cmp = rv->background->cmp = thread_timer_cmp;
	rv->timer->update = rv->background->update = thread_timer_update;
	rv->mutex = os_mutex_init();
	return rv;
}

struct thread_master *thread_master_module_create(int module)
{
	int i = 0;
	if (os_mt_init == 0)
	{
		int i = 0;
		for (i = 0; i < MODULE_MAX; i++)
		{
			master_thread[i] = NULL;
		}
		os_mt_init = 1;
	}
	if (NOT_INT_MAX_MIN_SPACE(module, MODULE_NONE, (MODULE_MAX - 1)))
		return NULL;

	for (i = 0; i < MODULE_MAX; i++)
	{
		if (master_thread[i] && master_thread[i]->module == module)
			return master_thread[i];
	}
	struct thread_master * m = thread_master_create();
	if (m)
	{
		m->module = module;
		return m;
	}
	return NULL;
}

/* Add a new thread to the list.  */
static void thread_list_add(struct thread_list *list, struct thread *thread)
{
	thread->next = NULL;
	thread->prev = list->tail;
	if (list->tail)
		list->tail->next = thread;
	else
		list->head = thread;
	list->tail = thread;
	list->count++;
}

/* Delete a thread from the list. */
static struct thread *
thread_list_delete(struct thread_list *list, struct thread *thread)
{
	if (thread->next)
		thread->next->prev = thread->prev;
	else
		list->tail = thread->prev;
	if (thread->prev)
		thread->prev->next = thread->next;
	else
		list->head = thread->next;
	thread->next = thread->prev = NULL;
	list->count--;
	return thread;
}

/*static void thread_delete_fd(struct thread **thread_array,
		struct thread *thread)
{
	thread_array[thread->u.fd] = NULL;
}

static void thread_add_fd(struct thread **thread_array, struct thread *thread)
{
	thread_array[thread->u.fd] = thread;
}*/

/* Move thread to unuse list. */
static void thread_add_unuse(struct thread_master *m, struct thread *thread)
{
	assert(m != NULL && thread != NULL);
	assert(thread->next == NULL);
	assert(thread->prev == NULL);
	assert(thread->type == THREAD_UNUSED);
	thread_list_add(&m->unuse, thread);
}

/* Free all unused thread. */
static void thread_list_free(struct thread_master *m, struct thread_list *list)
{
	struct thread *t;
	struct thread *next;

	for (t = list->head; t; t = next)
	{
		next = t->next;
		XFREE(MTYPE_THREAD, t);
		list->count--;
		m->alloc--;
	}
}

/*static void thread_array_free(struct thread_master *m,
		struct thread **thread_array)
{
	struct thread *t;
	int index;

	for (index = 0; index < m->fd_limit; ++index)
	{
		t = thread_array[index];
		if (t)
		{
			thread_array[index] = NULL;
			XFREE(MTYPE_THREAD, t);
			m->alloc--;
		}
	}
	XFREE(MTYPE_THREAD, thread_array);
}*/

static void thread_queue_free(struct thread_master *m, struct pqueue *queue)
{
	int i;

	for (i = 0; i < queue->size; i++)
		XFREE(MTYPE_THREAD, queue->array[i]);

	m->alloc -= queue->size;
	pqueue_delete(queue);
}

/* Stop thread scheduler. */
void thread_master_free(struct thread_master *m)
{
	thread_list_free(m, &m->read);
	thread_list_free(m, &m->write);
	thread_queue_free(m, m->timer);
	thread_list_free(m, &m->event);
	thread_list_free(m, &m->ready);
	thread_list_free(m, &m->unuse);
	thread_queue_free(m, m->background);
	if (m->mutex)
		os_mutex_exit(m->mutex);
	XFREE(MTYPE_THREAD_MASTER, m);

}

/* Thread list is empty or not.  */
static int thread_empty(struct thread_list *list)
{
	return list->head ? 0 : 1;
}

/* Delete top of the list and return it. */
static struct thread *
thread_trim_head(struct thread_list *list)
{
	/*	if(m->mutex)
	 os_mutex_lock(m->mutex, OS_WAIT_FOREVER);*/
	if (!thread_empty(list))
		return thread_list_delete(list, list->head);
	/*	if(m->mutex)
	 os_mutex_lock(m->mutex, OS_WAIT_FOREVER);*/
	return NULL;
}

/* Return remain time in second. */
unsigned long thread_timer_remain_second(struct thread *thread)
{
	os_get_relative(&thread->master->relative_time);

	if (thread->u.sands.tv_sec - thread->master->relative_time.tv_sec > 0)
		return thread->u.sands.tv_sec - thread->master->relative_time.tv_sec;
	else
		return 0;
}

struct timeval thread_timer_remain(struct thread *thread)
{
	os_get_relative(&thread->master->relative_time);
	return os_timeval_subtract(thread->u.sands, thread->master->relative_time);
}

static int thread_max_fd_update(struct thread_master *m, int fd)
{
	m->max_fd = MAX(m->max_fd, fd);
	return 0;
}

#define debugargdef  const char *funcname, const char *schedfrom, int fromln
#define debugargpass funcname, schedfrom, fromln

/* Get new thread.  */
static struct thread *
thread_get(struct thread_master *m, u_char type, int (*func)(struct thread *),
		void *arg, debugargdef)
{
	struct thread *thread = thread_trim_head(&m->unuse);

	if (!thread)
	{
		thread = XCALLOC(MTYPE_THREAD, sizeof(struct thread));
		m->alloc++;
	}
	thread->type = type;
	thread->add_type = type;
	thread->master = m;
	thread->func = func;
	thread->arg = arg;
	thread->index = -1;

	thread->funcname = funcname;
	thread->schedfrom = schedfrom;
	thread->schedfrom_line = fromln;

	return thread;
}

#define fd_copy_fd_set(X) (X)

static int fd_select(int size, thread_fd_set *read, thread_fd_set *write,
		thread_fd_set *except, struct timeval *t)
{
	return (select(size, read, write, except, t));
}

static int fd_is_set(int fd, thread_fd_set *fdset)
{
	return FD_ISSET(fd, fdset);
}

static int fd_clear_read_write(int fd, thread_fd_set *fdset)
{
	if (!FD_ISSET(fd, fdset))
		return 0;

	FD_CLR(fd, fdset);
	return 1;
}

static struct thread *
funcname_thread_add_read_write(int dir, struct thread_master *m,
		int (*func)(struct thread *), void *arg, int fd,
		debugargdef)
{
	struct thread *thread = NULL;
	thread_fd_set *fdset = NULL;
	if (m->mutex)
		os_mutex_lock(m->mutex, OS_WAIT_FOREVER);
	if (dir == THREAD_READ)
		fdset = &m->readfd;
	else
		fdset = &m->writefd;

	if (FD_ISSET(fd, fdset))
	{
		zlog(ZLOG_DEFAULT, LOG_WARNING, "There is already %s fd [%d]", (dir =
				THREAD_READ) ? "read" : "write", fd);
		if (m->mutex)
			os_mutex_unlock(m->mutex);
		return NULL;
	}

	FD_SET(fd, fdset);

	thread_max_fd_update(m, fd);

	thread = thread_get(m, dir, func, arg, debugargpass);
	thread->u.fd = fd;
	if (dir == THREAD_READ)
		thread_list_add(&m->read, thread);
	else
		thread_list_add(&m->write, thread);
	if (m->mutex)
		os_mutex_unlock(m->mutex);
	return thread;
}

/* Add new read thread. */
struct thread *
funcname_thread_add_read(struct thread_master *m, int (*func)(struct thread *),
		void *arg, int fd,
		debugargdef)
{
	return funcname_thread_add_read_write(THREAD_READ, m, func, arg, fd,
			debugargpass);
}

/* Add new write thread. */
struct thread *
funcname_thread_add_write(struct thread_master *m, int (*func)(struct thread *),
		void *arg, int fd,
		debugargdef)
{
	return funcname_thread_add_read_write(THREAD_WRITE, m, func, arg, fd,
			debugargpass);
}

static struct thread *
funcname_thread_add_timer_timeval(struct thread_master *m,
		int (*func)(struct thread *), int type, void *arg,
		struct timeval *time_relative,
		debugargdef)
{
	struct thread *thread;
	struct pqueue *queue;
	struct timeval alarm_time;

	assert(m != NULL);

	assert(type == THREAD_TIMER || type == THREAD_BACKGROUND);
	assert(time_relative);
	if (m->mutex)
		os_mutex_lock(m->mutex, OS_WAIT_FOREVER);
	queue = ((type == THREAD_TIMER) ? m->timer : m->background);
	thread = thread_get(m, type, func, arg, debugargpass);

	/* Do we need jitter here? */

	os_get_relative(&m->relative_time);
	alarm_time.tv_sec = m->relative_time.tv_sec + time_relative->tv_sec;
	alarm_time.tv_usec = m->relative_time.tv_usec + time_relative->tv_usec;

	thread->u.sands = os_timeval_adjust(alarm_time);

	pqueue_enqueue(thread, queue);
	if (m->mutex)
		os_mutex_unlock(m->mutex);
	return thread;
}

/* Add timer event thread. */
struct thread *
funcname_thread_add_timer(struct thread_master *m, int (*func)(struct thread *),
		void *arg, long timer,
		debugargdef)
{
	struct timeval trel;

	assert(m != NULL);

	trel.tv_sec = timer;
	trel.tv_usec = 0;

	return funcname_thread_add_timer_timeval(m, func, THREAD_TIMER, arg, &trel,
			debugargpass);
}

/* Add timer event thread with "millisecond" resolution */
struct thread *
funcname_thread_add_timer_msec(struct thread_master *m,
		int (*func)(struct thread *), void *arg, long timer,
		debugargdef)
{
	struct timeval trel;

	assert(m != NULL);

	trel.tv_sec = timer / 1000;
	trel.tv_usec = 1000 * (timer % 1000);

	return funcname_thread_add_timer_timeval(m, func, THREAD_TIMER, arg, &trel,
			debugargpass);
}

/* Add timer event thread with "millisecond" resolution */
struct thread *
funcname_thread_add_timer_tv(struct thread_master *m,
		int (*func)(struct thread *), void *arg, struct timeval *tv,
		debugargdef)
{
	return funcname_thread_add_timer_timeval(m, func, THREAD_TIMER, arg, tv,
			debugargpass);
}

/* Add a background thread, with an optional millisec delay */
struct thread *
funcname_thread_add_background(struct thread_master *m,
		int (*func)(struct thread *), void *arg, long delay,
		debugargdef)
{
	struct timeval trel;

	assert(m != NULL);

	if (delay)
	{
		trel.tv_sec = delay / 1000;
		trel.tv_usec = 1000 * (delay % 1000);
	}
	else
	{
		trel.tv_sec = 0;
		trel.tv_usec = 0;
	}

	return funcname_thread_add_timer_timeval(m, func, THREAD_BACKGROUND, arg,
			&trel, debugargpass);
}

/* Add simple event thread. */
struct thread *
funcname_thread_add_event(struct thread_master *m, int (*func)(struct thread *),
		void *arg, int val,
		debugargdef)
{
	struct thread *thread;

	assert(m != NULL);
	if (m->mutex)
		os_mutex_lock(m->mutex, OS_WAIT_FOREVER);
	thread = thread_get(m, THREAD_EVENT, func, arg, debugargpass);
	thread->u.val = val;
	thread_list_add(&m->event, thread);
//  OS_DEBUG("%s:%s\r\n",__func__,thread->funcname);
	if (m->mutex)
		os_mutex_unlock(m->mutex);
	return thread;
}

/* Cancel thread from scheduler. */
void thread_cancel(struct thread *thread)
{
	struct thread_list *list = NULL;
	struct pqueue *queue = NULL;
	//struct thread **thread_array = NULL;
	if (thread->master->mutex)
		os_mutex_lock(thread->master->mutex, OS_WAIT_FOREVER);
	switch (thread->type)
	{
	case THREAD_READ:
		assert(fd_clear_read_write(thread->u.fd, &thread->master->readfd));
		list = &thread->master->read;
		break;
	case THREAD_WRITE:
		assert(fd_clear_read_write(thread->u.fd, &thread->master->writefd));
		list = &thread->master->write;
		break;
	case THREAD_TIMER:
		queue = thread->master->timer;
		break;
	case THREAD_EVENT:
		list = &thread->master->event;
		break;
	case THREAD_READY:
		list = &thread->master->ready;
		break;
	case THREAD_BACKGROUND:
		queue = thread->master->background;
		break;
	default:
		return;
		break;
	}

	if (queue)
	{
		assert(thread->index >= 0);
		assert(thread == queue->array[thread->index]);
		pqueue_remove_at(thread->index, queue);
	}
	else if (list)
	{
		thread_list_delete(list, thread);
	}
/*	else if (thread_array)
	{
		thread_delete_fd(thread_array, thread);
	}*/
	else
	{
		assert(!"Thread should be either in queue or list or array!");
	}

	thread->type = THREAD_UNUSED;
	thread_add_unuse(thread->master, thread);
	if (thread->master->mutex)
		os_mutex_unlock(thread->master->mutex);
}

/* Delete all events which has argument value arg. */
unsigned int thread_cancel_event(struct thread_master *m, void *arg)
{
	unsigned int ret = 0;
	struct thread *thread;
	if (m->mutex)
		os_mutex_lock(m->mutex, OS_WAIT_FOREVER);
	thread = m->event.head;
	while (thread)
	{
		struct thread *t;

		t = thread;
		thread = t->next;

		if (t->arg == arg)
		{
			ret++;
			thread_list_delete(&m->event, t);
			t->type = THREAD_UNUSED;
			thread_add_unuse(m, t);
		}
	}

	/* thread can be on the ready list too */
	thread = m->ready.head;
	while (thread)
	{
		struct thread *t;

		t = thread;
		thread = t->next;

		if (t->arg == arg)
		{
			ret++;
			thread_list_delete(&m->ready, t);
			t->type = THREAD_UNUSED;
			thread_add_unuse(m, t);
		}
	}
	if (m->mutex)
		os_mutex_unlock(m->mutex);
	return ret;
}

static struct timeval *
thread_timer_wait(struct pqueue *queue, struct timeval *timer_val)
{
	if (queue->size)
	{
		struct thread *next_timer = queue->array[0];
		*timer_val = os_timeval_subtract(next_timer->u.sands,
				next_timer->master->relative_time);
		return timer_val;
	}
	return NULL;
}

static struct thread *
thread_run(struct thread_master *m, struct thread *thread, struct thread *fetch)
{
	if (m->mutex)
		os_mutex_lock(m->mutex, OS_WAIT_FOREVER);
	*fetch = *thread;
	thread->type = THREAD_UNUSED;
	thread_add_unuse(m, thread);
	if (m->mutex)
		os_mutex_unlock(m->mutex);
	return fetch;
}

static unsigned int thread_process_fds_helper(struct thread_master *m,
		struct thread_list *list, thread_fd_set *fdset)
{
	thread_fd_set *mfdset = NULL;
	struct thread *thread;
	struct thread *next;
	unsigned int ready = 0;
	if (!list)
		return 0;
	for (thread = list->head; thread; thread = next)
	{
		next = thread->next;
		if (fd_is_set(THREAD_FD(thread), fdset))
		{
			if (thread->type == THREAD_READ)
				mfdset = &m->readfd;
			else
				mfdset = &m->writefd;

			fd_clear_read_write(THREAD_FD(thread), mfdset);
			thread_list_delete(list, thread);
			thread->type = THREAD_READY;
			thread_list_add(&thread->master->ready, thread);
			ready++;
		}
	}
	return ready;
}
/*static int thread_process_fds_helper(struct thread_master *m,
		struct thread *thread, thread_fd_set *fdset)
{
	thread_fd_set *mfdset = NULL;
	struct thread **thread_array;

	if (!thread)
		return 0;

	if (thread->type == THREAD_READ)
	{
		mfdset = &m->readfd;
		thread_array = m->read;
	}
	else
	{
		mfdset = &m->writefd;
		thread_array = m->write;
	}

	if (fd_is_set(THREAD_FD(thread), fdset))
	{
		fd_clear_read_write(THREAD_FD(thread), mfdset);
		thread_delete_fd(thread_array, thread);
		thread_list_add(&m->ready, thread);
		thread->type = THREAD_READY;
		return 1;
	}
	return 0;
}*/

static int thread_process_fds(struct thread_master *m, thread_fd_set *rset,
		thread_fd_set *wset, int num)
{
	int ready = 0;//, index;
	if (m->mutex)
		os_mutex_lock(m->mutex, OS_WAIT_FOREVER);
	//for (index = 0; index < m->fd_limit && ready < num; ++index)
	{
		ready += thread_process_fds_helper(m, &m->read, rset);
		ready += thread_process_fds_helper(m, &m->write, wset);
	}
	if (m->mutex)
		os_mutex_unlock(m->mutex);
	return num - ready;
}

/* Add all timers that have popped to the ready list. */
static unsigned int thread_timer_process(struct pqueue *queue,
		struct timeval *timenow)
{
	struct thread *thread;
	unsigned int ready = 0;

	while (queue->size)
	{
		thread = queue->array[0];
		if (os_timeval_cmp(*timenow, thread->u.sands) < 0)
		{
			return ready;
		}
		pqueue_dequeue(queue);
		thread->type = THREAD_READY;
		thread_list_add(&thread->master->ready, thread);
		ready++;
	}
	return ready;
}

/* process a list en masse, e.g. for event thread lists */
static unsigned int thread_process(struct thread_list *list)
{
	struct thread *thread;
	struct thread *next;
	unsigned int ready = 0;
	for (thread = list->head; thread; thread = next)
	{
		next = thread->next;
		thread_list_delete(list, thread);
		thread->type = THREAD_READY;
		thread_list_add(&thread->master->ready, thread);
		ready++;
	}
	return ready;
}

/* Fetch next ready thread. */
struct thread *
thread_fetch(struct thread_master *m, struct thread *fetch)
{
	int num = 0;
	struct thread *thread;
	thread_fd_set readfd;
	thread_fd_set writefd;
	thread_fd_set exceptfd;
	struct timeval timer_val = { .tv_sec = 1, .tv_usec = TIMER_SECOND_MICRO };
	struct timeval timer_val_bg;
	struct timeval *timer_wait = &timer_val;
	struct timeval *timer_wait_bg;

	/*
	 extern void * vty_thread_master ();
	 m->ptid = os_task_pthread_self ();
	 m->taskId = os_task_id_self ();
	 */

	while (1)
	{
		/* Signals pre-empt everything */
		quagga_sigevent_process ();
		/* Drain the ready queue of already scheduled jobs, before scheduling
		 * more.
		 */
		if (m->mutex)
			os_mutex_lock(m->mutex, OS_WAIT_FOREVER);
		thread = thread_trim_head(&m->ready);
		if (m->mutex)
			os_mutex_unlock(m->mutex);
		if (thread != NULL)
			return thread_run(m, thread, fetch);

		/* To be fair to all kinds of threads, and avoid starvation, we
		 * need to be careful to consider all thread types for scheduling
		 * in each quanta. I.e. we should not return early from here on.
		 */
		if (m->mutex)
			os_mutex_lock(m->mutex, OS_WAIT_FOREVER);
		/* Normal event are the next highest priority.  */
		thread_process(&m->event);
/*		if (m->mutex)
			os_mutex_unlock(m->mutex);*/
		/* Structure copy.  */
		readfd = fd_copy_fd_set(m->readfd);
		writefd = fd_copy_fd_set(m->writefd);
		exceptfd = fd_copy_fd_set(m->exceptfd);

		/* Calculate select wait timer if nothing else to do */
		if (m->ready.count == 0)
		{
			os_get_relative(&m->relative_time);
			timer_wait = thread_timer_wait(m->timer, &timer_val);
			timer_wait_bg = thread_timer_wait(m->background, &timer_val_bg);

			if (timer_wait_bg
					&& (!timer_wait
							|| (os_timeval_cmp(*timer_wait, *timer_wait_bg) > 0)))
				timer_wait = timer_wait_bg;
		}
		if (pqueue_empty(m->timer) && pqueue_empty(m->background))
			timer_wait = NULL;

		if (timer_wait == NULL)
		{
			timer_val.tv_sec = 1;
			timer_val.tv_usec = TIMER_SECOND_MICRO;
			timer_wait = &timer_val;
		}
		if (timer_wait && (timer_wait->tv_sec = 0)
				&& (timer_wait->tv_usec == 0))
		{
			timer_val.tv_sec = 1;
			timer_val.tv_usec = TIMER_SECOND_MICRO;
			timer_wait = &timer_val;
		}
		if (m->mutex)
			os_mutex_unlock(m->mutex);


		//printf("%s: m->max_fd = %d background.size=%d timer_wait->tv_sec=%d -- %s\r\n", __func__, m->max_fd,
		//		m->background->size, timer_wait->tv_sec, module2name(m->module));
		/*      if(m->max_fd < 1)
		 {
		 sleep(1);
		 continue;
		 }*/
		//timer_wait->tv_sec >>= 2;
		//timer_wait->tv_usec >>= 2;
		//num = select (m->max_fd + 1, &readfd, &writefd, &exceptfd, timer_wait);
		//if(num <= 0)
		num = fd_select(m->max_fd + 1, &readfd, &writefd, &exceptfd, timer_wait);

		//printf("%s: fd_select-----%s\r\n", __func__, module2name(m->module));
//      num = fd_select (FD_SETSIZE, &readfd, &writefd, &exceptfd, timer_wait);

		/* Signals should get quick treatment */
		if (num < 0)
		{
			if (errno == EINTR/*  || m->max_fd == 0*/)
				continue; /* signal received - process it */
			zlog_warn(ZLOG_DEFAULT, "select() error: %s", safe_strerror(errno));
			return NULL;
		}
		if (m->mutex)
			os_mutex_lock(m->mutex, OS_WAIT_FOREVER);
		os_get_relative(&m->relative_time);
		thread_timer_process(m->timer, &m->relative_time);
		if (m->mutex)
			os_mutex_unlock(m->mutex);
		/* Got IO, process it */
		if (num > 0)
			thread_process_fds(m, &readfd, &writefd, num);

#if 0
		/* If any threads were made ready above (I/O or foreground timer),
		 perhaps we should avoid adding background timers to the ready
		 list at this time.  If this is code is uncommented, then background
		 timer threads will not run unless there is nothing else to do. */
		if(m->mutex)
		os_mutex_lock(m->mutex, OS_WAIT_FOREVER);
		thread = thread_trim_head (&m->ready);
		if(m->mutex)
		os_mutex_unlock(m->mutex);
		if (thread != NULL)
		//if ((thread = thread_trim_head (&m->ready)) != NULL)
		return thread_run (m, thread, fetch);
#endif

		if (m->mutex)
			os_mutex_lock(m->mutex, OS_WAIT_FOREVER);
		/* Background timer/events, lowest priority */
		thread_timer_process(m->background, &m->relative_time);

		thread = thread_trim_head(&m->ready);
		if (m->mutex)
			os_mutex_unlock(m->mutex);
		if (thread != NULL)
			//if ((thread = thread_trim_head (&m->ready)) != NULL)
			return thread_run(m, thread, fetch);
	}
}

unsigned long thread_consumed_time(struct timeval *now, struct timeval *start,
		unsigned long *cputime)
{
	return os_timeval_elapsed(*now, *start);
}

/* We should aim to yield after THREAD_YIELD_TIME_SLOT milliseconds. 
 Note: we are using real (wall clock) time for this calculation.
 It could be argued that CPU time may make more sense in certain
 contexts.  The things to consider are whether the thread may have
 blocked (in which case wall time increases, but CPU time does not),
 or whether the system is heavily loaded with other processes competing
 for CPU time.  On balance, wall clock time seems to make sense.
 Plus it has the added benefit that gettimeofday should be faster
 than calling getrusage. */
int thread_should_yield(struct thread *thread)
{
	os_get_relative(&thread->master->relative_time);
	unsigned long t = os_timeval_elapsed(thread->master->relative_time,
			thread->real);
	return ((t > THREAD_YIELD_TIME_SLOT) ? t : 0);
}

void thread_getrusage(struct timeval *real)
{
	os_get_relative(real);
}

struct thread *thread_current_get()
{
	u_int module = task_module_self();
	return master_thread[module]->thread_current;
	/*	int i = 0;
	 for(i = 0; i < MODULE_MAX; i++ )
	 {
	 if(master_thread[i] && master_thread[i]->ptid == os_task_pthread_self() )
	 return master_thread[i];
	 }
	 return NULL;*/
}

static void * thread_cpu_get_alloc(struct thread_master *m,
		struct cpu_thread_history *cpu)
{
	int i = 0;
	if (m == NULL)
		return NULL;
	for (i = 0; i < OS_THREAD_CPU_MAX; i++)
	{
		if (m->cpu_record[i].key == 0)
		{
			m->cpu_record[i].data = XCALLOC(MTYPE_THREAD_STATS,
					sizeof(struct cpu_thread_history));
			if (m->cpu_record[i].data)
			{
				struct cpu_thread_history *hist;
				m->cpu_record[i].key = (int) cpu->func;
				hist = (struct cpu_thread_history *) m->cpu_record[i].data;
				os_memset(hist, 0, sizeof(struct cpu_thread_history));
				hist->func = cpu->func;
				hist->funcname = cpu->funcname;
				return m->cpu_record[i].data;
			}
		}
	}
	return NULL;
}
static void * thread_cpu_get(struct thread_master *m,
		struct cpu_thread_history *cpu)
{
	int i = 0;
	if (m == NULL)
		return NULL;
	for (i = 0; i < OS_THREAD_CPU_MAX; i++)
	{
		if (m->cpu_record[i].key && m->cpu_record[i].key == (int) cpu->func)
			return m->cpu_record[i].data;
	}
	return thread_cpu_get_alloc(m, cpu);
}

/* We check thread consumed time. If the system has getrusage, we'll
 use that to get in-depth stats on the performance of the thread in addition
 to wall clock time stats from gettimeofday. */
void thread_call(struct thread *thread)
{
	unsigned long realtime, cputime;
	struct timeval before, after;

	/* Cache a pointer to the relevant cpu history thread, if the thread
	 * does not have it yet.
	 *
	 * Callers submitting 'dummy threads' hence must take care that
	 * thread->cpu is NULL
	 */
	if (thread && thread->add_type == THREAD_EVENT)
		;      //	  OS_DEBUG("%s:%s\r\n",__func__,thread->funcname);
	if (!thread->hist)
	{
		struct cpu_thread_history tmp;

		tmp.func = thread->func;
		tmp.funcname = thread->funcname;

		thread->hist = thread_cpu_get(thread->master, &tmp);

	}

	thread_getrusage(&before);
	thread->real = before;

	thread->master->thread_current = thread;
	(*thread->func)(thread);
	thread->master->thread_current = NULL;

	thread_getrusage(&after);

	realtime = thread_consumed_time(&after, &before, &cputime);
	if (thread->hist)
	{
		thread->hist->real.total += realtime;
		if (thread->hist->real.max < realtime)
			thread->hist->real.max = realtime;
		++(thread->hist->total_calls);
		thread->hist->types |= (1 << thread->add_type);
	}
#ifdef CONSUMED_TIME_CHECK
	if (realtime > CONSUMED_TIME_CHECK)
	{
		/*
		 * We have a CPU Hog on our hands.
		 * Whinge about it now, so we're aware this is yet another task
		 * to fix.
		 */
/*		zlog_warn(ZLOG_DEFAULT,
				"SLOW THREAD: task %s (%lx) ran for %lums (cpu time %lums)",
				thread->funcname, (unsigned long) thread->func, realtime / 1000,
				cputime / 1000);*/
	}
#endif /* CONSUMED_TIME_CHECK */
}

/* Ready thread */
struct thread *
funcname_thread_ready(struct thread_master *m, int (*func)(struct thread *),
		void *arg, int val,
		debugargdef)
{
	struct thread *thread;
	if (m->mutex)
		os_mutex_lock(m->mutex, OS_WAIT_FOREVER);
	thread = thread_get(m, THREAD_READY, func, arg, debugargpass);
	thread_list_add(&m->ready, thread);
	if (m->mutex)
		os_mutex_unlock(m->mutex);
	return NULL;
}

/* Execute thread */
struct thread *
funcname_thread_execute(struct thread_master *m, int (*func)(struct thread *),
		void *arg, int val,
		debugargdef)
{
	struct thread dummy;

	memset(&dummy, 0, sizeof(struct thread));
	//OS_DEBUG("%s:M=%s",__func__,m? "FULL":"NULL");
	dummy.master = m;
	dummy.type = THREAD_EVENT;
	dummy.add_type = THREAD_EXECUTE;
	//dummy.master = NULL;
	dummy.func = func;
	dummy.arg = arg;
	dummy.u.val = val;

	dummy.funcname = funcname;
	dummy.schedfrom = schedfrom;
	dummy.schedfrom_line = fromln;

	thread_call(&dummy);

	return NULL;
}

static int vty_thread_cpu_show_history(struct vty* vty,
		struct cpu_thread_history *a)
{
	char type[8];
	vty_out(vty, "%7ld.%03ld %9d %8ld %9ld", a->real.total / 1000,
			a->real.total % 1000, a->total_calls,
			a->real.total / a->total_calls, a->real.max);
	os_memset(type, 0, sizeof(type));
	if (a->types & (1 << THREAD_READ))
		strcat(type, "R");
	if (a->types & (1 << THREAD_WRITE))
		strcat(type, "W");
	if (a->types & (1 << THREAD_TIMER))
		strcat(type, "T");
	if (a->types & (1 << THREAD_EVENT))
		strcat(type, "E");
	if (a->types & (1 << THREAD_EXECUTE))
		strcat(type, "X");
	if (a->types & (1 << THREAD_BACKGROUND))
		strcat(type, "B");
	vty_out(vty, "  %-5s %s%s", type, a->funcname, VTY_NEWLINE);
	/*  vty_out(vty, " %c%c%c%c%c%c %s%s",
	 a->types & (1 << THREAD_READ) ? 'R':' ',
	 a->types & (1 << THREAD_WRITE) ? 'W':' ',
	 a->types & (1 << THREAD_TIMER) ? 'T':' ',
	 a->types & (1 << THREAD_EVENT) ? 'E':' ',
	 a->types & (1 << THREAD_EXECUTE) ? 'X':' ',
	 a->types & (1 << THREAD_BACKGROUND) ? 'B' : ' ',
	 a->funcname, VTY_NEWLINE);*/
	return 0;
}

static void vty_thread_cpu_show_head(struct vty *vty)
{
	vty_out(vty, "Runtime(ms)   Invoked Avg uSec Max uSecs");
	vty_out(vty, "  Type  Thread%s", VTY_NEWLINE);
}

static int vty_thread_cpu_get_history(struct thread_master *m)
{
	int i = 0;
	if (m == NULL)
		return 0;
	for (i = 0; i < OS_THREAD_CPU_MAX; i++)
	{
		if (m->cpu_record[i].key)
		{
			if (m->cpu_record[i].data)
			{
				return 1;
			}
		}
	}
	return 0;
}

static int vty_thread_cpu_show_detail(struct thread_master *m, struct vty *vty,
		int detail, thread_type type)
{
	int i = 0;
	if (m == NULL)
		return 0;
	for (i = 0; i < OS_THREAD_CPU_MAX; i++)
	{
		if (m->cpu_record[i].key)
		{
			if (m->cpu_record[i].data)
			{
				struct cpu_thread_history *hist;
				hist = (struct cpu_thread_history *) m->cpu_record[i].data;
				if (hist->types & type)
					vty_thread_cpu_show_history(vty, hist);
			}
		}
	}
	return 0;
}

static int vty_clear_thread_cpu(struct thread_master *m, struct vty *vty,
		thread_type type)
{
	int i = 0;
	if (m == NULL)
		return 0;
	for (i = 0; i < OS_THREAD_CPU_MAX; i++)
	{
		if (m->cpu_record[i].key)
		{
			if (m->cpu_record[i].data)
			{
				struct cpu_thread_history *hist;
				hist = (struct cpu_thread_history *) m->cpu_record[i].data;
				if (hist->types & type)
				{
					os_memset(hist, 0, sizeof(struct cpu_thread_history));
					//vty_thread_cpu_show_history(vty, hist);
				}
			}
		}
	}
	return 0;
}

static thread_type vty_thread_cpu_filter(struct vty *vty, const char *argv)
{
	int i = 0;
	thread_type filter = (thread_type) -1U;
	if (argv)
	{
		filter = 0;
		while (argv[i] != '\0')
		{
			switch (argv[i])
			{
			case 'r':
			case 'R':
				filter |= (1 << THREAD_READ);
				break;
			case 'w':
			case 'W':
				filter |= (1 << THREAD_WRITE);
				break;
			case 't':
			case 'T':
				filter |= (1 << THREAD_TIMER);
				break;
			case 'e':
			case 'E':
				filter |= (1 << THREAD_EVENT);
				break;
			case 'x':
			case 'X':
				filter |= (1 << THREAD_EXECUTE);
				break;
			case 'b':
			case 'B':
				filter |= (1 << THREAD_BACKGROUND);
				break;
			default:
				break;
			}
			++i;
		}
		if (filter == 0)
		{
			vty_out(vty, "Invalid filter \"%s\" specified,"
					" must contain at least one of 'RWTEXB'%s", argv,
			VTY_NEWLINE);
			return 0;
		}
	}
	return filter;
}

#if 0
static int strncasecmp(const char* s1, const char* s2, size_t n)
{
	char c1, c2;
	if (!n)
	return 0;
	do
	{
		c1 = *s1++;
		c2 = *s2++;
	}
	while (--n && c1 && c2 && (tolower(c1) == tolower(c2)));
	return tolower(c1) - tolower(c2);
}
#endif

DEFUN(show_thread_cpu,
		show_thread_cpu_cmd,
		"show thread cpu [FILTER]",
		SHOW_STR
		"Thread information\n"
		"Thread CPU usage\n"
		"Display filter (rwtexb)\n")
{
	int i = 0;
	thread_type filter = 0xff;
	//extern const char *zlog_proto_names[];
	if (argc == 1)
	{
		filter = vty_thread_cpu_filter(vty, argv[0]);
		if (filter == 0)
			return CMD_WARNING;
	}
	vty_thread_cpu_show_head(vty);
	for (i = 0; i < MODULE_MAX; i++)
	{
		if (master_thread[i])
		{
			if (vty_thread_cpu_get_history(master_thread[i]))
			{
				vty_out(vty, "%s of cpu process:%s", module2name(i),
				VTY_NEWLINE);
				vty_thread_cpu_show_detail(master_thread[i], vty, 1, filter);
			}
		}
	}
	return CMD_SUCCESS;
}

#define OS_THREAD_STR_TASK \
  "(zebra|rip|ripng|ospf|ospf6|isis|bgp|pim|olsr|frp)"
#define OS_THREAD_STR_TASK_HELP \
  "NSM process\n" \
  "RIP process\n" \
  "RIPng process\n" \
  "OSPFv2 process\n" \
  "OSPFv3 process\n" \
  "IS-IS process\n" \
  "BGP process\n" \
  "PIM process\n" \
  "OLSR process\n" \
  "FRP process\n"

DEFUN(show_thread_task_cpu,
		show_thread_task_cpu_cmd,
		"show thread " OS_THREAD_STR_TASK "cpu [FILTER]",
		SHOW_STR
		"Thread information\n"
		OS_THREAD_STR_TASK_HELP
		"Thread CPU usage\n"
		"Display filter (rwtexb)\n")
{
	int i = 0, index = 0;
	thread_type filter = 0xff;
	if (argc > 1)
	{
		//char module[16];
		char input[16];
		if (argc == 2)
		{
			filter = vty_thread_cpu_filter(vty, argv[1]);
			if (filter == 0)
				return CMD_WARNING;
		}
		os_memset(input, 0, sizeof(input));
		os_strcpy(input, argv[0]);
		index = name2module(input);
		/*		extern const char *zlog_proto_names[];
		 for(i = 0; i < MODULE_MAX; i++)
		 {
		 os_memset(module, 0, sizeof(module));
		 os_strcpy(module, zlog_proto_names[i]);
		 if(strncasecmp(input, module, sizeof(module)) == 0)
		 {
		 index = i;
		 break;
		 }
		 }*/
		if (master_thread[index])
		{
			if (vty_thread_cpu_get_history(master_thread[i]))
			{
				vty_out(vty, "%s of cpu process:%s", module2name(index),
				VTY_NEWLINE);
				vty_thread_cpu_show_head(vty);
				vty_thread_cpu_show_detail(master_thread[index], vty, 1,
						filter);
			}
			else
			{
				vty_out(vty, "%s of cpu process: no thread use cpu%s",
						module2name(index),
						VTY_NEWLINE);
			}
		}
	}
	return CMD_SUCCESS;
}

DEFUN(clear_thread_cpu,
		clear_thread_cpu_cmd,
		"clear thread cpu [FILTER]",
		"Clear stored data\n"
		"Thread information\n"
		"Thread CPU usage\n"
		"Display filter (rwtexb)\n")
{
	int i = 0;
	thread_type filter = 0xff;
	if (argc == 1)
	{
		filter = vty_thread_cpu_filter(vty, argv[0]);
		if (filter == 0)
			return CMD_WARNING;
	}
	for (i = 0; i < MODULE_MAX; i++)
	{
		if (master_thread[i])
		{
			if (vty_thread_cpu_get_history(master_thread[i]))
			{
				vty_clear_thread_cpu(master_thread[i], vty, filter);
			}
		}
	}
	return CMD_SUCCESS;
}

DEFUN(clear_thread_task_cpu,
		clear_thread_task_cpu_cmd,
		"clear thread " OS_THREAD_STR_TASK "cpu [FILTER]",
		"Clear stored data\n"
		"Thread information\n"
		OS_THREAD_STR_TASK_HELP
		"Thread CPU usage\n"
		"Display filter (rwtexb)\n")
{
	int i = 0, index = 0;
	thread_type filter = 0xff;
	if (argc > 1)
	{
		//char module[16];
		char input[16];
		if (argc == 2)
		{
			filter = vty_thread_cpu_filter(vty, argv[1]);
			if (filter == 0)
				return CMD_WARNING;
		}
		os_memset(input, 0, sizeof(input));
		os_strcpy(input, argv[0]);
		index = name2module(input);
		/*		extern const char *zlog_proto_names[];
		 for(i = 0; i < MODULE_MAX; i++)
		 {
		 os_memset(module, 0, sizeof(module));
		 os_strcpy(module, zlog_proto_names[i]);
		 if(strncasecmp(input, module, sizeof(module)) == 0)
		 {
		 index = i;
		 break;
		 }
		 }*/
		if (master_thread[index])
		{
			if (vty_thread_cpu_get_history(master_thread[i]))
			{
				vty_clear_thread_cpu(master_thread[index], vty, filter);
			}
			else
			{
				vty_out(vty, "%s of cpu process: no thread use cpu%s",
						module2name(index),
						VTY_NEWLINE);
			}
		}
	}
	return CMD_SUCCESS;
}

#if 1
static int cpu_thread_read_write_show(struct thread *lst, struct vty *vty)
{
	if (lst && lst->funcname)
	{
		char type[24];
		os_memset(type, 0, sizeof(type));
		switch (lst->add_type)
		{
		case THREAD_READ:
			sprintf(type, "%s", "read");
			break;
		case THREAD_WRITE:
			sprintf(type, "%s", "write");
			break;
		case THREAD_TIMER:
			sprintf(type, "%s", "timer");
			break;
		case THREAD_EVENT:
			sprintf(type, "%s", "event");
			break;
		case THREAD_READY:
			sprintf(type, "%s", "ready");
			break;
		case THREAD_BACKGROUND:
			sprintf(type, "%s", "backround");
			break;
			//case THREAD_UNUSED:
			//	break;
		case THREAD_EXECUTE:
			sprintf(type, "%s", "execute");
			break;
		default:
			break;
		}
		vty_out(vty, "%-32s [%s]%s", lst->funcname, type, VTY_NEWLINE);
	}
	return 0;
}

static int cpu_thread_list_show(struct thread_list *m, struct vty *vty)
{
	//unsigned int ret = 0;
	struct thread *thread;
	thread = m->head;
	while (thread)
	{
		struct thread *t;
		t = thread;
		thread = t->next;
		if (thread)
		{
			cpu_thread_read_write_show(thread, vty);
		}
	}
	return 0;
}

static int cpu_thread_pqueue_show(struct pqueue *m, struct vty *vty)
{
	struct thread *thread;
	int i = 0;
	for (i = 0; i < m->size; i++)
	{
		thread = m->array[i];
		if (thread)
			cpu_thread_read_write_show(thread, vty);
	}
	return 0;
}

int cpu_thread_show(struct thread_master *m, struct vty *vty)
{
	//int i = 0;

/*	for (i = 0; i < m->fd_limit; i++)
		cpu_thread_read_write_show(m->read[i], vty);
	for (i = 0; i < m->fd_limit; i++)
		cpu_thread_read_write_show(m->write[i], vty);*/
	cpu_thread_list_show(&m->read, vty);
	cpu_thread_list_show(&m->write, vty);
	cpu_thread_list_show(&m->event, vty);
	cpu_thread_pqueue_show(m->timer, vty);
	cpu_thread_pqueue_show(m->background, vty);
	cpu_thread_list_show(&m->ready, vty);
	return 0;
}

DEFUN (show_thread_dump,
		show_thread_dump_cmd,
		"show thread dump",
		SHOW_STR
		"system thread information\n"
		"thread dump information\n")
{
	int i = 0;
	for (i = 0; i < MODULE_MAX; i++)
	{
		if (master_thread[i])
			cpu_thread_show(master_thread[i], vty);
	}
	return CMD_SUCCESS;
}
#endif

int cmd_os_thread_init()
{
	install_element(ENABLE_NODE, &show_thread_dump_cmd);
	install_element(ENABLE_NODE, &show_thread_cpu_cmd);
	install_element(ENABLE_NODE, &show_thread_task_cpu_cmd);

	install_element(ENABLE_NODE, &clear_thread_cpu_cmd);
	install_element(ENABLE_NODE, &clear_thread_task_cpu_cmd);

	return 0;
}

