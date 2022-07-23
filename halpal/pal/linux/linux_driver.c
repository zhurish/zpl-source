/*
 * kernel_driver.c
 *
 *  Created on: 2019年9月21日
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "nsm_event.h"
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
#undef ZPL_SDK_KERNEL
#endif
#ifndef ZPL_SDK_USER
//#define ZPL_SDK_USER
#endif

static int linux_interface_create(struct interface *ifp)
{
  int ret = 0;
  switch (ifp->if_type)
  {
#if defined(ZPL_NSM_TUNNEL)
  case IF_TUNNEL:
    ret = linux_ioctl_tunnel_create(ifp);
    break;
#endif
#if defined(ZPL_NSM_SERIAL)
  case IF_SERIAL:
    break;
#endif
  case IF_ETHERNET:
  case IF_GIGABT_ETHERNET:
    ret = linux_ioctl_eth_create(ifp);
    break;
  case IF_WIRELESS:
    break;
#ifdef ZPL_NSM_BRIDGE
  case IF_BRIGDE:
    ret = linux_ioctl_bridge_create(ifp);
    break;
#endif
    break;
  case IF_LOOPBACK:
    break;
#if defined(ZPL_NSM_TRUNK)
  case IF_LAG:
    ret = linux_ioctl_bond_create(ifp);
    break;
#endif
#if defined(ZPL_NSM_VLAN)
  case IF_SUBVLAN:
    ret = linux_ioctl_vlan_create(ifp);
    break;
#endif
  default:
    break;
  }
  return ret;
}

static int linux_interface_delete(struct interface *ifp)
{
  int ret = 0;
  switch (ifp->if_type)
  {
#if defined(ZPL_NSM_TUNNEL)
  case IF_TUNNEL:
    ret = linux_ioctl_tunnel_delete(ifp);
    break;
#endif
#if defined(ZPL_NSM_SERIAL)
  case IF_SERIAL:
    break;
#endif
  case IF_ETHERNET:
  case IF_GIGABT_ETHERNET:
    ret = linux_ioctl_eth_destroy(ifp);
    break;
  case IF_WIRELESS:
    break;
#ifdef ZPL_NSM_BRIDGE
  case IF_BRIGDE:
    ret = linux_ioctl_bridge_delete(ifp);
    break;
#endif
    break;
  case IF_LOOPBACK:
    break;
#if defined(ZPL_NSM_TRUNK)
  case IF_LAG:
    ret = linux_ioctl_bond_delete(ifp);
    break;
#endif
#if defined(ZPL_NSM_VLAN)
  case IF_SUBVLAN:
    ret = linux_ioctl_vlan_destroy(ifp);
    break;
#endif
  default:
    break;
  }
  return ret;
}

static int linux_interface_add_slave(struct interface *ifp, struct interface *slave)
{
  int ret = 0;
  switch (ifp->if_type)
  {
#if defined(ZPL_NSM_TUNNEL)
  case IF_TUNNEL:
    break;
#endif
#if defined(ZPL_NSM_SERIAL)
  case IF_SERIAL:
    break;
#endif
  case IF_ETHERNET:
  case IF_GIGABT_ETHERNET:
    break;
  case IF_WIRELESS:
    break;
#ifdef ZPL_NSM_BRIDGE
  case IF_BRIGDE:
    ret = linux_ioctl_bridge_add_interface(ifp, slave);
    break;
#endif
    break;
  case IF_LOOPBACK:
    break;
#if defined(ZPL_NSM_TRUNK)
  case IF_LAG:
    ret = _if_bond_add_slave(ifp, slave);
    break;
#endif
#if defined(ZPL_NSM_VLAN)
  case IF_SUBVLAN:
    break;
#endif
  default:
    break;
  }
  return ret;
}

static int linux_interface_delete_slave(struct interface *ifp, struct interface *slave)
{
  int ret = 0;
  switch (ifp->if_type)
  {
#if defined(ZPL_NSM_TUNNEL)
  case IF_TUNNEL:
    break;
#endif
#if defined(ZPL_NSM_SERIAL)
  case IF_SERIAL:
    break;
#endif
  case IF_ETHERNET:
  case IF_GIGABT_ETHERNET:
    break;
  case IF_WIRELESS:
    break;
#ifdef ZPL_NSM_BRIDGE
  case IF_BRIGDE:
    ret = linux_ioctl_bridge_del_interface(ifp, slave);
    break;
#endif
    break;
  case IF_LOOPBACK:
    break;
#if defined(ZPL_NSM_TRUNK)
  case IF_LAG:
    ret = _if_bond_delete_slave(ifp, slave);
    break;
#endif
#if defined(ZPL_NSM_VLAN)
  case IF_SUBVLAN:
    break;
#endif
  default:
    break;
  }
  return ret;
}

static int linux_interface_update(struct interface *ifp)
{
  int ret = 0;
  switch (ifp->if_type)
  {
#if defined(ZPL_NSM_TUNNEL)
  case IF_TUNNEL:
    ret = linux_ioctl_tunnel_change(ifp);
    break;
#endif
#if defined(ZPL_NSM_SERIAL)
  case IF_SERIAL:
    break;
#endif
  case IF_ETHERNET:
  case IF_GIGABT_ETHERNET:
  case IF_WIRELESS:
    break;
#ifdef ZPL_NSM_BRIDGE
  case IF_BRIGDE:
    break;
#endif
    break;
  case IF_LOOPBACK:
    break;
#if defined(ZPL_NSM_TRUNK)
  case IF_LAG:
    break;
#endif
#if defined(ZPL_NSM_VLAN)
  case IF_VLAN:
    ret = linux_ioctl_vlan_change(ifp, 0);
    break;
#endif
  default:
    break;
  }
  return OK;
}

int iplinux_stack_init(void)
{
  // interface
  pal_stack.ip_stack_up = linux_ioctl_if_set_up;
  pal_stack.ip_stack_down = linux_ioctl_if_set_down;
  pal_stack.ip_stack_update_flag = linux_ioctl_if_update_flags;
  pal_stack.ip_stack_refresh_flag = linux_ioctl_if_get_flags;
  pal_stack.ip_stack_ifindex = if_nametoindex; // if_get_ifindex;

  pal_stack.ip_stack_set_mtu = linux_ioctl_if_set_mtu;
  pal_stack.ip_stack_set_lladdr = linux_ioctl_if_set_mac;
  pal_stack.ip_stack_get_lladdr = linux_ioctl_if_get_hwaddr;
  pal_stack.ip_stack_set_metric = linux_ioctl_if_set_metric;

  pal_stack.ip_stack_create = linux_interface_create;
  pal_stack.ip_stack_destroy = linux_interface_delete;
  pal_stack.ip_stack_update = linux_interface_update;
  pal_stack.ip_stack_member_add = linux_interface_add_slave;
  pal_stack.ip_stack_member_del = linux_interface_delete_slave;

  // pal_stack.ip_stack_set_vlan = linux_ioctl_set_vlan;
  pal_stack.ip_stack_set_vlanpri = NULL;

  // ip address
  pal_stack.ip_stack_ipv4_dstaddr_add = linux_ioctl_if_set_dst_prefix;
  pal_stack.ip_stack_ipv4_dstaddr_del = linux_ioctl_if_unset_dst_prefix;
  pal_stack.ip_stack_ipv4_replace = NULL;
  pal_stack.ip_stack_ipv4_add = linux_ioctl_if_set_prefix;
  pal_stack.ip_stack_ipv4_delete = linux_ioctl_if_unset_prefix;

#ifdef ZPL_BUILD_IPV6
  pal_stack.ip_stack_ipv6_add = linux_ioctl_if_prefix_add_ipv6;
  pal_stack.ip_stack_ipv6_delete = linux_ioctl_if_prefix_delete_ipv6;
#endif

  pal_stack.ip_stack_vrf_create = linux_ioctl_vrf_enable;
  pal_stack.ip_stack_vrf_delete = linux_ioctl_vrf_disable;
  pal_stack.ip_stack_set_vrf = linux_ioctl_vrf_set;
  pal_stack.ip_stack_socket_vrf = linux_vrf_socket;
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

  pal_stack.ip_stack_route_rib_add = librtnl_route_rib_add;
  pal_stack.ip_stack_route_rib_del = librtnl_route_rib_del;

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