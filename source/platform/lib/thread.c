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

#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "thread.h"
#include "log.h"
#include "hash.h"
#include "pqueue.h"
#include "command.h"
#include "pqueue.h"
#include "zmemory.h"
#if defined(__APPLE__)
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif

static zpl_uint32 os_mt_init = 0;

struct thread_master_list
{
	struct thread_master *head;
	struct thread_master *tail;
	zpl_uint32 count;
};

static struct thread_master_list _master_thread_list;
static void *_master_mutex = NULL;


/* Add a new thread to the list.  */
static void thread_master_list_add(struct thread_master_list *list, struct thread_master *master)
{
	master->next = NULL;
	master->prev = list->tail;
	if (list->tail)
		list->tail->next = master;
	else
		list->head = master;
	list->tail = master;
	list->count++;
}

/* Delete a thread from the list. */
static struct thread_master *thread_master_list_delete(struct thread_master_list *list, struct thread_master *master)
{
	if (master->next)
		master->next->prev = master->prev;
	else
		list->tail = master->prev;
	if (master->prev)
		master->prev->next = master->next;
	else
		list->head = master->next;
	master->next = master->prev = NULL;
	list->count--;
	return master;
}

/* Free all unused thread. */
/*static void thread_master_list_free(struct thread_master_list *list)
{
	struct thread_master *t;
	struct thread_master *next;
	for (t = list->head; t; t = next)
	{
		next = t->next;
		XFREE(MTYPE_THREAD_MASTER, t);
		list->count--;
	}
}
*/

static int thread_master_add_list(struct thread_master *node)
{
	if (_master_mutex)
		os_mutex_lock(_master_mutex, OS_WAIT_FOREVER);	
	thread_master_list_add(&_master_thread_list, node);
	if (_master_mutex)
		os_mutex_unlock(_master_mutex);	
	return OK;
}

static int thread_master_del_list(struct thread_master *node)
{
	struct thread_master *cutmp = NULL;
	if (_master_mutex)
		os_mutex_lock(_master_mutex, OS_WAIT_FOREVER);	
	cutmp = thread_master_list_delete(&_master_thread_list, node);
	//if (cutmp)
	//	XFREE(MTYPE_THREAD_MASTER, cutmp);
	if (_master_mutex)
		os_mutex_unlock(_master_mutex);	
	return OK;
}

static struct thread_master *thread_master_get_list(int mode)
{
	struct thread_master *cutmp = NULL;
	struct thread_master *next = NULL;
	if (_master_mutex)
		os_mutex_lock(_master_mutex, OS_WAIT_FOREVER);		
	for (cutmp = _master_thread_list.head; cutmp; cutmp = next)
	{
		next = cutmp->next;
		if (cutmp && cutmp->module == mode)
		{
			break;
		}
	}
	if (_master_mutex)
		os_mutex_unlock(_master_mutex);	
	return cutmp;
}


static zpl_int thread_timer_cmp(void *a, void *b)
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

static void thread_timer_update(void *node, zpl_uint32 actual_position)
{
	struct thread *thread = node;

	thread->index = actual_position;
}

/* Allocate new thread master.  */
struct thread_master *
thread_master_create()
{
	struct thread_master *rv;
	if (os_mt_init == 0)
	{
		memset(&_master_thread_list, 0, sizeof(_master_thread_list));
		_master_mutex = os_mutex_name_init("thremmutex");
		os_mt_init = 1;
	}
	// getrlimit(RLIMIT_NOFILE, &limit);

	rv = XCALLOC(MTYPE_THREAD_MASTER, sizeof(struct thread_master));
	if (rv == NULL)
	{
		return NULL;
	}
	rv->fd_limit = 0; //(int) limit.rlim_cur;
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

	// rv->ptid = os_task_pthread_self ();
	rv->bquit = zpl_false;
	/* Initialize the timer queues */
	rv->timer = pqueue_create();
	rv->background = pqueue_create();
	rv->timer->cmp = rv->background->cmp = thread_timer_cmp;
	rv->timer->update = rv->background->update = thread_timer_update;
	rv->mutex = os_mutex_name_init("threadmutex");

	thread_master_add_list(rv);

	return rv;
}

struct thread_master *thread_master_module_create(zpl_uint32 module)
{
	
	if (os_mt_init == 0)
	{
		memset(&_master_thread_list, 0, sizeof(_master_thread_list));
		os_mt_init = 1;
	}
	if (NOT_INT_MAX_MIN_SPACE(module, MODULE_NONE, (MODULE_MAX - 1)))
		return NULL;

	if (thread_master_get_list(module))
		return thread_master_get_list(module);
	struct thread_master *m = thread_master_create();
	if (m)
	{
		m->module = module;
		return m;
	}
	return NULL;
}

struct thread_master *thread_master_module_lookup (zpl_uint32 module)
{
	if (NOT_INT_MAX_MIN_SPACE(module, MODULE_NONE, (MODULE_MAX - 1)))
		return NULL;

	if (thread_master_get_list(module))
		return thread_master_get_list(module);
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


static void thread_queue_free(struct thread_master *m, struct pqueue *queue)
{
	zpl_uint32 i;

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
	thread_master_del_list(m);
	if (m->mutex)
		os_mutex_exit(m->mutex);



	XFREE(MTYPE_THREAD_MASTER, m);
}

/* Thread list is empty or not.  */
static zpl_bool thread_empty(struct thread_list *list)
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
zpl_ulong thread_timer_remain_second(struct thread *thread)
{
	os_get_monotonic(&thread->master->relative_time);

	if (thread->u.sands.tv_sec - thread->master->relative_time.tv_sec > 0)
		return thread->u.sands.tv_sec - thread->master->relative_time.tv_sec;
	else
		return 0;
}

struct timeval thread_timer_remain(struct thread *thread)
{
	os_get_monotonic(&thread->master->relative_time);
	return os_timeval_subtract(thread->u.sands, thread->master->relative_time);
}

static int thread_max_fd_update(struct thread_master *m, int fd)
{
	m->max_fd = MAX(m->max_fd, fd);
	return OK;
}

#define debugargdef const char *funcname, const char *schedfrom, zpl_uint32 fromln
#define debugargpass funcname, schedfrom, fromln

/* Get new thread.  */
static struct thread *
thread_get(struct thread_master *m, zpl_uchar type, int (*func)(struct thread *),
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

static zpl_int fd_select(zpl_uint32 size, thread_fd_set *read, thread_fd_set *write,
						 thread_fd_set *except, struct timeval *t)
{
	return (select(size, read, write, except, t));
}

static zpl_int fd_is_set(int fd, thread_fd_set *fdset)
{
	return FD_ISSET(fd, fdset);
}

static zpl_int fd_clear_read_write(int fd, thread_fd_set *fdset)
{
	if (!FD_ISSET(fd, fdset))
		return 0;

	FD_CLR(fd, fdset);
	return 1;
}

static struct thread *
funcname_thread_add_read_write(zpl_uint32 dir, struct thread_master *m,
							   int (*func)(struct thread *), void *arg, zpl_socket_t fd,
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

	if (FD_ISSET(ipstack_fd(fd), fdset))
	{
		zlog(MODULE_DEFAULT, ZLOG_LEVEL_WARNING, "There is already %s fd [%d]", (dir = THREAD_READ) ? "read" : "write", ipstack_fd(fd));
		if (m->mutex)
			os_mutex_unlock(m->mutex);
		return NULL;
	}

	FD_SET(ipstack_fd(fd), fdset);

	thread_max_fd_update(m, ipstack_fd(fd));

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
						 void *arg, zpl_socket_t fd,
						 debugargdef)
{
	return funcname_thread_add_read_write(THREAD_READ, m, func, arg, fd,
										  debugargpass);
}

/* Add new write thread. */
struct thread *
funcname_thread_add_write(struct thread_master *m, int (*func)(struct thread *),
						  void *arg, zpl_socket_t fd,
						  debugargdef)
{
	return funcname_thread_add_read_write(THREAD_WRITE, m, func, arg, fd,
										  debugargpass);
}

static struct thread *
funcname_thread_add_timer_timeval(struct thread_master *m,
								  int (*func)(struct thread *), zpl_uint32 type, void *arg,
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

	os_get_monotonic(&m->relative_time);
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
	if (m->mutex)
		os_mutex_unlock(m->mutex);
	return thread;
}

/* Cancel thread from scheduler. */
void thread_cancel(struct thread *thread)
{
	struct thread_list *list = NULL;
	struct pqueue *queue = NULL;
	if (!thread->master)
	{
		return;
	}
	// struct thread **thread_array = NULL;
	if (thread->master->mutex)
		os_mutex_lock(thread->master->mutex, OS_WAIT_FOREVER);
	switch (thread->type)
	{
	case THREAD_READ:
		assert(fd_clear_read_write(ipstack_fd(thread->u.fd), &thread->master->readfd));
		list = &thread->master->read;
		break;
	case THREAD_WRITE:
		assert(fd_clear_read_write(ipstack_fd(thread->u.fd), &thread->master->writefd));
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
		zlog_debug(MODULE_DEFAULT, "asdddddddd thread->type=%d", thread->type);
		if (thread->master->mutex)
			os_mutex_unlock(thread->master->mutex);
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
		if (thread->master->mutex)
			os_mutex_unlock(thread->master->mutex);
		assert(!"Thread should be either in queue or list or array!");
	}

	thread->type = THREAD_UNUSED;
	thread_add_unuse(thread->master, thread);
	if (thread->master->mutex)
		os_mutex_unlock(thread->master->mutex);
}

/* Delete all events which has argument value arg. */
zpl_uint32 thread_cancel_event(struct thread_master *m, void *arg)
{
	zpl_uint32 ret = 0;
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


static zpl_uint32 thread_process_fds_helper(struct thread_master *m,
											struct thread_list *list, thread_fd_set *fdset)
{
	thread_fd_set *mfdset = NULL;
	struct thread *thread;
	struct thread *next;
	zpl_uint32 ready = 0;
	if (!list)
		return 0;
	for (thread = list->head; thread; thread = next)
	{
		next = thread->next;
		if (fd_is_set(ipstack_fd(THREAD_FD(thread)), fdset))
		{

			if (thread->type == THREAD_READ)
				mfdset = &m->readfd;
			else
				mfdset = &m->writefd;

			fd_clear_read_write(ipstack_fd(THREAD_FD(thread)), mfdset);
			thread_list_delete(list, thread);
			thread->type = THREAD_READY;
			thread_list_add(&m->ready, thread);
			ready++;
		}
	}
	return ready;
}


static zpl_uint32 thread_process_fds(struct thread_master *m, thread_fd_set *rset,
									 thread_fd_set *wset, zpl_uint32 num)
{
	zpl_uint32 ready = 0; //, index;
	// if (m->mutex)
	//	os_mutex_lock(m->mutex, OS_WAIT_FOREVER);
	// for (index = 0; index < m->fd_limit && ready < num; ++index)
	{
		ready += thread_process_fds_helper(m, &m->read, rset);
		ready += thread_process_fds_helper(m, &m->write, wset);
	}
	// if (m->mutex)
	//	os_mutex_unlock(m->mutex);
	return num - ready;
}

/* Add all timers that have popped to the ready list. */
static zpl_uint32 thread_timer_process(struct pqueue *queue,
									   struct timeval *timenow)
{
	struct thread *thread;
	zpl_uint32 ready = 0;

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
static zpl_uint32 thread_process(struct thread_list *list)
{
	struct thread *thread;
	struct thread *next;
	zpl_uint32 ready = 0;
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

int thread_fetch_quit(struct thread_master *m)
{
	if (m)
	{
		m->bquit = zpl_true;
	}
	return OK;
}

int thread_wait_quit(struct thread_master *m)
{
	if (m)
	{
		while (m->bquit)
		{
			os_msleep(50);
		}
	}
	return OK;
}

/* Fetch next ready thread. */
struct thread *
thread_fetch(struct thread_master *m)
{
	zpl_int32 num = 0;
	struct thread *thread = NULL;
	thread_fd_set readfd;
	thread_fd_set writefd;
	thread_fd_set exceptfd;
	struct timeval timer_val = {.tv_sec = 1, .tv_usec = TIMER_SECOND_MICRO};
	struct timeval timer_val_bg;
	struct timeval *timer_wait = &timer_val;
	struct timeval *timer_wait_bg;

	while (OS_TASK_TRUE())
	{
		if (m->mutex)
			os_mutex_lock(m->mutex, OS_WAIT_FOREVER);
		thread = thread_trim_head(&m->ready);
		if (thread != NULL)
		{
			if (m->mutex)
				os_mutex_unlock(m->mutex);
			return thread;
		}

		if (m->bquit)
		{
			m->bquit = zpl_false;
			zlog_debug(MODULE_DEFAULT, "thread_fetch quit RET NULL");
			if (m->mutex)
				os_mutex_unlock(m->mutex);
			return NULL;
		}
		/* To be fair to all kinds of threads, and avoid starvation, we
		 * need to be careful to consider all thread types for scheduling
		 * in each quanta. I.e. we should not return early from here on.
		 */

		/* Normal event are the next highest priority.  */
		thread_process(&m->event);

		/* Structure copy.  */
		readfd = fd_copy_fd_set(m->readfd);
		writefd = fd_copy_fd_set(m->writefd);
		exceptfd = fd_copy_fd_set(m->exceptfd);

		/* Calculate select wait timer if nothing else to do */
		if (m->ready.count == 0)
		{
			os_get_monotonic(&m->relative_time);
			timer_wait = thread_timer_wait(m->timer, &timer_val);
			timer_wait_bg = thread_timer_wait(m->background, &timer_val_bg);

			if (timer_wait_bg && (!timer_wait || (os_timeval_cmp(*timer_wait, *timer_wait_bg) > 0)))
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
		if (timer_wait && (timer_wait->tv_sec = 0) && (timer_wait->tv_usec == 0))
		{
			timer_val.tv_sec = 1;
			timer_val.tv_usec = TIMER_SECOND_MICRO;
			timer_wait = &timer_val;
		}
		if (m->mutex)
			os_mutex_unlock(m->mutex);
		// timer_wait->tv_sec >>= 2;
		// timer_wait->tv_usec >>= 2;
		// num = select (m->max_fd + 1, &readfd, &writefd, &exceptfd, timer_wait);
		// if(num <= 0)
		num = fd_select(m->max_fd + 1, &readfd, &writefd, &exceptfd, timer_wait);

		/* Signals should get quick treatment */
		if (num < 0)
		{
			if (ipstack_errno == IPSTACK_ERRNO_EINTR || ipstack_errno == IPSTACK_ERRNO_EAGAIN/*  || m->max_fd == 0*/)
				continue; /* signal received - process it */
			zlog_warn(MODULE_DEFAULT, "select() error: %s", ipstack_strerror(ipstack_errno));

			return NULL;
		}
		if (m->mutex)
			os_mutex_lock(m->mutex, OS_WAIT_FOREVER);
		os_get_monotonic(&m->relative_time);
		thread_timer_process(m->timer, &m->relative_time);

		/* Got IO, process it */
		if (num > 0)
			thread_process_fds(m, &readfd, &writefd, num);

		/* Background timer/events, lowest priority */
		thread_timer_process(m->background, &m->relative_time);

		thread = thread_trim_head(&m->ready);
		if (thread != NULL)
		{
			if (m->mutex)
				os_mutex_unlock(m->mutex);
			return thread;
		}
		if (m->mutex)
			os_mutex_unlock(m->mutex);
	}
	return NULL;
}

struct thread *
thread_mainloop(struct thread_master *m)
{
	struct thread *rethread = NULL;
	while (OS_TASK_TRUE())
	{
		rethread = thread_fetch((struct thread_master *)m);
		if (rethread)
		{
			thread_call(rethread);
		}
		else
		{
			zlog_debug(MODULE_LIB, "thread_fetch RET NULL");
			return NULL;
		}
		if (m->bquit)
		{
			m->bquit = zpl_false;
			return NULL;
		}
	}
	return NULL;
}

zpl_ulong thread_consumed_time(struct timeval *now, struct timeval *start,
							   zpl_ulong *cputime)
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
	os_get_monotonic(&thread->master->relative_time);
	zpl_ulong t = os_timeval_elapsed(thread->master->relative_time,
									 thread->real);
	return ((t > THREAD_YIELD_TIME_SLOT) ? t : 0);
}

void thread_getrusage(struct timeval *real)
{
	os_get_monotonic(real);
}



static void *thread_cpu_get_alloc(struct thread_master *m,
								  struct cpu_thread_history *cpu)
{
	zpl_uint32 i = 0;
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
				m->cpu_record[i].key = (zpl_uint32)cpu->func;
				hist = (struct cpu_thread_history *)m->cpu_record[i].data;
				os_memset(hist, 0, sizeof(struct cpu_thread_history));
				hist->func = cpu->func;
				hist->funcname = cpu->funcname;
				return m->cpu_record[i].data;
			}
		}
	}
	return NULL;
}
static void *thread_cpu_get(struct thread_master *m,
							struct cpu_thread_history *cpu)
{
	zpl_uint32 i = 0;
	if (m == NULL)
		return NULL;
	for (i = 0; i < OS_THREAD_CPU_MAX; i++)
	{
		if (m->cpu_record[i].key && m->cpu_record[i].key == (zpl_uint32)cpu->func)
			return m->cpu_record[i].data;
	}
	return thread_cpu_get_alloc(m, cpu);
}

void thread_call(struct thread *thread)
{
	zpl_ulong realtime, cputime;
	struct timeval before, after;

	/* Cache a pointer to the relevant cpu history thread, if the thread
	 * does not have it yet.
	 *
	 * Callers submitting 'dummy threads' hence must take care that
	 * thread->cpu is NULL
	 */
	if (thread == NULL)
		return;
	if (thread->master && thread->master->mutex)
		os_mutex_lock(thread->master->mutex, OS_WAIT_FOREVER);		
	if (thread && thread->add_type == THREAD_EVENT)
		; //	  OS_DEBUG("%s:%s\r\n",__func__,thread->funcname);
	if (!thread->hist && thread->master)
	{
		struct cpu_thread_history tmp;

		tmp.func = thread->func;
		tmp.funcname = thread->funcname;

		thread->hist = thread_cpu_get(thread->master, &tmp);
	}

	thread_getrusage(&before);
	thread->real = before;

	zpl_backtrace_symb_set(thread->funcname, thread->schedfrom, thread->schedfrom_line);
	if(thread->master)
		thread->master->thread_current = thread;
	if (thread->master && thread->master->mutex)
		os_mutex_unlock(thread->master->mutex);

	(*thread->func)(thread);
	
	if (thread->master && thread->master->mutex)
		os_mutex_lock(thread->master->mutex, OS_WAIT_FOREVER);
	zpl_backtrace_symb_set(NULL, NULL, 0);
	if(thread->master)
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
		/*		zlog_warn(MODULE_DEFAULT,
						"SLOW THREAD: task %s (%lx) ran for %lums (cpu time %lums)",
						thread->funcname, (zpl_ulong) thread->func, realtime / 1000,
						cputime / 1000);*/
	}
#endif /* CONSUMED_TIME_CHECK */
	if (thread && thread->master && thread->add_type != THREAD_EXECUTE)
	{
		thread->type = THREAD_UNUSED;
		thread_add_unuse(thread->master, thread);
	}
	if (thread->master && thread->master->mutex)
		os_mutex_unlock(thread->master->mutex);
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
	// OS_DEBUG("%s:M=%s",__func__,m? "FULL":"NULL");
	//dummy.master = m;
	dummy.type = THREAD_EVENT;
	dummy.add_type = THREAD_EXECUTE;
	// dummy.master = NULL;
	dummy.func = func;
	dummy.arg = arg;
	dummy.u.val = val;

	dummy.funcname = funcname;
	dummy.schedfrom = schedfrom;
	dummy.schedfrom_line = fromln;

	thread_call(&dummy);

	return NULL;
}
#ifdef ZPL_SHELL_MODULE
static int vty_thread_cpu_show_history(struct vty *vty,
									   struct cpu_thread_history *a)
{
	zpl_char type[8];
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
	return OK;
}

static void vty_thread_cpu_show_head(struct vty *vty)
{
	vty_out(vty, "Runtime(ms)   Invoked Avg uSec Max uSecs");
	vty_out(vty, "  Type  Thread%s", VTY_NEWLINE);
}

static zpl_bool vty_thread_cpu_get_history(struct thread_master *m)
{
	zpl_uint32 i = 0;
	if (m == NULL)
		return 0;
	for (i = 0; i < OS_THREAD_CPU_MAX; i++)
	{
		if (m->cpu_record[i].key)
		{
			if (m->cpu_record[i].data)
			{
				return zpl_true;
			}
		}
	}
	return zpl_false;
}

static int vty_thread_cpu_show_detail(struct thread_master *m, struct vty *vty,
									  zpl_bool detail, thread_type type)
{
	zpl_uint32 i = 0;
	if (m == NULL)
		return 0;
	for (i = 0; i < OS_THREAD_CPU_MAX; i++)
	{
		if (m->cpu_record[i].key)
		{
			if (m->cpu_record[i].data)
			{
				struct cpu_thread_history *hist;
				hist = (struct cpu_thread_history *)m->cpu_record[i].data;
				if (hist->types & type)
					vty_thread_cpu_show_history(vty, hist);
			}
		}
	}
	return OK;
}

static int vty_clear_thread_cpu(struct thread_master *m, struct vty *vty,
								thread_type type)
{
	zpl_uint32 i = 0;
	if (m == NULL)
		return OK;
	for (i = 0; i < OS_THREAD_CPU_MAX; i++)
	{
		if (m->cpu_record[i].key)
		{
			if (m->cpu_record[i].data)
			{
				struct cpu_thread_history *hist;
				hist = (struct cpu_thread_history *)m->cpu_record[i].data;
				if (hist->types & type)
				{
					os_memset(hist, 0, sizeof(struct cpu_thread_history));
					// vty_thread_cpu_show_history(vty, hist);
				}
			}
		}
	}
	return OK;
}

static thread_type vty_thread_cpu_filter(struct vty *vty, const char *argv)
{
	zpl_uint32 i = 0;
	thread_type filter = (thread_type)-1U;
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
						 " must contain at least one of 'RWTEXB'%s",
					argv,
					VTY_NEWLINE);
			return 0;
		}
	}
	return filter;
}

#if 0
static int strncasecmp(const char* s1, const char* s2, zpl_size_t n)
{
	zpl_char c1, c2;
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
	thread_type filter = 0xff;

	struct thread_master *cutmp = NULL;
	struct thread_master *next = NULL;

	if (argc == 1)
	{
		filter = vty_thread_cpu_filter(vty, argv[0]);
		if (filter == 0)
			return CMD_WARNING;
	}
	vty_thread_cpu_show_head(vty);

	for (cutmp = _master_thread_list.head; cutmp; cutmp = next)
	{
		next = cutmp->next;
		if (cutmp)
		{
			if (vty_thread_cpu_get_history(cutmp))
			{
				vty_out(vty, "%s of cpu process:%s", module2name(cutmp->module),
						VTY_NEWLINE);
				vty_thread_cpu_show_detail(cutmp, vty, 1, filter);
			}
		}
	}
	return CMD_SUCCESS;
}

#define OS_THREAD_STR_TASK \
	"(zebra|rip|ripng|ospf|ospf6|isis|bgp|pim|olsr|frp)"
#define OS_THREAD_STR_TASK_HELP \
	"NSM process\n"             \
	"RIP process\n"             \
	"RIPng process\n"           \
	"OSPFv2 process\n"          \
	"OSPFv3 process\n"          \
	"IS-IS process\n"           \
	"BGP process\n"             \
	"PIM process\n"             \
	"OLSR process\n"            \
	"FRP process\n"

DEFUN(show_thread_task_cpu,
	  show_thread_task_cpu_cmd,
	  "show thread " OS_THREAD_STR_TASK "cpu [FILTER]",
	  SHOW_STR
	  "Thread information\n" OS_THREAD_STR_TASK_HELP
	  "Thread CPU usage\n"
	  "Display filter (rwtexb)\n")
{
	zpl_uint32 index = 0;
	thread_type filter = 0xff;

	struct thread_master *cutmp = NULL;
	struct thread_master *next = NULL;

	if (argc > 1)
	{
		zpl_char input[16];
		if (argc == 2)
		{
			filter = vty_thread_cpu_filter(vty, argv[1]);
			if (filter == 0)
				return CMD_WARNING;
		}
		os_memset(input, 0, sizeof(input));
		os_strcpy(input, argv[0]);
		index = name2module(input);

		for (cutmp = _master_thread_list.head; cutmp; cutmp = next)
		{
			next = cutmp->next;
			if (cutmp && cutmp->module == index)
			{
				if (vty_thread_cpu_get_history(cutmp))
				{
					vty_out(vty, "%s of cpu process:%s", module2name(index),
							VTY_NEWLINE);
					vty_thread_cpu_show_head(vty);
					vty_thread_cpu_show_detail(cutmp, vty, 1,
											   filter);
					return CMD_SUCCESS;
				}
			}
		}
		{
			vty_out(vty, "%s of cpu process: no thread use cpu%s",
					module2name(index),
					VTY_NEWLINE);
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
	thread_type filter = 0xff;

	struct thread_master *cutmp = NULL;
	struct thread_master *next = NULL;

	if (argc == 1)
	{
		filter = vty_thread_cpu_filter(vty, argv[0]);
		if (filter == 0)
			return CMD_WARNING;
	}
	for (cutmp = _master_thread_list.head; cutmp; cutmp = next)
	{
		next = cutmp->next;
		if (cutmp)
		{
			if (vty_thread_cpu_get_history(cutmp))
			{
				vty_clear_thread_cpu(cutmp, vty, filter);
			}
		}
	}
	return CMD_SUCCESS;
}

DEFUN(clear_thread_task_cpu,
	  clear_thread_task_cpu_cmd,
	  "clear thread " OS_THREAD_STR_TASK "cpu [FILTER]",
	  "Clear stored data\n"
	  "Thread information\n" OS_THREAD_STR_TASK_HELP
	  "Thread CPU usage\n"
	  "Display filter (rwtexb)\n")
{
	zpl_uint32 index = 0;
	thread_type filter = 0xff;

	struct thread_master *cutmp = NULL;
	struct thread_master *next = NULL;

	if (argc > 1)
	{

		zpl_char input[16];
		if (argc == 2)
		{
			filter = vty_thread_cpu_filter(vty, argv[1]);
			if (filter == 0)
				return CMD_WARNING;
		}
		os_memset(input, 0, sizeof(input));
		os_strcpy(input, argv[0]);
		index = name2module(input);

		for (cutmp = _master_thread_list.head; cutmp; cutmp = next)
		{
			next = cutmp->next;
			if (cutmp && cutmp->module == index)
			{
				if (vty_thread_cpu_get_history(cutmp))
				{
					vty_clear_thread_cpu(cutmp, vty, filter);
					return CMD_SUCCESS;
				}
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
		zpl_char type[24];
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
			// case THREAD_UNUSED:
			//	break;
		case THREAD_EXECUTE:
			sprintf(type, "%s", "execute");
			break;
		default:
			break;
		}
		vty_out(vty, "%-32s [%s]%s", lst->funcname, type, VTY_NEWLINE);
	}
	return OK;
}

static int cpu_thread_list_show(struct thread_list *m, struct vty *vty)
{
	struct thread *t = NULL;
	struct thread *thread = NULL;
	thread = m->head;
	while (thread)
	{
		if (thread)
		{
			cpu_thread_read_write_show(thread, vty);
		}
		t = thread;
		thread = t->next;
	}
	return OK;
}

static int cpu_thread_pqueue_show(struct pqueue *m, struct vty *vty)
{
	struct thread *thread;
	zpl_uint32 i = 0;
	for (i = 0; i < m->size; i++)
	{
		thread = m->array[i];
		if (thread)
			cpu_thread_read_write_show(thread, vty);
	}
	return OK;
}

static int cpu_thread_show(struct thread_master *m, struct vty *vty)
{
	cpu_thread_list_show(&m->read, vty);
	cpu_thread_list_show(&m->write, vty);
	cpu_thread_list_show(&m->event, vty);
	cpu_thread_pqueue_show(m->timer, vty);
	cpu_thread_pqueue_show(m->background, vty);
	cpu_thread_list_show(&m->ready, vty);
	return 0;
}

DEFUN(show_thread_dump,
	  show_thread_dump_cmd,
	  "show thread dump",
	  SHOW_STR
	  "system thread information\n"
	  "thread dump information\n")
{

	struct thread_master *cutmp = NULL;
	struct thread_master *next = NULL;

	for (cutmp = _master_thread_list.head; cutmp; cutmp = next)
	{
		next = cutmp->next;
		if (cutmp)
		{
			vty_out(vty, "%s of cpu process:%s", module2name(cutmp->module), VTY_NEWLINE);
			cpu_thread_show(cutmp, vty);
		}
	}

	return CMD_SUCCESS;
}
#endif

int cmd_os_thread_init(void)
{
	install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_thread_dump_cmd);
	install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_thread_cpu_cmd);
	install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_thread_task_cpu_cmd);

	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &clear_thread_cpu_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &clear_thread_task_cpu_cmd);

	return 0;
}

#endif