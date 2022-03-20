/* Interface related header.
   Copyright (C) 1997, 98, 99 Kunihiro Ishiguro

This file is part of GNU Zebra.

GNU Zebra is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 2, or (at your
option) any later version.

GNU Zebra is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Zebra; see the file COPYING.  If not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#ifndef _ZEBRA_IF_H
#define _ZEBRA_IF_H

#ifdef __cplusplus
extern "C" {
#endif


#include "log.h"
#include "linklist.h"

/* Interface link-layer type, if known. Derived from:
 *
 * net/if_arp.h on various platforms - Linux especially.
 * http://www.iana.org/assignments/arp-parameters/arp-parameters.xhtml
 *
 * Some of the more obviously defunct technologies left out.
 */
enum if_link_type
{
   IF_LLT_UNKNOWN = 0,
   IF_LLT_LOOPBACK,
   IF_LLT_ETHER,
   IF_LLT_GIETHER,
   IF_LLT_TUNNEL,
   IF_LLT_VLAN,
   IF_LLT_LAG,
   IF_LLT_SERIAL,
   IF_LLT_SLIP,
   IF_LLT_CSLIP,
   IF_LLT_SLIP6,
   IF_LLT_CSLIP6,

   IF_LLT_PPP,
   IF_LLT_CHDLC,
   IF_LLT_RAWHDLC,
   IF_LLT_IPIP,
   IF_LLT_IPIP6,
   IF_LLT_IPGRE,
   IF_LLT_IP6GRE,

   IF_LLT_ATM,
   IF_LLT_SIT,
   IF_LLT_BRIGDE,
   IF_LLT_WIFI,
   IF_LLT_MODEM,
   IF_LLT_WIRELESS,
};

/*
  Interface name length.

   Linux define value in /usr/include/linux/if.h.
   #define IFNAMSIZ        16

   FreeBSD define value in /usr/include/net/if.h.
   #define IFNAMSIZ        16
*/

#define INTERFACE_NAMSIZ 64
#define INTERFACE_HWADDR_MAX 20
#define MAX_CLASS_TYPE 8

#define IF_LOOPBACK_MAX 16
#define IF_VLAN_MAX 256
#define IF_LAG_MAX 16
#define IF_TUNNEL_SLOT 0
#define IF_TUNNEL_MAX 256
#define IF_BRIGDE_MAX 16

#define IF_MTU_DEFAULT 1500

#define IF_DATA_LOCK()   //if_data_lock()
#define IF_DATA_UNLOCK() //if_data_unlock()

typedef enum
{
   IF_TTY_USB = 1,
   IF_TTY_CAM,
   IF_TTY_S,
} tty_type_en;



struct if_stats
{
   zpl_ulong rx_packets;   /* total packets received       */
   zpl_ulong tx_packets;   /* total packets transmitted    */
   zpl_ulong rx_bytes;     /* total bytes received         */
   zpl_ulong tx_bytes;     /* total bytes transmitted      */
   zpl_ulong rx_errors;    /* bad packets received         */
   zpl_ulong tx_errors;    /* packet transmit problems     */
   zpl_ulong rx_dropped;   /* no space in linux buffers    */
   zpl_ulong tx_dropped;   /* no space available in linux  */
   zpl_ulong rx_multicast; /* multicast packets received   */
   zpl_ulong collisions;

   /* detailed rx_errors: */
   zpl_ulong rx_length_errors;
   zpl_ulong rx_over_errors;   /* receiver ring buff overflow  */
   zpl_ulong rx_crc_errors;    /* recved pkt with crc error    */
   zpl_ulong rx_frame_errors;  /* recv'd frame alignment error */
   zpl_ulong rx_fifo_errors;   /* recv'r fifo overrun          */
   zpl_ulong rx_missed_errors; /* receiver missed packet     */
   /* detailed tx_errors */
   zpl_ulong tx_aborted_errors;
   zpl_ulong tx_carrier_errors;
   zpl_ulong tx_fifo_errors;
   zpl_ulong tx_heartbeat_errors;
   zpl_ulong tx_window_errors;
   /* for cslip etc */
   zpl_ulong rx_compressed;
   zpl_ulong tx_compressed;
};

typedef enum if_type_s
{
   IF_NONE,
   IF_LOOPBACK,
   IF_SERIAL,
   IF_ETHERNET,
   IF_GIGABT_ETHERNET,
   IF_XGIGABT_ETHERNET,
   IF_ETHERNET_SUB,
   IF_WIRELESS, //wireless interface
   IF_TUNNEL,
   IF_LAG,
   IF_BRIGDE, //brigde interface
   IF_VLAN,
   IF_E1,
#ifdef CUSTOM_INTERFACE
   IF_WIFI,  //wifi interface wireless
   IF_MODEM, //modem interface
#endif
   IF_EPON,
   IF_GPON,
   IF_MAX
} if_type_t;

typedef enum if_mode_s
{
   IF_MODE_NONE,
   IF_MODE_ACCESS_L2,
   IF_MODE_TRUNK_L2,
   IF_MODE_L3,
   IF_MODE_BRIGDE,
} if_mode_t;

#define IF_MODE_DEFAULT IF_MODE_ACCESS_L2

typedef enum if_enca_s
{
   IF_ENCA_NONE,
   IF_ENCA_DOT1Q,        //VLAN
   IF_ENCA_DOT1Q_TUNNEL, //QINQ
   IF_ENCA_PPP,          //PPP
   IF_ENCA_PPPOE,        //PPPOE
   IF_ENCA_SLIP,         //SLIP
   IF_ENCA_GRE,          //GRE
   IF_ENCA_IPIP,         //IPIP
   IF_ENCA_IPIPV6,       //IPV4-IPV6
   IF_ENCA_IPV6IP,       //IPV6-IPV4
   IF_ENCA_SIT,          //SIT
   IF_ENCA_HDLC,         //HDLC
   IF_ENCA_RAW,          //RAW
} if_enca_t;

/* Interface structure */
struct interface
{
   /* Interface name.  This should probably never be changed after the
     interface is created, because the configuration info for this interface
     is associated with this structure.  For that reason, the interface
     should also never be deleted (to avoid losing configuration info).
     To delete, just set ifindex to IFINDEX_INTERNAL to indicate that the
     interface does not exist in the kernel.
   */
   zpl_char name[INTERFACE_NAMSIZ + 1];
   zpl_uint32  name_hash;
   /* Interface index (should be IFINDEX_INTERNAL for non-kernel or
     deleted interfaces). */
   ifindex_t ifindex;
#define IFINDEX_INTERNAL 0

   zpl_uint32  uspv;
   zpl_vlan_t encavlan; //子接口封装的VLAN ID

   zpl_bool have_kernel;
   zpl_char k_name[INTERFACE_NAMSIZ + 1];
   zpl_uint32  k_name_hash;
   ifindex_t k_ifindex;

   zpl_phyport_t  phyid;   //L2 phy port
	zpl_phyport_t  l3intfid;//L3 intf id
#define IFPHYID_INTERNAL -1
   if_type_t if_type;

   if_mode_t if_mode;
   if_enca_t if_enca;
   zpl_bool dynamic;

   zpl_bool online;  //板卡在线状态
   /* Zebra internal interface status */
   zpl_uint32 status;
#define ZEBRA_INTERFACE_ACTIVE (1 << 0)
#define ZEBRA_INTERFACE_LINKDETECTION (1 << 2)
#define ZEBRA_INTERFACE_ATTACH (1 << 3)
   /* Interface flags. */
   zpl_uint64 flags;

   /* Interface metric */
   zpl_uint32 metric;

   /* Interface MTU. */
   zpl_uint32  mtu;  /* IPv4 MTU */
   zpl_uint32  mtu6; /* IPv6 MTU - probably, but not neccessarily same as mtu */

   /* Link-layer information and hardware address */
   enum if_link_type ll_type;
   zpl_uchar hw_addr[INTERFACE_HWADDR_MAX];
   zpl_uint32 hw_addr_len;

   /* interface bandwidth, kbits */
   zpl_uint32  bandwidth;

   /* description of the interface. */
   zpl_char *desc;

   /* Distribute list. */
   void *distribute_in;
   void *distribute_out;

   /* Connected address list. */
   struct list *connected;
   zpl_bool dhcp;
   /* Daemon specific interface data pointer. */
   //void *info[ZLOG_MAX];
   void *info[MODULE_MAX];

   /* Statistics fileds. */
   struct if_stats stats;

   vrf_id_t vrf_id;

   zpl_uint32  ifmember;
#define IF_TRUNK_MEM (1 << 0)
#define IF_BRIDGE_MEM (1 << 2)

   zpl_uint32 count;
   zpl_uint32 raw_status;
};

/* Connected address structure. */
struct connected
{
   /* Attached interface. */
   struct interface *ifp;

   /* Flags for configuration. */
   zpl_uchar conf;
#define ZEBRA_IFC_CONFIGURED (1 << 1)
#define ZEBRA_IFC_DHCPC (1 << 2)
   /*
     The ZEBRA_IFC_REAL flag should be set if and only if this address
     exists in the kernel and is actually usable. (A case where it exists but
     is not yet usable would be IPv6 with DAD)
     The ZEBRA_IFC_CONFIGURED flag should be set if and only if this address
     was configured by the user from inside quagga.
     The ZEBRA_IFC_QUEUED flag should be set if and only if the address exists
     in the kernel. It may and should be set although the address might not be
     usable yet. (compare with ZEBRA_IFC_REAL)
   */

   /* Flags for connected address. */
   zpl_uchar flags;
#define ZEBRA_IFA_SECONDARY (1 << 0)
#define ZEBRA_IFA_PEER (1 << 1)
#define ZEBRA_IFA_UNNUMBERED (1 << 2)
#define ZEBRA_IFA_DHCPC (1 << 3)
   /* N.B. the ZEBRA_IFA_PEER flag should be set if and only if
     a peer address has been configured.  If this flag is set,
     the destination field must contain the peer address.  
     Otherwise, if this flag is not set, the destination address
     will either contain a broadcast address or be NULL.
   */

   /* Address of connected network. */
   struct prefix *address;

   /* Peer or Broadcast address, depending on whether ZEBRA_IFA_PEER is set.
     Note: destination may be NULL if ZEBRA_IFA_PEER is not set. */
   struct prefix *destination;

   zpl_uint32 count;
   zpl_uint32 raw_status;
};


#define IF_UNIT_ALL (0xEFFFFFFF)

#define IF_PORT_MAX_256 (256) //单板最大同类接口数量
#define IF_PORT_MAX_128 (128) //单板最大同类接口数量
#define IF_PORT_MAX_64 (64) //单板最大同类接口数量

#define IF_PORT_MAX  IF_PORT_MAX_64

#if IF_PORT_MAX == IF_PORT_MAX_256
#define IF_TYPE_GET(n) (((n) >> 27) & 0x1F)  //0-31, 32
#define IF_UNIT_GET(n) (((n) >> 24) & 0x07)  //0-7, 8
#define IF_SLOT_GET(n) (((n) >> 20) & 0x0F)  //0-15, 16
#define IF_PORT_GET(n) (((n) >> 12) & 0xFF)  //0-255, 256

#define IF_TYPE_SET(n) (((n)&0x1F) << 27)
#define IF_TYPE_CLR(n) (((n)) & 0x07FFFFFF)
#define IF_USPV_SET(u, s, p, v) (((u)&0x07) << 24) | (((s)&0x0F) << 20) | (((p)&0xFF) << 12) | ((v)&0x0FFF)

#elif IF_PORT_MAX == IF_PORT_MAX_128

#define IF_TYPE_GET(n) (((n) >> 26) & 0x3F)  //0-63, 64
#define IF_UNIT_GET(n) (((n) >> 23) & 0x07)  //0-7, 8
#define IF_SLOT_GET(n) (((n) >> 19) & 0x0F)  //0-15, 16
#define IF_PORT_GET(n) (((n) >> 12) & 0x7F)  //0-127, 128

#define IF_TYPE_SET(n) (((n)&0x3F) << 26)
#define IF_TYPE_CLR(n) (((n)) & 0x03FFFFFF)
#define IF_USPV_SET(u, s, p, v) (((u)&0x07) << 23) | (((s)&0x0F) << 19) | (((p)&0x7F) << 12) | ((v)&0x0FFF)

#elif IF_PORT_MAX == IF_PORT_MAX_64

#define IF_TYPE_GET(n) (((n) >> 26) & 0x3F)  //0-63, 64
#define IF_UNIT_GET(n) (((n) >> 23) & 0x07)  //0-7, 8
#define IF_SLOT_GET(n) (((n) >> 18) & 0x1F)  //0-31, 32
#define IF_PORT_GET(n) (((n) >> 12) & 0x3F)  //0-63, 64

#define IF_TYPE_SET(n) (((n)&0x3F) << 26)
#define IF_TYPE_CLR(n) (((n)) & 0x03FFFFFF)
#define IF_USPV_SET(u, s, p, v) (((u)&0x07) << 23) | (((s)&0x1F) << 18) | (((p)&0x3F) << 12) | ((v)&0x0FFF)
#endif

#define IF_ID_GET(n) ((n)&0x00000FFF)

#define IF_VLAN_GET(n) if_ifindex2vlan((n))


#define IF_IFINDEX_TYPE_GET(n) IF_TYPE_GET(n)
#define IF_IFINDEX_UNIT_GET(n) IF_UNIT_GET(n)
#define IF_IFINDEX_SLOT_GET(n) IF_SLOT_GET(n)
#define IF_IFINDEX_PORT_GET(n) IF_PORT_GET(n)
#define IF_IFINDEX_ID_GET(n) IF_ID_GET(n)

#define IF_IFINDEX_VLAN_GET(n) IF_VLAN_GET(n)
#define IF_IFINDEX_PHYID_GET(n) if_ifindex2phy(n)
#define IF_IFINDEX_VRFID_GET(n) if_ifindex2vrfid(n)

#define IF_IFINDEX_TYPE_SET(n) IF_TYPE_SET(n)

#define IF_IFINDEX_SET(t, uspv) (IF_TYPE_SET(t) | (uspv))

#define IF_IFINDEX_PARENT_GET(n) IF_TYPE_SET(IF_IFINDEX_TYPE_GET(n)) |   \
                                     IF_USPV_SET(IF_IFINDEX_UNIT_GET(n), \
                                                 IF_IFINDEX_SLOT_GET(n), \
                                                 IF_IFINDEX_PORT_GET(n), 0)

#define IF_IFINDEX_ROOT_GET(n) IF_IFINDEX_PARENT_GET(n)

#define IF_IS_SUBIF_GET(n) IF_ID_GET(n)



/* Does the destination field contain a peer address? */
#define CONNECTED_PEER(C) CHECK_FLAG((C)->flags, ZEBRA_IFA_PEER)

/* Prefix to insert into the RIB */
#define CONNECTED_PREFIX(C) \
   (CONNECTED_PEER(C) ? (C)->destination : (C)->address)

/* Identifying address.  We guess that if there's a peer address, but the
   local address is in the same prefix, then the local address may be unique. */
#define CONNECTED_ID(C) \
   ((CONNECTED_PEER(C) && !prefix_match((C)->destination, (C)->address)) ? (C)->destination : (C)->address)

/* There are some interface flags which are only supported by some
   operating system. */

#ifndef IPSTACK_IFF_NOTRAILERS
#define IPSTACK_IFF_NOTRAILERS 0x0
#endif /* IFF_NOTRAILERS */
#ifndef IPSTACK_IFF_OACTIVE
#define IPSTACK_IFF_OACTIVE 0x0
#endif /* IPSTACK_IFF_OACTIVE */
#ifndef IPSTACK_IFF_SIMPLEX
#define IPSTACK_IFF_SIMPLEX 0x0
#endif /* IPSTACK_IFF_SIMPLEX */
#ifndef IPSTACK_IFF_LINK0
#define IPSTACK_IFF_LINK0 0x0
#endif /* IPSTACK_IFF_LINK0 */
#ifndef IPSTACK_IFF_LINK1
#define IPSTACK_IFF_LINK1 0x0
#endif /* IPSTACK_IFF_LINK1 */
#ifndef IPSTACK_IFF_LINK2
#define IPSTACK_IFF_LINK2 0x0
#endif /* IPSTACK_IFF_LINK2 */
#ifndef IPSTACK_IFF_NOXMIT
#define IPSTACK_IFF_NOXMIT 0x0
#endif /* IFF_NOXMIT */
#ifndef IPSTACK_IFF_NORTEXCH
#define IPSTACK_IFF_NORTEXCH 0x0
#endif /* IFF_NORTEXCH */
#ifndef IPSTACK_IFF_IPV4
#define IPSTACK_IFF_IPV4 0x0
#endif /* IFF_IPV4 */
#ifndef IPSTACK_IFF_IPV6
#define IPSTACK_IFF_IPV6 0x0
#endif /* IFF_IPV6 */
#ifndef IPSTACK_IFF_VIRTUAL
#define IPSTACK_IFF_VIRTUAL 0x0
#endif /* IFF_VIRTUAL */

/* Prototypes. */
extern struct list *if_list_get(void);
extern int if_hook_add(int (*add_cb)(struct interface *), int (*del_cb)(struct interface *));
extern int if_new_llc_type_mode(zpl_uint32 llc, zpl_uint32 mode);
extern int if_make_llc_type(struct interface *ifp);
extern int if_cmp_func(struct interface *, struct interface *);
extern struct interface *if_create(const char *name, zpl_uint32 namelen);
extern struct interface *if_create_dynamic(const char *name, zpl_uint32 namelen);
extern struct interface *if_lookup_by_index(ifindex_t);
extern struct interface *if_lookup_exact_address(struct ipstack_in_addr);
extern struct interface *if_lookup_address(struct ipstack_in_addr);
extern struct interface *if_lookup_prefix(struct prefix *prefix);
extern struct interface *if_create_vrf_dynamic(const char *name, zpl_uint32 namelen, vrf_id_t vrf_id);
extern struct interface *if_create_vrf(const char *name, zpl_uint32 namelen,
                                       vrf_id_t vrf_id);
extern struct interface *if_lookup_by_index_vrf(ifindex_t, vrf_id_t vrf_id);
extern struct interface *if_lookup_exact_address_vrf(struct ipstack_in_addr,
                                                     vrf_id_t vrf_id);
extern struct interface *if_lookup_address_vrf(struct ipstack_in_addr,
                                               vrf_id_t vrf_id);
extern struct interface *if_lookup_prefix_vrf(struct prefix *prefix,
                                              vrf_id_t vrf_id);

/* These 2 functions are to be used when the ifname argument is terminated
   by a '\0' character: */
extern struct interface *if_lookup_by_name(const char *ifname);

extern struct interface *if_lookup_by_name_vrf(const char *ifname,
                                               vrf_id_t vrf_id);

/* For these 2 functions, the namelen argument should be the precise length
   of the ifname string (not counting any optional trailing '\0' character).
   In most cases, strnlen should be used to calculate the namelen value. */
extern struct interface *if_lookup_by_name_len(const char *ifname,
                                               zpl_uint32 namelen);

extern struct interface *if_lookup_by_name_len_vrf(const char *ifname,
                                                   zpl_uint32 namelen, vrf_id_t vrf_id);

extern struct interface *if_lookup_by_encavlan(zpl_ushort encavlan);

extern zpl_uint32 if_count_lookup_type(if_type_t type);

extern const char *if_enca_string(if_enca_t enca);

extern int if_name_set(struct interface *, const char *str);
extern int if_kname_set(struct interface *, const char *str);

/* Delete and free the interface structure: calls if_delete_retain and then
   deletes it from the interface list and frees the structure. */
extern void if_delete(struct interface *);

extern zpl_bool if_is_up(struct interface *);
extern zpl_bool if_is_running(struct interface *);
extern zpl_bool if_is_operative(struct interface *);
extern zpl_bool if_is_loopback(struct interface *);
extern zpl_bool if_is_broadcast(struct interface *);
extern zpl_bool if_is_pointopoint(struct interface *);
extern zpl_bool if_is_multicast(struct interface *);
extern zpl_bool if_is_serial(struct interface *ifp);
extern zpl_bool if_is_ethernet(struct interface *ifp);
extern zpl_bool if_is_tunnel(struct interface *ifp);
extern zpl_bool if_is_lag(struct interface *ifp);
extern zpl_bool if_is_lag_member(struct interface *ifp);
extern zpl_bool if_is_vlan(struct interface *ifp);
extern zpl_bool if_is_brigde(struct interface *ifp);
extern zpl_bool if_is_brigde_member(struct interface *ifp);
extern zpl_bool if_is_loop(struct interface *ifp);
extern zpl_bool if_is_wireless(struct interface *ifp);
extern zpl_bool if_is_l3intf(struct interface *ifp);
extern zpl_bool if_is_online(struct interface *);

extern int if_online(struct interface *ifp, zpl_bool enable);
extern int if_up(struct interface *ifp);
extern int if_down(struct interface *ifp);

extern void if_init(void);
extern void if_terminate(void);

extern void if_dump_all(void);
extern const char *if_flag_dump(zpl_ulong);
extern const char *if_link_type_str(enum if_link_type);

/* Please use ifindex2ifname instead of if_indextoname where possible;
   ifindex2ifname uses internal interface info, whereas if_indextoname must
   make a system call. */
extern const char *ifindex2ifname(ifindex_t);
extern const char *ifindex2ifname_vrf(ifindex_t, vrf_id_t vrf_id);

extern struct interface *if_lookup_by_kernel_name(const char *ifname);
extern struct interface *if_lookup_by_kernel_name_vrf(const char *ifname,
                                                      vrf_id_t vrf_id);
extern struct interface *if_lookup_by_kernel_index_vrf(ifindex_t kifindex, vrf_id_t vrf_id);
extern struct interface *if_lookup_by_kernel_index(ifindex_t kifindex);

extern const char *ifkernelindex2kernelifname(ifindex_t);
extern const char *ifkernelindex2kernelifname_vrf(ifindex_t, vrf_id_t vrf_id);
extern ifindex_t ifname2kernelifindex(const char *ifname);
extern ifindex_t ifname2kernelifindex_vrf(const char *ifname, vrf_id_t vrf_id);
extern ifindex_t ifindex2ifkernel(ifindex_t);
extern ifindex_t ifkernel2ifindex(ifindex_t);

//extern const char *ifkernelindex2ifname(ifindex_t kifindex);
/* Please use ifname2ifindex instead of if_nametoindex where possible;
   ifname2ifindex uses internal interface info, whereas if_nametoindex must
   make a system call. */
extern ifindex_t ifname2ifindex(const char *ifname);
extern ifindex_t ifname2ifindex_vrf(const char *ifname, vrf_id_t vrf_id);
extern zpl_vlan_t  if_ifindex2vlan(ifindex_t ifindex);
extern ifindex_t if_vlan2ifindex(zpl_vlan_t encavlan);
extern zpl_phyport_t  if_ifindex2phy(ifindex_t ifindex);
extern ifindex_t  if_phy2ifindex(zpl_phyport_t phyid);
extern vrf_id_t  if_ifindex2vrfid(ifindex_t ifindex);

extern zpl_phyport_t  if_ifindex2l3intfid(ifindex_t ifindex);
extern ifindex_t  if_l3intfid2ifindex(zpl_phyport_t l3intfid);

extern int if_update_phyid(ifindex_t ifindex, zpl_phyport_t phyid);
extern int if_update_phyid2(struct interface *ifp, zpl_phyport_t phyid);
extern int if_update_l3intfid(ifindex_t ifindex, zpl_phyport_t phyid);
extern int if_update_l3intfid2(struct interface *ifp, zpl_phyport_t phyid);

extern void *if_module_data(struct interface *ifp, module_t mid);

extern zpl_bool if_have_kernel(struct interface *ifp);

extern int if_list_each(int (*cb)(struct interface *ifp, void *pVoid), void *pVoid);

/* Connected address functions. */
extern struct connected *connected_new(void);
extern void connected_free(struct connected *);
extern void connected_add(struct interface *, struct connected *);
extern struct connected *connected_add_by_prefix(struct interface *,
                                                 struct prefix *,
                                                 struct prefix *);
extern struct connected *connected_delete_by_prefix(struct interface *,
                                                    struct prefix *);
extern struct connected *connected_lookup_address(struct interface *,
                                                  struct ipstack_in_addr);
extern struct connected *connected_check(struct interface *ifp, struct prefix *p);

extern int if_data_lock(void);
extern int if_data_unlock(void);

extern enum if_link_type netlink_to_if_link_type(zpl_uint32  hwt);
 
#ifdef __cplusplus
}
#endif

#endif /* _ZEBRA_IF_H */
