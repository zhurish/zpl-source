/*
 * os_sem.h
 *
 *  Created on: May 30, 2017
 *      Author: zhurish
 */

#ifndef __OS_SEM_H__
#define __OS_SEM_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "zpl_type.h"
#include "pthread.h"
#include "semaphore.h"

//#define OS_SEM_PROCESS
#define OS_LOCK_ERR_CHECK 
//#define OS_LOCK_ERR_CHECK_GRAPH_VIEW

#ifdef OS_SEM_PROCESS

/*
*	进程信号量集的大小，包含互斥和同步信号量的总数量
*/
#define OS_MUTEX_SEM_MAX	1000


typedef struct
{
	zpl_uint16 key;
	zpl_uint8 init;
	zpl_uint8 cnt;		//记录当前信号量有几个进程在使用
	zpl_uint8 type;
	union
	{
		zpl_pthread_mutex_t mutex;
		zpl_sem_t	sem;
	}value;
} os_mutex_t, os_sem_t;


/*
* 进程之间信号量集初始化，进程初始化和进程退出的时候执行
*	进程创建的信号量在进程退出的时候需要销毁
*   进程之间的信号量使用关键字key来标志，key = <1-65536>
*   返回值：
*		0： 成功
*		1： 超时
*		-1：失败
*/
int os_mutex_obj_init(void);
int os_mutex_obj_exit(void);


//#define WAIT_FOREVER -1
#define OS_WAIT_NO 0
#define OS_WAIT_FOREVER -1//WAIT_FOREVER



/*
* 进程之间互斥信号量
*/
extern os_mutex_t * os_mutex_init(zpl_uint32 key);
/*
*	wait:
*		0: 不等待
*	   -1: 永远等待，直到资源可用
*	   >0: 等待 时间 (s)
*/
extern int os_mutex_lock(os_mutex_t *, zpl_int32 wait_ms);
extern int os_mutex_unlock(os_mutex_t *);
extern int os_mutex_exit(os_mutex_t *);
/*
* 进程之间同步信号量
*/
extern os_sem_t * os_sem_init(int key);
/*
*	wait:
*		0: 不等待
*	   -1: 永远等待，直到资源可用
*	   >0: 等待 时间 (s)
*/
extern int os_sem_give(os_sem_t *);
extern int os_sem_take(os_sem_t *, zpl_int32 wait_ms);
extern int os_sem_exit(os_sem_t *);


#else
typedef struct os_sem_s
{
	char *name;
	zpl_sem_t  sem;
	#ifdef OS_LOCK_ERR_CHECK
	zpl_pid_t	self_lock;
	zpl_pid_t	wait_lock;
	#endif
}os_sem_t;

typedef struct os_mutex_s
{
	char *name;
	zpl_pthread_mutex_t mutex;
	#ifdef OS_LOCK_ERR_CHECK
	zpl_pid_t	self_lock;
	zpl_pid_t	wait_lock;
	#endif
}os_mutex_t;

typedef struct os_cond_s
{
	char *name;
	zpl_pthread_cond_t cond_wait;
	zpl_pthread_mutex_t mutex;
	#ifdef OS_LOCK_ERR_CHECK
	zpl_pid_t	self_lock;
	zpl_pid_t	wait_lock;
	#endif
}os_cond_t;

typedef struct os_spin_s
{
	char *name;
	pthread_spinlock_t spinlock;
	#ifdef OS_LOCK_ERR_CHECK
	zpl_pid_t	self_lock;
	zpl_pid_t	wait_lock;
	#endif
}os_spin_t;


#define OS_WAIT_NO 			0
#define OS_WAIT_FOREVER 	-1

#define OS_SEM_DEBUG 	0x01
#define OS_MUTEX_DEBUG 	0x02
#define OS_SPIN_DEBUG 	0x04
//#define OS_LOCK_DETAIL_DEBUG 	0x08


#ifdef OS_LOCK_DETAIL_DEBUG
extern int os_sem_module_debug(int val);

extern os_sem_t * os_sem_init(void);
extern os_sem_t * os_sem_name_init(const char *name);
extern int os_sem_give_entry(os_sem_t *, zpl_char *func, int line);
extern int os_sem_take_entry(os_sem_t *, zpl_int32 wait_ms, zpl_char *func, int line);
#define os_sem_give(n)		os_sem_give_entry(n, __func__, __LINE__)
#define os_sem_take(n, w)	os_sem_take_entry(n, w, __func__, __LINE__)
extern int os_sem_exit(os_sem_t *);


extern os_mutex_t * os_mutex_init(void);
extern os_mutex_t * os_mutex_name_init(const char *name);
extern int os_mutex_lock_entry(os_mutex_t *, zpl_int32 wait_ms, zpl_char *func, int line);
extern int os_mutex_unlock_entry(os_mutex_t *, zpl_char *func, int line);
#define os_mutex_unlock(n)		os_mutex_unlock_entry(n, __func__, __LINE__)
#define os_mutex_lock(n, w)		os_mutex_lock_entry(n, w, __func__, __LINE__)
extern int os_mutex_exit(os_mutex_t *);


extern os_cond_t * os_cond_init(void);
extern os_cond_t * os_cond_name_init(const char *name);
extern int os_cond_exit(os_cond_t *);

int os_cond_signal_entry(os_cond_t *oscond, zpl_char *func, int line);
int os_cond_broadcast_entry(os_cond_t *oscond, zpl_char *func, int line);
int os_cond_wait_entry(os_cond_t *oscond, zpl_int32 wait_ms, zpl_char *func, int line);

#define os_cond_signal(n)		os_cond_signal_entry(n, __func__, __LINE__)
#define os_cond_broadcast(n)	os_cond_broadcast_entry(n, __func__, __LINE__)
#define os_cond_wait(n, w)		os_cond_wait_entry(n, w, __func__, __LINE__)
extern int os_cond_unlock(os_cond_t *);
extern int os_cond_lock(os_cond_t *);

extern os_spin_t * os_spin_init(void);
extern os_spin_t * os_spin_name_init(const char *name);
extern int os_spin_lock_entry(os_spin_t *, zpl_char *func, int line);
extern int os_spin_unlock_entry(os_spin_t *, zpl_char *func, int line);
#define os_spin_lock(n)		os_spin_lock_entry(n, __func__, __LINE__)
#define os_spin_unlock(n)	os_spin_unlock_entry(n, __func__, __LINE__)
extern int os_spin_exit(os_spin_t *);

#else
extern int os_sem_module_debug(int val);

extern os_sem_t * os_sem_init(void);
extern os_sem_t * os_sem_name_init(const char *name);
extern int os_sem_give(os_sem_t *);
extern int os_sem_take(os_sem_t *, zpl_int32 wait_ms);
extern int os_sem_exit(os_sem_t *);


extern os_mutex_t * os_mutex_init(void);
extern os_mutex_t * os_mutex_name_init(const char *name);
extern int os_mutex_lock(os_mutex_t *, zpl_int32 wait_ms);
extern int os_mutex_unlock(os_mutex_t *);
extern int os_mutex_exit(os_mutex_t *);

extern os_cond_t * os_cond_init(void);
extern os_cond_t * os_cond_name_init(const char *name);
extern int os_cond_wait(os_cond_t *, zpl_int32 wait_ms);
extern int os_cond_signal(os_cond_t *);
extern int os_cond_broadcast(os_cond_t *);
extern int os_cond_exit(os_cond_t *);
extern int os_cond_unlock(os_cond_t *);
extern int os_cond_lock(os_cond_t *);


extern os_spin_t * os_spin_init(void);
extern os_spin_t * os_spin_name_init(const char *name);
extern int os_spin_lock(os_spin_t *);
extern int os_spin_unlock(os_spin_t *);
extern int os_spin_exit(os_spin_t *);

#endif
#endif

#ifdef __cplusplus
}
#endif


#endif /* __OS_SEM_H__ */
