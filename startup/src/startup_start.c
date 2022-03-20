/*
 * startup_start.c
 *
 *  Created on: Apr 30, 2017
 *      Author: zhurish
 */
#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"
//#include "sigevent.h"
#include "module.h"
#include "startup_module.h"
#include "startup_start.h"


struct startup_option startup_option;


/* SIGHUP handler. */
static void os_sighup(int signo, void *p)
{
#ifdef APP_V9_MODULE
	//v9_app_module_exit();
#endif
	vty_terminate();
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
	_exit(0);
}

/* SIGKILL handler. */
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
		.signal_handler = &os_sigint,
	},
	/*{
		.signal = SIGKILL,
	 	.signal_handler = &os_sigkill,
	},*/
	{
		.signal = SIGSEGV,
	 	.signal_handler = &os_sigkill,
	},
	{
		.signal = SIGCHLD,
	 	.signal_handler = &os_sighld,
	},
/*
 {
 .signal = SIGKILL,
 .signal_handler = &os_sigkill,
 },
 */
};


int zpl_base_signal_init(int daemon_mode)
{
	/* Daemonize. */
	if (daemon_mode && daemon(0, 0) < 0) {
		printf("Zebra daemon failed: %s", strerror(ipstack_errno));
		exit(1);
	}
	os_signal_init(os_signals, array_size(os_signals));
	//signal_init(NULL, array_size(os_signals), os_signals);
	return OK;
}

int startup_option_default(void)
{
	os_memset(&startup_option, 0, sizeof(struct startup_option));
	startup_option.progname = NULL;
	startup_option.config_file = DEFAULT_CONFIG_FILE;
	startup_option.pid_file = PATH_ZEBRA_PID;
	startup_option.vty_addr = NULL;
	// startup_option.zserv_path = "/var/run/ProcessMU.sock";//ZEBRA_VTYSH_PATH;
	startup_option.zserv_path = ZEBRA_VTYSH_PATH;
	startup_option.vty_port = ZEBRA_VTY_PORT;
	startup_option.daemon_mode = 0;
	startup_option.pid = 0;
	startup_option.tty = NULL;
	return OK;
}

