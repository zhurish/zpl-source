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
#include "zpl_type.h"
#ifdef ZPL_DHCPV6C_MODULE
#include "resolv.h"
#include "dhcpv6c.h"
#include "dhcpv6c_state.h"
#include "rtadv.h"


static uint8_t* dhcpv6c_resize_state(struct dhcpv6c_interface *ifp, enum dhcpv6c_state state, ssize_t len)
{
	if (len == 0)
		return ifp->state_data[state] + ifp->state_len[state];
	else if (ifp->state_len[state] + len > 1024)
		return NULL;

	uint8_t *n = realloc(ifp->state_data[state], ifp->state_len[state] + len);

	if (n || ifp->state_len[state] + len == 0) {
		ifp->state_data[state] = n;
		n += ifp->state_len[state];
		ifp->state_len[state] += len;
	}

	return n;
}


void dhcpv6c_state_clear(struct dhcpv6c_interface *ifp, enum dhcpv6c_state state)
{
	ifp->state_len[state] = 0;
}

int dhcpv6c_state_add(struct dhcpv6c_interface *ifp, enum dhcpv6c_state state, const void *data, size_t len)
{
	uint8_t *n = dhcpv6c_resize_state(ifp, state, len);

	if (!n)
		return -1;

	memcpy(n, data, len);

	return 0;
}

int dhcpv6c_state_insert(struct dhcpv6c_interface *ifp, enum dhcpv6c_state state, size_t offset, const void *data, size_t len)
{
	ssize_t len_after =ifp-> state_len[state] - offset;
	if (len_after < 0)
		return -1;

	uint8_t *n = dhcpv6c_resize_state(ifp, state, len);

	if (n) {
		uint8_t *sdata = ifp->state_data[state];

		memmove(sdata + offset + len, sdata + offset, len_after);
		memcpy(sdata + offset, data, len);
	}

	return 0;
}

size_t dhcpv6c_state_remove(struct dhcpv6c_interface *ifp, enum dhcpv6c_state state, size_t offset, size_t len)
{
	uint8_t *data = ifp->state_data[state];
	ssize_t len_after = ifp->state_len[state] - (offset + len);

	if (len_after < 0)
		return ifp->state_len[state];

	memmove(data + offset, data + offset + len, len_after);

	return ifp->state_len[state] -= len;
}

void* dhcpv6c_state_move(struct dhcpv6c_interface *ifp, enum dhcpv6c_state state, size_t *len)
{
	*len = ifp->state_len[state];
	void *data = ifp->state_data[state];

	ifp->state_len[state] = 0;
	ifp->state_data[state] = NULL;

	return data;
}

void* dhcpv6c_state_get(struct dhcpv6c_interface *ifp, enum dhcpv6c_state state, size_t *len)
{
	*len = ifp->state_len[state];
	return ifp->state_data[state];
}


uint8_t *dhcpv6c_state_find_option(struct dhcpv6c_interface *ifp, const uint16_t code)
{
	size_t opts_len;
	uint8_t *odata, *_o, *popts = dhcpv6c_state_get(ifp, DHCPV6C_STATE_OPTS, &opts_len);
	uint16_t otype, olen;

	dhcpv6c_for_each_option(popts, &popts[opts_len], otype, olen, odata) {
		if (otype == code)
			return &odata[-4];
	}

	return NULL;
}

#endif /*ZPL_DHCPV6C_MODULE*/