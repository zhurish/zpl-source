/*
 * iplink_vxlan.c	VXLAN device support
 *
 *              This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU General Public License
 *              as published by the Free Software Foundation; either version
 *              2 of the License, or (at your option) any later version.
 *
 * Authors:     Stephen Hemminger <shemminger@vyatta.com
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"
#include "thread.h"
#include "if.h"
#include "vty.h"
#include "vrf.h"
#include "command.h"
#include "prefix.h"

#include "pal_include.h"
#include "nsm_debug.h"
#include "nsm_vlan.h"
#include "nsm_arp.h"
#include "nsm_include.h"
#include "nsm_firewalld.h"
#include "nsm_vlaneth.h"
#include "linux_driver.h"

#ifdef ZPL_HISIMPP_MODULE
int vxlan_parse_opt(int v6, struct vxlan_param *param,
						   struct nlmsghdr *n)
{
    return - 1;
}
#else
int vxlan_parse_opt(int v6, struct vxlan_param *param,
						   struct nlmsghdr *n)
{
	if (param->id)
	{
		librtnl_addattr32(n, 1024, IFLA_VXLAN_ID, param->id);
	}
	else if (param->group)
	{
		librtnl_addattr_l(n, 1024, v6?IFLA_VXLAN_GROUP6:IFLA_VXLAN_GROUP,
				  param->address, v6?16:4);
	}
	else if (param->group == 0)
	{
		librtnl_addattr_l(n, 1024, v6?IFLA_VXLAN_GROUP6:IFLA_VXLAN_GROUP,
				  param->address, v6?16:4);
		// addattr_l(n, 1024, type, daddr.data, daddr.bytelen);
	}
	else if (param->localaddr)
	{
		librtnl_addattr_l(n, 1024, v6?IFLA_VXLAN_LOCAL6:IFLA_VXLAN_LOCAL, param->localaddr, v6?16:4);
	}
	else if (param->ifindex)
	{
		librtnl_addattr32(n, 1024, IFLA_VXLAN_LINK, param->ifindex);
	}
	else if (param->ttl)
	{
		if (param->inherit)
		{
			librtnl_addattr(n, 1024, 28/*IFLA_VXLAN_TTL_INHERIT*/);
		}
		else
		{
			librtnl_addattr8(n, 1024, 5/*IFLA_VXLAN_TTL*/, param->ttl);
		}
	}
	else if (param->tos)
	{
		librtnl_addattr8(n, 1024, IFLA_VXLAN_TOS, param->tos);
	}
	else if (param->df >= 0)
	{
		librtnl_addattr8(n, 1024, 29/*IFLA_VXLAN_DF*/, param->df);
	}
	else if (param->label)
	{
		librtnl_addattr32(n, 1024, IFLA_VXLAN_LABEL, htonl(param->label));
	}
	else if (param->ageing)
	{
		librtnl_addattr32(n, 1024, IFLA_VXLAN_AGEING, param->ageing);
	}
	else if (param->maxaddress)
	{
		librtnl_addattr32(n, 1024, IFLA_VXLAN_LIMIT, param->maxaddress);
	}
	else if (param->srcport)
	{
		struct ifla_vxlan_port_range range = {param->srcport, param->srcport_max};

		if (range.low || range.high)
		{
			librtnl_addattr_l(n, 1024, IFLA_VXLAN_PORT_RANGE,
					  &range, sizeof(range));
		}
	}
	else if (param->dstport)
	{
		librtnl_addattr16(n, 1024, IFLA_VXLAN_PORT, htons(param->dstport));
	}
	else if (param->learning)
	{
		librtnl_addattr8(n, 1024, IFLA_VXLAN_LEARNING, param->learning);
	}
	else if (param->proxy)
	{
		librtnl_addattr8(n, 1024, IFLA_VXLAN_PROXY, param->proxy);
	}
	else if (param->rsc)
	{
		librtnl_addattr8(n, 1024, IFLA_VXLAN_RSC, param->rsc);
	}
	else if (param->l2miss)
	{
		librtnl_addattr8(n, 1024, IFLA_VXLAN_L2MISS, param->l2miss);
	}
	else if (param->l3miss)
	{
		librtnl_addattr8(n, 1024, IFLA_VXLAN_L3MISS, param->l3miss);
	}
	else if (param->udpcsum)
	{
		librtnl_addattr8(n, 1024, IFLA_VXLAN_UDP_CSUM, param->udpcsum);
	}
	else if (param->udp6zerocsumtx)
	{
		librtnl_addattr8(n, 1024, IFLA_VXLAN_UDP_ZERO_CSUM6_TX, param->udp6zerocsumtx);
	}
	else if (param->udp6zerocsumrx)
	{
		librtnl_addattr8(n, 1024, IFLA_VXLAN_UDP_ZERO_CSUM6_RX, param->udp6zerocsumrx);
	}
	else if (param->remcsumtx)
	{
		librtnl_addattr8(n, 1024, IFLA_VXLAN_REMCSUM_TX, param->remcsumtx);
	}
	else if (param->remcsumrx)
	{
		librtnl_addattr8(n, 1024, IFLA_VXLAN_REMCSUM_RX, param->remcsumrx);
	}
	return 0;
}
#endif