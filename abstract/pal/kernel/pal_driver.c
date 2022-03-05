/*
 * pal_driver.c
 *
 *  Created on: Jan 28, 2018
 *      Author: zhurish
 */




#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"

#include "pal_driver.h"

pal_stack_t pal_stack;
static zpl_uint32 kernel_task_id = 0;
static void *master_eloop_kernel = NULL;
static int pal_module_task_init(void);

struct module_list module_list_pal = 
{ 
	.module=MODULE_PAL, 
	.name="PAL", 
	.module_init=pal_module_init, 
	.module_exit=NULL, 
	.module_task_init=pal_module_task_init, 
	.module_task_exit=NULL, 
	.module_cmd_init=NULL, 
	.module_write_config=NULL, 
	.module_show_config=NULL,
	.module_show_debug=NULL, 
	.taskid=0,
	.flags = 0,
};

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

int pal_interface_set_vr(struct interface *ifp, vrf_id_t vrf_id)
{
	if(pal_stack.ip_stack_set_vr && ifp->k_ifindex)
		return pal_stack.ip_stack_set_vr(ifp, vrf_id);
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


int pal_interface_change_dhcp(struct interface *ifp, zpl_bool enable)
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

int pal_iproute_rib_action(struct prefix *p, struct rib *old, struct rib *new)
{
	if(pal_stack.ip_stack_route_rib && p)
		return pal_stack.ip_stack_route_rib(p, old, new);
    return OK;
}
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

/*
 * 端口映射
 */
int pal_firewall_portmap_rule_set(firewall_t *rule, zpl_action action)
{
	if(pal_stack.ip_stack_firewall_portmap_rule_set)
		return pal_stack.ip_stack_firewall_portmap_rule_set(rule, action);
    return OK;
}
/*
 * 端口开放
 */
int pal_firewall_port_filter_rule_set(firewall_t *rule, zpl_action action)
{
	if(pal_stack.ip_stack_firewall_port_filter_rule_set)
		return pal_stack.ip_stack_firewall_port_filter_rule_set(rule, action);
    return OK;
}

int pal_firewall_mangle_rule_set(firewall_t *rule, zpl_action action)
{
	if(pal_stack.ip_stack_firewall_mangle_rule_set)
		return pal_stack.ip_stack_firewall_mangle_rule_set(rule, action);
    return OK;
}

int pal_firewall_raw_rule_set(firewall_t *rule, zpl_action action)
{
	if(pal_stack.ip_stack_firewall_raw_rule_set)
		return pal_stack.ip_stack_firewall_raw_rule_set(rule, action);
    return OK;
}

int pal_firewall_snat_rule_set(firewall_t *rule, zpl_action action)
{
	if(pal_stack.ip_stack_firewall_snat_rule_set)
		return pal_stack.ip_stack_firewall_snat_rule_set(rule, action);
    return OK;
}

int pal_firewall_dnat_rule_set(firewall_t *rule, zpl_action action)
{
	if(pal_stack.ip_stack_firewall_dnat_rule_set)
		return pal_stack.ip_stack_firewall_dnat_rule_set(rule, action);
    return OK;
}


int ipforward (void)
{
	return _ipkernel_ipforward();
}
int ipforward_on (void)
{
	return _ipkernel_ipforward_on();
}
int ipforward_off (void)
{
	return _ipkernel_ipforward_off();
}

int ipforward_ipv6 (void)
{
	return _ipkernel_ipforward_ipv6();
}
int ipforward_ipv6_on (void)
{
	return _ipkernel_ipforward_ipv6_on();
}
int ipforward_ipv6_off (void)
{
	return _ipkernel_ipforward_ipv6_off();
}

/***************************************************************/

static int pal_main_task(void *argv)
{
	module_setup_task(MODULE_PAL, os_task_id_self());
	host_waitting_loadconfig();
	eloop_mainloop(master_eloop_kernel);
	return OK;
}


static int kernel_task_init (void)
{
	if(master_eloop_kernel == NULL)
		master_eloop_kernel = eloop_master_module_create(MODULE_PAL);
	//master_thread[ZPL_SERVICE_TELNET] = thread_master_module_create(ZPL_SERVICE_TELNET);
	if(kernel_task_id == 0)
		kernel_task_id = os_task_create("kernelTask", OS_TASK_DEFAULT_PRIORITY,
	               0, pal_main_task, NULL, OS_TASK_DEFAULT_STACK);
	if(kernel_task_id)
	{
		module_setup_task(MODULE_PAL, kernel_task_id);
		return OK;
	}
	return ERROR;
}

int pal_module_init(void)
{
	if(master_eloop_kernel == NULL)
		master_eloop_kernel = eloop_master_module_create(MODULE_PAL);
	//kernel_task_init ();
	ip_ifp_stack_init();
	#ifdef ZPL_NSM_ARP
	ip_arp_stack_init();
	#endif
	kernel_packet_init();
	//kernel_driver_init();
	return 0;//ip_stack_init();
}

static int pal_module_task_init(void)
{
	return kernel_task_init ();
}