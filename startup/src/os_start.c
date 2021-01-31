/*
 * platform_start.c
 *
 *  Created on: Apr 30, 2017
 *      Author: zhurish
 */
#include "zebra.h"


#include "command.h"
#include "memory.h"
#include "prefix.h"
#include "sigevent.h"
#include "version.h"
#include <log.h>
#include "getopt.h"
//#include "nsm_filter.h"
//#include "nsm_plist.h"
#include "host.h"
#include "eloop.h"
#include "os_job.h"
#include "os_list.h"
//#include "os_log.h"
#include "os_memory.h"
#include "os_sem.h"
#include "os_task.h"
#include "os_module.h"
#include "os_start.h"



/* SIGHUP handler. */
static void os_sighup(void)
{
#ifdef APP_V9_MODULE
	v9_app_module_exit();
#endif
	vty_terminate();
	os_log("/app/signo.log", "%s:%d",__func__, os_task_gettid());
	//zlog_notice(MODULE_DEFAULT, "%s: SIGHUP received\r\n",__func__);
	//os_msgq_exit();
	//os_exit_all_module();

	_exit(0);
}

/* SIGINT handler. */
static void os_sigint(void)
{
#ifdef APP_V9_MODULE
	v9_app_module_exit();
#endif
	vty_terminate();
	os_log("/app/signo.log", "%s:%d",__func__, os_task_gettid());
	//zlog_notice(MODULE_DEFAULT, "%s: Terminating on signal\r\n",__func__);
	//os_msgq_exit();
	//os_exit_all_module();


	_exit(0);
}

/* SIGKILL handler. */
static void os_sigkill(void)
{
#ifdef APP_V9_MODULE
	v9_app_module_exit();
#endif
	vty_terminate();
	exit(0);
}

/* SIGUSR1 handler. */
static void os_sigusr1(void)
{
	os_log("/app/signo.log", "%s:%d",__func__, os_task_gettid());
	fprintf(stdout, "%s\r\n",__func__);
}

/* SIGCHLD handler. */
/*static void os_sighld(void)
{
	os_log("/app/signo.log", "%s:%d",__func__, os_task_gettid());
	fprintf(stdout, "%s\r\n",__func__);
}*/

static struct quagga_signal_t os_signals[] =
{
	{
		.signal = SIGHUP,
		.handler =&os_sighup,
	},
	{
		.signal = SIGUSR1,
		.handler = &os_sigusr1,
	},
	{
		.signal = SIGINT,
		.handler = &os_sigint,
	},
	{
		.signal = SIGTERM,
		.handler = &os_sigint,
	},
	{
		.signal = SIGKILL,
	 	.handler = &os_sigkill,
	},
	{
		.signal = SIGSEGV,
	 	.handler = &os_sigkill,
	},
/*	{
		.signal = SIGCHLD,
	 	.handler = &os_sighld,
	},*/
/*
 {
 .signal = SIGKILL,
 .handler = &os_sigkill,
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
#ifdef PL_BUILD_OPENWRT
	if(access(DEFAULT_CONFIG_FILE, F_OK) != 0)
		super_system("cp -af " SYSCONF_REAL_DIR"/default-config.cfg  " DEFAULT_CONFIG_FILE);
#else
#ifdef PL_BUILD_X86
	if(access(DEFAULT_CONFIG_FILE, F_OK) != 0)
		super_system("cp -af " SYSCONFDIR"/default-config.cfg  " DEFAULT_CONFIG_FILE);
	super_system("cp -arf " SYSCONFDIR"/*" " " PLSYSCONFDIR"/");
#endif
#endif

#ifdef PL_WEBGUI_MODULE
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

#ifdef PL_BUILD_OPENWRT
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


int os_base_init(void)
{
	lstLibInit();
	os_nvram_env_init();
	os_base_dir_init();
	return OK;
}

int os_base_load(void)
{
	os_base_dir_load();
	return OK;
}

int os_start_init(char *progname, module_t pro, int daemon_mode, char *tty)
{
	/* Daemonize. */
	if (daemon_mode && daemon(0, 0) < 0) {
		printf("Zebra daemon failed: %s", strerror(errno));
		exit(1);
	}

	/* Make master thread emulator. */
	master_thread[pro] = thread_master_module_create(pro);

	/* Vty related initialize. */
	remove("/app/signo.log");
	signal_init(array_size(os_signals), os_signals);

	if (os_task_init() == ERROR)
		return ERROR;

	openzlog (progname, pro, LOG_LOCAL7, 0);
	if(tty)
		vty_tty_init(tty);
	return OK;
}


int os_start_early(module_t pro, char *logpipe)
{
	master_thread[pro] = thread_master_module_create(pro);

	if (os_time_init() == ERROR)
		return ERROR;

	if (os_job_init() == ERROR)
		return ERROR;
	//nsm_log_init(master_eloop[pro], logpipe);
	return OK;
}




int os_start_all_module()
{

	os_ip_stack_init(8899);
	/*
	 * all module start:
	 */
	os_module_init();
	os_msleep(500);

	/*
	 * install all CMD
	 */
	os_module_cmd_init(1);
	os_msleep(500);

	/*
	 * start all task
	 */
	os_module_task_init();
	os_msleep(500);
#ifdef PL_PAL_MODULE
#ifdef USE_IPSTACK_KERNEL
	kernel_load_all();
#endif
#endif
	return OK;
}



int os_exit_all_module()
{

	//os_module_task_exit();

	os_module_exit();


	os_module_cmd_exit();

	return OK;
}


int os_start_pid(int pro, char *pid_file, int *pid)
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

int os_load_config(char *config)
{
	host.load = LOAD_NONE;
	printf("==================os_load_config %s\r\n",config);
	vty_load_config(config);
	printf("++++++++++++++++++os_load_config %s\r\n",config);
//	sleep(1);
	host.load = LOAD_DONE;
	signal_init(array_size(os_signals), os_signals);
	//os_task_give_broadcast();
	return OK;
}

BOOL os_load_config_done(void)
{
	if(host.load == LOAD_DONE)
		return TRUE;
	return FALSE;
}

static int os_thread_start(void *m, module_t pro)
{
	struct thread thread;
	struct thread_master *master = (struct thread_master *) m;
	if (master == NULL)
		master = thread_master_module_create(pro);
	if (master == NULL)
		return ERROR;
	module_setup_task(master->module, os_task_id_self());
	while(!os_load_config_done())
	{
		os_sleep(1);
	}
	/* Output pid of zebra. */
//	pid_output (pid_file);
	/* Needed for BSD routing socket. */
//	if(pid)
//		*pid = getpid ();
	//master->ptid = os_task_pthread_self();
	//master->taskId = os_task_id_self();
	//os_log_reopen(master->module);
//	zlog_notice ("Zebra %s starting pid:%d", QUAGGA_VERSION, getpid ());

	while (thread_fetch((struct thread_master *) master, &thread))
		thread_call(&thread);
	return OK;
}

int os_start_running(void *master, module_t pro)
{
	return os_thread_start(master, pro);
}


static int os_eloop_start(void *m, module_t pro)
{
	struct eloop thread;
	struct eloop_master *master = (struct eloop_master *) m;
	if (master == NULL)
		master = eloop_master_module_create(pro);
	if (master == NULL)
		return ERROR;
	module_setup_task(master->module, os_task_id_self());
	while(!os_load_config_done())
	{
		os_sleep(1);
	}
	//master->ptid = os_task_pthread_self();
	//master->taskId = os_task_id_self();
	//os_log_reopen(master->module);

	while (eloop_fetch((struct eloop_master *) master, &thread))
		eloop_call(&thread);
	return OK;
}

int eloop_start_running(void *master, module_t pro)
{
	return os_eloop_start(master, pro);
}

/*
 int os_zebra_start(void *master, char *pid_file)
 {
 os_log_default[ZLOG_ZEBRA] = os_log_open("ZEBRA", ZLOG_ZEBRA, NULL, 0);
 return OK;
 }
 */
