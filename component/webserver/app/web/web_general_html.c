/*
 * web_general_html.c
 *
 *  Created on: Apr 13, 2019
 *      Author: zhurish
 */

#define HAS_BOOL 1
#include "zpl_include.h"
#include "module.h"
#include "memory.h"
#include "zassert.h"
#include "command.h"
#include "prefix.h"
#include "host.h"
#include "log.h"
#include "vty.h"
#include "vty_user.h"


#include "web_util.h"
#include "web_jst.h"
#include "web_app.h"
#include "web_api.h"

//#ifdef ZPL_APP_MODULE
#include "application.h"



#if 0
static int web_video_general_get(Webs *wp, char *path, char *query)
{
	web_assert(wp);
	//char customizer[128];
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "application/json");
	websWriteEndHeaders(wp);

/*
	memset(customizer, 0, sizeof(customizer));
	if(x5b_app_customizer() == CUSTOMIZER_SECOM)
		sprintf(customizer, "SECOM");
	else if(x5b_app_customizer() == CUSTOMIZER_HUIFU)
		sprintf(customizer, "HUIFU");
	else
		sprintf(customizer, "NONE");
*/

	websWrite(wp,
		"{\"response\":\"%s\", \"mode\":\"%s\", \"frme\":\"%s\", \"qulity\":\"%s\", \"upload_qulity\":\"%s\",\
		\"smoothness\":%s, \"snapsrc\":%s, \"detection_module\":%s, \"upload_module\":%s}",
		"OK", "snap", "10", "60", "60", "true", "true", "false", "true");
	//snap 抓拍方案
	//reco 识别方案
	//helmet 安全帽方案
	websDone(wp);
	return OK;
}

static int web_video_general_handle(Webs *wp, char *path, char *query)
{
	int ret = 0;
	char *strval = NULL;
	web_assert(wp);
	strval = webs_get_var(wp, T("ACTION"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	if(strstr(strval, "GET"))
	{
		return web_video_general_get(wp, path, query);
	}

	strval = webs_get_var(wp, T("mode"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}

	strval = webs_get_var(wp, T("frme"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}

	strval = webs_get_var(wp, T("quality"), T(""));
	if (NULL != strval)
	{
		//strcpy(stream.username, strval);
	}
	strval = webs_get_var(wp, T("upload_qulity"), T(""));
	if (NULL != strval)
	{
		//strcpy(stream.password, strval);
	}

	strval = webs_get_var(wp, T("smoothness"), T(""));
	if (NULL != strval)
	{
		//stream.port = atoi(strval);
	}
	strval = webs_get_var(wp, T("snapsrc"), T(""));
	if (NULL != strval)
	{
		//stream.fps = atoi(strval);
	}
	strval = webs_get_var(wp, T("detection_module"), T(""));
	if (NULL != strval)
	{
		//stream.id = atoi(strval);
	}
	strval = webs_get_var(wp, T("upload_module"), T(""));
	if (NULL != strval)
	{
		//stream.id = atoi(strval);
	}

	ret =  OK;
	if(ret == OK)
		return web_return_text_plain(wp, OK);
	else
		return web_return_text_plain(wp, ERROR);
}
#endif


int web_general_app(void)
{
	//websFormDefine("general", web_video_general_handle);
	return 0;
}
//#endif
