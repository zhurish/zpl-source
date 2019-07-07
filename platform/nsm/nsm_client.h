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

#ifndef __NSM_CLIENT_H__
#define __NSM_CLIENT_H__

/* For struct zapi_ipv{4,6}. */
#include "if.h"
#include "prefix.h"
#include "vrf.h"


enum
{
	NSM_NONE = 0,
	NSM_PORT,
	NSM_TRUNK,
	NSM_VLAN,
	NSM_MAC,
	NSM_DOT1X,
	NSM_SEC,	//security
	NSM_QOS,
	NSM_ACL,
	NSM_MIRROR,
	NSM_PPP,
	NSM_PPPOE,
	NSM_SERIAL,
	NSM_DHCP,
	//NSM_DHCPS,

	NSM_WIFI,

	NSM_VETH,	// tun/tap interface
	NSM_TUNNEL,	//tunnel interface
	NSM_BRIDGE, //bridge interface

	NSM_KERENL,
	NSM_MAX,
};

/* Structure for the zebra client. */

struct nsm_client
{
  int module;
  int (*write_config_cb) (struct vty *);
  int (*service_write_config_cb) (struct vty *);
  int (*debug_write_config_cb) (struct vty *);
  int (*interface_write_config_cb) (struct vty *, struct interface *);

  //接口创建删除的时候触发创建删除对应模块的数据结构
  int (*notify_add_cb) (struct interface *);
  int (*notify_delete_cb) (struct interface *);

  //接口UP/DOWN，设置删除IP的时候通知其他模块
  int (*notify_up_cb) (struct interface *);
  int (*notify_down_cb) (struct interface *);

  int (*notify_address_add_cb) (struct interface *, struct connected *, int );
  int (*notify_address_del_cb) (struct interface *, struct connected *, int );

  int (*notify_parameter_change_cb) (struct interface *);
};



extern struct nsm_client *nsm_client_new (void);
extern void nsm_client_install (struct nsm_client *, int);
extern struct nsm_client * nsm_client_lookup (int module);
extern void nsm_client_init (void);
extern void nsm_client_free (struct nsm_client *client);


extern int nsm_client_write_config (int module, struct vty *vty);
extern int nsm_client_service_write_config (int module, struct vty *vty);
extern int nsm_client_debug_write_config (int module, struct vty *vty);
extern int nsm_client_interface_write_config (int module, struct vty *vty, struct interface *ifp);


extern int nsm_client_notify_interface_add(struct interface *ifp);
extern int nsm_client_notify_interface_delete (struct interface *ifp);
extern int nsm_client_notify_interface_up (struct interface *ifp);
extern int nsm_client_notify_interface_down (struct interface *ifp);
extern int nsm_client_notify_interface_add_ip (struct interface *ifp, struct connected *, int );
extern int nsm_client_notify_interface_del_ip (struct interface *ifp, struct connected *, int );

extern int nsm_client_notify_parameter_change (struct interface *ifp);


extern int nsm_pal_interface_add(struct interface *ifp);
extern int nsm_pal_interface_delete (struct interface *ifp);
extern int nsm_pal_interface_up (struct interface *ifp);
extern int nsm_pal_interface_down (struct interface *ifp);

extern int nsm_pal_interface_set_address (struct interface *ifp, struct connected *cp, int secondry);
extern int nsm_pal_interface_unset_address (struct interface *ifp, struct connected *cp, int secondry);
extern int nsm_pal_interface_mac (struct interface *ifp, unsigned char *mac, int len);
extern int nsm_pal_interface_mtu (struct interface *ifp, int mtu);
extern int nsm_pal_interface_vrf (struct interface *ifp, int vrf);
extern int nsm_pal_interface_multicast (struct interface *ifp, int multicast);

extern int nsm_pal_interface_bandwidth (struct interface *ifp, int bandwidth);
extern int nsm_pal_interface_speed (struct interface *ifp, int );
extern int nsm_pal_interface_get_statistics (struct interface *ifp);

//hal
extern int nsm_pal_interface_enca (struct interface *ifp, int mode, int value);
extern int nsm_pal_interface_mode (struct interface *ifp, int mode);
extern int nsm_pal_interface_metric (struct interface *ifp, int metric);
extern int nsm_pal_interface_linkdetect (struct interface *ifp, int link);

extern int nsm_pal_interface_stp (struct interface *ifp, int );
extern int nsm_pal_interface_loop (struct interface *ifp, int );
extern int nsm_pal_interface_8021x (struct interface *ifp, int );
extern int nsm_pal_interface_duplex (struct interface *ifp, int );



#endif /* __NSM_CLIENT_H__ */
