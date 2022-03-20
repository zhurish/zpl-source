#include "zpl_include.h"
#include "lib_include.h"
 
extern struct module_list module_list_default;
extern struct module_list module_list_lib;
extern struct module_list module_list_osal;
extern struct module_list module_list_timer;
extern struct module_list module_list_job;
extern struct module_list module_list_console;
extern struct module_list module_list_telnet;
extern struct module_list module_list_nsm;
extern struct module_list module_list_hal;
extern struct module_list module_list_pal;
extern struct module_list module_list_bsp;
extern struct module_list module_list_sdk;
 
struct module_alllist module_lists_tbl[MODULE_MAX] = {
  &module_list_default,
  &module_list_lib,
  &module_list_osal,
  &module_list_timer,
  &module_list_job,
  &module_list_console,
  &module_list_telnet,
  &module_list_nsm,
  &module_list_hal,
  &module_list_pal,
  &module_list_bsp,
  &module_list_sdk,
 NULL,
};
 
 


int zplib_module_name_show()
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->name != NULL)
		{
			liblog_trap( "Module : %s flags=0x%08x", module_lists_tbl[i].tbl->name, module_lists_tbl[i].tbl->flags);	
		}	
	}
	return OK;
}

int zplib_module_name_init(const char * name)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if( module_lists_tbl[i].tbl && 
			module_lists_tbl[i].tbl->module_init &&
			!CHECK_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_IS_INIT)) 
		{
			if(name && os_strcmp(module_lists_tbl[i].tbl->name, name) == 0)
			{
				liblog_trap( "Module : %s Init", module_lists_tbl[i].tbl->name);	
				SET_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_IS_INIT);
				return (module_lists_tbl[i].tbl->module_init)();
			}			
		}	
	}
	return ERROR;
}

int zplib_module_init(zpl_uint32 module)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if( module_lists_tbl[i].tbl && 
			module_lists_tbl[i].tbl->module_init &&
			!CHECK_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_IS_INIT)) 
		{
			if(module && module_lists_tbl[i].tbl->module  == module)
			{
				liblog_trap( "Module : %s Init", module_lists_tbl[i].tbl->name);
				SET_FLAG(module_lists_tbl[i].tbl->flags,ZPL_MODULE_IS_INIT);
				return (module_lists_tbl[i].tbl->module_init)();
			}				
		}	
	}
	return ERROR;
}

int zplib_module_exit(zpl_uint32 module)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if( module_lists_tbl[i].tbl && 
			module_lists_tbl[i].tbl->module_exit && 
			CHECK_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_IS_INIT) ) 
		{
			if(module && module_lists_tbl[i].tbl->module  == module)
			{
				liblog_trap( "Module : %s Exit", module_lists_tbl[i].tbl->name);
				UNSET_FLAG(module_lists_tbl[i].tbl->flags,ZPL_MODULE_IS_INIT);
				return (module_lists_tbl[i].tbl->module_exit)();
			}			
		}	
	}
	return ERROR;
}

int zplib_module_task_name_init(const char * name)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && 
			module_lists_tbl[i].tbl->module_task_init &&
			module_lists_tbl[i].tbl->taskid <= 0 &&
			CHECK_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_IS_INIT) &&
			!CHECK_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_INIT_TASK))
		{
			if(name && os_strcmp(module_lists_tbl[i].tbl->name, name) == 0)
			{
				liblog_trap( "Module : %s Create Task", module_lists_tbl[i].tbl->name);
				SET_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_INIT_TASK);
				return (module_lists_tbl[i].tbl->module_task_init)();
			}
		}
	}
	return ERROR;
}

int zplib_module_task_init(zpl_uint32 module)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && 
			module_lists_tbl[i].tbl->module_task_init &&
			module_lists_tbl[i].tbl->taskid <= 0 &&
			CHECK_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_IS_INIT) &&
			!CHECK_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_INIT_TASK))
		{
			if(module && module_lists_tbl[i].tbl->module  == module)
			{
				liblog_trap( "Module : %s Create Task", module_lists_tbl[i].tbl->name);
				SET_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_INIT_TASK);
				return (module_lists_tbl[i].tbl->module_task_init)();
			}
		}	
	}
	return ERROR;
}

int zplib_module_task_exit(zpl_uint32 module)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && 
			module_lists_tbl[i].tbl->module_task_exit &&
			module_lists_tbl[i].tbl->taskid > 0 &&
			CHECK_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_INIT_TASK))
		{
			if(module && module_lists_tbl[i].tbl->module  == module)
			{
				liblog_trap( "Module : %s Destroy Task", module_lists_tbl[i].tbl->name);
				UNSET_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_INIT_TASK);
				return (module_lists_tbl[i].tbl->module_task_exit)();
			}
		}
	}
	return ERROR;
}

int zplib_module_cmd_name_init(const char * name)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && 
			module_lists_tbl[i].tbl->module_cmd_init &&
			CHECK_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_IS_INIT) &&
			!CHECK_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_INIT_CMD))
		{
			if(name && os_strcmp(module_lists_tbl[i].tbl->name, name) == 0)
			{
				liblog_trap( "Module : %s CLI Init", module_lists_tbl[i].tbl->name);
				SET_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_INIT_CMD);
				return (module_lists_tbl[i].tbl->module_cmd_init)();
			}
		}		
	}
	return ERROR;
}

int zplib_module_cmd_init(zpl_uint32 module)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && 
			module_lists_tbl[i].tbl->module_cmd_init &&
			CHECK_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_IS_INIT) &&
			!CHECK_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_INIT_CMD))
		{
			if(module && module_lists_tbl[i].tbl->module  == module)
			{
				liblog_trap( "Module : %s CLI Init", module_lists_tbl[i].tbl->name);
				SET_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_INIT_CMD);
				return (module_lists_tbl[i].tbl->module_cmd_init)();
			}
		}	
	}
	return ERROR;
}

static int _zplib_module_alltable_init(zpl_uint32 type)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl)
		{
			switch(type)
			{
				case 1:
				{
					if(module_lists_tbl[i].tbl->module_init && 
						CHECK_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_NEED_INIT) &&
						!CHECK_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_IS_INIT))
					{
						liblog_trap( "Module : %s Init", module_lists_tbl[i].tbl->name);
						SET_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_IS_INIT);
						(module_lists_tbl[i].tbl->module_init)();
					}
				}
				break;
				case 2:
				{
					if(module_lists_tbl[i].tbl->module_exit && 
						CHECK_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_NEED_INIT) &&
						CHECK_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_IS_INIT))
					{
						liblog_trap( "Module : %s Exit", module_lists_tbl[i].tbl->name);
						UNSET_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_IS_INIT);
						(module_lists_tbl[i].tbl->module_exit)();
					}
				}
				break;
				case 3:
				{
					if(module_lists_tbl[i].tbl->module_task_init && 
						CHECK_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_NEED_INIT) &&
						CHECK_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_IS_INIT) &&
						!CHECK_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_INIT_TASK))
					{
						liblog_trap( "Module : %s Create Task", module_lists_tbl[i].tbl->name);
						SET_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_INIT_TASK);
						(module_lists_tbl[i].tbl->module_task_init)();
					}
				}
				break;
				case 4:
				{
					if(module_lists_tbl[i].tbl->module_task_exit && 
						CHECK_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_NEED_INIT) &&
						CHECK_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_IS_INIT) &&
						CHECK_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_INIT_TASK))
					{
						liblog_trap( "Module : %s Destroy Task", module_lists_tbl[i].tbl->name);
						UNSET_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_INIT_TASK);
						(module_lists_tbl[i].tbl->module_task_exit)();
					}
				}
				break;
				case 5:
				{
					if(module_lists_tbl[i].tbl->module_cmd_init && 
						CHECK_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_NEED_INIT) &&
						CHECK_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_IS_INIT) &&
						!CHECK_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_INIT_CMD))
					{
						liblog_trap( "Module : %s CLI Init", module_lists_tbl[i].tbl->name);
						SET_FLAG(module_lists_tbl[i].tbl->flags, ZPL_MODULE_INIT_CMD);
						(module_lists_tbl[i].tbl->module_cmd_init)();
					}
				}
				break;
			}
		} 
	}
	return OK;
}

int zplib_module_initall(void)
{
	return _zplib_module_alltable_init(1);
}

int zplib_module_exitall(void)
{
	return _zplib_module_alltable_init(2);
}

int zplib_module_task_startall(void)
{
	return _zplib_module_alltable_init(3);
}

int zplib_module_task_stopall(void)
{
	return _zplib_module_alltable_init(4);
}

int zplib_module_cmd_all(void)
{
	return _zplib_module_alltable_init(5);
}

struct module_list *zplib_module_info(zpl_uint32 module)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(module_lists_tbl); i++)
	{
		if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->module == (zpl_uint32)module)
			return module_lists_tbl[i].tbl;
		else
		{
			zpl_uint32 j = 0;
			for(j = 0; j < ZPL_SUB_MODULE_MAX; j++)
			{
				if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->submodule[j].module == (zpl_uint32)module)
					return &module_lists_tbl[i].tbl->submodule[j];
			}
		}	
	}
	return NULL;
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

 
