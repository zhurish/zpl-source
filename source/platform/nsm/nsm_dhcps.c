/*
 * nsm_dhcps.c
 *
 *  Created on: Oct 23, 2018
 *      Author: zhurish
 */


#include "auto_include.h"
#include "zplos_include.h"
#include "if.h"

#include "prefix.h"
#include "vty.h"
#include "zmemory.h"
#include "nsm_dhcp.h"

#ifdef ZPL_DHCPS_MODULE

#ifdef ZPL_DHCP_MODULE
#include "dhcp_config.h"
#include "dhcpd.h"
#include "dhcp_api.h"
#include "dhcp_lease.h"
#endif

Gnsm_dhcps_t dhcps_list;
static int nsm_dhcps_cleanup( zpl_bool all);

static int nsm_dhcps_host_cleanup(nsm_dhcps_t *dhcps);
static int nsm_dhcps_exclude_list_cleanup(nsm_dhcps_t *dhcps);


int nsm_dhcps_init(void)
{
	memset(&dhcps_list, 0, sizeof(dhcps_list));
	dhcps_list.dhcpslist = malloc(sizeof(LIST));
	dhcps_list.mutex = os_mutex_name_create("dhcps-mutex");
	lstInit(dhcps_list.dhcpslist);
	return OK;
}


int nsm_dhcps_exit(void)
{
	if(lstCount(dhcps_list.dhcpslist))
	{
		nsm_dhcps_cleanup(zpl_true);
		lstFree(dhcps_list.dhcpslist);
	}
	if(dhcps_list.dhcpslist)
		free(dhcps_list.dhcpslist);
	if(dhcps_list.mutex)
		os_mutex_destroy(dhcps_list.mutex);
	return OK;
}

static int nsm_dhcps_cleanup( zpl_bool all)
{
	nsm_dhcps_t *pstNode = NULL;
	NODE index;
	if(dhcps_list.mutex)
		os_mutex_lock(dhcps_list.mutex, OS_WAIT_FOREVER);
	for(pstNode = (nsm_dhcps_t *)lstFirst(dhcps_list.dhcpslist);
			pstNode != NULL;  pstNode = (nsm_dhcps_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode && all)
		{
			lstDelete(dhcps_list.dhcpslist, (NODE*)pstNode);
			nsm_dhcps_host_cleanup(pstNode);
			lstFree(&pstNode->hostlist);
			nsm_dhcps_exclude_list_cleanup(pstNode);
			lstFree(&pstNode->excludedlist);
			dhcpd_pool_del(pstNode->name);
			XFREE(MTYPE_DHCPS_POOL, pstNode);
		}
	}
	if(dhcps_list.mutex)
		os_mutex_unlock(dhcps_list.mutex);
	return OK;
}

static int nsm_dhcps_add_node(nsm_dhcps_t *node)
{
	//nsm_dhcps_t *node = XMALLOC(MTYPE_DHCPS_POOL, sizeof(nsm_dhcps_t));
	if(node)
	{
		//os_memset(node, 0, sizeof(nsm_dhcps_t));
		//os_memcpy(node, value, sizeof(nsm_dhcps_t));
		node->lease_time = DHCPS_LEASE_DEFAULT;
		node->active = zpl_false;
		lstInit(&node->hostlist);
		lstInit(&node->excludedlist);
		lstAdd(dhcps_list.dhcpslist, (NODE *)node);
		return OK;
	}
	return ERROR;
}

static nsm_dhcps_t * nsm_dhcps_lookup_node(zpl_char *name)
{
	nsm_dhcps_t *pstNode = NULL;
	NODE index;
	for(pstNode = (nsm_dhcps_t *)lstFirst(dhcps_list.dhcpslist);
			pstNode != NULL;  pstNode = (nsm_dhcps_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(strcmp (pstNode->name, name) == 0)
		{
			return pstNode;
		}
	}
	return NULL;
}

int nsm_dhcps_foreach(int(*cb)(void *, void *), void *p)
{
	nsm_dhcps_t *pstNode = NULL;
	NODE index;
	for(pstNode = (nsm_dhcps_t *)lstFirst(dhcps_list.dhcpslist);
			pstNode != NULL;  pstNode = (nsm_dhcps_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(cb)
		{
			(cb)(pstNode, p);
		}
	}
	return 0;
}

nsm_dhcps_t * nsm_dhcps_lookup_api(zpl_char *name)
{
	if(dhcps_list.mutex)
		os_mutex_lock(dhcps_list.mutex, OS_WAIT_FOREVER);
	nsm_dhcps_t *value = nsm_dhcps_lookup_node(name);
	if(dhcps_list.mutex)
		os_mutex_unlock(dhcps_list.mutex);
	return value;
}


int nsm_dhcps_add_api(zpl_char *name)
{
	int ret = ERROR;
	nsm_dhcps_t *node = NULL;
	if(dhcps_list.mutex)
		os_mutex_lock(dhcps_list.mutex, OS_WAIT_FOREVER);
	if(!nsm_dhcps_lookup_node(name))
	{
		node = XMALLOC(MTYPE_DHCPS_POOL, sizeof(nsm_dhcps_t));
		if(node)
		{
			os_memset(node, 0, sizeof(nsm_dhcps_t));
			os_strcpy(node->name, name);

			node->pool = dhcpd_pool_create(name);

			if(node->pool)
				ret = nsm_dhcps_add_node(node);
			else
			{
				XFREE(MTYPE_DHCPS_POOL, node);
			}
		}
		//TODO
	}
	if(dhcps_list.mutex)
		os_mutex_unlock(dhcps_list.mutex);
	return ret;
}


int nsm_dhcps_del_api(zpl_char *name)
{
	int ret = ERROR;
	nsm_dhcps_t *value = NULL;
	if(dhcps_list.mutex)
		os_mutex_lock(dhcps_list.mutex, OS_WAIT_FOREVER);
	value = nsm_dhcps_lookup_node(name);
	if(value)
	{
		nsm_dhcps_host_cleanup(value);
		lstFree(&value->hostlist);
		nsm_dhcps_exclude_list_cleanup(value);
		lstFree(&value->excludedlist);

		if(dhcpd_pool_del(name) == OK)
		{
			lstDelete(dhcps_list.dhcpslist, (NODE*)value);
			XFREE(MTYPE_DHCPS_POOL, value);
			ret = OK;
		}
	}
	if(dhcps_list.mutex)
		os_mutex_unlock(dhcps_list.mutex);
	return ret;
}

/******************************* host **********************************/
static int nsm_dhcps_host_cleanup(nsm_dhcps_t *dhcps)
{
	nsm_dhcps_host_t *pstNode = NULL;
	NODE index;
	for(pstNode = (nsm_dhcps_host_t *)lstFirst(&dhcps->hostlist);
			pstNode != NULL;  pstNode = (nsm_dhcps_host_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode)
		{
			lstDelete(&dhcps->hostlist, (NODE*)pstNode);
			XFREE(MTYPE_DHCPS_ADDR, pstNode);
		}
	}
	return OK;
}

static nsm_dhcps_host_t * nsm_dhcps_host_lookup_node(nsm_dhcps_t *dhcps, zpl_char *address, zpl_uint8 *mac)
{
	nsm_dhcps_host_t *pstNode = NULL;
	NODE index;
	for(pstNode = (nsm_dhcps_host_t *)lstFirst(&dhcps->hostlist);
			pstNode != NULL;  pstNode = (nsm_dhcps_host_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(address)
		{
			int ret = 0;
			struct prefix paddress;
			ret = str2prefix_ipv4 (address, (struct prefix_ipv4 *)&paddress);
			if (ret <= 0)
			{
				return NULL;
			}
			if(prefix_same(&paddress, &pstNode->address))
				return pstNode;
		}
		else
		{
			if(mac && memcmp(pstNode->mac, mac, ETHER_ADDR_LEN) == 0)
				return pstNode;
		}
	}
	return NULL;
}

nsm_dhcps_host_t * nsm_dhcps_host_lookup_api(nsm_dhcps_t *dhcps, zpl_char *address, zpl_uint8 *mac)
{
	nsm_dhcps_host_t *node = NULL;
	if(dhcps_list.mutex)
		os_mutex_lock(dhcps_list.mutex, OS_WAIT_FOREVER);
	node = nsm_dhcps_host_lookup_node(dhcps, address, mac);
	if(dhcps_list.mutex)
		os_mutex_unlock(dhcps_list.mutex);
	return node;
}

int nsm_dhcps_add_host_api(nsm_dhcps_t *dhcps, zpl_char *address, zpl_uint8 *mac)
{
	int ret = ERROR, create_node = 0;
	nsm_dhcps_host_t *node = NULL;
	if(dhcps_list.mutex)
		os_mutex_lock(dhcps_list.mutex, OS_WAIT_FOREVER);

	if(!nsm_dhcps_host_lookup_node(dhcps, address, mac))
	{
		node = XMALLOC(MTYPE_DHCPS_ADDR, sizeof(nsm_dhcps_host_t));
		create_node = 1;
	}
		if(node)
		{
			if(create_node)
				os_memset(node, 0, sizeof(nsm_dhcps_host_t));
			if(mac)
				memcpy(node->mac, mac, ETHER_ADDR_LEN);
			if(str2prefix_ipv4 (address, (struct prefix_ipv4 *)&node->address))
			{
				if(create_node)
					lstAdd(&dhcps->hostlist, (NODE *)node);
				ret = OK;
			}
		}
		//TODO
	//}
	if(dhcps_list.mutex)
		os_mutex_unlock(dhcps_list.mutex);
	return ret;
}


int nsm_dhcps_del_host_api(nsm_dhcps_t *dhcps, zpl_char *address, zpl_uint8 *mac)
{
	int ret = ERROR;
	nsm_dhcps_host_t *node = NULL;
	if(dhcps_list.mutex)
		os_mutex_lock(dhcps_list.mutex, OS_WAIT_FOREVER);
	node = nsm_dhcps_host_lookup_node(dhcps, address, mac);
	if(node)
	{
		lstDelete(&dhcps->hostlist, (NODE*)node);
		XFREE(MTYPE_DHCPS_ADDR, node);
		ret = OK;
		//TODO
	}
	if(dhcps_list.mutex)
		os_mutex_unlock(dhcps_list.mutex);
	return ret;
}


/******************************* exclude list **********************************/
static int nsm_dhcps_exclude_list_cleanup(nsm_dhcps_t *dhcps)
{
	nsm_dhcps_exclude_t *pstNode = NULL;
	NODE index;
	for(pstNode = (nsm_dhcps_exclude_t *)lstFirst(&dhcps->excludedlist);
			pstNode != NULL;  pstNode = (nsm_dhcps_exclude_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode)
		{
			lstDelete(&dhcps->excludedlist, (NODE*)pstNode);
			XFREE(MTYPE_DHCPS_ADDR, pstNode);
		}
	}
	return OK;
}

static nsm_dhcps_exclude_t * nsm_dhcps_exclude_list_lookup_node(nsm_dhcps_t *dhcps, zpl_char *address, zpl_char *endaddress)
{
	nsm_dhcps_exclude_t *pstNode = NULL;
	NODE index;
	for(pstNode = (nsm_dhcps_exclude_t *)lstFirst(&dhcps->excludedlist);
			pstNode != NULL;  pstNode = (nsm_dhcps_exclude_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(address)
		{
			int ret = 0;
			struct prefix s_address;
			struct prefix e_address;
			ret = str2prefix_ipv4 (address, (struct prefix_ipv4 *)&s_address);
			if (ret <= 0)
			{
				return NULL;
			}
			if(endaddress)
			{
				ret = str2prefix_ipv4 (address, (struct prefix_ipv4 *)&e_address);
				if (ret <= 0)
				{
					return NULL;
				}
			}
			if(!pstNode->range)
			{
				if(!endaddress)
				{
					if(prefix_same(&s_address, &pstNode->start_address))
						return pstNode;
				}
			}
			else
			{
				if(endaddress)
				{
					if(ntohl(s_address.u.prefix4.s_addr) >= ntohl(pstNode->start_address.u.prefix4.s_addr)
							&& ntohl(e_address.u.prefix4.s_addr) <= ntohl(pstNode->end_address.u.prefix4.s_addr) )
						return pstNode;
				}
				else
				{
					if(ntohl(s_address.u.prefix4.s_addr) >= ntohl(pstNode->start_address.u.prefix4.s_addr)
							&& ntohl(s_address.u.prefix4.s_addr) <= ntohl(pstNode->end_address.u.prefix4.s_addr) )
						return pstNode;
				}
			}
		}
	}
	return NULL;
}

nsm_dhcps_exclude_t * nsm_dhcps_exclude_list_lookup_api(nsm_dhcps_t *dhcps, zpl_char *address, zpl_char *endaddress)
{
	nsm_dhcps_exclude_t *node = NULL;
	if(dhcps_list.mutex)
		os_mutex_lock(dhcps_list.mutex, OS_WAIT_FOREVER);
	node = nsm_dhcps_exclude_list_lookup_node(dhcps, address, endaddress);
	if(dhcps_list.mutex)
		os_mutex_unlock(dhcps_list.mutex);
	return node;
}


int nsm_dhcps_add_exclude_list_api(nsm_dhcps_t *dhcps, zpl_char *address, zpl_char *endaddress)
{
	int ret = ERROR, create_node = 0;
	nsm_dhcps_exclude_t *node = NULL;
	if(dhcps_list.mutex)
		os_mutex_lock(dhcps_list.mutex, OS_WAIT_FOREVER);

	if(!nsm_dhcps_exclude_list_lookup_node(dhcps, address, endaddress))
	{
		node = XMALLOC(MTYPE_DHCPS_ADDR, sizeof(nsm_dhcps_host_t));
		create_node = 1;
	}
		if(node)
		{
			if(create_node)
				os_memset(node, 0, sizeof(nsm_dhcps_host_t));

			if(str2prefix_ipv4 (address, (struct prefix_ipv4 *)&node->start_address))
			{
				if(endaddress && str2prefix_ipv4 (endaddress, (struct prefix_ipv4 *)&node->end_address))
				{
					node->range = zpl_true;
					if(create_node)
						lstAdd(&dhcps->excludedlist, (NODE *)node);
					ret = OK;
				}
				else
				{
					if(create_node)
						lstAdd(&dhcps->excludedlist, (NODE *)node);
					ret = OK;
				}
			}
		}
		//TODO
	//}
	if(dhcps_list.mutex)
		os_mutex_unlock(dhcps_list.mutex);
	return ret;
}


int nsm_dhcps_del_exclude_list_api(nsm_dhcps_t *dhcps, zpl_char *address, zpl_char *endaddress)
{
	int ret = ERROR;
	nsm_dhcps_exclude_t *node = NULL;
	if(dhcps_list.mutex)
		os_mutex_lock(dhcps_list.mutex, OS_WAIT_FOREVER);
	node = nsm_dhcps_exclude_list_lookup_node(dhcps, address, endaddress);
	if(node)
	{
		lstDelete(&dhcps->excludedlist, (NODE*)node);
		XFREE(MTYPE_DHCPS_ADDR, node);
		ret = OK;
		//TODO
	}
	if(dhcps_list.mutex)
		os_mutex_unlock(dhcps_list.mutex);
	return ret;
}

/***************************************************************/


static int nsm_dhcps_address_check(nsm_dhcps_t *dhcps, zpl_uint32 cmd, void *val)
{
	int ret = ERROR;
	switch(cmd)
	{
	case DHCPS_CMD_NETWORK:
		if(prefix_check_addr(val))
			ret = OK;
		break;
	case DHCPS_CMD_NETWORK_START:
		if(prefix_check_addr(val))
			ret = OK;
		break;
	case DHCPS_CMD_NETWORK_END:
		if(prefix_check_addr(val))
			ret = OK;
		break;
	case DHCPS_CMD_GATEWAY:
	case DHCPS_CMD_GATEWAY_SECONDARY:
/*		if(prefix_check_addr(val))
		{
			if(dhcps->address.prefixlen)
			{
				if(prefix_match(&dhcps->address, val))
					ret = OK;
			}
			else if(dhcps->start_address.prefixlen)
			{
				if(prefix_match(&dhcps->start_address, val))
				{
					struct prefix *address = (struct prefix *)val;
					if(ntohl(address->u.prefix4.s_addr) < ntohl(dhcps->end_address.u.prefix4.s_addr))
						ret = OK;
				}
			}
		}
		if(if_lookup_prefix(val))
		{
			ret = OK;
		}
		else
			ret = ERROR;*/
		ret = OK;
		break;
	case DHCPS_CMD_NETBIOS:
	case DHCPS_CMD_NETBIOS_SECONDARY:
	case DHCPS_CMD_DNS:
	case DHCPS_CMD_DNS_SECONDARY:
	case DHCPS_CMD_TFTP:
		if(prefix_check_addr(val))
		{
			ret = OK;
		}
		break;
	case DHCPS_CMD_DOMAIN_NAME:
		ret = OK;
		break;
	case DHCPS_CMD_LEASE:
		ret = OK;
		break;
	case DHCPS_CMD_HOST:
		ret = OK;
		break;
	case DHCPS_CMD_EXCUDED:
		ret = OK;
		break;
	default:
		ret = OK;
		break;
	}
	return ret;
}

int nsm_dhcps_set_api(nsm_dhcps_t *dhcps, zpl_uint32 cmd, void *val)
{
	zpl_char prefix[128];
	union prefix46constptr pu;
	int ret = ERROR;
	if(!val)
		return ERROR;
	if(dhcps_list.mutex)
		os_mutex_lock(dhcps_list.mutex, OS_WAIT_FOREVER);
	if(nsm_dhcps_address_check(dhcps, cmd, val) != OK)
	{
		if(dhcps_list.mutex)
			os_mutex_unlock(dhcps_list.mutex);
		return ret;
	}
	switch(cmd)
	{
	case DHCPS_CMD_NETWORK:
		if(dhcps->pool)
		{
			struct prefix startp, endp;
			prefix_copy(&startp, val);
			prefix_copy(&endp, val);
			startp.u.prefix4.s_addr = htonl(ntohl(startp.u.prefix4.s_addr) | 1);
			endp.u.prefix4.s_addr = htonl(ntohl(endp.u.prefix4.s_addr) | 254);

			if(dhcpd_pool_set_address_range(dhcps->pool, startp.u.prefix4.s_addr,
					endp.u.prefix4.s_addr) == OK)
			{
				memcpy(&dhcps->start_address, &startp, sizeof(struct prefix));
				memcpy(&dhcps->end_address, &endp, sizeof(struct prefix));
				ret = OK;
			}
		}
		break;
	case DHCPS_CMD_NETWORK_START:
		if(dhcps->pool)
		{
			struct prefix startp;
			prefix_copy(&startp, val);
			if(dhcps->end_address.u.prefix4.s_addr)
			{
				if(dhcpd_pool_set_address_range(dhcps->pool, startp.u.prefix4.s_addr,
						dhcps->end_address.u.prefix4.s_addr) == OK)
				{
					memcpy(&dhcps->start_address, &startp, sizeof(struct prefix));
					ret = OK;
				}
			}
			else
			{
				memcpy(&dhcps->start_address, &startp, sizeof(struct prefix));
				ret = OK;
			}
		}
		memcpy(&dhcps->start_address, val, sizeof(struct prefix));
		ret = OK;
		break;
	case DHCPS_CMD_NETWORK_END:
		if(dhcps->pool)
		{
			struct prefix startp;
			prefix_copy(&startp, val);
			if(dhcps->start_address.u.prefix4.s_addr)
			{
				if(dhcpd_pool_set_address_range(dhcps->pool, dhcps->start_address.u.prefix4.s_addr,
						startp.u.prefix4.s_addr) == OK)
				{
					memcpy(&dhcps->end_address, &startp, sizeof(struct prefix));
					ret = OK;
				}
			}
			else
			{
				memcpy(&dhcps->end_address, &startp, sizeof(struct prefix));
				ret = OK;
			}
		}
		//ret = OK;
		break;
	case DHCPS_CMD_GATEWAY:
		memcpy(&dhcps->gateway, val, sizeof(struct prefix));
		if(dhcps->gateway.u.prefix4.s_addr)
		{
			memset(prefix, 0, sizeof(prefix));
			pu.p = &dhcps->gateway;
			prefix_2_address_str (pu, prefix, sizeof(prefix));

			dhcpd_pool_set_option(dhcps->pool, 3/*"routers"*/, prefix);

		}
		dhcps->ifp = if_lookup_prefix(val);
		ret = OK;
		break;
	case DHCPS_CMD_GATEWAY_SECONDARY:
		memcpy(&dhcps->gateway_secondary, val, sizeof(struct prefix));
		if(dhcps->gateway_secondary.u.prefix4.s_addr)
		{
			if(dhcps->gateway.u.prefix4.s_addr)
			{
				zpl_char prefix_sec[128];
				memset(prefix, 0, sizeof(prefix));
				pu.p = &dhcps->gateway;
				prefix_2_address_str (pu, prefix, sizeof(prefix));

				memset(prefix_sec, 0, sizeof(prefix_sec));
				pu.p = &dhcps->gateway_secondary;
				prefix_2_address_str (pu, prefix_sec, sizeof(prefix_sec));

				strcat(prefix, ",");
				strcat(prefix, prefix_sec);

				dhcpd_pool_set_option(dhcps->pool, 3/*"routers"*/, prefix);
			}
		}
		if(!dhcps->ifp)
			dhcps->ifp = if_lookup_prefix(val);
		ret = OK;
		break;
	case DHCPS_CMD_DOMAIN_NAME:
		memset(dhcps->domain_name, 0, sizeof(dhcps->domain_name));
		if(val)
			strcpy(dhcps->domain_name, (zpl_char *)val);

		if(strlen(dhcps->domain_name))
			dhcpd_pool_set_option(dhcps->pool, 15/*"routers"*/, dhcps->domain_name);

		ret = OK;
		break;
	case DHCPS_CMD_NETBIOS:
		memcpy(&dhcps->netbios, val, sizeof(struct prefix));
		if(dhcps->netbios.u.prefix4.s_addr)
		{
			memset(prefix, 0, sizeof(prefix));
			pu.p = &dhcps->netbios;
			prefix_2_address_str (pu, prefix, sizeof(prefix));

			dhcpd_pool_set_option(dhcps->pool, 44/*"routers"*/, prefix);

		}
		ret = OK;
		break;
	case DHCPS_CMD_NETBIOS_SECONDARY:
		memcpy(&dhcps->netbios_secondary, val, sizeof(struct prefix));
		if(dhcps->netbios_secondary.u.prefix4.s_addr)
		{
			if(dhcps->netbios.u.prefix4.s_addr)
			{
				zpl_char prefix_sec[128];
				memset(prefix, 0, sizeof(prefix));
				pu.p = &dhcps->netbios;
				prefix_2_address_str (pu, prefix, sizeof(prefix));

				memset(prefix_sec, 0, sizeof(prefix_sec));
				pu.p = &dhcps->netbios_secondary;
				prefix_2_address_str (pu, prefix_sec, sizeof(prefix_sec));

				strcat(prefix, ",");
				strcat(prefix, prefix_sec);

				dhcpd_pool_set_option(dhcps->pool, 44/*"routers"*/, prefix);

			}
		}
		ret = OK;
		break;
	case DHCPS_CMD_DNS:
		memcpy(&dhcps->dns, val, sizeof(struct prefix));
		if(dhcps->dns.u.prefix4.s_addr)
		{
			memset(prefix, 0, sizeof(prefix));
			pu.p = &dhcps->dns;
			prefix_2_address_str (pu, prefix, sizeof(prefix));

			dhcpd_pool_set_option(dhcps->pool, 6/*"routers"*/, prefix);

		}
		ret = OK;
		break;
	case DHCPS_CMD_DNS_SECONDARY:
		memcpy(&dhcps->dns_secondary, val, sizeof(struct prefix));
		if(dhcps->dns_secondary.u.prefix4.s_addr)
		{
			if(dhcps->dns.u.prefix4.s_addr)
			{
				zpl_char prefix_sec[128];
				memset(prefix, 0, sizeof(prefix));
				pu.p = &dhcps->dns;
				prefix_2_address_str (pu, prefix, sizeof(prefix));

				memset(prefix_sec, 0, sizeof(prefix_sec));
				pu.p = &dhcps->dns_secondary;
				prefix_2_address_str (pu, prefix_sec, sizeof(prefix_sec));

				strcat(prefix, ",");
				strcat(prefix, prefix_sec);

				dhcpd_pool_set_option(dhcps->pool, 6/*"routers"*/, prefix);

			}
		}
		ret = OK;
		break;
	case DHCPS_CMD_TFTP:
		memcpy(&dhcps->tftp, val, sizeof(struct prefix));
		if(dhcps->tftp.u.prefix4.s_addr)
		{
			memset(prefix, 0, sizeof(prefix));
			pu.p = &dhcps->tftp;
			prefix_2_address_str (pu, prefix, sizeof(prefix));

			dhcpd_pool_set_option(dhcps->pool, 66/*"routers"*/, prefix);
		}
		ret = OK;
		break;
	case DHCPS_CMD_LEASE:
		dhcps->lease_time = *((int*)val);

		dhcpd_pool_set_offer_time(dhcps->pool, dhcps->lease_time);

		ret = OK;
		break;
	case DHCPS_CMD_HOST:
		//lstAdd(dhcps_list.hostlist, (NODE *)node);
		break;
	case DHCPS_CMD_EXCUDED:
		//lstAdd(dhcps_list.excludedlist, (NODE *)node);
		break;
	default:
		break;
	}
	if(dhcps_list.mutex)
		os_mutex_unlock(dhcps_list.mutex);
	return ret;
}

int nsm_dhcps_unset_api(nsm_dhcps_t *dhcps, zpl_uint32 cmd)
{
	zpl_char prefix[128];
	union prefix46constptr pu;
	int ret = ERROR;
	if(dhcps_list.mutex)
		os_mutex_lock(dhcps_list.mutex, OS_WAIT_FOREVER);
	switch(cmd)
	{
	case DHCPS_CMD_NETWORK:
		memset(&dhcps->address, 0, sizeof(struct prefix));
		memset(&dhcps->start_address, 0, sizeof(struct prefix));
		memset(&dhcps->end_address, 0, sizeof(struct prefix));

		dhcpd_pool_set_address_range(dhcps->pool, 0, 0);

		ret = OK;
		break;
	case DHCPS_CMD_NETWORK_START:
	case DHCPS_CMD_NETWORK_END:
		//memset(&dhcps->address, 0, sizeof(struct prefix));
		memset(&dhcps->start_address, 0, sizeof(struct prefix));
		memset(&dhcps->end_address, 0, sizeof(struct prefix));

		dhcpd_pool_set_address_range(dhcps->pool, 0, 0);

		ret = OK;
		break;
	case DHCPS_CMD_GATEWAY:

		dhcpd_pool_set_option(dhcps->pool, 3/*"routers"*/, NULL);

		memset(&dhcps->gateway, 0, sizeof(struct prefix));
		ret = OK;
		if(dhcps->gateway_secondary.prefixlen)
			dhcps->ifp = if_lookup_prefix(&dhcps->gateway_secondary);
		else
			dhcps->ifp = NULL;
		break;
	case DHCPS_CMD_GATEWAY_SECONDARY:

		dhcpd_pool_set_option(dhcps->pool, 3/*"routers"*/, NULL);

		if(dhcps->netbios.u.prefix4.s_addr)
		{
			memset(prefix, 0, sizeof(prefix));
			pu.p = &dhcps->netbios;
			prefix_2_address_str (pu, prefix, sizeof(prefix));

			dhcpd_pool_set_option(dhcps->pool, 3/*"routers"*/, prefix);

		}
		memset(&dhcps->gateway_secondary, 0, sizeof(struct prefix));
		ret = OK;
		if(dhcps->gateway.prefixlen)
			dhcps->ifp = if_lookup_prefix(&dhcps->gateway);
		else
			dhcps->ifp = NULL;
		break;
	case DHCPS_CMD_DOMAIN_NAME:

		dhcpd_pool_set_option(dhcps->pool, 15/*"domain-name"*/, NULL);

		memset(dhcps->domain_name, 0, sizeof(dhcps->domain_name));
		ret = OK;
		break;
	case DHCPS_CMD_NETBIOS:

		dhcpd_pool_set_option(dhcps->pool, 44/*"netbios-name-servers"*/, NULL);

		memset(&dhcps->netbios, 0, sizeof(struct prefix));
		ret = OK;
		break;
	case DHCPS_CMD_NETBIOS_SECONDARY:

		dhcpd_pool_set_option(dhcps->pool, 44/*"netbios-name-servers"*/, NULL);

		if(dhcps->netbios.u.prefix4.s_addr)
		{
			memset(prefix, 0, sizeof(prefix));
			pu.p = &dhcps->netbios;
			prefix_2_address_str (pu, prefix, sizeof(prefix));

			dhcpd_pool_set_option(dhcps->pool, 44/*"netbios-name-servers"*/, prefix);

		}
		memset(&dhcps->netbios_secondary, 0, sizeof(struct prefix));
		ret = OK;
		break;
	case DHCPS_CMD_DNS:

		dhcpd_pool_set_option(dhcps->pool, 6/*"domain-name-servers"*/, NULL);

		memset(&dhcps->dns, 0, sizeof(struct prefix));
		ret = OK;
		break;
	case DHCPS_CMD_DNS_SECONDARY:

		dhcpd_pool_set_option(dhcps->pool, 6/*"domain-name-servers"*/, NULL);

		if(dhcps->dns.u.prefix4.s_addr)
		{
			memset(prefix, 0, sizeof(prefix));
			pu.p = &dhcps->dns;
			prefix_2_address_str (pu, prefix, sizeof(prefix));

			dhcpd_pool_set_option(dhcps->pool, 6/*"domain-name-servers"*/, prefix);

		}
		memset(&dhcps->dns_secondary, 0, sizeof(struct prefix));
		ret = OK;
		break;
	case DHCPS_CMD_TFTP:

		dhcpd_pool_set_option(dhcps->pool, 66/*"tftp-name-servers"*/, NULL);

		memset(&dhcps->tftp, 0, sizeof(struct prefix));
		ret = OK;
		break;
	case DHCPS_CMD_LEASE:

		dhcpd_pool_set_offer_time(dhcps->pool, DHCPS_LEASE_DEFAULT);

		dhcps->lease_time = DHCPS_LEASE_DEFAULT;
		ret = OK;
		break;
	case DHCPS_CMD_HOST:
		//lstAdd(dhcps_list.hostlist, (NODE *)node);
		break;
	case DHCPS_CMD_EXCUDED:
		//lstAdd(dhcps_list.excludedlist, (NODE *)node);
		break;
	default:
		break;
	}
	if(dhcps_list.mutex)
		os_mutex_unlock(dhcps_list.mutex);
	return ret;
}

int nsm_dhcps_get_api(nsm_dhcps_t *dhcps, zpl_uint32 cmd, void *val)
{
	int ret = ERROR;
	if(dhcps_list.mutex)
		os_mutex_lock(dhcps_list.mutex, OS_WAIT_FOREVER);
	switch(cmd)
	{
	case DHCPS_CMD_NETWORK:
		memcpy(val, &dhcps->address, sizeof(struct prefix));
		ret = OK;
		break;
	case DHCPS_CMD_NETWORK_START:
		memcpy(val, &dhcps->start_address, sizeof(struct prefix));
		ret = OK;
		break;
	case DHCPS_CMD_NETWORK_END:
		memcpy(val, &dhcps->end_address, sizeof(struct prefix));
		ret = OK;
		break;
	case DHCPS_CMD_GATEWAY:
		memcpy(val, &dhcps->gateway, sizeof(struct prefix));
		ret = OK;
		break;
	case DHCPS_CMD_GATEWAY_SECONDARY:
		memcpy(val, &dhcps->gateway_secondary, sizeof(struct prefix));
		ret = OK;
		break;
	case DHCPS_CMD_DOMAIN_NAME:
		strcpy(val, dhcps->domain_name);
		ret = OK;
		break;
	case DHCPS_CMD_NETBIOS:
		memcpy(val, &dhcps->netbios, sizeof(struct prefix));
		ret = OK;
		break;
	case DHCPS_CMD_NETBIOS_SECONDARY:
		memcpy(val, &dhcps->netbios_secondary, sizeof(struct prefix));
		ret = OK;
		break;
	case DHCPS_CMD_DNS:
		memcpy(val, &dhcps->dns, sizeof(struct prefix));
		ret = OK;
		break;
	case DHCPS_CMD_DNS_SECONDARY:
		memcpy(val, &dhcps->dns_secondary,  sizeof(struct prefix));
		ret = OK;
		break;
	case DHCPS_CMD_TFTP:
		memcpy(val, &dhcps->tftp, sizeof(struct prefix));
		ret = OK;
		break;
	case DHCPS_CMD_LEASE:
		*((int*)val) = dhcps->lease_time;
		ret = OK;
		break;
	case DHCPS_CMD_HOST:
		//lstAdd(dhcps_list.hostlist, (NODE *)node);
		break;
	case DHCPS_CMD_EXCUDED:
		//lstAdd(dhcps_list.excludedlist, (NODE *)node);
		break;
	default:
		break;
	}
	if(dhcps_list.mutex)
		os_mutex_unlock(dhcps_list.mutex);
	return ret;
}
#ifdef ZPL_SHELL_MODULE
int nsm_dhcps_write_config(struct vty *vty)
{
	zpl_char prefix[128];
	union prefix46constptr pu;
	NODE index;
	nsm_dhcps_t *pstNode = NULL;
	if(dhcps_list.mutex)
		os_mutex_lock(dhcps_list.mutex, OS_WAIT_FOREVER);

	for(pstNode = (nsm_dhcps_t *)lstFirst(dhcps_list.dhcpslist);
			pstNode != NULL;  pstNode = (nsm_dhcps_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		vty_out(vty, "ip dhcp pool %s%s", pstNode->name, VTY_NEWLINE);

		if(pstNode->address.prefixlen)
		{
			memset(prefix, 0, sizeof(prefix));
			pu.p = &pstNode->address;
			prefix2str (pu, prefix, sizeof(prefix));
			vty_out(vty, " network %s%s", prefix, VTY_NEWLINE);
		}
		//else
		{
			if(pstNode->start_address.prefixlen)
			{
				//struct ipstack_in_addr netmask;
				memset(prefix, 0, sizeof(prefix));
				pu.p = &pstNode->start_address;
				prefix_2_address_str (pu, prefix, sizeof(prefix));
				vty_out(vty, " address range %s", prefix);

				memset(prefix, 0, sizeof(prefix));
				pu.p = &pstNode->end_address;
				prefix_2_address_str (pu, prefix, sizeof(prefix));
				vty_out(vty, " %s%s", prefix, VTY_NEWLINE);

				//masklen2ip(pstNode->start_address.prefixlen, &netmask);

				//vty_out(vty, " %s%s", ipstack_inet_ntoa(netmask), VTY_NEWLINE);

			}
		}
		if(pstNode->gateway.prefixlen)
		{
			memset(prefix, 0, sizeof(prefix));
			pu.p = &pstNode->gateway;
			prefix_2_address_str (pu, prefix, sizeof(prefix));
			vty_out(vty, " default-router %s%s", prefix, VTY_NEWLINE);
		}
		if(pstNode->gateway_secondary.prefixlen)
		{
			memset(prefix, 0, sizeof(prefix));
			pu.p = &pstNode->gateway_secondary;
			prefix_2_address_str (pu, prefix, sizeof(prefix));
			vty_out(vty, " default-router %s secondary%s", prefix, VTY_NEWLINE);
		}
		if(pstNode->dns.prefixlen)
		{
			memset(prefix, 0, sizeof(prefix));
			pu.p = &pstNode->dns;
			prefix_2_address_str (pu, prefix, sizeof(prefix));
			vty_out(vty, " dns-server %s%s", prefix, VTY_NEWLINE);
		}
		if(pstNode->dns_secondary.prefixlen)
		{
			memset(prefix, 0, sizeof(prefix));
			pu.p = &pstNode->dns_secondary;
			prefix_2_address_str (pu, prefix, sizeof(prefix));
			vty_out(vty, " dns-server %s secondary%s", prefix, VTY_NEWLINE);
		}
		if(pstNode->netbios.prefixlen)
		{
			memset(prefix, 0, sizeof(prefix));
			pu.p = &pstNode->netbios;
			prefix_2_address_str (pu, prefix, sizeof(prefix));
			vty_out(vty, " netbios-name-server %s%s", prefix, VTY_NEWLINE);
		}
		if(pstNode->netbios_secondary.prefixlen)
		{
			memset(prefix, 0, sizeof(prefix));
			pu.p = &pstNode->netbios_secondary;
			prefix_2_address_str (pu, prefix, sizeof(prefix));
			vty_out(vty, " netbios-name-server %s secondary%s", prefix, VTY_NEWLINE);
		}
		if(pstNode->tftp.prefixlen)
		{
			memset(prefix, 0, sizeof(prefix));
			pu.p = &pstNode->tftp;
			prefix_2_address_str (pu, prefix, sizeof(prefix));
			vty_out(vty, " tftp-server %s%s", prefix, VTY_NEWLINE);
		}
		if(os_strlen(pstNode->domain_name))
		{
			vty_out(vty, " domain-name %s%s", pstNode->domain_name, VTY_NEWLINE);
		}
		if(pstNode->lease_time != DHCPS_LEASE_DEFAULT)
		{
			vty_out(vty, " lease %d%s", pstNode->lease_time, VTY_NEWLINE);
		}
		vty_out(vty, "!%s", VTY_NEWLINE);
	}
	if(dhcps_list.mutex)
		os_mutex_unlock(dhcps_list.mutex);
	return 1;
}
#endif

int nsm_interface_dhcps_enable(nsm_dhcps_t *pool, ifindex_t kifindex, zpl_bool enable)
{
	if(enable)
		return dhcpd_pool_add_interface(pool->pool, kifindex);
	else
		return dhcpd_pool_del_interface(pool->pool, kifindex);
}
int nsm_dhcps_lease_foreach(int(*cb)(void *, void *), void *p)
{

	return dhcpd_lease_foreach(cb, p);
}
#ifdef ZPL_SHELL_MODULE
int nsm_dhcps_lease_show(struct vty *vty, struct interface *ifp, zpl_char *poolname, zpl_bool detail)
{
	return dhcp_lease_show(vty, poolname, ifp ? ifp->ifindex : 0,  detail);
}

int nsm_dhcps_pool_show(struct vty *vty, zpl_bool detail)
{
	return dhcp_pool_show(vty, detail);
}
#endif
#endif
