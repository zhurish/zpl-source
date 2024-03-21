/**
 * @file      : web_html_interface.c
 * @brief     : Description
 * @author    : zhurish (zhurish@163.com)
 * @version   : 1.0
 * @date      : 2024-03-03
 *
 * @copyright : Copyright (c) - 2024 zhurish(zhurish@163.com).Co.Ltd. All rights reserved.
 *
 */
#include "goahead.h"
#include "webutil.h"
#include "if.h"
#include "nsm_ipvrf.h"
#include "nsm_interface.h"
#include "nsm_dhcp.h"
#ifdef ZPL_WIFI_MODULE
#include "iw_interface.h"
#endif
//#include "nsm_include.h"
#include "web_api.h"
#include "web_app.h"

static int web_interface_all_one_api(struct interface *ifp, void *pVoid)
{
	cJSON *array = pVoid;
	cJSON *obj = cJSON_CreateObject();

	if (obj && array && ifp)
	{
		struct prefix address;
		char prefixbuf[128];
		union prefix46constptr pu;
		//struct nsm_interface *zif = ifp->info[MODULE_NSM];
		//{ interface: 'loopback1', iftype: 'loopback', ifmode: 'access', switchport: false,
		// delete: true, prefix: '192.168.14.1/24', vrf: '0', admin: true, link: true }
		cJSON_AddStringToObject(obj, "interface", ifp->name);
		cJSON_AddStringToObject(obj, "iftype", getifpname(ifp->if_type));
		if (ifp->if_mode == IF_MODE_TRUNK_L2)
			cJSON_AddStringToObject(obj, "ifmode", "trunk");
		else if (ifp->if_mode == IF_MODE_MPLS_L2)
			cJSON_AddStringToObject(obj, "ifmode", "mpls");
		else // if(ifp->if_mode == IF_MODE_ACCESS_L2)
			cJSON_AddStringToObject(obj, "ifmode", "access");
		cJSON_AddBoolToObject(obj, "switchport", CHECK_FLAG(ifp->status, IF_INTERFACE_IS_SWITCHPORT) ? false : true);
		cJSON_AddBoolToObject(obj, "delete", CHECK_FLAG(ifp->status, IF_INTERFACE_CAN_DELETE) ? true : false);

		if (nsm_interface_address_get_api(ifp, &address) == OK)
		{
			pu.p = &address;
			cJSON_AddStringToObject(obj, "prefix", prefix2str(pu, prefixbuf, sizeof(prefixbuf)));
		}
		else
			cJSON_AddStringToObject(obj, "prefix", " ");
		cJSON_AddNumberToObject(obj, "vrf", ifp->vrf_id);

		if (ifp && ifp->shutdown == IF_INTERFACE_SHUTDOWN_ON)
			cJSON_AddBoolToObject(obj, "admin", true);
		else
			cJSON_AddBoolToObject(obj, "admin", false);
		cJSON_AddBoolToObject(obj, "link", if_is_up(ifp) ? true : false);

		cJSON_AddItemToArray(array, obj);
	}
	return 0;
}

static int web_interface_all_form(Webs *wp, char *path, char *query)
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
				if_list_each(web_interface_all_one_api, array);
				cJSON_AddItemToObject(obj, "interface", array);
			}
		}
		websResponseJson(wp, HTTP_CODE_OK, obj);
		return OK;
#endif
	}
	websResponse(wp, HTTP_CODE_BAD_METHOD, "not support method,just support post or get method");
	return 0;
}

static int web_interface_all_switchport_api(struct interface *ifp, void *pVoid)
{
	cJSON *array = pVoid;
	cJSON *obj = cJSON_CreateObject();

	if (obj && array && ifp && CHECK_FLAG(ifp->status, IF_INTERFACE_IS_SWITCHPORT))
	{
		struct prefix address;
		char prefixbuf[128];
		union prefix46constptr pu;
		//struct nsm_interface *zif = ifp->info[MODULE_NSM];
		//{ interface: 'loopback1', iftype: 'loopback', ifmode: 'access', switchport: false,
		// delete: true, prefix: '192.168.14.1/24', vrf: '0', admin: true, link: true }
		cJSON_AddStringToObject(obj, "interface", ifp->name);
		cJSON_AddStringToObject(obj, "iftype", getifpname(ifp->if_type));
		if (ifp->if_mode == IF_MODE_TRUNK_L2)
			cJSON_AddStringToObject(obj, "ifmode", "trunk");
		else if (ifp->if_mode == IF_MODE_MPLS_L2)
			cJSON_AddStringToObject(obj, "ifmode", "mpls");
		else // if(ifp->if_mode == IF_MODE_ACCESS_L2)
			cJSON_AddStringToObject(obj, "ifmode", "access");

		cJSON_AddBoolToObject(obj, "switchport", (ifp->if_mode == IF_MODE_L3) ? false : true);
		cJSON_AddBoolToObject(obj, "delete", CHECK_FLAG(ifp->status, IF_INTERFACE_CAN_DELETE) ? true : false);

		if (nsm_interface_address_get_api(ifp, &address) == OK)
		{
			pu.p = &address;
			cJSON_AddStringToObject(obj, "prefix", prefix2str(pu, prefixbuf, sizeof(prefixbuf)));
		}
		else 
		{
			if (ifp->if_mode == IF_MODE_TRUNK_L2)
			{
				char vlist[512];
				memset(vlist, 0, sizeof(vlist));
				if(nsm_vlan_interface_vlanlist(ifp, vlist) == OK)
					cJSON_AddStringToObject(obj, "prefix", vlist);
				else
					cJSON_AddStringToObject(obj, "prefix", " ");	
			}
			else
				cJSON_AddStringToObject(obj, "prefix", " ");
		}
		cJSON_AddStringToObject(obj, "vrf", ip_vrf_vrfid2name(ifp->vrf_id));

		if (ifp && ifp->shutdown == IF_INTERFACE_SHUTDOWN_ON)
			cJSON_AddBoolToObject(obj, "admin", true);
		else
			cJSON_AddBoolToObject(obj, "admin", false);
		cJSON_AddBoolToObject(obj, "link", if_is_up(ifp) ? true : false);

		cJSON_AddItemToArray(array, obj);
	}
	return 0;
}

static int web_interface_all_switchport_form(Webs *wp, char *path, char *query)
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
				if_list_each(web_interface_all_switchport_api, array);
				cJSON_AddItemToObject(obj, "interface", array);
			}
		}
		websResponseJson(wp, HTTP_CODE_OK, obj);
		return OK;
#endif
	}
	websResponse(wp, HTTP_CODE_BAD_METHOD, "not support method,just support post or get method");
	return 0;
}


static int web_interface_button(Webs *wp, void *pVoid)
{
#if ME_GOAHEAD_JSON
	int ret = ERROR;
	struct interface *ifp = NULL;
	cJSON *root = websGetJsonVar(wp);
	if (root)
	{
		char *btnid = cJSON_GetStringValue(root, "button-ID");
		char *action = cJSON_GetStringValue(root, "button-action");
		char *ifname = cJSON_GetStringValue(root, "interface");

		if (btnid && ifname && action && os_strstr(action, "interface-switchport") && os_strstr(btnid, "submit"))
		{
			zpl_bool switchport = cJSON_GetBoolValue(root, "switchport");
			ifp = if_lookup_by_name(ifname);
			if (ifp)
			{
				if_mode_t mode = 0, newmode = switchport ? IF_MODE_ACCESS_L2 : IF_MODE_L3;
				if (nsm_interface_mode_get_api(ifp, &mode) == OK)
				{
					if (newmode != mode)
						ret = nsm_interface_mode_set_api(ifp, newmode);
					else
						ret = OK;
				}
				if (ret == OK)
					return web_json_format_result(wp, 0, "OK");
				else
					return web_json_format_result(wp, ret, "failed");
			}
			return web_json_format_result(wp, ERROR, "can not get interface of '%s'", ifname);
		}
		else if (btnid && ifname && action && os_strstr(action, "interface") && os_strstr(btnid, "shutdown")) // shutdown or no shutdown
		{
			zpl_bool admin = cJSON_GetBoolValue(root, "admin");
			ifp = if_lookup_by_name(ifname);
			if (ifp)
			{
				if (admin)
					ret = nsm_interface_up_set_api(ifp);
				else
					ret = nsm_interface_down_set_api(ifp);
				if (ret == OK)
					return web_json_format_result(wp, 0, "OK");
				else
					return web_json_format_result(wp, ret, "failed");
			}
			return web_json_format_result(wp, ERROR, "can not get interface of '%s'", ifname);
		}
		else if (btnid && ifname && action && os_strstr(action, "interface-state") && os_strstr(btnid, "delete")) // delete interface
		{
			ifp = if_lookup_by_name(ifname);
			if (ifp)
			{
				struct nsm_interface *if_data = ifp->info[MODULE_NSM];
				if(if_data)
				{
					memset(&if_data->stats, 0, sizeof(struct if_stats));
				}
				ret = OK;//nsm_interface_delete_api(ifp);
				if (ret == OK)
					return web_json_format_result(wp, 0, "OK");
				else
					return web_json_format_result(wp, ret, "failed");
			}
			return web_json_format_result(wp, ERROR, "can not get interface of '%s'", ifname);
		}
		else if (btnid && ifname && action && os_strstr(action, "interface") && os_strstr(btnid, "delete")) // delete interface
		{
			ifp = if_lookup_by_name(ifname);
			if (ifp)
			{
				if (!CHECK_FLAG(ifp->status, IF_INTERFACE_CAN_DELETE))
				{
					return web_json_format_result(wp, ERROR, "can not delete interface of '%s'", ifname);
				}
				ret = nsm_interface_delete_api(ifp);
				if (ret == OK)
					return web_json_format_result(wp, 0, "OK");
				else
					return web_json_format_result(wp, ret, "failed");
			}
			return web_json_format_result(wp, ERROR, "can not get interface of '%s'", ifname);
		}
		else if (btnid && ifname && action && os_strstr(action, "interface") && os_strstr(btnid, "submit")) // update interface
		{
			char *ifmode = cJSON_GetStringValue(root, "ifmode");
			char *prefix = cJSON_GetStringValue(root, "prefix");
			//char *iftype = cJSON_GetStringValue(root, "iftype");
			//zpl_bool admin = cJSON_GetBoolValue(root, "admin");
			zpl_bool create = cJSON_GetBoolValue(root, "create");
			if (create)
			{
				if (if_lookup_by_name(ifname))
					return web_json_format_result(wp, ERROR, "can not create interface of '%s', this interface is already exist", ifname);
				if (nsm_interface_create_api(ifname) == OK)
				{
					ifp = if_lookup_by_name(ifname);
					if (ifp)
					{
						if (os_strstr(ifmode, "access"))
							ret = nsm_interface_mode_set_api(ifp, IF_MODE_ACCESS_L2);
						else if (os_strstr(ifmode, "trunk"))
							ret = nsm_interface_mode_set_api(ifp, IF_MODE_TRUNK_L2);
						else if (os_strstr(ifmode, "mpls"))
							ret = nsm_interface_mode_set_api(ifp, IF_MODE_MPLS_L2);
						else
							ret = nsm_interface_mode_set_api(ifp, IF_MODE_L3);
						if (prefix)
						{
							struct prefix cp;
							ret = str2prefix_ipv4(prefix, (struct prefix_ipv4 *)&cp);
							if (ret <= 0)
							{
								return web_json_format_result(wp, ERROR, "can not set interface '%s' of prefix address, is Malformed address", ifname);
							}
							if (IPV4_NET127(cp.u.prefix4.s_addr))
							{
								return web_json_format_result(wp, ERROR, "can not set interface '%s' of prefix address, is Lookback address", ifname);
							}
							if (IPV4_MULTICAST(cp.u.prefix4.s_addr))
							{
								return web_json_format_result(wp, ERROR, "can not set interface '%s' of prefix address, is Multicast address", ifname);
							}
							ret = nsm_interface_address_set_api(ifp, &cp, zpl_false);
						}
						if (ret == OK)
							return web_json_format_result(wp, 0, "OK");
						else
							return web_json_format_result(wp, ret, "failed");
					}
					return web_json_format_result(wp, ERROR, "can not get interface of '%s'", ifname);
				}
				return web_json_format_result(wp, ERROR, "can not create interface of '%s'", ifname);
			}
			else
			{
				ifp = if_lookup_by_name(ifname);
				if (ifp)
				{
					if (os_strstr(ifmode, "access"))
						ret = nsm_interface_mode_set_api(ifp, IF_MODE_ACCESS_L2);
					else if (os_strstr(ifmode, "trunk"))
						ret = nsm_interface_mode_set_api(ifp, IF_MODE_TRUNK_L2);
					else if (os_strstr(ifmode, "mpls"))
						ret = nsm_interface_mode_set_api(ifp, IF_MODE_MPLS_L2);
					else
						ret = nsm_interface_mode_set_api(ifp, IF_MODE_L3);
					if (prefix)
					{
						struct prefix cp;
						ret = str2prefix_ipv4(prefix, (struct prefix_ipv4 *)&cp);
						if (ret <= 0)
						{
							return web_json_format_result(wp, ERROR, "can not set interface '%s' of prefix address, is Malformed address", ifname);
						}
						if (IPV4_NET127(cp.u.prefix4.s_addr))
						{
							return web_json_format_result(wp, ERROR, "can not set interface '%s' of prefix address, is Lookback address", ifname);
						}
						if (IPV4_MULTICAST(cp.u.prefix4.s_addr))
						{
							return web_json_format_result(wp, ERROR, "can not set interface '%s' of prefix address, is Multicast address", ifname);
						}
						ret = nsm_interface_address_set_api(ifp, &cp, zpl_false);
					}
					else
					{
						struct prefix address;
						nsm_interface_address_get_api(ifp, &address);
						nsm_interface_address_unset_api(ifp, &address, zpl_false);
					}
					if (ret == OK)
						return web_json_format_result(wp, 0, "OK");
					else
						return web_json_format_result(wp, ret, "failed");
				}
				return web_json_format_result(wp, ERROR, "can not get interface of '%s'", ifname);
			}
			return web_json_format_result(wp, ERROR, "can not get interface of '%s'", ifname);
		}
		else
			return web_json_format_result(wp, ERROR, "can not get module value");
	}
	return web_json_format_result(wp, HTTP_CODE_UNSUPPORTED_MEDIA_TYPE, "webservice is not support json");
#endif
}

static int web_interface_all_state(struct interface *ifp, void *pVoid)
{
	cJSON *array = pVoid;
	cJSON *obj = cJSON_CreateObject();

	if (obj && array && ifp)
	{
		char tmp[64];
		struct nsm_interface *if_data = ifp->info[MODULE_NSM];
		//['interface', 'speed', 'rate', 'statistic', 'admin', 'link', 'action']
		cJSON_AddStringToObject(obj, "interface", ifp->name);

		memset(tmp, 0, sizeof(tmp));
		sprintf(tmp, "%s/%s", nsm_interface_speed_name(if_data->speed), nsm_interface_speed_name(if_data->hw_speed));
		cJSON_AddStringToObject(obj, "speed", tmp);
		memset(tmp, 0, sizeof(tmp));
		sprintf(tmp, "%s/%s", nsm_interface_duplex_name(if_data->duplex), nsm_interface_duplex_name(if_data->hw_duplex));
		cJSON_AddStringToObject(obj, "duplex", tmp);

		cJSON_AddNumberToObject(obj, "rx_packets", if_data->stats.rx_packets);
		cJSON_AddNumberToObject(obj, "tx_packets", if_data->stats.tx_packets);
		cJSON_AddNumberToObject(obj, "rx_bytes", if_data->stats.rx_bytes);
		cJSON_AddNumberToObject(obj, "tx_bytes", if_data->stats.tx_bytes);
		cJSON_AddNumberToObject(obj, "rx_errors", if_data->stats.rx_errors);
		cJSON_AddNumberToObject(obj, "tx_errors", if_data->stats.tx_errors);
		cJSON_AddNumberToObject(obj, "rx_dropped", if_data->stats.rx_dropped);
		cJSON_AddNumberToObject(obj, "tx_dropped", if_data->stats.tx_dropped);

		if (ifp && ifp->shutdown == IF_INTERFACE_SHUTDOWN_ON)
			cJSON_AddBoolToObject(obj, "admin", true);
		else
			cJSON_AddBoolToObject(obj, "admin", false);
		cJSON_AddBoolToObject(obj, "link", if_data->link_status ? true : false);

		cJSON_AddItemToArray(array, obj);
	}
	return 0;
}

static int web_interface_state_form(Webs *wp, char *path, char *query)
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
				if_list_each(web_interface_all_state, array);
				cJSON_AddItemToObject(obj, "interface", array);
			}
		}
		websResponseJson(wp, HTTP_CODE_OK, obj);
		return OK;
#endif
	}
	websResponse(wp, HTTP_CODE_BAD_METHOD, "not support method,just support post or get method");
	return 0;
}



int web_html_interface_init(void)
{
	websFormDefine("interface-all-list", web_interface_all_form);
	websFormDefine("interface-all-switchport", web_interface_all_switchport_form);
	websFormDefine("interface-state", web_interface_state_form);
	
	web_button_add_hook("switchport", "button-submit", web_interface_button, NULL);
	web_button_add_hook("interface", "button-shutdown", web_interface_button, NULL);
	web_button_add_hook("interface", "button-delete", web_interface_button, NULL);
	web_button_add_hook("interface", "button-submit", web_interface_button, NULL);
	web_button_add_hook("interface-state", "button-delete", web_interface_button, NULL);
	return 0;
}
