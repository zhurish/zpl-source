/*
 * nsm_vlaneth.c
 *
 *  Created on: Sep 13, 2018
 *      Author: zhurish
 */
#include "auto_include.h"
#include "zpl_type.h"
#include "module.h"
#include "route_types.h"
#include "zmemory.h"
#include "prefix.h"
#include "log.h"
#include "template.h"
#include "zclient.h"
#ifdef ZPL_SHELL_MODULE
#include "vty_include.h"
#endif
#include "nsm_interface.h"
#include "nsm_vlaneth.h"
#include "hal_include.h"




nsm_vlaneth_t * nsm_vlaneth_get(struct interface *ifp)
{
	return (nsm_vlaneth_t *)nsm_intf_module_data(ifp, NSM_INTF_VLANETH);
}


int nsm_vlaneth_interface_create_api(struct interface *ifp)
{
	nsm_vlaneth_t * vlaneth = nsm_intf_module_data(ifp, NSM_INTF_VLANETH);

	if((if_is_vlan(ifp) && IF_ID_GET(ifp->ifindex)))
	{
		if(!vlaneth)
			vlaneth = XMALLOC(MTYPE_IF_DATA, sizeof(nsm_vlaneth_t));
		zassert(vlaneth);
		os_memset(vlaneth, 0, sizeof(nsm_vlaneth_t));
		vlaneth->ifp = ifp;

		nsm_intf_module_data_set(ifp, NSM_INTF_VLANETH, vlaneth);
#ifdef ZPL_HAL_MODULE
		if(nsm_halpal_interface_add(ifp) == OK)
			return OK;
		else
		{
			nsm_intf_module_data_set(ifp, NSM_INTF_VLANETH, NULL);
			XFREE(MTYPE_IF_DATA, vlaneth);
			return ERROR;
		}	
#endif
	}
	return OK;
}


int nsm_vlaneth_interface_del_api(struct interface *ifp)
{
	nsm_vlaneth_t * vlaneth = nsm_vlaneth_get(ifp);
	if(vlaneth)
	{
		if((if_is_vlan(ifp) && IF_ID_GET(ifp->ifindex)))
		{
#ifdef ZPL_HAL_MODULE
			if(nsm_halpal_interface_delete(ifp) == OK)
			{
				XFREE(MTYPE_IF_DATA, vlaneth);
				vlaneth = NULL;
				nsm_intf_module_data_set(ifp, NSM_INTF_VLANETH, NULL);
				return OK;
			}
			else
			{
				return ERROR;
			}	
#endif
		}
	}
	return OK;
}

int nsm_vlaneth_init(void)
{
	nsm_interface_hook_add(NSM_INTF_VLANETH, nsm_vlaneth_interface_create_api, nsm_vlaneth_interface_del_api);
	return OK;
}

int nsm_vlaneth_exit(void)
{
	return OK;
}
