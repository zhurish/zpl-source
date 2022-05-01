/* Header file exported by rt_netlink.c to zebra.
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

#ifdef HAVE_NETLINK


#ifdef ZPL_KERNEL_NETLINK
/* Socket interface to kernel */
struct nlsock
{
	vrf_id_t vrf_id;
  zpl_socket_t sock;
  zpl_uint32 seq;
  struct ipstack_sockaddr_nl snl;
  const char *name;

  struct thread *t_netlink;

};
extern struct nlsock netlink_cmd;
#endif

#define NL_PKT_BUF_SIZE 8192
#define NL_DEFAULT_ROUTE_METRIC 20

#define NLMSG_TAIL(nmsg) \
	((struct ipstack_rtattr *) (((void *) (nmsg)) + IPSTACK_NLMSG_ALIGN((nmsg)->nlmsg_len)))

/*
 * linux_netlink.c
 */
extern const char * _netlink_msg_type_to_str(zpl_uint16 msg_type);
extern const char * _netlink_rtproto_to_str(zpl_uchar rtproto);

extern int _netlink_addattr_l(struct ipstack_nlmsghdr *n, size_t maxlen, zpl_uint32 type, void *data,
		size_t alen);
extern int _netlink_rta_addattr_l(struct ipstack_rtattr *rta, size_t maxlen, zpl_uint32 type,
		void *data, size_t alen);
extern int _netlink_addattr32(struct ipstack_nlmsghdr *n, size_t maxlen, zpl_uint32 type, zpl_uint32 data);

extern struct ipstack_rtattr *_netlink_addattr_nest(struct ipstack_nlmsghdr *n, size_t maxlen, zpl_uint32 type);
extern int _netlink_addattr_nest_end(struct ipstack_nlmsghdr *n, struct ipstack_rtattr *nest);



extern void _netlink_parse_rtattr(struct ipstack_rtattr **tb, zpl_uint32 max, struct ipstack_rtattr *rta, zpl_uint32 len);
extern void _netlink_interface_update_hw_addr(struct ipstack_rtattr **tb, struct interface *ifp);


extern void _netlink_route_debug(zpl_uint32 cmd, zpl_uint8 family, zpl_uint32 bytelen,
		hal_nexthop_t *nexthop, union g_addr address,
		vrf_id_t vrfid);


extern void _netlink_set_ifindex(struct interface *ifp, ifindex_t ifi_index);

extern int _netlink_socket(struct nlsock *nl, zpl_ulong groups, vrf_id_t vrf_id);

extern int _netlink_request(zpl_family_t family, zpl_uint32 type, struct nlsock *nl);

extern int _netlink_talk(struct ipstack_nlmsghdr *n, struct nlsock *nl,
		vrf_id_t vrfid);

extern int _netlink_parse_info(
		int (*filter)(struct ipstack_sockaddr_nl *, struct ipstack_nlmsghdr *, vrf_id_t),
		struct nlsock *nl, vrf_id_t vrfid);





extern void _netlink_open(vrf_id_t vrfid);
extern void _netlink_close(void);




#endif /* HAVE_NETLINK */

#ifdef __cplusplus
}
#endif

#endif /* __LINUX_NETLINK_H__ */
