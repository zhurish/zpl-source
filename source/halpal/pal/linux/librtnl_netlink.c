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

#include "librtnl_netlink.h"


struct nlsock netlink_cmd;

#ifdef ZPL_KERNEL_NETLINK

/* Hack for GNU libc version 2. */
#ifndef IPSTACK_MSG_TRUNC
#define IPSTACK_MSG_TRUNC      0x20
#endif /* IPSTACK_MSG_TRUNC */



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
 * librtnl_msg_type_to_str
 */
const char *
librtnl_msg_type_to_str(zpl_uint16 msg_type)
{
	return lookup(nlmsg_str, msg_type);
}

/*
 * librtnl_rtproto_to_str
 */
const char *
librtnl_rtproto_to_str(zpl_uchar rtproto)
{
	return lookup(rtproto_str, rtproto);
}

/* Utility function  comes from iproute2.
 Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru> */
int librtnl_addattr_l(struct ipstack_nlmsghdr *n, size_t maxlen, zpl_uint32 type, void *data,
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

int librtnl_addattr(struct ipstack_nlmsghdr *n, int maxlen, int type)
{
	return librtnl_addattr_l(n, maxlen, type, NULL, 0);
}


int librtnl_addattr8(struct ipstack_nlmsghdr *n, int maxlen, int type, zpl_uint8 data)
{
	return librtnl_addattr_l(n, maxlen, type, &data, sizeof(zpl_uint8));
}

int librtnl_addattr16(struct ipstack_nlmsghdr *n, int maxlen, int type, zpl_uint16 data)
{
	return librtnl_addattr_l(n, maxlen, type, &data, sizeof(zpl_uint16));
}


int librtnl_addattr64(struct ipstack_nlmsghdr *n, int maxlen, int type, zpl_uint64 data)
{
	return librtnl_addattr_l(n, maxlen, type, &data, sizeof(zpl_uint64));
}

int librtnl_addattrstrz(struct ipstack_nlmsghdr *n, int maxlen, int type, const char *str)
{
	return librtnl_addattr_l(n, maxlen, type, str, strlen(str)+1);
}

int librtnl_rta_addattr_l(struct ipstack_rtattr *rta, size_t maxlen, zpl_uint32 type, void *data,
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
int librtnl_addattr32(struct ipstack_nlmsghdr *n, size_t maxlen, zpl_uint32 type, zpl_uint32 data)
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

struct ipstack_rtattr *librtnl_addattr_nest(struct ipstack_nlmsghdr *n, size_t maxlen, zpl_uint32 type)
{
	struct ipstack_rtattr *nest = NLMSG_TAIL(n);
	librtnl_addattr_l(n, maxlen, type, NULL, 0);
	return nest;
}

int librtnl_addattr_nest_end(struct ipstack_nlmsghdr *n, struct ipstack_rtattr *nest)
{
	nest->rta_len = (char *) NLMSG_TAIL(n) - (char *) nest;
	return n->nlmsg_len;
}

/* Utility function for parse ipstack_rtattr. */
void librtnl_parse_rtattr(struct ipstack_rtattr **tb, zpl_uint32 max, struct ipstack_rtattr *rta,
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
void librtnl_interface_update_hw_addr(struct ipstack_rtattr **tb, struct interface *ifp)
{
	zpl_uint32 i;

	if (tb[IPSTACK_IFLA_ADDRESS])
	{
		int hw_addr_len;

		hw_addr_len = IPSTACK_RTA_PAYLOAD(tb[IPSTACK_IFLA_ADDRESS]);

		if (hw_addr_len > IF_HWADDR_MAX)
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


/* Note: on netlink systems, there should be a 1-to-1 mapping between interface
 names and ifindex values. */
void librtnl_set_ifindex(struct interface *ifp, ifkernindex_t ifi_index)
{
	struct interface *oifp;

	if (((oifp = if_lookup_by_kernel_index(ifi_index)) != NULL)
			&& (oifp != ifp))
	{
		if (ifi_index == IFINDEX_INTERNAL)
			zlog_err(MODULE_PAL,
					"Netlink is setting interface %s ker_ifindex to reserved "
							"internal value %u", ifp->ker_name, ifi_index);
		else
		{
			if (IS_NSM_DEBUG_KERNEL)
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
	ifp->ker_ifindex = ifi_index;
}

static int librtnl_cachesize(struct nlsock *nl, zpl_socket_t sock, zpl_uint32 newsize)
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

/* Get type specified information from netlink. */
int librtnl_request(struct nlsock *nl, zpl_family_t family, zpl_uint32 type)
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
int librtnl_parse_info(struct nlsock *nl, 
		int (*filter)(struct ipstack_sockaddr_nl *, struct ipstack_nlmsghdr *, void *),
		void *p)
{
	zpl_int32 status;
	int ret = 0;
	int error;
	struct ipstack_sockaddr_nl snl;
	struct ipstack_nlmsghdr *h;
	while (1)
	{
		struct ipstack_iovec iov =
		{
			.iov_base = nl->msgdata,
			.iov_len = nl->msgmax,
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

		for (h = (struct ipstack_nlmsghdr *) nl->msgdata;
				IPSTACK_NLMSG_OK(h, (zpl_int32) status); h = IPSTACK_NLMSG_NEXT(h, status))
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
					if (IS_NSM_DEBUG_KERNEL)
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
						if (IS_NSM_DEBUG_KERNEL)
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
			if (IS_NSM_DEBUG_KERNEL)
				zlog_debug(MODULE_PAL,
						"netlink_parse_info: %s type %s(%u), seq=%u, pid=%u",
						nl->name, lookup(nlmsg_str, h->nlmsg_type),
						h->nlmsg_type, h->nlmsg_seq, h->nlmsg_pid);
*/

			/* skip unsolicited messages originating from command ipstack_socket
			 * linux sets the originators port-id for {NEW|DEL}ADDR messages,
			 * so this has to be checked here. */

			if(filter)
			{

				zlog_debug(MODULE_PAL,
						"netlink_parse_info: %s type %s(%u), seq=%u, pid=%u",
						nl->name, lookup(nlmsg_str, h->nlmsg_type),
						h->nlmsg_type, h->nlmsg_seq, h->nlmsg_pid);

				error = (*filter)(&snl, h, p);
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

/* ipstack_sendmsg() to netlink ipstack_socket then ipstack_recvmsg(). */
int librtnl_talk(struct nlsock *nl, struct ipstack_nlmsghdr *n)
{
	zpl_int32 status;
	struct ipstack_sockaddr_nl snl;
	struct ipstack_iovec iov = { .iov_base = (void *) n, .iov_len = n->nlmsg_len };
	struct ipstack_msghdr msg =
	{ 
		.msg_name = (void *) &snl, 
		.msg_namelen = sizeof snl, 
		.msg_iov = &iov,
		.msg_iovlen = 1, 
	};
	int save_errno;

	memset(&snl, 0, sizeof snl);
	snl.nl_family = IPSTACK_AF_NETLINK;

	n->nlmsg_seq = ++nl->seq;

	/* Request an acknowledgement by setting IPSTACK_NLM_F_ACK */
	n->nlmsg_flags |= IPSTACK_NLM_F_ACK;

	if (IS_NSM_DEBUG_KERNEL)
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
	return librtnl_parse_info(nl, NULL, NULL);
}

int librtnl_talk_answer(struct nlsock *nl, struct ipstack_nlmsghdr *n, librtnl_filter_callback *cb, void *p)
{
	zpl_int32 status;
	struct ipstack_sockaddr_nl snl;
	struct ipstack_iovec iov = { .iov_base = (void *) n, .iov_len = n->nlmsg_len };
	struct ipstack_msghdr msg =
	{ 
		.msg_name = (void *) &snl, 
		.msg_namelen = sizeof snl, 
		.msg_iov = &iov,
		.msg_iovlen = 1, 
	};
	int save_errno;

	memset(&snl, 0, sizeof snl);
	snl.nl_family = IPSTACK_AF_NETLINK;

	n->nlmsg_seq = ++nl->seq;

	/* Request an acknowledgement by setting IPSTACK_NLM_F_ACK */
	n->nlmsg_flags |= IPSTACK_NLM_F_ACK;

	if (IS_NSM_DEBUG_KERNEL)
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
	return librtnl_parse_info(nl, cb, p);
}

static int librtnl_filter_ifnameindex_callback(struct ipstack_sockaddr_nl *nl, struct ipstack_nlmsghdr *answer, void *p)
{
	int *index_id = (int *)p;
	struct ipstack_ifinfomsg *ifm = IPSTACK_NLMSG_DATA(answer);
	if(ifm)
	{
		//zlog_err(MODULE_PAL, "------------ifi_index: %d", ifm->ifi_index);		
		if(index_id)
			*index_id = ifm->ifi_index;
		return ifm->ifi_index;
	}
	return 0;	
}

int librtnl_link_name2index(const char *name, int index)
{
	struct {
		struct ipstack_nlmsghdr		n;
		struct ipstack_ifinfomsg	ifm;
		char			buf[1024];
	} req = {
		.n.nlmsg_len = IPSTACK_NLMSG_LENGTH(sizeof(struct ipstack_ifinfomsg)),
		.n.nlmsg_flags = IPSTACK_NLM_F_REQUEST,
		.n.nlmsg_type = IPSTACK_RTM_GETLINK,
		.ifm.ifi_index = index,
	};
	unsigned int filt_mask = RTEXT_FILTER_VF | RTEXT_FILTER_SKIP_STATS;
	int rc = 0;

	librtnl_addattr32(&req.n, sizeof(req), IFLA_EXT_MASK, filt_mask);
	if (name)
		librtnl_addattr_l(&req.n, sizeof(req), IFLA_IFNAME/* : IFLA_ALT_IFNAME 53*/, name, strlen(name) + 1);

	librtnl_talk_answer( &netlink_cmd, &req.n, librtnl_filter_ifnameindex_callback, &rc);
	return rc;
}

int librtnl_link_updown(char *name, int up)
{
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
	req.iif.ifi_index = if_nametoindex(name);
	if(req.iif.ifi_index == 0)
		req.iif.ifi_index = librtnl_link_name2index(name, 0);
	if(up)
	{
		req.iif.ifi_change |= IPSTACK_IFF_UP|IPSTACK_IFF_RUNNING;
		req.iif.ifi_flags |= IPSTACK_IFF_UP|IPSTACK_IFF_RUNNING;
	}
	else
	{
		req.iif.ifi_change |= IPSTACK_IFF_UP|IPSTACK_IFF_RUNNING;
		req.iif.ifi_flags &= ~(IPSTACK_IFF_UP|IPSTACK_IFF_RUNNING);
	}
	return librtnl_talk(&netlink_cmd, &req.n);
}

int librtnl_link_destroy(char *name)
{
	struct
	{
		struct ipstack_nlmsghdr n;
		struct ipstack_ifinfomsg iif;
		char buf[NL_PKT_BUF_SIZE];
	} req;

	req.n.nlmsg_len = IPSTACK_NLMSG_LENGTH(sizeof(struct ipstack_ifinfomsg));
	req.n.nlmsg_flags = IPSTACK_NLM_F_REQUEST;
	req.n.nlmsg_type = IPSTACK_RTM_DELLINK;
	req.iif.ifi_family = IPSTACK_AF_UNSPEC;

	librtnl_addattr_l(&req.n, sizeof(req), IFLA_IFNAME, name, strlen(name) + 1);

	return librtnl_talk(&netlink_cmd, &req.n);
}


/* Make ipstack_socket for Linux netlink interface. */
int librtnl_socket(struct nlsock *nl, zpl_ulong groups, vrf_id_t vrf_id)
{
	int ret;
	struct ipstack_sockaddr_nl snl;
	zpl_socket_t sock;
	zpl_uint32 namelen;
	int save_errno;

	sock = ipstack_socket(IPSTACK_IPCOM, IPSTACK_AF_NETLINK, IPSTACK_SOCK_RAW, IPSTACK_NETLINK_ROUTE);
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
	if (librtnl_cachesize(nl, sock, nl->msgmax) != 0)
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

void librtnl_close(void)
{
#ifdef ZPL_LIBNL_MODULE	
	if (netlink_cmd.libnl_sock)
	{
		nl_close(netlink_cmd.libnl_sock);
		netlink_cmd.libnl_sock = NULL;
	}
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

int librtnl_open(vrf_id_t vrfid, zpl_uint32 msgsize)
{
	char nl_name[64];
	/* Initialize netlink sockets */

	snprintf(nl_name, 64, "nlcmd-%u", vrfid);
	netlink_cmd.name = XSTRDUP(MTYPE_NETLINK_NAME, nl_name);
	netlink_cmd.snl.nl_pid = getpid();
	netlink_cmd.vrf_id = vrfid;
	netlink_cmd.msgdata = XMALLOC(MTYPE_STREAM, msgsize);
	if(netlink_cmd.msgdata)
	{
		netlink_cmd.msgmax = msgsize;
		netlink_cmd.msglen = 0;
			
		librtnl_socket(&netlink_cmd, 0, netlink_cmd.vrf_id);
#ifdef ZPL_LIBNL_MODULE	
	netlink_cmd.libnl_sock = nl_socket_alloc();
	if (nl_connect(netlink_cmd.libnl_sock, NETLINK_ROUTE) < 0) {
		zlog_err(MODULE_PAL, "Can't get %s connect to route module, %s",
				ipstack_strerror(ipstack_errno));
		librtnl_close();
		return -1;
	}
#endif
		return OK;
	}
	if (netlink_cmd.name)
	{
		XFREE(MTYPE_NETLINK_NAME, netlink_cmd.name);
		netlink_cmd.name = NULL;
	}
	return ERROR;
}

#endif /* ZPL_KERNEL_NETLINK */

//ip link add vxlan0 type vxlan id 1 dstport 4789 192.168.100.2 local 192.168.100.1 ttl 64
/*
ip link add DEVICE type vxlan id VNI [ dev PHYS_DEV  ] [ { group | remote } IPADDR ] [ local { IPADDR | any
              } ] [ ttl TTL ] [ tos TOS ] [ df DF ] [ flowlabel FLOWLABEL ] [ dstport PORT ] [ srcport MIN MAX ] [
              [no]learning ] [ [no]proxy ] [ [no]rsc ] [ [no]l2miss ] [ [no]l3miss ] [ [no]udpcsum ] [ [no]udp6zerocsumtx
              ] [ [no]udp6zerocsumrx ] [ ageing SECONDS ] [ maxaddress NUMBER ] [ [no]external ] [ gbp ] [ gpe ]

ip link add DEVICE type { ipip | sit }  remote ADDR local ADDR [ encap { fou | gue | none } ] [ encap-sport
              { PORT | auto } ] [ encap-dport PORT ] [ [no]encap-csum ] [  [no]encap-remcsum ] [  mode  { ip6ip | ipip |
              mplsip | any } ] [ external ]

ip link add DEVICE type { gre | gretap }  remote ADDR local ADDR [ [no][i|o]seq ] [ [i|o]key KEY |
              no[i|o]key ] [ [no][i|o]csum ] [ ttl TTL ] [ tos TOS ] [ [no]pmtudisc ] [ [no]ignore-df ] [ dev PHYS_DEV ]
              [ encap { fou | gue | none } ] [ encap-sport { PORT | auto } ] [ encap-dport PORT ] [ [no]encap-csum ] [
              [no]encap-remcsum ] [ external ]

ip link add DEVICE type { ip6gre | ip6gretap } remote ADDR local ADDR [ [no][i|o]seq ] [ [i|o]key KEY |
              no[i|o]key ] [ [no][i|o]csum ] [ hoplimit TTL ] [ encaplimit ELIM ] [ tclass TCLASS ] [ flowlabel FLOWLABEL
              ] [ dscp inherit ] [ [no]allow-localremote ] [ dev PHYS_DEV ] [ external ]

ip link add DEVICE type { erspan | ip6erspan } remote ADDR local ADDR seq key KEY erspan_ver version [
              erspan IDX ] [ erspan_dir { ingress | egress } ] [ erspan_hwid hwid ] [ [no]allow-localremote ] [ external ]

ip link add DEVICE type geneve id VNI remote IPADDR [ ttl TTL ] [ tos TOS ] [ df DF ] [ flowlabel FLOWLABEL
              ] [ dstport PORT ] [ [no]external ] [ [no]udpcsum ] [ [no]udp6zerocsumtx ] [ [no]udp6zerocsumrx ]

ip link add DEVICE type vrf table TABLE

*/
#ifdef ZPL_LIBNL_MODULE
int libnl_netlink_link_updown(struct nlsock *nl, char *name, int up)
{
	int err = 0;
	struct rtnl_link *link, *change;
	struct nl_cache *cache;

	if ((err = rtnl_link_alloc_cache(nl->libnl_sock, AF_UNSPEC, &cache)) < 0) {
		zlog_err(MODULE_PAL, "Unable to allocate cache, %s", nl_geterror(err));
		return err;
	}

	if (!(link = rtnl_link_get_by_name(cache, name))) {
		zlog_err(MODULE_PAL, "Interface not found, %s", nl_geterror(err));
		err = 1;
		return err;
	}

	change = rtnl_link_alloc();
	
	if(up)
		rtnl_link_set_flags(change, IFF_UP);
	else
		rtnl_link_unset_flags(change, IFF_UP);
	if ((err = rtnl_link_change(nl->libnl_sock, link, change, 0)) < 0) {
		zlog_err(MODULE_PAL, "Unable to activate to %s,%s", name, nl_geterror(err));
		return err;
	}
	return err;
}

int libnl_netlink_link_del(struct nlsock *nl, char *name)
{
	int err = 0;
	struct rtnl_link *link;
	link = rtnl_link_alloc();
	rtnl_link_set_name(link, name);

	if ((err = rtnl_link_delete(nl->libnl_sock, link)) < 0) {
		zlog_err(MODULE_PAL, "Unable to delete link,%s", nl_geterror(err));
		return err;
	}
	rtnl_link_put(link);
	return err;
}

int libnl_netlink_link_add(struct nlsock *nl, void *data, int flag)
{
	int err = 0;
	if ((err = rtnl_link_add(nl->libnl_sock, data, flag)) < 0) {
		zlog_err(MODULE_PAL, "Unable to add link,%s", nl_geterror(err));
		return err;
	}
	rtnl_link_put(data);
	return err;
}
#endif
