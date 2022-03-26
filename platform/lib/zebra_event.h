/*
 * zebra_event.h
 *
 *  Created on: Jan 8, 2018
 *      Author: zhurish
 */

#ifndef __ZEBRA_EVENT_H__
#define __ZEBRA_EVENT_H__

#ifdef __cplusplus
extern "C" {
#endif



/* default zebra TCP port for zclient */
#define ZEBRA_PORT			2600

/* Zebra message types. */
#define ZEBRA_HELLO			               0
#define ZEBRA_INTERFACE_ADD                1
#define ZEBRA_INTERFACE_DELETE             2
#define ZEBRA_INTERFACE_ADDRESS_ADD        3
#define ZEBRA_INTERFACE_ADDRESS_DELETE     4
#define ZEBRA_INTERFACE_UP                 5
#define ZEBRA_INTERFACE_DOWN               6
#define ZEBRA_INTERFACE_MODE               7
#define ZEBRA_INTERFACE_RENAME             8


#define ZEBRA_ROUTER_ID_ADD               20
#define ZEBRA_ROUTER_ID_DELETE            21
#define ZEBRA_ROUTER_ID_UPDATE            22

#define ZEBRA_IPV4_ROUTE_ADD              30
#define ZEBRA_IPV4_ROUTE_DELETE           31
#define ZEBRA_IPV6_ROUTE_ADD              32
#define ZEBRA_IPV6_ROUTE_DELETE           33

#define ZEBRA_REDISTRIBUTE_ADD            40
#define ZEBRA_REDISTRIBUTE_DELETE         41
#define ZEBRA_REDISTRIBUTE_DEFAULT_ADD    42
#define ZEBRA_REDISTRIBUTE_DEFAULT_DELETE 43

#define ZEBRA_IPV4_NEXTHOP_LOOKUP         50
#define ZEBRA_IPV6_NEXTHOP_LOOKUP         51
#define ZEBRA_IPV4_IMPORT_LOOKUP          52
#define ZEBRA_IPV6_IMPORT_LOOKUP          53
#define ZEBRA_IPV4_NEXTHOP_LOOKUP_MRIB    54

#define ZEBRA_NEXTHOP_REGISTER            60
#define ZEBRA_NEXTHOP_UNREGISTER          61
#define ZEBRA_NEXTHOP_UPDATE              62

#define ZEBRA_VRF_REGISTER                70
#define ZEBRA_VRF_UNREGISTER              71

#define ZEBRA_MESSAGE_MAX                 90

/* Marker value used in new Zserv, in the byte location corresponding
 * the command value in the old nsm_zserv.header. To allow old and new
 * Zserv headers to be distinguished from each other.
 */
#define ZEBRA_HEADER_MARKER              255




/* Error codes of zebra. */
#define ZEBRA_ERR_NOERROR                0
#define ZEBRA_ERR_RTEXIST               -1
#define ZEBRA_ERR_RTUNREACH             -2
#define ZEBRA_ERR_EPERM                 -3
#define ZEBRA_ERR_RTNOEXIST             -4
#define ZEBRA_ERR_KERNEL                -5

/* Zebra message flags */
#define ZEBRA_FLAG_INTERNAL           0x01
#define ZEBRA_FLAG_SELFROUTE          0x02
#define ZEBRA_FLAG_BLACKHOLE          0x04
#define ZEBRA_FLAG_IBGP               0x08
#define ZEBRA_FLAG_SELECTED           0x10
#define ZEBRA_FLAG_FIB_OVERRIDE       0x20
#define ZEBRA_FLAG_STATIC             0x40
#define ZEBRA_FLAG_REJECT             0x80

/* Zebra nexthop flags. */
#define ZEBRA_NEXTHOP_IFINDEX            1
#define ZEBRA_NEXTHOP_IFNAME             2
#define ZEBRA_NEXTHOP_IPV4               3
#define ZEBRA_NEXTHOP_IPV4_IFINDEX       4
#define ZEBRA_NEXTHOP_IPV4_IFNAME        5
#define ZEBRA_NEXTHOP_IPV6               6
#define ZEBRA_NEXTHOP_IPV6_IFINDEX       7
#define ZEBRA_NEXTHOP_IPV6_IFNAME        8
#define ZEBRA_NEXTHOP_BLACKHOLE          9



/* Default Administrative Distance of each protocol. */
#define ZEBRA_KERNEL_DISTANCE_DEFAULT      0
#define ZEBRA_CONNECT_DISTANCE_DEFAULT     0
#define ZEBRA_STATIC_DISTANCE_DEFAULT      1
#define ZEBRA_RIP_DISTANCE_DEFAULT       120
#define ZEBRA_RIPNG_DISTANCE_DEFAULT     120
#define ZEBRA_OSPF_DISTANCE_DEFAULT      110
#define ZEBRA_OSPF6_DISTANCE_DEFAULT     110
#define ZEBRA_ISIS_DISTANCE_DEFAULT      115
#define ZEBRA_IBGP_DISTANCE_DEFAULT      200
#define ZEBRA_EBGP_DISTANCE_DEFAULT       20


/* For old definition. */
#ifndef IPSTACK_IN6_ARE_ADDR_EQUAL
#define IPSTACK_IN6_ARE_ADDR_EQUAL IN6_IS_ADDR_EQUAL
#endif /* IPSTACK_IN6_ARE_ADDR_EQUAL */




#ifdef __cplusplus
}
#endif

#endif /* __ZEBRA_EVENT_H__ */
