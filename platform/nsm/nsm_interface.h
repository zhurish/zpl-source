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

#include <auto_include.h>
#include <zplos_include.h>
#include <module.h>
#include "vty.h"
#include "prefix.h"

#define NSM_ENTER_FUNC() zlog_debug(MODULE_NSM, "Into %s line %d", __func__, __LINE__)
#define NSM_LEAVE_FUNC() zlog_debug(MODULE_NSM, "Leave %s line %d", __func__, __LINE__)


typedef enum
{
	NSM_INTF_ALL = -1,
	NSM_INTF_NONE = 0,
	NSM_INTF_PORT,
	NSM_INTF_TRUNK,
	NSM_INTF_VLAN,
	NSM_INTF_MAC,
	NSM_INTF_DOT1X,
	NSM_INTF_SEC,	//security
	NSM_INTF_QOS,
	NSM_INTF_ACL,
	NSM_INTF_MIRROR,
	NSM_INTF_PPP,
	NSM_INTF_PPPOE,
	NSM_INTF_SERIAL,
	//NSM_INTF_DHCP,
	//NSM_INTF_DHCPS,

	NSM_INTF_WIFI,

	NSM_INTF_VETH,	// tun/tap interface
	NSM_INTF_TUNNEL,	//tunnel interface
	NSM_INTF_BRIDGE, //bridge interface

	NSM_INTF_KERENL,
	NSM_INTF_MAX,
}nsm_submodule_t;


/* For interface shutdown configuration. */
#define IF_ZEBRA_SHUTDOWN_OFF    0
#define IF_ZEBRA_SHUTDOWN_ON     1


#define IF_ZEBRA_MTU_DEFAULT	1500


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

struct nsm_interface_cb
{
	int (*nsm_intf_add_cb)(struct interface *);
	int (*nsm_intf_del_cb)(struct interface *);
	int (*nsm_intf_updown_cb)(struct interface *, zpl_bool);
	int (*nsm_intf_write_cb)(struct vty *, struct interface *);
};

/* `zebra' daemon local interface structure. */
struct nsm_interface
{
	struct interface *ifp;
	/* Shutdown configuration. */
	zpl_uchar shutdown;

	nsm_duplex_en	duplex;
	nsm_speed_en	speed;


	void *nsm_intf_data[NSM_INTF_MAX];
};

extern void nsm_interface_init(void);

extern void *nsm_intf_module_data(struct interface *ifp, nsm_submodule_t mid);
extern int nsm_intf_module_data_set(struct interface *ifp, nsm_submodule_t mid, void *p);

extern int nsm_interface_hook_add(nsm_submodule_t module, int (*add_cb)(struct interface *), int (*del_cb)(struct interface *));
extern int nsm_interface_write_hook_add(nsm_submodule_t module, int (*show_cb)(struct vty *, struct interface *));
extern int nsm_interface_write_hook_handler(nsm_submodule_t module, struct vty *vty, struct interface *ifp);

#ifdef ZPL_SHELL_MODULE
extern zpl_bool nsm_interface_create_check_api(struct vty *vty, const char *ifname, const char *uspv);
#endif
extern int nsm_interface_create_api(const char *ifname);
extern int nsm_interface_delete_api(struct interface *ifp);

extern int nsm_interface_mode_set_api(struct interface *ifp, if_mode_t mode);
extern int nsm_interface_mode_get_api(struct interface *ifp, if_mode_t *mode);
extern int nsm_interface_enca_set_api(struct interface *ifp, if_enca_t enca, zpl_uint16 value);
extern int nsm_interface_enca_get_api(struct interface *ifp, if_enca_t *enca, zpl_uint16 *value);
extern int nsm_interface_desc_set_api(struct interface *ifp, const char *desc);
extern int nsm_interface_down_set_api(struct interface *ifp);
extern int nsm_interface_up_set_api(struct interface *ifp);


extern int nsm_interface_address_set_api(struct interface *ifp, struct prefix *cp, zpl_bool secondry);
extern int nsm_interface_address_unset_api(struct interface *ifp, struct prefix *cp, zpl_bool secondry);
extern int nsm_interface_address_get_api(struct interface *ifp, struct prefix *address);
extern int nsm_interface_ip_address_add(struct interface *ifp, struct prefix *cp,
		zpl_bool secondary, zpl_uint32 value);
extern int nsm_interface_ip_address_del(struct interface *ifp, struct prefix *cp,
		zpl_bool secondary, zpl_uint32 value);

extern int nsm_interface_statistics_get_api(struct interface *ifp, struct if_stats *stats);

extern int nsm_interface_bandwidth_set_api(struct interface *ifp, zpl_uint32  bandwidth);
extern int nsm_interface_bandwidth_get_api(struct interface *ifp, zpl_uint32  *bandwidth);
extern int nsm_interface_vrf_set_api(struct interface *ifp, vrf_id_t vrf_id);
extern int nsm_interface_vrf_get_api(struct interface *ifp, vrf_id_t *vrf_id);
extern int nsm_interface_metric_set_api(struct interface *ifp, zpl_uint32 metric);
extern int nsm_interface_metric_get_api(struct interface *ifp, zpl_uint32 *metric);
extern int nsm_interface_mtu_set_api(struct interface *ifp, zpl_uint32 mtu);
extern int nsm_interface_mtu_get_api(struct interface *ifp, zpl_uint32 *mtu);
extern int nsm_interface_mac_set_api(struct interface *ifp, zpl_uchar *mac, zpl_uint32 maclen);
extern int nsm_interface_mac_get_api(struct interface *ifp, zpl_uchar *mac, zpl_uint32 maclen);
extern int nsm_interface_duplex_set_api(struct interface *ifp, nsm_duplex_en duplex);
extern int nsm_interface_duplex_get_api(struct interface *ifp, nsm_duplex_en *duplex);
extern int nsm_interface_speed_set_api(struct interface *ifp, nsm_speed_en speed);
extern int nsm_interface_speed_get_api(struct interface *ifp, nsm_speed_en *speed);

extern int nsm_interface_update_kernel(struct interface *ifp, zpl_char *kname);
extern int nsm_interface_update_api(struct interface *ifp);
extern int nsm_interface_hw_update_api(struct interface *ifp);
#ifdef ZPL_SHELL_MODULE
extern void nsm_interface_show_api(struct vty *vty, struct interface *ifp);
extern void nsm_interface_show_brief_api(struct vty *vty, struct interface *ifp, zpl_bool status, zpl_bool *head);
extern void cmd_interface_init(void);
#endif
 
#ifdef __cplusplus
}
#endif


#endif /* _ZEBRA_INTERFACE_H */
