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

#ifndef _ZEBRA_RT_NETLINK_H
#define _ZEBRA_RT_NETLINK_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_NETLINK

#define NL_PKT_BUF_SIZE 8192
#define NL_DEFAULT_ROUTE_METRIC 20

#define NLMSG_TAIL(nmsg) \
	((struct rtattr *) (((void *) (nmsg)) + NLMSG_ALIGN((nmsg)->nlmsg_len)))

/*
 * kernel_netlink.c
 */
extern const char * nl_msg_type_to_str(ospl_uint16 msg_type);
extern const char * nl_rtproto_to_str(ospl_uchar rtproto);

extern int addattr_l(struct nlmsghdr *n, size_t maxlen, ospl_uint32 type, void *data,
		size_t alen);
extern int rta_addattr_l(struct rtattr *rta, size_t maxlen, ospl_uint32 type,
		void *data, size_t alen);
extern int addattr32(struct nlmsghdr *n, size_t maxlen, ospl_uint32 type, ospl_uint32 data);

extern struct rtattr *addattr_nest(struct nlmsghdr *n, size_t maxlen, ospl_uint32 type);
extern int addattr_nest_end(struct nlmsghdr *n, struct rtattr *nest);



extern void netlink_parse_rtattr(struct rtattr **tb, ospl_uint32 max, struct rtattr *rta, ospl_uint32 len);
extern void netlink_interface_update_hw_addr(struct rtattr **tb, struct interface *ifp);


extern void _netlink_route_debug(ospl_uint32 cmd, struct prefix *p,
		struct nexthop *nexthop, const char *routedesc, ospl_family_t family,
		struct nsm_vrf *zvrf);


extern void set_ifindex(struct interface *ifp, ifindex_t ifi_index);

extern int netlink_socket(struct nlsock *nl, ospl_ulong groups, vrf_id_t vrf_id);

extern int netlink_request(ospl_family_t family, ospl_uint32 type, struct nlsock *nl);

extern int netlink_talk(struct nlmsghdr *n, struct nlsock *nl,
		struct nsm_vrf *zvrf);

extern int netlink_parse_info(
		int (*filter)(struct sockaddr_nl *, struct nlmsghdr *, vrf_id_t),
		struct nlsock *nl, struct nsm_vrf *zvrf);


extern void kernel_open(struct nsm_vrf *zvrf);
extern void kernel_close(struct nsm_vrf *zvrf);
extern void kernel_load_all();


/*
 * kernel_nllisten.c
 */
extern int kernel_nllisten(struct nsm_vrf *zvrf);

/*
 * kernel_nliface.c
 */
extern int kernel_create_interface(struct interface *ifp);
extern int kernel_destroy_interface(struct interface *ifp);

/*
 * kernel_nladdress.c
 */
extern int kernel_address_add_ipv4 (struct interface *ifp, struct connected *ifc);
extern int kernel_address_delete_ipv4 (struct interface *ifp, struct connected *ifc);

/*
 * kernel_nlroute.c
 */
extern int kernel_route_rib (struct prefix *p, struct rib *old, struct rib *new);

/*
 * kernel_nlload.c
 */
extern int kernel_route_table_load(struct nsm_vrf *zvrf);
extern int kernel_interface_load(struct nsm_vrf *zvrf);


#endif /* HAVE_NETLINK */

#ifdef __cplusplus
}
#endif

#endif /* _ZEBRA_RT_NETLINK_H */
