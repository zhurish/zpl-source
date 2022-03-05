/*
 * VRF functions.
 * Copyright (C) 2014 6WIND S.A.
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"

#include "kernel_ioctl.h"
#include "kernel_driver.h"
#include "pal_driver.h"

static int kernel_packet_bindifid(zpl_socket_t fd, int ifindex)
{
	struct ipstack_sockaddr_ll sll;
	memset(&sll, 0, sizeof(sll));
	sll.sll_family = IPSTACK_AF_PACKET; // AF_PACKET;
	sll.sll_ifindex = ifindex;
	sll.sll_protocol = htons(IPSTACK_ETH_P_ALL); // htons(ETH_P_ALL);
	if (ipstack_bind(fd, (struct ipstack_sockaddr *)&sll, sizeof(sll)) == -1)
	{
		zlog_warn(MODULE_PAL, "ipstack_bind ifindex %d failed for packet: %s",
				  ifindex, strerror(ipstack_errno));
		return 0;
	}
	return 1;
}

static zpl_socket_t kernel_packet_socket_init(void)
{
	zpl_socket_t skfd;
	int onoff = 1;
	/*创建原始套接字*/
	skfd = ipstack_socket(IPCOM_STACK, IPSTACK_PF_PACKET, IPSTACK_SOCK_RAW, htons(IPSTACK_ETH_P_ALL));
	if (ipstack_invalid(skfd))
	{
		zlog_err(MODULE_PAL, "Can't open %s ipstack_socket: %s", "arp ipstack_socket",
				 strerror(ipstack_errno));
		return skfd;
	}
#ifdef IP_RECVIF
	onoff = 1;
	if (ipstack_setsockopt(skfd, IPSTACK_IPPROTO_IP, IPSTACK_IP_RECVIF, &onoff, sizeof(onoff)) != 0)
		zlog_warn(MODULE_PAL, "setsocketopt IP_RECVIF failed for packet: %s",
				  strerror(ipstack_errno));
#endif
#if defined(IP_PKTINFO)
	onoff = 1;
	if (ipstack_setsockopt(skfd, IPSTACK_IPPROTO_IP, IPSTACK_IP_PKTINFO, &onoff, sizeof(onoff)) != 0)
		zlog_warn(MODULE_PAL, "setsocketopt IP_PKTINFO failed for packet: %s",
				  strerror(ipstack_errno));
#endif
//if(onoff == 0)
	kernel_packet_bindifid(skfd, if_nametoindex("lan1"));

	return skfd;
}


static int kernel_packet_hexmsg(zpl_uint8 *buf, zpl_uint32 len, char *hdr)
{
    int ret = 0;
    zpl_char format[4096];
    memset(format, 0, sizeof(format));
    ret = os_loghex(format, sizeof(format), (const zpl_uchar *)buf,  len);
    zlog_debug(MODULE_HAL, "%s %d bytes:\r\n  hex:%s", hdr, len, format);
    return OK;
}

static int kernel_packet_handle(ifindex_t ifindex, struct ipstack_sockaddr_in *from, 
	zpl_uint8 *buf, zpl_uint32 len)
{
	kernel_packet_hexmsg(buf, min(64, len), "Kernel Recv:");
	/*
	struct interface *ifp = NULL;
	struct ipstack_ether_header *eth = NULL;
	struct ipstack_ether_arp *arp = NULL;
	eth = (struct ipstack_ether_header *)(buf);
	arp = (struct ipstack_ether_arp *)(buf + sizeof(struct ipstack_ether_header));
	ifp = if_lookup_by_kernel_index(ifindex);
	if (ntohs(arp->arp_op) == 2 && ifp)
	{
		ip_arp_t arpt;
		arpt.class = ARP_DYNAMIC;
		memcpy(&arpt.address.u.prefix4.s_addr, arp->arp_spa, arp->arp_pln);
		arpt.address.family = IPSTACK_AF_INET;
		memcpy(arpt.mac, eth->ether_shost, NSM_MAC_MAX);
		arpt.ifindex = ifindex;
		arpt.vrfid = ifp->vrf_id;
		arpt.ttl = NSM_ARP_TTL_DEFAULT;

		ip_arp_dynamic_cb(1, &arpt);
	}
	*/
	return 0;
}


static int kernel_packet_recv(zpl_socket_t sock)
{
	char cbuf[256];
	ssize_t len;
	struct ipstack_msghdr m;
	struct ipstack_cmsghdr *cm = NULL;
	struct ipstack_iovec iov[1];
	struct ipstack_sockaddr_storage ss;
	struct ipstack_sockaddr_in *sin4 = NULL;
#if defined(IP_PKTINFO)
	struct ipstack_in_pktinfo *pktinfo = NULL;
#endif
#ifdef IP_RECVIF
	struct ipstack_sockaddr_dl *sdl = NULL;
#endif
	zpl_uint8 packetbuf[2048];
	ifindex_t ifindex = 0;

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

zlog_warn(MODULE_PAL, "kernel_packet_recv");

	if ((len = ipstack_recvmsg(sock, &m, 0)) < 0)
	{
		zlog_warn(MODULE_PAL, "receiving a ARP message failed: %s", strerror(ipstack_errno));
		return -1;
	}
	zlog_warn(MODULE_PAL, "ss.ss_family %d", ss.ss_family);
	if (ss.ss_family != IPSTACK_AF_INET)
	{
		// zlog_warn(MODULE_PAL, "received ARP message is not IPSTACK_AF_INET");
		return -1;
	}
	for (cm = (struct ipstack_cmsghdr *)IPSTACK_CMSG_FIRSTHDR(&m);
		 m.msg_controllen != 0 && cm;
		 cm = (struct ipstack_cmsghdr *)IPSTACK_CMSG_NXTHDR(&m, cm))
	{
#ifdef IP_RECVIF
		if (cm->cmsg_level == IPSTACK_IPPROTO_IP &&
			cm->cmsg_type == IPSTACK_IP_RECVIF)
		{
			sdl = (struct ipstack_sockaddr_dl *)IPSTACK_CMSG_DATA(cm);
			if (sdl == NULL)
			{
				zlog_warn(MODULE_PAL, "could not get the received interface by IP_RECVIF");
				return -1;
			}
			ifindex = sdl->sdl_index;
		}
#endif

#if defined(IP_PKTINFO)
		if (cm->cmsg_level == IPSTACK_IPPROTO_IP &&
			cm->cmsg_type == IPSTACK_IP_PKTINFO)
		{
			pktinfo = (struct ipstack_in_pktinfo *)IPSTACK_CMSG_DATA(cm);
			if (pktinfo == NULL)
			{
				zlog_warn(MODULE_PAL, "could not get the received interface by IP_PKTINFO");
				return -1;
			}
			ifindex = pktinfo->ipi_ifindex;
		}
#endif
	}
	sin4 = (struct ipstack_sockaddr_in *)&ss;
	if (!ifindex)
	{
		zlog_warn(MODULE_PAL, "ss.ifindex %d", ifindex);
		return -1;
	}
	return kernel_packet_handle(ifindex, sin4, packetbuf, len);
}

static int kernel_packet_thread(struct eloop *eloop)
{
	zpl_socket_t sock = ELOOP_FD(eloop);

	if (eloop_master_module_lookup(MODULE_PAL))
		eloop_add_read(eloop_master_module_lookup(MODULE_PAL), kernel_packet_thread, NULL, sock);

	return kernel_packet_recv(sock);
}

int kernel_packet_init(void)
{
	return 0;
	zpl_socket_t sock = kernel_packet_socket_init();
	if (ipstack_invalid(sock))
	{
		
		if (eloop_master_module_lookup(MODULE_PAL))
			eloop_add_read(eloop_master_module_lookup(MODULE_PAL), kernel_packet_thread, NULL, sock);
	}
	return 0;
}