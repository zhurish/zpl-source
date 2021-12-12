/* NSM client header.
 * Copyright (C) 1999 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __NSM_PAL_H__
#define __NSM_PAL_H__

#ifdef __cplusplus
extern "C" {
#endif

/* For struct zapi_ipv{4,6}. */
#include "if.h"
#include "prefix.h"
#include "nsm_vrf.h"


extern int nsm_pal_interface_add(struct interface *ifp);
extern int nsm_pal_interface_delete (struct interface *ifp);
extern int nsm_pal_interface_up (struct interface *ifp);
extern int nsm_pal_interface_down (struct interface *ifp);

extern int nsm_pal_interface_set_address (struct interface *ifp, struct connected *cp, zpl_bool secondry);
extern int nsm_pal_interface_unset_address (struct interface *ifp, struct connected *cp, zpl_bool secondry);
extern int nsm_pal_interface_mac (struct interface *ifp, zpl_uchar *mac, zpl_uint32 len);
extern int nsm_pal_interface_mtu (struct interface *ifp, zpl_uint32 mtu);
extern int nsm_pal_interface_vrf (struct interface *ifp, zpl_uint32 vrf);
extern int nsm_pal_interface_multicast (struct interface *ifp, zpl_uint32 multicast);

extern int nsm_pal_interface_bandwidth (struct interface *ifp, zpl_uint32 bandwidth);
extern int nsm_pal_interface_speed (struct interface *ifp, zpl_uint32 );
extern int nsm_pal_interface_get_statistics (struct interface *ifp);

//hal
extern int nsm_pal_interface_enca (struct interface *ifp, zpl_uint32 mode, zpl_uint32 value);
extern int nsm_pal_interface_mode (struct interface *ifp, zpl_uint32 mode);
extern int nsm_pal_interface_metric (struct interface *ifp, zpl_uint32 metric);
extern int nsm_pal_interface_linkdetect (struct interface *ifp, zpl_uint32 link);

extern int nsm_pal_interface_stp (struct interface *ifp, zpl_uint32 );
extern int nsm_pal_interface_loop (struct interface *ifp, zpl_uint32 );
extern int nsm_pal_interface_8021x (struct interface *ifp, zpl_uint32 );
extern int nsm_pal_interface_duplex (struct interface *ifp, zpl_uint32 );


 
#ifdef __cplusplus
}
#endif

#endif /* __NSM_PAL_H__ */
