/*
 * nsm_mac.c
 *
 *  Created on: Jan 9, 2018
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

#include "os_list.h"

#include "nsm_mac.h"


static Gl2mac_t gMac;


int nsm_mac_init(void)
{
	gMac.macList = malloc(sizeof(LIST));
	gMac.mutex = os_mutex_init();
	lstInit(gMac.macList);

	//nsm_mac_create_api(1);
	//l2mac_add_untag_port(1, ifindex_t ifindex);
	return OK;
}

static int l2mac_cleanup(mac_type_t type, BOOL all, vlan_t vlan, ifindex_t ifindex)
{
	l2mac_t *pstNode = NULL;
	NODE index;
	if(gMac.mutex)
		os_mutex_lock(gMac.mutex, OS_WAIT_FOREVER);
	for(pstNode = (l2mac_t *)lstFirst(gMac.macList);
			pstNode != NULL;  pstNode = (l2mac_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode && pstNode->ifindex == ifindex && pstNode->vlan == vlan && pstNode->type == type)
		{
			lstDelete(gMac.macList, (NODE*)pstNode);
			XFREE(MTYPE_VLAN, pstNode);
		}
		if(pstNode && pstNode->vlan == vlan && pstNode->type == type)
		{
			lstDelete(gMac.macList, (NODE*)pstNode);
			XFREE(MTYPE_VLAN, pstNode);
		}
		else if(pstNode && pstNode->ifindex == ifindex && pstNode->type == type)
		{
			lstDelete(gMac.macList, (NODE*)pstNode);
			XFREE(MTYPE_VLAN, pstNode);
		}
		else if(pstNode && all)
		{
			lstDelete(gMac.macList, (NODE*)pstNode);
			XFREE(MTYPE_VLAN, pstNode);
		}
	}
	hal_mac_clr(ifindex, vlan);
	if(gMac.mutex)
		os_mutex_unlock(gMac.mutex);
	return OK;
}


int nsm_mac_exit(void)
{
	if(lstCount(gMac.macList))
		l2mac_cleanup(0, TRUE, 0, 0);

	if(gMac.mutex)
		os_mutex_exit(gMac.mutex);
	return OK;
}


int nsm_mac_cleanall_api(void)
{
	return l2mac_cleanup(0, TRUE, 0, 0);
}

int nsm_mac_clean_ifindex_api(mac_type_t type, ifindex_t ifindex)
{
	return l2mac_cleanup(type, TRUE, 0, ifindex);
}

int nsm_mac_clean_vlan_api(mac_type_t type, vlan_t vlan)
{
	return l2mac_cleanup(type, TRUE, vlan, 0);
}



static int l2mac_add_node(l2mac_t *value)
{
	l2mac_t *node = XMALLOC(MTYPE_VLAN, sizeof(l2mac_t));
	if(node)
	{
		os_memset(node, 0, sizeof(l2mac_t));
		os_memcpy(node, value, sizeof(l2mac_t));

		if(NSM_MAC_IS_BROADCAST(node->mac[0]))
			node->class = MAC_BROADCAST;
		else if(NSM_MAC_IS_MULTICAST(node->mac[0]))
			node->class = MAC_MULTICAST;
		else
			node->class = MAC_UNICAST;
		lstAdd(gMac.macList, (NODE *)node);
		return OK;
	}
	return ERROR;
}

static l2mac_t * l2mac_lookup_node(mac_t *value, vlan_t vlan)
{
	l2mac_t *pstNode = NULL;
	NODE index;
	for(pstNode = (l2mac_t *)lstFirst(gMac.macList);
			pstNode != NULL;  pstNode = (l2mac_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(os_memcmp(pstNode->mac, value, NSM_MAC_MAX) == 0)
		{
			if(vlan && vlan == pstNode->vlan)
				return pstNode;
			else
				return pstNode;
		}
	}
	return NULL;
}


int nsm_mac_callback_api(l2mac_cb cb, void *pVoid)
{
	l2mac_t *pstNode = NULL;
	NODE index;
	if(gMac.mutex)
		os_mutex_lock(gMac.mutex, OS_WAIT_FOREVER);
	for(pstNode = (l2mac_t *)lstFirst(gMac.macList);
			pstNode != NULL;  pstNode = (l2mac_t *)lstNext((NODE*)&index))
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
	if(gMac.mutex)
		os_mutex_unlock(gMac.mutex);
	return OK;
}


int nsm_mac_lookup_api(mac_t *mac, vlan_t vlan)
{
	if(gMac.mutex)
		os_mutex_lock(gMac.mutex, OS_WAIT_FOREVER);
	l2mac_t *value = l2mac_lookup_node(mac, vlan);
	if(gMac.mutex)
		os_mutex_unlock(gMac.mutex);
	return value ? OK:ERROR;
}


int nsm_mac_add_api(l2mac_t *mac)
{
	int ret = ERROR;
	l2mac_t *value;
	if(gMac.mutex)
		os_mutex_lock(gMac.mutex, OS_WAIT_FOREVER);

	value = l2mac_lookup_node(mac->mac, mac->vlan);
	if(!value)
	{
		ret = hal_mac_add(mac->ifindex, mac->vlan, mac->mac, 0);
		if(ret == OK)
			ret = l2mac_add_node(mac);
		//TODO
	}
	if(gMac.mutex)
		os_mutex_unlock(gMac.mutex);
	return ret;
}


int nsm_mac_del_api(l2mac_t *mac)
{
	int ret = ERROR;
	l2mac_t *value;
	if(gMac.mutex)
		os_mutex_lock(gMac.mutex, OS_WAIT_FOREVER);

	value = l2mac_lookup_node(mac->mac, mac->vlan);
	if(value)
	{
		ret = hal_mac_del(value->ifindex, value->vlan, value->mac, 0);
		if(ret == OK)
		{
			lstDelete(gMac.macList, (NODE*)value);
			XFREE(MTYPE_VLAN, value);
			ret = OK;
		}
	}
	if(gMac.mutex)
		os_mutex_unlock(gMac.mutex);
	return ret;
}


int nsm_mac_get_api(mac_t *mac, vlan_t vlan, l2mac_t *gmac)
{
	int ret = ERROR;
	l2mac_t *value;
	if(gMac.mutex)
		os_mutex_lock(gMac.mutex, OS_WAIT_FOREVER);

	value = l2mac_lookup_node(mac, vlan);
	if(value)
	{
		if(gmac)
			os_memcpy(gmac, value, sizeof(l2mac_t));
		ret = OK;
	}
	if(gMac.mutex)
		os_mutex_unlock(gMac.mutex);
	return ret;
}


int nsm_mac_ageing_time_set_api(int ageing)
{
	int ret = ERROR;
	l2mac_t *value;
	if(gMac.mutex)
		os_mutex_lock(gMac.mutex, OS_WAIT_FOREVER);
	gMac.ageing_time = ageing;
	//TODO
	ret = hal_mac_age(gMac.ageing_time);
	if(gMac.mutex)
		os_mutex_unlock(gMac.mutex);
	return ret;
}

int nsm_mac_ageing_time_get_api(int *ageing)
{
	int ret = ERROR;
	l2mac_t *value;
	if(gMac.mutex)
		os_mutex_lock(gMac.mutex, OS_WAIT_FOREVER);
	if(ageing)
		*ageing = gMac.ageing_time;
	ret = OK;
	//TODO
	if(gMac.mutex)
		os_mutex_unlock(gMac.mutex);
	return ret;
}
