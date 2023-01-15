/*
 * cmd_tunnel.c
 *
 *  Created on: Sep 15, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include <zplos_include.h>
#include "route_types.h"
#include "nsm_event.h"
#include "zmemory.h"
#include "if.h"
#include "nsm_interface.h"

#include "prefix.h"
#include "command.h"
#include "nsm_tunnel.h"


DEFUN (nsm_tunnel_protocol,
		nsm_tunnel_protocol_cmd,
		"tunnel protocol (ipip|gre|sit)",
		"Tunnel Interface configure\n"
		"tunnel protocol\n"
		"IPIP Encapsulation\n"
		"Generic routing Encapsulation\n"
		"IPv6 Encapsulation\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	tunnel_mode mode = NSM_TUNNEL_NONE, oldmode = NSM_TUNNEL_NONE;
	if (argc == 0)
		return CMD_SUCCESS;
	if(nsm_tunnel_mode_get_api(ifp, &oldmode) == OK)
	{
		if(os_strncmp(argv[0], "ipip", 3) == 0)
			mode = NSM_TUNNEL_IPIP;
		else if(os_strncmp(argv[0], "gre", 3) == 0)
			mode = NSM_TUNNEL_GRE;
		else if(os_strncmp(argv[0], "sit", 3) == 0)
			mode = NSM_TUNNEL_SIT;
		if(oldmode != mode)
		{
			ret = nsm_tunnel_mode_set_api(ifp, mode);
		}
		else
			ret = OK;
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_nsm_tunnel_protocol,
		no_nsm_tunnel_protocol_cmd,
		"no tunnel protocol",
		NO_STR
		"Tunnel Interface configure\n"
		"tunnel protocol\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	tunnel_mode mode = NSM_TUNNEL_NONE, oldmode = NSM_TUNNEL_NONE;
	if (argc == 0)
		return CMD_SUCCESS;
	if(nsm_tunnel_mode_get_api(ifp, &oldmode) == OK)
	{
		if(oldmode != mode)
		{
			ret = nsm_tunnel_mode_set_api(ifp, mode);
		}
		else
			ret = OK;
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (nsm_tunnel_tos,
		nsm_tunnel_tos_cmd,
		"tunnel tos <0-64>",
		"Tunnel Interface configure\n"
		"tunnel tos\n"
		"Tos\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	int tos = 0, oldtos = 0;
	if(nsm_tunnel_tos_get_api(ifp, &oldtos) == OK)
	{
		tos = atoi(argv[0]);
		if(oldtos != tos)
		{
			ret = nsm_tunnel_tos_set_api(ifp, tos);
		}
		else
			ret = OK;
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_nsm_tunnel_tos,
		no_nsm_tunnel_tos_cmd,
		"no tunnel tos",
		NO_STR
		"Tunnel Interface configure\n"
		"tunnel tos\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	int tos = 0, oldtos = 0;
	if(nsm_tunnel_tos_get_api(ifp, &oldtos) == OK)
	{
		if(oldtos != tos)
		{
			ret = nsm_tunnel_tos_set_api(ifp, tos);
		}
		else
			ret = OK;
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (nsm_tunnel_ttl,
		nsm_tunnel_ttl_cmd,
		"tunnel ttl <0-64>",
		"Tunnel Interface configure\n"
		"tunnel ttl\n"
		"ttl\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	int ttl = 0, oldttl = 0;
	if(nsm_tunnel_ttl_get_api(ifp, &oldttl) == OK)
	{
		ttl = atoi(argv[0]);
		if(oldttl != ttl)
		{
			ret = nsm_tunnel_ttl_set_api(ifp, ttl);
		}
		else
			ret = OK;
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_nsm_tunnel_ttl,
		no_nsm_tunnel_ttl_cmd,
		"no tunnel ttl",
		NO_STR
		"Tunnel Interface configure\n"
		"tunnel ttl\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	int ttl = 0, oldttl = 0;
	if(nsm_tunnel_ttl_get_api(ifp, &oldttl) == OK)
	{
		if(oldttl != ttl)
		{
			ret = nsm_tunnel_ttl_set_api(ifp, ttl);
		}
		else
			ret = OK;
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (nsm_tunnel_mtu,
		nsm_tunnel_mtu_cmd,
		"tunnel mtu <0-64>",
		"Tunnel Interface configure\n"
		"tunnel mtu\n"
		"mtu\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	int mtu = 0, oldmtu = 0;
	if(nsm_tunnel_mtu_get_api(ifp, &oldmtu) == OK)
	{
		mtu = atoi(argv[0]);
		if(oldmtu != mtu)
		{
			ret = nsm_tunnel_mtu_set_api(ifp, mtu);
		}
		else
			ret = OK;
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_nsm_tunnel_mtu,
		no_nsm_tunnel_mtu_cmd,
		"no tunnel mtu",
		NO_STR
		"Tunnel Interface configure\n"
		"tunnel mtu\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	int mtu = 0, oldmtu = 0;
	if(nsm_tunnel_mtu_get_api(ifp, &oldmtu) == OK)
	{
		if(oldmtu != mtu)
		{
			ret = nsm_tunnel_mtu_set_api(ifp, mtu);
		}
		else
			ret = OK;
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (nsm_tunnel_source,
		nsm_tunnel_source_cmd,
		"tunnel source " CMD_KEY_IPV4,
		"Tunnel Interface configure\n"
		"tunnel source address\n"
		CMD_KEY_IPV4_HELP)
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	struct prefix  source, oldsource;

	os_memset(&source, 0, sizeof(source));
	os_memset(&oldsource, 0, sizeof(oldsource));

	if(nsm_tunnel_source_get_api(ifp, &oldsource) == OK)
	{
		str2prefix (argv[0], &source);
		if(IPV4_NET127(source.u.prefix4.s_addr))
		{
			vty_out (vty, "%% Lookback address%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		if(IPV4_MULTICAST(source.u.prefix4.s_addr))
		{
			vty_out (vty, "%% Multicast address%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		if(!prefix_same(&source, &oldsource))
		{
			ret = nsm_tunnel_source_set_api(ifp, &source);
		}
		else
			ret = OK;
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_nsm_tunnel_source,
		no_nsm_tunnel_source_cmd,
		"no tunnel source",
		NO_STR
		"Tunnel Interface configure\n"
		"tunnel source address\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	struct prefix  source, oldsource;

	os_memset(&source, 0, sizeof(source));
	os_memset(&oldsource, 0, sizeof(oldsource));

	if(nsm_tunnel_source_get_api(ifp, &oldsource) == OK)
	{
		if(!prefix_same(&source, &oldsource))
		{
			ret = nsm_tunnel_source_set_api(ifp, &source);
		}
		else
			ret = OK;
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (nsm_tunnel_destination,
		nsm_tunnel_destination_cmd,
		"tunnel destination " CMD_KEY_IPV4,
		"Tunnel Interface configure\n"
		"tunnel destination address\n"
		CMD_KEY_IPV4_HELP)
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	struct prefix  dest, olddest;

	os_memset(&dest, 0, sizeof(dest));
	os_memset(&olddest, 0, sizeof(olddest));

	if(nsm_tunnel_destination_get_api(ifp, &olddest) == OK)
	{
		str2prefix (argv[0], &dest);
		if(IPV4_NET127(dest.u.prefix4.s_addr))
		{
			vty_out (vty, "%% Lookback address%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		if(IPV4_MULTICAST(dest.u.prefix4.s_addr))
		{
			vty_out (vty, "%% Multicast address%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		if(!prefix_same(&dest, &olddest))
		{
			ret = nsm_tunnel_destination_set_api(ifp, &dest);
		}
		else
			ret = OK;
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_nsm_tunnel_destination,
		no_nsm_tunnel_destination_cmd,
		"no tunnel destination",
		NO_STR
		"Tunnel Interface configure\n"
		"tunnel destination address\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	struct prefix  dest, olddest;

	os_memset(&dest, 0, sizeof(dest));
	os_memset(&olddest, 0, sizeof(olddest));

	if(nsm_tunnel_destination_get_api(ifp, &olddest) == OK)
	{
		if(!prefix_same(&dest, &olddest))
		{
			ret = nsm_tunnel_destination_set_api(ifp, &dest);
		}
		else
			ret = OK;
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


static int nsm_interface_tunnel_config_write(struct vty *vty)
{
	struct listnode *node = NULL;
	struct interface *ifp = NULL;
	struct list *if_list = NULL;
	struct nsm_interface *if_data = NULL;
	struct listnode *addrnode = NULL;
	struct connected *ifc = NULL;
	struct prefix *p = NULL;
	nsm_tunnel_t * tunnel = NULL;
	union prefix46constptr up;
	if_list = if_list_get();
	if (if_list)
	{
		IF_MASTER_LOCK();
		for (ALL_LIST_ELEMENTS_RO(if_list, node, ifp))
		{
			if (!if_is_tunnel(ifp))
				continue;

			if_data = ifp->info[MODULE_NSM];

			vty_out(vty, "interface %s%s", ifp->name, VTY_NEWLINE);

			if (ifp->desc)
				vty_out(vty, " description %s%s", ifp->desc, VTY_NEWLINE);

			if (ifp->if_mode == IF_MODE_L3)
			{
/*
#ifdef ZPL_IPCOM_MODULE
				if (ifp->vrf_id != VRF_DEFAULT)
				vty_out(vty, " ip forward vrf %s%s", ip_vrf_vrfid2name(ifp->vrf_id), VTY_NEWLINE);
#endif
				if (ifp->mtu != IF_MTU_DEFAULT)
					vty_out(vty, " mtu %d%s", ifp->mtu, VTY_NEWLINE);
				if (ifp->mtu6 != IF_MTU_DEFAULT)
					vty_out(vty, " mtu6 %d%s", ifp->mtu, VTY_NEWLINE);

				if (ifp->bandwidth != 0)
					vty_out(vty, " bandwidth %u%s", ifp->bandwidth,
							VTY_NEWLINE);

				if (ifp->if_enca != IF_ENCA_NONE)
					vty_out(vty, " encapsulation %s%s",
							if_enca_string(ifp->if_enca), VTY_NEWLINE);
*/
				for (ALL_LIST_ELEMENTS_RO(ifp->connected, addrnode, ifc))
				{
					if (CHECK_FLAG(ifc->conf, IF_IFC_CONFIGURED))
					{
						char buf[INET6_ADDRSTRLEN];
						p = ifc->address;
						up.p = p;
						vty_out(vty, " ip%s address %s",
									p->family == IPSTACK_AF_INET ? "" : "v6",
									prefix2str(up, buf, sizeof(buf)));
						vty_out(vty, "%s", VTY_NEWLINE);
					}
				}
				tunnel = nsm_tunnel_get(ifp);
				if(tunnel)
				{
					switch(tunnel->mode)
					{
					case NSM_TUNNEL_IPIP:
						vty_out(vty, " tunnel protocol ipip%s", VTY_NEWLINE);
						break;
					case NSM_TUNNEL_GRE:
						vty_out(vty, " tunnel protocol gre%s", VTY_NEWLINE);
						break;
					case NSM_TUNNEL_SIT:
						vty_out(vty, " tunnel protocol sit%s", VTY_NEWLINE);
						break;
					case NSM_TUNNEL_NONE:
					case NSM_TUNNEL_VTI:
					case NSM_TUNNEL_IPIPV6:
					case NSM_TUNNEL_GREV6:
					default:
						break;
					}
					vty_out(vty, " tunnel source %s%s", ipstack_inet_ntoa(tunnel->source.u.prefix4), VTY_NEWLINE);
					vty_out(vty, " tunnel destination %s%s", ipstack_inet_ntoa(tunnel->remote.u.prefix4), VTY_NEWLINE);

					if(tunnel->tun_mtu != NSM_TUNNEL_MTU_DEFAULT)
						vty_out(vty, " tunnel mtu %d%s", tunnel->tun_mtu, VTY_NEWLINE);

					if(tunnel->tun_ttl != NSM_TUNNEL_TTL_DEFAULT)
						vty_out(vty, " tunnel ttl %d%s", tunnel->tun_ttl, VTY_NEWLINE);

					if(tunnel->tun_tos != NSM_TUNNEL_TOS_DEFAULT)
						vty_out(vty, " tunnel tos %d%s", tunnel->tun_tos, VTY_NEWLINE);
				}
			}
			if (if_data)
			{
				if (if_data->shutdown == NSM_IF_SHUTDOWN_ON)
					vty_out(vty, " shutdown%s", VTY_NEWLINE);
			}
			vty_out(vty, "!%s", VTY_NEWLINE);
		}
		IF_MASTER_UNLOCK();
	}
	return OK;
}

static void cmd_tunnel_interface_init(int node)
{
	install_element(node, CMD_CONFIG_LEVEL, &nsm_tunnel_protocol_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_nsm_tunnel_protocol_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &nsm_tunnel_ttl_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_nsm_tunnel_ttl_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &nsm_tunnel_mtu_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_nsm_tunnel_mtu_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &nsm_tunnel_tos_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_nsm_tunnel_tos_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &nsm_tunnel_source_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_nsm_tunnel_source_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &nsm_tunnel_destination_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_nsm_tunnel_destination_cmd);
}


void cmd_tunnel_init(void)
{
	reinstall_node(TUNNEL_INTERFACE_NODE, nsm_interface_tunnel_config_write);
	cmd_tunnel_interface_init(TUNNEL_INTERFACE_NODE);
}

