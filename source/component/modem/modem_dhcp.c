/*
 * modem_dhcp.c
 *
 *  Created on: Jul 28, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include <zplos_include.h>
#include "zmemory.h"
#include "vty.h"
#include "command.h"
#include "if.h"

#include "nsm_include.h"

#include "modem.h"
#include "modem_client.h"
#include "modem_machine.h"
#include "modem_event.h"
#include "modem_pppd.h"
#include "modem_dhcp.h"

#include "modem_atcmd.h"

#ifdef MODEM_DHCPC_PROCESS
static int _dhcpc_start(zpl_uint32 id, char *process, char *ifname)
{
	MODEM_HDCP_DEBUG("dhcpc %d", id);
	char name[64];
	char *argv[] = {"-f", "-i", ifname, "-s", "/usr/share/udhcpc/udhcpc.script", "-S", NULL};
	os_memset(name, 0, sizeof(name));
	os_snprintf(name, sizeof(name), "dhcpc%d", id);
	return os_process_register(PROCESS_START, name,
			process, zpl_false, argv);
	return OK;
}

static int _dhcpc_stop(zpl_uint32 process)
{
	MODEM_HDCP_DEBUG(" dhcpc");
	return os_process_action(PROCESS_STOP, NULL, process);
}

#else
static int _dhcpc_start(modem_t *modem, struct interface *ifp)
{
	if(/*if_is_wireless(ifp) && */ifp->ker_ifindex)
	{
#ifdef ZPL_DHCPC_MODULE
		nsm_dhcp_type type = nsm_interface_dhcp_mode_get_api(ifp);
		switch(type)
		{
		case DHCP_NONE:
			if(nsm_interface_dhcp_mode_set_api(ifp, DHCP_CLIENT, NULL) == OK)
			{
				os_msleep(10);
				if(!nsm_interface_dhcpc_is_running(ifp))
					return nsm_interface_dhcpc_start(ifp, zpl_true);
			}
			break;
		case DHCP_CLIENT:
			if(!nsm_interface_dhcpc_is_running(ifp))
				return nsm_interface_dhcpc_start(ifp, zpl_true);
			break;
		case DHCP_SERVER:
			break;
		case DHCP_RELAY:
			break;
		}
		return ERROR;
#endif
	}
/*	else if(ifp->ker_ifindex)
		return nsm_interface_dhcp_mode_set_api(ifp, DHCP_CLIENT);*/
	return ERROR;
}

static int _dhcpc_stop(modem_t *modem, struct interface *ifp)
{
	MODEM_HDCP_DEBUG(" dhcpc");
#ifdef ZPL_DHCPC_MODULE
	nsm_dhcp_type type = nsm_interface_dhcp_mode_get_api(ifp);
	if(type == DHCP_CLIENT)
		return nsm_interface_dhcp_mode_set_api(ifp, DHCP_NONE, NULL);
#endif
		//return nsm_interface_dhcpc_start(ifp, zpl_false);
	//if(/*if_is_wireless(ifp) && */ifp->ker_ifindex)
	//{
	//	return nsm_interface_dhcp_mode_set_api(ifp, DHCP_NONE);
	//}
/*	else if(ifp->ker_ifindex)
		return nsm_interface_dhcp_mode_set_api(ifp, DHCP_NONE);*/
	return ERROR;
}
#endif

static int _modem_dhcp_nwcall(modem_client_t *client, zpl_bool enable)
{
	if(enable == zpl_false)
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
	//struct interface *ifp = NULL;
	assert(client);
	assert(client->modem);
	modem = client->modem;
#ifdef MODEM_DHCPC_PROCESS

	assert(modem->eth0);
	ifp = modem->eth0;
	modem->pid[modem->dialtype] = _dhcpc_start(0, "udhcpc",
			ifkernelindex2kernelifname(ifp->ker_ifindex));
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


zpl_bool modem_dhcpc_isconnect(modem_t *modem)
{
	assert(modem);
	if(modem->pid[modem->dialtype])
	{
		return zpl_true;
	}
	return zpl_false;
}

zpl_bool modem_dhcpc_islinkup(modem_t *modem)
{
	assert(modem);
	struct interface *ifp = modem->eth0;
	if(modem && ifp)
	{
		if(modem->pid[modem->dialtype])
		{
			if(nsm_halpal_interface_ifindex(ifp->ker_name))
			{
				if(if_is_running(ifp))
				{
					//modem_serial_interface_update_kernel(modem, ifp->ker_name);
				}
				return zpl_true;
			}
		}
	}
	return zpl_false;
}

int modem_dhcpc_attach(modem_t *modem)
{
	int ret = 0;
	assert(modem);
	assert(modem->client);
	ret = _modem_dhcp_nwcall(modem->client, zpl_true);
	//MODEM_HDCP_DEBUG("dhcpc start dial");
	return ret;
}

int modem_dhcpc_unattach(modem_t *modem)
{
	int ret = 0;
	assert(modem);
	assert(modem->client);
	ret = _modem_dhcp_nwcall(modem->client, zpl_false);
	//MODEM_HDCP_DEBUG("dhcpc stop dial");
	return ret;
}

int modem_dhcpc_start(modem_t *modem)
{
	int ret = OK;
	assert(modem);
	assert(modem->client);
	if(modem->pid[modem->dialtype] == 0)
		ret = _modem_dhcpc_start(modem->client);
	if(ret == OK)
		modem->pid[modem->dialtype] = 1;
	//MODEM_HDCP_DEBUG("dhcpc start");
	return ret;
}

int modem_dhcpc_exit(modem_t *modem)
{
	assert(modem);
	assert(modem->client);
	MODEM_HDCP_DEBUG("dhcpc exit");
	return _modem_dhcpc_stop(modem->client);
}
