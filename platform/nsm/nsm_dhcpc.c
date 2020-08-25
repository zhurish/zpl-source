/*
 * nsm_dhcpc.c
 *
 *  Created on: Oct 23, 2018
 *      Author: zhurish
 */

#include <zebra.h>

#include "if.h"
#include "vty.h"
#include "sockunion.h"
#include "prefix.h"
#include "command.h"
#include "memory.h"
#include "log.h"
#include "vrf.h"
#include "command.h"
#include "interface.h"

#include "if_name.h"
#include "rib.h"
#include "zserv.h"
#include "redistribute.h"
#include "debug.h"
#include "zclient.h"
#include "nsm_dhcp.h"

#ifdef PL_UDHCP_MODULE
#include "dhcp_api.h"
#endif

#ifdef PL_DHCPC_MODULE

int nsm_interface_dhcpc_enable(struct interface *ifp, BOOL enable)
{
#ifdef PL_UDHCP_MODULE
	return dhcpc_interface_enable_api(ifp, enable);
#endif
}

int nsm_interface_dhcpc_start(struct interface *ifp, BOOL enable)
{
	int ret = ERROR;
	if(ifp->dhcp == TRUE)
	{
		nsm_dhcp_ifp_t *dhcp = nsm_dhcp_get(ifp);
		if(dhcp && dhcp->running != enable)
		{
			dhcp->running = enable;
#ifdef PL_UDHCP_MODULE
#endif
		}
	}
	//ret = OK;
	return ret;
}

BOOL nsm_interface_dhcpc_is_running(struct interface *ifp)
{
	if(ifp->dhcp == TRUE)
	{
		nsm_dhcp_ifp_t *dhcp = nsm_dhcp_get(ifp);
		if(dhcp)
		{
			return dhcp->running;
		}
	}
	return FALSE;
}

int nsm_interface_dhcpc_option(struct interface *ifp, BOOL enable, int index, char *option)
{
	int ret = ERROR;
#ifdef PL_UDHCP_MODULE
	ret = dhcpc_interface_option_api(ifp, enable, index, option);
#endif
	return ret;
}


int nsm_interface_dhcpc_write_config(struct interface *ifp, struct vty *vty)
{
#ifdef PL_UDHCP_MODULE
	return dhcpc_interface_config(ifp, vty);
#endif
}

int nsm_interface_dhcpc_client_show(struct interface *ifp, struct vty *vty, BOOL detail)
{
#ifdef PL_UDHCP_MODULE
	return dhcpc_interface_lease_show(vty, ifp, detail);
#endif
}

#endif
