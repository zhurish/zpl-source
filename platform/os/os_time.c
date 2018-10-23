/*
 * os_time.c
 *
 *  Created on: May 30, 2017
 *      Author: zhurish
 */



#include "zebra.h"
//#include "vty.h"
#include "os_list.h"
//#include "os_log.h"
#include "os_sem.h"
#include "os_task.h"
#include "os_time.h"



static LIST *time_list = NULL;
static LIST *time_unuse_list = NULL;
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


struct timeval os_time_min(struct timeval a, struct timeval b)
{
	unsigned int ma = 0, mb = 0;
	ma = a.tv_sec * TIMER_SECOND_MICRO + a.tv_usec;
	mb = b.tv_sec * TIMER_SECOND_MICRO + b.tv_usec;

	if(ma > mb)
		return b;
	else
		return a;
}

struct timeval os_time_max(struct timeval a, struct timeval b)
{
	unsigned int ma = 0, mb = 0;
	ma = a.tv_sec * TIMER_SECOND_MICRO + a.tv_usec;
	mb = b.tv_sec * TIMER_SECOND_MICRO + b.tv_usec;

	if(ma > mb)
		return a;
	else
		return b;
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
  return time(t);
}


char *os_time_out (char *fmt, time_t t)
{
	int len = 0;
	struct tm *tm;
	static char data[128];
	time_t clock = t;
	os_memset(data, 0, sizeof(data));
	//UTC :Wed Apr 18 05:19:00 UTC 2018
	if(os_strstr(fmt, "bsd"))
	{
		tm = localtime(&clock);
		len = strftime(data, sizeof(data), "%b %e %T",tm);
	}
	else if(os_strstr(fmt, "/"))
	{
		tm = localtime(&clock);
		len = strftime(data, sizeof(data), "%Y/%m/%d %H:%M:%S",tm);
	}
	else if(os_strstr(fmt, "short"))
	{
		tm = localtime(&clock);
		len = strftime(data, sizeof(data), "%m/%d %H:%M:%S",tm);
	}
	else if(os_strstr(fmt, "iso"))
	{
		tm = gmtime(&clock);
		len = strftime(data, sizeof(data), "%Y-%m-%dT%H:%M:%S+08:00",tm);
	}
	else if(os_strstr(fmt, "rfc3164"))
	{
		tm = localtime(&clock);
		len = strftime(data, sizeof(data), "%b %d %T",tm);
	}
	else if(os_strstr(fmt, "rfc3339"))
	{
		tm = gmtime(&clock);
		len = strftime(data, sizeof(data), "%Y-%m-%dT%H:%M:%S-08:00",tm);
	}
	if(len > 0)
		return data;
	return "UNKNOWN";
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
		time_unuse_list = os_malloc(sizeof(LIST));
		if(time_unuse_list)
		{
			lstInit(time_unuse_list);
		}
		else
		{
			os_free(time_unuse_list);
			return ERROR;
		}

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
	if(time_unuse_list)
	{
		lstFree(time_unuse_list);
		time_unuse_list = NULL;
	}
	memset(&min_interval, 0, sizeof(struct timeval));
	return OK;
}

int os_time_load()
{
	if(time_task_id)
		return OK;
	return ERROR;
}

static os_time_t * os_time_get_node()
{
	NODE *node;
	os_time_t *t = NULL;
	if(time_mutex)
		os_mutex_lock(time_mutex, OS_WAIT_FOREVER);
	for(node = lstFirst(time_unuse_list); node != NULL; node = lstNext(node))
	{
		t = (os_time_t *)node;
		if(node && t->state == OS_TIMER_FALSE)
		{
			break;
		}
	}
	if(time_mutex)
		os_mutex_unlock(time_mutex);
	if(t)
		lstDelete (time_unuse_list, (NODE *)t);
	return t;
}

static os_time_t * os_time_entry_create(os_time_type type, int	(*time_entry)(void *),
		void *pVoid, int msec, char *func_name)
{
	os_time_t *t = os_time_get_node();
	if(t == NULL)
	{
		t = os_malloc(sizeof(os_time_t));
		if(t)
			os_memset(t, 0, sizeof(os_time_t));
	}
	if(t)
	{
		t->t_id = (int)t;
		t->msec = msec;
		t->pVoid = pVoid;
		t->time_entry = time_entry;
		t->type = type;
		os_memset(t->entry_name, 0, sizeof(t->entry_name));
		os_strcpy(t->entry_name, func_name);
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
	if(time_sem && signo == SIGALRM)
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
static int os_time_min_interval(struct timeval *a,struct timeval *b)
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

static int os_time_interval_refresh(os_time_t *t, int msec, BOOL add)
{
	struct timeval now;
	if(add == FALSE)
	{
		os_gettime (OS_CLK_REALTIME, &now);
		if(msec)
		{
			t->interval.tv_sec = now.tv_sec + msec/TIMER_MSEC_MICRO;
			t->interval.tv_usec = now.tv_usec + (msec%TIMER_MSEC_MICRO)*TIMER_MSEC_MICRO;
		}
		os_time_min_interval(&t->interval, &min_interval);
		os_time_interrupt_setting(&min_interval, &now);
	}
	else
	{
		os_gettime (OS_CLK_REALTIME, &now);
		t->interval.tv_sec = now.tv_sec + msec/TIMER_MSEC_MICRO;
		t->interval.tv_usec = now.tv_usec + (msec%TIMER_MSEC_MICRO)*TIMER_MSEC_MICRO;
		//if(t->node.previous == NULL)
		lstAdd(time_list, (NODE *)t);
		t->state = OS_TIMER_TRUE;
		if(min_interval.tv_sec == 0 && min_interval.tv_sec == min_interval.tv_usec)
			os_memcpy(&min_interval, &t->interval, sizeof(struct timeval));
		else
			os_time_min_interval(&t->interval, &min_interval);
		os_time_interrupt_setting(&min_interval, &now);
	}
	return OK;
}

#if 1
int os_time_create_entry(os_time_type type, int (*time_entry)(void *),
		void *pVoid, int msec, char *func_name)
{
	os_time_t * t = os_time_entry_create(type, time_entry, pVoid,  msec, func_name);
	if(t)
	{
		if(time_mutex)
			os_mutex_lock(time_mutex, OS_WAIT_FOREVER);
		os_time_interval_refresh(t,  msec, TRUE);
		if(time_mutex)
			os_mutex_unlock(time_mutex);
		return t->t_id;
	}
	return ERROR;
}
#else
static int os_time_create(int (*time_entry)(void *), void *pVoid, int msec)
{
	//int imsec = 0;
	//struct timeval now;
	//os_time_t *node;
	os_time_t * t = os_time_entry_create(time_entry, pVoid,  msec);
	if(t)
	{
		if(time_mutex)
			os_mutex_lock(time_mutex, OS_WAIT_FOREVER);
/*		os_gettime (OS_CLK_REALTIME, &now);
		t->interval.tv_sec = now.tv_sec + msec/TIMER_MSEC_MICRO;
		t->interval.tv_usec = now.tv_usec + (msec%TIMER_MSEC_MICRO)*TIMER_MSEC_MICRO;

		lstAdd(time_list, (NODE *)t);

		if(min_interval.tv_sec == 0 && min_interval.tv_sec == min_interval.tv_usec)
			os_memcpy(&min_interval, &t->interval, sizeof(struct timeval));
		else
			os_time_min(&t->interval, &min_interval);
		os_time_interrupt_setting(&min_interval, &now);*/
		os_time_interval_refresh(t,  msec, TRUE);
		if(time_mutex)
			os_mutex_unlock(time_mutex);
		return t->t_id;
	}
	return ERROR;
}
#endif


os_time_t *os_time_lookup(int id)
{
	NODE node;
	os_time_t *t;
	if(time_mutex)
		os_mutex_lock(time_mutex, OS_WAIT_FOREVER);
	for(t = (os_time_t *)lstFirst(time_list); t != NULL; t = (os_time_t *)lstNext(&node))
	{
		node = t->node;
		if(t && t->t_id && t->t_id == id && t->state != OS_TIMER_FALSE)
		{
			break;
		}
	}
	if(!t)
	{
		for(t = (os_time_t *)lstFirst(time_unuse_list); t != NULL; t = (os_time_t *)lstNext(&node))
		{
			node = t->node;
			if(t && t->t_id && t->t_id == id && t->state != OS_TIMER_FALSE)
			{
				break;
			}
		}
	}
	if(time_mutex)
		os_mutex_unlock(time_mutex);
	return t;
}
/*
static int os_time_entry_destroy(os_time_t *t)
{
	if(t)
		os_free(t);
	return OK;
}
*/


int os_time_destroy(int id)
{
	//struct timeval now;
	os_time_t *t = os_time_lookup(id);//(os_time_t *)id;
	if(t)
	{
		if(time_mutex)
			os_mutex_lock(time_mutex, OS_WAIT_FOREVER);
		t->state = OS_TIMER_FALSE;
		lstDelete (time_list, (NODE *)t);
		os_time_interval_refresh(t,  0, FALSE);
/*		os_gettime (OS_CLK_REALTIME, &now);
		os_time_min_interval(&t->interval, &min_interval);
		os_time_interrupt_setting(&min_interval, &now);*/
		t->t_id = 0;
		lstAdd(time_unuse_list, (NODE *)t);
		//os_time_entry_destroy(t);
		if(time_mutex)
			os_mutex_unlock(time_mutex);
		return OK;
	}
	return ERROR;
}

int os_time_cancel(int id)
{
	os_time_t *t = os_time_lookup(id);//(os_time_t *)id;
	if(t)
	{
		if(time_mutex)
			os_mutex_lock(time_mutex, OS_WAIT_FOREVER);
		t->state = OS_TIMER_CANCEL;
		lstDelete (time_list, (NODE *)t);
		lstAdd(time_unuse_list, (NODE *)t);
		os_time_interval_refresh(t,  0, FALSE);
		if(time_mutex)
			os_mutex_unlock(time_mutex);
		return OK;
	}
	return ERROR;
}

int os_time_restart(int id, int msec)
{
	os_time_t *t = os_time_lookup(id);//(os_time_t *)id;
	if(t && t->time_entry)
	{
		if(time_mutex)
			os_mutex_lock(time_mutex, OS_WAIT_FOREVER);
		t->state = OS_TIMER_TRUE;
		os_time_interval_refresh(t,  msec, FALSE);
		lstDelete(time_unuse_list, (NODE *)t);
		lstAdd (time_list, (NODE *)t);
		if(time_mutex)
			os_mutex_unlock(time_mutex);
		return OK;
	}
	return ERROR;
}


int os_timer_show(int (*show)(void *, char *fmt,...), void *pVoid)
{
	int i = 0;
	NODE *node;
	os_time_t *t;
	if (time_mutex)
		os_mutex_lock(time_mutex, OS_WAIT_FOREVER);
	if(lstCount(time_list) && show )
	{
		(show)(pVoid, "%-4s %-8s %-8s %s %s", "----", "--------", "----------", "----------------", "\r\n");
		(show)(pVoid, "%-4s %-8s %-8s %s %s", "ID", "TYPE", "INTERVAL", "NAME", "\r\n");
		(show)(pVoid, "%-4s %-8s %-8s %s %s", "----", "--------", "----------", "----------------", "\r\n");
	}
	for (node = lstFirst(time_list); node != NULL; node = lstNext(node))
	{
		t = (os_time_t *) node;
		if (node && show)
		{
			(show)(pVoid, "%-4d  %-8s %-8d %s%s", i++, (t->type == OS_TIMER_ONCE) ? "ONCE":"DEFAULT",
					t->msec, t->entry_name, "\r\n");
		}
	}
	if (time_mutex)
		os_mutex_unlock(time_mutex);
	return OK;
}

#if 1

static int os_time_task(void)
{
	int flag = 0;
	NODE node;
	os_time_t *t;
	struct timeval now;
	while(!os_load_config_done())
	{
		os_sleep(1);
	}
	while(1)
	{
		os_sem_take(time_sem, OS_WAIT_FOREVER);
		if(time_mutex)
			os_mutex_lock(time_mutex, OS_WAIT_FOREVER);

		os_gettime (OS_CLK_REALTIME, &now);
		for(t = (os_time_t *)lstFirst(time_list); t != NULL; t = (os_time_t *)lstNext(&node))
		{
			node = t->node;
			if(t && t->state != OS_TIMER_FALSE)
			{
				if(os_timeval_cmp (t->interval, now))
				{
					if(t->time_entry)
					{
						(t->time_entry)(t->pVoid);
					}
					if(t->type == OS_TIMER_ONCE)
					{
						t->msec = 0X0FFFFFFF;
						t->state = OS_TIMER_FALSE;
						t->t_id = 0;
						lstDelete (time_list, (NODE *)t);
						lstAdd(time_unuse_list, (NODE *)t);
					}
					os_time_interval_update(t);
					if(flag == 0)
					{
						flag = 1;
						os_memcpy(&min_interval, &t->interval, sizeof(struct timeval));
						//os_time_min_interval(&t->interval, &min_interval);
					}
					else
						os_time_min_interval(&t->interval, &min_interval);
				}
				else
					os_time_min_interval(&t->interval, &min_interval);
			}
		}
		flag = 0;
		os_time_interrupt_setting(&min_interval, &now);
		if(time_mutex)
			os_mutex_unlock(time_mutex);
	}
	return 0;
}
#else
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
		for(node = lstFirst(time_list); node != NULL; node = lstNext(node))
		{
			t = (os_time_t *)node;
			if(node && t->state)
			{
				if(os_timeval_cmp (t->interval, now))
				{
					if(t->time_entry)
					{
						(t->time_entry)(t->pVoid);
					}
					if(t->type == OS_TIMER_ONCE)
					{
						t->msec = 0X0FFFFFFF;
						t->state = OS_TIMER_FALSE;
						lstDelete (time_list, (NODE *)t);
						lstAdd(time_unuse_list, (NODE *)t);
					}
					os_time_interval_update(t);
					if(flag == 0)
					{
						flag = 1;
						os_memcpy(&min_interval, &t->interval, sizeof(struct timeval));
						//os_time_min_interval(&t->interval, &min_interval);
					}
					else
						os_time_min_interval(&t->interval, &min_interval);
				}
				else
					os_time_min_interval(&t->interval, &min_interval);
			}
		}
		flag = 0;
		os_time_interrupt_setting(&min_interval, &now);
		if(time_mutex)
			os_mutex_unlock(time_mutex);
	}
}
#endif
