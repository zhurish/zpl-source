/*
 * os_sem.c
 *
 *  Created on: May 30, 2017
 *      Author: zhurish
 */

#include "zebra.h"
#include "os_sem.h"

#include "pthread.h"
#include "semaphore.h"

os_sem_t * os_sem_init()
{
	os_sem_t *sem = os_malloc(sizeof(os_sem_t));
	if(sem)
	{
		if(sem_init(&sem->sem, 0, 0) == 0)
			return sem;
		else
		{
			os_free(sem);
			return NULL;
		}
	}
	return NULL;
}

int os_sem_give(os_sem_t *sem)
{
	if(sem)
		return sem_post(&sem->sem);
	return ERROR;
}

int os_sem_take(os_sem_t *sem, int wait)
{
	if(sem == NULL)
		return ERROR;
	if(wait == OS_WAIT_NO)
		return sem_trywait(&sem->sem);
	else if(wait == OS_WAIT_FOREVER)
		return sem_wait(&sem->sem);
	else
	{
		int ret = 0;
		struct timespec value;
		value.tv_sec=time(NULL) + wait;

		while ((ret = sem_timedwait(&sem->sem, &value)) == -1 && errno == EINTR)
			continue;
		/* Restart if interrupted by handler */
		//if(sem_timedwait (&sem->sem, &value)==0)
		///	return OK;
		if(ret == 0)
			return OK;
		else if(errno == ETIMEDOUT)
			return TIMEOUT;
	}
	return ERROR;
}

int os_sem_exit(os_sem_t *sem)
{
	if(sem)
		return sem_destroy(&sem->sem);
	return ERROR;
}



os_mutex_t * os_mutex_init()
{
	os_mutex_t *mutex = os_malloc(sizeof(os_mutex_t));
	if(mutex)
	{
		if(pthread_mutex_init(&mutex->mutex, NULL) == 0)
			return mutex;
		else
		{
			os_free(mutex);
			return NULL;
		}
	}
	return NULL;
}

int os_mutex_unlock(os_mutex_t *mutex)
{
	if(mutex)
		return pthread_mutex_unlock(&mutex->mutex);
	return ERROR;
}

int os_mutex_lock(os_mutex_t *mutex, int wait)
{
	if(mutex == NULL)
		return ERROR;
	if(wait == OS_WAIT_NO)
		return pthread_mutex_trylock(&mutex->mutex);
	else if(wait == OS_WAIT_FOREVER)
		return pthread_mutex_lock(&mutex->mutex);
	else
	{
		int ret = 0;
		struct timespec value;
		value.tv_sec=time(NULL) + wait;

		while ((ret = pthread_mutex_timedlock(&mutex->mutex, &value)) == -1 && errno == EINTR)
			continue;
		/* Restart if interrupted by handler */
		//if(sem_timedwait (&sem->sem, &value)==0)
		///	return OK;
		if(ret == 0)
			return OK;
		else if(errno == ETIMEDOUT)
			return TIMEOUT;
	}
	return ERROR;
}

int os_mutex_exit(os_mutex_t *mutex)
{
	if(mutex)
		return pthread_mutex_destroy(&mutex->mutex);
	return ERROR;
}


os_spin_t * os_spin_init()
{
	os_spin_t *spin = os_malloc(sizeof(os_spin_t));
	if(spin)
	{
		if(pthread_spin_init(&spin->lock, 0) == 0)
			return spin;
		else
		{
			os_free(spin);
			return NULL;
		}
	}
	return NULL;
}

int os_spin_unlock(os_spin_t *spin)
{
	if(spin)
		return pthread_spin_unlock(&spin->lock);
	return ERROR;
}

int os_spin_lock(os_spin_t *spin)
{
	if(spin == NULL)
		return ERROR;
	return pthread_spin_lock(&spin->lock);
}

int os_spin_exit(os_spin_t *spin)
{
	if(spin)
		return pthread_spin_destroy(&spin->lock);
	return ERROR;
}
