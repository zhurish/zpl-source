/*
 * hal_driver.c
 *
 *  Created on: 2019年9月8日
 *      Author: zhurish
 */
#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"
#include "if.h"

#include "vty.h"
#include "nsm_include.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"
#include "hal_ipcsrv.h"
#include "hal_driver.h"


struct module_list module_list_hal = 
{ 
	.module=MODULE_HAL, 
	.name="HAL\0", 
	.module_init=hal_module_init, 
	.module_exit=hal_module_exit, 
	.module_task_init=hal_module_task_init, 
	.module_task_exit=hal_module_task_exit, 
	.module_cmd_init=hal_module_cmd_init, 
	.taskid=0,
	.flags=ZPL_MODULE_NEED_INIT,
};


static hal_driver_t hal_driver;

static int hal_main_task(void *p)
{

	struct thread_master *master = (struct thread_master *)p;
	module_setup_task(master->module, os_task_id_self());

	while(thread_mainloop(master))
		;
	return OK;
}



int hal_module_init(void)
{
	memset(&hal_driver, 0, sizeof(hal_driver_t));
	hal_driver.master = thread_master_module_create(MODULE_HAL);
	hal_ipcsrv_init(hal_driver.master, os_netservice_port_get("hal_port"), os_netservice_sockpath_get(HAL_IPCMSG_CMD_PATH)/*HAL_IPCMSG_CMD_PATH*/);
	return OK;
}

int hal_module_exit(void)
{
	hal_ipcsrv_exit();
	if(hal_driver.master)
	{	
		thread_master_free(hal_driver.master);	
		hal_driver.master = NULL;
	}
	return OK;
}

int hal_module_task_init(void)
{
	if(!hal_driver.master)
	{	
		hal_driver.master = thread_master_module_create(MODULE_HAL);
	}
	if(hal_driver.taskid <= 0)
		hal_driver.taskid = os_task_create("halTask", OS_TASK_DEFAULT_PRIORITY,
	               0, hal_main_task, hal_driver.master, OS_TASK_DEFAULT_STACK*4);
	if(hal_driver.taskid > 0)
	{
		module_setup_task(MODULE_HAL, hal_driver.taskid);
		return OK;
	}
	return ERROR;
}

int hal_module_task_exit(void)
{
	if(hal_driver.taskid > 0)
		os_task_destroy(hal_driver.taskid);
	if(hal_driver.master)
	{	
		thread_master_free(hal_driver.master);	
		hal_driver.master = NULL;
	}
	return OK;
}

int hal_module_cmd_init(void)
{
	hal_ipcsrv_cli();
	return OK;
}



