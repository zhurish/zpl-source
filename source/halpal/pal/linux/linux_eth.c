/*
 * ipkernel_vlaneth.c
 *
 *  Created on: Sep 13, 2018
 *      Author: zhurish
 */


#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"
#include "thread.h"
#include "if.h"
#include "vty.h"
#include "command.h"
#include "prefix.h"

#include "pal_include.h"
#include "nsm_debug.h"
#include "nsm_vlan.h"
#include "nsm_arp.h"
#include "nsm_bridge.h"
#include "nsm_firewalld.h"

#include "linux_driver.h"
#define _LINUX_IP_H
#include <linux/if_tun.h>
#include "linux/if_vlan.h"
#include "linux/sockios.h"

#ifdef ZPL_KERNEL_NETLINK

#define IPKERNEL_TUN_NAME	"/dev/net/tun"
//#define IPKERNEL_TAP_NAME	"/dev/tap"
#define IPKERNEL_TAP_NAME	"/dev/net/tun"
static int _linux_kernel_eth_create (nsm_vlaneth_t *kifp)
{
	struct ipstack_ifreq ifr;
	struct interface *ifp;
    zpl_uchar macadd[6] = {0x20,0x5f,0x87,0x65,0x66,0x00};
	if (!kifp || !kifp->ifp)
		return ERROR;
	ifp = kifp->ifp;
	memset(&ifr, 0, sizeof(ifr));
	/*
	 *if we are using the tap driver here
	 *for the dtl interface then we need to
	 *a file descriptor to it and set up
	 *the corresponding network interface
	 */
	if(if_is_serial(ifp))
	{
		kifp->fd = ipstack_open(IPSTACK_IPCOM, IPKERNEL_TUN_NAME, O_RDWR);
		if (ipstack_invalid(kifp->fd))
		{
			zlog_err(MODULE_PAL, "Unable to open %s to create L3 interface(%s).",
					IPKERNEL_TUN_NAME, ipstack_strerror(ipstack_errno));
			return ERROR;
		}
	}
	else
	{
		kifp->fd = ipstack_open(IPSTACK_IPCOM, IPKERNEL_TAP_NAME, O_RDWR);
		if (ipstack_invalid(kifp->fd))
		{
			zlog_err(MODULE_PAL, "Unable to open %s to create L3 interface(%s).",
					IPKERNEL_TAP_NAME, ipstack_strerror(ipstack_errno));
			return ERROR;
		}
	}
	/*
	 *register this fd with the tap monitor.
	 *make sure to add a dtlCmd function
	 */
	if(if_is_serial(ifp))
	{
		ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
	}
	else
	{
		ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
	}
	strcpy(ifr.ifr_name, ifp->ker_name);
	/*
	 *Inidicate that this is a tap interface
	 *and that we provide no additional packet information
	 */

	if (ipstack_ioctl(kifp->fd, IPSTACK_TUNSETIFF, &ifr) < 0)
	{
		zlog_err(MODULE_PAL, "Unable to create corresponding L3 interface %s(%s).",
				ifp->name, ipstack_strerror(ipstack_errno));
		ipstack_close(kifp->fd);
		return ERROR;
	}

    macadd[5] = ifp->ifindex & 0xff;
    if(pal_interface_set_lladdr(ifp, macadd, 6) != OK)
	{
		zlog_err(MODULE_PAL, "Unable to set L3 interface  mac %s(%s).",
				ifp->name, ipstack_strerror(ipstack_errno));
		ipstack_close(kifp->fd);
		return ERROR;
	}
	ipstack_set_nonblocking(kifp->fd);
	ifp->ker_ifindex = if_nametoindex(ifp->ker_name);
	linux_ioctl_if_set_up(ifp);
	return OK;
}

static int _linux_kernel_eth_destroy (nsm_vlaneth_t *kifp)
{
	struct interface *ifp;
	if(!kifp)
		return -1;
	ifp = kifp->ifp;

	pal_interface_down (ifp);
	ipstack_close (kifp->fd);

	ifp->ker_ifindex = 0;
	return OK;

}




int linux_ioctl_eth_create (struct interface *ifp)
{
	nsm_vlaneth_t *kifp = nsm_vlaneth_get(ifp);
	if(kifp)
	{
		if(kifp->ifp->ll_type != IF_LLT_MODEM)
			return _linux_kernel_eth_create(kifp);
		return OK;
	}
	return ERROR;
}

int linux_ioctl_eth_destroy (struct interface *ifp)
{
	nsm_vlaneth_t *kifp = nsm_vlaneth_get(ifp);
	if(kifp)
	{
		if(kifp->ifp->ll_type != IF_LLT_MODEM)
			return _linux_kernel_eth_destroy(kifp);
		return OK;
	}
	return ERROR;
}


#endif