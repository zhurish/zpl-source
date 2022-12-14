/*
 * voip_app.c
 *
 *  Created on: Jan 1, 2019
 *      Author: zhurish
 */

#include "auto_include.h"
#include <zplos_include.h>
#include "lib_include.h"
#include "nsm_include.h"
#include "vty_include.h"


#include "pjsip_app_api.h"
#include "pjsua_app_common.h"
#include "pjsua_app_config.h"

#include "voip_app.h"
#include "voip_util.h"
#include "voip_log.h"
#include "voip_uci.h"
#include "voip_volume.h"

#include "application.h"



voip_app_t  *voip_app = NULL;


static int voip_app_dtmf_recv_callback(int id, void *p, int input);
static int voip_app_register_state_callback(int id, void *p, int input);
static int voip_app_call_state_callback(int id, void *p, int input);
static int voip_app_call_incoming_callback(int id, void *p, int input);

static int voip_app_call_session_delete(voip_app_t *app, voip_call_t *call);


static int voip_app_call_timeout(void *p);
static int voip_app_call_next_number(void *p);
static zpl_bool voip_app_call_next();

int void_module_init(pl_pjsip_t *pj)
{
	if(voip_app == NULL)
		voip_app = XMALLOC(MTYPE_VOIP_APP, sizeof(voip_app_t));
	zassert(voip_app != NULL);

	pjsip_callback_tbl cb;
	cb.pjsip_dtmf_recv = voip_app_dtmf_recv_callback;
	cb.pjsip_call_state = voip_app_call_state_callback;
	cb.pjsip_reg_state = voip_app_register_state_callback;
	cb.pjsip_reg_state = voip_app_register_state_callback;
	cb.cli_account_state_get = pl_pjsip_account_set_api;
	cb.pjsip_call_incoming = voip_app_call_incoming_callback;
	pjsip_app_callback_init(&app_config, &cb);

#ifdef X5B_APP_DATABASE
	voip_dbase_load();
#endif
	voip_thlog_init();

#ifdef ZPL_SERVICE_UBUS_SYNC
	//ubus_sync_hook_install(voip_ubus_uci_update_cb, NULL);
#endif

	voip_status_clear_api();
#ifdef APP_X5BA_MODULE
	x5b_app_module_init(NULL, 0);
	if(voip_app)
	voip_app->x5b_app = x5b_app_tmp();
#endif
	if(voip_app)
		voip_app->pjsip = pj;
	if(pj)
		pj->userdata = voip_app;

	/*
	 * 声音音量控制单元初始化
	 */
	voip_volume_module_init();


#ifdef ZPL_OPENWRT_UCI
	voip_uci_sip_config_load(pl_pjsip);
	voip_stream_config_load(pl_pjsip);
#endif
	VOIP_APP_DEBUG_ON(EVENT);
	return OK;
}

#ifdef ZPL_OPENWRT_UCI
int pl_pjsip_module_reload()
{
	voip_uci_sip_config_load(pl_pjsip);
	voip_stream_config_load(pl_pjsip);
	return OK;
}
#endif

int void_module_exit(pl_pjsip_t *pj)
{

	voip_volume_module_exit();
	voip_status_clear_api();
#ifdef APP_X5BA_MODULE
	x5b_app_module_exit();
#endif

	zassert(voip_app != NULL);
	XFREE(MTYPE_VOIP_APP, voip_app);
	voip_app = NULL;

	return OK;
}


int void_module_task_init()
{
#ifdef APP_X5BA_MODULE
	x5b_app_module_task_init();
#endif
	return OK;
}

int void_module_task_exit()
{
#ifdef APP_X5BA_MODULE
	x5b_app_module_task_exit();
#endif
	return OK;
}


voip_app_state_t voip_app_state_get(voip_app_t *app)
{
	if(app == NULL)
		return APP_STATE_TALK_IDLE;
	return app->state;
}

int voip_app_state_set(voip_app_t *app, voip_app_state_t state)
{
	if(app == NULL)
		return ERROR;
	if(state < APP_STATE_TALK_IDLE || state > APP_STATE_TALK_RUNNING)
		return ERROR;
	app->state = state;
	return OK;
}



/*
 * debug
 */
int voip_app_debug_set_api(int value)
{
	zassert(voip_app != NULL);
	voip_app->debug  = value;
	return OK;
}

int voip_app_debug_get_api()
{
	zassert(voip_app != NULL);
	return voip_app->debug;
}



/*
 * dtmf recv callback
 */
static int voip_app_dtmf_recv_callback(int id, void *p, int input)
{
	if (input == '#' && voip_app)
	{
		//int ret = 0;
		if(app_incoming_call())
		{
			if (VOIP_APP_DEBUG(EVENT))
				zlog_debug(MODULE_PJSIP, "HUIFU/NONE Product recv dtmf:%c and open door", input);
/*			if (voip_app && voip_app->session)
			{
											voip_app->incoming_session->remote_proto,
								voip_app->incoming_session->phone.phone,
								voip_app->incoming_session->remote_ip,
								voip_app->incoming_session->port);
*/
			if(voip_app->incoming_session)
			{
				voip_app->incoming_session->open_timer = os_time (NULL);

				voip_thlog_log1 (
						0,
						0,
						0,
						NULL,
						voip_app->incoming_session->phone.phone,
						" URL->%s, PORT->%d; Recv '#' Msg, and Open Door",
						/*voip_app->incoming_session->phone.username*/voip_app->incoming_session->remote_ip,
						/*voip_app->incoming_session->phone.user_id*/voip_app->incoming_session->port);
			}
			if (x5b_app_customizer () == CUSTOMIZER_HUIFU ||
						x5b_app_customizer () == CUSTOMIZER_NONE ||
						x5b_app_customizer () == CUSTOMIZER_SECOM)
			{
				if(voip_app->session && voip_app->session->source == APP_CALL_ID_UI)
				{
					if (x5b_app_mode_X5CM ())
						x5b_app_open_door_api (NULL, E_CMD_OPEN, E_CMD_TO_C);
					else
						return x5b_app_open_door_api (NULL, E_CMD_OPEN, E_CMD_TO_A);
				}
			}
			else
			{
				if (VOIP_APP_DEBUG(EVENT))
					zlog_debug(MODULE_PJSIP, "Unknonw Product recv dtmf:%c and open door", input);
			}
			return OK;
		}
		if (voip_app && voip_app->session)
			voip_app->session->open_timer = os_time (NULL);
#if 0
		if (x5b_app_customizer () == CUSTOMIZER_SECOM)
		{
			zpl_int8 cardid[32];
			memset (cardid, 0, sizeof(cardid));
			if (VOIP_APP_DEBUG(EVENT))
				zlog_debug(MODULE_PJSIP, "SECOM Product recv dtmf:%c and open door", input);
			if (voip_app && voip_app->session)
				ret = x5b_user_get_card (
							voip_app->session->phonetab[voip_app->session->index].username,
							voip_app->session->phonetab[voip_app->session->index].user_id,
							cardid, 0);
			if (ret == OK)
			{
				zpl_uint8 ID[8];
				if (strlen (((char *) cardid)))
				{
					card_id_string_to_hex (cardid, strlen (((char *) cardid)),
										   ID);
				}
				x5b_app_open_door_by_cardid (NULL, ID, strlen (cardid) / 2,
										E_CMD_TO_A);

/*				if (x5b_app_mode_X5CM ())
					x5b_app_open_door_api (NULL, E_CMD_OPEN, E_CMD_TO_C);*/
			}
			if (voip_app && voip_app->session)
			{
				if (strlen (cardid))
					voip_thlog_log1 (
							voip_app->session->building,
							voip_app->session->unit,
							voip_app->session->room_number,
							cardid,
							voip_app->session->phonetab[voip_app->session->index].phone,
							" user->%s, ID->%s; Recv '#' Msg, and Open Door",
							voip_app->session->phonetab[voip_app->session->index].username,
							voip_app->session->phonetab[voip_app->session->index].user_id);
				else
					voip_thlog_log1 (
							voip_app->session->building,
							voip_app->session->unit,
							voip_app->session->room_number,
							NULL,
							voip_app->session->phonetab[voip_app->session->index].phone,
							" user->%s, ID->%s; Recv '#' Msg, and Open Door",
							voip_app->session->phonetab[voip_app->session->index].username,
							voip_app->session->phonetab[voip_app->session->index].user_id);
			}
		}
		else
#endif
			if (x5b_app_customizer () == CUSTOMIZER_HUIFU ||
				x5b_app_customizer () == CUSTOMIZER_NONE ||
				x5b_app_customizer () == CUSTOMIZER_SECOM)
		{
			if (VOIP_APP_DEBUG(EVENT))
				zlog_debug(MODULE_PJSIP, "HUIFU/NONE Product recv dtmf:%c and open door", input);
			if (voip_app && voip_app->session)
			{
				char *phone = voip_app->session->phonetab[voip_app->session->index].phone;
				char *username = voip_app->session->phonetab[voip_app->session->index].username;
				char *user_id = voip_app->session->phonetab[voip_app->session->index].user_id;
				voip_thlog_log1 (
						voip_app->session->building,
						voip_app->session->unit,
						voip_app->session->room_number,
						NULL,
						strlen(phone)?phone:"123456",
						" user->%s, ID->%s; Recv '#' Msg, and Open Door",
						strlen(username)?username:"test",
						strlen(user_id)?user_id:"testid");
			}
			//voip_update_dblog(0, zpl_true);
			//start_timer
			//voip_thlog_log("Recv '#' Msg, and Open Door");
			//voip_thlog_log1(zpl_uint8 building, zpl_uint8 unit, zpl_uint16 room, char *phone, const char *format, ...);
			if(voip_app->session && voip_app->session->source == APP_CALL_ID_UI)
			{
				if (x5b_app_mode_X5CM ())
					x5b_app_open_door_api (NULL, E_CMD_OPEN, E_CMD_TO_C);
				else
					return x5b_app_open_door_api (NULL, E_CMD_OPEN, E_CMD_TO_A);
			}
			//return x5b_app_open_door_api(cmd);
		}
		else
		{
			if (VOIP_APP_DEBUG(EVENT))
				zlog_debug(MODULE_PJSIP, "Unknonw Product recv dtmf:%c and open door", input);
		}
	}
	else
	{
		if (VOIP_APP_DEBUG(EVENT))
			zlog_debug(MODULE_PJSIP, "module recv dtmf:%c", input);
		return ERROR;
	}
	//zlog_debug(MODULE_PJSIP, "module recv dtmf:%c", input);
	return OK;
}

static int voip_app_register_state_callback(int id, void *p, int input)
{
	pjsua_acc_info reginfo;
	memset(&reginfo, 0, sizeof(reginfo));
	if(pjsua_acc_get_info(id, &reginfo) == PJ_SUCCESS)
	{
		pl_pjsip_account_set_api(id, &reginfo);

		zassert(pl_pjsip != NULL);
		if(pl_pjsip->mutex)
			os_mutex_lock(pl_pjsip->mutex, OS_WAIT_FOREVER);

		if(pl_pjsip->sip_user.register_svr)
		{
			voip_status_register_api(pl_pjsip->sip_user.sip_state);
			voip_status_register_main_api(zpl_true);
		}
		else if(pl_pjsip->sip_user_sec.register_svr)
		{
			voip_status_register_api(pl_pjsip->sip_user_sec.sip_state);
			voip_status_register_main_api(zpl_false);
		}

		if(pl_pjsip->mutex)
			os_mutex_unlock(pl_pjsip->mutex);
	}
	return OK;
}

static int voip_app_call_state_callback(int id, void *p, int input)
{
	//zlog_debug(MODULE_PJSIP, "call state -> :%d", input);
	if(input == PJSIP_INV_STATE_NULL)
	{
		if(voip_app && voip_app->session)
		{
			if(voip_app->session->time_id > 0)//关闭呼叫超时定时器
			{
				//V_APP_DEBUG("==============%s: os_time_destroy for NULL===========", __func__);
				os_time_destroy(voip_app->session->time_id);
				voip_app->session->time_id = 0;
			}
			voip_app->call_index = voip_app->session->index + 1;
			voip_app_state_set(voip_app, APP_STATE_TALK_IDLE);
			if(voip_app->session->active && voip_app->session->num &&
					((voip_app->session->index + 1) <= voip_app->session->num) &&
					(APP_CALL_ID_UI == voip_app->session->source))
				x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_STOP, voip_app->call_index, E_CMD_TO_AUTO);
		}
		else if(voip_app && app_incoming_call())
		{
			V_APP_DEBUG("==============%s: delete incoming call session(call state -> :%d)", __func__, input);
			voip_app_state_set(voip_app, APP_STATE_TALK_IDLE);
			if(voip_app->session && voip_app->session->source == APP_CALL_ID_UI)
			{
				x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_STOP, 0, E_CMD_TO_AUTO);
			}
			if(voip_app->incoming_session)
			{
				XFREE(MTYPE_VOIP_SESSION, voip_app->incoming_session);
				voip_app->incoming_session = NULL;
			}
		}
	}
	else if(input == PJSIP_INV_STATE_CALLING)
	{
		if(voip_app && voip_app->session)
		{
			voip_app->call_index = voip_app->session->index + 1;
			voip_app_state_set(voip_app, APP_STATE_TALK_CALLING);
			if(voip_app->session->active && voip_app->session->num &&
					((voip_app->session->index + 1) <= voip_app->session->num) &&
					(APP_CALL_ID_UI == voip_app->session->source))
				x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_CALLING, voip_app->call_index, E_CMD_TO_AUTO);
		}
		else if(voip_app && app_incoming_call())
		{
			voip_app_state_set(voip_app, APP_STATE_TALK_CALLING);
			if(voip_app->session && voip_app->session->source == APP_CALL_ID_UI)
			{
				x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_INCOME, 0, E_CMD_TO_AUTO);
			}
		}
	}
	else if(input == PJSIP_INV_STATE_INCOMING)
	{
		V_APP_DEBUG("==============%s: os_time_destroy for INCOMING===========", __func__);
	}
	else if(input == PJSIP_INV_STATE_EARLY)
	{
		if(voip_app && voip_app->session)
		{
			voip_app->call_index = voip_app->session->index + 1;
			voip_app_state_set(voip_app, APP_STATE_TALK_CALLING);
			if(voip_app->session->active && voip_app->session->num &&
					((voip_app->session->index + 1) <= voip_app->session->num) &&
					(APP_CALL_ID_UI == voip_app->session->source))
				x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_CALLING, voip_app->call_index, E_CMD_TO_AUTO);
		}
		else if(voip_app && app_incoming_call())
		{
			V_APP_DEBUG("==============%s: incoming call session(call state -> :%d)", __func__, input);
			voip_app_state_set(voip_app, APP_STATE_TALK_CALLING);
			if(voip_app->session && voip_app->session->source == APP_CALL_ID_UI)
			{
				x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_CALLING, 0, E_CMD_TO_AUTO);
			}
		}
	}
	else if(input == PJSIP_INV_STATE_CONNECTING)
	{
		if(voip_app && voip_app->session)
		{
			if(voip_app->session->time_id > 0)//关闭呼叫超时定时器
			{
				//V_APP_DEBUG("==============%s: os_time_destroy for CONNECTING===========", __func__);
				os_time_destroy(voip_app->session->time_id);
				voip_app->session->time_id = 0;
			}
			voip_app->call_index = voip_app->session->index + 1;
			voip_app_state_set(voip_app, APP_STATE_TALK_SUCCESS);
			if(voip_app->session->active && voip_app->session->num &&
					((voip_app->session->index + 1) <= voip_app->session->num) &&
					(APP_CALL_ID_UI == voip_app->session->source))
				x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_TALKLING, voip_app->call_index, E_CMD_TO_AUTO);
		}
		else if(voip_app && app_incoming_call())
		{
			V_APP_DEBUG("==============%s: incoming call session(call state -> :%d)", __func__, input);
			if(voip_app->incoming_session)
				voip_app->incoming_session->start_timer = os_time(NULL);
			voip_app_state_set(voip_app, APP_STATE_TALK_SUCCESS);
			if(voip_app->session && voip_app->session->source == APP_CALL_ID_UI)
			{
				x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_TALKLING, 0, E_CMD_TO_AUTO);
			}
		}
	}
	else if(input == PJSIP_INV_STATE_CONFIRMED)
	{
		if(voip_app && voip_app->session)
		{
			if(voip_app->session->time_id > 0)//关闭呼叫超时定时器
			{
				//V_APP_DEBUG("==============%s: os_time_destroy for CONFIRMED===========", __func__);
				os_time_destroy(voip_app->session->time_id);
				voip_app->session->time_id = 0;
			}
			voip_app->call_index = voip_app->session->index + 1;
			voip_app_state_set(voip_app, APP_STATE_TALK_RUNNING);
			if(voip_app->session->active && voip_app->session->num &&
					((voip_app->session->index + 1) <= voip_app->session->num) &&
					(APP_CALL_ID_UI == voip_app->session->source))
				x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_TALKLING, voip_app->call_index, E_CMD_TO_AUTO);
		}
		else if(voip_app && app_incoming_call())
		{
			V_APP_DEBUG("==============%s: incoming call session(call state -> :%d)", __func__, input);
			if(voip_app->incoming_session)
				voip_app->incoming_session->start_timer = os_time(NULL);
			voip_app_state_set(voip_app, APP_STATE_TALK_RUNNING);
			if(voip_app->session && voip_app->session->source == APP_CALL_ID_UI)
			{
				x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_TALKLING, 0, E_CMD_TO_AUTO);
			}
		}
	}
	else if(input == PJSIP_INV_STATE_DISCONNECTED)
	{
		if(voip_app)
		{
			if(voip_app->session)
			{
				voip_app_state_t state = voip_app_state_get(voip_app);
				if(state <= APP_STATE_TALK_CALLING)
				{
					if(voip_app_call_next())//可以呼叫下一个号码
					{
						voip_app_state_set(voip_app, APP_STATE_TALK_IDLE);
						if(voip_app->session->time_id > 0)//关闭呼叫超时定时器
						{
							//V_APP_DEBUG("==============%s: os_time_destroy for call next===========", __func__);
							os_time_destroy(voip_app->session->time_id);
							voip_app->session->time_id = 0;
						}
						if(voip_app->stop_and_next == zpl_false)//呼叫异常（未呼通）情况下呼叫下一个号码
						{
							//voip_app->callingnext = zpl_true;
							//清除呼叫实例编号
							voip_app->session->instance = -1;
							//V_APP_DEBUG("================================%s-------> call next phone number", __func__);
							//触发下一个号码呼叫流程
							//os_job_add(OS_JOB_NONE,voip_app_call_timeout, voip_app->session);
							os_job_add(OS_JOB_NONE,voip_app_call_next_number, voip_app->session);
							//voip_app->session->time_id = os_time_create_once(voip_app_call_timeout, voip_app->session, 1000);
						}
					}
					else//最后一个号码，呼叫结束
					{
						voip_app->call_index = voip_app->session->index + 1;
						//V_APP_DEBUG("================================%s-------> call stop", __func__);
						if(voip_app->session->time_id > 0)
						{
							//V_APP_DEBUG("==============%s: os_time_destroy call last===========", __func__);
							os_time_destroy(voip_app->session->time_id);
							voip_app->session->time_id = 0;
						}
						voip_app_state_set(voip_app, APP_STATE_TALK_IDLE);
						//if(voip_app->session->time_id > 0)
						if((APP_CALL_ID_UI == voip_app->session->source))
							x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_STOP, voip_app->call_index, E_CMD_TO_AUTO);
						voip_app_call_session_delete(voip_app, voip_app->session);
					}
				}
				else
				{
					voip_app->call_index = voip_app->session->index + 1;
					//V_APP_DEBUG("================================%s-------> call hangup", __func__);
					if(voip_app->session->time_id > 0)
					{
						//V_APP_DEBUG("==============%s: os_time_destroy hangup===========", __func__);
						os_time_destroy(voip_app->session->time_id);
						voip_app->session->time_id = 0;
					}
					voip_app_state_set(voip_app, APP_STATE_TALK_IDLE);
					if((APP_CALL_ID_UI == voip_app->session->source))
						x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_STOP, voip_app->call_index, E_CMD_TO_AUTO);
					voip_app_call_session_delete(voip_app, voip_app->session);
				}
			}
			else if(app_incoming_call())
			{
				V_APP_DEBUG("==============%s: delete incoming call session(call state -> :%d)", __func__, input);
				//voip_app->incoming_session->stop_timer = os_time(NULL);
				if(voip_app->incoming_session)
					voip_app->incoming_session->stop_timer = os_time(NULL);
				voip_app_state_set(voip_app, APP_STATE_TALK_IDLE);
				if(voip_app->session && voip_app->session->source == APP_CALL_ID_UI)
				{
					x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_STOP, 0, E_CMD_TO_AUTO);
				}
				if(voip_app->incoming_session)
				{
					XFREE(MTYPE_VOIP_SESSION, voip_app->incoming_session);
					voip_app->incoming_session = NULL;
				}
			}
			//voip_app_state_set(voip_app, APP_STATE_TALK_IDLE);
			//voip_volume_control_api(zpl_false);
		}
	}
	return OK;
}

static int voip_app_call_frpm_split(const char *input, char *pro, char *num, char *url)
{
	/*"1111" <sip:1111@AIO100>*/
	char *str = input;
	char *brk = NULL;
	char sip[32];
	char nnum[32];
	char uurl[32];
	brk = strstr(str, "<");
	if(brk)
	{
		brk++;
		memset(sip, 0, sizeof(sip));
		memset(nnum, 0, sizeof(nnum));
		memset(uurl, 0, sizeof(uurl));
		if(strstr(brk, ":") && strstr(brk, "@"))
		{
			os_sscanf(brk, "%[^:]:%[^@]@%[^>]", sip, nnum, uurl);
			if(pro)
				strcpy(pro, sip);
			if(num)
				strcpy(num, nnum);
			if(url)
				strcpy(url, uurl);
		}
	}
	return OK;
}

static int voip_app_call_incoming_callback(int id, void *p, int input)
{
	if (app_incoming_call()) {
		if (find_current_call() == id) {
			pjsua_call_info *call_info = p;
			if (input == 0 && voip_app
					&& (voip_app_state_get(voip_app) == APP_STATE_TALK_IDLE)) {
				if (voip_app->incoming_session) {
					XFREE(MTYPE_VOIP_SESSION, voip_app->incoming_session);
					voip_app->incoming_session = NULL;
				}
				voip_app->incoming_session = XMALLOC(MTYPE_VOIP_SESSION,
						sizeof(voip_call_incoming_t));
				if (voip_app->incoming_session) {
					if (VOIP_APP_DEBUG(EVENT)) {
						zlog_debug(MODULE_PJSIP,
								" Incoming call for!\r\n"
								"   Media count: %d audio & %d video\r\n"
								"   From: %.*s\r\n"
								"   To: %.*s\r\n"
								"   Contact: %.*s\r\n",
								call_info->rem_aud_cnt, call_info->rem_vid_cnt,
								(int )call_info->remote_info.slen,
								call_info->remote_info.ptr,
								(int )call_info->local_info.slen,
								call_info->local_info.ptr,
								(int )call_info->remote_contact.slen,
								call_info->remote_contact.ptr);

						//zlog_debug(MODULE_PJSIP, "module recv dtmf:%c", input);
					}
					//<sip:1111@192.168.3.254:5060>
					if (call_info->remote_contact.slen) {
						char uurl[128];
						memset(uurl, 0, sizeof(uurl));
						voip_app_call_frpm_split(call_info->remote_contact.ptr,
								voip_app->incoming_session->remote_proto,
								voip_app->incoming_session->phone.phone, uurl);
						if (strstr(uurl, ":")) {
							os_sscanf(uurl, "%[^:]:%d",
									voip_app->incoming_session->remote_ip,
									&voip_app->incoming_session->port);
						} else {
							strcpy(voip_app->incoming_session->remote_ip, uurl);
							voip_app->incoming_session->port =
									PJSIP_PORT_DEFAULT;
						}

						__ZPL_PJSIP_DEBUG(
								"----%s---> proto=%s, num=%s, remote=%s port=%d\r\n",
								__func__,
								voip_app->incoming_session->remote_proto,
								voip_app->incoming_session->phone.phone,
								voip_app->incoming_session->remote_ip,
								voip_app->incoming_session->port);
					}
					/*"1111" <sip:1111@AIO100>*/
					else if (call_info->remote_info.slen) {
						char uurl[128];
						memset(uurl, 0, sizeof(uurl));
						voip_app_call_frpm_split(call_info->remote_info.ptr,
								voip_app->incoming_session->remote_proto,
								voip_app->incoming_session->phone.phone, uurl);

						if (strstr(uurl, ":")) {
							os_sscanf(uurl, "%[^:]:%d",
									voip_app->incoming_session->remote_ip,
									&voip_app->incoming_session->port);
						} else {
							strcpy(voip_app->incoming_session->remote_ip, uurl);
							voip_app->incoming_session->port =
									PJSIP_PORT_DEFAULT;
						}

						if (strstr(uurl, ".")) {
							__ZPL_PJSIP_DEBUG(
									"----%s---> proto=%s, num=%s, remote=%s port=%d\r\n",
									__func__,
									voip_app->incoming_session->remote_proto,
									voip_app->incoming_session->phone.phone,
									voip_app->incoming_session->remote_ip,
									voip_app->incoming_session->port);
						} else {
							__ZPL_PJSIP_DEBUG(
									"----%s---> proto=%s, num=%s, url=%s\r\n",
									__func__,
									voip_app->incoming_session->remote_proto,
									voip_app->incoming_session->phone.phone,
									voip_app->incoming_session->remote_ip);
						}
					}
					//<sip:1111@192.168.3.254:5060>
					/*					__ZPL_PJSIP_DEBUG("----%s---> Contact: %.*s\r\n", __func__, (int)call_info->remote_contact.slen,
					 call_info->remote_contact.ptr);*/
					voip_app->incoming_session->start_timer = os_time(NULL);
					voip_app_state_set(voip_app, APP_STATE_TALK_CALLING);
					if(voip_app->session && voip_app->session->source == APP_CALL_ID_UI)
					{
						x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_INCOME,
								0, E_CMD_TO_AUTO);
					}
					return OK;
				}
			} else {
				/*	Incoming call for account 0!
				 Media count: 1 audio & 0 video
				 From: "1111" <sip:1111@AIO100>
				 To: <sip:1005@192.168.3.13;ob>
				 Press a to answer or h to reject call
				 (int)call_info.remote_info.slen,
				 call_info.remote_info.ptr,
				 (int)call_info.local_info.slen,
				 call_info.local_info.ptr,

				 __ZPL_PJSIP_DEBUG(
				 "Incoming call for account %d!\r\n"
				 "Media count: %d audio & %d video\r\n"
				 "From: %.*s\r\n"
				 "To: %.*s\r\n"
				 "Press %s to answer or %s to reject call\r\n",
				 acc_id,
				 call_info.rem_aud_cnt,
				 call_info.rem_vid_cnt,
				 (int)call_info.remote_info.slen,
				 call_info.remote_info.ptr,
				 (int)call_info.local_info.slen,
				 call_info.local_info.ptr,
				 (app_config.use_cli?"ca a":"a"),
				 (app_config.use_cli?"g":"h"));
				 */
				//voip_app_state_set(voip_app, APP_STATE_TALK_SUCCESS);
				//x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_TALKLING, 0, E_CMD_TO_AUTO);
				return OK;
			}
		}
	}
	return ERROR;
}

static int voip_app_call_default(voip_call_t *call, app_call_source_t source)
{
	//app->state = APP_STATE_TALK_IDLE;
	//voip_stream_remote_t remote;
	//app->voip_call;
	//app->local_stop		= zpl_false;		//local stop
	zassert(call != NULL);
	call->active = zpl_false;
	//memset(call->username, 0, sizeof(call->username));
	call->room_number = 0;
#ifdef VOIP_MULTI_CALL_MAX
	call->num = 0;
	memset(call->phonetab, 0, sizeof(call->phonetab));
	call->index = 0;
	call->time_id = 0;			//定时器ID
	call->time_interval = APP_RINGING_TIME_INTERVAL;		//定时间隔（多长时间没有人接听）
#endif

	//call->phone[SIP_NUMBER_MAX];
	//memset(call->phone, 0, sizeof(call->phone));
	call->source = source;
	call->talking = zpl_false;
	call->building = 0;
	//call->debug;
	return OK;
}


int voip_app_call_make(voip_call_t *call, app_call_source_t source, zpl_uint8 building,
		zpl_uint8 unit, zpl_uint16 room)
{
	zassert(call != NULL);
	voip_app_call_default(call, source);

	call->room_number = (room);
	call->unit = (unit);
	call->building = (building);
#ifdef X5B_APP_DATABASE
	if(!x5b_app_mode_X5CM() || CUSTOMIZER_SECOM == x5b_app_customizer())
	{
		call->num = voip_dbase_get_call_phone( building,  unit,  room, call->phonetab);
	}
	else
	{

	}
#endif
	if(call->num == 0)
		return ERROR;
/*
	if(voip_app_call_phonelist_make(call, phonelist) != OK)
	{
			voip_app_call_default(call, source);
			zlog_err(MODULE_PJSIP,
						" error make Call Session phone number in number list");
			return ERROR;
	}*/
	call->active = zpl_false;
	call->source = source;
	call->talking = zpl_false;
	return OK;
}

static int voip_app_call_make_rebuild(voip_call_t *call, app_call_source_t source, char *json)
{
	zassert(call != NULL);
	voip_app_call_default(call, source);

	call->room_number = (0);
	call->unit = (0);
	call->building = (0);
	if(x5b_app_mode_X5CM() && json)
	{
		call->num = x5b_app_call_user_list(json, &call->building, &call->unit,
				&call->room_number,  call->phonetab);
	}

	if(call->num == 0)
		return ERROR;
/*
	if(voip_app_call_phonelist_make(call, phonelist) != OK)
	{
			voip_app_call_default(call, source);
			zlog_err(MODULE_PJSIP,
						" error make Call Session phone number in number list");
			return ERROR;
	}*/
	call->active = zpl_false;
	call->source = source;
	call->talking = zpl_false;
	return OK;
}

static int voip_app_call_make_cli(voip_call_t *call, char *num)
{
	zassert(call != NULL);
	voip_app_call_default(call, APP_CALL_ID_UI);

	call->room_number = (0);
	call->unit = (0);
	call->building = (0);

	strcpy(call->phonetab[0].phone, num);
	strcpy(call->phonetab[0].username, "cli-test");
	strcpy(call->phonetab[0].user_id, "cli01");
	call->num = 1;
	/*
	if(call->num == 0)
		return ERROR;
	*/
/*
	if(voip_app_call_phonelist_make(call, phonelist) != OK)
	{
			voip_app_call_default(call, source);
			zlog_err(MODULE_PJSIP,
						" error make Call Session phone number in number list");
			return ERROR;
	}*/
	call->active = zpl_false;
	call->source = APP_CALL_ID_UI;
	call->talking = zpl_false;
	return OK;
}

/*
 * call session
 */

static int voip_app_call_session_create(voip_app_t *app, voip_call_t *call)
{
	int i = 0;
	zassert(app != NULL);
	zassert(call != NULL);
	for(i = 0; i < VOIP_MULTI_CALL_MAX; i++)
	{
		if(app->call_session[i] == NULL)
		{
			app->call_session[i] = XMALLOC(MTYPE_VOIP_SESSION, sizeof(voip_call_t));
			memcpy(app->call_session[i], call, sizeof(voip_call_t));
			app->call_session[i]->app = app;
			app->call_session[i]->active = zpl_true;
			app->call_session[i]->instance = -1;
			app->session = app->call_session[i];
			//V_APP_DEBUG("=========Create Call Session[%d]", i);
			return OK;
		}
	}
	zlog_err(MODULE_PJSIP, "Can not Create Call Session");
	return ERROR;
}

voip_call_t * voip_app_call_session_lookup_by_number(voip_app_t *app, char *number)
{
	int i = 0, j = 0;
	char phone[PJSIP_NUMBER_MAX];
	zassert(app != NULL);
	zassert(number != NULL);
	memset(phone, 0, sizeof(phone));
	strcpy(phone, number);
	for(i = 0; i < VOIP_MULTI_CALL_MAX; i++)
	{
		if(app->call_session[i] != NULL && app->call_session[i]->active == zpl_true)
		{
			for(i = 0; i < app->call_session[i]->num; i++)
			{
				if(memcmp(app->call_session[i]->phonetab[j].phone, number, sizeof(phone)) == 0)
				{
					return app->call_session[i];
				}
			}
			zlog_err(MODULE_PJSIP,
						"Can not lookup Call Session by %s", number);
			return NULL;
		}
	}
	zlog_err(MODULE_PJSIP,
				"Can not lookup Call Session by %s", number);
	return NULL;
}

voip_call_t * voip_app_call_session_lookup_by_instance(voip_app_t *app, int instance)
{
	int i = 0;//, j = 0;
	zassert(app != NULL);
	for(i = 0; i < VOIP_MULTI_CALL_MAX; i++)
	{
		if(app->call_session[i] != NULL && app->call_session[i]->active == zpl_true)
		{
			for(i = 0; i < app->call_session[i]->num; i++)
			{
				if(app->call_session[i]->instance == instance)
				{
					return app->call_session[i];
				}
			}
			zlog_err(MODULE_PJSIP,
						"Can not lookup Call Session by instance %d", instance);
			return NULL;
		}
	}
	zlog_err(MODULE_PJSIP,
				"Can not lookup Call Session by instance %d", instance);
	return NULL;
}


static int voip_app_call_session_delete(voip_app_t *app, voip_call_t *call)
{
	int i = 0;
	zassert(app != NULL);
	//zassert(call != NULL);
	if(call)
	{
		if(app->session && app->session == call)
			app->session = NULL;
		for(i = 0; i < VOIP_MULTI_CALL_MAX; i++)
		{
			if(app->call_session[i] != NULL && app->call_session[i] == call)
			{
				//V_APP_DEBUG("=========Delete Call Session[%d]", i);
				XFREE(MTYPE_VOIP_SESSION, app->call_session[i]);
				app->call_session[i] = NULL;
				break;
			}
		}
	}
	else
	{
		for(i = 0; i < VOIP_MULTI_CALL_MAX; i++)
		{
			if(app->call_session[i] != NULL && app->call_session[i] == app->session)
			{
				//V_APP_DEBUG("=========Delete Call Session[%d]", i);
				XFREE(MTYPE_VOIP_SESSION, app->call_session[i]);
				app->call_session[i] = NULL;
				break;
			}
		}
		if(app->session)
			app->session = NULL;
	}
	app->call_index = 0;
	app->stop_and_next = zpl_false;
	return OK;
}




static int voip_app_pjsip_call_start(voip_call_t *call)
{
	int ret = 0;
	pl_pjsip_t *sip = NULL;
	zassert(call != NULL);
	zassert(call->app != NULL);
	sip = call->app->pjsip;
	if(sip)
	{
		if(call->active && call->num)
		{
			if(pl_pjsip_app_start_call(current_acc, call->phonetab[call->index].phone, &call->instance) == OK)
			{
				//V_APP_DEBUG("=========Create Call instance '%d'", call->instance);
				if(call->instance >= 0)
				{
					call->start_timer = os_time(NULL);
					if(ret == OK)
					{
						//voip_thlog_log1(call->building, call->unit, call->room_number, call->phonetab[call->index].phone,
						//	" user->%s, ID->%s; start Calling", call->phonetab[call->index].username, call->phonetab[call->index].user_id);
					}
					return ret;
				}
			}
			else
				zlog_err(MODULE_PJSIP,
							"pjsip Start Call to '%s'", call->phonetab[call->index].phone);
		}
		else
			zlog_err(MODULE_PJSIP,
						"call session error:active=%d, num=%d", call->active, call->num);
	}
	else
		zlog_err(MODULE_PJSIP,
				"sip session '%s'", "null");
	return ERROR;
}


static int voip_app_pjsip_call_stop(voip_call_t *call)
{
	pl_pjsip_t *sip = NULL;
	zassert(call != NULL);
	zassert(call->app != NULL);
/*	zassert(call->app->voip_sip != NULL);
	sip = call->app->voip_sip;*/
	sip = call->app->pjsip;
	if(sip && call->instance >= 0)
	{
		//call->app->local_stop = zpl_true;
		return pl_pjsip_app_stop_call(find_current_call(), zpl_true);
	}
	zlog_err(MODULE_PJSIP,
				"Can not Stop Call to '%s'", call->phonetab[call->index].phone);
	return ERROR;
}




/*
 * multi number call
 */
/*
 * 可以呼叫下一个号码
 */
static zpl_bool voip_app_call_next()
{
	zassert(voip_app != NULL);
	if(voip_app->session && voip_app->session->active && voip_app->session->num &&
			((voip_app->session->index + 1) < voip_app->session->num) )
		return zpl_true;
	return zpl_false;
}

static int voip_app_call_next_number(void *p)
{
	voip_call_t *call = (voip_call_t *)p;
	zassert(call != NULL);

	pjsip_media_wait_quit();

	if(call->active && call->num &&
			((call->index + 1) <= call->num) &&
			strlen(call->phonetab[call->index + 1].phone))
	{
		if(call->time_id > 0)
		{
			V_APP_DEBUG("==============%s: os_time_destroy next===========", __func__);
			//zlog_debug(MODULE_PJSIP, "================================%s-------> call stop", __func__);
			os_time_destroy(call->time_id);
			call->time_id = 0;
		}
		if(call->app)
			call->app->stop_and_next = zpl_false;
		call->index++;
		V_APP_DEBUG("================================%s-------> call next number", __func__);
		voip_app_pjsip_call_start(call);

		call->time_id = os_time_create_once(voip_app_call_timeout, call, \
				call->time_interval);

		if(APP_CALL_ID_UI == call->source)
		{
			//voip_app->call_index = call->index + 1;
			x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_CALLING, call->index + 1, E_CMD_TO_AUTO);
		}
		return OK;
	}
	if(call->time_id > 0)
	{
		V_APP_DEBUG("==============%s: os_time_destroy last===========", __func__);
		os_time_destroy(call->time_id);
		call->time_id = 0;
	}
	V_APP_DEBUG("================================%s-------> delete call session", __func__);
	if(APP_CALL_ID_UI == call->source)
		x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_FAIL, 0, E_CMD_TO_AUTO);
	if(call->app)
	{
		voip_app_state_set(voip_app, APP_STATE_TALK_IDLE);
		voip_app_call_session_delete(call->app, call);
	}
	return ERROR;
}


/*
 * 号码呼叫成功，但是没人接通，等待超时
 */
static int voip_app_call_timeout(void *p)
{
	voip_call_t *call = (voip_call_t *)p;
	zassert(call != NULL);
	struct timeval now;
	os_get_monotonic (&now);
	os_timeval_adjust(now);

	V_APP_DEBUG("==============Call timeout===========time=%u.%u msec",
			now.tv_sec*TIMER_MSEC_MICRO, now.tv_sec/TIMER_MSEC_MICRO);

	if(call->active)
	{
		//挂断电话
		voip_app_pjsip_call_stop(call);
		if(call->app)
			call->app->stop_and_next = zpl_true;
		//等待媒体断开
		//pjsip_media_wait_quit();
		//os_sleep(1);
		//当前是最后一个号码，超时终端呼叫
		if((call->index + 1) == call->num)
		{
			V_APP_DEBUG("==============voip_app_call_timeout===========voip_app_call_session_delete");
			if(APP_CALL_ID_UI == call->source)
				x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_FAIL, 0, E_CMD_TO_AUTO);
			if(call->app)
			{
				/*call->app->stop_and_next = zpl_false;
				call->app->session = NULL;*/
				voip_app_state_set(voip_app, APP_STATE_TALK_IDLE);
				voip_app_call_session_delete(call->app, call);
			}
			return OK;
		}
		V_APP_DEBUG("================================%s-------> call next", __func__);
		//进入下一个号码呼叫
		call->time_id = 0;
		os_job_add(OS_JOB_NONE,voip_app_call_next_number, call);
		//call->time_id = 0;
		//voip_app_call_next_number(call);
	}
	return OK;
}






static int _voip_app_call_start_api(voip_app_t *app,
		app_call_source_t source, zpl_uint8 building,
		zpl_uint8 unit, zpl_uint16 room, char *number, zpl_bool rebuild)
{
	int ret = 0;
	voip_call_t call;
	memset(&call, 0, sizeof(voip_call_t));
	zassert(app != NULL);
	app->stop_and_next = zpl_false;
	if(voip_app_state_get(app) == APP_STATE_TALK_IDLE)
	{
		if(rebuild == zpl_false)
		{
			if(number)
				ret = voip_app_call_make_cli(&call, number);
			else
				ret = voip_app_call_make(&call,  source, building, unit, room);
		}
		else
		{
			if(number)
				ret = voip_app_call_make_rebuild(&call,  source, number);
			else
				ret = voip_app_call_make(&call,  source, building, unit, room);
		}
		if(ret == OK)
		{
			if(voip_app_call_session_create(app, &call) == OK)
			{
				if(app->session)
				{
					ret = voip_app_pjsip_call_start(app->session);
					if(APP_CALL_ID_UI == source)
						x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_CALLING,
								app->session->index + 1, E_CMD_TO_AUTO);
					//该呼叫实例包含多个号码，启动轮询呼叫
					if(app->session->num > 1)
					{
						if(app->session->time_id > 0)
						{
							V_APP_DEBUG("==============Call Destroy Old call timeout ===========");
							os_time_destroy(app->session->time_id);
							app->session->time_id = 0;
						}
						app->session->time_id = os_time_create_once(voip_app_call_timeout, app->session, \
							app->session->time_interval);
					}
					return OK;
				}
				else
				{
					if(APP_CALL_ID_UI == source)
						x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_FAIL, 0, E_CMD_TO_AUTO);
					zlog_err(MODULE_PJSIP,
								"Can not set Current Call Session");
					voip_app_state_set(app, APP_STATE_TALK_IDLE);
					return ERROR;
				}
			}
			else
			{
				zlog_err(MODULE_PJSIP,
							"Can not Create Call Session");
				if(APP_CALL_ID_UI == source)
					x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_FAIL, 0, E_CMD_TO_AUTO);
			}
		}
		else
		{
			if(APP_CALL_ID_UI == source)
				x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_FAIL, 0, E_CMD_TO_AUTO);
			zlog_err(MODULE_PJSIP,
						"Can not make Call info");
		}
	}
	else
		zlog_warn(MODULE_PJSIP, "voip app is on talking state");
	voip_app_state_set(app, APP_STATE_TALK_IDLE);
	return ERROR;
}


static int _voip_app_call_stop_api(voip_app_t *app, voip_call_t *call)
{
	zassert(app != NULL);
	zassert(call != NULL);
	voip_app_state_t state = voip_app_state_get(app);
	app->stop_and_next = zpl_false;
	if(state != APP_STATE_TALK_IDLE)
	{
		if(voip_app_pjsip_call_stop(call) == OK)
		{
			if(call->source == APP_CALL_ID_UI)
			{
				if(app->local_stop != zpl_true &&
						voip_app_state_get(app) != APP_STATE_TALK_IDLE)
					x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_STOP, 0, E_CMD_TO_AUTO);
				//voip_app_state_set(app, APP_STATE_TALK_IDLE);
				return OK;
			}
			return OK;
		}
	}
	else
		zlog_warn(MODULE_PJSIP, "voip app is on IDLE state(%d)", state);
	voip_app_state_set(app, APP_STATE_TALK_IDLE);
	return ERROR;
}



/*
 * voip app UI
 */
int voip_app_stop_call_event_ui(voip_event_t *ev)
{
	//zassert(ev != NULL);
	if(pl_pjsip_isregister_api() == zpl_false)
	{
		//x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_UNREGISTER, 0, E_CMD_TO_AUTO);
		return OK;
	}
	zassert(voip_app != NULL);
	if(voip_app->session)
	{
		voip_app->local_stop = zpl_true;
		return _voip_app_call_stop_api(voip_app, voip_app->session);
	}
	return OK;
}

/*
 * 01#2004#0001:0002
 */
int voip_app_start_call_event_ui(voip_event_t *ev)
{
	zassert(ev != NULL);
	zassert(voip_app != NULL);
	zpl_uint16 room = 0;
	zpl_uint8 building = 0;
	zpl_uint8 unit = 0;
	voip_app->local_stop = zpl_false;

	if(pl_pjsip_isregister_api() == zpl_false)
	{
		x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_UNREGISTER, 0, E_CMD_TO_AUTO);
		return OK;
	}
	if(ev->dlen)
		x5b_app_call_room_param_get(ev->data, &building, &unit, &room);

	if(VOIP_APP_DEBUG(EVENT))
		zlog_debug(MODULE_PJSIP, "app start call @'%d'", room);

	if(voip_app_state_get(voip_app) == APP_STATE_TALK_IDLE)
	{
		 return _voip_app_call_start_api(voip_app,
				 APP_CALL_ID_UI, building, unit, room, NULL, zpl_false);
	}
	else
		zlog_warn(MODULE_PJSIP, "voip app is on talking state %d", voip_app_state_get(voip_app));
	return ERROR;
}

int voip_app_start_call_event_ui_phone(voip_event_t *ev)
{
	zassert(ev != NULL);
	zassert(voip_app != NULL);
	zpl_uint16 room = 0;
	zpl_uint8 building = 0;
	zpl_uint8 unit = 0;
	voip_app->local_stop = zpl_false;

	if(ev->dlen <= 0)
	{
		x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_FAIL, 0, E_CMD_TO_AUTO);
		return ERROR;
	}
	if(VOIP_APP_DEBUG(EVENT))
		zlog_debug(MODULE_PJSIP, "app start call @'%s'", ev->data);

	if(pl_pjsip_isregister_api() == zpl_false)
	{
		x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_UNREGISTER, 0, E_CMD_TO_AUTO);
		return OK;
	}
	if(voip_app_state_get(voip_app) == APP_STATE_TALK_IDLE)
	{
		 return _voip_app_call_start_api(voip_app,
				 APP_CALL_ID_UI, building, unit, room, ev->data, zpl_false);
	}
	else
		zlog_warn(MODULE_PJSIP, "voip app is on talking state %d", voip_app_state_get(voip_app));
	return ERROR;
}

int voip_app_start_call_event_ui_user(voip_event_t *ev)
{
	zassert(ev != NULL);
	zassert(voip_app != NULL);
	zpl_uint16 room = 0;
	zpl_uint8 building = 0;
	zpl_uint8 unit = 0;
	voip_app->local_stop = zpl_false;
	if(ev->dlen <= 0)
	{
		x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_FAIL, 0, E_CMD_TO_AUTO);
		return ERROR;
	}
	if(VOIP_APP_DEBUG(EVENT))
		zlog_debug(MODULE_PJSIP, "app start call @'%s'", ev->data);
	if(pl_pjsip_isregister_api() == zpl_false)
	{
		x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_UNREGISTER, 0, E_CMD_TO_AUTO);
		return OK;
	}
	if(voip_app_state_get(voip_app) == APP_STATE_TALK_IDLE)
	{
		 return _voip_app_call_start_api(voip_app,
				 APP_CALL_ID_UI, building, unit, room, ev->data, zpl_true);
	}
	else
		zlog_warn(MODULE_PJSIP, "voip app is on talking state %d", voip_app_state_get(voip_app));
	return ERROR;
}


int voip_app_start_call_event_cli_web(app_call_source_t source, zpl_uint8 building,
		zpl_uint8 unit, zpl_uint16 room, char *number)
{
	zassert(voip_app != NULL);
	if(VOIP_APP_DEBUG(EVENT))
		zlog_debug(MODULE_PJSIP, "app start call @'%d'", room);
	if(pl_pjsip_isregister_api() == zpl_false)
	{
		if(APP_CALL_ID_UI == source)
			x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_UNREGISTER, 0, E_CMD_TO_AUTO);
		return OK;
	}
	voip_app->local_stop = zpl_false;
	if(voip_app_state_get(voip_app) == APP_STATE_TALK_IDLE)
	{
		 return _voip_app_call_start_api(voip_app,
				 source, building, unit, room, number, zpl_false);
	}
	else
		zlog_warn(MODULE_PJSIP, "voip app is on talking state");
	return ERROR;
}



int voip_app_stop_call_event_cli_web(voip_call_t *call)
{
	zassert(voip_app != NULL);
	if(pl_pjsip_isregister_api() == zpl_false)
	{
		//x5b_app_call_internal_result_api(NULL, E_CALL_RESULT_UNREGISTER, 0, E_CMD_TO_AUTO);
		return OK;
	}
	voip_app->local_stop = zpl_true;
	return _voip_app_call_stop_api(voip_app, call ? call:voip_app->session);
}

zpl_bool voip_app_call_event_from_ui()
{
	zassert(voip_app != NULL);
	if(voip_app->session && voip_app->session->source == APP_CALL_ID_UI)
		return zpl_true;
	else
		return zpl_false;
}
zpl_bool voip_app_call_event_from_cli_web()
{
	zassert(voip_app != NULL);
	if(voip_app->session && (voip_app->session->source == APP_CALL_ID_CLI ||
			voip_app->session->source == APP_CALL_ID_WEB))
		return zpl_true;
	else
		return zpl_false;
}

void * voip_app_call_event_current()
{
	zassert(voip_app != NULL);
	return voip_app->session;
}

zpl_bool voip_app_already_call(voip_app_t *app)
{
	zassert(app != NULL);
	if(app->state == APP_STATE_TALK_SUCCESS ||			//通话建立
			app->state == APP_STATE_TALK_RUNNING)			//通话中
	{
		//if(app->voip_call.active)
			return zpl_true;
	}
	return zpl_false;
}

/*
 * osip register
 */

int voip_app_sip_register_start(zpl_bool reg)
{
	zassert(pl_pjsip != NULL);
	if(pl_pjsip)
	{
		return pl_pjsip_app_reg_acc(reg);
	}
	return ERROR;
}

#ifdef VOIP_APP_DEBUG
#endif
