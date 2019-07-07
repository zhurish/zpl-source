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



int console_enable = 0;

static int telnet_task_id = 0;
static int console_task_id = 0;

static int cli_telnet_task(void *argv)
{
	module_setup_task(MODULE_TELNET, os_task_id_self());
	while(!os_load_config_done())
	{
		os_sleep(1);
	}
	eloop_start_running(NULL, MODULE_TELNET);
	return 0;
}

static int cli_telnet_task_init ()
{
	if(master_eloop[MODULE_TELNET] == NULL)
		master_eloop[MODULE_TELNET] = eloop_master_module_create(MODULE_TELNET);
	//master_thread[MODULE_TELNET] = thread_master_module_create(MODULE_TELNET);
	if(telnet_task_id == 0)
		telnet_task_id = os_task_create("telnetdTask", OS_TASK_DEFAULT_PRIORITY,
	               0, cli_telnet_task, NULL, OS_TASK_DEFAULT_STACK);
	if(telnet_task_id)
		return OK;
	return ERROR;
}

static int cli_telnet_task_exit ()
{
	if(telnet_task_id)
		os_task_destroy(telnet_task_id);
	telnet_task_id = 0;
	if(master_eloop[MODULE_TELNET])
		eloop_master_free(master_eloop[MODULE_TELNET]);
	master_eloop[MODULE_TELNET] = NULL;
	return OK;
}

static int cli_console_task(void *argv)
{
	module_setup_task(MODULE_CONSOLE, os_task_id_self());
	while(!os_load_config_done())
	{
		os_sleep(1);
	}
	os_start_running(NULL, MODULE_CONSOLE);
	return 0;
}

static int cli_console_task_init ()
{
	if(master_thread[MODULE_CONSOLE] == NULL)
		master_thread[MODULE_CONSOLE] = thread_master_module_create(MODULE_CONSOLE);

	if(console_task_id == 0)
		console_task_id = os_task_create("consoleTask", OS_TASK_DEFAULT_PRIORITY,
	               0, cli_console_task, NULL, OS_TASK_DEFAULT_STACK);
	if(console_task_id)
		return OK;
	return ERROR;
}

static int cli_console_task_exit ()
{
	if(console_task_id)
		os_task_destroy(console_task_id);
	console_task_id = 0;
	if(master_thread[MODULE_CONSOLE])
		thread_master_free(master_thread[MODULE_CONSOLE]);
	master_thread[MODULE_CONSOLE] = NULL;
	return OK;
}



int os_shell_start(char *shell_path, char *shell_addr, int shell_port, const char *tty)
{
	/* Make vty server socket. */
	vty_serv_init(shell_addr, shell_port, shell_path, tty);
	//cli_console_task_init ();
	//cli_telnet_task_init ();
	/* Print banner. */
	zlog_notice(ZLOG_DEFAULT, "Zebra %s starting: vty@%d", OEM_VERSION, shell_port);
	//fprintf(stdout,"Zebra %s starting: vty@%d\r\n", QUAGGA_VERSION, shell_port);
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

	if(master_eloop[MODULE_NSM] == NULL)
		master_eloop[MODULE_NSM] = eloop_master_module_create(MODULE_NSM);
#ifdef PL_SNTPS_MODULE
	sntpsInit(master_eloop[MODULE_NSM]);
#endif
#ifdef PL_SNTPC_MODULE
	sntpcInit(master_eloop[MODULE_NSM]);
#endif
#ifdef PL_SYSLOG_MODULE
	syslogc_lib_init(master_eloop[MODULE_NSM], NULL);
#endif

#ifdef PL_NSM_MODULE
	nsm_module_init ();
#endif

	nsm_template_init();

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

	nsm_ip_dns_init();
	//ospf_module_init();
	nsm_tunnel_client_init();
	nsm_veth_client_init();
#ifdef PL_PAL_MODULE
	extern int pal_abstract_init();
	pal_abstract_init();
#endif
	systools_module_init();

#ifdef PL_WIFI_MODULE
	nsm_iw_client_init();
#endif

#ifdef PL_MODEM_MODULE
	modem_module_init ();
#endif

#ifdef PL_DHCP_MODULE
	nsm_dhcp_module_init ();
#endif

#ifdef PL_SSH_MODULE
	ssh_module_init();
#endif

#ifdef PL_OSIP_MODULE
	voip_module_init();
#endif

#ifdef PL_PJSIP_MODULE
	pl_pjsip_module_init();
#endif

#ifdef PL_WEBGUI_MODULE
	webgui_module_init();
#endif


#ifdef PL_BSP_MODULE
	bsp_usp_module_init();
#endif
	sleep(1);
	return OK;
}



int os_module_task_init(void)
{
	//printf("%s\r\n",__func__);
	//sleep(2);
	os_msleep(500);
	if(console_enable)
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
#ifdef PL_MODEM_MODULE
	modem_task_init ();
#endif
#ifdef PL_DHCP_MODULE
	nsm_dhcp_task_init ();
#endif

	systools_task_init();

#ifdef PL_SSH_MODULE
	ssh_module_task_init();
#endif


#ifdef PL_OSIP_MODULE
	voip_module_task_init();
#endif

#ifdef PL_PJSIP_MODULE
	pl_pjsip_module_task_init();
#endif

#ifdef PL_WEBGUI_MODULE
	webgui_module_task_init();
#endif

	return OK;
}


int os_module_cmd_init(int terminal)
{
	//system cmd
	cmd_host_init(terminal);
	cmd_log_init();
	cmd_os_init();
	cmd_nvram_env_init();
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


#ifdef PL_SNTPC_MODULE
	cmd_sntpc_init();
#endif
#ifdef PL_SNTPS_MODULE
	cmd_sntps_init();
#endif
	//service_module_cmd_init();

	cmd_trunk_init();
	cmd_dos_init ();
	cmd_dot1x_init ();
	cmd_mirror_init();
	zebra_debug_init ();
	cmd_serial_init();
	cmd_dns_init();
	cmd_ppp_init();
	cmd_tunnel_init();
	cmd_bridge_init();

#ifdef PL_WIFI_MODULE
	cmd_wireless_init();
#endif

#ifdef PL_MODEM_MODULE
	cmd_modem_init ();
#endif
#ifdef PL_DHCP_MODULE
	cmd_dhcp_init();
#endif

	systools_cmd_init();
#ifdef PL_SSH_MODULE
	ssh_cmd_init();
#endif

#ifdef PL_VOIP_MODULE
	cmd_voip_init();
#endif

#ifdef PL_WEBGUI_MODULE
	//webgui_module_task_init();
#endif

#ifdef PL_APP_MODULE
	cmd_app_init();
#endif


#ifdef OS_START_TEST
	/*
	 * test module
	 */
	extern int os_test ();
	os_test ();
#endif
	return OK;
}


int os_module_exit(void)
{
	/* reverse access_list_init */
	access_list_add_hook (NULL);
	access_list_delete_hook (NULL);
	access_list_reset ();

	/* reverse prefix_list_init */
	prefix_list_add_hook (NULL);
	prefix_list_delete_hook (NULL);
	prefix_list_reset ();

	systools_module_exit();

	if_terminate() ;
#ifdef PL_SYSLOG_MODULE
//	sntpsExit();
//	sntpcExit();
	syslogc_lib_uninit();
#endif

#ifdef PL_WIFI_MODULE
	nsm_iw_client_exit();
#endif

#ifdef PL_MODEM_MODULE
	modem_module_exit ();
#endif

#ifdef PL_DHCP_MODULE
	nsm_dhcp_module_exit ();
#endif

#ifdef PL_NSM_MODULE
	nsm_module_exit ();
#endif
#ifdef PL_OSPF_MODULE
	//ospfd_module_exit();
#endif

	nsm_template_exit();

	nsm_vlan_exit();
	nsm_mac_exit();
	nsm_ip_arp_exit();
	nsm_qos_exit();
	nsm_trunk_exit();
	nsm_security_exit();
	nsm_dos_exit();
	nsm_dot1x_exit();
	nsm_mirror_exit();
	nsm_serial_client_exit();

	nsm_ip_dns_exit();
	//ospf_module_init();
	nsm_tunnel_client_exit();
	nsm_veth_client_exit();
	//pal_abstract_exit();
#ifdef PL_SSH_MODULE
	ssh_module_exit();
#endif

#ifdef PL_OSIP_MODULE
	voip_module_exit();
#endif

#ifdef PL_PJSIP_MODULE
	pl_pjsip_module_exit();
#endif

#ifdef PL_WEBGUI_MODULE
	webgui_module_exit();
#endif

	return OK;
}

int os_module_task_exit(void)
{
#ifdef DOUBLE_PROCESS
	os_process_stop();
#endif

#ifdef PL_OSPF_MODULE
	//ospfd_task_init ();
#endif
#ifdef PL_MODEM_MODULE
	//modem_task_exit ();
#endif
#ifdef PL_DHCP_MODULE
	//nsm_dhcp_task_exit ();
#endif

#ifdef PL_NSM_MODULE
	//nsm_task_exit ();
#endif

#ifdef PL_WEBGUI_MODULE
	webgui_module_task_exit();
#endif

	systools_task_exit();
	os_time_exit();
	os_job_exit();
	if(console_enable)
		cli_console_task_exit ();
	cli_telnet_task_exit ();
#ifdef PL_SSH_MODULE
	ssh_module_task_exit();
#endif

#ifdef PL_OSIP_MODULE
	voip_module_task_exit();
#endif

#ifdef PL_PJSIP_MODULE
	pl_pjsip_module_task_exit();
#endif


	os_task_exit();
	return OK;
}

int os_module_cmd_exit(void)
{
	vrf_terminate();
	vty_terminate ();
	cmd_terminate ();

	if (zlog_default)
		closezlog (zlog_default);
	return OK;
}
