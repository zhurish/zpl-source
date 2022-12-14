/*
 * pal_interface.c
 *
 *  Created on: Jan 28, 2018
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
#include "thread.h"
#include "eloop.h"
#include "pal_interface.h"

pal_stack_t pal_stack;
static int kernel_task_id = 0;


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

int pal_interface_update_flag(struct interface *ifp, int flags)
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

int pal_interface_set_vr(struct interface *ifp, vrf_id_t vrf_id)
{
	if(pal_stack.ip_stack_set_vr && ifp->k_ifindex)
		return pal_stack.ip_stack_set_vr(ifp, vrf_id);
    return OK;
}


int pal_interface_set_mtu(struct interface *ifp, int mtu)
{
	if(pal_stack.ip_stack_set_mtu && ifp->k_ifindex)
		return pal_stack.ip_stack_set_mtu(ifp, mtu);
    return OK;
}

int pal_interface_set_lladdr(struct interface *ifp, unsigned char *mac, int len)
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

int pal_interface_vlan_set(struct interface *ifp, int vlan)
{
	if(pal_stack.ip_stack_set_vlan)
		return pal_stack.ip_stack_set_vlan(ifp, vlan);
    return OK;
}


int pal_interface_vlanpri_set(struct interface *ifp, int pri)
{
	if(pal_stack.ip_stack_set_vlanpri && ifp->k_ifindex)
		return pal_stack.ip_stack_set_vlanpri(ifp, pri);
    return OK;
}

int pal_interface_promisc_link(struct interface *ifp, BOOL enable)
{
	if(pal_stack.ip_stack_promisc && ifp->k_ifindex)
		return pal_stack.ip_stack_promisc(ifp, enable);
    return OK;
}


int pal_interface_change_dhcp(struct interface *ifp, BOOL enable)
{
	if(pal_stack.ip_stack_dhcp && ifp->k_ifindex)
		return pal_stack.ip_stack_dhcp(ifp, enable);
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

int pal_interface_ipv6_add(struct interface *ifp,struct connected *ifc, int secondry)
{
	if(pal_stack.ip_stack_ipv6_add && ifp->k_ifindex)
		return pal_stack.ip_stack_ipv6_add(ifp, ifc, secondry);
    return OK;
}

int pal_interface_ipv6_delete(struct interface *ifp, struct connected *ifc, int secondry)
{
	if(pal_stack.ip_stack_ipv6_delete && ifp->k_ifindex)
		return pal_stack.ip_stack_ipv6_delete(ifp, ifc, secondry);
    return OK;
}

// ip arp
int pal_interface_arp_add(struct interface *ifp, struct prefix *address, unsigned char *mac)
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

int pal_arp_gratuitousarp_enable(int enable)
{
	if(pal_stack.ip_stack_arp_gratuitousarp_enable)
		return pal_stack.ip_stack_arp_gratuitousarp_enable(enable);
    return OK;
}

int pal_arp_ttl(int ttl)
{
	if(pal_stack.ip_stack_arp_ttl)
		return pal_stack.ip_stack_arp_ttl(ttl);
    return OK;
}

int pal_arp_age_timeout(int timeout)
{
	if(pal_stack.ip_stack_arp_age_timeout)
		return pal_stack.ip_stack_arp_age_timeout(timeout);
    return OK;
}

int pal_arp_retry_interval(int interval)
{
	if(pal_stack.ip_stack_arp_retry_interval)
		return pal_stack.ip_stack_arp_retry_interval(interval);
    return OK;
}

//route
int pal_create_vr(vrf_id_t vrf_id)
{
	if(pal_stack.ip_stack_vrf_create)
		return pal_stack.ip_stack_vrf_create(vrf_id);
    return OK;
}

int pal_delete_vr(vrf_id_t vrf_id)
{
	if(pal_stack.ip_stack_vrf_delete)
		return pal_stack.ip_stack_vrf_delete(vrf_id);
    return OK;
}

int pal_interface_update_statistics(struct interface *ifp)
{
	if(pal_stack.ip_stack_update_statistics && ifp->k_ifindex)
		return pal_stack.ip_stack_update_statistics(ifp);
    return OK;
}


/***************************************************************/

static int pal_abstract_task(void *argv)
{
	module_setup_task(MODULE_KERNEL, os_task_id_self());
	while(!os_load_config_done())
	{
		os_sleep(1);
	}
	eloop_start_running(master_eloop[MODULE_KERNEL], MODULE_KERNEL);
	return OK;
}


static int kernel_task_init ()
{
	if(master_eloop[MODULE_KERNEL] == NULL)
		master_eloop[MODULE_KERNEL] = eloop_master_module_create(MODULE_KERNEL);
	//master_thread[MODULE_TELNET] = thread_master_module_create(MODULE_TELNET);
	kernel_task_id = os_task_create("kernelTask", OS_TASK_DEFAULT_PRIORITY,
	               0, pal_abstract_task, NULL, OS_TASK_DEFAULT_STACK);
	if(kernel_task_id)
		return OK;
	return ERROR;
}

int pal_abstract_init()
{
	if(master_eloop[MODULE_KERNEL] == NULL)
		master_eloop[MODULE_KERNEL] = eloop_master_module_create(MODULE_KERNEL);
	kernel_task_init ();
	ip_ifp_stack_init();
	ip_arp_stack_init();
	return 0;//ip_stack_init();
}
