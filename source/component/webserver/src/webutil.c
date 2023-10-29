/*
 * webutil.c
 *
 *  Created on: 2019年8月23日
 *      Author: DELL
 */

#define HAS_BOOL 1
#include "src/goahead.h"
#include "src/webutil.h"


const char *webs_get_var(Webs *wp, const char *var, const char *defaultGetValue)
{
	web_assert(wp);
	const char *value = websGetVar(wp, var, defaultGetValue);
	if(value != NULL)
	{
		if(strlen(value) <= 0)
		{
			return NULL;
		}
		if(all_space(value))
			return NULL;
		if(strcasecmp(value, "undefined") == 0)
			return NULL;

		return strrmtrim(value);
	}
	return NULL;
}



/*
 *
 */
int web_return_text_plain(Webs *wp, int ret, char *msg)
{
	web_assert(wp);
	websSetStatus (wp, (ret==0)?200:ret);
	if(msg)
	{
		websWriteCache (wp,"%s",msg);		
		websWriteHeaders (wp, websWriteCacheLen(wp), 0);
	}	
	else
		websWriteHeaders (wp, 0, 0);
	websWriteHeader (wp, "Content-Type", "text/plain");
	websWriteEndHeaders (wp);	
	websWriteCacheFinsh(wp);
	websDone (wp);	
	return 0;
}

int web_return_text_fmt(Webs *wp, int ret, char *fmt,...)
{
    va_list     vargs;
    ssize       rc;
	char tmp[1024];
    web_assert(websValid(wp));
    web_assert(fmt && *fmt);

    va_start(vargs, fmt);
	memset(tmp, 0, sizeof(tmp));
    rc = vsnprintf(tmp, sizeof(tmp), fmt, vargs);
    va_end(vargs);

	websSetStatus (wp, (ret==0)?200:ret);
	if(rc)
	{
		websWriteCache (wp,"%s",tmp);		
		websWriteHeaders (wp, websWriteCacheLen(wp), 0);
	}	
	else
		websWriteHeaders (wp, 0, 0);
	websWriteHeader (wp, "Content-Type", "text/plain");
	websWriteEndHeaders (wp);	
	websWriteCacheFinsh(wp);
	websDone (wp);	
	return 0;
}

#if ME_GOAHEAD_JSON
int web_return_application_json(Webs *wp,  int ret, cJSON* json)
{
	char *jsonstr = NULL;	
	web_assert(wp);
	if(json)
		jsonstr = cJSON_Print(json);	
	websSetStatus (wp, (ret==0)?200:ret);
	if(jsonstr)
	{
		websWriteCache (wp,"%s",jsonstr);		
		websWriteHeaders (wp, websWriteCacheLen(wp), 0);
		cJSON_PrintFree(jsonstr);
	}	
	else
		websWriteHeaders (wp, 0, 0);
	websWriteHeader (wp, "Content-Type", "application/json");
	websWriteEndHeaders (wp);	
	websWriteCacheFinsh(wp);
	websDone (wp);	
	return 0;
}

int web_return_application_json_fmt(Webs *wp, int ret, char *fmt,...)
{
    va_list     vargs;
    ssize       rc;
	char tmp[1024];
    web_assert(websValid(wp));
    web_assert(fmt && *fmt);

    va_start(vargs, fmt);
	memset(tmp, 0, sizeof(tmp));
    rc = vsnprintf(tmp, sizeof(tmp), fmt, vargs);
    va_end(vargs);

	websSetStatus (wp, (ret==0)?200:ret);
	if(rc)
	{
		websWriteCache(wp,"{\"result\":%d, \"message\":\"%s\"}", ret, tmp);	
		websWriteHeaders (wp, websWriteCacheLen(wp), 0);
	}	
	else
		websWriteHeaders (wp, 0, 0);
	websWriteHeader (wp, "Content-Type", "application/json");
	websWriteEndHeaders (wp);	
	websWriteCacheFinsh(wp);
	websDone (wp);	
	return 0;
}
#endif


