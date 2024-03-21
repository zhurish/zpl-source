/*
 * os_test.c
 *
 *  Created on: May 27, 2017
 *      Author: zhurish
 */
#include "auto_include.h"
#include "zplos_include.h"
#include "lib_include.h"
#include "getopt.h"
#include <log.h>
//#include "if_name.h"
//#include "nsm_zclient.h"
#include "command.h"
//#include "if.h"
#include "zmemory.h"
#include "prefix.h"
//#include "sigevent.h"
#include "sockunion.h"
#include "eloop.h"
#include "stream.h"
//#include "filter.h"
//#include "plist.h"
#include "vty.h"
#include "network.h"

#include "startup_start.h"
#include "startup_module.h"
#include "os_util.h"

struct nsm_rule
{
	int slot;
	int type;       //协商后的主备
	int cfg_type;   //配置的主备
	int flag;       //协商的标志
	int ttl;
	int reset;
};
struct nsm_rule _nsm_rule1 = {1, 1, 1, 0, 0, 0};
struct nsm_rule _nsm_rule2 = {2, 1, 1, 0, 0, 0};

static int nsm_rule_detault(int slot, int type, int cfg_type)
{
	if(slot==1)
	{
		_nsm_rule1.slot = 1;
		_nsm_rule1.type = type;
		_nsm_rule1.cfg_type = cfg_type;
		_nsm_rule1.flag = 0;
		_nsm_rule1.ttl = 0;
		_nsm_rule1.reset = 15;
	}
	else if(slot==2)
	{
		_nsm_rule2.slot = 1;
		_nsm_rule2.type = type;
		_nsm_rule2.cfg_type = cfg_type;
		_nsm_rule2.flag = 0;
		_nsm_rule2.ttl = 0;
		_nsm_rule2.reset = 15;
	}	
	return 0;
}
static int nsm_rule_update1(struct nsm_rule *rule)
{
        if(_nsm_rule1.slot < rule->slot)
		{
			if(_nsm_rule1.type != 1 && rule->flag == 0)
			{
                if(_nsm_rule1.type != 1)
				    zlog_debug(MODULE_LIB, "slot 1 change to master\r\n");
                _nsm_rule1.type = 1;//master  
                _nsm_rule1.flag = 1;  
			}
			if(_nsm_rule1.type != 1 && rule->flag == 1)
			{
				if(_nsm_rule1.type != 2)//standby
				    zlog_debug(MODULE_LIB, "slot 1 change to standby\r\n");
                _nsm_rule1.type = 2;//standby
                _nsm_rule1.flag = 1;
			}
		}
		else
		{
			if(_nsm_rule1.type != 1 && rule->flag == 0)
			{
				if(_nsm_rule1.type != 2)
				    zlog_debug(MODULE_LIB, "slot 1 change to standby\r\n");
                _nsm_rule1.type = 2;//standby
                _nsm_rule1.flag = 1;
			}
			if(_nsm_rule1.type != 1 && rule->flag == 1)
			{
				if(_nsm_rule1.type != 2)
				    zlog_debug(MODULE_LIB, "slot 1 change to standby\r\n");
                _nsm_rule1.type = 2;//standby
                _nsm_rule1.flag = 1;
			}
		}
		_nsm_rule1.ttl = 3;
	    return 0;
}
static int nsm_rule_update2(struct nsm_rule *rule)
{
        if(_nsm_rule2.slot < rule->slot)
		{
			if(_nsm_rule2.type != 1 && rule->flag == 0)
			{
                if(_nsm_rule2.type != 1)
				    zlog_debug(MODULE_LIB, "slot 2 change to master\r\n");
                _nsm_rule2.type = 1;//master   
                _nsm_rule2.flag = 1; 
			}
			if(_nsm_rule2.type != 1 && rule->flag == 1)
			{
				if(_nsm_rule2.type != 2)//standby
				    zlog_debug(MODULE_LIB, "slot 2 change to standby\r\n");
                _nsm_rule2.type = 2;//standby
                _nsm_rule2.flag = 1;
			}
		}
		else
		{
			if(_nsm_rule2.type != 1 && rule->flag == 0)
			{
				if(_nsm_rule2.type != 2)
				    zlog_debug(MODULE_LIB, "slot 2 change to standby\r\n");
                _nsm_rule2.type = 2;//standby
                _nsm_rule2.flag = 1;
            }
            if(_nsm_rule2.type != 1 && rule->flag == 1)
			{
				if(_nsm_rule2.type != 2)
				    zlog_debug(MODULE_LIB, "slot 2 change to standby\r\n");
                _nsm_rule2.type = 2;//standby
                _nsm_rule2.flag = 1;
			}
		}
        _nsm_rule2.ttl = 3;
	    return 0;
}

static int nsm_rule_test1(void *p)
{
    sleep(5);
	while(1)
	{
		sleep(1);
        if(_nsm_rule1.ttl)
            _nsm_rule1.ttl--;
        if(_nsm_rule1.ttl == 0)
        {
            if(_nsm_rule1.type != 1)
				zlog_debug(MODULE_LIB, "slot 1 change to master\r\n");
            _nsm_rule1.type = 1;//master   
            _nsm_rule1.flag = 1; 
        }    
        if (_nsm_rule2.reset)
            _nsm_rule2.reset--;
        if(_nsm_rule2.reset == 0)    
            nsm_rule_update1(&_nsm_rule2);
    }
	return 0;
}
static int nsm_rule_test2(void *p)
{
    sleep(5);
	while(1)
	{
		sleep(1);
        if(_nsm_rule2.ttl)
            _nsm_rule2.ttl--;
        if(_nsm_rule2.ttl == 0)
        {
            if(_nsm_rule2.type != 1)
				zlog_debug(MODULE_LIB, "slot 2 change to master\r\n");
            _nsm_rule2.type = 1;//master   
            _nsm_rule2.flag = 1; 
        }  
        if (_nsm_rule1.reset)
            _nsm_rule1.reset--;
        if(_nsm_rule1.reset == 0)   
		    nsm_rule_update2(&_nsm_rule1);
	}
	return 0;
}

DEFUN (rule_test_start,
		rule_test_start_cmd,
       "rule_test_start",
       "Select an interface to configure\n")
{
	pthread_t pid1;
	pthread_t pid2;
	pthread_create(&pid1, NULL, nsm_rule_test1, NULL);
	pthread_create(&pid2, NULL, nsm_rule_test2, NULL);
	return CMD_SUCCESS;
}

DEFUN (rule_test_reset,
		rule_test_reset_cmd,
       "rule_test_reset <1-2> <0-2> <0-2>",
       "Select an interface to configure\n"
	   "slot\n"
	   "type\n"
	   "cfg type\n")
{
	nsm_rule_detault(atoi(argv[0]), atoi(argv[1]), atoi(argv[2]));
	return CMD_SUCCESS;
}

DEFUN (rule_test_show,
		rule_test_show_cmd,
       "rule_test_show",
       "Select an interface to configure\n")
{
		if(_nsm_rule1.type == 2)
			vty_out(vty, "slot 1 is standby\r\n");
		else if(_nsm_rule1.type == 1)
			vty_out(vty, "slot 1 is master\r\n");
		else
			vty_out(vty, "slot 1 is sadada\r\n");
		if(_nsm_rule2.type == 2)
			vty_out(vty, "slot 2 is standby\r\n");
		else if(_nsm_rule2.type == 1)
			vty_out(vty, "slot 2 is master\r\n");
		else
			vty_out(vty, "slot 2 is sadada\r\n");
	return CMD_SUCCESS;
}


#if 0
struct nsm_ctrl_client
{
	int sock;
	struct stream *ibuf;
	struct stream *obuf;
	struct eloop	*t_read;
	struct eloop	*t_hello;
	int slot;
	int type;
	struct sockaddr_in	remote;
};
struct nsm_ctrl_server
{
	int sock;
	int rpt_sock;
	struct stream *ibuf;
	struct stream *obuf;
	struct eloop	*t_accept;
	struct eloop	*t_rpt_accept;
	struct nsm_ctrl_client *client[8];
	struct nsm_ctrl_client *rpt_client[8];
};

struct nsm_ctrl_server nsm_ctrl_server;
struct eloop_master *eloop_master;
static int nsm_ctrl_client_close(struct nsm_ctrl_client *);
static int nsm_ctrl_client_read(struct eloop *);
static int nsm_ctrl_client_hello(struct eloop *);


static int nsm_ctrl_client_send_message(struct nsm_ctrl_client *client)
{
	int ret = 0;
	int len = stream_get_endp(client->obuf);
	char *buf = STREAM_DATA(client->obuf);
	while(1)
	{
		ret = write(client->sock, buf, len);
		if(ret)
		{
			len -= ret;
			buf += ret;
		}
		else
		{
			if(ERRNO_IO_RETRY(errno))
			{
				continue;
			}
			nsm_ctrl_client_close(client);
			return -1;
		}
		if(len == 0)
			return 0;
	}
	return -1;
}

static int nsm_ctrl_client_close(struct nsm_ctrl_client *client)
{
	ELOOP_OFF(client->t_read);
	ELOOP_OFF(client->t_hello);
	if(client->sock)
	{
		close(client->sock);
	}
	if(client->type)
	{
		nsm_ctrl_server.rpt_client[client->slot] = NULL;
	}
	else
	{
		nsm_ctrl_server.client[client->slot] = NULL;
	}
	free(client);
	return 0;
}

static int nsm_ctrl_client_create(int type, int sock, struct sockaddr_in *remote)
{
	struct nsm_ctrl_client *client = malloc(sizeof(struct nsm_ctrl_client));
	if(client == NULL)
		return -1;
	memset(client, 0,sizeof(struct nsm_ctrl_client));	
	client->t_hello = NULL;
	client->t_read = NULL;
	client->slot = 1;
	if(type)
	{
		if(nsm_ctrl_server.rpt_client[client->slot]);
			nsm_ctrl_client_close(nsm_ctrl_server.rpt_client[client->slot]);
		nsm_ctrl_server.rpt_client[client->slot] = client;
	}
	else
	{
		if(nsm_ctrl_server.client[client->slot]);
			nsm_ctrl_client_close(nsm_ctrl_server.client[client->slot]);
		nsm_ctrl_server.client[client->slot] = client;
	}
	client->type = type;
	client->sock = sock;
	client->ibuf = nsm_ctrl_server.ibuf;
	client->obuf = nsm_ctrl_server.obuf;
	memcpy(&client->remote, remote, sizeof(struct sockaddr_in));
	if(type)
		client->t_read = eloop_add_read(eloop_master, nsm_ctrl_client_read, client, sock);
	else
		client->t_hello = eloop_add_timer(eloop_master, nsm_ctrl_client_read, client, 1);	
	return 0;	
}
#endif

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
	//extern int syslogc_out(int priority, int ffacility, char * pStr, int len);
	message = argv_concat(argv, argc, 0);
	//syslogc_out(ZLOG_LEVEL_DEBUG, 7, message, os_strlen(message));
	return CMD_SUCCESS;
}

#if 0
DEFUN (sdk_test,
		sdk_test_cmd,
       "sdk-speed <0-5> (10|100|1000)",
       "syslog-debug\n"
	   "dest")
{
#ifdef ZPL_HAL_MODULE
	ifindex_t ifindex = atoi(argv[0]);
	int value = atoi(argv[0]);
	hal_port_speed_set( ifindex,  value);
#endif
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

#ifdef ZPL_DHCP_MODULE
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
		taskid = os_process_register(PROCESS_START, "pppd", "pppd", zpl_true, argve);
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

#ifdef ZPL_TOOLS_PROCESS
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
					"dhcpc", zpl_false, inargv);
		break;
	case 2:
		res = os_process_register(PROCESS_STOP, "dhcpc-eth0",
					"dhcpc", zpl_false, inargv);
		break;
	case 3:
		res = os_process_register(PROCESS_RESTART, "dhcpc-eth0",
					"dhcpc", zpl_false, inargv);
		break;

	case 4:
		res = os_process_register(PROCESS_START, "dhcpc-eth1",
					"dhcpc1", zpl_true, inargv1);
		break;
	case 5:
		res = os_process_register(PROCESS_STOP, "dhcpc-eth2",
					"dhcpc1", zpl_true, inargv1);
		break;
	case 6:
		res = os_process_register(PROCESS_RESTART, "dhcpc-eth2",
					"dhcpc1", zpl_true, inargv1);
		break;
	}
	if((res == ERROR))
		vty_out(vty,"res:ERROR(%s)(%d)%s",strerror(ipstack_errno),sizeof(process_head),VTY_NEWLINE);
	else
		vty_out(vty,"res:%d(%s)(%d)%s",res,strerror(ipstack_errno),sizeof(process_head),VTY_NEWLINE);
	return CMD_SUCCESS;
}
#endif
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

#ifdef ZPL_OPENWRT_UCI
DEFUN (uci_get_test_cmd,
		uci_get_test_cmd_cmd,
       "uci-test (get-string|get-integer|get-list) NAME",
       "uco-test\n")
{
	char value[64];
	int valu = 0;
	char *lstv[8];

	vty_out(vty, "uci_get_test_cmd_cmd :%s %s %s", argv[0], argv[1], VTY_NEWLINE);

	if(strstr(argv[0], "string"))
		os_uci_get_string(argv[1], value);
	else if(strstr(argv[0], "integer"))
		os_uci_get_integer("sip_config.sip.localport", &valu);
	else
		os_uci_get_list(argv[1], lstv, &valu);

	if(strstr(argv[0], "string"))
		vty_out(vty, "get-string: %s=%s%s", argv[1], value, VTY_NEWLINE);
	else if(strstr(argv[0], "integer"))
		vty_out(vty, "get-integer: %s=%d%s", argv[1], valu, VTY_NEWLINE);
	else
	{
		int i = 0;
		for(i = 0; i < valu; i++)
			vty_out(vty, "get-list: %s=%s%s", argv[1], lstv[i],VTY_NEWLINE);
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
			os_uci_del(argv[0], argv[1], argv[2], NULL);
	else if(argc == 2)
			os_uci_del(argv[0], argv[1], NULL, NULL);
	else if(argc == 1)
			os_uci_del(argv[0], NULL, NULL, NULL);
	return CMD_SUCCESS;
}

ALIAS(uci_del_test_cmd,
		uci_del_test_vv_cmd,
       "uci-test del NAME OP",
       "uco-test\n");


DEFUN (uci_commit_test_cmd,
		uci_commit_test_cmd_cmd,
       "uci-test commit NAME",
       "uco-test\n")
{
	os_uci_commit(argv[0]);
	return CMD_SUCCESS;
}
#endif



int os_test(void)
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
	zpl_uint8 mac[7];
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

	install_element (ENABLE_NODE, CMD_CONFIG_LEVEL, &i_iusp_test_cmd);
	install_element (ENABLE_NODE, CMD_CONFIG_LEVEL, &i_mac_test_cmd);
	install_element (ENABLE_NODE, CMD_CONFIG_LEVEL, &syslog_debug_test_cmd);


	install_element (ENABLE_NODE, CMD_CONFIG_LEVEL, &rule_test_start_cmd);
	install_element (ENABLE_NODE, CMD_CONFIG_LEVEL, &rule_test_reset_cmd);
	install_element (ENABLE_NODE, CMD_CONFIG_LEVEL, &rule_test_show_cmd);
	
	

#if 0
	install_element (ENABLE_NODE, CMD_CONFIG_LEVEL, &wifi_list_cmd);
	install_element (ENABLE_NODE, CMD_CONFIG_LEVEL, &wifi_scan_cmd);
#ifdef ZPL_DHCP_MODULE
	install_element (ENABLE_NODE, CMD_CONFIG_LEVEL, &dhcp_test_cmd);
#endif
	install_element (ENABLE_NODE, CMD_CONFIG_LEVEL, &sdk_test_cmd);
	install_element (ENABLE_NODE, CMD_CONFIG_LEVEL, &bond_test_cmd);
	install_element (ENABLE_NODE, CMD_CONFIG_LEVEL, &os_process_test_cmd);

#ifdef ZPL_TOOLS_PROCESS
	install_element (ENABLE_NODE, CMD_CONFIG_LEVEL, &process_test_cmd);
#endif
#endif

#ifdef OS_TIMER_TEST
	install_element (ENABLE_NODE, CMD_CONFIG_LEVEL, &timet_test_cmd);
	install_element (ENABLE_NODE, CMD_CONFIG_LEVEL, &timet_test_once_cmd);
	install_element (ENABLE_NODE, CMD_CONFIG_LEVEL, &timet_test_exit_cmd);
	install_element (ENABLE_NODE, CMD_CONFIG_LEVEL, &timet_test_exit_once_cmd);
#endif
#ifdef ZPL_OPENWRT_UCI
	install_element (ENABLE_NODE, CMD_CONFIG_LEVEL, &uci_get_test_cmd_cmd);
	install_element (ENABLE_NODE, CMD_CONFIG_LEVEL, &uci_set_test_cmd_cmd);
	install_element (ENABLE_NODE, CMD_CONFIG_LEVEL, &uci_list_test_cmd_cmd);
	install_element (ENABLE_NODE, CMD_CONFIG_LEVEL, &uci_section_test_cmd_cmd);
	install_element (ENABLE_NODE, CMD_CONFIG_LEVEL, &uci_del_test_cmd_cmd);
	install_element (ENABLE_NODE, CMD_CONFIG_LEVEL, &uci_del_test_vv_cmd);
	install_element (ENABLE_NODE, CMD_CONFIG_LEVEL, &uci_commit_test_cmd_cmd);
#endif
	return 0;
}
