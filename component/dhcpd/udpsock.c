/*	$OpenBSD: udpsock.c,v 1.7 2016/04/27 10:16:10 mestre Exp $	*/

/*
 * Copyright (c) 2014 YASUOKA Masahiko <yasuoka@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <net/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netinet/udp.h>

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dhcp.h"
#include "tree.h"
#include "dhcpd.h"



static void	 udpsock_handler (struct protocol *);
static ssize_t	 udpsock_send_packet(struct interface_info *, struct dhcp_packet *,
    size_t, struct in_addr, struct sockaddr_in *, struct hardware *);

static int udpsock_send_socket(void);
static int udpsock_send_raw_packet(struct interface_info *interface,
		struct dhcp_packet *raw,
	    size_t len, struct in_addr from, struct sockaddr_in *to,
	    struct hardware *hto);


struct udpsock {
	int	 sock;
	int	 rawsock;
};

void
udpsock_startup(struct in_addr bindaddr)
{
	int			 sock, onoff;
	struct sockaddr_in	 sin4;
	struct udpsock		*udpsock = NULL;

	//struct sockaddr aaa;

#if 0//def SO_BINDTODEVICE
    struct ifreq ifr;
#endif
	if ((udpsock = calloc(1, sizeof(struct udpsock))) == NULL)
	{
		dhcpd_error("could not create udpsock: %s", strerror(errno));
		return;
	}
	memset(udpsock, 0, sizeof(struct udpsock));
	memset(&sin4, 0, sizeof(sin4));
	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		dhcpd_error("creating a socket failed for udp: %s", strerror(errno));
		free(udpsock);
		return;
	}
#ifdef IP_RECVIF
	onoff = 1;
	if (setsockopt(sock, IPPROTO_IP, IP_RECVIF, &onoff, sizeof(onoff)) != 0)
		dhcpd_error("setsocketopt IP_RECVIF failed for udp: %s",
		    strerror(errno));
#endif
#if defined (IP_PKTINFO)
	onoff = 1;
	if (setsockopt (sock, IPPROTO_IP, IP_PKTINFO, &onoff, sizeof (onoff)) != 0)
		dhcpd_error("setsocketopt IP_PKTINFO failed for udp: %s",
		    strerror(errno));
#endif

#if 0//def SO_BINDTODEVICE
    strncpy(ifr.ifr_name, "ppp1", IFNAMSIZ);
    if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, (char *)&ifr, sizeof(ifr)) < 0)
		dhcpd_error("setsocketopt SO_BINDTODEVICE failed for udp: %s",
		    strerror(errno));
#endif
    onoff = 1;
	if(setsockopt(sock,
                        SOL_SOCKET,
                        SO_REUSEADDR,
                        &onoff,
                        sizeof(onoff)) == -1)
	{
		dhcpd_error("setsocketopt SO_REUSEADDR failed for udp: %s",
		    strerror(errno));
	}

	onoff = 1;
    if(setsockopt(sock,
                        SOL_SOCKET,
                        SO_REUSEPORT,
                        &onoff,
                        sizeof(onoff)) == -1)
    {
		dhcpd_error("setsocketopt SO_REUSEPORT failed for udp: %s",
		    strerror(errno));
    }
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *) &onoff, sizeof(onoff));

	sin4.sin_family = AF_INET;
#ifdef HAVE_SA_LEN	
	sin4.sin_len = sizeof(sin4);
#endif
	sin4.sin_addr = bindaddr;
	sin4.sin_port = root_group.server_port;

	if (bind(sock, (struct sockaddr *)&sin4, sizeof(sin4)) != 0)
		dhcpd_error("bind failed for udp: %s", strerror(errno));

	udpsock->sock = sock;
	add_protocol("udp", sock, udpsock_handler, (void *)(intptr_t)udpsock);
	dhcpd_note("Listening(fd=%d) on %s:%d/udp.", udpsock->sock, inet_ntoa(sin4.sin_addr),
	    ntohs(root_group.server_port));


}

static void udpsock_handler(struct protocol *protocol)
{
	int			 sockio;
	/* Header and data both require alignment. */
	//char cbuf [CMSG_SPACE(SOPT_SIZE_CMSG_IFINDEX_IPV4())];
	char cbuf [256];
	ssize_t			 len;
	struct udpsock		*udpsock = protocol->local;
	struct msghdr		 m;
	struct cmsghdr		*cm = NULL;
	struct iovec		 iov[1];
	struct sockaddr_storage	 ss;
	struct sockaddr_in	*sin4 = NULL;
#if defined(IP_PKTINFO)
	struct in_pktinfo *pktinfo = NULL;
#endif
#ifdef IP_RECVIF
	struct sockaddr_dl	*sdl = NULL;
#endif
	struct interface_info	 *iface = NULL;
	struct iaddr		 from;
	unsigned char		 packetbuf[4095];
	struct dhcp_packet	*packet = (struct dhcp_packet *)packetbuf;
	struct hardware		 hw;
	//struct subnet		*subnet;
	int ifindex = 0;


	if(!dhcpd_service_is_enable() == 0)
		return;

	memset(&hw, 0, sizeof(hw));
	memset(&ss, 0, sizeof(ss));

	iov[0].iov_base = packetbuf;
	iov[0].iov_len = sizeof(packetbuf);
	memset(&m, 0, sizeof(m));
	m.msg_name = &ss;
	m.msg_namelen = sizeof(ss);
	m.msg_iov = iov;
	m.msg_iovlen = 1;
	m.msg_control = cbuf;
	m.msg_controllen = sizeof(cbuf);

	if ((len = recvmsg(udpsock->sock, &m, 0)) < 0) {
		dhcpd_warning("receiving a DHCP message failed: %s", strerror(errno));
		return;
	}
	if (ss.ss_family != AF_INET) {
		dhcpd_warning("received DHCP message is not AF_INET");
		return;
	}
	for (cm = (struct cmsghdr *)CMSG_FIRSTHDR(&m);
	    m.msg_controllen != 0 && cm;
	    cm = (struct cmsghdr *)CMSG_NXTHDR(&m, cm))
	{
#ifdef IP_RECVIF
		if (cm->cmsg_level == IPPROTO_IP &&
		    cm->cmsg_type == IP_RECVIF)
		{
			sdl = (struct sockaddr_dl *)CMSG_DATA(cm);
			if (sdl == NULL) {
				dhcpd_warning("could not get the received interface by IP_RECVIF");
				return;
			}
			ifindex = sdl->sdl_index;
		}
#endif

#if defined(IP_PKTINFO)
		if (cm->cmsg_level == IPPROTO_IP &&
			cm->cmsg_type == IP_PKTINFO)
		{
			pktinfo = (struct in_pktinfo *)CMSG_DATA(cm);
			if (pktinfo == NULL) {
				dhcpd_warning("could not get the received interface by IP_PKTINFO");
				return;
			}
			ifindex = pktinfo->ipi_ifindex;
		}
#endif
	}
	sin4 = (struct sockaddr_in *)&ss;
	if(!ifindex)
	{
		//dhcpd_warning("could not get the received interface");
		return;
	}
	iface = dhcpd_interface_lookup(ifindex);
	if(!iface)
	{
		//dhcpd_warning("could not get the interface witch enable dhcps");
		return;
	}
	iface->is_udpsock = 1;
	iface->send_packet = udpsock_send_packet;
	if(packet->giaddr.s_addr)
	{
		iface->relay_address = 1;
		iface->wfdesc = udpsock->sock;
	}
	else
	{
		iface->relay_address = 0;
		if(udpsock->rawsock == 0)
			udpsock->rawsock = udpsock_send_socket();
		iface->wfdesc = udpsock->rawsock;
	}

	//dhcpd_note("recving DHCP message on %s", iface->name);
//	iface.ifp = &ifr;

/*	strlcpy(iface.name, ifname, sizeof(iface.name));

	addr.len = 4;
	memcpy(&addr.iabuf, &iface.primary_address, addr.len);

	if ((subnet = find_subnet(addr)) == NULL)
		return;
	iface.shared_network = subnet->shared_network ;
	*/
	from.len = 4;
	memcpy(&from.iabuf, &sin4->sin_addr, from.len);
	do_packet(iface, packet, len, sin4->sin_port, from, &hw);
}


static ssize_t udpsock_send_packet(struct interface_info *interface, struct dhcp_packet *raw,
    size_t len, struct in_addr from, struct sockaddr_in *to,
    struct hardware *hto)
{
	//dhcpd_warning("%s: send to client on %s", __func__, interface->name);
	if(interface->relay_address && interface->wfdesc)
		return (sendto(interface->wfdesc, raw, len, 0, (struct sockaddr *)to,
				sizeof(struct sockaddr_in)));
	if(interface->wfdesc)
		return udpsock_send_raw_packet(interface, raw, len, from, to,  hto);
	return -1;
}

static int udpsock_send_socket(void)
{
	int fd = 0, onoff = 1;
	/* Get packet socket to write raw frames on */
	if((fd = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_IP))) < 0)
	{
		dhcpd_error("failed to open raw udp for relay(%s)", strerror(errno));
		return (-1);
	}
    setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (char *) &onoff, sizeof(onoff));
	return fd;
}

static int udpsock_send_raw_packet(struct interface_info *interface,
		struct dhcp_packet *raw,
	    size_t len, struct in_addr from, struct sockaddr_in *to,
	    struct hardware *hto)
{
	char buf[1536];
	struct iphdr *iph = (struct iphdr *)buf;
	struct udphdr *udph = (struct udphdr *)(buf + sizeof(struct iphdr));
	struct sockaddr_ll dest;
	/* Setup destination ll address */
	memset(&dest, 0, sizeof(dest));
	memset(buf, 0, sizeof(buf));

	dest.sll_family   = AF_PACKET;
	dest.sll_protocol = htons(ETH_P_IP);
	dest.sll_ifindex  = interface->kifindex;
	dest.sll_pkttype = PACKET_BROADCAST;

	if(hto)
	{
		//dhcpd_debug(" %02x:%02x:%02x hlen = %d", hto->haddr[0], hto->haddr[1], hto->haddr[2], hto->hlen);
		if(!hto->hlen)
			hto->hlen = 6;
		dest.sll_halen    = hto->hlen;
		if(memcmp(hto->haddr, buf, hto->hlen) == 0)
			memset(dest.sll_addr, 0xff, hto->hlen);
		else
			memcpy(dest.sll_addr, hto->haddr, hto->hlen);

		memset(dest.sll_addr, 0xff, hto->hlen);
	}
	else
	{
		dhcpd_debug(" hardware is null, default broadcast");
		dest.sll_halen    = 6;
		memset(dest.sll_addr, 0xff, 6);
	}

	memcpy(buf + sizeof(struct iphdr) + sizeof(struct udphdr), raw, len);
	/* Clear IP and UDP headers */
	iph->ihl = 5;
	iph->version = IPVERSION;
	iph->tos = 0;//IPTOS_LOWDELAY;
	//iph->tot_len;
	iph->id = htons(getpid());
	iph->frag_off = htons(IP_DF);
	iph->ttl = IPTTLDEC + 6;
	iph->protocol = IPPROTO_UDP;
	iph->check = 0;
	iph->saddr = from.s_addr;//interface->primary_address.s_addr;

//	dhcpd_debug("send buffer on %s", inet_ntoa(from));

	if(raw->flags)
		iph->daddr = INADDR_BROADCAST;
	else
	{
		if(raw->ciaddr.s_addr != 0)
		{
			/* Unicast packet to clients ciaddr */
			iph->daddr = raw->ciaddr.s_addr;
			//chaddr = handle->packet->hdr.chaddr;
		}
		else
		{
			/* Unicast packet to clients yiaddr */
			iph->daddr = raw->yiaddr.s_addr;
			//chaddr = handle->packet->hdr.chaddr;
		}
	}
	iph->daddr = to->sin_addr.s_addr;//INADDR_BROADCAST;
	iph->daddr = INADDR_BROADCAST;
	iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + len);

	udph->source = (root_group.server_port);
	udph->dest = to->sin_port;//htons(root_group.client_port);
	udph->len = htons(sizeof(struct udphdr) + len);
	udph->check = 0;

	/* Calculate UDP checksum (pseudo header+header+payload) */
	udph->check  = in_cksum((unsigned short*)buf, sizeof(struct iphdr) +
			sizeof(struct udphdr) + len );

	iph->check   = in_cksum((unsigned short*)buf, sizeof(struct iphdr));

	/* Transmit packet */
	if(interface->wfdesc)
		return sendto(interface->wfdesc,
						 buf, (int)ntohs(iph->tot_len), 0,
						  (struct sockaddr*) &dest,
						  sizeof(struct sockaddr_ll));
	return -1;
}
