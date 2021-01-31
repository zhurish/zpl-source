/*
 * x5_b_global.c
 *
 *  Created on: 2019年7月16日
 *      Author: DELL
 */


#include "zebra.h"
#include "network.h"
#include "vty.h"
#include "if.h"
#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "str.h"
#include "table.h"
#include "vector.h"


#include "x5_b_global.h"
#include "x5_b_cmd.h"
#include "x5_b_app.h"
#include "x5_b_util.h"
#include "x5_b_ctl.h"
#include "x5_b_json.h"
#include "x5_b_web.h"
#include "x5b_dbase.h"

x5b_app_global_t *x5b_app_global = NULL;
x5b_app_open_t *x5b_app_open = NULL;
make_face_config_t *x5b_app_face = NULL;

static int x5b_app_global_mode_config_load(x5b_app_global_t *gl);
//static int x5b_app_open_config_load(x5b_app_open_t *fct);


int x5b_app_global_mode_load()
{
	if(x5b_app_global == NULL)
	{
		x5b_app_global = malloc(sizeof(x5b_app_global_t));
		zassert(x5b_app_global != NULL);
		memset(x5b_app_global, 0, sizeof(x5b_app_global_t));
	}
	if(x5b_app_open == NULL)
	{
		x5b_app_open = malloc(sizeof(x5b_app_open_t));
		zassert(x5b_app_open != NULL);
		memset(x5b_app_open, 0, sizeof(x5b_app_open_t));
	}
	if(x5b_app_face == NULL)
	{
		x5b_app_face = malloc(sizeof(make_face_config_t));
		zassert(x5b_app_face != NULL);
		memset(x5b_app_face, 0, sizeof(make_face_config_t));
	}
	x5b_app_global_mode_config_load(x5b_app_global);
	x5b_app_open_config_load(x5b_app_open);
	x5b_app_face_config_load(x5b_app_face);
	return OK;
}


static int x5b_app_global_mode_config_load(x5b_app_global_t *gl)
{
#ifdef PL_OPENWRT_UCI
	char tmp[128];
	int ret = ERROR;
	zassert(gl != NULL);
	memset(tmp, 0, sizeof(tmp));
	ret = os_uci_get_string("product.global.type", tmp);
	if(ret == OK && strlen(tmp))
	{
		if(strcasestr (tmp, "X5BM"))
			gl->X5CM = FALSE;
		else if(strcasestr (tmp, "X5CM"))
			gl->X5CM = TRUE;
		else
			gl->X5CM = FALSE;
	}
	//zlog_debug(MODULE_APP, "===========%s %s X5CM=%d", __func__, tmp, gl->X5CM);
	memset(tmp, 0, sizeof(tmp));
	ret = os_uci_get_string("openconfig.open.opentype", tmp);
	if(ret == OK && strlen(tmp))
	{
		if(strcasestr (tmp, "FaceAndCard") == 0)
			gl->opentype = OPEN_FACE_AND_CARD;
		else if(strcasestr (tmp, "FaceOrCard") == 0)
			gl->opentype = OPEN_FACE_OR_CARD;
		else if(strcasestr (tmp, "Card") == 0)
			gl->opentype = OPEN_CARD;
		else if(strcasestr (tmp, "Face") == 0)
			gl->opentype = OPEN_FACE;
		else
			gl->opentype = OPEN_NONE;

		if(x5b_app_open)
			x5b_app_open->opentype = gl->opentype;
	}
	gl->customizer = CUSTOMIZER_NONE;
	memset(tmp, 0, sizeof(tmp));
	ret = os_uci_get_string("product.global.customizer", tmp);
	if(ret == OK && strlen(tmp))
	{
		if(strcasestr (tmp, "Secom"))
			gl->customizer = CUSTOMIZER_SECOM;
		else if(strcasestr (tmp, "Huifu"))
			gl->customizer = CUSTOMIZER_HUIFU;
		else
			gl->customizer = CUSTOMIZER_NONE;
	}


	memset(tmp, 0, sizeof(tmp));
	ret = os_uci_get_string("product.global.housing_location", tmp);
	if(ret == OK && strlen(tmp))
	{
		if(strcasestr (tmp, "Unit"))
			gl->housing = HOUSING_UNIT;
		else if(strcasestr (tmp, "Wall"))
			gl->housing = HOUSING_WALL;
		else if(strcasestr (tmp, "Building"))
			gl->housing = HOUSING_PL_BUILDING;
	}

	memset(tmp, 0, sizeof(tmp));
	ret = os_uci_get_string("product.global.scene", tmp);
	if(ret == OK && strlen(tmp))
	{
		if(strcasestr (tmp, "Housing"))
			gl->install_scene = APP_SCENE_HOUSING;
		else if(strcasestr (tmp, "Bussiness"))
			gl->install_scene = APP_SCENE_BUSSINESS;
		else if(strcasestr (tmp, "Commercial"))
			gl->install_scene = APP_SCENE_COMMERCIAL;
	}

	memset(tmp, 0, sizeof(tmp));
	ret = os_uci_get_string("product.global.direction", tmp);
	if(ret == OK && strlen(tmp))
	{
		if(strcasestr (tmp, "out"))
			x5b_app_out_direction_set_api(TRUE);
		else
			x5b_app_out_direction_set_api(FALSE);
	}

	memset(tmp, 0, sizeof(tmp));
	ret = os_uci_get_string("product.global.device_name", tmp);
	if(ret == OK && strlen(tmp))
	{
		x5b_app_devicename_set_api(tmp);
	}

	memset(tmp, 0, sizeof(tmp));
	ret = os_uci_get_string("product.global.server_address", tmp);
	if(ret == OK && strlen(tmp))
	{
		x5b_app_docking_platform_address_set_api(tmp);
	}

	memset(tmp, 0, sizeof(tmp));
	ret = os_uci_get_string("product.global.location", tmp);
	if(ret == OK && strlen(tmp))
	{
		x5b_app_location_address_set_api(tmp);
	}

	ret = os_uci_get_integer("product.global.bluetooth", &gl->bluetooth);
	ret = os_uci_get_integer("product.global.nfc", &gl->nfc_enable);
	ret = os_uci_get_integer("product.global.doorcontact", &gl->doorcontact);
	ret = os_uci_get_integer("product.global.topf", &gl->topf);
#else
	gl->X5CM = TRUE;
	gl->opentype = OPEN_FACE_AND_CARD;
	gl->customizer = CUSTOMIZER_SECOM;
	gl->install_scene = APP_SCENE_BUSSINESS;
	x5b_app_out_direction_set_api(TRUE);
#endif
	return OK;
}

int x5b_app_global_mode_config_save(void)
{
#ifdef PL_OPENWRT_UCI
	int ret = ERROR;
	zassert(x5b_app_global != NULL);

	if(x5b_app_global->X5CM)
		ret = os_uci_set_string("product.global.type", "X5CM");
	else
		ret = os_uci_set_string("product.global.type", "X5BM");

	if(x5b_app_global->customizer == CUSTOMIZER_SECOM)
		ret = os_uci_set_string("product.global.customizer", "Secom");
	else if(x5b_app_global->customizer == CUSTOMIZER_HUIFU)
		ret = os_uci_set_string("product.global.customizer", "Huifu");
	else
		ret = os_uci_set_string("product.global.customizer", "None");

	if(x5b_app_global->housing == HOUSING_UNIT)
		ret = os_uci_set_string("product.global.housing_location", "Unit");
	else if(x5b_app_global->housing == HOUSING_WALL)
		ret = os_uci_set_string("product.global.housing_location", "Wall");
	else if(x5b_app_global->housing == HOUSING_PL_BUILDING)
		ret = os_uci_set_string("product.global.housing_location", "Building");

	if(x5b_app_global->install_scene == APP_SCENE_HOUSING)
		ret = os_uci_set_string("product.global.scene", "Housing");
	else if(x5b_app_global->install_scene == APP_SCENE_BUSSINESS)
		ret = os_uci_set_string("product.global.scene", "Bussiness");
	else if(x5b_app_global->install_scene == APP_SCENE_COMMERCIAL)
		ret = os_uci_set_string("product.global.scene", "Commercial");


	if(x5b_app_global->out_direction)
		ret = os_uci_set_string("product.global.direction", "out");
	else
		ret = os_uci_set_string("product.global.direction", "in");

	if(strlen(x5b_app_global->devicename))
		ret = os_uci_set_string("product.global.device_name", x5b_app_global->devicename);

	if(strlen(x5b_app_global->docking_platform_address))
		ret = os_uci_set_string("product.global.server_address", x5b_app_global->docking_platform_address);

	if(strlen(x5b_app_global->location_address))
		ret = os_uci_set_string("product.global.location", x5b_app_global->location_address);

	ret = os_uci_set_integer("product.global.bluetooth", x5b_app_global->bluetooth);
	ret = os_uci_set_integer("product.global.nfc", x5b_app_global->nfc_enable);
	ret = os_uci_set_integer("product.global.doorcontact", x5b_app_global->doorcontact);
	ret = os_uci_set_integer("product.global.topf", x5b_app_global->topf);
	os_uci_save_config("product");
#endif
	return OK;
}


int x5b_app_open_config_load(x5b_app_open_t *fct)
{
#ifdef PL_OPENWRT_UCI
	char tmp[128];
	int ret = ERROR, val = 0;
	zassert(fct != NULL);
	memset(tmp, 0, sizeof(tmp));
	memset(tmp, 0, sizeof(tmp));
	ret = os_uci_get_string("openconfig.open.opentype", tmp);
	if(ret == OK && strlen(tmp))
	{
		if(strcasestr(tmp, "FaceAndCard") == 0)
			fct->opentype = OPEN_FACE_AND_CARD;
		else if(strcasestr(tmp, "FaceOrCard") == 0)
			fct->opentype = OPEN_FACE_OR_CARD;
		else if(strcasestr(tmp, "Card") == 0)
			fct->opentype = OPEN_CARD;
		else if(strcasestr(tmp, "Face") == 0)
			fct->opentype = OPEN_FACE;
		else
			fct->opentype = OPEN_NONE;
	}
	memset(tmp, 0, sizeof(tmp));
	ret = os_uci_get_string("openconfig.open.relay_active_level", tmp);
	if(ret == OK && strlen(tmp))
	{
		if(strcasestr(tmp, "HIGH"))
			fct->outrelay = TRUE;
		else if(strcasestr(tmp, "LOW"))
			fct->outrelay = FALSE;
		else
			fct->outrelay = FALSE;
	}

	memset(tmp, 0, sizeof(tmp));
	ret = os_uci_get_string("openconfig.open.wiggins", tmp);
	if(ret == OK && strlen(tmp))
	{
		if(strcasestr(tmp, "26-Bit"))
			fct->wiggins = 26;
		else if(strcasestr(tmp, "34-Bit"))
			fct->wiggins = 34;
		else if(strcasestr(tmp, "66-Bit"))
			fct->wiggins = 66;
	}

	ret = os_uci_get_integer("openconfig.open.opentime", &fct->waitopen);
	//ret = os_uci_get_integer("openconfig.open.waitopen", &fct->opentime);
	ret = os_uci_get_integer("openconfig.open.openwaittime", &fct->openhold);

	ret = os_uci_get_integer("openconfig.open.openholdtime", &fct->waitclose);

	ret = os_uci_get_integer("openconfig.open.tamperalarm", &val);
	fct->tamperalarm = (val==0)? FALSE:TRUE;
	ret = os_uci_get_integer("openconfig.open.advanced", &val);
	if(val)
	{
		ret = os_uci_get_integer("openconfig.open.keep_open_alarm", &val);
		fct->openalarm = (val==0)? FALSE:TRUE;
	}
#else
#endif
	return OK;
}

int x5b_app_open_config_save(void)
{
#ifdef PL_OPENWRT_UCI
	int ret = ERROR;
	zassert(x5b_app_open != NULL);

	ret = os_uci_set_integer("openconfig.open.opentime", x5b_app_open->waitopen);
	ret = os_uci_set_integer("openconfig.open.openwaittime", x5b_app_open->openhold);
	ret = os_uci_set_integer("openconfig.open.keep_open_alarm", x5b_app_open->openalarm);
	ret = os_uci_set_integer("openconfig.open.openholdtime", x5b_app_open->waitclose);
	ret = os_uci_set_integer("openconfig.open.tamperalarm", x5b_app_open->tamperalarm);

	if(x5b_app_open->openalarm)
		ret = os_uci_set_integer("openconfig.open.advanced", 1);

	if(x5b_app_open->opentype == OPEN_FACE_AND_CARD)
		ret = os_uci_set_string("openconfig.open.opentype", "FaceAndCard");
	else if(x5b_app_open->opentype == OPEN_FACE_OR_CARD)
		ret = os_uci_set_string("openconfig.open.opentype", "FaceOrCard");
	else if(x5b_app_open->opentype == OPEN_CARD)
		ret = os_uci_set_string("openconfig.open.opentype", "Card");
	else if(x5b_app_open->opentype == OPEN_FACE)
		ret = os_uci_set_string("openconfig.open.opentype", "Face");


	if(x5b_app_open->outrelay)
		ret = os_uci_set_string("openconfig.open.relay_active_level", "HIGH");
	else
		ret = os_uci_set_string("openconfig.open.relay_active_level", "LOW");

	//ret = os_uci_set_integer("openconfig.open.opentime", factory->opentime);

	if(x5b_app_open->wiggins == 26)
		ret = os_uci_set_string("openconfig.open.wiggins", "26-Bit");
	else if(x5b_app_open->wiggins == 34)
		ret = os_uci_set_string("openconfig.open.wiggins", "34-Bit");
	else if(x5b_app_open->wiggins == 66)
		ret = os_uci_set_string("openconfig.open.wiggins", "66-Bit");

	os_uci_save_config("openconfig");
#endif
	return OK;
}

int x5b_app_face_config_load(make_face_config_t *fct)
{
#ifdef PL_OPENWRT_UCI
	make_face_config_t info;
	int	 ret = 0;
	memset(&info, 0, sizeof(info));

	ret |= os_uci_get_integer("openconfig.faceopt.face_yaw_left", &info.faceYawLeft);
	if(ret != OK)
	{
		zlog_debug(MODULE_APP, "faceYawLeft ====== %d", info.faceYawLeft);
		return ERROR;
	}

	ret |= os_uci_get_integer("openconfig.faceopt.face_yaw_right", &info.faceYawRight);
	if(ret != OK)
	{
		zlog_debug(MODULE_APP, "faceYawRight ====== %d", info.faceYawRight);
		return ERROR;
	}

	ret |= os_uci_get_integer("openconfig.faceopt.face_pitch_up", &info.facePitchUp);
	if(ret != OK)
	{
		zlog_debug(MODULE_APP, "facePitchUp ====== %d", info.facePitchUp);
		return ERROR;
	}

	ret |= os_uci_get_integer("openconfig.faceopt.face_pitch_down", &info.facePitchDown);
	if(ret != OK)
	{
		zlog_debug(MODULE_APP, "facePitchDown ====== %d", info.facePitchDown);
		return ERROR;
	}

	ret |= os_uci_get_integer("openconfig.faceopt.face_record_width", &info.faceRecordWidth);
	if(ret != OK)
	{
		zlog_debug(MODULE_APP, "faceRecordWidth ====== %d", info.faceRecordWidth);
		return ERROR;
	}

	ret |= os_uci_get_integer("openconfig.faceopt.face_record_height", &info.faceRecordHeight);
	if(ret != OK)
	{
		zlog_debug(MODULE_APP, "faceRecordHeight ====== %d", info.faceRecordHeight);
		return ERROR;
	}

	ret |= os_uci_get_integer("openconfig.faceopt.face_recognize_width", &info.faceRecognizeWidth);
	if(ret != OK)
	{
		zlog_debug(MODULE_APP, "faceRecognizeWidth ====== %d", info.faceRecognizeWidth);
		return ERROR;
	}

	ret |= os_uci_get_integer("openconfig.faceopt.face_recognize_height", &info.faceRecognizeHeight);
	if(ret != OK)
	{
		zlog_debug(MODULE_APP, "faceRecognizeHeight ====== %d", info.faceRecognizeHeight);
		return ERROR;
	}

	ret |= os_uci_get_double("openconfig.faceopt.record_threshold", "%.3f", &info.similarRecord);
	if(ret != OK)
	{
		zlog_debug(MODULE_APP, "similarRecord ====== %.3f", info.similarRecord);
		return ERROR;
	}

	ret |= os_uci_get_double("openconfig.faceopt.recognize_threshold", "%.3f", &info.similarRecognize);
	if(ret != OK)
	{
		zlog_debug(MODULE_APP, "similarRecognize ====== %.3f", info.similarRecognize);
		return ERROR;
	}
	ret |= os_uci_get_double("openconfig.faceopt.recognize_sec_threshold", "%.3f", &info.similarSecRecognize);
	if(ret != OK)
	{
		zlog_debug(MODULE_APP, "similarSecRecognize ====== %.3f", info.similarSecRecognize);
		return ERROR;
	}


	ret |= os_uci_get_double("openconfig.faceopt.living_threshold", "%.3f", &info.similarLiving);
	if(ret != OK)
	{
		zlog_debug(MODULE_APP, "similarLiving ====== %.3f", info.similarLiving);
		return ERROR;
	}

	ret |= os_uci_get_integer("openconfig.faceopt.living_detection", &info.livenessSwitch);
	if(ret != OK)
	{
		zlog_debug(MODULE_APP, "livenessSwitch ====== %d", info.livenessSwitch);
		return ERROR;
	}

	ret |= os_uci_get_integer("openconfig.faceopt.face_roi_width", &info.faceRoiWidth);
	if(ret != OK)
	{
		zlog_debug(MODULE_APP, "faceRoiWidth ====== %d", info.faceRoiWidth);
		return ERROR;
	}

	ret |= os_uci_get_integer("openconfig.faceopt.face_roi_height", &info.faceRoiHeight);
	if(ret != OK)
	{
		zlog_debug(MODULE_APP, "faceRoiHeight ====== %d", info.faceRoiHeight);
		return ERROR;
	}


	ret |= os_uci_get_integer("openconfig.faceopt.success_intervals", &info.faceOKContinuousTime);
	if(ret != OK)
	{
		zlog_debug(MODULE_APP, "successful_intervals ====== %d", info.faceOKContinuousTime);
		return ERROR;
	}
	ret |= os_uci_get_integer("openconfig.faceopt.failure_intervals", &info.faceERRContinuousTime);
	if(ret != OK)
	{
		zlog_debug(MODULE_APP, "failure_intervals ====== %d", info.faceERRContinuousTime);
		return ERROR;
	}
	if(fct)
		memcpy(fct, &info, sizeof(info));
#endif
	return OK;
}


int x5b_app_face_config_save(void)
{
#ifdef PL_OPENWRT_UCI
	int ret = 0;
	ret |= os_uci_set_integer("openconfig.faceopt.face_yaw_left", x5b_app_face->faceYawLeft);
	ret |= os_uci_set_integer("openconfig.faceopt.face_yaw_right", x5b_app_face->faceYawRight);
	ret |= os_uci_set_integer("openconfig.faceopt.face_pitch_up", x5b_app_face->facePitchUp);
	ret |= os_uci_set_integer("openconfig.faceopt.face_pitch_down", x5b_app_face->facePitchDown);
	ret |= os_uci_set_integer("openconfig.faceopt.face_record_width", x5b_app_face->faceRecordWidth);
	ret |= os_uci_set_integer("openconfig.faceopt.face_record_height", x5b_app_face->faceRecordHeight);
	ret |= os_uci_set_integer("openconfig.faceopt.face_recognize_width", x5b_app_face->faceRecognizeWidth);
	ret |= os_uci_set_integer("openconfig.faceopt.face_recognize_height", x5b_app_face->faceRecognizeHeight);
	ret |= os_uci_set_double("openconfig.faceopt.record_threshold", "%.3f", x5b_app_face->similarRecord);
	ret |= os_uci_set_double("openconfig.faceopt.recognize_threshold", "%.3f", x5b_app_face->similarRecognize);
	ret |= os_uci_set_double("openconfig.faceopt.recognize_sec_threshold", "%.3f", x5b_app_face->similarSecRecognize);
	//ret |= os_uci_get_double("openconfig.faceopt.recognize_sec_threshold", "%.3f", x5b_app_face->similarSecRecognize);
	ret |= os_uci_set_double("openconfig.faceopt.living_threshold", "%.3f", x5b_app_face->similarLiving);
	ret |= os_uci_set_integer("openconfig.faceopt.living_detection", x5b_app_face->livenessSwitch);
	ret |= os_uci_set_integer("openconfig.faceopt.face_roi_width", x5b_app_face->faceRoiWidth);
	ret |= os_uci_set_integer("openconfig.faceopt.face_roi_height", x5b_app_face->faceRoiHeight);
	ret |= os_uci_set_integer("openconfig.faceopt.success_intervals", x5b_app_face->faceOKContinuousTime);
	ret |= os_uci_set_integer("openconfig.faceopt.failure_intervals", x5b_app_face->faceERRContinuousTime);
	os_uci_save_config("openconfig");
#endif
	return OK;
}












int x5b_app_global_config_load(void)
{
	if(x5b_app_global)
		return x5b_app_global_mode_config_load(x5b_app_global);
	return ERROR;
}

int x5b_app_global_device_config(void *app, int to)
{
	int len = 0;
	global_config_t gl_config;
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
			zlog_warn(MODULE_APP, "Remote is Not Register");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_warn(MODULE_APP, "OPEN CMD MSG Can not send, Unknown Remote IP Address");
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
	memset(&gl_config, 0, sizeof(gl_config));

	gl_config.x5cm = x5b_app_global->X5CM;//：（1 byte）
	gl_config.blu = x5b_app_global->bluetooth;//：（1 byte）
	gl_config.nfc = x5b_app_global->nfc_enable;//：（1 byte）
	gl_config.opentype = x5b_app_global->opentype;//：（1 byte）
	gl_config.custom = x5b_app_global->customizer;//：（1 byte）
	gl_config.scene = x5b_app_global->install_scene;//：（1 byte）
	gl_config.housing = x5b_app_global->housing;//：（1 byte）
	memcpy(gl_config.devname, x5b_app_global->devicename, X5B_APP_DEVICE_NAME_MAX);//：（64 byte）
	memcpy(gl_config.location, x5b_app_global->location_address, X5B_APP_DEVICE_NAME_MAX);//：（64 byte）
	gl_config.direction = x5b_app_global->out_direction;//：（1 byte）
	memcpy(gl_config.address1, x5b_app_global->docking_platform_address, X5B_APP_DEVICE_IP_MAX);//：（32 byte）
	memcpy(gl_config.address2, x5b_app_global->docking_platform_address1, X5B_APP_DEVICE_IP_MAX);//：（32byte）
	memcpy(gl_config.address3, x5b_app_global->docking_platform_address2, X5B_APP_DEVICE_IP_MAX);//：（32byte
	//if(strlen(output))
	{
		x5b_app_hdr_make(mgt);
		len = os_tlv_set_string(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_SET, E_CMD_SYSTEM_CONFIG), sizeof(gl_config), &gl_config);
		mgt->app->offset += len;
		x5b_app_crc_make(mgt);
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(MODULE_APP, "DEVICE CMD MSG to %s:%d %d byte", inet_address(mgt->app->address),
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

int x5b_app_global_mode_free()
{
	if(x5b_app_global != NULL)
	{
		memset(x5b_app_global, 0, sizeof(x5b_app_global_t));
		free(x5b_app_global);
	}
	x5b_app_global = NULL;
	return OK;
}

BOOL x5b_app_mode_X5CM()
{
	if(!x5b_app_global)
		return FALSE;
	return x5b_app_global->X5CM;
}

int x5b_app_open_mode()
{
	if(!x5b_app_global)
		return OPEN_NONE;
	return x5b_app_global->opentype;
}

int x5b_app_customizer()
{
	if(!x5b_app_global)
		return CUSTOMIZER_NONE;
	return x5b_app_global->customizer;
}

int x5b_app_mode_set_api(BOOL X5CM)
{
	if(x5b_app_global)
		x5b_app_global->X5CM = X5CM;
	return OK;
}

int x5b_app_open_mode_set_api(x5b_app_opentype_t opentype)
{
	if(x5b_app_global)
		x5b_app_global->opentype = opentype;
	return OK;
}

int x5b_app_customizer_set_api(x5b_app_customizer_t customizer)
{
	if(x5b_app_global)
		x5b_app_global->customizer = customizer;
	return OK;
}


int x5b_app_bluetooth_set_api(BOOL bluetooth)
{
	if(x5b_app_global)
		x5b_app_global->bluetooth = bluetooth;
	return OK;
}

int x5b_app_bluetooth_get_api()
{
	if(!x5b_app_global)
		return FALSE;
	return x5b_app_global->bluetooth;
}

int x5b_app_nfc_set_api(BOOL nfc_enable)
{
	if(x5b_app_global)
		x5b_app_global->nfc_enable = nfc_enable;
	return OK;
}

int x5b_app_nfc_get_api()
{
	if(!x5b_app_global)
		return FALSE;
	return x5b_app_global->nfc_enable;
}

/*********************************************************************/
int x5b_app_housing_set_api(x5b_app_housing_t housing)
{
	if(x5b_app_global)
		x5b_app_global->housing = housing;
	return OK;
}

x5b_app_housing_t x5b_app_housing_get_api(void)
{
	if(!x5b_app_global)
		return HOUSING_UNIT;
	return x5b_app_global->housing;
}

int x5b_app_scene_set_api(x5b_app_scene_t install_scene)
{
	if(x5b_app_global)
		x5b_app_global->install_scene = install_scene;
	return OK;
}

x5b_app_scene_t x5b_app_scene_get_api(void)
{
	if(!x5b_app_global)
		return APP_SCENE_HOUSING;
	return x5b_app_global->install_scene;
}

int x5b_app_devicename_set_api(char * devicename)
{
	if(x5b_app_global)
	{
		memset(x5b_app_global->devicename, 0, sizeof(x5b_app_global->devicename));
		if(devicename)
			strcpy(x5b_app_global->devicename, devicename);
	}
	return OK;
}

char * x5b_app_devicename_get_api(void)
{
	if(!x5b_app_global)
		return NULL;
	return x5b_app_global->devicename;
}

int x5b_app_location_address_set_api(char * location_address)
{
	if(x5b_app_global)
	{
		memset(x5b_app_global->location_address, 0, sizeof(x5b_app_global->location_address));
		if(location_address)
			strcpy(x5b_app_global->location_address, location_address);
	}
	return OK;
}

char * x5b_app_location_address_get_api(void)
{
	if(!x5b_app_global)
		return NULL;
	return x5b_app_global->location_address;
}

int x5b_app_out_direction_set_api(BOOL out_direction)
{
	if(x5b_app_global)
		x5b_app_global->out_direction = out_direction;
	return OK;
}

BOOL x5b_app_out_direction_get_api(void)
{
	if(!x5b_app_global)
		return FALSE;
	return x5b_app_global->out_direction;
}


int x5b_app_docking_platform_address_set_api(char * address)
{
	if(x5b_app_global)
	{
		memset(x5b_app_global->docking_platform_address, 0, sizeof(x5b_app_global->docking_platform_address));
		if(address)
			strcpy(x5b_app_global->docking_platform_address, address);
	}
	return OK;
}

char * x5b_app_docking_platform_address_get_api(void)
{
	if(!x5b_app_global)
		return NULL;
	return x5b_app_global->docking_platform_address;
}

int x5b_app_docking_platform_address1_set_api(char * address)
{
	if(x5b_app_global)
	{
		memset(x5b_app_global->docking_platform_address1, 0, sizeof(x5b_app_global->docking_platform_address1));
		if(address)
			strcpy(x5b_app_global->docking_platform_address1, address);
	}
	return OK;
}

char * x5b_app_docking_platform_address1_get_api(void)
{
	if(!x5b_app_global)
		return NULL;
	return x5b_app_global->docking_platform_address1;
}

int x5b_app_docking_platform_address2_set_api(char * address)
{
	if(x5b_app_global)
	{
		memset(x5b_app_global->docking_platform_address2, 0, sizeof(x5b_app_global->docking_platform_address2));
		if(address)
			strcpy(x5b_app_global->docking_platform_address2, address);
	}
	return OK;
}

char * x5b_app_docking_platform_address2_get_api(void)
{
	if(!x5b_app_global)
		return NULL;
	return x5b_app_global->docking_platform_address2;
}

/*******************************************************************************/
static int _x5b_app_global_show(struct vty *vty)
{
	if(x5b_app_global)
	{
		vty_out(vty, "x5b global config        : %s %s", x5b_app_mode_X5CM() ? "X5CM":"X5BM", VTY_NEWLINE);
		vty_out(vty, " Bluetooth               : %s %s", x5b_app_global->bluetooth ? "enabled":"disabled", VTY_NEWLINE);
		vty_out(vty, " NFC                     : %s %s", x5b_app_global->nfc_enable ? "enabled":"disabled", VTY_NEWLINE);
		vty_out(vty, " Doorcontact             : %s %s", x5b_app_global->doorcontact ? "enabled":"disabled", VTY_NEWLINE);
		vty_out(vty, " Direction               : %s %s", x5b_app_global->out_direction ? "Out":"In", VTY_NEWLINE);

		if(x5b_app_global->opentype == OPEN_NONE)
			vty_out(vty, " Open Type               : %s %s", "NONE", VTY_NEWLINE);
		else if(x5b_app_global->opentype == OPEN_CARD)
			vty_out(vty, " Open Type               : %s %s", "CARD", VTY_NEWLINE);
		else if(x5b_app_global->opentype == OPEN_FACE)
			vty_out(vty, " Open Type               : %s %s", "FACE", VTY_NEWLINE);
		else if(x5b_app_global->opentype == OPEN_FACE_AND_CARD)
			vty_out(vty, " Open Type               : %s %s", "FACE AND CARD", VTY_NEWLINE);
		else if(x5b_app_global->opentype == OPEN_FACE_OR_CARD)
			vty_out(vty, " Open Type               : %s %s", "FACE OR CARD", VTY_NEWLINE);

		if(x5b_app_global->customizer == CUSTOMIZER_NONE)
			vty_out(vty, " Customizer              : %s %s", "NONE", VTY_NEWLINE);
		else if(x5b_app_global->customizer == CUSTOMIZER_SECOM)
			vty_out(vty, " Customizer              : %s %s", "SECOM", VTY_NEWLINE);
		else if(x5b_app_global->customizer == CUSTOMIZER_HUIFU)
			vty_out(vty, " Customizer              : %s %s", "HUIFU", VTY_NEWLINE);

		if(x5b_app_global->install_scene == APP_SCENE_HOUSING)
			vty_out(vty, " Install Scene           : %s %s", "HOUSING", VTY_NEWLINE);
		else if(x5b_app_global->install_scene == APP_SCENE_BUSSINESS)
			vty_out(vty, " Install Scene           : %s %s", "BUSSINESS", VTY_NEWLINE);
		else if(x5b_app_global->install_scene == APP_SCENE_COMMERCIAL)
			vty_out(vty, " Install Scene           : %s %s", "COMMERCIAL", VTY_NEWLINE);

		if(x5b_app_global->housing == HOUSING_NONE)
			vty_out(vty, " Housing Scene           : %s %s", "NONE", VTY_NEWLINE);
		else if(x5b_app_global->housing == HOUSING_UNIT)
			vty_out(vty, " Housing Scene           : %s %s", "UNIT", VTY_NEWLINE);
		else if(x5b_app_global->housing == HOUSING_WALL)
			vty_out(vty, " Housing Scene           : %s %s", "WALL", VTY_NEWLINE);
		else if(x5b_app_global->housing == HOUSING_PL_BUILDING)
			vty_out(vty, " Housing Scene           : %s %s", "PL_BUILDING", VTY_NEWLINE);

		if(strlen(x5b_app_global->devicename))
			vty_out(vty, " Devicename              : %s %s", x5b_app_global->devicename, VTY_NEWLINE);
		else
			vty_out(vty, " Devicename              : %s %s", " ", VTY_NEWLINE);

		if(strlen(x5b_app_global->location_address))
			vty_out(vty, " Location                : %s %s", x5b_app_global->location_address, VTY_NEWLINE);
		else
			vty_out(vty, " Location                : %s %s", " ", VTY_NEWLINE);


		if(strlen(x5b_app_global->docking_platform_address))
			vty_out(vty, " Server                  : %s %s", x5b_app_global->docking_platform_address, VTY_NEWLINE);
		else
			vty_out(vty, " Server                  : %s %s", " ", VTY_NEWLINE);


		if(strlen(x5b_app_global->docking_platform_address1))
			vty_out(vty, " Server1                 : %s %s", x5b_app_global->docking_platform_address1, VTY_NEWLINE);
		else
			vty_out(vty, " Server1                 : %s %s", " ", VTY_NEWLINE);


		if(strlen(x5b_app_global->docking_platform_address2))
			vty_out(vty, " Server2                  : %s %s", x5b_app_global->docking_platform_address2, VTY_NEWLINE);
		else
			vty_out(vty, " Server2                 : %s %s", " ", VTY_NEWLINE);
	}
	return OK;
}

static int _x5b_app_open_show(struct vty *vty)
{
	if(x5b_app_open)
	{
		vty_out(vty, "x5b open config          : %s %s", x5b_app_mode_X5CM() ? "X5CM":"X5BM", VTY_NEWLINE);
		vty_out(vty, " Openalarm               : %s %s", x5b_app_open->openalarm ? "enabled":"disabled", VTY_NEWLINE);
		vty_out(vty, " Outrelay                : %s %s", x5b_app_open->outrelay ? "high":"low", VTY_NEWLINE);
		vty_out(vty, " Tamperalarm             : %s %s", x5b_app_open->tamperalarm ? "enabled":"disabled", VTY_NEWLINE);
		vty_out(vty, " Wiggins                 : %d-bit %s", x5b_app_open->wiggins, VTY_NEWLINE);

		vty_out(vty, " Waitopen                : %d Ms %s", x5b_app_open->waitopen, VTY_NEWLINE);
		vty_out(vty, " Openhold                : %d Ms %s", x5b_app_open->openhold, VTY_NEWLINE);
		vty_out(vty, " Waitclose               : %d Ms %s", x5b_app_open->waitclose, VTY_NEWLINE);
	}
	return OK;
}

static int _x5b_app_face_show(struct vty *vty)
{
	if(x5b_app_face)
	{
		vty_out(vty, "x5b open config          : %s %s", x5b_app_mode_X5CM() ? "X5CM":"X5BM", VTY_NEWLINE);
		vty_out(vty, " livenessSwitch          : %s %s", x5b_app_face->livenessSwitch ? "enabled":"disabled", VTY_NEWLINE);

		vty_out(vty, " faceYawLeft             : %d%s", x5b_app_face->faceYawLeft, VTY_NEWLINE);
		vty_out(vty, " faceYawRight            : %d%s", x5b_app_face->faceYawRight, VTY_NEWLINE);
		vty_out(vty, " facePitchUp             : %d%s", x5b_app_face->facePitchUp, VTY_NEWLINE);
		vty_out(vty, " facePitchDown           : %d%s", x5b_app_face->facePitchDown, VTY_NEWLINE);

		vty_out(vty, " faceRoiWidth            : %d%s", x5b_app_face->faceRoiWidth, VTY_NEWLINE);
		vty_out(vty, " faceRoiHeight           : %d%s", x5b_app_face->faceRoiHeight, VTY_NEWLINE);

		vty_out(vty, " faceRecordWidth         : %d%s", x5b_app_face->faceRecordWidth, VTY_NEWLINE);
		vty_out(vty, " faceRecordHeight        : %d%s", x5b_app_face->faceRecordHeight, VTY_NEWLINE);

		vty_out(vty, " faceRecognizeWidth      : %d%s", x5b_app_face->faceRecognizeWidth, VTY_NEWLINE);
		vty_out(vty, " faceRecognizeHeight     : %d%s", x5b_app_face->faceRecognizeHeight, VTY_NEWLINE);


		vty_out(vty, " similarRecord           : %.3f%s", x5b_app_face->similarRecord, VTY_NEWLINE);
		vty_out(vty, " similarRecognize        : %.3f%s", x5b_app_face->similarRecognize, VTY_NEWLINE);
		vty_out(vty, " similarLiving           : %.3f%s", x5b_app_face->similarLiving, VTY_NEWLINE);

		vty_out(vty, " faceOKContinuousTime    : %d%s", x5b_app_face->faceOKContinuousTime, VTY_NEWLINE);
		vty_out(vty, " faceERRContinuousTime   : %d%s", x5b_app_face->faceERRContinuousTime, VTY_NEWLINE);
	}
	return OK;
}

int x5b_app_show_param(struct vty *vty, int type)
{
	if(type == 0 && x5b_app_global)
	{
		 _x5b_app_global_show(vty);
	}
	else if(type == 1)
	{
		_x5b_app_open_show(vty);
	}
	else if(type == 2)
	{
		_x5b_app_face_show(vty);
	}
	return OK;
}
