/*
 * os_time.h
 *
 *  Created on: May 30, 2017
 *      Author: zhurish
 */

#ifndef __OS_TIME_H__
#define __OS_TIME_H__


//#define OS_TIMER_TEST

#define OS_TIMER_NAME_MAX	128
//10ms is one tick
//#define os_system_tick()	(jiffies)
#define os_system_rate()	(100)

extern int os_system_tick();


enum os_clkid {
  OS_CLK_REALTIME = 0,	/* ala gettimeofday() */
  OS_CLK_MONOTONIC,		/* monotonic, against an indeterminate base */
  OS_CLK_REALTIME_STABILISED, /* like realtime, but non-decrementing */
};

struct os_time_stats
{
  unsigned long total, max;
};

#define TIMER_SECOND_MICRO 1000000L
#define TIMER_MSEC_MICRO 1000L

#define TIMER_USEC(n) (n)*TIMER_SECOND_MICRO
#define TIMER_MSEC(n) (n)*TIMER_MSEC_MICRO


extern time_t os_time (time_t *t);
extern time_t quagga_time(time_t *t);
extern int os_gettime (enum os_clkid clkid, struct timeval *tv);


extern struct timeval os_timeval_adjust (struct timeval a);
extern struct timeval os_timeval_subtract (struct timeval a, struct timeval b);
extern int os_timeval_cmp (struct timeval a, struct timeval b);
extern int os_gettimeofday (struct timeval *tv);
extern int os_get_relative (struct timeval *tv);
extern void os_real_stabilised (struct timeval *tv);
extern unsigned int os_timeval_elapsed (struct timeval a, struct timeval b);

extern struct timeval os_time_min(struct timeval a, struct timeval b);
extern struct timeval os_time_max(struct timeval a, struct timeval b);

extern int os_msleep(unsigned int);
extern int os_sleep(unsigned int);

extern char *os_time_out (char *fmt, time_t t);

extern const char *os_build_time2date(char *str);
extern const char *os_date2build_time(char *str);

#define OS_TIMER_FOREVER 0X0FFFFFFF

typedef enum
{
	OS_TIMER_DEFAULE = 0,
	OS_TIMER_ONCE,
}os_time_type;

typedef struct os_time_s
{
	NODE	node;
	int 	t_id;
	int		(*time_entry)(void *);
	void	*pVoid;
	unsigned long msec;
	struct timeval interval;

	os_time_type type;
	enum {OS_TIMER_FALSE, OS_TIMER_TRUE, OS_TIMER_CANCEL} state;
	char    entry_name[OS_TIMER_NAME_MAX];
}os_time_t;


extern int os_time_init();
extern int os_time_exit();
extern int os_time_load();

extern int os_time_clean(BOOL all);
extern os_time_t *os_time_lookup(int id);

extern int os_time_destroy(int id);

extern int os_time_cancel(int id);

extern int os_time_restart(int id, int msec);

extern int os_time_show(int (*show)(void *, char *fmt,...), void *pVoid);
//extern int os_time_create(int (*time_entry)(void *), void *pVoid, int msec);

extern int os_time_create_entry(os_time_type type, int (*time_entry)(void *),
		void *pVoid, int msec, char *func_name);


#define os_time_create(f,p,v)		os_time_create_entry(OS_TIMER_DEFAULE,f,p,v,#f)
#define os_time_create_once(f,p,v)	os_time_create_entry(OS_TIMER_ONCE,f,p,v,#f)

#define quagga_time os_time
#define quagga_gettime os_gettime


#endif /* __OS_TIME_H__ */
