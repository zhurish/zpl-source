/*
 * os_module.c
 *
 *  Created on: Jun 10, 2017
 *      Author: zhurish
 */
#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"
#include "bmgt.h"
#include "os_start.h"
#include "os_module.h"
#include "module_tbl.h"
#ifdef ZPL_APP_MODULE
#include "application.h"
#endif

int console_enable = 0;

static int telnet_task_id = 0;
static int console_task_id = 0;

static void *master_eloop_default = NULL;

static int cli_telnet_task(void *argv)
{
	module_setup_task(MODULE_TELNET, os_task_id_self());
	host_config_load_waitting();
	eloop_start_running(NULL, MODULE_TELNET);
	return 0;
}

static int cli_telnet_task_init()
{
	cli_shell.telnet_master = eloop_master_module_create(MODULE_TELNET);
	//master_thread[MODULE_TELNET] = cli_shell.console_master_module_create(ZPL_SERVICE_TELNET);
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
	if (cli_shell.telnet_master)
		eloop_master_free(cli_shell.telnet_master);
	cli_shell.telnet_master = NULL;
	return OK;
}

static int cli_console_task(void *argv)
{
	module_setup_task(MODULE_CONSOLE, os_task_id_self());
	host_config_load_waitting();
	os_start_running(NULL, MODULE_CONSOLE);
	return 0;
}

static int cli_console_task_init()
{
	if (cli_shell.console_master == NULL)
		cli_shell.console_master = thread_master_module_create(MODULE_CONSOLE);
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
	if (cli_shell.console_master)
		thread_master_free(cli_shell.console_master);
	cli_shell.console_master = NULL;
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
	if (cli_shell.telnet_master == NULL)
		cli_shell.telnet_master = eloop_master_module_create(MODULE_TELNET);

	if (cli_shell.console_master == NULL)
		cli_shell.console_master = thread_master_module_create(MODULE_CONSOLE);

	cmd_init(1);
	vty_init(cli_shell.console_master, cli_shell.telnet_master);
	vty_user_init();
	memory_init();
	cmd_host_init(1);
	unit_board_init();
	//qos_access_list_init();
	return OK;
}

int os_ip_stack_init(int localport)
{
#ifdef ZPL_IPCOM_STACK_MODULE
	extern int ipcom_demo_init(int localport);
	extern void sys_signal_init(void);

	ipcom_demo_init(localport);
#endif
	return OK;
}
/*
extern int pl_module_name_init(const char * name);
extern int pl_module_init(zpl_uint32 module);
extern int pl_module_exit(zpl_uint32 module);
extern int pl_module_task_name_init(const char * name);
extern int pl_module_task_init(zpl_uint32 module);
extern int pl_module_task_exit(zpl_uint32 module);
extern int pl_module_cmd_name_init(const char * name);
extern int pl_module_cmd_init(zpl_uint32 module);
*/
int os_module_init(void)
{
	os_default_start();
	printf("=======os_default_start");
	pl_module_name_show();

	if (master_eloop_default == NULL)
		master_eloop_default = eloop_master_module_create(MODULE_DEFAULT);
printf( "=======pl_def_module_init");
	pl_def_module_init();
printf("=======af pl_def_module_init");
#ifdef ZPL_SERVICE_SNTPS
	sntpsInit(master_eloop_default);
#endif
#ifdef ZPL_SERVICE_SNTPC
	sntpcInit(master_eloop_default);
#endif
#ifdef ZPL_SERVICE_SYSLOG
	syslogc_lib_init(master_eloop_default, NULL);
#endif
	hal_bsp_init();

	sleep(1);
	return OK;
}

int os_module_exit(void)
{
	/* reverse access_list_init */
	//access_list_add_hook(NULL);
	//access_list_delete_hook(NULL);
	access_list_reset();

	/* reverse prefix_list_init */
	//prefix_list_add_hook(NULL);
	//prefix_list_delete_hook(NULL);
	prefix_list_reset();

	pl_def_module_exit();
	hal_bsp_exit();
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
	hal_bsp_task_init();
	return OK;
}

int os_module_task_exit(void)
{
#ifdef ZPL_TOOLS_PROCESS
	os_process_stop();
#endif

	pl_def_module_task_exit();

#ifdef ZPL_NSM_MODULE
	nsm_task_exit ();
#endif

	hal_bsp_task_exit();

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
	//cmd_host_init(terminal);
	cmd_log_init();
	cmd_os_init();
	cmd_nvram_env_init();
	cmd_os_thread_init();
	cmd_os_eloop_init();
	cmd_memory_init();
	cmd_vty_init();
	cmd_vty_user_init();

#ifdef ZPL_NSM_MODULE
	nsm_module_cmd_init();
#endif

#ifdef ZPL_ZPLMEDIA_MODULE
	zpl_media_cmd_init();
#endif

#ifdef OS_START_TEST
	/*
	 * test module
	 */
	extern int os_test();
	os_test();
#ifdef ZPL_HAL_MODULE
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
