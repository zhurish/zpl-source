/*
 * nsm_vlan.c
 *
 *  Created on: Jan 8, 2018
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

#include "nsm_vlan.h"
#include "hal_vlan.h"

static Gl2vlan_t gvlan;

static int nsm_vlan_client_init();
static l2vlan_t * l2vlan_lookup_node(vlan_t value);
#if 0
int vlan_string_explain(const char *str, vlan_t *value, int num, vlan_t *base, vlan_t *end)
{
	char tmp[32];
	int n = 0, i = 0, j = 0;
	int ibase = 0, iend = 0;
	char *nm = NULL;
	n = strspn(str, "0123456789,-");
	if(n != strlen(str))
	{
		fprintf(stderr,"ERROR:input:%s  n = %d\r\n", str, n);
		return ERROR;
	}
	nm = strstr(str, "-");
	if(nm)
	{
		nm++;
		nm = strstr(nm, "-");
		if(nm)
		{
			return ERROR;
		}
	}
	memset(tmp, 0, sizeof(tmp));
	n = 0;
	while(n < strlen(str))
	{
		if(str[n] == ',')
		{
			if(ibase)
			{
				iend = atoi(tmp);
			}
			else
				value[j++] = atoi(tmp);
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
	if(ibase && iend == 0)
		iend = atoi(tmp);
	if(base)
		*base = ibase;
	if(end)
		*end = iend;

	n = 0;
	for(i = 0; i < num; i++)
	{
		if(value[i])
		{
			n++;
		}
	}
	return n;
}
#endif

int nsm_vlan_init()
{
	gvlan.vlanList = malloc(sizeof(LIST));
	gvlan.mutex = os_mutex_init();
	lstInit(gvlan.vlanList);
	nsm_vlan_client_init();
	nsm_vlan_create_api(1);
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

int nsm_vlan_exit()
{
	if(lstCount(gvlan.vlanList))
		l2vlan_cleanup(1);

	if(gvlan.mutex)
		os_mutex_exit(gvlan.mutex);
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
	ret = hal_vlan_enable(TRUE);
	if(ret == OK)
		gvlan.enable = TRUE;
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	return ret;
}

BOOL nsm_vlan_is_enable(void)
{
	return gvlan.enable;
}


static int nsm_vlan_index_max(nsm_vlan_t *nsm_vlan)
{
	int i = 0;
	for(i = 0; i < VLAN_TABLE_MAX; i++)
	{
		if(nsm_vlan->trunk_allowed[i].maxvlan)
			nsm_vlan->allowed_max = MAX(nsm_vlan->allowed_max, nsm_vlan->trunk_allowed[i].maxvlan);
		else
			nsm_vlan->allowed_max = MAX(nsm_vlan->allowed_max, nsm_vlan->trunk_allowed[i].vlan);
	}
	return (nsm_vlan->allowed_max + 1);
}
/*static int vlan_value_sort(vlan_t *value, int num)
{
	vlan_t temp = 0;
	int i = 0, j = 0;
    for (i = 0; i < num; i++) // 10个数，10 - 1轮冒泡，每一轮都将当前最大的数推到最后
     {
           for (j = 0; j < num - i; j++) // 9 - i，意思是每当经过一轮冒泡后，就减少一次比较
           {
        	   if (value[j] > value[j+1])
			   {
					 temp = value[j];
					 value[j] = value[j+1];
					 value[j+1] = temp;
			   }
           }
     }
    return OK;
}*/

static int l2vlan_add_sort_node(l2vlan_t *value)
{
	int i = 1;
	l2vlan_t *node = l2vlan_lookup_node(value->vlan - 1);
	while(!node)
		node = l2vlan_lookup_node(value->vlan - (i++));
	if(node)
	{
		lstInsert (gvlan.vlanList, (NODE*)node, (NODE*)value);
	}
	else
		lstAdd(gvlan.vlanList, (NODE *)node);
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
			char name[64];
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

static int l2vlan_lookup_port(l2vlan_t *node, ifindex_t ifindex, vlan_mode_t mode)
{
	int i = 0;
	for(i = 0; i < PHY_PORT_MAX; i++)
	{
		if(mode == VLAN_TAG)
		{
			if(node->tagport[i] == ifindex)
				return OK;
		}
		else if(mode == VLAN_UNTAG)
		{
			if(node->untagport[i] == ifindex)
				return OK;
		}
	}
	return ERROR;
}


static int l2vlan_add_port(l2vlan_t *node, ifindex_t ifindex, vlan_mode_t mode)
{
	int i = 0;
	for(i = 0; i < PHY_PORT_MAX; i++)
	{
		if(mode == VLAN_TAG)
		{
			if(node->tagport[i] == 0)
			{
				node->tagport[i] = ifindex;
				return OK;
			}
		}
		else if(mode == VLAN_UNTAG)
		{
			if(node->untagport[i] == 0)
			{
				node->untagport[i] = ifindex;
				return OK;
			}
		}
	}
	return ERROR;
}

static int l2vlan_del_port(l2vlan_t *node, ifindex_t ifindex, vlan_mode_t mode)
{
	int i = 0;
	for(i = 0; i < PHY_PORT_MAX; i++)
	{
		if(mode == VLAN_TAG)
		{
			if(node->tagport[i] == ifindex)
			{
				node->tagport[i] = 0;
				return OK;
			}
		}
		else if(mode == VLAN_UNTAG)
		{
			if(node->untagport[i] == ifindex)
			{
				node->untagport[i] = 0;
				return OK;
			}
		}
	}
	return ERROR;
}


int nsm_vlan_create_api(vlan_t vlan)
{
	int ret = ERROR;
	l2vlan_t value;
	memset(&value, 0, sizeof(value));
	value.vlan = vlan;
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);

	ret = hal_vlan_create(vlan);
	if(ret == OK)
		ret = l2vlan_add_node(&value);
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
		ret = hal_vlan_destroy(vlan);
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
	int i = 0;
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
		ret = hal_vlan_create(i);
		if(ret == OK)
			ret = l2vlan_add_node(&value);
		if(ret != OK)
			break;
	}
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	return ret;
}

int nsm_vlan_batch_destroy_api(vlan_t minvlan, vlan_t maxvlan)
{
	int i = 0;
	int ret = ERROR;
	l2vlan_t *value;
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);
	for(i = minvlan; i <= maxvlan; i++)
	{
		value = l2vlan_lookup_node(i);
		if(value)
		{
			ret = hal_vlan_destroy(value->vlan);
			if(ret == OK)
			{
				lstDelete(gvlan.vlanList, (NODE*)value);
				if(value->vlan_name)
					XFREE(MTYPE_VLAN, value->vlan_name);
				XFREE(MTYPE_VLAN, value);
				ret = OK;
			}
		}
		else
			break;
	}

	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	return ret;
}


int nsm_vlan_list_create_api(const char *str)
{
	char tmp[32];
	int n = 0, i = 0, ret = 0;
	vlan_t ibase = 0, iend = 0;
	vlan_t value = 0;
	//char *nm = NULL;
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
				ret |= nsm_vlan_create_api(value);
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
			ret |= nsm_vlan_create_api(value);
		}
	}
	return ret;
}


int nsm_vlan_list_destroy_api(const char *str)
{
	char tmp[32];
	int n = 0, i = 0, ret = 0;
	vlan_t ibase = 0, iend = 0;
	vlan_t value = 0;
	//char *nm = NULL;
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


int nsm_interface_add_untag_vlan_api(vlan_t vlan, struct interface *ifp)
{
	int ret = ERROR;
	l2vlan_t *value;
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);

	value = l2vlan_lookup_node(vlan);
	if(value)
	{
		if(l2vlan_lookup_port(value,  ifp->ifindex, VLAN_UNTAG) == ERROR)
		{
			ret = hal_vlan_add_untag_port(ifp->ifindex, value->vlan);
			if(ret == OK)
				ret = l2vlan_add_port(value,  ifp->ifindex, VLAN_UNTAG);
		}
	}
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	return ret;
}


int nsm_interface_del_untag_vlan_api(vlan_t vlan, struct interface *ifp)
{
	int ret = ERROR;
	l2vlan_t *value;
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);

	value = l2vlan_lookup_node(vlan);
	if(value)
	{
		if(l2vlan_lookup_port(value,  ifp->ifindex, VLAN_UNTAG) == OK)
		{
			ret = hal_vlan_del_untag_port(ifp->ifindex, value->vlan);
			if(ret == OK)
				ret = l2vlan_del_port(value,  ifp->ifindex, VLAN_UNTAG);
		}
	}
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	return ret;
}

int nsm_interface_lookup_untag_vlan_api(vlan_t vlan, struct interface *ifp)
{
	int ret = ERROR;
	l2vlan_t *value;
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);

	value = l2vlan_lookup_node(vlan);
	if(value)
	{
		if(l2vlan_lookup_port(value,  ifp->ifindex, VLAN_UNTAG) == OK)
			ret = OK;
	}
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	return ret;
}

int nsm_interface_add_tag_vlan_api(vlan_t vlan, struct interface *ifp)
{
	int ret = ERROR;
	l2vlan_t *value;
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);

	value = l2vlan_lookup_node(vlan);
	if(value)
	{
		if(l2vlan_lookup_port(value,  ifp->ifindex, VLAN_TAG) != OK)
		{
			ret = hal_vlan_add_tag_port(ifp->ifindex, value->vlan);
			if(ret == OK)
				ret = l2vlan_add_port(value,  ifp->ifindex, VLAN_TAG);
		}
	}
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	return ret;
}

int nsm_interface_del_tag_vlan_api(vlan_t vlan, struct interface *ifp)
{
	int ret = ERROR;
	l2vlan_t *value;
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);

	value = l2vlan_lookup_node(vlan);
	if(value)
	{
		if(l2vlan_lookup_port(value,  ifp->ifindex, VLAN_TAG) == OK)
		{
			ret = hal_vlan_del_tag_port(ifp->ifindex, value->vlan);
			if(ret == OK)
				ret = l2vlan_del_port(value,  ifp->ifindex, VLAN_TAG);
		}
	}
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	return ret;
}



int nsm_interface_lookup_tag_vlan_api(vlan_t vlan, struct interface *ifp)
{
	int ret = ERROR;
	l2vlan_t *value;
	if(gvlan.mutex)
		os_mutex_lock(gvlan.mutex, OS_WAIT_FOREVER);

	value = l2vlan_lookup_node(vlan);
	if(value)
	{
		if(l2vlan_lookup_port(value,  ifp->ifindex, VLAN_TAG) == OK)
			ret = OK;
	}
	if(gvlan.mutex)
		os_mutex_unlock(gvlan.mutex);
	return ret;
}


int nsm_interface_native_vlan_set_api(struct interface *ifp, vlan_t vlan)
{
	int ret = ERROR;
	zassert(ifp);
	zassert(ifp->info[ZLOG_NSM]);
	struct nsm_interface *nsm = ifp->info[ZLOG_NSM];
	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm->nsm_client[NSM_VLAN];
	zassert(nsm_vlan);
	IF_DATA_LOCK();
	if(nsm_vlan_lookup_api(vlan))
	{
		if(nsm_vlan->native != vlan)
		{
			if(vlan)
			{
				ret = hal_port_add_native_vlan(ifp->ifindex, vlan);
				if(ret == OK)
					nsm_vlan->native = vlan;
			}
			else
			{
				ret = hal_port_del_native_vlan(ifp->ifindex, nsm_vlan->native);
				if(ret == OK)
					nsm_vlan->native = vlan;
			}
		}
	}
	IF_DATA_UNLOCK();
	return ret;
}

/*int nsm_interface_native_vlan_unset_api(struct interface *ifp, vlan_t vlan)
{
	int ret = ERROR;
	zassert(ifp);
	zassert(ifp->info[ZLOG_NSM]);
	struct nsm_interface *nsm = ifp->info[ZLOG_NSM];
	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm->nsm_client[NSM_VLAN];
	zassert(nsm_vlan);
	IF_DATA_LOCK();
	if(nsm_vlan_lookup_api(vlan))
	{
		if(nsm_vlan->native != vlan)
		{
			if(vlan)
			{
				ret = hal_port_add_native_vlan(ifp->ifindex, vlan);
				if(ret == OK)
					nsm_vlan->native = vlan;
			}
			else
			{
				ret = hal_port_del_native_vlan(ifp->ifindex, nsm_vlan->native);
				if(ret == OK)
					nsm_vlan->native = vlan;
			}
		}
	}
	IF_DATA_UNLOCK();
	return ret;
}*/

int nsm_interface_native_vlan_get_api(struct interface *ifp, vlan_t *vlan)
{
	int ret = OK;
	zassert(ifp);
	zassert(ifp->info[ZLOG_NSM]);
	struct nsm_interface *nsm = ifp->info[ZLOG_NSM];
	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm->nsm_client[NSM_VLAN];
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
	zassert(ifp->info[ZLOG_NSM]);
	struct nsm_interface *nsm = ifp->info[ZLOG_NSM];
	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm->nsm_client[NSM_VLAN];
	zassert(nsm_vlan);
	IF_DATA_LOCK();
	if(nsm_vlan_lookup_api(vlan))
	{
		nsm_interface_add_untag_vlan_api(vlan,  ifp);
		nsm_vlan->access = vlan;
	}
	IF_DATA_UNLOCK();
	return ret;
}

int nsm_interface_access_vlan_unset_api(struct interface *ifp, vlan_t vlan)
{
	int ret = ERROR;
	zassert(ifp);
	zassert(ifp->info[ZLOG_NSM]);
	struct nsm_interface *nsm = ifp->info[ZLOG_NSM];
	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm->nsm_client[NSM_VLAN];
	zassert(nsm_vlan);
	IF_DATA_LOCK();
	if(nsm_vlan_lookup_api(vlan))
	{
		nsm_interface_del_untag_vlan_api(vlan,  ifp);
		nsm_vlan->access = 0;
	}
	IF_DATA_UNLOCK();
	return ret;
}

int nsm_interface_access_vlan_get_api(struct interface *ifp, vlan_t *vlan)
{
	int ret = OK;
	zassert(ifp);
	zassert(ifp->info[ZLOG_NSM]);
	struct nsm_interface *nsm = ifp->info[ZLOG_NSM];
	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm->nsm_client[NSM_VLAN];
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



int nsm_interface_trunk_add_allowed_vlan_api(struct interface *ifp, vlan_t vlan)
{
	int ret = ERROR;
	int i =0;
	zassert(ifp);
	zassert(ifp->info[ZLOG_NSM]);
	struct nsm_interface *nsm = ifp->info[ZLOG_NSM];
	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm->nsm_client[NSM_VLAN];
	zassert(nsm_vlan);
	IF_DATA_LOCK();
	//if(nsm_vlan_lookup_api(vlan))
	{
		if(vlan)
		{
			if(nsm_vlan->trunk_allowed[vlan].vlan == 0)
			{
				ret = hal_port_add_allowed_tag_vlan(ifp->ifindex, vlan);
				if(ret == OK)
				{
					nsm_vlan->trunk_allowed[vlan].vlan = vlan;
					nsm_vlan_index_max(nsm_vlan);
				}
			}
		}
		else
		{
			if(ret == OK)
			{
				nsm_vlan->all = 1;
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
	zassert(ifp->info[ZLOG_NSM]);
	struct nsm_interface *nsm = ifp->info[ZLOG_NSM];
	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm->nsm_client[NSM_VLAN];
	zassert(nsm_vlan);
	IF_DATA_LOCK();
	//if(nsm_vlan_lookup_api(vlan))
	{
		if(vlan)
		{
			if(nsm_vlan->trunk_allowed[vlan].vlan)
			{
				ret = hal_port_del_allowed_tag_vlan(ifp->ifindex, vlan);
				if(ret == OK)
				{
					//nsm_interface_del_tag_vlan_api(vlan,  ifp);
					nsm_vlan->trunk_allowed[vlan].vlan = 0;
					nsm_vlan_index_max(nsm_vlan);
				}
			}
		}
		else
		{
			if(ret == OK)
			{
				nsm_vlan->all = 0;
			}
		}
	}
	IF_DATA_UNLOCK();
	return ret;
}


int nsm_interface_trunk_add_allowed_batch_vlan_api(struct interface *ifp, vlan_t minvlan, vlan_t maxvlan)
{
	int ret = OK;
	int i =0;
	zassert(ifp);
	zassert(ifp->info[ZLOG_NSM]);
	struct nsm_interface *nsm = ifp->info[ZLOG_NSM];
	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm->nsm_client[NSM_VLAN];
	zassert(nsm_vlan);
	IF_DATA_LOCK();
	for(i = minvlan; i <= maxvlan; i++)
	{
		if(nsm_vlan->trunk_allowed[i].vlan == 0)
		{
			ret |= hal_port_add_allowed_tag_vlan(ifp->ifindex, i);
			if(ret == OK)
			{
					nsm_vlan->trunk_allowed[i].vlan = i;
					nsm_vlan->trunk_allowed[i].minvlan = minvlan;
					nsm_vlan->trunk_allowed[i].maxvlan = maxvlan;
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
	int i = 0;
	zassert(ifp);
	zassert(ifp->info[ZLOG_NSM]);
	struct nsm_interface *nsm = ifp->info[ZLOG_NSM];
	nsm_vlan_t *nsm_vlan = (nsm_vlan_t *)nsm->nsm_client[NSM_VLAN];
	zassert(nsm_vlan);
	IF_DATA_LOCK();
	for(i = minvlan; i <= maxvlan; i++)
	{
		if(nsm_vlan->trunk_allowed[i].vlan)
		{
			ret |= hal_port_add_allowed_tag_vlan(ifp->ifindex, i);
			if(ret == OK)
			{
				//nsm_vlan->allowed_max = MIN(nsm_vlan->allowed_max, nsm_vlan->trunk_allowed[i].maxvlan);
				nsm_vlan->trunk_allowed[i].vlan = 0;
				nsm_vlan->trunk_allowed[i].minvlan = 0;
				nsm_vlan->trunk_allowed[i].maxvlan = 0;
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
	char tmp[32];
	int n = 0, i = 0, ret = 0;
	vlan_t ibase = 0, iend = 0;
	vlan_t value = 0;
	//char *nm = NULL;
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
					ret |= nsm_interface_trunk_add_allowed_vlan_api(ifp, value);
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
				ret |= nsm_interface_trunk_add_allowed_vlan_api(ifp, value);
		}
	}
	return ret;
}



static int nsm_vlan_add_interface(struct interface *ifp)
{
	struct nsm_interface *nsm = ifp->info[ZLOG_NSM];
	if(!nsm->nsm_client[NSM_VLAN])
		nsm->nsm_client[NSM_VLAN] = XMALLOC(MTYPE_VLAN, sizeof(nsm_vlan_t));
	zassert(nsm->nsm_client[NSM_VLAN]);
	os_memset(nsm->nsm_client[NSM_VLAN], 0, sizeof(nsm_vlan_t));
	nsm_interface_add_untag_vlan_api(1,  ifp);
	nsm_interface_add_tag_vlan_api(1,  ifp);
	return OK;
}


static int nsm_vlan_del_interface(struct interface *ifp)
{
	struct nsm_interface *nsm = ifp->info[ZLOG_NSM];
	nsm_interface_del_untag_vlan_api(1,  ifp);
	nsm_interface_del_tag_vlan_api(1,  ifp);
	if(nsm->nsm_client[NSM_VLAN])
		XFREE(MTYPE_VLAN, nsm->nsm_client[NSM_VLAN]);
	return OK;
}


static int nsm_vlan_interface_config(struct vty *vty, struct interface *ifp)
{
	int i = 0;
	int count = 0;
	char tmp[128];
	char tmpcli_str[256];
	struct nsm_interface *nsm_ifp = NULL;
	nsm_vlan_t *nsm_vlan = NULL;
	memset(tmpcli_str, 0, sizeof(tmpcli_str));
	nsm_ifp = (struct nsm_interface *)ifp->info[ZLOG_NSM];
	nsm_vlan = (nsm_vlan_t *)nsm_ifp->nsm_client[NSM_VLAN];
	if(!nsm_vlan)
		return OK;
	if(ifp->if_mode == IF_MODE_L3)
		vty_out(vty, " no switchport%s", VTY_NEWLINE);
	else if(ifp->if_mode == IF_MODE_ACCESS_L2)
	{
		//vty_out(vty, " switchport access mode%s", VTY_NEWLINE);
		if(nsm_vlan->access != 0)
			vty_out(vty, " switchport access vlan %d%s", nsm_vlan->access, VTY_NEWLINE);
	}
	else if(ifp->if_mode == IF_MODE_TRUNK_L2)
	{
		vty_out(vty, " switchport trunk mode%s", VTY_NEWLINE);
		if(nsm_vlan->native)
			vty_out(vty, " switchport trunk native vlan %d%s", nsm_vlan->native, VTY_NEWLINE);
		if(nsm_vlan->all)
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
	return OK;
}

static int nsm_vlan_client_init()
{
	struct nsm_client *nsm = nsm_client_new ();
	nsm->interface_add_cb = nsm_vlan_add_interface;
	nsm->interface_delete_cb = nsm_vlan_del_interface;
	nsm->interface_write_config_cb = nsm_vlan_interface_config;
	nsm_client_install (nsm, NSM_VLAN);
	return OK;
}
