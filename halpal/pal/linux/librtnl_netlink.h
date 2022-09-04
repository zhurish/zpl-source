/* Header file exported by rtlibrtnl.c to zebra.
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

#ifndef __LINUX_NETLINK_H__
#define __LINUX_NETLINK_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ZPL_LIBNL_MODULE
#include <netlink/netlink.h>
#include <netlink/route/link.h>
#include <netlink/route/link/bonding.h>
#include <netlink/route/link/vrf.h>
#include <netlink/route/link/bridge.h>
#include <netlink/route/link/can.h>
#include <netlink/route/link/ipgre.h>
#include <netlink/route/link/ipip.h>
#include <netlink/route/link/ipvlan.h>
#include <netlink/route/link/vlan.h>
#include <netlink/route/link/vxlan.h>

#include <netlink/route/link/ip6gre.h>
#include <netlink/route/link/ip6tnl.h>
#include <netlink/route/link/ip6vti.h>

#include <netlink/route/neighbour.h>
#include <netlink/route/neightbl.h>
#include <netlink/route/nexthop.h>
#include <netlink/route/route.h>
#include <linux/netlink.h>
#endif	

/* Socket interface to kernel */
struct nlsock
{
	vrf_id_t vrf_id;
#ifdef ZPL_KERNEL_NETLINK	
	zpl_socket_t sock;
	zpl_uint32 seq;
	struct ipstack_sockaddr_nl snl;
	const char *name;

	zpl_uint8	*msgdata;
	zpl_uint32	msglen;
	zpl_uint32	msgmax;
#endif /* ZPL_KERNEL_NETLINK*/
#ifdef ZPL_LIBNL_MODULE
	struct nl_sock *libnl_sock;
#endif		
};

extern struct nlsock netlink_cmd;


typedef int (*librtnl_filter_callback)(struct ipstack_sockaddr_nl *, struct ipstack_nlmsghdr *, void *p);

#ifdef ZPL_KERNEL_NETLINK

#define NL_PKT_BUF_SIZE 8192
#define NL_DEFAULT_ROUTE_METRIC 20

#define NLMSG_TAIL(nmsg) \
	((struct ipstack_rtattr *) (((char *) (nmsg)) + IPSTACK_NLMSG_ALIGN((nmsg)->nlmsg_len)))

/*
 * librtnl.c
 */
extern const char * librtnl_msg_type_to_str(zpl_uint16 msg_type);
extern const char * librtnl_rtproto_to_str(zpl_uchar rtproto);

extern int librtnl_addattr_l(struct ipstack_nlmsghdr *n, size_t maxlen, zpl_uint32 type, void *data,
		size_t alen);
extern int librtnl_rta_addattr_l(struct ipstack_rtattr *rta, size_t maxlen, zpl_uint32 type,
		void *data, size_t alen);
extern int librtnl_addattr(struct ipstack_nlmsghdr *n, int maxlen, int type);		
extern int librtnl_addattr32(struct ipstack_nlmsghdr *n, size_t maxlen, zpl_uint32 type, zpl_uint32 data);
extern int librtnl_addattr8(struct ipstack_nlmsghdr *n, int maxlen, int type, zpl_uint8 data);
extern int librtnl_addattr16(struct ipstack_nlmsghdr *n, int maxlen, int type, zpl_uint16 data);
extern int librtnl_addattr64(struct ipstack_nlmsghdr *n, int maxlen, int type, zpl_uint64 data);
extern int librtnl_addattrstrz(struct ipstack_nlmsghdr *n, int maxlen, int type, const char *str);

extern struct ipstack_rtattr *librtnl_addattr_nest(struct ipstack_nlmsghdr *n, size_t maxlen, zpl_uint32 type);
extern int librtnl_addattr_nest_end(struct ipstack_nlmsghdr *n, struct ipstack_rtattr *nest);



extern void librtnl_parse_rtattr(struct ipstack_rtattr **tb, zpl_uint32 max, struct ipstack_rtattr *rta, zpl_uint32 len);
extern void librtnl_interface_update_hw_addr(struct ipstack_rtattr **tb, struct interface *ifp);


extern void librtnl_set_ifindex(struct interface *ifp, ifindex_t ifi_index);

extern int librtnl_socket(struct nlsock *nl, zpl_ulong groups, vrf_id_t vrf_id);

extern int librtnl_request(struct nlsock *nl, zpl_family_t family, zpl_uint32 type);

extern int librtnl_talk(struct nlsock *nl, struct ipstack_nlmsghdr *n);
extern int librtnl_talk_answer(struct nlsock *nl, struct ipstack_nlmsghdr *n, librtnl_filter_callback *cb, void *p);

extern int librtnl_parse_info(struct nlsock *nl,
		int (*filter)(struct ipstack_sockaddr_nl *, struct ipstack_nlmsghdr *, void *),
		void *);

extern int librtnl_link_name2index(const char *name, int index);
extern int librtnl_link_updown(char *name, int up);
extern int librtnl_link_destroy(char *name);

#endif /* ZPL_KERNEL_NETLINK */
#ifdef ZPL_LIBNL_MODULE
extern int libnl_netlink_link_updown(struct nlsock *nl, char *name, int up);
extern int libnl_netlink_link_del(struct nlsock *nl, char *name);
extern int libnl_netlink_link_add(struct nlsock *nl, void *data, int flag);
#endif

extern int librtnl_open(vrf_id_t vrfid, zpl_uint32 msgsize);
extern void librtnl_close(void);






#ifdef __cplusplus
}
#endif

#endif /* __LINUX_NETLINK_H__ */
