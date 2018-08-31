/*
 * os_time.c
 *
 *  Created on: May 30, 2017
 *      Author: zhurish
 */



#include "zebra.h"

#include "os_list.h"
//#include "os_log.h"
#include "os_sem.h"
#include "os_task.h"
#include "os_time.h"



static LIST *time_list = NULL;
static os_sem_t *time_sem = NULL;
static os_mutex_t *time_mutex = NULL;
static struct timeval min_interval;
static unit32 time_task_id = 0;
static int os_time_task(void);
//static struct timeval os_time_msec;
/* Adjust so that tv_usec is in the range [0,TIMER_SECOND_MICRO).
   And change negative values to 0. */


int os_system_tick()
{
	struct timeval tv;
	if(os_gettime (OS_CLK_REALTIME, &tv) != OK)
	{
		os_gettimeofday(&tv);
	}
	return tv.tv_usec / 1000 + tv.tv_sec * 1000;
}

int os_msleep(unsigned int ms)
{
	return usleep(ms*1000);
}

int os_sleep(unsigned int s)
{
	return sleep(s);
}

struct timeval
os_timeval_adjust (struct timeval a)
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

struct timeval
os_timeval_subtract (struct timeval a, struct timeval b)
{
  struct timeval ret;

  ret.tv_usec = a.tv_usec - b.tv_usec;
  ret.tv_sec = a.tv_sec - b.tv_sec;

  return os_timeval_adjust (ret);
}

int
os_timeval_cmp (struct timeval a, struct timeval b)
{
  return (a.tv_sec == b.tv_sec
	  ? a.tv_usec - b.tv_usec : a.tv_sec - b.tv_sec);
}

unsigned int
os_timeval_elapsed (struct timeval a, struct timeval b)
{
  return (((a.tv_sec - b.tv_sec) * TIMER_SECOND_MICRO)
	  + (a.tv_usec - b.tv_usec));
}

int
os_gettimeofday (struct timeval *tv)
{
  assert (tv);
  return gettimeofday (tv, NULL);

}

int
os_get_relative (struct timeval *tv)
{
  int ret;
  assert (tv);
#ifdef HAVE_CLOCK_MONOTONIC
  {
    struct timespec tp;
    if (!(ret = clock_gettime (CLOCK_MONOTONIC, &tp)))
      {
    	tv->tv_sec = tp.tv_sec;
    	tv->tv_usec = tp.tv_nsec / 1000;
      }
  }
#else /* !HAVE_CLOCK_MONOTONIC && !__APPLE__ */
  ret = os_gettimeofday (tv);
#endif /* HAVE_CLOCK_MONOTONIC */
  return ret;
}

/* Get absolute time stamp, but in terms of the internal timer
 * Could be wrong, but at least won't go back.
 */
void
os_real_stabilised (struct timeval *tv)
{
  //*tv = relative_time_base;
  //tv->tv_sec += relative_time.tv_sec;
  //tv->tv_usec += relative_time.tv_usec;
  *tv = os_timeval_adjust (*tv);
}

/* Exported Quagga timestamp function.
 * Modelled on POSIX clock_gettime.
 */
int
os_gettime (enum os_clkid clkid, struct timeval *tv)
{
  switch (clkid)
    {
      case OS_CLK_REALTIME:
        return os_gettimeofday (tv);
      case OS_CLK_MONOTONIC:
        return os_get_relative (tv);
      case OS_CLK_REALTIME_STABILISED:
        os_real_stabilised (tv);
        return 0;
      default:
        errno = EINVAL;
        return -1;
    }
}

/* time_t value in terms of stabilised absolute time.
 * replacement for POSIX time()
 */
time_t
os_time (time_t *t)
{
/*  struct timeval tv;
  os_gettime(OS_CLK_REALTIME, &tv);
  os_real_stabilised (&tv);
  if (t)
    *t = tv.tv_sec;
  return tv.tv_sec;*/
  return time(NULL);
}


int os_time_init()
{
	if(time_list == NULL)
	{
		time_list = os_malloc(sizeof(LIST));
		if(time_list)
		{
			lstInit(time_list);
			time_sem = os_sem_init();
		}
		else
			return ERROR;
	}
	if(time_mutex == NULL)
	{
		time_mutex = os_mutex_init();
	}
	time_task_id = os_task_create("timeTask", OS_TASK_DEFAULT_PRIORITY,
	               0, os_time_task, NULL, OS_TASK_DEFAULT_STACK);
	if(time_task_id)
		return OK;
	os_time_exit();
	return ERROR;
}


int os_time_exit()
{
	if(time_task_id)
	{
		if(os_task_destroy(time_task_id)==OK)
			time_task_id = 0;
	}
	if(time_mutex)
	{
		if(os_mutex_exit(time_mutex)==OK)
			time_mutex = NULL;
	}
	if(time_sem)
	{
		if(os_sem_exit(time_sem)==OK)
			time_sem = NULL;
	}
	if(time_list)
	{
		lstFree(time_list);
		time_list = NULL;
	}
	memset(&min_interval, 0, sizeof(struct timeval));
	return OK;
}

int os_time_finsh()
{
	if(time_task_id)
		return OK;
	return ERROR;
}

static os_time_t * os_time_entry_create(int	(*time_entry)(void *), void *pVoid, int msec)
{
	os_time_t *t = os_malloc(sizeof(os_time_t));
	if(t)
	{
		t->t_id = (int)t;
		t->msec = msec;
		t->pVoid = pVoid;
		t->time_entry = time_entry;
		//t->interval
		return t;
	}
	return NULL;
}


static int os_time_interval_update(os_time_t *t)
{
	t->interval.tv_sec += t->msec/TIMER_MSEC_MICRO;
	t->interval.tv_usec += (t->msec%TIMER_MSEC_MICRO)*TIMER_MSEC_MICRO;
	return OK;
}



static void os_time_interrupt(int signo)
{
	if(time_sem)
		os_sem_give(time_sem);
//	OS_DEBUG("%s:\r\n",__func__);
	return;
}

static int os_time_interrupt_setting(struct timeval *interval, struct timeval *now)
{
	struct itimerval tick;
	struct timeval it_value;
	signal(SIGALRM, os_time_interrupt);
	memset(&tick, 0, sizeof(tick));
	it_value = os_timeval_subtract (*interval, *now);
	//Timeout to run first time
#if 1
	tick.it_value.tv_sec = it_value.tv_sec;
	tick.it_value.tv_usec = it_value.tv_usec;
#else
	tick.it_value.tv_sec = msec/TIMER_MSEC_MICRO;//interval->tv_sec;
	tick.it_value.tv_usec = (msec%TIMER_MSEC_MICRO)*TIMER_MSEC_MICRO;//interval->tv_usec;
#endif
//	OS_DEBUG("%s:%d.%d sec\r\n",__func__, tick.it_value.tv_sec, tick.it_value.tv_usec/TIMER_MSEC_MICRO);
	//After first, the Interval time for clock
	tick.it_interval.tv_sec = 0;
	tick.it_interval.tv_usec = 0;

	return setitimer(ITIMER_REAL, &tick, NULL);
}

/*
 * get min timeval
 */
static int os_time_min(struct timeval *a,struct timeval *b)
{
	min_interval.tv_sec = min(a->tv_sec, b->tv_sec);
	if(a->tv_sec > b->tv_sec)
	{
		min_interval.tv_sec = b->tv_sec;
		min_interval.tv_usec = b->tv_usec;
	}
	else if(a->tv_sec == b->tv_sec)
	{
		min_interval.tv_usec = min(a->tv_usec, b->tv_usec);
	}
	else if(a->tv_sec < b->tv_sec)
	{
		min_interval.tv_sec = a->tv_sec;
		min_interval.tv_usec = a->tv_usec;
	}
	return 0;
}


int os_time_create(int (*time_entry)(void *), void *pVoid, int msec)
{
	//int imsec = 0;
	struct timeval now;
	//os_time_t *node;
	os_time_t * t = os_time_entry_create(time_entry, pVoid,  msec);
	if(t)
	{
		if(time_mutex)
			os_mutex_lock(time_mutex, OS_WAIT_FOREVER);
		os_gettime (OS_CLK_REALTIME, &now);
		t->interval.tv_sec = now.tv_sec + msec/TIMER_MSEC_MICRO;
		t->interval.tv_usec = now.tv_usec + (msec%TIMER_MSEC_MICRO)*TIMER_MSEC_MICRO;

		lstAdd(time_list, (NODE *)t);
		//lstInsert (pList, pList->TAIL, pNode);
		if(min_interval.tv_sec == 0 && min_interval.tv_sec == min_interval.tv_usec)
			os_memcpy(&min_interval, &t->interval, sizeof(struct timeval));
		else
			os_time_min(&t->interval, &min_interval);
		os_time_interrupt_setting(&min_interval, &now);
		if(time_mutex)
			os_mutex_unlock(time_mutex);
		return t->t_id;
	}
	return ERROR;
}

static int os_time_entry_destroy(os_time_t *t)
{
	if(t)
		os_free(t);
	return OK;
}


int os_time_destroy(int id)
{
	struct timeval now;
	os_time_t *t = (os_time_t *)id;
	if(t)
	{
		if(time_mutex)
			os_mutex_lock(time_mutex, OS_WAIT_FOREVER);
		os_gettime (OS_CLK_REALTIME, &now);
		os_time_min(&t->interval, &min_interval);
		os_time_interrupt_setting(&min_interval, &now);
		os_time_entry_destroy(t);
		if(time_mutex)
			os_mutex_unlock(time_mutex);
	}
	return OK;
}

static int os_time_task(void)
{
	int flag = 0;
	NODE *node;
	os_time_t *t;
	struct timeval now;
	while(1)
	{
		os_sem_take(time_sem, OS_WAIT_FOREVER);
		//printf("%s: time_sem = %s\r\n", __func__,time_sem ? "full":"none");
		if(time_mutex)
			os_mutex_lock(time_mutex, OS_WAIT_FOREVER);
		os_gettime (OS_CLK_REALTIME, &now);
		for(node = lstFirst(time_list); node; node = lstNext(node))
		{
			if(node)
			{
				t = (os_time_t *)node;
				if(os_timeval_cmp (t->interval, now))
				{
					if(t->time_entry)
					{
						(t->time_entry)(t->pVoid);
					}
					os_time_interval_update(t);
					if(flag == 0)
					{
						flag = 1;
						os_memcpy(&min_interval, &t->interval, sizeof(struct timeval));
						//os_time_min(&t->interval, &min_interval);
					}
					else
						os_time_min(&t->interval, &min_interval);

				}
				else
						os_time_min(&t->interval, &min_interval);
			}
		}
		flag = 0;
		os_time_interrupt_setting(&min_interval, &now);
		if(time_mutex)
			os_mutex_unlock(time_mutex);
	}
}
