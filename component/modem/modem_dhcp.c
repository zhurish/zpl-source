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
#include "prefix.h"
#include "if.h"

#include "os_list.h"
#include "os_util.h"
#include "tty_com.h"
#include "nsm_dhcp.h"

#include "modem.h"
#include "modem_client.h"
#include "modem_machine.h"
#include "modem_event.h"
#include "modem_pppd.h"
#include "modem_dhcp.h"

#include "modem_atcmd.h"

#ifdef MODEM_DHCPC_PROCESS
static int _dhcpc_start(int id, char *process, char *ifname)
{
	MODEM_HDCP_DEBUG("dhcpc %d", id);
	char name[64];
	char *argv[] = {"-f", "-i", ifname, "-s", "/usr/share/udhcpc/udhcpc.script", "-S", NULL};
	os_memset(name, 0, sizeof(name));
	os_snprintf(name, sizeof(name), "dhcpc%d", id);
	return os_process_register(PROCESS_START, name,
			process, FALSE, argv);
	return OK;
}

static int _dhcpc_stop(int process)
{
	MODEM_HDCP_DEBUG(" dhcpc");
	return os_process_action(PROCESS_STOP, NULL, process);
}

#else
static int _dhcpc_start(modem_t *modem, struct interface *ifp)
{
	if(if_is_wireless(ifp))
	{
		if(nsm_interface_dhcp_mode_set_api(ifp, DHCP_CLIENT) == OK)
			return nsm_interface_dhcpc_start(ifp, TRUE);
		else
			return ERROR;
	}
	else
		return nsm_interface_dhcp_mode_set_api(ifp, DHCP_CLIENT);
}

static int _dhcpc_stop(modem_t *modem, struct interface *ifp)
{
	MODEM_HDCP_DEBUG(" dhcpc");
	if(if_is_wireless(ifp))
	{
		return nsm_interface_dhcp_mode_set_api(ifp, DHCP_NONE);
	}
	else
		return nsm_interface_dhcp_mode_set_api(ifp, DHCP_NONE);
}
#endif

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
#ifdef MODEM_DHCPC_PROCESS

	assert(modem->eth0);
	ifp = modem->eth0;
	modem->pid[modem->dialtype] = _dhcpc_start(0, "udhcpc",
			ifkernelindex2kernelifname(ifp->k_ifindex));
	if(modem->pid[modem->dialtype])
	{
		//modem->state = MODEM_STATE_NETWORK_ACTIVE;
		return OK;
	}
#else
	if(modem->eth0)
		return _dhcpc_start(modem, modem->eth0);
#endif
	return ERROR;
}

static int _modem_dhcpc_stop(modem_client_t *client)
{
	modem_t *modem = NULL;
	assert(client);
	assert(client->modem);
	modem = client->modem;
#ifdef MODEM_DHCPC_PROCESS
	if(modem->pid[modem->dialtype])
	{
		_dhcpc_stop(modem->pid[modem->dialtype]);
		modem->pid[modem->dialtype] = 0;
	}
#else
	if(modem->eth0)
		return _dhcpc_stop(modem, modem->eth0);
#endif
	return ERROR;
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
		;//ret = _modem_dhcpc_start(modem->client);
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
