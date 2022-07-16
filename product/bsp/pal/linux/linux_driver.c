/*
 * kernel_driver.c
 *
 *  Created on: 2019年9月21日
 *      Author: zhurish
 */


#include "auto_include.h"
#include "zplos_include.h"
#include "zebra_event.h"
#include "module.h"
#include "zmemory.h"
#include "thread.h"
#include "if.h"
#include "vty.h"
#include "command.h"
#include "prefix.h"
#include "vrf.h"
#include "nsm_debug.h"
#include "nsm_rib.h"
#include "module.h"
#include "zmemory.h"
#include "thread.h"
#include "if.h"
#include "vty.h"
#include "command.h"
#include "prefix.h"
#include "vrf.h"
#include "nsm_include.h"
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <linux/if_packet.h>

#include "linux_driver.h"
#include "pal_include.h"
#if defined(ZPL_SDK_KERNEL)
#undef  ZPL_SDK_KERNEL
#endif
#ifndef ZPL_SDK_USER
//#define ZPL_SDK_USER
#endif

#include "bsp_include.h"

#ifdef ZPL_KERNEL_FORWARDING
int linux_ioctl_create(struct interface *ifp)
{
  int ret = -1;
  switch (ifp->if_type)
  {
  case IF_SERIAL:
  case IF_ETHERNET:
  case IF_GIGABT_ETHERNET:
#ifdef ZPL_NSM_VLANETH
  case IF_VLAN:
  {
    nsm_vlaneth_t *vlaneth = nsm_vlaneth_get(ifp);
    if (vlaneth)
    {
      ret = linux_ioctl_vlaneth_create(vlaneth);
    }
  }
#endif
  break;
#ifdef ZPL_NSM_TUNNEL
  case IF_TUNNEL:
  {
    nsm_tunnel_t *tunnel = nsm_tunnel_get(ifp);
    if (tunnel)
    {
      ret = linux_ioctl_tunnel_create(tunnel);
    }
  }
  break;
#endif
#ifdef ZPL_NSM_TRUNK
  case IF_LAG:
    ret = linux_ioctl_bond_create(ifp);
    break;
#endif
  case IF_LOOPBACK:
    break;
#ifdef ZPL_NSM_BRIDGE
  case IF_BRIGDE:
  {
    nsm_bridge_t *bridge = nsm_bridge_get(ifp);
    if (bridge)
    {
      bridge->add_member_cb = linux_ioctl_bridge_add_interface;
      bridge->del_member_cb = linux_ioctl_bridge_del_interface;
      bridge->get_member_cb = linux_ioctl_bridge_list_interface;
      ret = linux_ioctl_bridge_create(bridge);
    }
  }
  break;
#endif
  default:
    break;
  }
  return ret;
}

int linux_ioctl_destroy(struct interface *ifp)
{
  int ret = -1;
  switch (ifp->if_type)
  {
  case IF_SERIAL:
  case IF_ETHERNET:
  case IF_GIGABT_ETHERNET:
#ifdef ZPL_NSM_VLANETH
  case IF_VLAN:
  {
    nsm_vlaneth_t *vlaneth = nsm_vlaneth_get(ifp);
    if (vlaneth)
    {
      ret = linux_ioctl_vlaneth_destroy(vlaneth);
    }
  }
#endif
  break;
#ifdef ZPL_NSM_TUNNEL
  case IF_TUNNEL:
  {
    nsm_tunnel_t *tunnel = nsm_tunnel_get(ifp);
    if (tunnel)
    {
      ret = linux_ioctl_tunnel_delete(tunnel);
    }
  }
  break;
#endif
#ifdef ZPL_NSM_TRUNK
  case IF_LAG:
    ret = linux_ioctl_bond_delete(ifp);
    break;
#endif
  case IF_LOOPBACK:
    break;
#ifdef ZPL_NSM_BRIDGE
  case IF_BRIGDE:
  {
    nsm_bridge_t *bridge = nsm_bridge_get(ifp);
    if (bridge)
    {
      ret = linux_ioctl_bridge_delete(bridge);
    }
  }
  break;
#endif
  default:
    break;
  }
  if (ret == 0)
  {
    if (pal_interface_up(ifp) != OK)
    {
      zlog_err(MODULE_PAL, "Unable to set L3 interface up %s(%s).",
               ifp->name, ipstack_strerror(ipstack_errno));
      return ERROR;
    }

    if (pal_interface_refresh_flag(ifp) != OK)
    {
      zlog_err(MODULE_PAL, "Unable to get L3 interface  flags %s(%s).",
               ifp->name, ipstack_strerror(ipstack_errno));
      return ERROR;
    }
    if (pal_interface_get_lladdr(ifp) != OK)
    {
      zlog_err(MODULE_PAL, "Unable to get L3 interface  mac %s(%s).",
               ifp->name, ipstack_strerror(ipstack_errno));
      return ERROR;
    }
  }
  return ret;
}

int linux_ioctl_change(struct interface *ifp)
{
  int ret = -1;
  switch (ifp->if_type)
  {
  case IF_SERIAL:
  case IF_ETHERNET:
  case IF_GIGABT_ETHERNET:
  case IF_VLAN:
    break;
#ifdef ZPL_NSM_TUNNEL
  case IF_TUNNEL:
  {
    nsm_tunnel_t *tunnel = nsm_tunnel_get(ifp);
    if (tunnel)
    {
      ret = linux_ioctl_tunnel_change(tunnel);
    }
  }
  break;
#endif
  case IF_LAG:
    break;
  case IF_LOOPBACK:
    break;
  case IF_BRIGDE:
    break;
  default:
    break;
  }
  if (ret == 0)
  {
    if (pal_interface_up(ifp) != OK)
    {
      zlog_err(MODULE_PAL, "Unable to set L3 interface up %s(%s).",
               ifp->name, ipstack_strerror(ipstack_errno));
      return ERROR;
    }

    if (pal_interface_refresh_flag(ifp) != OK)
    {
      zlog_err(MODULE_PAL, "Unable to get L3 interface  flags %s(%s).",
               ifp->name, ipstack_strerror(ipstack_errno));
      return ERROR;
    }
    if (pal_interface_get_lladdr(ifp) != OK)
    {
      zlog_err(MODULE_PAL, "Unable to get L3 interface  mac %s(%s).",
               ifp->name, ipstack_strerror(ipstack_errno));
      return ERROR;
    }
  }
  return ret;
}

int linux_ioctl_set_vlan(struct interface *ifp, vlan_t vlan)
{
  if ((if_is_ethernet(ifp) && IF_ID_GET(ifp->ifindex)))
  {
#ifdef ZPL_NSM_VLANETH
    nsm_vlaneth_t *vlaneth = nsm_vlaneth_get(ifp);
    if (vlaneth)
    {
      if (linux_ioctl_vlaneth_change(vlaneth, vlan) == 0)
      {
        //if(ret == 0)
        {
          if (pal_interface_up(ifp) != OK)
          {
            zlog_err(MODULE_PAL, "Unable to set L3 interface up %s(%s).",
                     ifp->name, ipstack_strerror(ipstack_errno));
            return ERROR;
          }

          if (pal_interface_refresh_flag(ifp) != OK)
          {
            zlog_err(MODULE_PAL, "Unable to get L3 interface  flags %s(%s).",
                     ifp->name, ipstack_strerror(ipstack_errno));
            return ERROR;
          }
          if (pal_interface_get_lladdr(ifp) != OK)
          {
            zlog_err(MODULE_PAL, "Unable to get L3 interface  mac %s(%s).",
                     ifp->name, ipstack_strerror(ipstack_errno));
            return ERROR;
          }
        }
      }
    }
#endif
    return ERROR;
  }
  return OK;
}
#endif


int iplinux_stack_init(void)
{
  
  //interface
  pal_stack.ip_stack_up = linux_ioctl_if_set_up;
  pal_stack.ip_stack_down = linux_ioctl_if_set_down;
  pal_stack.ip_stack_update_flag = linux_ioctl_if_update_flags;
  pal_stack.ip_stack_refresh_flag = linux_ioctl_if_get_flags;
  pal_stack.ip_stack_ifindex = if_nametoindex; //if_get_ifindex;

  pal_stack.ip_stack_set_mtu = linux_ioctl_if_set_mtu;
  pal_stack.ip_stack_set_lladdr = linux_ioctl_if_set_mac;
  pal_stack.ip_stack_get_lladdr = linux_ioctl_if_get_hwaddr;
  pal_stack.ip_stack_set_metric = linux_ioctl_if_set_metric;
#ifdef ZPL_KERNEL_FORWARDING
  pal_stack.ip_stack_create = linux_ioctl_create;
  pal_stack.ip_stack_destroy = linux_ioctl_destroy;
  pal_stack.ip_stack_change = linux_ioctl_change;
  pal_stack.ip_stack_set_vlan = linux_ioctl_set_vlan;
#else
  pal_stack.ip_stack_create = linux_netlink_create_interface;
  pal_stack.ip_stack_destroy = linux_netlink_destroy_interface;
  //pal_stack.ip_stack_change = linux_ioctl_change;
  //pal_stack.ip_stack_set_vlan = linux_ioctl_set_vlan;
#endif
  pal_stack.ip_stack_set_vlanpri = NULL;
  pal_stack.ip_stack_promisc = NULL;
  //ip address
  //pal_stack.ip_stack_ipv4_dstaddr_add = linux_ioctl_if_set_dst_prefix;
  //pal_stack.ip_stack_ipv4_dstaddr_del = linux_ioctl_if_unset_dst_prefix;
  pal_stack.ip_stack_ipv4_replace = NULL;
  //pal_stack.ip_stack_ipv4_add = linux_ioctl_if_set_prefix;
  //pal_stack.ip_stack_ipv4_delete = linux_ioctl_if_unset_prefix;

#ifdef ZPL_BUILD_IPV6
  //pal_stack.ip_stack_ipv6_add = linux_ioctl_if_prefix_add_ipv6;
  //pal_stack.ip_stack_ipv6_delete = linux_ioctl_if_prefix_delete_ipv6;
#endif

	pal_stack.ip_stack_vrf_create = linux_ioctl_vrf_enable;
	pal_stack.ip_stack_vrf_delete = linux_ioctl_vrf_disable;
  pal_stack.ip_stack_set_vrf = NULL;  
#ifdef ZPL_NSM_FIREWALLD
  /*
  * 防火墙
  */
  pal_stack.ip_stack_firewall_portmap_rule_set = linux_ioctl_firewall_portmap_rule_set;
  pal_stack.ip_stack_firewall_port_filter_rule_set = linux_ioctl_firewall_port_filter_rule_set;
  pal_stack.ip_stack_firewall_mangle_rule_set = linux_ioctl_firewall_mangle_rule_set;
  pal_stack.ip_stack_firewall_raw_rule_set = linux_ioctl_firewall_raw_rule_set;
  pal_stack.ip_stack_firewall_snat_rule_set = linux_ioctl_firewall_snat_rule_set;
  pal_stack.ip_stack_firewall_dnat_rule_set = linux_ioctl_firewall_dnat_rule_set;
#endif

#if defined(ZPL_SDK_MODULE) && defined(ZPL_SDK_USER)
	sdk_l3if.sdk_l3if_addif_cb = NULL;
	sdk_l3if.sdk_l3if_delif_cb = NULL;
	sdk_l3if.sdk_l3if_mac_cb = NULL;
	sdk_l3if.sdk_l3if_vrf_cb = NULL;

	sdk_l3if.sdk_l3if_add_addr_cb = linux_ioctl_if_set_prefix;
	sdk_l3if.sdk_l3if_del_addr_cb = linux_ioctl_if_unset_prefix;
	sdk_l3if.sdk_l3if_add_dstaddr_cb = linux_ioctl_if_set_dst_prefix;
	sdk_l3if.sdk_l3if_del_dstaddr_cb = linux_ioctl_if_unset_dst_prefix;


  sdk_route.sdk_route_add_cb = linux_netlink_route_rib_add;
  sdk_route.sdk_route_del_cb = linux_netlink_route_rib_del;
#endif
#ifdef ZPL_NSM_ARP
	pal_stack.ip_stack_arp_get = kernel_arp_get;
	pal_stack.ip_stack_arp_add = kernel_arp_set;
	pal_stack.ip_stack_arp_delete = kernel_arp_del;

	pal_stack.ip_stack_arp_gratuitousarp_enable = kernel_arp_gratuitousarp_enable;
	pal_stack.ip_stack_arp_ttl = NULL;
	pal_stack.ip_stack_arp_age_timeout = NULL;
	pal_stack.ip_stack_arp_retry_interval = NULL;
#endif
  iplink_test();
  return OK;
}