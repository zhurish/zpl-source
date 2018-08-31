/*
 * os_job.c
 *
 *  Created on: Jun 2, 2017
 *      Author: zhurish
 */

#include "zebra.h"

#include "os_list.h"
#include "os_memory.h"
#include "os_sem.h"
#include "os_task.h"
#include "os_job.h"

typedef struct os_job_s
{
	NODE	node;
	int 	t_id;
	int		(*job_entry)(void *);
	void	*pVoid;
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

int os_job_finsh()
{
	if(job_task_id)
		return OK;
	return ERROR;
}

static os_job_t * os_job_entry_create(int	(*job_entry)(void *), void *pVoid)
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
		return t;
	}
	return NULL;
}



int os_job_add(int (*job_entry)(void *), void *pVoid)
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
}



static int os_job_task(void)
{
	NODE *node;
	os_job_t *t;
	while(1)
	{
		os_sem_take(job_sem, OS_WAIT_FOREVER);

		if(job_mutex)
			os_mutex_lock(job_mutex, OS_WAIT_FOREVER);
		for(node = lstFirst(job_list); node; node = lstNext(node))
		{
			if(node)
			{
				t = (os_job_t *)node;

				if(t && t->job_entry)
				{
					(t->job_entry)(t->pVoid);
				}
				lstDelete(job_list, node);
				if(job_unused_list)
					lstAdd(job_unused_list, node);
			}
		}
		if(job_mutex)
			os_mutex_unlock(job_mutex);
	}
	return OK;
}
