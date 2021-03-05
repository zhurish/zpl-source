/*
 * os_time.h
 *
 *  Created on: May 30, 2017
 *      Author: zhurish
 */

#ifndef __OS_TIME_H__
#define __OS_TIME_H__


#ifdef __cplusplus
extern "C" {
#endif

//#define OS_TIMER_TEST

//#define OS_TIMER_DEBUG

#define OS_TIMER_POSIX

#define OS_TIMER_NAME_MAX	128


#define OS_SEC_MIN_V(n)		((60)*(n))
#define OS_SEC_HOU_V(n)		((3600)*(n))
#define OS_SEC_DAY_V(n)		((86400)*(n))


#define os_system_rate()	(100)

extern int os_system_tick();

enum os_tmtime_id {
  OS_TMTIME_LOCAL = 0,
  OS_TMTIME_UTC,
};

enum os_clkid {
  OS_CLK_REALTIME = 0,	/* ala gettimeofday() */
  OS_CLK_MONOTONIC,		/* monotonic, against an indeterminate base */
  OS_CLK_REALTIME_STABILISED, /* like realtime, but non-decrementing */
};

struct os_time_stats
{
  ospl_ulong total, max;
};

#define TIMER_SECOND_MICRO 1000000L
#define TIMER_MSEC_MICRO 1000L

#define TIMER_USEC(n) (n)*TIMER_SECOND_MICRO
#define TIMER_MSEC(n) (n)*TIMER_MSEC_MICRO

extern int os_system_time_base(ospl_char *dt);
extern ospl_time_t os_time (ospl_time_t *t);
extern ospl_time_t os_monotonic_time (void);
extern int os_gettime (enum os_clkid clkid, struct timeval *tv);


extern struct timeval os_timeval_adjust (struct timeval a);
extern struct timeval os_timeval_subtract (struct timeval a, struct timeval b);
extern int os_timeval_cmp (struct timeval a, struct timeval b);
extern int os_gettimeofday (struct timeval *tv);
extern int os_get_realtime (struct timeval *tv);
extern int os_get_monotonic (struct timeval *tv);
extern int os_get_monotonic_msec ();

extern ospl_uint32 os_timeval_elapsed (struct timeval a, struct timeval b);

extern struct timeval os_time_min(struct timeval a, struct timeval b);
extern struct timeval os_time_max(struct timeval a, struct timeval b);
extern ospl_uint32 os_timestamp_spilt(ospl_time_t t,  ospl_char *input);

extern int os_usleep(ospl_uint32 us);
extern int os_msleep(ospl_uint32);
extern int os_sleep(ospl_uint32);
extern void os_msleep_interrupt (ospl_uint32 mseconds);
extern void os_usleep_interrupt (ospl_uint32 useconds);

extern int os_tmtime_get (enum os_tmtime_id type, ospl_time_t t, struct tm *ptm);

extern ospl_char *os_time_fmt (ospl_char *fmt, ospl_time_t t);
extern ospl_char *os_time_string(ospl_time_t tInput);

extern const ospl_char *os_build_time2date(ospl_char *str);
extern const ospl_char *os_date2build_time(ospl_char *str);

extern int os_time_set_api(ospl_uint32 timesp);
extern int os_timezone_offset_api(ospl_char * res);
extern int os_timezone_set_api(ospl_uint32 tizone, ospl_char *timzstr);

#define OS_TIMER_FOREVER 0X0FFFFFFF

typedef enum
{
	OS_TIMER_DEFAULE = 0,
	OS_TIMER_ONCE,
}os_time_type;

typedef struct os_time_s
{
	NODE	node;
	ospl_uint32 t_id;
	int		(*time_entry)(void *);
	void	*pVoid;
	ospl_uint32 msec;

	//struct timeval interval;
	ospl_uint32 interrupt_timestamp;

	os_time_type type;
	enum {OS_TIMER_FALSE, OS_TIMER_TRUE, OS_TIMER_CANCEL} state;
	enum {OS_TIMER_NONE, OS_TIMER_READY, OS_TIMER_UNUSE} lstid;
	char    entry_name[OS_TIMER_NAME_MAX];
}os_time_t;


extern int os_time_init();
extern int os_time_exit();
extern int os_time_load();

extern int os_time_clean(ospl_bool all);
extern os_time_t *os_time_lookup(ospl_uint32 id);

extern int os_time_destroy(ospl_uint32 id);

extern int os_time_cancel(ospl_uint32 id);

extern int os_time_restart(ospl_uint32 id, ospl_uint32 msec);

extern int os_time_show(int (*show)(void *, ospl_char *fmt,...), void *pVoid);
//extern int os_time_create(int (*time_entry)(void *), void *pVoid, int msec);

extern ospl_uint32 os_time_create_entry(os_time_type type, int (*time_entry)(void *),
		void *pVoid, ospl_uint32 msec, const ospl_char *func_name);


#define os_time_create(f,p,v)		os_time_create_entry(OS_TIMER_DEFAULE,f,p,v,#f)
#define os_time_create_once(f,p,v)	os_time_create_entry(OS_TIMER_ONCE,f,p,v,#f)

#define quagga_time os_time
#define quagga_gettime os_gettime

#ifdef OS_TIMER_TEST
extern int timer_test(int time, ospl_uint32 type);
extern int timer_test_exit(ospl_uint32 type);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __OS_TIME_H__ */
