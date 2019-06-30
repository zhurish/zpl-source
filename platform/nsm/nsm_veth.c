/*
 * nsm_veth.c
 *
 *  Created on: Sep 13, 2018
 *      Author: zhurish
 */
#include "zebra.h"
#include "memory.h"
#include "command.h"
#include "memory.h"
#include "memtypes.h"
#include "prefix.h"
#include "if.h"
#include "interface.h"
#include <log.h>
#include "nsm_client.h"
#include "os_list.h"
#include "tty_com.h"
//#include "NSM_VETH.h"
#include "nsm_veth.h"


/*
 * ip_iface.c
 */

nsm_veth_t * nsm_veth_get(struct interface *ifp)
{
	struct nsm_interface *nsm = ifp->info[MODULE_NSM];
	return (nsm_veth_t *)nsm->nsm_client[NSM_VETH];
}


static int nsm_veth_add_interface(struct interface *ifp)
{
	nsm_veth_t * veth = NULL;
	struct nsm_interface *nsm = ifp->info[MODULE_NSM];
	if(if_is_serial(ifp) || (if_is_ethernet(ifp) && IF_ID_GET(ifp->ifindex)))
	{
		if(!nsm->nsm_client[NSM_VETH])
			nsm->nsm_client[NSM_VETH] = XMALLOC(MTYPE_IF, sizeof(nsm_veth_t));
		zassert(nsm->nsm_client[NSM_VETH]);
		os_memset(nsm->nsm_client[NSM_VETH], 0, sizeof(nsm_veth_t));
		veth = nsm->nsm_client[NSM_VETH];
		veth->ifp = ifp;
		if(IF_IS_SUBIF_GET(ifp->ifindex))
		{
			veth->root = if_lookup_by_index(IF_IFINDEX_ROOT_GET(ifp->ifindex));
		}
	}
	return OK;
}


static int nsm_veth_del_interface(struct interface *ifp)
{
	nsm_veth_t * veth = nsm_veth_get(ifp);
	if(veth)
	{
		if(if_is_serial(ifp) || (if_is_ethernet(ifp) && IF_ID_GET(ifp->ifindex)))
		{
			struct nsm_interface *nsm = ifp->info[MODULE_NSM];
			XFREE(MTYPE_IF, veth);
			nsm->nsm_client[NSM_VETH] = NULL;
		}
	}
	return OK;
}

int nsm_veth_interface_vid_set_api(struct interface *ifp, int vlan)
{
	//nsm_veth_t * veth = NULL;
	if(if_is_serial(ifp) || (if_is_ethernet(ifp) && IF_ID_GET(ifp->ifindex)))
	{
		nsm_veth_t * veth = nsm_veth_get(ifp);
		if(veth)
		{
#ifdef PL_HAL_MODULE
			if(pal_interface_vlan_set(ifp, vlan) == OK)
#endif
			{
				veth->vlanid = vlan;
				return OK;
			}
		}
	}
	return ERROR;
}

int nsm_veth_client_init()
{
	struct nsm_client *nsm = nsm_client_new ();
	nsm->notify_add_cb = nsm_veth_add_interface;
	nsm->notify_delete_cb = nsm_veth_del_interface;
	nsm->interface_write_config_cb = NULL;//nsm_veth_interface_write_config;
	nsm_client_install (nsm, NSM_VETH);
	return OK;
}

int nsm_veth_client_exit()
{
	struct nsm_client *nsm = nsm_client_lookup (NSM_VETH);
	if(nsm)
		nsm_client_free (nsm);
	return OK;
}
