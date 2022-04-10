#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "cli_node.h"
#include "vty.h"


static enum node_type vty_issubnode(enum node_type node)
{
	enum node_type ret;
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

/* When '^Z' is received from vty, move down to the enable mode. */
enum node_type cmd_end_node(struct vty *vty)
{
	switch (vty->node)
	{
	case VIEW_NODE:
	case ENABLE_NODE:
		/*case RESTRICTED_NODE:*/
		/* Nothing to do. */
		break;
	default:
		if (vty->node >= CONFIG_NODE && vty->node < CMD_NODE_MAX)
		{
			vty->index = NULL;
			vty->index_sub = NULL;
			vty->index_value = 0;
			vty->index_range = 0;
			memset(vty->vty_range_index, 0, sizeof(vty->vty_range_index));
			return ENABLE_NODE;
		}
		break;
	}
	vty->index = NULL;
	vty->index_sub = NULL;
	vty->index_value = 0;
	vty->index_range = 0;
	memset(vty->vty_range_index, 0, sizeof(vty->vty_range_index));
	return vty->node;
}

enum node_type cmd_stop_node(struct vty *vty)
{
	return cmd_end_node(vty);
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

enum node_type cmd_exit_node(struct vty *vty)
{
	enum node_type ret = vty->node;
	switch (vty->node)
	{
	case DHCPS_NODE:
	case TEMPLATE_NODE:
	case ALL_SERVICE_NODE:
	case MODEM_PROFILE_NODE:
	case MODEM_CHANNEL_NODE:
	case INTERFACE_NODE:
	case INTERFACE_L3_NODE: /* Interface mode node. */
	case WIRELESS_INTERFACE_NODE:
	case TUNNEL_INTERFACE_NODE:	  /* Tunnel Interface mode node. */
	case LOOPBACK_INTERFACE_NODE: /* Loopback Interface mode node. */
	case LAG_INTERFACE_NODE:	  /* Lag Interface mode node. */
	case LAG_INTERFACE_L3_NODE:	  /* Lag L3 Interface mode node. */
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
		ret = CONFIG_NODE;
		break;

	case QOS_POLICY_CLASS_MAP_NODE:
		ret = QOS_POLICY_MAP_NODE;
		break;

	case BGP_IPV4_NODE:
	case BGP_IPV4M_NODE:
	case BGP_VPNV4_NODE:
	case BGP_VPNV6_NODE:
	case BGP_ENCAP_NODE:
	case BGP_ENCAPV6_NODE:
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
	default:
		break;
	}
	if (vty_issubnode(vty->node) > CONFIG_NODE)
	{
		vty->index_sub = NULL;
	}
	else
	{
		vty->index = NULL;
		vty->index_sub = NULL;
		vty->index_value = 0;
		vty->index_range = 0;
		memset(vty->vty_range_index, 0, sizeof(vty->vty_range_index));
	}
	return ret;
}