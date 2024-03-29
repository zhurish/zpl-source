/*
 * nsm_dhcpc.c
 *
 *  Created on: Oct 23, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zpl_type.h"
#include "module.h"
#include "route_types.h"
#include "zmemory.h"
#include "prefix.h"
#include "log.h"
#ifdef ZPL_SHELL_MODULE
#include "vty_include.h"
#endif

#include "nsm_interface.h"
#include "nsm_dhcp.h"

#ifdef ZPL_DHCP_MODULE
#include "dhcp_api.h"
#endif

#ifdef ZPL_DHCPC_MODULE

int nsm_interface_dhcpc_enable(struct interface *ifp, zpl_bool enable)
{
	int ret = ERROR;
	nsm_dhcp_ifp_t *dhcp = nsm_dhcp_get(ifp);
	IF_NSM_DHCP_DATA_LOCK(dhcp);
	ret = dhcpc_interface_enable_api(ifp, enable);
	IF_NSM_DHCP_DATA_UNLOCK(dhcp);
	return ret;
}

int nsm_interface_dhcpc_start(struct interface *ifp, zpl_bool enable)
{
	int ret = ERROR;
	if(ifp->dhcp == zpl_true)
	{
		nsm_dhcp_ifp_t *dhcp = nsm_dhcp_get(ifp);
		IF_NSM_DHCP_DATA_LOCK(dhcp);
		if(dhcp && dhcp->running != enable)
		{
			dhcp->running = enable;
		}
		IF_NSM_DHCP_DATA_UNLOCK(dhcp);
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
	nsm_dhcp_ifp_t *dhcp = nsm_dhcp_get(ifp);
	IF_NSM_DHCP_DATA_LOCK(dhcp);
	ret = dhcpc_interface_option_api(ifp, enable, index, option);
	IF_NSM_DHCP_DATA_UNLOCK(dhcp);
	return ret;
}

#ifdef ZPL_SHELL_MODULE
int nsm_interface_dhcpc_write_config(struct interface *ifp, struct vty *vty)
{
	int ret = ERROR;
	nsm_dhcp_ifp_t *dhcp = nsm_dhcp_get(ifp);
	IF_NSM_DHCP_DATA_LOCK(dhcp);
	ret = dhcpc_interface_config(ifp, vty);
	IF_NSM_DHCP_DATA_UNLOCK(dhcp);
	return ret;
}

int nsm_interface_dhcpc_client_show(struct interface *ifp, struct vty *vty, zpl_bool detail)
{
	int ret = ERROR;
	nsm_dhcp_ifp_t *dhcp = nsm_dhcp_get(ifp);
	IF_NSM_DHCP_DATA_LOCK(dhcp);
	ret = dhcpc_interface_lease_show(vty, ifp, detail);
	IF_NSM_DHCP_DATA_UNLOCK(dhcp);
	return ret;
}
#endif
#endif
