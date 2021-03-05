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

#include <zebra.h>
#include "linklist.h"
#include "if.h"
#include "nsm_connected.h"
#include "log.h"
#include "prefix.h"
#include "table.h"
#include "memory.h"
#include "nsm_rib.h"
#include "thread.h"
#include "nsm_vrf.h"
#include "nexthop.h"

#include "nsm_zserv.h"

#include "nsm_redistribute.h"
#include "nsm_interface.h"
#include "nsm_debug.h"

#include "kernel_netlink.h"




/* Interface address modification. AF_UNSPEC */
static int netlink_ioctl_interface(ospl_uint32 cmd, ospl_family_t family, struct interface *ifp)
{
	ospl_bool create = ospl_false;
	ospl_uint32 flags = 0;
	struct
	{
		struct nlmsghdr n;
		struct ifinfomsg i;
		char buf[NL_PKT_BUF_SIZE];
	} req;
	struct rtattr *linkinfo;
	if (cmd == RTM_NEWLINK)
		flags |= NLM_F_CREATE|NLM_F_EXCL;
	if (cmd == RTM_DELLINK)
		flags |= 0;
	//if (cmd == RTM_NEWLINK)
	//	flags |= NLM_F_CREATE|NLM_F_REPLACE;

	struct nsm_vrf *zvrf = vrf_info_lookup(ifp->vrf_id);

	memset(&req, 0, sizeof req - NL_PKT_BUF_SIZE);

	//bytelen = (family == AF_INET ? 4 : 16);

	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
	req.n.nlmsg_flags = NLM_F_REQUEST | flags;
	req.n.nlmsg_type = cmd;

	req.i.ifi_family = family;
	req.i.__ifi_pad = 0;
	req.i.ifi_type = 0;		/* ARPHRD_* */
	if(!(flags & NLM_F_CREATE))
		req.i.ifi_index = if_nametoindex(ifp->k_name);
	req.i.ifi_flags = 0;
	req.i.ifi_change = 0;		/* IFF_* change mask */

	if (os_strlen(ifp->k_name))
	{
		addattr_l(&req.n, sizeof(req),
			  IFLA_IFNAME, ifp->k_name, strlen(ifp->k_name)+1);
	}


	linkinfo = addattr_nest(&req.n, sizeof(req), IFLA_LINKINFO);
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
		addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, "tunnel", strlen("tunnel"));
		create = ospl_true;
		break;
	case IF_LAG:
		addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, "bond", strlen("bond"));
		create = ospl_true;
		break;
	case IF_BRIGDE:		//brigde interface
		addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, "bridge", strlen("bridge"));
		create = ospl_true;
		break;
	case IF_VLAN:
		addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, "vlan", strlen("vlan"));
		create = ospl_true;
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
	addattr_nest_end(&req.n, linkinfo);
	if(create == ospl_true)
		return netlink_talk(&req.n, &zvrf->netlink_cmd, zvrf);
	return ERROR;
}


int kernel_create_interface(struct interface *ifp)
{
	return netlink_ioctl_interface(RTM_NEWLINK, AF_UNSPEC, ifp);
}

int kernel_destroy_interface(struct interface *ifp)
{
	return netlink_ioctl_interface(RTM_DELLINK, AF_UNSPEC, ifp);
}

