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
#include "vrf.h"
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
	.module_cmd_init=NULL, 
	.taskid=0,
};


static hal_driver_t hal_driver;

static int hal_main_task(void *p)
{
    //int rc = 0;
	struct thread_master *master = (struct thread_master *)p;
	module_setup_task(master->module, os_task_id_self());
	//host_waitting_loadconfig();
	while(thread_mainloop(master))
		;
	return OK;
}



int hal_module_init(void)
{
	memset(&hal_driver, 0, sizeof(hal_driver_t));
	hal_driver.master = thread_master_module_create(MODULE_HAL);
	hal_ipcsrv_init(hal_driver.master, -1, HAL_IPCMSG_CMD_PATH, -1, HAL_IPCMSG_EVENT_PATH);
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


/*
 * CPU Port
 */
int hal_cpu_port_mode(zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	command = IPCCMD_SET(HAL_MODULE_CPU, HAL_MODULE_CMD_REQ, HAL_SWITCH_CPU_MODE);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_cpu_port_enable(zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	command = IPCCMD_SET(HAL_MODULE_CPU, HAL_MODULE_CMD_REQ, HAL_SWITCH_CPU);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_cpu_port_speed(zpl_uint32 value)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, value);
	command = IPCCMD_SET(HAL_MODULE_CPU, HAL_MODULE_CMD_REQ, HAL_SWITCH_CPU_SPEED);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_cpu_port_duplex(zpl_uint32 value)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, value);
	command = IPCCMD_SET(HAL_MODULE_CPU, HAL_MODULE_CMD_REQ, HAL_SWITCH_CPU_DUPLEX);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_cpu_port_flow(zpl_bool rx, zpl_bool tx)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putc(&ipcmsg, rx);
	hal_ipcmsg_putc(&ipcmsg, tx);
	command = IPCCMD_SET(HAL_MODULE_CPU, HAL_MODULE_CMD_REQ, HAL_SWITCH_CPU_FLOW);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

