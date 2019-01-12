/*
 * dhcpd_main.c
 *
 *  Created on: Sep 20, 2018
 *      Author: zhurish
 */


/*

#include <sys/types.h>
#include <sys/socket.h>

#include <net/if.h>

#include <arpa/inet.h>

#include <err.h>
#include <netdb.h>
#include <paths.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include "dhcp.h"
#include "tree.h"
#include "dhcpd.h"
#include "sync.h"
*/
#define _GRP_H
#include "zebra.h"
#include "prefix.h"
#include "if.h"

#include "dhcp.h"
#include "tree.h"
#include "dhcpd.h"
#include "sync.h"
//#include "os_task.h"
#include "dhcpd_api.h"


static int dhcpd_task_id = 0;


static int dhcpd_task(void *pVoid)
{
	struct in_addr udpaddr;
	udpaddr.s_addr = 0;//htonl(INADDR_BROADCAST);
	/* Default DHCP/BOOTP ports. */
/*
	server_port = htons(SERVER_PORT);
	client_port = htons(CLIENT_PORT);

	tzset();


	dhcp_global_default_init();
	db_startup();
*/
	while(!os_load_config_done())
	{
		os_sleep(1);
	}
	time(&root_group.cur_time);


	udpsock_startup(udpaddr);

	//icmp_startup(1, lease_pinged);

	add_timeout(root_group.cur_time + 5, periodic_scan, NULL);
	//dhcpd_interface_add(2);
	dispatch();

	return 0;
}


int dhcps_module_init ()
{
	/* Default DHCP/BOOTP ports. */
	//root_group.server_port = htons(SERVER_PORT);
	//client_port = htons(CLIENT_PORT);

	//server_port = htons(6067);
	//client_port = htons(6068);
#ifdef DHCPD_ANSYNC_ENABLE
	dhcpd_lstmaster = os_ansync_lst_create(MODULE_DHCPD, 6);
	if(!dhcpd_lstmaster)
		return 0;
#endif

	tzset();
	dhcpd_global_default_init();
	db_startup();

	return 0;
}

int dhcps_task_init ()
{
	if(dhcpd_task_id == 0)
		dhcpd_task_id = os_task_create("dhcpsTask", OS_TASK_DEFAULT_PRIORITY,
	               0, dhcpd_task, NULL, OS_TASK_DEFAULT_STACK);
	if(dhcpd_task_id)
		return 0;
	return -1;

}

int dhcps_task_exit ()
{
	if(dhcpd_task_id)
		os_task_destroy(dhcpd_task_id);
	dhcpd_task_id = 0;
	return OK;
}

int dhcps_module_exit ()
{
#ifdef DHCPD_ANSYNC_ENABLE
	if(dhcpd_lstmaster)
		os_ansync_lst_destroy(dhcpd_lstmaster);
#endif
	return 0;
}
