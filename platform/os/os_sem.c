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


#ifdef OS_SEM_PROCESS
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define OS_MUTEX_TYPE 1
#define OS_SEM_TYPE 2

#define OS_MUTEX_SEM_ACTIVE 1
#define OS_MUTEX_SEM_INACTIVE 0



typedef struct
{
	int	 shmid;
	pthread_mutex_t mutex;	//对共享内存信号量集的保护
	char shm_cnt;			//记录当前共享内存有几个进程还在使用
	char *p;
}os_mshm_t;

static os_mutex_t	*os_local_mutex = NULL;
static os_mshm_t	*os_local_shm = NULL;

/*************************************************/
int os_mutex_obj_init()
{
	int size = sizeof(os_mutex_t)*OS_MUTEX_SEM_MAX + sizeof(os_mshm_t) + 8;
	int shmid;
	//创建共享内存 ，相当于打开文件，文件不存在则创建
	shmid = shmget(65530, size, IPC_CREAT | 0666);
	if(shmid == -1)
	{
		return ERROR;
	}
	os_local_shm = (os_mshm_t *)shmat(shmid, NULL, 0);

	if(os_local_shm)
	{
		//shm_id = shmid;
		os_local_shm->shmid = shmid;
		os_local_shm->shm_cnt++;	//增加使用该共享内存的计数
		os_local_shm->p = (char *)(os_local_shm + sizeof(os_mshm_t));
		os_local_mutex = (os_mutex_t *)os_local_shm->p;
		if(os_local_shm->shm_cnt == 1)//第一次打开
		{
			//初始化信号量和内存
			pthread_mutexattr_t mattr;
			pthread_mutexattr_init(&mattr);
			pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
			pthread_mutex_init ((pthread_mutex_t  *)&os_local_shm->mutex, &mattr);
			memset(os_local_mutex, 0, sizeof(os_mutex_t)*OS_MUTEX_SEM_MAX);
		}
		return OK;
	}
	return ERROR;
}

int os_mutex_obj_exit()
{
	if(os_local_shm)
	{
		int delete = 0, shmid = os_local_shm->shmid;
		os_local_shm->shm_cnt--;
		if(os_local_shm->shm_cnt == 0)
		{
			memset(os_local_mutex, 0, sizeof(os_mutex_t)*OS_MUTEX_SEM_MAX);
			pthread_mutex_destroy ((pthread_mutex_t  *)&os_local_shm->mutex);
			memset(os_local_shm, 0, sizeof(os_mshm_t));
			delete = 1;
		}
		shmdt(os_local_shm);//把当前共享内存从进程中分离
		if(delete && shmid != -1)
		{
			shmctl(shmid, IPC_RMID, NULL);
		}
		os_local_shm = NULL;
		os_local_mutex = NULL;
		return OK;
	}
	return ERROR;
}
/*************************************************/


static os_mutex_t * os_mutex_sem_lookup(int key, int type)
{
	int i = 0;
	if(!os_local_shm || !os_local_mutex)
		return NULL;
	for(i = 0; i < OS_MUTEX_SEM_MAX; i++)
	{
		if(os_local_mutex[i].key == key &&
		   os_local_mutex[i].type == type &&
		   os_local_mutex[i].init == OS_MUTEX_SEM_ACTIVE)
		   return &os_local_mutex[i];
	}
	return NULL;
}

static os_mutex_t * os_mutex_sem_get_empty()
{
	int i = 0;
	if(!os_local_shm || !os_local_mutex)
		return NULL;
	for(i = 0; i < OS_MUTEX_SEM_MAX; i++)
	{
		if(os_local_mutex[i].key == 0 &&
		   os_local_mutex[i].init == OS_MUTEX_SEM_INACTIVE)
		   return &os_local_mutex[i];
	}
	return NULL;
}


static os_mutex_t * os_mutex_sem_init(int key, int type)
{
	int ret = 0;
	if(!os_local_shm || !os_local_mutex)
		return NULL;
	pthread_mutex_lock(&os_local_shm->mutex);
	os_mutex_t * mutex = os_mutex_sem_lookup(key, type);
	if(mutex)
	{
		mutex->cnt++;
		pthread_mutex_unlock(&os_local_shm->mutex);
		return mutex;
	}
	else
	{
		mutex = os_mutex_sem_get_empty();
		if(mutex)
		{
			mutex->init = OS_MUTEX_SEM_ACTIVE;
			mutex->cnt++;
			mutex->key = key;
			mutex->type = type;
			if(mutex->type == OS_MUTEX_TYPE)
			{
				pthread_mutexattr_t mattr;
				pthread_mutexattr_init(&mattr);
				pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
				ret = pthread_mutex_init ((pthread_mutex_t  *)&mutex->value.mutex, &mattr);
			}
			else
			{
				ret = sem_init(&mutex->value.sem, 1, 0);
			}
			if(ret == 0)
			{
				pthread_mutex_unlock(&os_local_shm->mutex);
				return mutex;
			}
			else
			{
				mutex->init = OS_MUTEX_SEM_INACTIVE;
				mutex->cnt = 0;
				mutex->key = 0;
				mutex->type = 0;
				pthread_mutex_unlock(&os_local_shm->mutex);
				return NULL;
			}

		}
	}
	pthread_mutex_unlock(&os_local_shm->mutex);
	return mutex;
}

static int os_mutex_sem_exit(os_mutex_t * mutex)
{
	if(!os_local_shm || !os_local_mutex || !mutex)
		return ERROR;
	//os_mutex_t * mutex = os_mutex_lookup(key);
	pthread_mutex_lock(&os_local_shm->mutex);
	if(mutex)
	{
		mutex->cnt--;
	}
	if(mutex->cnt == 0)
	{
		if(mutex->type == OS_MUTEX_TYPE)
			pthread_mutex_destroy ((pthread_mutex_t  *)&mutex->value.mutex);
		else
		{
			sem_destroy(&mutex->value.sem);
		}
		mutex->init = OS_MUTEX_SEM_INACTIVE;
		mutex->cnt = 0;
		mutex->key = 0;
		mutex->type = 0;
	}
	pthread_mutex_unlock(&os_local_shm->mutex);
	return OK;
}


static int os_mutex_sem_lock(os_mutex_t * mutex, int wait)
{
	if(!os_local_shm || !os_local_mutex || !mutex)
		return ERROR;
	if(mutex)
	{
		if(mutex->type == OS_MUTEX_TYPE)
		{
			if(wait == OS_WAIT_NO)
				return pthread_mutex_trylock(&mutex->value.mutex);
			else if(wait == OS_WAIT_FOREVER)
				return pthread_mutex_lock(&mutex->value.mutex);
			else
			{
				int ret = 0;
				struct timespec value;
				value.tv_sec=time(NULL) + wait;
				while ((ret = pthread_mutex_timedlock(&mutex->value.mutex, &value)) == -1 && errno == EINTR)
					continue;
				/* Restart if interrupted by handler */
				if(ret == 0)
					return OK;
				else if(errno == ETIMEDOUT)
					return OS_TIMEOUT;//timeout
			}
			return ERROR;
		}
		else
		{
			if(wait == OS_WAIT_NO)
				return sem_trywait(&mutex->value.sem);
			else if(wait == OS_WAIT_FOREVER)
				return sem_wait(&mutex->value.sem);
			else
			{
				int ret = 0;
				struct timespec value;
				value.tv_sec=time(NULL) + wait;
				while ((ret = sem_timedwait(&mutex->value.sem, &value)) == -1 && errno == EINTR)
					continue;
				if(ret == 0)
					return OK;
				else if(errno == ETIMEDOUT)
					return OS_TIMEOUT;
			}
			return ERROR;
		}
	}
	return ERROR;
}

static int os_mutex_sem_unlock(os_mutex_t * mutex)
{
	if(!os_local_shm || !os_local_mutex || !mutex)
		return ERROR;
	if(mutex)
	{
		if(mutex->type == OS_MUTEX_TYPE)
		{
			return pthread_mutex_unlock(((pthread_mutex_t *)&mutex->value.mutex));
		}
		else
		{
			return sem_post(&mutex->value.sem);
		}
	}
	return ERROR;
}



os_mutex_t * os_mutex_init(int key)
{
	return os_mutex_sem_init(key, OS_MUTEX_TYPE);
}

int os_mutex_exit(os_mutex_t * mutex)
{
	return os_mutex_sem_exit(mutex);
}

int os_mutex_lock(os_mutex_t * mutex, int wait)
{
	return os_mutex_sem_lock(mutex, wait);
}

int os_mutex_unlock(os_mutex_t * mutex)
{
	return os_mutex_sem_unlock(mutex);
}


os_sem_t * os_sem_init(int key)
{
	return os_mutex_sem_init(key, OS_SEM_TYPE);
}

int os_sem_exit(os_sem_t * sem)
{
	return os_mutex_sem_exit(sem);
}

int os_sem_take(os_sem_t * sem, int wait)
{
	return os_mutex_sem_lock(sem, wait);
}

int os_sem_give(os_sem_t * sem)
{
	return os_mutex_sem_unlock(sem);
}

#else

os_sem_t * os_sem_init()
{
	os_sem_t *ossem = os_malloc(sizeof(os_sem_t));
	if(ossem)
	{
		if(sem_init(&ossem->sem, 0, 0) == 0)
			return ossem;
		else
		{
			os_free(ossem);
			return NULL;
		}
	}
	return NULL;
}

int os_sem_give(os_sem_t *ossem)
{
	if(ossem)
		return sem_post(&ossem->sem);
	return ERROR;
}

int os_sem_take(os_sem_t *ossem, int wait)
{
	if(ossem == NULL)
		return ERROR;
	if(wait == OS_WAIT_NO)
		return sem_trywait(&ossem->sem);
	else if(wait == OS_WAIT_FOREVER)
		return sem_wait(&ossem->sem);
	else
	{
		int ret = 0;
		struct timespec value;
		value.tv_sec=time(NULL) + wait;

		while ((ret = sem_timedwait(&ossem->sem, &value)) == -1 && errno == EINTR)
			continue;
		/* Restart if interrupted by handler */
		//if(sem_timedwait (&sem->sem, &value)==0)
		///	return OK;
		if(ret == 0)
			return OK;
		else if(errno == ETIMEDOUT)
			return OS_TIMEOUT;
	}
	return ERROR;
}

int os_sem_exit(os_sem_t *ossem)
{
	if(ossem)
		return sem_destroy(&ossem->sem);
	return ERROR;
}


os_mutex_t * os_mutex_init()
{
	os_mutex_t *osmutex = os_malloc(sizeof(os_mutex_t));
	if(osmutex)
	{
		if(pthread_mutex_init(&osmutex->mutex, NULL) == 0)
			return osmutex;
		else
		{
			os_free(osmutex);
			return NULL;
		}
	}
	return NULL;
}


int os_mutex_unlock(os_mutex_t *osmutex)
{
	if(osmutex)
		return pthread_mutex_unlock(&osmutex->mutex);
	return ERROR;
}

int os_mutex_lock(os_mutex_t *osmutex, int wait)
{
	if(osmutex == NULL)
		return ERROR;
	if(wait == OS_WAIT_NO)
		return pthread_mutex_trylock(&osmutex->mutex);
	else if(wait == OS_WAIT_FOREVER)
		return pthread_mutex_lock(&osmutex->mutex);
	else
	{
		int ret = 0;
		struct timespec value;
		value.tv_sec=time(NULL) + wait;

		while ((ret = pthread_mutex_timedlock(&osmutex->mutex, &value)) == -1 && errno == EINTR)
			continue;
		/* Restart if interrupted by handler */
		//if(sem_timedwait (&sem->sem, &value)==0)
		///	return OK;
		if(ret == 0)
			return OK;
		else if(errno == ETIMEDOUT)
			return OS_TIMEOUT;
	}
	return ERROR;
}

int os_mutex_exit(os_mutex_t *osmutex)
{
	if(osmutex)
		return pthread_mutex_destroy(&osmutex->mutex);
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

#endif
