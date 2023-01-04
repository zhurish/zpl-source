/*
 * VRF functions.
 * Copyright (C) 2014 6WIND S.A.
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
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

#ifdef ZPL_NETNS_ENABLE
#undef _GNU_SOURCE
#define _GNU_SOURCE

#include <sched.h>
#endif

#include "pal_include.h"
#include "nsm_debug.h"
#include "nsm_vlan.h"
#include "nsm_arp.h"
#include "nsm_include.h"
#include "nsm_firewalld.h"
#include "nsm_vlaneth.h"
#include "linux_driver.h"
#ifndef IFLA_VRF_TABLE
#define IFLA_VRF_TABLE 1
#endif
#ifdef ZPL_NETNS_ENABLE

#ifndef CLONE_NEWNET
#define CLONE_NEWNET 0x40000000 /* New network namespace (lo, device, names sockets, etc) */
#endif

#ifndef HAVE_SETNS
static inline int setns(int fd, int nstype)
{
#ifdef __NR_setns
	return syscall(__NR_setns, fd, nstype);
#else
	ipstack_errno = ENOSYS;
	return -1;
#endif
}
#endif /* HAVE_SETNS */

#define VRF_RUN_DIR "/var/run/netns"



#define VRF_DEFAULT_NAME "/proc/self/ns/net"
static int have_netns_enabled = -1;


static int have_netns(void)
{
	if (have_netns_enabled < 0)
	{
		zpl_fd_t fd = ipstack_open(IPSTACK_IPCOM, VRF_DEFAULT_NAME, O_RDONLY);

		if (ipstack_invalid(fd))
			have_netns_enabled = 0;
		else
		{
			have_netns_enabled = 1;
			ipstack_close(fd);
		}
	}
	return have_netns_enabled;
}

/*
 * VRF realization with NETNS
 */

static char *_netns_pathname(const char *name)
{
	static char pathname[PATH_MAX];
	char *result;

	if (name[0] == '/') /* absolute pathname */
		result = realpath(name, pathname);
	else /* relevant pathname */
	{
		char tmp_name[PATH_MAX];
		snprintf(tmp_name, PATH_MAX, "%s/%s", VRF_RUN_DIR, name);
		result = realpath(tmp_name, pathname);
	}

	if (!result)
	{
		zlog_err(MODULE_PAL, "Invalid pathname: %s", ipstack_strerror(ipstack_errno));
		return NULL;
	}
	return pathname;
}

/*
 * Check whether the VRF is enabled - that is, whether the VRF
 * is ready to allocate resources. Currently there's only one
 * type of resource: ipstack_socket.
 */
static int netns_is_enabled(struct ip_vrf *vrf)
{
	if (have_netns())
		return vrf && ipstack_fd(vrf->fd) >= 0;
	else
		return vrf && ipstack_fd(vrf->fd) == -2 && vrf->vrf_id == VRF_DEFAULT;
}

static int netns_enable(struct ip_vrf *vrf)
{
	if (!vrf)
		return -1;

	char *pathname = _netns_pathname(vrf->name);
	if (!netns_is_enabled(vrf))
	{
		if (have_netns())
		{
			vrf->fd = ipstack_open(IPSTACK_IPCOM, pathname, O_RDONLY);
		}
		else
		{
			// vrf->fd = -2; /* Remember that vrf_enable_hook has been called */
			ipstack_errno = -ENOTSUP;
			return -1;
		}

		if (!netns_is_enabled(vrf))
		{
			zlog_err(MODULE_PAL, "Can not enable VRF %u: %s!", vrf->vrf_id,
					 ipstack_strerror(ipstack_errno));
			return -1;
		}

		if (have_netns())
			zlog_info(MODULE_PAL, "VRF %u is associated with NETNS %s.",
					  vrf->vrf_id, vrf->name);
		// TODO
		zlog_info(MODULE_PAL, "VRF %u is enabled.", vrf->vrf_id);
	}
	return 0;
}

/*
 * Disable a VRF - that is, let the VRF be unusable.
 * The VRF_DELETE_HOOK callback will be called to inform
 * that they must release the resources in the VRF.
 */
static int netns_disable(struct ip_vrf *vrf)
{
	if (!vrf)
		return -1;
	if (netns_is_enabled(vrf))
	{
		zlog_info(MODULE_PAL, "VRF %u is to be disabled.", vrf->vrf_id);
		// TODO
		if (have_netns())
			ipstack_close(vrf->fd);

		ipstack_fd(vrf->fd) = -1;
		return 0;
	}
	return -1;
}

/* Create a socket for the VRF. */
static zpl_socket_t _kernel_vrf_socket(int domain, zpl_uint32 type, zpl_uint16 protocol, vrf_id_t vrf_id)
{
	struct ip_vrf *vrf = ip_vrf_lookup(vrf_id);
	int ret = -1;
	zpl_socket_t tmp;
	tmp = ipstack_init(IPSTACK_OS, -1);
	if (!vrf)
		return tmp;
	if (!netns_is_enabled(vrf))
	{
		errno = ENOSYS;
		return tmp;
	}

	if (have_netns())
	{
		ret = (vrf_id != VRF_DEFAULT) ? setns(ipstack_fd(vrf->fd), CLONE_NEWNET) : 0;
		if (ret >= 0)
		{
			ipstack_fd(tmp) = socket(domain, type, protocol);
			if (vrf_id != VRF_DEFAULT)
				setns(ipstack_fd(ip_vrf_lookup(VRF_DEFAULT)->fd), CLONE_NEWNET);
		}
	}
	else
		ipstack_fd(tmp) = socket(domain, type, protocol);
	return tmp;
}

#endif /* ZPL_NETNS_ENABLE */

static int librtnl_ipvrf_create(char *name, int table)
{
	struct ipstack_rtattr *linkinfo;
	struct ipstack_rtattr *data;
	struct
	{
		struct ipstack_nlmsghdr n;
		struct ipstack_ifinfomsg iif;
		char buf[NL_PKT_BUF_SIZE];
	} req;


	memset(&req, 0, sizeof req - NL_PKT_BUF_SIZE);

	req.n.nlmsg_len = IPSTACK_NLMSG_LENGTH(sizeof(struct ipstack_ifinfomsg));
	req.n.nlmsg_flags = IPSTACK_NLM_F_REQUEST|IPSTACK_NLM_F_CREATE|NLM_F_EXCL;
	req.n.nlmsg_type = IPSTACK_RTM_NEWLINK;
	req.iif.ifi_family = IPSTACK_AF_UNSPEC;

	librtnl_addattr_l(&req.n, sizeof(req), IFLA_IFNAME, name, strlen(name) + 1);

	linkinfo = librtnl_addattr_nest(&req.n, sizeof(req), IFLA_LINKINFO);
	librtnl_addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, "vrf",  strlen("vrf"));

	data = librtnl_addattr_nest(&req.n, sizeof(req), IFLA_INFO_DATA);
	librtnl_addattr32(&req.n, sizeof(req), IFLA_VRF_TABLE, table);
	librtnl_addattr_nest_end(&req.n, data);

	librtnl_addattr_nest_end(&req.n, linkinfo);
	return librtnl_talk(&netlink_cmd, &req.n);
}


static int librtnl_ipvrf_add(char *name, char * dev)
{
	int ifindex = 0;
	struct
	{
		struct ipstack_nlmsghdr n;
		struct ipstack_ifinfomsg iif;
		char buf[NL_PKT_BUF_SIZE];
	} req;

	req.n.nlmsg_len = IPSTACK_NLMSG_LENGTH(sizeof(struct ipstack_ifinfomsg));
	req.n.nlmsg_flags = IPSTACK_NLM_F_REQUEST;
	req.n.nlmsg_type = IPSTACK_RTM_NEWLINK;
	req.iif.ifi_family = IPSTACK_AF_UNSPEC;
	if(name)
		ifindex = librtnl_link_name2index(name, 0);

	librtnl_addattr_l(&req.n, sizeof(req), IFLA_MASTER, &ifindex, 4);
	req.iif.ifi_index = if_nametoindex(dev);
	return librtnl_talk(&netlink_cmd, &req.n);
}

int linux_ioctl_vrf_set(struct interface *ifp, struct ip_vrf *vrf)
{
	if(vrf)
		return librtnl_ipvrf_add(vrf->name, ifp->ker_name);
	else
		return librtnl_ipvrf_add(NULL, ifp->ker_name);
}
/*
 * Enable a VRF - that is, let the VRF be ready to use.
 * The VRF_ENABLE_HOOK callback will be called to inform
 * that they can allocate resources in this VRF.
 *
 * RETURN: 1 - enabled successfully; otherwise, 0.
 */
#ifdef ZPL_LIBNL_MODULE
int rtnl_ipvrf_create(int id, char *name)
{
	struct rtnl_link *link;
	int err;
	if (!(link = rtnl_link_vrf_alloc())) {
		fprintf(stderr, "Unable to allocate link");
		return -1;
	}
	rtnl_link_set_name(link, name);
	if ((err = rtnl_link_vrf_set_tableid(link, id)) < 0) {
		nl_perror(err, "Unable to set VRF table id");
		return err;
	}
	return libnl_netlink_link_add(&netlink_cmd, link, IPSTACK_NLM_F_CREATE);
}
//ipvrf_test_add enp0s25 abcd
int rtnl_ipvrf_add_dev(char *dev, char *name)
{
	struct rtnl_link *link;
	struct nl_cache *link_cache;
	int err = 0, master_index = 0;
	if (!(link = rtnl_link_alloc())) {
		fprintf(stderr, "Unable to allocate link");
		return -1;
	}
	
	if ((err = rtnl_link_alloc_cache(netlink_cmd.libnl_sock, AF_UNSPEC, &link_cache)) < 0) {
		nl_perror(err, "Unable to allocate cache");
		return err;
	}

	if (!(master_index = rtnl_link_name2i(link_cache, name))) {
		fprintf(stderr, "Unable to lookup eth0");
		return -1;
	}
	rtnl_link_set_name(link, dev);
	rtnl_link_set_master(link, master_index);

	return libnl_netlink_link_add(&netlink_cmd, link, RTM_NEWLINK);
}
#endif

int linux_ioctl_vrf_enable(struct ip_vrf *vrf)
{
#ifdef ZPL_NETNS_ENABLE	
	netns_enable(vrf);
#endif /* ZPL_NETNS_ENABLE */
#ifdef ZPL_LIBNL_MODULE
	if(rtnl_ipvrf_create(, ) == 0)
	{
		libnl_netlink_link_updown(&netlink_cmd, vrf->name, 1);
	}
#endif	
	if(librtnl_ipvrf_create(vrf->name, vrf->vrf_id) == OK)
	{
		vrf->kvrfid = librtnl_link_name2index(vrf->name, 0);
		return librtnl_link_updown(vrf->name, 1);
	}
	return ERROR;	
}

/*
 * Disable a VRF - that is, let the VRF be unusable.
 * The VRF_DELETE_HOOK callback will be called to inform
 * that they must release the resources in the VRF.
 */
int linux_ioctl_vrf_disable(struct ip_vrf *vrf)
{
#ifdef ZPL_NETNS_ENABLE	
	netns_disable(vrf);
#endif /* ZPL_NETNS_ENABLE */	
#ifdef ZPL_LIBNL_MODULE
	return 0;
	if(libnl_netlink_link_del(&netlink_cmd, vrf->name) == 0)
	{
	}
#endif
	librtnl_link_updown(vrf->name, 0);
	return librtnl_link_destroy(vrf->name);	
}

/* Create a socket for the VRF. */
zpl_socket_t linux_vrf_socket(int domain, zpl_uint32 type, zpl_uint16 protocol, vrf_id_t vrf_id)
{
#ifdef ZPL_NETNS_ENABLE	
	return _kernel_vrf_socket(domain, type, protocol, vrf_id);
#else
	zpl_socket_t tmpsock = ipstack_init(IPSTACK_OS, -1);
	ipstack_fd(tmpsock) = socket(domain, type, protocol);
	return tmpsock;
#endif /* ZPL_NETNS_ENABLE */	
}


