/*
 * Common ioctl functions.
 * Copyright (C) 1998 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

#ifndef __LINUX_IOCTL_H__
#define __LINUX_IOCTL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "if.h"

extern void linux_ioctl_ifreq_set_name (struct ipstack_ifreq *ipstack_ifreq, struct interface *ifp);
extern int linux_ioctl_if_ioctl (zpl_uint32, caddr_t);
#ifdef ZPL_BUILD_IPV6
extern int linux_ioctl_if_ioctl_ipv6 (zpl_uint32 request, caddr_t buffer);
#endif

int linux_ioctl_if_set_up(struct interface *ifp);
int linux_ioctl_if_set_down(struct interface *ifp);
int linux_ioctl_if_update_flags(struct interface *ifp, uint64_t flag);
int linux_ioctl_if_get_flags(struct interface *ifp);
int linux_ioctl_if_set_mtu(struct interface *ifp, zpl_uint32 mtu);
int linux_ioctl_if_get_hwaddr(struct interface *ifp);
int linux_ioctl_if_set_mac(struct interface *ifp, zpl_uint8 *mac, zpl_uint32 len);
int linux_ioctl_if_set_metric(struct interface *ifp, zpl_uint32 metric);




#ifdef __cplusplus
}
#endif

#endif /* __LINUX_IOCTL_H__ */
