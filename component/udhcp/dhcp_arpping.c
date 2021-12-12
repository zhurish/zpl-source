/* vi: set sw=4 ts=4: */
/*
 * Mostly stolen from: dhcpcd - DHCP client daemon
 * by Yoichi Hariguchi <yoichi@fore.com>
 *
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */

#include <netinet/if_ether.h>
#include <net/if_arp.h>

#include "dhcp_def.h"
#include "dhcp_util.h"

struct arpMsg {
	/* Ethernet header */
	zpl_uint8  h_dest[6];     /* 00 destination ether addr */
	zpl_uint8  h_source[6];   /* 06 source ether addr */
	zpl_uint16 h_proto;       /* 0c packet type ID field */

	/* ARP packet */
	zpl_uint16 htype;         /* 0e hardware type (must be ARPHRD_ETHER) */
	zpl_uint16 ptype;         /* 10 protocol type (must be ETH_P_IP) */
	zpl_uint8  hlen;          /* 12 hardware address length (must be 6) */
	zpl_uint8  plen;          /* 13 protocol address length (must be 4) */
	zpl_uint16 operation;     /* 14 ARP opcode */
	zpl_uint8  sHaddr[6];     /* 16 sender's hardware address */
	zpl_uint8  sInaddr[4];    /* 1c sender's IP address */
	zpl_uint8  tHaddr[6];     /* 20 target's hardware address */
	zpl_uint8  tInaddr[4];    /* 26 target's IP address */
	zpl_uint8  pad[18];       /* 2a pad for min. ethernet payload (60 bytes) */
} PACKED;

enum {
	ARP_MSG_SIZE = 0x2a
};

/* Returns 1 if no reply received */
int icmp_echo_request(zpl_uint32  test_nip,
		const zpl_uint8 *safe_mac,
		zpl_uint32  from_ip,
		zpl_uint8 *from_mac,
		const char *interface,
		zpl_uint32 timeo)
{
	int timeout_ms = 0;
	zpl_socket_t pfd;
	int rv = 1;             /* "no reply received" yet */
	struct ipstack_sockaddr addr;   /* for interface name */
	struct arpMsg arp;

	if (!timeo)
		return 1;

	pfd = ipstack_socket(IPCOM_STACK, PF_PACKET, SOCK_PACKET, htons(ETH_P_ARP));
	if (ipstack_invalid(pfd)) {
		zlog_err(MODULE_DHCP, "can't create raw socket");
		return -1;
	}
	if (ipstack_setsockopt(pfd, SOL_SOCKET, SO_BROADCAST, &rv, sizeof(int)) == -1) {
		zlog_err(MODULE_DHCP, "can't enable bcast on raw socket");
		goto ret;
	}

	/* send arp request */
	memset(&arp, 0, sizeof(arp));
	memset(arp.h_dest, 0xff, 6);                    /* MAC DA */
	memcpy(arp.h_source, from_mac, 6);              /* MAC SA */
	arp.h_proto = htons(ETH_P_ARP);                 /* protocol type (Ethernet) */
	arp.htype = htons(ARPHRD_ETHER);                /* hardware type */
	arp.ptype = htons(ETH_P_IP);                    /* protocol type (ARP message) */
	arp.hlen = 6;                                   /* hardware address length */
	arp.plen = 4;                                   /* protocol address length */
	arp.operation = htons(ARPOP_REQUEST);           /* ARP op code */
	memcpy(arp.sHaddr, from_mac, 6);                /* source hardware address */
	memcpy(arp.sInaddr, &from_ip, sizeof(from_ip)); /* source IP address */
	/* tHaddr is zero-filled */                     /* target hardware address */
	memcpy(arp.tInaddr, &test_nip, sizeof(test_nip));/* target IP address */

	memset(&addr, 0, sizeof(addr));
	strncpy(addr.sa_data, interface, sizeof(addr.sa_data));
	if (ipstack_sendto(pfd, &arp, sizeof(arp), 0, &addr, sizeof(addr)) < 0) {
		// TODO: error message? caller didn't expect us to fail,
		// just returning 1 "no reply received" misleads it.
		goto ret;
	}

	/* wait for arp reply, and check it */
	timeout_ms = (int)timeo;
	do {
		typedef zpl_uint32  aliased_uint32_t FIX_ALIASING;
		int r;
		unsigned prevTime = os_get_monotonic_msec();

		r = safe_poll(pfd, 1, timeout_ms);
		if (r < 0)
			break;
		if (r) {
			r = safe_read(pfd, &arp, sizeof(arp));
			if (r < 0)
				break;

			//log3("sHaddr %02x:%02x:%02x:%02x:%02x:%02x",
			//	arp.sHaddr[0], arp.sHaddr[1], arp.sHaddr[2],
			//	arp.sHaddr[3], arp.sHaddr[4], arp.sHaddr[5]);

			if (r >= ARP_MSG_SIZE
			 && arp.operation == htons(ARPOP_REPLY)
			 /* don't check it: Linux doesn't return proper tHaddr (fixed in 2.6.24?) */
			 /* && memcmp(arp.tHaddr, from_mac, 6) == 0 */
			 && *(aliased_uint32_t*)arp.sInaddr == test_nip
			) {
				/* if ARP source MAC matches safe_mac
				 * (which is client's MAC), then it's not a conflict
				 * (client simply already has this IP and replies to ARPs!)
				 */
				if (!safe_mac || memcmp(safe_mac, arp.sHaddr, 6) != 0)
					rv = 0;
				//else log2("sHaddr == safe_mac");
				break;
			}
		}
		timeout_ms -= (unsigned)os_get_monotonic_msec() - prevTime + 1;

		/* We used to check "timeout_ms > 0", but
		 * this is more under/overflow-resistant
		 * (people did see overflows here when system time jumps):
		 */
	} while ((unsigned)timeout_ms <= timeo);

 ret:
	ipstack_close(pfd);
	zlog_err(MODULE_DHCP, "%srp reply received for this address", rv ? "no a" : "A");
	return rv;
}



int icmp_echo_request_mac(zpl_uint32  test_nip,
		zpl_uint32  from_ip,
		zpl_uint8 *from_mac,
		const char *interface,
		zpl_uint32 timeo,
		zpl_uint8 *safe_mac)
{
	int timeout_ms = 0;
	zpl_socket_t s;          /* socket */
	int rv = 1;             /* "no reply received" yet */
	struct ipstack_sockaddr addr;   /* for interface name */
	struct arpMsg arp;

	if (!timeo)
		return 1;

	s = ipstack_socket(IPCOM_STACK, PF_PACKET, SOCK_PACKET, htons(ETH_P_ARP));
	if (ipstack_invalid(s)) {
		zlog_err(MODULE_DHCP, "can't create raw socket");
		return -1;
	}
	if (ipstack_setsockopt(s, SOL_SOCKET, SO_BROADCAST, &rv, sizeof(int)) == -1) {
		zlog_err(MODULE_DHCP, "can't enable bcast on raw socket");
		goto ret;
	}

	/* send arp request */
	memset(&arp, 0, sizeof(arp));
	memset(arp.h_dest, 0xff, 6);                    /* MAC DA */
	memcpy(arp.h_source, from_mac, 6);              /* MAC SA */
	arp.h_proto = htons(ETH_P_ARP);                 /* protocol type (Ethernet) */
	arp.htype = htons(ARPHRD_ETHER);                /* hardware type */
	arp.ptype = htons(ETH_P_IP);                    /* protocol type (ARP message) */
	arp.hlen = 6;                                   /* hardware address length */
	arp.plen = 4;                                   /* protocol address length */
	arp.operation = htons(ARPOP_REQUEST);           /* ARP op code */
	memcpy(arp.sHaddr, from_mac, 6);                /* source hardware address */
	memcpy(arp.sInaddr, &from_ip, sizeof(from_ip)); /* source IP address */
	/* tHaddr is zero-filled */                     /* target hardware address */
	memcpy(arp.tInaddr, &test_nip, sizeof(test_nip));/* target IP address */

	memset(&addr, 0, sizeof(addr));
	strncpy(addr.sa_data, interface, sizeof(addr.sa_data));
	if (ipstack_sendto(s, &arp, sizeof(arp), 0, &addr, sizeof(addr)) < 0) {
		// TODO: error message? caller didn't expect us to fail,
		// just returning 1 "no reply received" misleads it.
		goto ret;
	}

	/* wait for arp reply, and check it */
	timeout_ms = (int)timeo;
	do {
		typedef zpl_uint32  aliased_uint32_t FIX_ALIASING;
		int r;
		unsigned prevTime = os_get_monotonic_msec();

		r = safe_poll(s, 1, timeout_ms);
		if (r < 0)
			break;
		if (r) {
			r = safe_read(s, &arp, sizeof(arp));
			if (r < 0)
				break;

			//log3("sHaddr %02x:%02x:%02x:%02x:%02x:%02x",
			//	arp.sHaddr[0], arp.sHaddr[1], arp.sHaddr[2],
			//	arp.sHaddr[3], arp.sHaddr[4], arp.sHaddr[5]);

			if (r >= ARP_MSG_SIZE
			 && arp.operation == htons(ARPOP_REPLY)
			 /* don't check it: Linux doesn't return proper tHaddr (fixed in 2.6.24?) */
			 /* && memcmp(arp.tHaddr, from_mac, 6) == 0 */
			 && *(aliased_uint32_t*)arp.sInaddr == test_nip
			) {
				/* if ARP source MAC matches safe_mac
				 * (which is client's MAC), then it's not a conflict
				 * (client simply already has this IP and replies to ARPs!)
				 */
				if (!safe_mac)
				{
					memcpy(safe_mac, arp.sHaddr, 6);
					rv = 0;
				}
				//else log2("sHaddr == safe_mac");
				break;
			}
		}
		timeout_ms -= (unsigned)os_get_monotonic_msec() - prevTime + 1;

		/* We used to check "timeout_ms > 0", but
		 * this is more under/overflow-resistant
		 * (people did see overflows here when system time jumps):
		 */
	} while ((unsigned)timeout_ms <= timeo);

 ret:
	ipstack_close(s);
	zlog_err(MODULE_DHCP, "%srp reply received for this address", rv ? "no a" : "A");
	return rv;
}
