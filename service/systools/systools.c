/*
 * systools.c
 *
 *  Created on: Oct 28, 2018
 *      Author: zhurish
 */

#include "systools.h"

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


static struct vty *tftp_vty = NULL;

static int sys_task_id = 0;
static void *master_eloop = NULL;

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

#ifdef ZPL_SERVICE_FTPD
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
	host_config_load_waitting();
	eloop_start_running(NULL, MODULE_UTILS);
	return 0;
}

int systools_task_init ()
{
	if(master_eloop == NULL)
		master_eloop = eloop_master_module_create(MODULE_UTILS);
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
	if(master_eloop == NULL)
		master_eloop = eloop_master_module_create(MODULE_UTILS);
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

int systools_module_exit ()
{
#ifdef ZPL_SERVICE_FTPD
	ftpdDisable();
#endif
#ifdef ZPL_SERVICE_TFTPD
	tftpdUnInit();
#endif
#ifdef ZPL_SERVICE_UBUS_SYNC
	ubus_sync_exit();
#endif
	if(master_eloop)
		eloop_master_free(master_eloop);
	master_eloop = NULL;
	return OK;
}

/*int systools_cmd_init ()
{
	//cmd_tftp_init();
	return OK;
}*/
