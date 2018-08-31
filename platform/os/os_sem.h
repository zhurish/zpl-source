/*
 * os_sem.h
 *
 *  Created on: May 30, 2017
 *      Author: zhurish
 */

#ifndef PLATFORM_OS_OS_SEM_H_
#define PLATFORM_OS_OS_SEM_H_

#include "pthread.h"
#include "semaphore.h"

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

//#define WAIT_FOREVER -1
#define OS_WAIT_NO 0
#define OS_WAIT_FOREVER -1//WAIT_FOREVER

extern os_sem_t * os_sem_init();
extern int os_sem_give(os_sem_t *);
extern int os_sem_take(os_sem_t *, int wait);
extern int os_sem_exit(os_sem_t *);


extern os_mutex_t * os_mutex_init();
extern int os_mutex_lock(os_mutex_t *, int wait);
extern int os_mutex_unlock(os_mutex_t *);
extern int os_mutex_exit(os_mutex_t *);


extern os_spin_t * os_spin_init();
extern int os_spin_lock(os_spin_t *);
extern int os_spin_unlock(os_spin_t *);
extern int os_spin_exit(os_spin_t *);


#endif /* PLATFORM_OS_OS_SEM_H_ */
