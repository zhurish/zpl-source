/*
 * modem_dhcp.c
 *
 *  Created on: Jul 28, 2018
 *      Author: zhurish
 */

#include "zebra.h"
#include "log.h"
#include "memory.h"
#include "str.h"
#include "if.h"
#include "os_list.h"
#include "os_util.h"
#include "tty_com.h"

#include "modem.h"
#include "modem_client.h"
#include "modem_machine.h"
#include "modem_event.h"
#include "modem_pppd.h"
#include "modem_dhcp.h"

#include "modem_atcmd.h"


static int _dhcpc_start(int id, char *process, char *ifname)
{
#ifdef DOUBLE_PROCESS
	MODEM_HDCP_DEBUG("dhcpc %d", id);
	char name[64];
	char *argv[] = {"-f", "-i", ifname, "-s", "/usr/share/udhcpc/udhcpc.script", "-S", NULL};
	os_memset(name, 0, sizeof(name));
	os_snprintf(name, sizeof(name), "dhcpc%d", id);
	return os_process_register(PROCESS_START, name,
			process, FALSE, argv);
#else
	int id = 0;
	char *argv[] = {"-f", "-i", ifname, "-s", "/usr/share/udhcpc/udhcpc.script", "-S", NULL};
	if(!ifname)
		return 0;
	id = child_process_create();
	if(id == 0)
	{
		//super_system_execvp(process, argv);
		while(1)
			os_sleep(10);
	}
	else
	{
		MODEM_HDCP_DEBUG("dhcpc %d", id);
		return id;
	}
#endif
	return OK;
}

static int _dhcpc_stop(int process)
{
	MODEM_HDCP_DEBUG(" dhcpc");
#ifdef DOUBLE_PROCESS
	return os_process_action(PROCESS_STOP, NULL, process);
#else
	return child_process_destroy(process);
#endif
}


static int _modem_dhcp_nwcall(modem_client_t *client, BOOL enable)
{
	if(enable == FALSE)
	{
		if(modem_bitmap_chk(&client->hw_state, MODEM_STATE_HW_CGDCONT))
		{
			if(modem_nwcell_atcmd_set(client, enable) == OK)
			{
				return modem_bitmap_clr(&client->hw_state, MODEM_STATE_HW_CGDCONT);
			}
			else
				return ERROR;
		}
		else
			return OK;
	}
	if(!modem_bitmap_chk(&client->hw_state, MODEM_STATE_HW_CGDCONT))
	{
		if(modem_nwcell_atcmd_set(client, enable) == OK)
		{
			modem_bitmap_set(&client->hw_state, MODEM_STATE_HW_CGDCONT);
			return OK;
		}
		else
			return ERROR;
	}
	return OK;
}


static int _modem_dhcpc_start(modem_client_t *client)
{
	modem_t *modem = NULL;
	struct interface *ifp = NULL;
	assert(client);
	assert(client->modem);
	modem = client->modem;
#ifdef _MODEM_DHCPC_DEBUG
	modem->pid[modem->dialtype] = _dhcpc_start(client->driver->id, "udhcpc", "ifname");
#else
	assert(modem->ifp);
	ifp = modem->ifp;
	modem->pid[modem->dialtype] = _dhcpc_start("udhcpc",
			ifkernelindex2kernelifname(ifp->k_ifindex));
#endif
	if(modem->pid[modem->dialtype])
	{
		//modem->state = MODEM_STATE_NETWORK_ACTIVE;
		return OK;
	}
	return ERROR;
}

static int _modem_dhcpc_stop(modem_client_t *client)
{
	modem_t *modem = NULL;
	assert(client);
	assert(client->modem);
	modem = client->modem;
	if(modem->pid[modem->dialtype])
	{
		_dhcpc_stop(modem->pid[modem->dialtype]);
		modem->pid[modem->dialtype] = 0;
	}
	return OK;
}



int modem_dhcpc_attach(modem_t *modem)
{
	int ret = 0;
	assert(modem);
	assert(modem->client);
	ret = _modem_dhcp_nwcall(modem->client, TRUE);
	MODEM_HDCP_DEBUG("dhcpc start dial");
	return ret;
}

int modem_dhcpc_start(modem_t *modem)
{
	int ret = OK;
	assert(modem);
	assert(modem->client);
	if(modem->pid[modem->dialtype] == 0)
		ret = _modem_dhcpc_start(modem->client);
	MODEM_HDCP_DEBUG("dhcpc start");
	return ret;
}


int modem_dhcpc_unattach(modem_t *modem)
{
	int ret = 0;
	assert(modem);
	assert(modem->client);
	ret = _modem_dhcp_nwcall(modem->client, FALSE);
	MODEM_HDCP_DEBUG("dhcpc stop dial");
	return ret;
}

int modem_dhcpc_exit(modem_t *modem)
{
	assert(modem);
	assert(modem->client);
	MODEM_HDCP_DEBUG("dhcpc exit");
	return _modem_dhcpc_stop(modem->client);
}
