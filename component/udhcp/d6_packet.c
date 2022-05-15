/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 2011 Denys Vlasenko.
 *
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */
#include "auto_include.h"
#include <zplos_include.h>
#include "module.h"
#include "log.h"
#include "nsm_include.h"
#include "dhcpd.h"
#include "dhcpc.h"
#include "dhcp_util.h"
#include "d6_common.h"
#include "dhcpd.h"
#include "checksum.h"
#include <netinet/in.h>
#include <netinet/if_ether.h>
//#include <netpacket/packet.h>

#if defined CONFIG_UDHCP_DEBUG && CONFIG_UDHCP_DEBUG >= 2
void FAST_FUNC d6_dump_packet(struct d6_packet *packet)
{
	if (dhcp_verbose < 2)
		return;

	zlog_err(MODULE_DHCP,
		" xid %x"
		, packet->d6_xid32
	);
	//*bin2hex(buf, (void *) packet->chaddr, sizeof(packet->chaddr)) = '\0';
	//bb_error_msg(" chaddr %s", buf);
}
#endif

int FAST_FUNC d6_recv_kernel_packet(struct ipstack_in6_addr *peer_ipv6
	UNUSED_PARAM
	, struct d6_packet *packet, zpl_socket_t fd)
{
	int bytes;

	memset(packet, 0, sizeof(*packet));
	bytes = safe_read(fd, packet, sizeof(*packet));
	if (bytes < 0) {
		zlog_err(MODULE_DHCP,"packet read error, ignoring");
		return bytes; /* returns -1 */
	}

	if (bytes < offsetof(struct d6_packet, d6_options)) {
		zlog_err(MODULE_DHCP,"packet with bad magic, ignoring");
		return -2;
	}
	zlog_err(MODULE_DHCP,"received %s", "a packet");
	d6_dump_packet(packet);

	return bytes;
}

/* Construct a ipv6+udp header for a packet, send packet */
int FAST_FUNC d6_send_raw_packet(
		struct d6_packet *d6_pkt, unsigned d6_pkt_size,
		struct ipstack_in6_addr *src_ipv6, zpl_uint16 source_port,
		struct ipstack_in6_addr *dst_ipv6, zpl_uint16 dest_port, const zpl_uint8 *dest_arp,
		ifindex_t ifindex)
{
	struct ipstack_sockaddr_ll dest_sll;
	struct ip6_udp_d6_packet packet;
	zpl_socket_t fd;
	int result = -1;
	const char *msg;

	fd = ipstack_socket(IPCOM_STACK, IPSTACK_PF_PACKET, IPSTACK_SOCK_DGRAM, htons(IPSTACK_ETH_P_IPV6));
	if (fd._fd < 0) {
		msg = "socket(%s)";
		goto ret_msg;
	}

	memset(&dest_sll, 0, sizeof(dest_sll));
	memset(&packet, 0, offsetof(struct ip6_udp_d6_packet, data));
	packet.data = *d6_pkt; /* struct copy */

	dest_sll.sll_family = IPSTACK_AF_PACKET;
	dest_sll.sll_protocol = htons(IPSTACK_ETH_P_IPV6);
	dest_sll.sll_ifindex = ifindex;
	/*dest_sll.sll_hatype = ARPHRD_???;*/
	/*dest_sll.sll_pkttype = PACKET_???;*/
	dest_sll.sll_halen = 6;
	memcpy(dest_sll.sll_addr, dest_arp, 6);

	if (ipstack_bind(fd, (struct ipstack_sockaddr *)&dest_sll, sizeof(dest_sll)) < 0) {
		msg = "bind(%s)";
		goto ret_close;
	}

	packet.ip6.ip6_vfc = (6 << 4); /* 4 bits version, top 4 bits of tclass */
	if (src_ipv6)
		packet.ip6.ip6_src = *src_ipv6; /* struct copy */
	packet.ip6.ip6_dst = *dst_ipv6; /* struct copy */
	packet.udp.source = htons(source_port);
	packet.udp.dest = htons(dest_port);
	/* size, excluding IP header: */
	packet.udp.len = htons(sizeof(struct ipstack_udphdr) + d6_pkt_size);
	packet.ip6.ip6_plen = packet.udp.len;
	/*
	 * Someone was smoking weed (at least) while inventing UDP checksumming:
	 * UDP checksum skips first four bytes of IPv6 header.
	 * 'next header' field should be summed as if it is one more byte
	 * to the right, therefore we write its value (IPSTACK_IPPROTO_UDP)
	 * into ip6_hlim, and its 'real' location remains zero-filled for now.
	 */
	packet.ip6.ip6_hlim = IPSTACK_IPPROTO_UDP;
	packet.udp.check = in_cksum(
				(zpl_uint16 *)&packet + 2,
				offsetof(struct ip6_udp_d6_packet, data) - 4 + d6_pkt_size
	);
	/* fix 'hop limit' and 'next header' after UDP checksumming */
	packet.ip6.ip6_hlim = 1; /* observed Windows machines to use hlim=1 */
	packet.ip6.ip6_nxt = IPSTACK_IPPROTO_UDP;

	d6_dump_packet(d6_pkt);
	result = ipstack_sendto(fd, &packet, offsetof(struct ip6_udp_d6_packet, data) + d6_pkt_size,
			/*flags:*/ 0,
			(struct ipstack_sockaddr *) &dest_sll, sizeof(dest_sll)
	);
	msg = "sendto %s";
 ret_close:
	ipstack_close(fd);
	if (result < 0) {
 ret_msg:
 zlog_err(MODULE_DHCP,msg, "PACKET");
	}
	return result;
}

/* Let the kernel do all the work for packet generation */
int FAST_FUNC d6_send_kernel_packet(
		struct d6_packet *d6_pkt, unsigned d6_pkt_size,
		struct ipstack_in6_addr *src_ipv6, zpl_uint16 source_port,
		struct ipstack_in6_addr *dst_ipv6, zpl_uint16 dest_port,
		ifindex_t ifindex)
{
	struct ipstack_sockaddr_in6 sa;
	zpl_socket_t fd;
	int result = -1;
	const char *msg;

	fd = ipstack_socket(IPCOM_STACK, IPSTACK_PF_INET6, IPSTACK_SOCK_DGRAM, IPSTACK_IPPROTO_UDP);
	if (fd._fd < 0) {
		msg = "socket(%s)";
		goto ret_msg;
	}
	//setsockopt_reuseaddr(fd);

	memset(&sa, 0, sizeof(sa));
	sa.sin6_family = IPSTACK_AF_INET6;
	sa.sin6_port = htons(source_port);
	sa.sin6_addr = *src_ipv6; /* struct copy */
	if (ipstack_bind(fd, (struct ipstack_sockaddr *)&sa, sizeof(sa)) == -1) {
		msg = "bind(%s)";
		goto ret_close;
	}

	memset(&sa, 0, sizeof(sa));
	sa.sin6_family = IPSTACK_AF_INET6;
	sa.sin6_port = htons(dest_port);
	sa.sin6_addr = *dst_ipv6; /* struct copy */
	sa.sin6_scope_id = ifindex;
	if (ipstack_connect(fd, (struct ipstack_sockaddr *)&sa, sizeof(sa)) == -1) {
		msg = "connect %s";
		goto ret_close;
	}

	d6_dump_packet(d6_pkt);
	result = safe_write(fd, d6_pkt, d6_pkt_size);
	msg = "write %s";
 ret_close:
	ipstack_close(fd);
	if (result < 0) {
 ret_msg:
 zlog_err(MODULE_DHCP,msg, "UDP");
	}
	return result;
}
