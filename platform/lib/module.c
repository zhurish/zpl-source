/*
 * module.c
 *
 *  Created on: May 13, 2018
 *      Author: zhurish
 */


#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "log.h"



struct module_list module_list_default = 
{ 
	.module=MODULE_DEFAULT, 
	.name="DEFAULT\0", 
	.module_init=NULL, 
	.module_exit=NULL, 
	.module_task_init=NULL, 
	.module_task_exit=NULL, 
	.module_cmd_init=NULL, 
	.taskid=0,
	.flags=0,
};

struct module_list module_list_lib = 
{ 
	.module=MODULE_LIB, 
	.name="LIB\0", 
	.module_init=NULL, 
	.module_exit=NULL, 
	.module_task_init=NULL, 
	.module_task_exit=NULL, 
	.module_cmd_init=NULL, 
	.taskid=0,
	.flags=0,
};

struct module_list module_list_osal = 
{ 
	.module=MODULE_OSAL, 
	.name="OSAL\0", 
	.module_init=NULL, 
	.module_exit=NULL, 
	.module_task_init=NULL, 
	.module_task_exit=NULL, 
	.module_cmd_init=NULL, 
	.taskid=0,
	.flags=0,
};

struct module_list module_list_timer = 
{ 
	.module=MODULE_TIMER, 
	.name="TIMER\0", 
	.module_init=NULL, 
	.module_exit=NULL, 
	.module_task_init=NULL, 
	.module_task_exit=NULL, 
	.module_cmd_init=NULL, 
	.taskid=0,
	.flags=0,
};
struct module_list module_list_job = 
{ 
	.module=MODULE_JOB, 
	.name="JOB\0", 
	.module_init=NULL, 
	.module_exit=NULL, 
	.module_task_init=NULL, 
	.module_task_exit=NULL, 
	.module_cmd_init=NULL, 
	.taskid=0,
	.flags=0,
};



static struct module_alllist *_module_lsttable = NULL;

int zplib_module_install(struct module_alllist *_m_table)
{
	_module_lsttable = _m_table;
	return OK;
}

const char * module2name(zpl_uint32 module)
{
	zpl_uint32 i = 0;
	for(i = 0; i < MODULE_MAX; i++)
	{
		if(_module_lsttable[i].tbl && _module_lsttable[i].tbl->module == (zpl_uint32)module)
			return _module_lsttable[i].tbl->name;	
	}
	return "Unknow";
}

zpl_uint32 name2module(const char *name)
{
	zpl_uint32 i = 0;
	for(i = 0; i < MODULE_MAX; i++)
	{
		if(_module_lsttable[i].tbl && _module_lsttable[i].tbl->name && os_strcmp(_module_lsttable[i].tbl->name, name) == 0)
			return _module_lsttable[i].tbl->module;
	}
	return 0;
}

zpl_uint32 module2task(zpl_uint32 module)
{
	zpl_uint32 i = 0;
	for(i = 0; i < MODULE_MAX; i++)
	{
		if(_module_lsttable[i].tbl && _module_lsttable[i].tbl->module == (zpl_uint32)module)
			return _module_lsttable[i].tbl->taskid;
	}
	return 0;
}

zpl_uint32 task2module(zpl_uint32 taskid)
{
	zpl_uint32 i = 0;
	for(i = 0; i < MODULE_MAX; i++)
	{
		if(_module_lsttable[i].tbl && _module_lsttable[i].tbl->taskid == (zpl_uint32)taskid)
			return _module_lsttable[i].tbl->module;
	}
	return 0;
}

zpl_uint32 task_module_self(void)
{
	zpl_uint32 i = 0;
	zpl_uint32 taskid = os_task_id_self ();
	for(i = 0; i < MODULE_MAX; i++)
	{
		if(_module_lsttable[i].tbl && _module_lsttable[i].tbl->taskid == taskid)
			return _module_lsttable[i].tbl->module;
	}
	return 0;
}


int module_setup_task(zpl_uint32 module, zpl_uint32 taskid)
{
	zpl_uint32 i = 0;
	for(i = 0; i < MODULE_MAX; i++)
	{
		if(_module_lsttable[i].tbl && _module_lsttable[i].tbl->module == (zpl_uint32)module)
		{
			_module_lsttable[i].tbl->taskid = (zpl_uint32)taskid;
			return 0;
		}
	}
	return 0;
}




int zplib_module_name_show()
{
	zpl_uint32 i = 0;
	for(i = 0; i < MODULE_MAX; i++)
	{
		if(_module_lsttable[i].tbl && _module_lsttable[i].tbl->name != NULL)
		{
			liblog_trap( "Module : %s flags=0x%08x", _module_lsttable[i].tbl->name, _module_lsttable[i].tbl->flags);	
		}	
	}
	return OK;
}


int zplib_module_init(zpl_uint32 module)
{
	zpl_uint32 i = 0;
	for(i = 0; i < MODULE_MAX; i++)
	{
		if( _module_lsttable[i].tbl && 
			_module_lsttable[i].tbl->module_init &&
			!CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_IS_INIT)) 
		{
			if(module && _module_lsttable[i].tbl->module  == module)
			{
				if(_module_lsttable[i].tbl->name != NULL)
					liblog_trap( "Module : %s Init", _module_lsttable[i].tbl->name);
				SET_FLAG(_module_lsttable[i].tbl->flags,ZPL_MODULE_IS_INIT);
				return (_module_lsttable[i].tbl->module_init)();
			}				
		}	
	}
	return ERROR;
}

int zplib_module_exit(zpl_uint32 module)
{
	zpl_uint32 i = 0;
	for(i = 0; i < MODULE_MAX; i++)
	{
		if( _module_lsttable[i].tbl && 
			_module_lsttable[i].tbl->module_exit && 
			CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_IS_INIT) ) 
		{
			if(module && _module_lsttable[i].tbl->module  == module)
			{
				if(_module_lsttable[i].tbl->name != NULL)
					liblog_trap( "Module : %s Exit", _module_lsttable[i].tbl->name);
				UNSET_FLAG(_module_lsttable[i].tbl->flags,ZPL_MODULE_IS_INIT);
				return (_module_lsttable[i].tbl->module_exit)();
			}			
		}	
	}
	return ERROR;
}


int zplib_module_task_init(zpl_uint32 module)
{
	zpl_uint32 i = 0;
	for(i = 0; i < MODULE_MAX; i++)
	{
		if(_module_lsttable[i].tbl && 
			_module_lsttable[i].tbl->module_task_init &&
			_module_lsttable[i].tbl->taskid <= 0 &&
			CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_IS_INIT) &&
			!CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_INIT_TASK))
		{
			if(module && _module_lsttable[i].tbl->module  == module)
			{
				if(_module_lsttable[i].tbl->name != NULL)
					liblog_trap( "Module : %s Create Task", _module_lsttable[i].tbl->name);
				SET_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_INIT_TASK);
				return (_module_lsttable[i].tbl->module_task_init)();
			}
		}	
	}
	return ERROR;
}

int zplib_module_task_exit(zpl_uint32 module)
{
	zpl_uint32 i = 0;
	for(i = 0; i < MODULE_MAX; i++)
	{
		if(_module_lsttable[i].tbl && 
			_module_lsttable[i].tbl->module_task_exit &&
			_module_lsttable[i].tbl->taskid > 0 &&
			CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_INIT_TASK))
		{
			if(module && _module_lsttable[i].tbl->module  == module)
			{
				if(_module_lsttable[i].tbl->name != NULL)
					liblog_trap( "Module : %s Destroy Task", _module_lsttable[i].tbl->name);
				UNSET_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_INIT_TASK);
				return (_module_lsttable[i].tbl->module_task_exit)();
			}
		}
	}
	return ERROR;
}


int zplib_module_cmd_init(zpl_uint32 module)
{
	zpl_uint32 i = 0;
	for(i = 0; i < MODULE_MAX; i++)
	{
		if(_module_lsttable[i].tbl && 
			_module_lsttable[i].tbl->module_cmd_init &&
			CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_IS_INIT) &&
			!CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_INIT_CMD))
		{
			if(module && _module_lsttable[i].tbl->module  == module)
			{
				if(_module_lsttable[i].tbl->name != NULL)
					liblog_trap( "Module : %s CLI Init", _module_lsttable[i].tbl->name);
				SET_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_INIT_CMD);
				return (_module_lsttable[i].tbl->module_cmd_init)();
			}
		}	
	}
	return ERROR;
}

static int _zplib_module_alltable_init(zpl_uint32 type)
{
	zpl_uint32 i = 0;
	for(i = 0; i < MODULE_MAX; i++)
	{
		if(_module_lsttable[i].tbl)
		{
			switch(type)
			{
				case 1:
				{
					if(_module_lsttable[i].tbl->module_init && 
						CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_NEED_INIT) &&
						!CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_IS_INIT))
					{
						if(_module_lsttable[i].tbl->name != NULL)
							liblog_trap( "Module : %s Init", _module_lsttable[i].tbl->name);
						(_module_lsttable[i].tbl->module_init)();
						SET_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_IS_INIT);
					}
				}
				break;
				case 2:
				{
					if(_module_lsttable[i].tbl->module_exit && 
						CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_NEED_INIT) &&
						CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_IS_INIT))
					{
						if(_module_lsttable[i].tbl->name != NULL)
							liblog_trap( "Module : %s Exit", _module_lsttable[i].tbl->name);
						(_module_lsttable[i].tbl->module_exit)();
						UNSET_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_IS_INIT);
					}
				}
				break;
				case 3:
				{
					if(_module_lsttable[i].tbl->module_task_init && 
						CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_NEED_INIT) &&
						CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_IS_INIT) &&
						!CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_INIT_TASK))
					{
						if(_module_lsttable[i].tbl->name != NULL)
							liblog_trap( "Module : %s Create Task", _module_lsttable[i].tbl->name);
						(_module_lsttable[i].tbl->module_task_init)();
						SET_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_INIT_TASK);
					}
				}
				break;
				case 4:
				{
					if(_module_lsttable[i].tbl->module_task_exit && 
						CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_NEED_INIT) &&
						CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_IS_INIT) &&
						CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_INIT_TASK))
					{
						if(_module_lsttable[i].tbl->name != NULL)
							liblog_trap( "Module : %s Destroy Task", _module_lsttable[i].tbl->name);
						(_module_lsttable[i].tbl->module_task_exit)();
						UNSET_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_INIT_TASK);
					}
				}
				break;
				case 5:
				{
					if(_module_lsttable[i].tbl->module_cmd_init && 
						CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_NEED_INIT) &&
						CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_IS_INIT) &&
						!CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_INIT_CMD))
					{
						if(_module_lsttable[i].tbl->name != NULL)
							liblog_trap( "Module : %s CLI Init", _module_lsttable[i].tbl->name);
						(_module_lsttable[i].tbl->module_cmd_init)();
						SET_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_INIT_CMD);
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



int submodule_setup(zpl_uint32 module, zpl_uint32 submodule, char *name, zpl_uint32 taskid)
{
	zpl_uint32 i = 0;
	for(i = 0; i < MODULE_MAX; i++)
	{
		if(_module_lsttable[i].tbl && _module_lsttable[i].tbl->module == (zpl_uint32)module)
		{
			if(submodule)
			{
				zpl_uint32 j = 0;
				for(j = 0; j < ZPL_SUB_MODULE_MAX; j++)
				{
					if(_module_lsttable[i].tbl->submodule[j].module == (zpl_uint32)submodule)
					{
						_module_lsttable[i].tbl->submodule[j].taskid = (zpl_uint32)taskid;
						if(name)
							_module_lsttable[i].tbl->submodule[j].name = strdup(name);
						return 0;
					}
				}
			}
			else
			{
				_module_lsttable[i].tbl->taskid = (zpl_uint32)taskid;
				return 0;
			}
		}
	}
	return 0;
}
