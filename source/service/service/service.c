/*
 * service.c
 *
 *  Created on: Oct 28, 2018
 *      Author: zhurish
 */

#include "service.h"
#ifdef ZPL_SERVICE_SNTPC
#include "sntpcLib.h"
#endif
#ifdef ZPL_SERVICE_SNTPS
#include "sntpsLib.h"
#endif
#ifdef ZPL_SERVICE_TFTPC
#include "tftpLib.h"
#endif
#ifdef ZPL_SERVICE_TFTPD
#include "tftpdLib.h"
#endif
#ifdef ZPL_SERVICE_FTPC
#include "ftpLib.h"
#endif
#ifdef ZPL_SERVICE_FTPD
#include "ftpdLib.h"
#endif
#ifdef ZPL_SERVICE_TELNET
#include "telnetLib.h"
#endif
#ifdef ZPL_SERVICE_TELNETD
#include "telnetLib.h"
#endif
#ifdef ZPL_SERVICE_PING
#include "pingLib.h"
#endif
#ifdef ZPL_SERVICE_TRACEROUTE
#include "tracerouteLib.h"
#endif
#ifdef ZPL_SERVICE_UBUS_SYNC
#include "ubus_sync.h"
#endif



static zpl_taskid_t sys_task_id = 0;
static void *master_eloop = NULL;

struct module_list module_list_utils = 
{ 
	.module=MODULE_SERVICE, 
	.name="SERVICE\0", 
	.module_init=service_module_init, 
	.module_exit=service_module_exit, 
	.module_task_init=service_task_init, 
	.module_task_exit=service_task_exit, 
	.module_cmd_init=service_clicmd_init, 
	.taskid=0,
	.flags = 0,
};


#ifdef ZPL_SERVICE_FTPD
const char *ftpd_hostname(void)
{
	return "VxWorks 5.5";
}

static int ftpd_loginVerify(char *user, char *pass)
{
	return user_authentication(user, pass);
}
#endif

static int service_main_task(void *argv)
{
	module_list_utils.taskid = os_task_id_self();
	//module_setup_task(MODULE_SERVICE, os_task_id_self());
	host_waitting_loadconfig();
	eloop_mainloop(master_eloop);
	return 0;
}

int service_task_init (void)
{
	if(master_eloop == NULL)
		master_eloop = eloop_master_name_create("SERVICE");
	if(sys_task_id == 0)
		sys_task_id = os_task_create("sysTask", OS_TASK_DEFAULT_PRIORITY,
	               0, service_main_task, NULL, OS_TASK_DEFAULT_STACK);
	if(sys_task_id)
	{
		module_list_utils.taskid = sys_task_id;
		//module_setup_task(MODULE_SERVICE, sys_task_id);
		return OK;
	}
	return ERROR;
}

int service_task_exit (void)
{
	if(sys_task_id)
		os_task_destroy(sys_task_id);
	sys_task_id = 0;
	return OK;
}


int service_module_init (void)
{
	if(master_eloop == NULL)
		master_eloop = eloop_master_name_create("SERVICE");
#ifdef ZPL_SERVICE_TFTPC
#endif
#ifdef ZPL_SERVICE_TFTPD
	tftpdInit(master_eloop, NULL);
#endif
#ifdef ZPL_SERVICE_FTPC
	ftpLibInit(5);
#endif
#ifdef ZPL_SERVICE_FTPD
	ftpdInit (master_eloop, ftpd_loginVerify);
#endif
#ifdef ZPL_SERVICE_SNTPS
	sntpsInit(master_eloop);
#endif
#ifdef ZPL_SERVICE_SNTPC
	sntpcInit(master_eloop);
#endif
#ifdef ZPL_SERVICE_TELNET
#endif
#ifdef ZPL_SERVICE_TELNETD
#endif
#ifdef ZPL_SERVICE_PING
#endif
#ifdef ZPL_SERVICE_TRACEROUTE
#endif
#ifdef ZPL_SERVICE_UBUS_SYNC
	ubus_sync_init(master_eloop);
#endif

	return OK;
}

int service_module_exit (void)
{
#ifdef ZPL_SERVICE_FTPD
	ftpdDisable();
#endif
#ifdef ZPL_SERVICE_TFTPD
	tftpdUnInit();
#endif
#ifdef ZPL_SERVICE_SNTPS
	sntpsDisable();
#endif
#ifdef ZPL_SERVICE_SNTPC
	sntpcDisable();
#endif
#ifdef ZPL_SERVICE_UBUS_SYNC
	ubus_sync_exit();
#endif
	if(master_eloop)
		eloop_master_free(master_eloop);
	master_eloop = NULL;
	return OK;
}


