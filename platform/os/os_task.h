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
	zpl_llong total_realy;
	zpl_llong total;
	zpl_int32 cpu;
};

typedef int(*os_task_hook)(void *);


typedef struct os_task_s
{
	NODE		node;
	zpl_bool		active;
	zpl_uint32		td_id;		/* task id */
	zpl_char 		td_name[TASK_NAME_MAX];	/* name of task */
	zpl_uint32		td_name_key;
	zpl_pthread_t 	td_thread; /* POSIX thread ID */

	zpl_pid_t		td_pid;/* kernel process ID */
	zpl_pid_t		td_tid;/* kernel LWP ID */

	zpl_sem_t  		td_sem;
	zpl_pthread_mutex_t td_mutex;
	zpl_pthread_cond_t 	td_control;
	zpl_pthread_attr_t 	td_attr;
	zpl_pthread_spinlock_t td_spinlock;
	zpl_sched_param_t td_param;
	


	zpl_uint32			td_priority;	/* task priority */
	zpl_uint32			td_old_priority;
	zpl_uint32			td_status;	/* task status */
	zpl_uint32			td_options;	/* task option bits (see below) */
	zpl_uint32			td_stack_size;	/* size of stack in bytes */

	task_entry	td_entry;
	zpl_void 		*pVoid;

	zpl_char 		td_entry_name[TASK_NAME_MAX*2];

	enum os_task_state state;
	struct os_task_history hist;

	zpl_void 		*priv;
}os_task_t;

//typedef os_task_t * os_task_pt;



extern int os_limit_stack_size(zpl_int32 size);
extern int os_limit_core_size(zpl_int32 size);
extern int os_task_give_broadcast(void);

extern int os_task_init();
extern int os_task_exit();
extern int os_task_sigmask(zpl_int32 sigc, zpl_int32 signo[], sigset_t *mask);
extern int os_task_sigexecute(zpl_int32 sigc, zpl_int32 signo[], sigset_t *mask);
extern int os_task_sigmaskall();

extern int os_task_add_start_hook(os_task_hook *cb);
extern int os_task_add_create_hook(os_task_hook *cb);
extern int os_task_add_destroy_hook(os_task_hook *cb);

extern int os_task_del_start_hook(os_task_hook *cb);
extern int os_task_del_create_hook(os_task_hook *cb);
extern int os_task_del_destroy_hook(os_task_hook *cb);

extern os_task_t * os_task_tcb_get(zpl_uint32 id, zpl_pthread_t pid);
extern os_task_t * os_task_tcb_self(void);
extern zpl_pthread_t os_task_pthread_self( void);
extern zpl_uint32 os_task_id_self( void);
extern int os_task_gettid( void);
extern const zpl_char * os_task_self_name_alisa(void);

extern int os_task_name_get( zpl_uint32 task_id, char *task_name);
extern zpl_char * os_task_2_name( zpl_uint32 task_id);
extern zpl_uint32 os_task_lookup_by_name(char *task_name);
extern int os_task_priority_set(zpl_uint32 TaskID, zpl_uint32 Priority);
extern int os_task_priority_get(zpl_uint32 TaskID, zpl_uint32 *Priority);
extern int os_task_yield ( void );
extern int os_task_delay(zpl_int32 ticks);

extern int os_task_supend(zpl_uint32 TaskID);
extern int os_task_resume(zpl_uint32 TaskID);

extern int os_task_priv_set(zpl_uint32 TaskID, zpl_pthread_t td_thread, zpl_void *priv);
extern zpl_void * os_task_priv_get(zpl_uint32 TaskID, zpl_pthread_t td_thread);

extern int os_task_refresh_id(zpl_uint32 td_thread);
extern int os_task_del(zpl_uint32 td_thread);
extern int os_task_foreach(os_task_hook cb, void *p);

extern int os_task_entry_destroy(os_task_t *task);
extern int os_task_destroy(zpl_uint32 taskId);

extern zpl_uint32 os_task_entry_create(zpl_char *name, zpl_uint32 pri, zpl_uint32 op,
                         task_entry entry, void *pVoid,
                         zpl_char *func_name, zpl_uint32 stacksize);

#define os_task_create(n,p,o,f,a,s)	os_task_entry_create(n,p,o,f,a,#f,s)
//#define os_task_destroy(i)	os_task_entry_create(i)

extern zpl_uint32 os_task_entry_add(zpl_char *name, zpl_uint32 pri, zpl_uint32 op,
                         task_entry entry, void *pVoid,
                         zpl_char *func_name, zpl_uint32 stacksize, zpl_uint32 td_thread);

#define os_task_add(n,p,o,f,a,s,i)	os_task_entry_add(n,p,o,f,a,#f,s,i)
#define os_task_add_name(n,p,o,f,e,a,s,i)	os_task_entry_add(n,p,o,f,a,e,s,i)

#ifdef ZPL_SHELL_MODULE
extern int cmd_os_init();

extern int os_task_show(void *vty, zpl_char *task_name, zpl_uint32 detail);
extern int os_task_cli_hook_set(void *hook);
#endif

#ifdef __cplusplus
}
#endif


#endif /* __OS_TASK_H__ */
