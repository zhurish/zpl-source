/*
 * os_job.c
 *
 *  Created on: Jun 2, 2017
 *      Author: zhurish
 */

#include "zebra.h"
#include "vty.h"
#include "os_list.h"
#include "os_memory.h"
#include "os_sem.h"
#include "os_task.h"
#include "os_job.h"

#define OS_JOB_NAME_MAX 128

typedef struct os_job_s
{
	NODE	node;
	int 	t_id;
	int		(*job_entry)(void *);
	void	*pVoid;
	char    entry_name[OS_JOB_NAME_MAX];
	int		cnt;
}os_job_t;

static LIST *job_list = NULL;
static LIST *job_unused_list = NULL;
static os_sem_t *job_sem = NULL;
static os_mutex_t *job_mutex = NULL;
static unit32 job_task_id = 0;
static int os_job_task(void);

int os_job_init()
{
	if(job_list == NULL)
	{
		job_list = os_malloc(sizeof(LIST));
		if(job_list)
		{
			lstInit(job_list);
			job_sem = os_sem_init();
		}
		job_unused_list = os_malloc(sizeof(LIST));
		if(job_unused_list)
		{
			lstInit(job_unused_list);
		}
		else
			return ERROR;
	}
	if(job_mutex == NULL)
	{
		job_mutex = os_mutex_init();
	}
	job_task_id = os_task_create("jobTask", OS_TASK_DEFAULT_PRIORITY,
	               0, os_job_task, NULL, OS_TASK_DEFAULT_STACK);
	if(job_task_id)
		return OK;

	os_job_exit();
	return ERROR;
}

int os_job_exit()
{
	if(job_task_id)
	{
		if(os_task_destroy(job_task_id)==OK)
			job_task_id = 0;
	}
	if(job_mutex)
	{
		if(os_mutex_exit(job_mutex)==OK)
			job_mutex = NULL;
	}
	if(job_sem)
	{
		if(os_sem_exit(job_sem)==OK)
			job_sem = NULL;
	}
	if(job_list)
	{
		lstFree(job_list);
		job_list = NULL;
	}
	if(job_unused_list)
	{
		lstFree(job_unused_list);
		job_list = NULL;
	}
	return OK;
}

int os_job_load()
{
	if(job_task_id)
		return OK;
	return ERROR;
}

static os_job_t * os_job_entry_create(int	(*job_entry)(void *), void *pVoid, char *entry_name)
{
	os_job_t *t = NULL;
	if(job_unused_list)
		t = lstFirst(job_unused_list);
	if(t == NULL)
		t = os_malloc(sizeof(os_job_t));
	if(t)
	{
		t->t_id = (int)t;
		t->pVoid = pVoid;
		t->job_entry = job_entry;
		os_memset(t->entry_name, 0, sizeof(t->entry_name));
		os_strcpy(t->entry_name, entry_name);
		return t;
	}
	return NULL;
}

int os_job_del(int (*job_entry)(void *), void *pVoid)
{
	NODE node;
	os_job_t *t;
	if (job_mutex)
		os_mutex_lock(job_mutex, OS_WAIT_FOREVER);

	for(t = (os_job_t *)lstFirst(job_list); t != NULL; t = (os_job_t *)lstNext(&node))
	{
		node = t->node;
		if(t && t->job_entry == job_entry && t->pVoid == pVoid)
		{
			lstDelete (job_list, (NODE *)t);
			lstAdd (job_unused_list, (NODE *)t);
			break;
		}
	}
	if(job_mutex)
		os_mutex_unlock(job_mutex);
	return OK;
}

int os_job_add_entry(int (*job_entry)(void *), void *pVoid, char *entry_name)
{
	os_job_t * t = os_job_entry_create(job_entry, pVoid, entry_name);
	if(t)
	{
		if(job_mutex)
			os_mutex_lock(job_mutex, OS_WAIT_FOREVER);
		if(job_list)
			lstAdd(job_list, (NODE *)t);
		if(job_mutex)
			os_mutex_unlock(job_mutex);
		os_sem_give(job_sem);
		return t->t_id;
	}
	return ERROR;
}

/*int os_job_add(int (*job_entry)(void *), void *pVoid)
{
	os_job_t * t = os_job_entry_create(job_entry, pVoid);
	if(t)
	{
		if(job_mutex)
			os_mutex_lock(job_mutex, OS_WAIT_FOREVER);
		if(job_list)
			lstAdd(job_list, (NODE *)t);
		if(job_mutex)
			os_mutex_unlock(job_mutex);
		os_sem_give(job_sem);
		return t->t_id;
	}
	return ERROR;
}*/



static int os_job_task(void)
{
	NODE node;
	os_job_t *t;
	while(!os_load_config_done())
	{
		os_sleep(1);
	}
	while(1)
	{
		os_sem_take(job_sem, OS_WAIT_FOREVER);

		if(job_mutex)
			os_mutex_lock(job_mutex, OS_WAIT_FOREVER);
		for(t = lstFirst(job_list); t != NULL; t = lstNext(&node))
		{
			if(t)
			{
				node = t->node;

				if(t && t->job_entry)
				{
					if(job_mutex)
						os_mutex_unlock(job_mutex);
					(t->job_entry)(t->pVoid);
					t->cnt++;
					if(job_mutex)
						os_mutex_lock(job_mutex, OS_WAIT_FOREVER);
				}
				lstDelete(job_list, t);
				if(job_unused_list)
					lstAdd(job_unused_list, t);
			}
		}
		if(job_mutex)
			os_mutex_unlock(job_mutex);
	}
	return OK;
}

int os_job_show(void *pvoid)
{
	int i = 0;
	NODE *node;
	os_job_t *t;
	struct vty *vty = (struct vty *)pvoid;
	if (job_mutex)
		os_mutex_lock(job_mutex, OS_WAIT_FOREVER);
	if(lstCount(job_list) || lstCount(job_unused_list) )
	{
		vty_out(vty, "%-4s %-4s %-6s %-16s %s", "----", "----", "------", "----------------", VTY_NEWLINE);
		vty_out(vty, "%-4s %-4s %-6s %-16s %s", "ID", "CNT", "STATE", "NAME", VTY_NEWLINE);
		vty_out(vty, "%-4s %-4s %-6s %-16s %s", "----", "----", "------", "----------------", VTY_NEWLINE);
	}
	for (node = lstFirst(job_list); node != NULL; node = lstNext(node))
	{
		t = (os_job_t *) node;
		if (node)
		{
			vty_out(vty, "%-4d  %-4d %-6s %s%s", i++, t->cnt, "READY", t->entry_name, VTY_NEWLINE);
		}
	}
	for (node = lstFirst(job_unused_list); node != NULL; node = lstNext(node))
	{
		t = (os_job_t *) node;
		if (node)
		{
			vty_out(vty, "%-4d  %-4d %-6s %s%s", i++, t->cnt, "READY", t->entry_name, VTY_NEWLINE);
		}
	}
	if (job_mutex)
		os_mutex_unlock(job_mutex);
	return OK;
}
