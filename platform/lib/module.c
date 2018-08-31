/*
 * module.c
 *
 *  Created on: May 13, 2018
 *      Author: zhurish
 */


#include <zebra.h>
#include "module.h"



typedef struct module_str_s
{
	u_int 		module;
	u_int		taskid;
	const char 	*name;
}module_str_t;

#define MODULE_ENTRY(T) { (MODULE_ ##T), 0, (#T) }

static module_str_t module_string[] =
{
	MODULE_ENTRY(NONE),
	MODULE_ENTRY(DEFAULT),
	MODULE_ENTRY(TIMER),
	MODULE_ENTRY(JOB),
	MODULE_ENTRY(CONSOLE),
	MODULE_ENTRY(TELNET),
	MODULE_ENTRY(NSM),			//route table manage
	MODULE_ENTRY(RIP),
	MODULE_ENTRY(BGP),
	MODULE_ENTRY(OSPF),
	MODULE_ENTRY(RIPNG),
	MODULE_ENTRY(BABEL),
	MODULE_ENTRY(OSPF6),
	MODULE_ENTRY(ISIS),
	MODULE_ENTRY(PIM),
	MODULE_ENTRY(MASC),
	MODULE_ENTRY(NHRP),
	MODULE_ENTRY(HSLS),
	MODULE_ENTRY(OLSR),
	MODULE_ENTRY(VRRP),
	MODULE_ENTRY(FRP),
	MODULE_ENTRY(LLDP),
	MODULE_ENTRY(BFD),
	MODULE_ENTRY(LDP),
	MODULE_ENTRY(SNTP),
	MODULE_ENTRY(IMISH),
	MODULE_ENTRY(UTILS),
	MODULE_ENTRY(MAX),
};


const char * module2name(int module)
{
	int i = 0;
	for(i = 0; i < array_size(module_string); i++)
	{
		if(module_string[i].module == module)
			return module_string[i].name;
	}
	return "Unknow";
}

int name2module(const char *name)
{
	int i = 0;
	for(i = 0; i < array_size(module_string); i++)
	{
		if(os_strcmp(module_string[i].name, name) == 0)
			return module_string[i].module;
	}
	return 0;
}

int module2task(int module)
{
	int i = 0;
	for(i = 0; i < array_size(module_string); i++)
	{
		if(module_string[i].module == module)
			return module_string[i].taskid;
	}
	return 0;
}

int task2module(int taskid)
{
	int i = 0;
	for(i = 0; i < array_size(module_string); i++)
	{
		if(module_string[i].taskid == taskid)
			return module_string[i].module;
	}
	return 0;
}

int task_module_self(void)
{
	int i = 0;
	u_int taskid = os_task_id_self ();
	for(i = 0; i < array_size(module_string); i++)
	{
		if(module_string[i].taskid == taskid)
			return module_string[i].module;
	}
	return 0;
}


int module_setup_task(int module, int taskid)
{
	int i = 0;
	for(i = 0; i < array_size(module_string); i++)
	{
		if(module_string[i].module == module)
		{
			module_string[i].taskid = taskid;
			return 0;
		}
	}
	return 0;
}
