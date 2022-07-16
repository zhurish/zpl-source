/*
 * pal_router.c
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */


#include "zplos_include.h"
#include "nsm_include.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"
#include "pal_router.h"
#include "pal_global.h"

//route
int pal_create_vrf(struct ip_vrf *vrf)
{
	if(pal_stack.ip_stack_vrf_create)
		return pal_stack.ip_stack_vrf_create(vrf);
    return OK;
}

int pal_delete_vrf(struct ip_vrf *vrf)
{
	if(pal_stack.ip_stack_vrf_delete)
		return pal_stack.ip_stack_vrf_delete(vrf);
    return OK;
}

int pal_iproute_rib_action(struct prefix *p, struct rib *old, struct rib *new)
{
	if(pal_stack.ip_stack_route_rib && p)
		return pal_stack.ip_stack_route_rib(p, old, new);
    return OK;
}