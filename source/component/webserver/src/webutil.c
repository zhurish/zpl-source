/**
 * @file      : webutil.c
 * @brief     : Description
 * @author    : zhurish (zhurish@163.com)
 * @version   : 1.0
 * @date      : 2024-02-05
 * 
 * @copyright : Copyright (c) - 2024 zhurish(zhurish@163.com).Co.Ltd. All rights reserved.
 * 
 */
#define HAS_BOOL 1
#include "src/goahead.h"
#include "src/webutil.h"

static int webs_code_tohttp(int ret)
{
	int result = HTTP_CODE_OK;
	switch (ret)
	{
	case HTTP_CODE_CONTINUE:		  // 100     /**< Continue with request, only partial content transmitted */
	case HTTP_CODE_OK:					  // 200     /**< The request completed successfully */
	case HTTP_CODE_CREATED:				  // 201     /**< The request has completed and a new resource was created */
	case HTTP_CODE_ACCEPTED:				  // 202     /**< The request has been accepted and processing is continuing */
	case HTTP_CODE_NOT_AUTHORITATIVE:	  // 203     /**< The request has completed but content may be from another source */
	case HTTP_CODE_NO_CONTENT:			  // 204     /**< The request has completed and there is no response to send */
	case HTTP_CODE_RESET:				  // 205     /**< The request has completed with no content. Client must reset view */
	case HTTP_CODE_PARTIAL:				  // 206     /**< The request has completed and is returning partial content */
	case HTTP_CODE_MOVED_PERMANENTLY:	  // 301     /**< The requested URI has moved permanently to a new location */
	case HTTP_CODE_MOVED_TEMPORARILY:	  // 302     /**< The URI has moved temporarily to a new location */
	case HTTP_CODE_SEE_OTHER:			  // 303     /**< The requested URI can be found at another URI location */
	case HTTP_CODE_NOT_MODIFIED:			  // 304     /**< The requested resource has changed since the last request */
	case HTTP_CODE_USE_PROXY:			  // 305     /**< The requested resource must be accessed via the location proxy */
	case HTTP_CODE_TEMPORARY_REDIRECT:	  // 307     /**< The request should be repeated at another URI location */
	case HTTP_CODE_BAD_REQUEST:			  // 400     /**< The request is malformed */
	case HTTP_CODE_UNAUTHORIZED:			  // 401     /**< Authentication for the request has failed */
	case HTTP_CODE_PAYMENT_REQUIRED:		  // 402     /**< Reserved for future use */
	case HTTP_CODE_FORBIDDEN:			  // 403     /**< The request was legal, but the server refuses to process */
	case HTTP_CODE_NOT_FOUND:			  // 404     /**< The requested resource was not found */
	case HTTP_CODE_BAD_METHOD:			  // 405     /**< The request HTTP method was not supported by the resource */
	case HTTP_CODE_NOT_ACCEPTABLE:		  // 406     /**< The requested resource cannot generate the required content */
	case HTTP_CODE_REQUEST_TIMEOUT:		  // 408     /**< The server timed out waiting for the request to complete */
	case HTTP_CODE_CONFLICT:				  // 409     /**< The request had a conflict in the request headers and URI */
	case HTTP_CODE_GONE:					  // 410     /**< The requested resource is no longer available*/
	case HTTP_CODE_LENGTH_REQUIRED:		  // 411     /**< The request did not specify a required content length*/
	case HTTP_CODE_PRECOND_FAILED:		  // 412     /**< The server cannot satisfy one of the request preconditions */
	case HTTP_CODE_REQUEST_TOO_LARGE:	  // 413     /**< The request is too large for the server to process */
	case HTTP_CODE_REQUEST_URL_TOO_LARGE:  // 414     /**< The request URI is too long for the server to process */
	case HTTP_CODE_UNSUPPORTED_MEDIA_TYPE: // 415     /**< The request media type is not supported by the server or resource */
	case HTTP_CODE_RANGE_NOT_SATISFIABLE:  // 416     /**< The request content range does not exist for the resource */
	case HTTP_CODE_EXPECTATION_FAILED:	  // 417     /**< The server cannot satisfy the Expect header requirements */
	case HTTP_CODE_NO_RESPONSE:			  // 444     /**< The connection was closed with no response to the client */
	case HTTP_CODE_INTERNAL_SERVER_ERROR:  // 500     /**< Server processing or configuration error. No response generated */
	case HTTP_CODE_NOT_IMPLEMENTED:		  // 501     /**< The server does not recognize the request or method */
	case HTTP_CODE_BAD_GATEWAY:			  // 502     /**< The server cannot act as a gateway for the given request */
	case HTTP_CODE_SERVICE_UNAVAILABLE:	  // 503     /**< The server is currently unavailable or overloaded */
	case HTTP_CODE_GATEWAY_TIMEOUT:		  // 504     /**< The server gateway timed out waiting for the upstream server */
	case HTTP_CODE_BAD_VERSION:			  // 505     /**< The server does not support the HTTP protocol version */
	case HTTP_CODE_INSUFFICIENT_STORAGE:	  // 507     /**< The server has insufficient storage to complete the request */
		result = ret;
		break;
	default:
		break;
	}
	return result;
}

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
int web_textplain_result(Webs *wp, int ret, char *msg)
{
	web_assert(wp);
	websSetStatus (wp, webs_code_tohttp(ret));
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

int web_textplain_format_result(Webs *wp, int ret, char *fmt,...)
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

	websSetStatus (wp, webs_code_tohttp(ret));
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
int web_json_result(Webs *wp,  int ret, cJSON* json)
{
	cJSON *obj = NULL;
	web_assert(wp);
	obj = cJSON_CreateObject();
	if(obj)
	{
		cJSON_AddNumberToObject(obj,"result", ret);
		if(json)
			cJSON_AddItemToObject(obj,"message", json);	
	}
	websResponseJson(wp, webs_code_tohttp(ret), obj);
	return 0;
}

int web_json_format_result(Webs *wp, int ret, char *fmt,...)
{
    va_list     vargs;
	char tmp[1024];
	cJSON *obj = NULL;
    web_assert(websValid(wp));
    web_assert(fmt && *fmt);
	obj = cJSON_CreateObject();

    va_start(vargs, fmt);
	memset(tmp, 0, sizeof(tmp));
    vsnprintf(tmp, sizeof(tmp), fmt, vargs);
    va_end(vargs);
	if(obj)
	{
		cJSON_AddNumberToObject(obj,"result", ret);
		cJSON_AddStringToObject(obj,"message", tmp);
		websResponseJson(wp, webs_code_tohttp(ret), obj);
	}
	return 0;
}
#endif


