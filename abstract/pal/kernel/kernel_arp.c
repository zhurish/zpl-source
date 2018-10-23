/*
 * kernel_arp.c
 *
 *  Created on: Sep 29, 2018
 *      Author: zhurish
 */

#include <zebra.h>

#include "if.h"
#include "vty.h"
#include "sockunion.h"
#include "prefix.h"
#include "command.h"
#include "memory.h"
#include "log.h"
#include "zclient.h"
#include "thread.h"
#include "eloop.h"

#include "nsm_mac.h"
#include "nsm_arp.h"

#include "nsm_veth.h"
#include "nsm_tunnel.h"
#include "nsm_bridge.h"

#include "pal_interface.h"
#include "kernel_ioctl.h"


#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <linux/if_packet.h>
/*
1 #define ATF_COM         0x02    //已完成的邻居(成员ha有效，且含有正确的MAC地址)
2 #define ATF_PERM        0x04    //永久性的邻居(邻居状态有NUD_PERMANENT)
3 #define ATF_PUBL        0x08    //发布该邻居
4 #define ATF_USETRAILERS 0x10    //不是非常清楚
5 #define ATF_NETMASK     0x20    //仅用于代理ARP
6 #define ATF_DONTPUB     0x40    //不处理该邻居
 */

/* Delete an entry from the ARP cache. */
static int kernel_arp_del(struct interface *ifp, struct prefix *address)
{
	struct arpreq arpreq;
	struct sockaddr_in *sin;
	int rc;

	/*you must add this becasue some system will return "Invlid argument"
	 because some argument isn't zero */
	memset(&arpreq, 0, sizeof(struct arpreq));
	sin = (struct sockaddr_in *) &arpreq.arp_pa;
	memset(sin, 0, sizeof(struct sockaddr_in));
	sin->sin_family = address->family;
	memcpy(&sin->sin_addr, (char *) &address->u.prefix4, sizeof(struct in_addr));
	strcpy(arpreq.arp_dev, ifp->k_name);

	rc = if_ioctl (SIOCDARP, &arpreq);
	if (rc < 0)
	{
		return -1;
	}
	return 0;
}

/* Set an entry in the ARP cache. */
static int kernel_arp_set(struct interface *ifp, struct prefix *address, unsigned char *mac)
{
	struct arpreq arpreq;
	struct sockaddr_in *sin;
	int rc;
	/*you must add this becasue some system will return "Invlid argument"
	 because some argument isn't zero */
	memset(&arpreq, 0, sizeof(struct arpreq));
	sin = (struct sockaddr_in *) &arpreq.arp_pa;
	memset(sin, 0, sizeof(struct sockaddr_in));
	sin->sin_family = address->family;
	memcpy(&sin->sin_addr, (char *) &address->u.prefix4, sizeof(struct in_addr));
	strcpy(arpreq.arp_dev, ifp->k_name);
	memcpy((unsigned char *) arpreq.arp_ha.sa_data, mac, 6);

	arpreq.arp_flags = ATF_PERM | ATF_COM; //note, must set flag, if not,you will get error

	rc = if_ioctl (SIOCSARP, &arpreq);
	if (rc < 0)
	{
		return -1;
	}
	return 0;
}

static int kernel_arp_get(struct interface *ifp, struct prefix *address, unsigned char *mac)
{
	struct arpreq arpreq;
	struct sockaddr_in *sin;
	int rc;

	/*you must add this becasue some system will return "Invlid argument"
	 because some argument isn't zero */
	memset(&arpreq, 0, sizeof(struct arpreq));
	sin = (struct sockaddr_in *) &arpreq.arp_pa;
	memset(sin, 0, sizeof(struct sockaddr_in));
	sin->sin_family = address->family;
	memcpy(&sin->sin_addr, (char *) &address->u.prefix4, sizeof(struct in_addr));
	strcpy(arpreq.arp_dev, ifp->k_name);

	rc = if_ioctl (SIOCGARP, &arpreq);
	if (rc < 0)
	{
		return -1;
	}
	memcpy(mac, (unsigned char *) arpreq.arp_ha.sa_data, 6);
	return 0;
}



static int kernel_arp_init()
{
	int skfd = 0, onoff = 1;
	/*创建原始套接字*/
	skfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
	if (skfd < 0)
	{
		printf("socket() failed! \n");
		return -1;
	}
#ifdef IP_RECVIF
	onoff = 1;
	if (setsockopt(skfd, IPPROTO_IP, IP_RECVIF, &onoff, sizeof(onoff)) != 0)
		dhcpd_error("setsocketopt IP_RECVIF failed for udp: %s",
		    strerror(errno));
#endif
#if defined (IP_PKTINFO)
	onoff = 1;
	if (setsockopt (skfd, IPPROTO_IP, IP_PKTINFO, &onoff, sizeof (onoff)) != 0)
		dhcpd_error("setsocketopt IP_PKTINFO failed for udp: %s",
		    strerror(errno));
#endif
	return skfd;
}

static int kernel_arp_relay_handle(int ifindex, struct sockaddr_in	*from, unsigned char *buf, int len)
{
	struct interface * ifp = NULL;
	struct ether_header *eth = NULL;
	struct ether_arp *arp = NULL;
	eth = (struct ether_header *)(buf);
	arp = (struct ether_arp *)(buf + sizeof(struct ether_header));
	ifp = if_lookup_by_kernel_index (ifindex);
	if (ntohs(arp->arp_op) == 2 && ifp)
	{
		ip_arp_t arpt;
		arpt.class = ARP_DYNAMIC;
		memcpy(&arpt.address.u.prefix4.s_addr, arp->arp_spa, arp->arp_pln);
		arpt.address.family = AF_INET;
		memcpy(arpt.mac, eth->ether_shost, NSM_MAC_MAX);
		arpt.ifindex = ifindex;
		arpt.vrfid = ifp->vrf_id;
		arpt.ttl = NSM_ARP_TTL_DEFAULT;

		ip_arp_dynamic_cb(1, &arpt);
	}
	return 0;
}


static int kernel_arp_recv(int	sock)
{
	char 				cbuf [256];
	ssize_t			 	len;
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
	unsigned char		 packetbuf[1024];
	int ifindex = 0;


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

	if ((len = recvmsg(sock, &m, 0)) < 0) {
		zlog_warn(ZLOG_PAL, "receiving a ARP message failed: %s", strerror(errno));
		return -1;
	}
	if (ss.ss_family != AF_INET) {
		zlog_warn(ZLOG_PAL, "received ARP message is not AF_INET");
		return -1;
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
				zlog_warn(ZLOG_PAL, "could not get the received interface by IP_RECVIF");
				return -1;
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
				zlog_warn(ZLOG_PAL, "could not get the received interface by IP_PKTINFO");
				return -1;
			}
			ifindex = pktinfo->ipi_ifindex;
		}
#endif
	}
	sin4 = (struct sockaddr_in *)&ss;
	if(!ifindex)
	{
		return -1;
	}
	return kernel_arp_relay_handle( ifindex, sin4, packetbuf, len);
}



static int kernel_arp_thread(struct eloop *eloop)
{
	int sock = ELOOP_FD(eloop);

	if(master_eloop[MODULE_KERNEL])
		eloop_add_read(master_eloop[MODULE_KERNEL], kernel_arp_thread, NULL, sock);

	return kernel_arp_recv(sock);
}

static int kernel_arp_request(struct interface *ifp, struct prefix *address)
{
	struct ether_header *eth = NULL;
	struct ether_arp *arp = NULL;
	struct in_addr saddr;
	struct sockaddr_ll sll;

	int skfd;
	int n = 0, len = 0;

	unsigned char buf[1024];

	//daddr.s_addr = 0xffffffff;
	/*伪造 源IP*/
	saddr.s_addr = address->u.prefix4.s_addr;

	memset(buf, 0x00, sizeof(buf));

	/*创建原始套接字*/
	skfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
	if (skfd < 0)
	{
		printf("socket() failed! \n");
		return -1;
	}

	bzero(&sll, sizeof(sll));
	sll.sll_ifindex	= ifp->k_ifindex;
	sll.sll_family	= PF_PACKET;
	sll.sll_protocol = htons(ETH_P_ARP/*ETH_P_ALL*/);

	/*构造以太报文*/
	eth = (struct ether_header*)buf;
	eth->ether_type = htons(ETHERTYPE_ARP);
	memset(eth->ether_dhost, 0xff, ETH_ALEN);
	memcpy(eth->ether_shost, ifp->hw_addr, ifp->hw_addr_len);

	/*构造ARP报文*/
	arp = (struct ether_arp*)(buf + sizeof(struct ether_header));
	arp->arp_hrd = htons(ARPHRD_ETHER);
	arp->arp_pro = htons(ETHERTYPE_IP);
	arp->arp_hln = ETH_ALEN;
	arp->arp_pln = 4;
	arp->arp_op = htons(ARPOP_REQUEST);
	memcpy(arp->arp_sha, ifp->hw_addr, ifp->hw_addr_len);
	memcpy(arp->arp_spa, &saddr.s_addr, 4);
	/*
	    memcpy(arp->arp_tha, dmac, ETH_ALEN);*/
	memset(arp->arp_tpa, 0, 4);
	len = sizeof(struct ether_header) + sizeof(struct ether_arp);
	if(len < 128)
		len = 128;
	n = sendto(skfd, buf, len, 0, (struct sockaddr*)&sll, sizeof(struct sockaddr_ll));
	if (n < 0)
	{
		printf("sendto() failed!\n");
	}
	else
	{
		printf("sendto() n = %d \n", n);
	}
	close(skfd);
	return 0;
}

static int kernel_arp_gratuitousarp_enable(int enable)
{
	return 0;
}


int ip_arp_stack_init()
{
	int sock = 0;
	extern pal_stack_t pal_stack;
	//arp
	pal_stack.ip_stack_arp_add = kernel_arp_set;
	pal_stack.ip_stack_arp_delete = kernel_arp_del;
	pal_stack.ip_stack_arp_request = kernel_arp_request;
	pal_stack.ip_stack_arp_gratuitousarp_enable = kernel_arp_gratuitousarp_enable;
	pal_stack.ip_stack_arp_ttl = NULL;
	pal_stack.ip_stack_arp_age_timeout = NULL;
	pal_stack.ip_stack_arp_retry_interval = NULL;

/*
	sock = kernel_arp_init();

	if(master_eloop[MODULE_KERNEL])
		eloop_add_read(master_eloop[MODULE_KERNEL], kernel_arp_thread, NULL, sock);
*/

	return OK;
}

