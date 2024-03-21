/*
 * ip_brigde.c
 *
 *  Created on: Jul 14, 2018
 *      Author: zhurish
 */

/*
 * ip_brctl.c
 *
 *  Created on: Oct 16, 2016
 *      Author: zhurish
 */


#include "auto_include.h"
#include "zpl_type.h"
#include "module.h"
#include "zmemory.h"
#include "log.h"
#include "if.h"


#include "nsm_bridge.h"

#include "pal_include.h"
#include "linux_driver.h"
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <linux/if_packet.h>


#define _LINUX_IP_H

#include "linux/if_tunnel.h"
#include "linux/sockios.h"
#include "linux/if_bridge.h"

#ifdef ZPL_KERNEL_NETLINK

int linux_ioctl_bridge_create(struct interface *ifp)
{
	int ret;
	nsm_bridge_t *br = nsm_bridge_get(ifp);
	if(br == NULL)
		return ERROR;
#ifdef SIOCBRADDBR
	ret = linux_ioctl_if_ioctl(IPSTACK_SIOCBRADDBR, br->ifp->ker_name);
	if (ret < 0)
#endif
	{
		char _br[IFNAMSIZ];
		zpl_ulong arg[3] = { BRCTL_ADD_BRIDGE, (zpl_ulong) _br };
		strncpy(_br, br->ifp->ker_name, IFNAMSIZ);
		ret = linux_ioctl_if_ioctl(IPSTACK_SIOCSIFBR, (caddr_t)&arg);
	}
	return ret < 0 ? ipstack_errno : 0;
}

int linux_ioctl_bridge_delete(struct interface *ifp)
{
	int ret = -1;
	nsm_bridge_t *br = nsm_bridge_get(ifp);
	if(br == NULL)
		return ERROR;	
#ifdef SIOCBRDELBR
	ret = linux_ioctl_if_ioctl(IPSTACK_SIOCBRDELBR, br->ifp->ker_name);
	if (ret < 0)
#endif
	{
		char _br[IFNAMSIZ];
		zpl_ulong arg[3] = { BRCTL_DEL_BRIDGE, (zpl_ulong) _br };
		strncpy(_br, br->ifp->ker_name, IFNAMSIZ);
		ret = linux_ioctl_if_ioctl(IPSTACK_SIOCSIFBR, arg);
	}
	return  ret < 0 ? ipstack_errno : 0;
}

int linux_ioctl_bridge_add_interface(struct interface *ifp, struct interface *sifp)
{
	struct ipstack_ifreq ifr;
	int err = -1;
	nsm_bridge_t *br = nsm_bridge_get(ifp);
	if(br == NULL)
		return ERROR;	
	strncpy(ifr.ifr_name, br->ifp->ker_name, IFNAMSIZ);
#ifdef SIOCBRADDIF
	ifr.ifr_ifindex = sifp->ker_ifindex;
	err = linux_ioctl_if_ioctl(IPSTACK_SIOCBRADDIF, &ifr);
	if (err < 0)
#endif
	{
		zpl_ulong args[4] = { BRCTL_ADD_IF, sifp->ker_ifindex, 0, 0 };
		ifr.ifr_data = (char *) args;
		err = linux_ioctl_if_ioctl(IPSTACK_SIOCDEVPRIVATE, &ifr);
	}
	return err < 0 ? ipstack_errno : 0;
}

int linux_ioctl_bridge_del_interface(struct interface *ifp, struct interface *sifp)
{
	struct ipstack_ifreq ifr;
	int err = -1;
	nsm_bridge_t *br = nsm_bridge_get(ifp);
	if(br == NULL)
		return ERROR;	
	strncpy(ifr.ifr_name, br->ifp->ker_name, IFNAMSIZ);
#ifdef SIOCBRDELIF
	ifr.ifr_ifindex = sifp->ker_ifindex;
	err = linux_ioctl_if_ioctl(IPSTACK_SIOCBRDELIF, &ifr);
	if (err < 0)
#endif
	{
		zpl_ulong args[4] = { BRCTL_DEL_IF, sifp->ker_ifindex, 0, 0 };
		ifr.ifr_data = (char *) args;
		err = linux_ioctl_if_ioctl(IPSTACK_SIOCDEVPRIVATE, &ifr);
	}
	return err < 0 ? ipstack_errno : 0;
}


int linux_ioctl_bridge_list_interface(nsm_bridge_t *br, ifindex_t ifindex[])
{
	struct ipstack_ifreq ifr;
	int err = -1;
	ifindex_t port_index[BRIDGE_MEMBER_MAX];
	zpl_ulong args[4] = { BRCTL_GET_PORT_LIST,(zpl_ulong) &port_index, 0, 0 };
	strncpy(ifr.ifr_name, br->ifp->ker_name, IFNAMSIZ);
	ifr.ifr_data = (char *) &args;

	if (linux_ioctl_if_ioctl(IPSTACK_SIOCDEVPRIVATE, &ifr) < 0) {
	//	zlog_err(MODULE_PAL, "%s: can't get info %s\n",br->ifp->ker_name, strerror(ipstack_errno));
	//	return CMD_WARNING;
	}
	memcpy(ifindex, port_index, sizeof(port_index));
	return err < 0 ? ipstack_errno : 0;
}


int linux_ioctl_bridge_check_interface(char *br, ifindex_t ifindex)
{
	struct ipstack_ifreq ifr;
	//int err = -1;
	zpl_uint32 i = 0;
	ifindex_t port_index[BRIDGE_MEMBER_MAX] = {0};
	zpl_ulong bargs[4] = { BRCTL_GET_PORT_LIST,(zpl_ulong) &port_index, 0, 0 };
	for(i = 0; i < BRIDGE_MEMBER_MAX; i++)
	{
		port_index[i] = 0;
	}
	strcpy(ifr.ifr_name, br);
	ifr.ifr_data = (char *) &bargs;

	if (linux_ioctl_if_ioctl(IPSTACK_SIOCDEVPRIVATE, &ifr) < 0) {
	//	zlog_err(MODULE_PAL, "%s: can't get info %s\n",br->ifp->ker_name, strerror(ipstack_errno));
	//	return CMD_WARNING;
		return ERROR;
	}
	for(i = 0; i < BRIDGE_MEMBER_MAX; i++)
	{
		if(port_index[i] > 0 && port_index[i] == ifindex)
			return OK;
	}
	//memcpy(ifindex, port_index, sizeof(port_index));
	return ERROR;
}

#endif

