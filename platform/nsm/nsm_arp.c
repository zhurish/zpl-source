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
static int ip_arp_dynamic_update(void *pVoid);

#define ARP_DYNAMIC_INC(n)	(gIparp.dynamic_cnt) += (n)
#define ARP_DYNAMIC_SUB(n)	(gIparp.dynamic_cnt) -= (n)

#define ARP_STATIC_INC(n)	(gIparp.static_cnt) += (n)
#define ARP_STATIC_SUB(n)	(gIparp.static_cnt) -= (n)


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
	{
		ip_arp_cleanup(0, TRUE, 0, 0);
		lstFree(gIparp.arpList);
		free(gIparp.arpList);
		gIparp.arpList = NULL;
	}
	if(gIparp.mutex)
		os_mutex_exit(gIparp.mutex);
	return OK;
}



static int ip_arp_add_node(ip_arp_t *value)
{
	ip_arp_t *node = XMALLOC(MTYPE_ARP, sizeof(ip_arp_t));
	if(node)
	{
		os_memset(node, 0, sizeof(ip_arp_t));
		os_memcpy(node, value, sizeof(ip_arp_t));
		node->ttl = NSM_ARP_TTL_DEFAULT;
		if(node->class == ARP_DYNAMIC)
		{
			ARP_DYNAMIC_INC(1);
			if(gIparp.dynamic_cnt == 1)
				os_time_create_once(ip_arp_dynamic_update, NULL, 1);
		}
		else
			ARP_STATIC_INC(1);

		lstAdd(gIparp.arpList, (NODE *)node);
		return OK;
	}
	return ERROR;
}

static ip_arp_t * ip_arp_lookup_node(struct prefix *address)
{
	ip_arp_t *pstNode = NULL;
	NODE index;
	if(!lstCount(gIparp.arpList))
		return NULL;
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
	if(!lstCount(gIparp.arpList))
		return OK;
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
	ip_arp_t *node = NULL;
	if(gIparp.mutex)
		os_mutex_lock(gIparp.mutex, OS_WAIT_FOREVER);
	node = ip_arp_lookup_node(address);
	if(!node)
	{
		prefix_copy (&value.address, address);
		value.class = ARP_STATIC;
		value.ifindex = ifp->ifindex;
#ifdef PL_HAL_MODULE
		if(pal_interface_arp_add(ifp, address, mac) == OK)
#endif
		{
			os_memcpy(value.mac, mac, NSM_MAC_MAX);
			ret = ip_arp_add_node(&value);
		}
		//TODO
	}
	else
	{
		node->ttl = NSM_ARP_TTL_DEFAULT;
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
#ifdef PL_HAL_MODULE
		if(pal_interface_arp_delete(ifp, value->address) == OK)
#endif
		{
			ARP_STATIC_SUB(1);
			lstDelete(gIparp.arpList, (NODE*)value);
			XFREE(MTYPE_ARP, value);
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
	//ip_arp_t *value;
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
	//ip_arp_t *value;
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
	//ip_arp_t *value;
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
	//ip_arp_t *value;
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
	//ip_arp_t *value;
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
	if(!lstCount(gIparp.arpList))
		return OK;
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
			if(pstNode->class == ARP_DYNAMIC)
			{
				lstDelete(gIparp.arpList, (NODE*)pstNode);
				XFREE(MTYPE_ARP, pstNode);
				ARP_DYNAMIC_SUB(1);
			}
			else
			{
				ifp = if_lookup_by_index(pstNode->ifindex);
				if(ifp)
				{
#ifdef PL_HAL_MODULE
					if(pal_interface_arp_delete(ifp, pstNode->address) == OK)
#endif
					{
						lstDelete(gIparp.arpList, (NODE*)pstNode);
						XFREE(MTYPE_ARP, pstNode);
						ARP_STATIC_SUB(1);
					}
				}
			}
		}
		//all vrf
		if(vrf && pstNode->vrfid == vrf)
		{
			if(pstNode->class == ARP_DYNAMIC)
			{
				lstDelete(gIparp.arpList, (NODE*)pstNode);
				XFREE(MTYPE_ARP, pstNode);
				ARP_DYNAMIC_SUB(1);
			}
			else
			{
				ifp = if_lookup_by_index(pstNode->ifindex);
				if(ifp)
				{
#ifdef PL_HAL_MODULE
					if(pal_interface_arp_delete(ifp, pstNode->address) == OK)
#endif
					{
						lstDelete(gIparp.arpList, (NODE*)pstNode);
						XFREE(MTYPE_ARP, pstNode);
						ARP_STATIC_SUB(1);
					}
				}
			}
		}
		//all dynamic arp
		if(type && pstNode->class == type)
		{
			if(pstNode->class == ARP_DYNAMIC)
			{
				lstDelete(gIparp.arpList, (NODE*)pstNode);
				XFREE(MTYPE_ARP, pstNode);
				ARP_DYNAMIC_SUB(1);
			}
			else
			{
				ifp = if_lookup_by_index(pstNode->ifindex);
				if(ifp)
				{
#ifdef PL_HAL_MODULE
					if(pal_interface_arp_delete(ifp, pstNode->address) == OK)
#endif
					{
						lstDelete(gIparp.arpList, (NODE*)pstNode);
						XFREE(MTYPE_ARP, pstNode);
						ARP_STATIC_SUB(1);
					}
				}
			}
		}
		//all
		else if(pstNode && all)
		{
			if(pstNode->class == ARP_DYNAMIC)
			{
				lstDelete(gIparp.arpList, (NODE*)pstNode);
				XFREE(MTYPE_ARP, pstNode);
				ARP_DYNAMIC_SUB(1);
			}
			else
			{
				ifp = if_lookup_by_index(pstNode->ifindex);
				if(ifp)
				{
#ifdef PL_HAL_MODULE
					if(pal_interface_arp_delete(ifp, pstNode->address) == OK)
#endif
					{
						lstDelete(gIparp.arpList, (NODE*)pstNode);
						XFREE(MTYPE_ARP, pstNode);
						ARP_STATIC_SUB(1);
					}
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
	ip_arp_t *node = NULL;
	if(gIparp.mutex)
		os_mutex_lock(gIparp.mutex, OS_WAIT_FOREVER);
	node = ip_arp_lookup_node(&value->address);
	if(!node)
	{
		ret = ip_arp_add_node(value);
	}
	else
	{
		node->ttl = NSM_ARP_TTL_DEFAULT;
	}
	if(gIparp.mutex)
		os_mutex_unlock(gIparp.mutex);
	if(value)
		XFREE(MTYPE_ARP, value);
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
		ARP_DYNAMIC_SUB(1);
		lstDelete(gIparp.arpList, (NODE*)pvalue);
		XFREE(MTYPE_ARP, pvalue);
	}
	if(gIparp.mutex)
		os_mutex_unlock(gIparp.mutex);
	if(value)
		XFREE(MTYPE_ARP, value);
	return ret;
}

int ip_arp_dynamic_cb(int action, void *pVoid)
{
	if(action)
		return os_job_add(ip_arp_dynamic_add_cb, pVoid);
	else
		return os_job_add(ip_arp_dynamic_del_cb, pVoid);
}


static int ip_arp_dynamic_ttl_update(ip_arp_t *value, void *pVoid)
{
	if(value && value->class == ARP_DYNAMIC)
	{
		value->ttl--;
		if(value->ttl == 0)
		{
			ARP_DYNAMIC_SUB(1);
			lstDelete(gIparp.arpList, (NODE*)value);
			XFREE(MTYPE_ARP, value);
		}
	}
	return OK;
}

static int ip_arp_dynamic_update(void *pVoid)
{
	nsm_ip_arp_callback_api(ip_arp_dynamic_ttl_update, NULL);
	if(gIparp.dynamic_cnt == 0)
		os_time_create_once(ip_arp_dynamic_update, pVoid, 1);
	return OK;
}



static int _nsm_ip_arp_table_config(ip_arp_t *node, struct vty *vty)
{
	char mac[32], ip[128], ifname[32];
	union prefix46constptr pu;
	//struct vty *vty = user->vty;
	if(node->class  != MAC_STATIC)
		return 0;
	os_memset(mac, 0, sizeof(mac));
	os_memset(ip, 0, sizeof(ip));
	os_memset(ifname, 0, sizeof(ifname));
/*	sprintf(mac, "%02x%02x-%02x%02x-%02x%02x",node->mac[0],node->mac[1],node->mac[2],
											 node->mac[3],node->mac[4],node->mac[5]);*/

	sprintf(mac, "%s", if_mac_out_format(node->mac, NSM_MAC_MAX));
	sprintf(ifname, "%s",ifindex2ifname(node->ifindex));
	//ip arp 1.1.1.1 0000-1111-2222 interface gigabitethernet 0/1/1
	//ip arp 1.1.1.1 0000-1111-2222

	pu.p = &node->address;
	prefix_2_address_str (pu, ip, sizeof(ip));

	vty_out(vty, "ip arp %s %s %s %s", /*inet_ntoa(node->address.u.prefix4)*/ip, mac, ifname, VTY_NEWLINE);
/*	if(node->action == ARP_DYNAMIC)
	{
		vty_out(vty, "ip arp %s %s", mac, VTY_NEWLINE);
	}
	if(node->action == ARP_STATIC)
	{
		vty_out(vty, "mac-address-table %s forward interface %s %s %s", mac, ifname, vlan, VTY_NEWLINE);
	}*/
	return OK;
}

int nsm_ip_arp_config(struct vty *vty)
{
	nsm_ip_arp_callback_api((ip_arp_cb)_nsm_ip_arp_table_config, vty);
	return 1;
}

int nsm_ip_arp_ageing_config(struct vty *vty)
{
	int agtime = 0;
	nsm_ip_arp_ageing_time_get_api(&agtime);
	//ip arp ageing-time 33
	vty_out(vty, "ip arp ageing-time %d %s", agtime, VTY_NEWLINE);
	return 1;
}


//
