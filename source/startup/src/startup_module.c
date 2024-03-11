/*
 * startup_module.c
 *
 *  Created on: Jun 10, 2017
 *      Author: zhurish
 */
#include "auto_include.h"
#include <zplos_include.h>
#include "module.h"
#include "if.h"
#include "if_utsp.h"
#include "zmemory.h"
#include "log.h"
#include "host.h"
#include "thread.h"
#include "vector.h"
#include "vty.h"
#include "vty_user.h"
#include "command.h"
#include "workqueue.h"
#include "daemon.h"
#include "nsm_include.h"
#include "nsm_main.h"
#include "hal_include.h"
#ifdef ZPL_BSP_MODULE
#include "bsp_driver.h"
#endif
#ifdef ZPL_ACTIVE_STANDBY
#include "ipcstandby.h"
#endif
#include "startup_module.h"
#include "startup_disk.h"

extern struct module_alllist module_lists_tbl[MODULE_MAX];

static int ipcom_stack_init(int localport)
{
#ifdef ZPL_IPCOM_MODULE
	extern int ipcom_demo_init(int localport);
	extern void sys_signal_init(void);

	ipcom_demo_init(localport);
	os_msleep(100);
#endif
	return OK;
}

int zpl_stack_init(char *cfg)
{
	os_limit_stack_size(819200);
	zpl_base_env_init();
	zpl_base_env_load();

	zplib_module_install(module_lists_tbl);

	os_netservice_config_load(cfg);
	return OK;
}

int zpl_stack_start(const char *progname, int localport)
{
	if (os_task_init() == ERROR)
		return ERROR;

	if (os_time_init() == ERROR)
		return ERROR;

	if (os_job_init() == ERROR)
		return ERROR;

	memory_init();
	cmd_init(1);
	host_config_init();

	cmd_host_init(1);
	cmd_memory_init();
	cmd_vty_init();
	cmd_vty_user_init();

	cmd_log_init();
	cmd_os_init();
	cmd_nvram_env_init();
	cmd_os_thread_init();
#ifdef ZPL_IPCOM_MODULE
	cmd_os_eloop_init();
#endif
	zplib_module_cmd_all();
	
	openzlog(progname, MODULE_LIB, LOG_LOCAL7, 0);
	// zlog_set_level (ZLOG_DEST_STDOUT, ZLOG_LEVEL_DEBUG);

	ipcom_stack_init(localport);

	return OK;
}

int startup_module_init(int console_enable)
{
	zplib_module_name_show();
	_global_host.slot = os_netservice_port_get("slot");
#ifdef ZPL_NSM_MODULE
	unit_board_init();
#endif
	_global_host.console_enable = console_enable;
	// 设置准备初始化标志
	host_loadconfig_state(LOAD_INIT);
#ifdef ZPL_PAL_MODULE
	zplib_module_init(MODULE_PAL);
#endif
#ifdef ZPL_HAL_MODULE
	zplib_module_init(MODULE_HAL);
#endif
	os_msleep(10);
#ifdef ZPL_ACTIVE_STANDBY
	zplib_module_init(MODULE_STANDBY);
	zplib_module_task_init(MODULE_STANDBY);
#endif
	os_msleep(10);
#ifdef ZPL_PAL_MODULE
	zplib_module_task_init(MODULE_PAL);
#endif
#ifdef ZPL_HAL_MODULE
	zplib_module_task_init(MODULE_HAL);
#endif

#ifdef ZPL_BSP_MODULE
	zplib_module_init(MODULE_SDK);
	zplib_module_task_init(MODULE_SDK);
#endif
	os_msleep(50);

	return OK;
}

int startup_module_load(void)
{
#ifdef ZPL_NSM_MODULE
	zplib_module_init(MODULE_NSM);
	zplib_module_task_init(MODULE_NSM);	
#endif

	os_msleep(50);
#ifdef ZPL_BSP_MODULE
	bsp_module_start();
#endif

	os_msleep(50);
	zplib_module_initall();
	zplib_module_task_startall();
	os_msleep(50);
	return OK;
}


int startup_module_waitting(void)
{
	os_msleep(100);

#ifdef ZPL_ACTIVE_STANDBY
	ipcstandby_done(10);
#endif

	host_waitting_bspinit(10);

	unit_board_startup();

#ifdef ZPL_NSM_MODULE
	nsm_module_start();
#endif
	return OK;
}

int startup_module_stop(void)
{
#ifdef ZPL_TOOLS_PROCESS
	os_process_stop();
#endif
	zplib_module_task_stopall();

#ifdef ZPL_ACTIVE_STANDBY
	zplib_module_task_exit(MODULE_STANDBY);
#endif
#ifdef ZPL_BSP_MODULE
	zplib_module_task_exit(MODULE_SDK);
#endif
#ifdef ZPL_HAL_MODULE
	zplib_module_task_exit(MODULE_HAL);
#endif
#ifdef ZPL_PAL_MODULE
	zplib_module_task_exit(MODULE_PAL);
#endif
#ifdef ZPL_NSM_MODULE
	zplib_module_task_exit(MODULE_NSM);	
#endif

	zplib_module_task_exit(MODULE_SHELL);

	return OK;
}

int startup_module_exit(void)
{
	zplib_module_exitall();
#ifdef ZPL_ACTIVE_STANDBY
	zplib_module_exit(MODULE_STANDBY);
#endif
#ifdef ZPL_BSP_MODULE
	zplib_module_exit(MODULE_SDK);
#endif
#ifdef ZPL_HAL_MODULE
	zplib_module_exit(MODULE_HAL);
#endif
#ifdef ZPL_PAL_MODULE
	zplib_module_exit(MODULE_PAL);
#endif
#ifdef ZPL_NSM_MODULE
	zplib_module_exit(MODULE_NSM);	
#endif

	zplib_module_exit(MODULE_SHELL);
	
	cmd_terminate();

	os_time_exit();
	os_job_exit();

	os_task_exit();
	if (zlog_default)
		closezlog(zlog_default);
	return OK;
}

int zpl_base_shell_start(char *shell_path, char *shell_addr, int shell_port, const char *tty)
{
	zplib_module_init(MODULE_SHELL);
	//vty_tty_init(tty);
	vty_shell_start(shell_addr, shell_port, shell_path, tty);
	zlog_notice(MODULE_SHELL, "Zebra %s starting: vty@%d", OEM_VERSION, shell_port);

	zplib_module_task_init(MODULE_SHELL);
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

	zlog_notice(MODULE_SHELL, "Zebra %s starting pid:%d", OEM_VERSION, getpid());
	return OK;
}
