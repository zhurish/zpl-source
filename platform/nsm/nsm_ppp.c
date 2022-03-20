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
	if(!nsm_intf_module_data(ifp, NSM_INTF_PPP))
	{
		ppp = XMALLOC(MTYPE_PPP, sizeof(nsm_pppd_t));
		zassert(ppp);
		os_memset(ppp, 0, sizeof(nsm_pppd_t));
		nsm_intf_module_data_set(ifp, NSM_INTF_PPP, ppp);
	}
	else
	{
		ppp = nsm_intf_module_data(ifp, NSM_INTF_PPP);
	}
	nsm_ppp_interface_redisconnect(ppp, enable);
	if(ppp->enable != enable)
		ppp->enable = enable;
	return OK;
}







int nsm_ppp_interface_create_api(struct interface *ifp)
{
	nsm_pppd_t *ppp = NULL;
	if(!if_is_serial(ifp))
		return OK;
	ppp = nsm_intf_module_data(ifp, NSM_INTF_PPP);
	if(!ppp)
		ppp = XMALLOC(MTYPE_PPP, sizeof(nsm_pppd_t));
	zassert(ppp);
	os_memset(ppp, 0, sizeof(nsm_pppd_t));
	nsm_intf_module_data_set(ifp, NSM_INTF_PPP, ppp);
	return OK;
}


int nsm_ppp_interface_del_api(struct interface *ifp)
{
	nsm_pppd_t *ppp = NULL;
	if(!if_is_serial(ifp))
		return OK;
	ppp = nsm_intf_module_data(ifp, NSM_INTF_PPP);
	if(ppp)
		XFREE(MTYPE_PPP, ppp);
	ppp = NULL;
	nsm_intf_module_data_set(ifp, NSM_INTF_PPP, NULL);
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
	nsm_ppp = (nsm_pppd_t *)nsm_intf_module_data(ifp, NSM_INTF_PPP);
	if(!nsm_ppp)
		return OK;

	return OK;
}
#endif
int nsm_ppp_init(void)
{
	nsm_interface_hook_add(NSM_INTF_PPP, nsm_ppp_interface_create_api, nsm_ppp_interface_del_api);
	return OK;
}

int nsm_ppp_exit(void)
{
	return OK;
}
