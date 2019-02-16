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
//#include <net/if_arp.h>

/* Hack for GNU libc version 2. */
#ifndef MSG_TRUNC
#define MSG_TRUNC      0x20
#endif /* MSG_TRUNC */

#include "linklist.h"
#include "if.h"
#include "connected.h"
#include "log.h"
#include "prefix.h"
#include "table.h"
#include "memory.h"
#include "rib.h"
#include "thread.h"
#include "vrf.h"
#include "nexthop.h"

#include "zserv.h"

#include "redistribute.h"
#include "interface.h"
#include "debug.h"

#include "kernel_netlink.h"

static const struct message nlmsg_str[] =
{
		{ RTM_NEWROUTE, "RTM_NEWROUTE" },
		{ RTM_DELROUTE, "RTM_DELROUTE" },
		{ RTM_GETROUTE, "RTM_GETROUTE" },
		{ RTM_NEWLINK, "RTM_NEWLINK" },
		{ RTM_DELLINK, "RTM_DELLINK" },
		{ RTM_GETLINK, "RTM_GETLINK" },
		{ RTM_NEWADDR, "RTM_NEWADDR" },
		{ RTM_DELADDR, "RTM_DELADDR" },
		{ RTM_GETADDR, "RTM_GETADDR" },
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
		{ RTPROT_REDIRECT, "redirect" },
		{ RTPROT_KERNEL, "kernel" },
		{ RTPROT_BOOT, "boot" },
		{ RTPROT_STATIC, "static" },
		{ RTPROT_GATED, "GateD" },
		{ RTPROT_RA, "router advertisement" },
		{ RTPROT_MRT, "MRT" },
		{ RTPROT_ZEBRA, "Zebra" },
#ifdef RTPROT_BIRD
		{	RTPROT_BIRD, "BIRD"},
#endif /* RTPROT_BIRD */
		{ 0, NULL }
};

/*
 * nl_msg_type_to_str
 */
const char *
nl_msg_type_to_str(uint16_t msg_type)
{
	return lookup(nlmsg_str, msg_type);
}

/*
 * nl_rtproto_to_str
 */
const char *
nl_rtproto_to_str(u_char rtproto)
{
	return lookup(rtproto_str, rtproto);
}

/* Utility function  comes from iproute2.
 Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru> */
int addattr_l(struct nlmsghdr *n, size_t maxlen, int type, void *data,
		size_t alen)
{
	size_t len;
	struct rtattr *rta;

	len = RTA_LENGTH(alen);

	if (NLMSG_ALIGN(n->nlmsg_len) + len > maxlen)
		return -1;

	rta = (struct rtattr *) (((char *) n) + NLMSG_ALIGN(n->nlmsg_len));
	rta->rta_type = type;
	rta->rta_len = len;
	if (data)
		memcpy(RTA_DATA(rta), data, alen);
	n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + len;

	return 0;
}

int rta_addattr_l(struct rtattr *rta, size_t maxlen, int type, void *data,
		size_t alen)
{
	size_t len;
	struct rtattr *subrta;

	len = RTA_LENGTH(alen);

	if (RTA_ALIGN(rta->rta_len) + len > maxlen)
		return -1;

	subrta = (struct rtattr *) (((char *) rta) + RTA_ALIGN(rta->rta_len));
	subrta->rta_type = type;
	subrta->rta_len = len;
	if (data)
		memcpy(RTA_DATA(subrta), data, alen);
	rta->rta_len = NLMSG_ALIGN(rta->rta_len) + len;

	return 0;
}

/* Utility function comes from iproute2.
 Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru> */
int addattr32(struct nlmsghdr *n, size_t maxlen, int type, int data)
{
	size_t len;
	struct rtattr *rta;

	len = RTA_LENGTH(4);

	if (NLMSG_ALIGN(n->nlmsg_len) + len > maxlen)
		return -1;

	rta = (struct rtattr *) (((char *) n) + NLMSG_ALIGN(n->nlmsg_len));
	rta->rta_type = type;
	rta->rta_len = len;
	memcpy(RTA_DATA(rta), &data, 4);
	n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + len;

	return 0;
}

struct rtattr *addattr_nest(struct nlmsghdr *n, int maxlen, int type)
{
	struct rtattr *nest = NLMSG_TAIL(n);

	addattr_l(n, maxlen, type, NULL, 0);
	return nest;
}

int addattr_nest_end(struct nlmsghdr *n, struct rtattr *nest)
{
	nest->rta_len = (void *) NLMSG_TAIL(n) - (void *) nest;
	return n->nlmsg_len;
}

/* Utility function for parse rtattr. */
void netlink_parse_rtattr(struct rtattr **tb, int max, struct rtattr *rta,
		int len)
{
	while (RTA_OK(rta, len))
	{
		if (rta->rta_type <= max)
			tb[rta->rta_type] = rta;
		rta = RTA_NEXT(rta, len);
	}
}

/* Utility function to parse hardware link-layer address and update ifp */
void netlink_interface_update_hw_addr(struct rtattr **tb, struct interface *ifp)
{
	int i;

	if (tb[IFLA_ADDRESS])
	{
		int hw_addr_len;

		hw_addr_len = RTA_PAYLOAD(tb[IFLA_ADDRESS]);

		if (hw_addr_len > INTERFACE_HWADDR_MAX)
			zlog_warn(ZLOG_PAL, "Hardware address is too large: %d",
					hw_addr_len);
		else
		{
			ifp->hw_addr_len = hw_addr_len;
			memcpy(ifp->hw_addr, RTA_DATA(tb[IFLA_ADDRESS]), hw_addr_len);

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
void _netlink_route_debug(int cmd, struct prefix *p, struct nexthop *nexthop,
		const char *routedesc, int family, struct nsm_vrf *zvrf)
{
	if (IS_ZEBRA_DEBUG_KERNEL)
	{
		char buf[PREFIX_STRLEN];
		zlog_debug(ZLOG_PAL,
				"netlink_route_multipath() (%s): %s %s vrf %u type %s",
				routedesc, lookup(nlmsg_str, cmd),
				prefix2str(p, buf, sizeof(buf)), zvrf->vrf_id,
				nexthop_type_to_str(nexthop->type));
	}
}

/* Note: on netlink systems, there should be a 1-to-1 mapping between interface
 names and ifindex values. */
void set_ifindex(struct interface *ifp, ifindex_t ifi_index)
{
	struct interface *oifp;

	if (((oifp = if_lookup_by_kernel_index(ifi_index)) != NULL)
			&& (oifp != ifp))
	{
		if (ifi_index == IFINDEX_INTERNAL)
			zlog_err(ZLOG_PAL,
					"Netlink is setting interface %s k_ifindex to reserved "
							"internal value %u", ifp->k_name, ifi_index);
		else
		{
			if (IS_ZEBRA_DEBUG_KERNEL)
				zlog_debug(ZLOG_PAL,
						"interface index %d was renamed from %s to %s",
						ifi_index, oifp->name, ifp->name);
			if (if_is_up(oifp))
				zlog_err(ZLOG_PAL,
						"interface rename detected on up interface: index %d "
								"was renamed from %s to %s, results are uncertain!",
						ifi_index, oifp->name, ifp->name);
			// zhurish if_delete_update(oifp);
		}
	}
	ifp->k_ifindex = ifi_index;
}

/* Get type specified information from netlink. */
int netlink_request(int family, int type, struct nlsock *nl)
{
	int ret;
	struct sockaddr_nl snl;
	int save_errno;

	struct
	{
		struct nlmsghdr nlh;
		struct rtgenmsg g;
	} req;

	/* Check netlink socket. */
	if (nl->sock < 0)
	{
		zlog_err(ZLOG_PAL, "%s socket isn't active.", nl->name);
		return -1;
	}

	memset(&snl, 0, sizeof snl);
	snl.nl_family = AF_NETLINK;

	memset(&req, 0, sizeof req);
	req.nlh.nlmsg_len = sizeof req;
	req.nlh.nlmsg_type = type;
	req.nlh.nlmsg_flags = NLM_F_ROOT | NLM_F_MATCH | NLM_F_REQUEST;
	req.nlh.nlmsg_pid = nl->snl.nl_pid;
	req.nlh.nlmsg_seq = ++nl->seq;
	req.g.rtgen_family = family;

	ret = sendto(nl->sock, (void *) &req, sizeof req, 0,
			(struct sockaddr *) &snl, sizeof snl);
	save_errno = errno;

	if (ret < 0)
	{
		zlog_err(ZLOG_PAL, "%s sendto failed: %s", nl->name,
				safe_strerror(save_errno));
		return -1;
	}
	return 0;
}

/* Receive message from netlink interface and pass those information
 to the given function. */
int netlink_parse_info(
		int (*filter)(struct sockaddr_nl *, struct nlmsghdr *, vrf_id_t),
		struct nlsock *nl, struct nsm_vrf *zvrf)
{
	int status;
	int ret = 0;
	int error;
	struct sockaddr_nl snl;
	struct nlmsghdr *h;
	while (1)
	{
		struct iovec iov =
		{
			.iov_base = nl_rcvbuf.p,
			.iov_len = nl_rcvbuf.size,
		};

		struct msghdr msg =
		{
			.msg_name = (void *) &snl,
			.msg_namelen = sizeof snl,
			.msg_iov = &iov,
			.msg_iovlen = 1
		};


		status = recvmsg(nl->sock, &msg, 0);
		if (status < 0)
		{
			if (errno == EINTR)
				continue;
			if (errno == EWOULDBLOCK || errno == EAGAIN)
				break;
			zlog_err(ZLOG_PAL, "%s recvmsg overrun: %s", nl->name,
					safe_strerror(errno));
			continue;
		}

		if (status == 0)
		{
			zlog_err(ZLOG_PAL, "%s EOF", nl->name);
			return -1;
		}

		if (msg.msg_namelen != sizeof snl)
		{
			zlog_err(ZLOG_PAL, "%s sender address length error: length %d",
					nl->name, msg.msg_namelen);
			return -1;
		}

		for (h = (struct nlmsghdr *) nl_rcvbuf.p;
				NLMSG_OK(h, (unsigned int) status); h = NLMSG_NEXT(h, status))
		{
			/* Finish of reading. */
			if (h->nlmsg_type == NLMSG_DONE)
				return ret;

			/* Error handling. */
			if (h->nlmsg_type == NLMSG_ERROR)
			{
				struct nlmsgerr *err = (struct nlmsgerr *) NLMSG_DATA(h);
				int errnum = err->error;
				int msg_type = err->msg.nlmsg_type;

				/* If the error field is zero, then this is an ACK */
				if (err->error == 0)
				{
					if (IS_ZEBRA_DEBUG_KERNEL)
					{
						zlog_debug(ZLOG_PAL,
								"%s: %s ACK: type=%s(%u), seq=%u, pid=%u",
								__FUNCTION__, nl->name,
								lookup(nlmsg_str, err->msg.nlmsg_type),
								err->msg.nlmsg_type, err->msg.nlmsg_seq,
								err->msg.nlmsg_pid);
					}

					/* return if not a multipart message, otherwise continue */
					if (!(h->nlmsg_flags & NLM_F_MULTI))
					{
						return 0;
					}
					continue;
				}

				if (h->nlmsg_len < NLMSG_LENGTH(sizeof(struct nlmsgerr)))
				{
					zlog_err(ZLOG_PAL, "%s error: message truncated", nl->name);
					return -1;
				}

				/* Deal with errors that occur because of races in link handling */
				if (nl == &zvrf->netlink_cmd)
				{
					if( (msg_type == RTM_NEWROUTE && -errnum == EEXIST)
							|| (msg_type == RTM_DELROUTE && (-errnum == ENODEV || -errnum == ESRCH))
							|| (msg_type == RTM_NEWADDR && -errnum == EEXIST)
							|| (msg_type == RTM_DELADDR && (-errnum == ENODEV || -errnum == ESRCH))
							|| (msg_type == RTM_NEWLINK  && -errnum == EEXIST)
							|| (msg_type == RTM_DELLINK && (-errnum == ENODEV || -errnum == ESRCH)))
					{
						if (IS_ZEBRA_DEBUG_KERNEL)
							zlog_debug(ZLOG_PAL,
									"%s: error: %s type=%s(%u), seq=%u, pid=%u",
									nl->name, safe_strerror(-errnum),
									lookup(nlmsg_str, msg_type), msg_type,
									err->msg.nlmsg_seq, err->msg.nlmsg_pid);
						return 0;
					}
				}

				zlog_err(ZLOG_PAL, "%s error: %s, type=%s(%u), seq=%u, pid=%u",
						nl->name, safe_strerror(-errnum),
						lookup(nlmsg_str, msg_type), msg_type,
						err->msg.nlmsg_seq, err->msg.nlmsg_pid);
				return -1;
			}

			/* OK we got netlink message. */
			if (IS_ZEBRA_DEBUG_KERNEL)
				zlog_debug(ZLOG_PAL,
						"netlink_parse_info: %s type %s(%u), seq=%u, pid=%u",
						nl->name, lookup(nlmsg_str, h->nlmsg_type),
						h->nlmsg_type, h->nlmsg_seq, h->nlmsg_pid);

			/* skip unsolicited messages originating from command socket
			 * linux sets the originators port-id for {NEW|DEL}ADDR messages,
			 * so this has to be checked here. */
			if (nl == &zvrf->netlink
					&& h->nlmsg_pid == zvrf->netlink_cmd.snl.nl_pid
					&& (h->nlmsg_type != RTM_NEWADDR
							&& h->nlmsg_type != RTM_DELADDR))
			{
				if (IS_ZEBRA_DEBUG_KERNEL)
					zlog_debug(ZLOG_PAL,
							"netlink_parse_info: %s packet comes from %s",
							zvrf->netlink_cmd.name, nl->name);
				continue;
			}
			if(filter)
			{
				error = (*filter)(&snl, h, zvrf->vrf_id);
				if (error < 0)
				{
					zlog_err(ZLOG_PAL, "%s filter function error", nl->name);
					ret = error;
				}
			}
			else
				ret = 0;
		}

		/* After error care. */
		if (msg.msg_flags & MSG_TRUNC)
		{
			zlog_err(ZLOG_PAL, "%s error: message truncated!", nl->name);
			zlog_err(ZLOG_PAL, "Must restart with larger --nl-bufsize value!");
			continue;
		}
		if (status)
		{
			zlog_err(ZLOG_PAL, "%s error: data remnant size %d", nl->name,
					status);
			return -1;
		}
	}
	return ret;
}

/*
static int netlink_talk_filter(struct sockaddr_nl *snl, struct nlmsghdr *h,
		vrf_id_t vrf_id)
{
	zlog_warn(ZLOG_PAL, "netlink_talk: ignoring message type 0x%04x vrf %u",
			h->nlmsg_type, vrf_id);
	return 0;
}
*/

/* sendmsg() to netlink socket then recvmsg(). */
int netlink_talk(struct nlmsghdr *n, struct nlsock *nl, struct nsm_vrf *zvrf)
{
	int status;
	struct sockaddr_nl snl;
	struct iovec iov =
	{ .iov_base = (void *) n, .iov_len = n->nlmsg_len };
	struct msghdr msg =
	{ .msg_name = (void *) &snl, .msg_namelen = sizeof snl, .msg_iov = &iov,
			.msg_iovlen = 1, };
	int save_errno;

	memset(&snl, 0, sizeof snl);
	snl.nl_family = AF_NETLINK;

	n->nlmsg_seq = ++nl->seq;

	/* Request an acknowledgement by setting NLM_F_ACK */
	n->nlmsg_flags |= NLM_F_ACK;

	if (IS_ZEBRA_DEBUG_KERNEL)
		zlog_debug(ZLOG_PAL, "netlink_talk: %s type %s(%u), seq=%u", nl->name,
				lookup(nlmsg_str, n->nlmsg_type), n->nlmsg_type, n->nlmsg_seq);

	/* Send message to netlink interface. */
	status = sendmsg(nl->sock, &msg, 0);
	save_errno = errno;
	if (status < 0)
	{
		zlog_err(ZLOG_PAL, "netlink_talk sendmsg() error: %s",
				safe_strerror(save_errno));
		return -1;
	}

	/*
	 * Get reply from netlink socket.
	 * The reply should either be an acknowlegement or an error.
	 */
	return netlink_parse_info(NULL, nl, zvrf);
}

static int netlink_recvbuf(struct nlsock *nl, int sock, int newsize)
{
	int ret = 0, nl_rcvbufsize = newsize;

	ret = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &nl_rcvbufsize,
			sizeof(nl_rcvbufsize));
	if (ret < 0)
	{
		zlog_err(ZLOG_PAL, "Can't set %s receive buffer size: %s", nl->name,
				safe_strerror(errno));
		return -1;
	}
	ret = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &nl_rcvbufsize,
			sizeof(nl_rcvbufsize));
	if (ret < 0)
	{
		zlog_err(ZLOG_PAL, "Can't set %s send buffer size: %s", nl->name,
				safe_strerror(errno));
		return -1;
	}
	return 0;
}

/* Make socket for Linux netlink interface. */
int netlink_socket(struct nlsock *nl, unsigned long groups, vrf_id_t vrf_id)
{
	int ret;
	struct sockaddr_nl snl;
	int sock;
	int namelen;
	int save_errno;

	sock = vrf_socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE, vrf_id);
	if (sock < 0)
	{
		zlog_err(ZLOG_PAL, "Can't open %s socket: %s", nl->name,
				safe_strerror(errno));
		return -1;
	}

	memset(&snl, 0, sizeof snl);
	snl.nl_family = AF_NETLINK;
	snl.nl_groups = groups;

	/* Bind the socket to the netlink structure for anything. */
	ret = bind(sock, (struct sockaddr *) &snl, sizeof snl);
	save_errno = errno;

	if (ret < 0)
	{
		zlog_err(ZLOG_PAL, "Can't bind %s socket to group 0x%x: %s", nl->name,
				snl.nl_groups, safe_strerror(save_errno));
		close(sock);
		return -1;
	}
	if (netlink_recvbuf(nl, sock, NL_PKT_BUF_SIZE) != 0)
	{
		close(sock);
		return -1;
	}
	/* multiple netlink sockets will have different nl_pid */
	namelen = sizeof snl;
	ret = getsockname(sock, (struct sockaddr *) &snl, (socklen_t *) &namelen);
	if (ret < 0 || namelen != sizeof snl)
	{
		zlog_err(ZLOG_PAL, "Can't get %s socket name: %s", nl->name,
				safe_strerror(errno));
		close(sock);
		return -1;
	}
	if (snl.nl_family != AF_NETLINK)
	{
		zlog_err(ZLOG_PAL, "Wrong address family %d\n", snl.nl_family);
		close(sock);
		return -1;
	}
	nl->snl = snl;
	nl->sock = sock;
	nl->snl.nl_pid = getpid();
	return ret;
}

/* Filter out messages from self that occur on listener socket,
 caused by our actions on the command socket
 */
static void netlink_install_filter(int sock, __u32 pid)
{
	struct sock_filter filter[] =
	{
	/* 0: ldh [4]	          */
	BPF_STMT(BPF_LD | BPF_ABS | BPF_H, offsetof(struct nlmsghdr, nlmsg_type)),
	/* 1: jeq 0x18 jt 3 jf 6  */
	BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, htons(RTM_NEWROUTE), 1, 0),
	/* 2: jeq 0x19 jt 3 jf 6  */
	BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, htons(RTM_DELROUTE), 0, 3),
	/* 3: ldw [12]		  */
	BPF_STMT(BPF_LD | BPF_ABS | BPF_W, offsetof(struct nlmsghdr, nlmsg_pid)),
	/* 4: jeq XX  jt 5 jf 6   */
	BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, htonl(pid), 0, 1),
	/* 5: ret 0    (skip)     */
	BPF_STMT(BPF_RET | BPF_K, 0),
	/* 6: ret 0xffff (keep)   */
	BPF_STMT(BPF_RET | BPF_K, 0xffff), };

	struct sock_fprog prog =
	{ .len = array_size(filter), .filter = filter, };

	if (setsockopt(sock, SOL_SOCKET, SO_ATTACH_FILTER, &prog, sizeof(prog)) < 0)
		zlog_warn(ZLOG_PAL, "Can't install socket filter: %s\n",
				safe_strerror(errno));
}

/* Exported interface function.  This function simply calls
 netlink_socket (). */
static void kernel_nl_open(struct nsm_vrf *zvrf)
{
	unsigned long groups;

	groups = RTMGRP_LINK | RTMGRP_IPV4_ROUTE | RTMGRP_IPV4_IFADDR;
#ifdef HAVE_IPV6
	groups |= RTMGRP_IPV6_ROUTE | RTMGRP_IPV6_IFADDR;
#endif /* HAVE_IPV6 */
	netlink_socket(&zvrf->netlink, groups, zvrf->vrf_id);
	netlink_socket(&zvrf->netlink_cmd, 0, zvrf->vrf_id);

	/* Register kernel socket. */
	if (zvrf->netlink.sock > 0)
	{
		/* Only want non-blocking on the netlink event socket */
		if (fcntl(zvrf->netlink.sock, F_SETFL, O_NONBLOCK) < 0)
			zlog_err(ZLOG_PAL, "Can't set %s socket flags: %s",
					zvrf->netlink.name, safe_strerror(errno));

		/* Set receive buffer size if it's set from command line */
		nl_rcvbuf.p = XMALLOC(MTYPE_STREAM, NL_PKT_BUF_SIZE);
		nl_rcvbuf.size = NL_PKT_BUF_SIZE;

		netlink_install_filter(zvrf->netlink.sock, zvrf->netlink.snl.nl_pid);

		kernel_nllisten(zvrf);
	}
}

void kernel_close(struct nsm_vrf *zvrf)
{
	if (zvrf->t_netlink)
	{
		THREAD_READ_OFF(zvrf->t_netlink);
		zvrf->t_netlink = NULL;
	}
	if (zvrf->netlink.sock >= 0)
	{
		close(zvrf->netlink.sock);
		zvrf->netlink.sock = -1;
	}

	if (zvrf->netlink_cmd.sock >= 0)
	{
		close(zvrf->netlink_cmd.sock);
		zvrf->netlink_cmd.sock = -1;
	}
	if (zvrf->netlink.name)
	{
		XFREE(MTYPE_NETLINK_NAME, zvrf->netlink.name);
		zvrf->netlink.name = NULL;
	}
	if (zvrf->netlink_cmd.name)
	{
		XFREE(MTYPE_NETLINK_NAME, zvrf->netlink_cmd.name);
		zvrf->netlink_cmd.name = NULL;
	}
	zvrf->netlink.snl.nl_pid = 0;
	zvrf->netlink_cmd.snl.nl_pid = 0;
}

void kernel_open(struct nsm_vrf *zvrf)
{
	char nl_name[64];
	/* Initialize netlink sockets */
	snprintf(nl_name, 64, "nllisten-%u", zvrf->vrf_id);
	zvrf->netlink.sock = -1;
	zvrf->netlink.name = XSTRDUP(MTYPE_NETLINK_NAME, nl_name);

	snprintf(nl_name, 64, "nlcmd-%u", zvrf->vrf_id);
	zvrf->netlink_cmd.sock = -1;
	zvrf->netlink_cmd.name = XSTRDUP(MTYPE_NETLINK_NAME, nl_name);

	zvrf->netlink.snl.nl_pid = getpid();
	zvrf->netlink_cmd.snl.nl_pid = getpid();

	kernel_nl_open(zvrf);
	/*	interface_lookup_netlink(zvrf);
	 netlink_route_read(zvrf);*/
}

void kernel_load_all()
{
	//int ret = 0;
	struct nsm_vrf *zvrf;
	vrf_iter_t iter;

	for (iter = vrf_first(); iter != VRF_ITER_INVALID; iter = vrf_next(iter))
	{
		if ((zvrf = vrf_iter2info(iter)) != NULL)
		{
			kernel_open(zvrf);
		}
	}
	return;
}
