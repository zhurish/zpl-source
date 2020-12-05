/*
 * x5_b_app_ac.c
 *
 *  Created on: 2019年4月3日
 *      Author: DELL
 */
#include "zebra.h"
#include "vty.h"
#include "if.h"

#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "nsm_vrf.h"
#include "nsm_interface.h"
#include "eloop.h"
#include "cJSON.h"
/*
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
*/
#include "x5_b_app.h"
#include "x5_b_json.h"
#include "x5_b_cmd.h"
#include "x5_b_web.h"

#ifdef PL_PJSIP_MODULE
#include "voip_def.h"
#include "voip_app.h"
#endif


/**********************************************/
/**********************************************/

/****************************************************************************/
/* C 模块人脸参数 */
static int _x5b_app_face_config_cjson(char **output, make_face_config_t *card)
{
	char* szJSON = NULL;
	cJSON* pRoot = cJSON_CreateObject();
	cJSON* pArray = cJSON_CreateArray();
	cJSON* pItem = cJSON_CreateObject();
	if(pRoot && pArray && pItem && card)
	{
		cJSON_AddItemToObject(pRoot, "face-config", pArray);
		cJSON_AddNumberToObject(pItem, "faceYawLeft", card->faceYawLeft);

		cJSON_AddNumberToObject(pItem, "faceYawRight", card->faceYawRight);

		cJSON_AddNumberToObject(pItem, "facePitchUp", card->facePitchUp);

		cJSON_AddNumberToObject(pItem, "facePitchDown", card->facePitchDown);


		cJSON_AddNumberToObject(pItem, "faceRecordWidth", card->faceRecordWidth);

		cJSON_AddNumberToObject(pItem, "faceRecordHeight", card->faceRecordHeight);

		cJSON_AddNumberToObject(pItem, "faceRecognizeWidth", card->faceRecognizeWidth);

		cJSON_AddNumberToObject(pItem, "faceRecognizeHeight", card->faceRecognizeHeight);

		cJSON_AddNumberToObject(pItem, "faceRoiWidth", card->faceRoiWidth);

		cJSON_AddNumberToObject(pItem, "faceRoiHeight", card->faceRoiHeight);

		cJSON_AddNumberToObject(pItem, "similarRecord", card->similarRecord);

		cJSON_AddNumberToObject(pItem, "similarRecognize", card->similarRecognize);

		cJSON_AddNumberToObject(pItem, "similarSecondRecognize", card->similarSecRecognize);

		cJSON_AddNumberToObject(pItem, "similarLiving", card->similarLiving);

		if(card->livenessSwitch)
			cJSON_AddTrueToObject(pItem, "livenessSwitch");//livenessSwitch
		else
			cJSON_AddFalseToObject(pItem, "livenessSwitch");

		cJSON_AddNumberToObject(pItem, "successIntervals", card->faceOKContinuousTime);

		cJSON_AddNumberToObject(pItem, "failureIntervals", card->faceERRContinuousTime);


		cJSON_AddItemToArray(pArray, pItem);

		szJSON = cJSON_Print(pRoot);
		if(szJSON)
		{
			if(output)
				*output = strdup(szJSON);
			cjson_free(szJSON);
		}
		cJSON_Delete(pRoot);
		return OK;
	}
	return ERROR;
}

int x5b_app_face_config_json(x5b_app_mgt_t *app, void *info, int to)
{
	int len = 0;
	char *output = NULL;
	make_face_config_t *card = (make_face_config_t *)info;
	x5b_app_mgt_t *mgt = app;
	if(app == NULL)
		mgt = x5b_app_mgt;
	zassert(mgt != NULL);
	zassert(info != NULL);
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_warn(ZLOG_APP, "Remote is Not Register");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_warn(ZLOG_APP, "OPEN CMD MSG Can not send, Unknown Remote IP Address");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(!mgt->regsync)
	{
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	_x5b_app_face_config_cjson(&output, card);
	if(output)
	{
		x5b_app_hdr_make(mgt);
		len = os_tlv_set_string(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_SET, E_CMD_FACE_CONFIG), strlen(output), output);
		mgt->app->offset += len;
		x5b_app_crc_make(mgt);
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "FACE CONFIG CMD MSG to %s:%d %d byte", inet_address(mgt->app->address),
					mgt->app->remote_port, mgt->app->slen);
		free(output);
		len = x5b_app_send_msg(mgt);
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return len;
	}
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return ERROR;
}

/**************************************************************************/
/**************************************************************************/
/********************************* Huifu **********************************/
int x5b_app_device_json(x5b_app_mgt_t *app, char *device,
						char *address, char *dire, int topf, int to)
{
	int len = 0;
	char output[512];
	x5b_app_mgt_t *mgt = app;
	if(app == NULL)
		mgt = x5b_app_mgt;
	zassert(mgt != NULL);

	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_warn(ZLOG_APP, "Remote is Not Register");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_warn(ZLOG_APP, "OPEN CMD MSG Can not send, Unknown Remote IP Address");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(!mgt->regsync)
	{
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	memset(output, 0, sizeof(output));
	if(device || address ||dire)
	{
		char *szJSON = NULL;
		cJSON* pRoot = cJSON_CreateObject();
		if(pRoot)
		{
			if(device && strlen(device))
				cJSON_AddStringToObject(pRoot, "name", device);
			if(address && strlen(address))
				cJSON_AddStringToObject(pRoot, "address", address);
			if(dire && strlen(dire))
				cJSON_AddStringToObject(pRoot, "dir", dire);
			//if(x5b_app_global)
				cJSON_AddNumberToObject(pRoot, "tof", topf);

			szJSON = cJSON_Print(pRoot);
			if(szJSON)
			{
				strcpy(output, szJSON);
				//strcpy(output, strdup(szJSON));
				cjson_free(szJSON);
				cJSON_Delete(pRoot);
			}
			else
			{
				if(mgt->mutex)
					os_mutex_unlock(mgt->mutex);
				cJSON_Delete(pRoot);
				return ERROR;
			}
		}
		else
		{
			if(mgt->mutex)
				os_mutex_unlock(mgt->mutex);
			return ERROR;
		}
	}
	if(strlen(output))
	{
		x5b_app_hdr_make(mgt);
		len = os_tlv_set_string(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_SET, E_CMD_DEVICE_OPT), strlen(output), output);
		mgt->app->offset += len;
		x5b_app_crc_make(mgt);
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "DEVICE CMD MSG to %s:%d %d byte", inet_address(mgt->app->address),
					mgt->app->remote_port, mgt->app->slen);
		len = x5b_app_send_msg(mgt);
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return len;
	}
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return ERROR;
}

#ifdef PL_PJSIP_MODULE

#ifdef PL_PJSIP_MODULE
static int x5b_sip_load_from_json(char *input)
{
	char *us = NULL;
	s_int8 user[128];
	s_int8 pass[128];
	cJSON* pItem = cJSON_Parse (input);
	cJSON* pj_tmp = NULL;
	if (pItem == NULL)
	{
		return ERROR;
	}
	pj_tmp = cJSON_GetObjectItem (pItem, "phone");
	if (pj_tmp == NULL)
	{
		return ERROR;
	}
	us = pj_tmp->valuestring;
/*	if(us)
	{
		pl_pjsip_local_number_set_api(us, FALSE);
	}*/
	pj_tmp = cJSON_GetObjectItem (pItem, "username");
	if (pj_tmp)
	{
		us = pj_tmp->valuestring;
		if(us)
		{
			memset(user, 0, sizeof(user));
			memset(pass, 0, sizeof(pass));
			pl_pjsip_username_get_api(user, pass, FALSE);
			pl_pjsip_username_set_api(us, pass, FALSE);
		}
	}
	pj_tmp = cJSON_GetObjectItem (pItem, "password");
	if (pj_tmp)
	{
		us = pj_tmp->valuestring;
		if(us)
		{
			memset(user, 0, sizeof(user));
			memset(pass, 0, sizeof(pass));
			pl_pjsip_username_get_api(user, pass, FALSE);
			pl_pjsip_username_set_api(user, us, FALSE);
		}
	}
	pj_tmp = cJSON_GetObjectItem (pItem, "sip-address");
	if (pj_tmp)
	{
		us = pj_tmp->valuestring;
		if(us)
		{
			u_int16 port = 0;
			//u_int32 ip = ntohl(inet_addr(us));
			memset(user, 0, sizeof(user));
			pl_pjsip_server_get_api(user, &port, FALSE);
			pl_pjsip_server_set_api(us, port, FALSE);

/*			voip_sip_server_get_api(NULL, &port, FALSE);
			voip_sip_server_set_api(ip, port, FALSE);*/
		}
	}
	pj_tmp = cJSON_GetObjectItem (pItem, "sip-port");
	if (pj_tmp)
	{
		if(pj_tmp->valueint)
		{
			u_int16 port = 0;
			memset(user, 0, sizeof(user));
			pl_pjsip_server_get_api(user, &port, FALSE);
			pl_pjsip_server_set_api(user, pj_tmp->valueint, FALSE);

/*			u_int32 ip = ntohl(inet_addr(us));
			voip_sip_server_get_api(&ip, NULL, FALSE);
			voip_sip_server_set_api(ip, pj_tmp->valueint, FALSE);*/
		}
	}
	pj_tmp = cJSON_GetObjectItem (pItem, "codec");
	if (pj_tmp)
	{
		us = pj_tmp->valuestring;
		if(us)
		{
			pl_pjsip_codec_default_set_api(us);
		}
	}
	pj_tmp = cJSON_GetObjectItem (pItem, "dtmf");
	if (pj_tmp)
	{
		us = pj_tmp->valuestring;
		if(us)
		{
			if(strstr(us, "2833"))
				pl_pjsip_dtmf_set_api(PJSIP_DTMF_RFC2833);
			else if(strstr(us, "info") || strstr(us, "INFO"))
				pl_pjsip_dtmf_set_api(PJSIP_DTMF_INFO);
			else if(strstr(us, "INBAND") || strstr(us, "inband"))
				pl_pjsip_dtmf_set_api(PJSIP_DTMF_INBAND);
		}
	}
	//voip_osip_restart();
	//vty_execute_shell("write memory");
	return OK;
}
#endif
#endif

int x5b_app_sip_config_parse(x5b_app_mgt_t *mgt, os_tlv_t *tlv)
{
	char *temp = NULL;
	zassert(mgt != NULL);
	zassert(tlv != NULL);
	if(E_CMD_FROM_C(tlv->tag) && tlv->val.pval)
	{
		if(tlv->len >= 1024)
			return ERROR;
		temp = malloc(tlv->len);
		if(temp == NULL)
			return ERROR;
		memset(temp, 0, tlv->len);
		memcpy(temp, tlv->val.pval, tlv->len);
		//if(X5_B_ESP32_DEBUG(MSG) && X5_B_ESP32_DEBUG(WEB))
			zlog_debug(ZLOG_APP,"Recv make Face IMG respone len=%d val=%s\r\n", tlv->len, temp);
		//zlog_debug(ZLOG_APP,"tlv->len=%d ->%s", tlv->len, temp);
#ifdef PL_PJSIP_MODULE
		if(x5b_sip_load_from_json(temp) == OK)
		{
		}
#endif
		free(temp);
	}
	return OK;
}


int x5b_app_call_phone_number(x5b_app_mgt_t *mgt, os_tlv_t *tlv, BOOL list)
{
	char *temp = NULL;
	zassert(mgt != NULL);
	zassert(tlv != NULL);
	if(E_CMD_FROM_C(tlv->tag) && tlv->val.pval)
	{
		if(tlv->len >= 1024)
			return ERROR;
		temp = malloc(tlv->len + 1);
		if(temp == NULL)
			return ERROR;
		memset(temp, 0, tlv->len + 1);
		memcpy(temp, tlv->val.pval, tlv->len);
		temp[tlv->len] = '\0';
		zlog_debug(ZLOG_APP,"Recv Phone Call IMG respone len=%d val=%s\r\n", tlv->len, temp);
		if(list == FALSE)
			x5b_app_start_call_phone(TRUE, temp);
		else
			x5b_app_start_call_user(TRUE, temp);
		free(temp);
	}
	return OK;
}

/*
typedef struct
{
	char 			phone[32];
	char 			username[32];
	char 			user_id[32];
}call_phone_t;
*/

int x5b_app_call_user_list(char *json, u_int8 *building, u_int8 *unit,
		u_int16 *room_number, void *call_phone)
{
	int i = 0, num = 0;
	call_phone_t *phone = (call_phone_t *)call_phone;
	cJSON* pArray = cJSON_Parse(json);
	cJSON* pItem = NULL;//cJSON_Parse (json);
	cJSON* pj_tmp = NULL;
	if(pArray == NULL)
	{
		zlog_debug(ZLOG_APP, "Error:%s", cJSON_GetErrorPtr());
		return 0;
	}
	num = cJSON_GetArraySize(pArray);
	if(num > 8)
		return 0;
	for (i = 0; i < num; ++i)
	{
		pItem = cJSON_GetArrayItem(pArray, i);
		if (NULL == pItem)
		{
			continue;
		}
		pj_tmp = cJSON_GetObjectItem(pItem, "use");
		if(pj_tmp && pj_tmp->valuestring)
		{
			strcpy(phone[i].username, pj_tmp->valuestring);
			zlog_debug(ZLOG_APP, " -----> %d use:%s", i, pj_tmp->valuestring);
		}

		pj_tmp = cJSON_GetObjectItem(pItem, "ID");
		if(pj_tmp && pj_tmp->valuestring)
		{
			strcpy(phone[i].user_id, pj_tmp->valuestring);
			zlog_debug(ZLOG_APP, " -----> %d ID:%s", i, pj_tmp->valuestring);
		}

		pj_tmp = cJSON_GetObjectItem(pItem, "room");
		if(pj_tmp && pj_tmp->valuestring)
		{
			if(room_number)
				*room_number = atoi(pj_tmp->valuestring);
			zlog_debug(ZLOG_APP, " -----> %d room:%s", i, pj_tmp->valuestring);
		}

		pj_tmp = cJSON_GetObjectItem(pItem, "phone");
		if(pj_tmp && pj_tmp->valuestring)
		{
			strcpy(phone[i].phone, pj_tmp->valuestring);
			zlog_debug(ZLOG_APP, " -----> %d phone:%s", i, pj_tmp->valuestring);
		}
	}
	return num;
}

int x5b_app_call_user_list_test()
{
	call_phone_t	phonetab[8];
	//char json_test[1024];
	//memset(json_test, 0, sizeof(json_test));
	/*
[
{"use":"abc","ID":"123","room":"2002","phone":"1901212"},{"use":"efg","ID":"145","room":"2002","phone":"1803434"}
]

	 */
	//os_read_file("/home/zhurish/workspace/home-work/SWPlatform/calljson", json_test, sizeof(json_test));
	//fprintf(stdout, "------:%s----\n",json_test);
	//char *json_test = "[{\"use\":\"abc\",\"ID\":\"123\",\"room\":\"2002\",\"phone\":\"1901212\"},{\"use\":\"efg\",\"ID\":\"145\",\"room\":\"2002\",\"phone\":\"1803434\"},{\"use\":\"hij\",\"ID\":\"7566\",\"room\":\"2002\",\"phone\":\"6856463\"}]";
	char *json_test = "[{\"use\":\"abc\",\"ID\":\"123\",\"room\":\"2002\",\"phone\":\"1901212\"}]";
	//if(strlen(json_test)<=0)
	//	return 0;
	u_int16 room_number = 0;
	memset(phonetab, 0, sizeof(phonetab));
	x5b_app_call_user_list(json_test, NULL, NULL,
			&room_number, phonetab);
	return OK;
}
