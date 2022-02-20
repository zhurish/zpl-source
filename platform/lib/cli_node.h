#ifndef __CLI_NODE_H__
#define __CLI_NODE_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ZPL_SHELL_MODULE

#include "vty.h"

/* There are some command levels which called from command node. */
enum node_type 
{
  HOST_NODE,//只用于show running
  LOG_NODE,//只用于show running
  USER_NODE,//只用于show running
  HOSTSRV_NODE, //只用于show running
  LOGIN_NODE,
  AUTH_NODE,			/* Authentication mode of vty interface. */
  VIEW_NODE,			/* View node. Default mode of vty interface. */
  AUTH_ENABLE_NODE,		/* Authentication mode for change enable. */
  ENABLE_NODE,			/* Enable node. */
  CONFIG_NODE,			/* Config node. Default mode of config file. */

  VRF_NODE,			/* VRF node. */
  
  SERVICE_NODE, 		/* Service node. */
  ALL_SERVICE_NODE, 		/* Service node. */

#ifdef IMISH_IMI_MODULE  
  LINUX_SHELL_NODE,			/* IMI Module protocol node. */
#endif//IMISH_IMI_MODULE   

  DEBUG_NODE,			/* Debug node. */
  
  AAA_NODE,			/* AAA node. */
  KEYCHAIN_NODE,		/* Key-chain node. */
  KEYCHAIN_KEY_NODE,		/* Key-chain key node. */
  DHCPS_NODE,				/* DHDP server*/

  IP_ARP_NODE,				/* ip arp Service*/

  IP_DNS_NODE,				/* ip dns Service*/

  TEMPLATE_NODE,				/* templates server*/

  VLAN_DATABASE_NODE,	/* vlan database node*/
  VLAN_NODE,			/* vlan node */


  LOOPBACK_INTERFACE_NODE,	/* Loopback Interface mode node. */

  SERIAL_INTERFACE_NODE,	/* Serial Interface mode node. */

  INTERFACE_NODE,			/* L2 Interface mode node(default). */
  INTERFACE_RANGE_NODE,
  INTERFACE_L3_NODE,		/* Interface mode node. */
  INTERFACE_L3_RANGE_NODE,

  WIRELESS_INTERFACE_NODE,	/* Interface mode node. */
  TUNNEL_INTERFACE_NODE,	/* Tunnel Interface mode node. */

  LAG_INTERFACE_NODE,		/* Lag Interface mode node. */
  LAG_INTERFACE_L3_NODE,	/* Lag L3 Interface mode node. */

  E1_INTERFACE_L3_NODE,	/* E1 L3 Interface mode node. */
  E1_INTERFACE_L3_RANGE_NODE,

  EPON_INTERFACE_NODE,		/* EPON Interface mode node. */
  EPON_INTERFACE_L3_NODE,	/* EPON L3 Interface mode node. */
  EPON_INTERFACE_RANGE_NODE,
  EPON_INTERFACE_L3_RANGE_NODE,

  BRIGDE_INTERFACE_NODE,	/* Brigde Interface mode node. */
#ifdef CUSTOM_INTERFACE
  WIFI_INTERFACE_NODE,	/* Wifi Interface mode node. */
  MODEM_INTERFACE_NODE,		/* Modem Interface mode node. */
#endif

  TRUNK_NODE,			/* Trunk group mode node. */

  MODEM_PROFILE_NODE,		/* Modem Profile node. */
  MODEM_CHANNEL_NODE,		/* Modem Channel node. */

  ZEBRA_NODE,			/* zebra connection node. */
  TABLE_NODE,			/* rtm_table selection node. */
  RIP_NODE,			/* RIP protocol mode node. */ 
  RIPNG_NODE,			/* RIPng protocol mode node. */
  BABEL_NODE,			/* Babel protocol mode node. */
  BGP_NODE,			/* BGP protocol mode which includes BGP4+ */
  BGP_VPNV4_NODE,		/* BGP MPLS-VPN PE exchange. */
  BGP_VPNV6_NODE,		/* BGP MPLS-VPN PE exchange. */
  BGP_IPV4_NODE,		/* BGP IPv4 unicast address family.  */
  BGP_IPV4M_NODE,		/* BGP IPv4 multicast address family.  */
  BGP_IPV6_NODE,		/* BGP IPv6 address family */
  BGP_IPV6M_NODE,		/* BGP IPv6 multicast address family. */
  BGP_ENCAP_NODE,		/* BGP ENCAP SAFI */
  BGP_ENCAPV6_NODE,		/* BGP ENCAP SAFI */
  OSPF_NODE,			/* OSPF protocol mode */
  OSPF6_NODE,			/* OSPF protocol for IPv6 mode */
  ISIS_NODE,			/* ISIS protocol mode */
  PIM_NODE,			/* PIM protocol mode */

  HSLS_NODE,			/* HSLS protocol node. */
  OLSR_NODE,			/* OLSR protocol node. */
  VRRP_NODE,                /* VRRP protocol node */
  FRP_NODE,                /* FRP protocol node */
  LLDP_NODE,
  BFD_NODE,
  LDP_NODE,

  MASC_NODE,			/* MASC for multicast.  */
  IRDP_NODE,			/* ICMP Router Discovery Protocol mode. */ 
  IP_NODE,			/* Static ip route node. */

  ACCESS_NODE,			/* Access list node. */
  PREFIX_NODE,			/* Prefix list node. */
  ACCESS_IPV6_NODE,		/* Access list node. */
  PREFIX_IPV6_NODE,		/* Prefix list node. */
  ACCESS_MAC_NODE,		/* Access list node. */
  PREFIX_MAC_NODE,		/* Prefix list node. */
  QOS_ACCESS_NODE,		/* qos Access list node. */
  QOS_CLASS_MAP_NODE,		/* QOS Class map list node. */
  QOS_POLICY_MAP_NODE,		/* QOS Policy list node. */
  QOS_POLICY_CLASS_MAP_NODE,		/* QOS Policy Class list node. */

  AS_LIST_NODE,			/* AS list node. */
  COMMUNITY_LIST_NODE,		/* Community list node. */
  RMAP_NODE,			/* Route map node. */
  SMUX_NODE,			/* SNMP configuration node. */
  DUMP_NODE,			/* Packet dump node. */
  FORWARDING_NODE,		/* IP forwarding node. */
  PROTOCOL_NODE,                /* protocol filtering node */
  VTY_NODE,			/* Vty node. */
  LINK_PARAMS_NODE,		/* Link-parameters node */
  ZEBRA_IF_DEFAULTS_NODE,	/* If defaults dummy node */
  CMD_NODE_MAX
};

extern enum node_type cmd_end_node(struct vty *vty);
extern enum node_type cmd_stop_node(struct vty *vty);
extern enum node_type cmd_exit_node(struct vty *vty);
extern enum node_type node_parent(enum node_type node);

#endif /* ZPL_SHELL_MODULE */

#ifdef __cplusplus
}
#endif

#endif /* __CLI_NODE_H__ */
