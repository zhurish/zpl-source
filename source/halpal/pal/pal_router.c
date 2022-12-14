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


int pal_route_rib_add(zpl_uint8 processid, safi_t safi, struct prefix *p,
						struct rib *rib, zpl_uint8 num)
{
	if(pal_stack.ip_stack_route_rib_add && p)
		return pal_stack.ip_stack_route_rib_add(processid, safi, p, rib, num);
    return OK;
}

int pal_route_rib_del(zpl_uint8 processid, safi_t safi, struct prefix *p,
						struct rib *rib, zpl_uint8 num)
{
	if(pal_stack.ip_stack_route_rib_del && p)
		return pal_stack.ip_stack_route_rib_del(processid, safi, p, rib, num);
    return OK;
}

