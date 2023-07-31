/*
 * pjsip_jsoncfg.c
 *
 *  Created on: Feb 2, 2019
 *      Author: zhurish
 */

/* $Id: main.c 4752 2014-02-19 08:57:22Z ming $ */
/*
 * Copyright (C) 2008-2011 Teluu Inc. (http://www.teluu.com)
 * Copyright (C) 2003-2008 Benny Prijono <benny@prijono.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "pjsua_app.h"
#include "pjsip_app_api.h"
#include "pjsip_jsoncfg.h"

#include "cJSON.h"

#if 0
extern void *cjson_malloc (zpl_size_t size);
extern void cjson_free (void *ptr);

#define THIS_FILE "pjsip_jsoncfg.c"

#define PJ_JSON_READ_INT(c, o, v)               \
		if (cJSON_HaveObject(o, #v))           \
			c->v = cJSON_GetIntValue(o, #v); 
#define PJ_JSON_READ_BOOL(c, o, v)               \
		if (cJSON_HaveObject(o, #v))             \
			c->v = cJSON_GetBoolValue(o, #v); 

#define PJ_JSON_READ_DOUBLE(c, o, v)               \
		if (cJSON_HaveObject(o, #v))               \
			c->v = cJSON_GetDoubleValue(o, #v); 

#define PJ_JSON_READ_STRING(c, o, v)      \
		if (cJSON_HaveObject(o, #v))  { \
			if (cJSON_GetStringValue(o, #v)) \
				c->v = strdup(cJSON_GetStringValue(o, #v)); \
			else \
				c->v = NULL;\
		}

#define PJ_JSON_READ_INT_ARRAY(c, o, v)                              \
		if (cJSON_HaveObject(o, #v))                                 \
			_json_read_string_array_obj(true, o, #v, NULL, c->v);

#define PJ_JSON_READ_STRING_ARRAY(c, o, v)                            \
		if (cJSON_HaveObject(o, #v))                                  \
			_json_read_string_array_obj(false, o, #v, c->v, NULL);

#define PJ_JSON_WRITE_INT(c, o, v)                 \
		cJSON_AddNumberToObject(o, #v, c->v);

#define PJ_JSON_WRITE_BOOL(c, o, v)            \
		cJSON_AddBoolToObject(o, #v, c->v); 

#define PJ_JSON_WRITE_DOUBLE(c, o, v)            \
		cJSON_AddNumberToObject(o, #v, c->v); 

#define PJ_JSON_WRITE_STRING(c, o, v)            \
		if (c->v != NULL)                                \
			cJSON_AddStringToObject(o, #v, c->v);

#define PJ_JSON_WRITE_INT_ARRAY(c, o, v, n)                              \
		if (c->v != NULL && n > 0)                                                    \
			_json_write_string_array_obj(true, o, #v, NULL, c->v, n);

#define PJ_JSON_WRITE_STRING_ARRAY(c, o, v, n)                            \
		if (c->v != NULL && n > 0)                                                     \
			_json_write_string_array_obj(false, o, #v, c->v, NULL, n);

#define PJ_JSON_GET_ARRAY_SIZE(c, o, v)                   \
		_json_read_array_obj_size(o, #v); 


static int
_json_read_array_obj_size(cJSON *obj, const char *name)
{
	int i = 0;
	cJSON *array_obj = cJSON_GetObjectItem(obj, name); //
	if (array_obj)
	{
		int array_size = cJSON_GetArraySize(array_obj);
		return array_size;
	}
	return i;
}
static int
_json_read_string_array_obj(bool intval, cJSON *obj, const char *name,
							char *value[], int *intvalue)
{
	int i = 0;
	cJSON *array_obj = cJSON_GetObjectItem(obj, name); //
	if (array_obj)
	{
		int array_size = cJSON_GetArraySize(array_obj); //
		for (i = 0; i < array_size; i++)
		{
			cJSON *array_item = cJSON_GetArrayItem(array_obj, i); //
			if (array_item)
			{
				if (intval && intvalue)
				{
					intvalue[i] = (array_item->valueint);
				}
				else
				{
					if (array_item->valuestring && value)
						value[i] = strdup(array_item->valuestring); //
				}
			}
		}
		return array_size;
	}
	return i;
}

static void
_json_write_string_array_obj(bool intval, cJSON *obj, const char *name,
							 char *value[], int *intvalue, int cnt)
{
	if (cnt <= 0)
		return;
	cJSON *array_item = NULL;
	if(cnt <= 0)
		return;
	if (intval && intvalue)
	{
		array_item = cJSON_CreateIntArray(intvalue, cnt);
	}
	else if (!intval && value)
		array_item = cJSON_CreateStringArray(value, cnt);

	if (/*array_obj && */ array_item)
	{
		cJSON_AddItemToObject(obj, name, array_item);
		//cJSON_Delete(array_item);
	}
}

static int
pjsip_ua_config_json_read_obj(cJSON *obj, pjsip_ua_config_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_READ_INT(ua, obj, maxCalls);
		PJ_JSON_READ_INT(ua, obj, threadCnt);
		PJ_JSON_READ_BOOL(ua, obj, mainThreadOnly);
		PJ_JSON_READ_STRING(ua, obj, userAgent);

		PJ_JSON_READ_BOOL(ua, obj, stunTryIpv6);
		PJ_JSON_READ_BOOL(ua, obj, stunIgnoreFailure);
		PJ_JSON_READ_INT(ua, obj, natTypeInSdp);
		PJ_JSON_READ_BOOL(ua, obj, mwiUnsolicitedEnabled);
		ua->nameserverCnt = PJ_JSON_GET_ARRAY_SIZE(ua, obj, nameserver);
		ua->outboundProxiesCnt = PJ_JSON_GET_ARRAY_SIZE(ua, obj, outboundProxies);
		ua->stunServerCnt = PJ_JSON_GET_ARRAY_SIZE(ua, obj, stunServer);
		if(ua->nameserverCnt > 0 && ua->nameserverCnt <= PJSIP_NAMESERVER_MAX)
			PJ_JSON_READ_STRING_ARRAY(ua, obj, nameserver);
		if(ua->outboundProxiesCnt > 0 && ua->outboundProxiesCnt <= PJSIP_OUTBOUND_PROXY_MAX)
			PJ_JSON_READ_STRING_ARRAY(ua, obj, outboundProxies);
		if(ua->stunServerCnt > 0 && ua->stunServerCnt <= PJSIP_STUNSERVER_MAX)
			PJ_JSON_READ_STRING_ARRAY(ua, obj, stunServer);
		return OK;
	}
	return ERROR;
}

static int
pjsip_ua_config_json_write_obj(cJSON *obj, pjsip_ua_config_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_WRITE_INT(ua, obj, maxCalls);
		PJ_JSON_WRITE_INT(ua, obj, threadCnt);
		PJ_JSON_WRITE_BOOL(ua, obj, mainThreadOnly);
		PJ_JSON_WRITE_STRING(ua, obj, userAgent);

		PJ_JSON_WRITE_BOOL(ua, obj, stunTryIpv6);
		PJ_JSON_WRITE_BOOL(ua, obj, stunIgnoreFailure);
		PJ_JSON_WRITE_INT(ua, obj, natTypeInSdp);
		PJ_JSON_WRITE_BOOL(ua, obj, mwiUnsolicitedEnabled);

		PJ_JSON_WRITE_STRING_ARRAY(ua, obj, nameserver, ua->nameserverCnt);
		PJ_JSON_WRITE_STRING_ARRAY(ua, obj, outboundProxies,
								   ua->outboundProxiesCnt);
		PJ_JSON_WRITE_STRING_ARRAY(ua, obj, stunServer, ua->stunServerCnt);
		return OK;
	}
	return ERROR;
}

static int
pjsip_log_config_json_read_obj(cJSON *obj, pjsip_log_config_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_READ_INT(ua, obj, msgLogging);
		PJ_JSON_READ_INT(ua, obj, level);
		PJ_JSON_READ_INT(ua, obj, consoleLevel);
		PJ_JSON_READ_INT(ua, obj, decor);

		PJ_JSON_READ_STRING(ua, obj, filename);
		PJ_JSON_READ_INT(ua, obj, fileFlags);
		return OK;
	}
	return ERROR;
}

static int
pjsip_log_config_json_write_obj(cJSON *obj, pjsip_log_config_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_WRITE_INT(ua, obj, msgLogging);
		PJ_JSON_WRITE_INT(ua, obj, level);
		PJ_JSON_WRITE_INT(ua, obj, consoleLevel);
		PJ_JSON_WRITE_INT(ua, obj, decor);

		PJ_JSON_WRITE_STRING(ua, obj, filename);
		PJ_JSON_WRITE_INT(ua, obj, fileFlags);
		return OK;
	}
	return ERROR;
}

static int
pjsip_media_config_json_read_obj(cJSON *obj, pjsip_media_config_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_READ_INT(ua, obj, clockRate);
		PJ_JSON_READ_INT(ua, obj, sndClockRate);
		PJ_JSON_READ_INT(ua, obj, channelCount);
		PJ_JSON_READ_INT(ua, obj, audioFramePtime);
		PJ_JSON_READ_INT(ua, obj, maxMediaPorts);
		PJ_JSON_READ_BOOL(ua, obj, hasIoqueue);
		PJ_JSON_READ_INT(ua, obj, threadCnt);
		PJ_JSON_READ_INT(ua, obj, quality);
		PJ_JSON_READ_INT(ua, obj, ptime);
		PJ_JSON_READ_BOOL(ua, obj, noVad);
		PJ_JSON_READ_INT(ua, obj, ilbcMode);
		PJ_JSON_READ_INT(ua, obj, txDropPct);
		PJ_JSON_READ_INT(ua, obj, rxDropPct);
		PJ_JSON_READ_INT(ua, obj, ecOptions);
		PJ_JSON_READ_INT(ua, obj, ecTailLen);
		PJ_JSON_READ_INT(ua, obj, sndRecLatency);
		PJ_JSON_READ_INT(ua, obj, sndPlayLatency);
		PJ_JSON_READ_INT(ua, obj, jbInit);
		PJ_JSON_READ_INT(ua, obj, jbMinPre);
		PJ_JSON_READ_INT(ua, obj, jbMaxPre);
		PJ_JSON_READ_INT(ua, obj, jbMax);
		PJ_JSON_READ_INT(ua, obj, sndAutoCloseTime);
		PJ_JSON_READ_BOOL(ua, obj, vidPreviewEnableNative);
		return OK;
	}
	return ERROR;
}

static int
pjsip_media_config_json_write_obj(cJSON *obj, pjsip_media_config_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_WRITE_INT(ua, obj, clockRate);
		PJ_JSON_WRITE_INT(ua, obj, sndClockRate);
		PJ_JSON_WRITE_INT(ua, obj, channelCount);
		PJ_JSON_WRITE_INT(ua, obj, audioFramePtime);
		PJ_JSON_WRITE_INT(ua, obj, maxMediaPorts);
		PJ_JSON_WRITE_BOOL(ua, obj, hasIoqueue);
		PJ_JSON_WRITE_INT(ua, obj, threadCnt);
		PJ_JSON_WRITE_INT(ua, obj, quality);
		PJ_JSON_WRITE_INT(ua, obj, ptime);
		PJ_JSON_WRITE_BOOL(ua, obj, noVad);
		PJ_JSON_WRITE_INT(ua, obj, ilbcMode);
		PJ_JSON_WRITE_INT(ua, obj, txDropPct);
		PJ_JSON_WRITE_INT(ua, obj, rxDropPct);
		PJ_JSON_WRITE_INT(ua, obj, ecOptions);
		PJ_JSON_WRITE_INT(ua, obj, ecTailLen);
		PJ_JSON_WRITE_INT(ua, obj, sndRecLatency);
		PJ_JSON_WRITE_INT(ua, obj, sndPlayLatency);
		PJ_JSON_WRITE_INT(ua, obj, jbInit);
		PJ_JSON_WRITE_INT(ua, obj, jbMinPre);
		PJ_JSON_WRITE_INT(ua, obj, jbMaxPre);
		PJ_JSON_WRITE_INT(ua, obj, jbMax);
		PJ_JSON_WRITE_INT(ua, obj, sndAutoCloseTime);
		PJ_JSON_WRITE_BOOL(ua, obj, vidPreviewEnableNative);
		return OK;
	}
	return ERROR;
}

static int
pjsip_ep_config_json_read_obj(cJSON *obj, pjsip_ep_config_t *ua)
{
	if (obj && ua)
	{
		cJSON *tmpobj = cJSON_GetObjectItem(obj, "uaConfig");
		if (tmpobj)
		{
			ua->uaConfig = malloc(sizeof(pjsip_ua_config_t));
			if (ua->uaConfig)
			{
				if (pjsip_ua_config_json_read_obj(tmpobj, ua->uaConfig) != OK)
				{
					free(ua->uaConfig);
					ua->uaConfig = NULL;
					return ERROR;
				}
			}
			else
				return ERROR;
		}
		tmpobj = cJSON_GetObjectItem(obj, "logConfig");
		if (tmpobj)
		{
			ua->logConfig = malloc(sizeof(pjsip_log_config_t));
			if (ua->logConfig)
			{
				if (pjsip_log_config_json_read_obj(tmpobj, ua->logConfig) != OK)
				{
					free(ua->logConfig);
					ua->logConfig = NULL;
					return ERROR;
				}
			}
			else
				return ERROR;
		}
		tmpobj = cJSON_GetObjectItem(obj, "medConfig");
		if (tmpobj)
		{
			ua->medConfig = malloc(sizeof(pjsip_media_config_t));
			if (ua->medConfig)
			{
				if (pjsip_media_config_json_read_obj(tmpobj, ua->medConfig) != OK)
				{
					free(ua->medConfig);
					ua->medConfig = NULL;
					return ERROR;
				}
			}
			else
				return ERROR;
		}
		return OK;
	}
	return ERROR;
}

static int
pjsip_ep_config_json_write_obj(cJSON *obj, pjsip_ep_config_t *ua)
{
	if (obj && ua)
	{
		printf("pjsip_ep_config_json_write_obj\r\n");
		cJSON *tmpobj = cJSON_CreateObject();
		if (tmpobj)
		{
			if (ua->uaConfig)
			{
				printf("pjsip_ua_config_json_write_obj\r\n");
				if (pjsip_ua_config_json_write_obj(tmpobj, ua->uaConfig) != OK)
				{
					cJSON_Delete(tmpobj);
					return ERROR;
				}
				cJSON_AddItemToObject(obj, "uaConfig", tmpobj);
				//cJSON_Delete(tmpobj);
				tmpobj = NULL;
				printf("pjsip_ua_config_json_write_obj end\r\n");
			}
			else
			{
				cJSON_Delete(tmpobj);
				tmpobj = NULL;
			}
		}
		tmpobj = cJSON_CreateObject();
		if (tmpobj)
		{
			if (ua->logConfig)
			{
				printf("pjsip_log_config_json_write_obj\r\n");
				if (pjsip_log_config_json_write_obj(tmpobj, ua->logConfig) != OK)
				{
					cJSON_Delete(tmpobj);
					return ERROR;
				}
				printf("pjsip_log_config_json_write_obj end\r\n");
				cJSON_AddItemToObject(obj, "logConfig", tmpobj);
				//cJSON_Delete(tmpobj);
				tmpobj = NULL;
			}
			else
			{
				cJSON_Delete(tmpobj);
				tmpobj = NULL;
			}
		}
		tmpobj = cJSON_CreateObject();
		if (tmpobj)
		{
			if (ua->medConfig)
			{
				printf("pjsip_media_config_json_write_obj\r\n");
				if (pjsip_media_config_json_write_obj(tmpobj, ua->medConfig) != OK)
				{
					cJSON_Delete(tmpobj);
					return ERROR;
				}
				cJSON_AddItemToObject(obj, "medConfig", tmpobj);
				//cJSON_Delete(tmpobj);
				tmpobj = NULL;
				printf("pjsip_media_config_json_write_obj end\r\n");
			}
			else
			{
				cJSON_Delete(tmpobj);
				tmpobj = NULL;
			}
		}
		return OK;
	}
	return ERROR;
}

static int
pjsip_auth_cred_info_json_read_obj(cJSON *obj, pjsip_auth_cred_info_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_READ_STRING(ua, obj, scheme);
		PJ_JSON_READ_STRING(ua, obj, realm);
		PJ_JSON_READ_STRING(ua, obj, username);
		PJ_JSON_READ_INT(ua, obj, dataType);

		PJ_JSON_READ_STRING(ua, obj, data);
		PJ_JSON_READ_STRING(ua, obj, akaK);
		PJ_JSON_READ_STRING(ua, obj, akaOp);
		PJ_JSON_READ_STRING(ua, obj, akaAmf);
		return OK;
	}
	return ERROR;
}

static int
pjsip_auth_cred_info_json_write_obj(cJSON *obj, pjsip_auth_cred_info_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_WRITE_STRING(ua, obj, scheme);
		PJ_JSON_WRITE_STRING(ua, obj, realm);
		PJ_JSON_WRITE_STRING(ua, obj, username);
		PJ_JSON_WRITE_INT(ua, obj, dataType);

		PJ_JSON_WRITE_STRING(ua, obj, data);
		PJ_JSON_WRITE_STRING(ua, obj, akaK);
		PJ_JSON_WRITE_STRING(ua, obj, akaOp);
		PJ_JSON_WRITE_STRING(ua, obj, akaAmf);
		return OK;
	}
	return ERROR;
}

static int
pjsip_qos_params_json_read_obj(cJSON *obj, pj_qos_params *ua)
{
	if (obj && ua)
	{
		PJ_JSON_READ_INT(ua, obj, flags);
		PJ_JSON_READ_INT(ua, obj, dscp_val);
		PJ_JSON_READ_INT(ua, obj, so_prio);
		PJ_JSON_READ_INT(ua, obj, wmm_prio);
		return OK;
	}
	return ERROR;
}

static int
pjsip_qos_params_json_write_obj(cJSON *obj, pj_qos_params *ua)
{
	if (obj && ua)
	{
		PJ_JSON_WRITE_INT(ua, obj, flags);
		PJ_JSON_WRITE_INT(ua, obj, dscp_val);
		PJ_JSON_WRITE_INT(ua, obj, so_prio);
		PJ_JSON_WRITE_INT(ua, obj, wmm_prio);
		return OK;
	}
	return ERROR;
}


static int
pjsip_tls_config_json_read_obj(cJSON *obj, pjsip_tls_config_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_READ_STRING(ua, obj, CaListFile);
		PJ_JSON_READ_STRING(ua, obj, certFile);
		PJ_JSON_READ_STRING(ua, obj, privKeyFile);
		PJ_JSON_READ_STRING(ua, obj, password);

		PJ_JSON_READ_STRING(ua, obj, CaBuf);
		PJ_JSON_READ_STRING(ua, obj, certBuf);
		PJ_JSON_READ_STRING(ua, obj, privKeyBuf);
		PJ_JSON_READ_INT(ua, obj, method);
		PJ_JSON_READ_INT(ua, obj, proto);

		PJ_JSON_READ_BOOL(ua, obj, verifyServer);
		PJ_JSON_READ_BOOL(ua, obj, verifyClient);
		PJ_JSON_READ_BOOL(ua, obj, requireClientCert);
		PJ_JSON_READ_INT(ua, obj, msecTimeout);
		PJ_JSON_READ_INT(ua, obj, qosType);
		
		//PJ_JSON_READ_INT(ua, obj, qosParams);
		//pjsip_qos_params_json_read_obj(obj, &ua->qosParams);
		cJSON *tmpobj = cJSON_GetObjectItem(obj, "qosParams");
		if (tmpobj)
		{
			if (pjsip_qos_params_json_read_obj(tmpobj, &ua->qosParams) != OK)
			{
				return ERROR;
			}
		}
		PJ_JSON_READ_BOOL(ua, obj, qosIgnoreError);

		ua->ciphersCnt = PJ_JSON_GET_ARRAY_SIZE(ua, obj, ciphers);
		if (ua->ciphersCnt > 0)
		{
			ua->ciphers = malloc(sizeof(int) * ua->ciphersCnt);
			if (ua->ciphers)
			{
				PJ_JSON_READ_INT_ARRAY(ua, obj, ciphers);
			}
			else
				return ERROR;
		}
		return OK;
	}
	return ERROR;
}

static int
pjsip_tls_config_json_write_obj(cJSON *obj, pjsip_tls_config_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_WRITE_STRING(ua, obj, CaListFile);
		PJ_JSON_WRITE_STRING(ua, obj, certFile);
		PJ_JSON_WRITE_STRING(ua, obj, privKeyFile);
		PJ_JSON_WRITE_STRING(ua, obj, password);

		PJ_JSON_WRITE_STRING(ua, obj, CaBuf);
		PJ_JSON_WRITE_STRING(ua, obj, certBuf);
		PJ_JSON_WRITE_STRING(ua, obj, privKeyBuf);
		PJ_JSON_WRITE_INT(ua, obj, method);
		PJ_JSON_WRITE_INT(ua, obj, proto);
		PJ_JSON_WRITE_BOOL(ua, obj, verifyServer);
		PJ_JSON_WRITE_BOOL(ua, obj, verifyClient);
		PJ_JSON_WRITE_BOOL(ua, obj, requireClientCert);

		PJ_JSON_WRITE_INT(ua, obj, msecTimeout);
		PJ_JSON_WRITE_INT(ua, obj, qosType);
		//PJ_JSON_WRITE_INT(ua, obj, qosParams);
		cJSON *tmpobj = cJSON_CreateObject();
		if (tmpobj)
		{
			if (pjsip_qos_params_json_write_obj(obj, &ua->qosParams) != OK)
			{
				cJSON_Delete(tmpobj);
				return ERROR;
			}
			cJSON_AddItemToObject(obj, "qosParams", tmpobj);
			//cJSON_Delete(tmpobj);
		}
		PJ_JSON_WRITE_BOOL(ua, obj, qosIgnoreError);
		PJ_JSON_WRITE_INT_ARRAY(ua, obj, ciphers, ua->ciphersCnt);
		return OK;
	}
	return ERROR;
}

static int
pjsip_transport_config_json_read_obj(cJSON *obj, pjsip_transport_config_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_READ_INT(ua, obj, port);
		PJ_JSON_READ_INT(ua, obj, portRange);
		PJ_JSON_READ_STRING(ua, obj, publicAddress);
		PJ_JSON_READ_STRING(ua, obj, boundAddress);
		PJ_JSON_READ_INT(ua, obj, qosType);
		//PJ_JSON_READ_INT(ua, obj, qosParams);
		cJSON *tmpobj = cJSON_GetObjectItem(obj, "qosParams");
		if (tmpobj)
		{
			if (pjsip_qos_params_json_read_obj(tmpobj, &ua->qosParams) != OK)
			{
				return ERROR;
			}
		}
		tmpobj = cJSON_GetObjectItem(obj, "tlsConfig");
		if (tmpobj)
		{
			ua->tlsConfig = malloc(sizeof(pjsip_tls_config_t));
			if (ua->tlsConfig)
			{
				if (pjsip_tls_config_json_read_obj(tmpobj, ua->tlsConfig) != OK)
				{
					free(ua->tlsConfig);
					ua->tlsConfig = NULL;
					return ERROR;
				}
			}
			else
				return ERROR;
		}
		return OK;
	}
	return ERROR;
}

static int
pjsip_transport_config_json_write_obj(cJSON *obj, pjsip_transport_config_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_WRITE_INT(ua, obj, port);
		PJ_JSON_WRITE_INT(ua, obj, portRange);
		PJ_JSON_WRITE_STRING(ua, obj, publicAddress);
		PJ_JSON_WRITE_STRING(ua, obj, boundAddress);
		PJ_JSON_WRITE_INT(ua, obj, qosType);
		//PJ_JSON_WRITE_INT(ua, obj, qosParams);
		cJSON *tmpobj = cJSON_CreateObject();
		if (tmpobj)
		{
			if (pjsip_qos_params_json_write_obj(obj, &ua->qosParams) != OK)
			{
				cJSON_Delete(tmpobj);
				return ERROR;
			}
			cJSON_AddItemToObject(obj, "qosParams", tmpobj);
			//cJSON_Delete(tmpobj);
		}
		tmpobj = cJSON_CreateObject();
		if (tmpobj && ua->tlsConfig)
		{
			if (pjsip_tls_config_json_write_obj(tmpobj, ua->tlsConfig) != OK)
			{
				cJSON_Delete(tmpobj);
				return ERROR;
			}
			cJSON_AddItemToObject(obj, "tlsConfig", tmpobj);
			//cJSON_Delete(tmpobj);
		}
		return OK;
	}
	return ERROR;
}

static int
pjsip_account_reg_config_json_read_obj(cJSON *obj,
									   pjsip_account_reg_config_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_READ_STRING(ua, obj, registrarUri);
		PJ_JSON_READ_BOOL(ua, obj, registerOnAdd);
		PJ_JSON_READ_STRING(ua, obj, contactParams);

		PJ_JSON_READ_INT(ua, obj, timeoutSec);
		PJ_JSON_READ_INT(ua, obj, retryIntervalSec);
		PJ_JSON_READ_INT(ua, obj, firstRetryIntervalSec);
		PJ_JSON_READ_INT(ua, obj, randomRetryIntervalSec);
		PJ_JSON_READ_INT(ua, obj, delayBeforeRefreshSec);
		PJ_JSON_READ_BOOL(ua, obj, dropCallsOnFail);
		PJ_JSON_READ_INT(ua, obj, unregWaitMsec);
		PJ_JSON_READ_INT(ua, obj, proxyUse);
		return OK;
	}
	return ERROR;
}

static int
pjsip_account_reg_config_json_write_obj(cJSON *obj,
										pjsip_account_reg_config_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_WRITE_STRING(ua, obj, registrarUri);
		PJ_JSON_WRITE_BOOL(ua, obj, registerOnAdd);
		PJ_JSON_WRITE_STRING(ua, obj, contactParams);
		PJ_JSON_WRITE_INT(ua, obj, timeoutSec);
		PJ_JSON_WRITE_INT(ua, obj, retryIntervalSec);
		PJ_JSON_WRITE_INT(ua, obj, firstRetryIntervalSec);
		PJ_JSON_WRITE_INT(ua, obj, randomRetryIntervalSec);
		PJ_JSON_WRITE_INT(ua, obj, delayBeforeRefreshSec);
		PJ_JSON_WRITE_BOOL(ua, obj, dropCallsOnFail);
		PJ_JSON_WRITE_INT(ua, obj, unregWaitMsec);
		PJ_JSON_WRITE_INT(ua, obj, proxyUse);
		return OK;
	}
	return ERROR;
}

static int
pjsip_account_sip_config_json_read_obj(cJSON *obj,
									   pjsip_account_sip_config_t *ua)
{
	if (obj && ua)
	{
		ua->proxiesCnt = PJ_JSON_GET_ARRAY_SIZE(ua, obj, proxies);
		PJ_JSON_READ_STRING_ARRAY(ua, obj, proxies);
		PJ_JSON_READ_STRING(ua, obj, contactForced);
		PJ_JSON_READ_STRING(ua, obj, contactParams);
		PJ_JSON_READ_STRING(ua, obj, contactUriParams);
		PJ_JSON_READ_BOOL(ua, obj, authInitialEmpty);
		PJ_JSON_READ_STRING(ua, obj, authInitialAlgorithm);
		PJ_JSON_READ_INT(ua, obj, transportId);

		cJSON *tmpobj = cJSON_GetObjectItem(obj, "authCreds");
		if (tmpobj)
		{
			ua->authCreds = malloc(sizeof(pjsip_auth_cred_info_t));
			if (ua->authCreds)
			{
				if (pjsip_auth_cred_info_json_read_obj(tmpobj, ua->authCreds) != OK)
				{
					free(ua->authCreds);
					ua->authCreds = NULL;
					return ERROR;
				}
			}
			else
				return ERROR;
		}
		return OK;
	}
	return ERROR;
}

static int
pjsip_account_sip_config_json_write_obj(cJSON *obj,
										pjsip_account_sip_config_t *ua)
{
	if (obj && ua)
	{
		cJSON *tmpobj = cJSON_CreateObject();
		if (tmpobj && ua->authCreds)
		{
			if (pjsip_auth_cred_info_json_write_obj(tmpobj, ua->authCreds) != OK)
			{
				cJSON_Delete(tmpobj);
				return ERROR;
			}
			cJSON_AddItemToObject(obj, "authCreds", tmpobj);
			//cJSON_Delete(tmpobj);
		}
		PJ_JSON_WRITE_STRING_ARRAY(ua, obj, proxies, ua->proxiesCnt);
		PJ_JSON_WRITE_STRING(ua, obj, contactForced);
		PJ_JSON_WRITE_STRING(ua, obj, contactParams);
		PJ_JSON_WRITE_STRING(ua, obj, contactUriParams);
		PJ_JSON_WRITE_BOOL(ua, obj, authInitialEmpty);
		PJ_JSON_WRITE_STRING(ua, obj, authInitialAlgorithm);
		PJ_JSON_WRITE_INT(ua, obj, transportId);
		return OK;
	}
	return ERROR;
}

static int
pjsip_account_call_config_json_read_obj(cJSON *obj,
										pjsip_account_call_config_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_READ_INT(ua, obj, holdType);
		PJ_JSON_READ_INT(ua, obj, prackUse);
		PJ_JSON_READ_INT(ua, obj, timerUse);
		PJ_JSON_READ_INT(ua, obj, timerMinSESec);
		PJ_JSON_READ_INT(ua, obj, timerSessExpiresSec);
		return OK;
	}
	return ERROR;
}

static int
pjsip_account_call_config_json_write_obj(cJSON *obj,
										 pjsip_account_call_config_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_WRITE_INT(ua, obj, holdType);
		PJ_JSON_WRITE_INT(ua, obj, prackUse);
		PJ_JSON_WRITE_INT(ua, obj, timerUse);
		PJ_JSON_WRITE_INT(ua, obj, timerMinSESec);
		PJ_JSON_WRITE_INT(ua, obj, timerSessExpiresSec);
		return OK;
	}
	return ERROR;
}

static int
pjsip_account_pres_config_json_read_obj(cJSON *obj,
										pjsip_account_pres_config_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_READ_BOOL(ua, obj, publishEnabled);
		PJ_JSON_READ_BOOL(ua, obj, publishQueue);
		PJ_JSON_READ_INT(ua, obj, publishShutdownWaitMsec);
		PJ_JSON_READ_STRING(ua, obj, pidfTupleId);
		return OK;
	}
	return ERROR;
}

static int
pjsip_account_pres_config_json_write_obj(cJSON *obj,
										 pjsip_account_pres_config_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_WRITE_BOOL(ua, obj, publishEnabled);
		PJ_JSON_WRITE_BOOL(ua, obj, publishEnabled);
		PJ_JSON_WRITE_INT(ua, obj, publishShutdownWaitMsec);
		PJ_JSON_WRITE_STRING(ua, obj, pidfTupleId);
		return OK;
	}
	return ERROR;
}

static int
pjsip_account_mwi_config_json_read_obj(cJSON *obj,
									   pjsip_account_mwi_config_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_READ_BOOL(ua, obj, enabled);
		PJ_JSON_READ_INT(ua, obj, expirationSec);
		return OK;
	}
	return ERROR;
}

static int
pjsip_account_mwi_config_json_write_obj(cJSON *obj,
										pjsip_account_mwi_config_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_WRITE_BOOL(ua, obj, enabled);
		PJ_JSON_WRITE_INT(ua, obj, expirationSec);
		return OK;
	}
	return ERROR;
}

static int
pjsip_account_nat_config_json_read_obj(cJSON *obj,
									   pjsip_account_nat_config_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_READ_INT(ua, obj, sipStunUse);
		PJ_JSON_READ_INT(ua, obj, mediaStunUse);
		PJ_JSON_READ_INT(ua, obj, nat64Opt);
		PJ_JSON_READ_BOOL(ua, obj, iceEnabled);
		PJ_JSON_READ_INT(ua, obj, iceMaxHostCands);
		PJ_JSON_READ_BOOL(ua, obj, iceAggressiveNomination);
		PJ_JSON_READ_INT(ua, obj, iceNominatedCheckDelayMsec);
		PJ_JSON_READ_INT(ua, obj, iceWaitNominationTimeoutMsec);
		PJ_JSON_READ_BOOL(ua, obj, iceNoRtcp);
		PJ_JSON_READ_BOOL(ua, obj, iceAlwaysUpdate);
		PJ_JSON_READ_BOOL(ua, obj, turnEnabled);
		PJ_JSON_READ_STRING(ua, obj, turnServer);
		PJ_JSON_READ_INT(ua, obj, turnConnType);
		PJ_JSON_READ_STRING(ua, obj, turnUserName);
		PJ_JSON_READ_INT(ua, obj, turnPasswordType);
		PJ_JSON_READ_STRING(ua, obj, turnPassword);
		PJ_JSON_READ_INT(ua, obj, contactRewriteUse);
		PJ_JSON_READ_INT(ua, obj, contactRewriteMethod);
		PJ_JSON_READ_INT(ua, obj, contactUseSrcPort);
		PJ_JSON_READ_INT(ua, obj, viaRewriteUse);
		PJ_JSON_READ_INT(ua, obj, sdpNatRewriteUse);
		PJ_JSON_READ_INT(ua, obj, sipOutboundUse);
		PJ_JSON_READ_STRING(ua, obj, sipOutboundInstanceId);
		PJ_JSON_READ_STRING(ua, obj, sipOutboundRegId);
		PJ_JSON_READ_INT(ua, obj, udpKaIntervalSec);
		PJ_JSON_READ_STRING(ua, obj, udpKaData);
		return OK;
	}
	return ERROR;
}

static int
pjsip_account_nat_config_json_write_obj(cJSON *obj,
										pjsip_account_nat_config_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_WRITE_INT(ua, obj, sipStunUse);
		PJ_JSON_WRITE_INT(ua, obj, mediaStunUse);
		PJ_JSON_WRITE_INT(ua, obj, nat64Opt);
		PJ_JSON_WRITE_BOOL(ua, obj, iceEnabled);
		PJ_JSON_WRITE_INT(ua, obj, iceMaxHostCands);
		PJ_JSON_WRITE_BOOL(ua, obj, iceAggressiveNomination);
		PJ_JSON_WRITE_INT(ua, obj, iceNominatedCheckDelayMsec);
		PJ_JSON_WRITE_INT(ua, obj, iceWaitNominationTimeoutMsec);
		PJ_JSON_WRITE_BOOL(ua, obj, iceNoRtcp);
		PJ_JSON_WRITE_BOOL(ua, obj, iceAlwaysUpdate);
		PJ_JSON_WRITE_BOOL(ua, obj, turnEnabled);
		PJ_JSON_WRITE_STRING(ua, obj, turnServer);
		PJ_JSON_WRITE_INT(ua, obj, turnConnType);
		PJ_JSON_WRITE_STRING(ua, obj, turnUserName);
		PJ_JSON_WRITE_INT(ua, obj, turnPasswordType);
		PJ_JSON_WRITE_STRING(ua, obj, turnPassword);
		PJ_JSON_WRITE_INT(ua, obj, contactRewriteUse);
		PJ_JSON_WRITE_INT(ua, obj, contactRewriteMethod);
		PJ_JSON_WRITE_INT(ua, obj, contactUseSrcPort);
		PJ_JSON_WRITE_INT(ua, obj, viaRewriteUse);
		PJ_JSON_WRITE_INT(ua, obj, sdpNatRewriteUse);
		PJ_JSON_WRITE_INT(ua, obj, sipOutboundUse);
		PJ_JSON_WRITE_STRING(ua, obj, sipOutboundInstanceId);
		PJ_JSON_WRITE_STRING(ua, obj, sipOutboundRegId);
		PJ_JSON_WRITE_INT(ua, obj, udpKaIntervalSec);
		PJ_JSON_WRITE_STRING(ua, obj, udpKaData);
		return OK;
	}
	return ERROR;
}

static int
pjsip_srtp_crypto_json_read_obj(cJSON *obj, pjsip_srtp_crypto_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_READ_STRING(ua, obj, key);
		PJ_JSON_READ_STRING(ua, obj, name);
		PJ_JSON_READ_INT(ua, obj, flags);
		return OK;
	}
	return ERROR;
}

static int
pjsip_srtp_crypto_json_write_obj(cJSON *obj, pjsip_srtp_crypto_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_WRITE_STRING(ua, obj, key);
		PJ_JSON_WRITE_STRING(ua, obj, name);
		PJ_JSON_WRITE_INT(ua, obj, flags);
		return OK;
	}
	return ERROR;
}

static int
pjsip_srtp_option_json_read_obj(cJSON *obj, pjsip_srtp_option_t *ua)
{
	if (obj && ua)
	{
		ua->keying_num = PJ_JSON_GET_ARRAY_SIZE(ua, obj, keyings);
		PJ_JSON_READ_INT_ARRAY(ua, obj, keyings);

		cJSON *tmpobjarray = cJSON_GetObjectItem(obj, "cryptos");
		cJSON *tmpobj = NULL;//cJSON_GetObjectItem(obj, "cryptos");
		if (tmpobjarray)
		{
			ua->cryptos_num = cJSON_GetArraySize(tmpobjarray);
			if(ua->cryptos_num)
			{
				int i = 0;
				ua->cryptos = malloc(sizeof(pjsip_srtp_crypto_t)*ua->cryptos_num);
				if (ua->cryptos)
				{
					for (i = 0; i < ua->cryptos_num; i++)
					{
						tmpobj = cJSON_GetArrayItem(tmpobjarray, i); //
						if(tmpobj)
						{
							if (pjsip_srtp_crypto_json_read_obj(tmpobj, &ua->cryptos[i]) != OK)
							{
								free(ua->cryptos);
								ua->cryptos = NULL;
								return ERROR;
							}
						}
					}
				}
				else
					return ERROR;
			}
		}
		return OK;
	}
	return ERROR;
}

static int
pjsip_srtp_option_json_write_obj(cJSON *obj, pjsip_srtp_option_t *ua)
{
	if (obj && ua)
	{
		int i = 0;
		cJSON *tmpobjarray = cJSON_CreateArray();
		cJSON *tmpobj = NULL;//cJSON_CreateObject();
		if (tmpobjarray && ua->cryptos && ua->cryptos_num > 0)
		{
			for(i = 0; i < ua->cryptos_num; i++)
			{
				tmpobj = cJSON_CreateObject();
				if(tmpobj)
				{
					if (pjsip_srtp_crypto_json_write_obj(tmpobj, &ua->cryptos[i]) != OK)
					{
						cJSON_Delete(tmpobjarray);
						cJSON_Delete(tmpobj);
						return ERROR;
					}
					cJSON_AddItemToArray(tmpobjarray, tmpobj);
					//cJSON_Delete(tmpobj);
				}
			}
			cJSON_AddItemToObject(obj, "cryptos", tmpobjarray);
			//cJSON_Delete(tmpobjarray);
		}
		PJ_JSON_WRITE_INT_ARRAY(ua, obj, keyings, ua->keying_num);
		return OK;
	}
	return ERROR;
}

static int
pjsip_rtcp_fb_cap_json_read_obj(cJSON *obj, pjsip_rtcp_fb_cap_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_READ_STRING(ua, obj, codecId);
		PJ_JSON_READ_STRING(ua, obj, typeName);
		PJ_JSON_READ_INT(ua, obj, type);
		PJ_JSON_READ_STRING(ua, obj, param);
		return OK;
	}
	return ERROR;
}

static int
pjsip_rtcp_fb_cap_json_write_obj(cJSON *obj, pjsip_rtcp_fb_cap_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_WRITE_STRING(ua, obj, codecId);
		PJ_JSON_WRITE_STRING(ua, obj, typeName);
		PJ_JSON_WRITE_INT(ua, obj, type);
		PJ_JSON_WRITE_STRING(ua, obj, param);
		return OK;
	}
	return ERROR;
}

static int
pjsip_rtcp_fb_config_json_read_obj(cJSON *obj, pjsip_rtcp_fb_config_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_READ_BOOL(ua, obj, dontUseAvpf);
		cJSON *tmpobj = cJSON_GetObjectItem(obj, "caps");
		if (tmpobj)
		{
			ua->caps = malloc(sizeof(pjsip_rtcp_fb_cap_t));
			if (ua->caps)
			{
				if (pjsip_rtcp_fb_cap_json_read_obj(tmpobj, ua->caps) != OK)
				{
					free(ua->caps);
					ua->caps = NULL;
					return ERROR;
				}
			}
			else
				return ERROR;
		}
		return OK;
	}
	return ERROR;
}

static int
pjsip_rtcp_fb_config_json_write_obj(cJSON *obj, pjsip_rtcp_fb_config_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_WRITE_BOOL(ua, obj, dontUseAvpf);
		cJSON *tmpobj = cJSON_CreateObject();
		if (tmpobj && ua->caps)
		{
			if (pjsip_rtcp_fb_cap_json_write_obj(tmpobj, ua->caps) != OK)
			{
				cJSON_Delete(tmpobj);
				return ERROR;
			}
			cJSON_AddItemToObject(obj, "caps", tmpobj);
			//cJSON_Delete(tmpobj);
		}
		return OK;
	}
	return ERROR;
}

static int
pjsip_account_media_config_json_read_obj(cJSON *obj,
										 pjsip_account_media_config_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_READ_BOOL(ua, obj, lockCodecEnabled);
		PJ_JSON_READ_BOOL(ua, obj, streamKaEnabled);
		PJ_JSON_READ_INT(ua, obj, srtpUse);
		PJ_JSON_READ_INT(ua, obj, srtpSecureSignaling);
		PJ_JSON_READ_INT(ua, obj, ipv6Use);
		PJ_JSON_READ_BOOL(ua, obj, rtcpMuxEnabled);

		cJSON *tmpobj = cJSON_GetObjectItem(obj, "transportConfig");
		if (tmpobj)
		{
			ua->transportConfig = malloc(sizeof(pjsip_transport_config_t));
			if (ua->transportConfig)
			{
				if (pjsip_transport_config_json_read_obj(tmpobj,
														 ua->transportConfig) != OK)
				{
					free(ua->transportConfig);
					ua->transportConfig = NULL;
					return ERROR;
				}
			}
			else
				return ERROR;
		}
		tmpobj = cJSON_GetObjectItem(obj, "srtpOpt");
		if (tmpobj)
		{
			ua->srtpOpt = malloc(sizeof(pjsip_srtp_option_t));
			if (ua->srtpOpt)
			{
				if (pjsip_srtp_option_json_read_obj(tmpobj, ua->srtpOpt) != OK)
				{
					free(ua->srtpOpt);
					ua->srtpOpt = NULL;
					return ERROR;
				}
			}
			else
				return ERROR;
		}
		tmpobj = cJSON_GetObjectItem(obj, "rtcpFbConfig");
		if (tmpobj)
		{
			ua->rtcpFbConfig = malloc(sizeof(pjsip_rtcp_fb_config_t));
			if (ua->rtcpFbConfig)
			{
				if (pjsip_rtcp_fb_config_json_read_obj(tmpobj, ua->rtcpFbConfig) != OK)
				{
					free(ua->rtcpFbConfig);
					ua->rtcpFbConfig = NULL;
					return ERROR;
				}
			}
			else
				return ERROR;
		}
		return OK;
	}
	return ERROR;
}

static int
pjsip_account_media_config_json_write_obj(cJSON *obj,
										  pjsip_account_media_config_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_WRITE_BOOL(ua, obj, lockCodecEnabled);
		PJ_JSON_WRITE_BOOL(ua, obj, streamKaEnabled);
		PJ_JSON_WRITE_INT(ua, obj, srtpUse);
		PJ_JSON_WRITE_INT(ua, obj, srtpSecureSignaling);
		PJ_JSON_WRITE_INT(ua, obj, ipv6Use);
		PJ_JSON_WRITE_BOOL(ua, obj, rtcpMuxEnabled);

		cJSON *tmpobj = cJSON_CreateObject();
		if (tmpobj && ua->transportConfig)
		{
			if (pjsip_transport_config_json_write_obj(tmpobj, ua->transportConfig) != OK)
			{
				cJSON_Delete(tmpobj);
				return ERROR;
			}
			cJSON_AddItemToObject(obj, "transportConfig", tmpobj);
			//cJSON_Delete(tmpobj);
		}
		tmpobj = cJSON_CreateObject();
		if (tmpobj && ua->srtpOpt)
		{
			if (pjsip_srtp_option_json_write_obj(tmpobj, ua->srtpOpt) != OK)
			{
				cJSON_Delete(tmpobj);
				return ERROR;
			}
			cJSON_AddItemToObject(obj, "srtpOpt", tmpobj);
			//cJSON_Delete(tmpobj);
		}
		tmpobj = cJSON_CreateObject();
		if (tmpobj && ua->rtcpFbConfig)
		{
			if (pjsip_rtcp_fb_config_json_write_obj(tmpobj, ua->rtcpFbConfig) != OK)
			{
				cJSON_Delete(tmpobj);
				return ERROR;
			}
			cJSON_AddItemToObject(obj, "rtcpFbConfig", tmpobj);
			//cJSON_Delete(tmpobj);
		}
		return OK;
	}
	return ERROR;
}

static int
pjsip_account_video_config_json_read_obj(cJSON *obj,
										 pjsip_account_video_config_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_READ_BOOL(ua, obj, autoShowIncoming);
		PJ_JSON_READ_BOOL(ua, obj, autoTransmitOutgoing);
		PJ_JSON_READ_INT(ua, obj, windowFlags);
		PJ_JSON_READ_INT(ua, obj, defaultCaptureDevice);
		PJ_JSON_READ_INT(ua, obj, defaultRenderDevice);
		PJ_JSON_READ_INT(ua, obj, rateControlMethod);

		PJ_JSON_READ_INT(ua, obj, rateControlBandwidth);
		PJ_JSON_READ_INT(ua, obj, startKeyframeCount);
		PJ_JSON_READ_INT(ua, obj, startKeyframeInterval);
		return OK;
	}
	return ERROR;
}

static int
pjsip_account_video_config_json_write_obj(cJSON *obj,
										  pjsip_account_video_config_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_WRITE_BOOL(ua, obj, autoShowIncoming);
		PJ_JSON_WRITE_BOOL(ua, obj, autoTransmitOutgoing);
		PJ_JSON_WRITE_INT(ua, obj, windowFlags);
		PJ_JSON_WRITE_INT(ua, obj, defaultCaptureDevice);
		PJ_JSON_WRITE_INT(ua, obj, defaultRenderDevice);
		PJ_JSON_WRITE_INT(ua, obj, rateControlMethod);

		PJ_JSON_WRITE_INT(ua, obj, rateControlBandwidth);
		PJ_JSON_WRITE_INT(ua, obj, startKeyframeCount);
		PJ_JSON_WRITE_INT(ua, obj, startKeyframeInterval);
		return OK;
	}
	return ERROR;
}

static int
pjsip_account_IpChange_config_json_read_obj(
	cJSON *obj, pjsip_account_IpChange_config_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_READ_BOOL(ua, obj, shutdownTp);
		PJ_JSON_READ_BOOL(ua, obj, hangupCalls);
		PJ_JSON_READ_INT(ua, obj, reinviteFlags);
		return OK;
	}
	return ERROR;
}

static int
pjsip_account_IpChange_config_json_write_obj(
	cJSON *obj, pjsip_account_IpChange_config_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_WRITE_BOOL(ua, obj, shutdownTp);
		PJ_JSON_WRITE_BOOL(ua, obj, hangupCalls);
		PJ_JSON_WRITE_INT(ua, obj, reinviteFlags);
		return OK;
	}
	return ERROR;
}

static int
pjsip_account_config_json_read_obj(cJSON *obj, pjsip_account_config_t *ua)
{
	if (obj && ua)
	{
		PJ_JSON_READ_STRING(ua, obj, idUri);
		PJ_JSON_READ_INT(ua, obj, priority);

		cJSON *tmpobj = cJSON_GetObjectItem(obj, "regConfig");
		if (tmpobj)
		{
			ua->regConfig = malloc(sizeof(pjsip_account_reg_config_t));
			if (ua->regConfig)
			{
				if (pjsip_account_reg_config_json_read_obj(tmpobj, ua->regConfig) != OK)
				{
					free(ua->regConfig);
					ua->regConfig = NULL;
					return ERROR;
				}
			}
			else
				return ERROR;
		}
		tmpobj = cJSON_GetObjectItem(obj, "sipConfig");
		if (tmpobj)
		{
			ua->sipConfig = malloc(sizeof(pjsip_account_sip_config_t));
			if (ua->sipConfig)
			{
				if (pjsip_account_sip_config_json_read_obj(tmpobj, ua->sipConfig) != OK)
				{
					free(ua->sipConfig);
					ua->sipConfig = NULL;
					return ERROR;
				}
			}
			else
				return ERROR;
		}
		tmpobj = cJSON_GetObjectItem(obj, "callConfig");
		if (tmpobj)
		{
			ua->callConfig = malloc(sizeof(pjsip_account_call_config_t));
			if (ua->callConfig)
			{
				if (pjsip_account_call_config_json_read_obj(tmpobj, ua->callConfig) != OK)
				{
					free(ua->callConfig);
					ua->callConfig = NULL;
					return ERROR;
				}
			}
			else
				return ERROR;
		}
		tmpobj = cJSON_GetObjectItem(obj, "presConfig");
		if (tmpobj)
		{
			ua->presConfig = malloc(sizeof(pjsip_account_pres_config_t));
			if (ua->presConfig)
			{
				if (pjsip_account_pres_config_json_read_obj(tmpobj, ua->presConfig) != OK)
				{
					free(ua->presConfig);
					ua->presConfig = NULL;
					return ERROR;
				}
			}
			else
				return ERROR;
		}
		tmpobj = cJSON_GetObjectItem(obj, "mwiConfig");
		if (tmpobj)
		{
			ua->mwiConfig = malloc(sizeof(pjsip_account_mwi_config_t));
			if (ua->mwiConfig)
			{
				if (pjsip_account_mwi_config_json_read_obj(tmpobj, ua->mwiConfig) != OK)
				{
					free(ua->mwiConfig);
					ua->mwiConfig = NULL;
					return ERROR;
				}
			}
			else
				return ERROR;
		}

		tmpobj = cJSON_GetObjectItem(obj, "natConfig");
		if (tmpobj)
		{
			ua->natConfig = malloc(sizeof(pjsip_account_nat_config_t));
			if (ua->natConfig)
			{
				if (pjsip_account_nat_config_json_read_obj(tmpobj, ua->natConfig) != OK)
				{
					free(ua->natConfig);
					ua->natConfig = NULL;
					return ERROR;
				}
			}
			else
				return ERROR;
		}
		tmpobj = cJSON_GetObjectItem(obj, "mediaConfig");
		if (tmpobj)
		{
			ua->mediaConfig = malloc(sizeof(pjsip_account_media_config_t));
			if (ua->mediaConfig)
			{
				if (pjsip_account_media_config_json_read_obj(tmpobj,
															 ua->mediaConfig) != OK)
				{
					free(ua->mediaConfig);
					ua->mediaConfig = NULL;
					return ERROR;
				}
			}
			else
				return ERROR;
		}
		tmpobj = cJSON_GetObjectItem(obj, "videoConfig");
		if (tmpobj)
		{
			ua->videoConfig = malloc(sizeof(pjsip_account_video_config_t));
			if (ua->videoConfig)
			{
				if (pjsip_account_video_config_json_read_obj(tmpobj,
															 ua->videoConfig) != OK)
				{
					free(ua->videoConfig);
					ua->videoConfig = NULL;
					return ERROR;
				}
			}
			else
				return ERROR;
		}
		tmpobj = cJSON_GetObjectItem(obj, "ipChangeConfig");
		if (tmpobj)
		{
			ua->ipChangeConfig = malloc(sizeof(pjsip_account_IpChange_config_t));
			if (ua->ipChangeConfig)
			{
				if (pjsip_account_IpChange_config_json_read_obj(tmpobj,
																ua->ipChangeConfig) != OK)
				{
					free(ua->ipChangeConfig);
					ua->ipChangeConfig = NULL;
					return ERROR;
				}
			}
			else
				return ERROR;
		}
		return OK;
	}
	return ERROR;
}

static int
pjsip_account_config_json_write_obj(cJSON *obj, pjsip_account_config_t *ua)
{
	if (obj && ua)
	{
		cJSON *tmpobj = NULL;
		PJ_JSON_WRITE_STRING(ua, obj, idUri);
		PJ_JSON_WRITE_INT(ua, obj, priority);
		printf("pjsip_account_config_json_write_obj\r\n");
		tmpobj = cJSON_CreateObject();
		if (tmpobj && ua->regConfig)
		{
			printf("pjsip_account_reg_config_json_write_obj\r\n");
			if (pjsip_account_reg_config_json_write_obj(tmpobj, ua->regConfig) != OK)
			{
				cJSON_Delete(tmpobj);
				return ERROR;
			}
			printf("pjsip_account_reg_config_json_write_obj cJSON_AddItemToObject\r\n");
			cJSON_AddItemToObject(obj, "regConfig", tmpobj);
			//cJSON_Delete(tmpobj);
			printf("pjsip_account_reg_config_json_write_obj end\r\n");
		}
		tmpobj = cJSON_CreateObject();
		if (tmpobj && ua->sipConfig)
		{
			printf("pjsip_account_sip_config_json_write_obj\r\n");
			if (pjsip_account_sip_config_json_write_obj(tmpobj, ua->sipConfig) != OK)
			{
				cJSON_Delete(tmpobj);
				return ERROR;
			}
			printf("pjsip_account_sip_config_json_write_obj cJSON_AddItemToObject\r\n");
			cJSON_AddItemToObject(obj, "sipConfig", tmpobj);
			//cJSON_Delete(tmpobj);
			printf("pjsip_account_sip_config_json_write_obj end\r\n");
		}
		tmpobj = cJSON_CreateObject();
		if (tmpobj && ua->callConfig)
		{
			printf("pjsip_account_call_config_json_write_obj\r\n");
			if (pjsip_account_call_config_json_write_obj(tmpobj, ua->callConfig) != OK)
			{
				cJSON_Delete(tmpobj);
				return ERROR;
			}
			cJSON_AddItemToObject(obj, "callConfig", tmpobj);
			//cJSON_Delete(tmpobj);
			printf("pjsip_account_call_config_json_write_obj end\r\n");
		}
		tmpobj = cJSON_CreateObject();
		if (tmpobj && ua->presConfig)
		{
			printf("pjsip_account_pres_config_json_write_obj\r\n");
			if (pjsip_account_pres_config_json_write_obj(tmpobj, ua->presConfig) != OK)
			{
				cJSON_Delete(tmpobj);
				return ERROR;
			}
			cJSON_AddItemToObject(obj, "presConfig", tmpobj);
			//cJSON_Delete(tmpobj);
			printf("pjsip_account_pres_config_json_write_obj end\r\n");
		}
		tmpobj = cJSON_CreateObject();
		if (tmpobj && ua->mwiConfig)
		{
			printf("pjsip_account_mwi_config_json_write_obj\r\n");
			if (pjsip_account_mwi_config_json_write_obj(tmpobj, ua->mwiConfig) != OK)
			{
				cJSON_Delete(tmpobj);
				return ERROR;
			}
			cJSON_AddItemToObject(obj, "mwiConfig", tmpobj);
			//cJSON_Delete(tmpobj);
			printf("pjsip_account_mwi_config_json_write_obj end\r\n");
		}
		tmpobj = cJSON_CreateObject();
		if (tmpobj && ua->natConfig)
		{
			printf("pjsip_account_nat_config_json_write_obj\r\n");
			if (pjsip_account_nat_config_json_write_obj(tmpobj, ua->natConfig) != OK)
			{
				cJSON_Delete(tmpobj);
				return ERROR;
			}
			cJSON_AddItemToObject(obj, "natConfig", tmpobj);
			//cJSON_Delete(tmpobj);
			printf("pjsip_account_nat_config_json_write_obj end\r\n");
		}
		tmpobj = cJSON_CreateObject();
		if (tmpobj && ua->mediaConfig)
		{
			printf("pjsip_account_media_config_json_write_obj\r\n");
			if (pjsip_account_media_config_json_write_obj(tmpobj, ua->mediaConfig) != OK)
			{
				cJSON_Delete(tmpobj);
				return ERROR;
			}
			cJSON_AddItemToObject(obj, "mediaConfig", tmpobj);
			//cJSON_Delete(tmpobj);
			printf("pjsip_account_media_config_json_write_obj end\r\n");
		}
		tmpobj = cJSON_CreateObject();
		if (tmpobj && ua->videoConfig)
		{
			printf("pjsip_account_video_config_json_write_obj\r\n");
			if (pjsip_account_video_config_json_write_obj(tmpobj, ua->videoConfig) != OK)
			{
				cJSON_Delete(tmpobj);
				return ERROR;
			}
			cJSON_AddItemToObject(obj, "videoConfig", tmpobj);
			//cJSON_Delete(tmpobj);
			printf("pjsip_account_video_config_json_write_obj end\r\n");
		}
		tmpobj = cJSON_CreateObject();
		if (tmpobj && ua->ipChangeConfig)
		{
			printf("pjsip_account_IpChange_config_json_write_obj\r\n");
			if (pjsip_account_IpChange_config_json_write_obj(tmpobj,
															 ua->ipChangeConfig) != OK)
			{
				cJSON_Delete(tmpobj);
				return ERROR;
			}
			cJSON_AddItemToObject(obj, "ipChangeConfig", tmpobj);
			//cJSON_Delete(tmpobj);
			printf("pjsip_account_IpChange_config_json_write_obj end\r\n");
		}
		return OK;
	}
	return ERROR;
}

static int
pjsip_config_json_read_obj(cJSON *obj, pjsip_config_t *ua)
{
	cJSON *tmpobj = cJSON_GetObjectItem(obj, "endpoint");
	if (tmpobj)
	{
		if (pjsip_ep_config_json_read_obj(tmpobj, &ua->endpoint) != OK)
			return ERROR;
	}
	cJSON *tmpobjarray = NULL;
	cJSON *tmpobj_item = NULL;
	tmpobjarray = cJSON_GetObjectItem(obj, "accounts");
	if (tmpobjarray)
	{
		int i = 0;
		ua->table_cnt = cJSON_GetArraySize(tmpobjarray);
		for (i = 0; i < ua->table_cnt && i < 8; i++)
		{
			tmpobj_item = cJSON_GetArrayItem(tmpobjarray, i);
			if (tmpobj_item)
			{
				if (pjsip_account_config_json_read_obj(tmpobj_item,
													   &ua->pjsip_account_table[i]) != OK)
					return ERROR;
			}
		}
		return OK;
	}
	return ERROR;
}

static int
pjsip_config_json_write_obj(cJSON *obj, pjsip_config_t *ua)
{
	cJSON *tmpobjarray = NULL;
	cJSON *tmpobj = NULL;
	tmpobj = cJSON_CreateObject();
	if (tmpobj)
	{
		if (pjsip_ep_config_json_write_obj(tmpobj, &ua->endpoint) != OK)
		{
			cJSON_Delete(tmpobj);
			return ERROR;
		}
		printf("end pjsip_ep_config_json_write_obj\r\n");
		cJSON_AddItemToObject(obj, "endpoint", tmpobj);
		printf("pjsip_config_json_write_obj cJSON_AddItemToObject endpoint\r\n");
	}
	
	tmpobjarray = cJSON_CreateArray();
	if (tmpobjarray)
	{
		int i = 0;
		cJSON *tmpobj_item = NULL;
		printf("pjsip_config_json_write_obj table_cnt=%d\r\n", ua->table_cnt);

		cJSON_AddItemToObject(obj, "accounts", tmpobjarray);

		for (i = 0; i < ua->table_cnt && i < 8; i++)
		{
			tmpobj_item = cJSON_CreateObject();
			if (tmpobj_item)
			{
				if (pjsip_account_config_json_write_obj(tmpobj_item,
														&ua->pjsip_account_table[i]) != OK)
				{
					cJSON_Delete(tmpobj_item);
					return ERROR;
				}
				printf("pjsip_account_config_json_write_obj cJSON_AddItemToArray tmpobj_item\r\n");
				cJSON_AddItemToArray(tmpobjarray, tmpobj_item);
				//cJSON_Delete(tmpobj_item);
				tmpobj_item = NULL;
			}
		}
		printf("pjsip_account_config_json_write_obj cJSON_AddItemToObject tmpobjarray\r\n");
		//cJSON_AddItemToObject(obj, "accounts", tmpobjarray);
		//cJSON_Delete(tmpobjarray);
		return OK;
	}
	return ERROR;
}

int pjsip_config_load(char *filename, pjsip_config_t *ua)
{
	if (os_file_access(filename) != OK)
	{
		printf("pjsip_config_load :%s is not exist.\r\n", filename);
		return ERROR;
	}
	int file_size = (int)os_file_size(filename);
	char *buffer = (char *)malloc(file_size + 1);
	if(!buffer)
	{
		printf("pjsip_config_load : can not malloc buffer(%d byte)\r\n", file_size);
		return ERROR;
	}
	memset(buffer, 0, file_size + 1);
	if(os_read_file(filename, buffer, file_size) != OK)
	{
		printf("pjsip_config_load : can not read buffer(%d byte)\r\n", file_size);
		goto on_error;
	}
	//printf("pjsip_config_load szJSON:%s\r\n", buffer);
	cJSON *tmpobj = cJSON_Parse(buffer);
	if(tmpobj)
	{
		cJSON *pItem = cJSON_GetObjectItem (tmpobj, "pjsip-config");
		if (pItem)
		{
			if (pjsip_config_json_read_obj(pItem, ua) != OK)
			{
				cJSON_Delete(tmpobj);
				if (buffer != NULL)
					free(buffer);
				return ERROR;
			}
			cJSON_Delete(tmpobj);
		}
	}
	if (buffer != NULL)
		free(buffer);
	return OK;

on_error:
	if (buffer != NULL)
		free(buffer);
	return ERROR;
}

int pjsip_config_write(char *filename, pjsip_config_t *ua)
{
	int wrsize = 0;
	cJSON *tmpobj = cJSON_CreateObject();
	cJSON* pRoot = cJSON_CreateObject();
	if (tmpobj && pRoot)
	{
		cJSON_AddItemToObject(pRoot, "pjsip-config", tmpobj);
		printf("pjsip_config_json_write_obj\r\n");
		if (pjsip_config_json_write_obj(tmpobj, ua) != OK)
		{
			cJSON_Delete(tmpobj);
			return ERROR;
		}

		char *szJSON = NULL; //cJSON_Print(tmpobj);
		printf("pjsip_config_json_write_obj end\r\n");

		printf("pjsip_config_write cJSON_Print\r\n");
		szJSON = cJSON_Print(pRoot);
		if (szJSON)
		{
			wrsize = strlen(szJSON);
			printf("pjsip_config_write szJSON:%s\r\n", szJSON);

			if(os_write_file(filename, szJSON, wrsize) != OK)
			{
				cJSON_Delete(pRoot);
				cjson_free(szJSON);
				remove(filename);
				return ERROR;
			}
			cjson_free(szJSON);
		}
		cJSON_Delete(pRoot);
		return OK;
	}
	return ERROR;
}

#endif