/*
 * module_tbl.c
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
#include "template.h"
#include "if.h"
#include "nsm_vrf.h"
#ifdef PL_NSM_MODULE
#include "nsm_main.h"
#include "nsm_router-id.h"
#include "nsm_vrf.h"
#include "nsm_interface.h"
#include "nsm_debug.h"
#ifdef PL_NSM_PPP
#include "nsm_ppp.h"
#endif
#ifdef PL_NSM_MAC
#include "nsm_mac.h"
#endif
#ifdef PL_NSM_ARP
#include "nsm_arp.h"
#endif
#ifdef PL_NSM_DNS
#include "nsm_dns.h"
#endif
#ifdef PL_NSM_DOS
#include "nsm_dos.h"
#endif
#ifdef PL_NSM_8021X
#include "nsm_8021x.h"
#endif

#ifdef PL_NSM_VLAN
#include "nsm_vlan.h"
#endif
#ifdef PL_NSM_QOS
#include "nsm_qos.h"
#endif
#ifdef PL_NSM_TRUNK
#include "nsm_trunk.h"
#endif
#ifdef PL_NSM_MIRROR
#include "nsm_mirror.h"
#endif
#ifdef PL_NSM_TUNNEL
#include "nsm_tunnel.h"
#endif
#ifdef PL_NSM_SERIAL
#include "nsm_serial.h"
#endif

#ifdef PL_NSM_SECURITY
#include "nsm_security.h"
#endif
#ifdef PL_NSM_VETH
#include "nsm_veth.h"
#endif

#ifdef PL_NSM_BRIDGE
#include "nsm_bridge.h"
#endif

#endif

#ifdef PL_PAL_MODULE
#include "pal_driver.h"
#endif
#ifdef PL_SERVICE_MODULE
#include "systools.h"
#endif
#ifdef PL_SERVICE_SNTPC
#include "sntpcLib.h"
#endif
#ifdef PL_SERVICE_SNTPS
#include "sntpsLib.h"
#endif
#ifdef PL_PJSIP_MODULE
#include "pjsip_app_api.h"
#endif
#ifdef PL_WIFI_MODULE
#include "iw_interface.h"
#endif
#ifdef PL_MQTT_MODULE
#include "mqtt_app_conf.h"
#include "mqtt_app_api.h"
#endif
#ifdef PL_DHCP_MODULE
#include "nsm_dhcp.h"
#endif
#ifdef PL_LIBSSH_MODULE
#include "ssh_api.h"
#endif
#ifdef PL_HAL_MODULE
#include "hal_driver.h"
#endif
#ifdef PL_MODEM_MODULE
#include "modem.h"
#endif
#ifdef PL_WEBGUI_MODULE
#include "web_api.h"
#endif
#ifdef PL_BSP_MODULE
#include "if_manage.h"
#endif
#ifdef PL_APP_MODULE
#include "application.h"
#endif
#include "os_start.h"
#include "module_tbl.h"

/*
static const char *module_name[] =
	{
		"DEFAULT",
		"PJSIP",
		"LIB",
		"OSAL",
		"TIMER",
		"JOB",
		"CONSOLE",
		"TELNET",
		"UTILS",
		"IMISH",
		"KERNEL",
		"NSMDEBUG",
		"NSMDOT1X",
		"NSMARP",
		"NSMBRIDGE",
		"NSMDHCP",
		"NSM",
		"SNTP",
		"SNTPS",
		"PAL",
		"HAL",
		"MODEM",
		"DHCP",
		"WIFI",
		"MQTT",
		"WEB",
		"APP",
		NULL,
};
*/
int pl_def_module_init()
{
#ifdef PL_NSM_MODULE
	nsm_module_init();
#endif

#ifdef PL_SERVICE_MODULE
	systools_module_init();
#endif

#ifdef PL_WIFI_MODULE
	nsm_iw_client_init();
#endif
#ifdef PL_MODEM_MODULE
	modem_module_init();
#endif
#ifdef PL_DHCP_MODULE
	nsm_dhcp_module_init();
#endif
#ifdef PL_LIBSSH_MODULE
	ssh_module_init();
#endif

#ifdef PL_OSPF_MODULE
	ospfd_module_init();
#endif
#ifdef PL_PJSIP_MODULE
	pl_pjsip_module_init();
#endif
#ifdef PL_WEBGUI_MODULE
	web_app_module_init();
#endif
#ifdef PL_MQTT_MODULE
	mqtt_module_init();
#endif

#ifdef PL_APP_MODULE
	app_module_init();
#endif
#ifdef PL_BSP_MODULE
	bsp_usp_module_init();
#endif
	return OK;
}

int pl_def_module_exit()
{
#ifdef PL_APP_MODULE
	app_module_exit();
#endif

#ifdef PL_SERVICE_MODULE
	systools_module_exit();
#endif
#ifdef PL_WIFI_MODULE
	nsm_iw_client_exit();
#endif
#ifdef PL_MODEM_MODULE
	modem_module_exit();
#endif
#ifdef PL_DHCP_MODULE
	nsm_dhcp_module_exit();
#endif
#ifdef PL_LIBSSH_MODULE
	ssh_module_exit();
#endif

#ifdef PL_OSPF_MODULE
	ospfd_module_exit();
#endif
#ifdef PL_PJSIP_MODULE
	pl_pjsip_module_exit();
#endif
#ifdef PL_WEBGUI_MODULE
	web_app_module_exit();
#endif
#ifdef PL_MQTT_MODULE
	mqtt_module_exit();
#endif
#ifdef PL_SERVICE_SYSLOG
	syslogc_lib_uninit();
#endif
#ifdef PL_NSM_MODULE
	nsm_module_exit();
#endif

	nsm_template_exit();

	return OK;
}

int pl_def_module_task_init()
{
#ifdef PL_NSM_MODULE
	nsm_task_init();
#endif
#ifdef PL_SERVICE_MODULE
	systools_task_init();
#endif
#ifdef PL_MODEM_MODULE
	modem_task_init();
#endif
#ifdef PL_DHCP_MODULE
	nsm_dhcp_task_init();
#endif
#ifdef PL_LIBSSH_MODULE
	ssh_module_task_init();
#endif

#ifdef PL_PJSIP_MODULE
	pl_pjsip_module_task_init();
#endif
#ifdef PL_WEBGUI_MODULE
	web_app_module_task_init();
#endif
#ifdef PL_MQTT_MODULE
	mqtt_module_task_init();
#endif

#ifdef PL_APP_MODULE
	app_module_task_init();
#endif
	return OK;
}

int pl_def_module_task_exit()
{
#ifdef PL_LIBSSH_MODULE
	ssh_module_task_exit();
#endif
#ifdef PL_PJSIP_MODULE
	pl_pjsip_module_task_exit();
#endif
#ifdef PL_WEBGUI_MODULE
	web_app_module_task_exit();
#endif
#ifdef PL_MQTT_MODULE
	mqtt_module_task_exit();
#endif
#ifdef PL_APP_MODULE
	app_module_task_exit();
#endif
#ifdef PL_SERVICE_MODULE
	systools_task_exit();
#endif
	return OK;
}

int pl_def_module_cmd_init()
{
#ifdef PL_NSM_MODULE
	nsm_module_cmd_init();
#endif
	cmd_debug_init();

#ifdef PL_SERVICE_SNTPC
	cmd_sntpc_init();
#endif
#ifdef PL_SERVICE_SNTPS
	cmd_sntps_init();
#endif

#ifdef PL_WIFI_MODULE
	cmd_wireless_init();
#endif
#ifdef PL_MODEM_MODULE
	cmd_modem_init();
#endif
#ifdef PL_DHCP_MODULE
	cmd_dhcp_init();
#endif
#ifdef PL_LIBSSH_MODULE
	ssh_cmd_init();
#endif

#ifdef PL_OSPF_MODULE
#endif
#ifdef PL_PJSIP_MODULE
	cmd_voip_init();
#endif
#ifdef PL_WEBGUI_MODULE
	cmd_webserver_init();
#endif
#ifdef PL_MQTT_MODULE
	cmd_mqtt_init();
#endif
#ifdef PL_SERVICE_MODULE
	systools_cmd_init();
#endif
#ifdef PL_APP_MODULE
	cmd_app_init();
#endif
	return OK;
}
