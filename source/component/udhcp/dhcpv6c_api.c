/**
 * Copyright (C) 2012-2014 Steven Barth <steven@midlink.org>
 * Copyright (C) 2017-2018 Hans Dedecker <dedeckeh@gmail.com>
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
/*
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <limits.h>

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>

#include <net/if.h>
#include <sys/syscall.h>
#include <arpa/inet.h>
#include <linux/if_addr.h>
*/
#include "auto_include.h"
#include <zplos_include.h>
#ifdef ZPL_DHCPV6C_MODULE
#include "resolv.h"
#include "dhcpv6c.h"
#include "dhcpv6c_api.h"
#include "dhcpv6c_state.h"
#include "dhcpv6c_option.h"
#include "rtadv.h"

#ifndef IN6_IS_ADDR_UNIQUELOCAL
#define IN6_IS_ADDR_UNIQUELOCAL(a) \
	((((__const uint32_t *) (a))[0] & htonl (0xfe000000)) \
	 == htonl (0xfc000000))
#endif

bool dhcpv6c_addr_in_scope(struct dhcpv6c_interface *ifp, const struct ipstack_in6_addr *addr)
{
	FILE *fd = fopen("/proc/net/if_inet6", "r");
	int len;
	bool ret = false;
	char buf[256];

	if (fd == NULL)
		return false;

	while (fgets(buf, sizeof(buf), fd)) {
		struct ipstack_in6_addr inet6_addr;
		uint32_t flags, dummy;
		unsigned int i;
		char name[IF_NAMESIZE], addr_buf[33];

		len = strlen(buf);

		if ((len <= 0) || buf[len - 1] != '\n')
			break;

		buf[--len] = '\0';

		if (sscanf(buf, "%s %x %x %x %x %s",
				addr_buf, &dummy, &dummy, &dummy, &flags, name) != 6)
			break;
/*
		if (strcmp(name, ifname) ||
			(flags & (IFA_F_DADFAILED | IFA_F_TENTATIVE | IFA_F_DEPRECATED)))
			continue;
*/
		for (i = 0; i < strlen(addr_buf); i++) {
			if (!isxdigit(addr_buf[i]) || isupper(addr_buf[i]))
				break;
		}

		memset(&inet6_addr, 0, sizeof(inet6_addr));
		for (i = 0; i < (strlen(addr_buf) / 2); i++) {
			unsigned char byte;
			static const char hex[] = "0123456789abcdef";
			byte = ((index(hex, addr_buf[i * 2]) - hex) << 4) |
				(index(hex, addr_buf[i * 2 + 1]) - hex);
			inet6_addr.s6_addr[i] = byte;
		}

		if ((IN6_IS_ADDR_LINKLOCAL(&inet6_addr) == IN6_IS_ADDR_LINKLOCAL(addr)) &&
			(IN6_IS_ADDR_UNIQUELOCAL(&inet6_addr) == IN6_IS_ADDR_UNIQUELOCAL(addr))) {
			ret = true;
			break;
		}
	}

	fclose(fd);
	return ret;
}



// Don't want to pull-in librt and libpthread just for a monotonic clock...
uint64_t dhcpv6c_get_milli_time(void)
{
	struct timespec t;

	clock_gettime(CLOCK_MONOTONIC, &t);

	return ((uint64_t)t.tv_sec) * 1000 + ((uint64_t)t.tv_nsec) / 1000000;
}



static struct dhcpv6c_entry* dhcpv6c_find_entry(struct dhcpv6c_interface *difp, enum dhcpv6c_state state, const struct dhcpv6c_entry *new)
{
	size_t len, cmplen = offsetof(struct dhcpv6c_entry, target) + ((new->length + 7) / 8);
	uint8_t *start = dhcpv6c_state_get(difp, state, &len);

	for (struct dhcpv6c_entry *c = (struct dhcpv6c_entry*)start;
			(uint8_t*)c < &start[len] &&
			(uint8_t*)dhcpv6c_next_entry(c) <= &start[len];
			c = dhcpv6c_next_entry(c)) {
		if (!memcmp(c, new, cmplen) && !memcmp(c->auxtarget, new->auxtarget, new->auxlen))
			return c;
	}

	return NULL;
}

bool dhcpv6c_update_entry(struct dhcpv6c_interface *difp, enum dhcpv6c_state state, struct dhcpv6c_entry *new,
		uint32_t safe, unsigned int holdoff_interval)
{
	struct dhcpv6c_entry *x = dhcpv6c_find_entry(difp, state, new);

	if (x && x->valid > new->valid && new->valid < safe)
		new->valid = safe;

	if (x) {
		if (holdoff_interval && new->valid >= x->valid &&
				new->valid != UINT32_MAX &&
				new->valid - x->valid < holdoff_interval &&
				new->preferred >= x->preferred &&
				new->preferred != UINT32_MAX &&
				new->preferred - x->preferred < holdoff_interval)
			return false;

		x->valid = new->valid;
		x->preferred = new->preferred;
		x->t1 = new->t1;
		x->t2 = new->t2;
		x->iaid = new->iaid;
	} else if (dhcpv6c_state_add(difp, state, new, dhcpv6c_entry_size(new)))
		return false;

	return true;
}

static void dhcpv6c_expire_list(struct dhcpv6c_interface *difp, enum dhcpv6c_state state, uint32_t elapsed, bool remove_expired)
{
	size_t len;
	uint8_t *start = dhcpv6c_state_get(difp, state, &len);

	for (struct dhcpv6c_entry *c = (struct dhcpv6c_entry*)start;
			(uint8_t*)c < &start[len] &&
			(uint8_t*)dhcpv6c_next_entry(c) <= &start[len];
			) {
		if (c->t1 < elapsed)
			c->t1 = 0;
		else if (c->t1 != UINT32_MAX)
			c->t1 -= elapsed;

		if (c->t2 < elapsed)
			c->t2 = 0;
		else if (c->t2 != UINT32_MAX)
			c->t2 -= elapsed;

		if (c->preferred < elapsed)
			c->preferred = 0;
		else if (c->preferred != UINT32_MAX)
			c->preferred -= elapsed;

		if (c->valid < elapsed)
			c->valid = 0;
		else if (c->valid != UINT32_MAX)
			c->valid -= elapsed;

		if (!c->valid && remove_expired) {
			dhcpv6c_state_remove(difp, state, ((uint8_t*)c) - start, dhcpv6c_entry_size(c));
			start = dhcpv6c_state_get(difp, state, &len);
		} else
			c = dhcpv6c_next_entry(c);
	}
}

void dhcpv6c_expire(struct dhcpv6c_interface *difp, bool expire_ia_pd)
{
	time_t now = dhcpv6c_get_milli_time() / 1000;
	uint32_t elapsed = (difp->last_update > 0) ? now - difp->last_update : 0;

	difp->last_update = now;

	dhcpv6c_expire_list(difp, DHCPV6C_STATE_RA_PREFIX, elapsed, true);
	dhcpv6c_expire_list(difp, DHCPV6C_STATE_RA_ROUTE, elapsed, true);
	dhcpv6c_expire_list(difp, DHCPV6C_STATE_RA_DNS, elapsed, true);
	dhcpv6c_expire_list(difp, DHCPV6C_STATE_RA_SEARCH, elapsed, true);
	dhcpv6c_expire_list(difp, DHCPV6C_STATE_IA_NA, elapsed, true);
	dhcpv6c_expire_list(difp, DHCPV6C_STATE_IA_PD, elapsed, expire_ia_pd);
}

uint32_t dhcpv6c_elapsed(struct dhcpv6c_interface *difp)
{
	return dhcpv6c_get_milli_time() / 1000 - difp->last_update;
}

bool dhcpv6c_is_bound(struct dhcpv6c_interface *difp)
{
	return difp->bound;
}


#endif /*ZPL_DHCPV6C_MODULE*/