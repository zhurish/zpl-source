/*
 * x5_b_ubus.c
 *
 *  Created on: 2019年4月4日
 *      Author: DELL
 */

#include "zebra.h"
#include "memory.h"
#include "log.h"
#include "memory.h"
#include "str.h"
#include "linklist.h"
#include "prefix.h"
#include "table.h"
#include "vector.h"
#include "eloop.h"
#include "network.h"
#include "vty.h"
#include "cJSON.h"
#include "os_uci.h"
#include "uci_ubus.h"

#include "x5_b_app.h"
#include "x5_b_json.h"
#include "x5_b_cmd.h"
#include "x5b_dbase.h"

#include "web_x5b.h"




/*
local u_d = testing:get("facecard","db","username")
local ui_d = testing:get("facecard","db","userid")
local mc_d = testing:get("facecard","db","make_card")
local ci_d = testing:get("facecard","db","cardid")
  testing:delete("facecard","db","start_date")
  testing:delete("facecard","db","stop_date")
local ti_d = testing:get("facecard","db","cardtype")
local fi_d = testing:get("facecard","db","make_face")
local fimg_d = testing:get("facecard","db","face_img")
*/
#ifdef PL_OPENWRT_UCI
static int x5b_ubus_face_card_load(uci_face_card_t * info)
{
	char tmp[128];
	int	 ret = 0;
	if(!info)
	{
		zlog_err(ZLOG_APP, "uci face card info is null");
		return ERROR;
	}
	memset(tmp, 0, sizeof(tmp));
	ret |= os_uci_get_string("facecard.db.username", tmp);
	if(ret != OK)
	{
		if(X5_B_ESP32_DEBUG(UCI))
			zlog_err(ZLOG_APP, "can not get facecard db username!");
		return ERROR;
	}
	strncpy(info->username, tmp, MIN(sizeof(info->username), strlen(tmp)));
	memset(tmp, 0, sizeof(tmp));
	ret |= os_uci_get_string("facecard.db.userid", tmp);
	if(ret != OK)
	{
		if(X5_B_ESP32_DEBUG(UCI))
			zlog_err(ZLOG_APP, "can not get facecard db userid!");
		return ERROR;
	}
	strncpy(info->user_id, tmp, MIN(sizeof(info->user_id), strlen(tmp)));
/*	ret |= os_uci_get_integer("facecard.db.make_edit", &info->make_edit);
	if(ret != OK)
	{
		zlog_debug(ZLOG_APP, "make_edit ====== %d", info->make_edit);
		return ERROR;
	}*/
	ret = 0;
	ret |= os_uci_get_integer("facecard.db.make_card", &info->make_card);
	if(ret != OK)
	{
		if(X5_B_ESP32_DEBUG(UCI))
			zlog_err(ZLOG_APP, "can not get facecard db makecard!");
		//zlog_debug(ZLOG_APP, "make_card ====== %d", info->make_card);
		//return ERROR;
	}

	ret |= os_uci_get_integer("facecard.db.make_face", &info->make_face);
	if(ret != OK)
	{
		//zlog_debug(ZLOG_APP, "make_face ====== %d", info->make_face);
		if(X5_B_ESP32_DEBUG(UCI))
			zlog_err(ZLOG_APP, "can not get facecard db makeface!");
		//return ERROR;
	}
	if(info->make_card == 0 && info->make_face == 0)
		return ERROR;

	ret = 0;
	if(info->make_card)
	{
		memset(tmp, 0, sizeof(tmp));
		ret |= os_uci_get_string("facecard.db.cardid", tmp);
		if(ret != OK)
		{
			if(X5_B_ESP32_DEBUG(UCI))
				zlog_err(ZLOG_APP, "can not get facecard db card ID!");
			//zlog_debug(ZLOG_APP, "cardid ====== %s", tmp);
			return ERROR;
		}
		strncpy(info->cardid, tmp, MIN(APP_CARD_ID_MAX, strlen(tmp)));

		memset(tmp, 0, sizeof(tmp));
		ret |= os_uci_get_string("facecard.db.start_date", tmp);
		if(ret != OK)
		{
			if(X5_B_ESP32_DEBUG(UCI))
				zlog_err(ZLOG_APP, "can not get facecard db active time!");
			//zlog_debug(ZLOG_APP, "start_date ====== %s", tmp);
			return ERROR;
		}
		info->start_date = os_timestamp_spilt(0, tmp);
		//TODO Make Indate Time
		memset(tmp, 0, sizeof(tmp));
		ret |= os_uci_get_string("facecard.db.stop_date", tmp);
		if(ret != OK)
		{
			if(X5_B_ESP32_DEBUG(UCI))
				zlog_err(ZLOG_APP, "can not get facecard db inactive time!");
			//zlog_debug(ZLOG_APP, "stop_date ====== %s", tmp);
			return ERROR;
		}
		//TODO Make Indate Time
		info->stop_date = os_timestamp_spilt(0, tmp);
		memset(tmp, 0, sizeof(tmp));
		ret |= os_uci_get_string("facecard.db.typeid", tmp);
		if(ret != OK)
		{
			if(X5_B_ESP32_DEBUG(UCI))
				zlog_err(ZLOG_APP, "can not get facecard db cardtype!");
			//zlog_debug(ZLOG_APP, "typeid ====== %s", tmp);
			return ERROR;
		}
		strncpy(info->cardtype, tmp, MIN(sizeof(info->cardtype), strlen(tmp)));
	}

	if(info->make_face)
	{
		memset(tmp, 0, sizeof(tmp));
		ret |= os_uci_get_string("facecard.db.face_img", tmp);
		if(ret != OK)
		{
			if(X5_B_ESP32_DEBUG(UCI))
				zlog_err(ZLOG_APP, "can not get facecard db face img!");
			//zlog_debug(ZLOG_APP, "face_img ====== %s", tmp);
			return ERROR;
		}
		strncpy(info->face_img, tmp, MIN(sizeof(info->face_img), strlen(tmp)));

		//zlog_debug(ZLOG_APP, "face_img ====== %s", info->face_img);
		ret |= os_uci_get_integer("facecard.db.faceid", &info->face_id);
		if(ret != OK)
		{
			if(X5_B_ESP32_DEBUG(UCI))
				zlog_err(ZLOG_APP, "can not get facecard db faceid!");
			//zlog_debug(ZLOG_APP, "face_id ====== %d", info->face_id);
		}
	}
	return ret;
}


/*
static int x5b_ubus_face_card_ready(BOOL add)
{
	//int	 ret = ERROR;

	return OK;
}
*/

static int x5b_ubus_face_card_enable(BOOL start)
{
/*
	char tmp[APP_ID_MAX];
	int enable_tmp = 0;
	int	 ret = 0;
	ret |= os_uci_get_integer("facecard.db.enablemake", &enable_tmp);
	if(ret != OK)
	{
		zlog_debug(ZLOG_APP, "enablemake ====== %d", enable_tmp);
		return ERROR;
	}
	memset(tmp, 0, sizeof(tmp));
	ret |= os_uci_get_string("facecard.db.cardid", tmp);
	if(strlen(tmp) && x5b_app_mgt)
		x5b_app_mgt->make_edit = TRUE;
	else if(x5b_app_mgt)
		x5b_app_mgt->make_edit = FALSE;
*/

/*	if(x5b_app_mgt)
		x5b_app_mgt->make_card = start;*/
	return OK;
}

static int x5b_ubus_face_card_clear()
{
	os_uci_del("facecard", "db","start_date", NULL);
	os_uci_del("facecard", "db","stop_date", NULL);
	os_uci_del("facecard", "db","cardid", NULL);
	os_uci_del("facecard", "db","typeid", NULL);
	os_uci_del("facecard", "db","make_face", NULL);
	os_uci_del("facecard", "db","make_card", NULL);
	os_uci_del("facecard", "db","face_img", NULL);
	os_uci_commit("facecard");
	return OK;
}

static int x5b_ubus_make_face_card()
{
	//int cmd;
	int	 ret = ERROR;
	uci_face_card_t info;
	//face_card_t *card;
	memset(&info, 0, sizeof(info));
	ret = x5b_ubus_face_card_load(&info);
	if(ret != OK)
	{
		return ERROR;
	}
	x5b_ubus_face_card_clear();
	if(X5_B_ESP32_DEBUG(UCI) && X5_B_ESP32_DEBUG(MSG))
	{
		//zlog_err(ZLOG_APP, "can not get facecard db username!");
		zlog_debug(ZLOG_APP, "username:%s", strlen(info.username)? info.username:" ");
		//zlog_debug(ZLOG_APP, "username:%d", card->card_type);
		zlog_debug(ZLOG_APP, "user_id:%s", strlen(info.user_id)? info.user_id:" ");
		zlog_debug(ZLOG_APP, "card_id:%s", strlen(info.cardid)? info.cardid:" ");
		zlog_debug(ZLOG_APP, "img_id:%s",strlen(info.face_img)? info.face_img:" ");
		zlog_debug(ZLOG_APP, "face_id:%d", info.face_id);
		zlog_debug(ZLOG_APP, "start_time:%d", info.start_date);
		zlog_debug(ZLOG_APP, "stop_time:%d", info.stop_date);
		zlog_debug(ZLOG_APP, "card_type:%s", strlen(info.cardtype)? info.cardtype:" ");
		zlog_debug(ZLOG_APP, "make_card:%d", info.make_card);
		zlog_debug(ZLOG_APP, "make_face:%d", info.make_face);
	}
#if 1
	return x5b_app_make_card(&info);
#else
	if(voip_card_lookup_by_cardid_userid(info.cardid, info.user_id) == OK)
	{
		zlog_err(ZLOG_APP, "this Card and User is already exist.");
		return ERROR;
	}
	card = voip_card_node_lookup_by_username(NULL, info.user_id);
	if(card)
	{
		if(info.make_card && info.make_face)
		{
			zlog_debug(ZLOG_APP, " Update Card And Face");
			cmd = 1;
			ret |= x5b_app_delete_face_card(x5b_app_mgt, card->card_id, E_CMD_TO_A);
			//x5b_app_delete_face_card_hw(x5b_app_mgt_t *app, void *info, int to);
			face_card_t decard;
			memset(&decard, 0, sizeof(decard));
			memcpy(decard.username, card->username, sizeof(decard.username));
			memcpy(decard.user_id, card->user_id, sizeof(decard.user_id));
			memcpy(decard.card_id, card->card_id, sizeof(decard.card_id));
			decard.make_card = 1;
			decard.make_face = 1;
			ret |= x5b_app_delete_face_card_hw(x5b_app_mgt, &decard, E_CMD_TO_C);
			ret = 0;
		}
		else if(info.make_card)
		{
			zlog_debug(ZLOG_APP, " Update Card");
			cmd = 2;
			ret |= x5b_app_delete_face_card(x5b_app_mgt, card->card_id, E_CMD_TO_A);
			//x5b_app_delete_face_card_hw(x5b_app_mgt_t *app, void *info, int to);
			face_card_t decard;
			memset(&decard, 0, sizeof(decard));
			memcpy(decard.username, card->username, sizeof(decard.username));
			memcpy(decard.user_id, card->user_id, sizeof(decard.user_id));
			memcpy(decard.card_id, card->card_id, sizeof(decard.card_id));
			decard.make_card = 1;
			decard.make_face = 1;
			ret |= x5b_app_delete_face_card_hw(x5b_app_mgt, &decard, E_CMD_TO_C);
			ret = 0;
		}
		else if(info.make_face)
		{
			cmd = 3;
			zlog_debug(ZLOG_APP, " Update Face");
		}
	}
	else
		cmd = 1;

	//zlog_debug(ZLOG_APP, "datetime:%u-%u", info.start_date, info.stop_date);

	//sprintf(indate, "%d-%d", info.start_date, info.stop_date);
	if(info.make_card)
		ret |= x5b_app_add_face_card(x5b_app_mgt, &info, E_CMD_TO_A);

	if(x5b_app_mgt->X5CM/* && info.make_face*/)
		ret |= x5b_app_add_face_card(x5b_app_mgt, &info, E_CMD_TO_C);

	if(ret != OK)
		return ERROR;
	return voip_card_web_handle(cmd, &info);
#endif
}

static int x5b_ubus_delete_face_card()
{
	char tmp[128];
	uci_face_card_t info;
	//face_card_t *card;
	int	 ret = 0;
	memset(&info, 0, sizeof(info));
	ret |= os_uci_get_string("facecard.db.userid", tmp);
	if(ret != OK)
	{
		zlog_debug(ZLOG_APP, "user_id ====== %s", tmp);
		return ERROR;
	}
	strncpy(info.user_id, tmp, MIN(sizeof(info.user_id), strlen(tmp)));

	x5b_ubus_face_card_clear();

	//int x5b_app_make_card(uci_face_card_t *info);
	//int x5b_app_del_card(uci_face_card_t *info);
#if 1
	return x5b_app_del_card(&info);
#else
	card = voip_card_node_lookup_by_username(NULL, info.user_id);
	if(card == NULL)
		return OK;

	zlog_warn(ZLOG_APP, "------------------%s-send: %s", __func__, strlen(card->card_id)? card->card_id:" ");
	ret = OK;
	if(x5b_app_mgt->X5CM)
		ret |= x5b_app_delete_face_card(x5b_app_mgt, card, E_CMD_TO_C);
	if(ret == OK && card->make_card)
		ret |= x5b_app_delete_face_card(x5b_app_mgt, card->card_id, E_CMD_TO_A);
	if(ret != OK)
		return ERROR;
	return voip_card_web_handle(0, &info);
#endif
}

static int x5_b_ubus_a_open_option(void *p)
{
	char tmp[APP_ID_MAX];
	ConfiglockType info;
	int	 ret = 0, advanced = 0;
	memset(tmp, 0, sizeof(tmp));
	memset(&info, 0, sizeof(info));
	ret |= os_uci_get_integer("openconfig.open.opentime", &info.RelayOpenTime);
	if(ret != OK)
	{
		zlog_debug(ZLOG_APP, "RelayOpenTime ====== %s", info.RelayOpenTime);
		return ERROR;
	}
	ret |= os_uci_get_integer("openconfig.open.openwaittime", &info.RelayOpenWaitOpenDoorTime);
	if(ret != OK)
	{
		zlog_debug(ZLOG_APP, "RelayOpenWaitOpenDoorTime ====== %s", info.RelayOpenWaitOpenDoorTime);
		return ERROR;
	}
	ret |= os_uci_get_integer("openconfig.open.openholdtime", &info.DoorKeepOpenTime);
	if(ret != OK)
	{
		zlog_debug(ZLOG_APP, "DoorKeepOpenTime ====== %s", info.DoorKeepOpenTime);
		return ERROR;
	}
	ret |= os_uci_get_string("openconfig.open.relay_active_level", tmp);
	if(ret != OK)
	{
		zlog_debug(ZLOG_APP, "DoorSensorOutPutLevle ====== %s", tmp);
		return ERROR;
	}
	if(strstr(tmp, "High"))
		info.DoorSensorOutPutLevle = 1;
	else
		info.DoorSensorOutPutLevle = 0;

	ret |= os_uci_get_integer("openconfig.open.tamperalarm", &info.TamperAlarm);
	if(ret != OK)
	{
		zlog_debug(ZLOG_APP, "TamperAlarm ====== %s", info.TamperAlarm);
		return ERROR;
	}
	ret |= os_uci_get_integer("openconfig.open.keep_open_alarm", &info.DoorKeepOpentTimeOutAlarm);
	if(ret != OK)
	{
		zlog_debug(ZLOG_APP, "DoorKeepOpentTimeOutAlarm ====== %s", info.DoorKeepOpentTimeOutAlarm);
		return ERROR;
	}
	memset(tmp, 0, sizeof(tmp));
	ret |= os_uci_get_string("product.global.location", tmp);
	if(ret != OK)
	{
		zlog_debug(ZLOG_APP, "LockRole ====== %s", tmp);
		return ERROR;
	}
	if(strstr(tmp, "Unit"))
		info.LockRole = 2;
	else if(strstr(tmp, "Wall"))
		info.LockRole = 1;
	else if(strstr(tmp, "Building"))
		info.LockRole = 3;

	if(info.DoorKeepOpentTimeOutAlarm == 0)
		info.DoorKeepOpenTime = 0XFF;

	memset(tmp, 0, sizeof(tmp));
	ret |= os_uci_get_string("openconfig.open.opentype", tmp);
	if(ret != OK)
	{
		zlog_debug(ZLOG_APP, "opentype ====== %s", tmp);
		return ERROR;
	}
	if(strstr(tmp, "FaceOrCard"))
		x5b_app_mgt->opentype = OPEN_FACE_OR_CARD;
	else if(strstr(tmp, "FaceAndCard"))
		x5b_app_mgt->opentype = OPEN_FACE_AND_CARD;
	else if(strstr(tmp, "Face"))
		x5b_app_mgt->opentype = OPEN_FACE;
	else if(strstr(tmp, "Card"))
		x5b_app_mgt->opentype = OPEN_CARD;


	ret |= os_uci_get_integer("openconfig.open.advanced", &advanced);
	if(ret != OK)
	{
		zlog_debug(ZLOG_APP, "advanced ====== %d", advanced);
		//return ERROR;
	}
	//if(advanced)
	{
		u_int8 val = 0;
		memset(tmp, 0, sizeof(tmp));
		ret |= os_uci_get_string("openconfig.open.wiggins", tmp);
		if(ret != OK)
		{
			zlog_debug(ZLOG_APP, "wiggins ====== %s", tmp);
			return ERROR;
		}
		if(strstr(tmp, "26-Bit"))
			val = 26;
		else if(strstr(tmp, "34-Bit"))
			val = 34;
		else if(strstr(tmp, "66-Bit"))
			val = 66;
		if(val)
		{
			x5b_app_wiggins_setting(NULL, val, E_CMD_TO_A);
			os_msleep(500);
		}
		if(x5b_app_mgt)
			x5b_app_mgt->app_a.wiggins = val;
	}
	if(x5b_app_mgt && x5b_app_mgt->X5CM)
		ret |= x5b_app_open_option(NULL, &info, E_CMD_TO_C);
	if(ret != OK)
		return ERROR;
	return x5b_app_open_option(NULL, &info, E_CMD_TO_A);
}

static int x5_b_ubus_face_config_option(void *p)
{
	make_face_config_t info;
	int	 ret = 0;
	memset(&info, 0, sizeof(info));

	ret |= os_uci_get_integer("openconfig.faceopt.faceYawLeft", &info.faceYawLeft);
	if(ret != OK)
	{
		zlog_debug(ZLOG_APP, "faceYawLeft ====== %s", info.faceYawLeft);
		return ERROR;
	}

	ret |= os_uci_get_integer("openconfig.faceopt.faceYawRight", &info.faceYawRight);
	if(ret != OK)
	{
		zlog_debug(ZLOG_APP, "faceYawRight ====== %s", info.faceYawRight);
		return ERROR;
	}

	ret |= os_uci_get_integer("openconfig.faceopt.facePitchUp", &info.facePitchUp);
	if(ret != OK)
	{
		zlog_debug(ZLOG_APP, "facePitchUp ====== %s", info.facePitchUp);
		return ERROR;
	}

	ret |= os_uci_get_integer("openconfig.faceopt.facePitchDown", &info.facePitchDown);
	if(ret != OK)
	{
		zlog_debug(ZLOG_APP, "facePitchDown ====== %s", info.facePitchDown);
		return ERROR;
	}

	ret |= os_uci_get_integer("openconfig.faceopt.faceRecordWidth", &info.faceRecordWidth);
	if(ret != OK)
	{
		zlog_debug(ZLOG_APP, "faceRecordWidth ====== %s", info.faceRecordWidth);
		return ERROR;
	}

	ret |= os_uci_get_integer("openconfig.faceopt.faceRecordHeight", &info.faceRecordHeight);
	if(ret != OK)
	{
		zlog_debug(ZLOG_APP, "faceRecordHeight ====== %s", info.faceRecordHeight);
		return ERROR;
	}

	ret |= os_uci_get_integer("openconfig.faceopt.faceRecognizeWidth", &info.faceRecognizeWidth);
	if(ret != OK)
	{
		zlog_debug(ZLOG_APP, "faceRecognizeWidth ====== %s", info.faceRecognizeWidth);
		return ERROR;
	}

	ret |= os_uci_get_integer("openconfig.faceopt.faceRecognizeHeight", &info.faceRecognizeHeight);
	if(ret != OK)
	{
		zlog_debug(ZLOG_APP, "faceRecognizeHeight ====== %s", info.faceRecognizeHeight);
		return ERROR;
	}

	ret |= os_uci_get_float("openconfig.faceopt.similarRecord", &info.similarRecord);
	if(ret != OK)
	{
		zlog_debug(ZLOG_APP, "similarRecord ====== %s", info.similarRecord);
		return ERROR;
	}

	ret |= os_uci_get_float("openconfig.faceopt.similarRecognize", &info.similarRecognize);
	if(ret != OK)
	{
		zlog_debug(ZLOG_APP, "similarRecognize ====== %s", info.similarRecognize);
		return ERROR;
	}

	ret |= os_uci_get_float("openconfig.faceopt.similarSecondRecognize", &info.similarSecondRecognize);
	if(ret != OK)
	{
		zlog_debug(ZLOG_APP, "similarSecondRecognize ====== %s", &info.similarSecondRecognize);
		return ERROR;
	}

	ret |= os_uci_get_integer("openconfig.faceopt.livenessSwitch", &info.livenessSwitch);
	if(ret != OK)
	{
		zlog_debug(ZLOG_APP, "livenessSwitch ====== %d", info.livenessSwitch);
		return ERROR;
	}

	return x5b_app_face_config_json(NULL, &info, E_CMD_TO_C);
}
#endif


int x5b_app_A_update_test(char *filename)
{
	char path[128];
	if(!filename || strlen(filename) <= 0)
	{
		zlog_debug(ZLOG_APP, "x5b_app_A_update_test filename is null");
		return ERROR;
	}
	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path), "file install-%s", filename);
	return x5b_app_A_update_handle(path);
}

int x5b_app_A_unit_test_set_api(BOOL enable)
{
	int len = 0;
	x5b_app_mgt_t *mgt = x5b_app_mgt;
	zassert(mgt != NULL);
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	x5b_app_make_update(mgt, E_CMD_TO_A);
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
		zlog_warn(ZLOG_APP, "A Module Unit Test MSG Can not send, Unknown Remote IP Address");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}

	x5b_app_hdr_make(mgt);
	len = os_tlv_set_zero(mgt->app->sbuf + mgt->app->offset,
			E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_STATUS, E_CMD_UNIT_TEST), 0);
	mgt->app->offset += len;
	x5b_app_crc_make(mgt);
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "A Module Unit Test MSG to %s:%d %d byte", inet_address(mgt->app->address),
				mgt->app->remote_port, mgt->app->slen);
	len = x5b_app_send_msg(mgt);
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return len;
}

#ifdef PL_OPENWRT_UCI
static int x5_b_ubus_uci_factory_cb_handle(char *buf)
{
	char tmp[128];
	int	 ret = 0;
	char device[32];
	char address[32];
	char dire[32];
	os_msleep(1500);
	ret = os_uci_get_string("product.global.type", tmp);
	if(ret == OK && strlen(tmp) && x5b_app_mgt)
	{
		if(strstr(tmp, "BM"))
			x5b_app_mgt->X5CM = FALSE;
		else if(strstr(tmp, "CM"))
			x5b_app_mgt->X5CM = TRUE;
		else
			x5b_app_mgt->X5CM = FALSE;
	}
	memset(tmp, 0, sizeof(tmp));
	ret |= os_uci_get_string("product.global.customizer", tmp);
	if(ret == OK && strlen(tmp) && x5b_app_mgt)
	{
		if(strstr(tmp, "Secom"))
			x5b_app_mgt->customizer = CUSTOMIZER_SECOM;
		else if(strstr(tmp, "Huifu"))
			x5b_app_mgt->customizer = CUSTOMIZER_HUIFU;
		else
			x5b_app_mgt->customizer = CUSTOMIZER_NONE;
	}
	memset(device, 0, sizeof(device));
	ret |= os_uci_get_string("product.global.device_name", device);
	if(ret != OK)
	{
		zlog_debug(ZLOG_APP, "device ====== %s", device);
		return ERROR;
	}
	memset(address, 0, sizeof(address));
	ret |= os_uci_get_string("product.global.server_address", address);
	if(ret != OK)
	{
		zlog_debug(ZLOG_APP, "server_address ====== %s", address);
		return ERROR;
	}
	memset(dire, 0, sizeof(dire));
	ret |= os_uci_get_string("product.global.direction", dire);
	if(ret != OK)
	{
		zlog_debug(ZLOG_APP, "direction ====== %s", dire);
		return ERROR;
	}
	if(x5b_app_mgt)
		x5b_app_device_json(x5b_app_mgt, device,
							address, dire, E_CMD_TO_C);
	return OK;
}

/*
#ifdef PL_OPENWRT_UCI
	uci_ubus_cb_install(voip_ubus_uci_update_cb);
#endif
*/
#if 1
static int x5b_uci_userauth_cb_handle(char *buf);
static int x5_b_ubus_uci_update_cb_handle(char *buf)
{
	if(strstr(buf, "card"))
	{
		/*if(strstr(buf, "ready"))
		{
			free(buf);
			return x5b_ubus_face_card_ready(TRUE);
		}
		else */if(strstr(buf, "add"))
		{
			return x5b_ubus_make_face_card();
		}
		else if(strstr(buf, "del"))
		{
			return x5b_ubus_delete_face_card();
		}
		else if(strstr(buf, "start"))
		{
			return x5b_ubus_face_card_enable(TRUE);
		}
		else if(strstr(buf, "stop"))
		{
			return x5b_ubus_face_card_enable(FALSE);
		}
		return OK;
	}
	else if(strstr(buf, "app"))
	{
		if(strstr(buf, "open-option"))
		{
			//os_time_create_once(x5_b_ubus_a_open_option, NULL, 1500);
			return x5_b_ubus_a_open_option(NULL);
		}
		else if(strstr(buf, "face-option"))
		{
			//os_time_create_once(x5_b_ubus_a_open_option, NULL, 1500);
			return x5_b_ubus_face_config_option(NULL);
		}
	}
	x5b_uci_userauth_cb_handle(buf);

	x5b_ubus_face_card_enable(FALSE);
	return OK;
}

int x5_b_ubus_uci_update_cb(char *buf, int len)
{
	os_sleep(1);
	if(strstr(buf, "file"))
	{
		return x5b_app_A_update_handle(buf);
	}
	else if(strstr(buf, "factory"))
	{
		os_job_add(x5_b_ubus_uci_factory_cb_handle, NULL);
		return OK;//x5_b_ubus_uci_factory_cb_handle(buf);
	}
	else if(strstr(buf, "reboot"))
	{
		x5b_app_reboot_request(NULL, E_CMD_TO_A, FALSE);
		if(x5b_app_mgt && x5b_app_mgt->X5CM)
			x5b_app_reboot_request(NULL, E_CMD_TO_C, FALSE);
		return OK;
	}
	else if(strstr(buf, "reset"))
	{
		x5b_app_reboot_request(NULL, E_CMD_TO_A, TRUE);
		if(x5b_app_mgt && x5b_app_mgt->X5CM)
			x5b_app_reboot_request(NULL, E_CMD_TO_C, TRUE);

		return OK;
	}
	else if(strstr(buf, "card") && strstr(buf, "refresh"))
	{
		voip_card_web_select_all();
		return OK;
	}
	else if(strstr(buf, "app") && strstr(buf, "open-option"))
	{
		x5_b_ubus_uci_update_cb_handle(buf);
		return OK;
	}
	if(strstr(buf, "img-show"))
	{
		//zlog_debug(ZLOG_APP, "=============%s =%s", __func__, buf);
		;//return x5b_app_face_img_show(NULL, buf+strlen("img-show "));
	}
	return x5_b_ubus_uci_update_cb_handle(buf);
}
#else
static int x5b_uci_userauth_cb_handle(char *buf);
static int app_time_job_id = -1;

static int x5_b_ubus_uci_update_cb_handle(char *buf)
{
	if(app_time_job_id > 0)
		app_time_job_id = 0;
	if(strstr(buf, "card"))
	{
		/*if(strstr(buf, "ready"))
		{
			free(buf);
			return x5b_ubus_face_card_ready(TRUE);
		}
		else */if(strstr(buf, "add"))
		{
			if(app_time_job_id == 0)
			free(buf);
			return x5b_ubus_make_face_card();
		}
		else if(strstr(buf, "del"))
		{
			if(app_time_job_id == 0)
			free(buf);
			return x5b_ubus_delete_face_card();
		}
		else if(strstr(buf, "start"))
		{
			if(app_time_job_id == 0)
			free(buf);
			return x5b_ubus_face_card_enable(TRUE);
		}
		else if(strstr(buf, "stop"))
		{
			if(app_time_job_id == 0)
			free(buf);
			return x5b_ubus_face_card_enable(FALSE);
		}
		if(app_time_job_id == 0)
		free(buf);
		return OK;
	}
	else if(strstr(buf, "app"))
	{
		if(strstr(buf, "A-update"))
		{
			if(app_time_job_id == 0)
				free(buf);
			//os_time_create_once(x5_b_ubus_a_open_option, NULL, 1500);
			return x5_b_ubus_a_open_option(NULL);
		}
		else if(strstr(buf, "C-update"))
		{
			if(app_time_job_id == 0)
			free(buf);
			//os_time_create_once(x5_b_ubus_a_open_option, NULL, 1500);
			return x5_b_ubus_face_config_option(NULL);
		}
	}
	x5b_uci_userauth_cb_handle(buf);

	if(app_time_job_id == 0)
		free(buf);
	x5b_ubus_face_card_enable(FALSE);
	return OK;
}

int x5_b_ubus_uci_update_cb(char *buf, int len)
{
	os_sleep(1);
	if(strstr(buf, "file"))
	{
		return x5b_app_A_update_handle(buf);
	}
	else if(strstr(buf, "factory"))
	{
		return x5_b_ubus_uci_factory_cb_handle(buf);
	}
	else if(strstr(buf, "reboot"))
	{
		x5b_app_reboot_request(NULL, E_CMD_TO_A, FALSE);
		if(x5b_app_mgt && x5b_app_mgt->X5CM)
			x5b_app_reboot_request(NULL, E_CMD_TO_C, FALSE);
		return OK;
	}
	else if(strstr(buf, "reset"))
	{
/*		remove("/app/etc/dbase");
		remove("/app/etc/card");
		remove("/app/etc/thlog.log");*/
		x5b_app_reboot_request(NULL, E_CMD_TO_A, TRUE);
		if(x5b_app_mgt && x5b_app_mgt->X5CM)
			x5b_app_reboot_request(NULL, E_CMD_TO_C, TRUE);

		return OK;
	}
	else if(strstr(buf, "card") && strstr(buf, "refresh"))
	{
		voip_card_web_select_all();
		return OK;
	}
	else if(strstr(buf, "app") && strstr(buf, "update"))
	{
		x5_b_ubus_uci_update_cb_handle(buf);
/*		if(app_time_job_id > 0)
		{
			os_time_destroy(app_time_job_id);
			app_time_job_id = -1;
		}
		app_time_job_id = os_time_create_once(x5_b_ubus_uci_update_cb_handle, strdup(buf), 1000);*/
		return OK;
	}
	if(strstr(buf, "img-show"))
	{
		//zlog_debug(ZLOG_APP, "=============%s =%s", __func__, buf);
		;//return x5b_app_face_img_show(NULL, buf+strlen("img-show "));
	}
	return x5_b_ubus_uci_update_cb_handle(buf);
}
#endif


/***********************************************************************/
//  sys_app_sync_cmd("lua-sync -m userauth -c del")
/*testing:delete("userauth","db","username")
testing:delete("userauth","db","userid")
testing:delete("userauth","db","cardid")
testing:delete("userauth","db","phone")
testing:delete("userauth","db","room")
testing:delete("userauth","db","start_date")
testing:delete("userauth","db","stop_date")
testing:delete("userauth","db","cardtype")
testing:delete("userauth","db","faceimg")
testing:delete("userauth","db","faceid")*/
typedef struct
{
	char 			username[APP_USERNAME_MAX];
	char 			userid[APP_ID_MAX];
	char 			phone[APP_ID_MAX];
	char 			room[APP_ID_MAX];
	char 			cardid[APP_CARD_ID_MAX + 1];
	u_int32 		faceid;
	char 			faceimg[APP_IMG_ID_MAX];
	u_int32 		start_date;
	u_int32 		stop_date;
	u_int8	 		cardtype;
}uci_userauth_t;
#include "x5b_facecard.h"
static int x5b_ubus_userauth_load(uci_userauth_t * info)
{
	char tmp[128];
	int	 ret = 0;
	memset(tmp, 0, sizeof(tmp));
	ret |= os_uci_get_string("userauth.db.username", tmp);
	if(ret != OK)
	{
		zlog_debug(ZLOG_APP, "username ====== %s", tmp);
		return ERROR;
	}
	strncpy(info->username, tmp, MIN(sizeof(info->username), strlen(tmp)));
	memset(tmp, 0, sizeof(tmp));
	ret |= os_uci_get_string("userauth.db.userid", tmp);
	if(ret != OK)
	{
		zlog_debug(ZLOG_APP, "userid ====== %s", tmp);
		return ERROR;
	}
	strncpy(info->userid, tmp, MIN(sizeof(info->userid), strlen(tmp)));

	memset(tmp, 0, sizeof(tmp));
	ret |= os_uci_get_string("userauth.db.phone", tmp);
	if(strlen(tmp))
		strncpy(info->phone, tmp, MIN(sizeof(info->phone), strlen(tmp)));

	ret = 0;
	memset(tmp, 0, sizeof(tmp));
	ret |= os_uci_get_string("userauth.db.room", tmp);
	if(strlen(tmp))
		strncpy(info->room, tmp, MIN(sizeof(info->room), strlen(tmp)));

	ret = 0;
	//if(info->make_card)

	memset(tmp, 0, sizeof(tmp));
	ret |= os_uci_get_string("userauth.db.cardid", tmp);
	if(ret != OK)
	{
		zlog_debug(ZLOG_APP, "can not load cardid.");
		return ERROR;
	}
	if(strlen(tmp))
	{
		strncpy(info->cardid, tmp, MIN(APP_CARD_ID_MAX, strlen(tmp)));
		ret = 0;
		memset(tmp, 0, sizeof(tmp));
		ret |= os_uci_get_string("userauth.db.start_date", tmp);
		if(strlen(tmp))
			info->start_date = os_timestamp_spilt(0, tmp);

		ret = 0;
		//TODO Make Indate Time
		memset(tmp, 0, sizeof(tmp));
		ret |= os_uci_get_string("userauth.db.stop_date", tmp);
		//TODO Make Indate Time
		if(strlen(tmp))
			info->stop_date = os_timestamp_spilt(0, tmp);

		ret = 0;
		memset(tmp, 0, sizeof(tmp));
		ret |= os_uci_get_string("userauth.db.cardtype", tmp);
		if(strlen(tmp))
		{
			if(strstr(tmp, "Blacklist"))
				info->cardtype = 1;
			else if(strstr(tmp, "Whitelist"))
				info->cardtype = 2;
			else
				info->cardtype = 0;
		}
			//strncpy(info->cardtype, tmp, MIN(sizeof(info->cardtype), strlen(tmp)));
	}

	//if(info->make_face)
	if(x5b_app_mode_X5CM())
	{
		ret = 0;
		memset(tmp, 0, sizeof(tmp));
		ret |= os_uci_get_string("userauth.db.faceimg", tmp);
		if(ret != OK)
		{
			zlog_debug(ZLOG_APP, "face_img ====== %s", tmp);
			//return ERROR;
		}
		if(strlen(tmp))
		{
			strncpy(info->faceimg, tmp, MIN(sizeof(info->faceimg), strlen(tmp)));
			ret = 0;
			ret |= os_uci_get_integer("userauth.db.faceid", &info->faceid);
		}
	}
	return ret;
}

/*
static int x5b_ubus_userauth_clear(void *p)
{
	//return OK;
	os_uci_del("userauth", "db","username", NULL);
	os_uci_del("userauth", "db","userid", NULL);
	os_uci_del("userauth", "db","phone", NULL);
	os_uci_del("userauth", "db","room", NULL);
	os_uci_del("userauth", "db","cardid", NULL);
	os_uci_del("userauth", "db","cardtype", NULL);
	os_uci_del("userauth", "db","start_date", NULL);
	os_uci_del("userauth", "db","stop_date", NULL);
	os_uci_del("userauth", "db","faceid", NULL);
	os_uci_del("userauth", "db","faceimg", NULL);
	os_uci_commit("userauth");
	return OK;
}
*/

static int x5b_ubus_userauth_clear(void *p)
{
	//return OK;
	os_uci_del("userauth", "db","username", NULL);
	os_uci_del("userauth", "db","userid", NULL);
	os_uci_del("userauth", "db","phone", NULL);
	os_uci_del("userauth", "db","room", NULL);
	os_uci_del("userauth", "db","cardid", NULL);
	os_uci_del("userauth", "db","cardtype", NULL);
	os_uci_del("userauth", "db","start_date", NULL);
	os_uci_del("userauth", "db","stop_date", NULL);
	if(x5b_app_mode_X5CM())
	{
		os_uci_del("userauth", "db","faceid", NULL);
		os_uci_del("userauth", "db","faceimg", NULL);
	}
	os_uci_commit("userauth");
	return OK;
}


static int x5b_uci_add_userauth(uci_userauth_t *temp, user_face_card_t * card,
								x5b_app_mgt_t *app)
{
	int ret = 0;
	permiListType pacard;
	uci_face_card_t info;

	memset(&info, 0, sizeof(info));
	if(strlen(temp->username))
		strncpy(info.username, temp->username, MIN(sizeof(info.username), strlen(temp->username)));

	if(strlen(temp->userid))
		strncpy(info.user_id, temp->userid, MIN(sizeof(info.user_id), strlen(temp->userid)));

	if(strlen(temp->cardid))
	{
		info.make_card = 1;
		strncpy(info.cardid, temp->cardid, MIN(APP_CARD_ID_MAX, strlen(temp->cardid)));
	}
	if(x5b_app_mode_X5CM())
	{
		if(strlen(temp->faceimg))
		{
			info.make_face = 1;
			strncpy(info.face_img, temp->faceimg, MIN(sizeof(info.face_img), strlen(temp->faceimg)));
			//strcpy(info.face_img, temp->faceimg);
		}
	}
	if(temp->cardtype == 2)
		strcpy(info.cardtype, "Whitelist");
	else if(temp->cardtype == 1)
		strcpy(info.cardtype, "Blacklist");
	else
		strcpy(info.cardtype, "Unknow");

	if(card)
	{
		uci_face_card_t decard;
		memset(&decard, 0, sizeof(decard));
		ret = x5b_user_get_card(NULL, temp->userid, decard.cardid, 0);
		if(app->X5CM)
			ret = x5b_user_get_face(NULL, temp->userid, &decard.face_id, 0);

		if(info.make_card && info.make_face)//制卡，发脸
		{
			if(!str_isempty(decard.cardid, APP_CARD_ID_MAX))
			{
				decard.make_card = 1;
				//memcpy(decard.cardid, card->card_id, sizeof(decard.cardid));
				zlog_debug(ZLOG_APP, "delete old card to A");
				x5b_app_delete_card(app, decard.cardid, E_CMD_TO_A);
			}
			if(app->X5CM/* && decard.face_id*/)
			{
				zlog_debug(ZLOG_APP, "delete old face and card to C");
				if(strlen(card->username))
					memcpy(decard.username, card->username, sizeof(decard.username));
				if(strlen(card->userid))
					memcpy(decard.user_id, card->userid, sizeof(decard.user_id));
				decard.make_face = 1;
				//decard.face_id = card->face_id;
				x5b_app_del_card_json(app, &decard, E_CMD_TO_C);
			}
		}
		else if(info.make_card)
		{
			//uci_face_card_t decard;
			memset(&decard, 0, sizeof(decard));
			if(!str_isempty(decard.cardid, APP_CARD_ID_MAX))
			{
				decard.make_card = 1;
				//memcpy(decard.cardid, decard.cardid, sizeof(decard.cardid));
				zlog_debug(ZLOG_APP, "delete old card to A");
				x5b_app_delete_card(app, decard.cardid, E_CMD_TO_A);
			}
			if(app->X5CM /*&& decard.face_id*/)
			{
				zlog_debug(ZLOG_APP, "delete old card to C");
				if(strlen(card->username))
					memcpy(decard.username, card->username, sizeof(decard.username));
				if(strlen(card->userid))
					memcpy(decard.user_id, card->userid, sizeof(decard.user_id));
				x5b_app_del_card_json(app, &decard, E_CMD_TO_C);
			}
		}
		else if(app->X5CM && info.make_face)
		{
			//uci_face_card_t decard;
			memset(&decard, 0, sizeof(decard));
			//if(decard.face_id)
			{
				zlog_debug(ZLOG_APP, "delete old card to C");
				if(strlen(card->username))
					memcpy(decard.username, card->username, sizeof(decard.username));
				if(strlen(card->userid))
					memcpy(decard.user_id, card->userid, sizeof(decard.user_id));
				decard.make_face = 1;
				//decard.face_id = card->face_id;
				x5b_app_del_card_json(app, &decard, E_CMD_TO_C);
			}
		}
	}
	ret = 0;
	memset(&pacard, 0, sizeof(permiListType));
	if(strlen(temp->cardid))
	{
		card_id_string_to_hex(temp->cardid, MIN(strlen(temp->cardid), APP_CARD_ID_MAX), pacard.ID);
	}
	if(!str_isempty(pacard.ID, 8))
	{
		int zone = x5b_app_timezone_offset_api(NULL);
		if(zone < 0)
			info.start_date = pacard.start_time = htonl(temp->start_date /*+ OS_SEC_HOU_V(abs(zone))*/);
		else
			info.start_date = pacard.start_time = htonl(temp->start_date /*- OS_SEC_HOU_V(zone)*/);

		if(zone < 0)
			info.stop_date = pacard.stop_time = htonl(temp->stop_date /*+ OS_SEC_HOU_V(abs(zone))*/);
		else
			info.stop_date = pacard.stop_time = htonl(temp->stop_date /*- OS_SEC_HOU_V(zone)*/);

		//pacard.start_time = htonl(temp->start_date);
		//pacard.stop_time = htonl(temp->stop_date);
		pacard.status = temp->cardtype;
		ret |= x5b_app_add_card(app, &pacard, E_CMD_TO_A);
		if(ret != OK)
			zlog_debug(ZLOG_APP, "Add user '%s' to A warning", temp->username);
	}
	if(ret == OK && app->X5CM)
	{
		info.start_date = 0;
		if(info.start_date == 0)
		{
			int zone = x5b_app_timezone_offset_api(NULL);
			if(zone < 0)
				info.start_date = (temp->start_date /*+ OS_SEC_HOU_V(abs(zone))*/);
			else
				info.start_date = (temp->start_date /*- OS_SEC_HOU_V(zone)*/);
			if(zone < 0)
				info.stop_date = (temp->stop_date/* + OS_SEC_HOU_V(abs(zone))*/);
			else
				info.stop_date = (temp->stop_date/* - OS_SEC_HOU_V(zone)*/);
		}
		else
		{
			info.start_date = ntohl(info.start_date);
			info.stop_date = ntohl(info.stop_date);
		}
		ret |= x5b_app_add_card_json(app, &info, E_CMD_TO_C);
		if(ret != OK)
			zlog_debug(ZLOG_APP, "Add user '%s' to C warning", temp->username);
	}
	return ret;
}

static int x5b_uci_userauth_cb_handle(char *buf)
{
	int ret = 0;
	uci_userauth_t temp;
	user_face_card_t * card = NULL;
/*	if(app_time_job_id > 0)
		app_time_job_id = 0;*/
	if(strstr(buf, "userauth"))
	{
		memset(&temp, 0, sizeof(uci_userauth_t));

		if(strstr(buf, "add"))
		{
			if(x5b_ubus_userauth_load(&temp) == ERROR)
			{
				zlog_err(ZLOG_APP, "Can not load uci config param.");
				return ERROR;
			}

			zlog_debug(ZLOG_APP, "===================");
			zlog_debug(ZLOG_APP, "username:%s", strlen(temp.username)? temp.username:" ");
			zlog_debug(ZLOG_APP, "user_id:%s", strlen(temp.userid)? temp.userid:" ");
			zlog_debug(ZLOG_APP, "card_id:%s", strlen(temp.cardid)? temp.cardid:" ");
			if(x5b_app_mode_X5CM())
			{
				zlog_debug(ZLOG_APP, "img_id:%s",strlen(temp.faceimg)? temp.faceimg:" ");
				zlog_debug(ZLOG_APP, "face_id:%d", temp.faceid);
			}
			zlog_debug(ZLOG_APP, "start_time:%d", temp.start_date);
			zlog_debug(ZLOG_APP, "stop_time:%d", temp.stop_date);
			zlog_debug(ZLOG_APP, "card_type:%d", temp.cardtype);
			zlog_debug(ZLOG_APP, "phone:%s", strlen(temp.phone)? temp.phone:" ");
			zlog_debug(ZLOG_APP, "room:%d", strlen(temp.room)? atoi(temp.room):0);
			zlog_debug(ZLOG_APP, "===================");


/*			if(app_time_job_id == 0)
			free(buf);*/
			card = x5b_user_lookup_by_username(NULL, temp.userid);
			ret = x5b_uci_add_userauth(&temp, card, x5b_app_mgt);
			if(ret != OK)
			{
				voip_facecard_web_select_all();
				x5b_ubus_userauth_clear(NULL);
				return ERROR;
			}
			if(strlen(temp.phone) && strlen(temp.room))//update or add card
			{
				voip_dbase_t *db = NULL;
				//u_int16 room_number = 0;
				char user_id[64];
				//char phone[64];
				memset(user_id, 0, sizeof(user_id));
				//memset(phone, 0, sizeof(phone));
				if(voip_dbase_get_user_by_phone(temp.phone, NULL, user_id) == OK &&
						memcmp(user_id, temp.userid, sizeof(temp.userid)) != 0 )
				{
					x5b_ubus_userauth_clear(NULL);
					zlog_err(ZLOG_APP, "phone '%s' is already binding to another user.", temp.phone);
					return ERROR;
				}

				if((db = voip_dbase_lookup_by_room(0, 0, atoi(temp.room))))
				{
					int i = 0;
					for(i = 0; i < APP_MULTI_NUMBER_MAX; i++)
					{
						if(db->phonetab[i].use_flag == 0)
							continue;
						if( (memcmp(db->phonetab[i].user_id, temp.userid, sizeof(temp.userid)) != 0) )
						{
							x5b_ubus_userauth_clear(NULL);
							zlog_err(ZLOG_APP, "room '%s' is already binding to another user.", temp.room);
							return ERROR;
						}
					}
				}
/*				if(voip_dbase_get_room_phone_by_user(temp.userid, &room_number,
						phone, NULL) == OK && room_number != atoi(temp.room))
				{
					x5b_ubus_userauth_clear(NULL);
					zlog_err(ZLOG_APP, "room '%s' is already binding to another user.", temp.room);
					return ERROR;
				}*/
			}
			if(card)
			{
				//int x5b_app_add_card_json(x5b_app_mgt_t *app, void *info, int to)
				if(strlen(temp.cardid))//update or add card
				{
					user_face_card_t * cardtmp = x5b_user_lookup_by_cardid(temp.cardid);
					if(cardtmp == NULL)
					{
						zlog_debug(ZLOG_APP, "====================cardid=>%s", temp.cardid);
						ret |= x5b_user_update_card(temp.username, temp.userid, temp.cardid,
									  temp.start_date, temp.stop_date, temp.cardtype, 0);
					}
					else
					{
						if(memcmp(cardtmp->userid, card->userid, sizeof(card->userid)) == 0)
						{
							zlog_debug(ZLOG_APP, "====================cardid=>%s", temp.cardid);
							ret |= x5b_user_update_card(temp.username, temp.userid, temp.cardid,
										  temp.start_date, temp.stop_date, temp.cardtype, 0);
						}
						else
						{
							x5b_ubus_userauth_clear(NULL);
							zlog_err(ZLOG_APP, "card ID '%s' is already binding to another user.", temp.cardid);
							return ERROR;
						}
					}
				}
				if(x5b_app_mode_X5CM())
				{
					if(strlen(temp.faceimg))//update or add card
					{
						ret |= x5b_user_update_face(temp.username, temp.userid, temp.faceimg, temp.faceid);
					}
				}
				if(strlen(temp.phone) && strlen(temp.room))//update or add card
				{
					voip_dbase_del_room_phone(0, 0, 0, NULL, NULL, temp.userid);

					ret |= voip_dbase_add_room_phone(0, 0, atoi(temp.room),
													 temp.phone, temp.username, temp.userid);
/*					ret |= voip_dbase_update_room_phone(0, 0, atoi(temp.room),
													 temp.phone, temp.username, temp.userid);*/
					//ret |= x5b_user_add_face(temp.username, temp.userid, temp.faceimg, temp.faceid);
				}
				else
				{
					ret |= voip_dbase_del_room_phone(0, 0, 0, NULL, NULL, temp.userid);
				}
				voip_facecard_web_select_all();
				x5b_ubus_userauth_clear(NULL);
				return ret;
			}
			else
			{
				card = x5b_user_add(temp.username, temp.userid);
				if(card)
				{
					if(strlen(temp.cardid))//update or add card
					{
						ret |= x5b_user_add_card(temp.username, temp.userid, temp.cardid,
										  temp.start_date, temp.stop_date, temp.cardtype);
					}
					if(x5b_app_mode_X5CM())
					{
						if(strlen(temp.faceimg))//update or add card
						{
							ret |= x5b_user_add_face(temp.username, temp.userid, temp.faceimg, temp.faceid);
						}
					}
					if(strlen(temp.phone) && strlen(temp.room))//update or add card
					{
						ret |= voip_dbase_add_room_phone(0, 0, atoi(temp.room),
														 temp.phone, temp.username, temp.userid);
						//ret |= x5b_user_add_face(temp.username, temp.userid, temp.faceimg, temp.faceid);
					}
					voip_facecard_web_select_all();
					x5b_ubus_userauth_clear(NULL);
					return ret;
				}
				voip_facecard_web_select_all();
				x5b_ubus_userauth_clear(NULL);
				return ERROR;
			}
			voip_facecard_web_select_all();
			x5b_ubus_userauth_clear(NULL);
			return ERROR;
		}
		else if(strstr(buf, "del"))
		{
			char tmp[128];
			memset(tmp, 0, sizeof(tmp));
			memset(&temp, 0, sizeof(uci_userauth_t));
			ret |= os_uci_get_string("userauth.db.userid", tmp);
			if(ret != OK)
			{
				zlog_debug(ZLOG_APP, "userid ====== %s", tmp);
				return ERROR;
			}
			strncpy(temp.userid, tmp, MIN(sizeof(temp.userid), strlen(tmp)));
			memset(tmp, 0, sizeof(tmp));
			ret |= os_uci_get_string("userauth.db.cardid", tmp);
			if(ret != OK)
			{
				zlog_debug(ZLOG_APP, "cardid ====== %s", tmp);
				//return ERROR;
			}
			//strncpy(temp.cardid, tmp, MIN(APP_CARD_ID_MAX, strlen(tmp)));
			zlog_debug(ZLOG_APP, "===================");
			zlog_debug(ZLOG_APP, "user_id:%s", strlen(temp.userid)? temp.userid:" ");
			zlog_debug(ZLOG_APP, "card_id:%s", strlen(temp.cardid)? temp.cardid:" ");
			zlog_debug(ZLOG_APP, "===================");

			if(strlen(tmp))
				strncpy(temp.cardid, tmp, MIN(APP_CARD_ID_MAX, strlen(tmp)));

/*			if(app_time_job_id == 0)
				free(buf);*/
			card = x5b_user_lookup_by_username(NULL, temp.userid);
			if(card)
			{
				switch(x5b_app_open_mode())
				{
					case OPEN_FACE_AND_CARD:
					case OPEN_FACE_OR_CARD:
					case OPEN_FACE:
					case OPEN_CARD:
						{
							s_int8 cardid[16];
							uci_face_card_t decard;
							memset(&cardid, 0, sizeof(cardid));
							memset(&decard, 0, sizeof(decard));
							ret = x5b_user_get_card(NULL, temp.userid, cardid, 0);

							zlog_debug(ZLOG_APP, "=======ret=%d by userid:%s card=%s", ret, temp.userid, cardid);

							if(ret == OK && !str_isempty(cardid, 16))
							{
								decard.make_card = 1;
								//strcpy(decard.cardid, cardid);
								strncpy(decard.cardid, cardid, MIN(APP_CARD_ID_MAX, strlen(cardid)));
								zlog_debug(ZLOG_APP, "delete card to A");
								ret |= x5b_app_delete_card(x5b_app_mgt, cardid, E_CMD_TO_A);
							}
							if(ret == OK && x5b_app_mgt->X5CM)
							{
								ret = x5b_user_get_face(NULL, temp.userid, &decard.face_id, 0);
								zlog_debug(ZLOG_APP, "=======ret=%d by userid:%s faceid=%d", ret, temp.userid, decard.face_id);
								if(ret == OK/* && decard.face_id*/)
								{
									zlog_debug(ZLOG_APP, "delete face and card to C");
									if(strlen(card->username))
										strncpy(decard.username, card->username, MIN(sizeof(decard.username), strlen(card->username)));
										//strcpy(decard.username, card->username);
									if(strlen(temp.userid))
										strncpy(decard.user_id, temp.userid, MIN(sizeof(decard.user_id), strlen(temp.userid)));
										//strcpy(decard.user_id, temp.userid);
									decard.make_face = 1;
									//decard.face_id = 1;
									ret |= x5b_app_del_card_json(x5b_app_mgt, &decard, E_CMD_TO_C);
									if(ret != OK)
										zlog_debug(ZLOG_APP, "Delete user '%s' to C warning", decard.username);
								}
							}
						}
						break;
					default:
						zlog_debug(ZLOG_APP, "Unknow open mode %d", x5b_app_open_mode());
						ret = -1;
						break;
				}
				if(ret == 0)
				{
					ret |= x5b_user_del(NULL, temp.userid);
					ret |= voip_dbase_del_room_phone(0, 0, 0, NULL, NULL, temp.userid);
				}
			}
			else
			{
				zlog_debug(ZLOG_APP, " ==== can not lookup info by userid:%s", temp.userid);
			}
			voip_facecard_web_select_all();
			x5b_ubus_userauth_clear(NULL);
			return ERROR;
		}
		else if(strstr(buf, "refresh"))
		{
/*			if(app_time_job_id == 0)
				free(buf);*/
			zlog_debug(ZLOG_APP, "voip_facecard_web_select_all");
			voip_facecard_web_select_all();
			return OK;
		}

/*		if(app_time_job_id == 0)
			free(buf);*/
		return OK;
	}
/*	if(app_time_job_id == 0)
		free(buf);*/
	//x5b_ubus_face_card_enable(FALSE);
	voip_facecard_web_select_all();
	return OK;
}
#endif
