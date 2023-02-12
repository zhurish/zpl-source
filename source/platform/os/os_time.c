/*
 * os_time.c
 *
 *  Created on: May 30, 2017
 *      Author: zhurish
 */



#include "auto_include.h"
#include "zplos_include.h"
#include "log.h"





static LIST *time_list = NULL;
static LIST *time_unuse_list = NULL;

static os_sem_t *time_sem = NULL;
static os_mutex_t *time_mutex = NULL;

#ifdef OS_TIMER_POSIX
static os_time_t *current_time = NULL;
#else
static zpl_uint32	inter_value = 10;
#endif
static zpl_taskid_t time_task_id = 0;

#ifdef OS_TIMER_POSIX
static timer_t os_timerid = 0;
#ifndef OS_SIGNAL_SIGWAIT
static void os_time_interrupt(zpl_uint32 signo
#ifdef SA_SIGINFO
	     , siginfo_t *siginfo, void *context
#endif
);
#endif
#endif
static int os_time_task(void *p);


int os_system_time_base(zpl_char *dt)
{
	//int sysb = os_time(NULL);
	//if(sysb < 1546272000 || sysb >= 1577808000)//2019-01-01 00:00:00 < x < 2020-01-01 00:00:00
	{
		zpl_char dtcmd[128];
		memset(dtcmd, 0, sizeof(dtcmd));
		snprintf(dtcmd, sizeof(dtcmd), "date -s\"%s\"", dt);
		//super_system(dtcmd);
	}
	return OK;
}

int os_system_tick(void)
{
	struct timeval tv;
	if(os_gettime (OS_CLK_MONOTONIC, &tv) != OK)
	{
		os_gettimeofday(&tv);
	}
	return tv.tv_usec / 1000 + tv.tv_sec * 1000;
}


static void _os_usleep_interrupt (zpl_uint32 useconds)
{
	struct timeval delay;
	zpl_uint32 sec = 0;
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

void os_usleep_interrupt (zpl_uint32 useconds)
{
	_os_usleep_interrupt (useconds);
}

void os_msleep_interrupt (zpl_uint32 mseconds)
{
	_os_usleep_interrupt (mseconds * 1000);
}

int os_usleep(zpl_uint32 us)
{
	return usleep(us);
}

int os_msleep(zpl_uint32 ms)
{
	return usleep(ms*1000);
}

int os_sleep(zpl_uint32 s)
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

zpl_uint32
os_timeval_elapsed (struct timeval a, struct timeval b)
{
  return (((a.tv_sec - b.tv_sec) * TIMER_SECOND_MICRO)
	  + (a.tv_usec - b.tv_usec));
}

int
os_gettimeofday (struct timeval *tv)
{
  //assert (tv);
  return gettimeofday (tv, NULL);

}

int
os_get_realtime (struct timeval *tv)
{
	int ret;
	//assert(tv);
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
  //assert (tv);
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

int os_get_monotonic_msec (void)
{
	struct timeval tv;
	os_get_monotonic (&tv);
	os_timeval_adjust(tv);
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
zpl_ullong   monotonic_ns(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000000ULL + tv.tv_usec * 1000;
}
zpl_ullong   monotonic_us(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000ULL + tv.tv_usec;
}
zpl_ullong   monotonic_ms(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000ULL + tv.tv_usec / 1000;
}
unsigned  monotonic_sec(void)
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
        ipstack_errno = EINVAL;
        return -1;
    }
}

/*
 * 20181024160232  -> 2018/10/24 16:02:32
 */
const zpl_char *os_build_time2date(zpl_char *str)
{
	zpl_uint32 i = 0, j = 0;
	static zpl_char buf[64];
	zpl_char *p = str;
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
const zpl_char *os_date2build_time(zpl_char *str)
{
	zpl_uint32 i = 0;
	static zpl_char buf[64];
	zpl_char *p = str;
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


zpl_uint32 os_timestamp_spilt(zpl_time_t t,  zpl_char *input)
{
	zpl_uint32 i = 0, b = 0;
	struct tm tm_tmp;
	zpl_time_t time_tmp = 0;
	if(!input || !strlen(input))
	{
		return os_time(NULL);
	}
	zpl_char *brk = strstr(input, ":");
	if(t == 0)
		time_tmp = os_time(NULL);
	else
		time_tmp = t;
	localtime_r(&time_tmp, &tm_tmp);
	tm_tmp.tm_sec = 0;
	tm_tmp.tm_year = 0;
	memset(&tm_tmp, 0, sizeof(struct tm));
	brk = input;
	while((zpl_uint32)i < strlen(brk))
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
	return (zpl_uint32)time_tmp;
}

struct timeval os_time_min(struct timeval a, struct timeval b)
{
	zpl_uint32 ma = 0, mb = 0;
	ma = a.tv_sec * TIMER_SECOND_MICRO + a.tv_usec;
	mb = b.tv_sec * TIMER_SECOND_MICRO + b.tv_usec;

	if(ma > mb)
		return b;
	else
		return a;
}

struct timeval os_time_max(struct timeval a, struct timeval b)
{
	zpl_uint32 ma = 0, mb = 0;
	ma = a.tv_sec * TIMER_SECOND_MICRO + a.tv_usec;
	mb = b.tv_sec * TIMER_SECOND_MICRO + b.tv_usec;

	if(ma > mb)
		return a;
	else
		return b;
}

/* zpl_time_t value in terms of stabilised absolute time.
 * replacement for POSIX time()
 */
zpl_time_t os_time (zpl_time_t *t)
{
/*  struct timeval tv;
  os_gettime(OS_CLK_REALTIME, &tv);
  os_real_stabilised (&tv);
  if (t)
    *t = tv.tv_sec;
  return tv.tv_sec;*/
  return time(t);
}

zpl_time_t os_monotonic_time (void)
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
/*zpl_time_t os_time_scanf (char *fmt, zpl_time_t t)
{

}*/
int os_tmtime_get (enum os_tmtime_id type, zpl_time_t t, struct tm *ptm)
{
	zpl_time_t ticlock = t;
	struct tm tm;
	os_memset(&tm, 0, sizeof(tm));
	if(type == OS_TMTIME_LOCAL)
	{
		if(localtime_r(&ticlock, &tm) && ptm)
		{
			memcpy(ptm, &tm, sizeof(struct tm));
			return OK;
		}
	}
	else if (type == OS_TMTIME_UTC)
	{
		if(gmtime_r(&ticlock, &tm) && ptm)
		{
			memcpy(ptm, &tm, sizeof(struct tm));
			return OK;
		}
	}
	return ERROR;
}

zpl_char *os_time_fmt (const zpl_char *fmt, zpl_time_t t)
{
	zpl_uint32 len = 0;
	struct tm tm;
	//struct tm *ptm = NULL;
	static zpl_int8 data[128];
	zpl_time_t ticlock = t;
	if(t == 0)
		ticlock = os_time(NULL);
	os_memset(data, 0, sizeof(data));
	os_memset(&tm, 0, sizeof(tm));
	//UTC :Wed Apr 18 05:19:00 UTC 2018
	if(os_strstr(fmt, "bsd"))
	{
		localtime_r(&ticlock, &tm);
		len = strftime(data, sizeof(data), "%b %e %T", &tm);
	}
	else if(os_strstr(fmt, "/")||os_strstr(fmt, "date"))
	{
		localtime_r(&ticlock, &tm);
		len = strftime(data, sizeof(data), "%Y/%m/%d %H:%M:%S", &tm);
	}
	else if(os_strstr(fmt, "-"))
	{
		localtime_r(&ticlock, &tm);
		len = strftime(data, sizeof(data), "%Y-%m-%d %H:%M:%S", &tm);
	}
	else if(os_strstr(fmt, "sql"))
	{
		localtime_r(&ticlock, &tm);
		len = strftime(data, sizeof(data), "%Y-%m-%d %H:%M", &tm);
	}
	else if(os_strstr(fmt, "zpl_int16"))
	{
		localtime_r(&ticlock, &tm);
		len = strftime(data, sizeof(data), "%m/%d %H:%M:%S", &tm);
	}
	else if(os_strstr(fmt, "iso"))
	{
		//ptm = gmtime(&ticlock);
		gmtime_r(&ticlock, &tm);
		//len = strftime(data, sizeof(data), "%Y-%m-%dT%H:%M:%S+08:00",tm);
		len = strftime(data, sizeof(data), "%Y-%m-%dT%H:%M:%S", &tm);
	}
	else if(os_strstr(fmt, "rfc3164"))
	{
		localtime_r(&ticlock, &tm);
		len = strftime(data, sizeof(data), "%b %d %T", &tm);
	}
	else if(os_strstr(fmt, "rfc3339"))
	{
		//ptm = gmtime(&clock);
		gmtime_r(&ticlock, &tm);
		len = strftime(data, sizeof(data), "%Y-%m-%dT%H:%M:%S", &tm);
	}
	else if(os_strstr(fmt, "filename"))
	{
		localtime_r(&ticlock, &tm);
		len = strftime(data, sizeof(data), "%Y-%m-%d-%H-%M-%S", &tm);
	}
	if(len > 0)
		return data;
	return "UNKNOWN";
}

zpl_char *os_time_string(zpl_time_t tInput)
{
	static zpl_char tString[64];
	if(tInput)
	{
			zpl_uint32 sec = 0, minu = 0, hou = 0, day = 0;
			zpl_time_t local_t;
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

int os_time_init(void)
{
	os_system_time_base("2019-01-01 00:00:00");
	if(time_list == NULL)
	{
		time_list = os_malloc(sizeof(LIST));
		if(time_list)
		{
			lstInit(time_list);
			lstSortInit(time_list, os_time_compar);
			time_sem = os_sem_name_create("os_timer_sem");
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
		time_mutex = os_mutex_name_create("timemutex");
	}
	time_task_id = os_task_create("timeTask", OS_TASK_DEFAULT_PRIORITY,
	               0, os_time_task, NULL, OS_TASK_DEFAULT_STACK);
	if(time_task_id)
		return OK;
	os_time_exit();
	return ERROR;
}


int os_time_exit(void)
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
		if(os_mutex_destroy(time_mutex)==OK)
			time_mutex = NULL;
	}
	if(time_sem)
	{
		if(os_sem_destroy(time_sem)==OK)
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

int os_time_load(void)
{
	if(time_task_id)
		return OK;
	return ERROR;
}

int os_time_clean(zpl_bool all)
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
			t->lstid = OS_TIMER_NONE;
			if(!all)
			{
				t->lstid = OS_TIMER_UNUSE;
				lstAdd (time_unuse_list, (NODE *)t);
			}
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
				t->lstid = OS_TIMER_NONE;
				os_free(t);
			}
		}
	}
	if(time_mutex)
		os_mutex_unlock(time_mutex);
	return OK;
}

static os_time_t * os_time_get_node(void)
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
	{
		t->lstid = OS_TIMER_NONE;
		lstDelete (time_unuse_list, (NODE *)t);
	}
	if(time_mutex)
		os_mutex_unlock(time_mutex);
	return t;
}





static os_time_t * os_time_entry_create(os_time_type type, int (*time_entry)(void *),
		void *pVoid, zpl_uint32 msec, zpl_char *func_name)
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
		t->t_id = (zpl_uint32)t;
		//t->t_id = (int)os_get_monotonic_msec ();
		t->msec = msec;
		t->pVoid = pVoid;
		t->time_entry = time_entry;
		t->type = type;
		t->lstid = OS_TIMER_NONE;
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
#ifndef OS_SIGNAL_SIGWAIT
static void os_time_interrupt(zpl_uint32 signo
#ifdef SA_SIGINFO
	     , siginfo_t *siginfo, void *context
#endif
)
{
#ifdef SA_SIGINFO
	//fprintf(stdout,"____%s_____:PID=%d UID=%d si_addr=%p\r\n",__func__,
	//		siginfo->si_pid, siginfo->si_uid, siginfo->si_addr);
#endif
	if(time_sem && signo == SIGUSR2)
		os_sem_give(time_sem);
	//fprintf(stdout,"%s:\r\n",__func__);
	current_time = NULL;
	//OS_DEBUG("%s:\r\n",__func__);
	return;
}
#endif

static int timer_connect(timer_t timerid, int (*routine)(void *p))
{
	struct sigevent evp;
	memset(&evp, 0, sizeof(struct sigevent));

	evp.sigev_signo = SIGUSR2;
#ifndef OS_SIGNAL_SIGWAIT
	evp.sigev_notify = SIGEV_SIGNAL;
#else
	evp.sigev_notify = SIGEV_THREAD_ID;
	evp._sigev_un._tid = os_task_gettid();
#endif
	if (timer_create(CLOCK_MONOTONIC, &evp, &os_timerid) == -1)
	{
		return ERROR;
	}
#ifndef OS_SIGNAL_SIGWAIT
	os_register_signal(SIGUSR2, os_time_interrupt);
#endif
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

static zpl_ulong os_time_interrupt_interval_get(os_time_t *t)
{
	return (t->interrupt_timestamp - os_get_monotonic_msec ());
/*	struct timeval current_tv;
	os_timer_timeval (&current_tv);
	current_tv = os_timeval_subtract (t->interval, current_tv);
	return (((current_tv.tv_sec) * TIMER_SECOND_MICRO)
		  + (current_tv.tv_usec))/TIMER_MSEC_MICRO;*/
}

static int os_time_interrupt_setting(zpl_uint32 msec)
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
#ifdef OS_TIMER_DEBUG
	//fprintf(stdout, "%s time=%u.%u\n", __func__, tick.it_value.tv_sec,
	//		(tick.it_value.tv_nsec)/1000000);
	//zlog_debug(MODULE_NSM, "%s time=%u.%u", __func__,tick.it_value.tv_sec, tick.it_value.tv_usec/1000);
#endif
#ifndef OS_SIGNAL_SIGWAIT
	timer_connect(0, NULL);
#endif
//	OS_DEBUG("%s:%d.%d sec\r\n",__func__, tick.it_value.tv_sec, tick.it_value.tv_usec/TIMER_MSEC_MICRO);
	//After first, the Interval time for clock

	if(os_timerid)
		timer_settime(os_timerid, 0, &tick, NULL);
	return OK;
}


static int os_time_interval_refresh(void)
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


zpl_uint32 os_time_create_entry(os_time_type type, int (*time_entry)(void *),
		void *pVoid, zpl_uint32 msec, const zpl_char *func_name)
{
	os_time_t * t = os_time_entry_create(type, time_entry, pVoid,  msec, func_name);
	if(t)
	{
		if(time_mutex)
			os_mutex_lock(time_mutex, OS_WAIT_FOREVER);

		//os_timer_timeval(&t->interval);
		os_time_interval_update(t);
		t->state = OS_TIMER_TRUE;
		t->lstid = OS_TIMER_READY;
		lstAddSort(time_list, (NODE *)t);

		os_time_interval_refresh();

#ifdef OS_TIMER_DEBUG
	zlog_debug(MODULE_DEFAULT, "%s '%s' time=%lu.%lu\r\n", __func__, func_name, t->interrupt_timestamp/TIMER_MSEC_MICRO,
			(t->interrupt_timestamp)%TIMER_MSEC_MICRO);
#endif
		if(time_mutex)
			os_mutex_unlock(time_mutex);
		return t->t_id;
	}
	return (zpl_uint32)0;
}

static os_time_t *os_time_lookup_raw(zpl_uint32 id)
{
	NODE node;
	os_time_t *t = NULL;
	for(t = (os_time_t *)lstFirst(time_list); t != NULL; t = (os_time_t *)lstNext(&node))
	{
		node = t->node;
		if(t && t->t_id > 0 && t->t_id == id && t->state != OS_TIMER_FALSE)
		{
			break;
		}
	}
	if(!t)
	{
		for(t = (os_time_t *)lstFirst(time_unuse_list); t != NULL; t = (os_time_t *)lstNext(&node))
		{
			node = t->node;
			if(t && t->t_id > 0 && t->t_id == id && t->state != OS_TIMER_FALSE)
			{
				break;
			}
		}
	}
	return t;
}

os_time_t *os_time_lookup(zpl_uint32 id)
{
	os_time_t *t = NULL;
	if(time_mutex)
		os_mutex_lock(time_mutex, OS_WAIT_FOREVER);
	t = os_time_lookup_raw(id);
	if(time_mutex)
		os_mutex_unlock(time_mutex);
	return t;
}


int os_time_destroy(zpl_uint32 id)
{
	os_time_t *t = NULL;
	if(time_mutex)
		os_mutex_lock(time_mutex, OS_WAIT_FOREVER);
	t = os_time_lookup_raw(id);
	if(t)
	{
		if(t->lstid == OS_TIMER_READY)
		{
			lstDelete (time_list, (NODE *)t);
			lstAdd(time_unuse_list, (NODE *)t);
			t->lstid = OS_TIMER_UNUSE;
		}
		t->state = OS_TIMER_FALSE;
		//zlog_debug(MODULE_DEFAULT, "=========================%s:%s", __func__, t->entry_name);
		t->t_id = 0;
		os_time_interval_refresh();
		if(time_mutex)
			os_mutex_unlock(time_mutex);
		return OK;
	}
	if(time_mutex)
		os_mutex_unlock(time_mutex);
	return ERROR;
}

int os_time_cancel(zpl_uint32 id)
{
	os_time_t *t = NULL;
	if(time_mutex)
		os_mutex_lock(time_mutex, OS_WAIT_FOREVER);
	t = os_time_lookup_raw(id);
	if(t)
	{
		if(t->lstid == OS_TIMER_READY)
		{
			lstDelete (time_list, (NODE *)t);
			lstAdd(time_unuse_list, (NODE *)t);
			t->lstid = OS_TIMER_UNUSE;
		}
		t->state = OS_TIMER_CANCEL;
		os_time_interval_refresh();
		if(time_mutex)
			os_mutex_unlock(time_mutex);
		return OK;
	}
	if(time_mutex)
		os_mutex_unlock(time_mutex);
	return ERROR;
}

int os_time_restart(zpl_uint32 id, zpl_uint32 msec)
{
	os_time_t *t = NULL;
	if(time_mutex)
		os_mutex_lock(time_mutex, OS_WAIT_FOREVER);
	t = os_time_lookup_raw(id);
	if(t)
	{
		if(t->lstid == OS_TIMER_READY)
		{
			lstDelete (time_list, (NODE *)t);
			lstAdd(time_unuse_list, (NODE *)t);
			t->lstid = OS_TIMER_UNUSE;
		}
		t->msec = msec;
		os_time_interval_update(t);
#ifdef OS_TIMER_DEBUG
		zlog_debug(MODULE_DEFAULT, "%s '%s' time=%lu.%lu\r\n", __func__, t->entry_name, t->interrupt_timestamp/TIMER_MSEC_MICRO,
			(t->interrupt_timestamp)%TIMER_MSEC_MICRO);
#endif
		t->state = OS_TIMER_TRUE;
		t->lstid = OS_TIMER_READY;
		lstAddSort(time_list, (NODE *)t);

		os_time_interval_refresh();
		if(time_mutex)
			os_mutex_unlock(time_mutex);
		return OK;
	}
	if(time_mutex)
		os_mutex_unlock(time_mutex);
	return ERROR;
}


int os_time_show(int (*show)(void *, zpl_char *fmt,...), void *pVoid)
{
	zpl_uint32 i = 0;
	NODE *node = NULL;
	os_time_t *t = NULL;
	zpl_uint32 timestamp = os_get_monotonic_msec ();
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

static int os_time_task(void *p)
{
	NODE node;
	os_time_t *t;
	zpl_uint32 interrupt_timestamp = 0;
#ifdef OS_SIGNAL_SIGWAIT
	zpl_uint32 signum = 0, err = 0;
#else
	zpl_uint32 signo_tbl[] = {SIGUSR2};
	os_task_sigexecute(1, signo_tbl);
#endif
	os_sleep(5);
	///host_waitting_loadconfig();
#ifdef OS_SIGNAL_SIGWAIT
	zpl_uint32 signo_tbl[] = {SIGUSR2};
	os_task_sigexecute(1, signo_tbl);
	timer_connect(0, NULL);
#endif
	while(OS_TASK_TRUE())
	{
#ifdef OS_SIGNAL_SIGWAIT
		err = sigwait(&set, &signum);
		if(err != 0)
		{
			zlog_debug(MODULE_DEFAULT, "=========================%s:%s(SIGUSR2=%d, signum=%d)",
					__func__, strerror(ipstack_errno),SIGUSR2, signum);
			continue;
		}
		zlog_debug(MODULE_DEFAULT, "=========================%s SIGUSR2=%d, signum=%d",
				__func__, SIGUSR2, signum);
		current_time = NULL;
#else
		os_sem_take(time_sem, OS_WAIT_FOREVER);
#endif
		if(time_mutex)
			os_mutex_lock(time_mutex, OS_WAIT_FOREVER);

		interrupt_timestamp = os_get_monotonic_msec ();

		for(t = (os_time_t *)lstFirst(time_list); t != NULL; t = (os_time_t *)lstNext(&node))
		{
			node = t->node;
			if(t && t->state == OS_TIMER_TRUE)
			{
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
#ifdef OS_TIMER_DEBUG
						zlog_debug(MODULE_DEFAULT, "%s '%s'\r\n", __func__, t->entry_name);
#endif
						t->lstid = OS_TIMER_UNUSE;
						lstAdd(time_unuse_list, (NODE *)t);
					}
					else
					{
						os_time_interval_update(t);
						lstDelete (time_list, (NODE *)t);
#ifdef OS_TIMER_DEBUG
						zlog_debug(MODULE_DEFAULT, "%s '%s'\r\n", __func__, t->entry_name);
#endif
						t->lstid = OS_TIMER_UNUSE;
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
#ifdef OS_TIMER_DEBUG
					zlog_debug(MODULE_DEFAULT, "%s resetting '%s'\r\n", __func__, t->entry_name);
#endif
					t->lstid = OS_TIMER_READY;
					lstAddSort(time_list, (NODE *)t);
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

static zpl_uint32 t_time_test = 0, t_time_test1 = 0;
static int timer_test_handle(void *p)
{
	struct timeval now;
	os_timer_timeval(&now);
	fprintf(stdout, "%s time=%lu.%lu msec\n", __func__,now.tv_sec*TIMER_MSEC_MICRO, now.tv_sec/TIMER_MSEC_MICRO);
	//zlog_debug(MODULE_NSM, "%s time=%u.%u msec", __func__,now.tv_sec, now.tv_sec/1000);
	t_time_test = 0;
	return OK;
}

int timer_test(zpl_uint32 time, zpl_uint32 type)
{
	static char clean_flag = 0;
	struct timeval now;
	os_timer_timeval(&now);
	if(clean_flag == 0)
	{
		//os_time_clean(zpl_false);
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
				fprintf(stdout, "%s time=%lu.%lu msec\n", __func__,now.tv_sec*TIMER_MSEC_MICRO, now.tv_sec/TIMER_MSEC_MICRO);
				return OK;
			}
		}
		fprintf(stdout, "%s time=%lu.%lu msec once\n", __func__,now.tv_sec*TIMER_MSEC_MICRO, now.tv_sec/TIMER_MSEC_MICRO);
		t_time_test = os_time_create_once(timer_test_handle, NULL, time);
	}
	else
	{
		fprintf(stdout, "%s time=%lu.%lu msec\n", __func__,now.tv_sec*TIMER_MSEC_MICRO, now.tv_sec/TIMER_MSEC_MICRO);
		t_time_test1 = os_time_create(timer_test_handle, NULL, time);
	}
	return OK;
}

int timer_test_exit(zpl_uint32 type)
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


int os_time_set_api(zpl_uint32 timesp)
{
	struct timespec sntpTime; /* storage for retrieved time value */
	zpl_uint32 local_timesp = 0, value = 0;
	sntpTime.tv_sec = sntpTime.tv_nsec = rand();

	local_timesp = os_time(NULL);

	value = timesp - local_timesp;
	if (abs(value) <= 5)
	{
		return OK;
	}
	sntpTime.tv_sec = timesp;
	value = 5;
	while (value)
	{
		ipstack_errno = 0;
		if (clock_settime(CLOCK_REALTIME, &sntpTime) != 0)//SET SYSTEM LOCAL TIME
		{
			value--;
		}
		else
		{
			break;
		}
	}
	if (value > 0)
	{
		sync();
		return OK;
	}
	else
		return ERROR;
}

int os_timezone_set_api(zpl_int32 tizone, zpl_char *timzstr)
{
	if(timzstr)
	{
		if(setenv("TZ", timzstr, 1) == 0)
		{
			tzset();
			return OK;
		}
	}
	else
	{
		zpl_char tizmp[64];
		memset(tizmp, 0, sizeof(tizmp));
		if(tizone == 0)
		{
			sprintf(tizmp, "%s", "UTC");
		}
		else if(tizone > 0)
		{
			sprintf(tizmp, "GMT-%d", tizone);
		}
		else if(tizone < 0)
		{
			sprintf(tizmp, "GMT+%d", tizone);
		}
		if(setenv("TZ", tizmp, 1) == 0)
		{
			tzset();
			return OK;
		}
	}
	return ERROR;
}



struct time_zone
{
	const zpl_char *time_zone;
	zpl_uint32	offset;
}time_zone_tbl[] =
{
	{ "UTC", 0 },
	{ "Africa/Abidjan", 0 },
	{ "Africa/Accra", 0 },
	{ "Africa/Addis Ababa", -3 },
	{ "Africa/Algiers", -1 },
	{ "Africa/Asmara", -3 },
	{ "Africa/Bamako", 0 },
	{ "Africa/Bangui", -1 },
	{ "Africa/Banjul", 0 },
	{ "Africa/Bissau", 0 },
	{ "Africa/Blantyre", -2 },
	{ "Africa/Brazzaville", -1 },
	{ "Africa/Bujumbura", -2 },
		{ "Africa/Cairo", -2 },
		{ "Africa/Ceuta", -2 },
		{ "Africa/Conakry", 0 },
		{ "Africa/Dakar", 0 },
		{ "Africa/Dar es Salaam", -3 },
		{ "Africa/Djibouti", 03 },
		{ "Africa/Douala", -2 },
		{ "Africa/Freetown", 0 },
		{ "Africa/Gaborone", -2 },
		{ "Africa/Harare", -2 },
		{ "Africa/Johannesburg", -2 },
		{ "Africa/Juba", -3 },
		{ "Africa/Kampala", -3 },
		{ "Africa/Khartoum", -2 },
		{ "Africa/Kigali", -2 },
		{ "Africa/Kinshasa", -1 },
		{ "Africa/Lagos", -1 },
		{ "Africa/Libreville", -1 },
		{ "Africa/Lome", 0 },
		{ "Africa/Luanda", -1 },
		{ "Africa/Lubumbashi", -2 },
		{ "Africa/Lusaka", -2 },
		{ "Africa/Malabo", -1 },
		{ "Africa/Maputo", -2 },
		{ "Africa/Maseru", -2 },
		{ "Africa/Mbabane", -2 },
		{ "Africa/Mogadishu", -3 },
		{ "Africa/Monrovia", 0 },
		{ "Africa/Nairobi", -3 },
		{ "Africa/Ndjamena", -1 },
		{ "Africa/Niamey", -1 },
		{ "Africa/Nouakchott", 0 },
		{ "Africa/Ouagadougou", 0 },
		{ "Africa/Porto-Novo", -1 },
		{ "Africa/Sao Tome", 0 },
		{ "Africa/Tripoli", -2 },
		{ "Africa/Tunis", -1 },
		{ "Africa/Windhoek", -2 },
		{ "America/Adak", 0 },
		{ "America/Anchorage", 0 },
		{ "America/Anguilla", 4 },
		{ "America/Antigua", 4 },
		{ "America/Araguaina", 3 },
		{ "America/Argentina/Buenos Aires", 3 },
		{ "America/Argentina/Catamarca", 3 },
		{ "America/Argentina/Cordoba", 3 },
		{ "America/Argentina/Jujuy", 3 },
		{ "America/Argentina/La Rioja", 3 },
		{ "America/Argentina/Mendoza", 3 },
		{ "America/Argentina/Rio Gallegos", 3 },
		{ "America/Argentina/Salta", 3 },
		{ "America/Argentina/San Juan", 3 },
		{ "America/Argentina/San Luis", 3 },
		{ "America/Argentina/Tucuman", 3 },
		{ "America/Argentina/Ushuaia", 3 },
		{ "America/Aruba", 4 },
		{ "America/Asuncion", 4 },
		{ "America/Atikokan", 5 },
		{ "America/Bahia", 3 },
		{ "America/Bahia Banderas", 5 },
		{ "America/Barbados", 4 },
		{ "America/Belem", 3 },
		{ "America/Belize", 6 },
		{ "America/Blanc-Sablon", 4 },
		{ "America/Boa Vista", 4 },
		{ "America/Bogota", 5 },
		{ "America/Boise", 3 },
		{ "America/Cambridge Bay", 3 },
		{ "America/Campo Grande", 1 },
		{ "America/Cancun", 5 },
		{ "America/Caracas", 4 },
		{ "America/Cayenne", 3 },
		{ "America/Cayman", 5 },
		{ "America/Chicago", 5 },
		{ "America/Chihuahua", 0 },
		{ "America/Costa Rica", 6 },
		{ "America/Creston", 7 },
		{ "America/Cuiaba", 4 },
		{ "America/Curacao", 4 },
		{ "America/Danmarkshavn", 0 },
		{ "America/Dawson", 3 },
		{ "America/Dawson Creek", 7 },
		{ "America/Denver", 6 },
		{ "America/Detroit", 4 },
		{ "America/Dominica", 4 },
		{ "America/Edmonton", 6 },
		{ "America/Eirunepe", 5 },
		{ "America/El Salvador", 6 },
		{ "America/Fort Nelson", 7 },
		{ "America/Fortaleza", 3 },
		{ "America/Glace Bay", 3 },
		{ "America/Godthab", 3 },
		{ "America/Goose Bay", 3 },
		{ "America/Grand Turk", 4 },
		{ "America/Grenada", 4 },
		{ "America/Guadeloupe", 4 },
		{ "America/Guatemala", 6 },
		{ "America/Guayaquil", 5 },
		{ "America/Guyana", 4 },
		{ "America/Halifax", 4 },
		{ "America/Havana", 5 },
		{ "America/Hermosillo", 7 },
		{ "America/Indiana/Indianapolis", 5 },
		{ "America/Indiana/Knox", 6 },
		{ "America/Indiana/Marengo", 5 },
		{ "America/Indiana/Petersburg", 5 },
		{ "America/Indiana/Tell City", 6 },
		{ "America/Indiana/Vevay", 5 },
		{ "America/Indiana/Vincennes", 5 },
		{ "America/Indiana/Winamac", 5 },
		{ "America/Inuvik", 7 },
		{ "America/Iqaluit", 5 },
		{ "America/Jamaica", 5 },
		{ "America/Juneau", 8 },
		{ "America/Kentucky/Louisville", 5 },
		{ "America/Kentucky/Monticello", 5 },
		{ "America/Kralendijk", 4 },
		{ "America/La Paz", 4 },
		{ "America/Lima", 5 },
		{ "America/Los Angeles", 7 },
		{ "America/Lower Princes", 4 },
		{ "America/Maceio", 3 },
		{ "America/Managua", 6 },
		{ "America/Manaus", 4 },
		{ "America/Marigot", 4 },
		{ "America/Martinique", 4 },
		{ "America/Matamoros", 6 },
		{ "America/Mazatlan", 7 },
		{ "America/Menominee", 6 },
		{ "America/Merida", 6 },
		{ "America/Metlakatla", 8 },
		{ "America/Mexico City", 6 },
		{ "America/Miquelon", 3 },
		{ "America/Moncton", 4 },
		{ "America/Monterrey", 6 },
		{ "America/Montevideo", 3 },
		{ "America/Montserrat", 4 },
		{ "America/Nassau", 5 },
		{ "America/New York", 5 },
		{ "America/Nipigon", 5 },
		{ "America/Nome", 8 },
		{ "America/Noronha", 2 },
		{ "America/North Dakota/Beulah", 6 },
		{ "America/North Dakota/Center", 6 },
		{ "America/North Dakota/New Salem", 6 },
		{ "America/Ojinaga", 6 },
		{ "America/Panama", 5 },
		{ "America/Pangnirtung", 5 },
		{ "America/Paramaribo", 3 },
		{ "America/Phoenix", 7 },
		{ "America/Port of Spain", 4 },
		{ "America/Port-au-Prince", 5 },
		{ "America/Porto Velho", 4 },
		{ "America/Puerto Rico", 4 },
		{ "America/Punta Arenas", 3 },
		{ "America/Rainy River", 6 },
		{ "America/Rankin Inlet", 6 },
		{ "America/Recife", 3 },
		{ "America/Regina", 6 },
		{ "America/Resolute", 6 },
		{ "America/Rio Branco", 5 },
		{ "America/Santarem", 3 },
		{ "America/Santiago", 4 },
		{ "America/Santo Domingo", 4 },
		{ "America/Sao Paulo", 3 },
		{ "America/Scoresbysund", 1 },
		{ "America/Sitka", 8 },
		{ "America/St Barthelemy", 4 },
		{ "America/St Johns", 3 },
		{ "America/St Kitts", 4 },
		{ "America/St Lucia", 4 },
		{ "America/St Thomas", 4 },
		{ "America/St Vincent", 4 },
		{ "America/Swift Current", 6 },
		{ "America/Tegucigalpa", 6 },
		{ "America/Thule", 4 },
		{ "America/Thunder Bay", 5 },
		{ "America/Tijuana", 7 },
		{ "America/Toronto", 5 },
		{ "America/Tortola", 4 },
		{ "America/Vancouver", 7 },
		{ "America/Whitehorse", 7 },
		{ "America/Winnipeg", 6 },
		{ "America/Yakutat", 8 },
		{ "America/Yellowknife", 6 },
		{ "Antarctica/Casey", 8 },
		{ "Antarctica/Davis", 7 },
		{ "Antarctica/DumontDUrville", 10 },
		{ "Antarctica/Macquarie",11 },
		{ "Antarctica/Mawson", 5 },
		{ "Antarctica/McMurdo", -12 },
		{ "Antarctica/Palmer", 3 },
		{ "Antarctica/Rothera", 3 },
		{ "Antarctica/Syowa", 3 },
		{ "Antarctica/Troll", -2 },
		{ "Antarctica/Vostok", -6 },
		{ "Arctic/Longyearbyen", -2 },
		{ "Asia/Aden", -3 },
		{ "Asia/Almaty", -6 },
		{ "Asia/Amman", -2 },
		{ "Asia/Anadyr", -12 },
		{ "Asia/Aqtau", -5 },
		{ "Asia/Aqtobe", -5 },
		{ "Asia/Ashgabat", -5 },
		{ "Asia/Atyrau", -5 },
		{ "Asia/Baghdad", -3 },
		{ "Asia/Bahrain", -3 },
		{ "Asia/Baku", -4 },
		{ "Asia/Bangkok", -7 },
		{ "Asia/Barnaul", -7 },
		{ "Asia/Beirut", -3 },
		{ "Asia/Bishkek", -6 },
		{ "Asia/Brunei", -8 },
		{ "Asia/Chita", -9 },
		{ "Asia/Choibalsan", -8 },
		{ "Asia/Colombo", -5 },
		{ "Asia/Damascus", -3 },
		{ "Asia/Dhaka", -6 },
		{ "Asia/Dili", -9 },
		{ "Asia/Dubai", -4 },
		{ "Asia/Dushanbe", -5 },
		{ "Asia/Famagusta", -3 },
		{ "Asia/Gaza", -3 },
		{ "Asia/Hebron", -3 },
		{ "Asia/Ho Chi Minh", -7 },
		{ "Asia/Hong Kong", -8 },
		{ "Asia/Hovd", -7 },
		{ "Asia/Irkutsk", -8 },
		{ "Asia/Jakarta", -7 },
		{ "Asia/Jayapura", -9 },
		{ "Asia/Jerusalem", 0 },
		{ "Asia/Kabul", -4 },
		{ "Asia/Kamchatka", -12 },
		{ "Asia/Karachi", -5 },
		{ "Asia/Kathmandu", -5 },
		{ "Asia/Khandyga", -9 },
		{ "Asia/Kolkata", -5 },
		{ "Asia/Krasnoyarsk", -7 },
		{ "Asia/Kuala Lumpur", -8 },
		{ "Asia/Kuching", -8 },
		{ "Asia/Kuwait", -3 },
		{ "Asia/Macau", -8 },
		{ "Asia/Magadan",-11 },
		{ "Asia/Makassar", -8 },
		{ "Asia/Manila", -8 },
		{ "Asia/Muscat", -4 },
		{ "Asia/Nicosia", -3 },
		{ "Asia/Novokuznetsk", -7 },
		{ "Asia/Novosibirsk", -7 },
		{ "Asia/Omsk", -6 },
		{ "Asia/Oral", -5 },
		{ "Asia/Phnom Penh", -7 },
		{ "Asia/Pontianak", -7 },
		{ "Asia/Pyongyang", -9 },
		{ "Asia/Qatar", -3 },
		{ "Asia/Qostanay", -6 },
		{ "Asia/Qyzylorda", -5 },
		{ "Asia/Riyadh", -3 },
		{ "Asia/Sakhalin",-11 },
		{ "Asia/Samarkand", -5 },
		{ "Asia/Seoul", -9 },
		{ "Asia/Shanghai", -8 },
		{ "Asia/Singapore", -8 },
		{ "Asia/Srednekolymsk",-11 },
		{ "Asia/Taipei", -8 },
		{ "Asia/Tashkent", -5 },
		{ "Asia/Tbilisi", -4 },
		{ "Asia/Tehran", -3 },
		{ "Asia/Thimphu", -6 },
		{ "Asia/Tokyo", -9 },
		{ "Asia/Tomsk", -7 },
		{ "Asia/Ulaanbaatar", -8 },
		{ "Asia/Urumqi", -6 },
		{ "Asia/Ust-Nera", -10 },
		{ "Asia/Vientiane", -7 },
		{ "Asia/Vladivostok", -10 },
		{ "Asia/Yakutsk", -9 },
		{ "Asia/Yangon", -6 },
		{ "Asia/Yekaterinburg", -5 },
		{ "Asia/Yerevan", -4 },
		{ "Atlantic/Azores", 0 },
		{ "Atlantic/Bermuda", 3 },
		{ "Atlantic/Canary", -1 },
		{ "Atlantic/Cape Verde", 3 },
		{ "Atlantic/Faroe", -1 },
		{ "Atlantic/Madeira", -1 },
		{ "Atlantic/Reykjavik", 0 },
		{ "Atlantic/South Georgia", 2 },
		{ "Atlantic/St Helena", 0 },
		{ "Atlantic/Stanley", -3 },
		{ "Australia/Adelaide", -9 },
		{ "Australia/Brisbane", -10 },
		{ "Australia/Broken Hill", -9 },
		{ "Australia/Currie", -10 },
		{ "Australia/Darwin", -9 },
		{ "Australia/Eucla", -8 },
		{ "Australia/Hobart", -10 },
		{ "Australia/Lindeman", -10 },
		{ "Australia/Lord Howe", -10 },
		{ "Australia/Melbourne", -10 },
		{ "Australia/Perth", -8 },
		{ "Australia/Sydney", -10 },
		{ "Etc/GMT", 0 },
		{ "Etc/GMT+1", 1 },
		{ "Etc/GMT+2", 2 },
		{ "Etc/GMT+3", 3 },
		{ "Etc/GMT+4", 4 },
		{ "Etc/GMT+5", 5 },
		{ "Etc/GMT+6", 6 },
		{ "Etc/GMT+7", 7 },
		{ "Etc/GMT+8", 8 },
		{ "Etc/GMT+9", 9 },
		{ "Etc/GMT+10", 10 },
		{ "Etc/GMT+11", 11 },
		{ "Etc/GMT+12", 12 },

		{ "Etc/GMT-1", -1 },
		{ "Etc/GMT-2", -2 },
		{ "Etc/GMT-3", -3 },
		{ "Etc/GMT-4", -4 },
		{ "Etc/GMT-5", -5 },
		{ "Etc/GMT-6", -6 },
		{ "Etc/GMT-7", -7 },
		{ "Etc/GMT-8", -8 },
		{ "Etc/GMT-9", -9 },
		{ "Etc/GMT-10", -10 },
		{ "Etc/GMT-11", -11 },
		{ "Etc/GMT-12", -12 },
		{ "Europe/Amsterdam", 2 },
		{ "Europe/Andorra", 2 },
		{ "Europe/Astrakhan", 4 },
		{ "Europe/Athens", 3 },
		{ "Europe/Belgrade", 2 },
		{ "Europe/Berlin", 2 },
		{ "Europe/Bratislava", 2 },
		{ "Europe/Brussels", 2 },
		{ "Europe/Bucharest", 3 },
		{ "Europe/Budapest", 2 },
		{ "Europe/Busingen", 2 },
		{ "Europe/Chisinau", 3 },
		{ "Europe/Copenhagen", 2 },
		{ "Europe/Dublin", 2 },
		{ "Europe/Gibraltar", 2 },
		{ "Europe/Guernsey", -1 },
		{ "Europe/Helsinki", 3 },
		{ "Europe/Isle of Man", -1 },
		{ "Europe/Istanbul", -3 },
		{ "Europe/Jersey", -1 },
		{ "Europe/Kaliningrad", -2 },
		{ "Europe/Kiev", 3 },
		{ "Europe/Kirov", -3 },
		{ "Europe/Lisbon", -1 },
		{ "Europe/Ljubljana", -2 },
		{ "Europe/London", -1 },
		{ "Europe/Luxembourg", -2 },
		{ "Europe/Madrid", -2 },
		{ "Europe/Malta", -2 },
		{ "Europe/Mariehamn", 3 },
		{ "Europe/Minsk", -3 },
		{ "Europe/Monaco", -2 },
		{ "Europe/Moscow", -3 },
		{ "Europe/Oslo", -2 },
		{ "Europe/Paris", -2 },
		{ "Europe/Podgorica", -2 },
		{ "Europe/Prague", -2 },
		{ "Europe/Riga", 3 },
		{ "Europe/Rome", -2 },
		{ "Europe/Samara", -4 },
		{ "Europe/San Marino", -2 },
		{ "Europe/Sarajevo", -2 },
		{ "Europe/Saratov", -4 },
		{ "Europe/Simferopol", -3 },
		{ "Europe/Skopje", -2 },
		{ "Europe/Sofia", 3 },
		{ "Europe/Stockholm", -2 },
		{ "Europe/Tallinn", 3 },
		{ "Europe/Tirane", -2 },
		{ "Europe/Ulyanovsk", -4 },
		{ "Europe/Uzhgorod", 3 },
		{ "Europe/Vaduz", -2 },
		{ "Europe/Vatican", -2 },
		{ "Europe/Vienna", -2 },
		{ "Europe/Vilnius", 3 },
		{ "Europe/Volgograd", -4 },
		{ "Europe/Warsaw", -2 },
		{ "Europe/Zagreb", -2 },
		{ "Europe/Zaporozhye", 3 },
		{ "Europe/Zurich", -2 },
		{ "Indian/Antananarivo", -3 },
		{ "Indian/Chagos", -6 },
		{ "Indian/Christmas", -7 },
		{ "Indian/Cocos", -6 },
		{ "Indian/Comoro", -3 },
		{ "Indian/Kerguelen", -5 },
		{ "Indian/Mahe", -4 },
		{ "Indian/Maldives", -5 },
		{ "Indian/Mauritius", -4 },
		{ "Indian/Mayotte", -3 },
		{ "Indian/Reunion", -4 },
		{ "Pacific/Apia", -12 },
		{ "Pacific/Auckland", -12 },
		{ "Pacific/Bougainville",-11 },
		{ "Pacific/Chatham", -12 },
		{ "Pacific/Chuuk", -10 },
		{ "Pacific/Easter", -6 },
		{ "Pacific/Efate",-11 },
		{ "Pacific/Enderbury", -13 },
		{ "Pacific/Fakaofo", -13 },
		{ "Pacific/Fiji", -12 },
		{ "Pacific/Funafuti", -12 },
		{ "Pacific/Galapagos", 6 },
		{ "Pacific/Gambier", 9 },
		{ "Pacific/Guadalcanal",-11 },
		{ "Pacific/Guam", 10 },
		{ "Pacific/Honolulu", 10 },
		{ "Pacific/Kiritimati", -14 },
		{ "Pacific/Kosrae",-11 },
		{ "Pacific/Kwajalein", -12 },
		{ "Pacific/Majuro", -12 },
		{ "Pacific/Marquesas", 9 },
		{ "Pacific/Midway", 11 },
		{ "Pacific/Nauru", -12 },
		{ "Pacific/Niue", 11 },
		{ "Pacific/Norfolk",-11 },
		{ "Pacific/Noumea",-11 },
		{ "Pacific/Pago Pago", 11 },
		{ "Pacific/Palau", -9 },
		{ "Pacific/Pitcairn", 8 },
		{ "Pacific/Pohnpei",-11 },
		{ "Pacific/Port Moresby", -10 },
		{ "Pacific/Rarotonga", 10 },
		{ "Pacific/Saipan", 10 },
		{ "Pacific/Tahiti", 10 },
		{ "Pacific/Tarawa", -12 },
		{ "Pacific/Tongatapu", -13 },
		{ "Pacific/Wake", -12 },
		{ "Pacific/Wallis", -12 },
};


int os_timezone_offset_api(zpl_char * res)
{
	zpl_uint32 i = 0;
	zpl_char tmp[128];
	int	 ret = 0;
	if(res)
	{
		for(i = 0; i < sizeof(time_zone_tbl)/sizeof(time_zone_tbl[0]); i++)
		{
			if(strcmp(time_zone_tbl[i].time_zone, res) == 0)
			{
				return time_zone_tbl[i].offset;
			}
		}
	}
	else
	{
		memset(tmp, 0, sizeof(tmp));
#if 0//def ZPL_OPENWRT_UCI
		ret |= os_uci_get_string("system.@system[0].zonename", tmp);
#else
		ret = OK;
		strcpy(tmp, "Etc/GMT-8");
#endif
		if(ret != OK)
		{
			return 0;
		}
		for(i = 0; i < sizeof(time_zone_tbl)/sizeof(time_zone_tbl[0]); i++)
		{
			if(strcmp(time_zone_tbl[i].time_zone, tmp) == 0)
			{
				return time_zone_tbl[i].offset;
			}
		}
	}
	return 0;
}


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
	ipstack_errno = EINVAL;
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
	zpl_ulong filesize = 0;
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
