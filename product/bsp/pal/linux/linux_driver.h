/*
 * linux_driver.h
 *
 *  Created on: 2019年9月21日
 *      Author: zhurish
 */

#ifndef __LINUX_DRIVER_H__
#define __LINUX_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "nsm_include.h"
//#include "nsm_firewalld.h"

#include "linux_ioctl.h"
#include "linux_bond.h"
#include "linux_brigde.h"
#include "linux_firewalld.h"

#include "linux_address.h"
#include "linux_route.h"
#include "linux_iface.h"
#include "linux_tunnel.h"
#include "linux_vlaneth.h"
#include "linux_vrf.h"
#include "linux_arp.h"
#include "linux_netlink.h"

#ifdef ZPL_KERNEL_FORWARDING






int linux_ioctl_create(struct interface *ifp);
int linux_ioctl_destroy(struct interface *ifp);
int linux_ioctl_change(struct interface *ifp);
int linux_ioctl_set_vlan(struct interface *ifp, vlan_t vlan);

#endif






extern int iplinux_stack_init(void);


#ifdef __cplusplus
}
#endif


#endif /* __LINUX_DRIVER_H__ */
