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
#include "module.h"
#include "zmemory.h"
#include "thread.h"
#include "if.h"
#include "vty.h"
#include "command.h"
#include "prefix.h"
#include "vrf.h"
#include "nexthop.h"
#include "pal_include.h"
#include "nsm_debug.h"
#include "nsm_vlan.h"
#include "nsm_arp.h"
#include "nsm_bridge.h"
#include "nsm_firewalld.h"
#include "nsm_vlaneth.h"
#include "nsm_include.h"
#include "linux_driver.h"

/* Hack for GNU libc version 2. */
#ifndef IPSTACK_MSG_TRUNC
#define IPSTACK_MSG_TRUNC      0x20
#endif /* IPSTACK_MSG_TRUNC */


#include "linux_netlink.h"

struct nlsock netlink_cmd;

static const struct message nlmsg_str[] =
{
		{ IPSTACK_RTM_NEWROUTE, "IPSTACK_RTM_NEWROUTE" },
		{ IPSTACK_RTM_DELROUTE, "IPSTACK_RTM_DELROUTE" },
		{ IPSTACK_RTM_GETROUTE, "IPSTACK_RTM_GETROUTE" },
		{ IPSTACK_RTM_NEWLINK, "IPSTACK_RTM_NEWLINK" },
		{ IPSTACK_RTM_DELLINK, "IPSTACK_RTM_DELLINK" },
		{ IPSTACK_RTM_GETLINK, "IPSTACK_RTM_GETLINK" },
		{ IPSTACK_RTM_NEWADDR, "IPSTACK_RTM_NEWADDR" },
		{ IPSTACK_RTM_DELADDR, "IPSTACK_RTM_DELADDR" },
		{ IPSTACK_RTM_GETADDR, "IPSTACK_RTM_GETADDR" },
		{ 0, NULL }
};

//extern struct zebra_t zebrad;

static struct
{
	char *p;
	size_t size;
} nl_rcvbuf;

static const struct message rtproto_str[] =
{
		{ IPSTACK_RTPROT_REDIRECT, "redirect" },
		{ IPSTACK_RTPROT_KERNEL, "kernel" },
		{ IPSTACK_RTPROT_BOOT, "boot" },
		{ IPSTACK_RTPROT_STATIC, "static" },
		{ IPSTACK_RTPROT_GATED, "GateD" },
		{ IPSTACK_RTPROT_RA, "router advertisement" },
		{ IPSTACK_RTPROT_MRT, "MRT" },
		{ IPSTACK_RTPROT_ZEBRA, "Zebra" },
#ifdef IPSTACK_RTPROT_BIRD
		{	IPSTACK_RTPROT_BIRD, "BIRD"},
#endif /* IPSTACK_RTPROT_BIRD */
		{ 0, NULL }
};

/*
 * _netlink_msg_type_to_str
 */
const char *
_netlink_msg_type_to_str(zpl_uint16 msg_type)
{
	return lookup(nlmsg_str, msg_type);
}

/*
 * _netlink_rtproto_to_str
 */
const char *
_netlink_rtproto_to_str(zpl_uchar rtproto)
{
	return lookup(rtproto_str, rtproto);
}

/* Utility function  comes from iproute2.
 Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru> */
int _netlink_addattr_l(struct ipstack_nlmsghdr *n, size_t maxlen, zpl_uint32 type, void *data,
		size_t alen)
{
	size_t len;
	struct ipstack_rtattr *rta;

	len = IPSTACK_RTA_LENGTH(alen);

	if (IPSTACK_NLMSG_ALIGN(n->nlmsg_len) + len > maxlen)
		return -1;

	rta = (struct ipstack_rtattr *) (((char *) n) + IPSTACK_NLMSG_ALIGN(n->nlmsg_len));
	rta->rta_type = type;
	rta->rta_len = len;
	if (data)
		memcpy(IPSTACK_RTA_DATA(rta), data, alen);
	n->nlmsg_len = IPSTACK_NLMSG_ALIGN(n->nlmsg_len) + len;

	return 0;
}

int _netlink_rta_addattr_l(struct ipstack_rtattr *rta, size_t maxlen, zpl_uint32 type, void *data,
		size_t alen)
{
	size_t len;
	struct ipstack_rtattr *subrta;

	len = IPSTACK_RTA_LENGTH(alen);

	if (IPSTACK_RTA_ALIGN(rta->rta_len) + len > maxlen)
		return -1;

	subrta = (struct ipstack_rtattr *) (((char *) rta) + IPSTACK_RTA_ALIGN(rta->rta_len));
	subrta->rta_type = type;
	subrta->rta_len = len;
	if (data)
		memcpy(IPSTACK_RTA_DATA(subrta), data, alen);
	rta->rta_len = IPSTACK_NLMSG_ALIGN(rta->rta_len) + len;

	return 0;
}

/* Utility function comes from iproute2.
 Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru> */
int _netlink_addattr32(struct ipstack_nlmsghdr *n, size_t maxlen, zpl_uint32 type, zpl_uint32 data)
{
	size_t len;
	struct ipstack_rtattr *rta;

	len = IPSTACK_RTA_LENGTH(4);

	if (IPSTACK_NLMSG_ALIGN(n->nlmsg_len) + len > maxlen)
		return -1;

	rta = (struct ipstack_rtattr *) (((char *) n) + IPSTACK_NLMSG_ALIGN(n->nlmsg_len));
	rta->rta_type = type;
	rta->rta_len = len;
	memcpy(IPSTACK_RTA_DATA(rta), &data, 4);
	n->nlmsg_len = IPSTACK_NLMSG_ALIGN(n->nlmsg_len) + len;

	return 0;
}

struct ipstack_rtattr *_netlink_addattr_nest(struct ipstack_nlmsghdr *n, size_t maxlen, zpl_uint32 type)
{
	struct ipstack_rtattr *nest = NLMSG_TAIL(n);

	_netlink_addattr_l(n, maxlen, type, NULL, 0);
	return nest;
}

int _netlink_addattr_nest_end(struct ipstack_nlmsghdr *n, struct ipstack_rtattr *nest)
{
	nest->rta_len = (void *) NLMSG_TAIL(n) - (void *) nest;
	return n->nlmsg_len;
}

/* Utility function for parse ipstack_rtattr. */
void _netlink_parse_rtattr(struct ipstack_rtattr **tb, zpl_uint32 max, struct ipstack_rtattr *rta,
		zpl_uint32 len)
{
	while (IPSTACK_RTA_OK(rta, len))
	{
		if (rta->rta_type <= max)
			tb[rta->rta_type] = rta;
		rta = IPSTACK_RTA_NEXT(rta, len);
	}
}

/* Utility function to parse hardware link-layer address and update ifp */
void _netlink_interface_update_hw_addr(struct ipstack_rtattr **tb, struct interface *ifp)
{
	zpl_uint32 i;

	if (tb[IPSTACK_IFLA_ADDRESS])
	{
		int hw_addr_len;

		hw_addr_len = IPSTACK_RTA_PAYLOAD(tb[IPSTACK_IFLA_ADDRESS]);

		if (hw_addr_len > INTERFACE_HWADDR_MAX)
			zlog_warn(MODULE_PAL, "Hardware address is too large: %d",
					hw_addr_len);
		else
		{
			ifp->hw_addr_len = hw_addr_len;
			memcpy(ifp->hw_addr, IPSTACK_RTA_DATA(tb[IPSTACK_IFLA_ADDRESS]), hw_addr_len);

			for (i = 0; i < hw_addr_len; i++)
				if (ifp->hw_addr[i] != 0)
					break;

			if (i == hw_addr_len)
				ifp->hw_addr_len = 0;
			else
				ifp->hw_addr_len = hw_addr_len;
		}
	}
}

/* Log debug information for netlink_route_multipath
 * if debug logging is enabled.
 *
 * @param cmd: Netlink command which is to be processed
 * @param p: Prefix for which the change is due
 * @param nexthop: Nexthop which is currently processed
 * @param routedesc: Semantic annotation for nexthop
 *                     (recursive, multipath, etc.)
 * @param family: Address family which the change concerns
 */
void _netlink_route_debug(zpl_uint32 cmd, zpl_uint8 family, zpl_uint32 bytelen,
		hal_nexthop_t *nexthop, union g_addr address,
		vrf_id_t vrfid)
{
	if (IS_ZEBRA_DEBUG_KERNEL)
	{
		zlog_debug(MODULE_PAL, "(%s): %s/%d vrf %u",
				lookup(nlmsg_str, cmd),
				inet_ntoa(address.ipv4), bytelen, vrfid);
	}
}

/* Note: on netlink systems, there should be a 1-to-1 mapping between interface
 names and ifindex values. */
void _netlink_set_ifindex(struct interface *ifp, ifindex_t ifi_index)
{
	struct interface *oifp;

	if (((oifp = if_lookup_by_kernel_index(ifi_index)) != NULL)
			&& (oifp != ifp))
	{
		if (ifi_index == IFINDEX_INTERNAL)
			zlog_err(MODULE_PAL,
					"Netlink is setting interface %s k_ifindex to reserved "
							"internal value %u", ifp->k_name, ifi_index);
		else
		{
			if (IS_ZEBRA_DEBUG_KERNEL)
				zlog_debug(MODULE_PAL,
						"interface index %d was renamed from %s to %s",
						ifi_index, oifp->name, ifp->name);
			if (if_is_up(oifp))
				zlog_err(MODULE_PAL,
						"interface rename detected on up interface: index %d "
								"was renamed from %s to %s, results are uncertain!",
						ifi_index, oifp->name, ifp->name);
			// zhurish if_delete_update(oifp);
		}
	}
	ifp->k_ifindex = ifi_index;
}

/* Get type specified information from netlink. */
int _netlink_request(zpl_family_t family, zpl_uint32 type, struct nlsock *nl)
{
	int ret;
	struct ipstack_sockaddr_nl snl;
	int save_errno;

	struct
	{
		struct ipstack_nlmsghdr nlh;
		struct ipstack_rtgenmsg g;
	} req;

	/* Check netlink ipstack_socket. */
	if (ipstack_invalid(nl->sock))
	{
		zlog_err(MODULE_PAL, "%s ipstack_socket isn't active.", nl->name);
		return -1;
	}

	memset(&snl, 0, sizeof snl);
	snl.nl_family = IPSTACK_AF_NETLINK;

	memset(&req, 0, sizeof req);
	req.nlh.nlmsg_len = sizeof req;
	req.nlh.nlmsg_type = type;
	req.nlh.nlmsg_flags = IPSTACK_NLM_F_ROOT | IPSTACK_NLM_F_MATCH | IPSTACK_NLM_F_REQUEST;
	req.nlh.nlmsg_pid = nl->snl.nl_pid;
	req.nlh.nlmsg_seq = ++nl->seq;
	req.g.rtgen_family = family;

	ret = ipstack_sendto(nl->sock, (void *) &req, sizeof req, 0,
			(struct ipstack_sockaddr *) &snl, sizeof snl);
	save_errno = ipstack_errno;

	if (ret < 0)
	{
		zlog_err(MODULE_PAL, "%s ipstack_sendto failed: %s", nl->name,
				ipstack_strerror(save_errno));
		return -1;
	}
	return 0;
}

/* Receive message from netlink interface and pass those information
 to the given function. */
int _netlink_parse_info(
		int (*filter)(struct ipstack_sockaddr_nl *, struct ipstack_nlmsghdr *, vrf_id_t),
		struct nlsock *nl, vrf_id_t vrfid)
{
	zpl_uint32 status;
	int ret = 0;
	int error;
	struct ipstack_sockaddr_nl snl;
	struct ipstack_nlmsghdr *h;
	while (1)
	{
		struct ipstack_iovec iov =
		{
			.iov_base = nl_rcvbuf.p,
			.iov_len = nl_rcvbuf.size,
		};

		struct ipstack_msghdr msg =
		{
			.msg_name = (void *) &snl,
			.msg_namelen = sizeof snl,
			.msg_iov = &iov,
			.msg_iovlen = 1
		};


		status = ipstack_recvmsg(nl->sock, &msg, 0);
		if (status < 0)
		{
			if (ipstack_errno == IPSTACK_ERRNO_EINTR)
				continue;
			if (ipstack_errno == IPSTACK_ERRNO_EWOULDBLOCK || ipstack_errno == IPSTACK_ERRNO_EAGAIN)
				break;
			zlog_err(MODULE_PAL, "%s ipstack_recvmsg overrun: %s", nl->name,
					ipstack_strerror(ipstack_errno));
			continue;
		}

		if (status == 0)
		{
			zlog_err(MODULE_PAL, "%s EOF", nl->name);
			return -1;
		}

		if (msg.msg_namelen != sizeof snl)
		{
			zlog_err(MODULE_PAL, "%s sender address length error: length %d",
					nl->name, msg.msg_namelen);
			return -1;
		}

		for (h = (struct ipstack_nlmsghdr *) nl_rcvbuf.p;
				IPSTACK_NLMSG_OK(h, (zpl_uint32) status); h = IPSTACK_NLMSG_NEXT(h, status))
		{
			/* Finish of reading. */
			if (h->nlmsg_type == IPSTACK_NLMSG_DONE)
				return ret;

			/* Error handling. */
			if (h->nlmsg_type == IPSTACK_NLMSG_ERROR)
			{
				struct ipstack_nlmsgerr *err = (struct ipstack_nlmsgerr *) IPSTACK_NLMSG_DATA(h);
				int errnum = err->error;
				int msg_type = err->msg.nlmsg_type;

				/* If the error field is zero, then this is an ACK */
				if (err->error == 0)
				{
					if (IS_ZEBRA_DEBUG_KERNEL)
					{
						zlog_debug(MODULE_PAL,
								"%s: %s ACK: type=%s(%u), seq=%u, pid=%u",
								__FUNCTION__, nl->name,
								lookup(nlmsg_str, err->msg.nlmsg_type),
								err->msg.nlmsg_type, err->msg.nlmsg_seq,
								err->msg.nlmsg_pid);
					}

					/* return if not a multipart message, otherwise continue */
					if (!(h->nlmsg_flags & IPSTACK_NLM_F_MULTI))
					{
						return 0;
					}
					continue;
				}

				if (h->nlmsg_len < IPSTACK_NLMSG_LENGTH(sizeof(struct ipstack_nlmsgerr)))
				{
					zlog_err(MODULE_PAL, "%s error: message truncated", nl->name);
					return -1;
				}

				/* Deal with errors that occur because of races in link handling */
				//if (nl == &netlink_cmd.netlink_cmd)
				{
					if( (msg_type == IPSTACK_RTM_NEWROUTE && -errnum == IPSTACK_ERRNO_EEXIST)
							|| (msg_type == IPSTACK_RTM_DELROUTE && (-errnum == IPSTACK_ERRNO_ENODEV || -errnum == IPSTACK_ERRNO_ESRCH))
							|| (msg_type == IPSTACK_RTM_NEWADDR && -errnum == IPSTACK_ERRNO_EEXIST)
							|| (msg_type == IPSTACK_RTM_DELADDR && (-errnum == IPSTACK_ERRNO_ENODEV || -errnum == IPSTACK_ERRNO_ESRCH))
							|| (msg_type == IPSTACK_RTM_NEWLINK  && -errnum == IPSTACK_ERRNO_EEXIST)
							|| (msg_type == IPSTACK_RTM_DELLINK && (-errnum == IPSTACK_ERRNO_ENODEV || -errnum == IPSTACK_ERRNO_ESRCH)))
					{
						if (IS_ZEBRA_DEBUG_KERNEL)
							zlog_debug(MODULE_PAL,
									"%s: error: %s type=%s(%u), seq=%u, pid=%u",
									nl->name, ipstack_strerror(-errnum),
									lookup(nlmsg_str, msg_type), msg_type,
									err->msg.nlmsg_seq, err->msg.nlmsg_pid);
						return 0;
					}
				}

				zlog_err(MODULE_PAL, "%s error: %s, type=%s(%u), seq=%u, pid=%u",
						nl->name, ipstack_strerror(-errnum),
						lookup(nlmsg_str, msg_type), msg_type,
						err->msg.nlmsg_seq, err->msg.nlmsg_pid);
				return -1;
			}

			/* OK we got netlink message. */
/*
			if (IS_ZEBRA_DEBUG_KERNEL)
				zlog_debug(MODULE_PAL,
						"netlink_parse_info: %s type %s(%u), seq=%u, pid=%u",
						nl->name, lookup(nlmsg_str, h->nlmsg_type),
						h->nlmsg_type, h->nlmsg_seq, h->nlmsg_pid);
*/

			/* skip unsolicited messages originating from command ipstack_socket
			 * linux sets the originators port-id for {NEW|DEL}ADDR messages,
			 * so this has to be checked here. */
#ifdef ZPL_KERNEL_FORWARDING
			if ((h->nlmsg_type != IPSTACK_RTM_NEWADDR
							&& h->nlmsg_type != IPSTACK_RTM_DELADDR))
			{
				//if (IS_ZEBRA_DEBUG_KERNEL)
					zlog_debug(MODULE_PAL,
							"netlink_parse_info: %s packet comes from %s",
							netlink_cmd.netlink_cmd.name, nl->name);
				continue;
			}
#endif
			if(filter)
			{
/*
				zlog_debug(MODULE_PAL,
						"netlink_parse_info: %s type %s(%u), seq=%u, pid=%u",
						nl->name, lookup(nlmsg_str, h->nlmsg_type),
						h->nlmsg_type, h->nlmsg_seq, h->nlmsg_pid);
*/
				error = (*filter)(&snl, h, vrfid);
				if (error < 0)
				{
					zlog_err(MODULE_PAL, "%s filter function error", nl->name);
					ret = error;
				}
			}
			else
				ret = 0;
		}

		/* After error care. */
		if (msg.msg_flags & IPSTACK_MSG_TRUNC)
		{
			zlog_err(MODULE_PAL, "%s error: message truncated!", nl->name);
			zlog_err(MODULE_PAL, "Must restart with larger --nl-bufsize value!");
			continue;
		}
		if (status)
		{
			zlog_err(MODULE_PAL, "%s error: data remnant size %d", nl->name,
					status);
			return -1;
		}
	}
	return ret;
}

/*
static int netlink_talk_filter(struct ipstack_sockaddr_nl *snl, struct ipstack_nlmsghdr *h,
		vrf_id_t vrf_id)
{
	zlog_warn(MODULE_PAL, "netlink_talk: ignoring message type 0x%04x vrf %u",
			h->nlmsg_type, vrf_id);
	return 0;
}
*/

/* ipstack_sendmsg() to netlink ipstack_socket then ipstack_recvmsg(). */
int _netlink_talk(struct ipstack_nlmsghdr *n, struct nlsock *nl, vrf_id_t vrfid)
{
	zpl_uint32 status;
	struct ipstack_sockaddr_nl snl;
	struct ipstack_iovec iov =
	{ .iov_base = (void *) n, .iov_len = n->nlmsg_len };
	struct ipstack_msghdr msg =
	{ .msg_name = (void *) &snl, .msg_namelen = sizeof snl, .msg_iov = &iov,
			.msg_iovlen = 1, };
	int save_errno;

	memset(&snl, 0, sizeof snl);
	snl.nl_family = IPSTACK_AF_NETLINK;

	n->nlmsg_seq = ++nl->seq;

	/* Request an acknowledgement by setting IPSTACK_NLM_F_ACK */
	n->nlmsg_flags |= IPSTACK_NLM_F_ACK;

	if (IS_ZEBRA_DEBUG_KERNEL)
		zlog_debug(MODULE_PAL, "netlink_talk: %s type %s(%u), seq=%u", nl->name,
				lookup(nlmsg_str, n->nlmsg_type), n->nlmsg_type, n->nlmsg_seq);

	/* Send message to netlink interface. */
	status = ipstack_sendmsg(nl->sock, &msg, 0);
	save_errno = ipstack_errno;
	if (status < 0)
	{
		zlog_err(MODULE_PAL, "netlink_talk ipstack_sendmsg() error: %s",
				ipstack_strerror(save_errno));
		return -1;
	}

	/*
	 * Get reply from netlink ipstack_socket.
	 * The reply should either be an acknowlegement or an error.
	 */
	return _netlink_parse_info(NULL, nl, vrfid);
}

static int _netlink_recvbuf(struct nlsock *nl, zpl_socket_t sock, zpl_uint32 newsize)
{
	int ret = 0;
	zpl_uint32 nl_rcvbufsize = newsize;

	ret = ipstack_setsockopt(sock, IPSTACK_SOL_SOCKET, IPSTACK_SO_RCVBUF, &nl_rcvbufsize,
			sizeof(nl_rcvbufsize));
	if (ret < 0)
	{
		zlog_err(MODULE_PAL, "Can't set %s receive buffer size: %s", nl->name,
				ipstack_strerror(ipstack_errno));
		return -1;
	}
	ret = ipstack_setsockopt(sock, IPSTACK_SOL_SOCKET, IPSTACK_SO_SNDBUF, &nl_rcvbufsize,
			sizeof(nl_rcvbufsize));
	if (ret < 0)
	{
		zlog_err(MODULE_PAL, "Can't set %s ipstack_send buffer size: %s", nl->name,
				ipstack_strerror(ipstack_errno));
		return -1;
	}
	return 0;
}

/* Make ipstack_socket for Linux netlink interface. */
int _netlink_socket(struct nlsock *nl, zpl_ulong groups, vrf_id_t vrf_id)
{
	int ret;
	struct ipstack_sockaddr_nl snl;
	zpl_socket_t sock;
	zpl_uint32 namelen;
	int save_errno;

	sock = _kernel_vrf_socket(IPSTACK_AF_NETLINK, IPSTACK_SOCK_RAW, IPSTACK_NETLINK_ROUTE, vrf_id);
	if (ipstack_invalid(sock))
	{
		zlog_err(MODULE_PAL, "Can't open %s ipstack_socket: %s", nl->name,
				ipstack_strerror(ipstack_errno));
		return -1;
	}

	memset(&snl, 0, sizeof snl);
	snl.nl_family = IPSTACK_AF_NETLINK;
	snl.nl_groups = groups;

	/* Bind the ipstack_socket to the netlink structure for anything. */
	ret = ipstack_bind(sock, (struct ipstack_sockaddr *) &snl, sizeof snl);
	save_errno = ipstack_errno;

	if (ret < 0)
	{
		zlog_err(MODULE_PAL, "Can't ipstack_bind %s ipstack_socket to group 0x%x: %s", nl->name,
				snl.nl_groups, ipstack_strerror(save_errno));
		ipstack_close(sock);
		return -1;
	}
	if (_netlink_recvbuf(nl, sock, NL_PKT_BUF_SIZE) != 0)
	{
		ipstack_close(sock);
		return -1;
	}
	/* multiple netlink sockets will have different nl_pid */
	namelen = sizeof snl;
	ret = ipstack_getsockname(sock, (struct ipstack_sockaddr *) &snl, (socklen_t *) &namelen);
	if (ret < 0 || namelen != sizeof snl)
	{
		zlog_err(MODULE_PAL, "Can't get %s ipstack_socket name: %s", nl->name,
				ipstack_strerror(ipstack_errno));
		ipstack_close(sock);
		return -1;
	}
	if (snl.nl_family != IPSTACK_AF_NETLINK)
	{
		zlog_err(MODULE_PAL, "Wrong address family %d\n", snl.nl_family);
		ipstack_close(sock);
		return -1;
	}
	nl->snl = snl;
	nl->sock = sock;
	nl->snl.nl_pid = getpid();
	return ret;
}

/* Filter out messages from self that occur on listener ipstack_socket,
 caused by our actions on the command ipstack_socket
 */
#ifdef ZPL_KERNEL_FORWARDING
static void _netlink_install_filter(zpl_socket_t sock, __u32 pid)
{
	struct sock_filter filter[] =
	{
	/* 0: ldh [4]	          */
	BPF_STMT(BPF_LD | BPF_ABS | BPF_H, offsetof(struct ipstack_nlmsghdr, nlmsg_type)),
	/* 1: jeq 0x18 jt 3 jf 6  */
	BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, htons(IPSTACK_RTM_NEWROUTE), 1, 0),
	/* 2: jeq 0x19 jt 3 jf 6  */
	BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, htons(IPSTACK_RTM_DELROUTE), 0, 3),
	/* 3: ldw [12]		  */
	BPF_STMT(BPF_LD | BPF_ABS | BPF_W, offsetof(struct ipstack_nlmsghdr, nlmsg_pid)),
	/* 4: jeq XX  jt 5 jf 6   */
	BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, htonl(pid), 0, 1),
	/* 5: ret 0    (skip)     */
	BPF_STMT(BPF_RET | BPF_K, 0),
	/* 6: ret 0xffff (keep)   */
	BPF_STMT(BPF_RET | BPF_K, 0xffff), };

	struct sock_fprog prog =
	{ .len = array_size(filter), .filter = filter, };

	if (ipstack_setsockopt(sock, IPSTACK_SOL_SOCKET, SO_ATTACH_FILTER, &prog, sizeof(prog)) < 0)
		zlog_warn(MODULE_PAL, "Can't install ipstack_socket filter: %s\n",
				ipstack_strerror(ipstack_errno));
}
#endif
/* Exported interface function.  This function simply calls
 netlink_socket (). */
static void _kernel_nl_open(struct nlsock *nl)
{
	zpl_ulong groups;

	groups = IPSTACK_RTMGRP_LINK | IPSTACK_RTMGRP_IPV4_ROUTE | IPSTACK_RTMGRP_IPV4_IFADDR;
#ifdef ZPL_BUILD_IPV6
	groups |= IPSTACK_RTMGRP_IPV6_ROUTE | IPSTACK_RTMGRP_IPV6_IFADDR;
#endif /* ZPL_BUILD_IPV6 */
#ifdef ZPL_KERNEL_FORWARDING
	_netlink_socket(nl, groups, nl->vrf_id);
	/* Register kernel ipstack_socket. */
	if (!ipstack_invalid(nl->sock))
	{
		/* Only want non-blocking on the netlink event ipstack_socket */
		ipstack_set_nonblocking(nl->sock);
		/* Set receive buffer size if it's set from command line */
		nl_rcvbuf.p = XMALLOC(MTYPE_STREAM, NL_PKT_BUF_SIZE);
		nl_rcvbuf.size = NL_PKT_BUF_SIZE;

		_netlink_install_filter(nl->sock, nl->snl.nl_pid);

		_netlink_listen(zvrf);
	}
#endif
	_netlink_socket(nl, 0, nl->vrf_id);
}

void _netlink_close(void)
{
#ifdef ZPL_KERNEL_FORWARDING
	if (netlink_cmd.t_netlink)
	{
		THREAD_READ_OFF(netlink_cmd.t_netlink);
		netlink_cmd.t_netlink = NULL;
	}
	if (!ipstack_invalid(netlink_cmd.sock))
	{
		ipstack_close(netlink_cmd.sock);
	}
	if (netlink_cmd.name)
	{
		XFREE(MTYPE_NETLINK_NAME, netlink_cmd.name);
		netlink_cmd.name = NULL;
	}
	netlink_cmd.snl.nl_pid = 0;
#endif
	if (!ipstack_invalid(netlink_cmd.sock))
	{
		ipstack_close(netlink_cmd.sock);
	}
	if (netlink_cmd.name)
	{
		XFREE(MTYPE_NETLINK_NAME, netlink_cmd.name);
		netlink_cmd.name = NULL;
	}
	netlink_cmd.snl.nl_pid = 0;
}

void _netlink_open(vrf_id_t vrfid)
{
	char nl_name[64];
	/* Initialize netlink sockets */
#ifdef ZPL_KERNEL_FORWARDING
	snprintf(nl_name, 64, "nllisten-%u", vrfid);

	netlink_cmd.name = XSTRDUP(MTYPE_NETLINK_NAME, nl_name);
	netlink_cmd.snl.nl_pid = getpid();
#endif
	snprintf(nl_name, 64, "nlcmd-%u", vrfid);
	netlink_cmd.name = XSTRDUP(MTYPE_NETLINK_NAME, nl_name);
	netlink_cmd.snl.nl_pid = getpid();
	netlink_cmd.vrf_id = vrfid;
	_kernel_nl_open(&netlink_cmd);
}

#if 0
static int _netlink_load_all_one (struct ip_vrf *vrf, void *p)
{
	struct nsm_ip_vrf *zvrf = vrf->info;
	_netlink_open(zvrf);

	//kernel_interface_load(zvrf);
	//kernel_route_table_load(zvrf);
	return 0;			
}

void _netlink_load_all()
{
	ip_vrf_foreach(_netlink_load_all_one, NULL);
	return;
}
#endif