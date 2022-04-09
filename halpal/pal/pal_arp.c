/*
 * pal_arp.c
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */


#include "zplos_include.h"
#include "nsm_include.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"
#include "pal_arp.h"
#include "pal_global.h"

#ifdef ZPL_NSM_ARP
// ip arp
int pal_interface_arp_add(struct interface *ifp, struct prefix *address, zpl_uint8 *mac)
{
	if(pal_stack.ip_stack_arp_add && ifp->k_ifindex)
		return pal_stack.ip_stack_arp_add(ifp, address, mac);
    return OK;
}

int pal_interface_arp_delete(struct interface *ifp, struct prefix *address)
{
	if(pal_stack.ip_stack_arp_delete && ifp->k_ifindex)
		return pal_stack.ip_stack_arp_delete(ifp, address);
    return OK;
}

int pal_interface_arp_request(struct interface *ifp, struct prefix *address)
{
	if(pal_stack.ip_stack_arp_request && ifp->k_ifindex)
		return pal_stack.ip_stack_arp_request(ifp, address);
    return OK;
}

int pal_arp_gratuitousarp_enable(zpl_bool enable)
{
	if(pal_stack.ip_stack_arp_gratuitousarp_enable)
		return pal_stack.ip_stack_arp_gratuitousarp_enable(enable);
    return OK;
}

int pal_arp_ttl(zpl_uint32 ttl)
{
	if(pal_stack.ip_stack_arp_ttl)
		return pal_stack.ip_stack_arp_ttl(ttl);
    return OK;
}

int pal_arp_age_timeout(zpl_uint32 timeout)
{
	if(pal_stack.ip_stack_arp_age_timeout)
		return pal_stack.ip_stack_arp_age_timeout(timeout);
    return OK;
}

int pal_arp_retry_interval(zpl_uint32 interval)
{
	if(pal_stack.ip_stack_arp_retry_interval)
		return pal_stack.ip_stack_arp_retry_interval(interval);
    return OK;
}
#endif