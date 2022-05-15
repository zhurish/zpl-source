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

#ifndef __NSM_INTERFACE_H__
#define __NSM_INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "vty.h"
#include "if_def.h"
#include "prefix.h"
#ifdef ZPL_NSM_IRDP
#include "nsm_irdp.h"
#endif

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


#if defined (ZPL_NSM_RTADV)
/* Router advertisement parameter.  From RFC4861, RFC6275 and RFC4191. */
struct nsm_rtadvconf
{
  /* A flag indicating whether or not the router sends periodic Router
     Advertisements and responds to Router Solicitations.
     Default: FALSE */
  int AdvSendAdvertisements;

  /* The maximum time allowed between sending unsolicited multicast
     Router Advertisements from the interface, in milliseconds.
     MUST be no less than 70 ms [RFC6275 7.5] and no greater
     than 1800000 ms [RFC4861 6.2.1].

     Default: 600000 milliseconds */
  int MaxRtrAdvInterval;
#define RTADV_MAX_RTR_ADV_INTERVAL 600000

  /* The minimum time allowed between sending unsolicited multicast
     Router Advertisements from the interface, in milliseconds.
     MUST be no less than 30 ms [RFC6275 7.5].
     MUST be no greater than .75 * MaxRtrAdvInterval.

     Default: 0.33 * MaxRtrAdvInterval */
  int MinRtrAdvInterval; /* This field is currently unused. */
#define RTADV_MIN_RTR_ADV_INTERVAL (0.33 * RTADV_MAX_RTR_ADV_INTERVAL)

  /* Unsolicited Router Advertisements' interval timer. */
  int AdvIntervalTimer;

  /* The TRUE/FALSE value to be placed in the "Managed address
     configuration" flag field in the Router Advertisement.  See
     [ADDRCONF].
 
     Default: FALSE */
  int AdvManagedFlag;


  /* The TRUE/FALSE value to be placed in the "Other stateful
     configuration" flag field in the Router Advertisement.  See
     [ADDRCONF].

     Default: FALSE */
  int AdvOtherConfigFlag;

  /* The value to be placed in MTU options sent by the router.  A
     value of zero indicates that no MTU options are sent.

     Default: 0 */
  int AdvLinkMTU;


  /* The value to be placed in the Reachable Time field in the Router
     Advertisement messages sent by the router.  The value zero means
     unspecified (by this router).  MUST be no greater than 3,600,000
     milliseconds (1 hour).

     Default: 0 */
  u_int32_t AdvReachableTime;
#define RTADV_MAX_REACHABLE_TIME 3600000


  /* The value to be placed in the Retrans Timer field in the Router
     Advertisement messages sent by the router.  The value zero means
     unspecified (by this router).

     Default: 0 */
  int AdvRetransTimer;

  /* The default value to be placed in the Cur Hop Limit field in the
     Router Advertisement messages sent by the router.  The value
     should be set to that current diameter of the Internet.  The
     value zero means unspecified (by this router).

     Default: The value specified in the "Assigned Numbers" RFC
     [ASSIGNED] that was in effect at the time of implementation. */
  int AdvCurHopLimit;

  /* The value to be placed in the Router Lifetime field of Router
     Advertisements sent from the interface, in seconds.  MUST be
     either zero or between MaxRtrAdvInterval and 9000 seconds.  A
     value of zero indicates that the router is not to be used as a
     default router.

     Default: 3 * MaxRtrAdvInterval */
  int AdvDefaultLifetime;
#define RTADV_MAX_RTRLIFETIME 9000 /* 2.5 hours */

  /* A list of prefixes to be placed in Prefix Information options in
     Router Advertisement messages sent from the interface.

     Default: all prefixes that the router advertises via routing
     protocols as being on-link for the interface from which the
     advertisement is sent. The link-local prefix SHOULD NOT be
     included in the list of advertised prefixes. */
  struct list *AdvPrefixList;

  /* The TRUE/FALSE value to be placed in the "Home agent"
     flag field in the Router Advertisement.  See [RFC6275 7.1].

     Default: FALSE */
  int AdvHomeAgentFlag;
#ifndef ND_RA_FLAG_HOME_AGENT
#define ND_RA_FLAG_HOME_AGENT 	0x20
#endif

  /* The value to be placed in Home Agent Information option if Home 
     Flag is set.
     Default: 0 */
  int HomeAgentPreference;

  /* The value to be placed in Home Agent Information option if Home 
     Flag is set. Lifetime (seconds) MUST not be greater than 18.2 
     hours. 
     The value 0 has special meaning: use of AdvDefaultLifetime value.
     
     Default: 0 */
  int HomeAgentLifetime;
#define RTADV_MAX_HALIFETIME 65520 /* 18.2 hours */

  /* The TRUE/FALSE value to insert or not an Advertisement Interval
     option. See [RFC 6275 7.3]

     Default: FALSE */
  int AdvIntervalOption;

  /* The value to be placed in the Default Router Preference field of
     a router advertisement. See [RFC 4191 2.1 & 2.2]

     Default: 0 (medium) */
  int DefaultPreference;
#define RTADV_PREF_MEDIUM 0x0 /* Per RFC4191. */
};

#endif /* ZPL_NSM_RTADV */


/* `zebra' daemon local interface structure. */
struct nsm_interface
{
	struct interface *ifp;
	/* Shutdown configuration. */
	zpl_uchar shutdown;

	nsm_duplex_en	duplex;
	nsm_speed_en	speed;

  /* Installed addresses chains tree. */
  //struct route_table *ipv4_subnets;

#if defined(ZPL_NSM_RTADV)
  struct nsm_rtadvconf rtadv;
#endif /* RTADV */

#ifdef ZPL_NSM_IRDP
  struct nsm_irdp irdp;
#endif

   /* Statistics fileds. */
   struct if_stats stats;

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
 

extern int nsm_interface_hook_handler(zpl_bool add, nsm_submodule_t module, struct interface *ifp);
extern int nsm_interface_create_hook(struct interface *ifp);

#ifdef __cplusplus
}
#endif


#endif /* __NSM_INTERFACE_H__ */
