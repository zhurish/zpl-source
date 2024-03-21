/**
 * @file      : web_html_dhcp.c
 * @brief     : Description
 * @author    : zhurish (zhurish@163.com)
 * @version   : 1.0
 * @date      : 2024-03-02
 *
 * @copyright : Copyright (c) - 2024 zhurish(zhurish@163.com).Co.Ltd. All rights reserved.
 *
 */
#define HAS_BOOL 1
#include "goahead.h"
#include "webutil.h"
#include "prefix.h"
#include "web_api.h"
#include "web_app.h"
#include "service.h"

#ifdef ZPL_NSM_DHCP
#include "nsm_dhcp.h"
#ifdef ZPL_DHCPS_MODULE
#include "dhcp_config.h"
#include "dhcpd.h"
#include "dhcp_api.h"
#include "dhcp_lease.h"
#endif
#endif

#ifdef ZPL_DHCPS_MODULE
static int web_dhcps_lease_get_one_api(dyn_lease_t *lease, void *val)
{
	cJSON *array = val;
	cJSON *obj = cJSON_CreateObject();
	if (obj && array && lease)
	{
		char prefixbufstr[64];
		struct ipstack_in_addr inaddr;
		struct prefix p;
		union prefix46constptr prefixptr;
		inaddr.s_addr = lease->lease_netmask;

		p.family = IPSTACK_AF_INET;
		p.prefixlen = ip_masklen(inaddr);
		p.u.prefix4.s_addr = lease->lease_address;

		prefixptr.p = &p;

		//'address', 'MAC', 'gateway', 'dns', 'tftp', 'leases', 'vendor'
		cJSON_AddStringToObject(obj, "address", prefix2str (prefixptr, prefixbufstr, sizeof(prefixbufstr)));
		cJSON_AddStringToObject(obj, "MAC", inet_ethernet(lease->lease_mac));
		cJSON_AddStringToObject(obj, "gateway", inet_address(lease->lease_gateway));
		cJSON_AddStringToObject(obj, "gateway2", inet_address(lease->lease_gateway2));
		cJSON_AddStringToObject(obj, "dns", inet_address(lease->lease_dns1));
		cJSON_AddStringToObject(obj, "dns2", inet_address(lease->lease_dns2));
		cJSON_AddStringToObject(obj, "tftp", inet_address(lease->lease_tftp));
		cJSON_AddNumberToObject(obj, "leases", lease->expires);
		cJSON_AddStringToObject(obj, "vendor", lease->vendor);
		cJSON_AddStringToObject(obj, "clientid", lease->client_id);
		cJSON_AddStringToObject(obj, "hostname", lease->hostname);

		if(lease->mode==LEASE_STATIC)
			cJSON_AddStringToObject(obj, "mode", "static");
		else
			cJSON_AddStringToObject(obj, "mode", "dynamic");	

		cJSON_AddStringToObject(obj, "interface", ifindex2ifname(lease->ifindex));

		cJSON_AddItemToArray(array, obj);
	}
	return 0;
}

static int web_dhcps_get_one_api(nsm_dhcps_t *dhcps, void *val)
{
	dhcp_pool_t *pool;
	cJSON *array = val;
	cJSON *obj = cJSON_CreateObject();
	pool = dhcps->pool;
	if (obj && array && pool)
	{
		//{ poolname: 'guest', start: '1.1.1.1', end: '2.2.2.2',
		// gateway: '1.1.1.1', dns: '1.1.1.1', tftp: '2.2.2.2', leases: 12, total: 112, client: 4 }
		cJSON_AddStringToObject(obj, "poolname", dhcps->name);
		cJSON_AddStringToObject(obj, "start", inet_address(pool->start_ip));
		cJSON_AddStringToObject(obj, "end", inet_address(pool->end_ip));
		cJSON_AddStringToObject(obj, "gateway", inet_address(pool->gateway));
		cJSON_AddStringToObject(obj, "dns", (pool->dns));
		cJSON_AddStringToObject(obj, "tftp", inet_address(pool->tftp_server));
		cJSON_AddNumberToObject(obj, "leases", pool->offer_time);
		cJSON_AddNumberToObject(obj, "total", pool->end_ip - pool->start_ip);
		cJSON_AddNumberToObject(obj, "client", lstCount(&pool->dhcp_lease_list));

		cJSON_AddItemToArray(array, obj);
	}
	return 0;
}

static int web_dhcppool_form(Webs *wp, char *path, char *query)
{
	if (websGetMethodCode(wp) == WEBS_METHOD_GET)
	{
#if ME_GOAHEAD_JSON
		cJSON *obj = cJSON_CreateObject();
		if (obj)
		{
			cJSON *array = cJSON_CreateArray();
			if (array)
			{
				nsm_dhcps_foreach(web_dhcps_get_one_api, array);
				cJSON_AddItemToObject(obj, "dhcppool", array);
			}
		}
		websResponseJson(wp, HTTP_CODE_OK, obj);
		return OK;
#endif
	}
	else if (websGetMethodCode(wp) == WEBS_METHOD_POST)
	{
		int ret = ERROR;
#if ME_GOAHEAD_JSON
		cJSON *root = websGetJsonVar(wp);
		if (root)
		{
			nsm_dhcps_t *pool = NULL;
			struct prefix address;
			zpl_bool enable = cJSON_GetBoolValue(root, "enable");
			char *name = cJSON_GetStringValue(root, "poolname");
			char *start_address = cJSON_GetStringValue(root, "start_address");
			char *end_address = cJSON_GetStringValue(root, "end_address");
			char *gateway = cJSON_GetStringValue(root, "gateway");
			char *gateway2 = cJSON_GetStringValue(root, "gateway2");
			char *dns = cJSON_GetStringValue(root, "dns");
			char *dns2 = cJSON_GetStringValue(root, "dns2");
			char *tftp = cJSON_GetStringValue(root, "tftp");
			int maxleases = cJSON_GetIntValue(root, "maxleases");
			//int minleases = cJSON_GetIntValue(root, "minleases");
			//cJSON *options = cJSON_GetObjectItem(root, "options");

			if (!enable)
			{
				pool = nsm_dhcps_lookup_api(name);
				if (!pool)
				{
					return web_json_format_result(wp, ERROR, "failed, dhcp pool is not exist.");
				}
				ret = nsm_dhcps_del_api(name);

				if (ret == OK)
					return web_json_format_result(wp, 0, "OK");
				else
					return web_json_format_result(wp, ret, "failed");
			}
			pool = nsm_dhcps_lookup_api(name);
			if (!pool)
			{
				nsm_dhcps_add_api(name);
				pool = nsm_dhcps_lookup_api(name);
			}
			if (!pool)
			{
				return web_json_format_result(wp, ERROR, "failed, dhcp pool is not exist.");
			}
			prefix_zero(&address);
			if (start_address)
			{
				prefix_zero(&address);
				ret = str2prefix_ipv4(start_address, (struct prefix_ipv4 *)&address);
				if (ret <= 0)
				{
					return web_json_format_result(wp, ERROR, "failed, Malformed Start Address");
				}
				ret = nsm_dhcps_set_api(pool, DHCPS_CMD_NETWORK_START, &address);
				if (ret <= 0)
				{
					return web_json_format_result(wp, ERROR, "failed, Can not set Start Address");
				}
			}
			else
				nsm_dhcps_unset_api(pool, DHCPS_CMD_NETWORK_START);
			if (end_address)
			{
				prefix_zero(&address);
				ret = str2prefix_ipv4(end_address, (struct prefix_ipv4 *)&address);
				if (ret <= 0)
				{
					return web_json_format_result(wp, ERROR, "failed, Malformed End Address");
				}
				ret = nsm_dhcps_set_api(pool, DHCPS_CMD_NETWORK_END, &address);
				if (ret <= 0)
				{
					return web_json_format_result(wp, ERROR, "failed, Can not set End Address");
				}
			}
			else
				nsm_dhcps_unset_api(pool, DHCPS_CMD_NETWORK_END);
			if (gateway)
			{
				prefix_zero(&address);
				ret = str2prefix_ipv4(gateway, (struct prefix_ipv4 *)&address);
				if (ret <= 0)
				{
					return web_json_format_result(wp, ERROR, "failed, Gateway Address");
				}
				ret = nsm_dhcps_set_api(pool, DHCPS_CMD_GATEWAY, &address);
				if (ret <= 0)
				{
					return web_json_format_result(wp, ERROR, "failed, Can not Gateway Address");
				}
			}
			else
				nsm_dhcps_unset_api(pool, DHCPS_CMD_GATEWAY);
			if (gateway2)
			{
				prefix_zero(&address);
				ret = str2prefix_ipv4(gateway2, (struct prefix_ipv4 *)&address);
				if (ret <= 0)
				{
					return web_json_format_result(wp, ERROR, "failed, Gateway2 Address");
				}
				ret = nsm_dhcps_set_api(pool, DHCPS_CMD_GATEWAY_SECONDARY, &address);
				if (ret <= 0)
				{
					return web_json_format_result(wp, ERROR, "failed, Can not Gateway2 Address");
				}
			}
			else
				nsm_dhcps_unset_api(pool, DHCPS_CMD_GATEWAY_SECONDARY);
			if (dns)
			{
				prefix_zero(&address);
				ret = str2prefix_ipv4(dns, (struct prefix_ipv4 *)&address);
				if (ret <= 0)
				{
					return web_json_format_result(wp, ERROR, "failed, dns Address");
				}
				ret = nsm_dhcps_set_api(pool, DHCPS_CMD_DNS, &address);
				if (ret <= 0)
				{
					return web_json_format_result(wp, ERROR, "failed, Can not dns Address");
				}
			}
			else
				nsm_dhcps_unset_api(pool, DHCPS_CMD_DNS);
			if (dns2)
			{
				prefix_zero(&address);
				ret = str2prefix_ipv4(dns2, (struct prefix_ipv4 *)&address);
				if (ret <= 0)
				{
					return web_json_format_result(wp, ERROR, "failed, dns2 Address");
				}
				ret = nsm_dhcps_set_api(pool, DHCPS_CMD_DNS_SECONDARY, &address);
				if (ret <= 0)
				{
					return web_json_format_result(wp, ERROR, "failed, Can not dns2 Address");
				}
			}
			else
				nsm_dhcps_unset_api(pool, DHCPS_CMD_DNS_SECONDARY);
			if (tftp)
			{
				prefix_zero(&address);
				ret = str2prefix_ipv4(tftp, (struct prefix_ipv4 *)&address);
				if (ret <= 0)
				{
					return web_json_format_result(wp, ERROR, "failed, tftp Address");
				}
				ret = nsm_dhcps_set_api(pool, DHCPS_CMD_TFTP, &address);
				if (ret <= 0)
				{
					return web_json_format_result(wp, ERROR, "failed, Can not tftp Address");
				}
			}
			else
				nsm_dhcps_unset_api(pool, DHCPS_CMD_TFTP);

			ret = nsm_dhcps_set_api(pool, DHCPS_CMD_LEASE, &maxleases);
			if (ret <= 0)
			{
				return web_json_format_result(wp, ERROR, "failed, Can not tftp Address");
			}
			// int maxleases = cJSON_GetIntValue(root, "maxleases");
			// int minleases = cJSON_GetIntValue(root, "minleases");
			// cJSON *options = cJSON_GetObjectItem(root, "options");
			if (ret == OK)
				return web_json_format_result(wp, 0, "OK");
			else
				return web_json_format_result(wp, ret, "failed");
		}
		web_json_format_result(wp, HTTP_CODE_UNSUPPORTED_MEDIA_TYPE, "webservice is not support json");
#endif
	}
	websResponse(wp, HTTP_CODE_BAD_METHOD, "not support method,just support post or get method");
	return 0;
}

static int web_dhcppool_button(Webs *wp, void *pVoid)
{
#if ME_GOAHEAD_JSON
	int ret = ERROR;
	cJSON *root = websGetJsonVar(wp);
	if (root)
	{
		nsm_dhcps_t *pool = NULL;
		struct prefix address;
		zpl_bool enable = cJSON_GetBoolValue(root, "enable");
		char *name = cJSON_GetStringValue(root, "poolname");
		char *start_address = cJSON_GetStringValue(root, "start_address");
		char *end_address = cJSON_GetStringValue(root, "end_address");
		char *gateway = cJSON_GetStringValue(root, "gateway");
		char *gateway2 = cJSON_GetStringValue(root, "gateway2");
		char *dns = cJSON_GetStringValue(root, "dns");
		char *dns2 = cJSON_GetStringValue(root, "dns2");
		char *tftp = cJSON_GetStringValue(root, "tftp");
		int maxleases = cJSON_GetIntValue(root, "maxleases");
		//int minleases = cJSON_GetIntValue(root, "minleases");
		//cJSON *options = cJSON_GetObjectItem(root, "options");

		if (!enable)
		{
			pool = nsm_dhcps_lookup_api(name);
			if (!pool)
			{
				return web_json_format_result(wp, ERROR, "failed, dhcp pool is not exist.");
			}
			ret = nsm_dhcps_del_api(name);

			if (ret == OK)
				return web_json_format_result(wp, 0, "OK");
			else
				return web_json_format_result(wp, ret, "failed");
		}
		pool = nsm_dhcps_lookup_api(name);
		if (!pool)
		{
			nsm_dhcps_add_api(name);
			pool = nsm_dhcps_lookup_api(name);
		}
		if (!pool)
		{
			return web_json_format_result(wp, ERROR, "failed, dhcp pool is not exist.");
		}
		prefix_zero(&address);
		if (start_address)
		{
			prefix_zero(&address);
			ret = str2prefix_ipv4(start_address, (struct prefix_ipv4 *)&address);
			if (ret <= 0)
			{
				return web_json_format_result(wp, ERROR, "failed, Malformed Start Address");
			}
			ret = nsm_dhcps_set_api(pool, DHCPS_CMD_NETWORK_START, &address);
			if (ret <= 0)
			{
				return web_json_format_result(wp, ERROR, "failed, Can not set Start Address");
			}
		}
		else
			nsm_dhcps_unset_api(pool, DHCPS_CMD_NETWORK_START);
		if (end_address)
		{
			prefix_zero(&address);
			ret = str2prefix_ipv4(end_address, (struct prefix_ipv4 *)&address);
			if (ret <= 0)
			{
				return web_json_format_result(wp, ERROR, "failed, Malformed End Address");
			}
			ret = nsm_dhcps_set_api(pool, DHCPS_CMD_NETWORK_END, &address);
			if (ret <= 0)
			{
				return web_json_format_result(wp, ERROR, "failed, Can not set End Address");
			}
		}
		else
			nsm_dhcps_unset_api(pool, DHCPS_CMD_NETWORK_END);
		if (gateway)
		{
			prefix_zero(&address);
			ret = str2prefix_ipv4(gateway, (struct prefix_ipv4 *)&address);
			if (ret <= 0)
			{
				return web_json_format_result(wp, ERROR, "failed, Gateway Address");
			}
			ret = nsm_dhcps_set_api(pool, DHCPS_CMD_GATEWAY, &address);
			if (ret <= 0)
			{
				return web_json_format_result(wp, ERROR, "failed, Can not Gateway Address");
			}
		}
		else
			nsm_dhcps_unset_api(pool, DHCPS_CMD_GATEWAY);
		if (gateway2)
		{
			prefix_zero(&address);
			ret = str2prefix_ipv4(gateway2, (struct prefix_ipv4 *)&address);
			if (ret <= 0)
			{
				return web_json_format_result(wp, ERROR, "failed, Gateway2 Address");
			}
			ret = nsm_dhcps_set_api(pool, DHCPS_CMD_GATEWAY_SECONDARY, &address);
			if (ret <= 0)
			{
				return web_json_format_result(wp, ERROR, "failed, Can not Gateway2 Address");
			}
		}
		else
			nsm_dhcps_unset_api(pool, DHCPS_CMD_GATEWAY_SECONDARY);
		if (dns)
		{
			prefix_zero(&address);
			ret = str2prefix_ipv4(dns, (struct prefix_ipv4 *)&address);
			if (ret <= 0)
			{
				return web_json_format_result(wp, ERROR, "failed, dns Address");
			}
			ret = nsm_dhcps_set_api(pool, DHCPS_CMD_DNS, &address);
			if (ret <= 0)
			{
				return web_json_format_result(wp, ERROR, "failed, Can not dns Address");
			}
		}
		else
			nsm_dhcps_unset_api(pool, DHCPS_CMD_DNS);
		if (dns2)
		{
			prefix_zero(&address);
			ret = str2prefix_ipv4(dns2, (struct prefix_ipv4 *)&address);
			if (ret <= 0)
			{
				return web_json_format_result(wp, ERROR, "failed, dns2 Address");
			}
			ret = nsm_dhcps_set_api(pool, DHCPS_CMD_DNS_SECONDARY, &address);
			if (ret <= 0)
			{
				return web_json_format_result(wp, ERROR, "failed, Can not dns2 Address");
			}
		}
		else
			nsm_dhcps_unset_api(pool, DHCPS_CMD_DNS_SECONDARY);
		if (tftp)
		{
			prefix_zero(&address);
			ret = str2prefix_ipv4(tftp, (struct prefix_ipv4 *)&address);
			if (ret <= 0)
			{
				return web_json_format_result(wp, ERROR, "failed, tftp Address");
			}
			ret = nsm_dhcps_set_api(pool, DHCPS_CMD_TFTP, &address);
			if (ret <= 0)
			{
				return web_json_format_result(wp, ERROR, "failed, Can not tftp Address");
			}
		}
		else
			nsm_dhcps_unset_api(pool, DHCPS_CMD_TFTP);

		ret = nsm_dhcps_set_api(pool, DHCPS_CMD_LEASE, &maxleases);
		if (ret <= 0)
		{
			return web_json_format_result(wp, ERROR, "failed, Can not tftp Address");
		}
		// int maxleases = cJSON_GetIntValue(root, "maxleases");
		// int minleases = cJSON_GetIntValue(root, "minleases");
		// cJSON *options = cJSON_GetObjectItem(root, "options");
		if (ret == OK)
			return web_json_format_result(wp, 0, "OK");
		else
			return web_json_format_result(wp, ret, "failed");
	}
	else
		return web_json_format_result(wp, ERROR, "can not get module value");
#else
	return web_json_format_result(wp, HTTP_CODE_UNSUPPORTED_MEDIA_TYPE, "webservice is not support json");
#endif
}


static int web_dhcplease_button(Webs *wp, void *pVoid)
{
#if ME_GOAHEAD_JSON
	//int ret = ERROR;
	cJSON *root = websGetJsonVar(wp);
	if (root)
		{
			nsm_dhcps_t *pool = NULL;
			cJSON *obj = NULL;
			char *name = cJSON_GetStringValue(root, "poolname");
			pool = nsm_dhcps_lookup_api(name);
			if (!pool)
			{
				return web_json_format_result(wp, ERROR, "failed, dhcp pool is not exist.");
			}
			obj = cJSON_CreateObject();
			if (obj)
			{
				cJSON *array = cJSON_CreateArray();
				if (array)
				{
					dhcp_lease_foreach_one(pool, web_dhcps_lease_get_one_api, array);
					cJSON_AddItemToObject(obj, "dhcplease", array);
				}
			}
			websResponseJson(wp, HTTP_CODE_OK, obj);
			return 0;
		}
	else
		return web_json_format_result(wp, ERROR, "can not get module value");
#else
	return web_json_format_result(wp, HTTP_CODE_UNSUPPORTED_MEDIA_TYPE, "webservice is not support json");
#endif
}

#endif /*ZPL_DHCPS_MODULE*/

#ifdef ZPL_NSM_DHCP
int web_html_dhcp_init(void)
{
	websFormDefine("dhcpserver", web_dhcppool_form);
	web_button_add_hook("dhcpserver", "button-submit", web_dhcppool_button, NULL);
	web_button_add_hook("dhcplease", "button-submit", web_dhcplease_button, NULL);
	return 0;
}
#endif /*ZPL_NSM_DHCP*/