/*
 * platform_main.c
 *
 *  Created on: Apr 30, 2017
 *      Author: zhurish
 */
//#define USE_IPSTACK_KERNEL
#ifdef USE_IPSTACK_KERNEL

#endif


#include "zebra.h"
#include "getopt.h"
#include <log.h>


#include "command.h"
#include "memory.h"
#include "prefix.h"
#include "sigevent.h"
#include "thread.h"
//#include "version.h"
#include "vrf.h"
#include "os_start.h"
#include "os_module.h"
#include "platform/nsm/filter.h"
#include "platform/nsm/plist.h"

//extern struct zebra_privs_t os_privs;
//extern struct quagga_signal_t os_signals[];
//handle SIGUSR2 nostop noprint
/*
 * /proc/sys/net/ipv4/tcp_orphan_retries 1
 * /proc/sys/net/ipv4/tcp_fin_timeout 10
 * cat /proc/sys/net/ipv4/tcp_keepalive_time 30
 *
 * 编辑/etc/sysctl.conf文件，增加三行：
引用
net.ipv4.tcp_fin_timeout = 30
net.ipv4.tcp_keepalive_time = 1200
net.ipv4.tcp_syncookies = 1
net.ipv4.tcp_tw_reuse = 1
net.ipv4.tcp_tw_recycle = 1
net.ipv4.ip_local_port_range = 1024    65000
net.ipv4.tcp_max_syn_backlog = 8192
net.ipv4.tcp_max_tw_buckets = 5000
net.ipv4.route.gc_timeout = 100
net.ipv4.tcp_syn_retries = 1
net.ipv4.tcp_synack_retries = 1
说明：
net.ipv4.tcp_syncookies = 1 表示开启SYN Cookies。当出现SYN等待队列溢出时，启用cookies来处理，可防范少量SYN攻击，默认为0，表示关闭；
net.ipv4.tcp_tw_reuse = 1 表示开启重用。允许将TIME-WAIT sockets重新用于新的TCP连接，默认为0，表示关闭；
net.ipv4.tcp_tw_recycle = 1 表示开启TCP连接中TIME-WAIT sockets的快速回收，默认为0，表示关闭。
再执行以下命令，让修改结果立即生效：
引用
/sbin/sysctl -p
 */
struct os_main_option
{
	char *progname;
	char *config_file;
	char *pid_file;
	char *vty_addr;
	char *zserv_path;

	int vty_port;
	int daemon_mode;

	char *tty;
	/* process id. */
	pid_t pid;
};

struct os_main_option main_data;

/*
static char *progname;
static char config_default[] = DEFAULT_CONFIG_FILE;//SYSCONFDIR
static char *config_file = NULL;
static char *pid_file = PATH_ZEBRA_PID;
static char *vty_addr = NULL;
static char *zserv_path = NULL;

static int vty_port = ZEBRA_VTY_PORT;
static int daemon_mode = 0;
static pid_t pid;


gdbserver 1.1.1.2:50000 SWP-V0.0.1.bin

/opt/toolchain/toolchain-mipsel_24kc_gcc-7.3.0_glibc/bin/mipsel-openwrt-linux-gdb SWP-V0.0.1.bin
target remote 1.1.1.1:50000
c

*/

#ifndef BUILD_MAIN
#define BUILD_MAIN
#endif
/******************************************************/
#ifdef BUILD_MAIN
/* Command line options. */
struct option longopts[] =
{
  { "daemon",      no_argument,       NULL, 'd'},
  { "config_file", required_argument, NULL, 'f'},
  { "pid_file",    required_argument, NULL, 'i'},
  { "socket",      required_argument, NULL, 'z'},
  { "help",        no_argument,       NULL, 'h'},
  { "vty_addr",    required_argument, NULL, 'A'},
  { "vty_port",    required_argument, NULL, 'P'},
  { "tty",    required_argument, NULL, 't'},
#ifdef HAVE_NETLINK
  { "nl-bufsize",  required_argument, NULL, 's'},
#endif /* HAVE_NETLINK */
  { "user",        required_argument, NULL, 'u'},
  { "group",       required_argument, NULL, 'g'},
  { "version",     no_argument,       NULL, 'v'},
  { 0 }
};
/* Help information display. */
static void
usage (char *progname, int status)
{
  if (status != 0)
    fprintf (stderr, "Try `%s --help' for more information.\n", progname);
  else
    {
      printf ("Usage : %s [OPTION...]\n\n"\
	      "Daemon which manages kernel routing table management and "\
	      "redistribution between different routing protocols.\n\n"\
	      "-d, --daemon       Runs in daemon mode\n"\
	      "-f, --config_file  Set configuration file name\n"\
	      "-i, --pid_file     Set process identifier file name\n"\
	      "-z, --socket       Set path of zebra socket\n"\
	      "-A, --vty_addr     Set vty's bind address\n"\
	      "-P, --vty_port     Set vty's port number\n"\
	      "-t, --tty          Set tty for shell\n"\
	      "-u, --user         User to run as\n"\
	      "-g, --group	  Group to run as\n", progname);
#ifdef HAVE_NETLINK
      printf ("-s, --nl-bufsize   Set netlink receive buffer size\n");
#endif /* HAVE_NETLINK */
      printf ("-v, --version      Print program version\n"\
	      "-h, --help         Display this help and exit\n"\
	      "\n"\
	      "Report bugs to %s\n", OEM_BUG_ADDRESS);
    }

  exit (status);
}

static int main_getopt(int argc, char **argv)
{
	while (1)
	{
	      int opt;
#ifdef HAVE_NETLINK
	      opt = getopt_long (argc, argv, "df:i:z:hA:P:t:ru:g:vs", longopts, 0);
#else
	      opt = getopt_long (argc, argv, "df:i:z:hA:P:t:ru:g:vC", longopts, 0);
#endif /* HAVE_NETLINK */

	      if (opt == EOF)
	    	  break;

	      switch (opt)
	      {
	      case 0:
	    	  break;
	      case 'd':
	    	  main_data.daemon_mode = 1;
	    	  break;

	      case 'f':
	    	  //if(main_data.config_file)
	    	//	  os_free(main_data.config_file);
	    	  //main_data.config_file = os_strdup(optarg);
	    	  main_data.config_file = optarg;
	    	  break;

	      case 'A':
	    	  //if(main_data.vty_addr)
	    		//  os_free(main_data.vty_addr);
	    	  //main_data.vty_addr = os_strdup(optarg);
	    	  main_data.vty_addr = optarg;
//	    	  vty_addr = optarg;
	    	  break;
	      case 'i':
	    	  //if(main_data.pid_file)
	    		//  os_free(main_data.pid_file);
	    	  main_data.pid_file = optarg;
//	          pid_file = optarg;
	          break;
	      case 'z':
	    	  //if(main_data.zserv_path)
	    		//  os_free(main_data.zserv_path);
	    	  main_data.zserv_path = optarg;
//	    	  zserv_path = optarg;
	    	  break;
	      case 'P':
	    	  /* Deal with atoi() returning 0 on failure, and zebra not
		     listening on zebra port... */
	    	  if (strcmp(optarg, "0") == 0)
	    	  {
	    		  main_data.vty_port = 0;
	    		  break;
	    	  }
	    	  main_data.vty_port = atoi (optarg);
	    	  if (main_data.vty_port <= 0 || main_data.vty_port > 0xffff)
	    		  main_data.vty_port = ZEBRA_VTY_PORT;
	    	  break;
/*	      case 'u':
	    	  os_privs.user = optarg;
	    	  break;
	      case 'g':
	    	  os_privs.group = optarg;
	    	  break;*/
	      case 't':
	    	  if(main_data.tty)
	    		  free(main_data.tty);
	    	  //main_data.tty = strdup("/dev/ttyS0");
	    	  main_data.tty = (optarg);
	    	  break;

	      case 'v':
	    	  print_version (main_data.progname);
	    	  exit (0);
	    	  break;
	      case 'h':
	    	  usage (main_data.progname, 0);
	    	  break;
	      default:
	    	  usage (main_data.progname, 1);
	    	  break;
		}
	}
	return OK;
}
/*
static int os_privs_high()
{
	if ( os_privs.change (ZPRIVS_RAISE) )
		fprintf (stdout, "%s: could not raise privs, %s",
				   __func__,os_strerror (errno) );
	return 0;
}
static int os_privs_low()
{
	if ( os_privs.change (ZPRIVS_LOWER) )
		fprintf (stdout, "%s: could not lower privs, %s",
				   __func__,os_strerror (errno) );
	return 0;
}*/

/*static int main_timer_thread(struct thread *thread)
{
	//int sock = THREAD_FD(thread);
	//ospf_thread_check();
	//thread_add_timer(thread->master, main_timer_thread, NULL, 1);
	return 0;
}*/


/* Main startup routine. */
int main (int argc, char **argv)
{
	char *p;
	//extern int console_enable;
#ifdef OS_SIGNAL_SIGWAIT
	int signo[] = {SIGUSR2};
	sigset_t mask;
#endif
	/* Set umask before anything for security */
	umask (0027);

	os_memset(&main_data, 0, sizeof(struct os_main_option) );

	main_data.progname = NULL;
	main_data.config_file = DEFAULT_CONFIG_FILE;
	main_data.pid_file = PATH_ZEBRA_PID;
	main_data.vty_addr = NULL;
	//main_data.zserv_path = "/var/run/ProcessMU.sock";//ZEBRA_VTYSH_PATH;
	main_data.zserv_path = ZEBRA_VTYSH_PATH;
	main_data.vty_port = ZEBRA_VTY_PORT;
	main_data.daemon_mode = 0;
	main_data.pid = 0;
	main_data.tty = NULL;
	/* preserve my name */
	main_data.progname = ((p = strrchr (argv[0], '/')) ? ++p : argv[0]);

	//voip_test();

	main_getopt (argc, argv);

/*	if(os_nvram_voip_is_enable() == 0)
	{
		fprintf(stdout, "voip is disable\r\n");
		return 0;
	}*/
	os_limit_stack_size(10240);
#ifdef OS_SIGNAL_SIGWAIT
	os_task_sigmaskall();
#endif
	os_base_init();

	os_base_load();
/*	if(child_process_create() == 0)
	{
		chdir("/app");
		super_system_execvp("./TimerMgr", NULL);
	}
	if(child_process_create() == 0)
	{
		chdir("/app");
		super_system_execvp("./VmrMgr", NULL);
	}
	*/
#ifdef DOUBLE_PROCESS
/*	if(name2pid("ProcessMU") <= 0)
		super_system("cd /app;./ProcessMU -D");*/

	//if(name2pid("ProcessMU") <= 0)
	super_system("killall -9 ProcessMU");
	super_system("killall -9 ProcessMU");
	os_msleep(200);
	super_system("cd /app;./ProcessMU -D");
#endif

	//pl_pjsip_json_test();
	
	if(main_data.tty)
		console_enable = 1;
	//console_enable = 1;


	//b53125_mdio_probe();

	os_start_init(main_data.progname, ZLOG_DEFAULT, main_data.daemon_mode, main_data.tty);

	zlog_set_level (ZLOG_DEST_STDOUT, LOG_DEBUG);

	os_start_early(MODULE_DEFAULT,  NULL);

	os_start_all_module();

	//os_start_module (ZLOG_DEFAULT, main_data.config_file, NULL);

	os_start_pid(MODULE_DEFAULT, main_data.pid_file, &main_data.pid);

	/*
	 * os shell start
	 */
	os_shell_start (main_data.zserv_path, main_data.vty_addr, main_data.vty_port, main_data.tty);

	/*
	 * load config file
	 */
	os_load_config (main_data.config_file);
	zlog_notice(ZLOG_DEFAULT, "Zebra os_load_config");

	//thread_add_timer(master_thread[MODULE_DEFAULT], main_timer_thread, NULL, 1);

	//os_start_running(NULL, MODULE_DEFAULT);
#ifdef OS_SIGNAL_SIGWAIT
	os_task_sigmask(1, signo, &mask);
#endif
	while(1)
	{
#ifdef QUAGGA_SIGNAL_REAL_TIMER
		real_sigevent_process (2);
#else
		quagga_sigevent_process();
		os_msleep_interrupt(2000);
		//sleep(2);
#endif
	}
#ifdef DOUBLE_PROCESS
	os_process_stop();
#endif
	return 0;
}

#else

int os_main (int argc, char **argv)
{
	char *p;
	/* Set umask before anything for security */
	umask (0027);

	os_memset(&main_data, 0, sizeof(struct os_main_option) );

	main_data.progname = NULL;
	main_data.config_file = DEFAULT_CONFIG_FILE;
	main_data.pid_file = PATH_ZEBRA_PID;
	main_data.vty_addr = NULL;
	main_data.zserv_path = ZEBRA_VTYSH_PATH;
	main_data.vty_port = ZEBRA_VTY_PORT;
	main_data.daemon_mode = 0;
	main_data.pid = 0;


	/* preserve my name */
	main_data.progname = ((p = strrchr (argv[0], '/')) ? ++p : argv[0]);

	main_getopt (argc, argv);

	os_start_init(main_data.progname, ZLOG_NSM, main_data.daemon_mode);

	os_start_module (ZLOG_NSM, main_data.config_file, NULL);

	os_start_pid(ZLOG_NSM, main_data.pid_file, &main_data.pid);

	/*
	 * load config file
	*/
	os_load_config (main_data.config_file);

	/*
	* os shell start
	*/
	os_shell_start (main_data.zserv_path, NULL, main_data.vty_port);


	os_start_running(NULL, ZLOG_NSM);

	return 0;
}
#endif




