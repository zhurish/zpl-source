/*
 * nsm_include.h
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#ifndef __NSM_INCLUDE_H__
#define __NSM_INCLUDE_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "route_types.h"
#include "nexthop.h"
#include "table.h"
#include "queue.h"
#include "nsm_event.h"
#ifdef ZPL_RTPL_MODULE
#include "nsm_rib.h"
#include "router-id.h"
#endif
#include "nsm_ipforward.h"
#include "nsm_interface.h"
#include "nsm_pal.h"
#include "nsm_debug.h"



#ifdef ZPL_NSM_MAC
#include "nsm_mac.h"
#endif
#ifdef ZPL_NSM_DHCP
#include "nsm_dhcp.h"
#endif
#ifdef ZPL_NSM_ARP
#include "nsm_arp.h"
#endif
#ifdef ZPL_NSM_8021X
#include "nsm_8021x.h"
#endif
#ifdef ZPL_NSM_BRIDGE
#include "nsm_bridge.h"
#endif
#ifdef ZPL_NSM_DNS
#include "nsm_dns.h"
#endif
#ifdef ZPL_NSM_DOS
#include "nsm_dos.h"
#endif
#ifdef ZPL_NSM_FIREWALLD
#include "nsm_firewalld.h"
#endif
#ifdef ZPL_NSM_VLAN
#include "nsm_vlan.h"
#endif

#ifdef ZPL_NSM_QOS
#include "nsm_qos.h"
#endif
#ifdef ZPL_NSM_TRUNK
#include "nsm_trunk.h"
#endif
#ifdef ZPL_NSM_MIRROR
#include "nsm_mirror.h"
#endif
#ifdef ZPL_NSM_TUNNEL
#include "nsm_tunnel.h"
#endif
#ifdef ZPL_NSM_SERIAL
#include "nsm_serial.h"
#endif
#ifdef ZPL_NSM_PPP
#include "nsm_ppp.h"
#endif
#ifdef ZPL_NSM_SECURITY
#include "nsm_security.h"
#endif
#ifdef ZPL_NSM_VLANETH
#include "nsm_vlaneth.h"
#endif

//#include "nsm_main.h"

#ifdef __cplusplus
}
#endif

#endif /* __NSM_INCLUDE_H__ */
