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
#include "interface.h"
#include <log.h>
#include "nsm_client.h"
#include "os_list.h"

#include "nsm_ppp.h"



int nsm_ppp_interface_redisconnect(nsm_pppd_t *ppp, BOOL enable)
{
	if(ppp->enable && !enable)
	{
		if(ppp->disconnect)
			(ppp->disconnect)(&ppp->pppd_options);
	}
	return OK;
}


int nsm_ppp_interface_enable(struct interface *ifp, BOOL enable)
{
	nsm_pppd_t *ppp = NULL;
	struct nsm_interface *nsm = ifp->info[ZLOG_NSM];
	if(!nsm->nsm_client[NSM_PPP])
	{
		nsm->nsm_client[NSM_PPP] = XMALLOC(NSM_PPP, sizeof(nsm_pppd_t));
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
	struct nsm_interface *nsm = ifp->info[ZLOG_NSM];
	if(!nsm->nsm_client[NSM_PPP])
		nsm->nsm_client[NSM_PPP] = XMALLOC(NSM_PPP, sizeof(nsm_pppd_t));
	zassert(nsm->nsm_client[NSM_PPP]);
	os_memset(nsm->nsm_client[NSM_PPP], 0, sizeof(nsm_pppd_t));
	return OK;
}


static int nsm_ppp_del_interface(struct interface *ifp)
{
	struct nsm_interface *nsm = ifp->info[ZLOG_NSM];
	if(nsm->nsm_client[NSM_PPP])
		XFREE(NSM_PPP, nsm->nsm_client[NSM_PPP]);
	return OK;
}



static int nsm_ppp_interface_config(struct vty *vty, struct interface *ifp)
{
	int i = 0;
	int count = 0;
	char tmp[128];
	char tmpcli_str[256];
	struct nsm_interface *nsm_ifp = NULL;
	nsm_pppd_t *nsm_ppp = NULL;
	memset(tmpcli_str, 0, sizeof(tmpcli_str));
	nsm_ifp = (struct nsm_interface *)ifp->info[ZLOG_NSM];
	nsm_ppp = (nsm_pppd_t *)nsm_ifp->nsm_client[NSM_PPP];
	if(!nsm_ppp)
		return OK;

	return OK;
}

static int nsm_ppp_client_init()
{
	struct nsm_client *nsm = nsm_client_new ();
	nsm->interface_add_cb = nsm_ppp_add_interface;
	nsm->interface_delete_cb = nsm_ppp_del_interface;
	nsm->interface_write_config_cb = nsm_ppp_interface_config;
	nsm_client_install (nsm, NSM_PPP);
	return OK;
}
