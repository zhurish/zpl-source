/*
 * module.c
 *
 *  Created on: May 13, 2018
 *      Author: zhurish
 */


#include "os_include.h"
#include "zpl_include.h"
#include "lib_include.h"
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
	.flags=0,
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
	.flags=0,
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
	.flags=0,
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
	.flags=0,
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
	.flags=0,
};



const char * module2name(zpl_uint32 module)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(module_tbl); i++)
	{
		if(module_tbl[i].module == (zpl_uint32)module)
			return module_tbl[i].name;	
	}
	return "Unknow";
}

zpl_uint32 name2module(const char *name)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(module_tbl); i++)
	{
		if(module_tbl[i].name && os_strcmp(module_tbl[i].name, name) == 0)
			return module_tbl[i].module;
		/*else
		{
			zpl_uint32 j = 0;
			for(j = 0; j < ZPL_SUB_MODULE_MAX; j++)
			{
				if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->submodule[j].name && 
					os_strcmp(module_lists_tbl[i].tbl->submodule[j].name, name) == 0)
					return module_lists_tbl[i].tbl->submodule[j].module;
			}
		}
		*/
	}
	return 0;
}

zpl_uint32 module2task(zpl_uint32 module)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(module_tbl); i++)
	{
		if(module_tbl[i].module == (zpl_uint32)module)
			return module_tbl[i].taskid;
		/*else
		{
			zpl_uint32 j = 0;
			for(j = 0; j < ZPL_SUB_MODULE_MAX; j++)
			{
				if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->submodule[j].module == (zpl_uint32)module)
					return module_lists_tbl[i].tbl->submodule[j].taskid;
			}
		}
		*/
	}
	return 0;
}

zpl_uint32 task2module(zpl_uint32 taskid)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(module_tbl); i++)
	{
		if(module_tbl[i].taskid == (zpl_uint32)taskid)
			return module_tbl[i].module;
		/*else
		{
			zpl_uint32 j = 0;
			for(j = 0; j < ZPL_SUB_MODULE_MAX; j++)
			{
				if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->submodule[j].taskid == (zpl_uint32)taskid)
					return module_lists_tbl[i].tbl->submodule[j].module;
			}		
		}*/
	}
	return 0;
}

zpl_uint32 task_module_self(void)
{
	zpl_uint32 i = 0;
	zpl_uint32 taskid = os_task_id_self ();
	for(i = 0; i < array_size(module_tbl); i++)
	{
		if(module_tbl[i].taskid == taskid)
			return module_tbl[i].module;
		/*else
		{
			zpl_uint32 j = 0;
			for(j = 0; j < ZPL_SUB_MODULE_MAX; j++)
			{
				if(module_lists_tbl[i].tbl && module_lists_tbl[i].tbl->submodule[j].taskid == (zpl_uint32)taskid)
					return module_lists_tbl[i].tbl->submodule[j].module;
			}		
		}*/
	}
	return 0;
}


int module_setup_task(zpl_uint32 module, zpl_uint32 taskid)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(module_tbl); i++)
	{
		if(module_tbl[i].module == (zpl_uint32)module)
		{
			module_tbl[i].taskid = (zpl_uint32)taskid;
			return 0;
		}
		/*else
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
		}*/
	}
	return 0;
}

