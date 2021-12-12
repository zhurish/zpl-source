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
	memset(packet, 0, sizeof(struct dhcp_packet));
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
static void dhcp_packet_dump(struct dhcp_packet *packet)
{
	char buf[sizeof(packet->chaddr)*2 + 1];
	//char tmp[128];
	int len = 0;
	int message = 0;
	if(packet->op == BOOTREQUEST)
		zlog_debug(MODULE_DHCP, " DHCP Message Type : Request");
	else if(packet->op == BOOTREPLY)
		zlog_debug(MODULE_DHCP, " DHCP Message Type : Reply");

	if(packet->htype == HTYPE_ETHER)
	{
		zlog_debug(MODULE_DHCP, "  Hardware Type : Ethernet");
		zlog_debug(MODULE_DHCP, "   Hardware address length : %d", packet->hlen);
	}
	else if(packet->htype == HTYPE_IPSEC_TUNNEL)
	{
		zlog_debug(MODULE_DHCP, "  Hardware Type : IPsec Tunnel");
		zlog_debug(MODULE_DHCP, "   Hardware address length : %d", packet->hlen);
	}
	zlog_debug(MODULE_DHCP, "  Transaction ID : 0x%x", ntohl(packet->xid));

	if(ntohs(packet->flags) == BROADCAST_FLAG)
		zlog_debug(MODULE_DHCP, "  Message Type : Request");
	else if(ntohs(packet->flags) == UNICAST_FLAG)
		zlog_debug(MODULE_DHCP, "  Message Type : Reply");

	zlog_debug(MODULE_DHCP, "  Client IP address : %s", inet_address(ntohl(packet->ciaddr)));
	zlog_debug(MODULE_DHCP, "  Your IP address : %s", inet_address(ntohl(packet->yiaddr)));
	zlog_debug(MODULE_DHCP, "  Next server IP address : %s", inet_address(ntohl(packet->siaddr_nip)));
	zlog_debug(MODULE_DHCP, "  Relay agent IP address : %s", inet_address(ntohl(packet->gateway_nip)));

	bin2hex(buf, (void *) packet->chaddr, sizeof(packet->chaddr));
	zlog_debug(MODULE_DHCP, "  Client Hardware address : %s", buf);

	if(strlen(packet->sname))
		zlog_debug(MODULE_DHCP, "  Server name option : %s", packet->sname);

	if(strlen(packet->file))
		zlog_debug(MODULE_DHCP, "  Boot file name option : %s", packet->file);

	if(strlen(packet->file))
		zlog_debug(MODULE_DHCP, "  Boot file name option : %s", packet->file);

	//zpl_uint8 options[DHCP_OPTIONS_BUFSIZE + CONFIG_UDHCPC_SLACK_FOR_BUGGY_SERVERS];
	len = dhcp_option_get_length(packet->options);

	message = dhcp_option_message_type_get(packet->options, len);
	switch (message)
	{
	case DHCPDISCOVER:
		zlog_debug(MODULE_DHCP, "  Message Type : Discover");
		break;
	case DHCPOFFER:
		zlog_debug(MODULE_DHCP, "  Message Type : Offer");
		break;
	case DHCPREQUEST:
		zlog_debug(MODULE_DHCP, "  Message Type : Request");
		break;
	case DHCPDECLINE:
		zlog_debug(MODULE_DHCP, "  Message Type : Decline");
		break;
	case DHCPACK:
		zlog_debug(MODULE_DHCP, "  Message Type : ACK");
		break;
	case DHCPNAK:
		zlog_debug(MODULE_DHCP, "  Message Type : NACK");
		break;
	case DHCPRELEASE:
		zlog_debug(MODULE_DHCP, "  Message Type : Release");
		break;
	case DHCPINFORM:
		zlog_debug(MODULE_DHCP, "  Message Type : Inform");
		break;
	default:
		break;
	}


}
#if 0
static void _udhcp_dump_packet(struct dhcp_packet *packet)
{
	char buf[sizeof(packet->chaddr)*2 + 1];
	zlog_err(MODULE_DHCP,
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
	zlog_err(MODULE_DHCP," chaddr %s", buf);
}
#endif

/* Read a packet from socket fd, return -1 on read error, -2 on packet error */
int FAST_FUNC udhcp_recv_packet(struct dhcp_packet *packet, zpl_socket_t fd, zpl_uint32 *ifindex)
{
#if 1
	int bytes = 0;
	zpl_uint32 d_ifindex = 0;
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
	bytes = ipstack_recvmsg(fd, &msgh, 0);//MSG_PEEK | MSG_TRUNC
	if (bytes < 0)
	{
		zlog_err(MODULE_DHCP, "packet read error, ignoring");
		return bytes; /* returns -1 */
	}
	if (ss.ss_family != AF_INET) {
		zlog_err(MODULE_DHCP, "received DHCP message is not AF_INET");
		return ERROR;
	}
	sin4 = (struct sockaddr_in *)&ss;
	//memcpy(&from.iabuf, &sin4->sin_addr, from.len);

	//zlog_err(MODULE_DHCP, "-------------%s", ipstack_inet_ntoa(sin4->sin_addr));

	d_ifindex = getsockopt_ifindex(AF_INET, &msgh);
	if (ifindex)
		*ifindex = d_ifindex;

	if (bytes < offsetof(struct dhcp_packet, options)
			|| packet->cookie != htonl(DHCP_MAGIC))
	{
		zlog_err(MODULE_DHCP, "packet with bad magic, ignoring");
		return -2;
	}
	//zlog_err(MODULE_DHCP, "===================received %s ifindex=0x%x", "a packet", d_ifindex);
	//_udhcp_dump_packet(packet);
	if (DHCPC_DEBUG_ISON(RECV))
	{
		if(DHCPC_DEBUG_ISON(DETAIL))
		{
			zlog_debug(MODULE_DHCP," dhcp client received packet(%d byte) on interface %s", bytes, ifindex2ifname(d_ifindex));
			dhcp_packet_dump(packet);
		}
		else
			zlog_debug(MODULE_DHCP," dhcp client received packet(%d byte) on interface %s", bytes, ifindex2ifname(d_ifindex));
	}

	return bytes;
#else
	int bytes;

	memset(packet, 0, sizeof(*packet));
	bytes = safe_read(fd, packet, sizeof(*packet));
	if (bytes < 0)
	{
		zlog_err(MODULE_DHCP,"packet read error, ignoring");
		return bytes; /* returns -1 */
	}

	if (bytes < offsetof(struct dhcp_packet, options)
			|| packet->cookie != htonl(DHCP_MAGIC)
	)
	{
		zlog_err(MODULE_DHCP,"packet with bad magic, ignoring");
		return -2;
	}
	zlog_err(MODULE_DHCP,"received %s", "a packet");
	_udhcp_dump_packet(packet);

	return bytes;
#endif
}

/* Construct a ip/udp header for a packet, send packet */
int FAST_FUNC udhcp_send_raw_packet(zpl_socket_t fd, struct dhcp_packet *dhcp_pkt,
	struct udhcp_packet_cmd *source,
	struct udhcp_packet_cmd *dest, const zpl_uint8 *dest_arp,
		ifindex_t ifindex)
{
	struct sockaddr_ll dest_sll;
	struct ip_udp_dhcp_packet packet;
	unsigned padding;
	int result = -1;

	memset(&dest_sll, 0, sizeof(dest_sll));
	memset(&packet, 0, offsetof(struct ip_udp_dhcp_packet, data));
	//packet.data = *dhcp_pkt; /* struct copy */
	memcpy(&packet.data, dhcp_pkt, sizeof(struct dhcp_packet));

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
	 * Some devices have filters which drop DHCP packets zpl_int16er than that.
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
	packet.udp.check = in_cksum/*inet_cksum*/((zpl_uint16 *)&packet,
			IP_UDP_DHCP_SIZE - padding);
	/* but for sending, it is set to IP packet len */
	packet.ip.tot_len = htons(IP_UDP_DHCP_SIZE - padding);
	packet.ip.ihl = sizeof(packet.ip) >> 2;
	packet.ip.version = IPVERSION;
	packet.ip.ttl = IPDEFTTL;
	packet.ip.check = in_cksum/*inet_cksum*/((zpl_uint16 *)&packet.ip, sizeof(packet.ip));

	//_udhcp_dump_packet(dhcp_pkt);
	result = ipstack_sendto(fd, &packet, IP_UDP_DHCP_SIZE - padding, /*flags:*/ 0,
			(struct sockaddr *) &dest_sll, sizeof(dest_sll));

	if (DHCPC_DEBUG_ISON(SEND))
	{
		if(DHCPC_DEBUG_ISON(DETAIL))
		{
			zlog_debug(MODULE_DHCP," dhcp client sending raw packet(%d byte)", result);
			dhcp_packet_dump(dhcp_pkt);
		}
		else
			zlog_debug(MODULE_DHCP," dhcp client sending raw packet(%d byte)", result);
	}
	return result;
}

/* Let the kernel do all the work for packet generation */
int FAST_FUNC udhcp_send_udp_packet(zpl_socket_t fd, struct dhcp_packet *dhcp_pkt,
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

	//_udhcp_dump_packet(dhcp_pkt);
	padding = DHCP_OPTIONS_BUFSIZE - 1 - udhcp_end_option(dhcp_pkt->options);
	if (padding > DHCP_SIZE - 300)
		padding = DHCP_SIZE - 300;

	result = ipstack_sendto(fd, dhcp_pkt, DHCP_SIZE - padding, 0, (struct sockaddr *)&sa,
		sizeof(struct sockaddr_in));

	if (DHCPC_DEBUG_ISON(SEND))
	{
		if(DHCPC_DEBUG_ISON(DETAIL))
		{
			zlog_debug(MODULE_DHCP," dhcp client sending UDP packet(%d byte)", result);
			dhcp_packet_dump(dhcp_pkt);
		}
		else
			zlog_debug(MODULE_DHCP," dhcp client sending UDP packet(%d byte)", result);
	}
	//result = safe_write(fd, dhcp_pkt, DHCP_SIZE - padding);
	//zlog_err(MODULE_DHCP, "sendto UDP %d byte", DHCP_SIZE - padding);

	return result;
}
