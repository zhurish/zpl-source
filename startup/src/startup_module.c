/*
 * startup_module.c
 *
 *  Created on: Jun 10, 2017
 *      Author: zhurish
 */
#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"
#include "bmgt.h"
#include "module.h"
#include "startup_module.h"



static int ipcom_stack_init(int localport)
{
#ifdef ZPL_IPCOM_STACK_MODULE
	extern int ipcom_demo_init(int localport);
	extern void sys_signal_init(void);

	ipcom_demo_init(localport);
	os_msleep(100);
#endif
	return OK;
}

int zpl_stack_init(void)
{
	os_limit_stack_size(10240);

	zpl_base_env_init();
	zpl_base_env_load();
	
	return OK;
}

int zpl_stack_start(const char* progname, int localport)
{
	if (os_task_init() == ERROR)
		return ERROR;

	if (os_time_init() == ERROR)
		return ERROR;

	if (os_job_init() == ERROR)
		return ERROR;

	memory_init();
	cmd_init(1);

	openzlog (progname, MODULE_DEFAULT, LOG_LOCAL7, 0);
	//zlog_set_level (ZLOG_DEST_STDOUT, ZLOG_LEVEL_DEBUG);

	vty_user_init();

	cmd_host_init(1);
	cmd_memory_init();
	cmd_vty_init();
	cmd_vty_user_init();

	cmd_log_init();
	cmd_os_init();
	cmd_nvram_env_init();
	cmd_os_thread_init();
#ifdef ZPL_IPCOM_STACK_MODULE
	cmd_os_eloop_init();
#endif

#ifdef ZPL_NSM_MODULE
	nsm_module_cmd_init();
#endif

#ifdef ZPL_ZPLMEDIA_MODULE
	zpl_media_cmd_init();
#endif
	ipcom_stack_init(localport);
	return OK;
}




int startup_module_init(int console_enable)
{
	zplib_module_name_show();
	
	unit_board_init();
	_global_host.console_enable = console_enable;
	//设置准备初始化标志
	host_loadconfig_stats(LOAD_INIT);
	
	zplib_module_init(MODULE_HAL);
	zplib_module_init(MODULE_PAL);
	os_msleep(50);

	zplib_module_task_init(MODULE_HAL);
	zplib_module_task_init(MODULE_PAL);

	//等待BSP初始化，最长等待15s时间
	zplib_module_init(MODULE_SDK);
	zplib_module_task_init(MODULE_SDK);

	os_msleep(50);
	return OK;
}

int startup_module_load(void)
{
	bsp_module_start();
	os_msleep(50);

	zplib_module_init(MODULE_NSM);
	zplib_module_initall();
	os_msleep(50);

	zplib_module_task_init(MODULE_NSM);
	zplib_module_task_startall();

	os_msleep(50);
	return OK;
}
/*
int startup_module_start(void)
{
	os_msleep(50);
	return OK;
}
*/
int startup_module_waitting(void)
{
	os_msleep(2000);

	host_waitting_bspinit(15);
	
	nsm_module_start();

#ifdef ZPL_IPCBCBSP_MODULE
	bsp_usp_module_init();
#endif

#ifdef ZPL_KERNEL_STACK_MODULE
	_netlink_load_all();
#endif
	//eth_drv_init(0);
	//eth_drv_start(0);
	return OK;
}

int startup_module_stop(void)
{
#ifdef ZPL_TOOLS_PROCESS
	os_process_stop();
#endif
	zplib_module_task_stopall();
	return OK;
}

int startup_module_exit(void)
{
	vrf_terminate();
	vty_terminate();
	cmd_terminate();

	zplib_module_exitall();

	os_time_exit();
	os_job_exit();

	os_task_exit();
	if (zlog_default)
		closezlog(zlog_default);
	return OK;
}


int zpl_base_shell_start(char *shell_path, char *shell_addr, int shell_port, const char *tty)
{
	zplib_module_init(MODULE_CONSOLE);
	zplib_module_init(MODULE_TELNET);
	vty_tty_init(tty);
	vty_serv_init(shell_addr, shell_port, shell_path, tty);
	zlog_notice(MODULE_DEFAULT, "Zebra %s starting: vty@%d", OEM_VERSION, shell_port);
	zplib_module_task_init(MODULE_CONSOLE);
	zplib_module_task_init(MODULE_TELNET);
	return OK;
}

int zpl_base_start_pid(int pro, char *pid_file, int *pid)
{
	/* Output pid of zebra. */
	if (pid_file)
		pid_output(pid_file);

	/* Needed for BSD routing socket. */
	if (pid)
		*pid = getpid();

	zlog_notice(MODULE_DEFAULT,"Zebra %s starting pid:%d", OEM_VERSION, getpid());
	return OK;
}
