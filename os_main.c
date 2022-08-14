/*
 * platform_main.c
 *
 *  Created on: Apr 30, 2017
 *      Author: zhurish
 */


#include "zplos_include.h"
#include "getopt.h"
#include <log.h>

#include "command.h"
#include "zmemory.h"
#include "prefix.h"
//#include "sigevent.h"
#include "thread.h"
#include "host.h"
#include "module.h"
#ifdef ZPL_HAL_MODULE
#include "hal_driver.h"
#endif
#if defined(ZPL_SDK_MODULE) && defined(ZPL_SDK_USER)
//#include "sdk_driver.h"
#endif

#include "startup_start.h"
#include "startup_module.h"
//#include "platform/nsm/filter.h"
//#include "platform/nsm/plist.h"

// extern struct nsm_privs_t os_privs;
// extern struct quagga_signal_t os_signals[];
// handle SIGUSR2 nostop noprint
/*
sudo ip link add link enp0s25 name enp0s25.200 type vlan id 200
sudo ip link set enp0s25.200 up
sudo ip address add 1.1.1.1/24 dev enp0s25.200
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

/*
gdbserver 1.1.1.2:50000 SWP-V0.0.1.bin

/opt/toolchain/toolchain-mipsel_24kc_gcc-7.3.0_glibc/bin/mipsel-openwrt-linux-gdb SWP-V0.0.1.bin
target remote 1.1.1.1:50000
c

*/

#ifndef BUILD_MAIN
#define BUILD_MAIN
#endif
/******************************************************/

/* Command line options. */
static struct option zpllongopts[] =
	{
		// df:i:z:hA:P:t:u:g:v
		{"daemon", no_argument, NULL, 'd'},
		{"config_file", required_argument, NULL, 'f'},
		{"pid_file", required_argument, NULL, 'i'},
		{"socket", required_argument, NULL, 'z'},
		{"help", no_argument, NULL, 'h'},
		{"vty_addr", required_argument, NULL, 'A'},
		{"vty_port", required_argument, NULL, 'P'},
		{"tty", required_argument, NULL, 't'},
		{"user", required_argument, NULL, 'u'},
		{"group", required_argument, NULL, 'g'},
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
			   "-i, --pid_file     Set process identifier file name\n"
			   "-z, --socket       Set path of zebra socket\n"
			   "-A, --vty_addr     Set vty's bind address\n"
			   "-P, --vty_port     Set vty's port number\n"
			   "-t, --tty          Set tty Device Name for shell\n"
			   "-u, --user         User to run as\n"
			   "-g, --group	  Group to run as\n"
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

static int zplmain_getopt(int argc, char **argv)
{
	while (1)
	{
		int opt = getopt_long(argc, argv, "df:i:z:hA:P:t:u:g:nv", zpllongopts, NULL);
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

		case 'f':
			// if(startup_option.config_file)
			//	  os_free(startup_option.config_file);
			// startup_option.config_file = os_strdup(optarg);
			startup_option.config_file = optarg;
			break;

		case 'A':
			// if(startup_option.vty_addr)
			//   os_free(startup_option.vty_addr);
			// startup_option.vty_addr = os_strdup(optarg);
			startup_option.vty_addr = optarg;
			//	    	  vty_addr = optarg;
			break;
		case 'i':
			// if(startup_option.pid_file)
			//   os_free(startup_option.pid_file);
			startup_option.pid_file = optarg;
			//	          pid_file = optarg;
			break;
		case 'z':
			// if(startup_option.zserv_path)
			//   os_free(startup_option.zserv_path);
			startup_option.zserv_path = optarg;
			//	    	  zserv_path = optarg;
			break;
		case 'P':
			/* Deal with atoi() returning 0 on failure, and zebra not
		   listening on zebra port... */
			if (strcmp(optarg, "0") == 0)
			{
				startup_option.vty_port = 0;
				break;
			}
			startup_option.vty_port = atoi(optarg);
			if (startup_option.vty_port <= 0 || startup_option.vty_port > 0xffff)
				startup_option.vty_port = PLCLI_VTY_PORT;
			break;
		case 'u':
			startup_option.user = optarg;
			break;
		case 'g':
			startup_option.group = optarg;
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

extern const char *nl_strerror_l(int err);
extern void _nl_socket_used_ports_release_all(const uint32_t *used_ports);
extern int ip_main(int argc, char **argv);
/* Main startup routine. */
int main(int argc, char **argv)
{
	char *p = NULL;

	/* Set umask before anything for security */
	umask(0027);

	//ip_main( argc, argv);
	//tc_main( argc, argv);
	//nl_strerror_l(0);
	//_nl_socket_used_ports_release_all(NULL);
	startup_option_default();
	startup_option.progname = ((p = strrchr(argv[0], '/')) ? ++p : argv[0]);

	zplmain_getopt(argc, argv);

	os_signal_default(zlog_signal, zlog_signal);
	zpl_base_signal_init(startup_option.daemon_mode);

	zpl_stack_init();
	zpl_stack_start(startup_option.progname, 8890);
	
	startup_module_init(1);

	startup_module_load();

	startup_module_waitting();


	zpl_base_start_pid(MODULE_DEFAULT, startup_option.pid_file, &startup_option.pid);

	/*
	 * os shell start
	 */
	zpl_base_shell_start(startup_option.zserv_path, startup_option.vty_addr, startup_option.vty_port, startup_option.tty);

	/*
	 * load config file
	 */
	openzlog_start(NULL);
	host_config_loading(startup_option.config_file);

	zlog_notice(MODULE_DEFAULT, "Zebra host_config_loading");
	os_task_sigexecute(0, NULL);
	while (1)
	{
		//os_ansync_empty_running(NULL, NULL, 1000);
		os_signal_process(200);
	}
#ifdef ZPL_TOOLS_PROCESS
	os_process_stop();
#endif
	return 0;
}

