/*
 * web_syslog_html.c
 *
 *  Created on: 2019年8月9日
 *      Author: DELL
 */

#define HAS_BOOL 1
#include "zplos_include.h"

#include "module.h"
#include "zmemory.h"
#include "zassert.h"
#include "command.h"
#include "prefix.h"
#include "host.h"
#include "log.h"
#include "web_api.h"
#include "web_jst.h"
#include "web_app.h"

struct web_menu_table web_menu_tbl[] = {
    { "/home/Status", 		"Status", 		NULL, 0, NULL },
    { "/home/System", 		"System", 		NULL, 0, NULL  },
    { "/home/Interface", 	"Interface", 	NULL, 0, NULL  },
    { "/home/l2Stp", 		"l2Stp", 		NULL, 0, NULL },
    { "/home/Vlan", 		"Vlan", 		NULL, 0, NULL },
    { "/home/Qos", 			"Qos", 			NULL, 0, NULL  },
    { "/home/StaticRoute", 	"StaticRoute", 	NULL, 0, NULL  },
    { "/home/Multimedia", 	"Multimedia", 		NULL, 0, NULL  },
    { "/home/Service", 		"Service", 		NULL, 0, NULL  },
    { "/home/About", 		"About", 		NULL, 0, NULL  },
    { "/home/Upgrade", 		"Upgrade", 		NULL, 0, NULL  },
};

struct web_menu_table web_sub_router_tbl[] = {
	{ "/home/Status", 		"Status", 		"SysStatus", 0, NULL  },
	{ "/home/System", 		"System", 		"SysStatus", 0, NULL  },
	{ "/home/Interface", 	"Interface", 	"SysStatus", 0, NULL  },
	{ "/home/l2Stp", 		"l2Stp", 		"Stp", 0, NULL },
	{ "/home/Vlan", 		"Vlan", 		"SysStatus", 0, NULL },
	{ "/home/Qos", 			"Qos", 			"SysStatus", 0, NULL  },
	{ "/home/StaticRoute", 	"StaticRoute", 	"StaticRoute", 0, NULL  },
	{ "/home/Multimedia", 	"Multimedia", 		"SysStatus", 0, NULL  },
	{ "/home/Service", 		"Service", 		"SysStatus", 0, NULL  },
	{ "/home/About", 		"About", 		"SysStatus", 0, NULL  },
	{ "/home/Upgrade", 		"Upgrade", 		"Upgrade", 0, NULL  },
	{ NULL, 		NULL, 		NULL, 0, NULL  }
};

struct web_menu_table web_router_tbl[] = {
    { "/home", 		"home", 		"HomeView", 0, web_sub_router_tbl},
	{ NULL, 		NULL, 		NULL, 0, NULL  }
};

static int web_menu_get(Webs *wp, char *path, char *query)
{
#if ME_GOAHEAD_JSON
	cJSON *array = NULL;
	cJSON *obj = NULL;
	int i = 0;
	for(i = 0; i < sizeof(web_menu_tbl)/sizeof(web_menu_tbl[0]); i++)
	{
		obj = cJSON_CreateObject();
		if(obj)
		{
			cJSON_AddStringToObject(obj,"topath", web_menu_tbl[i].path);
			cJSON_AddStringToObject(obj,"name", web_menu_tbl[i].name);
			
			if(array == NULL)
			{
				array = cJSON_CreateArray();
			}
			if(array)
				cJSON_AddItemToArray(array, obj);
		}
	}
	if(array)
		websResponseJson(wp, HTTP_CODE_OK, array);
	else
#endif
		websResponse(wp, HTTP_CODE_NOT_FOUND, NULL);	
	return OK;
}
#if ME_GOAHEAD_JSON
static cJSON * web_menu_array_get(Webs *wp, struct web_menu_table *menuarray)
{
	int i = 0;
	cJSON *array = NULL;
	cJSON *obj = NULL;
	cJSON *meta = NULL;
	cJSON *rule = NULL;
	char *roles[] = {"administrator", "customer"};
	while(menuarray[i].path && menuarray[i].name && menuarray[i].component)
	{
		obj = cJSON_CreateObject();
		if(obj)
		{
			cJSON_AddStringToObject(obj,"path", menuarray[i].path);
			cJSON_AddStringToObject(obj,"name", menuarray[i].name);
			cJSON_AddStringToObject(obj,"component", menuarray[i].component);
			meta = cJSON_CreateObject();
			if(meta)
			{
				cJSON_AddTrueToObject(meta, "requiresAuth");
    
				rule = cJSON_CreateStringArray(roles, 2);
				if(rule)
					cJSON_AddItemToObject(meta, "roles", rule);

				cJSON_AddItemToObject(obj, "meta", meta);
			}
			if(menuarray[i].children)
			{
				cJSON *subarray = NULL;
				subarray = web_menu_array_get(wp, menuarray[i].children);
				cJSON_AddItemToObject(obj, "children", subarray);
			}
			if(array == NULL)
			{
				array = cJSON_CreateArray();
			}
			if(array)
				cJSON_AddItemToArray(array, obj);
		}
		i++;
	}
	return array;
}
#endif
static int web_menu_route_get(Webs *wp, char *path, char *query)
{
#if ME_GOAHEAD_JSON
	cJSON *array = NULL;
	array = web_menu_array_get(wp, web_router_tbl);
	if(array)
		websResponseJson(wp, HTTP_CODE_OK, array);
	else
#endif
		websResponse(wp, HTTP_CODE_NOT_FOUND, NULL);	
	return OK;
}

static int web_menu_tabs_get(Webs *wp, char *path, char *query)
{
#if ME_GOAHEAD_JSON
	cJSON *array = NULL;
	cJSON *obj = NULL;
	int i = 0;
	for(i = 0; i < sizeof(web_menu_tbl)/sizeof(web_menu_tbl[0]); i++)
	{
		obj = cJSON_CreateObject();
		if(obj)
		{
			cJSON_AddStringToObject(obj,"topath", web_menu_tbl[i].path);
			cJSON_AddStringToObject(obj,"name", web_menu_tbl[i].name);
			if(array == NULL)
			{
				array = cJSON_CreateArray();
			}
			if(array)
				cJSON_AddItemToArray(array, obj);
		}
	}
	if(array)
		websResponseJson(wp, HTTP_CODE_OK, array);
	else
#endif
		websResponse(wp, HTTP_CODE_NOT_FOUND, NULL);	
	return OK;
}

int web_menu_app(void)
{
	websFormDefine("menu", web_menu_get);
	websFormDefine("router", web_menu_route_get);
	websFormDefine("tabs", web_menu_tabs_get);
	return 0;
}

