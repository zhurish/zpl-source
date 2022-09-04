/*
 * pal_arp.c
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */


#include "zplos_include.h"
#include "nsm_include.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"
#include "pal_arp.h"
#include "pal_global.h"
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <linux/if_packet.h>

#ifdef ZPL_NSM_ARP
// ip arp
int pal_interface_arp_add(struct interface *ifp, struct prefix *address, zpl_uint8 *mac)
{
	if(pal_stack.ip_stack_arp_add && ifp->k_ifindex)
		return pal_stack.ip_stack_arp_add(ifp, address, mac);
    return OK;
}

int pal_interface_arp_delete(struct interface *ifp, struct prefix *address)
{
	if(pal_stack.ip_stack_arp_delete && ifp->k_ifindex)
		return pal_stack.ip_stack_arp_delete(ifp, address);
    return OK;
}

int pal_interface_arp_request(struct interface *ifp, struct prefix *address)
{
	if(pal_stack.ip_stack_arp_request && ifp->k_ifindex)
		return pal_stack.ip_stack_arp_request(ifp, address);
    return OK;
}

int pal_arp_gratuitousarp_enable(zpl_bool enable)
{
	if(pal_stack.ip_stack_arp_gratuitousarp_enable)
		return pal_stack.ip_stack_arp_gratuitousarp_enable(enable);
    return OK;
}

int pal_arp_ttl(zpl_uint32 ttl)
{
	if(pal_stack.ip_stack_arp_ttl)
		return pal_stack.ip_stack_arp_ttl(ttl);
    return OK;
}

int pal_arp_age_timeout(zpl_uint32 timeout)
{
	if(pal_stack.ip_stack_arp_age_timeout)
		return pal_stack.ip_stack_arp_age_timeout(timeout);
    return OK;
}

int pal_arp_retry_interval(zpl_uint32 interval)
{
	if(pal_stack.ip_stack_arp_retry_interval)
		return pal_stack.ip_stack_arp_retry_interval(interval);
    return OK;
}



static zpl_socket_t pal_arp_sock_init(void)
{
	zpl_socket_t skfd;
	int onoff = 1;
	/*创建原始套接字*/
	skfd = ipstack_socket(IPCOM_STACK, IPSTACK_PF_PACKET, IPSTACK_SOCK_RAW, htons(IPSTACK_ETH_P_ARP));
	if (ipstack_invalid(skfd))
	{
		zlog_err(MODULE_PAL, "Can't open %s ipstack_socket: %s", "arp ipstack_socket",
				strerror(ipstack_errno));
		//printf("ipstack_socket() failed! \n");
		return skfd;
	}
#ifdef IP_RECVIF
	onoff = 1;
	if (ipstack_setsockopt(skfd, IPSTACK_IPPROTO_IP, IPSTACK_IP_RECVIF, &onoff, sizeof(onoff)) != 0)
		zlog_warn(MODULE_PAL, "setsocketopt IP_RECVIF failed for packet: %s",
		    strerror(ipstack_errno));
#endif
#if defined (IP_PKTINFO)
	onoff = 1;
	if (ipstack_setsockopt (skfd, IPSTACK_IPPROTO_IP, IPSTACK_IP_PKTINFO, &onoff, sizeof (onoff)) != 0)
		zlog_warn(MODULE_PAL, "setsocketopt IP_PKTINFO failed for packet: %s",
		    strerror(ipstack_errno));
#endif
	return skfd;
}

static int pal_arp_relay_handle(ifindex_t ifindex, struct ipstack_sockaddr_in *from, zpl_uint8 *buf, zpl_uint32 len)
{
	struct interface * ifp = NULL;
	struct ipstack_ether_header *eth = NULL;
	struct ipstack_ether_arp *arp = NULL;
	eth = (struct ipstack_ether_header *)(buf);
	arp = (struct ipstack_ether_arp *)(buf + sizeof(struct ipstack_ether_header));
	ifp = if_lookup_by_kernel_index (ifindex);
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
	return 0;
}


static int pal_arp_recv(zpl_socket_t	sock)
{
	char 				cbuf [256];
	ssize_t			 	len;
	struct ipstack_msghdr		 m;
	struct ipstack_cmsghdr		*cm = NULL;
	struct ipstack_iovec		 iov[1];
	struct ipstack_sockaddr_storage	 ss;
	struct ipstack_sockaddr_in	*sin4 = NULL;
#if defined(IP_PKTINFO)
	struct ipstack_in_pktinfo *pktinfo = NULL;
#endif
#ifdef IP_RECVIF
	struct ipstack_sockaddr_dl	*sdl = NULL;
#endif
	zpl_uint8		 packetbuf[1024];
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

	if ((len = ipstack_recvmsg(sock, &m, 0)) < 0) {
		zlog_warn(MODULE_PAL, "receiving a ARP message failed: %s", strerror(ipstack_errno));
		return -1;
	}
	if (ss.ss_family != IPSTACK_AF_INET) {
		//zlog_warn(MODULE_PAL, "received ARP message is not IPSTACK_AF_INET");
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
			if (sdl == NULL) {
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
			if (pktinfo == NULL) {
				zlog_warn(MODULE_PAL, "could not get the received interface by IP_PKTINFO");
				return -1;
			}
			ifindex = pktinfo->ipi_ifindex;
		}
#endif
	}
	sin4 = (struct ipstack_sockaddr_in *)&ss;
	if(!ifindex)
	{
		return -1;
	}
	return pal_arp_relay_handle( ifindex, sin4, packetbuf, len);
}



static int pal_arp_thread(struct eloop *eloop)
{
	zpl_socket_t sock = ELOOP_FD(eloop);

	if(eloop_master_module_lookup(MODULE_PAL))
		eloop_add_read(eloop_master_module_lookup(MODULE_PAL), pal_arp_thread, NULL, sock);

	return pal_arp_recv(sock);
}

static int pal_arp_request(struct interface *ifp, struct prefix *address)
{
	struct ipstack_ether_header *eth = NULL;
	struct ipstack_ether_arp *arp = NULL;
	struct ipstack_in_addr saddr;
	struct ipstack_sockaddr_ll sll;

	zpl_socket_t skfd;
	zpl_int32 n = 0, len = 0;

	zpl_uint8 buf[1024];

	//daddr.s_addr = 0xffffffff;
	/* 伪造 源IP */
	saddr.s_addr = address->u.prefix4.s_addr;

	memset(buf, 0x00, sizeof(buf));

	/* 创建原始套接字 */
	skfd = ipstack_socket(IPCOM_STACK, IPSTACK_PF_PACKET, IPSTACK_SOCK_RAW, htons(IPSTACK_ETH_P_ARP));
	if (ipstack_invalid(skfd))
	{
		zlog_err(MODULE_PAL, "Can't open %s ipstack_socket: %s", "arp ipstack_socket",
				strerror(ipstack_errno));
		return -1;
	}

	bzero(&sll, sizeof(sll));
	sll.sll_ifindex	= ifp->k_ifindex;
	sll.sll_family	= IPSTACK_PF_PACKET;
	sll.sll_protocol = htons(IPSTACK_ETH_P_ARP/*ETH_P_ALL*/);

	/* 构造以太报文 */
	eth = (struct ipstack_ether_header*)buf;
	eth->ether_type = htons(IPSTACK_ETHERTYPE_ARP);
	memset(eth->ether_dhost, 0xff, IPSTACK_ETH_ALEN);
	memcpy(eth->ether_shost, ifp->hw_addr, ifp->hw_addr_len);

	/* 构造ARP报文 */
	arp = (struct ipstack_ether_arp*)(buf + sizeof(struct ipstack_ether_header));
	arp->arp_hrd = htons(IPSTACK_ARPHRD_ETHER);
	arp->arp_pro = htons(IPSTACK_ETHERTYPE_IP);
	arp->arp_hln = IPSTACK_ETH_ALEN;
	arp->arp_pln = 4;
	arp->arp_op = htons(IPSTACK_ARPOP_REQUEST);
	memcpy(arp->arp_sha, ifp->hw_addr, ifp->hw_addr_len);
	memcpy(arp->arp_spa, &saddr.s_addr, 4);
	/* memcpy(arp->arp_tha, dmac, ETH_ALEN);*/
	memset(arp->arp_tpa, 0, 4);
	len = sizeof(struct ipstack_ether_header) + sizeof(struct ipstack_ether_arp);
	if(len < 128)
		len = 128;
	n = ipstack_sendto(skfd, buf, len, 0, (struct ipstack_sockaddr*)&sll, sizeof(struct ipstack_sockaddr_ll));
	if (n < 0)
	{
		zlog_warn(MODULE_PAL, "ipstack_send a ARP message failed: %s", strerror(ipstack_errno));
	}
	else
	{
		//printf("ipstack_sendto() n = %d \n", n);
	}
	ipstack_close(skfd);
	return 0;
}



int pal_arp_stack_init(void)
{
	zpl_socket_t sock;

	pal_stack.ip_stack_arp_request = pal_arp_request;
	sock = pal_arp_sock_init();

	if(!ipstack_invalid(sock) && eloop_master_module_lookup(MODULE_PAL))
		eloop_add_read(eloop_master_module_lookup(MODULE_PAL), pal_arp_thread, NULL, sock);

	return OK;
}
#endif