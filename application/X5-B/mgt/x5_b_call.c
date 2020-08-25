/*
 * x5_b_call.c
 *
 *  Created on: 2019年10月19日
 *      Author: zhurish
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

#ifdef PL_PJSIP_MODULE
#include "pjsip_app_api.h"
#endif



/* send open door CMD by '#' signal */
int x5b_app_open_door_api(x5b_app_mgt_t *app, int res, int to)
{
	int len = 0;
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
	x5b_app_hdr_make(mgt);
	len = os_tlv_set_zero(mgt->app->sbuf + mgt->app->offset,
			E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_BASE, E_CMD_OPEN), 0);
	mgt->app->offset += len;
	x5b_app_crc_make(mgt);
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "OPEN CMD MSG to %s:%d %d byte", inet_address(mgt->app->address),
				mgt->app->remote_port, mgt->app->slen);
	len = x5b_app_send_msg(mgt);
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return len;
}

/*
 * respone cardid(send card id to A module)
 */
int x5b_app_open_door_by_cardid_api(x5b_app_mgt_t *app, u_int8 *cardid, int clen, int to)
{
	int len = 0;
	open_cardid_respone respone;
	x5b_app_mgt_t *mgt = app;
	if(app == NULL)
		mgt = x5b_app_mgt;
	zassert(mgt != NULL);
	zassert(cardid != NULL);
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
		zlog_warn(ZLOG_APP, "CardID Respone MSG Can not send, Unknown Remote IP Address");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}

	x5b_app_hdr_make(mgt);
	memset(&respone, 0, sizeof(open_cardid_respone));
	if(cardid)
	{
		memcpy(respone.ID, cardid, clen);
		respone.clen = clen;
		len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_BASE, E_CMD_SECOM_OPEN_REQ),
				sizeof(open_cardid_respone), &respone);
	}
	mgt->app->offset += len;
	x5b_app_crc_make(mgt);
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "CardID Respone MSG to %s:%d %d byte", inet_address(mgt->app->address),
				mgt->app->remote_port, mgt->app->slen);
	len = x5b_app_send_msg(mgt);
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return len;
}


/*
 * Call Result CMD
 */
static char *x5b_app_call_result_event(int ret)
{
	switch(ret)
	{
	case E_CALL_RESULT_NO_SEARCH:
		return "No Search Room";
		break;
	case E_CALL_RESULT_UNREGISTER:
		return "Local ID Is Not Register";
		break;
	case E_CALL_RESULT_CALLING:
		return "Calling";
		break;
	case E_CALL_RESULT_TALKLING:
		return "Talking";
		break;
	case E_CALL_RESULT_STOP:
		return "Stop";
		break;
	case E_CALL_RESULT_FAIL:
		return "Fail";
		break;
	default:
		return "UNKNOW";
		break;
	}
	return "UNKNOW";
}



/* phone register status change report */
int x5b_app_register_status_api(x5b_app_mgt_t *app, int res, int to)
{
#ifdef PL_OPENWRT_UCI
	x5b_app_register_ack_t state;
	int len = 0;
	x5b_app_mgt_t *mgt = app;
	if(app == NULL)
		mgt = x5b_app_mgt;
	zassert(mgt != NULL);
	memset(&state, 0, sizeof(state));
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
		zlog_warn(ZLOG_APP, "Register Status MSG Can not send, Unknown Remote IP Address");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}

	x5b_app_hdr_make(mgt);

#ifdef PL_PJSIP_MODULE
	if(pl_pjsip_multiuser_get_api())
	{
		pl_pjsip_username_get_api(state.phone, NULL, FALSE);

		int rlen = (1 + strlen(state.phone));
		if(pl_pjsip->sip_user.sip_state == PJSIP_STATE_REGISTER_SUCCESS)
			state.reg_state = 1;
		else
			state.reg_state = 0;
		//state.reg_state = voip_sip_main_regstate() & 0xff;

		len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_STATUS, E_CMD_REG_STATUS), rlen, &state);
		mgt->app->offset += len;

		pl_pjsip_username_get_api(state.phone, NULL, TRUE);
		rlen = (1 + strlen(state.phone));
		if(pl_pjsip->sip_user_sec.sip_state == PJSIP_STATE_REGISTER_SUCCESS)
			state.reg_state = 1;
		else
			state.reg_state = 0;

		len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_STATUS, E_CMD_REG_STATUS), rlen, &state);
		mgt->app->offset += len;
	}
	else
	{
		pl_pjsip_username_get_api(state.phone, NULL, FALSE);

		int rlen = (1 + strlen(state.phone));
		if(pl_pjsip->sip_user.sip_state == PJSIP_STATE_REGISTER_SUCCESS)
			state.reg_state = 1;
		else
			state.reg_state = 0;

		len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_STATUS, E_CMD_REG_STATUS), rlen, &state);
		mgt->app->offset += len;
	}
#endif
	x5b_app_crc_make(mgt);
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "Register Status MSG to %s:%d %d byte", inet_address(mgt->app->address),
				mgt->app->remote_port, mgt->app->slen);
	len = x5b_app_send_msg(mgt);
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return len;
#else
	return OK;
#endif
}

/* sip info request ack */
int x5b_app_register_information_ack(x5b_app_mgt_t *mgt, int to)
{
	x5b_app_phone_register_ack_t reginfo;
	int len = 0;
	zassert(mgt != NULL);
	memset(&reginfo, 0, sizeof(x5b_app_phone_register_ack_t));
/*	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);*/
	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Remote is Not Register");
/*		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);*/
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Register Info MSG Can not send, Unknown Remote IP Address");
/*		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);*/
		return ERROR;
	}

	if(x5b_app_local_register_info_get(&reginfo) == OK)
	{
		x5b_app_hdr_make(mgt);
		len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_STATUS, E_CMD_ACK_REGISTER),
				sizeof(x5b_app_phone_register_ack_t), &reginfo);
		mgt->app->offset += len;
		x5b_app_crc_make(mgt);
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "Register Info MSG to %s:%d %d byte", inet_address(mgt->app->address),
					mgt->app->remote_port, mgt->app->slen);
		len = x5b_app_send_msg(mgt);
/*		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);*/
		return len;
	}
/*	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);*/
	return ERROR;
}

int x5b_app_call_result_api(x5b_app_mgt_t *app, int res, int inde, int to)
{
	u_int8 val[2];
	int len = 0;
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
		zlog_warn(ZLOG_APP, "CALL Result MSG Can not send, Unknown Remote IP Address");
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
	x5b_app_hdr_make(mgt);
	val[0] = (res & 0xff);
	val[1] = (inde & 0xff);
	len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
			E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_CALL, E_CMD_CALL_RESULT), E_CMD_CALL_RESULT_LEN, val);
	mgt->app->offset += len;
	x5b_app_crc_make(mgt);
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "CALL Result(%s) MSG to %s:%d %d byte", x5b_app_call_result_event(res),
				inet_address(mgt->app->address),
				mgt->app->remote_port, mgt->app->slen);
	len = x5b_app_send_msg(mgt);
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return len;
}

int x5b_app_call_internal_result_api(x5b_app_mgt_t *app, int res, int inde, int to)
{
	u_int8 val[2];
	int len = 0;
	x5b_app_mgt_t *mgt = app;
	if(app == NULL)
		mgt = x5b_app_mgt;
	zassert(mgt != NULL);

/*	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);*/
	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Remote is Not Register");
/*		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);*/
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "CALL Result MSG Can not send, Unknown Remote IP Address");
/*		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);*/
		return ERROR;
	}
	if(!mgt->regsync)
	{
/*		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);*/
		return ERROR;
	}
	x5b_app_hdr_make(mgt);
	val[0] = (res & 0xff);
	val[1] = (inde & 0xff);
	len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
			E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_CALL, E_CMD_CALL_RESULT), E_CMD_CALL_RESULT_LEN, val);
	mgt->app->offset += len;
	x5b_app_crc_make(mgt);
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "CALL Result(%s) MSG to %s:%d %d byte", x5b_app_call_result_event(res),
				inet_address(mgt->app->address),
				mgt->app->remote_port, mgt->app->slen);
	mgt->wait_timeout = 500;
	len = x5b_app_send_msg(mgt);
	mgt->wait_timeout = 0;

	//len = x5b_app_send_msg_without_ack(mgt);
/*	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);*/
	return len;
}



#ifdef X5B_APP_DATABASE
/* 房间号鉴权 应答 */
int x5b_app_authentication_ack_api(x5b_app_mgt_t *mgt, voip_dbase_t *dbtest, int res, int to)
{
	int len = 0;
	x5b_app_room_auth_ack_t ack;
	zassert(mgt != NULL);
	zassert(dbtest != NULL);
	//zassert(tlv->val.pval != NULL);
/*	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);*/
	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Remote is Not Register");
/*		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);*/
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Keepalive MSG Can not send, Unknown Remote IP Address");
/*		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);*/
		return ERROR;
	}
/*	if(!mgt->regsync)
	{
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}*/
	x5b_app_hdr_make(mgt);
	if(res == 0)
	{
		ack.building = dbtest->building;
		ack.unit = dbtest->unit;
		memcpy(ack.room_number, dbtest->room_number, sizeof(dbtest->room_number));
		ack.result = res;

		len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_BASE, E_CMD_ROOM_AUTH_ACK),
				sizeof(x5b_app_room_auth_t) + 1, &ack);
		mgt->app->offset += len;
	}
	else
	{
		ack.building = dbtest->building;
		ack.unit = dbtest->unit;
		//ack.room_number = htons(dbtest->room_number);
		memcpy(ack.room_number, dbtest->room_number, sizeof(dbtest->room_number));
		ack.result = res;

		len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_BASE, E_CMD_ROOM_AUTH_ACK),
				sizeof(x5b_app_room_auth_t) + 1, &ack);
		mgt->app->offset += len;
	}
	x5b_app_crc_make(mgt);
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "Auth ACK MSG to %s:%d %d byte", inet_address(mgt->app->address),
				mgt->app->remote_port, mgt->app->slen);
	len = x5b_app_send_msg(mgt);
/*	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);*/
	return len;
}

/* 接收到房间号鉴权请求 */
int x5b_app_read_authentication_tlv(x5b_app_mgt_t *mgt, os_tlv_t *tlv)
{
	voip_dbase_t *dbtest = NULL;
	zassert(mgt != NULL);
	zassert(tlv != NULL);
	zassert(tlv->val.pval != NULL);
	x5b_app_room_auth_t *room = tlv->val.pval;
	//room->room_number = atoi(room->room_number);

	dbtest = voip_dbase_lookup_by_room(room->building, room->unit, atoi(room->room_number));
	if(dbtest)
	{
		if(E_CMD_FROM_A(tlv->tag))
			x5b_app_authentication_ack_api(mgt, dbtest, 0, E_CMD_TO_A);
		else if(E_CMD_FROM_C(tlv->tag))
			x5b_app_authentication_ack_api(mgt, dbtest, 0, E_CMD_TO_C);
	}
	else
	{
		voip_dbase_t ack;
		ack.building = room->building;
		ack.unit = room->unit;
		//ack.room_number = htons(dbtest->room_number);
		memcpy(ack.room_number, room->room_number, sizeof(room->room_number));

		if(E_CMD_FROM_A(tlv->tag))
			x5b_app_authentication_ack_api(mgt, &ack, 1, E_CMD_TO_A);
		else if(E_CMD_FROM_C(tlv->tag))
			x5b_app_authentication_ack_api(mgt, &ack, 1, E_CMD_TO_C);
	}
	return OK;
}
#endif
