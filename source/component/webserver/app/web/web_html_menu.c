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

#include "web_app.h"

#define ME_GOAHEAD_JSON 1
#if ME_GOAHEAD_JSON
static cJSON* web_resource_load(char *filename)
{
    if (os_file_access(filename) != OK)
    {
        return NULL;
    }
    int file_size = (int)os_file_size(filename);
    char *buffer = (char *)malloc(file_size + 1);
    if (!buffer)
    {
        return NULL;
    }
    memset(buffer, 0, file_size + 1);
    if (os_read_file(filename, buffer, file_size) != OK)
    {
    if (buffer != NULL)
        free(buffer);
        return NULL;
    }
    cJSON *tmpobj = cJSON_Parse(buffer);
    if (tmpobj)
    {
    }
    if (buffer != NULL)
        free(buffer);
    return tmpobj;
}
#endif

static int web_menu_get(Webs *wp, char *path, char *query)
{
#if ME_GOAHEAD_JSON
	cJSON *obj = web_resource_load(WEBGUI_VUEMENU);
	if(obj)
		websResponseJson(wp, HTTP_CODE_OK, obj);
	else
#endif
		websResponse(wp, HTTP_CODE_NOT_FOUND, NULL);	
	return OK;
}

static int web_menu_route_get(Webs *wp, char *path, char *query)
{
#if ME_GOAHEAD_JSON
	cJSON *array = NULL;
	array = web_resource_load(WEBGUI_VUEROUTE);
	if(array)
		websResponseJson(wp, HTTP_CODE_OK, array);
	else
#endif
		websResponse(wp, HTTP_CODE_NOT_FOUND, NULL);	
	return OK;
}


int web_html_menu_init(void)
{
	websFormDefine("menu", web_menu_get);
	websFormDefine("router", web_menu_route_get);
	return 0;
}

