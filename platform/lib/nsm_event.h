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



/* default zebra TCP port for zclient */
#define NSM_ZSERV_PORT			2600

/* Zebra message types. */
#define NSM_EVENT_HELLO			               0
#define NSM_EVENT_INTERFACE_ADD                1
#define NSM_EVENT_INTERFACE_DELETE             2
#define NSM_EVENT_INTERFACE_ADDRESS_ADD        3
#define NSM_EVENT_INTERFACE_ADDRESS_DELETE     4
#define NSM_EVENT_INTERFACE_UP                 5
#define NSM_EVENT_INTERFACE_DOWN               6
#define NSM_EVENT_INTERFACE_MODE               7
#define NSM_EVENT_INTERFACE_RENAME             8


#define NSM_EVENT_ROUTER_ID_ADD               20
#define NSM_EVENT_ROUTER_ID_DELETE            21
#define NSM_EVENT_ROUTER_ID_UPDATE            22

#define NSM_EVENT_IPV4_ROUTE_ADD              30
#define NSM_EVENT_IPV4_ROUTE_DELETE           31
#define NSM_EVENT_IPV6_ROUTE_ADD              32
#define NSM_EVENT_IPV6_ROUTE_DELETE           33

#define NSM_EVENT_REDISTRIBUTE_ADD            40
#define NSM_EVENT_REDISTRIBUTE_DELETE         41
#define NSM_EVENT_REDISTRIBUTE_DEFAULT_ADD    42
#define NSM_EVENT_REDISTRIBUTE_DEFAULT_DELETE 43

#define NSM_EVENT_IPV4_NEXTHOP_LOOKUP         50
#define NSM_EVENT_IPV6_NEXTHOP_LOOKUP         51
#define NSM_EVENT_IPV4_IMPORT_LOOKUP          52
#define NSM_EVENT_IPV6_IMPORT_LOOKUP          53
#define NSM_EVENT_IPV4_NEXTHOP_LOOKUP_MRIB    54

#define NSM_EVENT_NEXTHOP_REGISTER            60
#define NSM_EVENT_NEXTHOP_UNREGISTER          61
#define NSM_EVENT_NEXTHOP_UPDATE              62

#define NSM_EVENT_VRF_REGISTER                70
#define NSM_EVENT_VRF_UNREGISTER              71


#define NSM_EVENT_MESSAGE_MAX                 90

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




#ifdef __cplusplus
}
#endif

#endif /* __NSM_EVENT_H__ */
