/*
 * os_job.c
 *
 *  Created on: Jun 2, 2017
 *      Author: zhurish
 */
#include "auto_include.h"
#include "zplos_include.h"
#ifdef ZPL_SHELL_MODULE
#include "vty.h"
#endif
#define OS_JOB_NAME_MAX 128

typedef struct os_job_s
{
	NODE	node;
	zpl_uint32 	t_id;
	int		(*job_entry)(void *);
	zpl_void	*pVoid;
	zpl_char    entry_name[OS_JOB_NAME_MAX];
	zpl_uint32		cnt;
}os_job_t;


typedef struct os_gjob_s
{
	LIST *job_list;
	LIST *job_unused_list;
	os_sem_t *job_sem;
	os_mutex_t *job_mutex;
	zpl_taskid_t job_task_id;
}os_global_job_t;

static os_global_job_t _global_job;
static int os_job_task(void*p);

int os_job_init(void)
{	
	memset(&_global_job, 0, sizeof(os_global_job_t));
	if(_global_job.job_list == NULL)
	{
		_global_job.job_list = os_malloc(sizeof(LIST));
		if(_global_job.job_list)
		{
			lstInit(_global_job.job_list);
			_global_job.job_sem = os_sem_name_create("job_sem");
		}
		_global_job.job_unused_list = os_malloc(sizeof(LIST));
		if(_global_job.job_unused_list)
		{
			lstInit(_global_job.job_unused_list);
		}
		else
			return ERROR;
	}
	if(_global_job.job_mutex == NULL)
	{
		_global_job.job_mutex = os_mutex_name_create("jobmutex");
	}
	_global_job.job_task_id = os_task_create("jobTask", OS_TASK_DEFAULT_PRIORITY,
	               0, os_job_task, NULL, OS_TASK_DEFAULT_STACK);
	if(_global_job.job_task_id)
		return OK;

	os_job_exit();
	return ERROR;
}

int os_job_exit(void)
{
	if(_global_job.job_task_id)
	{
		if(os_task_destroy(_global_job.job_task_id)==OK)
			_global_job.job_task_id = 0;
	}
	if(_global_job.job_mutex)
	{
		if(os_mutex_destroy(_global_job.job_mutex)==OK)
			_global_job.job_mutex = NULL;
	}
	if(_global_job.job_sem)
	{
		if(os_sem_destroy(_global_job.job_sem)==OK)
			_global_job.job_sem = NULL;
	}
	if(_global_job.job_list)
	{
		lstFree(_global_job.job_list);
		_global_job.job_list = NULL;
	}
	if(_global_job.job_unused_list)
	{
		lstFree(_global_job.job_unused_list);
		_global_job.job_list = NULL;
	}
	return OK;
}

int os_job_load(void)
{
	if(_global_job.job_task_id)
		return OK;
	return ERROR;
}

static os_job_t * os_job_entry_create(int type, int (*job_entry)(void *), void *pVoid, zpl_char *entry_name)
{
	os_job_t *t = NULL;
	if(_global_job.job_unused_list)
		t = (os_job_t *)lstFirst(_global_job.job_unused_list);
	if(t == NULL)
		t = os_malloc(sizeof(os_job_t));
	if(t)
	{
		t->t_id = (zpl_uint32)t;
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
	if (_global_job.job_mutex)
		os_mutex_lock(_global_job.job_mutex, OS_WAIT_FOREVER);

	for(t = (os_job_t *)lstFirst(_global_job.job_list); t != NULL; t = (os_job_t *)lstNext(&node))
	{
		node = t->node;
		if(t && t->job_entry == job_entry && t->pVoid == pVoid)
		{
			lstDelete (_global_job.job_list, (NODE *)t);
			lstAdd (_global_job.job_unused_list, (NODE *)t);
			break;
		}
	}
	if(_global_job.job_mutex)
		os_mutex_unlock(_global_job.job_mutex);
	return OK;
}

int os_job_add_entry(int type, int (*job_entry)(void *), void *pVoid, const zpl_char *entry_name)
{
	os_job_t * t = os_job_entry_create(type, job_entry, pVoid, entry_name);
	if(t)
	{
		if(_global_job.job_mutex)
			os_mutex_lock(_global_job.job_mutex, OS_WAIT_FOREVER);
		if(_global_job.job_list)
			lstAdd(_global_job.job_list, (NODE *)t);
		if(_global_job.job_mutex)
			os_mutex_unlock(_global_job.job_mutex);
		os_sem_give(_global_job.job_sem);
		return t->t_id;
	}
	return ERROR;
}

/*int os_job_add(int (*job_entry)(void *), void *pVoid)
{
	os_job_t * t = os_job_entry_create(job_entry, pVoid);
	if(t)
	{
		if(_global_job.job_mutex)
			os_mutex_lock(_global_job.job_mutex, OS_WAIT_FOREVER);
		if(_global_job.job_list)
			lstAdd(_global_job.job_list, (NODE *)t);
		if(_global_job.job_mutex)
			os_mutex_unlock(_global_job.job_mutex);
		os_sem_give(_global_job.job_sem);
		return t->t_id;
	}
	return ERROR;
}*/



static int os_job_task(void *p)
{
	NODE node;
	os_job_t *t;
	//host_waitting_loadconfig();
	while(OS_TASK_TRUE())
	{
		os_sem_take(_global_job.job_sem, OS_WAIT_FOREVER);

		if(_global_job.job_mutex)
			os_mutex_lock(_global_job.job_mutex, OS_WAIT_FOREVER);
		for(t = (os_job_t *)lstFirst(_global_job.job_list); t != NULL; t = (os_job_t *)lstNext(&node))
		{
			if(t)
			{
				node = t->node;

				if(t && t->job_entry)
				{
					if(_global_job.job_mutex)
						os_mutex_unlock(_global_job.job_mutex);
					(t->job_entry)(t->pVoid);
					if(_global_job.job_mutex)
						os_mutex_lock(_global_job.job_mutex, OS_WAIT_FOREVER);
					t->cnt++;
				}
				lstDelete(_global_job.job_list, (NODE*)t);
				if(_global_job.job_unused_list)
					lstAdd(_global_job.job_unused_list, (NODE*)t);
			}
		}
		if(_global_job.job_mutex)
			os_mutex_unlock(_global_job.job_mutex);
	}
	return OK;
}

int os_job_show(void *pvoid)
{
	zpl_uint32 i = 0;
	NODE *node;
	os_job_t *t;
#ifdef ZPL_SHELL_MODULE	
	struct vty *vty = (struct vty *)pvoid;
#endif	
	if (_global_job.job_mutex)
		os_mutex_lock(_global_job.job_mutex, OS_WAIT_FOREVER);
#ifdef ZPL_SHELL_MODULE
	if(lstCount(_global_job.job_list) || lstCount(_global_job.job_unused_list) )
	{
		vty_out(vty, "%-4s %-4s %-6s %-16s %s", "----", "----", "------", "----------------", VTY_NEWLINE);
		vty_out(vty, "%-4s %-4s %-6s %-16s %s", "ID", "CNT", "STATE", "NAME", VTY_NEWLINE);
		vty_out(vty, "%-4s %-4s %-6s %-16s %s", "----", "----", "------", "----------------", VTY_NEWLINE);
	}
	for (node = lstFirst(_global_job.job_list); node != NULL; node = lstNext(node))
	{
		t = (os_job_t *) node;
		if (node)
		{
			vty_out(vty, "%-4d  %-4d %-6s %s%s", i++, t->cnt, "READY", t->entry_name, VTY_NEWLINE);
		}
	}
	for (node = lstFirst(_global_job.job_unused_list); node != NULL; node = lstNext(node))
	{
		t = (os_job_t *) node;
		if (node)
		{
			vty_out(vty, "%-4d  %-4d %-6s %s%s", i++, t->cnt, "READY", t->entry_name, VTY_NEWLINE);
		}
	}
#else
	if(lstCount(_global_job.job_list) || lstCount(_global_job.job_unused_list) )
	{
		fprintf(stdout, "%-4s %-4s %-6s %-16s %s", "----", "----", "------", "----------------", VTY_NEWLINE);
		fprintf(stdout, "%-4s %-4s %-6s %-16s %s", "ID", "CNT", "STATE", "NAME", VTY_NEWLINE);
		fprintf(stdout, "%-4s %-4s %-6s %-16s %s", "----", "----", "------", "----------------", VTY_NEWLINE);
	}
	for (node = lstFirst(_global_job.job_list); node != NULL; node = lstNext(node))
	{
		t = (os_job_t *) node;
		if (node)
		{
			fprintf(stdout, "%-4d  %-4d %-6s %s%s", i++, t->cnt, "READY", t->entry_name, VTY_NEWLINE);
		}
	}
	for (node = lstFirst(_global_job.job_unused_list); node != NULL; node = lstNext(node))
	{
		t = (os_job_t *) node;
		if (node)
		{
			fprintf(stdout, "%-4d  %-4d %-6s %s%s", i++, t->cnt, "READY", t->entry_name, VTY_NEWLINE);
		}
	}
	fflush(stdout);
#endif	
	if (_global_job.job_mutex)
		os_mutex_unlock(_global_job.job_mutex);
	return OK;
}
