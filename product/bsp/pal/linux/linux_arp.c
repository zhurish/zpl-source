/*
 * kernel_arp.c
 *
 *  Created on: Sep 29, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include <zplos_include.h>
#include "module.h"
#include "zmemory.h"
#include "thread.h"
#include "if.h"
#include "vty.h"
#include "vrf.h"
#include "command.h"
#include "prefix.h"
#include "nsm_arp.h"
#include "nsm_firewalld.h"
#include "pal_include.h"
#include "linux_driver.h"

#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <linux/if_packet.h>

#ifdef ZPL_NSM_ARP
/* Delete an entry from the ARP cache. */
int kernel_arp_del(struct interface *ifp, struct prefix *address)
{
	struct ipstack_arpreq ipstack_arpreq;
	struct ipstack_sockaddr_in *sin;
	int rc;

	/*you must add this becasue some system will return "Invlid argument"
	 because some argument isn't zero */
	memset(&ipstack_arpreq, 0, sizeof(struct ipstack_arpreq));
	sin = (struct ipstack_sockaddr_in *) &ipstack_arpreq.arp_pa;
	memset(sin, 0, sizeof(struct ipstack_sockaddr_in));
	sin->sin_family = address->family;
	memcpy(&sin->sin_addr, (char *) &address->u.prefix4, sizeof(struct ipstack_in_addr));
	strcpy(ipstack_arpreq.arp_dev, ifp->k_name);

	rc = linux_ioctl_if_ioctl (IPSTACK_SIOCDARP, &ipstack_arpreq);
	if (rc < 0)
	{
		return -1;
	}
	return 0;
}

/* Set an entry in the ARP cache. */
int kernel_arp_set(struct interface *ifp, struct prefix *address, zpl_uint8 *mac)
{
	struct ipstack_arpreq ipstack_arpreq;
	struct ipstack_sockaddr_in *sin;
	int rc;
	/*you must add this becasue some system will return "Invlid argument"
	 because some argument isn't zero */
	memset(&ipstack_arpreq, 0, sizeof(struct ipstack_arpreq));
	sin = (struct ipstack_sockaddr_in *) &ipstack_arpreq.arp_pa;
	memset(sin, 0, sizeof(struct ipstack_sockaddr_in));
	sin->sin_family = address->family;
	memcpy(&sin->sin_addr, (char *) &address->u.prefix4, sizeof(struct ipstack_in_addr));
	strcpy(ipstack_arpreq.arp_dev, ifp->k_name);
	memcpy((zpl_uint8 *) ipstack_arpreq.arp_ha.sa_data, mac, 6);

	ipstack_arpreq.arp_flags = IPSTACK_ATF_PERM | IPSTACK_ATF_COM; //note, must set flag, if not,you will get error

	rc = linux_ioctl_if_ioctl (IPSTACK_SIOCSARP, &ipstack_arpreq);
	if (rc < 0)
	{
		return -1;
	}
	return 0;
}

int kernel_arp_get(struct interface *ifp, struct prefix *address, zpl_uint8 *mac)
{
	struct ipstack_arpreq ipstack_arpreq;
	struct ipstack_sockaddr_in *sin;
	int rc;

	/*you must add this becasue some system will return "Invlid argument"
	 because some argument isn't zero */
	memset(&ipstack_arpreq, 0, sizeof(struct ipstack_arpreq));
	sin = (struct ipstack_sockaddr_in *) &ipstack_arpreq.arp_pa;
	memset(sin, 0, sizeof(struct ipstack_sockaddr_in));
	sin->sin_family = address->family;
	memcpy(&sin->sin_addr, (char *) &address->u.prefix4, sizeof(struct ipstack_in_addr));
	strcpy(ipstack_arpreq.arp_dev, ifp->k_name);

	rc = linux_ioctl_if_ioctl (IPSTACK_SIOCGARP, &ipstack_arpreq);
	if (rc < 0)
	{
		return -1;
	}
	memcpy(mac, (zpl_uint8 *) ipstack_arpreq.arp_ha.sa_data, 6);
	return 0;
}


int kernel_arp_gratuitousarp_enable(int enable)
{
	return 0;
}




#endif
