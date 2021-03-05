/*
 * os_task.h
 *
 *  Created on: May 30, 2017
 *      Author: zhurish
 */

#ifndef __OS_TASK_H__
#define __OS_TASK_H__

#ifdef __cplusplus
extern "C" {
#endif

//#define OS_SIGNAL_SIGWAIT


#define TASK_NAME_MAX	32
#define TASK_COUNT_MAX	64
#define TASK_CBTBL_MAX	16

#define OS_TASK_TICK_MSEC 		(10 * 1000000)

#define OS_TASK_MAX_PRIORITY 	(256)
#define OS_TASK_STACK_MIN		(1024 * 4)
#define OS_TASK_STACK_MAX		(1024 * 1024)

/* option */
#define OS_TASK_TIME_SLICED    	(0x00000001)
#define OS_TASK_DELETE_SAFE    	(0x00000002)
#define OS_TASK_DELETED        	(0x00000004)

/* default value */
#define OS_TASK_DEFAULT_STACK		(1024 * 32)
#define OS_TASK_DEFAULT_SLICE		(1)
#define OS_TASK_DEFAULT_PRIORITY	OS_TASK_MAX_PRIORITY-1


#define OS_TASK_DEBUG
//#define OS_TASK_DEBUG_LOG

typedef int(*task_entry)(void *);

enum os_task_state
{
	OS_STATE_CREATE,//C
	OS_STATE_READY = 1,//R
	OS_STATE_RUNNING,//R
	OS_STATE_WAIT,//W
	OS_STATE_SLEEP,//S
	OS_STATE_STOP,//T
	OS_STATE_DEAD,//D
	OS_STATE_ZOMBIE,//Z
};

struct os_task_history
{
	ospl_llong total_realy;
	ospl_llong total;
	ospl_int32 cpu;
};

typedef int(*os_task_hook)(void *);


typedef struct os_task_s
{
	NODE		node;
	ospl_bool		active;
	ospl_uint32		td_id;		/* task id */
	ospl_char 		td_name[TASK_NAME_MAX];	/* name of task */
	ospl_uint32		td_name_key;
	ospl_pthread_t 	td_thread; /* POSIX thread ID */

	ospl_pid_t		td_pid;/* kernel process ID */
	ospl_pid_t		td_tid;/* kernel LWP ID */

	ospl_sem_t  		td_sem;
	ospl_pthread_mutex_t td_mutex;
	ospl_pthread_cond_t 	td_control;
	ospl_pthread_attr_t 	td_attr;
	ospl_pthread_spinlock_t td_spinlock;
	ospl_sched_param_t td_param;
	


	ospl_uint32			td_priority;	/* task priority */
	ospl_uint32			td_old_priority;
	ospl_uint32			td_status;	/* task status */
	ospl_uint32			td_options;	/* task option bits (see below) */
	ospl_uint32			td_stack_size;	/* size of stack in bytes */

	task_entry	td_entry;
	ospl_void 		*pVoid;

	ospl_char 		td_entry_name[TASK_NAME_MAX*2];

	enum os_task_state state;
	struct os_task_history hist;

	ospl_void 		*priv;
}os_task_t;

//typedef os_task_t * os_task_pt;



extern int os_limit_stack_size(ospl_int32 size);
extern int os_limit_core_size(ospl_int32 size);
extern int os_task_give_broadcast(void);

extern int os_task_init();
extern int os_task_exit();
extern int os_task_sigmask(ospl_int32 sigc, ospl_int32 signo[], sigset_t *mask);
extern int os_task_sigexecute(ospl_int32 sigc, ospl_int32 signo[], sigset_t *mask);
extern int os_task_sigmaskall();

extern int os_task_add_start_hook(os_task_hook *cb);
extern int os_task_add_create_hook(os_task_hook *cb);
extern int os_task_add_destroy_hook(os_task_hook *cb);

extern int os_task_del_start_hook(os_task_hook *cb);
extern int os_task_del_create_hook(os_task_hook *cb);
extern int os_task_del_destroy_hook(os_task_hook *cb);

extern os_task_t * os_task_tcb_get(ospl_uint32 id, ospl_pthread_t pid);
extern os_task_t * os_task_tcb_self(void);
extern ospl_pthread_t os_task_pthread_self( void);
extern ospl_uint32 os_task_id_self( void);
extern int os_task_gettid( void);
extern const ospl_char * os_task_self_name_alisa(void);

extern int os_task_name_get( ospl_uint32 task_id, char *task_name);
extern ospl_char * os_task_2_name( ospl_uint32 task_id);
extern ospl_uint32 os_task_lookup_by_name(char *task_name);
extern int os_task_priority_set(ospl_uint32 TaskID, ospl_uint32 Priority);
extern int os_task_priority_get(ospl_uint32 TaskID, ospl_uint32 *Priority);
extern int os_task_yield ( void );
extern int os_task_delay(ospl_int32 ticks);

extern int os_task_supend(ospl_uint32 TaskID);
extern int os_task_resume(ospl_uint32 TaskID);

extern int os_task_priv_set(ospl_uint32 TaskID, ospl_pthread_t td_thread, ospl_void *priv);
extern ospl_void * os_task_priv_get(ospl_uint32 TaskID, ospl_pthread_t td_thread);

extern int os_task_refresh_id(ospl_uint32 td_thread);
extern int os_task_del(ospl_uint32 td_thread);
extern int os_task_foreach(os_task_hook cb, void *p);

extern int os_task_entry_destroy(os_task_t *task);
extern int os_task_destroy(ospl_uint32 taskId);

extern ospl_uint32 os_task_entry_create(ospl_char *name, ospl_uint32 pri, ospl_uint32 op,
                         task_entry entry, void *pVoid,
                         ospl_char *func_name, ospl_uint32 stacksize);

#define os_task_create(n,p,o,f,a,s)	os_task_entry_create(n,p,o,f,a,#f,s)
//#define os_task_destroy(i)	os_task_entry_create(i)

extern ospl_uint32 os_task_entry_add(ospl_char *name, ospl_uint32 pri, ospl_uint32 op,
                         task_entry entry, void *pVoid,
                         ospl_char *func_name, ospl_uint32 stacksize, ospl_uint32 td_thread);

#define os_task_add(n,p,o,f,a,s,i)	os_task_entry_add(n,p,o,f,a,#f,s,i)
#define os_task_add_name(n,p,o,f,e,a,s,i)	os_task_entry_add(n,p,o,f,a,e,s,i)

extern int cmd_os_init();
extern int os_task_show(void *vty, ospl_char *task_name, ospl_uint32 detail);
extern int os_task_cli_hook_set(void *hook);


#ifdef __cplusplus
}
#endif


#endif /* __OS_TASK_H__ */
