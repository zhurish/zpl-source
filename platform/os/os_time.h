/*
 * os_time.h
 *
 *  Created on: May 30, 2017
 *      Author: zhurish
 */

#ifndef PLATFORM_OS_OS_TIME_H_
#define PLATFORM_OS_OS_TIME_H_


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
extern int os_msleep(unsigned int);
extern int os_sleep(unsigned int);
typedef struct os_time_s
{
	NODE	node;
	int 	t_id;
	int		(*time_entry)(void *);
	void	*pVoid;
	unsigned long msec;
	struct timeval interval;
}os_time_t;


extern int os_time_init();
extern int os_time_exit();
extern int os_time_finsh();
extern int os_time_create(int (*time_entry)(void *), void *pVoid, int msec);
extern int os_time_destroy(int id);

#define quagga_time os_time
#define quagga_gettime os_gettime


#endif /* PLATFORM_OS_OS_TIME_H_ */
