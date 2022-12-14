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

#ifndef __DHCPV6C_API_H__
#define __DHCPV6C_API_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ZPL_DHCPV6C_MODULE

#include "dhcpv6c.h"





bool dhcpv6c_addr_in_scope(struct dhcpv6c_interface *difp, const struct in6_addr *addr);

uint64_t dhcpv6c_get_milli_time(void);
bool dhcpv6c_is_bound(struct dhcpv6c_interface *);

// Entry manipulation
bool dhcpv6c_update_entry(struct dhcpv6c_interface *, enum dhcpv6c_state state, struct dhcpv6c_entry *new,
				uint32_t safe, unsigned int holdoff_interval);

void dhcpv6c_expire(struct dhcpv6c_interface *, bool expire_ia_pd);
uint32_t dhcpv6c_elapsed(struct dhcpv6c_interface *);


#endif /*ZPL_DHCPV6C_MODULE*/

#ifdef __cplusplus
}
#endif

#endif /* __DHCPV6C_API_H__ */