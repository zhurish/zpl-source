/*
 * nsm_ppp.c
 *
 *  Created on: Aug 12, 2018
 *      Author: zhurish
 */

#include "os_include.h"
#include "zpl_include.h"
#include "lib_include.h"
#include "nsm_include.h"



int nsm_ppp_interface_redisconnect(nsm_pppd_t *ppp, zpl_bool enable)
{
	if(ppp->enable && !enable)
	{
		if(ppp->disconnect)
			(ppp->disconnect)(&ppp->pppd_options);
	}
	return OK;
}


int nsm_ppp_interface_enable(struct interface *ifp, zpl_bool enable)
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







int nsm_ppp_interface_create_api(struct interface *ifp)
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


int nsm_ppp_interface_del_api(struct interface *ifp)
{
	struct nsm_interface *nsm = ifp->info[MODULE_NSM];
	if(!if_is_serial(ifp))
		return OK;
	if(nsm->nsm_client[NSM_PPP])
		XFREE(MTYPE_PPP, nsm->nsm_client[NSM_PPP]);
	nsm->nsm_client[NSM_PPP] = NULL;
	return OK;
}


#ifdef ZPL_SHELL_MODULE
int nsm_ppp_interface_write_config(struct vty *vty, struct interface *ifp)
{
	//zpl_uint32 = 0;
	//int count = 0;
	//zpl_char tmp[128];
	zpl_char tmpcli_str[256];
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
#endif
int nsm_ppp_init()
{
	nsm_interface_hook_add(NSM_PPP, nsm_ppp_interface_create_api, nsm_ppp_interface_del_api);
	return OK;
}

int nsm_ppp_exit()
{
	return OK;
}
