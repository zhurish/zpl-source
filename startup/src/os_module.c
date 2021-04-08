/*
 * os_module.c
 *
 *  Created on: Jun 10, 2017
 *      Author: zhurish
 */
#include "zebra.h"
#include "memory.h"
#include "command.h"
#include "memory.h"
#include "prefix.h"
#include "sigevent.h"
#include "version.h"
#include <log.h>
#include "getopt.h"
#include "eloop.h"
#include "os_start.h"
#include "os_module.h"
#include "module_tbl.h"
#ifdef PL_APP_MODULE
#include "application.h"
#endif

int console_enable = 0;

static int telnet_task_id = 0;
static int console_task_id = 0;

static int cli_telnet_task(void *argv)
{
	module_setup_task(MODULE_TELNET, os_task_id_self());
	while (!os_load_config_done())
	{
		os_sleep(1);
	}
	eloop_start_running(NULL, MODULE_TELNET);
	return 0;
}

static int cli_telnet_task_init()
{
	if (master_eloop[MODULE_TELNET] == NULL)
		master_eloop[MODULE_TELNET] = eloop_master_module_create(MODULE_TELNET);
	//master_thread[MODULE_TELNET] = thread_master_module_create(PL_SERVICE_TELNET);
	if (telnet_task_id == 0)
		telnet_task_id = os_task_create("telnetdTask", OS_TASK_DEFAULT_PRIORITY,
										0, cli_telnet_task, NULL, OS_TASK_DEFAULT_STACK);
	if (telnet_task_id)
		return OK;
	return ERROR;
}

static int cli_telnet_task_exit()
{
	if (telnet_task_id)
		os_task_destroy(telnet_task_id);
	telnet_task_id = 0;
	if (master_eloop[MODULE_TELNET])
		eloop_master_free(master_eloop[MODULE_TELNET]);
	master_eloop[MODULE_TELNET] = NULL;
	return OK;
}

static int cli_console_task(void *argv)
{
	module_setup_task(MODULE_CONSOLE, os_task_id_self());
	while (!os_load_config_done())
	{
		os_sleep(1);
	}
	os_start_running(NULL, MODULE_CONSOLE);
	return 0;
}

static int cli_console_task_init()
{
	if (master_thread[MODULE_CONSOLE] == NULL)
		master_thread[MODULE_CONSOLE] = thread_master_module_create(MODULE_CONSOLE);

	if (console_task_id == 0)
		console_task_id = os_task_create("consoleTask", OS_TASK_DEFAULT_PRIORITY,
										 0, cli_console_task, NULL, OS_TASK_DEFAULT_STACK);
	if (console_task_id)
		return OK;
	return ERROR;
}

static int cli_console_task_exit()
{
	if (console_task_id)
		os_task_destroy(console_task_id);
	console_task_id = 0;
	if (master_thread[MODULE_CONSOLE])
		thread_master_free(master_thread[MODULE_CONSOLE]);
	master_thread[MODULE_CONSOLE] = NULL;
	return OK;
}

int os_shell_start(char *shell_path, char *shell_addr, int shell_port, const char *tty)
{
	/* Make vty server socket. */
	vty_serv_init(shell_addr, shell_port, shell_path, tty);
	//cli_console_task_init ();
	//cli_telnet_task_init ();
	/* Print banner. */
	zlog_notice(MODULE_DEFAULT, "Zebra %s starting: vty@%d", OEM_VERSION, shell_port);
	//fprintf(stdout,"Zebra %s starting: vty@%d\r\n", QUAGGA_VERSION, shell_port);
	return OK;
}

static int os_default_start()
{
	/* Make master thread emulator. */
	if (master_eloop[MODULE_TELNET] == NULL)
		master_eloop[MODULE_TELNET] = eloop_master_module_create(MODULE_TELNET);

	if (master_thread[MODULE_CONSOLE] == NULL)
		master_thread[MODULE_CONSOLE] = thread_master_module_create(MODULE_CONSOLE);

	cmd_init(1);
	vty_init(master_thread[MODULE_CONSOLE], master_eloop[MODULE_TELNET]);
	vty_user_init();
	memory_init();
	return OK;
}

int os_ip_stack_init(int localport)
{
#ifndef USE_IPSTACK_KERNEL
	extern int ipcom_demo_init(int localport);
	extern void sys_signal_init(void);

	ipcom_demo_init(localport);
#endif
	return OK;
}
/*
extern int pl_module_name_init(const char * name);
extern int pl_module_init(ospl_uint32 module);
extern int pl_module_exit(ospl_uint32 module);
extern int pl_module_task_name_init(const char * name);
extern int pl_module_task_init(ospl_uint32 module);
extern int pl_module_task_exit(ospl_uint32 module);
extern int pl_module_cmd_name_init(const char * name);
extern int pl_module_cmd_init(ospl_uint32 module);
*/
int os_module_init(void)
{
	os_default_start();

	pl_module_name_show();

	if (master_eloop[MODULE_DEFAULT] == NULL)
		master_eloop[MODULE_DEFAULT] = eloop_master_module_create(MODULE_DEFAULT);

	pl_def_module_init();

#ifdef PL_SERVICE_SNTPS
	sntpsInit(master_eloop[MODULE_DEFAULT]);
#endif
#ifdef PL_SERVICE_SNTPC
	sntpcInit(master_eloop[MODULE_DEFAULT]);
#endif
#ifdef PL_SERVICE_SYSLOG
	syslogc_lib_init(master_eloop[MODULE_DEFAULT], NULL);
#endif

	sleep(1);
	return OK;
}

int os_module_exit(void)
{
	/* reverse access_list_init */
	access_list_add_hook(NULL);
	access_list_delete_hook(NULL);
	access_list_reset();

	/* reverse prefix_list_init */
	prefix_list_add_hook(NULL);
	prefix_list_delete_hook(NULL);
	prefix_list_reset();

	pl_def_module_exit();

	return OK;
}

int os_module_task_init(void)
{
	//printf("%s\r\n",__func__);
	//sleep(2);
	os_msleep(500);
	if (console_enable)
		cli_console_task_init();
	cli_telnet_task_init();

	pl_def_module_task_init();

	return OK;
}

int os_module_task_exit(void)
{
#ifdef DOUBLE_PROCESS
	os_process_stop();
#endif

	pl_def_module_task_exit();

#ifdef PL_NSM_MODULE
	nsm_task_exit ();
#endif

	os_time_exit();
	os_job_exit();
	if (console_enable)
		cli_console_task_exit();
	cli_telnet_task_exit();

	os_task_exit();
	return OK;
}

int os_module_cmd_init(int terminal)
{
	//system cmd
	cmd_host_init(terminal);
	cmd_log_init();
	cmd_os_init();
	cmd_nvram_env_init();
	cmd_os_thread_init();
	cmd_os_eloop_init();
	cmd_memory_init();
	cmd_vty_init();
	cmd_vty_user_init();

#ifdef PL_NSM_MODULE
	nsm_module_cmd_init();
#endif


#ifdef OS_START_TEST
	/*
	 * test module
	 */
	extern int os_test();
	os_test();
#ifdef PL_HAL_MODULE
	hal_test_init();
#endif
#endif
	return OK;
}

int os_module_cmd_exit(void)
{
	vrf_terminate();
	vty_terminate();
	cmd_terminate();

	if (zlog_default)
		closezlog(zlog_default);
	return OK;
}
