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
static Gl2vlan_t gvlan;


static l2vlan_t * l2vlan_lookup_node(vlan_t value);

int nsm_vlan_list_split_api(const char *str, vlan_t *vlanlist)
{
	zpl_char tmp[32];
	int n = 0, i = 0, ret = 0, j = 0,vid = 0;
	vlan_t ibase = 0, iend = 0;
	vlan_t value = 0;
	//zpl_char *nm = NULL;
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
				for(vid = ibase; vid <= iend; vid++)
					vlanlist[j++] = vid;
				ibase = 0;
				iend = 0;
			}
			else
			{
				value = atoi(tmp);
				vlanlist[j++] = value;	
				value = 0;
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
			for(vid = ibase; vid <= iend; vid++)
				vlanlist[j++] = vid;
		}
		else
		{
			value = atoi(tmp);
			vlanlist[j++] = value;
		}
	}
	return j;
}



int nsm_vlan_list_lookup_api(vlan_t *vlanlist, zpl_uint32 num)
{
	zpl_uint32 n = 0;
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);
	for(n = 0; n < num; n++)
	{
		if(vlanlist[n])
		{
			if(!l2vlan_lookup_node(vlanlist[n]))
			{
				if(gvlan.mutex)
					os_mutex_unlock(gvlan.mutex);
				return 0;
			}
		}
	}
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	return 1;	
}

int nsm_vlan_default(void)
{
	nsm_vlan_enable();
	nsm_vlan_create_api(1, NULL);
	return OK;
}

int nsm_vlan_init(void)
{
	gvlan.vlanList = malloc(sizeof(LIST));
	gvlan.mutex = os_mutex_init();
	lstInit(gvlan.vlanList);
	nsm_interface_hook_add(NSM_INTF_VLAN, nsm_vlan_interface_create_api, nsm_vlan_interface_del_api);
	nsm_interface_write_hook_add(NSM_INTF_VLAN, nsm_vlan_interface_write_config);
	//nsm_vlan_client_init();
	//nsm_vlan_create_api(1, NULL);
	//l2vlan_add_untag_port(1, ifindex_t ifindex);
	return OK;
}

static int l2vlan_cleanup(int all)
{
	l2vlan_t *pstNode = NULL;
	NODE index;
	for(pstNode = (l2vlan_t *)lstFirst(gvlan.vlanList);
			pstNode != NULL;  pstNode = (l2vlan_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode)
		{
			if(all)
			{
				lstDelete(gvlan.vlanList, (NODE*)pstNode);
				if(pstNode->vlan_name)
					XFREE(MTYPE_VLAN, pstNode->vlan_name);
				XFREE(MTYPE_VLAN, pstNode);
			}
			else if(pstNode->vlan != 1)
			{
				lstDelete(gvlan.vlanList, (NODE*)pstNode);
				if(pstNode->vlan_name)
					XFREE(MTYPE_VLAN, pstNode->vlan_name);
				XFREE(MTYPE_VLAN, pstNode);
			}
		}
	}
	return OK;
}

int nsm_vlan_exit(void)
{
	if(lstCount(gvlan.vlanList))
	{
		l2vlan_cleanup(1);
		lstFree(gvlan.vlanList);
		free(gvlan.vlanList);
		gvlan.vlanList = NULL;
	}
	if(gvlan.mutex)
		os_mutex_exit(gvlan.mutex);
	//nsm_vlan_client_exit();
	return OK;
}

int nsm_vlan_cleanall(void)
{
	return l2vlan_cleanup(0);
}

int nsm_vlan_enable(void)
{
	int ret = 0;
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);
#ifdef ZPL_HAL_MODULE
	ret = hal_vlan_enable(zpl_true);
#endif
	if(ret == OK)
		gvlan.enable = zpl_true;
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	return ret;
}

zpl_bool nsm_vlan_is_enable(void)
{
	return gvlan.enable;
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


static int l2vlan_add_sort_node(l2vlan_t *value)
{
	zpl_uint32 i = 1;
	l2vlan_t *node = l2vlan_lookup_node(value->vlan - 1);
	while(!node)
		node = l2vlan_lookup_node(value->vlan - (i++));
	if(node)
	{
		lstInsert (gvlan.vlanList, (NODE*)node, (NODE*)value);
	}
	else
		lstAdd(gvlan.vlanList, (NODE *)value);
	return OK;
}

static int l2vlan_add_node(l2vlan_t *value)
{
	l2vlan_t *node = XMALLOC(MTYPE_VLAN, sizeof(l2vlan_t));
	if(node)
	{
		memset(node, 0, sizeof(l2vlan_t));
		memcpy(node, value, sizeof(l2vlan_t));
		if(value->vlan_name)
		{
			node->vlan_name = XSTRDUP(MTYPE_VLAN, value->vlan_name);
			node->name_hash = string_hash_make(node->vlan_name);
			XFREE(MTYPE_VLAN, value->vlan_name);
		}
/*		else
		{
			zpl_char name[64];
			memset(name, 0, sizeof(name));
			sprintf(name, "VLAN%04d", node->vlan);
			node->vlan_name = XSTRDUP(MTYPE_VLAN, name);
			node->name_hash = string_hash_make(node->vlan_name);
		}*/
		if(lstCount(gvlan.vlanList) == 0)
			lstAdd(gvlan.vlanList, (NODE *)node);
		else
			l2vlan_add_sort_node(node);
		return OK;
	}
	return ERROR;
}

static l2vlan_t * l2vlan_lookup_node(vlan_t value)
{
	l2vlan_t *pstNode = NULL;
	NODE index;
	for(pstNode = (l2vlan_t *)lstFirst(gvlan.vlanList);
			pstNode != NULL;  pstNode = (l2vlan_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode->vlan == value)
		{
			return pstNode;
		}
	}
	return NULL;
}

static l2vlan_t * l2vlan_lookup_node_by_name(const char *name)
{
	l2vlan_t *pstNode = NULL;
	NODE index;
	for(pstNode = (l2vlan_t *)lstFirst(gvlan.vlanList);
			pstNode != NULL;  pstNode = (l2vlan_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode->name_hash == string_hash_make(name))
		{
			return pstNode;
		}
	}
	return NULL;
}

static int _l2vlan_lookup_port(l2vlan_t *value, ifindex_t ifindex, vlan_mode_t mode)
{
	zpl_uint32 i = 0;
	for(i = 0; i < PHY_PORT_MAX; i++)
	{
		if(mode == VLAN_UNTAG)
		{
			if(value->untagport[i] == ifindex)
			{
				return 1;
			}
		}
		if(mode == VLAN_TAG)
		{
			if(value->tagport[i] == ifindex)
			{
				return 1;
			}
		}
	}
	return 0;
}


static int _l2vlan_del_port(l2vlan_t *value, ifindex_t ifindex, vlan_mode_t mode)
{
	zpl_uint32 i = 0;
	for(i = 0; i < PHY_PORT_MAX; i++)
	{
		if(mode == VLAN_UNTAG)
		{
			if(value->untagport[i] == ifindex)
			{
				value->untagport[i] = 0;
				return 1;
			}
		}
		else if(mode == VLAN_TAG)
		{
			if(value->tagport[i] == ifindex)
			{
				value->tagport[i] = 0;
				return 1;
			}
		}
	}
	return 0;
}

static int _l2vlan_untag_port_update(ifindex_t ifindex)
{
	l2vlan_t *pstNode = NULL;
	NODE index;
	for(pstNode = (l2vlan_t *)lstFirst(gvlan.vlanList);
			pstNode != NULL;  pstNode = (l2vlan_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode)
		{
			if(_l2vlan_lookup_port(pstNode,  ifindex,  VLAN_UNTAG))
			{
				_l2vlan_del_port(pstNode,  ifindex,  VLAN_UNTAG);
			}
		}
	}
	return 0;
}

static int _l2vlan_add_port(l2vlan_t *value, ifindex_t ifindex, vlan_mode_t mode)
{
	zpl_uint32 i = 0;
	for(i = 0; i < PHY_PORT_MAX; i++)
	{
		if(mode == VLAN_UNTAG)
		{
			if(value->untagport[i] == 0)
			{
				value->untagport[i] = ifindex;
				return 1;
			}
		}
		else if(mode == VLAN_TAG)
		{
			if(value->tagport[i] == 0)
			{
				value->tagport[i] = ifindex;
				return 1;
			}
		}
	}
	return 0;
}

static int nsm_l2vlan_add_port(vlan_t vlan, ifindex_t ifindex, vlan_mode_t mode)
{
	l2vlan_t *value = NULL;
	value = l2vlan_lookup_node(vlan);
	if(value)
	{
		_l2vlan_add_port(value,  ifindex,  mode);
	}
	return 0;
}
static int nsm_l2vlan_del_port(vlan_t vlan, ifindex_t ifindex, vlan_mode_t mode)
{
	l2vlan_t *value = NULL;
	value = l2vlan_lookup_node(vlan);
	if(value)
	{
		_l2vlan_del_port(value,  ifindex,  mode);
	}
	return 0;
}

int nsm_vlan_create_api(vlan_t vlan, const char *name)
{
	int ret = ERROR;
	l2vlan_t value;
	memset(&value, 0, sizeof(value));
	value.vlan = vlan;
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);

	#ifdef ZPL_HAL_MODULE
		ret = hal_vlan_create(vlan);
	#else
		ret = OK;
	#endif
		if(ret == OK)
		{
			if(name)
				value.vlan_name = XSTRDUP(MTYPE_VLAN, name);
			ret = l2vlan_add_node(&value);
		}

	else
		ret = OK;
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	return ret;
}


int nsm_vlan_destroy_api(vlan_t vlan)
{
	int ret = ERROR;
	l2vlan_t *value;
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);
	value = l2vlan_lookup_node(vlan);
	if(value)
	{
#ifdef ZPL_HAL_MODULE
		ret = hal_vlan_destroy(vlan);
#else
		ret = OK;
#endif
		if(ret == OK)
		{
			lstDelete(gvlan.vlanList, (NODE*)value);
			if(value->vlan_name)
				XFREE(MTYPE_VLAN, value->vlan_name);
			XFREE(MTYPE_VLAN, value);
			ret = OK;
		}
	}
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	return ret;
}

int nsm_vlan_batch_create_api(vlan_t minvlan, vlan_t maxvlan)
{
	zpl_uint32 i = 0;
	int ret = ERROR;
	l2vlan_t value;
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);
	for(i = minvlan; i <= maxvlan; i++)
	{
		memset(&value, 0, sizeof(value));
		value.vlan = i;
		//if(minvlan == i)
		{
			value.minvlan = minvlan;
			value.maxvlan = maxvlan;
		}
		if(!l2vlan_lookup_node(i))
		{
	#ifdef ZPL_HAL_MODULE
			ret = hal_vlan_create(i);
	#else
			ret = OK;
	#endif
			if(ret == OK)
				ret = l2vlan_add_node(&value);
			if(ret != OK)
				break;
		}
		else
		{
			ret = OK;
		}
	}
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	return ret;
}

int nsm_vlan_batch_destroy_api(vlan_t minvlan, vlan_t maxvlan)
{
	zpl_uint32 i = 0;
	int ret = ERROR;
	l2vlan_t *value;
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);
	for(i = minvlan; i <= maxvlan; i++)
	{
		value = l2vlan_lookup_node(i);
		if(value)
		{
#ifdef ZPL_HAL_MODULE
			ret = hal_vlan_destroy(value->vlan);
#else
			ret = OK;
#endif
			if(ret == OK)
			{
				lstDelete(gvlan.vlanList, (NODE*)value);
				if(value->vlan_name)
					XFREE(MTYPE_VLAN, value->vlan_name);
				XFREE(MTYPE_VLAN, value);
				ret = OK;
			}
		}
	}
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	return ret;
}


int nsm_vlan_list_create_api(const char *str)
{
	zpl_char tmp[32];
	int n = 0, i = 0, ret = 0;
	vlan_t ibase = 0, iend = 0;
	vlan_t value = 0;
	//zpl_char *nm = NULL;
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
				ret |= nsm_vlan_batch_create_api(ibase, iend);
				ibase = 0;
				iend = 0;
			}
			else
			{
				value = atoi(tmp);
				if(!nsm_vlan_lookup_api(value))
					ret |= nsm_vlan_create_api(value, NULL);	
				value = 0;
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
			ret |= nsm_vlan_batch_create_api(ibase, iend);
		}
		else
		{
			value = atoi(tmp);
			if(!nsm_vlan_lookup_api(value))
				ret |= nsm_vlan_create_api(value, NULL);
		}
	}
	return ret;
}


int nsm_vlan_list_destroy_api(const char *str)
{
	zpl_char tmp[32];
	int n = 0, i = 0, ret = 0;
	vlan_t ibase = 0, iend = 0;
	vlan_t value = 0;
	//zpl_char *nm = NULL;
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
				ret |= nsm_vlan_batch_destroy_api(ibase, iend);
				ibase = 0;
				iend = 0;
			}
			else
			{
				value = atoi(tmp);
				if(l2vlan_lookup_node(value))
					ret |= nsm_vlan_destroy_api(value);
				value = 0;
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
			ret |= nsm_vlan_batch_destroy_api(ibase, iend);
		}
		else
		{
			value = atoi(tmp);
			if(l2vlan_lookup_node(value))
				ret |= nsm_vlan_destroy_api(value);
		}
	}
	return ret;
}

int nsm_vlan_name_api(vlan_t vlan, const char *name)
{
	int ret = ERROR;
	l2vlan_t *value;
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);
	value = l2vlan_lookup_node(vlan);
	if(value)
	{
		if(value->vlan_name)
			XFREE(MTYPE_VLAN, value->vlan_name);
		if(name)
			value->vlan_name = XSTRDUP(MTYPE_VLAN, name);
		ret = OK;
	}
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	return ret;
}


void * nsm_vlan_lookup_api(vlan_t vlan)
{
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);
	l2vlan_t *value = l2vlan_lookup_node(vlan);
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	return value;
}

void * nsm_vlan_lookup_by_name_api(const char *name)
{
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);
	l2vlan_t *value = l2vlan_lookup_node_by_name(name);
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	return value;
}

int nsm_vlan_callback_api(l2vlan_cb cb, void *pVoid)
{
	l2vlan_t *pstNode = NULL;
	NODE index;
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);
	for(pstNode = (l2vlan_t *)lstFirst(gvlan.vlanList);
			pstNode != NULL;  pstNode = (l2vlan_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode)
		{
			if(cb)
			{
				(cb)(pstNode, pVoid);
			}
		}
	}
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	return OK;
}



int nsm_interface_native_vlan_set_api(struct interface *ifp, vlan_t vlan)
{
	int ret = ERROR;
	zassert(ifp);
	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm_intf_module_data(ifp, NSM_INTF_VLAN);
	zassert(nsm_vlan);
	IF_DATA_LOCK();
	if(vlan)
	{
		if(nsm_vlan_lookup_api(vlan))
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
					//_l2vlan_untag_port_update(ifp->ifindex);//把该接口从其他vlan的untag列表删除
					nsm_l2vlan_add_port( vlan, ifp->ifindex, VLAN_TAG);
					nsm_l2vlan_del_port( vlan, ifp->ifindex, VLAN_UNTAG);
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
			//nsm_l2vlan_add_port( vlan, ifp->ifindex, VLAN_TAG);
			nsm_l2vlan_del_port( nsm_vlan->native, ifp->ifindex, VLAN_TAG);
			nsm_vlan->native = vlan;
		}
	}
	IF_DATA_UNLOCK();
	return ret;
}


int nsm_interface_native_vlan_get_api(struct interface *ifp, vlan_t *vlan)
{
	int ret = OK;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm_intf_module_data(ifp, NSM_INTF_VLAN);
	zassert(nsm_vlan);
	IF_DATA_LOCK();
	if(nsm_vlan)
	{
		if(vlan)
			*vlan = nsm_vlan->native;
	}
	IF_DATA_UNLOCK();
	return ret;
}

int nsm_interface_access_vlan_set_api(struct interface *ifp, vlan_t vlan)
{
	int ret = ERROR;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm_intf_module_data(ifp, NSM_INTF_VLAN);
	zassert(nsm_vlan);
	IF_DATA_LOCK();
	if(nsm_vlan_lookup_api(vlan))
	{
#ifdef ZPL_HAL_MODULE
		ret = hal_port_add_access_vlan(ifp->ifindex, vlan);
#else
		ret = OK;
#endif
		if(ret == OK)
		{
			_l2vlan_untag_port_update(ifp->ifindex);//把该接口从其他vlan的untag列表删除
			nsm_l2vlan_add_port( vlan, ifp->ifindex, VLAN_UNTAG);//把该接口加入到vlan的untag接口列表
			nsm_l2vlan_del_port( vlan, ifp->ifindex, VLAN_TAG);//把该接口从vlan的tag列表删除
			nsm_vlan->access = vlan;
		}
	}
	IF_DATA_UNLOCK();
	return ret;
}

int nsm_interface_access_vlan_unset_api(struct interface *ifp, vlan_t vlan)
{
	int ret = ERROR;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm_intf_module_data(ifp, NSM_INTF_VLAN);
	zassert(nsm_vlan);
	IF_DATA_LOCK();
	if(nsm_vlan_lookup_api(vlan))
	{
#ifdef ZPL_HAL_MODULE
		ret = hal_port_del_access_vlan(ifp->ifindex, vlan);
#else
		ret = OK;
#endif
		if(ret == OK)
		{
			//nsm_l2vlan_add_port( nsm_vlan->access, ifp->ifindex, VLAN_UNTAG);
			nsm_l2vlan_del_port( nsm_vlan->access, ifp->ifindex, VLAN_UNTAG);//把该接口从vlan的tag列表删除
			nsm_vlan->access = 0;
		}
	}
	IF_DATA_UNLOCK();
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
	zassert(nsm_vlan);
	IF_DATA_LOCK();
	if(nsm_vlan)
	{
		if(vlan)
			*vlan = nsm_vlan->access;
	}
	IF_DATA_UNLOCK();
	return ret;
}


int nsm_interface_trunk_add_allowed_vlan_lookup_api(struct interface *ifp, vlan_t vlan)
{
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm_intf_module_data(ifp, NSM_INTF_VLAN);
	zassert(nsm_vlan);
	IF_DATA_LOCK();

	if(vlan)
	{
		if(nsm_vlan->trunk_allowed[vlan].vlan != vlan)
		{
			IF_DATA_UNLOCK();
			return 0;
		}
		else
		{
			IF_DATA_UNLOCK();
			return 1;
		}
	}
	IF_DATA_UNLOCK();
	return 0;
}
int nsm_interface_trunk_add_allowed_vlan_api(struct interface *ifp, vlan_t vlan)
{
	int ret = ERROR;
	//zpl_uint32 =0;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);

	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm_intf_module_data(ifp, NSM_INTF_VLAN);
	zassert(nsm_vlan);
	IF_DATA_LOCK();
	if(nsm_vlan_lookup_api(vlan))
	{
		if(vlan)
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
					nsm_l2vlan_add_port( vlan, ifp->ifindex, VLAN_TAG);
					nsm_l2vlan_del_port( vlan, ifp->ifindex, VLAN_UNTAG);
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
	IF_DATA_UNLOCK();
	return ret;
}

int nsm_interface_trunk_del_allowed_vlan_api(struct interface *ifp, vlan_t vlan)
{
	int ret = ERROR;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);

	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm_intf_module_data(ifp, NSM_INTF_VLAN);
	zassert(nsm_vlan);
	IF_DATA_LOCK();
	//if(nsm_vlan_lookup_api(vlan))
	{
		if(vlan)
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
					nsm_l2vlan_del_port( vlan, ifp->ifindex, VLAN_TAG);
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
	IF_DATA_UNLOCK();
	return ret;
}


int nsm_interface_trunk_add_allowed_batch_vlan_api(struct interface *ifp, vlan_t minvlan, vlan_t maxvlan)
{
	int ret = OK;
	zpl_uint32 i = 0;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);

	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm_intf_module_data(ifp, NSM_INTF_VLAN);
	zassert(nsm_vlan);
	IF_DATA_LOCK();
	for(i = minvlan; i <= maxvlan; i++)
	{
		if(nsm_vlan->trunk_allowed[i].vlan == 0)
		{
#ifdef ZPL_HAL_MODULE
			ret |= hal_port_add_allowed_tag_vlan(ifp->ifindex, i);
#else
			ret = OK;
#endif
			if(ret == OK)
			{
				nsm_vlan->trunk_allowed[i].vlan = i;
				nsm_vlan->trunk_allowed[i].minvlan = minvlan;
				nsm_vlan->trunk_allowed[i].maxvlan = maxvlan;
				nsm_l2vlan_add_port( i, ifp->ifindex, VLAN_TAG);
				nsm_l2vlan_del_port( i, ifp->ifindex, VLAN_UNTAG);
			}
			else
				break;
		}
	}
	nsm_vlan_index_max(nsm_vlan);
	IF_DATA_UNLOCK();
	return ret;
}

int nsm_interface_trunk_del_allowed_batch_vlan_api(struct interface *ifp, vlan_t minvlan, vlan_t maxvlan)
{
	int ret = OK;
	zpl_uint32 i = 0;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);

	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm_intf_module_data(ifp, NSM_INTF_VLAN);
	zassert(nsm_vlan);
	IF_DATA_LOCK();
	for(i = minvlan; i <= maxvlan; i++)
	{
		if(nsm_vlan->trunk_allowed[i].vlan)
		{
#ifdef ZPL_HAL_MODULE
			ret |= hal_port_add_allowed_tag_vlan(ifp->ifindex, i);
#else
			ret = OK;
#endif
			if(ret == OK)
			{
				//nsm_vlan->allowed_max = MIN(nsm_vlan->allowed_max, nsm_vlan->trunk_allowed[i].maxvlan);
				nsm_vlan->trunk_allowed[i].vlan = 0;
				nsm_vlan->trunk_allowed[i].minvlan = 0;
				nsm_vlan->trunk_allowed[i].maxvlan = 0;
				//nsm_l2vlan_add_port( i, ifp->ifindex, VLAN_TAG);
				nsm_l2vlan_del_port( i, ifp->ifindex, VLAN_TAG);
			}
			else
				break;
		}
	}
	nsm_vlan_index_max(nsm_vlan);
	IF_DATA_UNLOCK();
	return ret;
}


int nsm_interface_trunk_allowed_vlan_list_api(int add, struct interface *ifp, const char *str)
{
	zpl_char tmp[32];
	int n = 0, i = 0, ret = 0;
	vlan_t ibase = 0, iend = 0;
	vlan_t value = 0;
	//zpl_char *nm = NULL;
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
	zassert(nsm_vlan);
	IF_DATA_LOCK();

	for(n = 0; n < num; n++)
	{
		if(vlanlist[n])
		{
			if(nsm_vlan->trunk_allowed[vlanlist[n]].vlan != vlanlist[n])
			{
				IF_DATA_UNLOCK();
				return 0;	
			}
		}
	}
	IF_DATA_UNLOCK();
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
			nsm_intf_module_data_set(ifp, NSM_INTF_VLAN, nsm_vlan);
		}
		if(if_is_ethernet(ifp) && nsm_vlan_lookup_api(1))
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
		if(if_is_ethernet(ifp) && nsm_vlan_lookup_api(1))
		{
			nsm_interface_access_vlan_unset_api(ifp, 1);
		}
		if(nsm_vlan)
			XFREE(MTYPE_VLAN, nsm_vlan);
		nsm_vlan = NULL;
		nsm_intf_module_data_set(ifp, NSM_INTF_VLAN, NULL);
	}
	return OK;
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

		if(ifp->if_mode == IF_MODE_L3)
			vty_out(vty, " no switchport%s", VTY_NEWLINE);
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
	}
	return OK;
}
#endif
