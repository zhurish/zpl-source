/*
 * os_sem.c
 *
 *  Created on: May 30, 2017
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "os_sem_errchk.h"
#ifdef OS_SEM_PROCESS
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#endif

#define OS_SEMM_LOG_FILE	RSYSLOGDIR"/mutex.log"

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
	zpl_uint32	 shmid;
	zpl_pthread_mutex_t mutex;	//对共享内存信号量集的保护
	zpl_char shm_cnt;			//记录当前共享内存有几个进程还在使用
	zpl_char *p;
}os_mshm_t;

static os_mutex_t	*os_local_mutex = NULL;
static os_mshm_t	*os_local_shm = NULL;

/*************************************************/
int os_mutex_obj_init(void)
{
	zpl_uint32 size = sizeof(os_mutex_t)*OS_MUTEX_SEM_MAX + sizeof(os_mshm_t) + 8;
	zpl_uint32 shmid;
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
			pthread_mutex_init ((zpl_pthread_mutex_t  *)&os_local_shm->mutex, &mattr);
			memset(os_local_mutex, 0, sizeof(os_mutex_t)*OS_MUTEX_SEM_MAX);
		}
		return OK;
	}
	return ERROR;
}

int os_mutex_obj_exit(void)
{
	if(os_local_shm)
	{
		zpl_uint32 delete = 0, shmid = os_local_shm->shmid;
		os_local_shm->shm_cnt--;
		if(os_local_shm->shm_cnt == 0)
		{
			memset(os_local_mutex, 0, sizeof(os_mutex_t)*OS_MUTEX_SEM_MAX);
			pthread_mutex_destroy ((zpl_pthread_mutex_t  *)&os_local_shm->mutex);
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


static os_mutex_t * os_mutex_sem_lookup(zpl_uint32 key, zpl_uint32 type)
{
	zpl_uint32 i = 0;
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

static os_mutex_t * os_mutex_sem_get_empty(void)
{
	zpl_uint32 i = 0;
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


static os_mutex_t * os_mutex_sem_init(zpl_uint32 key, zpl_uint32 type)
{
	zpl_uint32 ret = 0;
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
				ret = pthread_mutex_init ((zpl_pthread_mutex_t  *)&mutex->value.mutex, &mattr);
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
			pthread_mutex_destroy ((zpl_pthread_mutex_t  *)&mutex->value.mutex);
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


static int os_mutex_sem_lock(os_mutex_t * mutex, zpl_int32 wait_ms)
{
	if(!os_local_shm || !os_local_mutex || !mutex)
		return ERROR;
	if(mutex)
	{
		if(mutex->type == OS_MUTEX_TYPE)
		{
			if(wait_ms == OS_WAIT_NO)
				return pthread_mutex_trylock(&mutex->value.mutex);
			else if(wait_ms == OS_WAIT_FOREVER)
				return pthread_mutex_lock(&mutex->value.mutex);
			else
			{
				int ret = 0;
				struct timespec value;
				value.tv_sec=time(NULL) + (wait_ms/1000);
				value.tv_nsec=(wait_ms%1000)*10000000;
				while ((ret = pthread_mutex_timedlock(&mutex->value.mutex, &value)) == -1 && ipstack_errno == EINTR)
					continue;
				/* Restart if interrupted by handler */
				if(ret == 0)
					return OK;
				else if(ipstack_errno == ETIMEDOUT)
					return OS_TIMEOUT;//timeout
			}
			return ERROR;
		}
		else
		{
			if(wait_ms == OS_WAIT_NO)
				return sem_trywait(&mutex->value.sem);
			else if(wait_ms == OS_WAIT_FOREVER)
				return sem_wait(&mutex->value.sem);
			else
			{
				int ret = 0;
				struct timespec value;
				value.tv_sec=time(NULL) + (wait_ms/1000);
				value.tv_nsec=(wait_ms%1000)*10000000;
				while ((ret = sem_timedwait(&mutex->value.sem, &value)) == -1 && ipstack_errno == EINTR)
					continue;
				if(ret == 0)
					return OK;
				else if(ipstack_errno == ETIMEDOUT)
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
			return pthread_mutex_unlock(((zpl_pthread_mutex_t *)&mutex->value.mutex));
		}
		else
		{
			return sem_post(&mutex->value.sem);
		}
	}
	return ERROR;
}



os_mutex_t * os_mutex_init(zpl_uint32 key)
{
	return os_mutex_sem_init(key, OS_MUTEX_TYPE);
}

int os_mutex_exit(os_mutex_t * mutex)
{
	return os_mutex_sem_exit(mutex);
}
#ifdef OS_LOCK_DETAIL_DEBUG
int os_mutex_lock_entry(os_mutex_t * mutex, zpl_int32 wait_ms, zpl_char *func, int line)
#else
int os_mutex_lock(os_mutex_t * mutex, zpl_int32 wait_ms)
#endif
{
	return os_mutex_sem_lock(mutex, wait_ms);
}
#ifdef OS_LOCK_DETAIL_DEBUG
int os_mutex_unlock_entry(os_mutex_t * mutex, zpl_char *func, int line)
#else
int os_mutex_unlock(os_mutex_t *mutex)
#endif
{
	return os_mutex_sem_unlock(mutex);
}


os_sem_t * os_sem_init(zpl_uint32 key)
{
	return os_mutex_sem_init(key, OS_SEM_TYPE);
}

int os_sem_exit(os_sem_t * sem)
{
	return os_mutex_sem_exit(sem);
}
#ifdef OS_LOCK_DETAIL_DEBUG
int os_sem_take_entry(os_sem_t * sem, zpl_int32 wait_ms, zpl_char *func, int line)
#else
int os_sem_take(os_sem_t *ossem)
#endif
{
	return os_mutex_sem_lock(sem, wait_ms);
}
#ifdef OS_LOCK_DETAIL_DEBUG
int os_sem_give_entry(os_sem_t * sem, zpl_char *func, int line)
#else
int os_sem_give(os_sem_t *ossem)
#endif
{
	return os_mutex_sem_unlock(sem);
}

#else

#define OS_SEM_DEBUG 	0x01
#define OS_MUTEX_DEBUG 	0x02
static int _sem_debug = OS_MUTEX_DEBUG;

int os_sem_module_debug(int val)
{
	_sem_debug = val;
	return OK;
}

static char *os_semmutex_name(char *hdr)
{
	static int sem_count = 0;
	static char nametmp[64];
	os_memset(nametmp, 0, sizeof(nametmp));
	os_sprintf(nametmp, "%s-%d", hdr, sem_count++);
	return nametmp;
}

os_sem_t * os_sem_name_init(const char *name)
{
	os_sem_t *ossem = os_malloc(sizeof(os_sem_t));
	if(ossem)
	{
		os_memset(ossem, 0, sizeof(os_sem_t));
		if(sem_init(&ossem->sem, 0, 0) == 0)
		{
			if(name)
				ossem->name = strdup(name);
			#ifdef OS_LOCK_ERR_CHECK
			task_mutex_graph_add(0, ossem);	
			#endif	
			return ossem;
		}
		else
		{
			os_free(ossem);
			return NULL;
		}
	}
	return NULL;
}

os_sem_t * os_sem_init(void)
{
	os_sem_t * nsem = os_sem_name_init(os_semmutex_name("sem"));
	return nsem;
}
#ifdef OS_LOCK_DETAIL_DEBUG
int os_sem_give_entry(os_sem_t *ossem, zpl_char *func, int line)
#else
int os_sem_give(os_sem_t *ossem)
#endif
{
	int ret = 0;
	if(ossem)
	{
		#ifdef OS_LOCK_DETAIL_DEBUG
		os_log_info("sem '%s' give on %s[%d]", ossem->name, func, line);
		#endif
		ret = sem_post(&ossem->sem);
		if(ret == 0)
		{
			#ifdef OS_LOCK_ERR_CHECK
			ossem->wait_lock = 0;
			ossem->self_lock = 0;
			#endif
		}
		return ret;
	}
	return ERROR;
}
#ifdef OS_LOCK_DETAIL_DEBUG
int os_sem_take_entry(os_sem_t *ossem, zpl_int32 wait_ms, zpl_char *func, int line)
#else
int os_sem_take(os_sem_t *ossem, zpl_int32 wait_ms)
#endif
{
	int ret = 0;
	if(ossem == NULL)
		return ERROR;
	#ifdef OS_LOCK_DETAIL_DEBUG
	os_log_info("sem '%s' take on %s[%d]", ossem->name, func, line);
	#endif	
	#ifdef OS_LOCK_ERR_CHECK	
	ossem->wait_lock = os_task_gettid();
	#endif
	if(wait_ms == OS_WAIT_NO)
		ret = sem_trywait(&ossem->sem);
	else if(wait_ms == OS_WAIT_FOREVER)
		ret = sem_wait(&ossem->sem);
	else
	{
		struct timespec value;
		value.tv_sec=time(NULL) + (wait_ms/1000);
		value.tv_nsec=(wait_ms%1000)*10000000;

		while ((ret = sem_timedwait(&ossem->sem, &value)) == -1 && 
			(ipstack_errno == EINTR || ipstack_errno == EAGAIN))
			continue;

		if(ret == 0)
		{
			#ifdef OS_LOCK_ERR_CHECK
			ossem->wait_lock = 0;
			ossem->self_lock = os_task_gettid();
			#endif
			return OK;
		}
		else if(ipstack_errno == ETIMEDOUT)
		{
			#ifdef OS_LOCK_ERR_CHECK
			ossem->self_lock = 0;
			#endif
			return OS_TIMEOUT;
		}
	}
	if(ret == 0)
	{
		#ifdef OS_LOCK_ERR_CHECK
		ossem->wait_lock = 0;
		ossem->self_lock = os_task_gettid();
		#endif
		return OK;
	}
	return ERROR;
}

int os_sem_exit(os_sem_t *ossem)
{
	if(ossem)
	{
		#ifdef OS_LOCK_ERR_CHECK
		task_mutex_graph_del(0, ossem);	
		#endif
		sem_destroy(&ossem->sem);
		if(ossem->name)
		{
			os_free(ossem->name);
			ossem->name = NULL;
		}
		os_free(ossem);
		ossem = NULL;
		return OK;
	}	
	return ERROR;
}


os_mutex_t * os_mutex_name_init(const char *name)
{
	os_mutex_t *osmutex = os_malloc(sizeof(os_mutex_t));
	if(osmutex)
	{
		pthread_mutexattr_t _mutexattr;
		os_memset(osmutex, 0, sizeof(os_mutex_t));
		pthread_mutexattr_init(&_mutexattr);
		pthread_mutexattr_setpshared(&_mutexattr, PTHREAD_PROCESS_PRIVATE);
		//PTHREAD_MUTEX_NORMAL 不检测死锁
		//PTHREAD_MUTEX_RECURSIVE 嵌套锁
		pthread_mutexattr_settype(&_mutexattr, PTHREAD_MUTEX_ERRORCHECK);
		if(pthread_mutex_init(&osmutex->mutex, &_mutexattr) == 0)
		{
			pthread_mutexattr_destroy(&_mutexattr);
			if(name)
				osmutex->name = strdup(name);
			#ifdef OS_LOCK_ERR_CHECK
			task_mutex_graph_add(1, osmutex);	
			#endif			
			return osmutex;
		}
		else
		{
			pthread_mutexattr_destroy(&_mutexattr);
			os_free(osmutex);
			return NULL;
		}
	}
	return NULL;
}


os_mutex_t * os_mutex_init(void)
{
	os_mutex_t * osmutex = os_mutex_name_init(os_semmutex_name("mutex"));
	return osmutex;
}
#ifdef OS_LOCK_DETAIL_DEBUG
int os_mutex_unlock_entry(os_mutex_t *osmutex, zpl_char *func, int line)
#else
int os_mutex_unlock(os_mutex_t *osmutex)
#endif
{
	int ret = 0;
	if(osmutex)
	{
	#ifdef OS_LOCK_DETAIL_DEBUG
	os_log_info("mutex '%s' unlock on %s[%d]", osmutex->name, func, line);
	#endif			
		if(_sem_debug & OS_MUTEX_DEBUG)
		{
			os_log(OS_SEMM_LOG_FILE, "unlock mutex '%s'", osmutex->name);
		}		
		ret = pthread_mutex_unlock(&osmutex->mutex);
		if(ret == 0)
		{
			#ifdef OS_LOCK_ERR_CHECK
			osmutex->wait_lock = 0;
			osmutex->self_lock = 0;
			#endif
			#ifdef OS_LOCK_ERR_CHECK_GRAPH_VIEW
			task_mutex_graph_unlock_after(os_task_gettid(), osmutex);
			#endif
		}
		return ret;
	}
	return ERROR;
}
#ifdef OS_LOCK_DETAIL_DEBUG
int os_mutex_lock_entry(os_mutex_t *osmutex, zpl_int32 wait_ms, zpl_char *func, int line)
#else
int os_mutex_lock(os_mutex_t *osmutex, zpl_int32 wait_ms)
#endif
{
	int ret = 0;
	if(osmutex == NULL)
		return ERROR;
	#ifdef OS_LOCK_DETAIL_DEBUG
	os_log_info("mutex '%s' lock on %s[%d]", osmutex->name, func, line);
	#endif			
	if(_sem_debug & OS_MUTEX_DEBUG)
	{
		os_log(OS_SEMM_LOG_FILE, "lock mutex '%s'", osmutex->name);
	}
	#ifdef OS_LOCK_ERR_CHECK
	osmutex->wait_lock = os_task_gettid();
	#endif
	#ifdef OS_LOCK_ERR_CHECK_GRAPH_VIEW
	task_mutex_graph_lock_before(os_task_gettid(), osmutex);
	#endif
	if(wait_ms == OS_WAIT_NO)
		ret = pthread_mutex_trylock(&osmutex->mutex);
	else if(wait_ms == OS_WAIT_FOREVER)
		ret = pthread_mutex_lock(&osmutex->mutex);
	else
	{
		struct timespec value;
		value.tv_sec=time(NULL) + (wait_ms/1000);
		value.tv_nsec=(wait_ms%1000)*10000000;

		while ((ret = pthread_mutex_timedlock(&osmutex->mutex, &value)) == -1 && 
			(ipstack_errno == EINTR || ipstack_errno == EAGAIN))
			continue;

		if(ret == 0)
		{
			#ifdef OS_LOCK_ERR_CHECK
			osmutex->wait_lock = 0;
			osmutex->self_lock = os_task_gettid();
			#endif
			#ifdef OS_LOCK_ERR_CHECK_GRAPH_VIEW
			task_mutex_graph_lock_after(os_task_gettid(), osmutex);
			#endif
			return OK;
		}
		else if(ipstack_errno == ETIMEDOUT)
		{
			#ifdef OS_LOCK_ERR_CHECK
			osmutex->self_lock = 0;
			#endif
			return OS_TIMEOUT;
		}
	}
	if(ret == 0)
	{
		#ifdef OS_LOCK_ERR_CHECK
		osmutex->wait_lock = 0;
		osmutex->self_lock = os_task_gettid();
		#endif
		#ifdef OS_LOCK_ERR_CHECK_GRAPH_VIEW
		task_mutex_graph_lock_after(os_task_gettid(), osmutex);
		#endif
		return OK;
	}
	return ERROR;
}

int os_mutex_exit(os_mutex_t *osmutex)
{
	if(osmutex)
	{
		pthread_mutex_destroy(&osmutex->mutex);
		#ifdef OS_LOCK_ERR_CHECK
		task_mutex_graph_del(1, osmutex);
		#endif
		if(osmutex->name)
		{
			os_free(osmutex->name);
			osmutex->name = NULL;
		}
		os_free(osmutex);
		osmutex = NULL;
		return OK;
	}
	return ERROR;
}


os_cond_t * os_cond_name_init(const char *name)
{
	os_cond_t *oscond = os_malloc(sizeof(os_cond_t));
	if(oscond)
	{
		os_memset(oscond, 0, sizeof(os_cond_t));
		if(pthread_cond_init(&oscond->cond_wait, NULL) == 0)
		{
			pthread_mutex_init(&oscond->mutex, NULL);
			if(name)
				oscond->name = strdup(name);
			return oscond;
		}
		else
		{
			os_free(oscond);
			return NULL;
		}
	}
	return NULL;
}

os_cond_t * os_cond_init(void)
{
	os_cond_t * oscond = os_cond_name_init(os_semmutex_name("cond"));
	return oscond;
}

int os_cond_unlock(os_cond_t *oscond)
{
	if(oscond == NULL)
		return ERROR;
	return pthread_mutex_unlock(&oscond->mutex);
}

int os_cond_lock(os_cond_t *oscond)
{
	if(oscond == NULL)
		return ERROR;
	return pthread_mutex_lock(&oscond->mutex);
}

#ifdef OS_LOCK_DETAIL_DEBUG
int os_cond_signal_entry(os_cond_t *oscond, zpl_char *func, int line)
#else
int os_cond_signal(os_cond_t *oscond)
#endif
{
	if(oscond)
	{
	#ifdef OS_LOCK_DETAIL_DEBUG
	os_log_info("mutex '%s' unlock on %s[%d]", oscond->name, func, line);
	#endif			

		if(_sem_debug & OS_MUTEX_DEBUG)
		{
			os_log(OS_SEMM_LOG_FILE, "unlock mutex '%s'", oscond->name);
		}		
		return pthread_cond_signal(&oscond->cond_wait);
	}
	return ERROR;
}

#ifdef OS_LOCK_DETAIL_DEBUG
int os_cond_broadcast_entry(os_cond_t *oscond, zpl_char *func, int line)
#else
int os_cond_broadcast(os_cond_t *oscond)
#endif
{
	if(oscond)
	{
	#ifdef OS_LOCK_DETAIL_DEBUG
	os_log_info("mutex '%s' unlock on %s[%d]", oscond->name, func, line);
	#endif			

		if(_sem_debug & OS_MUTEX_DEBUG)
		{
			os_log(OS_SEMM_LOG_FILE, "unlock mutex '%s'", oscond->name);
		}		
		return pthread_cond_broadcast(&oscond->cond_wait);
	}
	return ERROR;
}

#ifdef OS_LOCK_DETAIL_DEBUG
int os_cond_wait_entry(os_cond_t *oscond, zpl_int32 wait_ms, zpl_char *func, int line)
#else
int os_cond_wait(os_cond_t *oscond, zpl_int32 wait_ms)
#endif
{
	if(oscond == NULL)
		return ERROR;
	//pthread_mutex_lock(&oscond->mutex);	
	#ifdef OS_LOCK_DETAIL_DEBUG
	os_log_info("mutex '%s' lock on %s[%d]", oscond->name, func, line);
	#endif			
	if(_sem_debug & OS_MUTEX_DEBUG)
	{
		os_log(OS_SEMM_LOG_FILE, "lock mutex '%s'", oscond->name);
	}
	if(wait_ms == OS_WAIT_FOREVER)
		return pthread_cond_wait(&oscond->cond_wait, &oscond->mutex);
	else
	{
		int ret = 0;
		struct timespec value;
		if(wait_ms == OS_WAIT_NO)
		{
			value.tv_sec=time(NULL);
			value.tv_nsec=1;
		}
		else
		{
			value.tv_sec=time(NULL) + (wait_ms/1000);
			value.tv_nsec=(wait_ms%1000)*10000000;
		}
		while ((ret = pthread_cond_timedwait(&oscond->cond_wait, &oscond->mutex, &value)) == -1 && 
			(ipstack_errno == EINTR || ipstack_errno == EAGAIN))
			continue;
		if(ret == 0)
			return OK;
		else if(ipstack_errno == ETIMEDOUT)
			return OS_TIMEOUT;
	}
	return ERROR;
}

int os_cond_exit(os_cond_t *oscond)
{
	if(oscond)
	{
		pthread_cond_destroy(&oscond->cond_wait);
		pthread_mutex_destroy(&oscond->mutex);
		if(oscond->name)
		{
			os_free(oscond->name);
			oscond->name = NULL;
		}
		os_free(oscond);
		oscond = NULL;
		return OK;
	}
	return ERROR;
}

os_spin_t * os_spin_name_init(const char *name)
{
	os_spin_t *spin = os_malloc(sizeof(os_spin_t));
	if(spin)
	{
		os_memset(spin, 0, sizeof(os_spin_t));
		if(pthread_spin_init(&spin->spinlock, 0) == 0)
		{
			if(name)
				spin->name = strdup(name);
			#ifdef OS_LOCK_ERR_CHECK
			task_mutex_graph_add(2, spin);	
			#endif
			return spin;
		}
		else
		{
			os_free(spin);
			return NULL;
		}
	}
	return NULL;
}

os_spin_t * os_spin_init(void)
{
	os_spin_t * spin = os_spin_name_init(os_semmutex_name("spin"));
	return spin;
}
#ifdef OS_LOCK_DETAIL_DEBUG
int os_spin_unlock_entry(os_spin_t *spin, zpl_char *func, int line)
#else
int os_spin_unlock(os_spin_t *spin)
#endif
{
	if(spin)
	{
	#ifdef OS_LOCK_DETAIL_DEBUG
	os_log_info("spin '%s' unlock on %s[%d]", spin->name, func, line);
	#endif			
		if(_sem_debug & OS_SPIN_DEBUG)
		{
			os_log(OS_SEMM_LOG_FILE, "unlock spin '%s'", spin->name);
		}			
		#ifdef OS_LOCK_ERR_CHECK
		spin->self_lock = 0;
		#endif
		return pthread_spin_unlock(&spin->spinlock);
	}
	return ERROR;
}
#ifdef OS_LOCK_DETAIL_DEBUG
int os_spin_lock_entry(os_spin_t *spin, zpl_char *func, int line)
#else
int os_spin_lock(os_spin_t *spin)
#endif
{
	if(spin == NULL)
		return ERROR;
	#ifdef OS_LOCK_DETAIL_DEBUG
	os_log_info("spin '%s' lock on %s[%d]", spin->name, func, line);
	#endif			
	if(_sem_debug & OS_SPIN_DEBUG)
	{
		os_log(OS_SEMM_LOG_FILE, "lock spin '%s'", spin->name);
	}
	#ifdef OS_LOCK_ERR_CHECK
	spin->self_lock = os_task_gettid();
	#endif
	return pthread_spin_lock(&spin->spinlock);
}

int os_spin_exit(os_spin_t *spin)
{
	if(spin)
	{
		#ifdef OS_LOCK_ERR_CHECK
		task_mutex_graph_del(2, spin);	
		#endif
		pthread_spin_destroy(&spin->spinlock);
		if(spin->name)
		{
			os_free(spin->name);
			spin->name = NULL;
		}
		os_free(spin);
		spin = NULL;
		return OK;
	}
	return ERROR;
}

#endif
