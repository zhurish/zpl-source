/*
 * startup_start.c
 *
 *  Created on: Apr 30, 2017
 *      Author: zhurish
 */
#include "auto_include.h"
#include <zplos_include.h>
#include "module.h"
#include "getopt.h"
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
#include "if_utsp.h"
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
	zlog_warn(MODULE_LIB, "++++++++++++++++++++%s++++++++++++++++++++signo=%d\r\n",__func__, signo);
	fprintf(stdout, "++++++++++++++++++++%s++++++++++++++++++++signo=%d\r\n",__func__, signo);
	fflush(stdout);
	vty_terminate();
	//zlog_notice(MODULE_DEFAULT, "%s: SIGHUP received\r\n",__func__);
	//os_msgq_exit();
	//os_exit_all_module();

	_exit(0);
}

/* SIGINT handler. */
static void os_sigint(int signo, void *p)
{
	zlog_warn(MODULE_LIB, "++++++++++++++++++++%s++++++++++++++++++++signo=%d\r\n",__func__, signo);
	fprintf(stdout, "++++++++++++++++++++%s++++++++++++++++++++signo=%d\r\n",__func__, signo);
	fflush(stdout);
	vty_terminate();
	//zlog_notice(MODULE_DEFAULT, "%s: Terminating on signal\r\n",__func__);
	//os_msgq_exit();
	//os_exit_all_module();
	_exit(0);
}

/* SIGKILL handler. */
/*
static void os_sigkill(int signo, void *p)
{
	fprintf(stdout, "%s\r\n",__func__);
	fflush(stdout);
	vty_terminate();
	exit(0);
}
*/
/* SIGUSR1 handler. */
static void os_sigusr1(int signo, void *p)
{
	zlog_warn(MODULE_LIB, "++++++++++++++++++++%s++++++++++++++++++++signo=%d\r\n",__func__, signo);
	fprintf(stdout, "++++++++++++++++++++%s++++++++++++++++++++signo=%d\r\n",__func__, signo);
	fflush(stdout);
}

/* SIGCHLD handler. */
static void os_sighld(int signo, void *p)
{
	zlog_warn(MODULE_LIB, "++++++++++++++++++++%s++++++++++++++++++++signo=%d\r\n",__func__, signo);
	fprintf(stdout, "++++++++++++++++++++%s++++++++++++++++++++signo=%d\r\n",__func__, signo);
	fflush(stdout);
	//exit(0);
}
/* SIGTERM handler. */
static void os_sigterm(int signo, void *p)
{
	zlog_warn(MODULE_LIB, "++++++++++++++++++++%s++++++++++++++++++++signo=%d\r\n",__func__, signo);
	fprintf(stdout, "++++++++++++++++++++%s++++++++++++++++++++signo=%d\r\n",__func__, signo);
	fflush(stdout);
	vty_terminate();
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
	startup_option.service_file = SERVICE_CONFIG_FILE;
	startup_option.vty_addr = NULL;
	startup_option.daemon_mode = 0;
	startup_option.tty = NULL;
	
	return OK;
}


/* Command line options. */
static struct option zpllongopts[] =
	{
		// df:i:z:hA:P:t:u:g:v
		{"daemon", no_argument, NULL, 'd'},
		{"config_file", required_argument, NULL, 'f'},
		{"service_file", required_argument, NULL, 's'},
		{"help", no_argument, NULL, 'h'},
		{"vty_addr", required_argument, NULL, 'A'},
		{"tty", required_argument, NULL, 't'},
		{"none", no_argument, NULL, 'n'},
		{"version", no_argument, NULL, 'v'},
		{0}};
/* Help information display. */
static void
usage(char *progname, int status)
{
	if (status != 0)
		fprintf(stderr, "Try `%s --help' for more information.\n", progname);
	else
	{
		printf("Usage : %s [OPTION...]\n\n"
			   "Daemon which manages kernel routing table management and "
			   "redistribution between different routing protocols.\n\n"
			   "-d, --daemon       Runs in daemon mode\n"
			   "-f, --config_file  Set configuration file name\n"
			   "-s, --service_file  Set Service file name\n"
			   "-A, --vty_addr     Set vty's bind address\n"
			   "-t, --tty          Set tty Device Name for shell\n"
			   "-n, --none	  none stdin for shell\n",
			   progname);
		printf("-v, --version      Print program version\n"
			   "-h, --help         Display this help and exit\n"
			   "\n"
			   "Report bugs to %s\n",
			   OEM_BUG_ADDRESS);
	}

	exit(status);
}

int zplmain_getopt(int argc, char **argv)
{
	while (1)
	{
		int opt = getopt_long(argc, argv, "df:hA:t:s:nv", zpllongopts, NULL);
		if (opt == EOF || opt == -1)
		{
			break;
		}
		switch (opt)
		{
		case 0:
			break;
		case 'd':
			startup_option.daemon_mode = 1;
			break;
		case 's':
			startup_option.service_file = optarg;
			break;

		case 'f':
			startup_option.config_file = optarg;
			break;

		case 'A':
			// if(startup_option.vty_addr)
			//   os_free(startup_option.vty_addr);
			// startup_option.vty_addr = os_strdup(optarg);
			startup_option.vty_addr = optarg;
			//	    	  vty_addr = optarg;
			break;

		case 't':
			if (startup_option.tty)
				free(startup_option.tty);
			// startup_option.tty = strdup("/dev/ttyS0");
			startup_option.tty = (optarg);
			break;
		case 'n':
			if (startup_option.tty)
				free(startup_option.tty);
			startup_option.tty = NULL;
			break;
		case 'v':
  			printf("%s version %s\n", startup_option.progname, OEM_VERSION);
  			printf("%s\n", OEM_PACKAGE_COPYRIGHT);
			exit(0);
			break;
		case 'h':
			usage(startup_option.progname, 0);
			break;
		default:
			usage(startup_option.progname, 1);
			break;
		}
	}
	return OK;
}