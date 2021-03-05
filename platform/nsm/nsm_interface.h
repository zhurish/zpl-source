/* Interface function header.
 * Copyright (C) 1999 Kunihiro Ishiguro
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

#ifndef _ZEBRA_INTERFACE_H
#define _ZEBRA_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nsm_redistribute.h"

#include "nsm_client.h"




/* For interface multicast configuration. */
#define IF_ZEBRA_MULTICAST_UNSPEC 0
#define IF_ZEBRA_MULTICAST_ON     1
#define IF_ZEBRA_MULTICAST_OFF    2

/* For interface shutdown configuration. */
#define IF_ZEBRA_SHUTDOWN_OFF    0
#define IF_ZEBRA_SHUTDOWN_ON     1


#define IF_ZEBRA_MTU_DEFAULT	1500


/* Global user-configured default for interface link-detect */
typedef enum {
  IF_LINKDETECT_UNSPEC = 0,
  IF_LINKDETECT_ON,
  IF_LINKDETECT_OFF,
} nsm_linkdetect_en;

typedef enum {
  NSM_IF_DUPLEX_NONE = 0,
  NSM_IF_DUPLEX_AUTO,
  NSM_IF_DUPLEX_FULL,
  NSM_IF_DUPLEX_HALF,
} nsm_duplex_en;

typedef enum {
	NSM_IF_SPEED_NONE = 0,
	NSM_IF_SPEED_AUTO,
	NSM_IF_SPEED_10M,
	NSM_IF_SPEED_100M,
	NSM_IF_SPEED_1000M,
	NSM_IF_SPEED_10000M,
	NSM_IF_SPEED_1000M_B,
	NSM_IF_SPEED_1000M_MP,
	NSM_IF_SPEED_1000M_MP_NO_FIBER,

} nsm_speed_en;

/* `zebra' daemon local interface structure. */
struct nsm_interface
{
  struct interface *ifp;
  /* Shutdown configuration. */
  ospl_uchar shutdown;

  /* Multicast configuration. */
  ospl_uchar multicast;
  
  nsm_duplex_en	duplex;
  nsm_speed_en	speed;

  /* Interface specific link-detect configuration state */
  nsm_linkdetect_en linkdetect;
  
  void *nsm_client[NSM_MAX];

/*  void	*vlan;

  void	*l2switch;

  void *ipkernel;*/

};

extern void nsm_interface_init(void);
extern void cmd_interface_init(void);



extern ospl_bool nsm_interface_create_check_api(struct vty *vty, const char *ifname, const char *uspv);
extern int nsm_interface_create_api(const char *ifname);
extern int nsm_interface_delete_api(struct interface *ifp);

extern int nsm_interface_mode_set_api(struct interface *ifp, if_mode_t mode);
extern int nsm_interface_mode_get_api(struct interface *ifp, if_mode_t *mode);
extern int nsm_interface_enca_set_api(struct interface *ifp, if_enca_t enca, ospl_uint16 value);
extern int nsm_interface_enca_get_api(struct interface *ifp, if_enca_t *enca, ospl_uint16 *value);
extern int nsm_interface_desc_set_api(struct interface *ifp, const char *desc);
extern int nsm_interface_down_set_api(struct interface *ifp);
extern int nsm_interface_up_set_api(struct interface *ifp);

extern int nsm_interface_linkdetect_set_api(struct interface *ifp, nsm_linkdetect_en linkdetect);
extern int nsm_interface_linkdetect_get_api(struct interface *ifp, nsm_linkdetect_en *linkdetect);
extern int nsm_interface_address_set_api(struct interface *ifp, struct prefix *cp, ospl_bool secondry);
extern int nsm_interface_address_unset_api(struct interface *ifp, struct prefix *cp, ospl_bool secondry);
extern int nsm_interface_address_get_api(struct interface *ifp, struct prefix *address);
extern int nsm_interface_ip_address_add(struct interface *ifp, struct prefix *cp,
		ospl_bool secondary, ospl_uint32 value);
extern int nsm_interface_ip_address_del(struct interface *ifp, struct prefix *cp,
		ospl_bool secondary, ospl_uint32 value);

extern int nsm_interface_statistics_get_api(struct interface *ifp, struct if_stats *stats);
extern int nsm_interface_multicast_set_api(struct interface *ifp, ospl_bool enable);
extern int nsm_interface_multicast_get_api(struct interface *ifp, ospl_bool *enable);
extern int nsm_interface_bandwidth_set_api(struct interface *ifp, ospl_uint32  bandwidth);
extern int nsm_interface_bandwidth_get_api(struct interface *ifp, ospl_uint32  *bandwidth);
extern int nsm_interface_vrf_set_api(struct interface *ifp, vrf_id_t vrf_id);
extern int nsm_interface_vrf_get_api(struct interface *ifp, vrf_id_t *vrf_id);
extern int nsm_interface_metric_set_api(struct interface *ifp, ospl_uint32 metric);
extern int nsm_interface_metric_get_api(struct interface *ifp, ospl_uint32 *metric);
extern int nsm_interface_mtu_set_api(struct interface *ifp, ospl_uint32 mtu);
extern int nsm_interface_mtu_get_api(struct interface *ifp, ospl_uint32 *mtu);
extern int nsm_interface_mac_set_api(struct interface *ifp, ospl_uchar *mac, ospl_uint32 maclen);
extern int nsm_interface_mac_get_api(struct interface *ifp, ospl_uchar *mac, ospl_uint32 maclen);
extern int nsm_interface_duplex_set_api(struct interface *ifp, nsm_duplex_en duplex);
extern int nsm_interface_duplex_get_api(struct interface *ifp, nsm_duplex_en *duplex);
extern int nsm_interface_speed_set_api(struct interface *ifp, nsm_speed_en speed);
extern int nsm_interface_speed_get_api(struct interface *ifp, nsm_speed_en *speed);

extern int nsm_interface_update_kernel(struct interface *ifp, ospl_char *kname);
extern int nsm_interface_update_api(struct interface *ifp);
extern int nsm_interface_hw_update_api(struct interface *ifp);

extern void nsm_interface_show_api(struct vty *vty, struct interface *ifp);
extern void nsm_interface_show_brief_api(struct vty *vty, struct interface *ifp, ospl_bool status, ospl_bool *head);


extern int nsm_port_interface_config(struct vty *vty, struct interface *ifp);
extern void cmd_port_init (void);


 
#ifdef __cplusplus
}
#endif


#endif /* _ZEBRA_INTERFACE_H */
