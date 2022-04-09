/*
 * nsm_bridge.c
 *
 *  Created on: Sep 13, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "zmemory.h"
#include "if.h"
#include "prefix.h"
#include "nsm_include.h"
#include "hal_include.h"

#ifdef ZPL_WIFI_MODULE
#include "iw_ap.h"
#endif

nsm_bridge_t * nsm_bridge_get(struct interface *ifp)
{
	return nsm_intf_module_data(ifp, NSM_INTF_BRIDGE);
}

static int nsm_bridge_member_lookup(nsm_bridge_t *bridge, ifindex_t ifindex)
{
	zpl_uint32 i = 0;
	for(i = 0; i < BRIDGE_MEMBER_MAX; i++)
	{
		if(bridge->member[i] == ifindex)
			return 1;
	}
	return 0;
}

static int nsm_bridge_member_add(nsm_bridge_t *bridge, ifindex_t ifindex)
{
	int ret = 0, i = 0;
	i = nsm_bridge_member_lookup(bridge, ifindex);
	if(i)
		return ERROR;
	for(i = 0; i < BRIDGE_MEMBER_MAX; i++)
	{
		if(bridge->member[i] == 0)
		{
			if(bridge->add_member_cb)
				ret = bridge->add_member_cb(bridge, ifindex);
			if(ret)
				return 0;
			bridge->member[i] = ifindex;
			return 1;
		}
	}
	return 0;
}

static int nsm_bridge_member_del(nsm_bridge_t *bridge, ifindex_t ifindex)
{
	int ret = 0,i = 0;
	i = nsm_bridge_member_lookup(bridge, ifindex);
	if(i == 0)
		return ERROR;
	for(i = 0; i < BRIDGE_MEMBER_MAX; i++)
	{
		if(bridge->member[i] == ifindex)
		{
			if(bridge->del_member_cb)
				ret = bridge->del_member_cb(bridge, ifindex);
			if(ret)
				return 0;
			bridge->member[i] = 0;
			return 1;
		}
	}
	return 0;
}

static int nsm_bridge_member_del_all(nsm_bridge_t *bridge)
{
	zpl_uint32 i = 0;
	struct interface *ifp = NULL;
	for(i = 0; i < BRIDGE_MEMBER_MAX; i++)
	{
		if(bridge->member[i])
		{
			ifp = if_lookup_by_index(bridge->member[i]);
			if(ifp)
				UNSET_FLAG(ifp->ifmember, IF_BRIDGE_MEM);
		}
	}
	return 0;
}

static int nsm_bridge_interface_add_check(zpl_bool add, struct interface *ifp)
{
	if(add)
	{
		if(if_is_brigde(ifp) || if_is_ethernet(ifp))
		{
			if(!if_is_lag_member(ifp) && !if_is_brigde_member(ifp))
				return 1;
		}
		return 0;
	}
	else
	{
		if(if_is_brigde(ifp) || if_is_ethernet(ifp))
		{
			if(if_is_brigde_member(ifp))
				return 1;
		}
		return 0;
	}
	return 0;
}

int nsm_bridge_add_interface_api(struct interface *bridge, struct interface *ifp)
{
	if(if_is_brigde(bridge))
	{
		int ret = -1;
		if(!nsm_bridge_interface_add_check(zpl_true, ifp))
			return ERROR;
		nsm_bridge_t * bri = nsm_bridge_get(bridge);
		if(bri)
			ret = nsm_bridge_member_add(bri, ifp->ifindex);
		if(ret == 0)
		{
			SET_FLAG(ifp->ifmember, IF_BRIDGE_MEM);
#ifdef ZPL_WIFI_MODULE
			if(iw_ap_lookup_api(ifp))
				iw_ap_bridge_set_api(iw_ap_lookup_api(ifp), bridge->ifindex);
#endif
			return ret;
		}
	}
	return ERROR;
}

int nsm_bridge_del_interface_api(struct interface *bridge, struct interface *ifp)
{
	if(if_is_brigde(bridge))
	{
		int ret = -1;
		if(!nsm_bridge_interface_add_check(zpl_false, ifp))
			return ERROR;
		nsm_bridge_t * bri = nsm_bridge_get(bridge);
		if(bri)
			ret = nsm_bridge_member_del(bri, ifp->ifindex);
		if(ret == 0)
		{
			UNSET_FLAG(ifp->ifmember, IF_BRIDGE_MEM);
#ifdef ZPL_WIFI_MODULE
			if(iw_ap_lookup_api(ifp))
				iw_ap_bridge_set_api(iw_ap_lookup_api(ifp), 0);
#endif
			return ret;
		}
	}
	return ERROR;
}

int nsm_bridge_update_member_api(struct interface *bridge)
{
	if (if_is_brigde(bridge))
	{
		int ret = -1;
		nsm_bridge_t * bri = nsm_bridge_get(bridge);
		if (bri)
		{
			int kifindex[BRIDGE_MEMBER_MAX];
			memset(kifindex, 0, sizeof(kifindex));
			if (bri->get_member_cb)
				ret = bri->get_member_cb(bri, kifindex);
			if (ret)
			{
				zpl_uint32 i = 0;
				nsm_bridge_member_del_all(bri);
				for(i = 0; i < BRIDGE_MEMBER_MAX; i++)
				{
					if(kifindex[i] > 0 && ifkernel2ifindex(kifindex[i]) > 0)
					{
						nsm_bridge_member_add(bri, ifkernel2ifindex(kifindex[i]));
					}
				}
				//extern ifindex_t ifindex2ifkernel(ifindex_t);
				//extern ifindex_t ifkernel2ifindex(ifindex_t);
				return OK;
			}
		}
	}
	return ERROR;
}


int nsm_bridge_interface_stp_set_api(struct interface *bridge, zpl_bool stp)
{
	if(if_is_brigde(bridge))
	{
		//int ret = -1;
		nsm_bridge_t * bri = nsm_bridge_get(bridge);
		if(bri)
		{
			bri->br_stp = stp;//网桥生成树
			return OK;
		}
	}
	return ERROR;
}

int nsm_bridge_interface_max_age_set_api(struct interface *bridge, zpl_uint32 max_age)
{
	if(if_is_brigde(bridge))
	{
		//int ret = -1;
		nsm_bridge_t * bri = nsm_bridge_get(bridge);
		if(bri)
		{
			bri->max_age = max_age;//网桥生成树
			return OK;
		}
	}
	return ERROR;
}

int nsm_bridge_interface_hello_time_set_api(struct interface *bridge, zpl_uint32 hello_time)
{
	if(if_is_brigde(bridge))
	{
		//int ret = -1;
		nsm_bridge_t * bri = nsm_bridge_get(bridge);
		if(bri)
		{
			bri->hello_time = hello_time;//网桥生成树
			return OK;
		}
	}
	return ERROR;
}

int nsm_bridge_interface_forward_delay_set_api(struct interface *bridge, zpl_uint32 forward_delay)
{
	if(if_is_brigde(bridge))
	{
		//int ret = -1;
		nsm_bridge_t * bri = nsm_bridge_get(bridge);
		if(bri)
		{
			bri->forward_delay = forward_delay;//网桥生成树
			return OK;
		}
	}
	return ERROR;
}

int nsm_bridge_interface_create_api(struct interface *ifp)
{
	nsm_bridge_t * bridge = NULL;
	if(if_is_brigde(ifp))
	{
		bridge = XMALLOC(MTYPE_IF, sizeof(nsm_bridge_t));
		zassert(bridge);
		os_memset(bridge, 0, sizeof(nsm_bridge_t));
		bridge->ifp = ifp;
		bridge->br_mode = BRIDGE_IF;
		nsm_intf_module_data_set(ifp, NSM_INTF_BRIDGE, bridge);
	}
	return OK;
}


int nsm_bridge_interface_del_api(struct interface *ifp)
{
	if(if_is_brigde(ifp))
	{
		nsm_bridge_t * bridge = nsm_bridge_get(ifp);
		if(bridge)
		{
			nsm_bridge_member_del_all(bridge);
			struct nsm_interface *nsm = ifp->info[MODULE_NSM];
			XFREE(MTYPE_IF, bridge);
			nsm_intf_module_data_set(ifp, NSM_INTF_BRIDGE, NULL);
		}
	}
	return OK;
}
#ifdef ZPL_SHELL_MODULE
int nsm_bridge_interface_write_config(struct vty *vty, struct interface *ifp)
{
	zpl_uint32 i = 0;
	if(if_is_brigde(ifp))
	{
		nsm_bridge_t * bridge = nsm_bridge_get(ifp);
		if(bridge && bridge->br_mode == BRIDGE_IF)
		{
			for(i = 0; i < BRIDGE_MEMBER_MAX; i++)
			{
				if(bridge->member[i])
				{
					vty_out(vty, " bridge-interface %s%s", ifindex2ifname(bridge->member[i]), VTY_NEWLINE);
				}
			}
			if(bridge->br_stp)
				vty_out(vty, " bridge-protocol ieee%s", VTY_NEWLINE);
			if(bridge->max_age)
				vty_out(vty, " ieee age %d%s", bridge->max_age, VTY_NEWLINE);

			if(bridge->hello_time)
				vty_out(vty, " ieee hello-time %d%s", bridge->hello_time, VTY_NEWLINE);

			if(bridge->forward_delay)
				vty_out(vty, " ieee forward-delay %d%s", bridge->forward_delay, VTY_NEWLINE);
		}
	}
	return OK;
}
#endif
int nsm_bridge_init(void)
{
	nsm_interface_hook_add(NSM_INTF_BRIDGE, nsm_bridge_interface_create_api, nsm_bridge_interface_del_api);
	return OK;
}

int nsm_bridge_exit(void)
{
	return OK;
}

