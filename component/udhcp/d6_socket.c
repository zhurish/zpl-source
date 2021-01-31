/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 2011 Denys Vlasenko.
 *
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */
#include "common.h"
#include "d6_common.h"
#include <net/if.h>
#include <ifaddrs.h>
#include <netpacket/packet.h>

int FAST_FUNC d6_read_interface(const char *interface, int *ifindex, struct in6_addr *nip6, uint8_t *mac)
{
	int retval = 3;
	struct ifaddrs *ifap, *ifa;

	getifaddrs(&ifap);

	for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
		struct sockaddr_in6 *sip6;

		if (!ifa->ifa_addr || (strcmp(ifa->ifa_name, interface) != 0))
			continue;

		sip6 = (struct sockaddr_in6*)(ifa->ifa_addr);

		if (ifa->ifa_addr->sa_family == AF_PACKET) {
			struct sockaddr_ll *sll = (struct sockaddr_ll*)(ifa->ifa_addr);
			memcpy(mac, sll->sll_addr, 6);
			zlog_err(MODULE_DHCP,"MAC %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			zlog_err(MODULE_DHCP,"ifindex %d", sll->sll_ifindex);
			*ifindex = sll->sll_ifindex;
			retval &= (0xf - (1<<0));
		}
#if 0
		if (ifa->ifa_addr->sa_family == AF_INET) {
			*nip = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;
			log1("IP %s", inet_ntoa(((struct sockaddr_in *)ifa->ifa_addr)->sin_addr));
		}
#endif
		if (ifa->ifa_addr->sa_family == AF_INET6
		 && IN6_IS_ADDR_LINKLOCAL(&sip6->sin6_addr)
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

int FAST_FUNC d6_listen_socket(int port, const char *inf)
{
	int fd;
	struct sockaddr_in6 addr;

	zlog_err(MODULE_DHCP,"opening listen socket on *:%d %s", port, inf);
	fd = socket(PF_INET6, SOCK_DGRAM, IPPROTO_UDP);

	setsockopt_reuseaddr(fd);
	if (setsockopt_broadcast(fd) == -1)
		zlog_err(MODULE_DHCP,"SO_BROADCAST");

	/* NB: bug 1032 says this doesn't work on ethernet aliases (ethN:M) */
	if (setsockopt_bindtodevice(fd, inf))
		;//xfunc_die(); /* warning is already printed */

	memset(&addr, 0, sizeof(addr));
	addr.sin6_family = AF_INET6;
	addr.sin6_port = htons(port);
	/* addr.sin6_addr is all-zeros */
	bind(fd, (struct sockaddr *)&addr, sizeof(addr));

	return fd;
}
