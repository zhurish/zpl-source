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


#include "if.h"
#include "prefix.h"
#ifdef ZPL_VRF_MODULE
#include "vrf.h"
#endif
#include "nsm_interface.h"

extern int nsm_halpal_interface_add(struct interface *ifp);
extern int nsm_halpal_interface_delete (struct interface *ifp);
extern int nsm_halpal_interface_up (struct interface *ifp);
extern int nsm_halpal_interface_down (struct interface *ifp);
extern int nsm_halpal_interface_change (struct interface *ifp);
extern int nsm_halpal_interface_refresh_flag(struct interface *ifp);
extern int nsm_halpal_interface_update_flag(struct interface *ifp, zpl_uint32 flags);
extern int nsm_halpal_interface_ifindex(char *k_name);

extern int nsm_halpal_interface_set_address (struct interface *ifp, struct connected *cp, zpl_bool secondry);
extern int nsm_halpal_interface_unset_address (struct interface *ifp, struct connected *cp, zpl_bool secondry);
extern int nsm_halpal_interface_set_dstaddr (struct interface *ifp, struct connected *cp, zpl_bool secondry);
extern int nsm_halpal_interface_unset_dstaddr (struct interface *ifp, struct connected *cp, zpl_bool secondry);

extern int nsm_halpal_interface_mac (struct interface *ifp, zpl_uchar *mac, zpl_uint32 len);
extern int nsm_halpal_interface_mtu (struct interface *ifp, zpl_uint32 mtu);
extern int nsm_halpal_interface_vrf (struct interface *ifp, struct ip_vrf *vrf);
extern int nsm_halpal_interface_multicast (struct interface *ifp, zpl_bool multicast);

extern int nsm_halpal_interface_bandwidth (struct interface *ifp, zpl_uint32 bandwidth);
extern int nsm_halpal_interface_speed (struct interface *ifp, nsm_speed_en );
extern int nsm_halpal_interface_get_statistics (struct interface *ifp);


extern int nsm_halpal_interface_vlan_set(struct interface *ifp, vlan_t vlan);
extern int nsm_halpal_interface_vlanpri_set(struct interface *ifp, zpl_uint32 pri);
extern int nsm_halpal_interface_promisc_link(struct interface *ifp, zpl_bool enable);

extern int nsm_halpal_interface_enca (struct interface *ifp, zpl_uint32 mode, zpl_uint32 value);
extern int nsm_halpal_interface_mode (struct interface *ifp, zpl_uint32 mode);


extern int nsm_halpal_interface_loop (struct interface *ifp, zpl_bool );

extern int nsm_halpal_interface_duplex (struct interface *ifp, nsm_duplex_en );


extern int nsm_halpal_create_vrf(struct ip_vrf *vrf);
extern int nsm_halpal_delete_vrf(struct ip_vrf *vrf);
extern int nsm_halpal_iproute_rib_action(struct prefix *p, struct rib *old, struct rib *new);



#ifdef ZPL_NSM_ARP
extern int nsm_halpal_interface_arp_add(struct interface *ifp, struct prefix *address, zpl_uint8 *mac);
extern int nsm_halpal_interface_arp_delete(struct interface *ifp, struct prefix *address);
extern int nsm_halpal_interface_arp_request(struct interface *ifp, struct prefix *address);
extern int nsm_halpal_arp_gratuitousarp_enable(zpl_bool enable);
extern int nsm_halpal_arp_ttl(zpl_uint32 ttl);
extern int nsm_halpal_arp_age_timeout(zpl_uint32 timeout);
extern int nsm_halpal_arp_retry_interval(zpl_uint32 interval);
#endif


#ifdef __cplusplus
}
#endif

#endif /* __NSM_PAL_H__ */
