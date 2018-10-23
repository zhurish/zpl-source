/*
 * dhcpd_api.c
 *
 *  Created on: Sep 22, 2018
 *      Author: zhurish
 */


#define _GRP_H
#include "zebra.h"
#include "prefix.h"
#include "if.h"

#include "dhcp.h"
#include "tree.h"
#include "dhcpd.h"
#include "sync.h"
#include "dhcpd_api.h"

/*
struct dhcpd_config
{
	struct group *group;
	struct class *class;
	struct shared_network *share;
	struct subnet *subnet;
	struct host_decl *host;
};

struct dhcpd_config gdhcpd_config;
*/

const char *itoa(int value, int base)
{
	static char buf[128];
	memset(buf, 0, sizeof(buf));
	if(base == 0 || base == 10)
		sprintf("%d", value);
	else if(base == 16)
		sprintf("%x", value);
	return buf;
}


int dhcpd_service_is_enable(void)
{
	return 0;
}

void * dhcpd_pool_add_api(char *name)
{
	struct shared_network *share = dhcpd_shared_network_decl_create(&root_group, name);
	if(share)
		return share;
	return NULL;
}

int dhcpd_pool_del_api(char *name)
{
	int ret = 0;
	ret = dhcpd_shared_network_decl_destroy(name);
	if(ret == 0)
		return OK;
	return ERROR;
}



void * dhcpd_pool_subnet_add_api(void *pool, struct prefix subnet)
{
	struct shared_network *share = (struct shared_network *)pool;
	if(share)
	{
		struct subnet *gsubnet = NULL;
		struct in_addr mask;
		struct iaddr net, netmask;
		memset(&net, 0, sizeof(net));
		memset(&netmask, 0, sizeof(netmask));
		masklen2ip (subnet.prefixlen, &mask);
		mask.s_addr = (mask.s_addr);
		memcpy(netmask.iabuf, &mask.s_addr, 4);
		mask.s_addr = (subnet.u.prefix4.s_addr);
		memcpy(net.iabuf, &mask.s_addr, 4);
		net.len = 4;
		netmask.len = 4;
		gsubnet = dhcpd_subnet_decl_create(share, &net, &netmask);
		if(gsubnet)
		{
			dhcpd_interface_refresh();

			//dhcp-parameter-request-list
			//dhcpd_pool_option_add_api(gsubnet, 55, "1,3,6,12,15,28,42");
			//unsigned char optionkey[] = {1, 3, 6, 12, 15, 28, 42};
			//dhcpd_option55_set(gsubnet->group, optionkey, sizeof(optionkey));
			return gsubnet;
		}
	}
	return NULL;
}


int dhcpd_pool_address_range_add_api(void *subnet, struct prefix startp, struct prefix endp)
{
	if(subnet)
	{
		int ret = 0;
		struct in_addr address;
		struct iaddr start, end;
		memset(&start, 0, sizeof(start));
		memset(&end, 0, sizeof(end));

		address.s_addr = (startp.u.prefix4.s_addr);
		memcpy(start.iabuf, &address.s_addr, 4);

		address.s_addr = (endp.u.prefix4.s_addr);
		memcpy(end.iabuf, &address.s_addr, 4);

		zlog_debug(ZLOG_NSM, "start : %d.%d.%d.%d - %d.%d.%d.%d",
				start.iabuf[0],start.iabuf[1],start.iabuf[2],start.iabuf[3],
				end.iabuf[0],end.iabuf[1],end.iabuf[2],end.iabuf[3]);
		start.len = 4;
		end.len = 4;
		ret = dhcpd_subnet_address_range(subnet, &start, &end, 0);
		if(ret == 0)
		{
			dhcpd_interface_refresh();
			return OK;
		}
	}
	return ERROR;
}


int dhcpd_pool_option_add_api(void *subnet, int option, char *value)
{
	struct subnet *gsubnet = (struct subnet *)subnet;
	int ret = ERROR;
	if(option < 256 && option >= 0)
	{
		char *opt = dhcpd_option_to_name(option);
		if(opt && !strstr(opt, "unknown"))
			ret = dhcpd_option_set(gsubnet->group, opt, value);
	}
	return ret;
}

int dhcpd_pool_option_del_api(void *subnet, int option)
{
	struct subnet *gsubnet = (struct subnet *)subnet;
	int ret = ERROR;
	if(option < 256 && option >= 0)
	{
		char *opt = dhcpd_option_to_name(option);
		if(opt && !strstr(opt, "unknown"))
			ret = dhcpd_option_unset(gsubnet->group, opt);
	}
	return ret;
}

int dhcpd_bootp_lease_set_api(void *subnet, int value)
{
	int ret = ERROR;
	struct subnet *gsubnet = (struct subnet *)subnet;
	if(gsubnet)
	{
		ret = dhcpd_bootp_lease_set(gsubnet->group, value);
	}
	return ret;
}

int dhcpd_max_lease_set_api(void *subnet, int value)
{
	int ret = ERROR;
	struct subnet *gsubnet = (struct subnet *)subnet;
	if(gsubnet)
	{
		ret = dhcpd_max_lease_set(gsubnet->group, value);
		dhcpd_pool_option_add_api(gsubnet, 51, itoa(value, 0));
	}
	return ret;
}

int dhcpd_default_lease_set_api(void *subnet, int value)
{
	int ret = ERROR;
	struct subnet *gsubnet = (struct subnet *)subnet;
	if(gsubnet)
	{
		ret = dhcpd_max_lease_set(gsubnet->group, value);
	}
	return ret;
}

int dhcpd_authoritative_set_api(void *subnet, BOOL value)
{
	int ret = ERROR;
	struct subnet *gsubnet = (struct subnet *)subnet;
	if(gsubnet)
	{
		ret = dhcpd_authoritative_set(gsubnet->group, value);
	}
	return ret;
}

int dhcpd_allow_deny_set_api(void *subnet, int cmd, BOOL value)
{
	int ret = ERROR;
	struct subnet *gsubnet = (struct subnet *)subnet;
	if(gsubnet)
	{
		ret = dhcpd_allow_deny_set(gsubnet->group, cmd, value);
	}
	return ret;
}


int dhcpd_server_name_set_api(void *subnet, char *servername)
{
	int ret = ERROR;
	struct subnet *gsubnet = (struct subnet *)subnet;
	if(gsubnet)
	{
		ret = dhcpd_server_name_set(gsubnet->group, servername);
		//dhcpd_pool_option_add_api(gsubnet, 51, servername);
	}
	return ret;
}

int dhcpd_filename_set_api(void *subnet, char *filename)
{
	int ret = ERROR;
	struct subnet *gsubnet = (struct subnet *)subnet;
	if(gsubnet)
	{
		ret = dhcpd_filename_set(gsubnet->group, filename);
	}
	return ret;
}

int dhcpd_server_identifier_set_api(void *subnet, char *hostname_or_address)
{
	int ret = ERROR;
	struct subnet *gsubnet = (struct subnet *)subnet;
	if(gsubnet)
	{
		ret = dhcpd_server_identifier_set(gsubnet->group, hostname_or_address);
		dhcpd_pool_option_add_api(gsubnet, 54, hostname_or_address);
	}
	return ret;
}

int dhcpd_next_server_set_api(void *subnet, char *hostname_or_address)
{
	int ret = ERROR;
	struct subnet *gsubnet = (struct subnet *)subnet;
	if(gsubnet)
	{
		ret = dhcpd_next_server_set(gsubnet->group, hostname_or_address);
	}
	return ret;
}


/*
 * interface
 */

struct interface  * dhcpd_interface_lookup_api(int ifindex)
{
	struct interface_info *iface = dhcpd_interface_lookup(ifindex);
	if(iface)
		return iface->ifp;
	return NULL;
}

int dhcpd_interface_add_api(int ifindex)
{
	int ret = dhcpd_interface_add(ifindex);
	if(ret != -1)
		return OK;
	return ERROR;
}

int dhcpd_interface_del_api(int ifindex)
{
	int ret = dhcpd_interface_del(ifindex);
	if(ret == 0)
		return OK;
	return ERROR;
}





void
pfmsg(char c, struct lease *lp)
{
	return;
}
