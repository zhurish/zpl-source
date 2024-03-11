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


/*
 handle SIGUSR2 nostop noprint
   r -s /home/zhurish/workspace/working/zpl-source/netservice1.txt
     r -s /home/zhurish/workspace/working/zpl-source/netservice2.txt
*/
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


extern const char *nl_strerror_l(int err);
extern void _nl_socket_used_ports_release_all(const uint32_t *used_ports);
extern int ip_main(int argc, char **argv);
extern int ftp_download(void *v, char *hostName, int port, char *path, char *fileName, char *usr,
                 char *passwd, char *localfileName);
extern int ortp_create_init(void);
extern int testmain(int argc, char **argv);
extern int testmain111(int argc, char **argv);
extern int get_frame_h264_test(void);
int zpl_media_sps_test(void);
extern int ms_factory_default_init(void);
extern int main_test(int argc, char * argv[]);
extern int main_stest(int argc, char *argv[]);
extern int main_test1(int argc, char *argv[]);
extern void pl_voip_init();
extern int pjmain(int argc, char *argv[]);
extern int jrtplib_api_test(void);
extern int zpl_media_text_bitmap_text(void);
/* Main startup routine. */
int main(int argc, char **argv)
{
	char *p = NULL;
	int intval = 0;
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

	//"/home/zhurish/workspace/working/zpl-source/os_netservice.txt"
	zpl_stack_init(startup_option.service_file);


	zpl_stack_start(startup_option.progname, 8890);
	//pl_voip_init();
	//pjmain(0, NULL);
	//main_test(0, NULL);
	//main_test1(0, NULL);
	//main_stest(0, NULL);

#if 0
	rtsp_client_t * saa = rtsp_client_create("test", "rtsp://34.227.104.115/vod/mp4");
	if(saa)
	{
		if(rtsp_client_connect(saa, 15) == 0)
		{
			if(rtsp_client_open(saa) == 0)
				;//rtsp_client_task_init(saa);
		}
	}

	//zpl_media_channel_t * chan = zpl_media_channel_filecreate("/home/zhurish/workspace/working/zpl-source/source/multimedia/media/out.h264", 1);
#endif
	//zpl_media_text_bitmap_text();
	//exit(0);
	//zpl_media_sps_test();
	//jrtplib_api_test();
	//get_frame_h264_test();
	//return 0;
	startup_module_init(1);

	startup_module_load();

	startup_module_waitting();

	zpl_base_start_pid(MODULE_LIB, os_netservice_sockpath_get(PL_PID), NULL);

	/*
	 * os shell start
	 */
	zpl_base_shell_start(os_netservice_sockpath_get(SHELL_SOCKET_PATH), 
		startup_option.vty_addr, os_netservice_port_get("vty_port"), startup_option.tty);

	/*
	 * load config file
	 */
	openzlog_start(NULL);
	host_config_loading(startup_option.config_file);

	zlog_notice(MODULE_LIB, "Zebra host_config_loading");
	os_task_sigexecute(0, NULL);
	os_test();
    //testmain(0, NULL);

    // ortp_create_init();
    //  os_url_test();
    //  ftp_download(NULL, "127.0.0.1", 0, NULL, "fsdd.pdf", "zhurish", "centos", "aa.pdf");

    while (1)
	{
		//os_ansync_empty_running(NULL, NULL, 1000);
		os_signal_process(200);
		intval++;
		if(intval == 100)
		{
		//task_mutex_graph_show();
		intval = 0;
		}
	}
#ifdef ZPL_TOOLS_PROCESS
	os_process_stop();
#endif
	return 0;
}

