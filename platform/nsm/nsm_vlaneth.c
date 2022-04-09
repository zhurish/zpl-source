/*
 * nsm_vlaneth.c
 *
 *  Created on: Sep 13, 2018
 *      Author: zhurish
 */
#include "auto_include.h"
#include "zplos_include.h"
#include "if.h"
#include "vty.h"
#include "zmemory.h"
#include "template.h"
#include "nsm_include.h"
#include "hal_include.h"


/*
 * ip_iface.c
 */

nsm_vlaneth_t * nsm_vlaneth_get(struct interface *ifp)
{
	return (nsm_vlaneth_t *)nsm_intf_module_data(ifp, NSM_INTF_VETH);
}


int nsm_vlaneth_interface_create_api(struct interface *ifp)
{
	nsm_vlaneth_t * vlaneth = nsm_intf_module_data(ifp, NSM_INTF_VETH);

	if(if_is_serial(ifp) || (if_is_ethernet(ifp) && IF_ID_GET(ifp->ifindex)))
	{
		if(!vlaneth)
			vlaneth = XMALLOC(MTYPE_IF, sizeof(nsm_vlaneth_t));
		zassert(vlaneth);
		os_memset(vlaneth, 0, sizeof(nsm_vlaneth_t));
		vlaneth->ifp = ifp;
		if(IF_IS_SUBIF_GET(ifp->ifindex))
		{
			vlaneth->root = if_lookup_by_index(IF_IFINDEX_ROOT_GET(ifp->ifindex));
		}
		nsm_intf_module_data_set(ifp, NSM_INTF_VETH, vlaneth);
	}
	return OK;
}


int nsm_vlaneth_interface_del_api(struct interface *ifp)
{
	nsm_vlaneth_t * vlaneth = nsm_vlaneth_get(ifp);
	if(vlaneth)
	{
		if(if_is_serial(ifp) || (if_is_ethernet(ifp) && IF_ID_GET(ifp->ifindex)))
		{
			struct nsm_interface *nsm = ifp->info[MODULE_NSM];
			XFREE(MTYPE_IF, vlaneth);
			vlaneth = NULL;
			nsm_intf_module_data_set(ifp, NSM_INTF_VETH, NULL);
		}
	}
	return OK;
}

int nsm_vlaneth_interface_vid_set_api(struct interface *ifp, vlan_t vlan)
{
	//nsm_vlaneth_t * vlaneth = NULL;
	if(if_is_serial(ifp) || (if_is_ethernet(ifp) && IF_ID_GET(ifp->ifindex)))
	{
		nsm_vlaneth_t * vlaneth = nsm_vlaneth_get(ifp);
		if(vlaneth)
		{
#ifdef ZPL_HAL_MODULE
			if(nsm_halpal_interface_vlan_set(ifp, vlan) == OK)
#endif
			{
				vlaneth->vlanid = vlan;
				return OK;
			}
		}
	}
	return ERROR;
}

int nsm_vlaneth_init(void)
{
	nsm_interface_hook_add(NSM_INTF_VETH, nsm_vlaneth_interface_create_api, nsm_vlaneth_interface_del_api);
	return OK;
}

int nsm_vlaneth_exit(void)
{
	return OK;
}
