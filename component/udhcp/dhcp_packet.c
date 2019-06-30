/* vi: set sw=4 ts=4: */
/*
 * Packet ops
 *
 * Rewrite by Russ Dill <Russ.Dill@asu.edu> July 2001
 *
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */
#include "dhcp_def.h"
#include "dhcp_util.h"
#include "dhcpd.h"
/*
#include <netinet/in.h>
#include <netinet/if_ether.h>
*/
//#include <netpacket/packet.h>

#if ENABLE_UDHCPC || ENABLE_UDHCPD
void FAST_FUNC udhcp_header_init(struct dhcp_packet *packet, char type)
{
	memset(packet, 0, sizeof(*packet));
	packet->op = BOOTREQUEST; /* if client to a server */
	switch (type) {
	case DHCPOFFER:
	case DHCPACK:
	case DHCPNAK:
		packet->op = BOOTREPLY; /* if server to client */
	}
	packet->htype = 1; /* ethernet */
	packet->hlen = 6;
	packet->cookie = htonl(DHCP_MAGIC);
	if (DHCP_END != 0)
		packet->options[0] = DHCP_END;

	dhcp_option_message_type(packet->options, type);
	//udhcp_add_simple_option(packet, DHCP_MESSAGE_TYPE, type);
}
#endif

static void _udhcp_dump_packet(struct dhcp_packet *packet)
{
	char buf[sizeof(packet->chaddr)*2 + 1];
	zlog_err(ZLOG_DHCP,
		//" op %x"
		//" htype %x"
		" hlen %x"
		//" hops %x"
		" xid %x"
		//" secs %x"
		//" flags %x"
		" ciaddr %x"
		" yiaddr %x"
		" siaddr %x"
		" giaddr %x"
		//" sname %s"
		//" file %s"
		//" cookie %x"
		//" options %s"
		//, packet->op
		//, packet->htype
		, packet->hlen
		//, packet->hops
		, packet->xid
		//, packet->secs
		//, packet->flags
		, packet->ciaddr
		, packet->yiaddr
		, packet->siaddr_nip
		, packet->gateway_nip
		//, packet->sname[64]
		//, packet->file[128]
		//, packet->cookie
		//, packet->options[]
	);
	bin2hex(buf, (void *) packet->chaddr, sizeof(packet->chaddr));
	zlog_err(ZLOG_DHCP," chaddr %s", buf);
}


/* Read a packet from socket fd, return -1 on read error, -2 on packet error */
int FAST_FUNC udhcp_recv_packet(struct dhcp_packet *packet, int fd, u_int32 *ifindex)
{
#if 1
	int bytes = 0;

	memset(packet, 0, sizeof(*packet));
	struct iovec iov;
	/* Header and data both require alignment. */
	char buff[CMSG_SPACE(SOPT_SIZE_CMSG_IFINDEX_IPV4())];
	struct msghdr msgh;
	struct sockaddr_storage	 ss;
	struct sockaddr_in	*sin4 = NULL;
	memset(&msgh, 0, sizeof(struct msghdr));
	msgh.msg_name = &ss;
	msgh.msg_namelen = sizeof(ss);
	msgh.msg_iov = &iov;
	msgh.msg_iovlen = 1;
	msgh.msg_control = (caddr_t) buff;
	msgh.msg_controllen = sizeof(buff);
	iov.iov_base = packet;
	iov.iov_len = sizeof(struct dhcp_packet);
	bytes = recvmsg(fd, &msgh, 0);//MSG_PEEK | MSG_TRUNC
	if (bytes < 0)
	{
		zlog_err(ZLOG_DHCP, "packet read error, ignoring");
		return bytes; /* returns -1 */
	}
	if (ss.ss_family != AF_INET) {
		zlog_err(ZLOG_DHCP, "received DHCP message is not AF_INET");
		return ERROR;
	}
	sin4 = (struct sockaddr_in *)&ss;
	//memcpy(&from.iabuf, &sin4->sin_addr, from.len);

	zlog_err(ZLOG_DHCP, "-------------%s", inet_ntoa(sin4->sin_addr));
	if (ifindex)
		*ifindex = getsockopt_ifindex(AF_INET, &msgh);

	if (bytes < offsetof(struct dhcp_packet, options)
			|| packet->cookie != htonl(DHCP_MAGIC))
	{
		zlog_err(ZLOG_DHCP, "packet with bad magic, ignoring");
		return -2;
	}
	zlog_err(ZLOG_DHCP, "===================received %s ifindex=0x%x", "a packet", *ifindex);
	_udhcp_dump_packet(packet);

	return bytes;
#else
	int bytes;

	memset(packet, 0, sizeof(*packet));
	bytes = safe_read(fd, packet, sizeof(*packet));
	if (bytes < 0)
	{
		zlog_err(ZLOG_DHCP,"packet read error, ignoring");
		return bytes; /* returns -1 */
	}

	if (bytes < offsetof(struct dhcp_packet, options)
			|| packet->cookie != htonl(DHCP_MAGIC)
	)
	{
		zlog_err(ZLOG_DHCP,"packet with bad magic, ignoring");
		return -2;
	}
	zlog_err(ZLOG_DHCP,"received %s", "a packet");
	_udhcp_dump_packet(packet);

	return bytes;
#endif
}

/* Construct a ip/udp header for a packet, send packet */
int FAST_FUNC udhcp_send_raw_packet(int fd, struct dhcp_packet *dhcp_pkt,
	struct udhcp_packet_cmd *source,
	struct udhcp_packet_cmd *dest, const uint8_t *dest_arp,
		int ifindex)
{
	struct sockaddr_ll dest_sll;
	struct ip_udp_dhcp_packet packet;
	unsigned padding;
	int result = -1;

	memset(&dest_sll, 0, sizeof(dest_sll));
	memset(&packet, 0, offsetof(struct ip_udp_dhcp_packet, data));
	packet.data = *dhcp_pkt; /* struct copy */

	dest_sll.sll_family = AF_PACKET;
	dest_sll.sll_protocol = htons(ETH_P_IP);
	dest_sll.sll_ifindex = ifindex2ifkernel(ifindex);
	/*dest_sll.sll_hatype = ARPHRD_???;*/
	/*dest_sll.sll_pkttype = PACKET_???;*/
	dest_sll.sll_halen = 6;
	memcpy(dest_sll.sll_addr, dest_arp, 6);

	/* We were sending full-sized DHCP packets (zero padded),
	 * but some badly configured servers were seen dropping them.
	 * Apparently they drop all DHCP packets >576 *ethernet* octets big,
	 * whereas they may only drop packets >576 *IP* octets big
	 * (which for typical Ethernet II means 590 octets: 6+6+2 + 576).
	 *
	 * In order to work with those buggy servers,
	 * we truncate packets after end option byte.
	 *
	 * However, RFC 1542 says "The IP Total Length and UDP Length
	 * must be large enough to contain the minimal BOOTP header of 300 octets".
	 * Thus, we retain enough padding to not go below 300 BOOTP bytes.
	 * Some devices have filters which drop DHCP packets shorter than that.
	 */
	padding = DHCP_OPTIONS_BUFSIZE - 1 - udhcp_end_option(packet.data.options);
	if (padding > DHCP_SIZE - 300)
		padding = DHCP_SIZE - 300;

	packet.ip.protocol = IPPROTO_UDP;
	packet.ip.saddr = source->ip;
	packet.ip.daddr = dest->ip;
	packet.udp.source = htons(source->port);
	packet.udp.dest = htons(dest->port);
	/* size, excluding IP header: */
	packet.udp.len = htons(UDP_DHCP_SIZE - padding);
	/* for UDP checksumming, ip.len is set to UDP packet len */
	packet.ip.tot_len = packet.udp.len;
	packet.udp.check = in_cksum/*inet_cksum*/((uint16_t *)&packet,
			IP_UDP_DHCP_SIZE - padding);
	/* but for sending, it is set to IP packet len */
	packet.ip.tot_len = htons(IP_UDP_DHCP_SIZE - padding);
	packet.ip.ihl = sizeof(packet.ip) >> 2;
	packet.ip.version = IPVERSION;
	packet.ip.ttl = IPDEFTTL;
	packet.ip.check = in_cksum/*inet_cksum*/((uint16_t *)&packet.ip, sizeof(packet.ip));

	_udhcp_dump_packet(dhcp_pkt);
	result = sendto(fd, &packet, IP_UDP_DHCP_SIZE - padding, /*flags:*/ 0,
			(struct sockaddr *) &dest_sll, sizeof(dest_sll));

	zlog_err(ZLOG_DHCP, "sendto %s", "PACKET");
	return result;
}

/* Let the kernel do all the work for packet generation */
int FAST_FUNC udhcp_send_udp_packet(int fd, struct dhcp_packet *dhcp_pkt,
	struct udhcp_packet_cmd *source,
	struct udhcp_packet_cmd *dest)
{
	struct sockaddr_in sa;
	unsigned padding;
	int result = -1;

	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(dest->port);
	sa.sin_addr.s_addr = dest->ip;

	_udhcp_dump_packet(dhcp_pkt);
	padding = DHCP_OPTIONS_BUFSIZE - 1 - udhcp_end_option(dhcp_pkt->options);
	if (padding > DHCP_SIZE - 300)
		padding = DHCP_SIZE - 300;

	result = sendto(fd, dhcp_pkt, DHCP_SIZE - padding, 0, (struct sockaddr *)&sa,
		sizeof(struct sockaddr_in));

	//result = safe_write(fd, dhcp_pkt, DHCP_SIZE - padding);
	zlog_err(ZLOG_DHCP, "sendto %s", "UDP");

	return result;
}
