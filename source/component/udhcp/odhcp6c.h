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

#ifndef __ODHCP6C_H__
#define __ODHCP6C_H__

#ifdef __cplusplus
extern "C" {
#endif




int script_init(const char *path, const char *ifname);
ssize_t script_unhexlify(uint8_t *dst, size_t len, const char *src);
void script_call(const char *status, int delay, bool resume);

bool dhcpv6c_signal_process(void);

int dhcpv6c_random(void *buf, size_t len);



// Entry manipulation


int dhcpv6_main(int argc, char* const argv[]);

#endif /*ZPL_DHCPV6C_MODULE*/
#ifdef __cplusplus
}
#endif

#endif /* __ODHCP6C_H__ */