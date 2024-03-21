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
#include "os_list.h"
//#define OS_TIMER_DEBUG

#define OS_TIMER_POSIX

#define OS_TIMER_NAME_MAX	128


#define OS_SEC_MIN_V(n)		((60)*(n))
#define OS_SEC_HOU_V(n)		((3600)*(n))
#define OS_SEC_DAY_V(n)		((86400)*(n))


#define os_system_rate()	(100)

extern int os_system_tick(void);

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
  zpl_ulong total, max;
};

#define TIMER_SECOND_MICRO 1000000L
#define TIMER_MSEC_MICRO 1000L

#define TIMER_USEC(n) (n)*TIMER_SECOND_MICRO
#define TIMER_MSEC(n) (n)*TIMER_MSEC_MICRO

extern int os_system_time_base(zpl_char *dt);
extern zpl_time_t os_time (zpl_time_t *t);
extern zpl_time_t os_monotonic_time (void);
extern int os_gettime (enum os_clkid clkid, struct timeval *tv);


extern struct timeval os_timeval_adjust (struct timeval a);
extern struct timeval os_timeval_subtract (struct timeval a, struct timeval b);
extern int os_timeval_cmp (struct timeval a, struct timeval b);
extern int os_gettimeofday (struct timeval *tv);
extern int os_get_realtime (struct timeval *tv);
extern int os_get_monotonic (struct timeval *tv);
extern int os_get_monotonic_msec (void);

extern zpl_uint32 os_timeval_elapsed (struct timeval a, struct timeval b);

extern struct timeval os_time_min(struct timeval a, struct timeval b);
extern struct timeval os_time_max(struct timeval a, struct timeval b);
extern zpl_uint32 os_timestamp_spilt(zpl_time_t t,  zpl_char *input);

extern int os_usleep(zpl_uint32 us);
extern int os_msleep(zpl_uint32);
extern int os_sleep(zpl_uint32);
extern void os_msleep_interrupt (zpl_uint32 mseconds);
extern void os_usleep_interrupt (zpl_uint32 useconds);

extern int os_tmtime_get (enum os_tmtime_id type, zpl_time_t t, struct tm *ptm);

extern zpl_char *os_time_fmt (const zpl_char *fmt, zpl_time_t t);
extern zpl_char *os_time_string(zpl_time_t tInput);

extern const zpl_char *os_build_time2date(zpl_char *str);
extern const zpl_char *os_date2build_time(zpl_char *str);

extern int os_time_set_api(zpl_uint32 timesp);
extern int os_timezone_offset_api(zpl_char * res);
extern int os_timezone_set_api(zpl_int32 tizone, zpl_char *timzstr);

#define OS_TIMER_FOREVER 0X0FFFFFFF

typedef enum
{
	OS_TIMER_DEFAULE = 0,
	OS_TIMER_ONCE,
}os_time_type;

typedef struct os_time_s
{
	NODE	node;
	zpl_uint32 t_id;
	int		(*time_entry)(void *);
	void	*pVoid;
	zpl_uint32 msec;

	//struct timeval interval;
	zpl_ulong interrupt_timestamp;

	os_time_type type;
	enum {OS_TIMER_FALSE, OS_TIMER_TRUE, OS_TIMER_CANCEL} state;
	enum {OS_TIMER_NONE, OS_TIMER_READY, OS_TIMER_UNUSE} lstid;
	char    entry_name[OS_TIMER_NAME_MAX];
}os_time_t;


extern int os_time_init(void);
extern int os_time_exit(void);
extern int os_time_load(void);

extern int os_time_clean(zpl_bool all);
extern os_time_t *os_time_lookup(zpl_uint32 id);

extern int os_time_destroy(zpl_uint32 id);

extern int os_time_cancel(zpl_uint32 id);

extern int os_time_restart(zpl_uint32 id, zpl_uint32 msec);

extern int os_time_show(int (*show)(void *, zpl_char *fmt,...), void *pVoid);
//extern int os_time_create(int (*time_entry)(void *), void *pVoid, int msec);

extern zpl_uint32 os_time_create_entry(os_time_type type, int (*time_entry)(void *),
		void *pVoid, zpl_uint32 msec, const zpl_char *func_name);


#define os_time_create(f,p,v)		os_time_create_entry(OS_TIMER_DEFAULE,f,p,v,#f)
#define os_time_create_once(f,p,v)	os_time_create_entry(OS_TIMER_ONCE,f,p,v,#f)


#ifdef OS_TIMER_TEST
extern int timer_test(int time, zpl_uint32 type);
extern int timer_test_exit(zpl_uint32 type);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __OS_TIME_H__ */
