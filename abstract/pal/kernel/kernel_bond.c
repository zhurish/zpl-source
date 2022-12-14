/*
 * kernel_bond.c
 *
 *  Created on: Sep 14, 2018
 *      Author: zhurish
 */

#include <zebra.h>

#include "if.h"
#include "vty.h"
#include "sockunion.h"
#include "prefix.h"
#include "command.h"
#include "memory.h"
#include "log.h"
#include "zclient.h"
#include "thread.h"
#include "nsm_veth.h"
#include "nsm_tunnel.h"
#include "nsm_bridge.h"

#include "kernel_ioctl.h"

//#include "linux/if_tunnel.h"
//#include "linux/if_bonding.h"
#include "linux/sockios.h"

#if 0
#define SIOCBONDENSLAVE	0x8990		/* enslave a device to the bond */
#define SIOCBONDRELEASE 0x8991		/* release a slave from the bond*/
#define SIOCBONDSETHWADDR      0x8992	/* set the hw addr of the bond  */
#define SIOCBONDSLAVEINFOQUERY 0x8993   /* rtn info about slave state   */
#define SIOCBONDINFOQUERY      0x8994	/* rtn info about bond state    */
#define SIOCBONDCHANGEACTIVE   0x8995   /* update to a new active slave */

/* userland - kernel ABI version (2003/05/08) */
#define BOND_ABI_VERSION 2

/*
 * We can remove these ioctl definitions in 2.5.  People should use the
 * SIOC*** versions of them instead
 */
#define BOND_ENSLAVE_OLD		(SIOCDEVPRIVATE)
#define BOND_RELEASE_OLD		(SIOCDEVPRIVATE + 1)
#define BOND_SETHWADDR_OLD		(SIOCDEVPRIVATE + 2)
#define BOND_SLAVE_INFO_QUERY_OLD	(SIOCDEVPRIVATE + 11)
#define BOND_INFO_QUERY_OLD		(SIOCDEVPRIVATE + 12)
#define BOND_CHANGE_ACTIVE_OLD		(SIOCDEVPRIVATE + 13)

#define BOND_CHECK_MII_STATUS	(SIOCGMIIPHY)

#define BOND_MODE_ROUNDROBIN	0
#define BOND_MODE_ACTIVEBACKUP	1
#define BOND_MODE_XOR		2
#define BOND_MODE_BROADCAST	3
#define BOND_MODE_8023AD        4
#define BOND_MODE_TLB           5
#define BOND_MODE_ALB		6 /* TLB + RLB (receive load balancing) */

/* each slave's link has 4 states */
#define BOND_LINK_UP    0           /* link is up and running */
#define BOND_LINK_FAIL  1           /* link has just gone down */
#define BOND_LINK_DOWN  2           /* link has been down for too long time */
#define BOND_LINK_BACK  3           /* link is going back */

/* each slave has several states */
#define BOND_STATE_ACTIVE       0   /* link is active */
#define BOND_STATE_BACKUP       1   /* link is backup */

#define BOND_DEFAULT_MAX_BONDS  1   /* Default maximum number of devices to support */

#define BOND_DEFAULT_TX_QUEUES 16   /* Default number of tx queues per device */

#define BOND_DEFAULT_RESEND_IGMP	1 /* Default number of IGMP membership reports */

/* hashing types */
#define BOND_XMIT_POLICY_LAYER2		0 /* layer 2 (MAC only), default */
#define BOND_XMIT_POLICY_LAYER34	1 /* layer 3+4 (IP ^ (TCP || UDP)) */
#define BOND_XMIT_POLICY_LAYER23	2 /* layer 2+3 (IP ^ MAC) */
#define BOND_XMIT_POLICY_ENCAP23	3 /* encapsulated layer 2+3 */
#define BOND_XMIT_POLICY_ENCAP34	4 /* encapsulated layer 3+4 */

typedef struct ifbond {
	__s32 bond_mode;
	__s32 num_slaves;
	__s32 miimon;
} ifbond;

typedef struct ifslave {
	__s32 slave_id; /* Used as an IN param to the BOND_SLAVE_INFO_QUERY ioctl */
	char slave_name[IFNAMSIZ];
	__s8 link;
	__s8 state;
	__u32  link_failure_count;
} ifslave;

struct ad_info {
	__u16 aggregator_id;
	__u16 ports;
	__u16 actor_key;
	__u16 partner_key;
	__u8 partner_system[ETH_ALEN];
};

#endif

#if 0
static int _if_bond_add_slave(struct interface *ifp, struct interface *slave)
{
	struct ifreq ifr;
	strcpy (ifr.ifr_name, "bond1212");
	strcpy (ifr.ifr_slave, "enp0s25");
/*

	strcpy (ifr.ifr_name, ifp->k_name);
	strcpy (ifr.ifr_slave, slave->k_name);
*/

	if (if_ioctl (SIOCBONDENSLAVE, (caddr_t) &ifr) < 0)
	    return -1;
	return 0;
}


static int _if_bond_delete_slave(struct interface *ifp, struct interface *slave)
{
	struct ifreq ifr;
	strcpy (ifr.ifr_name, ifp->k_name);
	strcpy (ifr.ifr_slave, slave->k_name);

	if (if_ioctl (SIOCBONDRELEASE, (caddr_t) &ifr) < 0)
	    return -1;
	return 0;
}



static int _if_bond_slave_active(struct interface *ifp, struct interface *slave)
{
	struct ifreq ifr;
	strcpy (ifr.ifr_name, ifp->k_name);
	strcpy (ifr.ifr_slave, slave->k_name);

	if (if_ioctl (SIOCBONDCHANGEACTIVE, (caddr_t) &ifr) < 0)
	    return -1;
	return 0;
}
#endif


int _ipkernel_bond_create(struct interface *ifp)
{
	return 	kernel_create_interface(ifp);
}


int _ipkernel_bond_delete(struct interface *ifp)
{
	return kernel_destroy_interface(ifp);
}


int _if_bond_test()
{
	struct interface ifp;
	memset(&ifp, 0, sizeof(ifp));
	ifp.vrf_id = 0;
	strcpy(ifp.k_name, "bond1223");

	//_if_bond_create(NULL, NULL);
#ifdef PL_IPROUTE2_MODULE
	int argc = 2;
	char *argv[] = {"ip", "link", "show", NULL};
	ip_main( argc, argv);
#endif
	return 0;
}
