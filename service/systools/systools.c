/*
 * systools.c
 *
 *  Created on: Oct 28, 2018
 *      Author: zhurish
 */
#include "zebra.h"
#include "vty.h"
#include "eloop.h"

#include "systools.h"

#ifdef PL_TFTPC_MODULE
#include "tftpLib.h"
#endif
#ifdef PL_TFTPD_MODULE
#include "tftpdLib.h"
#endif
#ifdef PL_FTPC_MODULE
#include "ftpLib.h"
#endif
#ifdef PL_FTPD_MODULE
#include "ftpdLib.h"
#endif
#ifdef PL_TELNET_MODULE
#include "telnetLib.h"
#endif
#ifdef PL_TELNETD_MODULE
#include "telnetLib.h"
#endif
#ifdef PL_PING_MODULE
#include "pingLib.h"
#endif
#ifdef PL_TRACEROUTE_MODULE
#include "tracerouteLib.h"
#endif
#ifdef PL_UBUS_MODULE
#include "ubus_sync.h"
#endif


static struct vty *tftp_vty = NULL;

static int sys_task_id = 0;


int systools_set(void *vty)
{
	tftp_vty = vty;
	return OK;
}

int systools_printf(const char *format, ...)
{
	va_list args;
	char buf[1024];
	int len = 0;
	struct vty *vty = tftp_vty;
	memset(buf, 0, sizeof(buf));
	if (!tftp_vty)
	{
		va_start(args, format);
		len = vprintf(format, args);
		va_end(args);
	}
	else
	{
		va_start(args, format);
		len = vsnprintf(buf, sizeof(buf), format, args);
		va_end(args);
		if (len)
			vty_out(vty, "%s", buf);
	}
	return len;
}

#ifdef PL_FTPD_MODULE
const char *ftpd_hostname()
{
	return "VxWorks 5.5";
}

static int ftpd_loginVerify(char *user, char *pass)
{
	return user_authentication(user, pass);
}
#endif

static int systools_task(void *argv)
{
	module_setup_task(MODULE_UTILS, os_task_id_self());
	while(!os_load_config_done())
	{
		os_sleep(1);
	}
	eloop_start_running(NULL, MODULE_UTILS);
	return 0;
}

int systools_task_init ()
{
	if(master_eloop[MODULE_UTILS] == NULL)
		master_eloop[MODULE_UTILS] = eloop_master_module_create(MODULE_UTILS);
	//master_thread[MODULE_UTILS] = thread_master_module_create(MODULE_UTILS);
	if(sys_task_id == 0)
		sys_task_id = os_task_create("sysTask", OS_TASK_DEFAULT_PRIORITY,
	               0, systools_task, NULL, OS_TASK_DEFAULT_STACK);
	if(sys_task_id)
		return OK;
	return ERROR;
}

int systools_task_exit ()
{
	if(sys_task_id)
		os_task_destroy(sys_task_id);
	sys_task_id = 0;
	return OK;
}


int systools_module_init ()
{
	if(master_eloop[MODULE_UTILS] == NULL)
		master_eloop[MODULE_UTILS] = eloop_master_module_create(MODULE_UTILS);
#ifdef PL_TFTPC_MODULE
#endif
#ifdef PL_TFTPD_MODULE
	tftpdInit(master_eloop[MODULE_UTILS], NULL);
#endif
#ifdef PL_FTPC_MODULE
	ftpLibInit(5);
#endif
#ifdef PL_FTPD_MODULE
	ftpdInit (master_eloop[MODULE_UTILS], ftpd_loginVerify);
#endif
#ifdef PL_TELNET_MODULE
#endif
#ifdef PL_TELNETD_MODULE
#endif
#ifdef PL_PING_MODULE
#endif
#ifdef PL_TRACEROUTE_MODULE
#endif
#ifdef PL_UBUS_MODULE
	ubus_sync_init(master_eloop[MODULE_UTILS]);
#endif

	return OK;
}

int systools_module_exit ()
{
#ifdef PL_FTPD_MODULE
	ftpdDisable();
#endif
#ifdef PL_TFTPD_MODULE
	tftpdUnInit();
#endif
#ifdef PL_UBUS_MODULE
	ubus_sync_exit();
#endif
	if(master_eloop[MODULE_UTILS])
		eloop_master_free(master_eloop[MODULE_UTILS]);
	master_eloop[MODULE_UTILS] = NULL;
	return OK;
}

/*int systools_cmd_init ()
{
	//cmd_tftp_init();
	return OK;
}*/
