/*
 * os_task.c
 *
 *  Created on: May 30, 2017
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zpl_type.h"
#include "os_list.h"
#include "os_sem.h"
#include "os_time.h"
#include "os_task.h"
#ifdef ZPL_SHELL_MODULE
#include "vty_include.h"
#endif

#define OS_TASK_ENTRY_LOCK(n) if(n) pthread_mutex_lock(&n->td_mutex)
#define OS_TASK_ENTRY_UNLOCK(n) if(n) pthread_mutex_unlock(&n->td_mutex)


typedef struct os_gtask_s
{
	LIST *os_task_list;
	os_mutex_t *task_mutex;
	zpl_uint32 os_moniter_id;
	struct os_task_history total_cpu;
}os_global_task_t;


static os_global_task_t _global_task;


typedef struct os_task_hook_tbl
{
	os_task_hook 	cb_create;
	os_task_hook 	cb_start;
	os_task_hook 	cb_destroy;
}os_task_hook_tbl_t;

static os_task_hook_tbl_t hook_tbl[TASK_CBTBL_MAX];

static int os_task_entry_destroy(os_task_t *task);
static os_task_t * os_task_tcb_lookup(zpl_taskid_t id, zpl_pthread_t pid, zpl_bool mutex);

#ifdef OS_TASK_DEBUG_LOG
#define OS_TACK_TMP_LOG	"/tmp/app/log/ostask.log"
#endif

int os_limit_stack_size(int size)
{
	struct rlimit limit;
	if(getrlimit(RLIMIT_STACK, &limit) != 0)
	{
		printf("%s get RLIMIT_STACK :%s\r\n", __func__, strerror(ipstack_errno));
		return ERROR;
	}
	if((uint)size < limit.rlim_cur)
		return ERROR;
	limit.rlim_cur = size;
	if((uint)size > limit.rlim_max)
		limit.rlim_max = size;
	if(setrlimit(RLIMIT_STACK, &limit) != 0)
	{
		printf("%s set RLIMIT_STACK :%s\r\n", __func__, strerror(ipstack_errno));
		return ERROR;
	}
	return OK;
}

int os_limit_core_size(int size)
{
	struct rlimit limit;
	if(getrlimit(RLIMIT_CORE, &limit) != 0)
	{
		printf("%s get RLIMIT_CORE :%s\r\n", __func__, strerror(ipstack_errno));
		return ERROR;
	}
	if((uint)size < limit.rlim_cur)
		return ERROR;
	limit.rlim_cur = size;
	if((uint)size > limit.rlim_max)
		limit.rlim_max = size;
	if(setrlimit(RLIMIT_CORE, &limit) != 0)
	{
		printf("%s set RLIMIT_CORE :%s\r\n", __func__, strerror(ipstack_errno));
		return ERROR;
	}
	return OK;
}

int os_task_init(void)
{
#ifdef OS_TASK_DEBUG_LOG
	remove(OS_TACK_TMP_LOG);
	sync();
#endif
	if (_global_task.task_mutex == NULL)
	{
		_global_task.task_mutex = os_mutex_name_create("taskmutex");
	}
	if (_global_task.os_task_list == NULL)
	{
		_global_task.os_task_list = os_malloc(sizeof(LIST));
		if (_global_task.os_task_list)
		{
			os_memset(hook_tbl, 0, sizeof(hook_tbl));
			os_memset(&_global_task.total_cpu, 0, sizeof(_global_task.total_cpu));
#ifndef HAVE_GET_NPROCS
			_global_task.total_cpu.cpu = get_nprocs();
#endif
			lstInit(_global_task.os_task_list);
			return OK;
		}
		os_mutex_destroy(_global_task.task_mutex);
		return ERROR;
	}
	return OK;
}


int os_task_exit(void)
{
	if (_global_task.task_mutex)
	{
		os_mutex_lock(_global_task.task_mutex, OS_WAIT_FOREVER);
		os_mutex_destroy(_global_task.task_mutex);
		if (_global_task.os_task_list)
			lstFree(_global_task.os_task_list);
		return OK;
	}
	return OK;
}

int os_task_sigmask(int sigc, int signo[])
{
	int i = 0;
	sigset_t mask;
	sigemptyset(&mask);
	if(sigc)
	{
		for(i = 0; i < sigc; i++)
		{
			sigaddset(&mask, signo[i]);
		}
	}
	return pthread_sigmask(SIG_SETMASK, &mask, NULL);
}

int os_task_sigexecute(int sigc, int signo[])
{
	int i = 0;
	sigset_t mask;
	sigfillset(&mask);
	if(sigc)
	{
		for(i = 0; i < sigc; i++)
		{
			sigdelset(&mask, signo[i]);
		}
	}
	return pthread_sigmask(SIG_SETMASK, &mask, NULL);
}

int os_task_sigmaskall(void)
{
	sigset_t mask;
	sigfillset(&mask);
	return pthread_sigmask(SIG_SETMASK, &mask, NULL);
}

int os_task_killsignal(zpl_taskid_t task_id, int signno)
{
	os_task_t *task = os_task_tcb_lookup(task_id, 0, zpl_true);
	if(task == NULL)
		return ERROR;	
	if (task)
	{
		if (_global_task.task_mutex)
			os_mutex_lock(_global_task.task_mutex, OS_WAIT_FOREVER);
		pthread_kill(task->td_thread, signno);
		if (_global_task.task_mutex)
			os_mutex_unlock(_global_task.task_mutex);
	}
	return OK;
}

static int _os_task_create_callback(os_task_t *task)
{
	int i = 0;
	for(i = 0; i < TASK_CBTBL_MAX; i++)
	{
		if(hook_tbl[i].cb_create)
			hook_tbl[i].cb_create(task);
	}
	return OK;
}

static int _os_task_start_callback(os_task_t *task)
{
	int i = 0;
	for(i = 0; i < TASK_CBTBL_MAX; i++)
	{
		if(hook_tbl[i].cb_start)
			hook_tbl[i].cb_start(task);
	}
	return OK;
}

static int _os_task_destroy_callback(os_task_t *task)
{
	int i = 0;
	for(i = 0; i < TASK_CBTBL_MAX; i++)
	{
		if(hook_tbl[i].cb_destroy)
			hook_tbl[i].cb_destroy(task);
	}
	return OK;
}

int os_task_add_start_hook(os_task_hook cb)
{
	int i = 0;
	for(i = 0; i < TASK_CBTBL_MAX; i++)
	{
		if(hook_tbl[i].cb_start == NULL)
		{
			hook_tbl[i].cb_start = cb;
			//cb_tbl[i].cb_create = cb->cb_create;
			//cb_tbl[i].cb_destroy = cb->cb_destroy;
			//cb_tbl[i].pVoid = cb->pVoid;
			return OK;
		}
	}
	return ERROR;
}
int os_task_add_create_hook(os_task_hook cb)
{
	int i = 0;
	for(i = 0; i < TASK_CBTBL_MAX; i++)
	{
		if(hook_tbl[i].cb_create == NULL)
		{
			//cb_tbl[i].cb_start = cb->cb_start;
			hook_tbl[i].cb_create = cb;
			//cb_tbl[i].cb_destroy = cb->cb_destroy;
			//cb_tbl[i].pVoid = cb->pVoid;
			return OK;
		}
	}
	return ERROR;
}
int os_task_add_destroy_hook(os_task_hook cb)
{
		int i = 0;
	for(i = 0; i < TASK_CBTBL_MAX; i++)
	{
		if(hook_tbl[i].cb_destroy == NULL/* && cb_tbl[i].cb_stop == NULL*/)
		{
			//cb_tbl[i].cb_start = cb->cb_start;
			//cb_tbl[i].cb_create = cb->cb_create;
			hook_tbl[i].cb_destroy = cb;
			//cb_tbl[i].pVoid = cb->pVoid;
			return OK;
		}
	}
	return ERROR;
}

int os_task_del_start_hook(os_task_hook cb)
{
	int i = 0;
	for(i = 0; i < TASK_CBTBL_MAX; i++)
	{
		if(hook_tbl[i].cb_start == cb)
		{
			hook_tbl[i].cb_start = NULL;
			//cb_tbl[i].cb_create = cb->cb_create;
			//cb_tbl[i].cb_destroy = cb->cb_destroy;
			//cb_tbl[i].pVoid = cb->pVoid;
			return OK;
		}
	}
	return ERROR;
}
int os_task_del_create_hook(os_task_hook cb)
{
	int i = 0;
	for(i = 0; i < TASK_CBTBL_MAX; i++)
	{
		if(hook_tbl[i].cb_create == cb)
		{
			//cb_tbl[i].cb_start = cb->cb_start;
			hook_tbl[i].cb_create = NULL;
			//cb_tbl[i].cb_destroy = cb->cb_destroy;
			//cb_tbl[i].pVoid = cb->pVoid;
			return OK;
		}
	}
	return ERROR;
}
int os_task_del_destroy_hook(os_task_hook cb)
{
	int i = 0;
	for(i = 0; i < TASK_CBTBL_MAX; i++)
	{
		if(hook_tbl[i].cb_destroy == cb/* && cb_tbl[i].cb_stop == NULL*/)
		{
			//cb_tbl[i].cb_start = cb->cb_start;
			//cb_tbl[i].cb_create = cb->cb_create;
			hook_tbl[i].cb_destroy = NULL;
			//cb_tbl[i].pVoid = cb->pVoid;
			return OK;
		}
	}
	return ERROR;
}

static os_task_t * os_task_tcb_lookup(zpl_taskid_t id, zpl_pthread_t pid, zpl_bool mutex)
{
	NODE *node = NULL;
	os_task_t *task = NULL;
	if (_global_task.os_task_list == NULL)
		return NULL;
	if(id == (zpl_taskid_t)ERROR)
		return NULL;		
	if(mutex)
	{
		if (_global_task.task_mutex)
			os_mutex_lock(_global_task.task_mutex, OS_WAIT_FOREVER);
	}
	node = lstFirst(_global_task.os_task_list);
	while (node)
	{
		if (node)
		{
			task = (os_task_t *) node;
			if ((id != 0) && id == task->td_id)
			{
				if(mutex)
				{
					if (_global_task.task_mutex)
						os_mutex_unlock(_global_task.task_mutex);
				}
				return task;
			}
			else if (((zpl_pthread_t) pid != 0)
					&& (zpl_pthread_t) pid == (zpl_pthread_t) task->td_thread)
			{
				if(mutex)
				{
					if (_global_task.task_mutex)
						os_mutex_unlock(_global_task.task_mutex);
				}
				return task;
			}
		}
		node = lstNext(node);
	}
	if(mutex)
	{
		if (_global_task.task_mutex)
			os_mutex_unlock(_global_task.task_mutex);
	}
	return NULL;
}

static zpl_uint32 os_task_name_hash_key(const char *str)
{
	zpl_uint32 hash = 0;
	const char *p = str;
	while (*p)
		hash = (hash * 33) ^ (zpl_uint32) *p++;

	return hash;
}

static os_task_t * os_task_node_lookup_by_name(char *task_name)
{
	NODE *node;
	os_task_t *task;
	if (_global_task.os_task_list == NULL)
		return NULL;
	if (_global_task.task_mutex)
		os_mutex_lock(_global_task.task_mutex, OS_WAIT_FOREVER);
	node = lstFirst(_global_task.os_task_list);
	while (node)
	{
		task = (os_task_t *) node;
		if (task->td_name_key == os_task_name_hash_key(task_name))
		{
			if (_global_task.task_mutex)
				os_mutex_unlock(_global_task.task_mutex);
			return task;
		}
		node = lstNext(node);
	}
	if (_global_task.task_mutex)
		os_mutex_unlock(_global_task.task_mutex);
	return NULL;
}

zpl_taskid_t os_task_lookup_by_name(const zpl_char *task_name)
{
	os_task_t *task = os_task_node_lookup_by_name(task_name);
	if (task)
		return task->td_id;
	return ERROR;
}

os_task_t * os_task_self(void)
{
	os_task_t *task = os_task_tcb_lookup(0, os_task_pthread_self(), zpl_true);
	if (task)
	{
		return task;
	}
	return NULL;
}

zpl_bool os_task_running_state(void)
{
	os_task_t * ptask = os_task_self();
	if(ptask)
		return ptask->active;
	return zpl_false;	
}

os_task_t * os_task_lookup(zpl_taskid_t id, zpl_pthread_t pid)
{
	os_task_t *task = os_task_tcb_lookup(id, pid, zpl_true);
	if (task)
	{
		return task;
	}
	return NULL;
}


zpl_pthread_t os_task_pthread_self(void)
{
	return pthread_self();
}

zpl_taskid_t os_task_id_self(void)
{
	os_task_t *task = os_task_tcb_lookup(0, os_task_pthread_self(), zpl_true);
	if (task)
	{
		return task->td_id;
	}
	return ERROR;
}


int os_task_gettid(void)
{
	return syscall(SYS_gettid);
}

const zpl_char * os_task_self_name_alisa(void)
{
	NODE *node = NULL;
	os_task_t *task = NULL;
	static char name[TASK_NAME_MAX * 2];
	os_memset(name, 0, sizeof(name));
	if (_global_task.os_task_list == NULL)
	{
		snprintf(name, sizeof(name), "[LWP:%ld]", syscall(SYS_gettid));
		return name;
	}
	if (_global_task.task_mutex)
		os_mutex_lock(_global_task.task_mutex, OS_WAIT_FOREVER);
	node = lstFirst(_global_task.os_task_list);
	while (node)
	{
		if (node)
		{
			task = (os_task_t *) node;
			if (task->td_tid == syscall(SYS_gettid))
			{
				snprintf(name, sizeof(name), "%s[LWP:%d]", task->td_name, task->td_tid);
				if (_global_task.task_mutex)
					os_mutex_unlock(_global_task.task_mutex);
				return name;
			}
		}
		node = lstNext(node);
	}
	if (_global_task.task_mutex)
		os_mutex_unlock(_global_task.task_mutex);
	snprintf(name, sizeof(name), "[LWP:%ld]", syscall(SYS_gettid));
	return name;
}

int os_task_name_get(zpl_taskid_t task_id, char *task_name)
{
	os_task_t *task = os_task_tcb_lookup(task_id, 0, zpl_true);
	if(task == NULL)
		return ERROR;	
	if (task)
	{
		if (_global_task.task_mutex)
			os_mutex_lock(_global_task.task_mutex, OS_WAIT_FOREVER);
		if (task_name)
		{
			os_strcpy(task_name, task->td_name);
			if (_global_task.task_mutex)
				os_mutex_unlock(_global_task.task_mutex);
			return OK;
		}
		if (_global_task.task_mutex)
			os_mutex_unlock(_global_task.task_mutex);
	}
	return ERROR;
}

zpl_char * os_task_2_name(zpl_taskid_t task_id)
{
	static char name[TASK_NAME_MAX];
	os_memset(name, 0, sizeof(name));
	if(task_id == (zpl_taskid_t)ERROR)
		return "Unknow";		
	if (os_task_name_get(task_id, name) == OK)
		return name;
	return "Unknow";
}
zpl_char * os_task_name_pthreadid( zpl_pthread_t pid)
{
	os_task_t *task = os_task_tcb_lookup(0, pid, zpl_true);
	if (task)
	{
		return task->td_name;
	}
	return "Unknow";
}
zpl_char * os_task_name_LWPID( zpl_pid_t tid)
{
	//os_task_gettid();
	NODE *node = NULL;
	os_task_t *task = NULL;
	if (_global_task.task_mutex)
		os_mutex_lock(_global_task.task_mutex, OS_WAIT_FOREVER);
	node = lstFirst(_global_task.os_task_list);
	while (node)
	{
		if (node)
		{
			task = (os_task_t *) node;
			if (task->td_tid == tid)
			{
				if (_global_task.task_mutex)
					os_mutex_unlock(_global_task.task_mutex);
				return task->td_name;
			}
		}
		node = lstNext(node);
	}
	if (_global_task.task_mutex)
		os_mutex_unlock(_global_task.task_mutex);
	return "Unknow";
}
int os_task_priority_set(zpl_taskid_t taskId, zpl_int32 Priority)
{
	int policy; //,pri;
	struct sched_param sp;
	if(Priority >= OS_TASK_MAX_PRIORITY || Priority < 0)
		return ERROR;
	os_task_t *osapiTask = os_task_tcb_lookup(taskId, 0, zpl_true);
	if(osapiTask == NULL)
		return ERROR;
	if(osapiTask && !CHECK_FLAG(osapiTask->flags, OS_TASK_FLAGS_CREATE))
		return OK;
	if (osapiTask)
	{
		if (_global_task.task_mutex)
			os_mutex_lock(_global_task.task_mutex, OS_WAIT_FOREVER);
		OS_TASK_ENTRY_LOCK(osapiTask);
//		pri = osapiTask->td_priority;
		osapiTask->td_old_priority = osapiTask->td_priority;
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

			osapiTask->td_priority = Priority;
		}
		//osapiTask->td_priority = Priority;
		//osapiTask->td_old_priority = policy;
		sp.sched_priority = osapiTask->td_priority;
		OS_TASK_ENTRY_UNLOCK(osapiTask);
		if (_global_task.task_mutex)
			os_mutex_unlock(_global_task.task_mutex);
		pthread_setschedparam(osapiTask->td_thread, policy, &sp);
		return (OK);
	}
	return ERROR;
}

int os_task_priority_get(zpl_taskid_t taskId, zpl_int32 *Priority)
{
	os_task_t *osapiTask = os_task_tcb_lookup(taskId, 0, zpl_true);
	if(osapiTask == NULL)
		return ERROR;
	if (osapiTask)
	{
		OS_TASK_ENTRY_LOCK(osapiTask);
		if (Priority)
			*Priority = osapiTask->td_priority;
		OS_TASK_ENTRY_UNLOCK(osapiTask);
		return (OK);
	}
	return ERROR;
}

int os_task_supend(zpl_taskid_t taskId)
{
	os_task_t *osapiTask = os_task_tcb_lookup(taskId, 0, zpl_true);
	if(osapiTask == NULL)
		return ERROR;	
	if (osapiTask)
	{
		OS_TASK_ENTRY_LOCK(osapiTask);
		pthread_kill(osapiTask->td_thread, SIGSTOP);
		SET_FLAG(osapiTask->flags, OS_TASK_FLAGS_SUPEND);
		UNSET_FLAG(osapiTask->flags, OS_TASK_FLAGS_RESUME);
		OS_TASK_ENTRY_UNLOCK(osapiTask);
		return (OK);
	}
	return ERROR;
}

int os_task_resume(zpl_taskid_t taskId)
{
	os_task_t *osapiTask = os_task_tcb_lookup(taskId, 0, zpl_true);
	if(osapiTask == NULL)
		return ERROR;	
	if (osapiTask)
	{
		OS_TASK_ENTRY_LOCK(osapiTask);
		pthread_kill(osapiTask->td_thread, SIGCONT);
		UNSET_FLAG(osapiTask->flags, OS_TASK_FLAGS_SUPEND);
		SET_FLAG(osapiTask->flags, OS_TASK_FLAGS_RESUME);
		OS_TASK_ENTRY_UNLOCK(osapiTask);
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
	usec = ticks * OS_TASK_TICK_MSEC;
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
#ifdef ZPL_SHELL_MODULE
static int os_task_state_to_string(enum os_task_state st, char *state)
{
	if (state)
	{
		switch (st)
		{
		case OS_TASK_STATE_CREATE: //C
			strcat(state, "C");
			break;
		case OS_TASK_STATE_READY: //R
			strcat(state, "R");
			break;
		case OS_TASK_STATE_RUNNING: //R
			strcat(state, "R");
			break;
		case OS_TASK_STATE_WAIT: //W
			strcat(state, "W");
			break;
		case OS_TASK_STATE_SLEEP: //S
			strcat(state, "S");
			break;
		case OS_TASK_STATE_STOP: //T
			strcat(state, "T");
			break;
		case OS_TASK_STATE_DEAD: //D
			strcat(state, "D");
			break;
		case OS_TASK_STATE_ZOMBIE: //Z
			strcat(state, "Z");
			break;
		default:
			strcat(state, "U");
			break;
		}
	}
	return 0;
}
#endif
static int os_task_string_to_state(char *state, enum os_task_state *st)
{
	if (state)
	{
		if (st)
		{
			if (os_strstr(state, "C"))
				*st = OS_TASK_STATE_CREATE;
			else if (os_strstr(state, "R"))
				*st = OS_TASK_STATE_READY;
			else if (os_strstr(state, "R"))
				*st = OS_TASK_STATE_RUNNING;
			else if (os_strstr(state, "W"))
				*st = OS_TASK_STATE_WAIT;
			else if (os_strstr(state, "S"))
				*st = OS_TASK_STATE_SLEEP;
			else if (os_strstr(state, "T"))
				*st = OS_TASK_STATE_STOP;
			else if (os_strstr(state, "D"))
				*st = OS_TASK_STATE_DEAD;
			else if (os_strstr(state, "Z"))
				*st = OS_TASK_STATE_ZOMBIE;
		}
	}
	return 0;
}

static int os_task_refresh_total_cpu(struct os_task_history *hist)
{
	char path[128];
	char buf[1024];
	FILE *fp = NULL;
	os_memset(path, 0, sizeof(path));
	os_memset(buf, 0, sizeof(buf));
	sprintf(path, "/proc/stat");
#ifndef HAVE_GET_NPROCS
	if (hist->cpu == 0)
		hist->cpu = get_nprocs();
#endif
	fp = fopen(path, "r");
	if (fp)
	{
		int user = 0, nice = 0, system = 0, idle = 0, iowait = 0, irq = 0, softirq = 0, stealstolen = 0, guest = 0;
		if (fgets(buf, sizeof(buf), fp))
		{
			sscanf(buf, "%*s %d %d %d %d %d %d %d %d %d", &user, &nice, &system,
					&idle, &iowait, &irq, &softirq, &stealstolen, &guest);
			hist->total = (user + nice + system + idle + iowait + irq + softirq
					+ stealstolen + guest);
			hist->total_realy = (user + nice + system + idle + iowait + irq
					+ softirq + stealstolen + guest) - hist->total_realy;
		}
		fclose(fp);
	}
	else
	{
		printf("can't open path:%s\r\n", path);
	}
	return 0;
}

static int os_task_refresh_cpu(os_task_t *task)
{
	char path[128];
	char buf[2048];
	FILE *fp = NULL;
	int ret = 0;
	os_memset(path, 0, sizeof(path));
	os_memset(buf, 0, sizeof(buf));
#ifndef HAVE_GET_NPROCS
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
		int utime = 0, stime = 0, cutime = 0, cstime = 0, start_time = 0, priority = 0, nice = 0;
		unsigned int vsize = 0; /* Virtual memory size */
		unsigned int rss = 0; /* Resident Set Size */
		unsigned int rlim = 0; /* Current limit in bytes on the rss */
		ret = fread(buf, 1, sizeof(buf), fp);
		if (ret)
		{
			sscanf(buf, "%*d %*s %s %*d %*d %*d %*d %*d %*d %*d %*d %*d "
					"%d %d %d %d %d %d %*d %*d %d %d %d %d", sta, &utime, &stime,
					&cutime, &cstime, &priority, &nice, &start_time, &vsize, &rss, &rlim);
			os_task_string_to_state(sta, &task->state);
			task->hist.total_realy = (utime + stime + cutime + cstime)
					- task->hist.total_realy;
			task->hist.total = (utime + stime + cutime + cstime)
					- task->hist.total;
					
			task->hist.mtotal = (vsize>>10);
			task->hist.mrss = (rss<<2)>>10;
			task->hist.mlimit = (rlim);
		}
		fclose(fp);
	}
	else
	{
		if(task->state > OS_TASK_STATE_CREATE)
			task->state = OS_TASK_STATE_DEAD;
	}
	return 0;
}

static int os_task_refresh_time(void *argv)
{
//	if(os_job_finsh() == OK)
	{
		NODE node;
		os_task_t *task = NULL;
		if (_global_task.os_task_list == NULL)
			return ERROR;
		os_task_refresh_total_cpu(&_global_task.total_cpu);
		if (_global_task.task_mutex)
			os_mutex_lock(_global_task.task_mutex, OS_WAIT_FOREVER);
		task = (os_task_t *)lstFirst(_global_task.os_task_list);

		for(task = (os_task_t *)lstFirst(_global_task.os_task_list);
				task != NULL;
				task = (os_task_t *)lstNext(&node))
		{
			node = task->node;

			if( task->state == OS_TASK_STATE_ZOMBIE ||
				task->state == OS_TASK_STATE_DEAD )
			{
				if (_global_task.task_mutex)
					os_mutex_unlock(_global_task.task_mutex);
				os_task_entry_destroy(task);
				if (_global_task.task_mutex)
					os_mutex_lock(_global_task.task_mutex, OS_WAIT_FOREVER);
			}

			if (task && task->td_tid != 0)
			{
				os_task_refresh_cpu(task);
			}
			if (task && task->td_tid == 0)
			{
				if(task->state > OS_TASK_STATE_CREATE)
					task->state = OS_TASK_STATE_ZOMBIE;
			}
		}
		if (_global_task.task_mutex)
			os_mutex_unlock(_global_task.task_mutex);
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
					if(_global_task.task_mutex)
					os_mutex_lock(_global_task.task_mutex, OS_WAIT_FOREVER);
					if(task)
					{
						tid = atol(ptr->d_name);
						task->td_tid = (pid_t)tid;
					}
					if(_global_task.task_mutex)
					os_mutex_unlock(_global_task.task_mutex);
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
		if(_global_task.os_task_list == NULL)
		return ERROR;
		if(_global_task.task_mutex)
		os_mutex_lock(_global_task.task_mutex, OS_WAIT_FOREVER);
		node = lstFirst(_global_task.os_task_list);
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
		if(_global_task.task_mutex)
		os_mutex_unlock(_global_task.task_mutex);
		if(addjob)
			return os_job_add(OS_JOB_NONE, os_task_refresh_tid_task, NULL);
	}
	return 0;
}
#endif

static int os_task_attribute_destroy(os_task_t *task)
{
	pthread_mutex_destroy(&(task->td_mutex));
	if(CHECK_FLAG(task->flags, OS_TASK_FLAGS_CREATE))
	{
		pthread_cond_destroy(&(task->td_control));
		pthread_attr_destroy(&(task->td_attr));
	}
	return OK;
}

static int os_task_entry_destroy(os_task_t *task)
{
	if (_global_task.os_task_list == NULL)
	{
		return ERROR;
	}
	if (_global_task.task_mutex)
		os_mutex_lock(_global_task.task_mutex, OS_WAIT_FOREVER);
	lstDelete(_global_task.os_task_list, (NODE *)task);
	_os_task_destroy_callback(task);
	if (task->priv)
	{
		task->priv = NULL;
	}
	if (lstCount(_global_task.os_task_list) == 0)
	{
		if (_global_task.os_moniter_id)
		{
			os_time_destroy(_global_task.os_moniter_id);
		}
	}
	os_task_attribute_destroy(task);
	os_free(task);
	task = NULL;
	if (_global_task.task_mutex)
		os_mutex_unlock(_global_task.task_mutex);
	return OK;
}

int os_task_destroy(zpl_taskid_t taskId)
{
	os_task_t *task = os_task_tcb_lookup(taskId, 0, zpl_true);
	if(task == NULL)
		return ERROR;
	if (task && !CHECK_FLAG(task->flags, OS_TASK_FLAGS_CANCEL))
	{
		int ret = 0;
		OS_TASK_ENTRY_LOCK(task);
		SET_FLAG(task->flags, OS_TASK_FLAGS_DESTRIOY);
		task->active = zpl_false;
		if(task->td_thread)
		{
			pthread_cancel(task->td_thread);
			pthread_join(task->td_thread, (void **) NULL);
			task->td_thread = 0;
		}
		
		ret = os_task_entry_destroy(task);
		OS_TASK_ENTRY_UNLOCK(task);
		return ret;
	}
	return ERROR;
}

int os_task_cancel(zpl_taskid_t taskId)
{
	os_task_t *task = os_task_tcb_lookup(taskId, 0, zpl_true);
	if(task == NULL)
		return ERROR;
	if (task)
	{
		OS_TASK_ENTRY_LOCK(task);
		SET_FLAG(task->flags, OS_TASK_FLAGS_CANCEL);
		task->active = zpl_false;
		OS_TASK_ENTRY_UNLOCK(task);
		return OK;
	}
	return ERROR;
}

static int os_task_attribute_create(os_task_t *task, zpl_bool bcreate)
{
	pthread_mutexattr_t attr;

	pthread_mutexattr_init(&attr);
#ifndef PTHREAD_MUTEX_ERRORCHECK
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK_NP);
#else
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
#endif
	pthread_mutex_init(&(task->td_mutex), (pthread_mutexattr_t *) &attr);

	if(bcreate == zpl_false)
		return OK;

	pthread_cond_init(&(task->td_control), NULL);

	//pthread_condattr_init(&cat);

	pthread_attr_init(&(task->td_attr));
	pthread_attr_getschedparam(&(task->td_attr), &(task->td_param));

	if (task->td_priority == OS_TASK_DEFAULT_PRIORITY)
	{
		pthread_attr_setschedpolicy(&(task->td_attr), SCHED_OTHER);
		task->td_param.sched_priority = 0;
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
		task->td_param.sched_priority = task->td_priority;
	}
	pthread_attr_setschedparam(&(task->td_attr), &(task->td_param));
	pthread_attr_setstacksize(&(task->td_attr), task->td_stack_size);
	return OK;
}


static void *os_task_entry_running(void *pVoid)
{
	os_task_t *task = (os_task_t *)pVoid;
    struct sched_param param;
    int policy;	
	usleep(100000);
#ifdef OS_TASK_DEBUG_LOG
	os_log(OS_TACK_TMP_LOG, "os task create task:%s(%u->LWP=%u)\r\n", task->td_name, task->td_thread,
			(pid_t) syscall(SYS_gettid));
#endif
    pthread_getschedparam(task->td_thread, &policy, &param);

	printf("\r\ncreate task:%s(%lu->LWP=%u %d-%d[%d,%d])\r\n", task->td_name, task->td_thread,
			(pid_t) syscall(SYS_gettid), policy, param.sched_priority, 
			sched_get_priority_min(policy), sched_get_priority_max(policy));
	os_task_sigmaskall();
	OS_TASK_ENTRY_LOCK(task);
	task->td_tid = os_task_gettid();
	task->active = zpl_true;
	
    prctl(PR_SET_NAME, task->td_name, 0);

	_os_task_start_callback(task);
	OS_TASK_ENTRY_UNLOCK(task);
	task->td_entry(task->pVoid);
	OS_TASK_ENTRY_LOCK(task);
	if(CHECK_FLAG(task->flags, OS_TASK_FLAGS_CANCEL))
	{
		task->td_thread = 0;
		os_task_entry_destroy(task);
	}
	OS_TASK_ENTRY_UNLOCK(task);
	return NULL;
}

static int os_task_entry_start(os_task_t *task)
{
	if (pthread_create(&(task->td_thread), &(task->td_attr),
			os_task_entry_running, (void *) task) == 0)
	{
		pthread_detach(task->td_thread);
		OS_DEBUG("%s:\r\n",__func__);
		return OK;
	}
	return ERROR;
}

static os_task_t * os_task_entry_malloc(zpl_char *name, zpl_int32 pri, zpl_uint32 op,
		task_entry entry, void *pVoid, zpl_char *func_name, zpl_uint32 stacksize, zpl_bool bcreate)
{
	if(pri > OS_TASK_MAX_PRIORITY || pri < 0)
		return NULL;

	if(!name || strlen(name) >= TASK_NAME_MAX)
		return NULL;

	if(!func_name || strlen(func_name) >= 2*TASK_NAME_MAX)
		return NULL;

	if(stacksize > OS_TASK_STACK_MAX || stacksize <= 0)
		return NULL;

	os_task_t *task = os_malloc(sizeof(os_task_t));
	if (task)
	{
		os_memset(task, 0, sizeof(os_task_t));
		task->td_id = (zpl_taskid_t) task; /* task id */
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
		task->td_options = op | OS_TASK_DELETED | OS_TASK_DELETE_SAFE|OS_TASK_TIME_SLICED; /* task option bits (see below) */
		if (stacksize < OS_TASK_STACK_MIN)
			task->td_stack_size = OS_TASK_STACK_MIN; /* size of stack in bytes */
		else
			task->td_stack_size = stacksize; /* size of stack in bytes */

		task->td_entry = entry;
		task->pVoid = pVoid;
		if(bcreate)
			SET_FLAG(task->flags, OS_TASK_FLAGS_CREATE);
		os_strcpy(task->td_entry_name, func_name);
		task->state = OS_TASK_STATE_CREATE;

		if (_global_task.task_mutex)
			os_mutex_lock(_global_task.task_mutex, OS_WAIT_FOREVER);
		lstAdd(_global_task.os_task_list, (NODE *) task);
		if (_global_task.task_mutex)
			os_mutex_unlock(_global_task.task_mutex);

		if (os_task_attribute_create(task, bcreate) == OK)
		{
			_os_task_create_callback(task);
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
	if (_global_task.os_moniter_id == 0)
	{
		_global_task.os_moniter_id = os_time_create(os_task_refresh_time, NULL, 10000);
		return OK;
	}
	return ERROR;
}

zpl_taskid_t os_task_entry_create(const zpl_char *name, zpl_uint32 pri, zpl_uint32 op, task_entry entry,
		void *pVoid, zpl_char *func_name, zpl_uint32 stacksize)
{
	os_task_t * task = os_task_entry_malloc(name, pri, op, entry, pVoid,
			func_name, stacksize, zpl_true);
	if (task == NULL)
		return ERROR;
	if (os_task_entry_start(task) == OK)
	{
#ifdef HAVE_PTHREAD_SETNAME_NP
		//prctl(PR_SET_NAME,"THREAD2");
		//int pthread_setname_np(zpl_pthread_t thread, const char *name);
		//int pthread_getname_np(zpl_pthread_t thread,
		//                              char *name, size_t len);

		int ret = 0;

		ret = pthread_setname_np(task->td_thread, task->td_name);

		if (ret != OK)
			OS_DEBUG("%s: could not lower privs, %s", __func__, os_strerror(ipstack_errno));
#endif

		OS_DEBUG("\r\ncreate task:%s(%u %u->%u:ret=%d) pid=%d\r\n",task->td_name,
				task->td_thread,(zpl_uint32)task,task->td_id,ret,getpid());
		/*		fprintf(stdout,"\r\ncreate task:%s(%u %u->%u:ret=%d) pid=%d\r\n",task->td_name,
		 task->td_thread,(zpl_uint32)task,task->td_id,ret,getpid());*/
#ifndef OS_TASK_DEBUG
		os_task_finsh_job();
#endif
		if (os_time_load() == OK)
		{
			if (_global_task.os_moniter_id == 0)
			{
				os_task_moniter_init();
			}
		}
		return task->td_id;
	}
	else
	{
		os_task_entry_destroy(task);
		return OK;
	}
	return ERROR;
}



zpl_taskid_t os_task_entry_add(const zpl_char *name, zpl_uint32 pri, zpl_uint32 op,
                         task_entry entry, void *pVoid,
                         zpl_char *func_name, zpl_uint32 stacksize, zpl_uint32 td_thread)
{
	os_task_t * task = os_task_entry_malloc(name, pri, op, entry, pVoid,
			func_name, stacksize, zpl_false);
	if (task == NULL)
	{
		return ERROR;
	}
	task->td_thread = td_thread;
	task->td_id = (zpl_taskid_t)task;
#ifndef OS_TASK_DEBUG
	os_task_finsh_job();
#endif
	if (os_time_load() == OK)
	{
		if (_global_task.os_moniter_id == 0)
		{
			os_task_moniter_init();
		}
	}
	OS_DEBUG("\r\nadd task:%s(%u %u->%u) pid=%d\r\n",task->td_name,
				task->td_thread,(zpl_uint32)task,task->td_id,getpid());
	return task->td_id;
}

int os_task_priv_set(zpl_taskid_t taskId, zpl_pthread_t td_thread, void *priv)
{
	if(taskId > 0)
	{
		os_task_t *osapiTask = os_task_tcb_lookup(taskId, 0, zpl_true);
		if(osapiTask == NULL)
			return ERROR;
		if (osapiTask)
		{
			if (_global_task.task_mutex)
				os_mutex_lock(_global_task.task_mutex, OS_WAIT_FOREVER);
			osapiTask->priv = priv;
			if (_global_task.task_mutex)
				os_mutex_unlock(_global_task.task_mutex);
			return (OK);
		}
	}
	else
	{
		os_task_t *task = os_task_tcb_lookup(0, td_thread, zpl_true);
		if(task == NULL)
			return ERROR;
		if (task)
		{
			if (_global_task.task_mutex)
				os_mutex_lock(_global_task.task_mutex, OS_WAIT_FOREVER);
			task->priv = priv;
			if (_global_task.task_mutex)
				os_mutex_unlock(_global_task.task_mutex);
			return (OK);
		}
	}
	return ERROR;
}

void * os_task_priv_get(zpl_taskid_t taskId, zpl_pthread_t td_thread)
{
	os_task_t *task = NULL;
	void *priv = NULL;
	if(taskId > 0)
	{
		 task = os_task_tcb_lookup(taskId, 0, zpl_true);
		if(task == NULL)
			return NULL;
		if (task)
		{
			if (_global_task.task_mutex)
				os_mutex_lock(_global_task.task_mutex, OS_WAIT_FOREVER);
			priv = task->priv;
			if (_global_task.task_mutex)
				os_mutex_unlock(_global_task.task_mutex);
			return (priv);
		}
	}
	else
	{
		task = os_task_tcb_lookup(0, td_thread, zpl_true);
		if(task == NULL)
			return NULL;
		if (task)
		{
			if (_global_task.task_mutex)
				os_mutex_lock(_global_task.task_mutex, OS_WAIT_FOREVER);
			priv = task->priv;
			if (_global_task.task_mutex)
				os_mutex_unlock(_global_task.task_mutex);
			return (priv);
		}
	}
	return NULL;
}

int os_task_thread_del(zpl_pthread_t td_thread)
{
	if(td_thread == (zpl_pthread_t)ERROR)
		return ERROR;			
	os_task_t *task = os_task_tcb_lookup(0, td_thread, zpl_true);
	if (task)
		return os_task_entry_destroy(task);
	return ERROR;
}

int os_task_thread_refresh_id(zpl_pthread_t td_thread)
{
	if(td_thread == (zpl_pthread_t)ERROR)
		return ERROR;		
	if (_global_task.task_mutex)
		os_mutex_lock(_global_task.task_mutex, OS_WAIT_FOREVER);
	os_task_t *task = os_task_tcb_lookup(0, td_thread, zpl_false);
	if(task)
	{
		task->td_pid = getpid();/* kernel process ID */
		task->td_tid = os_task_gettid();/* kernel LWP ID */
		os_task_refresh_cpu(task);
		//if(task->state > OS_TASK_STATE_CREATE)
	}
	if (_global_task.task_mutex)
		os_mutex_unlock(_global_task.task_mutex);
	return OK;
}

int os_task_foreach(os_task_hook cb, void *p)
{
	NODE node;
	os_task_t *task = NULL;
	if (_global_task.os_task_list == NULL)
		return ERROR;
	if (_global_task.task_mutex)
		os_mutex_lock(_global_task.task_mutex, OS_WAIT_FOREVER);

	for(task = (os_task_t *)lstFirst(_global_task.os_task_list);
			task != NULL;
			task = (os_task_t *)lstNext(&node))
	{
		node = task->node;
		if (task)
		{
			if(cb && p)
				(cb)(p);
			else
			{
				if(cb)
					(cb)(task);
			}
		}
	}
	if (_global_task.task_mutex)
		os_mutex_unlock(_global_task.task_mutex);
	return OK;
}

#ifdef ZPL_SHELL_MODULE
/*
 * for CLI cmd
 */
static int (*_os_task_cli_show)(struct vty *, const char *, ...);

int os_task_cli_hook_set(void *hook)
{
	_os_task_cli_show = hook;
	return OK;
} 

static int os_task_show_head(struct vty *vty, int detail)
{
	//extern int vty_out(struct vty *, const char *, ...);
	if(_os_task_cli_show)
		(_os_task_cli_show)(vty,
			"%s                  CPU (user+system): Real (wall-clock): Mem (K):%s",
			VTY_NEWLINE, VTY_NEWLINE);

	if (detail) 
	{
		if(_os_task_cli_show)
			(_os_task_cli_show)(vty, "  %-16s %-12s %-20s %-8s %-12s %-4s %-8s %-6s %-8s %-8s %-8s %-8s %-8s%s",
				"taskNmae", "taskId", "entry", "LVPID","PTHREAD ID", "pri", "tacksize", "state",
				"Real", "Total", "MemRSS", "MemLimit", "MemTotal", VTY_NEWLINE);
	}
	else 
	{
		if(_os_task_cli_show)
			(_os_task_cli_show)(vty, "  %-16s %-12s %-20s %-4s %-8s %-6s %-8s %-8s%s",
				"taskNmae", "taskId", "entry", "pri", "tacksize", "state",
				"Real", "Total", VTY_NEWLINE);
	}
	return 0;
}

static int os_task_show_detail(void *p, os_task_t *task, int detail)
{
	struct vty *vty = (struct vty *)p;
	char taskId[16];
	char pri[16];
	char tacksize[16];
	char state[16];
	char real[16];
	char lvpid[16];
	char pidstr[16];
	char total[16];
	char mtotal[16];
	char mrss[16];
	char mlimit[16];
	os_memset(taskId, 0, sizeof(taskId));
	os_memset(pri, 0, sizeof(pri));
	os_memset(tacksize, 0, sizeof(tacksize));
	os_memset(state, 0, sizeof(state));
	os_memset(real, 0, sizeof(real));
	os_memset(total, 0, sizeof(total));
	os_memset(lvpid, 0, sizeof(lvpid));
	os_memset(pidstr, 0, sizeof(pidstr));

	os_memset(mtotal, 0, sizeof(mtotal));
	os_memset(mrss, 0, sizeof(mrss));
	os_memset(mlimit, 0, sizeof(mlimit));

	os_task_refresh_total_cpu(&_global_task.total_cpu);

	if (task->td_tid != 0)
	{
		os_task_refresh_cpu(task);
	}
	else
	{
		if(task->state > OS_TASK_STATE_CREATE)
			task->state = OS_TASK_STATE_ZOMBIE;
	}
	os_task_state_to_string(task->state, state);

	sprintf(taskId, "%u", task->td_id);
	sprintf(pri, "%d", task->td_priority);
	sprintf(tacksize, "0x%x", task->td_stack_size);

	if (_global_task.total_cpu.total_realy)
		sprintf(real, "%.2lf%%",
				_global_task.total_cpu.cpu * 100
						* (zpl_double) ((zpl_double) task->hist.total_realy
								/ (zpl_double) _global_task.total_cpu.total_realy));
	else
		sprintf(real, "%s", "0.00%");

	if (_global_task.total_cpu.total)
		sprintf(total, "%.2lf%%",
				_global_task.total_cpu.cpu * 100
						* (zpl_double) ((zpl_double) task->hist.total
								/ (zpl_double) _global_task.total_cpu.total));
	else
		sprintf(total, "%s", "0.00%");

	if(detail)
	{
		sprintf(mtotal, "%uK", task->hist.mtotal);
		sprintf(mrss, "%uK", task->hist.mrss);
		sprintf(mlimit, "%uK", task->hist.mlimit);

		sprintf(lvpid, "%d", task->td_tid);
		sprintf(pidstr, "%u", task->td_thread);
		if(_os_task_cli_show)
			(_os_task_cli_show)(vty, "  %-16s %-12s %-20s %-8s %-12s %-4s %-8s %-6s %-8s %-8s %-8s %-8s %-8s%s",
			task->td_name, taskId, task->td_entry_name, lvpid, pidstr, pri, tacksize, state,
			real, total, mrss, mlimit, mtotal, VTY_NEWLINE);
	}
	else
	{
		if(_os_task_cli_show)
			(_os_task_cli_show)(vty, "  %-16s %-12s %-20s %-4s %-8s %-6s %-8s %-8s%s",
			task->td_name, taskId, task->td_entry_name, pri, tacksize, state,
			real, total, VTY_NEWLINE);
	}
	return 0;
}

int os_task_show(void *vty, const zpl_char *task_name, zpl_uint32 detail)
{
	NODE *node = NULL;
	os_task_t *task = NULL;
	if (_global_task.os_task_list == NULL)
		return ERROR;

	if (task_name)
		task = os_task_node_lookup_by_name(task_name);

	if (task_name && task)
	{
		if (_global_task.task_mutex)
			os_mutex_lock(_global_task.task_mutex, OS_WAIT_FOREVER);
		if (task)
		{
			os_task_show_head(vty, detail);
			os_task_show_detail(vty, task, detail);
		}
		if (_global_task.task_mutex)
			os_mutex_unlock(_global_task.task_mutex);
		return 0;
	}
	if (_global_task.task_mutex)
		os_mutex_lock(_global_task.task_mutex, OS_WAIT_FOREVER);

	node = lstFirst(_global_task.os_task_list);
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
	if (_global_task.task_mutex)
		os_mutex_unlock(_global_task.task_mutex);
	return 0;
}
#endif

#if 0
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
		buffer_put (vty->obuf, (zpl_uchar *) p, len);

		/* If p is not different with buf, it is allocated buffer.  */
		if (p != buf)
		XFREE (MTYPE_VTY_OUT_BUF, p);
	}

	return len;
}
#endif


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


int cmd_os_init(void)
{
	install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_process_cmd);
	install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_process_detail_cmd);

	install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_ipcom_process_cmd);

	return 0;
}
#endif
/*
linux内核的三种 调度策略 ：
SCHED_OTHER 分时调度策略，（默认的）
SCHED_FIFO实时调度策略，先到先服务
SCHED_RR实时调度策略，时间片轮转
      实时进程将得到优先调用，实时进程根据实时优先级决定调度权值，分时进程则通过nice和counter值决定权值，
      	  nice越小，counter越大，被调度的概率越大，也就是曾经使用了cpu最少的进程将会得到优先调度。
SHCED_RR和SCHED_FIFO的不同：

      当采用SHCED_RR策略的进程的时间片用完，系统将重新分配时间片，并置于就绪队列尾。放在队列尾保证了所有具有相同优先级的RR任务的调度公平。

         SCHED_FIFO一旦占用cpu则一直运行。一直运行直到有 更高优先级任务到达或自己放弃 。
         如果有相同优先级的实时进程（根据优先级计算的调度权值是一样的）已经准备好，FIFO时必须等待该进程主动放弃后才可以运行
         	 这个优先级相同的任务。而RR可以让每个任务都执行一段时间。

相同点：

RR和FIFO都只用于实时任务。
创建时优先级大于0(1-99)。
按照可抢占优先级调度算法进行。
就绪态的实时任务立即抢占非实时任务。
当所有任务都采用分时调度策略时（SCHED_OTHER）：
1.创建任务指定采用分时调度策略，并指定优先级nice值(-20~19)。
2.将根据每个任务的nice值确定在cpu上的执行时间( counter )。
3.如果没有等待资源，则将该任务加入到就绪队列中。
4.调度程序遍历就绪队列中的任务，通过对每个任务动态优先级的计算(counter+20-nice)结果，选择计算结果最大的一个去运行，
	当这个时间片用完后(counter减至0)或者主动放弃cpu时，该任务将被放在就绪队列末尾(时间片用完)或等待队列(因等待资源而放弃cpu)中。
5.此时调度程序重复上面计算过程，转到第4步。
6.当调度程序发现所有就绪任务计算所得的权值都为不大于0时，重复第2步。

当所有任务都采用FIFO调度策略时（SCHED_FIFO）：
1.创建进程时指定采用FIFO，并设置实时优先级rt_priority(1-99)。
2.如果没有等待资源，则将该任务加入到就绪队列中。
3.调度程序遍历就绪队列，根据实时优先级计算调度权值,选择权值最高的任务使用cpu， 该FIFO任务将一直占有cpu直到
	有优先级更高的任务就绪(即使优先级相同也不行)或者主动放弃(等待资源)。
4.调度程序发现有优先级更高的任务到达(高优先级任务可能被中断或定时器任务唤醒，再或被当前运行的任务唤醒，等等)，
	则调度程序立即在当前任务堆栈中保存当前cpu寄存器的所有数据，重新从高优先级任务的堆栈中加载寄存器数据到cpu，
		此时高优先级的任务开始运行。重复第3步。
5.如果当前任务因等待资源而主动放弃cpu使用权，则该任务将从就绪队列中删除，加入等待队列，此时重复第3步。
当所有任务都采用RR调度策略（SCHED_RR）时：
1.创建任务时指定调度参数为RR， 并设置任务的实时优先级和nice值(nice值将会转换为该任务的时间片的长度)。
2.如果没有等待资源，则将该任务加入到就绪队列中。
3.调度程序遍历就绪队列，根据实时优先级计算调度权值,选择权值最高的任务使用cpu。
4. 如果就绪队列中的RR任务时间片为0，则会根据nice值设置该任务的时间片，同时将该任务放入就绪队列的末尾 。重复步骤3。
5.当前任务由于等待资源而主动退出cpu，则其加入等待队列中。重复步骤3。
系统中既有分时调度，又有时间片轮转调度和先进先出调度：
1.RR调度和FIFO调度的进程属于实时进程，以分时调度的进程是非实时进程。
2. 当实时进程准备就绪后，如果当前cpu正在运行非实时进程，则实时进程立即抢占非实时进程 。
3. RR进程和FIFO进程都采用实时优先级做为调度的权值标准，RR是FIFO的一个延伸。FIFO时，如果两个进程的优先级一样，
	则这两个优先级一样的进程具体执行哪一个是由其在队列中的未知决定的，这样导致一些不公正性(优先级是一样的，为什么要让你一直运行?),
		如果将两个优先级一样的任务的调度策略都设为RR,则保证了这两个任务可以循环执行，保证了公平。
 */
