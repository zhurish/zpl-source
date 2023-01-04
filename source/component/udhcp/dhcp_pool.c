/*
 * dhcp_pool.c
 *
 *  Created on: Apr 21, 2019
 *      Author: zhurish
 */
#include "auto_include.h"
#include <zplos_include.h>
#include "dhcp_def.h"
#include "dhcpd.h"
#include "dhcp_pool.h"
#include "dhcp_lease.h"

dhcp_pool_t * dhcpd_pool_lookup(char *name)
{
	NODE index;
	char poolname[DHCPD_POOLNAME_MAX];
	dhcp_pool_t *pstNode = NULL;
	if (!lstCount(&dhcp_global_config.pool_list))
		return NULL;
	memset(poolname, 0, sizeof(poolname));
	strcpy(poolname, name);
	for (pstNode = (dhcp_pool_t *)lstFirst(&dhcp_global_config.pool_list);
		pstNode != NULL;  pstNode = (dhcp_pool_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if (memcmp(pstNode->poolname, poolname, sizeof(poolname)) == 0)
		{
			return pstNode;
		}
	}
	return NULL;
}

dhcp_pool_t * dhcpd_pool_lookup_by_poolid(zpl_uint32 poolid)
{
	NODE index;
	dhcp_pool_t *pstNode = NULL;
	if (!lstCount(&dhcp_global_config.pool_list))
		return NULL;
	for (pstNode = (dhcp_pool_t *)lstFirst(&dhcp_global_config.pool_list);
		pstNode != NULL;  pstNode = (dhcp_pool_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if (pstNode->poolid == poolid)
		{
			return pstNode;
		}
	}
	return NULL;
}

dhcp_pool_t * dhcpd_pool_create(char *name)
{
	dhcp_pool_t *pool = NULL;
	if (dhcpd_pool_lookup(name))
		return NULL;
	pool = XMALLOC(MTYPE_DHCPS_POOL, sizeof(dhcp_pool_t));
	if (pool)
	{
		memset(pool, 0, sizeof(dhcp_pool_t));
		strcpy(pool->poolname, name);
		pool->global = &dhcp_global_config;
		pool->poolid = if_name_hash_make(pool->poolname);
		lstInit(&pool->interf);
		lstInit(&pool->dhcp_lease_list);
		lstAdd(&dhcp_global_config.pool_list, pool);
		return pool;
	}
	return NULL;
}


int dhcpd_pool_del(char *name)
{
	dhcp_pool_t * ifter = dhcpd_pool_lookup(name);
	if (ifter)
	{
		lstDelete(&dhcp_global_config.pool_list, ifter);
		//XFREE(MTYPE_DHCPS_ADDR, ifter->g_leases);
		XFREE(MTYPE_DHCPS_POOL, ifter);
		return OK;
	}
	return ERROR;
}

int dhcpd_pool_clean(void)
{
	dhcp_pool_t *pstNode = NULL;
	NODE index;
	if (!lstCount(&dhcp_global_config.pool_list))
		return OK;
	for (pstNode = (dhcp_pool_t *) lstFirst(&dhcp_global_config.pool_list);
			pstNode != NULL;
			pstNode = (dhcp_pool_t *) lstNext((NODE*) &index))
	{
		index = pstNode->node;
		if (pstNode)
		{
			dhcpd_interface_clean(pstNode);
			dhcp_lease_clean(&pstNode->dhcp_lease_list);
			lstDelete(&dhcp_global_config.pool_list, (NODE*)pstNode);
			XFREE(MTYPE_DHCPS_POOL, pstNode);
		}
	}
	return OK;
}

dhcp_pool_t * dhcpd_pool_interface_lookup(zpl_uint32 ifindex)
{
	NODE index;
	dhcp_pool_t *pstNode = NULL;
	if (!lstCount(&dhcp_global_config.pool_list))
		return NULL;
	for (pstNode = (dhcp_pool_t *)lstFirst(&dhcp_global_config.pool_list);
		pstNode != NULL;  pstNode = (dhcp_pool_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if (dhcpd_lookup_interface(pstNode, ifindex))
			return pstNode;
	}
	return NULL;
}

char * dhcpd_pool_poolid2name(zpl_uint32 poolid)
{
	dhcp_pool_t * pool = dhcpd_pool_lookup_by_poolid(poolid);
	if(pool)
		return pool->poolname;
	return "Unknow";
}
/************************************************************************************/

/************************************************************************************/
/************************************************************************************/
int dhcpd_pool_set_address_range(dhcp_pool_t *config, zpl_uint32  start, zpl_uint32  end)
{
	if(start && end)
	{
/*		udhcp_str2nip(start, &config->start_ip);
		udhcp_str2nip(end, &config->end_ip);*/
		config->start_ip = ntohl(start);
		config->end_ip = ntohl(end);
		config->max_leases = config->end_ip - config->start_ip + 1;
	}
	else
	{
		config->start_ip = 0;
		config->end_ip = 0;
		config->max_leases = 0;
	}
	return OK;
}

int dhcpd_pool_set_leases(dhcp_pool_t *config, zpl_uint32 max_lease_sec, zpl_uint32 min_lease_sec)
{
	config->max_lease_sec = max_lease_sec;
	config->min_lease_sec = min_lease_sec;
	return OK;
}

int dhcpd_pool_set_autotime(dhcp_pool_t *config, zpl_uint32 autotime) {
	config->auto_time = autotime;
	return OK;
}

int dhcpd_pool_set_decline_time(dhcp_pool_t *config, zpl_uint32 decline) {
	config->decline_time = decline;
	return OK;
}

int dhcpd_pool_set_conflict_time(dhcp_pool_t *config, zpl_uint32 conflict) {
	config->conflict_time = conflict;
	return OK;
}

int dhcpd_pool_set_offer_time(dhcp_pool_t *config, zpl_uint32 offer) {
	config->offer_time = offer;
	return OK;
}

int dhcpd_pool_set_siaddr(dhcp_pool_t *config, zpl_uint32 saddr) {
	config->siaddr_nip = ntohl(saddr);
	return OK;
}

int dhcpd_pool_set_notify_file(dhcp_pool_t *config, char *str)
{
	if(config->notify_file)
	{
		free(config->notify_file);
		config->notify_file = NULL;
	}
	if(str)
	{
		config->notify_file = strdup(str);
	}
	return OK;
}

int dhcpd_pool_set_sname(dhcp_pool_t *config, char *str) {
	if(config->boot_server_name)
	{
		free(config->boot_server_name);
		config->boot_server_name = NULL;
	}
	if(str)
	{
		config->boot_server_name = strdup(str);
	}
	return OK;
}

int dhcpd_pool_set_boot_file(dhcp_pool_t *config, char *str) {
	if(config->boot_file)
	{
		free(config->boot_file);
		config->boot_file = NULL;
	}
	if(str)
	{
		config->boot_file = strdup(str);
	}
	return OK;
}
#if 0
static struct option_set*  udhcp_find_option_prev(struct option_set *opt_list, zpl_uint8 code)
{
	struct option_set* next = opt_list;
	struct option_set* prev = opt_list;
	if (next && next->data && next->data[OPT_CODE] == code)
		return prev;
	while (next)
	{
		prev = next;
		next = next->next;

		if (next && next->data && next->data[OPT_CODE] == code)
			return prev;
	}
	return NULL;
}
#endif

int dhcpd_pool_set_option(dhcp_pool_t *config, zpl_uint8 code, char *str)
{
#if 1
	if(str)
	{
		if(config->gateway == 0 && code == DHCP_OPTION_ROUTER)
		{
			config->gateway = ipstack_inet_addr(str);
		}
		return dhcp_option_add(config->options,  code, str, strlen(str));
	}
		//return dhcp_option_string_set(config->options,  code, str);
	if(config->gateway && code == DHCP_OPTION_ROUTER)
	{
		config->gateway = 0;
	}
	return dhcp_option_del(config->options,  code);
#else
	if(str)
		read_optset(code, str, &config->options);
	else
	{
		struct option_set* next = NULL;
		struct option_set* prev = udhcp_find_option_prev(config->options,  code);
		if(prev)
		{
			next = prev->next;
			struct option_set* opt = next;
			if(opt)
				prev->next = opt->next;
			free(opt);
			return OK;
		}
		return ERROR;
	}
	return OK;
#endif
}