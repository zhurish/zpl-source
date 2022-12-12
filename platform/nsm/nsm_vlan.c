/*
 * nsm_vlan.c
 *
 *  Created on: Jan 8, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "if.h"
#include "vty.h"
#include "zmemory.h"
#include "log.h"
#include "template.h"
#include "hash.h"
#include "nsm_include.h"
#include "hal_include.h"

#ifdef ZPL_HAL_MODULE
#include "hal_vlan.h"
#endif



static int nsm_vlan_mode_callback(struct interface *, if_mode_t, if_mode_t);


int nsm_vlan_init(void)
{
	nsm_vlan_database_init();
	nsm_interface_hook_add(NSM_INTF_VLAN, nsm_vlan_interface_create_api, nsm_vlan_interface_del_api);
	nsm_interface_write_hook_add(NSM_INTF_VLAN, nsm_vlan_interface_write_config);
	nsm_interface_mode_hook_add(NSM_INTF_VLAN, nsm_vlan_mode_callback);

	//nsm_vlan_client_init();
	//nsm_vlan_create_api(1, NULL);
	//l2vlan_add_untag_port(1, ifindex_t ifindex);
	return OK;
}


static int nsm_vlan_database_default_interface(struct interface *ifp, void *pVoid)
{
	if(if_is_ethernet(ifp))
	{
		nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm_intf_module_data(ifp, NSM_INTF_VLAN);
		if(nsm_vlan_database_lookup_api(1))
		{
			nsm_vlan_database_add_port( 1, ifp->ifindex, NSM_VLAN_UNTAG);//把该接口加入到vlan的untag接口列表
			nsm_vlan_database_del_port( 1, ifp->ifindex, NSM_VLAN_TAG);//把该接口从vlan的tag列表删除
			nsm_vlan->access = 1;
		}
		return OK;
	}
	return OK;	
}

int nsm_vlan_exit(void)
{
	return nsm_vlan_database_exit();	
}

int nsm_vlan_cleanall(void)
{
	return nsm_vlan_database_cleanall();	
}


int nsm_vlan_default(void)
{
	if(nsm_vlan_database_default() == OK)
		if_list_each(nsm_vlan_database_default_interface, NULL);
	return OK;
}

int nsm_vlan_enable(zpl_bool enable)
{
	return nsm_vlan_database_enable(enable);	
}

zpl_bool nsm_vlan_is_enable(void)
{
	return nsm_vlan_database_is_enable();	
}



static int nsm_vlan_index_max(nsm_vlan_t *nsm_vlan)
{
	zpl_uint32 i = 0;
	for(i = 0; i < VLAN_TABLE_MAX; i++)
	{
		if(nsm_vlan->trunk_allowed[i].maxvlan)
			nsm_vlan->allowed_max = MAX(nsm_vlan->allowed_max, nsm_vlan->trunk_allowed[i].maxvlan);
		else
			nsm_vlan->allowed_max = MAX(nsm_vlan->allowed_max, nsm_vlan->trunk_allowed[i].vlan);
	}
	return (nsm_vlan->allowed_max + 1);
}







int nsm_interface_native_vlan_set_api(struct interface *ifp, vlan_t vlan)
{
	int ret = ERROR;
	zassert(ifp);
	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm_intf_module_data(ifp, NSM_INTF_VLAN);
	if(!if_is_ethernet(ifp))
		return ERROR;	
	zassert(nsm_vlan);
	IF_NSM_VLAN_DATA_LOCK(nsm_vlan);
	if(vlan)
	{
		if(nsm_vlan_database_lookup_api(vlan))
		{
			if(nsm_vlan->native != vlan)
			{
#ifdef ZPL_HAL_MODULE
				ret = hal_port_add_native_vlan(ifp->ifindex, vlan);
#else
				ret = OK;
#endif
				if(ret == OK)
				{
					nsm_vlan->native = vlan;
					//nsm_vlan_database_untag_port_update(ifp->ifindex);//把该接口从其他vlan的untag列表删除
					nsm_vlan_database_add_port( vlan, ifp->ifindex, NSM_VLAN_TAG);
					nsm_vlan_database_del_port( vlan, ifp->ifindex, NSM_VLAN_UNTAG);
				}
			}
		}
		else
		{
			zpl_errno_set(IPSTACK_ERRNO_EEXIST);
			ret = ERROR;
		}
	}
	else
	{
#ifdef ZPL_HAL_MODULE
		ret = hal_port_del_native_vlan(ifp->ifindex, nsm_vlan->native);
#else
		ret = OK;
#endif
		if(ret == OK)
		{
			//nsm_vlan_database_add_port( vlan, ifp->ifindex, NSM_VLAN_TAG);
			nsm_vlan_database_del_port( nsm_vlan->native, ifp->ifindex, NSM_VLAN_TAG);
			nsm_vlan->native = vlan;
		}
	}
	IF_NSM_VLAN_DATA_UNLOCK(nsm_vlan);
	return ret;
}


int nsm_interface_native_vlan_get_api(struct interface *ifp, vlan_t *vlan)
{
	int ret = OK;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm_intf_module_data(ifp, NSM_INTF_VLAN);
	if(!if_is_ethernet(ifp))
		return ERROR;	
	zassert(nsm_vlan);
	IF_NSM_VLAN_DATA_LOCK(nsm_vlan);
	if(nsm_vlan)
	{
		if(vlan)
			*vlan = nsm_vlan->native;
	}
	IF_NSM_VLAN_DATA_UNLOCK(nsm_vlan);
	return ret;
}

int nsm_interface_access_vlan_set_api(struct interface *ifp, vlan_t vlan)
{
	int ret = ERROR;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm_intf_module_data(ifp, NSM_INTF_VLAN);
	if(!if_is_ethernet(ifp))
		return ERROR;	
	zassert(nsm_vlan);
	IF_NSM_VLAN_DATA_LOCK(nsm_vlan);
	if(nsm_vlan_database_lookup_api(vlan))
	{
#ifdef ZPL_HAL_MODULE
		ret = hal_port_add_access_vlan(ifp->ifindex, vlan);
#else
		ret = OK;
#endif
		if(ret == OK)
		{
			nsm_vlan_database_add_port( vlan, ifp->ifindex, NSM_VLAN_UNTAG);//把该接口加入到vlan的untag接口列表
			nsm_vlan_database_del_port( vlan, ifp->ifindex, NSM_VLAN_TAG);//把该接口从vlan的tag列表删除
			nsm_vlan->access = vlan;
		}
	}
	IF_NSM_VLAN_DATA_UNLOCK(nsm_vlan);
	return ret;
}

int nsm_interface_access_vlan_unset_api(struct interface *ifp, vlan_t vlan)
{
	int ret = ERROR;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm_intf_module_data(ifp, NSM_INTF_VLAN);
	if(!if_is_ethernet(ifp))
		return ERROR;	
	zassert(nsm_vlan);
	IF_NSM_VLAN_DATA_LOCK(nsm_vlan);
	if(nsm_vlan_database_lookup_api(vlan))
	{
#ifdef ZPL_HAL_MODULE
		ret = hal_port_del_access_vlan(ifp->ifindex, vlan);
#else
		ret = OK;
#endif
		if(ret == OK)
		{
			//nsm_vlan_database_add_port( nsm_vlan->access, ifp->ifindex, NSM_VLAN_UNTAG);
			nsm_vlan_database_del_port( nsm_vlan->access, ifp->ifindex, NSM_VLAN_UNTAG);//把该接口从vlan的tag列表删除
			nsm_vlan->access = 0;
		}
	}
	IF_NSM_VLAN_DATA_UNLOCK(nsm_vlan);
	if(ret == OK)
		ret = nsm_interface_access_vlan_set_api(ifp, 1);//重新加入到vlan 1
	return ret;
}

int nsm_interface_access_vlan_get_api(struct interface *ifp, vlan_t *vlan)
{
	int ret = OK;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm_intf_module_data(ifp, NSM_INTF_VLAN);
	if(!if_is_ethernet(ifp))
		return ERROR;	
	zassert(nsm_vlan);
	IF_NSM_VLAN_DATA_LOCK(nsm_vlan);
	if(nsm_vlan)
	{
		if(vlan)
			*vlan = nsm_vlan->access;
	}
	IF_NSM_VLAN_DATA_UNLOCK(nsm_vlan);
	return ret;
}


int nsm_interface_trunk_add_allowed_vlan_lookup_api(struct interface *ifp, vlan_t vlan)
{
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm_intf_module_data(ifp, NSM_INTF_VLAN);
	if(!if_is_ethernet(ifp))
		return ERROR;
	zassert(nsm_vlan);
	IF_NSM_VLAN_DATA_LOCK(nsm_vlan);

	if(vlan)
	{
		if(nsm_vlan->allow_all)
		{
			IF_NSM_VLAN_DATA_UNLOCK(nsm_vlan);
			return 1;
		}	
		else if(vlan >= nsm_vlan->trunk_allowed[vlan].minvlan && vlan <= nsm_vlan->trunk_allowed[vlan].maxvlan)
		{
			IF_NSM_VLAN_DATA_UNLOCK(nsm_vlan);
			return 1;
		}		
		else if(nsm_vlan->trunk_allowed[vlan].vlan == vlan)
		{
			IF_NSM_VLAN_DATA_UNLOCK(nsm_vlan);
			return 1;
		}
		else
		{
			IF_NSM_VLAN_DATA_UNLOCK(nsm_vlan);
			return 0;
		}
	}
	IF_NSM_VLAN_DATA_UNLOCK(nsm_vlan);
	return 0;
}
int nsm_interface_trunk_add_allowed_vlan_api(struct interface *ifp, vlan_t vlan)
{
	int ret = ERROR;
	//zpl_uint32 =0;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm_intf_module_data(ifp, NSM_INTF_VLAN);
	if(!if_is_ethernet(ifp))
		return ERROR;
	zassert(nsm_vlan);
	IF_NSM_VLAN_DATA_LOCK(nsm_vlan);
	//if(nsm_vlan_database_lookup_api(vlan))
	{
		if(vlan && nsm_vlan_database_lookup_api(vlan))
		{
			if(nsm_vlan->trunk_allowed[vlan].vlan == 0)
			{
#ifdef ZPL_HAL_MODULE
				ret = hal_port_add_allowed_tag_vlan(ifp->ifindex, vlan);
#else
				ret = OK;
#endif
				if(ret == OK)
				{
					nsm_vlan->trunk_allowed[vlan].vlan = vlan;
					nsm_vlan_index_max(nsm_vlan);
					nsm_vlan_database_add_port( vlan, ifp->ifindex, NSM_VLAN_TAG);
					nsm_vlan_database_del_port( vlan, ifp->ifindex, NSM_VLAN_UNTAG);
				}
			}
		}
		else
		{
#ifdef ZPL_HAL_MODULE
			ret = hal_port_add_allowed_tag_vlan(ifp->ifindex, vlan);
#else
			ret = OK;
#endif
			if(ret == OK)
			{
				nsm_vlan->allow_all = zpl_true;
			}
		}
	}
	IF_NSM_VLAN_DATA_UNLOCK(nsm_vlan);
	return ret;
}

int nsm_interface_trunk_del_allowed_vlan_api(struct interface *ifp, vlan_t vlan)
{
	int ret = ERROR;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm_intf_module_data(ifp, NSM_INTF_VLAN);
	if(!if_is_ethernet(ifp))
		return ERROR;
	zassert(nsm_vlan);
	IF_NSM_VLAN_DATA_LOCK(nsm_vlan);
	//if(nsm_vlan_database_lookup_api(vlan))
	{
		if(vlan && nsm_vlan_database_lookup_api(vlan))
		{
			if(nsm_vlan->trunk_allowed[vlan].vlan)
			{
#ifdef ZPL_HAL_MODULE
				ret = hal_port_del_allowed_tag_vlan(ifp->ifindex, vlan);
#else
				ret = OK;
#endif
				if(ret == OK)
				{
					nsm_vlan->trunk_allowed[vlan].vlan = 0;
					nsm_vlan_index_max(nsm_vlan);
					nsm_vlan_database_del_port( vlan, ifp->ifindex, NSM_VLAN_TAG);
				}
			}
		}
		else
		{
#ifdef ZPL_HAL_MODULE
			ret = hal_port_del_allowed_tag_vlan(ifp->ifindex, vlan);
#else
			ret = OK;
#endif
			if(ret == OK)
			{
				nsm_vlan->allow_all = zpl_false;
			}
		}
	}
	IF_NSM_VLAN_DATA_UNLOCK(nsm_vlan);
	return ret;
}


int nsm_interface_trunk_add_allowed_batch_vlan_api(struct interface *ifp, vlan_t minvlan, vlan_t maxvlan)
{
	int ret = ERROR;
	zpl_uint32 i = 0;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm_intf_module_data(ifp, NSM_INTF_VLAN);
	if(!if_is_ethernet(ifp))
		return ERROR;
	zassert(nsm_vlan);
	IF_NSM_VLAN_DATA_LOCK(nsm_vlan);

	for(i = minvlan; i <= maxvlan; i++)
	{
		if(i && nsm_vlan_database_lookup_api(i) == NULL)
		{
			IF_NSM_VLAN_DATA_UNLOCK(nsm_vlan);
			return ret;
		}
	}
#ifdef ZPL_HAL_MODULE
	ret = hal_port_add_allowed_tag_batch_vlan(ifp->ifindex, minvlan, maxvlan);
#else
	ret = OK;
#endif	
	if(ret != OK)
	{
		IF_NSM_VLAN_DATA_UNLOCK(nsm_vlan);
		return ret;
	}
	for(i = minvlan; i <= maxvlan; i++)
	{
		if(nsm_vlan->trunk_allowed[i].vlan == 0)
		{
			if(ret == OK)
			{
				nsm_vlan->trunk_allowed[i].vlan = i;
				nsm_vlan->trunk_allowed[i].minvlan = minvlan;
				nsm_vlan->trunk_allowed[i].maxvlan = maxvlan;
				nsm_vlan_database_add_port( i, ifp->ifindex, NSM_VLAN_TAG);
				nsm_vlan_database_del_port( i, ifp->ifindex, NSM_VLAN_UNTAG);
			}
		}
	}
	nsm_vlan_index_max(nsm_vlan);
	IF_NSM_VLAN_DATA_UNLOCK(nsm_vlan);
	return ret;
}

int nsm_interface_trunk_del_allowed_batch_vlan_api(struct interface *ifp, vlan_t minvlan, vlan_t maxvlan)
{
	int ret = OK;
	zpl_uint32 i = 0;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm_intf_module_data(ifp, NSM_INTF_VLAN);
	if(!if_is_ethernet(ifp))
		return ERROR;
	zassert(nsm_vlan);
	IF_NSM_VLAN_DATA_LOCK(nsm_vlan);
	for(i = minvlan; i <= maxvlan; i++)
	{
		if(i && nsm_vlan_database_lookup_api(i) == NULL)
		{
			IF_NSM_VLAN_DATA_UNLOCK(nsm_vlan);
			return ret;
		}
	}
#ifdef ZPL_HAL_MODULE
	ret = hal_port_del_allowed_tag_batch_vlan(ifp->ifindex, minvlan, maxvlan);
#else
	ret = OK;
#endif
	if(ret != OK)
	{
		IF_NSM_VLAN_DATA_UNLOCK(nsm_vlan);
		return ret;
	}
	for(i = minvlan; i <= maxvlan; i++)
	{
		if(nsm_vlan->trunk_allowed[i].vlan)
		{
			if(ret == OK)
			{
				//nsm_vlan->allowed_max = MIN(nsm_vlan->allowed_max, nsm_vlan->trunk_allowed[i].maxvlan);
				nsm_vlan->trunk_allowed[i].vlan = 0;
				nsm_vlan->trunk_allowed[i].minvlan = 0;
				nsm_vlan->trunk_allowed[i].maxvlan = 0;
				//nsm_vlan_database_add_port( i, ifp->ifindex, NSM_VLAN_TAG);
				nsm_vlan_database_del_port( i, ifp->ifindex, NSM_VLAN_TAG);
			}
			else
				break;
		}
	}
	nsm_vlan_index_max(nsm_vlan);
	IF_NSM_VLAN_DATA_UNLOCK(nsm_vlan);
	return ret;
}


int nsm_interface_trunk_allowed_vlan_list_api(int add, struct interface *ifp, const char *str)
{
	zpl_char tmp[32];
	int n = 0, i = 0, ret = 0;
	vlan_t ibase = 0, iend = 0;
	vlan_t value = 0;
	if(!if_is_ethernet(ifp))
		return ERROR;
	n = strspn(str, "0123456789,-");
	if(n != strlen(str))
	{
		fprintf(stderr,"ERROR:input:%s  n = %d\r\n", str, n);
		return ERROR;
	}
	//2,4,6,7,8-12,14-33,36,78
	//2,4,6,7,8-12,14,15,56,36-78
	//8-12,14,15,56,36-78
	memset(tmp, 0, sizeof(tmp));
	n = 0;
	while(n < strlen(str))
	{
		if(str[n] == ',')
		{
			if(ibase)
			{
				iend = atoi(tmp);
				if(add)
					ret |= nsm_interface_trunk_add_allowed_batch_vlan_api(ifp, ibase, iend);
				else
					ret |= nsm_interface_trunk_del_allowed_batch_vlan_api(ifp, ibase, iend);
				ibase = 0;
				iend = 0;
				if(ret == ERROR)
					break;
			}
			else
			{
				value = atoi(tmp);
				if(add)
					ret |= nsm_interface_trunk_add_allowed_vlan_api(ifp, value);
				else
					ret |= nsm_interface_trunk_del_allowed_vlan_api(ifp, value);
				value = 0;
				if(ret == ERROR)
					break;
			}
			memset(tmp, 0, sizeof(tmp));
			i = 0;
		}
		else if(str[n] == '-')
		{
			ibase = atoi(tmp);
			memset(tmp, 0, sizeof(tmp));
			i = 0;
		}
		else
			tmp[i++] = str[n];
		n++;
	}
	if(i)
	{
		if(ibase)
		{
			iend = atoi(tmp);
			if(add)
				ret |= nsm_interface_trunk_add_allowed_batch_vlan_api(ifp, ibase, iend);
			else
				ret |= nsm_interface_trunk_del_allowed_batch_vlan_api(ifp, ibase, iend);
		}
		else
		{
			value = atoi(tmp);
			if(add)
				ret |= nsm_interface_trunk_add_allowed_vlan_api(ifp, value);
			else
				ret |= nsm_interface_trunk_del_allowed_vlan_api(ifp, value);
		}
	}
	return ret;
}

int nsm_interface_trunk_allowed_vlan_list_lookup_api(struct interface *ifp, vlan_t *vlanlist, zpl_uint32 num)
{
	zpl_uint32 n = 0;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm_intf_module_data(ifp, NSM_INTF_VLAN);
	if(!if_is_ethernet(ifp))
		return 0;
	zassert(nsm_vlan);	
	IF_NSM_VLAN_DATA_LOCK(nsm_vlan);

	for(n = 0; n < num; n++)
	{
		if(vlanlist[n])
		{
			if(nsm_vlan->trunk_allowed[vlanlist[n]].vlan != vlanlist[n])
			{
				IF_NSM_VLAN_DATA_UNLOCK(nsm_vlan);
				return 0;	
			}
		}
	}
	IF_NSM_VLAN_DATA_UNLOCK(nsm_vlan);
	return 1;	
}

int nsm_vlan_interface_create_api(struct interface *ifp)
{
	nsm_vlan_t *nsm_vlan = nsm_intf_module_data(ifp, NSM_INTF_VLAN);
	NSM_ENTER_FUNC();
	if(if_is_ethernet(ifp) || if_is_lag(ifp)/*&& !IF_IS_SUBIF_GET(ifp->ifindex)*/)
	{
		NSM_ENTER_FUNC();
		if(!nsm_vlan)
		{
			nsm_vlan = XMALLOC(MTYPE_VLAN, sizeof(nsm_vlan_t));
			zassert(nsm_vlan);
			os_memset(nsm_vlan, 0, sizeof(nsm_vlan_t));
			if (nsm_vlan->mutex == NULL)
				nsm_vlan->mutex = os_mutex_name_init("if_vlan_mutex");
			nsm_intf_module_data_set(ifp, NSM_INTF_VLAN, nsm_vlan);
		}
		if(if_is_ethernet(ifp) && nsm_vlan_database_lookup_api(1))
		{
			nsm_interface_access_vlan_set_api(ifp, 1);
		}
	}
	return OK;
}


int nsm_vlan_interface_del_api(struct interface *ifp)
{
	nsm_vlan_t *nsm_vlan = nsm_intf_module_data(ifp, NSM_INTF_VLAN);
	NSM_ENTER_FUNC();
	if(if_is_ethernet(ifp) /*&& !IF_IS_SUBIF_GET(ifp->ifindex)*/)
	{
		if(if_is_ethernet(ifp) && nsm_vlan_database_lookup_api(1))
		{
			nsm_interface_access_vlan_unset_api(ifp, 1);
		}
		if(nsm_vlan)
		{
			if(nsm_vlan->mutex)
			{
				os_mutex_exit(nsm_vlan->mutex);
				nsm_vlan->mutex = NULL;
			}
			XFREE(MTYPE_VLAN, nsm_vlan);
		}
		nsm_vlan = NULL;
		nsm_intf_module_data_set(ifp, NSM_INTF_VLAN, NULL);
	}
	return OK;
}

static int nsm_vlan_mode_change_vlan_remove(struct interface *ifp, nsm_vlan_t *nsm_vlan, int type)
{
	int ret = 0;
	if(type & 1)
	{
		if(nsm_vlan->allow_all)
		{
			nsm_vlan->allow_all = zpl_false;
			return 1;
		}	
		else
		{
			memset(nsm_vlan->trunk_allowed, 0, sizeof(nsm_vlan->trunk_allowed));
			nsm_vlan_database_clear_port(ifp->ifindex, NSM_VLAN_TAG);
		}
	}
	if((type & 2) && nsm_vlan->access != 1)
	{
		if(ret == OK)
		{
			nsm_vlan_database_clear_port(ifp->ifindex, NSM_VLAN_UNTAG);//把该接口从其他vlan的untag列表删除
			nsm_vlan->access = 1;
		}
	}
	return 0;
}

static int nsm_vlan_mode_callback(struct interface *ifp, if_mode_t oldmode, if_mode_t newmode)
{
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm_intf_module_data(ifp, NSM_INTF_VLAN);
	zassert(nsm_vlan);

	if (IF_MODE_ACCESS_L2 == newmode)
	{
		/* 把 trunk 的 vlan 全部移除 */
		nsm_vlan_mode_change_vlan_remove(ifp, nsm_vlan, 1);
	}
	else if (IF_MODE_TRUNK_L2 == newmode)
	{
		/* 把 access 的 vlan 全部移除 */
		nsm_vlan_mode_change_vlan_remove(ifp, nsm_vlan, 2);
	}
	else if (IF_MODE_L3 == newmode)
	{
		/* 把 vlan 全部移除 */
		nsm_vlan_mode_change_vlan_remove(ifp, nsm_vlan, 3);
	}
	else if (IF_MODE_BRIGDE == newmode)
	{
	}
	return OK;
}

int nsm_interface_qinq_vlan_set_api(struct interface *ifp, vlan_t vlan)
{
	int ret = ERROR;
	zassert(ifp);
	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm_intf_module_data(ifp, NSM_INTF_VLAN);
	if(!if_is_ethernet(ifp))
		return ERROR;	
	zassert(nsm_vlan);
	IF_NSM_VLAN_DATA_LOCK(nsm_vlan);
	if(vlan)
	{
		if(nsm_vlan_database_lookup_api(vlan))
		{
			if(nsm_vlan->native != vlan)
			{
#ifdef ZPL_HAL_MODULE
				ret = hal_port_add_native_vlan(ifp->ifindex, vlan);
#else
				ret = OK;
#endif
				if(ret == OK)
				{
					nsm_vlan->native = vlan;
					//nsm_vlan_database_untag_port_update(ifp->ifindex);//把该接口从其他vlan的untag列表删除
					nsm_vlan_database_add_port( vlan, ifp->ifindex, NSM_VLAN_TAG);
					nsm_vlan_database_del_port( vlan, ifp->ifindex, NSM_VLAN_UNTAG);
				}
			}
		}
		else
		{
			zpl_errno_set(IPSTACK_ERRNO_EEXIST);
			ret = ERROR;
		}
	}
	else
	{
#ifdef ZPL_HAL_MODULE
		ret = hal_port_del_native_vlan(ifp->ifindex, nsm_vlan->native);
#else
		ret = OK;
#endif
		if(ret == OK)
		{
			nsm_vlan_database_del_port( nsm_vlan->native, ifp->ifindex, NSM_VLAN_TAG);
			nsm_vlan->native = vlan;
		}
	}
	IF_NSM_VLAN_DATA_UNLOCK(nsm_vlan);
	return ret;
}

#ifdef ZPL_SHELL_MODULE
int nsm_vlan_interface_write_config(struct vty *vty, struct interface *ifp)
{
	zpl_uint32 i = 0;
	int count = 0;
	zpl_char tmp[128];
	zpl_char tmpcli_str[256];
	nsm_vlan_t *nsm_vlan = NULL;
	//NSM_ENTER_FUNC();
	if(if_is_ethernet(ifp)/* && !IF_IS_SUBIF_GET(ifp->ifindex)*/)
	{
		memset(tmpcli_str, 0, sizeof(tmpcli_str));

		nsm_vlan = (nsm_vlan_t *)nsm_intf_module_data(ifp, NSM_INTF_VLAN);
		if(!nsm_vlan)
			return OK;
		IF_NSM_VLAN_DATA_LOCK(nsm_vlan);
		if(ifp->if_mode == IF_MODE_L3)
			vty_out(vty, " no switchport%s", VTY_NEWLINE);
		else if(ifp->if_mode == IF_MODE_DOT1Q_TUNNEL)
		{
			vty_out(vty, " switchport%s", VTY_NEWLINE);
			vty_out(vty, " switchport dot1q-tunnel%s", VTY_NEWLINE);
			if(nsm_vlan->native)
				vty_out(vty, " switchport dot1q-tunnel vlan  %d%s", nsm_vlan->native, VTY_NEWLINE);
		}
		else if(ifp->if_mode == IF_MODE_ACCESS_L2)
		{
			vty_out(vty, " switchport%s", VTY_NEWLINE);	
			//vty_out(vty, " switchport mode access%s", VTY_NEWLINE);
			if(nsm_vlan->access != 0 && nsm_vlan->access != 1)
				vty_out(vty, " switchport access vlan %d%s", nsm_vlan->access, VTY_NEWLINE);
		}
		else if(ifp->if_mode == IF_MODE_TRUNK_L2)
		{
			vty_out(vty, " switchport%s", VTY_NEWLINE);	
			vty_out(vty, " switchport mode trunk%s", VTY_NEWLINE);
			if(nsm_vlan->native)
				vty_out(vty, " switchport trunk native vlan %d%s", nsm_vlan->native, VTY_NEWLINE);
			if(nsm_vlan->allow_all)
				vty_out(vty, " switchport trunk allowed vlan all%s", VTY_NEWLINE);
			else
			{
				int max_count = nsm_vlan->allowed_max + 1;
				for(i = 0; i < max_count; i++)
				{
					memset(tmp, 0, sizeof(tmp));
					if(nsm_vlan->trunk_allowed[i].minvlan && nsm_vlan->trunk_allowed[i].maxvlan)
					{
						if(nsm_vlan->trunk_allowed[i].vlan == nsm_vlan->trunk_allowed[i].minvlan)
						{
							sprintf(tmp, "%d-%d", nsm_vlan->trunk_allowed[i].vlan, nsm_vlan->trunk_allowed[i].maxvlan);
							if(count)
							{
								strcat(tmpcli_str, ",");
							}
							strcat(tmpcli_str, tmp);
							count = 1;
						}
					}
					else
					{
						if(nsm_vlan->trunk_allowed[i].vlan)
						{
							sprintf(tmp, "%d", nsm_vlan->trunk_allowed[i].vlan);
							if(count)
							{
								strcat(tmpcli_str, ",");
							}
							strcat(tmpcli_str, tmp);
							count = 1;
						}
					}
				}//end for
				if(count)
					vty_out(vty, " switchport trunk allowed add vlan %s%s", tmpcli_str, VTY_NEWLINE);
			}
		}
		IF_NSM_VLAN_DATA_UNLOCK(nsm_vlan);
	}
	return OK;
}
#endif
