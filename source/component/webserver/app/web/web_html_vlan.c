/**
 * @file      : web_html_vlan.c
 * @brief     : Description
 * @author    : zhurish (zhurish@163.com)
 * @version   : 1.0
 * @date      : 2024-03-07
 * 
 * @copyright : Copyright (c) - 2024 zhurish(zhurish@163.com).Co.Ltd. All rights reserved.
 * 
 */
#include "zplos_include.h"
#include "zassert.h"
#include "vty.h"
#include "if.h"
#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "zmemory.h"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "nsm_ipvrf.h"
#include "nsm_interface.h"
#include "nsm_include.h"
#include "web_api.h"
#include "web_app.h"



static int _nsm_vlan_database_member(nsm_vlan_database_t *vlaninfo, void *pVoid)
{
	zpl_uint32 i = 0;
	cJSON *untag_array = NULL;
	cJSON *tag_array = NULL;
	struct interface *ifp = NULL;
	cJSON *obj = cJSON_CreateObject();
	cJSON *root = pVoid;

	if (obj && vlaninfo)
	{
		for (i = 0; i < PHY_PORT_MAX; i++)
		{
			if (vlaninfo->untagport[i] != 0)
			{
				ifp = if_lookup_by_index(vlaninfo->untagport[i]);
				if (ifp)
				{
					if (untag_array == NULL)
						untag_array = cJSON_CreateArray();
					cJSON_AddStringToObject(obj, "interface", ifp->name);
					cJSON_AddStringToObject(obj, "iftype", getifpname(ifp->if_type));
					if (ifp->if_mode == IF_MODE_TRUNK_L2)
						cJSON_AddStringToObject(obj, "ifmode", "trunk");
					else if (ifp->if_mode == IF_MODE_MPLS_L2)
						cJSON_AddStringToObject(obj, "ifmode", "mpls");
					else
						cJSON_AddStringToObject(obj, "ifmode", "access");

					cJSON_AddBoolToObject(obj, "switchport", CHECK_FLAG(ifp->status, IF_INTERFACE_IS_SWITCHPORT) ? false : true);
					cJSON_AddBoolToObject(obj, "delete", CHECK_FLAG(ifp->status, IF_INTERFACE_CAN_DELETE) ? true : false);
					cJSON_AddStringToObject(obj, "vrf", ip_vrf_vrfid2name(ifp->vrf_id));

					if (ifp && ifp->shutdown == IF_INTERFACE_SHUTDOWN_ON)
						cJSON_AddBoolToObject(obj, "admin", true);
					else
						cJSON_AddBoolToObject(obj, "admin", false);
					cJSON_AddBoolToObject(obj, "link", if_is_up(ifp) ? true : false);

					cJSON_AddItemToArray(untag_array, obj);
				}
			}
			if (vlaninfo->tagport[i] != 0)
			{
				ifp = if_lookup_by_index(vlaninfo->tagport[i]);
				if (ifp)
				{
					if (tag_array == NULL)
						tag_array = cJSON_CreateArray();
					cJSON_AddStringToObject(obj, "interface", ifp->name);
					cJSON_AddStringToObject(obj, "iftype", getifpname(ifp->if_type));
					if (ifp->if_mode == IF_MODE_TRUNK_L2)
						cJSON_AddStringToObject(obj, "ifmode", "trunk");
					else if (ifp->if_mode == IF_MODE_MPLS_L2)
						cJSON_AddStringToObject(obj, "ifmode", "mpls");
					else
						cJSON_AddStringToObject(obj, "ifmode", "access");

					cJSON_AddBoolToObject(obj, "switchport", CHECK_FLAG(ifp->status, IF_INTERFACE_IS_SWITCHPORT) ? false : true);
					cJSON_AddBoolToObject(obj, "delete", CHECK_FLAG(ifp->status, IF_INTERFACE_CAN_DELETE) ? true : false);
					cJSON_AddStringToObject(obj, "vrf", ip_vrf_vrfid2name(ifp->vrf_id));

					if (ifp && ifp->shutdown == IF_INTERFACE_SHUTDOWN_ON)
						cJSON_AddBoolToObject(obj, "admin", true);
					else
						cJSON_AddBoolToObject(obj, "admin", false);
					cJSON_AddBoolToObject(obj, "link", if_is_up(ifp) ? true : false);

					cJSON_AddItemToArray(tag_array, obj);
				}
			}
		}
	}
	if (root && tag_array)
	{
		cJSON_AddItemToObject(root, "taglist", tag_array);
	}
	if (root && untag_array)
	{
		cJSON_AddItemToObject(root, "untaglist", untag_array);
	}
	return 0;
}

static int web_vlan_member_form(Webs *wp, char *path, char *query)
{
	if (websGetMethodCode(wp) == WEBS_METHOD_GET)
	{
#if ME_GOAHEAD_JSON
		vlan_t vlanid = 0;
		cJSON *obj = cJSON_CreateObject();
		cJSON *root = websGetJsonVar(wp);
		if (root)
		{
			vlanid = cJSON_GetIntValue(root, "vlanid");
		}
		if (obj)
		{
			nsm_vlan_database_callback_api(vlanid, _nsm_vlan_database_member, obj);
		}
		websResponseJson(wp, HTTP_CODE_OK, obj);
		return OK;
#endif
	}
	websResponse(wp, HTTP_CODE_BAD_METHOD, "not support method,just support post or get method");
	return 0;
}

static int web_interface_vlanif(struct interface *ifp, void *pVoid)
{
	cJSON *array = pVoid;
	cJSON *obj = cJSON_CreateObject();

	if (obj && array && ifp)
	{
		char tmp[64];
		nsm_vlan_t *nsm_vlan = NULL;
		nsm_vlan = (nsm_vlan_t *)nsm_intf_module_data(ifp, NSM_INTF_VLAN);
		if(nsm_vlan && (ifp->if_mode == IF_MODE_TRUNK_L2||ifp->if_mode == IF_MODE_MPLS_L2||ifp->if_mode == IF_MODE_ACCESS_L2))
		{
			////vlaniffields: ['interface', 'ifmode', 'vlanlist', 'native', 'enable'],
			//IF_NSM_VLAN_DATA_LOCK(nsm_vlan);
			cJSON_AddStringToObject(obj, "interface", ifp->name);
			if (ifp->if_mode == IF_MODE_TRUNK_L2)
				cJSON_AddStringToObject(obj, "ifmode", "trunk");
			else if (ifp->if_mode == IF_MODE_MPLS_L2)
				cJSON_AddStringToObject(obj, "ifmode", "mpls");
			else if(ifp->if_mode == IF_MODE_ACCESS_L2)
				cJSON_AddStringToObject(obj, "ifmode", "access");

			if(ifp->if_mode == IF_MODE_TRUNK_L2)
			{
				//if(nsm_vlan->native)
					cJSON_AddNumberToObject(obj, "native", nsm_vlan->native);
				if(nsm_vlan->allow_all)
					cJSON_AddStringToObject(obj, "vlanlist", "all");
				else	
				{
					memset(tmp, 0, sizeof(tmp));
					if(nsm_vlan_interface_vlanlist(ifp, tmp) == OK)
						cJSON_AddStringToObject(obj, "vlanlist", tmp);
					else
						cJSON_AddStringToObject(obj, "vlanlist", " ");	
				}
			}
			else if(ifp->if_mode == IF_MODE_ACCESS_L2)
			{
				memset(tmp, 0, sizeof(tmp));
				sprintf(tmp, "%d", nsm_vlan->access);
				cJSON_AddStringToObject(obj, "vlanlist", tmp);
				cJSON_AddNumberToObject(obj, "native", nsm_vlan->access);
			}
			else if(ifp->if_mode == IF_MODE_MPLS_L2)
			{
				//if(nsm_vlan->native)
					cJSON_AddNumberToObject(obj, "native", nsm_vlan->native);
				if(nsm_vlan->allow_all)
					cJSON_AddStringToObject(obj, "vlanlist", "all");
				else	
				{
					memset(tmp, 0, sizeof(tmp));
					if(nsm_vlan_interface_vlanlist(ifp, tmp) == OK)
						cJSON_AddStringToObject(obj, "vlanlist", tmp);
					else
						cJSON_AddStringToObject(obj, "vlanlist", " ");	
				}
			}
			cJSON_AddBoolToObject(obj, "enable", true);
			cJSON_AddItemToArray(array, obj);
		}
	}
	return 0;
}

static int web_interface_vlan_form(Webs *wp, char *path, char *query)
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
				if_list_each(web_interface_vlanif, array);
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

int web_html_vlan_init(void)
{
	websFormDefine("vlan-ifmember", web_vlan_member_form);
	websFormDefine("interface-vlanlist", web_interface_vlan_form);
	return 0;
}
