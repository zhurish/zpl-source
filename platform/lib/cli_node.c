#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"
#include "vty_include.h"
#include "cli_node.h"


/* When '^Z' is received from vty, move down to the enable mode. */
enum node_type cmd_end_node(enum node_type node)
{
	switch (node)
	{
	case VIEW_NODE:
	case ENABLE_NODE:
		/*case RESTRICTED_NODE:*/
		/* Nothing to do. */
		break;
    default:
		if(node >= CONFIG_NODE && node < CMD_NODE_MAX)
        	return ENABLE_NODE;
		break;
	}
    return node;
}

enum node_type cmd_stop_node(enum node_type node)
{
    return cmd_end_node(node);
}
/* return parent node */
/* MUST eventually converge on CONFIG_NODE */
enum node_type node_parent(enum node_type node)
{
  enum node_type ret;

  assert(node > CONFIG_NODE);

  switch (node)
  {
  case BGP_VPNV4_NODE:
  case BGP_VPNV6_NODE:
  case BGP_ENCAP_NODE:
  case BGP_ENCAPV6_NODE:
  case BGP_IPV4_NODE:
  case BGP_IPV4M_NODE:
  case BGP_IPV6_NODE:
  case BGP_IPV6M_NODE:
    ret = BGP_NODE;
    break;
  case KEYCHAIN_KEY_NODE:
    ret = KEYCHAIN_NODE;
    break;
  case LINK_PARAMS_NODE:
    ret = INTERFACE_NODE;
    break;
    case QOS_POLICY_CLASS_MAP_NODE:
		ret = QOS_POLICY_MAP_NODE;
		break;
  default:
    ret = CONFIG_NODE;
    break;
  }

  return ret;
}

enum node_type cmd_exit_node(enum node_type node)
{
	switch (node)
 	{
	case DHCPS_NODE:
	case TEMPLATE_NODE:
	case ALL_SERVICE_NODE:
	case MODEM_PROFILE_NODE:
	case MODEM_CHANNEL_NODE:
	case INTERFACE_NODE:
	case INTERFACE_L3_NODE:		/* Interface mode node. */
	case WIRELESS_INTERFACE_NODE:
	case TUNNEL_INTERFACE_NODE:	/* Tunnel Interface mode node. */
	case LOOPBACK_INTERFACE_NODE:	/* Loopback Interface mode node. */
	case LAG_INTERFACE_NODE:		/* Lag Interface mode node. */
	case LAG_INTERFACE_L3_NODE:	/* Lag L3 Interface mode node. */
	case SERIAL_INTERFACE_NODE:
	case BRIGDE_INTERFACE_NODE:
#ifdef CUSTOM_INTERFACE
	case WIFI_INTERFACE_NODE:
	case MODEM_INTERFACE_NODE:
#endif
	case TRUNK_NODE:
	case ZEBRA_NODE:
	case BGP_NODE:
	case RIP_NODE:
	case RIPNG_NODE:
	case BABEL_NODE:
	case OSPF_NODE:
	case OSPF6_NODE:
	case ISIS_NODE:
	case KEYCHAIN_NODE:
	case MASC_NODE:
	case RMAP_NODE:
	case PIM_NODE:
	case VTY_NODE:

	case HSLS_NODE: /* HSLS protocol node. */
	case OLSR_NODE: /* OLSR protocol node. */
	case VRRP_NODE:
	case FRP_NODE: /* FRP protocol node */
	case LLDP_NODE:

	case BFD_NODE:
	case LDP_NODE:
    case VRF_NODE:
    case VLAN_DATABASE_NODE:
    case VLAN_NODE:

	case QOS_ACCESS_NODE:
	case QOS_CLASS_MAP_NODE:
    case QOS_POLICY_MAP_NODE:
		return CONFIG_NODE;
		break;

    case QOS_POLICY_CLASS_MAP_NODE:
		return QOS_POLICY_MAP_NODE;
		break;

	case BGP_IPV4_NODE:
	case BGP_IPV4M_NODE:
	case BGP_VPNV4_NODE:
	case BGP_VPNV6_NODE:
	case BGP_ENCAP_NODE:
	case BGP_ENCAPV6_NODE:
	case BGP_IPV6_NODE:
	case BGP_IPV6M_NODE:
		return BGP_NODE;
		break;
	case KEYCHAIN_KEY_NODE:
		return KEYCHAIN_NODE;
		break;
	case LINK_PARAMS_NODE:
		return INTERFACE_NODE;
		break;
	default:
		break;
	}
    return node;
}