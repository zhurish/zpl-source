/*
 * startup_start.c
 *
 *  Created on: Apr 30, 2017
 *      Author: zhurish
 */
#include "auto_include.h"
#include <zplos_include.h>
#include "module.h"
#include "if.h"
#include "zmemory.h"
#include "log.h"
#include "host.h"
#include "thread.h"
#include "vector.h"
#include "vty.h"
#include "vty_user.h"
#include "command.h"
#include "workqueue.h"
#include "bmgt.h"
#include "daemon.h"
#include "nsm_include.h"
#include "nsm_main.h"
#include "hal_include.h"
#include "bsp_driver.h"
#include "startup_module.h"
#include "startup_disk.h"
#include "startup_start.h"


struct startup_option startup_option;


/* SIGHUP handler. */
static void os_sighup(int signo, void *p)
{
#ifdef APP_V9_MODULE
	//v9_app_module_exit();
#endif
	//vty_terminate();
	os_log(SYSCONFDIR"/signo.log", "%s:%d",__func__, os_task_gettid());
	fprintf(stdout, "%s\r\n",__func__);
	fflush(stdout);
	//zlog_notice(MODULE_DEFAULT, "%s: SIGHUP received\r\n",__func__);
	//os_msgq_exit();
	//os_exit_all_module();

	_exit(0);
}

/* SIGINT handler. */
static void os_sigint(int signo, void *p)
{
#ifdef APP_V9_MODULE
	//v9_app_module_exit();
#endif
	vty_terminate();
	os_log(SYSCONFDIR"/signo.log", "%s:%d",__func__, os_task_gettid());
	fprintf(stdout, "%s\r\n",__func__);
	fflush(stdout);
	//zlog_notice(MODULE_DEFAULT, "%s: Terminating on signal\r\n",__func__);
	//os_msgq_exit();
	//os_exit_all_module();
	//_exit(0);
}

/* SIGKILL handler. */
/*
static void os_sigkill(int signo, void *p)
{
#ifdef APP_V9_MODULE
	//v9_app_module_exit();
#endif
	fprintf(stdout, "%s\r\n",__func__);
	fflush(stdout);
	vty_terminate();
	exit(0);
}
*/
/* SIGUSR1 handler. */
static void os_sigusr1(int signo, void *p)
{
	os_log(SYSCONFDIR"/signo.log", "%s:%d",__func__, os_task_gettid());
	fprintf(stdout, "%s\r\n",__func__);
	fflush(stdout);
}

/* SIGCHLD handler. */
static void os_sighld(int signo, void *p)
{
	os_log(SYSCONFDIR"/signo.log", "%s:%d",__func__, os_task_gettid());
	fprintf(stdout, "%s\r\n",__func__);
	exit(0);
}
/* SIGTERM handler. */
static void os_sigterm(int signo, void *p)
{
#ifdef APP_V9_MODULE
	//v9_app_module_exit();
#endif
	vty_terminate();
	os_log(SYSCONFDIR"/signo.log", "%s:%d",__func__, os_task_gettid());
	fprintf(stdout, "%s\r\n",__func__);
	fflush(stdout);
	//zlog_notice(MODULE_DEFAULT, "%s: Terminating on signal\r\n",__func__);
	//os_msgq_exit();
	//os_exit_all_module();
	_exit(0);
}
/*
- SIGTERM：正常退出，清理数据
- SIGHUP，并不退出，通常用来重新装载配置文件，进行某些初始化
- SIGINT，处理Ctrl-C
- SIGCHLD，子进程退出
*/
static struct os_signal_t os_signals[] =
{
	{
		.signal = SIGHUP,
		.signal_handler =&os_sighup,
	},
	{
		.signal = SIGUSR1,
		.signal_handler = &os_sigusr1,
	},
	{
		.signal = SIGINT,
		.signal_handler = &os_sigint,
	},
	{
		.signal = SIGTERM,
		.signal_handler = &os_sigterm,
	},
	/*{
		.signal = SIGKILL,
	 	.signal_handler = &os_sigkill,
	},
	{
		.signal = SIGSEGV,
	 	.signal_handler = &os_sigkill,
	},*/
	{
		.signal = SIGCHLD+1,
	 	.signal_handler = &os_sighld,
	},
};


int zpl_base_signal_init(int daemon_mode)
{
	/* Daemonize. */
	if (daemon_mode && daemon(0, 0) < 0) {
		printf("Zebra daemon failed: %s", strerror(ipstack_errno));
		exit(1);
	}
	os_signal_init(os_signals, array_size(os_signals));
	//signal_init( array_size(os_signals), os_signals);
	return OK;
}

int zpl_base_signal_reload(void)
{
	os_signal_init(os_signals, array_size(os_signals));
	//signal_init( array_size(os_signals), os_signals);
	return OK;
}

int startup_option_default(void)
{
	os_memset(&startup_option, 0, sizeof(struct startup_option));
	startup_option.progname = NULL;
	startup_option.config_file = DEFAULT_CONFIG_FILE;
	startup_option.pid_file = OSPL_PATH_PID;
	startup_option.vty_addr = NULL;
	// startup_option.zserv_path = "/var/run/ProcessMU.sock";//NSM_VTYSH_PATH;
	startup_option.zserv_path = NSM_VTYSH_PATH;
	startup_option.vty_port = PLCLI_VTY_PORT;
	startup_option.daemon_mode = 0;
	startup_option.pid = 0;
	startup_option.tty = NULL;

	return OK;
}

