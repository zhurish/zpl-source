/*
 * nsm_security.c
 *
 *  Created on: Apr 19, 2018
 *      Author: zhurish
 */


#include "os_include.h"
#include "zpl_include.h"
#include "lib_include.h"
#include "nsm_include.h"


static nsm_security_t * _nsm_security_get(struct interface *ifp)
{
	struct nsm_interface *nsm = ifp->info[MODULE_NSM];
	if(nsm)
		return (nsm_security_t *)(nsm->nsm_client[NSM_INTF_SEC]);
	return NULL;
}









int nsm_security_interface_create_api(struct interface *ifp)
{
	struct nsm_interface *nsm = ifp->info[MODULE_NSM];
	if(if_is_ethernet(ifp))
	{
		nsm_security_t *security = nsm->nsm_client[NSM_INTF_SEC] = XMALLOC(MTYPE_SECURITY, sizeof(nsm_security_t));
		os_memset(nsm->nsm_client[NSM_INTF_SEC], 0, sizeof(nsm_security_t));
		security->ifindex = ifp->ifindex;
	}
	return OK;
}


int nsm_security_interface_del_api(struct interface *ifp)
{
	struct nsm_interface *nsm = ifp->info[MODULE_NSM];
	if(if_is_ethernet(ifp))
	{
		if(nsm->nsm_client[NSM_INTF_SEC])
			XFREE(MTYPE_SECURITY, nsm->nsm_client[NSM_INTF_SEC]);
		nsm->nsm_client[NSM_INTF_SEC] = NULL;
	}
	return OK;
}

#ifdef ZPL_SHELL_MODULE
int nsm_security_interface_write_config(struct vty *vty, struct interface *ifp)
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
#endif



int nsm_security_init(void)
{
	nsm_interface_hook_add(NSM_INTF_SEC, nsm_security_interface_create_api, nsm_security_interface_del_api);
	//nsm_firewall_init();

	return OK;
}

int nsm_security_exit(void)
{
	//nsm_firewall_exit();
	return OK;
}


void cmd_security_init(void)
{
	//cmd_firewall_init ();
}
