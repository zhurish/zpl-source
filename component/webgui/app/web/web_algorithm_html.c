/*
 * web_algorithm_html.c
 *
 *  Created on: Apr 13, 2019
 *      Author: zhurish
 */

#define HAS_BOOL 1
#include "zebra.h"
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

//#ifdef PL_APP_MODULE
#include "application.h"


static int web_video_detecting_get(Webs *wp, char *path, char *query)
{
	char *strval = NULL;
	u_int32 id = 0;
	u_int32 ch = 0;
	strval = webs_get_var(wp, T("ID"), T(""));
	if (NULL == strval)
	{
		zlog_warn(ZLOG_APP," can not get ID");
		return web_return_text_plain(wp, ERROR);
	}
	id = atoi(strval);
	strval = webs_get_var(wp, T("ch"), T(""));
	if (NULL == strval)
	{
		zlog_warn(ZLOG_APP," can not get CH");
		return web_return_text_plain(wp, ERROR);
	}
	ch = atoi(strval);
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "application/json");
	websWriteEndHeaders(wp);

#ifdef V9_VIDEO_SDK_API
	ST_SDKSnapInfo stSnapInfo;
	memset(&stSnapInfo, 0, sizeof(ST_SDKSnapInfo));
	if(v9_video_sdk_snap_config_get_api(APP_BOARD_CALCU_ID(id), ch, &stSnapInfo) == OK)
	{
		websWrite(wp,
			"{\"response\":\"%s\", \"ID\":\"%d\", \"ch\":\"%d\", \"time\":\"%d\", \"facemaxpx\":\"%d\", \"faceminpx\":\"%d\",\
			\"qulityscore\":\"%d\", \"confi\":\"%d\", \"mode\":\"%d\"}",
			"OK", APP_BOARD_CALCU_ID(id), ch, stSnapInfo.nIntervalTime, stSnapInfo.nMaxSize,
			stSnapInfo.nMinSize, stSnapInfo.nQulityScore, stSnapInfo.nConfi, stSnapInfo.nSnapMode-1);
	}
	else
	{
		zlog_warn(ZLOG_APP," can not get v9_video_sdk_snap_config_get_api");
		websWrite(wp,
			"{\"response\":\"%s\", \"ID\":\"%s\", \"ch\":\"%s\", \"time\":\"%s\", \"facemaxpx\":\"%s\", \"faceminpx\":\"%s\",\
			\"qulityscore\":\"%s\", \"confi\":\"%s\", \"mode\":\"%s\"}",
			"ERROR", "2", "1", "60", "1080", "720", "85", "90", "2");
	}
#else
	websWrite(wp,
		"{\"response\":\"%s\", \"ID\":\"%s\", \"ch\":\"%s\", \"time\":\"%s\", \"facemaxpx\":\"%s\", \"faceminpx\":\"%s\",\
		\"qulityscore\":\"%s\", \"confi\":\"%s\", \"mode\":\"%s\"}",
		"OK", "2", "1", "60", "1080", "720", "85", "90", "2");
#endif
	websDone(wp);
	return OK;
}

static int web_video_detecting_handle(Webs *wp, char *path, char *query)
{
	int ret = 0;
	char *strval = NULL;
	u_int32 id = 0;
	u_int32 ch = 0;
#ifdef V9_VIDEO_SDK_API
	ST_SDKSnapInfo stSnapInfo;
	ST_SDKSnapInfo stSnapInfoOld;
	memset(&stSnapInfo, 0, sizeof(ST_SDKSnapInfo));
	memset(&stSnapInfoOld, 0, sizeof(ST_SDKSnapInfo));
#endif
	strval = webs_get_var(wp, T("ACTION"), T(""));
	if (NULL == strval)
	{
		zlog_warn(ZLOG_APP," can not get ACTION");
		return web_return_text_plain(wp, ERROR);
	}
	if(strstr(strval, "GET"))
	{
		return web_video_detecting_get(wp, path, query);
	}

	strval = webs_get_var(wp, T("ID"), T(""));
	if (NULL == strval)
	{
		zlog_warn(ZLOG_APP," can not get ID");
		return web_return_text_plain(wp, ERROR);
	}
	id = atoi(strval);
	strval = webs_get_var(wp, T("ch"), T(""));
	if (NULL == strval)
	{
		zlog_warn(ZLOG_APP," can not get CH");
		return web_return_text_plain(wp, ERROR);
	}
	ch = atoi(strval);
#ifdef V9_VIDEO_SDK_API
	if(v9_video_sdk_snap_config_get_api(APP_BOARD_CALCU_ID(id), ch, &stSnapInfoOld) == OK)
	{
		//return web_return_text_plain(wp, ERROR);
		memcpy(&stSnapInfo, &stSnapInfoOld, sizeof(ST_SDKSnapInfo));
	}
	else
		memset(&stSnapInfo, 0, sizeof(ST_SDKSnapInfo));
#endif
	strval = webs_get_var(wp, T("mode"), T(""));
	if (NULL != strval)
	{
#ifdef V9_VIDEO_SDK_API
		stSnapInfo.nSnapMode = atoi(strval) - 1;
#endif
	}

	strval = webs_get_var(wp, T("time"), T(""));
	if (NULL != strval)
	{
#ifdef V9_VIDEO_SDK_API
		stSnapInfo.nIntervalTime = atoi(strval);
#endif
	}
	strval = webs_get_var(wp, T("facemaxpx"), T(""));
	if (NULL != strval)
	{
#ifdef V9_VIDEO_SDK_API
		stSnapInfo.nMaxSize = atoi(strval);
#endif
	}
	strval = webs_get_var(wp, T("faceminpx"), T(""));
	if (NULL != strval)
	{
#ifdef V9_VIDEO_SDK_API
		stSnapInfo.nMinSize = atoi(strval);
#endif
	}
	strval = webs_get_var(wp, T("qulityscore"), T(""));
	if (NULL != strval)
	{
#ifdef V9_VIDEO_SDK_API
		stSnapInfo.nQulityScore = atoi(strval);
#endif
	}
	strval = webs_get_var(wp, T("confi"), T(""));
	if (NULL != strval)
	{
#ifdef V9_VIDEO_SDK_API
		stSnapInfo.nConfi = atoi(strval);
#endif
	}
#ifdef V9_VIDEO_SDK_API
	zlog_warn(ZLOG_APP," board ID=%d nSnapMode=%d nIntervalTime=%d nMaxSize=%d nMinSize=%d nQulityScore=%d nConfi=%d",
			  APP_BOARD_CALCU_ID(id), stSnapInfo.nSnapMode, stSnapInfo.nIntervalTime, stSnapInfo.nMaxSize, stSnapInfo.nMinSize,
			  stSnapInfo.nQulityScore, stSnapInfo.nConfi);
	if(v9_video_sdk_snap_config_set_api(id, ch, &stSnapInfo) != OK)
	{
		return web_return_text_plain(wp, ERROR);
	}
	ret =  OK;
#else
	ret =  OK;
#endif
	if(ret == OK)
		return web_return_text_plain(wp, OK);
	else
		return web_return_text_plain(wp, ERROR);
}

/****************************************************************************/
static int web_video_identify_get(Webs *wp, char *path, char *query)
{
	int nOutSimilarity = 0;
	int nRegisterQuality = 0;
	BOOL nOpenUpload = FALSE;
	char *strval = NULL;
	u_int32 id = 0;
	//u_int32 ch = 0;
	strval = webs_get_var(wp, T("ID"), T(""));
	if (NULL == strval)
	{
		zlog_warn(ZLOG_APP," can not get ID");
		return web_return_text_plain(wp, ERROR);
	}
	id = atoi(strval);
/*	strval = webs_get_var(wp, T("ch"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	ch = atoi(strval);*/
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "application/json");
	websWriteEndHeaders(wp);
#ifdef V9_VIDEO_SDK_API
	if(v9_video_sdk_recognize_config_get_api(APP_BOARD_CALCU_ID(id), &nOutSimilarity,
										  &nRegisterQuality, &nOpenUpload) == OK)
	{
		websWrite(wp,
			"{\"response\":\"%s\", \"ID\":\"%d\", \"ch\":\"%d\", \"de_threshold\":\"%d\", \"in_threshold\":\"%d\", \"opt_threshold\":\"%d\",\
			\"uploadcheck\":%s}",
			"OK", APP_BOARD_CALCU_ID(id), 0, nOutSimilarity, nRegisterQuality, "80", nOpenUpload ? "true":"false");
	}
	else
	{
		zlog_warn(ZLOG_APP," can not get v9_video_sdk_recognize_config_get_api");
		websWrite(wp,
			"{\"response\":\"%s\", \"ID\":\"%s\", \"ch\":\"%s\", \"de_threshold\":\"%s\", \"in_threshold\":\"%s\", \"opt_threshold\":\"%s\",\
			\"uploadcheck\":%s}",
			"ERROR", "2", "1", "60", "70", "80", "true");
	}
#else
	websWrite(wp,
		"{\"response\":\"%s\", \"ID\":\"%s\", \"ch\":\"%s\", \"de_threshold\":\"%s\", \"in_threshold\":\"%s\", \"opt_threshold\":\"%s\",\
		\"uploadcheck\":%s}",
		"OK", "2", "1", "60", "70", "80", "true");
#endif
	websDone(wp);
	return OK;
}

static int web_video_identify_handle(Webs *wp, char *path, char *query)
{
	int ret = 0;
	char *strval = NULL;
	u_int32 id = 0;
	//u_int32 ch = 0;
	int nOutSimilarity = 0;
	int nRegisterQuality = 0;
	BOOL nOpenUpload = FALSE;
	strval = webs_get_var(wp, T("ACTION"), T(""));
	if (NULL == strval)
	{
		zlog_warn(ZLOG_APP," can not get ACTION");
		return web_return_text_plain(wp, ERROR);
	}
	if(strstr(strval, "GET"))
	{
		return web_video_identify_get(wp, path, query);
	}
	strval = webs_get_var(wp, T("ID"), T(""));
	if (NULL == strval)
	{
		zlog_warn(ZLOG_APP," can not get ID");
		return web_return_text_plain(wp, ERROR);
	}
	id = atoi(strval);
/*
	strval = webs_get_var(wp, T("ch"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	ch = atoi(strval);
*/

	strval = webs_get_var(wp, T("de_threshold"), T(""));
	if (NULL != strval)
	{
		nOutSimilarity = atoi(strval);
	}

	strval = webs_get_var(wp, T("in_threshold"), T(""));
	if (NULL != strval)
	{
		nRegisterQuality = atoi(strval);
	}
	strval = webs_get_var(wp, T("opt_threshold"), T(""));
	if (NULL != strval)
	{
		//stream.fps = atoi(strval);
	}
	strval = webs_get_var(wp, T("uploadcheck"), T(""));
	if (NULL != strval)
	{
		nOpenUpload = strstr(strval,"true") ? 1:0;
	}
#ifdef V9_VIDEO_SDK_API
	zlog_warn(ZLOG_APP," board ID=%d nOutSimilarity=%d nRegisterQuality=%d nOpenUpload=%d",
			  APP_BOARD_CALCU_ID(id), nOutSimilarity, nRegisterQuality, nOpenUpload);
	ret = v9_video_sdk_recognize_config_set_api(APP_BOARD_CALCU_ID(id), nOutSimilarity,
										  nRegisterQuality, nOpenUpload);
#else
	ret =  OK;
#endif
	if(ret == OK)
		return web_return_text_plain(wp, OK);
	else
		return web_return_text_plain(wp, ERROR);
}

/****************************************************************************/
static int web_video_helmet_get(Webs *wp, char *path, char *query)
{
	//int ret = 0;
	char *strval = NULL;
	u_int32 id = 0;
	u_int32 ch = 0;
	strval = webs_get_var(wp, T("ID"), T(""));
	if (NULL == strval)
	{
		zlog_warn(ZLOG_APP," can not get ID");
		return web_return_text_plain(wp, ERROR);
	}
	id = atoi(strval);
	strval = webs_get_var(wp, T("ch"), T(""));
	if (NULL == strval)
	{
		zlog_warn(ZLOG_APP," can not get CH");
		return web_return_text_plain(wp, ERROR);
	}
	ch = atoi(strval);
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "application/json");
	websWriteEndHeaders(wp);

#ifdef V9_VIDEO_SDK_API
	ST_SDKHelmetInfo HelmetInfo;
	memset(&HelmetInfo, 0, sizeof(ST_SDKHelmetInfo));
	if(v9_video_sdk_helmet_config_get_api(APP_BOARD_CALCU_ID(id), ch, &HelmetInfo) == OK)
	{
		websWrite(wp,
			"{\"response\":\"%s\", \"ID\":\"%d\", \"ch\":\"%d\", \"sendpic\":%s, \"trace\":%s, \"frame\":%s,\
			\"quality\":\"%d\", \"snap_interval\":\"%d\", \"warn_interval\":\"%d\", \"snap_qulityscore\":\"%d\",\
			\"threshold\":\"%d\"}",
			"OK", APP_BOARD_CALCU_ID(id), ch,
			HelmetInfo.nSentImage ? "true":"false",
			HelmetInfo.nUseTracking ? "true":"false",
			HelmetInfo.nDrawRectangle ? "true":"false",
			HelmetInfo.nImageRatio, HelmetInfo.nSnapInterval,
			HelmetInfo.nAlarmInterval, HelmetInfo.nSnapRatio,
			HelmetInfo.nThreshold);
	}
	else
	{
		zlog_warn(ZLOG_APP," can not get v9_video_sdk_helmet_config_get_api");
		websWrite(wp,
			"{\"response\":\"%s\", \"ID\":\"%s\", \"ch\":\"%s\", \"sendpic\":%s, \"trace\":%s, \"frame\":%s,\
			\"quality\":\"%s\", \"snap_interval\":\"%s\", \"warn_interval\":\"%s\", \"snap_qulityscore\":\"%s\",\
			\"threshold\":\"%s\"}",
			"OK", "2", "1", "true", "true", "true", "80", "85", "90", "95", "96");
	}
#else
	websWrite(wp,
		"{\"response\":\"%s\", \"ID\":\"%s\", \"ch\":\"%s\", \"sendpic\":%s, \"trace\":%s, \"frame\":%s,\
		\"quality\":\"%s\", \"snap_interval\":\"%s\", \"warn_interval\":\"%s\", \"snap_qulityscore\":\"%s\",\
		\"threshold\":\"%s\"}",
		"OK", "2", "1", "true", "true", "true", "80", "85", "90", "95", "96");
#endif

	websDone(wp);
	return OK;
}

static int web_video_helmet_handle(Webs *wp, char *path, char *query)
{
	int ret = 0;
	char *strval = NULL;
	u_int32 id = 0;
	u_int32 ch = 0;
#ifdef V9_VIDEO_SDK_API
	ST_SDKHelmetInfo stSnapInfo;
	ST_SDKHelmetInfo stSnapInfoOld;
	memset(&stSnapInfo, 0, sizeof(ST_SDKHelmetInfo));
	memset(&stSnapInfoOld, 0, sizeof(ST_SDKHelmetInfo));
#endif
	strval = webs_get_var(wp, T("ACTION"), T(""));
	if (NULL == strval)
	{
		zlog_warn(ZLOG_APP," can not get ACTION");
		return web_return_text_plain(wp, ERROR);
	}
	if(strstr(strval, "GET"))
	{
		return web_video_helmet_get(wp, path, query);
	}

	strval = webs_get_var(wp, T("ID"), T(""));
	if (NULL == strval)
	{
		zlog_warn(ZLOG_APP," can not get ID");
		return web_return_text_plain(wp, ERROR);
	}
	id = (atoi(strval));
	strval = webs_get_var(wp, T("ch"), T(""));
	if (NULL == strval)
	{
		zlog_warn(ZLOG_APP," can not get CH");
		return web_return_text_plain(wp, ERROR);
	}
	ch = atoi(strval);
#ifdef V9_VIDEO_SDK_API
	if(v9_video_sdk_helmet_config_get_api(APP_BOARD_CALCU_ID(id), ch, &stSnapInfoOld) != OK)
	{
		zlog_warn(ZLOG_APP," can not get v9_video_sdk_helmet_config_get_api");
		return web_return_text_plain(wp, ERROR);
	}
	memcpy(&stSnapInfo, &stSnapInfoOld, sizeof(ST_SDKHelmetInfo));
#endif
	strval = webs_get_var(wp, T("quality"), T(""));
	if (NULL != strval)
	{
#ifdef V9_VIDEO_SDK_API
		stSnapInfo.nImageRatio = atoi(strval);
#endif
	}

	strval = webs_get_var(wp, T("snap_interval"), T(""));
	if (NULL != strval)
	{
#ifdef V9_VIDEO_SDK_API
		stSnapInfo.nSnapInterval = atoi(strval);
#endif
	}
	strval = webs_get_var(wp, T("warn_interval"), T(""));
	if (NULL != strval)
	{
#ifdef V9_VIDEO_SDK_API
		stSnapInfo.nAlarmInterval = atoi(strval);
#endif
	}
	strval = webs_get_var(wp, T("snap_qulityscore"), T(""));
	if (NULL != strval)
	{
#ifdef V9_VIDEO_SDK_API
		stSnapInfo.nSnapRatio = atoi(strval);
#endif
	}
	strval = webs_get_var(wp, T("threshold"), T(""));
	if (NULL != strval)
	{
#ifdef V9_VIDEO_SDK_API
		stSnapInfo.nThreshold = atoi(strval);
#endif
	}

	strval = webs_get_var(wp, T("sendpic"), T(""));
	if (NULL != strval)
	{
#ifdef V9_VIDEO_SDK_API
		stSnapInfo.nSentImage = strstr(strval,"true") ? 1:0;
#endif
	}
	strval = webs_get_var(wp, T("trace"), T(""));
	if (NULL != strval)
	{
#ifdef V9_VIDEO_SDK_API
		stSnapInfo.nUseTracking = strstr(strval,"true") ? 1:0;
#endif
	}
	strval = webs_get_var(wp, T("frame"), T(""));
	if (NULL != strval)
	{
#ifdef V9_VIDEO_SDK_API
		stSnapInfo.nDrawRectangle = strstr(strval,"true") ? 1:0;
#endif
	}
#ifdef V9_VIDEO_SDK_API
	zlog_warn(ZLOG_APP," board ID=%d nImageRatio=%d nSnapInterval=%d nAlarmInterval=%d nSnapRatio=%d nThreshold=%d nSentImage=%d nUseTracking=%d nDrawRectangle=%d",
			  APP_BOARD_CALCU_ID(id), stSnapInfo.nImageRatio, stSnapInfo.nSnapInterval,
			  stSnapInfo.nAlarmInterval, stSnapInfo.nSnapRatio, stSnapInfo.nThreshold, stSnapInfo.nSentImage,
			  stSnapInfo.nUseTracking, stSnapInfo.nDrawRectangle);
	if(v9_video_sdk_helmet_config_set_api(APP_BOARD_CALCU_ID(id), ch, &stSnapInfo) != OK)
	{
		return web_return_text_plain(wp, ERROR);
	}
	ret =  OK;
#else
	ret =  OK;
#endif
	if(ret == OK)
		return web_return_text_plain(wp, OK);
	else
		return web_return_text_plain(wp, ERROR);
}
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
int web_algorithm_app(void)
{
	websFormDefine("detecting", web_video_detecting_handle);
	websFormDefine("identify", web_video_identify_handle);
	websFormDefine("helmet", web_video_helmet_handle);
	return 0;
}
//#endif
