/*
 * os_start.c
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
#include "os_module.h"
#include "os_start.h"



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

static int os_base_dir_init(void)
{
	if(access(BASE_DIR, F_OK) != 0)
		mkdir(BASE_DIR, 0644);

	if(access(RSYSLOGDIR, F_OK) != 0)
		mkdir(RSYSLOGDIR, 0644);		// /etc

	if(access(PLSYSCONFDIR, F_OK) != 0)
		mkdir(PLSYSCONFDIR, 0644);		// /etc
	if(access(SYSCONFDIR, F_OK) != 0)
		mkdir(SYSCONFDIR, 0644);		// /etc/app
	if(access(SYSLIBDIR, F_OK) != 0)
		mkdir(SYSLIBDIR, 0644);			// /lib
	if(access(SYSSBINDIR, F_OK) != 0)
		mkdir(SYSSBINDIR, 0644);		// /sbin
	if(access(SYSBINDIR, F_OK) != 0)
		mkdir(SYSBINDIR, 0644);			// /bin
	if(access(SYSRUNDIR, F_OK) != 0)
		mkdir(SYSRUNDIR, 0644);			// /run
	if(access(SYSLOGDIR, F_OK) != 0)
		mkdir(SYSLOGDIR, 0644);			// /log
	if(access(SYSVARDIR, F_OK) != 0)
		mkdir(SYSVARDIR, 0644);			// /var
	if(access(SYSTMPDIR, F_OK) != 0)
		mkdir(SYSTMPDIR, 0644);			// /tmp

	if(access(SYSWWWDIR, F_OK) != 0)
		mkdir(SYSWWWDIR, 0644);			// /www
	if(access(SYSWWWCACHEDIR, F_OK) != 0)
		mkdir(SYSWWWCACHEDIR, 0644);			// /www/cache

	if(access(SYSTFTPBOOTDIR, F_OK) != 0)
		mkdir(SYSTFTPBOOTDIR, 0644);			// /tftpboot

	if(access(SYSUPLOADDIR, F_OK) != 0)
		mkdir(SYSUPLOADDIR, 0644);			// /tftpboot

	if(access(BASE_DIR"/img", F_OK) != 0)
		mkdir(BASE_DIR"/img", 0644);

	return 0;
}

static int os_base_dir_load(void)
{
#ifdef ZPL_BUILD_OS_OPENWRT
	if(access(DEFAULT_CONFIG_FILE, F_OK) != 0)
		super_system("cp -af " SYSCONF_REAL_DIR"/default-config.cfg  " DEFAULT_CONFIG_FILE);
#else
#ifdef ZPL_BUILD_ARCH_X86
	if(access(DEFAULT_CONFIG_FILE, F_OK) != 0)
		super_system("cp -af " SYSCONFDIR"/default-config.cfg  " DEFAULT_CONFIG_FILE);
	super_system("cp -arf " SYSCONFDIR"/*" " " PLSYSCONFDIR"/");
#endif
#endif

#ifdef ZPL_WEBGUI_MODULE
	if(access(RSYSWWWDIR"/build.tar.gz", F_OK) == 0)
	{
		//super_system("cp -arf " RSYSWWWDIR"/build.tar.gz /tmp/");
		//super_system("cd /tmp/; tar -zxvf build.tar.gz");
		if(access(SYSWWWDIR"/index.html", F_OK) != 0)
		{
			super_system("tar -zxvf  "RSYSWWWDIR"/build.tar.gz -C /tmp");
			super_system("cp -arf /tmp/build/* " SYSWWWDIR"/");
			super_system("rm -rf /tmp/build ");
		}
	}
	else
	{
		if(access(SYSWWWDIR"/index.html", F_OK) != 0)
			super_system("cp -arf " RSYSWWWDIR"/*" " " SYSWWWDIR"/");
	}
#endif

#ifdef ZPL_BUILD_OS_OPENWRT
#ifdef APP_X5BA_MODULE
	if(access("/etc/config/product", F_OK) != 0)
	{
		if(access(SYSCONFDIR"/product", F_OK) == 0)
			super_system("cp -af " SYSCONFDIR"/product  /etc/config/");
	}
	if(access("/etc/config/voipconfig", F_OK) != 0)
	{
		if(access(SYSCONFDIR"/voipconfig", F_OK) == 0)
			super_system("cp -af " SYSCONFDIR"/voipconfig  /etc/config/");
	}
	if(access("/etc/config/openconfig", F_OK) != 0)
	{
		if(access(SYSCONFDIR"/openconfig", F_OK) == 0)
			super_system("cp -af " SYSCONFDIR"/openconfig  /etc/config/");
	}
#endif
#endif
	return 0;
}


int os_base_env_init(void)
{
	lstLibInit();
	os_nvram_env_init();
	os_base_dir_init();
	return OK;
}

int os_base_env_load(void)
{
	os_base_dir_load();
	return OK;
}

int os_base_signal_init(int daemon_mode)
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

static int os_ip_stack_init(int localport)
{
#ifdef ZPL_IPCOM_STACK_MODULE
	extern int ipcom_demo_init(int localport);
	extern void sys_signal_init(void);

	ipcom_demo_init(localport);
	os_msleep(100);
#endif
	return OK;
}

int os_base_stack_init(const char *tty)
{
	if (os_task_init() == ERROR)
		return ERROR;

	if (os_time_init() == ERROR)
		return ERROR;

	if (os_job_init() == ERROR)
		return ERROR;

	memory_init();
	cmd_init(1);
	
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
	if(tty)
		_global_host.console_enable = zpl_true;
	return OK;
}


int os_base_zlog_open(char *progname)
{
	openzlog (progname, MODULE_DEFAULT, LOG_LOCAL7, 0);
	//zlog_set_level (ZLOG_DEST_STDOUT, ZLOG_LEVEL_DEBUG);
	return OK;
}

int os_base_shell_start(char *shell_path, char *shell_addr, int shell_port, const char *tty)
{
	pl_module_init(MODULE_CONSOLE);
	pl_module_init(MODULE_TELNET);
	vty_tty_init(tty);
	vty_serv_init(shell_addr, shell_port, shell_path, tty);
	zlog_notice(MODULE_DEFAULT, "Zebra %s starting: vty@%d", OEM_VERSION, shell_port);
	pl_module_task_init(MODULE_CONSOLE);
	pl_module_task_init(MODULE_TELNET);
	return OK;
}

int os_base_start_pid(int pro, char *pid_file, int *pid)
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



int os_base_module_start_all()
{

	os_ip_stack_init(8899);

	/*
	 * all module start:
	 */
	os_module_init();
	

	/*
	 * install all CMD
	 */
	os_module_cmd_init();


	/*
	 * start all task
	 */
	os_module_task_init();

	//设置准备初始化标志
	host_loadconfig_stats(LOAD_INIT);
	os_msleep(100);


	//等待BSP初始化，最长等待15s时间
	pl_module_init(MODULE_SDK);
	pl_module_task_init(MODULE_SDK);

	bsp_module_start();
	
	host_waitting_bspinit(15);
#ifdef ZPL_IPCBCBSP_MODULE
	bsp_usp_module_init();
#endif

#ifdef ZPL_KERNEL_STACK_MODULE
	_netlink_load_all();
#endif

	return OK;
}



int os_base_module_exit_all()
{

	//os_module_task_exit();

	os_module_exit();


	os_module_cmd_exit();

	return OK;
}

