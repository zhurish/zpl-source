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


/* Interface address modification. */
static int netlink_address(ospl_uint32 cmd, ospl_family_t family, struct interface *ifp,
		struct connected *ifc)
{
	ospl_uint32 bytelen;
	struct prefix *p;

	struct
	{
		struct nlmsghdr n;
		struct ifaddrmsg ifa;
		char buf[NL_PKT_BUF_SIZE];
	} req;

	struct nsm_vrf *zvrf = vrf_info_lookup(ifp->vrf_id);

	p = ifc->address;
	memset(&req, 0, sizeof req - NL_PKT_BUF_SIZE);

	bytelen = (family == AF_INET ? 4 : 16);

	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
	req.n.nlmsg_flags = NLM_F_REQUEST;
	req.n.nlmsg_type = cmd;
	req.ifa.ifa_family = family;

	req.ifa.ifa_index = ifp->k_ifindex;
	req.ifa.ifa_prefixlen = p->prefixlen;

	addattr_l(&req.n, sizeof req, IFA_LOCAL, &p->u.prefix, bytelen);

	if (family == AF_INET && cmd == RTM_NEWADDR)
	{
		if (!CONNECTED_PEER(ifc) && ifc->destination)
		{
			p = ifc->destination;
			addattr_l(&req.n, sizeof req, IFA_BROADCAST, &p->u.prefix, bytelen);
		}
	}

	if (CHECK_FLAG(ifc->flags, ZEBRA_IFA_SECONDARY))
		SET_FLAG(req.ifa.ifa_flags, IFA_F_SECONDARY);

	if (IS_ZEBRA_DEBUG_KERNEL)
		zlog_debug(MODULE_PAL, "netlink %s address on %s(%d)", (cmd == RTM_NEWADDR) ? "add":"del",
				ifp->name, req.ifa.ifa_index);
	/*
	 if (ifc->label)
	 addattr_l (&req.n, sizeof req, IFA_LABEL, ifc->label,
	 strlen (ifc->label) + 1);
	 */
	return netlink_talk(&req.n, &zvrf->netlink_cmd, zvrf);
}

int kernel_address_add_ipv4(struct interface *ifp, struct connected *ifc)
{
	return netlink_address(RTM_NEWADDR, AF_INET, ifp, ifc);
}

int kernel_address_delete_ipv4(struct interface *ifp, struct connected *ifc)
{
	return netlink_address(RTM_DELADDR, AF_INET, ifp, ifc);
}

