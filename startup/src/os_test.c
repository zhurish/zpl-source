/*
 * os_test.c
 *
 *  Created on: May 27, 2017
 *      Author: zhurish
 */
#include "zebra.h"
#include "getopt.h"
#include <log.h>
#include "if_name.h"
#include "zclient.h"
#include "command.h"
#include "if.h"
#include "memory.h"
#include "prefix.h"
#include "sigevent.h"
#include "sockunion.h"
//#include "version.h"
#include "vrf.h"
#include "filter.h"
#include "plist.h"
#include "vty.h"
#include "nsm_vlan.h"

#include "os_start.h"
#include "os_module.h"
#include "os_util.h"


DEFUN (i_iusp_test,
		i_iusp_test_cmd,
       "interface <unit/slot/port>",
       "Select an interface to configure\n"
       "specify interface name:<unit/slot/port> (e.g. 0/1/3)\n")
{
	vty_out(vty,"argv:%s%s",argv[0],VTY_NEWLINE);
	return CMD_SUCCESS;
}

DEFUN (i_mac_test,
		i_mac_test_cmd,
       "mac-table " CMD_MAC_STR,
       "Select an interface to configure\n"
	   CMD_MAC_STR_HELP)
{
	vty_out(vty,"argv:%s%s",argv[0],VTY_NEWLINE);
	return CMD_SUCCESS;
}

DEFUN (syslog_debug_test,
		syslog_debug_test_cmd,
       "syslog-debug .LINE",
       "syslog-debug\n"
	   "dest")
{
	char *message;
	extern int syslogc_out(int priority, char * pStr, int len);
	message = argv_concat(argv, argc, 0);
	syslogc_out(LOG_DEBUG, message, os_strlen(message));
	return CMD_SUCCESS;
}

DEFUN (sdk_test,
		sdk_test_cmd,
       "sdk-speed <0-5> (10|100|1000)",
       "syslog-debug\n"
	   "dest")
{
	ifindex_t ifindex = atoi(argv[0]);
	int value = atoi(argv[0]);
	hal_port_speed_set( ifindex,  value);
	//hal_port_duplex_set(ifindex_t ifindex, int value);
	return CMD_SUCCESS;
}

DEFUN (wifi_list,
		wifi_list_cmd,
       "wifi-list",
       "syslog-debug\n"
	   "dest")
{
	//extern int wifi_show(struct vty *vty);
	//extern int wifi_show_interface(struct interface *ifp, struct vty *vty);
	//wifi_show_interface(NULL, vty);
	//wifi_show(vty);
	//wifi_connect_show(vty);
	return CMD_SUCCESS;
}

DEFUN (wifi_scan,
		wifi_scan_cmd,
       "wifi-scan",
       "syslog-debug\n"
	   "dest")
{
	//extern int wifi_show(struct vty *vty);
	//extern int wifi_show_interface(struct interface *ifp, struct vty *vty);
	//wifi_show_interface(NULL, vty);
	//wifi_show(vty);
	//wifi_scan_ap(vty);
	//iw_client_scan_test(vty);
	return CMD_SUCCESS;
}


DEFUN (bond_test,
		bond_test_cmd,
       "bond-test",
       "syslog-debug\n"
	   "dest")
{
	_if_bond_test();
	return CMD_SUCCESS;
}

#ifdef PL_DHCP_MODULE
DEFUN (dhcp_test,
		dhcp_test_cmd,
       "dhcp-test",
       "syslog-debug\n"
	   "dest")
{
	dhcpc_enable_test();
	return CMD_SUCCESS;
}
#endif

DEFUN (os_process_test,
		os_process_test_cmd,
       "process-test (start|stop)",
       "syslog-debug\n"
	   "dest")
{
	static int taskid = 0;
	if(memcmp(argv[0], "start", 3) == 0)
	{
		char *argve[] = {"call", "file", "/etc/ppp/peers/dial-auto", NULL};
		taskid = os_process_register(PROCESS_START, "pppd", "pppd", TRUE, argve);
		if(taskid)
		{
			vty_out(vty, "pppd task start OK(%d).%s", taskid, VTY_NEWLINE);
		}
		else
			vty_out(vty, "pppd task start faile.%s", VTY_NEWLINE);
	}
	if(memcmp(argv[0], "stop", 3) == 0)
	{
		if(taskid)
		{
			os_process_action(PROCESS_STOP, "pppd", taskid);
			taskid = 0;
		}
		else
			vty_out(vty, "pppd task is not exist.%s", VTY_NEWLINE);
	}
	return CMD_SUCCESS;
}

#ifdef DOUBLE_PROCESS
DEFUN (process_test,
		process_test_cmd,
       "process_test <1-10>",
       "syslog-debug\n"
	   "dest")
{
	int id = atoi(argv[0]);
	int res = 0;
	char *inargv[] = {"-f", "-a", "log", "-d", NULL};
	char *inargv1[] = {"-f", "-a", "log", "-d", "aaaa", "-D", NULL};
	switch(id)
	{
	case 1:
		res = os_process_register(PROCESS_START, "dhcpc-eth0",
					"dhcpc", FALSE, inargv);
		break;
	case 2:
		res = os_process_register(PROCESS_STOP, "dhcpc-eth0",
					"dhcpc", FALSE, inargv);
		break;
	case 3:
		res = os_process_register(PROCESS_RESTART, "dhcpc-eth0",
					"dhcpc", FALSE, inargv);
		break;

	case 4:
		res = os_process_register(PROCESS_START, "dhcpc-eth1",
					"dhcpc1", TRUE, inargv1);
		break;
	case 5:
		res = os_process_register(PROCESS_STOP, "dhcpc-eth2",
					"dhcpc1", TRUE, inargv1);
		break;
	case 6:
		res = os_process_register(PROCESS_RESTART, "dhcpc-eth2",
					"dhcpc1", TRUE, inargv1);
		break;
	}
	if((res == ERROR))
		vty_out(vty,"res:ERROR(%s)(%d)%s",strerror(errno),sizeof(process_head),VTY_NEWLINE);
	else
		vty_out(vty,"res:%d(%s)(%d)%s",res,strerror(errno),sizeof(process_head),VTY_NEWLINE);
	return CMD_SUCCESS;
}
#endif


#ifdef OS_TIMER_TEST

DEFUN (timet_test,
		timet_test_cmd,
       "timer-test <1-6000000000>",
       "timer-test\n"
	   "value msec\n")
{
	if(argc == 2)
		timer_test(atoi(argv[0]), 1);
	else
		timer_test(atoi(argv[0]), 0);
	return CMD_SUCCESS;
}


ALIAS(timet_test,
		timet_test_once_cmd,
		"timer-test <1-6000000000> (once|)",
		"timer-test\n"
		"value msec\n");

DEFUN (timet_test_exit,
		timet_test_exit_cmd,
       "timer-test-exit",
       "timer-test\n")
{
	if(argc == 1)
		timer_test_exit(1);
	else
		timer_test_exit(0);
	return CMD_SUCCESS;
}

ALIAS(timet_test_exit,
		timet_test_exit_once_cmd,
		"timer-test (once|)",
		"timer-test\n"
		"once msec\n");
#endif

#ifdef PL_OPENWRT_UCI
DEFUN (uci_get_test_cmd,
		uci_get_test_cmd_cmd,
       "uci-test (get-string|get-integer) NAME",
       "uco-test\n")
{
	char value[64];
	int valu = 0;
	char *lstv[8];
	if(strstr(argv[0], "string"))
		os_uci_get_string(argv[1], value);
	else
		os_uci_get_list(argv[1], lstv, &valu);
	if(strstr(argv[0], "string"))
		vty_out(vty, "get %s %s", argv[1], value);
	else
	{
		int i = 0;
		for(i = 0; i < valu; i++)
			vty_out(vty, "get %s:%s", argv[1], lstv[i]);
	}
	return CMD_SUCCESS;
}

DEFUN (uci_set_test_cmd,
		uci_set_test_cmd_cmd,
       "uci-test (set-string|set-integer) NAME VALUE",
       "uco-test\n")
{
	if(strstr(argv[0], "string"))
		os_uci_set_string(argv[1], argv[2]);
	else
		os_uci_set_integer(argv[1], atoi(argv[2]));
	return CMD_SUCCESS;
}

DEFUN (uci_list_test_cmd,
		uci_list_test_cmd_cmd,
       "uci-test (add-list|del-list) NAME VALUE",
       "uco-test\n")
{
	if(strstr(argv[0], "add"))
		os_uci_list_add(argv[1], argv[2]);
	else
		os_uci_list_del(argv[1], (argv[2]));
	return CMD_SUCCESS;
}

DEFUN (uci_section_test_cmd,
		uci_section_test_cmd_cmd,
       "uci-test section NAME VALUE",
       "uco-test\n")
{
	os_uci_section_add(argv[1], argv[2]);
	return CMD_SUCCESS;
}

DEFUN (uci_del_test_cmd,
		uci_del_test_cmd_cmd,
       "uci-test del NAME OP VALUE",
       "uco-test\n")
{
	if(argc == 3)
			os_uci_del(argv[0], argv[1], argv[2]);
	else if(argc == 2)
			os_uci_del(argv[0], argv[1], NULL);
	else if(argc == 1)
			os_uci_del(argv[0], NULL, NULL);
	return CMD_SUCCESS;
}

ALIAS(uci_del_test_cmd,
		uci_del_test_vv_cmd,
       "uci-test del NAME OP",
       "uco-test\n");


DEFUN (uci_commit_test_cmd,
		uci_commit_test_cmd_cmd,
       "uci-test del NAME OP VALUE",
       "uco-test\n")
{
	os_uci_commit(argv[0]);
	return CMD_SUCCESS;
}
#endif



int os_test()
{
	//modem_pppd_test();
/*	 int num = 0;
	 vlan_t value[100];
	 int base = 0, end = 0;
	 memset(value, 0, sizeof(value));
	 num = vlan_listvalue_explain("2,4,5,6,7-13", value, 100, &base, &end);

	 fprintf(stderr,"input:%d-%d\r\n", base, end);*/
	// vlan_listvalue_format(value,  num, NULL);
/*
	unsigned char mac[7];
	int unit, slot, port, vlan;
	vty_iusp_format ("1/2", &unit, &slot, &port, &vlan);
	vty_iusp_format ("3/4.3", &unit, &slot, &port, &vlan);
	vty_iusp_format ("1/5/2", &unit, &slot, &port, &vlan);
	vty_iusp_format ("2/2/6.3", &unit, &slot, &port, &vlan);
	vty_mac_get ("0001-2223-3132", mac);
	vty_mac_get ("0a01-2DFB-3ef2", mac);
*/
/*	char str[128];
	struct prefix p4;
	p4.family = AF_INET;
	p4.prefixlen = 24;
	p4.u.prefix4.s_addr = inet_addr("192.1.1.1");
	printf("%s:%s\r\n",__func__,prefix2str (&p4, str,  sizeof(str)));*/

	/* Zebra related initialize. */
/*	int nsm_main_task(void *argv);
	nsm_main_task(NULL);*/
	//extern int nsm_main (int argc, char **argv);
	//nsm_main (0, NULL);
//	unit_slot_module_init();

	//dhcpc_module_init();

	//extern int syslogcLibInit (char * pServer);
	//syslogcLibInit ("127.0.0.1");
	//modem_test_init();
	install_element (ENABLE_NODE, &wifi_list_cmd);
	install_element (ENABLE_NODE, &wifi_scan_cmd);
#ifdef PL_DHCP_MODULE
	install_element (ENABLE_NODE, &dhcp_test_cmd);
#endif
	install_element (ENABLE_NODE, &sdk_test_cmd);
	install_element (ENABLE_NODE, &syslog_debug_test_cmd);
	install_element (ENABLE_NODE, &bond_test_cmd);
	install_element (ENABLE_NODE, &os_process_test_cmd);

#ifdef DOUBLE_PROCESS
	install_element (ENABLE_NODE, &process_test_cmd);
#endif
#ifdef OS_TIMER_TEST
	install_element (ENABLE_NODE, &timet_test_cmd);
	install_element (ENABLE_NODE, &timet_test_once_cmd);
	install_element (ENABLE_NODE, &timet_test_exit_cmd);
	install_element (ENABLE_NODE, &timet_test_exit_once_cmd);
#endif
#ifdef PL_OPENWRT_UCI
	install_element (ENABLE_NODE, &uci_get_test_cmd_cmd);
	install_element (ENABLE_NODE, &uci_set_test_cmd_cmd);
	install_element (ENABLE_NODE, &uci_list_test_cmd_cmd);
	install_element (ENABLE_NODE, &uci_section_test_cmd_cmd);
	install_element (ENABLE_NODE, &uci_del_test_cmd_cmd);
	install_element (ENABLE_NODE, &uci_del_test_vv_cmd);
	install_element (ENABLE_NODE, &uci_commit_test_cmd_cmd);
#endif
	return 0;
}
