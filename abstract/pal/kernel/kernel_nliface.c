/* Kernel routing table updates using netlink over GNU/Linux system.
 * Copyright (C) 1997, 98, 99 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"

#include "kernel_netlink.h"



#ifdef ZPL_KERNEL_STACK_NETLINK
/* Interface address modification. IPSTACK_AF_UNSPEC */
static int _netlink_ioctl_interface(zpl_uint32 cmd, zpl_family_t family, struct interface *ifp)
{
	zpl_bool create = zpl_false;
	zpl_uint32 flags = 0;
	struct
	{
		struct ipstack_nlmsghdr n;
		struct ipstack_ifinfomsg i;
		char buf[NL_PKT_BUF_SIZE];
	} req;
	struct ipstack_rtattr *linkinfo;
	if (cmd == IPSTACK_RTM_NEWLINK)
		flags |= IPSTACK_NLM_F_CREATE|NLM_F_EXCL;
	if (cmd == IPSTACK_RTM_DELLINK)
		flags |= 0;
	//if (cmd == IPSTACK_RTM_NEWLINK)
	//	flags |= IPSTACK_NLM_F_CREATE|IPSTACK_NLM_F_REPLACE;

	struct nsm_ip_vrf *zvrf = ip_vrf_info_lookup(ifp->vrf_id);

	memset(&req, 0, sizeof req - NL_PKT_BUF_SIZE);

	//bytelen = (family == IPSTACK_AF_INET ? 4 : 16);

	req.n.nlmsg_len = IPSTACK_NLMSG_LENGTH(sizeof(struct ipstack_ifinfomsg));
	req.n.nlmsg_flags = IPSTACK_NLM_F_REQUEST | flags;
	req.n.nlmsg_type = cmd;

	req.i.ifi_family = family;
	req.i.__ifi_pad = 0;
	req.i.ifi_type = 0;		/* ARPHRD_* */
	if(!(flags & IPSTACK_NLM_F_CREATE))
		req.i.ifi_index = if_nametoindex(ifp->k_name);
	req.i.ifi_flags = 0;
	req.i.ifi_change = 0;		/* IFF_* change mask */

	if (os_strlen(ifp->k_name))
	{
		_netlink_addattr_l(&req.n, sizeof(req),
			  IPSTACK_IFLA_IFNAME, ifp->k_name, strlen(ifp->k_name)+1);
	}


	linkinfo = _netlink_addattr_nest(&req.n, sizeof(req), IPSTACK_IFLA_LINKINFO);
	/*
	 * TYPE := { vlan | veth | vcan | dummy | ifb | macvlan | macvtap |
          bridge | bond | team | ipoib | ip6tnl | ipip | sit | vxlan |
          gre | gretap | ip6gre | ip6gretap | vti | nlmon | team_slave |
          bond_slave | ipvlan | geneve | bridge_slave | vrf | macsec }
	 */
	switch(ifp->if_type)
	{
	case IF_LOOPBACK:
		break;
	case IF_SERIAL:
		break;
	case IF_ETHERNET:
		break;
	case IF_GIGABT_ETHERNET:
		break;
	case IF_TUNNEL:
		_netlink_addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, "tunnel", strlen("tunnel"));
		create = zpl_true;
		break;
	case IF_LAG:
		_netlink_addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, "bond", strlen("bond"));
		create = zpl_true;
		break;
	case IF_BRIGDE:		//brigde interface
		_netlink_addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, "bridge", strlen("bridge"));
		create = zpl_true;
		break;
	case IF_VLAN:
		_netlink_addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, "vlan", strlen("vlan"));
		create = zpl_true;
		break;
#ifdef CUSTOM_INTERFACE
	case IF_WIFI:		//wifi interface
		break;
	case IF_MODEM:		//modem interface
		break;
#endif
	default:
		break;
	}
	_netlink_addattr_nest_end(&req.n, linkinfo);
	if(create == zpl_true)
		return _netlink_talk(&req.n, &zvrf->netlink_cmd, zvrf);
	return ERROR;
}


int _netlink_create_interface(struct interface *ifp)
{
	return _netlink_ioctl_interface(IPSTACK_RTM_NEWLINK, IPSTACK_AF_UNSPEC, ifp);
}

int _netlink_destroy_interface(struct interface *ifp)
{
	return _netlink_ioctl_interface(IPSTACK_RTM_DELLINK, IPSTACK_AF_UNSPEC, ifp);
}
#endif
