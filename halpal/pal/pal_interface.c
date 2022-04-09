/*
 * pal_interface.c
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */


#include "zplos_include.h"
#include "nsm_include.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"
#include "pal_interface.h"
#include "pal_global.h"

int pal_interface_up(struct interface *ifp)
{
	if(pal_stack.ip_stack_up/* && ifp->k_ifindex*/)
		return pal_stack.ip_stack_up(ifp);
    return OK;
}

int pal_interface_down(struct interface *ifp)
{
	if(pal_stack.ip_stack_down/* && ifp->k_ifindex*/)
		return pal_stack.ip_stack_down(ifp);
    return OK;
}

int pal_interface_update_flag(struct interface *ifp, zpl_uint32 flags)
{
	if(pal_stack.ip_stack_update_flag/* && ifp->k_ifindex*/)
		return pal_stack.ip_stack_update_flag(ifp, flags);
    return OK;
}

int pal_interface_refresh_flag(struct interface *ifp)
{
	if(pal_stack.ip_stack_refresh_flag/* && ifp->k_ifindex*/)
		return pal_stack.ip_stack_refresh_flag(ifp);
    return OK;
}

int pal_interface_ifindex(char *k_name)
{
	if(pal_stack.ip_stack_ifindex)
		return pal_stack.ip_stack_ifindex(k_name);
    return OK;
}

int pal_interface_set_vrf(struct interface *ifp, struct ip_vrf *vrf)
{
	if(pal_stack.ip_stack_set_vrf && ifp->k_ifindex)
		return pal_stack.ip_stack_set_vrf(ifp, vrf);
    return OK;
}


int pal_interface_set_mtu(struct interface *ifp, zpl_uint32 mtu)
{
	if(pal_stack.ip_stack_set_mtu && ifp->k_ifindex)
		return pal_stack.ip_stack_set_mtu(ifp, mtu);
    return OK;
}

int pal_interface_set_lladdr(struct interface *ifp, zpl_uint8 *mac, zpl_uint32 len)
{
	if(pal_stack.ip_stack_set_lladdr && ifp->k_ifindex)
		return pal_stack.ip_stack_set_lladdr(ifp, mac, len);
    return OK;
}

int pal_interface_get_lladdr(struct interface *ifp)
{
	if(pal_stack.ip_stack_get_lladdr && ifp->k_ifindex)
		return pal_stack.ip_stack_get_lladdr(ifp);
    return OK;
}

int pal_interface_create(struct interface *ifp)
{
	if(pal_stack.ip_stack_create)
		return pal_stack.ip_stack_create(ifp);
    return OK;
}

int pal_interface_destroy(struct interface *ifp)
{
	if(pal_stack.ip_stack_destroy && ifp->k_ifindex)
		return pal_stack.ip_stack_destroy(ifp);
    return OK;
}


int pal_interface_change(struct interface *ifp)
{
	if(pal_stack.ip_stack_change && ifp->k_ifindex)
		return pal_stack.ip_stack_change(ifp);
    return OK;
}

int pal_interface_vlan_set(struct interface *ifp, vlan_t vlan)
{
	if(pal_stack.ip_stack_set_vlan)
		return pal_stack.ip_stack_set_vlan(ifp, vlan);
    return OK;
}


int pal_interface_vlanpri_set(struct interface *ifp, zpl_uint32 pri)
{
	if(pal_stack.ip_stack_set_vlanpri && ifp->k_ifindex)
		return pal_stack.ip_stack_set_vlanpri(ifp, pri);
    return OK;
}

int pal_interface_promisc_link(struct interface *ifp, zpl_bool enable)
{
	if(pal_stack.ip_stack_promisc && ifp->k_ifindex)
		return pal_stack.ip_stack_promisc(ifp, enable);
    return OK;
}



int pal_interface_ipv4_dstaddr_add(struct interface *ifp, struct connected *ifc)
{
	if(pal_stack.ip_stack_ipv4_dstaddr_add && ifp->k_ifindex)
		return pal_stack.ip_stack_ipv4_dstaddr_add(ifp, ifc);
    return OK;
}

int pal_interface_ipv4_dstaddr_delete(struct interface *ifp, struct connected *ifc)
{
	if(pal_stack.ip_stack_ipv4_dstaddr_del && ifp->k_ifindex)
		return pal_stack.ip_stack_ipv4_dstaddr_del(ifp, ifc);
    return OK;
}

int pal_interface_ipv4_replace(struct interface *ifp, struct connected *ifc)
{
	if(pal_stack.ip_stack_ipv4_replace && ifp->k_ifindex)
		return pal_stack.ip_stack_ipv4_replace(ifp, ifc);
    return OK;
}

int pal_interface_ipv4_add(struct interface *ifp,struct connected *ifc)
{
	if(pal_stack.ip_stack_ipv4_add && ifp->k_ifindex)
		return pal_stack.ip_stack_ipv4_add(ifp, ifc);
    return OK;
}

int pal_interface_ipv4_delete(struct interface *ifp, struct connected *ifc)
{
	if(pal_stack.ip_stack_ipv4_delete && ifp->k_ifindex)
		return pal_stack.ip_stack_ipv4_delete(ifp, ifc);
    return OK;
}

int pal_interface_ipv6_add(struct interface *ifp,struct connected *ifc, zpl_bool secondry)
{
	if(pal_stack.ip_stack_ipv6_add && ifp->k_ifindex)
		return pal_stack.ip_stack_ipv6_add(ifp, ifc, secondry);
    return OK;
}

int pal_interface_ipv6_delete(struct interface *ifp, struct connected *ifc, zpl_bool secondry)
{
	if(pal_stack.ip_stack_ipv6_delete && ifp->k_ifindex)
		return pal_stack.ip_stack_ipv6_delete(ifp, ifc, secondry);
    return OK;
}


int pal_interface_update_statistics(struct interface *ifp)
{
	if(pal_stack.ip_stack_update_statistics && ifp->k_ifindex)
		return pal_stack.ip_stack_update_statistics(ifp);
    return OK;
}