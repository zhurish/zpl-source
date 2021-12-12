/*
 * module.c
 *
 *  Created on: May 13, 2018
 *      Author: zhurish
 */


#include "os_include.h"
#include "zpl_include.h"
#include "lib_include.h"



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
	zpl_uint32 i = 0;
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
	zpl_uint32 i = 0;
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

int pl_module_init(zpl_uint32 module)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->module == (zpl_uint32)module)
		{
			if(module_lists_tbl[i].tbl->module_init)
				return (module_lists_tbl[i].tbl->module_init)();
		}	
	}
	return -1;
}

int pl_module_exit(zpl_uint32 module)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->module == (zpl_uint32)module)
		{
			if(module_lists_tbl[i].tbl->module_exit)
				return (module_lists_tbl[i].tbl->module_exit)();
		}	
	}
	return -1;
}

int pl_module_task_name_init(const char * name)
{
	zpl_uint32 i = 0;
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

int pl_module_task_init(zpl_uint32 module)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->module == (zpl_uint32)module)
		{
			if(module_lists_tbl[i].tbl->module_task_init)
				return (module_lists_tbl[i].tbl->module_task_init)();
		}	
	}
	return -1;
}

int pl_module_task_exit(zpl_uint32 module)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->module == (zpl_uint32)module)
		{
			if(module_lists_tbl[i].tbl->module_task_exit)
				return (module_lists_tbl[i].tbl->module_task_exit)();
		}	
	}
	return -1;
}

int pl_module_cmd_name_init(const char * name)
{
	zpl_uint32 i = 0;
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

int pl_module_cmd_init(zpl_uint32 module)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->module == (zpl_uint32)module)
		{
			if(module_lists_tbl[i].tbl->module_cmd_init)
				return (module_lists_tbl[i].tbl->module_cmd_init)();
		}	
	}
	return -1;
}

const char * module2name(zpl_uint32 module)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->module == (zpl_uint32)module)
			return module_lists_tbl[i].tbl->name;
		else
		{
			zpl_uint32 j = 0;
			for(j = 0; j < ZPL_SUB_MODULE_MAX; j++)
			{
				if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->submodule[j].module == (zpl_uint32)module)
					return module_lists_tbl[i].tbl->submodule[j].name;
			}
		}	
	}
	return "Unknow";
}

zpl_uint32 name2module(const char *name)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && os_strcmp(module_lists_tbl[i].tbl->name, name) == 0)
			return module_lists_tbl[i].tbl->module;
		else
		{
			zpl_uint32 j = 0;
			for(j = 0; j < ZPL_SUB_MODULE_MAX; j++)
			{
				if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->submodule[j].name && 
					os_strcmp(module_lists_tbl[i].tbl->submodule[j].name, name) == 0)
					return module_lists_tbl[i].tbl->submodule[j].module;
			}
		}
	}
	return 0;
}

zpl_uint32 module2task(zpl_uint32 module)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->module == (zpl_uint32)module)
			return module_lists_tbl[i].tbl->taskid;
		else
		{
			zpl_uint32 j = 0;
			for(j = 0; j < ZPL_SUB_MODULE_MAX; j++)
			{
				if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->submodule[j].module == (zpl_uint32)module)
					return module_lists_tbl[i].tbl->submodule[j].taskid;
			}
		}
	}
	return 0;
}

zpl_uint32 task2module(zpl_uint32 taskid)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->taskid == (zpl_uint32)taskid)
			return module_lists_tbl[i].tbl->module;
		else
		{
			zpl_uint32 j = 0;
			for(j = 0; j < ZPL_SUB_MODULE_MAX; j++)
			{
				if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->submodule[j].taskid == (zpl_uint32)taskid)
					return module_lists_tbl[i].tbl->submodule[j].module;
			}		
		}
	}
	return 0;
}

zpl_uint32 task_module_self(void)
{
	zpl_uint32 i = 0;
	zpl_uint32 taskid = os_task_id_self ();
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->taskid == taskid)
			return module_lists_tbl[i].tbl->module;
		else
		{
			zpl_uint32 j = 0;
			for(j = 0; j < ZPL_SUB_MODULE_MAX; j++)
			{
				if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->submodule[j].taskid == (zpl_uint32)taskid)
					return module_lists_tbl[i].tbl->submodule[j].module;
			}		
		}
	}
	return 0;
}


int module_setup_task(zpl_uint32 module, zpl_uint32 taskid)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->module == (zpl_uint32)module)
		{
			module_lists_tbl[i].tbl->taskid = (zpl_uint32)taskid;
			return 0;
		}
		else
		{
			zpl_uint32 j = 0;
			for(j = 0; j < ZPL_SUB_MODULE_MAX; j++)
			{
				if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->submodule[j].module == (zpl_uint32)module)
				{
					module_lists_tbl[i].tbl->submodule[j].taskid = (zpl_uint32)taskid;
					return 0;
				}
			}
		}
	}
	return 0;
}

int submodule_setup(zpl_uint32 module, zpl_uint32 submodule, char *name, zpl_uint32 taskid)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->module == (zpl_uint32)module)
		{
			if(submodule)
			{
				zpl_uint32 j = 0;
				for(j = 0; j < ZPL_SUB_MODULE_MAX; j++)
				{
					if(module_lists_tbl[i].tbl->submodule[j].module == (zpl_uint32)submodule)
					{
						module_lists_tbl[i].tbl->submodule[j].taskid = (zpl_uint32)taskid;
						if(name)
							module_lists_tbl[i].tbl->submodule[j].name = strdup(name);
						return 0;
					}
				}
			}
			else
			{
				module_lists_tbl[i].tbl->taskid = (zpl_uint32)taskid;
				return 0;
			}
		}
	}
	return 0;
}

zpl_char *zlog_backtrace_module()
{
	static zpl_char backtrace_string[128];
	os_memset(backtrace_string, 0, sizeof(backtrace_string));
	os_snprintf(backtrace_string, sizeof(backtrace_string), "%s [%s]",
			os_task_2_name(os_task_id_self()), module2name(task_module_self()));

	return backtrace_string;
}

zpl_char *zlog_backtrace_funcname()
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
	//os_ansync_lst * gansync = os_ansync_global_lookup(os_task_id_self(), task_module_self());
	os_ansync_lst * gansync = os_ansync_current_get();
	if (gansync && gansync->os_ansync)
		return gansync->os_ansync->entryname;
#endif
	return "NULL";
}

zpl_char *zlog_backtrace_schedfrom()
{
#ifdef OS_THREAD
	struct thread *thread_current = thread_current_get();
	if (thread_current)
		return thread_current->schedfrom;
#endif
#ifdef ELOOP_THREAD
	{
		struct eloop *eloop = eloop_current_get();
		if (eloop)
			return eloop->schedfrom;
	}
#endif
#ifdef OS_ANSYNC_GLOBAL_LIST
	//os_ansync_lst * gansync = os_ansync_global_lookup(os_task_id_self(), task_module_self());
	os_ansync_lst * gansync = os_ansync_current_get();
	if (gansync && gansync->os_ansync)
		return gansync->os_ansync->filename;
#endif
	return "NULL";
}

zpl_uint32 zlog_backtrace_schedfrom_line()
{
#ifdef OS_THREAD
	struct thread *thread_current = thread_current_get();
	if (thread_current)
		return thread_current->schedfrom_line;
#endif
#ifdef ELOOP_THREAD
	{
		struct eloop *eloop = eloop_current_get();
		if (eloop)
			return eloop->schedfrom_line;
	}
#endif
#ifdef OS_ANSYNC_GLOBAL_LIST
	//os_ansync_lst * gansync = os_ansync_global_lookup(os_task_id_self(), task_module_self());
	os_ansync_lst * gansync = os_ansync_current_get();
	if (gansync && gansync->os_ansync)
		return gansync->os_ansync->line;
#endif
	return 0;
}
