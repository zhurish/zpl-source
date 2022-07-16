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

#include "auto_include.h"
#include "zplos_include.h"
#include "zebra_event.h"
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
#include "nsm_include.h"
#include "linux_driver.h"



#ifdef ZPL_KERNEL_NETLINK
/* Interface address modification. IPSTACK_AF_UNSPEC */
static int linux_netlink_ioctl_interface(zpl_uint32 cmd, zpl_family_t family, struct interface *ifp)
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
		linux_netlink_addattr_l(&req.n, sizeof(req),
			  IPSTACK_IFLA_IFNAME, ifp->k_name, strlen(ifp->k_name)+1);
	}


	linkinfo = linux_netlink_addattr_nest(&req.n, sizeof(req), IPSTACK_IFLA_LINKINFO);
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
		linux_netlink_addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, "tunnel", strlen("tunnel"));
		create = zpl_true;
		break;
	case IF_LAG:
		linux_netlink_addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, "bond", strlen("bond"));
		create = zpl_true;
		break;
	case IF_BRIGDE:		//brigde interface
		linux_netlink_addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, "bridge", strlen("bridge"));
		create = zpl_true;
		break;
	case IF_VLAN:
		linux_netlink_addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, "vlan", strlen("vlan"));
		create = zpl_true;
		break;
#ifdef CUSTOM_INTERFACE
	case IF_WIFI:		//wifi interface
		break;
	case IF_MODEM:		//modem interface
		break;
#endif
	default:
	//linux_netlink_addattr32(&req.n, sizeof(req), IFLA_VRF_TABLE, table);
	//linux_netlink_addattr_l(&req.n, sizeof(req), IFLA_VRF_TABLE, "vlan", strlen("vlan"));
	//addattr32(n, 1024, IFLA_VRF_TABLE, table);
		break;
	}
	//data = addattr_nest(&req.n, sizeof(req), iflatype);

	linux_netlink_addattr_nest_end(&req.n, linkinfo);
	if(create == zpl_true)
		return linux_netlink_talk(&req.n, &netlink_cmd, zvrf->vrf_id);
	return ERROR;
}


int linux_netlink_create_interface(struct interface *ifp)
{
	return linux_netlink_ioctl_interface(IPSTACK_RTM_NEWLINK, IPSTACK_AF_UNSPEC, ifp);
}

int linux_netlink_destroy_interface(struct interface *ifp)
{
	return linux_netlink_ioctl_interface(IPSTACK_RTM_DELLINK, IPSTACK_AF_UNSPEC, ifp);
}

#if 0
static int iplink_mdf(zpl_uint32 cmd, zpl_family_t family, int if_type, char *k_name)
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
	struct ipstack_rtattr *rtdata;
	if (cmd == IPSTACK_RTM_NEWLINK)
		flags |= IPSTACK_NLM_F_CREATE|NLM_F_EXCL;
	if (cmd == IPSTACK_RTM_DELLINK)
		flags |= 0;
	//if (cmd == IPSTACK_RTM_NEWLINK)
	//	flags |= IPSTACK_NLM_F_CREATE|IPSTACK_NLM_F_REPLACE;

	memset(&req, 0, sizeof req - NL_PKT_BUF_SIZE);

	//bytelen = (family == IPSTACK_AF_INET ? 4 : 16);

	req.n.nlmsg_len = IPSTACK_NLMSG_LENGTH(sizeof(struct ipstack_ifinfomsg));
	req.n.nlmsg_flags = IPSTACK_NLM_F_REQUEST | flags;
	req.n.nlmsg_type = cmd;

	req.i.ifi_family = family;
	req.i.__ifi_pad = 0;
	req.i.ifi_type = 0;		/* ARPHRD_* */
	if(!(flags & IPSTACK_NLM_F_CREATE))
		req.i.ifi_index = if_nametoindex(k_name);
	req.i.ifi_flags = 0;
	req.i.ifi_change = 0;		/* IFF_* change mask */

	if (os_strlen(k_name))
	{
		linux_netlink_addattr_l(&req.n, sizeof(req),
			  IPSTACK_IFLA_IFNAME, k_name, strlen(k_name)+1);
	}
	//linux_netlink_addattr32(&req.n, sizeof(req), IPSTACK_IFLA_LINK, ifindex);


	linkinfo = linux_netlink_addattr_nest(&req.n, sizeof(req), IPSTACK_IFLA_LINKINFO);
	/*
	 * TYPE := { vlan | veth | vcan | dummy | ifb | macvlan | macvtap |
          bridge | bond | team | ipoib | ip6tnl | ipip | sit | vxlan |
          gre | gretap | ip6gre | ip6gretap | vti | nlmon | team_slave |
          bond_slave | ipvlan | geneve | bridge_slave | vrf | macsec }
	 */
	switch(if_type)
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
		linux_netlink_addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, "tunnel", strlen("tunnel"));
		create = zpl_true;
		break;
	case IF_LAG:
		linux_netlink_addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, "bond", strlen("bond"));
		create = zpl_true;
		break;
	case IF_BRIGDE:		//brigde interface
		linux_netlink_addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, "bridge", strlen("bridge"));
		create = zpl_true;
		break;
	case IF_VLAN:
		linux_netlink_addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, "vlan", strlen("vlan"));
		create = zpl_true;
		break;
#ifdef CUSTOM_INTERFACE
	case IF_WIFI:		//wifi interface
		break;
	case IF_MODEM:		//modem interface
		break;
#endif
	default:
	//linux_netlink_addattr32(&req.n, sizeof(req), IFLA_VRF_TABLE, table);
	//linux_netlink_addattr_l(&req.n, sizeof(req), IFLA_VRF_TABLE, "vlan", strlen("vlan"));
	//addattr32(n, 1024, IFLA_VRF_TABLE, table);
		break;
	}
	//iflatype = IFLA_INFO_DATA;
	rtdata = linux_netlink_addattr_nest(&req.n, sizeof(req), IFLA_INFO_DATA);

	linux_netlink_addattr_nest_end(&req.n, rtdata);

	linux_netlink_addattr_nest_end(&req.n, linkinfo);
	if(create == zpl_true)
		return linux_netlink_talk(&req.n, &netlink_cmd, 0);
	return ERROR;
}
#endif

//ip link add test type vrf table 200
//ip link set dev red up
// ip link set ens38 master green
//ip link set dev ens37 nomaster
//iplink_vrftable IPSTACK_RTM_NEWLINK test 200
static int iplink_vrftable(int cmd, int id, char *name, int table)
{
	zpl_uint32 flags = 0;
	struct
	{
		struct ipstack_nlmsghdr n;
		struct ipstack_ifinfomsg i;
		char buf[NL_PKT_BUF_SIZE];
	} req;
	struct ipstack_rtattr *linkinfo;
	struct ipstack_rtattr *rtdata;
	if (cmd == IPSTACK_RTM_NEWLINK)
		flags |= IPSTACK_NLM_F_CREATE | NLM_F_EXCL;
	if (cmd == IPSTACK_RTM_DELLINK)
		flags |= 0;

	memset(&req, 0, sizeof req - NL_PKT_BUF_SIZE);

	req.n.nlmsg_len = IPSTACK_NLMSG_LENGTH(sizeof(struct ipstack_ifinfomsg));
	req.n.nlmsg_flags = IPSTACK_NLM_F_REQUEST | flags;
	req.n.nlmsg_type = cmd;

	req.i.ifi_family = IPSTACK_AF_UNSPEC;
	req.i.__ifi_pad = 0;
	req.i.ifi_type = 0;
	req.i.ifi_flags = 0;
	req.i.ifi_change = 0;

	req.i.ifi_index = id;
	if (os_strlen(name))
	{
		linux_netlink_addattr_l(&req.n, sizeof(req),
								IPSTACK_IFLA_IFNAME, name, strlen(name) + 1);
	}
	// linux_netlink_addattr32(&req.n, sizeof(req), IPSTACK_IFLA_LINK, ifindex);

	linkinfo = linux_netlink_addattr_nest(&req.n, sizeof(req), IPSTACK_IFLA_LINKINFO);
	linux_netlink_addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, "vrf", strlen("vrf"));
	// iflatype = IFLA_INFO_DATA;
	rtdata = linux_netlink_addattr_nest(&req.n, sizeof(req), IFLA_INFO_DATA);
	linux_netlink_addattr32(&req.n, sizeof(req), IFLA_VRF_TABLE, table);
	linux_netlink_addattr_nest_end(&req.n, rtdata);

	linux_netlink_addattr_nest_end(&req.n, linkinfo);

	return linux_netlink_talk(&req.n, &netlink_cmd, 0);
}

static int iplink_vrftable_updown(int cmd, int id, int up)
{
	struct
	{
		struct ipstack_nlmsghdr n;
		struct ipstack_ifinfomsg i;
		char buf[NL_PKT_BUF_SIZE];
	} req;
	/*
		if (cmd == IPSTACK_RTM_NEWLINK)
			flags |= 0;
		if (cmd == IPSTACK_RTM_DELLINK)
			flags |= 0;
	*/
	memset(&req, 0, sizeof req - NL_PKT_BUF_SIZE);

	req.n.nlmsg_len = IPSTACK_NLMSG_LENGTH(sizeof(struct ipstack_ifinfomsg));
	req.n.nlmsg_flags = IPSTACK_NLM_F_REQUEST | 0;
	req.n.nlmsg_type = cmd;

	req.i.ifi_family = IPSTACK_AF_UNSPEC;
	req.i.__ifi_pad = 0;
	req.i.ifi_type = 0;
	req.i.ifi_flags = 0;
	req.i.ifi_change = 0;

	req.i.ifi_index = id;//ll_name_to_index(name);
	if (up)
	{
		req.i.ifi_change |= IFF_UP;
		req.i.ifi_flags |= IFF_UP;
		printf("=====%s=%d===== up\r\n", __func__, __LINE__);
	}
	else // if (strcmp(*argv, "down") == 0)
	{
		req.i.ifi_change |= IFF_UP;
		req.i.ifi_flags &= ~IFF_UP;
	}

	return linux_netlink_talk(&req.n, &netlink_cmd, 0);
}

static int iplink_vrftable_add(int cmd, int id, char *dev)
{
	int flags = 0;
	struct
	{
		struct ipstack_nlmsghdr n;
		struct ipstack_ifinfomsg i;
		char buf[NL_PKT_BUF_SIZE];
	} req;
	//struct ipstack_rtattr *linkinfo;
	//struct ipstack_rtattr *rtdata;
	if (cmd == IPSTACK_RTM_NEWLINK)
		flags |= 0;
	if (cmd == IPSTACK_RTM_DELLINK)
		flags |= 0;

	memset(&req, 0, sizeof req - NL_PKT_BUF_SIZE);

	req.n.nlmsg_len = IPSTACK_NLMSG_LENGTH(sizeof(struct ipstack_ifinfomsg));
	req.n.nlmsg_flags = IPSTACK_NLM_F_REQUEST | flags;
	req.n.nlmsg_type = cmd;

	req.i.ifi_family = IPSTACK_AF_UNSPEC;
	req.i.__ifi_pad = 0;
	req.i.ifi_type = 0;
	req.i.ifi_flags = 0;
	req.i.ifi_change = 0;

	req.i.ifi_index = if_nametoindex(dev);

	int ifindex = id;
	//ifindex = if_nametoindex(name);
	linux_netlink_addattr_l(&req.n, sizeof(req), IFLA_MASTER, &ifindex, 4);
	return linux_netlink_talk(&req.n, &netlink_cmd, 0);
}

DEFUN (iplink_vrf_test,
		iplink_vrf_test_cmd,
		"ipvrf test create NAME id <0-100000> table <1-10000>",
		"ipvrf\n"
		"test information\n"
		"create\n"
		"Name\n"
		"id\n"
		"id value\n"
		"table\n"
		"table value")
{
	iplink_vrftable(IPSTACK_RTM_NEWLINK, atoi(argv[1]), argv[0], atoi(argv[2]));
	return CMD_SUCCESS;
}

DEFUN (iplink_vrf_test_up,
		iplink_vrf_test_up_cmd,
		"ipvrf test id <0-100000> (up|down)",
		"ipvrf\n"
		"test information\n"
		"id\n"
		"id value\n"
		"up\n"
		"down")
{
	iplink_vrftable_updown(IPSTACK_RTM_NEWLINK, atoi(argv[0]), strstr(argv[1], "up")?1:0);
	return CMD_SUCCESS;
}

DEFUN (iplink_vrf_test_add,
		iplink_vrf_test_add_cmd,
		"ipvrf test  add id <0-100000> eth NAME",
		"ipvrf\n"
		"test information\n"
		"add\n"
		"id\n"
		"id value\n"
		"eth\n"
		"name")
{
	iplink_vrftable_add(IPSTACK_RTM_NEWLINK, atoi(argv[0]), argv[1]);
	return CMD_SUCCESS;
}

int iplink_test(void)
{
	install_element(ENABLE_NODE,  CMD_VIEW_LEVEL,  &iplink_vrf_test_cmd);
	install_element(ENABLE_NODE,  CMD_VIEW_LEVEL,  &iplink_vrf_test_up_cmd);
	install_element(ENABLE_NODE,  CMD_VIEW_LEVEL,  &iplink_vrf_test_add_cmd);
	return 0;
}
#endif
