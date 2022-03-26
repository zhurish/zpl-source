/*
 * kernel_driver.c
 *
 *  Created on: 2019年9月21日
 *      Author: zhurish
 */


#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"

#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <linux/if_packet.h>

#include "kernel_driver.h"
#include "kernel_ioctl.h"
#include "kernel_netlink.h"
#include "pal_driver.h"

#ifdef ZPL_KERNEL_SORF_FORWARDING
int _ipkernel_create(struct interface *ifp)
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
      ret = _ipkernel_linux_create(vlaneth);
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
      ret = _ipkernel_tunnel_create(tunnel);
    }
  }
  break;
#endif
#ifdef ZPL_NSM_TRUNK
  case IF_LAG:
    ret = _ipkernel_bond_create(ifp);
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
      bridge->add_member_cb = _ipkernel_bridge_add_interface;
      bridge->del_member_cb = _ipkernel_bridge_del_interface;
      bridge->get_member_cb = _ipkernel_bridge_list_interface;
      ret = _ipkernel_bridge_create(bridge);
    }
  }
  break;
#endif
  default:
    break;
  }
  return ret;
}

int _ipkernel_destroy(struct interface *ifp)
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
      ret = _ipkernel_linux_destroy(vlaneth);
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
      ret = _ipkernel_tunnel_delete(tunnel);
    }
  }
  break;
#endif
#ifdef ZPL_NSM_TRUNK
  case IF_LAG:
    ret = _ipkernel_bond_delete(ifp);
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
      ret = _ipkernel_bridge_delete(bridge);
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

int _ipkernel_change(struct interface *ifp)
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
      ret = _ipkernel_tunnel_change(tunnel);
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

int _ipkernel_set_vlan(struct interface *ifp, vlan_t vlan)
{
  if ((if_is_ethernet(ifp) && IF_ID_GET(ifp->ifindex)))
  {
#ifdef ZPL_NSM_VLANETH
    nsm_vlaneth_t *vlaneth = nsm_vlaneth_get(ifp);
    if (vlaneth)
    {
      if (_ipkernel_linux_change(vlaneth, vlan) == 0)
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


int ip_ifp_stack_init(void)
{
  //interface
  pal_stack.ip_stack_up = _ipkernel_if_set_up;
  pal_stack.ip_stack_down = _ipkernel_if_set_down;
  pal_stack.ip_stack_update_flag = _ipkernel_if_update_flags;
  pal_stack.ip_stack_refresh_flag = _ipkernel_if_get_flags;
  pal_stack.ip_stack_ifindex = if_nametoindex; //if_get_ifindex;
  pal_stack.ip_stack_set_vr = NULL;
  pal_stack.ip_stack_set_mtu = _ipkernel_if_set_mtu;
  pal_stack.ip_stack_set_lladdr = _ipkernel_if_set_mac;
  pal_stack.ip_stack_get_lladdr = _ipkernel_if_get_hwaddr;
  pal_stack.ip_stack_set_metric = _ipkernel_if_set_metric;
#ifdef ZPL_KERNEL_SORF_FORWARDING
  pal_stack.ip_stack_create = _ipkernel_create;
  pal_stack.ip_stack_destroy = _ipkernel_destroy;
  pal_stack.ip_stack_change = _ipkernel_change;
  pal_stack.ip_stack_set_vlan = _ipkernel_set_vlan;
#else
  pal_stack.ip_stack_create = _netlink_create_interface;
  pal_stack.ip_stack_destroy = _netlink_destroy_interface;
  //pal_stack.ip_stack_change = _ipkernel_change;
  //pal_stack.ip_stack_set_vlan = _ipkernel_set_vlan;
#endif
  pal_stack.ip_stack_set_vlanpri = NULL;
  pal_stack.ip_stack_promisc = NULL;
  //ip address
  pal_stack.ip_stack_dhcp = NULL;
  pal_stack.ip_stack_ipv4_dstaddr_add = _ipkernel_if_set_dst_prefix;
  pal_stack.ip_stack_ipv4_dstaddr_del = _ipkernel_if_unset_dst_prefix;
  pal_stack.ip_stack_ipv4_replace = NULL;
  pal_stack.ip_stack_ipv4_add = _ipkernel_if_set_prefix;
  pal_stack.ip_stack_ipv4_delete = _ipkernel_if_unset_prefix;

#ifdef HAVE_IPV6
  pal_stack.ip_stack_ipv6_add = _ipkernel_if_prefix_add_ipv6;
  pal_stack.ip_stack_ipv6_delete = _ipkernel_if_prefix_delete_ipv6;
#endif

	pal_stack.ip_stack_vrf_create = _ipkernel_vrf_enable;
	pal_stack.ip_stack_vrf_delete = _ipkernel_vrf_disable;
#ifdef ZPL_NSM_FIREWALLD
  /*
  * 防火墙
  */
  pal_stack.ip_stack_firewall_portmap_rule_set = _ipkernel_firewall_portmap_rule_set;
  pal_stack.ip_stack_firewall_port_filter_rule_set = _ipkernel_firewall_port_filter_rule_set;
  pal_stack.ip_stack_firewall_mangle_rule_set = _ipkernel_firewall_mangle_rule_set;
  pal_stack.ip_stack_firewall_raw_rule_set = _ipkernel_firewall_raw_rule_set;
  pal_stack.ip_stack_firewall_snat_rule_set = _ipkernel_firewall_snat_rule_set;
  pal_stack.ip_stack_firewall_dnat_rule_set = _ipkernel_firewall_dnat_rule_set;
#endif
  return OK;
}