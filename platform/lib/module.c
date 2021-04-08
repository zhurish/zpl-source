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

int pl_module_name_show()
{
	ospl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->name != NULL)
		{
			zlog_force_trap(MODULE_DEFAULT, "module : %s", module_lists_tbl[i].tbl->name);	
		}	
	}
	return -1;
}

int pl_module_name_init(const char * name)
{
	ospl_uint32 i = 0;
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

int pl_module_init(ospl_uint32 module)
{
	ospl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->module == (ospl_uint32)module)
		{
			if(module_lists_tbl[i].tbl->module_init)
				return (module_lists_tbl[i].tbl->module_init)();
		}	
	}
	return -1;
}

int pl_module_exit(ospl_uint32 module)
{
	ospl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->module == (ospl_uint32)module)
		{
			if(module_lists_tbl[i].tbl->module_exit)
				return (module_lists_tbl[i].tbl->module_exit)();
		}	
	}
	return -1;
}

int pl_module_task_name_init(const char * name)
{
	ospl_uint32 i = 0;
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

int pl_module_task_init(ospl_uint32 module)
{
	ospl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->module == (ospl_uint32)module)
		{
			if(module_lists_tbl[i].tbl->module_task_init)
				return (module_lists_tbl[i].tbl->module_task_init)();
		}	
	}
	return -1;
}

int pl_module_task_exit(ospl_uint32 module)
{
	ospl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->module == (ospl_uint32)module)
		{
			if(module_lists_tbl[i].tbl->module_task_exit)
				return (module_lists_tbl[i].tbl->module_task_exit)();
		}	
	}
	return -1;
}

int pl_module_cmd_name_init(const char * name)
{
	ospl_uint32 i = 0;
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

int pl_module_cmd_init(ospl_uint32 module)
{
	ospl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->module == (ospl_uint32)module)
		{
			if(module_lists_tbl[i].tbl->module_cmd_init)
				return (module_lists_tbl[i].tbl->module_cmd_init)();
		}	
	}
	return -1;
}

const char * module2name(ospl_uint32 module)
{
	ospl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->module == (ospl_uint32)module)
			return module_lists_tbl[i].tbl->name;
	}
	return "Unknow";
}

ospl_uint32 name2module(const char *name)
{
	ospl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && os_strcmp(module_lists_tbl[i].tbl->name, name) == 0)
			return module_lists_tbl[i].tbl->module;
	}
	return 0;
}

ospl_uint32 module2task(ospl_uint32 module)
{
	ospl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->module == (ospl_uint32)module)
			return module_lists_tbl[i].tbl->taskid;
	}
	return 0;
}

ospl_uint32 task2module(ospl_uint32 taskid)
{
	ospl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->taskid == (ospl_uint32)taskid)
			return module_lists_tbl[i].tbl->module;
	}
	return 0;
}

ospl_uint32 task_module_self(void)
{
	ospl_uint32 i = 0;
	ospl_uint32 taskid = os_task_id_self ();
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->taskid == taskid)
			return module_lists_tbl[i].tbl->module;
	}
	return 0;
}


int module_setup_task(ospl_uint32 module, ospl_uint32 taskid)
{
	ospl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->module == (ospl_uint32)module)
		{
			module_lists_tbl[i].tbl->taskid = (ospl_uint32)taskid;
			return 0;
		}
	}
	return 0;
}

ospl_char *zlog_backtrace_module()
{
	static ospl_char backtrace_string[128];
	os_memset(backtrace_string, 0, sizeof(backtrace_string));
	os_snprintf(backtrace_string, sizeof(backtrace_string), "%s [%s]",
			os_task_2_name(os_task_id_self()), module2name(task_module_self()));

	return backtrace_string;
}

ospl_char *zlog_backtrace_funcname()
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

ospl_char *zlog_backtrace_schedfrom()
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

ospl_uint32 zlog_backtrace_schedfrom_line()
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
