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
#include "nsm_event.h"
#include "module.h"
#include "zmemory.h"
#include "thread.h"
#include "if.h"
#include "vty.h"
#include "command.h"
#include "prefix.h"

#include "nsm_debug.h"
#include "nsm_rib.h"
#include "nsm_include.h"
#include "linux_driver.h"



#ifdef ZPL_KERNEL_NETLINK
/* Interface address modification. IPSTACK_AF_UNSPEC */
static int librtnl_ioctl_interface(zpl_uint32 cmd, zpl_family_t family, struct interface *ifp)
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

	//struct nsm_ipvrf *zvrf = ip_vrf_info_lookup(ifp->vrf_id);

	memset(&req, 0, sizeof req - NL_PKT_BUF_SIZE);

	//bytelen = (family == IPSTACK_AF_INET ? 4 : 16);

	req.n.nlmsg_len = IPSTACK_NLMSG_LENGTH(sizeof(struct ipstack_ifinfomsg));
	req.n.nlmsg_flags = IPSTACK_NLM_F_REQUEST | flags;
	req.n.nlmsg_type = cmd;

	req.i.ifi_family = family;
	req.i.__ifi_pad = 0;
	req.i.ifi_type = 0;		/* ARPHRD_* */
	if(!(flags & IPSTACK_NLM_F_CREATE))
		req.i.ifi_index = if_nametoindex(ifp->ker_name);
	req.i.ifi_flags = 0;
	req.i.ifi_change = 0;		/* IFF_* change mask */

	if (os_strlen(ifp->ker_name))
	{
		librtnl_addattr_l(&req.n, sizeof(req),
			  IPSTACK_IFLA_IFNAME, ifp->ker_name, strlen(ifp->ker_name)+1);
	}


	linkinfo = librtnl_addattr_nest(&req.n, sizeof(req), IPSTACK_IFLA_LINKINFO);
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
	case IF_XGIGABT_ETHERNET:
		break;
	case IF_TUNNEL:
		librtnl_addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, "tunnel", strlen("tunnel"));
		create = zpl_true;
		break;
	case IF_LAG:
		librtnl_addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, "bond", strlen("bond"));
		create = zpl_true;
		break;
	case IF_BRIGDE:		//brigde interface
		librtnl_addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, "bridge", strlen("bridge"));
		create = zpl_true;
		break;
	case IF_VLAN:
		librtnl_addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, "vlan", strlen("vlan"));
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
	//data = addattr_nest(&req.n, sizeof(req), iflatype);

	librtnl_addattr_nest_end(&req.n, linkinfo);
	if(create == zpl_true)
		return librtnl_talk(&netlink_cmd, &req.n);
	return ERROR;
}


int librtnl_create_interface(struct interface *ifp)
{
	return librtnl_ioctl_interface(IPSTACK_RTM_NEWLINK, IPSTACK_AF_UNSPEC, ifp);
}

int librtnl_destroy_interface(struct interface *ifp)
{
	return librtnl_ioctl_interface(IPSTACK_RTM_DELLINK, IPSTACK_AF_UNSPEC, ifp);
}


#if 0
//ip link add test type vrf table 200
//ip link set dev red up
//ip link set ens38 master green
//ip link set dev ens37 nomaster
//ip -d link show type vrf
//ip route show vrf abcd
//ip address show vrf abcd
//iplink_vrftable IPSTACK_RTM_NEWLINK test 200
extern int rtnl_iplink_vrf_create(char *name, int table);
extern int rtnl_iplink_vrf_add(char *name, char * dev);
extern int ip_main(int argc, char **argv);
extern int rtnl_iplink_updown(char *name, int up);
//extern int librtnl_iplink_updown(char *name, int up);

DEFUN (ipvrf_test_create,
		ipvrf_test_create_cmd,
		"ipvrf_test_create <1-256> NAME",
		"table id\n"
		"vrf name\n")
{
	librtnl_vrf_create(argv[1], atoi(argv[0]));
	return CMD_SUCCESS;
}
DEFUN (ipvrf_test_updown,
		ipvrf_test_updown_cmd,
		"ipvrf_test_updown  NAME <0-1>",
		"vrf name\n"
		"up/down\n")
{
	librtnl_link_destroy(argv[0]);
	return CMD_SUCCESS;
}
//ipvrf_test_add enp0s25 abcd
DEFUN (ipvrf_test_add,
		ipvrf_test_add_cmd,
		"ipvrf_test_add NAME DEV",
		"table id\n"
		"vrf name\n")
{
	//int margc = 4;
	//char *margv[] = { "set", argv[1], "master", argv[0], NULL};
	//ip_main(margc, margv);
	librtnl_iplink_vrf_add((argv[0]), argv[1]);
	return CMD_SUCCESS;
}
#endif

int iplink_test(void)
{
	//install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &ipvrf_test_create_cmd);
	//install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &ipvrf_test_add_cmd);
	//install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &ipvrf_test_updown_cmd);
	
	return 0;
}
#endif

#ifdef ZPL_LIBNL_MODULE
static int rtnl_bond_interface_create(char *name)
{
	struct rtnl_link *link;
	if (!(link = rtnl_link_bond_alloc())) {
		fprintf(stderr, "Unable to allocate link");
		return -1;
	}

	rtnl_link_set_name(link, name);

	return libnl_netlink_link_add(&netlink_cmd, link, IPSTACK_NLM_F_CREATE);
}
static int rtnl_bridge_interface_create(char *name)
{
	struct rtnl_link *link;
	int err;
	if (!(link = rtnl_link_alloc())) {
		fprintf(stderr, "Unable to allocate link");
		return -1;
	}
	
	if ((err = rtnl_link_set_type(link, "bridge")) < 0) {
		rtnl_link_put(link);
		return err;
	}
	rtnl_link_set_name(link, name);

	return libnl_netlink_link_add(&netlink_cmd, link, IPSTACK_NLM_F_CREATE);
}

static int rtnl_vlan_interface_create(char *praent, char *name, int vid)
{
	struct rtnl_link *link;
	struct nl_cache *link_cache;
	int err, master_index;

	if ((err = rtnl_link_alloc_cache(netlink_cmd.libnl_sock, AF_UNSPEC, &link_cache)) < 0) {
		nl_perror(err, "Unable to allocate cache");
		return err;
	}

	if (!(master_index = rtnl_link_name2i(link_cache, praent))) {
		fprintf(stderr, "Unable to lookup eth0");
		return -1;
	}

	link = rtnl_link_vlan_alloc();

	rtnl_link_set_link(link, master_index);

	rtnl_link_vlan_set_id(link, vid);

	return libnl_netlink_link_add(&netlink_cmd, link, IPSTACK_NLM_F_CREATE);
}

static int rtnl_tunnel_interface_create(char *praent, char *name, int type, int ttl, char *localip, char *remoteip)
{
	struct nl_cache *link_cache;
	struct rtnl_link *link;
	struct in_addr addr;
	struct in6_addr addr6;
	int err, if_mindex;

	err = rtnl_link_alloc_cache(netlink_cmd.libnl_sock, AF_UNSPEC, &link_cache);
	if ( err < 0) {
		nl_perror(err, "Unable to allocate cache");
		return err;
	}

	if_mindex = rtnl_link_name2i(link_cache, praent);
	if (!if_mindex) {
		fprintf(stderr, "Unable to lookup eno16777736");
		return -1;
	}
	if(type == IPSTACK_IPPROTO_IPIP)
	{
#if 0		
		link = rtnl_link_ipvti_alloc();
		if(!link) {
			nl_perror(err, "Unable to allocate link");
			return -1;

		}
		rtnl_link_set_name(link, name/*"ipvti-tun"*/);
		rtnl_link_ipvti_set_link(link, if_mindex);

		inet_pton(AF_INET, "192.168.254.12", &addr.s_addr);
		rtnl_link_ipvti_set_local(link, addr.s_addr);

		inet_pton(AF_INET, "192.168.254.13", &addr.s_addr);
		rtnl_link_ipvti_set_remote(link, addr.s_addr);
#endif
		link = rtnl_link_ipip_alloc();
		if(!link) {
			nl_perror(err, "Unable to allocate link");
			return -1;
		}

		rtnl_link_set_name(link, name/*"ipip-tun"*/);
		rtnl_link_ipip_set_link(link, if_mindex);

		inet_pton(AF_INET, localip, &addr.s_addr);
		rtnl_link_ipip_set_local(link, addr.s_addr);

		inet_pton(AF_INET, remoteip, &addr.s_addr);
		rtnl_link_ipip_set_remote(link, addr.s_addr);

		rtnl_link_ipip_set_ttl(link, ttl);
	}
	else if(type == IPSTACK_IPPROTO_GRE)
	{
		if(strstr(name, "tap"))
		{
			link = rtnl_link_ipgretap_alloc();
			if(!link) {
				nl_perror(err, "Unable to allocate link");
				return -1;
			}
		}
		else
		{
			link = rtnl_link_ipgre_alloc();
			if(!link) {
				nl_perror(err, "Unable to allocate link");
				return -1;
			}
		}
		rtnl_link_set_name(link, name);
		rtnl_link_ipgre_set_link(link, if_mindex);

		inet_pton(AF_INET, localip, &addr.s_addr);
		rtnl_link_ipgre_set_local(link, addr.s_addr);

		inet_pton(AF_INET, remoteip, &addr.s_addr);
		rtnl_link_ipgre_set_remote(link, addr.s_addr);

		rtnl_link_ipgre_set_ttl(link, ttl);
	}
	else if(type == IPSTACK_IPPROTO_IPV6)
	{
#if 0		
		//return _if_tunnel_create("sit0", &p);
		link = rtnl_link_sit_alloc();
		if(!link) {
			nl_perror(err, "Unable to allocate link");
			return -1;

		}
		rtnl_link_set_name(link, name/*"sit-tun"*/);
		rtnl_link_sit_set_link(link, if_mindex);

		inet_pton(AF_INET, localip, &addr.s_addr);
		rtnl_link_sit_set_local(link, addr.s_addr);

		inet_pton(AF_INET, remoteip, &addr.s_addr);
		rtnl_link_sit_set_remote(link, addr.s_addr);

		rtnl_link_sit_set_ttl(link, ttl);
#endif
		link = rtnl_link_ip6_tnl_alloc();
		if(!link) {
			nl_perror(err, "Unable to allocate link");
			return -1;

		}
		rtnl_link_set_name(link, name/*"ip6tnl-tun"*/);
		rtnl_link_ip6_tnl_set_link(link, if_mindex);

		inet_pton(AF_INET6, localip, &addr6);
		rtnl_link_ip6_tnl_set_local(link, &addr6);

		inet_pton(AF_INET6, remoteip, &addr6);
		rtnl_link_ip6_tnl_set_remote(link, &addr6);
	}
	return libnl_netlink_link_add(&netlink_cmd, link, IPSTACK_NLM_F_CREATE);
}


static int rtnl_ipvlan_interface_create(char *praent, char *name, int vid)
{
	struct rtnl_link *link;
	struct nl_cache *link_cache;
	int err, master_index;

	if ((err = rtnl_link_alloc_cache(netlink_cmd.libnl_sock, AF_UNSPEC, &link_cache)) < 0) {
		nl_perror(err, "Unable to allocate cache");
		return err;
	}

	if (!(master_index = rtnl_link_name2i(link_cache, praent))) {
		fprintf(stderr, "Unable to lookup eth0");
		return -1;
	}

	if (!(link = rtnl_link_ipvlan_alloc())) {
		fprintf(stderr, "Unable to allocate link");
		return -1;
	}

	rtnl_link_set_link(link, master_index);
	rtnl_link_ipvlan_set_mode(link, rtnl_link_ipvlan_str2mode("l2"));

	return libnl_netlink_link_add(&netlink_cmd, link, IPSTACK_NLM_F_CREATE);
}

static int rtnl_vxlan_interface_create(char *name, int vid, char *group)
{
	struct rtnl_link *link;
	struct nl_addr *addr;
	int err;

	link = rtnl_link_vxlan_alloc();

	rtnl_link_set_name(link, name);

	if ((err = rtnl_link_vxlan_set_id(link, vid)) < 0) {
		nl_perror(err, "Unable to set VXLAN network identifier");
		return err;
	}

	if ((err = nl_addr_parse(/*"239.0.0.1"*/group, AF_INET, &addr)) < 0) {
		nl_perror(err, "Unable to parse IP address");
		return err;
	}

	if ((err = rtnl_link_vxlan_set_group(link, addr)) < 0) {
		nl_perror(err, "Unable to set multicast IP address");
		return err;
	}
	nl_addr_put(addr);

	return libnl_netlink_link_add(&netlink_cmd, link, IPSTACK_NLM_F_CREATE);
}

int linux_netlink_create_interface(struct interface *ifp)
{
	int ret = 0;
	switch(ifp->if_type)
	{
	case IF_LOOPBACK:
		break;
	case IF_SERIAL:
		break;
	case IF_ETHERNET:
		break;
	case IF_GIGABT_ETHERNET:
	case IF_XGIGABT_ETHERNET:
		break;
	case IF_TUNNEL:
		ret = rtnl_tunnel_interface_create(NULL, ifp->ker_name, IPSTACK_IPPROTO_GRE, 64, "1.1.1.1", "1.1.1.2");
		break;
	case IF_LAG:
		ret = rtnl_bond_interface_create(ifp->ker_name);
		break;
	case IF_BRIGDE:		//brigde interface
		ret = rtnl_bridge_interface_create(ifp->ker_name);
		break;
	case IF_SUBVLAN:
		ret = rtnl_vlan_interface_create(ifp->ker_name, NULL, 0);
		break;
	case IF_VLAN:
		break;
	case IF_VXLAN:
		ret = rtnl_vxlan_interface_create(ifp->ker_name, 0, NULL);
		break;
		
#ifdef CUSTOM_INTERFACE
	case IF_WIFI:		//wifi interface
		break;
	case IF_MODEM:		//modem interface
		break;
#endif
	default:
		rtnl_ipvlan_interface_create(ifp->ker_name, NULL, 0);
		break;
	}	
	return ret;
}

int linux_netlink_destroy_interface(struct interface *ifp)
{
	if(libnl_netlink_link_del(&netlink_cmd, ifp->ker_name) == 0)
	{
		return OK;
	}
	return ERROR;
}
int iplink_test(void)
{
	return 0;
}
#endif