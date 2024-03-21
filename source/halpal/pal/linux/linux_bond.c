/*
 * kernel_bond.c
 *
 *  Created on: Sep 14, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zpl_type.h"
#include "module.h"
#include "zmemory.h"
#include "log.h"
#include "if.h"

#include "pal_include.h"
#include "linux_driver.h"
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <linux/if_packet.h>


//#include "linux/if_tunnel.h"
//#include "linux/if_bonding.h"
#include "linux/sockios.h"

#ifdef ZPL_KERNEL_NETLINK
int _if_bond_add_slave(struct interface *ifp, struct interface *slave)
{
	struct ipstack_ifreq ifr;
	//strcpy (ifr.ifr_name, "bond1212");
	//strcpy (ifr.ifr_slave, "enp0s25");


	strcpy (ifr.ifr_name, ifp->ker_name);
	strcpy (ifr.ifr_slave, slave->ker_name);


	if (linux_ioctl_if_ioctl (SIOCBONDENSLAVE, (caddr_t) &ifr) < 0)
	    return -1;
	return 0;
}


int _if_bond_delete_slave(struct interface *ifp, struct interface *slave)
{
	struct ipstack_ifreq ifr;
	strcpy (ifr.ifr_name, ifp->ker_name);
	strcpy (ifr.ifr_slave, slave->ker_name);

	if (linux_ioctl_if_ioctl (SIOCBONDRELEASE, (caddr_t) &ifr) < 0)
	    return -1;
	return 0;
}



int _if_bond_slave_active(struct interface *ifp, struct interface *slave)
{
	struct ipstack_ifreq ifr;
	strcpy (ifr.ifr_name, ifp->ker_name);
	strcpy (ifr.ifr_slave, slave->ker_name);

	if (linux_ioctl_if_ioctl (SIOCBONDCHANGEACTIVE, (caddr_t) &ifr) < 0)
	    return -1;
	return 0;
}



int linux_ioctl_bond_create(struct interface *ifp)
{
	return 	librtnl_create_interface(ifp);
}


int linux_ioctl_bond_delete(struct interface *ifp)
{
	return librtnl_destroy_interface(ifp);
}

#endif
