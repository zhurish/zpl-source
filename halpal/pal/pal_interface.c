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
	if(pal_stack.ip_stack_up/* && ifp->ker_ifindex*/)
		return pal_stack.ip_stack_up(ifp);
    return OK;
}

int pal_interface_down(struct interface *ifp)
{
	if(pal_stack.ip_stack_down/* && ifp->ker_ifindex*/)
		return pal_stack.ip_stack_down(ifp);
    return OK;
}

int pal_interface_update_flag(struct interface *ifp, zpl_uint32 flags)
{
	if(pal_stack.ip_stack_update_flag/* && ifp->ker_ifindex*/)
		return pal_stack.ip_stack_update_flag(ifp, flags);
    return OK;
}

int pal_interface_refresh_flag(struct interface *ifp)
{
	if(pal_stack.ip_stack_refresh_flag/* && ifp->ker_ifindex*/)
		return pal_stack.ip_stack_refresh_flag(ifp);
    return OK;
}

int pal_interface_ifindex(char *ker_name)
{
	if(pal_stack.ip_stack_ifindex)
		return pal_stack.ip_stack_ifindex(ker_name);
    return OK;
}

int pal_interface_set_vrf(struct interface *ifp, struct ip_vrf *vrf)
{
	if(pal_stack.ip_stack_set_vrf && ifp->ker_ifindex)
		return pal_stack.ip_stack_set_vrf(ifp, vrf);
    return OK;
}

int pal_interface_unset_vrf(struct interface *ifp, struct ip_vrf *vrf)
{
	if(pal_stack.ip_stack_unset_vrf && ifp->ker_ifindex)
		return pal_stack.ip_stack_unset_vrf(ifp, vrf);
    return OK;
}

int pal_interface_set_mtu(struct interface *ifp, zpl_uint32 mtu)
{
	if(pal_stack.ip_stack_set_mtu && ifp->ker_ifindex)
		return pal_stack.ip_stack_set_mtu(ifp, mtu);
    return OK;
}

int pal_interface_set_lladdr(struct interface *ifp, zpl_uint8 *mac, zpl_uint32 len)
{
	if(pal_stack.ip_stack_set_lladdr && ifp->ker_ifindex)
		return pal_stack.ip_stack_set_lladdr(ifp, mac, len);
    return OK;
}

int pal_interface_get_lladdr(struct interface *ifp)
{
	if(pal_stack.ip_stack_get_lladdr && ifp->ker_ifindex)
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
	if(pal_stack.ip_stack_destroy && ifp->ker_ifindex)
		return pal_stack.ip_stack_destroy(ifp);
    return OK;
}


int pal_interface_update(struct interface *ifp)
{
	if(pal_stack.ip_stack_update && ifp->ker_ifindex)
		return pal_stack.ip_stack_update(ifp);
    return OK;
}

int pal_interface_add_slave(struct interface *ifp, struct interface *sifp)
{
	if(pal_stack.ip_stack_member_add && ifp->ker_ifindex && sifp->ker_ifindex)
		return pal_stack.ip_stack_member_add(ifp, sifp);
    return OK;
}

int pal_interface_del_slave(struct interface *ifp, struct interface *sifp)
{
	if(pal_stack.ip_stack_member_del && ifp->ker_ifindex && sifp->ker_ifindex)
		return pal_stack.ip_stack_member_del(ifp, sifp);
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
	if(pal_stack.ip_stack_set_vlanpri && ifp->ker_ifindex)
		return pal_stack.ip_stack_set_vlanpri(ifp, pri);
    return OK;
}


int pal_interface_ipv4_dstaddr_add(struct interface *ifp, struct connected *ifc)
{
	if(pal_stack.ip_stack_ipv4_dstaddr_add && ifp->ker_ifindex)
		return pal_stack.ip_stack_ipv4_dstaddr_add(ifp, ifc);
    return OK;
}

int pal_interface_ipv4_dstaddr_delete(struct interface *ifp, struct connected *ifc)
{
	if(pal_stack.ip_stack_ipv4_dstaddr_del && ifp->ker_ifindex)
		return pal_stack.ip_stack_ipv4_dstaddr_del(ifp, ifc);
    return OK;
}

int pal_interface_ipv4_replace(struct interface *ifp, struct connected *ifc)
{
	if(pal_stack.ip_stack_ipv4_replace && ifp->ker_ifindex)
		return pal_stack.ip_stack_ipv4_replace(ifp, ifc);
    return OK;
}

int pal_interface_ipv4_add(struct interface *ifp,struct connected *ifc)
{
	if(pal_stack.ip_stack_ipv4_add && ifp->ker_ifindex)
		return pal_stack.ip_stack_ipv4_add(ifp, ifc);
    return OK;
}

int pal_interface_ipv4_delete(struct interface *ifp, struct connected *ifc)
{
	if(pal_stack.ip_stack_ipv4_delete && ifp->ker_ifindex)
		return pal_stack.ip_stack_ipv4_delete(ifp, ifc);
    return OK;
}

int pal_interface_ipv6_add(struct interface *ifp,struct connected *ifc, zpl_bool secondry)
{
	if(pal_stack.ip_stack_ipv6_add && ifp->ker_ifindex)
		return pal_stack.ip_stack_ipv6_add(ifp, ifc, secondry);
    return OK;
}

int pal_interface_ipv6_delete(struct interface *ifp, struct connected *ifc, zpl_bool secondry)
{
	if(pal_stack.ip_stack_ipv6_delete && ifp->ker_ifindex)
		return pal_stack.ip_stack_ipv6_delete(ifp, ifc, secondry);
    return OK;
}


