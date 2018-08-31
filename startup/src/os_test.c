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
#include "version.h"
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

	//extern int syslogcLibInit (char * pServer);
	//syslogcLibInit ("127.0.0.1");
	//modem_test_init();
	install_element (ENABLE_NODE, &wifi_list_cmd);
	install_element (ENABLE_NODE, &sdk_test_cmd);
	install_element (ENABLE_NODE, &syslog_debug_test_cmd);
#ifdef DOUBLE_PROCESS
	install_element (ENABLE_NODE, &process_test_cmd);
#endif
	return 0;
}
