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




static LIST *time_list = NULL;
static LIST *time_unuse_list = NULL;

static os_sem_t *time_sem = NULL;
static os_mutex_t *time_mutex = NULL;

#ifdef OS_TIMER_POSIX
static os_time_t *current_time = NULL;
#else
static int	inter_value = 10;
#endif
static unit32 time_task_id = 0;

#ifdef OS_TIMER_POSIX
static timer_t os_timerid = 0;
static void os_time_interrupt(int signo);
#endif
static int os_time_task(void);


int os_system_time_base(char *dt)
{
	//int sysb = os_time(NULL);
	//if(sysb < 1546272000 || sysb >= 1577808000)//2019-01-01 00:00:00 < x < 2020-01-01 00:00:00
	{
		char dtcmd[128];
		memset(dtcmd, 0, sizeof(dtcmd));
		snprintf(dtcmd, sizeof(dtcmd), "date -s\"%s\"", dt);
		//super_system(dtcmd);
	}
	return OK;
}

int os_system_tick()
{
	struct timeval tv;
	if(os_gettime (OS_CLK_MONOTONIC, &tv) != OK)
	{
		os_gettimeofday(&tv);
	}
	return tv.tv_usec / 1000 + tv.tv_sec * 1000;
}


static void _os_usleep_interrupt (int useconds)
{
	struct timeval delay;
	int sec = 0;
	sec = (int) useconds / 1000000;
	if (sec > 0)
	{
		delay.tv_sec = sec;
		delay.tv_usec = (useconds % 1000000);
	}
	else
	{
		delay.tv_sec = 0;
		delay.tv_usec = useconds;
	}
	select(0, 0, 0, 0, &delay);
}

void os_usleep_interrupt (int useconds)
{
	_os_usleep_interrupt (useconds);
}

void os_msleep_interrupt (int mseconds)
{
	_os_usleep_interrupt (mseconds * 1000);
}

int os_usleep(unsigned int us)
{
	return usleep(us);
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

int os_get_monotonic_msec ()
{
	struct timeval tv;
	os_get_monotonic (&tv);
	return TIMER_MSEC(tv.tv_sec) + tv.tv_usec/TIMER_MSEC_MICRO;
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
 *
unsigned long long FAST_FUNC monotonic_ns(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000000ULL + tv.tv_usec * 1000;
}
unsigned long long FAST_FUNC monotonic_us(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000ULL + tv.tv_usec;
}
unsigned long long FAST_FUNC monotonic_ms(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000ULL + tv.tv_usec / 1000;
}
unsigned FAST_FUNC monotonic_sec(void)
{
	return time(NULL);
}
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
/*
 *      option start_date '2019-04-09T00:00'
        option stop_date '2019-04-11T00:00'
 */
#if 0
struct tm {
   int tm_sec;         /* 秒，范围从 0 到 59        */
   int tm_min;         /* 分，范围从 0 到 59        */
   int tm_hour;        /* 小时，范围从 0 到 23        */
   int tm_mday;        /* 一月中的第几天，范围从 1 到 31    */
   int tm_mon;         /* 月，范围从 0 到 11        */
   int tm_year;        /* 自 1900 年起的年数        */
   int tm_wday;        /* 一周中的第几天，范围从 0 到 6    */
   int tm_yday;        /* 一年中的第几天，范围从 0 到 365    */
   int tm_isdst;       /* 夏令时                */
};
#endif


u_int32 os_timestamp_spilt(time_t t,  char *input)
{
	int i = 0, b = 0;
	struct tm tm_tmp;
	time_t time_tmp = 0;
	if(!input || !strlen(input))
	{
		return os_time(NULL);
	}
	char *brk = strstr(input, ":");
	if(t == 0)
		time_tmp = os_time(NULL);
	else
		time_tmp = t;
	localtime_r(&time_tmp, &tm_tmp);
	tm_tmp.tm_sec = 0;
	tm_tmp.tm_year = 0;
	memset(&tm_tmp, 0, sizeof(struct tm));
	brk = input;
	while(i < strlen(brk))
	{
		if(brk[i] == 'T')
			brk[i] = ' ';
		if(brk[i] == ':')
			b++;
		i++;
	}
	if(b == 2)
	{
		sscanf(input, "%d-%d-%d %d:%d:%d", &tm_tmp.tm_year, &tm_tmp.tm_mon,
				&tm_tmp.tm_mday, &tm_tmp.tm_hour, &tm_tmp.tm_min, &tm_tmp.tm_sec);
	}
	else if(b == 1)
	{
		sscanf(input, "%d-%d-%d %d:%d", &tm_tmp.tm_year, &tm_tmp.tm_mon,
				&tm_tmp.tm_mday, &tm_tmp.tm_hour, &tm_tmp.tm_min);
	}
	tm_tmp.tm_mon -= 1;
	tm_tmp.tm_year -= 1900;
	time_tmp = mktime(&tm_tmp);
	return (u_int32)time_tmp;
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
/*
option start_date '2019-04-09T00:00'
option stop_date '2019-04-11T00:00'
*/
/*time_t os_time_scanf (char *fmt, time_t t)
{

}*/

char *os_time_fmt (char *fmt, time_t t)
{
	int len = 0;
	struct tm tm;
	struct tm *ptm = NULL;
	static char data[128];
	time_t clock = t;
	os_memset(data, 0, sizeof(data));
	os_memset(&tm, 0, sizeof(tm));
	//UTC :Wed Apr 18 05:19:00 UTC 2018
	if(os_strstr(fmt, "bsd"))
	{
		localtime_r(&clock, &tm);
		len = strftime(data, sizeof(data), "%b %e %T", &tm);
	}
	else if(os_strstr(fmt, "/")||os_strstr(fmt, "date"))
	{
		localtime_r(&clock, &tm);
		len = strftime(data, sizeof(data), "%Y/%m/%d %H:%M:%S", &tm);
	}
	else if(os_strstr(fmt, "short"))
	{
		localtime_r(&clock, &tm);
		len = strftime(data, sizeof(data), "%m/%d %H:%M:%S", &tm);
	}
	else if(os_strstr(fmt, "iso"))
	{
		ptm = gmtime(&clock);
		//len = strftime(data, sizeof(data), "%Y-%m-%dT%H:%M:%S+08:00",tm);
		len = strftime(data, sizeof(data), "%Y-%m-%dT%H:%M:%S", ptm);
	}
	else if(os_strstr(fmt, "rfc3164"))
	{
		localtime_r(&clock, &tm);
		len = strftime(data, sizeof(data), "%b %d %T", &tm);
	}
	else if(os_strstr(fmt, "rfc3339"))
	{
		ptm = gmtime(&clock);
		len = strftime(data, sizeof(data), "%Y-%m-%dT%H:%M:%S", ptm);
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

			//if(local_t > OS_SEC_MIN_V(1))
			sec = (local_t) % OS_SEC_MIN_V(1);

			if(local_t > OS_SEC_MIN_V(1))
				minu = ((local_t) % OS_SEC_HOU_V(1))/OS_SEC_MIN_V(1);

			if(local_t > OS_SEC_HOU_V(1))
				hou = ((local_t) % OS_SEC_DAY_V(1))/OS_SEC_HOU_V(1) ;

			if(local_t > OS_SEC_DAY_V(1))
				day = (local_t) / OS_SEC_DAY_V(1);

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








/*
 * os time module
 */
#ifdef OS_TIMER_TEST
static int os_timer_timeval (struct timeval *tv)
{
  os_get_monotonic (tv);
  os_timeval_adjust(*tv);
  return 0;
}
#endif

static int os_time_compar(os_time_t *new, os_time_t *old)
{
	//lstAddSort
	//if(os_timeval_cmp(new->interval, old->interval) < 0)
	if(new->interrupt_timestamp < old->interrupt_timestamp)
		return -1;
	else
		return 1;
}

int os_time_init()
{
	os_system_time_base("2019-01-01 00:00:00");
	if(time_list == NULL)
	{
		time_list = os_malloc(sizeof(LIST));
		if(time_list)
		{
			lstInit(time_list);
			lstSortInit(time_list, os_time_compar);
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
		for(t = (os_time_t *)lstFirst(time_unuse_list); t != NULL; t = (os_time_t *)lstNext(&node))
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
		return t;
	}
	return NULL;
}


/*
 *设置定时中断点
 */
#ifdef OS_TIMER_POSIX
static void os_time_interrupt(int signo)
{
	if(time_sem && signo == SIGUSR2)
		os_sem_give(time_sem);
	//fprintf(stdout,"%s:\r\n",__func__);
	current_time = NULL;
	//OS_DEBUG("%s:\r\n",__func__);
	return;
}

static int timer_connect(timer_t timerid, int (*routine)(void *p))
{
	struct sigevent evp;
	memset(&evp, 0, sizeof(struct sigevent));

	evp.sigev_signo = SIGUSR2;
	evp.sigev_notify = SIGEV_SIGNAL;

	if (timer_create(CLOCK_MONOTONIC, &evp, &os_timerid) == -1)
	{
		return ERROR;
	}
	os_register_signal(SIGUSR2, os_time_interrupt);
//	os_register_signal(SIGALRM, os_time_interrupt);

	//signal(SIGUSR2, os_time_interrupt);
	return OK;
}
#endif /* OS_TIMER_POSIX */

static int os_time_interval_update(os_time_t *t)
{
//#ifdef OS_TIMER_POSIX
	t->interrupt_timestamp = os_get_monotonic_msec () + t->msec;
/*	t->interval.tv_sec += t->msec/TIMER_MSEC_MICRO;
	t->interval.tv_usec += (t->msec%TIMER_MSEC_MICRO)*TIMER_MSEC_MICRO;
	os_timeval_adjust(t->interval);*/
//#endif
	return OK;
}

static u_long os_time_interrupt_interval_get(os_time_t *t)
{
	return (t->interrupt_timestamp - os_get_monotonic_msec ());
/*	struct timeval current_tv;
	os_timer_timeval (&current_tv);
	current_tv = os_timeval_subtract (t->interval, current_tv);
	return (((current_tv.tv_sec) * TIMER_SECOND_MICRO)
		  + (current_tv.tv_usec))/TIMER_MSEC_MICRO;*/
}

static int os_time_interrupt_setting(int msec)
{
	struct itimerspec tick;
	memset(&tick, 0, sizeof(tick));

	tick.it_value.tv_sec = msec / 1000;
	tick.it_value.tv_nsec = (msec % 1000) * 1000 * 1000;

	tick.it_interval.tv_sec = 0;
	tick.it_interval.tv_nsec = 0;


	if(tick.it_value.tv_sec == 0 && tick.it_value.tv_nsec <= TIMER_MSEC(1))
	{
		tick.it_value.tv_nsec = TIMER_MSEC(1) * 1000000;
	}
#ifdef OS_TIMER_TEST
	//fprintf(stdout, "%s time=%u.%u\n", __func__, tick.it_value.tv_sec,
	//		(tick.it_value.tv_nsec)/1000000);
	//zlog_debug(ZLOG_NSM, "%s time=%u.%u", __func__,tick.it_value.tv_sec, tick.it_value.tv_usec/1000);
#endif
	timer_connect(0, NULL);
//	OS_DEBUG("%s:%d.%d sec\r\n",__func__, tick.it_value.tv_sec, tick.it_value.tv_usec/TIMER_MSEC_MICRO);
	//After first, the Interval time for clock

	if(os_timerid)
		timer_settime(os_timerid, 0, &tick, NULL);
	return OK;
}


static int os_time_interval_refresh()
{
#ifdef OS_TIMER_POSIX
	os_time_t *to = NULL;
	to = (os_time_t *)lstFirst(time_list);
	//if(to == current_time)
	//	return OK;
	//else
	{
		current_time = to;
		//struct timeval now;
		//os_timer_timeval(&now);
		if(current_time)
		{
			u_long inter_time = os_time_interrupt_interval_get(current_time);
			if(inter_time > 0)
				os_time_interrupt_setting(inter_time/*current_time->msec*/);
			else
				os_time_interrupt_setting(current_time->msec);
		}
	}
#endif
	return OK;
}


int os_time_create_entry(os_time_type type, int (*time_entry)(void *),
		void *pVoid, int msec, char *func_name)
{
	os_time_t * t = os_time_entry_create(type, time_entry, pVoid,  msec, func_name);
	if(t)
	{
		if(time_mutex)
			os_mutex_lock(time_mutex, OS_WAIT_FOREVER);

		//os_timer_timeval(&t->interval);
		os_time_interval_update(t);
		t->state = OS_TIMER_TRUE;
		lstAddSort(time_list, (NODE *)t);

		os_time_interval_refresh();

#ifdef OS_TIMER_TEST
/*	fprintf(stdout, "%s time=%u.%u\n", __func__, t->interval.tv_sec*TIMER_MSEC_MICRO,
			(t->interval.tv_usec)/TIMER_MSEC_MICRO);*/
	fprintf(stdout, "%s time=%u.%u\n", __func__, t->interrupt_timestamp/TIMER_MSEC_MICRO,
			(t->interrupt_timestamp)%TIMER_MSEC_MICRO);

	//zlog_debug(ZLOG_NSM, "%s time=%u.%u", __func__,tick.it_value.tv_sec, tick.it_value.tv_usec/1000);
#endif
		if(time_mutex)
			os_mutex_unlock(time_mutex);
		return t->t_id;
	}
	return ERROR;
}



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


int os_time_destroy(int id)
{
	os_time_t *t = os_time_lookup(id);//(os_time_t *)id;
	if(t)
	{
		if(t->state == OS_TIMER_FALSE)
			return OK;
		if(time_mutex)
			os_mutex_lock(time_mutex, OS_WAIT_FOREVER);

		if(t->state == OS_TIMER_TRUE)
			lstDelete (time_list, (NODE *)t);
		t->state = OS_TIMER_FALSE;
		zlog_debug(ZLOG_DEFAULT, "%s:%s", __func__, t->entry_name);
		t->t_id = 0;
		lstAdd(time_unuse_list, (NODE *)t);
		os_time_interval_refresh();
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
		if(t->state == OS_TIMER_CANCEL)
			return OK;
		if(time_mutex)
			os_mutex_lock(time_mutex, OS_WAIT_FOREVER);
		t->state = OS_TIMER_CANCEL;
		lstDelete (time_list, (NODE *)t);
		zlog_debug(ZLOG_DEFAULT, "%s:%s", __func__, t->entry_name);
		lstAdd(time_unuse_list, (NODE *)t);
		os_time_interval_refresh();
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
		if(t->state == OS_TIMER_TRUE)
			return OK;
		if(time_mutex)
			os_mutex_lock(time_mutex, OS_WAIT_FOREVER);

		lstDelete(time_unuse_list, (NODE *)t);

		//os_timer_timeval(&t->interval);

		os_time_interval_update(t);
		t->state = OS_TIMER_TRUE;
		//lstAdd (time_list, (NODE *)t);
		lstAddSort(time_list, (NODE *)t);

		os_time_interval_refresh();

		if(time_mutex)
			os_mutex_unlock(time_mutex);
		return OK;
	}
	return ERROR;
}


int os_time_show(int (*show)(void *, char *fmt,...), void *pVoid)
{
	int i = 0;
	NODE *node = NULL;
	os_time_t *t = NULL;
	int timestamp = os_get_monotonic_msec ();
	if (time_mutex)
		os_mutex_lock(time_mutex, OS_WAIT_FOREVER);
	if(lstCount(time_list) && show )
	{
		(show)(pVoid, "%-4s %-8s %-8s %-8s %s %s", "----", "--------", "----------", "----------", "----------------", "\r\n");
		(show)(pVoid, "%-4s %-8s %-8s %-8s %s %s", "ID", "TYPE", "DELAY","INTERVAL", "NAME", "\r\n");
		(show)(pVoid, "%-4s %-8s %-8s %-8s %s %s", "----", "--------", "----------", "----------", "----------------", "\r\n");
	}
	for (node = lstFirst(time_list); node != NULL; node = lstNext(node))
	{
		t = (os_time_t *) node;
		if (node && show)
		{
			(show)(pVoid, "%-4d  %-8s %-8d %-8d %s%s", i++, (t->type == OS_TIMER_ONCE) ? "ONCE":"DEFAULT",
					t->interrupt_timestamp - timestamp, t->msec, t->entry_name, "\r\n");
		}
	}
	if (time_mutex)
		os_mutex_unlock(time_mutex);
	return OK;
}

static int os_time_task(void)
{
	//int flag = 0;
	NODE node;
	os_time_t *t;
	unsigned int interrupt_timestamp = 0;
	//struct timeval now;

	while(!os_load_config_done())
	{
		os_sleep(1);
	}
	while(1)
	{
		os_sem_take(time_sem, OS_WAIT_FOREVER);
		if(time_mutex)
			os_mutex_lock(time_mutex, OS_WAIT_FOREVER);

		//os_timer_timeval(&now);
		interrupt_timestamp = os_get_monotonic_msec ();

		for(t = (os_time_t *)lstFirst(time_list); t != NULL; t = (os_time_t *)lstNext(&node))
		{
			node = t->node;
			if(t && t->state == OS_TIMER_TRUE)
			{
				//if(os_timeval_cmp (now, t->interval))
				if(interrupt_timestamp >= t->interrupt_timestamp)
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
						zlog_debug(ZLOG_DEFAULT, "==========%s:%s", __func__, t->entry_name);
						lstAdd(time_unuse_list, (NODE *)t);
					}
					else
					{
						os_time_interval_update(t);
						lstDelete (time_list, (NODE *)t);
						//zlog_debug(ZLOG_DEFAULT, "%s:%s", __func__, t->entry_name);
						lstAdd(time_unuse_list, (NODE *)t);
						//lstAddSort(time_list, (NODE *)t);
					}
				}
				else
				{

				}
			}
		}
		//
		for(t = (os_time_t *)lstFirst(time_unuse_list); t != NULL; t = (os_time_t *)lstNext(&node))
		{
			node = t->node;
			if(t && t->state == OS_TIMER_TRUE)
			{
				if(t->type != OS_TIMER_ONCE)
				{
					lstDelete (time_unuse_list, (NODE *)t);
					//zlog_debug(ZLOG_DEFAULT, "==========%s:add %s", __func__, t->entry_name);
					lstAddSort(time_list, (NODE *)t);
					//lstAdd(time_list, (NODE *)t);
				}
			}
		}
		os_time_interval_refresh();

		if(time_mutex)
			os_mutex_unlock(time_mutex);
	}
	return 0;
}


#ifdef OS_TIMER_TEST

static u_int32 t_time_test = 0, t_time_test1 = 0;
static int timer_test_handle(void *p)
{
	struct timeval now;
	os_timer_timeval(&now);
	fprintf(stdout, "%s time=%u.%u msec\n", __func__,now.tv_sec*TIMER_MSEC_MICRO, now.tv_sec/TIMER_MSEC_MICRO);
	//zlog_debug(ZLOG_NSM, "%s time=%u.%u msec", __func__,now.tv_sec, now.tv_sec/1000);
	t_time_test = 0;
	return OK;
}

int timer_test(int time, int type)
{
	static char clean_flag = 0;
	struct timeval now;
	os_timer_timeval(&now);
	if(clean_flag == 0)
	{
		//os_time_clean(FALSE);
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
				fprintf(stdout, "%s time=%u.%u msec\n", __func__,now.tv_sec*TIMER_MSEC_MICRO, now.tv_sec/TIMER_MSEC_MICRO);
				return OK;
			}
		}
		fprintf(stdout, "%s time=%u.%u msec once\n", __func__,now.tv_sec*TIMER_MSEC_MICRO, now.tv_sec/TIMER_MSEC_MICRO);
		t_time_test = os_time_create_once(timer_test_handle, NULL, time);
	}
	else
	{
		fprintf(stdout, "%s time=%u.%u msec\n", __func__,now.tv_sec*TIMER_MSEC_MICRO, now.tv_sec/TIMER_MSEC_MICRO);
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


#if 0
/*******************************************************************************
*
* timer_connect - connect a user routine to the timer signal
*
* This routine sets the specified <routine> to be invoked with <arg> when
* fielding a signal indicated by the timer's <evp> signal number which is
* setup via the timer_create() routine.  If a signal number is not specified,
* i.e. if the <evp> parameter that is passed to timer_create() is NULL, the
* default signal (SIGALRM) will be used instead.
*
* The caller of timer_connect() must be the same task which called
* timer_create() associated with the <timerid> in order to field the signal.
* Calling timer_connect() from a task other than the caller timer_create ()
* will have undefined results.
*
* The signal handling routine should be declared as:
*
* \cs
*     void my_handler
*         (
*         timer_t timerid,     /@ expired timer ID @/
*         int     arg          /@ user argument    @/
*         )
* \ce
*
* NOTE
* This is a non-POSIX API.
*
* RETURNS: 0 (OK), or -1 (ERROR) if the timer is invalid or cannot bind the
* signal handler.
*
* ERRNO:
* \is
* \i EINVAL
* \ie
*/

int timer_connect
    (
    timer_t     timerid,	/* timer ID      */
    VOIDFUNCPTR routine,	/* user routine  */
    int         arg		/* user argument */
    )
    {
    static struct sigaction timerSig;

    if (intContext ())
	return (ERROR);

    if (timerid == NULL)
	{
	errno = EINVAL;
	return (ERROR);
	}

    if (timerSig.sa_handler == 0)
	{
	/* just the first time */
	timerSig.sa_handler = (void (*)(int))timerConHandler;
	(void) sigemptyset (&timerSig.sa_mask);
	timerSig.sa_flags = 0;	/* !SA_SIGINFO: cause timerid to be passed */
	}

    timerid->routine = routine;
    timerid->arg     = arg;

    timerid->sevent.sigev_value.sival_ptr = timerid;	/* !SA_SIGINFO */

    if (sigaction (timerid->sevent.sigev_signo, &timerSig, NULL) == ERROR)
	return (ERROR);
    else
	return (OK);
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
#define OS_SEC_MIN_V(1)		(60)
#define OS_SEC_HOU_V(1)		(3600)
#define OS_SEC_DAY_V(1)		(86400)
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
