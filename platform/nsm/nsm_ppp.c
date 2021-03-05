/*
 * nsm_ppp.c
 *
 *  Created on: Aug 12, 2018
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
#include "nsm_client.h"
#include "os_list.h"

#include "nsm_ppp.h"



int nsm_ppp_interface_redisconnect(nsm_pppd_t *ppp, ospl_bool enable)
{
	if(ppp->enable && !enable)
	{
		if(ppp->disconnect)
			(ppp->disconnect)(&ppp->pppd_options);
	}
	return OK;
}


int nsm_ppp_interface_enable(struct interface *ifp, ospl_bool enable)
{
	nsm_pppd_t *ppp = NULL;
	struct nsm_interface *nsm = ifp->info[MODULE_NSM];
	if(!nsm->nsm_client[NSM_PPP])
	{
		nsm->nsm_client[NSM_PPP] = XMALLOC(MTYPE_PPP, sizeof(nsm_pppd_t));
		zassert(nsm->nsm_client[NSM_PPP]);
		os_memset(nsm->nsm_client[NSM_PPP], 0, sizeof(nsm_pppd_t));
		ppp = nsm->nsm_client[NSM_PPP];
	}
	else
	{
		zassert(nsm->nsm_client[NSM_PPP]);
		ppp = nsm->nsm_client[NSM_PPP];
	}
	nsm_ppp_interface_redisconnect(ppp, enable);
	if(ppp->enable != enable)
		ppp->enable = enable;
	return OK;
}







static int nsm_ppp_add_interface(struct interface *ifp)
{
	struct nsm_interface *nsm = ifp->info[MODULE_NSM];
	if(!if_is_serial(ifp))
		return OK;
	if(!nsm->nsm_client[NSM_PPP])
		nsm->nsm_client[NSM_PPP] = XMALLOC(MTYPE_PPP, sizeof(nsm_pppd_t));
	zassert(nsm->nsm_client[NSM_PPP]);
	os_memset(nsm->nsm_client[NSM_PPP], 0, sizeof(nsm_pppd_t));
	return OK;
}


static int nsm_ppp_del_interface(struct interface *ifp)
{
	struct nsm_interface *nsm = ifp->info[MODULE_NSM];
	if(!if_is_serial(ifp))
		return OK;
	if(nsm->nsm_client[NSM_PPP])
		XFREE(MTYPE_PPP, nsm->nsm_client[NSM_PPP]);
	nsm->nsm_client[NSM_PPP] = NULL;
	return OK;
}



static int nsm_ppp_interface_config(struct vty *vty, struct interface *ifp)
{
	//ospl_uint32 = 0;
	//int count = 0;
	//ospl_char tmp[128];
	ospl_char tmpcli_str[256];
	struct nsm_interface *nsm_ifp = NULL;
	nsm_pppd_t *nsm_ppp = NULL;
	if(!if_is_serial(ifp))
		return OK;
	memset(tmpcli_str, 0, sizeof(tmpcli_str));
	nsm_ifp = (struct nsm_interface *)ifp->info[MODULE_NSM];
	nsm_ppp = (nsm_pppd_t *)nsm_ifp->nsm_client[NSM_PPP];
	if(!nsm_ppp)
		return OK;

	return OK;
}

int nsm_ppp_client_init()
{
	struct nsm_client *nsm = nsm_client_new ();
	nsm->notify_add_cb = nsm_ppp_add_interface;
	nsm->notify_delete_cb = nsm_ppp_del_interface;
	nsm->interface_write_config_cb = nsm_ppp_interface_config;
	nsm_client_install (nsm, NSM_PPP);
	return OK;
}

int nsm_ppp_client_exit()
{
	struct nsm_client *nsm = nsm_client_lookup (NSM_PPP);
	if(nsm)
		nsm_client_free (nsm);
	return OK;
}
