/*
 * nsm_dhcp.c
 *
 *  Created on: Sep 8, 2018
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
#include "nsm_vrf.h"
#include "command.h"
#include "nsm_interface.h"

#include "if_name.h"
#include "nsm_rib.h"
#include "nsm_zserv.h"
#include "nsm_redistribute.h"
#include "nsm_debug.h"
#include "zclient.h"
#include "nsm_dhcp.h"


#ifdef PL_DHCP_MODULE

nsm_dhcp_ifp_t *nsm_dhcp_get(struct interface *ifp)
{
	nsm_dhcp_ifp_t *nsm_dhcp = NULL;
	if(if_is_ethernet(ifp) || if_is_lag(ifp) || if_is_vlan(ifp) ||
			if_is_brigde(ifp) || if_is_wireless(ifp))
	{
		nsm_dhcp = (nsm_dhcp_ifp_t *)ifp->info[MODULE_DHCP];
		return nsm_dhcp;
	}
	return NULL;
}


static int nsm_dhcp_add_interface(struct interface *ifp)
{
	if(if_is_ethernet(ifp) || if_is_lag(ifp) || if_is_vlan(ifp) ||
			if_is_brigde(ifp) || if_is_wireless(ifp))
	{
		if(ifp->info[MODULE_DHCP] == NULL)
		{
			ifp->info[MODULE_DHCP] = XMALLOC(MTYPE_DHCP, sizeof(nsm_dhcp_ifp_t));
			zassert(ifp->info[MODULE_DHCP]);
			os_memset(ifp->info[MODULE_DHCP], 0, sizeof(nsm_dhcp_ifp_t));
			((nsm_dhcp_ifp_t *)(ifp->info[MODULE_DHCP]))->ifp = ifp;
		}
		else
		{
			nsm_dhcp_ifp_t *dhcp = ifp->info[MODULE_DHCP];
			dhcp->ifp = ifp;
		}
	}
	return OK;
}


static int nsm_dhcp_del_interface(struct interface *ifp)
{
	if(if_is_ethernet(ifp) || if_is_lag(ifp) || if_is_vlan(ifp) ||
			if_is_brigde(ifp) || if_is_wireless(ifp))
	{
		if(ifp->info[MODULE_DHCP])
			XFREE(MTYPE_DHCP, ifp->info[MODULE_DHCP]);
		ifp->info[MODULE_DHCP] = NULL;
	}
	return OK;
}


int nsm_dhcp_interface_set_pravite(struct interface *ifp, nsm_dhcp_type type, void *pVoid)
{
	nsm_dhcp_ifp_t *nsm_dhcp = NULL;
	nsm_dhcp = nsm_dhcp_get(ifp);
	if(!nsm_dhcp)
		return OK;
	switch(type)
	{
	case DHCP_CLIENT:
		nsm_dhcp->client = pVoid;
		break;
	case DHCP_SERVER:
		nsm_dhcp->server = pVoid;
		break;
	case DHCP_RELAY:
		nsm_dhcp->relay = pVoid;
		break;
	default:
		break;
	}
	return OK;
}

void * nsm_dhcp_interface_get_pravite(struct interface *ifp, nsm_dhcp_type type)
{
	nsm_dhcp_ifp_t *nsm_dhcp = NULL;
	nsm_dhcp = nsm_dhcp_get(ifp);
	if(!nsm_dhcp)
		return NULL;
	switch(type)
	{
	case DHCP_CLIENT:
		return nsm_dhcp->client;
		break;
	case DHCP_SERVER:
		return nsm_dhcp->server;
		break;
	case DHCP_RELAY:
		return nsm_dhcp->relay;
		break;
	default:
		break;
	}
	return NULL;
}


nsm_dhcp_type nsm_interface_dhcp_mode_get_api(struct interface *ifp)
{
	nsm_dhcp_ifp_t *nsm_dhcp = NULL;
	nsm_dhcp = nsm_dhcp_get(ifp);
	if(nsm_dhcp)
	{
		if(ifp->dhcp)
			return nsm_dhcp->type;
	}
	return DHCP_NONE;
}


int nsm_interface_dhcp_mode_set_api(struct interface *ifp, nsm_dhcp_type type, char *name)
{
	nsm_dhcp_ifp_t *nsm_dhcp = NULL;
	nsm_dhcp = nsm_dhcp_get(ifp);
	if(nsm_dhcp)
	{
		if(!ifp->dhcp)
		{
			ifp->dhcp = TRUE;
		}
		if(nsm_dhcp->type != type)
		{
#ifdef PL_DHCPC_MODULE
			if(nsm_dhcp->type == DHCP_NONE && type == DHCP_CLIENT)
			{
				nsm_interface_dhcpc_enable(ifp,  TRUE);
				ifp->dhcp = TRUE;
			}
			if(nsm_dhcp->type == DHCP_CLIENT && type == DHCP_NONE)
			{
				nsm_interface_dhcpc_enable(ifp,  FALSE);
				ifp->dhcp = FALSE;
			}
#endif
#ifdef PL_DHCPC_MODULE
			if(nsm_dhcp->type == DHCP_NONE && type == DHCP_SERVER)
			{
				nsm_dhcps_t * pool = nsm_dhcps_lookup_api(name);
				if(pool)
				{
					nsm_interface_dhcps_enable(pool, ifp->ifindex, TRUE);
					nsm_dhcp->server = pool;
					ifp->dhcp = TRUE;
				}
				else
					return ERROR;
			}
			if(nsm_dhcp->type == DHCP_SERVER && type == DHCP_NONE)
			{
				nsm_dhcps_t * pool = nsm_dhcps_lookup_api(name);
				if(pool)
				{
					nsm_interface_dhcps_enable(pool, ifp->ifindex, FALSE);
					nsm_dhcp->server = pool;
					ifp->dhcp = FALSE;
				}
				else
					return ERROR;
			}
#endif
#ifdef PL_DHCPC_MODULE
			if(nsm_dhcp->type == DHCP_NONE && type == DHCP_RELAY)
			{
				//dhcpc_interface_enable_api(ifp,  TRUE);
				ifp->dhcp = TRUE;
			}
			if(nsm_dhcp->type == DHCP_RELAY && type == DHCP_NONE)
			{
				//dhcpc_interface_enable_api(ifp,  FALSE);
				ifp->dhcp = FALSE;
			}
#endif
			nsm_dhcp->type = type;
			return OK;
		}
	}
	return ERROR;
}

int nsm_interface_dhcp_config(struct vty *vty, struct interface *ifp)
{
	nsm_dhcp_ifp_t *nsm_dhcp = NULL;
	nsm_dhcp = nsm_dhcp_get(ifp);
	if(!nsm_dhcp)
		return OK;
	if (ifp->dhcp)
	{
#ifdef PL_DHCPC_MODULE
		if(nsm_dhcp->type == DHCP_CLIENT)
		{
			vty_out(vty, " ip address dhcp%s", VTY_NEWLINE);
			nsm_interface_dhcpc_write_config(ifp, vty);
		}
#endif
#ifdef PL_DHCPD_MODULE
		if(nsm_dhcp->type == DHCP_SERVER)
		{
			vty_out(vty, " dhcp select server%s", VTY_NEWLINE);
		}
#endif
#ifdef PL_DHCPR_MODULE
		if(nsm_dhcp->type == DHCP_RELAY)
		{
			vty_out(vty, " dhcp select relay%s", VTY_NEWLINE);
		}
#endif
	}
	return OK;
}


static int nsm_dhcp_client_init()
{
	struct nsm_client *nsm = nsm_client_new ();
	nsm->notify_add_cb = nsm_dhcp_add_interface;
	nsm->notify_delete_cb = nsm_dhcp_del_interface;
	//nsm->interface_write_config_cb = nsm_dhcp_interface_config;
	nsm_client_install (nsm, NSM_DHCP);
	return OK;
}

int nsm_dhcp_module_init ()
{
	nsm_dhcp_client_init();
#ifdef PL_DHCPD_MODULE
	nsm_dhcps_init();
	udhcp_module_init();

#endif
#ifdef PL_DHCPC_MODULE
	udhcp_module_init();
#endif
	return OK;
}

int nsm_dhcp_task_init ()
{
#ifdef PL_DHCPC_MODULE
	udhcp_module_task_init();
#endif
#ifdef PL_DHCPD_MODULE
	udhcp_module_task_init();
#endif
	return OK;
}

int nsm_dhcp_task_exit ()
{
#ifdef PL_DHCPC_MODULE
	udhcp_module_task_exit ();
#endif
#ifdef PL_DHCPD_MODULE
	udhcp_module_task_exit ();
#endif
	return OK;
}

int nsm_dhcp_module_exit ()
{
	struct nsm_client *nsm = nsm_client_lookup (NSM_DHCP);
	if(nsm)
		nsm_client_free (nsm);
#ifdef PL_DHCPC_MODULE
	udhcp_module_exit();
#endif
#ifdef PL_DHCPD_MODULE
	udhcp_module_exit();
	nsm_dhcps_exit();
#endif
	return OK;
}


#endif
