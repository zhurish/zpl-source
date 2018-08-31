/*
 * nsm_trunk.c
 *
 *  Created on: May 7, 2018
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

#include "nsm_trunk.h"
#include "hal_trunk.h"

static Gl2trunk_t gtrunk;



int nsm_trunk_init()
{
	int i = 0;
	os_memset(&gtrunk, 0, sizeof(Gl2trunk_t));
	for(i = 0; i < NSM_TRUNK_ID_MAX; i++)
	{
		gtrunk.group[i].trunkList = malloc(sizeof(LIST));
		gtrunk.group[i].global = &gtrunk;
		gtrunk.lacp_system_priority = LACP_SYSTEM_PRIORITY_DEFAULT;
		gtrunk.load_balance = LOAD_BALANCE_DEFAULT;
		lstInit(gtrunk.group[i].trunkList);
	}
	gtrunk.mutex = os_mutex_init();
	return OK;
}

/*static int l2trunk_cleanup(int all)
{
	l2trunk_t *pstNode = NULL;
	NODE index;
	for(pstNode = (l2trunk_t *)lstFirst(gtrunk.trunkList);
			pstNode != NULL;  pstNode = (l2trunk_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode)
		{
			if(all)
			{
				lstDelete(gtrunk.trunkList, (NODE*)pstNode);
				XFREE(MTYPE_TRUNK, pstNode);
			}
			else if(pstNode->vlan != 1)
			{
				lstDelete(gtrunk.trunkList, (NODE*)pstNode);
				XFREE(MTYPE_TRUNK, pstNode);
			}
		}
	}
	return OK;
}*/

int nsm_trunk_exit()
{
/*	int i = 0;
	for(i = 0; i < NSM_TRUNK_ID_MAX; i++)
	{
		gtrunk.group[i].trunkList = malloc(sizeof(LIST));
		lstInit(gtrunk.group[i].trunkList);
	}*/
/*	if(lstCount(gtrunk.trunkList))
		l2trunk_cleanup(1);*/

	if(gtrunk.mutex)
		os_mutex_exit(gtrunk.mutex);
	return OK;
}


int nsm_trunk_enable(void)
{
	int ret = 0;
	if(gtrunk.mutex)
		os_mutex_lock(gtrunk.mutex, OS_WAIT_FOREVER);
	ret = hal_trunk_enable(TRUE);
	if(ret == OK)
		gtrunk.enable = TRUE;
	if(gtrunk.mutex)
		os_mutex_unlock(gtrunk.mutex);
	return ret;
}

BOOL nsm_trunk_is_enable(void)
{
	return gtrunk.enable;
}


//
static l2trunk_group_t * l2trunk_group_lookup_node(u_int trunkid)
{
	int i = 0;
	for(i = 0; i < NSM_TRUNK_ID_MAX; i++)
	{
		if(gtrunk.group[i].trunkId == trunkid)
		return &(gtrunk.group[i]);
	}
	return NULL;
}


/*
static int l2trunk_add_sort_node(l2trunk_t *value)
{
	int i = 1;
	l2trunk_t *node = l2trunk_lookup_node(value->trunkId);
	while(!node)
		node = l2trunk_lookup_node(value->trunkId);
	if(node)
	{
		lstInsert (gtrunk.trunkList, (NODE*)node, (NODE*)value);
	}
	else
		lstAdd(gtrunk.trunkList, (NODE *)node);
	return OK;
}
*/

static l2trunk_t * l2trunk_lookup_node(void *pList, ifindex_t ifindex)
{
	l2trunk_t *pstNode = NULL;
	NODE index;
	for(pstNode = (l2trunk_t *)lstFirst(pList);
			pstNode != NULL;  pstNode = (l2trunk_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode->ifindex == ifindex)
		{
			return pstNode;
		}
	}
	return NULL;
}

static int l2trunk_add_node(l2trunk_group_t *group, l2trunk_t *value)
{
	//l2trunk_group_t *group = l2trunk_group_lookup_node(value->trunkId);
	if(!group)
		return ERROR;
	l2trunk_t *node = XMALLOC(MTYPE_TRUNK, sizeof(l2trunk_t));
	if(node)
	{
		memset(node, 0, sizeof(l2trunk_t));
		memcpy(node, value, sizeof(l2trunk_t));
		node->group = group;
		node->lacp_port_priority = LACP_PORT_PRIORITY_DEFAULT;
		node->lacp_timeout = LACP_TIMEOUT_DEFAULT;
		lstAdd(group->trunkList, (NODE *)node);
		return OK;
	}
	return ERROR;
}

static int l2trunk_del_node(l2trunk_group_t *group, l2trunk_t *value)
{
	//l2trunk_group_t *group = l2trunk_group_lookup_node(value->trunkId);
	if(!group)
		return ERROR;
	if(value)
	{
		lstDelete(group->trunkList, (NODE *)value);
		value->group = NULL;
		XFREE(MTYPE_TRUNK, value);
		return OK;
	}
	return ERROR;
}



static l2trunk_t * l2trunk_lookup_port(ifindex_t ifindex)
{
	int i = 0;
	l2trunk_t *pstNode = NULL;
	for(i = 0; i < NSM_TRUNK_ID_MAX; i++)
	{
		pstNode = l2trunk_lookup_node(gtrunk.group[i].trunkList, ifindex);
		if(pstNode)
			return pstNode;
	}
	return NULL;
}


static int l2trunk_add_port(l2trunk_group_t *group, l2trunk_t *value)
{
	int i = 0;
	if(hal_trunk_interface_enable(value->ifindex, group->trunkId) == OK)
	{
/*		l2trunk_t value;
		value.ifindex = ifindex;
		value.trunkId = group->trunkId;*/
		return l2trunk_add_node(group, value);
	}
	else
		return ERROR;
	return ERROR;
}

static int l2trunk_del_port(l2trunk_group_t *group, l2trunk_t *value)
{
	if(hal_trunk_interface_disable(value->ifindex, group->trunkId) == OK)
	{
/*		l2trunk_t value;
		value.ifindex = ifindex;
		value.trunkId = group->trunkId;*/
		return l2trunk_del_node(group, value);
	}
	else
		return ERROR;
}

BOOL l2trunk_lookup_api(u_int trunkid)
{
	if(l2trunk_group_lookup_node(trunkid))
		return TRUE;
	return FALSE;
}

int l2trunk_lookup_interface_count_api(u_int trunkid)
{
	l2trunk_group_t *group = l2trunk_group_lookup_node(trunkid);
	if(group)
		return lstCount(group->trunkList);
	return -1;
}

int nsm_trunk_create_api(u_int trunkid, trunk_type_t type)
{
	int ret = ERROR;
	l2trunk_group_t *pstNode = NULL;
	if(gtrunk.mutex)
		os_mutex_lock(gtrunk.mutex, OS_WAIT_FOREVER);
	pstNode = l2trunk_group_lookup_node(trunkid);
	if(!pstNode)
	{
		int i = 0;
		for(i = 0; i < NSM_TRUNK_ID_MAX; i++)
		{
			if(gtrunk.group[i].trunkId == 0)
			{
				gtrunk.group[i].trunkId = trunkid;
				gtrunk.group[i].type = type;
				//gtrunk.group[i].global = &gtrunk;
				//gtrunk.group[i].lacp_system_priority = gtrunk.lacp_system_priority;
				//gtrunk.group[i].load_balance = gtrunk.load_balance;
				ret = OK;
			}
		}
	}
	if(gtrunk.mutex)
		os_mutex_unlock(gtrunk.mutex);
	return ret;
}


int nsm_trunk_destroy_api(u_int trunkid)
{
	int ret = ERROR;
	l2trunk_group_t *pstNode = NULL;
	if(gtrunk.mutex)
		os_mutex_lock(gtrunk.mutex, OS_WAIT_FOREVER);
	pstNode = l2trunk_group_lookup_node(trunkid);
	if(pstNode)
	{
		pstNode->trunkId = 0;
		pstNode->type = 0;
		//pstNode->global = NULL;
		ret = OK;
	}
	if(gtrunk.mutex)
		os_mutex_unlock(gtrunk.mutex);
	return ret;
}


BOOL l2trunk_lookup_interface_api(ifindex_t ifindex)
{
	if(l2trunk_lookup_port(ifindex))
		return TRUE;
	return FALSE;
}

int nsm_trunk_get_ID_interface_api(ifindex_t ifindex, u_int *trunkId)
{
	l2trunk_t *pstNode = l2trunk_lookup_port(ifindex);
	if(pstNode)
	{
		if(trunkId)
			*trunkId = pstNode->trunkId;
		return OK;
	}
	return -1;
}

int nsm_trunk_add_interface_api(u_int trunkid, trunk_type_t type, trunk_mode_t mode, struct interface *ifp)
{
	int ret = ERROR;
	l2trunk_t *value = NULL;
	l2trunk_group_t *group = NULL;
	if(gtrunk.mutex)
		os_mutex_lock(gtrunk.mutex, OS_WAIT_FOREVER);
	value = l2trunk_lookup_port(ifp->ifindex);
	if(value)
	{
		if(gtrunk.mutex)
			os_mutex_unlock(gtrunk.mutex);
		return ERROR;
	}
	group = l2trunk_group_lookup_node(trunkid);
	if(group)
	{
		l2trunk_t trunk;
		os_memset(&trunk, 0, sizeof(l2trunk_t));
		trunk.ifindex = ifp->ifindex;
		trunk.trunkId = trunkid;
		trunk.type = type;
		trunk.mode = mode;
		ret = l2trunk_add_port(group, &trunk);
	}
	if(gtrunk.mutex)
		os_mutex_unlock(gtrunk.mutex);
	return ret;
}


int nsm_trunk_del_interface_api(u_int trunkid, struct interface *ifp)
{
	int i = 0;
	int ret = ERROR;
	l2trunk_t *value;
	if(gtrunk.mutex)
		os_mutex_lock(gtrunk.mutex, OS_WAIT_FOREVER);
	value = l2trunk_lookup_port(ifp->ifindex);
	if(value)
	{
		ret = l2trunk_del_port(l2trunk_group_lookup_node(value->trunkId), value);
	}
	if(gtrunk.mutex)
		os_mutex_unlock(gtrunk.mutex);
	return ret;
}


int nsm_trunk_load_balance_api(u_int trunkid, load_balance_t mode)
{
	int ret = ERROR;
	//l2trunk_group_t *value;
	if(gtrunk.mutex)
		os_mutex_lock(gtrunk.mutex, OS_WAIT_FOREVER);

/*	value = l2trunk_group_lookup_node(trunkid);
	if(value)
	{
		value->load_balance = mode;
		ret = OK;
	}*/
	if(hal_trunk_mode(mode) == OK)
	{
		gtrunk.load_balance = mode;
		if(mode == TRUNK_LOAD_BALANCE_NONE)
			gtrunk.load_balance = LOAD_BALANCE_DEFAULT;

		ret = OK;
	}
	else
		ret = ERROR;
	if(gtrunk.mutex)
		os_mutex_unlock(gtrunk.mutex);
	return ret;
}

int nsm_trunk_lacp_port_priority_api(ifindex_t ifindex, u_int pri)
{
	int i = 0;
	int ret = ERROR;
	l2trunk_t *value;
	if(gtrunk.mutex)
		os_mutex_lock(gtrunk.mutex, OS_WAIT_FOREVER);

	for(i = 0; i < NSM_TRUNK_ID_MAX; i++)
	{
		if(gtrunk.group[i].trunkId)
		{
			value = l2trunk_lookup_node(gtrunk.group[i].trunkList, ifindex);
			if(value)
			{
				value->lacp_port_priority = pri;
				if(pri == 0)
					value->lacp_port_priority = LACP_PORT_PRIORITY_DEFAULT;

				ret = OK;
				break;
			}
		}
	}

	if(gtrunk.mutex)
		os_mutex_unlock(gtrunk.mutex);
	return ret;
}

int nsm_trunk_lacp_timeout_api(ifindex_t ifindex, u_int timeout)
{
	int i = 0;
	int ret = ERROR;
	l2trunk_t *value;
	if(gtrunk.mutex)
		os_mutex_lock(gtrunk.mutex, OS_WAIT_FOREVER);

	for(i = 0; i < NSM_TRUNK_ID_MAX; i++)
	{
		if(gtrunk.group[i].trunkId)
		{
			value = l2trunk_lookup_node(gtrunk.group[i].trunkList, ifindex);
			if(value)
			{
				value->lacp_timeout = timeout;
				if(timeout == 0)
					value->lacp_timeout = LACP_TIMEOUT_DEFAULT;

				ret = OK;
				break;
			}
		}
	}
	if(gtrunk.mutex)
		os_mutex_unlock(gtrunk.mutex);
	return ret;
}

int nsm_trunk_lacp_system_priority_api(u_int trunkid, u_int pri)
{
	int ret = ERROR;
	//l2trunk_group_t *value;
	if(gtrunk.mutex)
		os_mutex_lock(gtrunk.mutex, OS_WAIT_FOREVER);
/*	value = l2trunk_group_lookup_node(trunkid);
	if(value)
	{
		value->lacp_system_priority = pri;
		ret = OK;
	}*/
	gtrunk.lacp_system_priority = pri;
	if(pri == 0)
		gtrunk.lacp_system_priority = LACP_SYSTEM_PRIORITY_DEFAULT;

	ret = OK;
	if(gtrunk.mutex)
		os_mutex_unlock(gtrunk.mutex);
	return ret;
}

int nsm_trunk_group_callback_api(l2trunk_group_cb cb, void *pVoid)
{
	int i = 0, ret = 0;
	if(gtrunk.mutex)
		os_mutex_lock(gtrunk.mutex, OS_WAIT_FOREVER);
	for(i = 0; i < NSM_TRUNK_ID_MAX; i++)
	{
		if(cb)
			ret = (cb)(&gtrunk.group[i], pVoid);
		if(ret == ERROR)
			break;
	}
	if(gtrunk.mutex)
		os_mutex_unlock(gtrunk.mutex);
	return OK;
}

int nsm_trunk_callback_api(l2trunk_cb cb, void *pVoid)
{
	int i = 0, ret = 0;
	l2trunk_t *pstNode = NULL;
	NODE index;
	if(gtrunk.mutex)
		os_mutex_lock(gtrunk.mutex, OS_WAIT_FOREVER);
	for(i = 0; i < NSM_TRUNK_ID_MAX; i++)
	{
		for(pstNode = (l2trunk_t *)lstFirst(gtrunk.group[i].trunkList);
				pstNode != NULL;  pstNode = (l2trunk_t *)lstNext((NODE*)&index))
		{
			index = pstNode->node;
			if(pstNode->ifindex)
			{
				if(cb)
					ret = (cb)(pstNode, pVoid);
				if(ret == ERROR)
					break;
			}
		}
	}
	if(gtrunk.mutex)
		os_mutex_unlock(gtrunk.mutex);
	return OK;
}
