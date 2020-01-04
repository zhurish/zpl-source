/*
 * dhcp_lease.c
 *
 *  Created on: Apr 21, 2019
 *      Author: zhurish
 */
#include "zebra.h"
#include "if.h"
#include "memory.h"
#include "command.h"
#include "prefix.h"
#include "log.h"
#include "eloop.h"
#include "vty.h"

#include "dhcp_def.h"
#include "dhcpd.h"
#include "dhcp_lease.h"



//CONFIG_DHCPD_LEASES_FILE

dyn_lease_t * dhcp_lease_lookup_by_lease_address(LIST *lst, lease_mode_t mode, u_int32 lease_address)
{
	NODE index;
	dyn_lease_t *pstNode = NULL;
	if (!lstCount(lst))
		return NULL;
	for (pstNode = (dyn_lease_t *)lstFirst(lst);
		pstNode != NULL;  pstNode = (dyn_lease_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if ((pstNode->lease_address == lease_address) && (pstNode->mode == mode))
		{
			return pstNode;
		}
	}
	return NULL;
}


dyn_lease_t * dhcp_lease_lookup_by_lease_mac(LIST *lst, lease_mode_t mode, u_int8 *lease_mac)
{
	NODE index;
	dyn_lease_t *pstNode = NULL;
	if (!lstCount(lst))
		return NULL;
	for (pstNode = (dyn_lease_t *)lstFirst(lst);
		pstNode != NULL;  pstNode = (dyn_lease_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if ( (memcmp(pstNode->lease_mac, lease_mac, ETHER_ADDR_LEN) == 0) && (pstNode->mode == mode) )
		{
			return pstNode;
		}
	}
	return NULL;
}


dyn_lease_t * dhcp_lease_add(LIST *lst, dyn_lease_t *lease)
{
	dyn_lease_t *pstNode = NULL;
	u_int8 chaddr[ETHER_ADDR_LEN] = { 0, 0, 0, 0, 0, 0 };
	if ((memcmp(lease->lease_mac, chaddr, ETHER_ADDR_LEN) != 0))
	{
		pstNode = dhcp_lease_lookup_by_lease_mac(lst, lease->mode, lease->lease_mac);
	}
	else if (lease->lease_address)
	{
		pstNode = dhcp_lease_lookup_by_lease_address(lst, lease->mode, lease->lease_address);
	}
/*	else
	{
		return NULL;
	}*/
	if (pstNode)
	{
		NODE	node = pstNode->node;
		memcpy(pstNode, lease, sizeof(dyn_lease_t));
		pstNode->node = node;
		return pstNode;
	}
	pstNode = XMALLOC(MTYPE_DHCPS_ADDR, sizeof(dyn_lease_t));
	if (pstNode)
	{
		memcpy(pstNode, lease, sizeof(dyn_lease_t));
		//pstNode->node
		lstAdd(lst, pstNode);
		return pstNode;
	}
	return NULL;
}

int dhcp_lease_update(LIST *lst, dyn_lease_t *lease)
{
	dyn_lease_t *pstNode = dhcp_lease_lookup_by_lease_mac(lst, lease->mode, lease->lease_mac);
	if (pstNode)
	{
		NODE	node = pstNode->node;
		memcpy(pstNode, lease, sizeof(dyn_lease_t));
		pstNode->node = node;
		return OK;
	}
	return ERROR;
}

int dhcp_lease_del_address(LIST *lst, u_int32 lease_address)
{
	dyn_lease_t *pstNode = dhcp_lease_lookup_by_lease_address(lst, LEASE_DYNAMIC, lease_address);
	if (pstNode)
	{
		lstDelete(lst, pstNode);
		XFREE(MTYPE_DHCPS_ADDR, pstNode);
		return OK;
	}
	return ERROR;
}

int dhcp_lease_del_mac(LIST *lst, u_int8 *lease_mac)
{
	dyn_lease_t *pstNode = dhcp_lease_lookup_by_lease_mac(lst, LEASE_DYNAMIC, lease_mac);
	if (pstNode)
	{
		lstDelete(lst, pstNode);
		XFREE(MTYPE_DHCPS_ADDR, pstNode);
		return OK;
	}
	return ERROR;
}

int dhcp_lease_del(LIST *lst, dyn_lease_t *pstNode)
{
	//dyn_lease_t *pstNode = dhcp_lease_lookup_by_lease_mac(lst, LEASE_DYNAMIC, lease->lease_mac);
	if (pstNode)
	{
		lstDelete(lst, pstNode);
		XFREE(MTYPE_DHCPS_ADDR, pstNode);
		return OK;
	}
	return ERROR;
}

int dhcp_lease_clean(LIST *lst)
{
	dyn_lease_t *pstNode = NULL;
	NODE index;
	if (!lstCount(lst))
		return NULL;
	for (pstNode = (dyn_lease_t *) lstFirst(lst);
			pstNode != NULL;
			pstNode = (dyn_lease_t *) lstNext((NODE*) &index))
	{
		index = pstNode->node;
		if (pstNode)
		{
			lstDelete(lst, (NODE*)pstNode);
			XFREE(MTYPE_DHCPS_ADDR, pstNode);
		}
	}
	return OK;
}

dyn_lease_t *dhcp_lease_lookup_expired_lease(LIST *lst)
{
	NODE index;
	dyn_lease_t *pstNode = NULL;
	leasetime_t oldest_time = time(NULL);
	if (!lstCount(lst))
		return NULL;
	for (pstNode = (dyn_lease_t *)lstFirst(lst);
		pstNode != NULL;  pstNode = (dyn_lease_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if ((pstNode->ends < oldest_time) && (pstNode->mode == LEASE_DYNAMIC))
		{
			return pstNode;
		}
	}
	return NULL;
}

static int dhcp_lease_foreach_one(dhcp_pool_t *config, int(*cb)(dyn_lease_t *, void *p), void *p)
{
	NODE index;
	dyn_lease_t *pstNode = NULL;
	//leasetime_t oldest_time = time(NULL);
	if (!lstCount(&config->dhcp_lease_list))
		return OK;
	for (pstNode = (dyn_lease_t *)lstFirst(&config->dhcp_lease_list);
		pstNode != NULL;  pstNode = (dyn_lease_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode)
		{
			if(cb)
				(cb)(pstNode, p);
		}
/*		if ((pstNode->ends > oldest_time) && (pstNode->mode == LEASE_DYNAMIC))
		{
		}*/
	}
	return OK;
}

int dhcpd_lease_foreach(int(*cb)(dyn_lease_t *, void *p), void *p)
{
	//FILE *fp = NULL;
	NODE index;
	dhcp_pool_t *pstNode = NULL;
	if (!lstCount(&dhcp_global_config.pool_list))
		return OK;

	for (pstNode = (dhcp_pool_t *)lstFirst(&dhcp_global_config.pool_list);
			pstNode != NULL;  pstNode = (dhcp_pool_t *)lstNext((NODE*)&index))
		{
			index = pstNode->node;
			if (pstNode)
			{
				dhcp_lease_foreach_one(pstNode, cb, p);
				//dhcp_lease_save_one(fp, pstNode);
			}
		}
		return OK;
}



static int dhcp_lease_save_one(FILE *fp, dhcp_pool_t *config)
{
	NODE index;
	dyn_lease_t *pstNode = NULL;
	leasetime_t oldest_time = time(NULL);
	if (!lstCount(&config->dhcp_lease_list))
		return OK;
	for (pstNode = (dyn_lease_t *)lstFirst(&config->dhcp_lease_list);
		pstNode != NULL;  pstNode = (dyn_lease_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if ((pstNode->ends > oldest_time) && (pstNode->mode == LEASE_DYNAMIC))
		{
			fwrite(pstNode, sizeof(dyn_lease_t), 1, fp);
			fflush(fp);
		}
	}
	return OK;
}

int dhcpd_lease_save()
{
	FILE *fp = NULL;
	NODE index;
	dhcp_pool_t *pstNode = NULL;
	if (!lstCount(&dhcp_global_config.pool_list))
		return OK;

	fp = fopen(CONFIG_DHCPD_LEASES_FILE".tm", "w");
	if(fp)
	{
		for (pstNode = (dhcp_pool_t *)lstFirst(&dhcp_global_config.pool_list);
			pstNode != NULL;  pstNode = (dhcp_pool_t *)lstNext((NODE*)&index))
		{
			index = pstNode->node;
			if (pstNode)
			{
				dhcp_lease_save_one(fp, pstNode);
			}
		}
		fflush(fp);
		fclose(fp);
		rename(CONFIG_DHCPD_LEASES_FILE".tm", CONFIG_DHCPD_LEASES_FILE);
		sync();
		return OK;
	}
	return OK;
}


int dhcpd_lease_load()
{
	FILE *fp = NULL;
	int len = 0;
	dyn_lease_t pstNode;
	dhcp_pool_t * pool = NULL;
	fp = fopen(CONFIG_DHCPD_LEASES_FILE, "r");
	if(fp)
	{
		while(1)
		{
			len = fread(&pstNode, sizeof(dyn_lease_t), 1, fp);
			if(len == sizeof(dyn_lease_t))
			{
				pool = dhcpd_pool_lookup_by_poolid(pstNode.poolid);
				if(pool)
					dhcp_lease_add(&pool->dhcp_lease_list, &pstNode);
			}
			else
				break;
		}
		fclose(fp);
		return OK;
	}
	return OK;
}

static int dhcp_lease_show_one(struct vty *vty, dyn_lease_t *lease, BOOL detail)
{
	if(vty && lease)
	{
		char *lease_type[3] = {"Unknow", "dynamic", "static"};
		vty_out(vty, " MAC                  : %s%s", inet_ethernet(lease->lease_mac), VTY_NEWLINE);
		vty_out(vty, "  type                : %s%s", lease_type[lease->mode], VTY_NEWLINE);
		vty_out(vty, "  address             : %s%s", inet_address(ntohl(lease->lease_address)), VTY_NEWLINE);
		vty_out(vty, "  netmask             : %s%s", inet_address(ntohl(lease->lease_netmask)), VTY_NEWLINE);
		if(lease->lease_gateway)
			vty_out(vty, "  gateway             : %s %s%s", inet_address(ntohl(lease->lease_gateway)),
				lease->lease_gateway2 ? inet_address(ntohl(lease->lease_gateway2)):" ",VTY_NEWLINE);
		if(lease->lease_dns1)
			vty_out(vty, "  DNS server          : %s %s%s", inet_address(ntohl(lease->lease_dns1)),
				lease->lease_dns2 ? inet_address(ntohl(lease->lease_dns2)):" ",VTY_NEWLINE);

		if(strlen(lease->hostname))
			vty_out(vty, "  hostname            : %s%s", lease->hostname, VTY_NEWLINE);
		if(detail)
		{
			if(strlen(lease->vendor))
				vty_out(vty, "  vendor              : %s%s", lease->vendor, VTY_NEWLINE);
			if(strlen(lease->client_id))
				vty_out(vty, "  client id           : %s%s", lease->client_id, VTY_NEWLINE);
		}
		vty_out(vty, "  expires             : %s%s", os_time_string(lease->expires), VTY_NEWLINE);
		vty_out(vty, "  interface           : %s%s", ifindex2ifname(lease->ifindex), VTY_NEWLINE);
		if(detail)
			vty_out(vty, "  dhcp pool           : %s%s", dhcpd_pool_poolid2name(lease->poolid), VTY_NEWLINE);
	}
	return OK;
}


static int dhcp_lease_show_one_in_pool(struct vty *vty, dhcp_pool_t *config, ifindex_t ifindex, BOOL detail)
{
	NODE index;
	dyn_lease_t *pstNode = NULL;
	if (!lstCount(&config->dhcp_lease_list))
		return OK;
	for (pstNode = (dyn_lease_t *)lstFirst(&config->dhcp_lease_list);
		pstNode != NULL;  pstNode = (dyn_lease_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(ifindex && ifindex == pstNode->ifindex)
			dhcp_lease_show_one(vty, pstNode, detail);
		else
			dhcp_lease_show_one(vty, pstNode, detail);
		vty_out(vty, "%s", VTY_NEWLINE);
	}
	return OK;
}

int dhcp_lease_show(struct vty *vty, char *poolname, ifindex_t ifindex, BOOL detail)
{
	NODE index;
	dhcp_pool_t *pstNode = NULL;
	if (!lstCount(&dhcp_global_config.pool_list))
		return OK;
	if(poolname == NULL)
	{
		for (pstNode = (dhcp_pool_t *) lstFirst(&dhcp_global_config.pool_list);
				pstNode != NULL;
				pstNode = (dhcp_pool_t *) lstNext((NODE*) &index)) {
			index = pstNode->node;
			if (pstNode) {
				dhcp_lease_show_one_in_pool(vty, pstNode, ifindex, detail);
				vty_out(vty, "%s", VTY_NEWLINE);
			}
		}
	}
	else
	{
		pstNode = dhcpd_pool_lookup(poolname);
		if(pstNode)
		{
			dhcp_lease_show_one_in_pool(vty, pstNode, ifindex, detail);
			vty_out(vty, "%s", VTY_NEWLINE);
		}
	}
	return OK;
}
/*
static void read_leases(dhcp_pool_t *config, const char *file) {

}
*/
