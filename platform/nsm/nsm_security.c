/*
 * nsm_security.c
 *
 *  Created on: Apr 19, 2018
 *      Author: zhurish
 */


#include "zebra.h"
#include "memory.h"
#include "command.h"
#include "memory.h"
#include "memtypes.h"
#include "prefix.h"
#include "if.h"
#include "nsm_interface.h"
#include <log.h>

#include "os_list.h"

#include "nsm_security.h"


static nsm_security_t * _nsm_security_get(struct interface *ifp)
{
	struct nsm_interface *nsm = ifp->info[MODULE_NSM];
	if(nsm)
		return (nsm_security_t *)(nsm->nsm_client[NSM_SEC]);
	return NULL;
}









static int nsm_security_add_interface(struct interface *ifp)
{
	struct nsm_interface *nsm = ifp->info[MODULE_NSM];
	if(if_is_ethernet(ifp))
	{
		nsm_security_t *security = nsm->nsm_client[NSM_SEC] = XMALLOC(MTYPE_SECURITY, sizeof(nsm_security_t));
		os_memset(nsm->nsm_client[NSM_SEC], 0, sizeof(nsm_security_t));
		security->ifindex = ifp->ifindex;
	}
	return OK;
}


static int nsm_security_del_interface(struct interface *ifp)
{
	struct nsm_interface *nsm = ifp->info[MODULE_NSM];
	if(if_is_ethernet(ifp))
	{
		if(nsm->nsm_client[NSM_SEC])
			XFREE(MTYPE_SECURITY, nsm->nsm_client[NSM_SEC]);
		nsm->nsm_client[NSM_SEC] = NULL;
	}
	return OK;
}


static int nsm_security_interface_config(struct vty *vty, struct interface *ifp)
{
	nsm_security_t *security = _nsm_security_get(ifp);
	if(security && if_is_ethernet(ifp))
	{
/*		if(qos->qos_storm_enable)
		{
			//vty_out(vty, " storm enable%s", VTY_NEWLINE);
			vty_out(vty, " storm control unicast%s%d%s", (qos->qos_storm.qos_storm_type == STORM_PER) ? " percent ":" ",qos->qos_storm.qos_unicast, VTY_NEWLINE);
			vty_out(vty, " storm control multicast%s%d%s", (qos->qos_storm.qos_storm_type == STORM_PER) ? " percent ":" ",qos->qos_storm.qos_multicast, VTY_NEWLINE);
			vty_out(vty, " storm control broadcast%s%d%s", (qos->qos_storm.qos_storm_type == STORM_PER) ? " percent ":" ",qos->qos_storm.qos_broadcast, VTY_NEWLINE);
		}
*/
	}
	return OK;
}


static int nsm_security_client_init()
{
	struct nsm_client *nsm = nsm_client_new ();
	nsm->notify_add_cb = nsm_security_add_interface;
	nsm->notify_delete_cb = nsm_security_del_interface;
	nsm->interface_write_config_cb = nsm_security_interface_config;
	nsm_client_install (nsm, NSM_SEC);
	return OK;
}

int nsm_security_init()
{
	nsm_firewall_init();

	return nsm_security_client_init();
}

int nsm_security_exit()
{
	nsm_firewall_exit();
	struct nsm_client *nsm = nsm_client_lookup (NSM_SEC);
	if(nsm)
		nsm_client_free (nsm);
	return OK;
}


void cmd_security_init()
{
	cmd_firewall_init ();
}
