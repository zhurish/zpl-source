/*
 * x5_b_io.c
 *
 *  Created on: 2019年10月19日
 *      Author: zhurish
 */
#include "zebra.h"
#include "vty.h"
#include "if.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "str.h"
#include "table.h"
#include "vector.h"

#include "x5_b_global.h"
#include "x5_b_cmd.h"
#include "x5_b_app.h"
#include "x5_b_json.h"
#include "x5_b_ctl.h"
#include "x5b_dbase.h"
#include "x5_b_util.h"
#include "x5_b_web.h"



/* A 模块开门参数设置 */
int x5b_app_open_option(x5b_app_mgt_t *app, void *info, int to)
{
	int len = 0;
	//ConfiglockType *card = (ConfiglockType *)info;
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
		return ERROR;
	}
	if(info)
	{
		x5b_app_hdr_make(mgt);
		if(to == E_CMD_TO_A)
			len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_SET, E_CMD_OPEN_OPTION), sizeof(ConfiglockType), info);
		else if(to == E_CMD_TO_C)
		{
			//u_int8 open_type =(*(u_int8)info);//x5b_app_open_mode();
			len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_SET, E_CMD_OPEN_OPTION), sizeof(u_int8), info);
		}
		mgt->app->offset += len;
		x5b_app_crc_make(mgt);
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "OPEN OPTION MSG to %s:%d %d byte", inet_address(mgt->app->address),
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


int x5b_app_wiggins_setting(x5b_app_mgt_t *app, int wiggins, int to)
{
	int len = 0;
	u_int8 val = wiggins & 0xff;
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
		zlog_warn(ZLOG_APP, "wiggins MSG Can not send, Unknown Remote IP Address");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}

	x5b_app_hdr_make(mgt);
	len = os_tlv_set_integer(mgt->app->sbuf + mgt->app->offset,
			E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_SET, E_CMD_WIGGINS), 1, &val);
	mgt->app->offset += len;
	x5b_app_crc_make(mgt);
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "Wiggins MSG to %s:%d %d byte", inet_address(mgt->app->address),
				mgt->app->remote_port, mgt->app->slen);
	len = x5b_app_send_msg(mgt);
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return len;
}



#ifdef X5B_APP_DATABASE
/*
 *A 模块制卡  make card
 */
int x5b_app_add_card(x5b_app_mgt_t *app, void *info, int to)
{
	int len = 0;
	permiListType *card  = (permiListType *)info;
	x5b_app_mgt_t *mgt = app;
	//make_face_card_t *inputcard = (make_face_card_t *)info;
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
/*
	memset(&card, 0, sizeof(card));

	if(strlen(inputcard->cardid))
	{
		card_id_string_to_hex(inputcard->cardid, strlen(inputcard->cardid), card.ID);
	}
	card.start_time = htonl(inputcard->start_date);
	card.stop_time = htonl(inputcard->stop_date);

	if(strlen(inputcard->cardtype))
	{
		if(strstr(inputcard->cardtype,"Blacklist"))
			card.status = 1;
		else if(strstr(inputcard->cardtype,"Whitelist"))
			card.status = 2;
	}
	zlog_debug(ZLOG_APP, "===================%s -> type=%d", __func__, card.status);
*/

	x5b_app_hdr_make(mgt);
	if(to == E_CMD_TO_A)
		len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_SET, E_CMD_MAKE_CARD), sizeof(permiListType), card);

	mgt->app->offset += len;
	x5b_app_crc_make(mgt);
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "MAKE Card CMD MSG to %s:%d %d byte", inet_address(mgt->app->address),
				mgt->app->remote_port, mgt->app->slen);
	len = x5b_app_send_msg(mgt);
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return len;
}

int x5b_app_delete_card(x5b_app_mgt_t *app, void *info, int to)
{
	int len = 0;
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
	x5b_app_hdr_make(mgt);

	if(to == E_CMD_TO_A)
	{
		s_int8     cardid[APP_CARD_ID_MAX + 1];
		u_int8     ID[8];
		memset(cardid, 0, sizeof(cardid));
		memcpy(cardid, info, APP_CARD_ID_MAX);
		if(strlen(cardid) <= 16)
		{
			card_id_string_to_hex(cardid, strlen(cardid), ID);
		}
		len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_SET, E_CMD_DELETE_CARD), sizeof(ID), ID);
	}

	mgt->app->offset += len;
	x5b_app_crc_make(mgt);
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "Delete Card CMD MSG to %s:%d %d byte", inet_address(mgt->app->address),
				mgt->app->remote_port, mgt->app->slen);
	len = x5b_app_send_msg(mgt);
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return len;
}
#endif
