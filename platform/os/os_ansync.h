/*
 * os_ansync.h
 *
 *  Created on: Sep 3, 2018
 *      Author: zhurish
 */

#ifndef __OS_ANSYNC_H__
#define __OS_ANSYNC_H__

#include "sys/epoll.h"
#include "os_list.h"

//#define __OS_ANSYNC_DEBUG

#define OS_ANSYNC_GLOBAL_LIST

#define OS_ENTRY_NAME_MAX	128
#define OS_EVENTS_MAX		2048

#define OS_EVENTS_TIMER_DEFAULT		5000

#define OS_EVENTS_TIMER_INTERVAL_MAX		0x0fffffff
//#define OS_EPOLL_MSEC	1000


#define OS_ANSYNC_MSEC_MICRO		(1000)
#define OS_ANSYNC_MSEC(n)		(n)
#define OS_ANSYNC_SEC(n)		((n)*OS_ANSYNC_MSEC_MICRO)

#define OS_ANSYNC_TVTOMSEC(n)	(((struct timeval)(n)).tv_sec * OS_ANSYNC_MSEC_MICRO + \
			(((struct timeval)(n)).tv_usec % OS_ANSYNC_MSEC_MICRO))

#define OS_ANSYNC_MSECTOTV(t, n)	((struct timeval)(t)).tv_sec = ( (n) / OS_ANSYNC_MSEC_MICRO); \
			((struct timeval)(t)).tv_usec = ( (n) * OS_ANSYNC_MSEC_MICRO)*OS_ANSYNC_MSEC_MICRO;

typedef enum
{
	OS_ANSYNC_NONE = 0,
	OS_ANSYNC_INPUT,
	OS_ANSYNC_OUTPUT,
	OS_ANSYNC_TIMER,
	OS_ANSYNC_TIMER_ONCE,
	OS_ANSYNC_EVENT,
}os_ansync_type;

typedef enum
{
	OS_ANSYNC_STATE_NONE = 0,
	OS_ANSYNC_STATE_READY,
}os_ansync_state;

typedef enum
{
	OS_ANSYNC_EXECUTE_NONE = 0,
	OS_ANSYNC_EXECUTE_ARGV,
}os_ansync_exe;


typedef int (*os_ansync_cb)(void *);

typedef struct os_ansync_s
{
	NODE			node;
	int				fd;
	os_ansync_type	type;
	os_ansync_cb	ansync_cb;
	void			*pVoid;
	struct timeval	timeout;	//时间轴
	int				interval;	//定时间隔
	char 			entryname[OS_ENTRY_NAME_MAX];
	char 			filename[OS_ENTRY_NAME_MAX];
	int 			line;
	os_ansync_state	state;
}os_ansync_t;


typedef struct
{
	NODE	node;
	LIST	*list;
	LIST	*unuselist;
	void	*mutex;		//for list
	void	*ansync_mutex;	//for execute
	int		module;
	int		taskid;

	struct timeval 	timeout;		//最小定时时间轴 ms
	int 	interval;				//最小定时时间 ms

	int 	epoll_fd;
	int 	max_fd;
	struct epoll_event *events;
	os_ansync_t		*os_ansync;
	BOOL	bquit;
}os_ansync_lst;

#define OS_ANSYNC_FD(n)		(((os_ansync_t *)(n))->fd)
#define OS_ANSYNC_VAL(n)	(((os_ansync_t *)(n))->interval)
#define OS_ANSYNC_ARGV(n)	(((os_ansync_t *)(n))->pVoid)


extern os_ansync_lst *os_ansync_lst_create(int module, int maxfd);
extern int os_ansync_lst_destroy(os_ansync_lst *lst);

extern int os_ansync_lock(os_ansync_lst *lst);
extern int os_ansync_unlock(os_ansync_lst *lst);

extern os_ansync_t * os_ansync_lookup_api(os_ansync_lst *lst, os_ansync_t *value);
extern int os_ansync_add_api(os_ansync_lst *lst, os_ansync_t *value);
extern int os_ansync_del_api(os_ansync_lst *lst, os_ansync_t *value);

extern int os_ansync_timeout_api(os_ansync_lst *lst, int value);

//extern int os_ansync_wait(os_ansync_lst *lst);

extern os_ansync_t *os_ansync_fetch(os_ansync_lst *lst);
extern int os_ansync_execute(os_ansync_lst *lst, os_ansync_t *value, os_ansync_exe exe);

extern int os_ansync_main(os_ansync_lst *lst, os_ansync_exe exe);

extern int os_ansync_fetch_quit (os_ansync_lst *);
extern int os_ansync_fetch_wait (os_ansync_lst *);


#define os_ansync_add(l, t, c, p, v)	_os_ansync_register_api(l, t, c, p, v, #c, __FILE__, __LINE__)
#define os_ansync_add_event(l, c, p)	_os_ansync_register_event_api(l, OS_ANSYNC_EVENT, c, \
																	p, 0, #c, __FILE__, __LINE__)
#define os_ansync_del(l, t, c, p, v)	_os_ansync_unregister_api(l, t, c, p, v)
#define os_ansync_del_all(l, t, c, p, v)	_os_ansync_unregister_all_api(l, t, c, p, v)


#define os_ansync_register_api(l, t, c, p, v)	_os_ansync_register_api(l, t, c, p, v, #c, __FILE__, __LINE__)
#define os_ansync_unregister_api(l, t, c, p, v)	_os_ansync_unregister_api(l, t, c, p, v)
#define os_ansync_unregister_all_api(l, t, c, p, v)	_os_ansync_unregister_all_api(l, t, c, p, v)


extern int _os_ansync_register_api(os_ansync_lst *lst, os_ansync_type type, os_ansync_cb cb,
		void *pVoid, int value, char *func_name, char *file, int line);

extern int _os_ansync_register_event_api(os_ansync_lst *lst, os_ansync_type type, os_ansync_cb cb,
		void *pVoid, int value, char *func_name, char *file, int line);

extern int _os_ansync_unregister_api(os_ansync_lst *lst, os_ansync_type type, os_ansync_cb cb,
		void *pVoid, int value);

extern int _os_ansync_unregister_all_api(os_ansync_lst *lst, os_ansync_type type, os_ansync_cb cb,
		void *pVoid, int value);


extern int os_ansync_show(os_ansync_lst *lst, int (*show)(void *, char *fmt,...), void *pVoid);


#ifdef OS_ANSYNC_GLOBAL_LIST
extern os_ansync_lst * os_ansync_global_lookup(int taskid, int module);
#endif


#ifdef __OS_ANSYNC_DEBUG
extern void os_ansync_debug_printf(void *fp, char *func, int line,  const char *format, ...);
#define OS_ANSYNC_DEBUG(fmt,...)	os_ansync_debug_printf(stderr, __func__, __LINE__,fmt, ##__VA_ARGS__)
#else
#define OS_ANSYNC_DEBUG(fmt,...)

#endif


#endif /* __OS_ANSYNC_H__ */
