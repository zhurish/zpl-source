/*
 * os_task.c
 *
 *  Created on: May 30, 2017
 *      Author: zhurish
 */

#include "zebra.h"
#include "pthread.h"
#include "semaphore.h"
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <dirent.h>
#include "command.h"

#include "os_job.h"
#include "os_list.h"
#include "os_sem.h"
#include "os_time.h"
#include "os_task.h"
#include "vty.h"

static LIST *os_task_list = NULL;
static os_mutex_t *task_mutex = NULL;
static int os_moniter_id = 0;
struct os_task_history total_cpu;

#ifndef USE_IPSTACK_KERNEL
extern int ipcom_os_task_tcb_entry_create_cb(char *name, int pid,
		char *func_name);
extern int ipcom_os_task_tcb_entry_delete_cb(int pid);
#endif

int os_task_init()
{
	if (task_mutex == NULL)
	{
		task_mutex = os_mutex_init();
	}
	if (os_task_list == NULL)
	{
		os_task_list = os_malloc(sizeof(LIST));
		if (os_task_list)
		{
			os_memset(&total_cpu, 0, sizeof(total_cpu));
#ifndef __UCLIBC__
			total_cpu.cpu = get_nprocs();
#endif
			lstInit(os_task_list);
			return OK;
		}
		os_mutex_exit(task_mutex);
		return ERROR;
	}
	return OK;
}


int os_task_exit()
{
	if (task_mutex)
	{
		os_mutex_lock(task_mutex, OS_WAIT_FOREVER);
		os_mutex_exit(task_mutex);
		if (os_task_list)
			lstFree(os_task_list);
		return OK;
	}
	return OK;
}

static os_task_t * os_task_lookup(unit32 id, pthread_t pid)
{
	NODE *node;
	os_task_t *task;
	if (os_task_list == NULL)
		return NULL;
	if (task_mutex)
		os_mutex_lock(task_mutex, OS_WAIT_FOREVER);
	node = lstFirst(os_task_list);
	while (node)
	{
		if (node)
		{
			task = (os_task_t *) node;
			if ((id != 0) && id == task->td_id)
			{
				if (task_mutex)
					os_mutex_unlock(task_mutex);
				return task;
			}
			else if (((pthread_t) pid != 0)
					&& (pthread_t) pid == (pthread_t) task->td_thread)
			{
				if (task_mutex)
					os_mutex_unlock(task_mutex);
				return task;
			}
		}
		node = lstNext(node);
	}
	if (task_mutex)
		os_mutex_unlock(task_mutex);
	return NULL;
}

static unsigned int os_task_name_hash_key(const char *str)
{
	unsigned int hash = 0;
	const char *p = str;
	while (*p)
		hash = (hash * 33) ^ (unsigned int) *p++;

	return hash;
}

static os_task_t * os_task_node_lookup_by_name(char *task_name)
{
	NODE *node;
	os_task_t *task;
	if (os_task_list == NULL)
		return NULL;
	if (task_mutex)
		os_mutex_lock(task_mutex, OS_WAIT_FOREVER);
	node = lstFirst(os_task_list);
	while (node)
	{
		if (node)
		{
			task = (os_task_t *) node;
			if (task->td_name_key == os_task_name_hash_key(task_name))
			{
				if (task_mutex)
					os_mutex_unlock(task_mutex);
				return task;
			}
		}
		node = lstNext(node);
	}
	if (task_mutex)
		os_mutex_unlock(task_mutex);
	return NULL;
}

unit32 os_task_lookup_by_name(char *task_name)
{
	os_task_t *task = os_task_node_lookup_by_name(task_name);
	if (task)
		return task->td_id;
	return ERROR;
}

pthread_t os_task_pthread_self(void)
{
	return pthread_self();
}

unit32 os_task_id_self(void)
{
	os_task_t *task = os_task_lookup(0, os_task_pthread_self());
	if (task)
	{
		return task->td_id;
	}
	return ERROR;
}

/*
 int os_task_tid_self( void)
 {
 os_task_t *task = os_task_lookup(0, os_task_pthread_self());
 if(task)
 {
 if(task_mutex)
 os_mutex_lock(task_mutex, OS_WAIT_FOREVER);
 task->td_tid = syscall(SYS_gettid);
 if(task_mutex)
 os_mutex_unlock(task_mutex);
 return OK;
 }
 return ERROR;
 }
 */

int os_task_gettid(void)
{
	return syscall(SYS_gettid);
}

int os_task_name_get(unit32 task_id, char *task_name)
{
	os_task_t *task = (os_task_t *) task_id;
	if (task)
	{
		if (task_mutex)
			os_mutex_lock(task_mutex, OS_WAIT_FOREVER);
		if (task_name)
		{
			os_strcpy(task_name, task->td_name);
			if (task_mutex)
				os_mutex_unlock(task_mutex);
			return OK;
		}
		if (task_mutex)
			os_mutex_unlock(task_mutex);
	}
	return ERROR;
}

char * os_task_2_name(unit32 task_id)
{
	static char name[TASK_NAME_MAX];
	os_memset(name, 0, sizeof(name));
	if (os_task_name_get(task_id, name) == OK)
		return name;
	return "Unknow";
}

int os_task_priority_set(unit32 TaskID, int Priority)
{
	int policy; //,pri;
	struct sched_param sp;
	os_task_t *osapiTask = (os_task_t *) TaskID;
	if (osapiTask)
	{
		if (task_mutex)
			os_mutex_lock(task_mutex, OS_WAIT_FOREVER);
		pthread_mutex_lock(&(osapiTask->td_mutex));
//		pri = osapiTask->td_priority;

		if (Priority == OS_TASK_DEFAULT_PRIORITY)
		{
			policy = SCHED_OTHER;
			osapiTask->td_priority = 0;
		}
		else
		{
			if (osapiTask->td_options & OS_TASK_TIME_SLICED)
				policy = SCHED_RR;
			else
				policy = SCHED_FIFO;

			osapiTask->td_priority = policy;
		}
		osapiTask->td_priority = Priority;
		osapiTask->td_old_priority = policy;
		sp.sched_priority = osapiTask->td_priority;
		pthread_mutex_unlock(&(osapiTask->td_mutex));
		if (task_mutex)
			os_mutex_unlock(task_mutex);
		pthread_setschedparam(osapiTask->td_thread, policy, &sp);
		return (OK);
	}
	return ERROR;
}

int os_task_priority_get(unit32 TaskID, int *Priority)
{
	os_task_t *osapiTask = (os_task_t *) TaskID;
	if (osapiTask)
	{
		if (task_mutex)
			os_mutex_lock(task_mutex, OS_WAIT_FOREVER);
		if (Priority)
			*Priority = osapiTask->td_priority;
		if (task_mutex)
			os_mutex_unlock(task_mutex);
		return (OK);
	}
	return ERROR;
}

int os_task_supend(unit32 TaskID)
{
	os_task_t *osapiTask = (os_task_t *) TaskID;
	if (osapiTask)
	{
		if (task_mutex)
			os_mutex_lock(task_mutex, OS_WAIT_FOREVER);
		pthread_kill(osapiTask->td_thread, SIGSTOP);
		if (task_mutex)
			os_mutex_unlock(task_mutex);
		return (OK);
	}
	return ERROR;
}

int os_task_resume(unit32 TaskID)
{
	os_task_t *osapiTask = (os_task_t *) TaskID;
	if (osapiTask)
	{
		if (task_mutex)
			os_mutex_lock(task_mutex, OS_WAIT_FOREVER);
		pthread_kill(osapiTask->td_thread, SIGCONT);
		if (task_mutex)
			os_mutex_unlock(task_mutex);
		return (OK);
	}
	return ERROR;
}

int os_task_yield(void)
{
#ifdef L7_LINUX_24
	os_task_t *self = os_task_id_self();
	int sched_policy;

	pthread_attr_getschedpolicy(&(self->attr), &sched_policy);

	if ((SCHED_FIFO == sched_policy) || (SCHED_RR == sched_policy))
	{
		sched_yield();
	}
#else

#ifdef __USE_GNU
	pthread_yield();
#endif
	sched_yield();
#endif
	return (OK);
}

/*
 * 5ms is one tick
 */
int os_task_delay(int ticks)
{
	struct timespec timereq, timerem;
	int usec;
	/* don't really need to check for "interrupt context" */
	usec = ticks * OS_TASK_TICK_MSEC; //OS_TASK_TICK_USEC
	timereq.tv_sec = ((usec >= 1000000) ? (usec / 1000000) : 0);
	timereq.tv_nsec = (
			(usec >= 1000000) ? ((usec % 1000000) * 1000) : (usec * 1000));
	if (usec > 0)
	{
		for (;;)
		{
			pthread_testcancel();

			if (nanosleep(&timereq, &timerem) != 0)
			{
				timereq = timerem;
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		sched_yield();
	}
	return (OK);
}

static int os_task_state_to_string(enum os_task_state st, char *state)
{
	if (state)
	{
		switch (st)
		{
		case OS_STATE_READY: //R
			strcat(state, "R");
			break;
		case OS_STATE_RUNNING: //R
			strcat(state, "R");
			break;
		case OS_STATE_WAIT: //W
			strcat(state, "W");
			break;
		case OS_STATE_SLEEP: //S
			strcat(state, "S");
			break;
		case OS_STATE_STOP: //T
			strcat(state, "T");
			break;
		case OS_STATE_DEAD: //D
			strcat(state, "D");
			break;
		case OS_STATE_ZOMBIE: //Z
			strcat(state, "Z");
			break;
		default:
			strcat(state, "U");
			break;
		}
	}
	return 0;
}

static int os_task_string_to_state(char *state, enum os_task_state *st)
{
	if (state)
	{
		if (st)
		{
			if (os_strstr(state, "R"))
				*st = OS_STATE_READY;
			else if (os_strstr(state, "R"))
				*st = OS_STATE_RUNNING;
			else if (os_strstr(state, "W"))
				*st = OS_STATE_WAIT;
			else if (os_strstr(state, "S"))
				*st = OS_STATE_SLEEP;
			else if (os_strstr(state, "T"))
				*st = OS_STATE_STOP;
			else if (os_strstr(state, "D"))
				*st = OS_STATE_DEAD;
			else if (os_strstr(state, "Z"))
				*st = OS_STATE_ZOMBIE;
		}
	}
	return 0;
}

static int os_task_refresh_total_cpu(struct os_task_history *hist)
{
	char path[128];
	char buf[1024];
	FILE *fp = NULL;
	int ret = 0;
	os_memset(path, 0, sizeof(path));
	os_memset(buf, 0, sizeof(buf));
	sprintf(path, "/proc/stat");
#ifndef __UCLIBC__
	if (hist->cpu == 0)
		hist->cpu = get_nprocs();
#endif
	fp = fopen(path, "r");
	if (fp)
	{
		int user, nice, system, idle, iowait, irq, softirq, stealstolen, guest;
		ret = fgets(buf, sizeof(buf), fp);
		if (ret)
		{
			sscanf(buf, "%*s %d %d %d %d %d %d %d %d %d", &user, &nice, &system,
					&idle, &iowait, &irq, &softirq, &stealstolen, &guest);
			hist->total = (user + nice + system + idle + iowait + irq + softirq
					+ stealstolen + guest);
			hist->total_realy = (user + nice + system + idle + iowait + irq
					+ softirq + stealstolen + guest) - hist->total_realy;
//			OS_DEBUG("total cpu(%f)\r\n",100*(float)(hist->total-idle)/hist->total);
		}
		else
			zlog_err(ZLOG_DEFAULT, "can't read path:%s(%s)\r\n", path,
					strerror(errno));
		fclose(fp);
	}
	else
		printf("can't open path:%s\r\n", path);
	return 0;
}

static int os_task_refresh_cpu(os_task_t *task)
{
	char path[128];
	char buf[4094];
	FILE *fp = NULL;
	int ret = 0;
	os_memset(path, 0, sizeof(path));
	os_memset(buf, 0, sizeof(buf));
#ifndef __UCLIBC__
	if (task->hist.cpu == 0)
		task->hist.cpu = get_nprocs();
#endif
	if (task->td_tid)
		sprintf(path, "/proc/%d/task/%d/stat", task->td_pid, task->td_tid);
	else
		sprintf(path, "/proc/%d/stat", task->td_pid);
	fp = fopen(path, "r");
	if (fp)
	{
		char sta[16];
		int utime, stime, cutime, cstime, start_time, priority, nice;

		ret = fread(buf, 1, sizeof(buf), fp);
		if (ret)
		{
			sscanf(buf, "%*d %*s %s %*d %*d %*d %*d %*d %*d %*d %*d %*d "
					"%d %d %d %d %d %d %*d %*d %d", sta, &utime, &stime,
					&cutime, &cstime, &priority, &nice, &start_time);

//			OS_DEBUG("%s:%s=%s %d %d %d %d %d %d %d %d\r\n",__func__,task->td_name,sta,utime,stime,cutime,
//					cstime,priority,nice,start_time);
//			OS_DEBUG("thread total cpu(%d)\r\n",utime + stime + cutime + cstime);

			os_task_string_to_state(sta, &task->state);
			task->hist.total_realy = (utime + stime + cutime + cstime)
					- task->hist.total_realy;
			task->hist.total = (utime + stime + cutime + cstime)
					- task->hist.total;

		}
		else
			zlog_err(ZLOG_DEFAULT, "can't read path:%s(%s)\r\n", path,
					strerror(errno));
		fclose(fp);
	}
	else
		printf("can't open path:%s\r\n", path);
	return 0;
}

static int os_task_refresh_time(void *argv)
{
//	if(os_job_finsh() == OK)
	{
		NODE *node = NULL;
		os_task_t *task;
		if (os_task_list == NULL)
			return ERROR;
		os_task_refresh_total_cpu(&total_cpu);
		if (task_mutex)
			os_mutex_lock(task_mutex, OS_WAIT_FOREVER);
		node = lstFirst(os_task_list);
		while (node)
		{
			if (node)
			{
				task = (os_task_t *) node;
				if (task->td_tid != 0)
				{
					os_task_refresh_cpu(task);
				}
			}
			node = lstNext(node);
		}
		if (task_mutex)
			os_mutex_unlock(task_mutex);
	}
	return OK;
}

#ifndef OS_TASK_DEBUG
static int os_task_refresh_tid_one(char *dirpath)
{
	DIR *dir;
	struct dirent *ptr;
	FILE *fp;
	os_task_t *task = NULL;
	int tid = 0;
	char filepath[128];
	char cur_task_name[64];
	dir = opendir(dirpath);
	if (NULL != dir)
	{
		while ((ptr = readdir(dir)) != NULL)
		{

			if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
			continue;
			if (DT_DIR != ptr->d_type)
			continue;
			os_memset(filepath, 0, sizeof(filepath));
			sprintf(filepath, "/%s/%s/comm", dirpath,ptr->d_name);
			fp = fopen(filepath, "r");
			if (NULL != fp)
			{
				os_memset(cur_task_name, 0, sizeof(cur_task_name));
				fscanf(fp, "%s", cur_task_name);
				task = os_task_node_lookup_by_name(cur_task_name);
				if(task)
				{
					if(task->td_tid)
					{
						fclose(fp);
						continue;
					}
					if(task_mutex)
					os_mutex_lock(task_mutex, OS_WAIT_FOREVER);
					if(task)
					{
						tid = atol(ptr->d_name);
						task->td_tid = (pid_t)tid;
					}
					if(task_mutex)
					os_mutex_unlock(task_mutex);
				}
				fclose(fp);
			}
		}
		closedir(dir);
	}
	return 0;
}

static int os_task_refresh_tid_task(void *p)
{
	char path_name[TASK_NAME_MAX];
	os_memset(path_name, 0, sizeof(path_name));
	sprintf(path_name, "/proc/%d/task", getpid());
	os_task_refresh_tid_one(path_name);
	return 0;
}

static int os_task_finsh_job(void)
{
	if(os_job_finsh() == OK)
	{
		int addjob = 0;
		NODE *node;
		os_task_t *task;
		if(os_task_list == NULL)
		return ERROR;
		if(task_mutex)
		os_mutex_lock(task_mutex, OS_WAIT_FOREVER);
		node = lstFirst(os_task_list);
		while(node)
		{
			if(node)
			{
				task = (os_task_t *)node;
				if(task->td_tid== 0)
				{
					addjob = 1;
					break;
				}
			}
			node = lstNext(node);
		}
		if(task_mutex)
		os_mutex_unlock(task_mutex);
		if(addjob)
		return os_job_add(os_task_refresh_tid_task, NULL);
	}
	return 0;
}
#endif

static int os_task_tcb_entry_destroy(os_task_t *task)
{
	//sem_destroy(&(task->td_sem));
	pthread_mutex_destroy(&(task->td_mutex));
	pthread_cond_destroy(&(task->td_control));
	pthread_attr_destroy(&(task->td_attr));
	//pthread_spin_destroy(&(task->td_spinlock));
	//pthread_spin_destroy(&(task->td_param));
	os_free(task);
	return OK;
}

int os_task_entry_destroy(os_task_t *task)
{
#ifndef USE_IPSTACK_KERNEL
	ipcom_os_task_tcb_entry_delete_cb(task->td_thread);
#endif
	if (task->td_thread == pthread_self())
	{
		/*
		 if (pthread_mutex_lock(&zombie_tasks_lock) != 0)
		 {
		 osapi_printf("osapiTaskDelete: zombie_tasks_lock error\n");
		 }
		 pthread_mutex_unlock(&zombie_tasks_lock);
		 pthread_cond_signal(&zombie_tasks_cond);
		 */
		if (os_task_list == NULL)
			return ERROR;
		if (task_mutex)
			os_mutex_lock(task_mutex, OS_WAIT_FOREVER);
		lstDelete(os_task_list, (NODE *) task);
		if (lstCount(os_task_list) == 0)
		{
			if (os_moniter_id)
			{
				os_time_destroy(os_moniter_id);
			}
		}

		if (task_mutex)
			os_mutex_unlock(task_mutex);
		pthread_exit(0);
	}
	else
	{
		if (task_mutex)
			os_mutex_lock(task_mutex, OS_WAIT_FOREVER);
		pthread_cancel(task->td_thread);
		pthread_join(task->td_thread, (void **) NULL);
		if (os_task_list == NULL)
		{
			if (task_mutex)
				os_mutex_unlock(task_mutex);
			return ERROR;
		}
		lstDelete(os_task_list, (NODE *) task);
		os_task_tcb_entry_destroy(task);
		if (lstCount(os_task_list) == 0)
		{
			if (os_moniter_id)
			{
				os_time_destroy(os_moniter_id);
			}
		}
		if (task_mutex)
			os_mutex_unlock(task_mutex);
	}
	return OK;
}

int os_task_destroy(unit32 taskId)
{
	os_task_t *task = (os_task_t *) taskId;
	if (task)
		return os_task_entry_destroy(task);
	return ERROR;
}

static int os_task_tcb_create(os_task_t *task)
{
	//int time_slice = 0;
	pthread_mutexattr_t attr;
	//pthread_condattr_t 	cat;

	pthread_mutexattr_init(&attr);
#ifndef PTHREAD_MUTEX_ERRORCHECK
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK_NP);
#else
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
#endif
	pthread_mutex_init(&(task->td_mutex), (pthread_mutexattr_t *) &attr);
	pthread_cond_init(&(task->td_control), NULL);

	//pthread_condattr_init(&cat);

	pthread_attr_init(&(task->td_attr));
	pthread_attr_getschedparam(&(task->td_attr), &(task->td_param));

	if (task->td_priority == OS_TASK_DEFAULT_PRIORITY)
	{
		pthread_attr_setschedpolicy(&(task->td_attr), SCHED_OTHER);
	}
	else
	{
		int policy = 0;
		pthread_attr_setinheritsched(&(task->td_attr), PTHREAD_EXPLICIT_SCHED);
		if (task->td_options & OS_TASK_TIME_SLICED)
			policy = SCHED_RR;
		else
			policy = SCHED_FIFO;
		pthread_attr_setschedpolicy(&(task->td_attr), policy);
	}
	task->td_param.sched_priority = task->td_priority;
	pthread_attr_setschedparam(&(task->td_attr), &(task->td_param));

	pthread_attr_setstacksize(&(task->td_attr), task->td_stack_size);

	return OK;
}

#ifdef OS_TASK_DEBUG
static int os_task_entry_start(os_task_t *task)
{
	usleep(100000);
	printf("\r\ncreate task:%s(%u->LWP=%u)\r\n", task->td_name, task->td_thread,
			(pid_t) syscall(SYS_gettid));

	task->td_tid = os_task_gettid();
	task->td_entry(task->pVoid);

	if (task_mutex)
		os_mutex_lock(task_mutex, OS_WAIT_FOREVER);
	lstDelete(os_task_list, (NODE *) task);
	os_task_tcb_entry_destroy(task);
	if (lstCount(os_task_list) == 0)
	{
		if (os_moniter_id)
			os_time_destroy(os_moniter_id);
	}
	if (task_mutex)
		os_mutex_unlock(task_mutex);

	//pthread_exit(0);
	return 0;
}

static int os_task_tcb_start(os_task_t *task)
{
	if (pthread_create(&(task->td_thread), &(task->td_attr),
			os_task_entry_start, (void *) task) == 0)
	{
		pthread_detach(task->td_thread);
//		pthread_attr_destroy(&(task->td_attr));
#ifndef USE_IPSTACK_KERNEL
		ipcom_os_task_tcb_entry_create_cb(task->td_name, task->td_thread,
				task->td_entry_name);
#endif
		OS_DEBUG("%s:\r\n",__func__);
//		pthread_exit (0);
		return OK;
	}
	return ERROR;
}
#else
static int os_task_tcb_start(os_task_t *task)
{
	return pthread_create(&(task->td_thread), &(task->td_attr),
			task->td_entry, (void *)task->pVoid);
}
#endif

static os_task_t * os_task_tcb_entry_create(char *name, int pri, int op,
		task_entry entry, void *pVoid, char *func_name, int stacksize)
{
	os_task_t *task = os_malloc(sizeof(os_task_t));
	if (task)
	{
		os_memset(task, 0, sizeof(os_task_t));
		task->td_id = (unit32) task; /* task id */
		os_strcpy(task->td_name, name); /* name of task */
		task->td_name_key = os_task_name_hash_key(task->td_name);
		/*
		 task->td_thread = 0;
		 task->td_sem;
		 task->td_mutex;
		 task->td_control;
		 task->td_attr;
		 task->td_spinlock;
		 task->td_param;
		 */
		task->td_pid = getpid();
		task->td_priority = pri; /* task priority */
		task->td_old_priority = pri;
		task->td_status = 0; /* task status */
		task->td_options = op | OS_TASK_DELETED | OS_TASK_DELETE_SAFE; /* task option bits (see below) */
		if (stacksize < OS_TASK_STACK_MIN)
			task->td_stack_size = OS_TASK_STACK_MIN; /* size of stack in bytes */
		else
			task->td_stack_size = stacksize; /* size of stack in bytes */

		task->td_entry = entry;
		task->pVoid = pVoid;

		os_strcpy(task->td_entry_name, func_name);

		if (task_mutex)
			os_mutex_lock(task_mutex, OS_WAIT_FOREVER);
		lstAdd(os_task_list, (NODE *) task);
		if (task_mutex)
			os_mutex_unlock(task_mutex);

		if (os_task_tcb_create(task) == OK)
		{
			return task;
		}
		else
		{
			os_free(task);
			return NULL;
		}
	}
	return NULL;
}

static int os_task_moniter_init(void)
{
	if (os_moniter_id == 0)
	{
		os_moniter_id = os_time_create(os_task_refresh_time, NULL, 10000);
		return OK;
	}
	return ERROR;
}

unit32 os_task_entry_create(char *name, int pri, int op, task_entry entry,
		void *pVoid, char *func_name, int stacksize)
{
	os_task_t * task = os_task_tcb_entry_create(name, pri, op, entry, pVoid,
			func_name, stacksize);
	if (task == NULL)
		return ERROR;
	if (os_task_tcb_start(task) == OK)
	{
#ifndef __UCLIBC__
		//prctl(PR_SET_NAME,"THREAD2");
		//int pthread_setname_np(pthread_t thread, const char *name);
		//int pthread_getname_np(pthread_t thread,
		//                              char *name, size_t len);
		//extern struct zebra_privs_t os_privs;
		int ret = 0;

		ret = pthread_setname_np(task->td_thread, task->td_name);

		if (ret != OK)
			zlog_err(ZLOG_DEFAULT, "%s: could not lower privs, %s", __func__,
					os_strerror(errno));
#endif
		OS_DEBUG("\r\ncreate task:%s(%u %u->%u:ret=%d) pid=%d\r\n",task->td_name,
				task->td_thread,(unit32)task,task->td_id,ret,getpid());
		/*		fprintf(stdout,"\r\ncreate task:%s(%u %u->%u:ret=%d) pid=%d\r\n",task->td_name,
		 task->td_thread,(unit32)task,task->td_id,ret,getpid());*/
#ifndef OS_TASK_DEBUG
		os_task_finsh_job();
#endif
		if (os_time_load() == OK)
		{
			if (os_moniter_id == 0)
			{
				os_task_moniter_init();
			}
		}
		return task->td_id;
	}
	else
	{
		os_task_tcb_entry_destroy(task);
		return OK;
	}
	return ERROR;
}

/*
 * for CLI cmd
 */
static int os_task_show_head(struct vty *vty, int detail)
{
	extern int vty_out(struct vty *, const char *, ...);

	vty_out(vty,
			"%s                   	   CPU (user+system): Real (wall-clock):%s",
			VTY_NEWLINE, VTY_NEWLINE);
//vty_out(vty, "Runtime(ms)   Invoked Avg uSec Max uSecs Avg uSec Max uSecs  Type  Thread%s",VTY_NEWLINE);

	if (detail)
		vty_out(vty, "  %-16s %-12s %-20s %-8s %-4s %-8s %-6s %-8s %-8s%s",
				"taskNmae", "taskId", "entry", "LVPID","pri", "tacksize", "state",
				"Real", "Total", VTY_NEWLINE);
	else
		vty_out(vty, "  %-16s %-12s %-20s %-4s %-8s %-6s %-8s %-8s%s",
				"taskNmae", "taskId", "entry", "pri", "tacksize", "state",
				"Real", "Total", VTY_NEWLINE);
	return 0;
}

static int os_task_show_detail(void *p, os_task_t *task, int detail)
{
	struct vty *vty = (struct vty *)p;
	extern int vty_out(struct vty *, const char *, ...);
	char taskId[16];
	char pri[16];
	char tacksize[16];
	char state[16];
	char real[16];
	char lvpid[16];
	char total[16];
	os_memset(taskId, 0, sizeof(taskId));
	os_memset(pri, 0, sizeof(pri));
	os_memset(tacksize, 0, sizeof(tacksize));
	os_memset(state, 0, sizeof(state));
	os_memset(real, 0, sizeof(real));
	os_memset(total, 0, sizeof(total));
	os_memset(lvpid, 0, sizeof(lvpid));
	os_task_refresh_total_cpu(&total_cpu);
	os_task_refresh_cpu(task);
	os_task_state_to_string(task->state, state);

	sprintf(taskId, "%u", task->td_id);
	sprintf(pri, "%d", task->td_priority);
	sprintf(tacksize, "%x", task->td_stack_size);

	if (total_cpu.total_realy)
		sprintf(real, "%.2f%%",
				total_cpu.cpu * 100
						* (float) ((float) task->hist.total_realy
								/ (float) total_cpu.total_realy));
	else
		sprintf(real, "%s", "0.00%");

	if (total_cpu.total)
		sprintf(total, "%.2f%%",
				total_cpu.cpu * 100
						* (float) ((float) task->hist.total
								/ (float) total_cpu.total));
	else
		sprintf(total, "%s", "0.00%");

	if(detail)
	{

		sprintf(lvpid, "%d", task->td_tid);
		vty_out(vty, "  %-16s %-12s %-20s %-8s %-4s %-8s %-6s %-8s %-8s%s",
			task->td_name, taskId, task->td_entry_name, lvpid, pri, tacksize, state,
			real, total, VTY_NEWLINE);
	}
	else
		vty_out(vty, "  %-16s %-12s %-20s %-4s %-8s %-6s %-8s %-8s%s",
			task->td_name, taskId, task->td_entry_name, pri, tacksize, state,
			real, total, VTY_NEWLINE);
	return 0;
}

int os_task_show(void *vty, char *task_name, int detail)
{
//	unit32 task_id = 0;
	NODE *node = NULL;
	os_task_t *task = NULL;
	if (os_task_list == NULL)
		return ERROR;

	if (task_name)
		task = os_task_node_lookup_by_name(task_name);

	if (task_name && task)
	{
		if (task_mutex)
			os_mutex_lock(task_mutex, OS_WAIT_FOREVER);
		if (task)
		{
			os_task_show_head(vty, detail);
			os_task_show_detail(vty, task, detail);
		}
		if (task_mutex)
			os_mutex_unlock(task_mutex);
		return 0;
	}
	if (task_mutex)
		os_mutex_lock(task_mutex, OS_WAIT_FOREVER);

	node = lstFirst(os_task_list);
	if(node)
		os_task_show_head(vty, detail);
	while (node)
	{
		if (node)
		{
			task = (os_task_t *) node;
			os_task_show_detail(vty, task, detail);
		}
		node = lstNext(node);
	}

/*	node = lstFirst(os_task_list);
	os_task_show_head(vty, detail);
	while (node)
	{
		if (node)
		{
			task = (os_task_t *) node;
			os_task_show_detail(vty, task, detail);
		}
		node = lstNext(node);
	}*/
	if (task_mutex)
		os_mutex_unlock(task_mutex);
	return 0;
}

DEFUN (show_process,
		show_process_cmd,
		"show process cpu [NAME]",
		SHOW_STR
		"system process information\n"
		"process CPU usage\n"
		"system process name\n")
{
	if (argc == 1)
		os_task_show(vty, argv[0], 0);
	else
		os_task_show(vty, NULL, 0);
	return CMD_SUCCESS;
}


DEFUN (show_process_detail,
		show_process_detail_cmd,
		"show process cpu detail [NAME]",
		SHOW_STR
		"system process information\n"
		"process CPU usage\n"
		"system process name\n")
{
	if (argc == 1)
		os_task_show(vty, argv[0], 1);
	else
		os_task_show(vty, NULL, 1);
	return CMD_SUCCESS;
}
#if 0
struct vty *vty
static int ipcom_process_printf(const char *format, ...)
{
	va_list args;
	int len = 0;
	int size = 1024;
	char buf[1024];
	char *p = NULL;

	if (vty_shell (vty))
	{
		va_start (args, format);
		vprintf (format, args);
		va_end (args);
	}
	else
	{
		/* Try to write to initial buffer.  */
		va_start (args, format);
		len = vsnprintf (buf, sizeof(buf), format, args);
		va_end (args);

		/* Initial buffer is not enough.  */
		if (len < 0 || len >= size)
		{
			while (1)
			{
				if (len > -1)
				size = len + 1;
				else
				size = size * 2;

				p = XREALLOC (MTYPE_VTY_OUT_BUF, p, size);
				if (! p)
				return -1;

				va_start (args, format);
				len = vsnprintf (p, size, format, args);
				va_end (args);

				if (len > -1 && len < size)
				break;
			}
		}

		/* When initial buffer is enough to store all output.  */
		if (! p)
		p = buf;

		/* Pointer p must point out buffer. */
		buffer_put (vty->obuf, (u_char *) p, len);

		/* If p is not different with buf, it is allocated buffer.  */
		if (p != buf)
		XFREE (MTYPE_VTY_OUT_BUF, p);
	}

	return len;
}
#endif

#ifndef USE_IPSTACK_KERNEL
DEFUN (show_ipcom_process,
		show_ipcom_process_cmd,
		"show ipcom process",
		SHOW_STR
		"ipcom process information\n"
		"process CPU usage\n")
{
	ipcom_process_show(vty_out, vty);
	return CMD_SUCCESS;
}
#endif

int cmd_os_init()
{
	install_element(ENABLE_NODE, &show_process_cmd);
	install_element(ENABLE_NODE, &show_process_detail_cmd);
#ifndef USE_IPSTACK_KERNEL
	install_element(ENABLE_NODE, &show_ipcom_process_cmd);
#endif
	return 0;
}
