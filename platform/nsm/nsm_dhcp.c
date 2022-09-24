/*
 * nsm_dhcp.c
 *
 *  Created on: Sep 8, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "if.h"
#include "zmemory.h"
#include "nsm_interface.h"
#include "nsm_dhcp.h"

#ifdef ZPL_DHCP_MODULE
#include "dhcp_api.h"


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


int nsm_dhcp_interface_create_api(struct interface *ifp)
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


int nsm_dhcp_interface_del_api(struct interface *ifp)
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


int nsm_interface_dhcp_mode_set_api(struct interface *ifp, nsm_dhcp_type type, zpl_char *name)
{
	nsm_dhcp_ifp_t *nsm_dhcp = NULL;
	nsm_dhcp = nsm_dhcp_get(ifp);
	if(nsm_dhcp)
	{
		if(!ifp->dhcp)
		{
			ifp->dhcp = zpl_true;
		}
		if(nsm_dhcp->type != type)
		{
#ifdef ZPL_DHCPC_MODULE
			if(nsm_dhcp->type == DHCP_NONE && type == DHCP_CLIENT)
			{
				nsm_interface_dhcpc_enable(ifp,  zpl_true);
				ifp->dhcp = zpl_true;
			}
			if(nsm_dhcp->type == DHCP_CLIENT && type == DHCP_NONE)
			{
				nsm_interface_dhcpc_enable(ifp,  zpl_false);
				ifp->dhcp = zpl_false;
			}
#endif
#ifdef ZPL_DHCPC_MODULE
			if(nsm_dhcp->type == DHCP_NONE && type == DHCP_SERVER)
			{
				nsm_dhcps_t * pool = nsm_dhcps_lookup_api(name);
				if(pool)
				{
					nsm_interface_dhcps_enable(pool, ifp->ifindex, zpl_true);
					nsm_dhcp->server = pool;
					ifp->dhcp = zpl_true;
				}
				else
					return ERROR;
			}
			if(nsm_dhcp->type == DHCP_SERVER && type == DHCP_NONE)
			{
				nsm_dhcps_t * pool = nsm_dhcps_lookup_api(name);
				if(pool)
				{
					nsm_interface_dhcps_enable(pool, ifp->ifindex, zpl_false);
					nsm_dhcp->server = pool;
					ifp->dhcp = zpl_false;
				}
				else
					return ERROR;
			}
#endif
#ifdef ZPL_DHCPC_MODULE
			if(nsm_dhcp->type == DHCP_NONE && type == DHCP_RELAY)
			{
				//dhcpc_interface_enable_api(ifp,  zpl_true);
				ifp->dhcp = zpl_true;
			}
			if(nsm_dhcp->type == DHCP_RELAY && type == DHCP_NONE)
			{
				//dhcpc_interface_enable_api(ifp,  zpl_false);
				ifp->dhcp = zpl_false;
			}
#endif
			nsm_dhcp->type = type;
			return OK;
		}
	}
	return ERROR;
}
#ifdef ZPL_SHELL_MODULE
int nsm_interface_dhcp_config(struct vty *vty, struct interface *ifp)
{
	nsm_dhcp_ifp_t *nsm_dhcp = NULL;
	nsm_dhcp = nsm_dhcp_get(ifp);
	if(!nsm_dhcp)
		return OK;
	if (ifp->dhcp)
	{
#ifdef ZPL_DHCPC_MODULE
		if(nsm_dhcp->type == DHCP_CLIENT)
		{
			vty_out(vty, " ip address dhcp%s", VTY_NEWLINE);
			nsm_interface_dhcpc_write_config(ifp, vty);
		}
#endif
#ifdef ZPL_DHCPD_MODULE
		if(nsm_dhcp->type == DHCP_SERVER)
		{
			vty_out(vty, " dhcp select server%s", VTY_NEWLINE);
		}
#endif
#ifdef ZPL_DHCPR_MODULE
		if(nsm_dhcp->type == DHCP_RELAY)
		{
			vty_out(vty, " dhcp select relay%s", VTY_NEWLINE);
		}
#endif
	}
	return OK;
}
#endif


int nsm_dhcp_module_init (void)
{
	//nsm_interface_hook_add(NSM_INTF_DHCP, nsm_dhcp_interface_create_api, nsm_dhcp_interface_del_api);
#ifdef ZPL_DHCPD_MODULE
	nsm_dhcps_init();
	udhcp_module_init();

#endif
#ifdef ZPL_DHCPC_MODULE
	udhcp_module_init();
#endif
	return OK;
}

int nsm_dhcp_task_init (void)
{
#ifdef ZPL_DHCPC_MODULE
	udhcp_module_task_init();
#endif
#ifdef ZPL_DHCPD_MODULE
	udhcp_module_task_init();
#endif
	return OK;
}

int nsm_dhcp_task_exit (void)
{
#ifdef ZPL_DHCPC_MODULE
	udhcp_module_task_exit ();
#endif
#ifdef ZPL_DHCPD_MODULE
	udhcp_module_task_exit ();
#endif
	return OK;
}

int nsm_dhcp_module_exit (void)
{
#ifdef ZPL_DHCPC_MODULE
	udhcp_module_exit();
#endif
#ifdef ZPL_DHCPD_MODULE
	udhcp_module_exit();
	nsm_dhcps_exit();
#endif
	return OK;
}

struct module_list module_list_nsmdhcp = 
{ 
	.module=MODULE_DHCP, 
	.name="DHCP\0", 
	.module_init=nsm_dhcp_module_init, 
	.module_exit=nsm_dhcp_module_exit, 
	.module_task_init=nsm_dhcp_task_init, 
	.module_task_exit=nsm_dhcp_task_exit, 
	.module_cmd_init=cmd_dhcp_init, 
	.flags = 0,
	.taskid=0,
};

#endif
