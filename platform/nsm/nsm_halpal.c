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
#include "if.h"
#include "vrf.h"
#include "prefix.h"

#include "nsm_include.h"

#ifdef ZPL_HAL_MODULE
#include "hal_include.h"
#endif
#ifdef ZPL_PAL_MODULE
#include "pal_include.h"
#endif


int nsm_halpal_interface_add(struct interface *ifp)
{
	int ret = 0;
	if(os_strlen(ifp->k_name))
	{
		ret = pal_interface_create(ifp);
		if(ret != OK)
			return ret;
	}
	return ret;
}

int nsm_halpal_interface_delete (struct interface *ifp)
{
	int ret = 0;
	ret = pal_interface_destroy(ifp);
	if(ret != OK)
		return ret;
	return ret;
}

int nsm_halpal_interface_up (struct interface *ifp)
{
	int ret = 0;
	if(if_is_l3intf(ifp))
		ret = pal_interface_up(ifp);
	if(ret != OK)
		return ret;
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
#ifdef ZPL_HAL_MODULE
	ret = hal_port_down(ifp->ifindex);
	if(ret != OK)
		return ret;
#endif
	if(if_is_l3intf(ifp))
		ret = pal_interface_down(ifp);
	if(ret != OK)
		return ret;
	return ret;
}

int nsm_halpal_interface_change (struct interface *ifp)
{
	int ret = 0;
	ret = pal_interface_change(ifp);
	if(ret != OK)
		return ret;
	return ret;
}

int nsm_halpal_interface_refresh_flag(struct interface *ifp)
{
	int ret = 0;
	ret = pal_interface_refresh_flag(ifp);
	if(ret != OK)
		return ret;
	return ret;
}

int nsm_halpal_interface_update_flag(struct interface *ifp, zpl_uint32 flags)
{
	int ret = 0;
	ret = pal_interface_update_flag(ifp, flags);
	if(ret != OK)
		return ret;
	return ret;
}

int nsm_halpal_interface_ifindex(char *k_name)
{
	int ret = 0;
	ret = pal_interface_ifindex(k_name);
	if(ret != OK)
		return ret;
	return ret;
}


int nsm_halpal_interface_mtu (struct interface *ifp, zpl_uint32 mtu)
{
	int ret = 0;
	ret = pal_interface_set_mtu(ifp, mtu);
	if(ret != OK)
		return ret;
#ifdef ZPL_HAL_MODULE
	//ret = hal_port_mtu_set(ifp->ifindex, mtu);
#endif
	return ret;
}


int nsm_halpal_interface_vrf (struct interface *ifp, struct ip_vrf *vrf)
{
	int ret = 0;
	ret = pal_interface_set_vrf(ifp, vrf);
	if(ret != OK)
		return ret;
#ifdef ZPL_HAL_MODULE
	ret = hal_port_vrf_set(ifp->ifindex, vrf->vrf_id);
#endif
	return ret;
}

int nsm_halpal_interface_multicast (struct interface *ifp, zpl_bool multicast)
{
	int ret = 0;
#ifdef ZPL_HAL_MODULE
	//ret = hal_port_multicast_set(ifp->ifindex, multicast);
#endif
	return ret;
}

int nsm_halpal_interface_bandwidth (struct interface *ifp, zpl_uint32 bandwidth)
{
	int ret = 0;
#ifdef ZPL_HAL_MODULE
	//ret = hal_port_bandwidth_set(ifp->ifindex, bandwidth);
#endif
	return ret;
}

int nsm_halpal_interface_set_address (struct interface *ifp, struct connected *ifc, zpl_bool secondry)
{
	//printf("%s\r\n", __func__);
	int ret = 0;
	ret = pal_interface_ipv4_add(ifp, ifc);
	if(ret != OK)
		return ret;
	//ret = hal_port_address_set(ifp->ifindex, cp, secondry);
	return ret;
}

int nsm_halpal_interface_unset_address (struct interface *ifp, struct connected *ifc, zpl_bool secondry)
{
	int ret = 0;
	//ret = hal_port_address_unset(ifp->ifindex, cp, secondry);
	//if(ret != OK)
	//	return ret;
	ret = pal_interface_ipv4_delete(ifp, ifc);
	return ret;
}

int nsm_halpal_interface_set_dstaddr (struct interface *ifp, struct connected *cp, zpl_bool secondry)
{
	int ret = 0;
	ret = pal_interface_ipv4_dstaddr_add(ifp, cp);
	return ret;
}
int nsm_halpal_interface_unset_dstaddr (struct interface *ifp, struct connected *cp, zpl_bool secondry)
{
	int ret = 0;
	ret = pal_interface_ipv4_dstaddr_delete(ifp, cp);
	return ret;
}



int nsm_halpal_interface_mac (struct interface *ifp, zpl_uchar *mac, zpl_uint32 len)
{
	int ret = 0;
	ret = pal_interface_set_lladdr(ifp, mac, len);
	if(ret != OK)
		return ret;
	//ret = hal_port_mac_set(ifp->ifindex, mac, len);
	return ret;
}

int nsm_halpal_interface_get_statistics (struct interface *ifp)
{
	int ret = 0;
	ret = pal_interface_update_statistics(ifp);
	if(ret != OK)
		return ret;
	return ret;
}

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


int nsm_halpal_interface_enca (struct interface *ifp, zpl_uint32 mode, zpl_uint32 value)
{
	int ret = 0;
	#ifdef ZPL_NSM_SERIAL
	if(if_is_serial(ifp))
		ret = nsm_serial_interface_enca_set_api(ifp, mode);
	else if(if_is_ethernet(ifp) &&
	#else
	if(if_is_ethernet(ifp) &&
	#endif
			IF_IS_SUBIF_GET(ifp->ifindex) &&
			mode == IF_ENCA_DOT1Q &&
			ifp->encavlan != value)
	{
		#ifdef ZPL_NSM_VLANETH
		ret = nsm_vlaneth_interface_vid_set_api(ifp, value);
		#endif
	}
	return ret;
}




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



int nsm_halpal_iproute_rib_action(struct prefix *p, struct rib *old, struct rib *new)
{
	int ret = 0;
	ret = pal_iproute_rib_action(p, old, new);
	return ret;
}


int nsm_halpal_interface_vlan_set(struct interface *ifp, vlan_t vlan)
{
	int ret = 0;
	ret = pal_interface_vlan_set(ifp, vlan);
	return ret;
}

int nsm_halpal_interface_vlanpri_set(struct interface *ifp, zpl_uint32 pri)
{
	int ret = 0;
	ret = pal_interface_vlanpri_set(ifp, pri);
	return ret;
}

int nsm_halpal_interface_promisc_link(struct interface *ifp, zpl_bool enable)
{
	int ret = 0;
	ret = pal_interface_promisc_link(ifp, enable);
	return ret;
}



int nsm_halpal_create_vrf(struct ip_vrf *vrf)
{
	int ret = 0;
	ret = pal_create_vrf(vrf);
	return ret;
}

int nsm_halpal_delete_vrf(struct ip_vrf *vrf)
{
	int ret = 0;
	ret = pal_delete_vrf(vrf);
	return ret;
}

#ifdef ZPL_NSM_ARP
int nsm_halpal_interface_arp_add(struct interface *ifp, struct prefix *address, zpl_uint8 *mac)
{
	int ret = 0;
	ret = pal_interface_arp_add(ifp, address, mac);
	return ret;
}

int nsm_halpal_interface_arp_delete(struct interface *ifp, struct prefix *address)
{
	int ret = 0;
	ret = pal_interface_arp_delete(ifp, address);
	return ret;
}

int nsm_halpal_interface_arp_request(struct interface *ifp, struct prefix *address)
{
	int ret = 0;
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