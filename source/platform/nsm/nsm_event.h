/*
 * nsm_event.h
 *
 *  Created on: Jan 8, 2018
 *      Author: zhurish
 */

#ifndef __NSM_EVENT_H__
#define __NSM_EVENT_H__

#ifdef __cplusplus
extern "C" {
#endif




#define NSM_ZSERV_PORT			2600


struct nsm_event_desc_table
{
  unsigned int event;
  const char *string;
};


/* nen event types. */
typedef enum 
{
  NSM_EVENT_HELLO = 0,
  NSM_EVENT_INTERFACE_ADD,
  NSM_EVENT_INTERFACE_DELETE,
  NSM_EVENT_INTERFACE_ADDRESS_ADD,
  NSM_EVENT_INTERFACE_ADDRESS_DELETE,
  NSM_EVENT_INTERFACE_UP,
  NSM_EVENT_INTERFACE_DOWN,
  NSM_EVENT_INTERFACE_MODE,
  NSM_EVENT_INTERFACE_RENAME,

  NSM_EVENT_INTERFACE_VRF_BIND,
  NSM_EVENT_INTERFACE_VRF_UNBIND,

  NSM_EVENT_ROUTER_ID_ADD,
  NSM_EVENT_ROUTER_ID_DELETE,
  NSM_EVENT_ROUTER_ID_UPDATE,

  NSM_EVENT_IPV4_ROUTE_ADD,
  NSM_EVENT_IPV4_ROUTE_DELETE,
  NSM_EVENT_IPV6_ROUTE_ADD,
  NSM_EVENT_IPV6_ROUTE_DELETE,

  NSM_EVENT_REDISTRIBUTE_ADD,
  NSM_EVENT_REDISTRIBUTE_DELETE,
  NSM_EVENT_REDISTRIBUTE_DEFAULT_ADD,
  NSM_EVENT_REDISTRIBUTE_DEFAULT_DELETE,

  NSM_EVENT_IPV4_NEXTHOP_LOOKUP,
  NSM_EVENT_IPV6_NEXTHOP_LOOKUP,
  NSM_EVENT_IPV4_IMPORT_LOOKUP,
  NSM_EVENT_IPV6_IMPORT_LOOKUP,
  NSM_EVENT_IPV4_NEXTHOP_LOOKUP_MRIB,

  NSM_EVENT_NEXTHOP_REGISTER,
  NSM_EVENT_NEXTHOP_UNREGISTER,
  NSM_EVENT_NEXTHOP_UPDATE,

  NSM_EVENT_VRF_REGISTER,
  NSM_EVENT_VRF_UNREGISTER,

  NSM_EVENT_MESSAGE_MAX,
}nsm_event_type;
/* Marker value used in new Zserv, in the byte location corresponding
 * the command value in the old nsm_zserv.header. To allow old and new
 * Zserv headers to be distinguished from each other.
 */
#define MSG_HEADER_MARKER              255




/* Error codes of zebra. */
#define NSM_RIB_ERR_NOERROR                0
#define NSM_RIB_ERR_RTEXIST               -1
#define NSM_RIB_ERR_RTUNREACH             -2
#define NSM_RIB_ERR_EPERM                 -3
#define NSM_RIB_ERR_RTNOEXIST             -4
#define NSM_RIB_ERR_KERNEL                -5

/* Zebra message flags */
#define NSM_RIB_FLAG_INTERNAL           0x01
#define NSM_RIB_FLAG_SELFROUTE          0x02
#define NSM_RIB_FLAG_BLACKHOLE          0x04
#define NSM_RIB_FLAG_IBGP               0x08
#define NSM_RIB_FLAG_SELECTED           0x10
#define NSM_RIB_FLAG_FIB_OVERRIDE       0x20
#define NSM_RIB_FLAG_STATIC             0x40
#define NSM_RIB_FLAG_REJECT             0x80

/* Zebra nexthop flags. */
#define NSM_NEXTHOP_IFINDEX            1
#define NSM_NEXTHOP_IFNAME             2
#define NSM_NEXTHOP_IPV4               3
#define NSM_NEXTHOP_IPV4_IFINDEX       4
#define NSM_NEXTHOP_IPV4_IFNAME        5
#define NSM_NEXTHOP_IPV6               6
#define NSM_NEXTHOP_IPV6_IFINDEX       7
#define NSM_NEXTHOP_IPV6_IFNAME        8
#define NSM_NEXTHOP_BLACKHOLE          9



/* Default Administrative Distance of each protocol. */
#define NSM_RIB_KERNEL_DISTANCE_DEFAULT      0
#define NSM_RIB_CONNECT_DISTANCE_DEFAULT     0
#define NSM_RIB_STATIC_DISTANCE_DEFAULT      1
#define NSM_RIB_RIP_DISTANCE_DEFAULT       120
#define NSM_RIB_RIPNG_DISTANCE_DEFAULT     120
#define NSM_RIB_OSPF_DISTANCE_DEFAULT      110
#define NSM_RIB_OSPF6_DISTANCE_DEFAULT     110
#define NSM_RIB_ISIS_DISTANCE_DEFAULT      115
#define NSM_RIB_IBGP_DISTANCE_DEFAULT      200
#define NSM_RIB_EBGP_DISTANCE_DEFAULT       20


/* For old definition. */
#ifndef IPSTACK_IN6_ARE_ADDR_EQUAL
#define IPSTACK_IN6_ARE_ADDR_EQUAL IN6_IS_ADDR_EQUAL
#endif /* IPSTACK_IN6_ARE_ADDR_EQUAL */



extern const char *zserv_command_string (zpl_uint32 command);


#ifdef __cplusplus
}
#endif

#endif /* __NSM_EVENT_H__ */
