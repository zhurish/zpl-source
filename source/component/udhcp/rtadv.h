/**
 * Copyright (C) 2012-2014 Steven Barth <steven@midlink.org>
 * Copyright (C) 2018 Hans Dedecker <dedeckeh@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef __RTADV_H__
#define __RTADV_H__

#ifdef __cplusplus
extern "C" {
#endif
#ifdef ZPL_DHCPV6C_MODULE
#pragma once

#define ALL_IPV6_NODES {{{0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01}}}

#define ALL_IPV6_ROUTERS {{{0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02}}}

struct icmpv6_opt {
	uint8_t type;
	uint8_t len;
	uint8_t data[6];
};

struct icmpv6_opt_route_info {
	uint8_t type;
	uint8_t len;
	uint8_t prefix_len;
	uint8_t flags;
	uint32_t lifetime;
	uint8_t prefix[];
};

#define ND_OPT_ROUTE_INFORMATION 24


#define icmpv6_for_each_option(opt, start, end)\
	for (opt = (struct icmpv6_opt*)(start);\
	(void*)(opt + 1) <= (void*)(end) && opt->len > 0 &&\
	(void*)(opt + opt->len) <= (void*)(end); opt += opt->len)


int ra_init(const char *ifname, const struct ipstack_in6_addr *ifid,
		unsigned int options, unsigned int holdoff_interval);
bool ra_link_up(void);
bool ra_process(void);

int ra_get_hoplimit(void);
int ra_get_mtu(void);
int ra_get_reachable(void);
int ra_get_retransmit(void);

#endif /*ZPL_DHCPV6C_MODULE*/
#ifdef __cplusplus
}
#endif

#endif /* __RTADV_H__ */