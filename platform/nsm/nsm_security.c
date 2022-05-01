/*
 * nsm_security.c
 *
 *  Created on: Apr 19, 2018
 *      Author: zhurish
 */


#include "auto_include.h"
#include "zplos_include.h"
#include "if.h"
#include "vrf.h"
#include "prefix.h"
#include "vty.h"
#include "zmemory.h"
#include "template.h"
#include "algorithm.h"
#include "nsm_interface.h"
#include "nsm_security.h"
#include "nsm_qos.h"
#include "hal_port.h"
#include "hal_qos.h"

static Global_Security_t Global_Security;

static nsm_security_t * _nsm_security_get(struct interface *ifp)
{
	return (nsm_security_t *)nsm_intf_module_data(ifp, NSM_INTF_SEC);
}



int nsm_stormcontrol_enable_set_api(struct interface *ifp, zpl_bool enable)
{
	nsm_security_t *stormcontrol = _nsm_security_get(ifp);
	if (stormcontrol)
	{
		stormcontrol->stormcontrol_enable = enable;
		return OK;
	}
	return ERROR;
}

/*
 * port storm control
 */
zpl_bool nsm_stormcontrol_enable_get_api(struct interface *ifp)
{
	nsm_security_t *stormcontrol = _nsm_security_get(ifp);
	if (stormcontrol)
	{
		return stormcontrol->stormcontrol_enable;
	}
	return zpl_false;
}

int nsm_stormcontrol_unicast_set_api(struct interface *ifp, zpl_uint32 stormcontrol_unicast,
								  zpl_uint8 unicastflag)
{
	nsm_security_t *stormcontrol = _nsm_security_get(ifp);
	if (stormcontrol)
	{
		stormcontrol->stormcontrol.stormcontrol_unicast = stormcontrol_unicast;
		stormcontrol->stormcontrol.unicast_flags = unicastflag;
		return OK;
	}
	return ERROR;
}

int nsm_stormcontrol_unicast_get_api(struct interface *ifp, zpl_uint32 *stormcontrol_unicast,
								  zpl_uint8 *unicastflag)
{
	nsm_security_t *stormcontrol = _nsm_security_get(ifp);
	if (stormcontrol)
	{
		if (stormcontrol_unicast)
			*stormcontrol_unicast = stormcontrol->stormcontrol.stormcontrol_unicast;
		if (unicastflag)
			*unicastflag = stormcontrol->stormcontrol.unicast_flags;
		return OK;
	}
	return ERROR;
}

int nsm_stormcontrol_multicast_set_api(struct interface *ifp, zpl_uint32 stormcontrol_multicast,
									zpl_uint8 multicastflag)
{
	nsm_security_t *stormcontrol = _nsm_security_get(ifp);
	if (stormcontrol)
	{
		if(hal_port_stormcontrol_set(ifp->ifindex, 1, stormcontrol_multicast, NSM_STORMCONTROL_RATE) == OK)
		{
			stormcontrol->stormcontrol.multicast_flags = multicastflag;
			stormcontrol->stormcontrol.stormcontrol_multicast = stormcontrol_multicast;
			return OK;
		}
	}
	return ERROR;
}

int nsm_stormcontrol_multicast_get_api(struct interface *ifp, zpl_uint32 *stormcontrol_multicast,
									zpl_uint8 *multicastflag)
{
	nsm_security_t *stormcontrol = _nsm_security_get(ifp);
	if (stormcontrol)
	{
		if (multicastflag)
			*multicastflag = stormcontrol->stormcontrol.multicast_flags;
		if (stormcontrol_multicast)
			*stormcontrol_multicast = stormcontrol->stormcontrol.stormcontrol_multicast;
		return OK;
	}
	return ERROR;
}

int nsm_stormcontrol_broadcast_set_api(struct interface *ifp, zpl_uint32 stormcontrol_broadcast,
									zpl_uint8 broadcastflag)
{
	nsm_security_t *stormcontrol = _nsm_security_get(ifp);
	if (stormcontrol)
	{
		if(hal_port_stormcontrol_set(ifp->ifindex, 2, stormcontrol_broadcast, NSM_STORMCONTROL_RATE) == OK)
		{
			stormcontrol->stormcontrol.broadcast_flags = broadcastflag;
			stormcontrol->stormcontrol.stormcontrol_broadcast = stormcontrol_broadcast;
			return OK;
		}
	}
	return ERROR;
}

int nsm_stormcontrol_broadcast_get_api(struct interface *ifp, zpl_uint32 *stormcontrol_broadcast,
									zpl_uint8 *broadcastflag)
{
	nsm_security_t *stormcontrol = _nsm_security_get(ifp);
	if (stormcontrol)
	{
		if (broadcastflag)
			*broadcastflag = stormcontrol->stormcontrol.broadcast_flags;
		if (stormcontrol_broadcast)
			*stormcontrol_broadcast = stormcontrol->stormcontrol.stormcontrol_broadcast;
		return OK;
	}
	return ERROR;
}

#ifdef ZPL_SHELL_MODULE
static int nsm_stormcontrol_interface_storm_write_config(struct vty *vty, nsm_security_t *stormcontrol)
{
	if (stormcontrol->stormcontrol_enable)
	{
		if (stormcontrol->stormcontrol.stormcontrol_unicast)
		{
			switch (stormcontrol->stormcontrol.unicast_flags)
			{
			case NSM_STORMCONTROL_RATE:
				vty_out(vty, " storm control unicast %d%s", stormcontrol->stormcontrol.stormcontrol_unicast, VTY_NEWLINE);
				break;
			case NSM_STORMCONTROL_PERCENT:
				vty_out(vty, " storm control unicast %s%d%s", " percent ", stormcontrol->stormcontrol.stormcontrol_unicast, VTY_NEWLINE);
				break;
			case NSM_STORMCONTROL_PACKET:
				vty_out(vty, " storm control unicast%s%d%s", " pps ", stormcontrol->stormcontrol.stormcontrol_unicast, VTY_NEWLINE);
				break;
			}
		}
		if (stormcontrol->stormcontrol.stormcontrol_multicast)
		{
			switch (stormcontrol->stormcontrol.multicast_flags)
			{
			case NSM_STORMCONTROL_RATE:
				vty_out(vty, " storm control multicast %d%s", stormcontrol->stormcontrol.stormcontrol_multicast, VTY_NEWLINE);
				break;
			case NSM_STORMCONTROL_PERCENT:
				vty_out(vty, " storm control multicast %s%d%s", " percent ", stormcontrol->stormcontrol.stormcontrol_multicast, VTY_NEWLINE);
				break;
			case NSM_STORMCONTROL_PACKET:
				vty_out(vty, " storm control multicast%s%d%s", " pps ", stormcontrol->stormcontrol.stormcontrol_multicast, VTY_NEWLINE);
				break;
			}
		}
		if (stormcontrol->stormcontrol.stormcontrol_broadcast)
		{
			switch (stormcontrol->stormcontrol.broadcast_flags)
			{
			case NSM_STORMCONTROL_RATE:
				vty_out(vty, " storm control broadcast %d%s", stormcontrol->stormcontrol.stormcontrol_broadcast, VTY_NEWLINE);
				break;
			case NSM_STORMCONTROL_PERCENT:
				vty_out(vty, " storm control broadcast %s%d%s", " percent ", stormcontrol->stormcontrol.stormcontrol_broadcast, VTY_NEWLINE);
				break;
			case NSM_STORMCONTROL_PACKET:
				vty_out(vty, " storm control broadcast%s%d%s", " pps ", stormcontrol->stormcontrol.stormcontrol_broadcast, VTY_NEWLINE);
				break;
			}
		}
	}
	return 0;
}
#endif

int nsm_security_interface_create_api(struct interface *ifp)
{
	nsm_security_t *security = NULL;
	if(if_is_ethernet(ifp))
	{
		security = (nsm_security_t *)nsm_intf_module_data(ifp, NSM_INTF_SEC);
		if(security == NULL)
		{
			security = XMALLOC(MTYPE_SECURITY, sizeof(nsm_security_t));
			os_memset(security, 0, sizeof(nsm_security_t));
			nsm_intf_module_data_set(ifp, NSM_INTF_SEC, security);
		}
	}
	return OK;
}


int nsm_security_interface_del_api(struct interface *ifp)
{
	nsm_security_t *security = NULL;
	security = (nsm_security_t *)nsm_intf_module_data(ifp, NSM_INTF_SEC);
	if(if_is_ethernet(ifp) && security)
	{
		XFREE(MTYPE_SECURITY, security);
		security = NULL;
		nsm_intf_module_data_set(ifp, NSM_INTF_SEC, NULL);
	}
	return OK;
}

#ifdef ZPL_SHELL_MODULE
int nsm_security_interface_write_config(struct vty *vty, struct interface *ifp)
{
	nsm_security_t *security = _nsm_security_get(ifp);
	if(security && if_is_ethernet(ifp) && !if_is_l3intf(ifp))
	{
		nsm_stormcontrol_interface_storm_write_config(vty, security);		
	}
	return OK;
}
#endif



int nsm_security_init(void)
{
	memset(&Global_Security, 0, sizeof(Global_Security));
	nsm_interface_hook_add(NSM_INTF_SEC, nsm_security_interface_create_api, nsm_security_interface_del_api);
	nsm_interface_write_hook_add(NSM_INTF_SEC, nsm_security_interface_write_config);
	return OK;
}


int nsm_security_exit(void)
{
	return OK;
}



