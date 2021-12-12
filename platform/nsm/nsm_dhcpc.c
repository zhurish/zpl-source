/*
 * nsm_dhcpc.c
 *
 *  Created on: Oct 23, 2018
 *      Author: zhurish
 */

#include "os_include.h"
#include "zpl_include.h"
#include "lib_include.h"
#include "nsm_include.h"

#ifdef ZPL_DHCP_MODULE
#include "dhcp_api.h"
#endif

#ifdef ZPL_DHCPC_MODULE

int nsm_interface_dhcpc_enable(struct interface *ifp, zpl_bool enable)
{
	return dhcpc_interface_enable_api(ifp, enable);
}

int nsm_interface_dhcpc_start(struct interface *ifp, zpl_bool enable)
{
	int ret = ERROR;
	if(ifp->dhcp == zpl_true)
	{
		nsm_dhcp_ifp_t *dhcp = nsm_dhcp_get(ifp);
		if(dhcp && dhcp->running != enable)
		{
			dhcp->running = enable;
		}
	}
	//ret = OK;
	return ret;
}

zpl_bool nsm_interface_dhcpc_is_running(struct interface *ifp)
{
	if(ifp->dhcp == zpl_true)
	{
		nsm_dhcp_ifp_t *dhcp = nsm_dhcp_get(ifp);
		if(dhcp)
		{
			return dhcp->running;
		}
	}
	return zpl_false;
}

int nsm_interface_dhcpc_option(struct interface *ifp, zpl_bool enable, zpl_uint32 index, zpl_char *option)
{
	int ret = ERROR;
	ret = dhcpc_interface_option_api(ifp, enable, index, option);
	return ret;
}

#ifdef ZPL_SHELL_MODULE
int nsm_interface_dhcpc_write_config(struct interface *ifp, struct vty *vty)
{
	return dhcpc_interface_config(ifp, vty);
}

int nsm_interface_dhcpc_client_show(struct interface *ifp, struct vty *vty, zpl_bool detail)
{
	return dhcpc_interface_lease_show(vty, ifp, detail);
}
#endif
#endif
