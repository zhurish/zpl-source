/* Zebra's client library.
 * Copyright (C) 1999 Kunihiro Ishiguro
 * Copyright (C) 2005 Andrew J. Schorr
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "lib_include.h"
#ifdef ZPL_SHELL_MODULE
#include "vty_include.h"
#endif
#include "nsm_interface.h"
#include "nsm_qos.h"
#include "nsm_rib.h"
#include "nsm_halpal.h"
#ifdef ZPL_HAL_MODULE
#include "hal_include.h"
#endif
#ifdef ZPL_PAL_MODULE
#include "pal_include.h"
#endif


int nsm_halpal_interface_add(struct interface *ifp)
{
	int ret = 0;
	if(os_strlen(ifp->ker_name))
	{
		if(if_is_l3intf(ifp))
		{
			char mac[8];
			host_config_get_api(API_GET_SYSMAC_CMD, mac);
			memcpy(ifp->hw_addr, mac, NSM_MAC_MAX);
   			ifp->hw_addr_len = NSM_MAC_MAX;
		}
		if(if_is_l3intf(ifp) && pal_interface_create(ifp) != OK)
			return ERROR;
		ret = hal_l3if_add(ifp->ifindex, ifp->ker_name, NULL);
		if(ret != OK)
			return ret;
		pal_interface_up(ifp);	
	}
	return ret;
}

int nsm_halpal_interface_delete (struct interface *ifp)
{
	int ret = 0;
	if(if_is_l3intf(ifp) && pal_interface_destroy(ifp) != OK)
		return ERROR;
	ret = hal_l3if_del(ifp->ifindex);
	if(ret != OK)
		return ret;
	return ret;
}

int nsm_halpal_interface_update (struct interface *ifp)
{
	int ret = OK;
	if(if_is_l3intf(ifp))
		ret = pal_interface_update(ifp);
	if(ret != OK)
		return ret;
	return ret;
}

int nsm_halpal_interface_add_slave (struct interface *ifp, struct interface *sifp)
{
	int ret = OK;
	if(if_is_l3intf(ifp))
		ret = pal_interface_add_slave(ifp, sifp);
	if(ret != OK)
		return ret;
	return ret;
}

int nsm_halpal_interface_del_slave (struct interface *ifp, struct interface *sifp)
{
	int ret = OK;
	if(if_is_l3intf(ifp))
		ret = pal_interface_del_slave(ifp, sifp);
	if(ret != OK)
		return ret;
	return ret;
}

int nsm_halpal_interface_up (struct interface *ifp)
{
	int ret = 0;
	if(if_is_l3intf(ifp) && pal_interface_up(ifp) != OK)
		return ERROR;	
#ifdef ZPL_HAL_MODULE
	ret = hal_port_up(ifp->ifindex);
	if(ret != OK)
		return ret;
#endif
	return ret;
}

int nsm_halpal_interface_down (struct interface *ifp)
{
	int ret = 0;
	if(if_is_l3intf(ifp) && pal_interface_down(ifp) != OK)
		return ERROR;
#ifdef ZPL_HAL_MODULE
	ret = hal_port_down(ifp->ifindex);
	if(ret != OK)
		return ret;
#endif
	return ret;
}


int nsm_halpal_interface_ifindex(char *ker_name)
{
	int ret = OK;
	ret = pal_interface_ifindex(ker_name);
	//ret = if_nametoindex(ker_name);
	return ret;
}

int nsm_halpal_interface_mtu (struct interface *ifp, zpl_uint32 mtu)
{
	int ret = 0;
	if(if_is_l3intf(ifp) && pal_interface_set_mtu(ifp, mtu) != OK)
		return ERROR;
	ret = hal_port_mtu_set(ifp->ifindex, mtu);
	return ret;
}


int nsm_halpal_interface_vrf (struct interface *ifp, struct ip_vrf *vrf)
{
	int ret = 0;
	if(if_is_l3intf(ifp) && pal_interface_set_vrf(ifp, vrf) != OK)
		return ERROR;
	ret = hal_port_vrf_set(ifp->ifindex, vrf->vrf_id);
	return ret;
}

int nsm_halpal_interface_multicast (struct interface *ifp, zpl_uint32 multicast)
{
	int ret = 0;
#ifdef ZPL_HAL_MODULE
	ret = hal_port_multicast_set(ifp->ifindex, multicast);
#endif
	return ret;
}

int nsm_halpal_interface_bandwidth (struct interface *ifp, zpl_uint32 bandwidth)
{
	int ret = 0;
#ifdef ZPL_HAL_MODULE
	ret = hal_port_bandwidth_set(ifp->ifindex, bandwidth);
#endif
	return ret;
}


#ifdef ZPL_NSM_L3MODULE
int nsm_halpal_interface_set_address (struct interface *ifp, struct connected *ifc, zpl_bool secondry)
{
	int ret = 0;
	struct prefix *address = ifc->address;
	if(address && PREFIX_FAMILY(address)== IPSTACK_AF_INET)
	{
		if(if_is_l3intf(ifp) && pal_interface_ipv4_add(ifp, ifc) != OK)
			return ERROR;
	}
	if(address && PREFIX_FAMILY(address)== IPSTACK_AF_INET6)
	{
		if(if_is_l3intf(ifp) && pal_interface_ipv6_add(ifp, ifc, secondry) != OK)
			return ERROR;
	}
	ret = hal_l3if_addr_add(ifp->ifindex, ifc->address, secondry);
	if(ret != OK)
		return ret;
	return ret;
}

int nsm_halpal_interface_unset_address (struct interface *ifp, struct connected *ifc, zpl_bool secondry)
{
	int ret = 0;
	struct prefix *address = ifc->address;
	if(address && PREFIX_FAMILY(address)== IPSTACK_AF_INET)
	{
		if(if_is_l3intf(ifp) && pal_interface_ipv4_delete(ifp, ifc) != OK)
			return ERROR;
	}
	if(address && PREFIX_FAMILY(address)== IPSTACK_AF_INET6)
	{
		if(if_is_l3intf(ifp) && pal_interface_ipv6_delete(ifp, ifc, secondry) != OK)
			return ERROR;
	}
	ret = hal_l3if_addr_del(ifp->ifindex, ifc->address, secondry);
	return ret;
}

int nsm_halpal_interface_set_dstaddr (struct interface *ifp, struct connected *cp, zpl_bool secondry)
{
	int ret = 0;
	struct prefix *address = cp->address;
	if(address && PREFIX_FAMILY(address)== IPSTACK_AF_INET)
	{
		if(if_is_l3intf(ifp) && pal_interface_ipv4_dstaddr_add(ifp, cp) != OK)
			return ERROR;
	}
	ret = hal_l3if_dstaddr_add(ifp->ifindex, cp->address);
	return ret;
}
int nsm_halpal_interface_unset_dstaddr (struct interface *ifp, struct connected *cp, zpl_bool secondry)
{
	int ret = 0;
	struct prefix *address = cp->address;
	if(address && PREFIX_FAMILY(address)== IPSTACK_AF_INET)
	{
		if(if_is_l3intf(ifp) && pal_interface_ipv4_dstaddr_delete(ifp, cp) != OK)
			return ERROR;
	}
	ret = hal_l3if_dstaddr_del(ifp->ifindex, cp->address);
	return ret;
}



int nsm_halpal_interface_mac (struct interface *ifp, zpl_uchar *mac, zpl_uint32 len)
{
	int ret = 0;
	if(if_is_l3intf(ifp) && pal_interface_set_lladdr(ifp, mac, len) != OK)
		return ERROR;
	ret = hal_l3if_mac_set(ifp->ifindex, mac);
	return ret;
}
#endif



int nsm_halpal_interface_speed (struct interface *ifp,  nsm_speed_en speed )
{
	int ret = 0;
#ifdef ZPL_HAL_MODULE
	ret = hal_port_speed_set(ifp->ifindex, speed);
#endif
	return ret;
}



int nsm_halpal_interface_mode (struct interface *ifp, if_mode_t mode)
{
	int ret = 0;
#ifdef ZPL_HAL_MODULE
	ret = hal_port_mode_set(ifp->ifindex, mode);
#endif
	return ret;
}

#ifdef IF_ENCAPSULATION_ENABLE
int nsm_halpal_interface_enca (struct interface *ifp, zpl_uint32 mode)
{
	int ret = 0;
	#ifdef ZPL_NSM_SERIAL
	if(if_is_l3intf(ifp) && if_is_serial(ifp))
		ret = nsm_serial_interface_enca_set_api(ifp, mode);
	#endif	
	return ret;
}
#endif



int nsm_halpal_interface_loop (struct interface *ifp,  zpl_bool loop )
{
	int ret = 0;
#ifdef ZPL_HAL_MODULE
	ret = hal_port_loop_set(ifp->ifindex, loop);
#endif
	return ret;
}


int nsm_halpal_interface_duplex (struct interface *ifp, nsm_duplex_en duplex)
{
	int ret = 0;
#ifdef ZPL_HAL_MODULE
	ret = hal_port_duplex_set(ifp->ifindex, duplex);
#endif
	return ret;
}





int nsm_halpal_interface_vlan_set(struct interface *ifp, vlan_t vlan)
{
	int ret = 0;
	if(if_is_l3intf(ifp) && pal_interface_vlan_set(ifp, vlan) != OK)
		return ERROR;
	ret = pal_interface_vlan_set(ifp, vlan);
	return ret;
}

int nsm_halpal_interface_vlanpri_set(struct interface *ifp, zpl_uint32 pri)
{
	int ret = 0;
	if(if_is_l3intf(ifp) && pal_interface_vlanpri_set(ifp, pri) != OK)
		return ERROR;
	ret = pal_interface_vlanpri_set(ifp, pri);
	return ret;
}

int nsm_halpal_create_vrf(struct ip_vrf *vrf)
{
	int ret = 0;
	if(vrf->vrf_id != VRF_DEFAULT)
		ret = pal_create_vrf(vrf);
	return ret;
}

int nsm_halpal_delete_vrf(struct ip_vrf *vrf)
{
	int ret = 0;
	if(vrf->vrf_id != VRF_DEFAULT)
		ret = pal_delete_vrf(vrf);
	return ret;
}

#ifdef ZPL_NSM_ARP
int nsm_halpal_interface_arp_add(struct interface *ifp, struct prefix *address, zpl_uint8 *mac)
{
	int ret = OK;
	if(if_is_l3intf(ifp))
		ret = pal_interface_arp_add(ifp, address, mac);
	return ret;
}

int nsm_halpal_interface_arp_delete(struct interface *ifp, struct prefix *address)
{
	int ret = OK;
	if(if_is_l3intf(ifp))
		ret = pal_interface_arp_delete(ifp, address);
	return ret;
}

int nsm_halpal_interface_arp_request(struct interface *ifp, struct prefix *address)
{
	int ret = OK;
	if(if_is_l3intf(ifp))
		ret = pal_interface_arp_request(ifp, address);
	return ret;
}

int nsm_halpal_arp_gratuitousarp_enable(zpl_bool enable)
{
	int ret = 0;
	ret = pal_arp_gratuitousarp_enable(enable);
	return ret;
}

int nsm_halpal_arp_ttl(zpl_uint32 ttl)
{
	int ret = 0;
	ret = pal_arp_ttl(ttl);
	return ret;
}

int nsm_halpal_arp_age_timeout(zpl_uint32 timeout)
{
	int ret = 0;
	ret = pal_arp_age_timeout(timeout);
	return ret;
}

int nsm_halpal_arp_retry_interval(zpl_uint32 interval)
{
	int ret = 0;
	ret = pal_arp_retry_interval(interval);
	return ret;
}
#endif

#ifdef ZPL_NSM_FIREWALLD
int nsm_halpal_firewall_portmap_rule_set(firewall_t *rule, zpl_action action)
{
	int ret = 0;
	ret = pal_firewall_portmap_rule_set(rule, action);
	return ret;
}
/*
 * 端口开放
 */
int nsm_halpal_firewall_port_filter_rule_set(firewall_t *rule, zpl_action action)
{
	int ret = 0;
	ret = pal_firewall_port_filter_rule_set(rule, action);
	return ret;
}
int nsm_halpal_firewall_mangle_rule_set(firewall_t *rule, zpl_action action)
{
	int ret = 0;
	ret = pal_firewall_mangle_rule_set(rule, action);
	return ret;
}
int nsm_halpal_firewall_raw_rule_set(firewall_t *rule, zpl_action action)
{
	int ret = 0;
	ret = pal_firewall_raw_rule_set(rule, action);
	return ret;
}
int nsm_halpal_firewall_snat_rule_set(firewall_t *rule, zpl_action action)
{
	int ret = 0;
	ret = pal_firewall_snat_rule_set(rule, action);
	return ret;
}
int nsm_halpal_firewall_dnat_rule_set(firewall_t *rule, zpl_action action)
{
	int ret = 0;
	ret = pal_firewall_dnat_rule_set(rule, action);
	return ret;
}
#endif

#ifdef ZPL_NSM_L3MODULE
int nsm_halpal_route_multipath_add(safi_t safi, struct prefix *p,
                          struct rib *rib, zpl_uint8 num)
{
	int ret = 0;
	if(pal_route_rib_add(0,  safi,p, rib,  num) != OK)
		return ERROR;
	ret = hal_route_multipath_add(0, safi, p, rib, num);
	return ret;
}

int nsm_halpal_route_multipath_del(safi_t safi, struct prefix *p,
                          struct rib *rib, zpl_uint8 num)
{
	int ret = 0;
	if(pal_route_rib_del(0,  safi,p, rib,  num) != OK)
		return ERROR;
	ret = hal_route_multipath_del(0, safi, p, rib, num);
	return ret;
}
#endif