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
#include "ftpdLib.h"
#include "ftpLib.h"
#include "tftpdLib.h"
#include "tftpLib.h"
#include "pingLib.h"
#include "telnetLib.h"
#include "tracerouteLib.h"

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


const char *ftpd_hostname()
{
	return "VxWorks 5.5";
}

static int ftpd_loginVerify(char *user, char *pass)
{
	return user_authentication(user, pass);
}

static int systools_task(void *argv)
{
	module_setup_task(MODULE_TELNET, os_task_id_self());
	while(!os_load_config_done())
	{
		os_sleep(1);
	}
	eloop_start_running(NULL, MODULE_TELNET);
	return 0;
}

int systools_task_init ()
{
	if(master_eloop[MODULE_TELNET] == NULL)
		master_eloop[MODULE_TELNET] = eloop_master_module_create(MODULE_TELNET);
	//master_thread[MODULE_TELNET] = thread_master_module_create(MODULE_TELNET);
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

	ftpLibInit(5);
	ftpdInit (master_eloop[MODULE_UTILS], ftpd_loginVerify);
	tftpdInit(master_eloop[MODULE_UTILS], NULL);

	return OK;
}

int systools_module_exit ()
{
	ftpdDisable();
	tftpdUnInit();


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
