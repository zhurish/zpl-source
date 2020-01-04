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
	MODULE_ENTRY(SSH),
	MODULE_ENTRY(NSM),			//route table manage
	MODULE_ENTRY(MODEM),
	MODULE_ENTRY(WIFI),
	MODULE_ENTRY(DHCP),
	MODULE_ENTRY(DHCPD),
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
	MODULE_ENTRY(KERNEL),
	MODULE_ENTRY(VOIP),
	MODULE_ENTRY(APP_START),
	MODULE_ENTRY(APP_STOP),
	MODULE_ENTRY(MAX),
};


const char * module2name(int module)
{
	u_int i = 0;
	for(i = 0; i < array_size(module_string); i++)
	{
		if(module_string[i].module == (u_int)module)
			return module_string[i].name;
	}
	return "Unknow";
}

int name2module(const char *name)
{
	u_int i = 0;
	for(i = 0; i < array_size(module_string); i++)
	{
		if(os_strcmp(module_string[i].name, name) == 0)
			return module_string[i].module;
	}
	return 0;
}

int module2task(int module)
{
	u_int i = 0;
	for(i = 0; i < array_size(module_string); i++)
	{
		if(module_string[i].module == (u_int)module)
			return module_string[i].taskid;
	}
	return 0;
}

int task2module(int taskid)
{
	u_int i = 0;
	for(i = 0; i < array_size(module_string); i++)
	{
		if(module_string[i].taskid == (u_int)taskid)
			return module_string[i].module;
	}
	return 0;
}

int task_module_self(void)
{
	u_int i = 0;
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
	u_int i = 0;
	for(i = 0; i < array_size(module_string); i++)
	{
		if(module_string[i].module == (u_int)module)
		{
			module_string[i].taskid = (u_int)taskid;
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
