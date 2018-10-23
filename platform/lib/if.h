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

#include "zebra.h"
#include "log.h"
#include "linklist.h"

/* Interface link-layer type, if known. Derived from:
 *
 * net/if_arp.h on various platforms - Linux especially.
 * http://www.iana.org/assignments/arp-parameters/arp-parameters.xhtml
 *
 * Some of the more obviously defunct technologies left out.
 */
enum zebra_link_type
{
	ZEBRA_LLT_UNKNOWN = 0,
	ZEBRA_LLT_LOOPBACK,
	ZEBRA_LLT_ETHER,
	ZEBRA_LLT_GIETHER,
	ZEBRA_LLT_TUNNEL,
	ZEBRA_LLT_VLAN,
	ZEBRA_LLT_LAG,
	ZEBRA_LLT_SERIAL,
	ZEBRA_LLT_SLIP,
	ZEBRA_LLT_CSLIP,
	ZEBRA_LLT_SLIP6,
	ZEBRA_LLT_CSLIP6,

	ZEBRA_LLT_PPP,
	ZEBRA_LLT_CHDLC,
	ZEBRA_LLT_RAWHDLC,
	ZEBRA_LLT_IPIP,
	ZEBRA_LLT_IPIP6,
	ZEBRA_LLT_IPGRE,
	ZEBRA_LLT_IP6GRE,

	ZEBRA_LLT_ATM,
	ZEBRA_LLT_SIT,
	ZEBRA_LLT_BRIGDE,
	ZEBRA_LLT_WIFI,
	ZEBRA_LLT_MODEM,
	ZEBRA_LLT_WIRELESS,
};

/*
  Interface name length.

   Linux define value in /usr/include/linux/if.h.
   #define IFNAMSIZ        16

   FreeBSD define value in /usr/include/net/if.h.
   #define IFNAMSIZ        16
*/

#define INTERFACE_NAMSIZ      64
#define INTERFACE_HWADDR_MAX  20
#define MAX_CLASS_TYPE          8

#define IF_LOOPBACK_MAX     16
#define IF_VLAN_MAX      	256
#define IF_LAG_MAX      	16
#define IF_TUNNEL_SLOT      0
#define IF_TUNNEL_MAX      	256
#define IF_BRIGDE_MAX     16

#define IF_MTU_DEFAULT		1500

#define IF_DATA_LOCK()		//if_data_lock()
#define IF_DATA_UNLOCK()	//if_data_unlock()

typedef enum
{
	IF_TTY_USB  = 1,
	IF_TTY_CAM,
	IF_TTY_S,
}tty_type_en;


typedef unsigned int ifindex_t;


struct if_stats
{
  unsigned long rx_packets;   /* total packets received       */
  unsigned long tx_packets;   /* total packets transmitted    */
  unsigned long rx_bytes;     /* total bytes received         */
  unsigned long tx_bytes;     /* total bytes transmitted      */
  unsigned long rx_errors;    /* bad packets received         */
  unsigned long tx_errors;    /* packet transmit problems     */
  unsigned long rx_dropped;   /* no space in linux buffers    */
  unsigned long tx_dropped;   /* no space available in linux  */
  unsigned long rx_multicast; /* multicast packets received   */
  unsigned long collisions;

  /* detailed rx_errors: */
  unsigned long rx_length_errors;
  unsigned long rx_over_errors;       /* receiver ring buff overflow  */
  unsigned long rx_crc_errors;        /* recved pkt with crc error    */
  unsigned long rx_frame_errors;      /* recv'd frame alignment error */
  unsigned long rx_fifo_errors;       /* recv'r fifo overrun          */
  unsigned long rx_missed_errors;     /* receiver missed packet     */
  /* detailed tx_errors */
  unsigned long tx_aborted_errors;
  unsigned long tx_carrier_errors;
  unsigned long tx_fifo_errors;
  unsigned long tx_heartbeat_errors;
  unsigned long tx_window_errors;
	/* for cslip etc */
  unsigned long rx_compressed;
  unsigned long tx_compressed;
};


typedef enum if_type_s
{
	IF_NONE,
	IF_LOOPBACK,
	IF_SERIAL,
	IF_ETHERNET,
	IF_GIGABT_ETHERNET,
	IF_WIRELESS,	//wireless interface
	IF_TUNNEL,
	IF_LAG,
	IF_BRIGDE,		//brigde interface
	IF_VLAN,
#ifdef CUSTOM_INTERFACE
	IF_WIFI,		//wifi interface wireless
	IF_MODEM,		//modem interface
#endif
	IF_MAX
}if_type_t;

typedef enum if_mode_s
{
	IF_MODE_NONE,
	IF_MODE_ACCESS_L2,
	IF_MODE_TRUNK_L2,
	IF_MODE_L3,
	IF_MODE_BRIGDE,
}if_mode_t;

typedef enum if_enca_s
{
	IF_ENCA_NONE,
	IF_ENCA_DOT1Q,	//VLAN
	IF_ENCA_DOT1Q_TUNNEL,	//QINQ
	IF_ENCA_PPP,	//PPP
	IF_ENCA_PPPOE,	//PPPOE
	IF_ENCA_SLIP,	//SLIP
	IF_ENCA_GRE,	//GRE
	IF_ENCA_IPIP,	//IPIP
	IF_ENCA_IPIPV6,	//IPV4-IPV6
	IF_ENCA_IPV6IP,	//IPV6-IPV4
	IF_ENCA_SIT,	//SIT
	IF_ENCA_HDLC,	//HDLC
	IF_ENCA_RAW,	//RAW
}if_enca_t;

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
  char name[INTERFACE_NAMSIZ + 1];
  unsigned int name_hash;
  /* Interface index (should be IFINDEX_INTERNAL for non-kernel or
     deleted interfaces). */
  ifindex_t ifindex;
#define IFINDEX_INTERNAL	0

  unsigned int uspv;
  unsigned short encavlan;		//子接口封装的VLAN ID

  char k_name[INTERFACE_NAMSIZ + 1];
  unsigned int k_name_hash;
  ifindex_t k_ifindex;

  if_type_t if_type;

  if_mode_t if_mode;
  if_enca_t	if_enca;
  BOOL		dynamic;
  /* Zebra internal interface status */
  u_char status;
#define ZEBRA_INTERFACE_ACTIVE     (1 << 0)
#define ZEBRA_INTERFACE_LINKDETECTION (1 << 2)
#define ZEBRA_INTERFACE_ATTACH	 (1 << 3)
  /* Interface flags. */
  uint64_t flags;

  /* Interface metric */
  int metric;

  /* Interface MTU. */
  unsigned int mtu;    /* IPv4 MTU */
  unsigned int mtu6;   /* IPv6 MTU - probably, but not neccessarily same as mtu */

  /* Link-layer information and hardware address */
  enum zebra_link_type ll_type;
  u_char hw_addr[INTERFACE_HWADDR_MAX];
  int hw_addr_len;

  /* interface bandwidth, kbits */
  unsigned int bandwidth;
  
  /* description of the interface. */
  char *desc;			

  /* Distribute list. */
  void *distribute_in;
  void *distribute_out;

  /* Connected address list. */
  struct list *connected;
  BOOL		dhcp;
  /* Daemon specific interface data pointer. */
  //void *info[ZLOG_MAX];
  void *info[MODULE_MAX];

  /* Statistics fileds. */
  struct if_stats stats;

  vrf_id_t vrf_id;

  unsigned int ifmember;
#define IF_TRUNK_MEM     (1 << 0)
#define IF_BRIDGE_MEM 	 (1 << 2)


  int count;
  int raw_status;
};

/* Connected address structure. */
struct connected
{
  /* Attached interface. */
  struct interface *ifp;

  /* Flags for configuration. */
  u_char conf;
#define ZEBRA_IFC_CONFIGURED   (1 << 1)
#define ZEBRA_IFC_DHCPC   	   (1 << 2)
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
  u_char flags;
#define ZEBRA_IFA_SECONDARY    (1 << 0)
#define ZEBRA_IFA_PEER         (1 << 1)
#define ZEBRA_IFA_UNNUMBERED   (1 << 2)
#define ZEBRA_IFA_DHCPC		   (1 << 3)
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

  int count;
  int raw_status;
};

#define IF_TYPE_GET(n)		(((n)>>28)&0x0F)
#define IF_UNIT_GET(n)		(((n)>>24)&0x0F)
#define IF_SLOT_GET(n)		(((n)>>20)&0x0F)
#define IF_PORT_GET(n)		(((n)>>12)&0xFF)
#define IF_ID_GET(n)		((n)&0x0FFF)

#define IF_VLAN_GET(n)		ifindex2vlan((n))

#define IF_TYPE_SET(n)		(((n)&0x0F)<<28)
#define IF_TYPE_CLR(n)		(((n))&0x0FFFFFFF)
#define IF_USPV_SET(u,s,p,v)	(((u)&0x0F)<<24)|(((s)&0x0F)<<20) |(((p)&0xFF)<<12)|((v)&0x0FFF)

#define IF_IFINDEX_TYPE_GET(n)		IF_TYPE_GET(n)
#define IF_IFINDEX_UNIT_GET(n)		IF_UNIT_GET(n)
#define IF_IFINDEX_SLOT_GET(n)		IF_SLOT_GET(n)
#define IF_IFINDEX_PORT_GET(n)		IF_PORT_GET(n)
#define IF_IFINDEX_ID_GET(n)		IF_ID_GET(n)

#define IF_IFINDEX_VLAN_GET(n)		IF_VLAN_GET(n)

#define IF_IFINDEX_TYPE_SET(n)		IF_TYPE_SET(n)

#define IF_IFINDEX_SET(t,uspv)		(IF_TYPE_SET(t)|(uspv) )

#define IF_IFINDEX_PARENT_GET(n)	IF_TYPE_SET(IF_IFINDEX_TYPE_GET(n)) | \
									IF_USPV_SET(IF_IFINDEX_UNIT_GET(n), \
										IF_IFINDEX_SLOT_GET(n), \
										IF_IFINDEX_PORT_GET(n), 0)

#define IF_IFINDEX_ROOT_GET(n)		IF_IFINDEX_PARENT_GET(n)


#define IF_IS_SUBIF_GET(n)			IF_ID_GET(n)


/* Does the destination field contain a peer address? */
#define CONNECTED_PEER(C) CHECK_FLAG((C)->flags, ZEBRA_IFA_PEER)

/* Prefix to insert into the RIB */
#define CONNECTED_PREFIX(C) \
	(CONNECTED_PEER(C) ? (C)->destination : (C)->address)

/* Identifying address.  We guess that if there's a peer address, but the
   local address is in the same prefix, then the local address may be unique. */
#define CONNECTED_ID(C)	\
	((CONNECTED_PEER(C) && !prefix_match((C)->destination, (C)->address)) ?\
	 (C)->destination : (C)->address)


/* There are some interface flags which are only supported by some
   operating system. */

#ifndef IFF_NOTRAILERS
#define IFF_NOTRAILERS 0x0
#endif /* IFF_NOTRAILERS */
#ifndef IFF_OACTIVE
#define IFF_OACTIVE 0x0
#endif /* IFF_OACTIVE */
#ifndef IFF_SIMPLEX
#define IFF_SIMPLEX 0x0
#endif /* IFF_SIMPLEX */
#ifndef IFF_LINK0
#define IFF_LINK0 0x0
#endif /* IFF_LINK0 */
#ifndef IFF_LINK1
#define IFF_LINK1 0x0
#endif /* IFF_LINK1 */
#ifndef IFF_LINK2
#define IFF_LINK2 0x0
#endif /* IFF_LINK2 */
#ifndef IFF_NOXMIT
#define IFF_NOXMIT 0x0
#endif /* IFF_NOXMIT */
#ifndef IFF_NORTEXCH
#define IFF_NORTEXCH 0x0
#endif /* IFF_NORTEXCH */
#ifndef IFF_IPV4
#define IFF_IPV4 0x0
#endif /* IFF_IPV4 */
#ifndef IFF_IPV6
#define IFF_IPV6 0x0
#endif /* IFF_IPV6 */
#ifndef IFF_VIRTUAL
#define IFF_VIRTUAL 0x0
#endif /* IFF_VIRTUAL */


/* Prototypes. */
extern struct list *if_list_get();
extern int if_hook_add(int (*add_cb) (struct interface *), int (*del_cb) (struct interface *));
extern int if_new_llc_type_mode(int llc, int mode);
extern int if_make_llc_type(struct interface *ifp);
extern int if_cmp_func (struct interface *, struct interface *);
extern struct interface *if_create (const char *name, int namelen);
extern struct interface *if_create_dynamic(const char *name, int namelen);
extern struct interface *if_lookup_by_index (ifindex_t);
extern struct interface *if_lookup_exact_address (struct in_addr);
extern struct interface *if_lookup_address (struct in_addr);
extern struct interface *if_lookup_prefix (struct prefix *prefix);
extern struct interface *if_create_vrf_dynamic(const char *name, int namelen, vrf_id_t vrf_id);
extern struct interface *if_create_vrf (const char *name, int namelen,
                                vrf_id_t vrf_id);
extern struct interface *if_lookup_by_index_vrf (ifindex_t, vrf_id_t vrf_id);
extern struct interface *if_lookup_exact_address_vrf (struct in_addr,
                                vrf_id_t vrf_id);
extern struct interface *if_lookup_address_vrf (struct in_addr,
                                vrf_id_t vrf_id);
extern struct interface *if_lookup_prefix_vrf (struct prefix *prefix,
                                vrf_id_t vrf_id);

/* These 2 functions are to be used when the ifname argument is terminated
   by a '\0' character: */
extern struct interface *if_lookup_by_name (const char *ifname);

extern struct interface *if_lookup_by_name_vrf (const char *ifname,
                                vrf_id_t vrf_id);

/* For these 2 functions, the namelen argument should be the precise length
   of the ifname string (not counting any optional trailing '\0' character).
   In most cases, strnlen should be used to calculate the namelen value. */
extern struct interface *if_lookup_by_name_len(const char *ifname,
                                              size_t namelen);

extern struct interface *if_lookup_by_name_len_vrf(const char *ifname,
                                size_t namelen, vrf_id_t vrf_id);


extern struct interface *if_lookup_by_encavlan(unsigned short encavlan);

extern int if_count_lookup_type(if_type_t type);

extern const char *if_enca_string(if_enca_t	enca);

extern int if_name_set(struct interface *, const char *str);
extern int if_kname_set(struct interface *, const char *str);

/* Delete and free the interface structure: calls if_delete_retain and then
   deletes it from the interface list and frees the structure. */
extern void if_delete (struct interface *);

extern int if_is_up (struct interface *);
extern int if_is_running (struct interface *);
extern int if_is_operative (struct interface *);
extern int if_is_loopback (struct interface *);
extern int if_is_broadcast (struct interface *);
extern int if_is_pointopoint (struct interface *);
extern int if_is_multicast (struct interface *);
extern int if_is_serial(struct interface *ifp);
extern int if_is_ethernet(struct interface *ifp);
extern int if_is_tunnel(struct interface *ifp);
extern int if_is_lag(struct interface *ifp);
extern int if_is_lag_member(struct interface *ifp);
extern int if_is_vlan(struct interface *ifp);
extern int if_is_brigde(struct interface *ifp);
extern int if_is_brigde_member(struct interface *ifp);
extern int if_is_loop(struct interface *ifp);
extern int if_is_wireless(struct interface *ifp);
extern int if_up (struct interface *ifp);
extern int if_down (struct interface *ifp);

extern void if_init ();
extern void if_terminate ();


extern void if_dump_all (void);
extern const char *if_flag_dump(unsigned long);
extern const char *if_link_type_str (enum zebra_link_type);

/* Please use ifindex2ifname instead of if_indextoname where possible;
   ifindex2ifname uses internal interface info, whereas if_indextoname must
   make a system call. */
extern const char *ifindex2ifname (ifindex_t);
extern const char *ifindex2ifname_vrf (ifindex_t, vrf_id_t vrf_id);


extern struct interface *if_lookup_by_kernel_name (const char *ifname);
extern struct interface *if_lookup_by_kernel_name_vrf (const char *ifname,
                                vrf_id_t vrf_id);
extern struct interface * if_lookup_by_kernel_index_vrf (ifindex_t kifindex, vrf_id_t vrf_id);
extern struct interface * if_lookup_by_kernel_index (ifindex_t kifindex);

extern const char *ifkernelindex2kernelifname(ifindex_t);
extern const char *ifkernelindex2kernelifname_vrf (ifindex_t, vrf_id_t vrf_id);
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
extern unsigned int ifindex2vlan(ifindex_t ifindex);

extern int if_list_each(int (*cb)(struct interface *ifp, void *pVoid), void *pVoid);

/* Connected address functions. */
extern struct connected *connected_new (void);
extern void connected_free (struct connected *);
extern void connected_add (struct interface *, struct connected *);
extern struct connected  *connected_add_by_prefix (struct interface *,
                                            struct prefix *,
                                            struct prefix *);
extern struct connected  *connected_delete_by_prefix (struct interface *, 
                                               struct prefix *);
extern struct connected  *connected_lookup_address (struct interface *, 
                                             struct in_addr);
extern struct connected *connected_check (struct interface *ifp, struct prefix *p);

extern int if_data_lock ();
extern int if_data_unlock ();

#endif /* _ZEBRA_IF_H */
