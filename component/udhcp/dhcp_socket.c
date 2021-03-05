/* vi: set sw=4 ts=4: */
/*
 * DHCP server client/server socket creation
 *
 * udhcp client/server
 * Copyright (C) 1999 Matthew Ramsay <matthewr@moreton.com.au>
 *			Chris Trew <ctrew@moreton.com.au>
 *
 * Rewrite by Russ Dill <Russ.Dill@asu.edu> July 2001
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include "dhcp_def.h"
#include "dhcp_util.h"
//#include <net/if.h>


/* 1. None of the callers expects it to ever fail */
/* 2. ip was always INADDR_ANY */
int udhcp_udp_socket(/*ospl_uint32  ip,*/ospl_uint16 port)
{
	int fd = 0;
	fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd)
	{
		sockopt_reuseaddr(fd);
		if (sockopt_broadcast(fd) == -1)
			zlog_err(MODULE_DHCP, "SO_BROADCAST");

		if (setsockopt_ifindex(AF_INET, fd, 1) == -1)
			zlog_err(MODULE_DHCP, "setsockopt_ifindex");

		sockopt_ttl(AF_INET, fd, 1);
		os_set_nonblocking(fd);
		if (sock_bind(fd, NULL, port) == OK)
			return fd;
		zlog_err(MODULE_DHCP, "sock_bind");
	}
	zlog_err(MODULE_DHCP, "udhcp_udp_socket");
	return ERROR;
}

int udhcp_raw_socket(void)
{
	int fd = 0, onoff = 1;
	/* Get packet socket to write raw frames on */
	if ((fd = raw_sock_create(SOCK_DGRAM, (ETH_P_IP))) < 0)
	{
		zlog_err(MODULE_DHCP, "failed to open raw udp for relay(%s)", strerror(errno));
		return (-1);
	}
	setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (char *)&onoff, sizeof(onoff));
	return fd;
}


int udhcp_client_socket_bind(int fd, ifindex_t ifindex)
{
	int ret = 0;
	ifindex_t kifindex = ifindex2ifkernel(ifindex);
	//zlog_err(MODULE_DHCP, "Can not bind raw socket(%s)", kifindex);
	ret = raw_sock_bind(fd, PF_PACKET, ETH_P_IP, kifindex);
	return ret;
}

int udhcp_client_socket_filter(int fd, ospl_uint16 port)
{
	/* Several users reported breakage when BPF filter is used */
	{
		/* Use only if standard port is in use */
		/*
		 *	I've selected not to see LL header, so BPF doesn't see it, too.
		 *	The filter may also pass non-IP and non-ARP packets, but we do
		 *	a more complete check when receiving the message in userspace.
		 *
		 * and filter shamelessly stolen from:
		 *
		 *	http://www.flamewarmaster.de/software/dhcpclient/
		 *
		 * There are a few other interesting ideas on that page (look under
		 * "Motivation").  Use of netlink events is most interesting.  Think
		 * of various network servers listening for events and reconfiguring.
		 * That would obsolete sending HUP signals and/or make use of restarts.
		 *
		 * Copyright: 2006, 2007 Stefan Rompf <sux@loplof.de>.
		 * License: GPL v2.
		 */
		static const struct sock_filter filter_instr[] = {
		/* load 9th byte (protocol) */
		BPF_STMT(BPF_LD | BPF_B | BPF_ABS, 9),
		/* jump to L1 if it is IPPROTO_UDP, else to L4 */
		BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, IPPROTO_UDP, 0, 6),
		/* L1: load halfword from offset 6 (flags and frag offset) */
		BPF_STMT(BPF_LD | BPF_H | BPF_ABS, 6),
		/* jump to L4 if any bits in frag offset field are set, else to L2 */
		BPF_JUMP(BPF_JMP | BPF_JSET | BPF_K, 0x1fff, 4, 0),
		/* L2: skip IP header (load index reg with header len) */
		BPF_STMT(BPF_LDX | BPF_B | BPF_MSH, 0),
		/* load udp destination port from halfword[header_len + 2] */
		BPF_STMT(BPF_LD | BPF_H | BPF_IND, 2),
		/* jump to L3 if udp dport is CLIENT_PORT, else to L4 */
		BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, 68/*port*/, 0, 1),
		/* L3: accept packet ("accept 0x7fffffff bytes") */
		/* Accepting 0xffffffff works too but kernel 2.6.19 is buggy */
		BPF_STMT(BPF_RET | BPF_K, 0x7fffffff),
		/* L4: discard packet ("accept zero bytes") */
		BPF_STMT(BPF_RET | BPF_K, 0), };
		static const struct sock_fprog filter_prog = { .len =
				sizeof(filter_instr) / sizeof(filter_instr[0]),
		/* casting const away: */
		.filter = (struct sock_filter *) filter_instr, };
		/* Ignoring error (kernel may lack support for this) */
		if (setsockopt(fd, SOL_SOCKET, SO_ATTACH_FILTER, &filter_prog,
				sizeof(filter_prog)) < 0)
		{
			zlog_err(MODULE_DHCP, "attached filter to raw socket fail:%s", strerror(errno));
			return ERROR;
		}
	}
	return OK;
}
