/*
 * pal_driver.c
 *
 *  Created on: Jan 28, 2018
 *      Author: zhurish
 */




#include "zpl_include.h"
#include "zmemory.h"
#include "command.h"
#include "zmemory.h"
#include "memtypes.h"
#include "prefix.h"
#include "if.h"
#include "nsm_interface.h"
#include <log.h>
#include "pal_driver.h"

pal_stack_t pal_stack;

int pal_ip_stack_up(struct interface *ifp)
{
	if(pal_stack.ip_stack_up)
		return pal_stack.ip_stack_up(ifp);
    return ERROR;
}

int pal_ip_stack_down(struct interface *ifp)
{
	if(pal_stack.ip_stack_down)
		return pal_stack.ip_stack_down(ifp);
    return ERROR;
}

int pal_ip_stack_update_flag(struct interface *ifp)
{
	if(pal_stack.ip_stack_update_flag)
		return pal_stack.ip_stack_update_flag(ifp);
    return ERROR;
}

int pal_ip_stack_set_vr(struct interface *ifp, vrf_id_t vrf_id)
{
	if(pal_stack.ip_stack_set_vr)
		return pal_stack.ip_stack_set_vr(ifp, vrf_id);
    return ERROR;
}


int pal_ip_stack_set_mtu(struct interface *ifp, zpl_uint32 mtu)
{
	if(pal_stack.ip_stack_set_mtu)
		return pal_stack.ip_stack_set_mtu(ifp, mtu);
    return ERROR;
}

int pal_ip_stack_set_lladdr(struct interface *ifp, zpl_uint8 *mac, zpl_uint32 len)
{
	if(pal_stack.ip_stack_set_lladdr)
		return pal_stack.ip_stack_set_lladdr(ifp, mac, len);
    return ERROR;
}

int pal_ip_stack_create(struct interface *ifp)
{
	if(pal_stack.ip_stack_create)
		return pal_stack.ip_stack_create(ifp);
    return ERROR;
}

int pal_ip_stack_destroy(struct interface *ifp)
{
	if(pal_stack.ip_stack_destroy)
		return pal_stack.ip_stack_destroy(ifp);
    return ERROR;
}



int pal_ip_stack_vlan_set(struct interface *ifp, vlan_t vlan)
{
	if(pal_stack.ip_stack_set_vlan)
		return pal_stack.ip_stack_set_vlan(ifp, vlan);
    return ERROR;
}


int pal_ip_stack_vlanpri_set(struct interface *ifp, zpl_uint32 pri)
{
	if(pal_stack.ip_stack_set_vlanpri)
		return pal_stack.ip_stack_set_vlanpri(ifp, pri);
    return ERROR;
}

int pal_ip_stack_promisc_link(struct interface *ifp, zpl_bool enable)
{
	if(pal_stack.ip_stack_promisc)
		return pal_stack.ip_stack_promisc(ifp, enable);
    return ERROR;
}


int pal_ip_stack_change_dhcp(struct interface *ifp, zpl_bool enable)
{
	if(pal_stack.ip_stack_dhcp)
		return pal_stack.ip_stack_dhcp(ifp, enable);
    return ERROR;
}

int pal_ip_stack_ipv4_dstaddr_add(struct interface *ifp, struct connected *ifc)
{
	if(pal_stack.ip_stack_ipv4_dstaddr_add)
		return pal_stack.ip_stack_ipv4_dstaddr_add(ifp, ifc);
    return ERROR;
}

int pal_ip_stack_ipv4_dstaddr_delete(struct interface *ifp, struct connected *ifc)
{
	if(pal_stack.ip_stack_ipv4_dstaddr_del)
		return pal_stack.ip_stack_ipv4_dstaddr_del(ifp, ifc);
    return ERROR;
}

int pal_ip_stack_ipv4_replace(struct interface *ifp, struct connected *ifc)
{
	if(pal_stack.ip_stack_ipv4_replace)
		return pal_stack.ip_stack_ipv4_replace(ifp, ifc);
    return ERROR;
}

int pal_ip_stack_ipv4_add(struct interface *ifp,struct connected *ifc)
{
	if(pal_stack.ip_stack_ipv4_add)
		return pal_stack.ip_stack_ipv4_add(ifp, ifc);
    return ERROR;
}

int pal_ip_stack_ipv4_delete(struct interface *ifp, struct connected *ifc)
{
	if(pal_stack.ip_stack_ipv4_delete)
		return pal_stack.ip_stack_ipv4_delete(ifp, ifc);
    return ERROR;
}

// ip arp
int pal_ip_stack_arp_add(struct interface *ifp, struct prefix *address, zpl_uint8 *mac)
{
	if(pal_stack.ip_stack_arp_add)
		return pal_stack.ip_stack_arp_add(ifp, address, mac);
    return ERROR;
}

int pal_ip_stack_arp_delete(struct interface *ifp, struct prefix *address)
{
	if(pal_stack.ip_stack_arp_delete)
		return pal_stack.ip_stack_arp_delete(ifp, address);
    return ERROR;
}

int pal_ip_stack_arp_request(struct interface *ifp, struct prefix *address)
{
	if(pal_stack.ip_stack_arp_request)
		return pal_stack.ip_stack_arp_request(ifp, address);
    return ERROR;
}

int pal_ip_stack_arp_gratuitousarp_enable(zpl_bool enable)
{
	if(pal_stack.ip_stack_arp_gratuitousarp_enable)
		return pal_stack.ip_stack_arp_gratuitousarp_enable(enable);
    return ERROR;
}

int pal_ip_stack_arp_ttl(zpl_uint32 ttl)
{
	if(pal_stack.ip_stack_arp_ttl)
		return pal_stack.ip_stack_arp_ttl(ttl);
    return ERROR;
}

int pal_ip_stack_arp_age_timeout(zpl_uint32 timeout)
{
	if(pal_stack.ip_stack_arp_age_timeout)
		return pal_stack.ip_stack_arp_age_timeout(timeout);
    return ERROR;
}

int pal_ip_stack_arp_retry_interval(zpl_uint32 interval)
{
	if(pal_stack.ip_stack_arp_retry_interval)
		return pal_stack.ip_stack_arp_retry_interval(interval);
    return ERROR;
}

//route
int pal_ip_stack_create_vr(vrf_id_t vrf_id)
{
	if(pal_stack.ip_stack_vrf_create)
		return pal_stack.ip_stack_vrf_create(vrf_id);
    return ERROR;
}

int pal_ip_stack_delete_vr(vrf_id_t vrf_id)
{
	if(pal_stack.ip_stack_vrf_delete)
		return pal_stack.ip_stack_vrf_delete(vrf_id);
    return ERROR;
}

int pal_ip_stack_init()
{
	ip_ifp_stack_init();
	return 0;//ip_stack_init();
}
