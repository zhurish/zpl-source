/*
 * nsm_arp.c
 *
 *  Created on: Apr 8, 2018
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
#include "nsm_arp.h"

static Gip_arp_t gIparp;

static int ip_arp_cleanup(arp_class_t type, BOOL all, ifindex_t ifindex, vrf_id_t id);

int nsm_ip_arp_init(void)
{
	gIparp.arpList = malloc(sizeof(LIST));
	gIparp.mutex = os_mutex_init();
	lstInit(gIparp.arpList);
	return OK;
}


int nsm_ip_arp_exit(void)
{
	if(lstCount(gIparp.arpList))
		ip_arp_cleanup(0, TRUE, 0, 0);

	if(gIparp.mutex)
		os_mutex_exit(gIparp.mutex);
	return OK;
}



static int ip_arp_add_node(ip_arp_t *value)
{
	ip_arp_t *node = XMALLOC(MTYPE_VLAN, sizeof(ip_arp_t));
	if(node)
	{
		os_memset(node, 0, sizeof(ip_arp_t));
		os_memcpy(node, value, sizeof(ip_arp_t));
		lstAdd(gIparp.arpList, (NODE *)node);
		return OK;
	}
	return ERROR;
}

static ip_arp_t * ip_arp_lookup_node(struct prefix *address)
{
	ip_arp_t *pstNode = NULL;
	NODE index;
	for(pstNode = (ip_arp_t *)lstFirst(gIparp.arpList);
			pstNode != NULL;  pstNode = (ip_arp_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(prefix_same (&pstNode->address, address))
		{
			return pstNode;
		}
	}
	return NULL;
}


int nsm_ip_arp_callback_api(ip_arp_cb cb, void *pVoid)
{
	ip_arp_t *pstNode = NULL;
	NODE index;
	if(gIparp.mutex)
		os_mutex_lock(gIparp.mutex, OS_WAIT_FOREVER);
	for(pstNode = (ip_arp_t *)lstFirst(gIparp.arpList);
			pstNode != NULL;  pstNode = (ip_arp_t *)lstNext((NODE*)&index))
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
	if(gIparp.mutex)
		os_mutex_unlock(gIparp.mutex);
	return OK;
}


int nsm_ip_arp_lookup_api(struct prefix *address)
{
	if(gIparp.mutex)
		os_mutex_lock(gIparp.mutex, OS_WAIT_FOREVER);
	ip_arp_t *value = ip_arp_lookup_node(address);
	if(gIparp.mutex)
		os_mutex_unlock(gIparp.mutex);
	return value ? OK:ERROR;
}


int nsm_ip_arp_add_api(struct interface *ifp, struct prefix *address, char *mac)
{
	int ret = ERROR;
	ip_arp_t value;
	if(gIparp.mutex)
		os_mutex_lock(gIparp.mutex, OS_WAIT_FOREVER);
	if(!ip_arp_lookup_node(address))
	{
		prefix_copy (&value.address, address);
		value.class = ARP_STATIC;
		value.ifindex = ifp->ifindex;
		if(pal_ip_stack_arp_add(ifp, address, mac) == OK)
		{
			os_memcpy(value.mac, mac, NSM_MAC_MAX);
			ret = ip_arp_add_node(&value);
		}
		//TODO
	}
	if(gIparp.mutex)
		os_mutex_unlock(gIparp.mutex);
	return ret;
}


int nsm_ip_arp_del_api(struct interface *ifp, struct prefix *address)
{
	int ret = ERROR;
	ip_arp_t *value;
	if(gIparp.mutex)
		os_mutex_lock(gIparp.mutex, OS_WAIT_FOREVER);

	value = ip_arp_lookup_node(address);
	if(value && value->class == ARP_STATIC)
	{
		if(pal_ip_stack_arp_delete(ifp, value->address) == OK)
		{
			lstDelete(gIparp.arpList, (NODE*)value);
			XFREE(MTYPE_VLAN, value);
			ret = OK;
		}
	}
	if(gIparp.mutex)
		os_mutex_unlock(gIparp.mutex);
	return ret;
}


int nsm_ip_arp_get_api(struct prefix *address, ip_arp_t *gip_arp)
{
	int ret = ERROR;
	ip_arp_t *value;
	if(gIparp.mutex)
		os_mutex_lock(gIparp.mutex, OS_WAIT_FOREVER);

	value = ip_arp_lookup_node(address);
	if(value)
	{
		if(gip_arp)
			os_memcpy(gip_arp, value, sizeof(ip_arp_t));
		ret = OK;
	}
	if(gIparp.mutex)
		os_mutex_unlock(gIparp.mutex);
	return ret;
}


int nsm_ip_arp_ageing_time_set_api(int ageing)
{
	int ret = ERROR;
	if(gIparp.mutex)
		os_mutex_lock(gIparp.mutex, OS_WAIT_FOREVER);
	gIparp.ageing_time = ageing;
	//TODO
	ret = OK;
	if(gIparp.mutex)
		os_mutex_unlock(gIparp.mutex);
	return ret;
}

int nsm_ip_arp_ageing_time_get_api(int *ageing)
{
	int ret = ERROR;
	ip_arp_t *value;
	if(gIparp.mutex)
		os_mutex_lock(gIparp.mutex, OS_WAIT_FOREVER);
	if(ageing)
		*ageing = gIparp.ageing_time;
	ret = OK;
	//TODO
	if(gIparp.mutex)
		os_mutex_unlock(gIparp.mutex);
	return ret;
}

int nsm_ip_arp_timeout_set_api(int ageing)
{
	int ret = ERROR;
	if(gIparp.mutex)
		os_mutex_lock(gIparp.mutex, OS_WAIT_FOREVER);
	gIparp.timeout = ageing;
	//TODO
	ret = OK;
	if(gIparp.mutex)
		os_mutex_unlock(gIparp.mutex);
	return ret;
}
int nsm_ip_arp_timeout_get_api(int *ageing)
{
	int ret = ERROR;
	ip_arp_t *value;
	if(gIparp.mutex)
		os_mutex_lock(gIparp.mutex, OS_WAIT_FOREVER);
	if(ageing)
		*ageing = gIparp.timeout;
	ret = OK;
	//TODO
	if(gIparp.mutex)
		os_mutex_unlock(gIparp.mutex);
	return ret;
}


int nsm_ip_arp_retry_interval_set_api(int ageing)
{
	int ret = ERROR;
	if(gIparp.mutex)
		os_mutex_lock(gIparp.mutex, OS_WAIT_FOREVER);
	gIparp.retry_interval = ageing;
	//TODO
	ret = OK;
	if(gIparp.mutex)
		os_mutex_unlock(gIparp.mutex);
	return ret;
}
int nsm_ip_arp_retry_interval_get_api(int *ageing)
{
	int ret = ERROR;
	ip_arp_t *value;
	if(gIparp.mutex)
		os_mutex_lock(gIparp.mutex, OS_WAIT_FOREVER);
	if(ageing)
		*ageing = gIparp.retry_interval;
	ret = OK;
	//TODO
	if(gIparp.mutex)
		os_mutex_unlock(gIparp.mutex);
	return ret;
}

int nsm_ip_arp_proxy_set_api(int ageing)
{
	int ret = ERROR;
	if(gIparp.mutex)
		os_mutex_lock(gIparp.mutex, OS_WAIT_FOREVER);
	gIparp.arp_proxy = ageing;
	//TODO
	ret = OK;
	if(gIparp.mutex)
		os_mutex_unlock(gIparp.mutex);
	return ret;
}
int nsm_ip_arp_proxy_get_api(int *ageing)
{
	int ret = ERROR;
	ip_arp_t *value;
	if(gIparp.mutex)
		os_mutex_lock(gIparp.mutex, OS_WAIT_FOREVER);
	if(ageing)
		*ageing = gIparp.arp_proxy;
	ret = OK;
	//TODO
	if(gIparp.mutex)
		os_mutex_unlock(gIparp.mutex);
	return ret;
}

int nsm_ip_arp_proxy_local_set_api(int ageing)
{
	int ret = ERROR;
	if(gIparp.mutex)
		os_mutex_lock(gIparp.mutex, OS_WAIT_FOREVER);
	gIparp.arp_proxy_local = ageing;
	//TODO
	ret = OK;
	if(gIparp.mutex)
		os_mutex_unlock(gIparp.mutex);
	return ret;
}

int nsm_ip_arp_proxy_local_get_api(int *ageing)
{
	int ret = ERROR;
	ip_arp_t *value;
	if(gIparp.mutex)
		os_mutex_lock(gIparp.mutex, OS_WAIT_FOREVER);
	if(ageing)
		*ageing = gIparp.arp_proxy_local;
	ret = OK;
	//TODO
	if(gIparp.mutex)
		os_mutex_unlock(gIparp.mutex);
	return ret;
}

static int ip_arp_cleanup(arp_class_t type, BOOL all, ifindex_t ifindex, vrf_id_t vrf)
{
	ip_arp_t *pstNode = NULL;
	NODE index;
	struct interface *ifp = NULL;
	if(ifindex)
		ifp = if_lookup_by_index(ifp);
	if(gIparp.mutex)
		os_mutex_lock(gIparp.mutex, OS_WAIT_FOREVER);
	for(pstNode = (ip_arp_t *)lstFirst(gIparp.arpList);
			pstNode != NULL;  pstNode = (ip_arp_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		//all dynamic arp on interface
		if(ifindex && pstNode && pstNode->ifindex == ifindex && pstNode->class == type)
		{
			if(ifp)
			{
				if(pal_ip_stack_arp_delete(ifp, pstNode->address) == OK)
				{
					lstDelete(gIparp.arpList, (NODE*)pstNode);
					XFREE(MTYPE_VLAN, pstNode);
				}
			}
		}
		//all vrf
		if(vrf && pstNode->vrfid == vrf)
		{
			ifp = if_lookup_by_index(pstNode->ifindex);
			if(ifp)
			{
				if(pal_ip_stack_arp_delete(ifp, pstNode->address) == OK)
				{
					lstDelete(gIparp.arpList, (NODE*)pstNode);
					XFREE(MTYPE_VLAN, pstNode);
				}
			}
		}
		//all dynamic arp
		if(type && pstNode->class == type)
		{
			ifp = if_lookup_by_index(pstNode->ifindex);
			if(ifp)
			{
				if(pal_ip_stack_arp_delete(ifp, pstNode->address) == OK)
				{
					lstDelete(gIparp.arpList, (NODE*)pstNode);
					XFREE(MTYPE_VLAN, pstNode);
				}
			}
		}
		//all
		else if(pstNode && all)
		{
			ifp = if_lookup_by_index(pstNode->ifindex);
			if(ifp)
			{
				if(pal_ip_stack_arp_delete(ifp, pstNode->address) == OK)
				{
					lstDelete(gIparp.arpList, (NODE*)pstNode);
					XFREE(MTYPE_VLAN, pstNode);
				}
			}
		}
	}
	if(gIparp.mutex)
		os_mutex_unlock(gIparp.mutex);
	return OK;
}


int ip_arp_cleanup_api(arp_class_t type, BOOL all, ifindex_t ifindex)
{
	return ip_arp_cleanup( type,  all,  ifindex, 0);
}

static int ip_arp_dynamic_add_cb(ip_arp_t *value)
{
	int ret = ERROR;
	//ip_arp_t value;
	if(gIparp.mutex)
		os_mutex_lock(gIparp.mutex, OS_WAIT_FOREVER);
	if(!ip_arp_lookup_node(&value->address))
	{
		ret = ip_arp_add_node(value);
	}
	if(gIparp.mutex)
		os_mutex_unlock(gIparp.mutex);
	if(value)
		XFREE(MTYPE_VLAN, value);
	return ret;
}

static int ip_arp_dynamic_del_cb(ip_arp_t *value)
{
	int ret = ERROR;
	ip_arp_t *pvalue = NULL;
	if(gIparp.mutex)
		os_mutex_lock(gIparp.mutex, OS_WAIT_FOREVER);
	pvalue = ip_arp_lookup_node(&value->address);
	if(pvalue)
	{
		lstDelete(gIparp.arpList, (NODE*)pvalue);
		XFREE(MTYPE_VLAN, pvalue);
	}
	if(gIparp.mutex)
		os_mutex_unlock(gIparp.mutex);
	if(value)
		XFREE(MTYPE_VLAN, value);
	return ret;
}

int ip_arp_dynamic_cb(int action, void *pVoid)
{
	if(action)
		return os_job_add(ip_arp_dynamic_add_cb, pVoid);
	else
		return os_job_add(ip_arp_dynamic_del_cb, pVoid);
}
