/*
 * os_time.c
 *
 *  Created on: May 30, 2017
 *      Author: zhurish
 */



#include "zebra.h"
//#include "vty.h"
#include "os_list.h"
#include "log.h"
#include "os_sem.h"
#include "os_task.h"
#include "os_time.h"


#define MPLS_MIN_V		(60)
#define MPLS_HOU_V		(3600)
#define MPLS_DAY_V		(86400)


static LIST *time_list = NULL;
static LIST *time_unuse_list = NULL;
static os_sem_t *time_sem = NULL;
static os_mutex_t *time_mutex = NULL;
static struct timeval min_interval;//, min_interval_once;
static unit32 time_task_id = 0;
#ifdef OS_TIMER_POSIX
static timer_t os_timerid = 0;
#endif
static void os_time_interrupt(int signo);
static int os_time_task(void);
//static struct timeval os_time_msec;
/* Adjust so that tv_usec is in the range [0,TIMER_SECOND_MICRO).
   And change negative values to 0. */


int os_system_tick()
{
	struct timeval tv;
	if(os_gettime (OS_CLK_MONOTONIC, &tv) != OK)
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
os_get_realtime (struct timeval *tv)
{
	int ret;
	assert(tv);
	struct timespec tp;
	if (!(ret = clock_gettime(CLOCK_REALTIME, &tp)))
	{
		tv->tv_sec = tp.tv_sec;
		tv->tv_usec = tp.tv_nsec / 1000;
	}
	return ret;
}

int
os_get_monotonic (struct timeval *tv)
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
/*void
os_real_stabilised (struct timeval *tv)
{
  //*tv = relative_time_base;
  //tv->tv_sec += relative_time.tv_sec;
  //tv->tv_usec += relative_time.tv_usec;
  *tv = os_timeval_adjust (*tv);
}*/

/* Exported Quagga timestamp function.
 * Modelled on POSIX clock_gettime.
 *
 * clk_id : 检索和设置的clk_id指定的时钟时间。
CLOCK_REALTIME:系统实时时间,随系统实时时间改变而改变,即从UTC1970-1-1 0:0:0开始计时,
中间时刻如果系统时间被用户改成其他,则对应的时间相应改变
　　CLOCK_MONOTONIC:从系统启动这一刻起开始计时,不受系统时间被用户改变的影响
　　CLOCK_PROCESS_CPUTIME_ID:本进程到当前代码系统CPU花费的时间
　　CLOCK_THREAD_CPUTIME_ID:本线程到当前代码系统CPU花费的时间
 *
 */
int
os_gettime (enum os_clkid clkid, struct timeval *tv)
{
  switch (clkid)
    {
      case OS_CLK_REALTIME:
    	os_get_realtime (tv);
        os_timeval_adjust(*tv);
        return 0;
      case OS_CLK_MONOTONIC:
        os_get_monotonic (tv);
        os_timeval_adjust(*tv);
        return 0;
      case OS_CLK_REALTIME_STABILISED:
    	os_get_realtime (tv);
    	os_timeval_adjust (*tv);
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

time_t os_monotonic_time (void)
{
  struct timeval tv;
  os_get_monotonic (&tv);
  os_timeval_adjust(tv);
  return tv.tv_sec;
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

char *os_time_string(time_t tInput)
{
	static char tString[64];
	if(tInput)
	{
			int sec = 0, minu = 0, hou = 0, day = 0;
			time_t local_t;
			local_t = tInput;

			//if(local_t > MPLS_MIN_V)
			sec = (local_t) % MPLS_MIN_V;

			if(local_t > MPLS_MIN_V)
				minu = ((local_t) % MPLS_HOU_V)/MPLS_MIN_V;

			if(local_t > MPLS_HOU_V)
				hou = ((local_t) % MPLS_DAY_V)/MPLS_HOU_V ;

			if(local_t > MPLS_DAY_V)
				day = (local_t) / MPLS_DAY_V;

			memset(tString, 0, sizeof(tString));
			sprintf(tString, "%d days, %d hours, %d minutes, %d seconds", day, hou, minu, sec);
	}
	else
	{
		memset(tString, 0, sizeof(tString));
		sprintf(tString, "%s", "------");
	}
	return tString;
}



#ifdef OS_TIMER_POSIX
static int os_posix_timer_init()
{
	struct sigevent evp;
	memset(&evp, 0, sizeof(struct sigevent));
#define OS_POSIX_SIGNAL
#ifdef OS_POSIX_SIGNAL
	//evp.sigev_signo = SIGUSR2;
	evp.sigev_notify = SIGEV_SIGNAL;
#else
	evp.sigev_value.sival_int = 111; 				//也是标识定时器的，这和timerid有什么区别？回调函数可以获得
	evp.sigev_notify = SIGEV_THREAD; 				//线程通知的方式，派驻新线程
	evp.sigev_notify_function = os_time_interrupt;	//线程函数地址
#endif
	if (timer_create(CLOCK_MONOTONIC, &evp, &os_timerid) == -1)
	{
		return ERROR;
	}
#ifdef OS_POSIX_SIGNAL
//	os_register_signal(SIGUSR2, os_time_interrupt);
	os_register_signal(SIGALRM, os_time_interrupt);

	//signal(SIGUSR2, os_time_interrupt);
#endif
	return OK;
}
#endif




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
#ifdef OS_TIMER_POSIX
	os_posix_timer_init();
#endif
	time_task_id = os_task_create("timeTask", OS_TASK_DEFAULT_PRIORITY,
	               0, os_time_task, NULL, OS_TASK_DEFAULT_STACK);
	if(time_task_id)
		return OK;
	os_time_exit();
	return ERROR;
}


int os_time_exit()
{
#ifdef OS_TIMER_POSIX
	if(os_timerid)
		timer_delete(os_timerid);
#endif

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
		os_free(time_list);
		time_list = NULL;
	}
	if(time_unuse_list)
	{
		lstFree(time_unuse_list);
		os_free(time_unuse_list);
		time_unuse_list = NULL;
	}
	memset(&min_interval, 0, sizeof(struct timeval));
	//memset(&min_interval_once, 0, sizeof(struct timeval));
	return OK;
}

int os_time_load()
{
	if(time_task_id)
		return OK;
	return ERROR;
}

int os_time_clean(BOOL all)
{
	NODE node;
	os_time_t *t = NULL;
	if(time_mutex)
		os_mutex_lock(time_mutex, OS_WAIT_FOREVER);
	for(t = (os_time_t *)lstFirst(time_list); t != NULL; t = (os_time_t *)lstNext(&node))
	{
		node = t->node;
		if(t)
		{
			lstDelete (time_list, (NODE *)t);
			if(!all)
				lstAdd (time_unuse_list, (NODE *)t);
			else
				os_free(t);
		}
	}
	if(all)
	{
		for(t = (os_time_t *)lstFirst(time_list); t != NULL; t = (os_time_t *)lstNext(&node))
		{
			node = t->node;
			if(t)
			{
				lstDelete (time_unuse_list, (NODE *)t);
				os_free(t);
			}
		}
	}
	if(time_mutex)
		os_mutex_unlock(time_mutex);
	return t;
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
	if(t)
		lstDelete (time_unuse_list, (NODE *)t);
	if(time_mutex)
		os_mutex_unlock(time_mutex);
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
		os_memset(t, 0, sizeof(os_time_t));
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
#ifdef OS_TIMER_POSIX
	if(time_sem && signo == SIGUSR2)
		os_sem_give(time_sem);
#else
	if(time_sem && signo == SIGALRM)
		os_sem_give(time_sem);
#endif
	fprintf(stdout,"%s:\r\n",__func__);
	//OS_DEBUG("%s:\r\n",__func__);
	return;
}

static int os_time_interrupt_setting(struct timeval *interval, struct timeval *now)
{
#ifdef OS_TIMER_POSIX
	struct itimerspec tick;
	//struct itimerval tick;
	struct timeval it_value;
	memset(&tick, 0, sizeof(tick));
	it_value = os_timeval_subtract (*interval, *now);

	tick.it_value.tv_sec = it_value.tv_sec;
	tick.it_value.tv_nsec = it_value.tv_usec * 1000;
	if(tick.it_value.tv_sec == 0 && tick.it_value.tv_nsec <= TIMER_MSEC(1))
	{
		min_interval.tv_sec = tick.it_value.tv_sec = now->tv_sec + OS_TIMER_FOREVER;
	}
#ifdef OS_TIMER_TEST
	fprintf(stdout, "%s time=%u.%u\n", __func__, tick.it_value.tv_sec,
			(tick.it_value.tv_nsec)/1000000);
	//zlog_debug(ZLOG_NSM, "%s time=%u.%u", __func__,tick.it_value.tv_sec, tick.it_value.tv_usec/1000);
#endif
//	OS_DEBUG("%s:%d.%d sec\r\n",__func__, tick.it_value.tv_sec, tick.it_value.tv_usec/TIMER_MSEC_MICRO);
	//After first, the Interval time for clock
	tick.it_interval.tv_sec = 0;
	tick.it_interval.tv_nsec = 0;
	if(os_timerid)
		timer_settime(os_timerid, 0, &tick, NULL);
	return OK;
#else
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
	if(tick.it_value.tv_sec == 0 && tick.it_value.tv_usec <= TIMER_MSEC(1))
	{
		min_interval.tv_sec = tick.it_value.tv_sec = now->tv_sec + OS_TIMER_FOREVER;
	}
#ifdef OS_TIMER_TEST
	fprintf(stdout, "%s time=%u.%u\n", __func__, tick.it_value.tv_sec,
			(tick.it_value.tv_usec)/1000);
	//zlog_debug(ZLOG_NSM, "%s time=%u.%u", __func__,tick.it_value.tv_sec, tick.it_value.tv_usec/1000);
#endif
//	OS_DEBUG("%s:%d.%d sec\r\n",__func__, tick.it_value.tv_sec, tick.it_value.tv_usec/TIMER_MSEC_MICRO);
	//After first, the Interval time for clock
	tick.it_interval.tv_sec = 0;
	tick.it_interval.tv_usec = 0;

	return setitimer(ITIMER_REAL, &tick, NULL);
#endif
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
		os_gettime (OS_CLK_MONOTONIC, &now);
		if(msec == OS_TIMER_FOREVER)
		{
			NODE node;
			os_time_t *to = NULL;
			for(to = (os_time_t *)lstFirst(time_list); to != NULL; to = (os_time_t *)lstNext(&node))
			{
				node = to->node;
				if(to && to->state == OS_TIMER_TRUE)
				{
					os_time_min_interval(&to->interval, &min_interval);
				}
			}
			t->interval.tv_sec = now.tv_sec + OS_TIMER_FOREVER;
/*			if(os_timeval_cmp (min_interval, t->interval) == 0)
			{
				t->interval.tv_sec = now.tv_sec + OS_TIMER_FOREVER;
				//min_interval.tv_sec = now.tv_sec + OS_TIMER_FOREVER;
				//os_time_min_interval(&t->interval, &min_interval);
			}
			else
			{
				os_time_min_interval(&t->interval, &min_interval);
			}*/
		}
		else //if(msec)
		{
			t->interval.tv_sec = now.tv_sec + msec/TIMER_MSEC_MICRO;
			t->interval.tv_usec = now.tv_usec + (msec%TIMER_MSEC_MICRO)*TIMER_MSEC_MICRO;
		}
		if(t->state == OS_TIMER_TRUE)
			os_time_min_interval(&t->interval, &min_interval);
		os_time_interrupt_setting(&min_interval, &now);
	}
	else
	{
		os_gettime (OS_CLK_MONOTONIC, &now);
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
/*		os_gettime (OS_CLK_MONOTONIC, &now);
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
	os_time_t *t = NULL;
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
		os_time_interval_refresh(t,  OS_TIMER_FOREVER, FALSE);
/*		os_gettime (OS_CLK_MONOTONIC, &now);
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
		os_time_interval_refresh(t,  OS_TIMER_FOREVER, FALSE);
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


int os_time_show(int (*show)(void *, char *fmt,...), void *pVoid)
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

		os_gettime (OS_CLK_MONOTONIC, &now);
		for(t = (os_time_t *)lstFirst(time_list); t != NULL; t = (os_time_t *)lstNext(&node))
		{
			node = t->node;
			if(t && t->state == OS_TIMER_TRUE)
			{
				if(os_timeval_cmp (now, t->interval))
				{
					if(t->time_entry)
					{
						(t->time_entry)(t->pVoid);
					}
					if(t->type == OS_TIMER_ONCE)
					{
						t->msec = OS_TIMER_FOREVER;
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
		os_gettime (OS_CLK_MONOTONIC, &now);
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

/*
 * 20181024160232  -> 2018/10/24 16:02:32
 */
const char *os_build_time2date(char *str)
{
	int i = 0, j = 0;
	static char buf[64];
	char *p = str;
	memset(buf, 0, sizeof(buf));
	while (*p != '\0')
	{
		if (j==4 || j==6)
		{
			buf[i++] = '/';
			buf[i++] = *p;
		}
		else if (j==10 || j==12)
		{
			buf[i++] = ':';
			buf[i++] = *p;
		}
		else if (j==8)
		{
			buf[i++] = ' ';
			buf[i++] = *p;
		}
		else
			buf[i++] = *p;
		p++;
		j++;
	}
	return buf;
}

/*
 * 2018/10/24 16:02:32 -> 20181024160232
 */
const char *os_date2build_time(char *str)
{
	int i = 0;
	static char buf[64];
	char *p = str;
	memset(buf, 0, sizeof(buf));
	while (*p != '\0')
	{
		if (*p == '/' || *p <= ':'  || *p <= ' ')
			;
		else
			buf[i++] = *p;
		p++;
	}
	return buf;
}

#ifdef OS_TIMER_TEST

static u_int32 t_time_test = 0, t_time_test1 = 0;
static int timer_test_handle(void *p)
{
	struct timeval now;
	os_gettime (OS_CLK_MONOTONIC, &now);
	fprintf(stdout, "%s time=%u.%u msec\n", __func__,now.tv_sec, now.tv_sec/1000);
	//zlog_debug(ZLOG_NSM, "%s time=%u.%u msec", __func__,now.tv_sec, now.tv_sec/1000);
	t_time_test = 0;
	return OK;
}

int timer_test(int time, int type)
{
	static char clean_flag = 0;
	struct timeval now;
	os_gettime (OS_CLK_MONOTONIC, &now);
	if(clean_flag == 0)
	{
		os_time_clean(FALSE);
		clean_flag = 1;
	}
	if(type)
	{
		if(t_time_test)
		{
			if(os_time_lookup(t_time_test))
			{
				os_time_cancel(t_time_test);
				os_time_restart(t_time_test, time);
				fprintf(stdout, "%s time=%u.%u msec\n", __func__,now.tv_sec, now.tv_sec/1000);
				return OK;
			}
		}
		fprintf(stdout, "%s time=%u.%u msec\n", __func__,now.tv_sec, now.tv_sec/1000);
		t_time_test = os_time_create_once(timer_test_handle, NULL, time);
	}
	else
	{
		fprintf(stdout, "%s time=%u.%u msec\n", __func__,now.tv_sec, now.tv_sec/1000);
		t_time_test1 = os_time_create(timer_test_handle, NULL, time);
	}
	return OK;
}

int timer_test_exit(int type)
{
	if(type)
	{
		if(t_time_test)
		{
			if(os_time_lookup(t_time_test))
				os_time_destroy(t_time_test);
			t_time_test = 0;
		}
	}
	else
	{
		if(t_time_test1)
		{
			if(os_time_lookup(t_time_test1))
				os_time_destroy(t_time_test1);
			t_time_test1 = 0;
		}
	}
	return OK;
}
#endif

/*
*** Error in `/home/test/SWP-V0.0.1.bin': corrupted size vs. prev_size: 0x717032b0 ***

Thread 2 "timeTask" received signal SIGABRT, Aborted.
[Switching to LWP 2611]
0x77ac1f80 in raise () from /lib/libc.so.6
(gdb)
(gdb) bt
#0  0x77ac1f80 in raise () from /lib/libc.so.6
#1  0x77ac3884 in abort () from /lib/libc.so.6
 */
/*
#define MPLS_MIN_V		(60)
#define MPLS_HOU_V		(3600)
#define MPLS_DAY_V		(86400)
char * mpls_get_file_size(const char *path,  u_long  *ulFilesize )
{
	static char sizeString[64];
	unsigned long filesize = 0;
	struct stat statbuff;
	if(stat(path, &statbuff) < 0)
	{
		if(ulFilesize)
			*ulFilesize = filesize;
		sprintf(sizeString, "%s", "0");
	}
	else
	{
		int size1 = 0;
		if(ulFilesize)
			*ulFilesize = statbuff.st_size;
		memset(sizeString, 0, sizeof(sizeString));
		filesize = statbuff.st_size;
		if(MPLS_X_M(filesize))
		{
			size1 = filesize % MPLS_1_M(1);
			sprintf(sizeString, "%d.%dM", (int)MPLS_X_M(filesize), (int)(MPLS_X_K(size1) + 99)/100);
		}
		else if(MPLS_X_K(filesize))
		{
			size1 = filesize % MPLS_1_K(1);
			sprintf(sizeString, "%d.%dK", (int)MPLS_X_K(filesize), (int)(MPLS_X_B(size1) + 99)/100);
		}
		else
		{
			sprintf(sizeString, "%dB", (int)MPLS_X_B(filesize));
		}
	}
	return sizeString;
} */
