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
		pthread_mutex_t mutex;
		sem_t			sem;
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
extern int os_mutex_lock(os_mutex_t *, zpl_int32 wait);
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
extern int os_sem_take(os_sem_t *, zpl_int32 wait);
extern int os_sem_exit(os_sem_t *);


#else
typedef struct os_sem_s
{
	sem_t  sem;
}os_sem_t;

typedef struct os_mutex_s
{
	pthread_mutex_t mutex;
}os_mutex_t;

typedef struct os_spin_s
{
	pthread_spinlock_t lock;
}os_spin_t;


#define OS_WAIT_NO 			0
#define OS_WAIT_FOREVER 	-1

extern os_sem_t * os_sem_init(void);
extern int os_sem_give(os_sem_t *);
extern int os_sem_take(os_sem_t *, zpl_int32 wait);
extern int os_sem_exit(os_sem_t *);


extern os_mutex_t * os_mutex_init(void);
extern int os_mutex_lock(os_mutex_t *, zpl_int32 wait);
extern int os_mutex_unlock(os_mutex_t *);
extern int os_mutex_exit(os_mutex_t *);


extern os_spin_t * os_spin_init(void);
extern int os_spin_lock(os_spin_t *);
extern int os_spin_unlock(os_spin_t *);
extern int os_spin_exit(os_spin_t *);
#endif


#ifdef __cplusplus
}
#endif


#endif /* __OS_SEM_H__ */
