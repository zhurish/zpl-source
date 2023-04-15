/*
 * web_factory_html.c
 *
 *  Created on: 2019年8月25日
 *      Author: zhurish
 */

#define HAS_BOOL 1
#include "zplos_include.h"
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
#include "nsm_client.h"

#include "web_util.h"
#include "web_jst.h"
#include "web_app.h"
#include "web_api.h"

#ifdef ZPL_APP_MODULE
#include "application.h"

#ifdef APP_X5BA_MODULE
static int web_factory_action(Webs *wp, char *path, char *query)
{
	int ret = 0;
	char *strval = NULL;
	x5b_app_global_t glinfo;
	strval = webs_get_var(wp, T("ACTION"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	if(strstr(strval, "GET"))
	{
		char customizer[128];
		websSetStatus(wp, 200);
		websWriteHeaders(wp, -1, 0);
		websWriteHeader(wp, "Content-Type", "application/json");
		websWriteEndHeaders(wp);

		memset(customizer, 0, sizeof(customizer));
		if(x5b_app_customizer() == CUSTOMIZER_SECOM)
			sprintf(customizer, "SECOM");
		else if(x5b_app_customizer() == CUSTOMIZER_HUIFU)
			sprintf(customizer, "HUIFU");
		else
			sprintf(customizer, "NONE");

		websWrite(wp,
			"{\"response\":\"%s\", \"device\":\"%s\", \"location\":\"%s\", \"server\":\"%s\",\
			\"product\":\"%s\", \"customizer\":\"%s\", \"direction\":\"%s\", \"state\":\"%s\",\
			\"doorcontact\":%s, \"topf\":%d}",
			"OK", strlen(x5b_app_devicename_get_api()) ? x5b_app_devicename_get_api():"",
					strlen(x5b_app_location_address_get_api()) ? x5b_app_location_address_get_api():"",
					strlen(x5b_app_docking_platform_address_get_api()) ? x5b_app_docking_platform_address_get_api():"",
					x5b_app_mode_X5CM() ? "X5CM":"X5BM",
					customizer,
					x5b_app_out_direction_get_api() ? "out":"in","true",
					x5b_app_global->doorcontact ? "true":"false",
					x5b_app_global->topf);

		websDone(wp);
		return OK;
	}
	memset(&glinfo, 0, sizeof(x5b_app_global_t));
	strval = webs_get_var(wp, T("product"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	if(strstr(strval, "X5CM"))
		glinfo.X5CM = (zpl_true);
	else
		glinfo.X5CM = (zpl_false);

	strval = webs_get_var(wp, T("device"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	strcpy(glinfo.devicename, strval);

	strval = webs_get_var(wp, T("location"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	strcpy(glinfo.location_address, strval);

	strval = webs_get_var(wp, T("customizer"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	if(strstr(strval, "SECOM"))
		glinfo.customizer = (CUSTOMIZER_SECOM);
	else if(strstr(strval, "HUIFU"))
		glinfo.customizer = (CUSTOMIZER_HUIFU);
	else
		glinfo.customizer = (CUSTOMIZER_NONE);

	strval = webs_get_var(wp, T("direction"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	if(strstr(strval, "out"))
		glinfo.out_direction = (zpl_true);
	else
		glinfo.out_direction = (zpl_false);

	strval = webs_get_var(wp, T("server"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	strcpy(glinfo.docking_platform_address, strval);

	strval = webs_get_var(wp, T("doorcontact"), T(""));
	if(strstr(strval, "true"))
		glinfo.doorcontact = zpl_true;
	else
		glinfo.doorcontact = zpl_false;

	strval = webs_get_var(wp, T("topf"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	glinfo.topf = atoi(strval);

	if(glinfo.doorcontact != x5b_app_global->doorcontact)
	{
		x5b_app_global->doorcontact = glinfo.doorcontact;
		x5b_app_open_option_action(x5b_app_open, zpl_false, zpl_true);
	}

	ret = x5b_app_global_config_action(&glinfo, zpl_true);

	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	if(ret == OK)
		websWrite(wp, "%s", "OK");
	else
		websWrite(wp, "%s", "ERROR");
	websDone(wp);

	return OK;
}

static int web_openconfig_action(Webs *wp, char *path, char *query)
{
	int ret = 0;
	char *strval = NULL;
	x5b_app_open_t openconf;
	//ConfiglockType info;
	strval = webs_get_var(wp, T("ACTION"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	if(strstr(strval, "GET"))
	{
		char waitopen[32];
		char waitclose[32];
		char openhold[32];
		char wiggins[32];

		websSetStatus(wp, 200);
		websWriteHeaders(wp, -1, 0);
		websWriteHeader(wp, "Content-Type", "application/json");
		websWriteEndHeaders(wp);

		memset(waitopen, 0, sizeof(waitopen));
		memset(waitclose, 0, sizeof(waitclose));
		memset(openhold, 0, sizeof(openhold));
		memset(wiggins, 0, sizeof(wiggins));

		sprintf(waitopen, "%d", x5b_app_open->waitopen);
		sprintf(waitclose, "%d", x5b_app_open->waitclose);
		sprintf(openhold, "%d", x5b_app_open->openhold);
		sprintf(wiggins, "%d", x5b_app_open->wiggins);

		websWrite(wp,
			"{\"response\":\"%s\", \"waitopen\":\"%s\", \"openalarm\":%s, \"waitclose\":\"%s\",\
			\"openhold\":\"%s\", \"wiggins\":\"%s\", \"opentype\":\"%s\", \"outrelay\":\"%s\", \
			\"tamperalarm\":%s, \"doorcontact\":%s}",
			"OK", waitopen, x5b_app_open->openalarm ? "true":"false",
					waitclose, openhold, wiggins, itoa(x5b_app_open->opentype, 10),
					x5b_app_open->outrelay ? "HIGH":"LOW",
					x5b_app_open->tamperalarm ? "true":"false",
					x5b_app_global->doorcontact ? "true":"false");

		websDone(wp);
		return OK;
	}

	strval = webs_get_var(wp, T("waitopen"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	openconf.waitopen = atoi(strval);

	strval = webs_get_var(wp, T("openhold"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	openconf.openhold = atoi(strval);

	strval = webs_get_var(wp, T("openalarm"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	if(strstr(strval, "true"))
		openconf.openalarm = zpl_true;
	else
		openconf.openalarm = zpl_false;

	if(openconf.openalarm)
	{
		strval = webs_get_var(wp, T("waitclose"), T(""));
		if (NULL == strval)
		{
			return web_return_text_plain(wp, ERROR);
		}
		openconf.waitclose = atoi(strval);
	}
	strval = webs_get_var(wp, T("tamperalarm"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	if(strstr(strval, "true"))
		openconf.tamperalarm = zpl_true;
	else
		openconf.tamperalarm = zpl_false;

/*	strval = webs_get_var(wp, T("doorcontact"), T(""));
	if (NULL != strval)
	{
		if(strstr(strval, "true"))
			x5b_app_open.rev = zpl_true;
		else
			x5b_app_open.rev = zpl_false;
	}*/

	strval = webs_get_var(wp, T("opentype"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	openconf.opentype = atoi(strval);

	strval = webs_get_var(wp, T("outrelay"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	if(strstr(strval, "HIGH"))
		openconf.outrelay = zpl_true;
	else
		openconf.outrelay = zpl_false;

	strval = webs_get_var(wp, T("wiggins"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	openconf.wiggins = atoi(strval);

	ret = x5b_app_open_option_action(&openconf, zpl_true, zpl_true);
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	if(ret == OK)
		websWrite(wp, "%s", "OK");
	else
		websWrite(wp, "%s", "ERROR");
	websDone(wp);
	return OK;
}

static int web_faceconfig_get(Webs *wp, char *path, char *query)
{
	char tmp[128];
	char cmdbuf[2048];

	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "application/json");
	websWriteEndHeaders(wp);

	memset(tmp, 0, sizeof(tmp));
	memset(cmdbuf, 0, sizeof(cmdbuf));

	sprintf(cmdbuf, "{\"response\":\"OK\",");
	sprintf(tmp, "\"record_threshold\":\"%.3f\",", x5b_app_face->similarRecord);
	strcat(cmdbuf, tmp);

	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "\"recognize_threshold\":\"%.3f\",", x5b_app_face->similarRecognize);
	strcat(cmdbuf, tmp);

	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "\"recognize_sec_threshold\":\"%.3f\",", x5b_app_face->similarSecRecognize);
	strcat(cmdbuf, tmp);

	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "\"living_detection\":%s,", x5b_app_face->livenessSwitch ? "true":"false");
	strcat(cmdbuf, tmp);

	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "\"living_threshold\":\"%.3f\",", x5b_app_face->similarLiving);
	strcat(cmdbuf, tmp);

	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "\"succeszpl_intervals\":\"%d\",", x5b_app_face->faceOKContinuousTime);
	strcat(cmdbuf, tmp);

	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "\"failure_intervals\":\"%d\",", x5b_app_face->faceERRContinuousTime);
	strcat(cmdbuf, tmp);


	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "\"yaw_left\":\"%d\",\"yaw_right\":\"%d\", ", x5b_app_face->faceYawLeft, x5b_app_face->faceYawRight);
	strcat(cmdbuf, tmp);

	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "\"pitch_up\":\"%d\",\"pitch_down\":\"%d\", ", x5b_app_face->facePitchUp, x5b_app_face->faceYawRight);
	strcat(cmdbuf, tmp);

	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "\"roi_width\":\"%d\",\"roi_height\":\"%d\", ", x5b_app_face->faceRoiWidth, x5b_app_face->faceRoiHeight);
	strcat(cmdbuf, tmp);

	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "\"record_width\":\"%d\",\"record_height\":\"%d\", ", x5b_app_face->faceRecordWidth, x5b_app_face->faceRecordHeight);
	strcat(cmdbuf, tmp);

	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "\"recognize_width\":\"%d\",\"recognize_height\":\"%d\"}",
			x5b_app_face->faceRecognizeWidth, x5b_app_face->faceRecognizeHeight);
	strcat(cmdbuf, tmp);

	websWrite(wp, cmdbuf);

	websDone(wp);

	return OK;
}
static int web_faceconfig_set(Webs *wp, char *path, char *query)
{
	char *strval = NULL;
	make_face_config_t info;

	strval = webs_get_var(wp, T("record_threshold"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	//sscanf(strval, "%.3f", &info.similarRecord);
	info.similarRecord = atof(strval);
	//_WEB_DBG_TRAP("----%s----:similarRecord=%f=%s", __func__, info.similarRecord, strval);
	strval = webs_get_var(wp, T("recognize_threshold"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	//sscanf(strval, "%.3f", &info.similarRecognize);
	info.similarRecognize = atof(strval);

	strval = webs_get_var(wp, T("recognize_sec_threshold"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	info.similarSecRecognize = atof(strval);

	//_WEB_DBG_TRAP("----%s----:similarRecognize=%f=%s", __func__, info.similarRecognize, strval);
	strval = webs_get_var(wp, T("living_detection"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	if(strstr(strval, "true"))
		info.livenessSwitch = zpl_true;
	else
		info.livenessSwitch = zpl_false;

	//if(info.livenessSwitch)
	{
		strval = webs_get_var(wp, T("living_threshold"), T(""));
		if (NULL != strval)
		{
			info.similarLiving = atof(strval);
			//return web_return_text_plain(wp, ERROR);
		}
		else
			info.similarLiving = 0.0;
		//info.similarLiving = atof(strval);
		//sscanf(strval, "%.3f", &info.similarLiving);
		//_WEB_DBG_TRAP("----%s----:similarLiving=%f=%s", __func__, info.similarLiving, strval);
	}

	strval = webs_get_var(wp, T("yaw_left"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	info.faceYawLeft = atoi(strval);

	strval = webs_get_var(wp, T("yaw_right"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	info.faceYawRight = atoi(strval);

	strval = webs_get_var(wp, T("pitch_up"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	info.facePitchUp = atoi(strval);

	strval = webs_get_var(wp, T("pitch_down"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	info.facePitchDown = atoi(strval);

	strval = webs_get_var(wp, T("roi_width"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	info.faceRoiWidth = atoi(strval);

	strval = webs_get_var(wp, T("roi_height"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	info.faceRoiHeight = atoi(strval);

	strval = webs_get_var(wp, T("record_width"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	info.faceRecordWidth = atoi(strval);

	strval = webs_get_var(wp, T("record_height"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	info.faceRecordHeight = atoi(strval);

	strval = webs_get_var(wp, T("recognize_width"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	info.faceRecognizeWidth = atoi(strval);

	strval = webs_get_var(wp, T("recognize_height"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	info.faceRecognizeHeight = atoi(strval);

	strval = webs_get_var(wp, T("succeszpl_intervals"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	info.faceOKContinuousTime = atoi(strval);

	strval = webs_get_var(wp, T("failure_intervals"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	info.faceERRContinuousTime = atoi(strval);
	return x5b_app_face_config_action(&info, zpl_true);
/*	if(x5b_app_mgt && x5b_app_mode_X5CM())
	{
		if(x5b_app_face_config_json(NULL, &info, E_CMD_TO_C) == OK)
		{
			memcpy(x5b_app_face, &info, sizeof(make_face_config_t));
			return OK;
		}
	}
	else
	{
		memcpy(x5b_app_face, &info, sizeof(make_face_config_t));
		return OK;
	}*/
	return ERROR;
}

static int web_faceconfig_action(Webs *wp, char *path, char *query)
{
	char *strval = NULL;

	strval = webs_get_var(wp, T("ACTION"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	if(strstr(strval, "GET"))
	{
		if(x5b_app_face)
			return web_faceconfig_get(wp, path, query);
		else
			return OK;
	}
	if(web_faceconfig_set(wp, path, query) == OK)
	{
		return web_return_text_plain(wp, OK);
	}
	return OK;
}

#endif
#endif

int web_factory_app(void)
{
#ifdef ZPL_APP_MODULE
#ifdef APP_X5BA_MODULE
	websFormDefine("factory", web_factory_action);
	websFormDefine("open", web_openconfig_action);
	websFormDefine("faceset", web_faceconfig_action);
#endif
#endif
	return 0;
}
