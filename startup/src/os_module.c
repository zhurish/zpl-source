/*
 * os_module.c
 *
 *  Created on: Jun 10, 2017
 *      Author: zhurish
 */
#include "zebra.h"
#include "memory.h"
#include "command.h"
#include "memory.h"
#include "prefix.h"
#include "sigevent.h"
#include "version.h"
#include <log.h>
#include "getopt.h"
#include "eloop.h"
#include "os_start.h"
#include "os_module.h"





static int telnet_task_id = 0;
static int console_task_id = 0;

static int cli_telnet_task(void *argv)
{
	module_setup_task(MODULE_TELNET, os_task_id_self());
	while(!os_load_config_done())
	{
		sleep(1);
	}
	eloop_start_running(NULL, MODULE_TELNET);
	return 0;
}

static int cli_telnet_task_init ()
{
	if(master_eloop[MODULE_TELNET] == NULL)
		master_eloop[MODULE_TELNET] = eloop_master_module_create(MODULE_TELNET);
	//master_thread[MODULE_TELNET] = thread_master_module_create(MODULE_TELNET);
	telnet_task_id = os_task_create("telnetTask", OS_TASK_DEFAULT_PRIORITY,
	               0, cli_telnet_task, NULL, OS_TASK_DEFAULT_STACK);
	if(telnet_task_id)
		return OK;
	return ERROR;
}

static int cli_console_task(void *argv)
{
	module_setup_task(MODULE_CONSOLE, os_task_id_self());
	while(!os_load_config_done())
	{
		sleep(1);
	}
	os_start_running(NULL, MODULE_CONSOLE);
	return 0;
}

static int cli_console_task_init ()
{
	if(master_thread[MODULE_CONSOLE] == NULL)
		master_thread[MODULE_CONSOLE] = thread_master_module_create(MODULE_CONSOLE);

	console_task_id = os_task_create("consoleTask", OS_TASK_DEFAULT_PRIORITY,
	               0, cli_console_task, NULL, OS_TASK_DEFAULT_STACK);
	if(console_task_id)
		return OK;
	return ERROR;
}


int os_shell_start(char *shell_path, char *shell_addr, int shell_port, const char *tty)
{
	/* Make vty server socket. */
	vty_serv_init(shell_addr, shell_port, shell_path, tty);
	//cli_console_task_init ();
	//cli_telnet_task_init ();
	/* Print banner. */
	//zlog_notice("Zebra %s starting: vty@%d", QUAGGA_VERSION, shell_port);
	fprintf(stdout,"Zebra %s starting: vty@%d\r\n", QUAGGA_VERSION, shell_port);
	return OK;
}

static int os_default_start()
{
	/* Make master thread emulator. */
	if(master_eloop[MODULE_TELNET] == NULL)
		master_eloop[MODULE_TELNET] = eloop_master_module_create(MODULE_TELNET);

	if(master_thread[MODULE_CONSOLE] == NULL)
		master_thread[MODULE_CONSOLE] = thread_master_module_create(MODULE_CONSOLE);

	cmd_init(1);
	vty_init(master_thread[MODULE_CONSOLE], master_eloop[MODULE_TELNET]);
	vty_user_init();
	memory_init();
	return OK;
}

int os_ip_stack_init(int localport)
{
#ifndef USE_IPSTACK_KERNEL
	extern int ipcom_demo_init(int localport);
	extern void sys_signal_init(void);

	ipcom_demo_init(localport);
#endif
	return OK;
}


int os_module_init(void)
{
	os_default_start();

#ifdef PL_SERVICE_MODULE
	if(master_eloop[MODULE_NSM] == NULL)
		master_eloop[MODULE_NSM] = eloop_master_module_create(MODULE_NSM);
	sntpsInit(master_eloop[MODULE_NSM]);
	sntpcInit(master_eloop[MODULE_NSM]);
	syslogc_lib_init(master_eloop[MODULE_NSM], NULL);
#endif
#ifdef PL_NSM_MODULE
	nsm_module_init ();
#endif
#ifdef PL_OSPF_MODULE
	//ospfd_module_init();
#endif
#ifdef PL_PAL_MODULE
	//pal_api_init();
#endif

	nsm_vlan_init();
	nsm_mac_init();
	nsm_ip_arp_init();
	nsm_qos_init();
	nsm_trunk_init();
	nsm_security_init();
	nsm_dos_init();
	nsm_dot1x_init();
	nsm_mirror_init();
	nsm_serial_client_init();
	//ospf_module_init();

	extern int pal_ip_stack_init();
	pal_ip_stack_init();
	//sleep(3);
#ifdef PL_BSP_MODULE
	bsp_usp_module_init();
#endif
	modem_module_init ();

	dhcpc_module_init ();
	sleep(1);
	return OK;
}

int os_module_task_init(void)
{
	//printf("%s\r\n",__func__);
	sleep(2);

	cli_console_task_init ();
	cli_telnet_task_init ();

#ifdef PL_NSM_MODULE
	nsm_task_init ();
#endif

	os_msleep(50);

	//ospf_task_init ();
#ifdef PL_OSPF_MODULE
	//ospfd_task_init ();
#endif

	modem_task_init ();
	dhcpc_task_init ();
	return OK;
}


int os_module_cmd_init(int terminal)
{
	//system cmd
	cmd_host_init(terminal);
	cmd_log_init();
	cmd_os_init();
	cmd_os_thread_init();
	cmd_os_eloop_init();
	cmd_memory_init();
	cmd_vty_init();
	cmd_vty_user_init();
	cmd_router_id_init();

	cmd_interface_init();
#ifdef USE_IPSTACK_IPCOM
	cmd_ip_vrf_init ();
#endif
	cmd_route_init ();
	cmd_mac_init();
	cmd_arp_init();
	cmd_vlan_init();
	cmd_port_init();
#ifdef PL_SERVICE_MODULE
	cmd_sntpc_init();
	cmd_sntps_init();
	//service_module_cmd_init();
#endif
	cmd_trunk_init();
	cmd_dos_init ();
	cmd_dot1x_init ();
	cmd_mirror_init();
	zebra_debug_init ();

	cmd_modem_init ();
#ifdef OS_START_TEST
	/*
	 * test module
	 */
	extern int os_test ();
	os_test ();
#endif
	return OK;
}


