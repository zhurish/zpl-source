/*
 * os_task.h
 *
 *  Created on: May 30, 2017
 *      Author: zhurish
 */

#ifndef __OS_TASK_H__
#define __OS_TASK_H__

#define TASK_NAME_MAX	32
#define TASK_COUNT_MAX	64


#define OS_TASK_TICK_MSEC 		(10 * 1000000)

#define OS_TASK_MAX_PRIORITY 	(255)

#define OS_TASK_TIME_SLICED    	(0x00000001)
#define OS_TASK_DELETE_SAFE    	(0x00000002)
#define OS_TASK_DELETED        	(0x00000004)

#define OS_TASK_DEFAULT_STACK		(1024 * 32)
#define OS_TASK_STACK_MIN			(1024 * 4)
#define OS_TASK_DEFAULT_SLICE		(1)
#define OS_TASK_DEFAULT_PRIORITY	(255)


#define OS_TASK_DEBUG


typedef int(*task_entry)(void *);

typedef  unsigned int unit32;

enum os_task_state
{
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
	unsigned long total_realy;
	//unsigned long total_300_sec;
	unsigned long total;
	int cpu;
};

typedef struct os_task_s
{
	NODE		node;
	unit32	td_id;		/* task id */
	char 		td_name[TASK_NAME_MAX];	/* name of task */
	unit32	td_name_key;
	pthread_t 	td_thread; /* POSIX thread ID */

	pid_t		td_pid;/* kernel process ID */
	pid_t		td_tid;/* kernel LWP ID */

	sem_t  		td_sem;
	pthread_mutex_t td_mutex;
	pthread_cond_t 	td_control;
	pthread_attr_t 	td_attr;
	pthread_spinlock_t td_spinlock;
	struct sched_param td_param;

	int			td_priority;	/* task priority */
	int			td_old_priority;
	int			td_status;	/* task status */
	int			td_options;	/* task option bits (see below) */
	int			td_stack_size;	/* size of stack in bytes */

	task_entry	td_entry;
	void 		*pVoid;

	char 		td_entry_name[TASK_NAME_MAX*2];

	enum os_task_state state;
	struct os_task_history hist;
}os_task_t;

//typedef os_task_t * os_task_pt;



#define OS_TASK_DEFAULT_PRIORITY	(255)

extern int os_task_init();
extern int os_task_exit();

extern pthread_t os_task_pthread_self( void);
extern unit32 os_task_id_self( void);
extern int os_task_gettid( void);

extern int os_task_name_get( unit32 task_id, char *task_name);
extern char * os_task_2_name( unit32 task_id);
extern unit32 os_task_lookup_by_name(char *task_name);
extern int os_task_priority_set(unit32 TaskID, int Priority);
extern int os_task_priority_get(unit32 TaskID, int *Priority);
extern int os_task_yield ( void );
extern int os_task_delay(int ticks);

extern int os_task_supend(unit32 TaskID);
extern int os_task_resume(unit32 TaskID);

extern unit32 os_task_entry_create(char *name, int pri, int op,
                         task_entry entry, void *pVoid,
                         char *func_name, int stacksize);

extern int os_task_entry_destroy(os_task_t *task);
extern int os_task_destroy(unit32 taskId);

#define os_task_create(n,p,o,f,a,s)	os_task_entry_create(n,p,o,f,a,#f,s)
//#define os_task_destroy(i)	os_task_entry_create(i)

extern int cmd_os_init();
extern int os_task_show(void *vty, char *task_name, int detail);


#endif /* __OS_TASK_H__ */
