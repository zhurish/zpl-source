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
#include <net/if.h>
#include <ifaddrs.h>
//#include <netpacket/packet.h>

int FAST_FUNC d6_read_interface(const char *interface, ifindex_t *ifindex, struct ipstack_in6_addr *nip6, zpl_uint8 *mac)
{
	int retval = 3;
	struct ifaddrs *ifap, *ifa;

	getifaddrs(&ifap);

	for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
		struct ipstack_sockaddr_in6 *sip6;

		if (!ifa->ifa_addr || (strcmp(ifa->ifa_name, interface) != 0))
			continue;

		sip6 = (struct ipstack_sockaddr_in6*)(ifa->ifa_addr);

		if (ifa->ifa_addr->sa_family == IPSTACK_AF_PACKET) {
			struct ipstack_sockaddr_ll *sll = (struct ipstack_sockaddr_ll*)(ifa->ifa_addr);
			memcpy(mac, sll->sll_addr, 6);
			zlog_err(MODULE_DHCP,"MAC %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			zlog_err(MODULE_DHCP,"ifindex %d", sll->sll_ifindex);
			*ifindex = sll->sll_ifindex;
			retval &= (0xf - (1<<0));
		}
#if 0
		if (ifa->ifa_addr->sa_family == IPSTACK_AF_INET) {
			*nip = ((struct ipstack_sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;
			log1("IP %s", ipstack_inet_ntoa(((struct ipstack_sockaddr_in *)ifa->ifa_addr)->sin_addr));
		}
#endif
		if (ifa->ifa_addr->sa_family == IPSTACK_AF_INET6
		 && IPSTACK_IN6_IS_ADDR_LINK_LOCAL(&sip6->sin6_addr)
		) {
			*nip6 = sip6->sin6_addr; /* struct copy */
			zlog_err(MODULE_DHCP,
				"IPv6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
				nip6->s6_addr[0], nip6->s6_addr[1],
				nip6->s6_addr[2], nip6->s6_addr[3],
				nip6->s6_addr[4], nip6->s6_addr[5],
				nip6->s6_addr[6], nip6->s6_addr[7],
				nip6->s6_addr[8], nip6->s6_addr[9],
				nip6->s6_addr[10], nip6->s6_addr[11],
				nip6->s6_addr[12], nip6->s6_addr[13],
				nip6->s6_addr[14], nip6->s6_addr[15]
			);
			retval &= (0xf - (1<<1));
		}
	}

	freeifaddrs(ifap);
	if (retval == 0)
		return retval;

	if (retval & (1<<0))
		zlog_err(MODULE_DHCP,"can't get %s", "MAC");
	if (retval & (1<<1))
		zlog_err(MODULE_DHCP,"can't get %s", "link-local IPv6 address");
	return -1;
}

zpl_socket_t FAST_FUNC d6_listen_socket(zpl_uint16 port, const char *inf)
{
	zpl_socket_t fd;
	struct ipstack_sockaddr_in6 addr;

	zlog_err(MODULE_DHCP,"opening listen socket on *:%d %s", port, inf);
	fd = ipstack_socket(IPCOM_STACK, IPSTACK_PF_INET6, IPSTACK_SOCK_DGRAM, IPSTACK_IPPROTO_UDP);

	//setsockopt_reuseaddr(fd);
	//if (setsockopt_broadcast(fd) == -1)
	//	zlog_err(MODULE_DHCP,"IPSTACK_SO_BROADCAST");

	/* NB: bug 1032 says this doesn't work on ethernet aliases (ethN:M) */
	//if (setsockopt_bindtodevice(fd, inf))
	//	;//xfunc_die(); /* warning is already printed */

	memset(&addr, 0, sizeof(addr));
	addr.sin6_family = IPSTACK_AF_INET6;
	addr.sin6_port = htons(port);
	/* addr.sin6_addr is all-zeros */
	ipstack_bind(fd, (struct ipstack_sockaddr *)&addr, sizeof(addr));

	return fd;
}
