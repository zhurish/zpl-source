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
#include "filter.h"
#include "plist.h"
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
	zlog_notice(ZLOG_DEFAULT, "%s: SIGHUP received\r\n",__func__);
	os_msgq_exit();
	os_exit_all_module();

	exit(0);
}

/* SIGINT handler. */
static void os_sigint(void)
{
	zlog_notice(ZLOG_DEFAULT, "%s: Terminating on signal\r\n",__func__);
	os_msgq_exit();
	os_exit_all_module();

	exit(0);
}

/* SIGKILL handler. */
static void os_sigkill(void)
{
	zlog_notice(ZLOG_DEFAULT, "%s\r\n",__func__);
	os_msgq_exit();
	os_exit_all_module();
/*	zlog_backtrace_sigsafe(LOG_DEBUG,"AAAAAAAA");
	zlog_backtrace(LOG_DEBUG);*/
	exit(0);
}

/* SIGUSR1 handler. */
static void os_sigusr1(void)
{
	fprintf(stdout, "%s\r\n",__func__);
}

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
		.signal = SIGSEGV,
	 	.handler = &os_sigkill,
	},
/*
 {
 .signal = SIGKILL,
 .handler = &os_sigkill,
 },
 */
};


int os_start_init(char *progname, module_t pro, int daemon_mode)
{
	/* Make master thread emulator. */
	master_thread[pro] = thread_master_module_create(pro);

	/* Daemonize. */
	if (daemon_mode && daemon(0, 0) < 0) {
		printf("Zebra daemon failed: %s", strerror(errno));
		exit(1);
	}

	lstLibInit();
	os_msgq_init();
	/* Vty related initialize. */
	signal_init(array_size(os_signals), os_signals);

	if (os_task_init() == ERROR)
		return ERROR;

	openzlog (progname, pro, LOG_LOCAL7, 0);
	return OK;
}


int os_log_start(module_t pro, char *logpipe)
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

#ifdef USE_IPSTACK_KERNEL
	kernel_load_all();
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

	zlog_notice(ZLOG_DEFAULT,"Zebra %s starting pid:%d", OEM_VERSION, getpid());
	return OK;
}

int os_load_config(char *config)
{
	host.load = LOAD_NONE;
	vty_load_config(config);
//	sleep(1);
	host.load = LOAD_DONE;
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
