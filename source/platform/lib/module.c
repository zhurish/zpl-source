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
static int module_init_seq = 1;

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

zpl_taskid_t module2task(zpl_uint32 module)
{
	zpl_uint32 i = 0;
	for(i = 0; i < MODULE_MAX; i++)
	{
		if(_module_lsttable[i].tbl && _module_lsttable[i].tbl->module == (zpl_uint32)module)
			return _module_lsttable[i].tbl->taskid;
	}
	return 0;
}

zpl_uint32 task2module(zpl_taskid_t taskid)
{
	zpl_uint32 i = 0;
	for(i = 0; i < MODULE_MAX; i++)
	{
		if(_module_lsttable[i].tbl && _module_lsttable[i].tbl->taskid == (zpl_taskid_t)taskid)
			return _module_lsttable[i].tbl->module;
	}
	return 0;
}

zpl_taskid_t task_module_self(void)
{
	zpl_uint32 i = 0;
	zpl_taskid_t taskid = os_task_id_self ();
	for(i = 0; i < MODULE_MAX; i++)
	{
		if(_module_lsttable[i].tbl && _module_lsttable[i].tbl->taskid == taskid)
			return _module_lsttable[i].tbl->module;
	}
	return 0;
}


int module_setup_task(zpl_uint32 module, zpl_taskid_t taskid)
{
	zpl_uint32 i = 0;
	for(i = 0; i < MODULE_MAX; i++)
	{
		if(_module_lsttable[i].tbl && _module_lsttable[i].tbl->module == (zpl_uint32)module)
		{
			_module_lsttable[i].tbl->taskid = (zpl_taskid_t)taskid;
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
	int ret = OK;
	zpl_uint32 i = 0;
	for(i = 0; i < MODULE_MAX; i++)
	{
		if( _module_lsttable[i].tbl && 
			_module_lsttable[i].tbl->module_init &&
			!CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_IS_INIT)) 
		{
			if(module)
			{
				if(_module_lsttable[i].tbl->module  == module)
				{
					if(_module_lsttable[i].tbl->name != NULL)
						liblog_trap( "Module : %s Init", _module_lsttable[i].tbl->name);
					SET_FLAG(_module_lsttable[i].tbl->flags,ZPL_MODULE_IS_INIT);
					_module_lsttable[i].tbl->index_num = module_init_seq++;
					return (_module_lsttable[i].tbl->module_init)();
				}
			}	
			else
			{
				if(CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_NEED_INIT))
					continue;
				if (_module_lsttable[i].tbl->name != NULL)
					liblog_trap( "Module : %s Init", _module_lsttable[i].tbl->name);
				SET_FLAG(_module_lsttable[i].tbl->flags,ZPL_MODULE_IS_INIT);
				_module_lsttable[i].tbl->index_num = module_init_seq++;
				ret = (_module_lsttable[i].tbl->module_init)();
				if(ret != OK)
					return ERROR;
			}
		}	
	}
	return ret;
}


static int zplib_module_init_seq(int seq)
{
	zpl_uint32 i = 0;
	for(i = 0; i < MODULE_MAX; i++)
	{
		if( _module_lsttable[i].tbl && 
			_module_lsttable[i].tbl->index_num == seq && 
			CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_IS_INIT) ) 
		{
			return i;
		}
	}
	return 0;
}

int zplib_module_exit(zpl_uint32 module)
{
	zpl_uint32 i = 0;
	
	if(module == 0)
	{
		while(module_init_seq)
		{
			i = zplib_module_init_seq(module_init_seq);
			if(i)
			{
				if(!CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_NEED_INIT))
				{
					if((_module_lsttable[i].tbl->module_task_exit) && CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_IS_TASK))
					{
						(_module_lsttable[i].tbl->module_task_exit)();
						UNSET_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_IS_TASK);
					}
					if((_module_lsttable[i].tbl->module_exit) && CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_IS_INIT))
					{
						(_module_lsttable[i].tbl->module_exit)();
						UNSET_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_IS_INIT);
					}
				}
			}
			module_init_seq--;
		}
		return OK;
	}
	for(i = 0; i < MODULE_MAX; i++)
	{
		if( _module_lsttable[i].tbl && 
			_module_lsttable[i].tbl->module_exit && 
			CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_IS_INIT) ) 
		{
			if(module)
			{
				if(_module_lsttable[i].tbl->module  == module)
				{
					if(_module_lsttable[i].tbl->name != NULL)
						liblog_trap( "Module : %s Exit", _module_lsttable[i].tbl->name);
					UNSET_FLAG(_module_lsttable[i].tbl->flags,ZPL_MODULE_IS_INIT);
					return (_module_lsttable[i].tbl->module_exit)();
				}
			}	
			else
			{
				if(_module_lsttable[i].tbl->name != NULL)
					liblog_trap( "Module : %s Exit", _module_lsttable[i].tbl->name);
				UNSET_FLAG(_module_lsttable[i].tbl->flags,ZPL_MODULE_IS_INIT);
				if( (_module_lsttable[i].tbl->module_exit)() != OK)
					return ERROR;
			}	
		}	
	}
	return OK;
}


int zplib_module_task_init(zpl_uint32 module)
{
	zpl_uint32 i = 0;
	for(i = 0; i < MODULE_MAX; i++)
	{
		if(_module_lsttable[i].tbl && 
			_module_lsttable[i].tbl->module_task_init &&
			CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_IS_INIT) &&
			!CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_IS_TASK))
		{
			if(module)
			{
				if(_module_lsttable[i].tbl->module  == module)
				{
					if(_module_lsttable[i].tbl->name != NULL)
						liblog_trap( "Module : %s Create Task", _module_lsttable[i].tbl->name);
					SET_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_IS_TASK);
					return (_module_lsttable[i].tbl->module_task_init)();
				}
			}
			else
			{
				if(CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_NEED_INIT))
					continue;
				if(_module_lsttable[i].tbl->name != NULL)
					liblog_trap( "Module : %s Create Task", _module_lsttable[i].tbl->name);
				SET_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_IS_TASK);
				if( (_module_lsttable[i].tbl->module_task_init)() != OK)
					return ERROR;
			}
		}	
	}
	return OK;
}

int zplib_module_task_exit(zpl_uint32 module)
{
	zpl_uint32 i = 0;
	for(i = 0; i < MODULE_MAX; i++)
	{
		if(_module_lsttable[i].tbl && 
			_module_lsttable[i].tbl->module_task_exit &&
			_module_lsttable[i].tbl->taskid > 0 &&
			CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_IS_TASK))
		{
			if(module)
			{
				if(_module_lsttable[i].tbl->module  == module)
				{
					if(_module_lsttable[i].tbl->name != NULL)
						liblog_trap( "Module : %s Destroy Task", _module_lsttable[i].tbl->name);
					UNSET_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_IS_TASK);
					return (_module_lsttable[i].tbl->module_task_exit)();
				}
			}
			else
			{
				if(CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_NEED_INIT))
					continue;
				if(_module_lsttable[i].tbl->name != NULL)
					liblog_trap( "Module : %s Destroy Task", _module_lsttable[i].tbl->name);
				UNSET_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_IS_TASK);
				if( (_module_lsttable[i].tbl->module_task_exit)() != OK)
					return ERROR;
			}
		}
	}
	return OK;
}


int zplib_module_cmd_init(zpl_uint32 module)
{
	zpl_uint32 i = 0;
	for(i = 0; i < MODULE_MAX; i++)
	{
		if(_module_lsttable[i].tbl && 
			_module_lsttable[i].tbl->module_cmd_init &&
			!CHECK_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_IS_CMD))
		{
			if(module)
			{
				if(_module_lsttable[i].tbl->module  == module)
				{
					if(_module_lsttable[i].tbl->name != NULL)
						liblog_trap( "Module : %s CLI Init", _module_lsttable[i].tbl->name);
					SET_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_IS_CMD);
					return (_module_lsttable[i].tbl->module_cmd_init)();
				}
			}
			else
			{
				if(_module_lsttable[i].tbl->name != NULL)
					liblog_trap( "Module : %s CLI Init", _module_lsttable[i].tbl->name);
				SET_FLAG(_module_lsttable[i].tbl->flags, ZPL_MODULE_IS_CMD);
				if( (_module_lsttable[i].tbl->module_cmd_init)() != OK)
					return ERROR;
			}
		}	
	}
	return OK;
}

int zplib_module_initall(void)
{
	return zplib_module_init(0);
}

int zplib_module_exitall(void)
{
	return zplib_module_exit(0);
}

int zplib_module_task_startall(void)
{
	return zplib_module_task_init(0);
}

int zplib_module_task_stopall(void)
{
	return zplib_module_task_exit(0);
}

int zplib_module_cmd_all(void)
{
	return zplib_module_cmd_init(0);
}



int submodule_setup(zpl_uint32 module, zpl_uint32 submodule, char *name, zpl_taskid_t taskid)
{
	zpl_uint32 i = 0;
	for(i = 0; i < MODULE_MAX; i++)
	{
		if(_module_lsttable[i].tbl && _module_lsttable[i].tbl->module == (zpl_uint32)module)
		{
			/*if(submodule)
			{
				zpl_uint32 j = 0;
				for(j = 0; j < ZPL_SUB_MODULE_MAX; j++)
				{
					if(_module_lsttable[i].tbl->submodule[j].module == (zpl_uint32)submodule)
					{
						_module_lsttable[i].tbl->submodule[j].taskid = (zpl_taskid_t)taskid;
						if(name)
							_module_lsttable[i].tbl->submodule[j].name = strdup(name);
						return 0;
					}
				}
			}
			else*/
			{
				_module_lsttable[i].tbl->taskid = (zpl_taskid_t)taskid;
				return 0;
			}
		}
	}
	return 0;
}
