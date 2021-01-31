/*
 * module.c
 *
 *  Created on: May 13, 2018
 *      Author: zhurish
 */


#include <zebra.h>
#include "module.h"
#include "thread.h"
#include "eloop.h"
#include "os_ansync.h"



struct module_list module_list_default = 
{ 
	.module=MODULE_DEFAULT, 
	.name="DEFAULT", 
	.module_init=NULL, 
	.module_exit=NULL, 
	.module_task_init=NULL, 
	.module_task_exit=NULL, 
	.module_cmd_init=NULL, 
	.module_write_config=NULL, 
	.module_show_config=NULL,
	.module_show_debug=NULL, 
	.taskid=0,
};

struct module_list module_list_lib = 
{ 
	.module=MODULE_LIB, 
	.name="LIB", 
	.module_init=NULL, 
	.module_exit=NULL, 
	.module_task_init=NULL, 
	.module_task_exit=NULL, 
	.module_cmd_init=NULL, 
	.module_write_config=NULL, 
	.module_show_config=NULL,
	.module_show_debug=NULL, 
	.taskid=0,
};

struct module_list module_list_osal = 
{ 
	.module=MODULE_OSAL, 
	.name="OSAL", 
	.module_init=NULL, 
	.module_exit=NULL, 
	.module_task_init=NULL, 
	.module_task_exit=NULL, 
	.module_cmd_init=NULL, 
	.module_write_config=NULL, 
	.module_show_config=NULL,
	.module_show_debug=NULL, 
	.taskid=0,
};

struct module_list module_list_timer = 
{ 
	.module=MODULE_TIMER, 
	.name="TIMER", 
	.module_init=NULL, 
	.module_exit=NULL, 
	.module_task_init=NULL, 
	.module_task_exit=NULL, 
	.module_cmd_init=NULL, 
	.module_write_config=NULL, 
	.module_show_config=NULL,
	.module_show_debug=NULL, 
	.taskid=0,
};
struct module_list module_list_job = 
{ 
	.module=MODULE_JOB, 
	.name="JOB", 
	.module_init=NULL, 
	.module_exit=NULL, 
	.module_task_init=NULL, 
	.module_task_exit=NULL, 
	.module_cmd_init=NULL, 
	.module_write_config=NULL, 
	.module_show_config=NULL,
	.module_show_debug=NULL, 
	.taskid=0,
};
struct module_list module_list_console = 
{ 
	.module=MODULE_CONSOLE, 
	.name="CONSOLE", 
	.module_init=NULL, 
	.module_exit=NULL, 
	.module_task_init=NULL, 
	.module_task_exit=NULL, 
	.module_cmd_init=NULL, 
	.module_write_config=NULL, 
	.module_show_config=NULL,
	.module_show_debug=NULL, 
	.taskid=0,
};

struct module_list module_list_telnet = 
{ 
	.module=MODULE_TELNET, 
	.name="TELNET", 
	.module_init=NULL, 
	.module_exit=NULL, 
	.module_task_init=NULL, 
	.module_task_exit=NULL, 
	.module_cmd_init=NULL, 
	.module_write_config=NULL, 
	.module_show_config=NULL,
	.module_show_debug=NULL, 
	.taskid=0,
};

struct module_list module_list_utils = 
{ 
	.module=MODULE_UTILS, 
	.name="UTILS", 
	.module_init=NULL, 
	.module_exit=NULL, 
	.module_task_init=NULL, 
	.module_task_exit=NULL, 
	.module_cmd_init=NULL, 
	.module_write_config=NULL, 
	.module_show_config=NULL,
	.module_show_debug=NULL, 
	.taskid=0,
};

struct module_list module_list_imish = 
{ 
	.module=MODULE_IMISH, 
	.name="IMISH", 
	.module_init=NULL, 
	.module_exit=NULL, 
	.module_task_init=NULL, 
	.module_task_exit=NULL, 
	.module_cmd_init=NULL, 
	.module_write_config=NULL, 
	.module_show_config=NULL,
	.module_show_debug=NULL, 
	.taskid=0,
};

struct module_list module_list_kernel = 
{ 
	.module=MODULE_KERNEL, 
	.name="KERNEL", 
	.module_init=NULL, 
	.module_exit=NULL, 
	.module_task_init=NULL, 
	.module_task_exit=NULL, 
	.module_cmd_init=NULL, 
	.module_write_config=NULL, 
	.module_show_config=NULL,
	.module_show_debug=NULL, 
	.taskid=0,
};

int pl_module_name_init(const char * name)
{
	u_int i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && os_strcmp(module_lists_tbl[i].tbl->name, name) == 0)
		{
			if(module_lists_tbl[i].tbl->module_init)
				return (module_lists_tbl[i].tbl->module_init)();
		}	
	}
	return -1;
}

int pl_module_init(int module)
{
	u_int i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->module == (u_int)module)
		{
			if(module_lists_tbl[i].tbl->module_init)
				return (module_lists_tbl[i].tbl->module_init)();
		}	
	}
	return -1;
}

int pl_module_exit(int module)
{
	u_int i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->module == (u_int)module)
		{
			if(module_lists_tbl[i].tbl->module_exit)
				return (module_lists_tbl[i].tbl->module_exit)();
		}	
	}
	return -1;
}

int pl_module_task_name_init(const char * name)
{
	u_int i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && os_strcmp(module_lists_tbl[i].tbl->name, name) == 0)
		{
			if(module_lists_tbl[i].tbl->module_task_init)
				return (module_lists_tbl[i].tbl->module_task_init)();
		}	
	}
	return -1;
}

int pl_module_task_init(int module)
{
	u_int i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->module == (u_int)module)
		{
			if(module_lists_tbl[i].tbl->module_task_init)
				return (module_lists_tbl[i].tbl->module_task_init)();
		}	
	}
	return -1;
}

int pl_module_task_exit(int module)
{
	u_int i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->module == (u_int)module)
		{
			if(module_lists_tbl[i].tbl->module_task_exit)
				return (module_lists_tbl[i].tbl->module_task_exit)();
		}	
	}
	return -1;
}

int pl_module_cmd_name_init(const char * name)
{
	u_int i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && os_strcmp(module_lists_tbl[i].tbl->name, name) == 0)
		{
			if(module_lists_tbl[i].tbl->module_cmd_init)
				return (module_lists_tbl[i].tbl->module_cmd_init)();
		}	
	}
	return -1;
}

int pl_module_cmd_init(int module)
{
	u_int i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->module == (u_int)module)
		{
			if(module_lists_tbl[i].tbl->module_cmd_init)
				return (module_lists_tbl[i].tbl->module_cmd_init)();
		}	
	}
	return -1;
}

const char * module2name(int module)
{
	u_int i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->module == (u_int)module)
			return module_lists_tbl[i].tbl->name;
	}
	return "Unknow";
}

int name2module(const char *name)
{
	u_int i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && os_strcmp(module_lists_tbl[i].tbl->name, name) == 0)
			return module_lists_tbl[i].tbl->module;
	}
	return 0;
}

int module2task(int module)
{
	u_int i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->module == (u_int)module)
			return module_lists_tbl[i].tbl->taskid;
	}
	return 0;
}

int task2module(int taskid)
{
	u_int i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->taskid == (u_int)taskid)
			return module_lists_tbl[i].tbl->module;
	}
	return 0;
}

int task_module_self(void)
{
	u_int i = 0;
	u_int taskid = os_task_id_self ();
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->taskid == taskid)
			return module_lists_tbl[i].tbl->module;
	}
	return 0;
}


int module_setup_task(int module, int taskid)
{
	u_int i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->module == (u_int)module)
		{
			module_lists_tbl[i].tbl->taskid = (u_int)taskid;
			return 0;
		}
	}
	return 0;
}

char *zlog_backtrace_module()
{
	static char backtrace_string[128];
	os_memset(backtrace_string, 0, sizeof(backtrace_string));
	os_snprintf(backtrace_string, sizeof(backtrace_string), "%s [%s]",
			os_task_2_name(os_task_id_self()), module2name(task_module_self()));

	return backtrace_string;
}

char *zlog_backtrace_funcname()
{
#ifdef OS_THREAD
	struct thread *thread_current = thread_current_get();
	if (thread_current)
		return thread_current->funcname;
	else
#endif
#ifdef ELOOP_THREAD
	{
		struct eloop *eloop = eloop_current_get();
		if (eloop)
			return eloop->funcname;
	}
#endif
#ifdef OS_ANSYNC_GLOBAL_LIST
	os_ansync_lst * gansync = os_ansync_global_lookup(os_task_id_self(), task_module_self());
	if (gansync && gansync->os_ansync)
		return gansync->os_ansync->entryname;
#endif
	return "NULL";
}

char *zlog_backtrace_schedfrom()
{
#ifdef OS_THREAD
	struct thread *thread_current = thread_current_get();
	if (thread_current)
		return thread_current->schedfrom;
	else
#endif
#ifdef ELOOP_THREAD
	{
		struct eloop *eloop = eloop_current_get();
		if (eloop)
			return eloop->schedfrom;
	}
#endif
#ifdef OS_ANSYNC_GLOBAL_LIST
	os_ansync_lst * gansync = os_ansync_global_lookup(os_task_id_self(), task_module_self());
	if (gansync && gansync->os_ansync)
		return gansync->os_ansync->filename;
#endif
	return "NULL";
}

int zlog_backtrace_schedfrom_line()
{
#ifdef OS_THREAD
	struct thread *thread_current = thread_current_get();
	if (thread_current)
		return thread_current->schedfrom_line;
	else
#endif
#ifdef ELOOP_THREAD
	{
		struct eloop *eloop = eloop_current_get();
		if (eloop)
			return eloop->schedfrom_line;
	}
#endif
#ifdef OS_ANSYNC_GLOBAL_LIST
	os_ansync_lst * gansync = os_ansync_global_lookup(os_task_id_self(), task_module_self());
	if (gansync && gansync->os_ansync)
		return gansync->os_ansync->line;
#endif
	return 0;
}
