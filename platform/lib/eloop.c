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

/* #define DEBUG */

#include "zebra.h"
#include "command.h"
#include "hash.h"
#include "log.h"
#include "memory.h"
/*
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
//#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <assert.h>
*/
#include "eloop.h"
#include "os_list.h"
#include "os_time.h"
#include "os_sem.h"

#if defined(__APPLE__)
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif

static int os_mt_init = 0;
struct eloop_master * master_eloop[MODULE_MAX];

#if 0
/* Public export of recent_relative_time by value */
struct timeval
eloop_recent_relative_time (void)
{
	int i = 0;
	static struct timeval relative_time;
	for(i = 0; i < MODULE_MAX; i++ )
	{
		if(master_eloop[i] && master_eloop[i]->taskId == os_task_id_self() )
		return master_eloop[i]->relative_time;
	}
	os_gettime (OS_CLK_REALTIME, &relative_time);
	return relative_time;
}

static int
eloop_timer_cmp(void *a, void *b)
{
	struct eloop *eloop_a = a;
	struct eloop *eloop_b = b;

	long cmp = os_timeval_cmp(eloop_a->u.sands, eloop_b->u.sands);

	if (cmp < 0)
	return -1;
	if (cmp > 0)
	return 1;
	return 0;
}

static void
eloop_timer_update(void *node, int actual_position)
{
	struct eloop *eloop = node;

	eloop->index = actual_position;
}
#endif
/* Allocate new eloop master.  */
struct eloop_master *
eloop_master_create()
{
	struct eloop_master *rv;
	if (os_mt_init == 0)
	{
		int i = 0;
		for (i = 0; i < MODULE_MAX; i++)
		{
			master_eloop[i] = NULL;
		}
		os_mt_init = 1;
	}
	rv = XCALLOC(MTYPE_THREAD_MASTER, sizeof(struct eloop_master));
	if (rv == NULL)
	{
		return NULL;
	}

	rv->fd_limit = 0;
/*	rv->read = XCALLOC(MTYPE_THREAD, sizeof(struct eloop *) * rv->fd_limit);
	if (rv->read == NULL)
	{
		XFREE(MTYPE_THREAD_MASTER, rv);
		return NULL;
	}

	rv->write = XCALLOC(MTYPE_THREAD, sizeof(struct eloop *) * rv->fd_limit);
	if (rv->write == NULL)
	{
		XFREE(MTYPE_THREAD, rv->read);
		XFREE(MTYPE_THREAD_MASTER, rv);
		return NULL;
	}*/

	//rv->ptid = os_task_pthread_self ();

	/* Initialize the timer queues */
	//rv->timer = pqueue_create();
	//rv->background = pqueue_create();
	//rv->timer->cmp = rv->background->cmp = eloop_timer_cmp;
	//rv->timer->update = rv->background->update = eloop_timer_update;
	rv->mutex = os_mutex_init();
	return rv;
}

struct eloop_master *eloop_master_module_create(int module)
{
	int i = 0;
	if (os_mt_init == 0)
	{
		for (i = 0; i < MODULE_MAX; i++)
		{
			master_eloop[i] = NULL;
		}
		os_mt_init = 1;
	}
	if (NOT_INT_MAX_MIN_SPACE(module, MODULE_NONE, (MODULE_MAX - 1)))
		return NULL;

	for (i = 0; i < MODULE_MAX; i++)
	{
		if (master_eloop[i] && master_eloop[i]->module == module)
			return master_eloop[i];
	}
	struct eloop_master * m = eloop_master_create();
	if (m)
	{
		m->module = module;
		return m;
	}
	return NULL;
}

/* Add a new eloop to the list.  */
static void eloop_list_add(struct eloop_list *list, struct eloop *eloop)
{
	eloop->next = NULL;
	eloop->prev = list->tail;
	if (list->tail)
		list->tail->next = eloop;
	else
		list->head = eloop;
	list->tail = eloop;
	list->count++;
}

/* Delete a eloop from the list. */
static struct eloop *
eloop_list_delete(struct eloop_list *list, struct eloop *eloop)
{
	if (eloop->next)
		eloop->next->prev = eloop->prev;
	else
		list->tail = eloop->prev;
	if (eloop->prev)
		eloop->prev->next = eloop->next;
	else
		list->head = eloop->next;
	eloop->next = eloop->prev = NULL;
	list->count--;
	return eloop;
}

/* Move eloop to unuse list. */
static void eloop_add_unuse(struct eloop_master *m, struct eloop *eloop)
{
	assert(m != NULL && eloop != NULL);
	assert(eloop->next == NULL);
	assert(eloop->prev == NULL);
	assert(eloop->type == ELOOP_UNUSED);
	eloop_list_add(&m->unuse, eloop);
}

/* Free all unused eloop. */
static void eloop_list_free(struct eloop_master *m, struct eloop_list *list)
{
	struct eloop *t;
	struct eloop *next;

	for (t = list->head; t; t = next)
	{
		next = t->next;
		XFREE(MTYPE_THREAD, t);
		list->count--;
		m->alloc--;
	}
}


/* Stop eloop scheduler. */
void eloop_master_free(struct eloop_master *m)
{
	eloop_list_free(m, &m->read);
	eloop_list_free(m, &m->write);
	eloop_list_free(m, &m->timer);
	eloop_list_free(m, &m->event);
	eloop_list_free(m, &m->ready);
	eloop_list_free(m, &m->unuse);
	if (m->mutex)
		os_mutex_exit(m->mutex);
	XFREE(MTYPE_THREAD_MASTER, m);

}

/* Thread list is empty or not.  */
static int eloop_empty(struct eloop_list *list)
{
	return list->head ? 0 : 1;
}

/* Delete top of the list and return it. */
static struct eloop *
eloop_trim_head(struct eloop_list *list)
{
	/*	if(m->mutex)
	 os_mutex_lock(m->mutex, OS_WAIT_FOREVER);*/
	if (!eloop_empty(list))
		return eloop_list_delete(list, list->head);
	/*	if(m->mutex)
	 os_mutex_lock(m->mutex, OS_WAIT_FOREVER);*/
	return NULL;
}

/* Return remain time in second. */
unsigned long eloop_timer_remain_second(struct eloop *eloop)
{
	os_get_monotonic(&eloop->master->relative_time);

	if (eloop->u.sands.tv_sec - eloop->master->relative_time.tv_sec > 0)
		return eloop->u.sands.tv_sec - eloop->master->relative_time.tv_sec;
	else
		return 0;
}

struct timeval eloop_timer_remain(struct eloop *eloop)
{
	os_get_monotonic(&eloop->master->relative_time);
	return os_timeval_subtract(eloop->u.sands, eloop->master->relative_time);
}

static int eloop_max_fd_update(struct eloop_master *m, int fd)
{
	m->max_fd = MAX(m->max_fd, fd);
	return 0;
}

#define debugargdef  const char *funcname, const char *schedfrom, int fromln
#define debugargpass funcname, schedfrom, fromln

/* Get new eloop.  */
static struct eloop *
eloop_get(struct eloop_master *m, u_char type, int (*func)(struct eloop *),
		void *arg, debugargdef)
{
	struct eloop *eloop = eloop_trim_head(&m->unuse);

	if (!eloop)
	{
		eloop = XCALLOC(MTYPE_THREAD, sizeof(struct eloop));
		m->alloc++;
	}
	eloop->type = type;
	eloop->add_type = type;
	eloop->master = m;
	eloop->func = func;
	eloop->arg = arg;
	eloop->index = -1;

	eloop->funcname = funcname;
	eloop->schedfrom = schedfrom;
	eloop->schedfrom_line = fromln;

	return eloop;
}

#define fd_copy_fd_set(X) (X)

static int fd_select(int size, eloop_fd_set *read, eloop_fd_set *write,
		eloop_fd_set *except, struct timeval *t)
{
	return (ip_select(size, read, write, except, t));
}

static int fd_is_set(int fd, eloop_fd_set *fdset)
{
	return FD_ISSET(fd, fdset);
}

static int fd_clear_read_write(int fd, eloop_fd_set *fdset)
{
	if (!FD_ISSET(fd, fdset))
		return 0;

	FD_CLR(fd, fdset);
	return 1;
}

static struct eloop *
funcname_eloop_add_read_write(int dir, struct eloop_master *m,
		int (*func)(struct eloop *), void *arg, int fd,
		debugargdef)
{
	struct eloop *eloop = NULL;
	eloop_fd_set *fdset = NULL;
	if (m->mutex)
		os_mutex_lock(m->mutex, OS_WAIT_FOREVER);
	if (dir == ELOOP_READ)
		fdset = &m->readfd;
	else
		fdset = &m->writefd;

	if (FD_ISSET(fd, fdset))
	{
		zlog(ZLOG_DEFAULT, LOG_WARNING, "There is already %s fd [%d]", (dir =
				ELOOP_READ) ? "read" : "write", fd);
		if (m->mutex)
			os_mutex_unlock(m->mutex);
		return NULL;
	}

	FD_SET(fd, fdset);

	eloop_max_fd_update(m, fd);

	eloop = eloop_get(m, dir, func, arg, debugargpass);
	eloop->u.fd = fd;
	if (dir == ELOOP_READ)
		eloop_list_add(&m->read, eloop);
	else
		eloop_list_add(&m->write, eloop);
	if (m->mutex)
		os_mutex_unlock(m->mutex);
	return eloop;
}

/* Add new read eloop. */
struct eloop *
funcname_eloop_add_read(struct eloop_master *m, int (*func)(struct eloop *),
		void *arg, int fd,
		debugargdef)
{
	return funcname_eloop_add_read_write(ELOOP_READ, m, func, arg, fd,
			debugargpass);
}

/* Add new write eloop. */
struct eloop *
funcname_eloop_add_write(struct eloop_master *m, int (*func)(struct eloop *),
		void *arg, int fd,
		debugargdef)
{
	return funcname_eloop_add_read_write(ELOOP_WRITE, m, func, arg, fd,
			debugargpass);
}

/* Add a new thread just before the point.  */
static void eloop_list_add_before(struct eloop_list *list,
		struct eloop *point, struct eloop *thread)
{
	thread->next = point;
	thread->prev = point->prev;
	if (point->prev)
		point->prev->next = thread;
	else
		list->head = thread;
	point->prev = thread;
	list->count++;
}

static struct eloop *
funcname_eloop_add_timer_timeval(struct eloop_master *m,
		int (*func)(struct eloop *), int type, void *arg,
		struct timeval *time_relative,
		debugargdef)
{
	struct eloop *eloop;

	struct eloop_list *list;
	struct timeval alarm_time;
	struct eloop *tt;
	assert(m != NULL);

	assert(type == ELOOP_TIMER || type == ELOOP_BACKGROUND);
	assert(time_relative);
	if (m->mutex)
		os_mutex_lock(m->mutex, OS_WAIT_FOREVER);
	list = ((type == ELOOP_TIMER) ? &m->timer : &m->background);
	//list = &m->timer;
	eloop = eloop_get(m, type, func, arg, debugargpass);

	/* Do we need jitter here? */

	os_get_monotonic(&m->relative_time);
	alarm_time.tv_sec = m->relative_time.tv_sec + time_relative->tv_sec;
	alarm_time.tv_usec = m->relative_time.tv_usec + time_relative->tv_usec;

	eloop->u.sands = os_timeval_adjust(alarm_time);

	/* Sort by timeval. */
	for (tt = list->head; tt; tt = tt->next)
		if (os_timeval_cmp(eloop->u.sands, tt->u.sands) <= 0)
			break;

	if (tt)
		eloop_list_add_before(list, tt, eloop);
	else
		eloop_list_add(list, eloop);

	if (m->mutex)
		os_mutex_unlock(m->mutex);
	return eloop;
}

/* Add timer event eloop. */
struct eloop *
funcname_eloop_add_timer(struct eloop_master *m, int (*func)(struct eloop *),
		void *arg, long timer,
		debugargdef)
{
	struct timeval trel;

	assert(m != NULL);

	trel.tv_sec = timer;
	trel.tv_usec = 0;

	return funcname_eloop_add_timer_timeval(m, func, ELOOP_TIMER, arg, &trel,
			debugargpass);
}

/* Add timer event eloop with "millisecond" resolution */
struct eloop *
funcname_eloop_add_timer_msec(struct eloop_master *m,
		int (*func)(struct eloop *), void *arg, long timer,
		debugargdef)
{
	struct timeval trel;

	assert(m != NULL);

	trel.tv_sec = timer / 1000;
	trel.tv_usec = 1000 * (timer % 1000);

	return funcname_eloop_add_timer_timeval(m, func, ELOOP_TIMER, arg, &trel,
			debugargpass);
}

/* Add timer event eloop with "millisecond" resolution */
struct eloop *
funcname_eloop_add_timer_tv(struct eloop_master *m, int (*func)(struct eloop *),
		void *arg, struct timeval *tv,
		debugargdef)
{
	return funcname_eloop_add_timer_timeval(m, func, ELOOP_TIMER, arg, tv,
			debugargpass);
}

/* Add a background eloop, with an optional millisec delay */
struct eloop *
funcname_eloop_add_background(struct eloop_master *m,
		int (*func)(struct eloop *), void *arg, long delay,
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

	return funcname_eloop_add_timer_timeval(m, func, ELOOP_BACKGROUND, arg,
			&trel, debugargpass);
}

/* Add simple event eloop. */
struct eloop *
funcname_eloop_add_event(struct eloop_master *m, int (*func)(struct eloop *),
		void *arg, int val,
		debugargdef)
{
	struct eloop *eloop;

	assert(m != NULL);
	if (m->mutex)
		os_mutex_lock(m->mutex, OS_WAIT_FOREVER);
	eloop = eloop_get(m, ELOOP_EVENT, func, arg, debugargpass);
	eloop->u.val = val;
	eloop_list_add(&m->event, eloop);
	if (m->mutex)
		os_mutex_unlock(m->mutex);
	return eloop;
}

/* Cancel eloop from scheduler. */
void eloop_cancel(struct eloop *eloop)
{
	struct eloop_list *list = NULL;
	if (eloop->master->mutex)
		os_mutex_lock(eloop->master->mutex, OS_WAIT_FOREVER);
	switch (eloop->type)
	{
	case ELOOP_READ:
		assert(fd_clear_read_write(eloop->u.fd, &eloop->master->readfd));
		list = &eloop->master->read;
		break;
	case ELOOP_WRITE:
		assert(fd_clear_read_write(eloop->u.fd, &eloop->master->writefd));
		list = &eloop->master->write;
		break;
	case ELOOP_TIMER:
		list = &eloop->master->timer;
		break;
	case ELOOP_EVENT:
		list = &eloop->master->event;
		break;
	case ELOOP_READY:
		list = &eloop->master->ready;
		break;
	case ELOOP_BACKGROUND:
		//	list = eloop->master->background;
		break;
	default:
		return;
		break;
	}

	/*  if (queue)
	 {
	 assert(eloop->index >= 0);
	 assert(eloop == queue->array[eloop->index]);
	 pqueue_remove_at(eloop->index, queue);
	 }
	 else */if (list)
	{
		eloop_list_delete(list, eloop);
	}
/*	else if (eloop_array)
	{
		eloop_delete_fd(eloop_array, eloop);
	}*/
	else
	{
		assert(!"Thread should be either in queue or list or array!");
	}

	eloop->type = ELOOP_UNUSED;
	eloop_add_unuse(eloop->master, eloop);
	if (eloop->master->mutex)
		os_mutex_unlock(eloop->master->mutex);
}

/* Delete all events which has argument value arg. */
unsigned int eloop_cancel_event(struct eloop_master *m, void *arg)
{
	unsigned int ret = 0;
	struct eloop *eloop;
	if (m->mutex)
		os_mutex_lock(m->mutex, OS_WAIT_FOREVER);
	eloop = m->event.head;
	while (eloop)
	{
		struct eloop *t;

		t = eloop;
		eloop = t->next;

		if (t->arg == arg)
		{
			ret++;
			eloop_list_delete(&m->event, t);
			t->type = ELOOP_UNUSED;
			eloop_add_unuse(m, t);
		}
	}

	/* eloop can be on the ready list too */
	eloop = m->ready.head;
	while (eloop)
	{
		struct eloop *t;

		t = eloop;
		eloop = t->next;

		if (t->arg == arg)
		{
			ret++;
			eloop_list_delete(&m->ready, t);
			t->type = ELOOP_UNUSED;
			eloop_add_unuse(m, t);
		}
	}
	if (m->mutex)
		os_mutex_unlock(m->mutex);
	return ret;
}

static struct timeval *
eloop_timer_wait(struct eloop_list *list, struct timeval *timer_val)
{
	if (!eloop_empty(list))
	{
		struct eloop *thread = list->head;
		if (thread)
		{
			*timer_val = os_timeval_subtract(thread->u.sands,
					thread->master->relative_time);
			return timer_val;
		}
	}
	return NULL;
	/*	struct eloop *thread;
	 unsigned int ready = 0;

	 for (thread = list->head; thread; thread = thread->next)
	 {
	 if(thread && thread->next)
	 {
	 next = thread->next;
	 //eloop_list_delete(list, thread);
	 //thread->type = ELOOP_READY;
	 //eloop_list_add(&thread->master->ready, thread);
	 *timer_val = os_timeval_subtract(next->u.sands,
	 next->master->relative_time);
	 ready++;
	 }
	 break;
	 }
	 return timer_val;*/
	/*  if (queue->size)
	 {
	 struct eloop *next_timer = queue->array[0];
	 *timer_val = os_timeval_subtract (next_timer->u.sands, next_timer->master->relative_time);
	 return timer_val;
	 }
	 return NULL;*/
}

static struct eloop *
eloop_run(struct eloop_master *m, struct eloop *eloop, struct eloop *fetch)
{
	if (m->mutex)
		os_mutex_lock(m->mutex, OS_WAIT_FOREVER);
	*fetch = *eloop;
	eloop->type = ELOOP_UNUSED;
	eloop_add_unuse(m, eloop);
	if (m->mutex)
		os_mutex_unlock(m->mutex);
	return fetch;
}

static int eloop_process_fds_helper(struct eloop_master *m, struct eloop_list *list,
		eloop_fd_set *fdset)
{
	eloop_fd_set *mfdset = NULL;
	//struct eloop *list;
	struct eloop *eloop;
	struct eloop *next;
	unsigned int ready = 0;
	if (!list)
		return 0;

	for (eloop = list->head; eloop; eloop = next)
	{
		next = eloop->next;
		if (fd_is_set(ELOOP_FD(eloop), fdset))
		{
			if (eloop->type == ELOOP_READ)
				mfdset = &m->readfd;
			else
				mfdset = &m->writefd;
			fd_clear_read_write(ELOOP_FD(eloop), mfdset);
			eloop_list_delete(list, eloop);
			eloop_list_add(&m->ready, eloop);
			eloop->type = ELOOP_READY;
			ready++;
		}
	}
	return ready;

/*	if (eloop->type == ELOOP_READ)
	{
		mfdset = &m->readfd;
		list = &m->read;
	}
	else
	{
		mfdset = &m->writefd;
		list = &m->write;
	}
	if (fd_is_set(ELOOP_FD(eloop), fdset))
	{
		fd_clear_read_write(ELOOP_FD(eloop), mfdset);
		eloop_list_delete(list, eloop);
		eloop_list_add(&m->ready, eloop);
		eloop->type = ELOOP_READY;
		return 1;
	}*/
	//return 0;
}

static int eloop_process_fds(struct eloop_master *m, eloop_fd_set *rset,
		eloop_fd_set *wset, int num)
{
	int ready = 0;//, index;
	if (m->mutex)
		os_mutex_lock(m->mutex, OS_WAIT_FOREVER);
	//for (index = 0; index < m->fd_limit && ready < num; ++index)
	{
		ready += eloop_process_fds_helper(m, &m->read, rset);
		ready += eloop_process_fds_helper(m, &m->write, wset);
	}
	if (m->mutex)
		os_mutex_unlock(m->mutex);
	return num - ready;
}

/* Add all timers that have popped to the ready list. */
/*static unsigned int
 eloop_timer_process (struct pqueue *queue, struct timeval *timenow)
 {
 struct eloop *eloop;
 unsigned int ready = 0;

 while (queue->size)
 {
 eloop = queue->array[0];
 if (os_timeval_cmp (*timenow, eloop->u.sands) < 0)
 {
 return ready;
 }
 pqueue_dequeue(queue);
 eloop->type = ELOOP_READY;
 eloop_list_add (&eloop->master->ready, eloop);
 ready++;
 }
 return ready;
 }*/
static unsigned int eloop_timer_process(struct eloop_list *list,
		struct timeval *timenow)
{
	struct timeval timer_val =
	{ .tv_sec = 0, .tv_usec = 0 };
	struct eloop *thread;
	struct eloop *next;
	unsigned int ready = 0;

	for (thread = list->head; thread; thread = next)
	{
		next = thread->next;
		if (thread)
		{
			timer_val = os_timeval_subtract(*timenow, thread->u.sands);
			if (timer_val.tv_sec > 0 || timer_val.tv_usec > 0)
			{
				eloop_list_delete(list, thread);
				thread->type = ELOOP_READY;
				eloop_list_add(&thread->master->ready, thread);
				ready++;
			}
		}
	}
	return ready;
}
/* process a list en masse, e.g. for event eloop lists */
static unsigned int eloop_process(struct eloop_list *list)
{
	struct eloop *eloop;
	struct eloop *next;
	unsigned int ready = 0;
	for (eloop = list->head; eloop; eloop = next)
	{
		next = eloop->next;
		eloop_list_delete(list, eloop);
		eloop->type = ELOOP_READY;
		eloop_list_add(&eloop->master->ready, eloop);
		ready++;
	}
	return ready;
}

/* Fetch next ready eloop. */
struct eloop *
eloop_fetch(struct eloop_master *m, struct eloop *fetch)
{
	struct eloop *eloop;
	eloop_fd_set readfd;
	eloop_fd_set writefd;
	eloop_fd_set exceptfd;
	struct timeval timer_val = { .tv_sec = 1, .tv_usec = TIMER_SECOND_MICRO };
	struct timeval timer_val_bg;
	struct timeval *timer_wait = &timer_val;
	//struct timeval *timer_wait_bg;

	//extern void * vty_eloop_master ();
	//m->ptid = os_task_pthread_self ();
	//m->taskId = os_task_id_self ();

	while (1)
	{
		int num = 0;

		/* Signals pre-empt everything */
		quagga_sigevent_process();

		/* Drain the ready queue of already scheduled jobs, before scheduling
		 * more.
		 */
		if (m->mutex)
			os_mutex_lock(m->mutex, OS_WAIT_FOREVER);
		eloop = eloop_trim_head(&m->ready);
		if (m->mutex)
			os_mutex_unlock(m->mutex);
		if (eloop != NULL)
			return eloop_run(m, eloop, fetch);

		/* To be fair to all kinds of eloops, and avoid starvation, we
		 * need to be careful to consider all eloop types for scheduling
		 * in each quanta. I.e. we should not return early from here on.
		 */
		if (m->mutex)
			os_mutex_lock(m->mutex, OS_WAIT_FOREVER);
		/* Normal event are the next highest priority.  */
		eloop_process(&m->event);
		if (m->mutex)
			os_mutex_unlock(m->mutex);
		/* Structure copy.  */
		readfd = fd_copy_fd_set(m->readfd);
		writefd = fd_copy_fd_set(m->writefd);
		exceptfd = fd_copy_fd_set(m->exceptfd);

		/* Calculate select wait timer if nothing else to do */
		if (m->ready.count == 0)
		{
			os_get_monotonic(&m->relative_time);
			timer_wait = eloop_timer_wait(&m->timer, &timer_val_bg);
			//timer_wait_bg = eloop_timer_wait (m->background, &timer_val_bg);

			/*          if (timer_wait_bg &&
			 (!timer_wait || (os_timeval_cmp (*timer_wait, *timer_wait_bg) > 0)))
			 timer_wait = timer_wait_bg;*/
		}
		//if(eloop_empty(&m->timer))
		if (m->timer.count == 0)
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
/*
		timer_wait->tv_sec = 1;

		printf("%s: m->max_fd = %d timer.count=%d tv_sec=%d tv_usec=%d\r\n",
				module2name(m->module), m->max_fd, m->timer.count,
				timer_wait->tv_sec, timer_wait->tv_usec);
*/

		errno = 0;
		num = fd_select(m->max_fd + 1, &readfd, &writefd, &exceptfd, timer_wait);
		/* Signals should get quick treatment */
		if (num < 0)
		{
			if (errno == EINTR/* || m->max_fd == 0*/)
				continue; /* signal received - process it */
			printf("select(max_fd=%d) error: %s\r\n", m->max_fd, safe_strerror(errno));
			zlog_warn(ZLOG_DEFAULT, "select() error: %s", safe_strerror(errno));
			return NULL;
		}
		if (m->mutex)
			os_mutex_lock(m->mutex, OS_WAIT_FOREVER);
		os_get_monotonic(&m->relative_time);
		eloop_timer_process(&m->timer, &m->relative_time);
		if (m->mutex)
			os_mutex_unlock(m->mutex);
		/* Got IO, process it */
		if (num > 0)
			eloop_process_fds(m, &readfd, &writefd, num);

#if 0
		/* If any eloops were made ready above (I/O or foreground timer),
		 perhaps we should avoid adding background timers to the ready
		 list at this time.  If this is code is uncommented, then background
		 timer eloops will not run unless there is nothing else to do. */
		if(m->mutex)
		os_mutex_lock(m->mutex, OS_WAIT_FOREVER);
		eloop = eloop_trim_head (&m->ready);
		if(m->mutex)
		os_mutex_unlock(m->mutex);
		if (eloop != NULL)
		//if ((eloop = eloop_trim_head (&m->ready)) != NULL)
		return eloop_run (m, eloop, fetch);
#endif

		if (m->mutex)
			os_mutex_lock(m->mutex, OS_WAIT_FOREVER);
		/* Background timer/events, lowest priority */
		//eloop_timer_process (m->background, &m->relative_time);
		eloop = eloop_trim_head(&m->ready);
		if (m->mutex)
			os_mutex_unlock(m->mutex);
		if (eloop != NULL)
			//if ((eloop = eloop_trim_head (&m->ready)) != NULL)
			return eloop_run(m, eloop, fetch);
	}
}

unsigned long eloop_consumed_time(struct timeval *now, struct timeval *start,
		unsigned long *cputime)
{
	return os_timeval_elapsed(*now, *start);
}

/* We should aim to yield after THREAD_YIELD_TIME_SLOT milliseconds.
 Note: we are using real (wall clock) time for this calculation.
 It could be argued that CPU time may make more sense in certain
 contexts.  The things to consider are whether the eloop may have
 blocked (in which case wall time increases, but CPU time does not),
 or whether the system is heavily loaded with other processes competing
 for CPU time.  On balance, wall clock time seems to make sense.
 Plus it has the added benefit that gettimeofday should be faster
 than calling getrusage. */
int eloop_should_yield(struct eloop *eloop)
{

	os_get_monotonic(&eloop->master->relative_time);
	unsigned long t = os_timeval_elapsed(eloop->master->relative_time,
			eloop->real);
	return ((t > ELOOP_YIELD_TIME_SLOT) ? t : 0);
}

void eloop_getrusage(struct timeval *real)
{
	os_get_monotonic(real);
}

struct eloop *eloop_current_get()
{
	u_int module = task_module_self();
	return master_eloop[module]->eloop_current;
	/*	int i = 0;
	 for(i = 0; i < MODULE_MAX; i++ )
	 {
	 if(master_eloop[i] && master_eloop[i]->ptid == os_task_pthread_self() )
	 return master_eloop[i];
	 }
	 return NULL;
	 */
}

static void * eloop_cpu_get_alloc(struct eloop_master *m,
		struct cpu_eloop_history *cpu)
{
	int i = 0;
	if (m == NULL)
		return NULL;
	for (i = 0; i < OS_ELOOP_CPU_MAX; i++)
	{
		if (m->cpu_record[i].key == 0)
		{
			m->cpu_record[i].data = XCALLOC(MTYPE_THREAD_STATS,
					sizeof(struct cpu_eloop_history));
			if (m->cpu_record[i].data)
			{
				struct cpu_eloop_history *hist;
				m->cpu_record[i].key = (int) cpu->func;
				hist = (struct cpu_eloop_history *) m->cpu_record[i].data;
				os_memset(hist, 0, sizeof(struct cpu_eloop_history));
				hist->func = cpu->func;
				hist->funcname = cpu->funcname;
				return m->cpu_record[i].data;
			}
		}
	}
	return NULL;
}
static void * eloop_cpu_get(struct eloop_master *m,
		struct cpu_eloop_history *cpu)
{
	int i = 0;
	if (m == NULL)
		return NULL;
	for (i = 0; i < OS_ELOOP_CPU_MAX; i++)
	{
		if (m->cpu_record[i].key && m->cpu_record[i].key == (int) cpu->func)
			return m->cpu_record[i].data;
	}
	return eloop_cpu_get_alloc(m, cpu);
}

/* We check eloop consumed time. If the system has getrusage, we'll
 use that to get in-depth stats on the performance of the eloop in addition
 to wall clock time stats from gettimeofday. */
void eloop_call(struct eloop *eloop)
{
	unsigned long realtime, cputime;
	struct timeval before, after;

	/* Cache a pointer to the relevant cpu history eloop, if the eloop
	 * does not have it yet.
	 *
	 * Callers submitting 'dummy eloops' hence must take care that
	 * eloop->cpu is NULL
	 */
	if (eloop && eloop->add_type == ELOOP_EVENT)
		;  //	  OS_DEBUG("%s:%s\r\n",__func__,eloop->funcname);
	if (!eloop->hist)
	{
		struct cpu_eloop_history tmp;

		tmp.func = eloop->func;
		tmp.funcname = eloop->funcname;

		eloop->hist = eloop_cpu_get(eloop->master, &tmp);

	}

	eloop_getrusage(&before);
	eloop->real = before;

	eloop->master->eloop_current = eloop;
	(*eloop->func)(eloop);
	eloop->master->eloop_current = NULL;

	eloop_getrusage(&after);

	realtime = eloop_consumed_time(&after, &before, &cputime);
	if (eloop->hist)
	{
		eloop->hist->real.total += realtime;
		if (eloop->hist->real.max < realtime)
			eloop->hist->real.max = realtime;
		++(eloop->hist->total_calls);
		eloop->hist->types |= (1 << eloop->add_type);
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
				"SLOW ELOOP: task %s (%lx) ran for %lums (cpu time %lums)",
				eloop->funcname, (unsigned long) eloop->func, realtime / 1000,
				cputime / 1000);*/
	}
#endif /* CONSUMED_TIME_CHECK */
}

/* Ready eloop */
struct eloop *
funcname_eloop_ready(struct eloop_master *m, int (*func)(struct eloop *),
		void *arg, int val,
		debugargdef)
{
	struct eloop *eloop;
	if (m->mutex)
		os_mutex_lock(m->mutex, OS_WAIT_FOREVER);
	eloop = eloop_get(m, ELOOP_READY, func, arg, debugargpass);
	eloop_list_add(&m->ready, eloop);
	if (m->mutex)
		os_mutex_unlock(m->mutex);
	return NULL;
}

/* Execute eloop */
struct eloop *
funcname_eloop_execute(struct eloop_master *m, int (*func)(struct eloop *),
		void *arg, int val,
		debugargdef)
{
	struct eloop dummy;

	memset(&dummy, 0, sizeof(struct eloop));
	//OS_DEBUG("%s:M=%s",__func__,m? "FULL":"NULL");
	dummy.master = m;
	dummy.type = ELOOP_EVENT;
	dummy.add_type = ELOOP_EXECUTE;
	//dummy.master = NULL;
	dummy.func = func;
	dummy.arg = arg;
	dummy.u.val = val;

	dummy.funcname = funcname;
	dummy.schedfrom = schedfrom;
	dummy.schedfrom_line = fromln;

	eloop_call(&dummy);

	return NULL;
}

static int vty_eloop_cpu_show_history(struct vty* vty,
		struct cpu_eloop_history *a)
{
	char type[8];
	vty_out(vty, "%7ld.%03ld %9d %8ld %9ld", a->real.total / 1000,
			a->real.total % 1000, a->total_calls,
			a->real.total / a->total_calls, a->real.max);
	os_memset(type, 0, sizeof(type));
	if (a->types & (1 << ELOOP_READ))
		strcat(type, "R");
	if (a->types & (1 << ELOOP_WRITE))
		strcat(type, "W");
	if (a->types & (1 << ELOOP_TIMER))
		strcat(type, "T");
	if (a->types & (1 << ELOOP_EVENT))
		strcat(type, "E");
	if (a->types & (1 << ELOOP_EXECUTE))
		strcat(type, "X");
	if (a->types & (1 << ELOOP_BACKGROUND))
		strcat(type, "B");
	vty_out(vty, "  %-5s %s%s", type, a->funcname, VTY_NEWLINE);
	/*  vty_out(vty, " %c%c%c%c%c%c %s%s",
	 a->types & (1 << ELOOP_READ) ? 'R':' ',
	 a->types & (1 << ELOOP_WRITE) ? 'W':' ',
	 a->types & (1 << ELOOP_TIMER) ? 'T':' ',
	 a->types & (1 << ELOOP_EVENT) ? 'E':' ',
	 a->types & (1 << ELOOP_EXECUTE) ? 'X':' ',
	 a->types & (1 << ELOOP_BACKGROUND) ? 'B' : ' ',
	 a->funcname, VTY_NEWLINE);*/
	return 0;
}

static void vty_eloop_cpu_show_head(struct vty *vty)
{
	vty_out(vty, "Runtime(ms)   Invoked Avg uSec Max uSecs");
	vty_out(vty, "  Type  Thread%s", VTY_NEWLINE);
}

static int vty_eloop_cpu_get_history(struct eloop_master *m)
{
	int i = 0;
	if (m == NULL)
		return 0;
	for (i = 0; i < OS_ELOOP_CPU_MAX; i++)
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

static int vty_eloop_cpu_show_detail(struct eloop_master *m, struct vty *vty,
		int detail, eloop_type type)
{
	int i = 0;
	if (m == NULL)
		return 0;
	for (i = 0; i < OS_ELOOP_CPU_MAX; i++)
	{
		if (m->cpu_record[i].key)
		{
			if (m->cpu_record[i].data)
			{
				struct cpu_eloop_history *hist;
				hist = (struct cpu_eloop_history *) m->cpu_record[i].data;
				if (hist->types & type)
					vty_eloop_cpu_show_history(vty, hist);
			}
		}
	}
	return 0;
}

static int vty_clear_eloop_cpu(struct eloop_master *m, struct vty *vty,
		eloop_type type)
{
	int i = 0;
	if (m == NULL)
		return 0;
	for (i = 0; i < OS_ELOOP_CPU_MAX; i++)
	{
		if (m->cpu_record[i].key)
		{
			if (m->cpu_record[i].data)
			{
				struct cpu_eloop_history *hist;
				hist = (struct cpu_eloop_history *) m->cpu_record[i].data;
				if (hist->types & type)
				{
					os_memset(hist, 0, sizeof(struct cpu_eloop_history));
					//vty_eloop_cpu_show_history(vty, hist);
				}
			}
		}
	}
	return 0;
}

static eloop_type vty_eloop_cpu_filter(struct vty *vty, const char *argv)
{
	int i = 0;
	eloop_type filter = (eloop_type) -1U;
	if (argv)
	{
		filter = 0;
		while (argv[i] != '\0')
		{
			switch (argv[i])
			{
			case 'r':
			case 'R':
				filter |= (1 << ELOOP_READ);
				break;
			case 'w':
			case 'W':
				filter |= (1 << ELOOP_WRITE);
				break;
			case 't':
			case 'T':
				filter |= (1 << ELOOP_TIMER);
				break;
			case 'e':
			case 'E':
				filter |= (1 << ELOOP_EVENT);
				break;
			case 'x':
			case 'X':
				filter |= (1 << ELOOP_EXECUTE);
				break;
			case 'b':
			case 'B':
				filter |= (1 << ELOOP_BACKGROUND);
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

DEFUN(show_eloop_cpu,
		show_eloop_cpu_cmd,
		"show eloop cpu [FILTER]",
		SHOW_STR
		"Thread information\n"
		"Thread CPU usage\n"
		"Display filter (rwtexb)\n")
{
	int i = 0;
	eloop_type filter = 0xff;
	if (argc == 1)
	{
		filter = vty_eloop_cpu_filter(vty, argv[0]);
		if (filter == 0)
			return CMD_WARNING;
	}
	vty_eloop_cpu_show_head(vty);
	for (i = 0; i < MODULE_MAX; i++)
	{
		if (master_eloop[i])
		{
			if (vty_eloop_cpu_get_history(master_eloop[i]))
			{
				vty_out(vty, "%s of cpu process:%s", module2name(i),
				VTY_NEWLINE);
				vty_eloop_cpu_show_detail(master_eloop[i], vty, 1, filter);
			}
		}
	}
	return CMD_SUCCESS;
}

#define OS_ELOOP_STR_TASK \
  "(zebra|rip|ripng|ospf|ospf6|isis|bgp|pim|olsr|frp)"
#define OS_ELOOP_STR_TASK_HELP \
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

DEFUN(show_eloop_task_cpu,
		show_eloop_task_cpu_cmd,
		"show eloop " OS_ELOOP_STR_TASK "cpu [FILTER]",
		SHOW_STR
		"Thread information\n"
		OS_ELOOP_STR_TASK_HELP
		"Thread CPU usage\n"
		"Display filter (rwtexb)\n")
{
	int i = 0, index = 0;
	eloop_type filter = 0xff;
	if (argc > 1)
	{
		//char module[16];
		char input[16];
		if (argc == 2)
		{
			filter = vty_eloop_cpu_filter(vty, argv[1]);
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
		if (master_eloop[index])
		{
			if (vty_eloop_cpu_get_history(master_eloop[i]))
			{
				vty_out(vty, "%s of cpu process:%s", module2name(index),
				VTY_NEWLINE);
				vty_eloop_cpu_show_head(vty);
				vty_eloop_cpu_show_detail(master_eloop[index], vty, 1, filter);
			}
			else
			{
				vty_out(vty, "%s of cpu process: no eloop use cpu%s",
						module2name(index),
						VTY_NEWLINE);
			}
		}
	}
	return CMD_SUCCESS;
}

DEFUN(clear_eloop_cpu,
		clear_eloop_cpu_cmd,
		"clear eloop cpu [FILTER]",
		"Clear stored data\n"
		"Thread information\n"
		"Thread CPU usage\n"
		"Display filter (rwtexb)\n")
{
	int i = 0;
	eloop_type filter = 0xff;
	if (argc == 1)
	{
		filter = vty_eloop_cpu_filter(vty, argv[0]);
		if (filter == 0)
			return CMD_WARNING;
	}
	for (i = 0; i < MODULE_MAX; i++)
	{
		if (master_eloop[i])
		{
			if (vty_eloop_cpu_get_history(master_eloop[i]))
			{
				vty_clear_eloop_cpu(master_eloop[i], vty, filter);
			}
		}
	}
	return CMD_SUCCESS;
}

DEFUN(clear_eloop_task_cpu,
		clear_eloop_task_cpu_cmd,
		"clear eloop " OS_ELOOP_STR_TASK "cpu [FILTER]",
		"Clear stored data\n"
		"Thread information\n"
		OS_ELOOP_STR_TASK_HELP
		"Thread CPU usage\n"
		"Display filter (rwtexb)\n")
{
	int i = 0, index = 0;
	eloop_type filter = 0xff;
	if (argc > 1)
	{
		//char module[16];
		char input[16];
		if (argc == 2)
		{
			filter = vty_eloop_cpu_filter(vty, argv[1]);
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
		if (master_eloop[index])
		{
			if (vty_eloop_cpu_get_history(master_eloop[i]))
			{
				vty_clear_eloop_cpu(master_eloop[index], vty, filter);
			}
			else
			{
				vty_out(vty, "%s of cpu process: no eloop use cpu%s",
						module2name(index),
						VTY_NEWLINE);
			}
		}
	}
	return CMD_SUCCESS;
}

#if 1
static int cpu_eloop_read_write_show(struct eloop *lst, struct vty *vty)
{
	if (lst && lst->funcname)
	{
		char type[24];
		os_memset(type, 0, sizeof(type));
		switch (lst->add_type)
		{
		case ELOOP_READ:
			sprintf(type, "%s", "read");
			break;
		case ELOOP_WRITE:
			sprintf(type, "%s", "write");
			break;
		case ELOOP_TIMER:
			sprintf(type, "%s", "timer");
			break;
		case ELOOP_EVENT:
			sprintf(type, "%s", "event");
			break;
		case ELOOP_READY:
			sprintf(type, "%s", "ready");
			break;
		case ELOOP_BACKGROUND:
			sprintf(type, "%s", "backround");
			break;
			//case THREAD_UNUSED:
			//	break;
		case ELOOP_EXECUTE:
			sprintf(type, "%s", "execute");
			break;
		default:
			break;
		}
		vty_out(vty, "%-32s [%s]%s", lst->funcname, type, VTY_NEWLINE);
	}
	return 0;
	/*	if(lst && lst->funcname)
	 vty_out(vty,"%-32s %s",lst->funcname, VTY_NEWLINE);
	 return 0;*/
}

static int cpu_eloop_list_show(struct eloop_list *m, struct vty *vty)
{
	//unsigned int ret = 0;
	struct eloop *eloop;
	eloop = m->head;
	while (eloop)
	{
		struct eloop *t;
		t = eloop;
		eloop = t->next;
		if (eloop)
		{
			cpu_eloop_read_write_show(eloop, vty);
		}
	}
	return 0;
}

/*static int cpu_eloop_pqueue_show(struct pqueue *m, struct vty *vty)
 {
 struct eloop *eloop;
 int i = 0;
 for(i = 0; i < m->size; i++)
 {
 eloop = m->array[i];
 if(eloop)
 cpu_eloop_read_write_show(eloop, vty);
 }
 return 0;
 }*/

int cpu_eloop_show(struct eloop_master *m, struct vty *vty)
{
	cpu_eloop_list_show(&m->read, vty);
	cpu_eloop_list_show(&m->write, vty);
	cpu_eloop_list_show(&m->event, vty);
	cpu_eloop_list_show(&m->timer, vty);
	cpu_eloop_list_show(&m->ready, vty);
	cpu_eloop_list_show(&m->background, vty);
	return 0;
}

DEFUN (show_eloop_dump,
		show_eloop_dump_cmd,
		"show eloop dump",
		SHOW_STR
		"system eloop information\n"
		"eloop dump information\n")
{
	int i = 0;
	for (i = 0; i < MODULE_MAX; i++)
	{
		if (master_eloop[i])
			cpu_eloop_show(master_eloop[i], vty);
	}
	return CMD_SUCCESS;
}
#endif

int cmd_os_eloop_init()
{
	install_element(ENABLE_NODE, &show_eloop_dump_cmd);
	install_element(ENABLE_NODE, &show_eloop_cpu_cmd);
	install_element(ENABLE_NODE, &show_eloop_task_cpu_cmd);

	install_element(ENABLE_NODE, &clear_eloop_cpu_cmd);
	install_element(ENABLE_NODE, &clear_eloop_task_cpu_cmd);

	return 0;
}

