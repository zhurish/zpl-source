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


#ifdef PL_DHCPC_MODULE


#endif


int nsm_interface_dhcpc_enable(struct interface *ifp, BOOL enable)
{
	if(enable)
		return dhcpc_interface_enable_api(ifp, enable);
	else
		return dhcpc_interface_enable_api(ifp, enable);
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
			dhcpc_interface_start_api(ifp,  enable);
		}
	}
	ret = OK;
	return ret;
}

int nsm_interface_dhcpc_option(struct interface *ifp, BOOL enable, int index, char *option)
{
	int ret = ERROR;
	ret = dhcpc_interface_option_api(ifp, enable, index, option);
	return ret;
}


int nsm_interface_dhcpc_write_config(struct interface *ifp, struct vty *vty)
{
	return dhcpc_interface_config(ifp, vty);
}

int nsm_interface_dhcpc_client_show(struct interface *ifp, struct vty *vty, BOOL detail)
{
	if(detail)
		return dhcpc_client_interface_detail_show(ifp, vty);
	else
		return dhcpc_client_interface_show(ifp, vty);
}
