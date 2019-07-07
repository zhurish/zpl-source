/*
 * voip_app.c
 *
 *  Created on: Jan 1, 2019
 *      Author: zhurish
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
#include "os_util.h"
#include "os_socket.h"

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

static int voip_app_call_session_delete(voip_app_t *app, voip_call_t *call);

int pl_pjsip_module_init()
{
/*
	if(!voip_global_enabled())
		return OK;
*/

	_pl_pjsip_module_init();

	pjsip_callback_tbl cb;
	cb.pjsip_dtmf_recv = voip_app_dtmf_recv_callback;
	cb.pjsip_call_state = voip_app_call_state_callback;
	cb.pjsip_reg_state = voip_app_register_state_callback;
	cb.pjsip_reg_state = voip_app_register_state_callback;
	cb.cli_account_state_get = pl_pjsip_account_set_api;
	pjsip_app_callback_init(&app_config, &cb);

	if(voip_app == NULL)
		voip_app = XMALLOC(MTYPE_VOIP_APP, sizeof(voip_app_t));
	zassert(voip_app != NULL);

	voip_dbase_load();

	voip_thlog_init();

#ifdef PL_OPENWRT_UCI
	uci_ubus_cb_install(voip_ubus_uci_update_cb);
#endif

	voip_status_clear_api();
#ifdef APP_X5BA_MODULE
	x5b_app_module_init(NULL, 0);
	if(voip_app)
	voip_app->x5b_app = x5b_app_tmp();
#endif

	voip_app->pjsip = pl_pjsip;

	/*
	 * 声音音量控制单元初始化
	 */
	voip_volume_module_init();


#ifdef PL_OPENWRT_UCI
	voip_uci_sip_config_load(pl_pjsip);
	voip_stream_config_load(pl_pjsip);
#endif

	return OK;
}

#ifdef PL_OPENWRT_UCI
int pl_pjsip_module_reload()
{
	voip_uci_sip_config_load(pl_pjsip);
	voip_stream_config_load(pl_pjsip);
	return OK;
}
#endif

int pl_pjsip_module_exit()
{
/*	if(!voip_global_enabled())
		return OK;*/

	voip_volume_module_exit();
	voip_status_clear_api();
#ifdef APP_X5BA_MODULE
	x5b_app_module_exit();
#endif

	_pl_pjsip_module_exit();

	zassert(voip_app != NULL);
	XFREE(MTYPE_VOIP_APP, voip_app);
	voip_app = NULL;

	return OK;
}


int pl_pjsip_module_task_init()
{
	_pl_pjsip_module_task_init();
#ifdef APP_X5BA_MODULE
	x5b_app_module_task_init();
#endif
	return OK;
}

int pl_pjsip_module_task_exit()
{
	_pl_pjsip_module_task_exit();
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
	if(state < APP_STATE_TALK_IDLE && state > APP_STATE_TALK_RUNNING)
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
 * osip register
 */

int voip_app_sip_register_start(BOOL reg)
{
	zassert(pl_pjsip != NULL);
	if(pl_pjsip)
	{
		return pl_pjsip_app_reg_acc(reg);
	}
	return ERROR;
}

/*
 * dtmf recv callback
 */
static int voip_app_dtmf_recv_callback(int id, void *p, int input)
{
	if (input == '#' && voip_app)
	{
		int ret = 0;

		if (VOIP_APP_DEBUG(EVENT))
			zlog_debug(ZLOG_VOIP, "module recv dtmf:%c and open door", input);

		if (voip_app && voip_app->session)
			voip_app->session->open_timer = os_time (NULL);

		if (x5b_app_customizer () == CUSTOMIZER_SECOM)
		{
			s_int8 cardid[32];

			memset (cardid, 0, sizeof(cardid));

			ret = x5b_user_get_card (
							voip_app->session->phonetab[voip_app->session->index].username,
							voip_app->session->phonetab[voip_app->session->index].user_id,
							cardid, 0);
			if (ret == OK)
			{
				u_int8 ID[8];
				//zlog_debug(ZLOG_VOIP, "asddddddddd:%s->%d", cardid, strlen(cardid));
				if (strlen (((char *) cardid)))
				{
					card_id_string_to_hex (cardid, strlen (((char *) cardid)),
										   ID);
				}
				x5b_app_cardid_respone (NULL, ID, strlen (cardid) / 2,
										E_CMD_TO_A);

				if (x5b_app_mode_X5CM ())
					x5b_app_open_door_api (NULL, E_CMD_OPEN, E_CMD_TO_C);
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
		{
			if (voip_app && voip_app->session)
			{
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
			//voip_update_dblog(0, TRUE);
			//start_timer
			//voip_thlog_log("Recv '#' Msg, and Open Door");
			//voip_thlog_log1(u_int8 building, u_int8 unit, u_int16 room, char *phone, const char *format, ...);
			if (x5b_app_mode_X5CM ())
				x5b_app_open_door_api (NULL, E_CMD_OPEN, E_CMD_TO_C);
			return x5b_app_open_door_api (NULL, E_CMD_OPEN, E_CMD_TO_A);
			//return x5b_app_open_door_api(cmd);
		}
	}
	else
	{
		if (VOIP_APP_DEBUG(EVENT))
			zlog_debug(ZLOG_VOIP, "module recv dtmf:%c", input);
	}
	zlog_debug(ZLOG_VOIP, "module recv dtmf:%c", input);
	return OK;
}

static int voip_app_register_state_callback(int id, void *p, int input)
{
	pjsua_acc_info reginfo;
	memset(&reginfo, 0, sizeof(reginfo));
	if(pjsua_acc_get_info(id, &reginfo) == PJ_SUCCESS)
	{
		pl_pjsip_account_set_api(id, &reginfo);
	}
	return OK;
}

static int voip_app_call_state_callback(int id, void *p, int input)
{
	if(input == PJSIP_INV_STATE_NULL)
	{
		if(voip_app)
			voip_app_state_set(voip_app, APP_STATE_TALK_IDLE);
	}
	else if(input == PJSIP_INV_STATE_CALLING)
	{
		if(voip_app)
			voip_app_state_set(voip_app, APP_STATE_TALK_CALLING);
	}
	else if(input == PJSIP_INV_STATE_EARLY)
	{
		if(voip_app)
			voip_app_state_set(voip_app, APP_STATE_TALK_CALLING);
	}
	else if(input == PJSIP_INV_STATE_CONNECTING)
	{
		if(voip_app)
			voip_app_state_set(voip_app, APP_STATE_TALK_SUCCESS);
	}
	else if(input == PJSIP_INV_STATE_CONNECTING)
	{
		if(voip_app)
			voip_app_state_set(voip_app, APP_STATE_TALK_RUNNING);
	}
	else if(input == PJSIP_INV_STATE_DISCONNECTED)
	{
		if(voip_app)
		{
			if(voip_app->session)
				voip_app_call_session_delete(voip_app, voip_app->session);
			voip_app_state_set(voip_app, APP_STATE_TALK_IDLE);
		}
	}
	return OK;
}


static int voip_app_call_default(voip_call_t *call, app_call_source_t source)
{
	//app->state = APP_STATE_TALK_IDLE;
	//voip_stream_remote_t remote;
	//app->voip_call;
	//app->local_stop		= FALSE;		//local stop
	zassert(call != NULL);
	call->active = FALSE;
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
	call->talking = FALSE;
	call->building = 0;
	//call->debug;
	return OK;
}

BOOL voip_app_already_call(voip_app_t *app)
{
	zassert(app != NULL);
	if(app->state == APP_STATE_TALK_SUCCESS ||			//通话建立
			app->state == APP_STATE_TALK_RUNNING)			//通话中
	{
		//if(app->voip_call.active)
			return TRUE;
	}
	return FALSE;
}



int voip_app_call_make(voip_call_t *call, app_call_source_t source, u_int8 building,
		u_int8 unit, u_int16 room)
{
	zassert(call != NULL);
	voip_app_call_default(call, source);

	call->room_number = (room);
	call->unit = (unit);
	call->building = (building);

	call->num = voip_dbase_get_call_phone( building,  unit,  room, call->phonetab);
	if(call->num == 0)
		return ERROR;
/*
	if(voip_app_call_phonelist_make(call, phonelist) != OK)
	{
			voip_app_call_default(call, source);
			zlog_err(ZLOG_VOIP,
						" error make Call Session phone number in number list");
			return ERROR;
	}*/
	call->active = FALSE;
	call->source = source;
	call->talking = FALSE;
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
			zlog_err(ZLOG_VOIP,
						" error make Call Session phone number in number list");
			return ERROR;
	}*/
	call->active = FALSE;
	call->source = APP_CALL_ID_UI;
	call->talking = FALSE;
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
			app->call_session[i]->active = TRUE;
			app->session = app->call_session[i];
			zlog_err(ZLOG_VOIP, "=========Create Call Session");
			return OK;
		}
	}
	zlog_err(ZLOG_VOIP, "Can not Create Call Session");
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
		if(app->call_session[i] != NULL && app->call_session[i]->active == TRUE)
		{
			for(i = 0; i < app->call_session[i]->num; i++)
			{
				if(memcmp(app->call_session[i]->phonetab[j].phone, number, sizeof(phone)) == 0)
				{
					return app->call_session[i];
				}
			}
			zlog_err(ZLOG_VOIP,
						"Can not lookup Call Session by %s", number);
			return NULL;
		}
	}
	zlog_err(ZLOG_VOIP,
				"Can not lookup Call Session by %s", number);
	return NULL;
}

voip_call_t * voip_app_call_session_lookup_by_instance(voip_app_t *app, int instance)
{
	int i = 0;//, j = 0;
	zassert(app != NULL);
	for(i = 0; i < VOIP_MULTI_CALL_MAX; i++)
	{
		if(app->call_session[i] != NULL && app->call_session[i]->active == TRUE)
		{
			for(i = 0; i < app->call_session[i]->num; i++)
			{
				if(app->call_session[i]->instance == instance)
				{
					return app->call_session[i];
				}
			}
			zlog_err(ZLOG_VOIP,
						"Can not lookup Call Session by instance %d", instance);
			return NULL;
		}
	}
	zlog_err(ZLOG_VOIP,
				"Can not lookup Call Session by instance %d", instance);
	return NULL;
}


static int voip_app_call_session_delete(voip_app_t *app, voip_call_t *call)
{
#if 0
	os_time_create_once(voip_app_call_session_flush, app, 1000);
#else
	int i = 0;
	zassert(app != NULL);
	zassert(call != NULL);
	if(app->session && app->session == call)
		app->session = NULL;
	for(i = 0; i < VOIP_MULTI_CALL_MAX; i++)
	{
		if(app->call_session[i] != NULL && app->call_session[i] == call)
		{
			XFREE(MTYPE_VOIP_SESSION, app->call_session[i]);
			app->call_session[i] = NULL;
			break;
		}
	}
	if((call->index + 1) == call->num)
	{
		//memset(call, 0, sizeof(voip_call_t));
		app->session = NULL;
	}
#endif
	return OK;
}




static int voip_app_osip_call_start(voip_call_t *call)
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
				zlog_debug(ZLOG_VOIP,"=====get Call instance '%d'", call->instance);
				if(call->instance)
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
		}
	}
	zlog_err(ZLOG_VOIP,
				"Can not Start Call to '%s'", call->phonetab[call->index].phone);
	return ERROR;
}


static int voip_app_pjsip_call_stop(voip_call_t *call)
{
	pl_pjsip_t *sip = NULL;
	zassert(call != NULL);
	zassert(call->app != NULL);
/*	zassert(call->app->voip_sip != NULL);
	sip = call->app->voip_sip;*/
	if(sip && call->instance)
	{
		return pl_pjsip_app_stop_call(find_current_call(), TRUE);
	}
	zlog_err(ZLOG_VOIP,
				"Can not Stop Call to '%s'", call->phonetab[call->index].phone);
	return ERROR;
}




/*
 * multi call
 */
static int voip_app_call_timeout(void *p);

int voip_app_call_next_number(void *p)
{
	//voip_call_t *call = VOIP_EVENT_ARGV(p);
	voip_call_t *call = (voip_call_t *)p;
	//voip_call_t *call = (voip_call_t *)p;
	zassert(call != NULL);
	if(call->active && call->num &&
			((call->index + 1) < call->num) &&
			strlen(call->phonetab[call->index + 1].phone))
	{
		if(call->time_id)
		{
			zlog_debug(ZLOG_VOIP, "==============Call os_time_destroy===========");
			os_time_destroy(call->time_id);
			call->time_id = 0;
		}
		call->index++;

		voip_app_osip_call_start(call);

		if(APP_CALL_ID_UI == call->source)
			x5b_app_call_result_api(NULL, E_CALL_RESULT_CALLING, call->index + 1, E_CMD_TO_AUTO);

		call->time_id = os_time_create_once(voip_app_call_timeout, call, \
				call->time_interval);
		return OK;
	}
	if(APP_CALL_ID_UI == call->source)
		x5b_app_call_result_api(NULL, E_CALL_RESULT_FAIL, 0, E_CMD_TO_AUTO);

	if(call->time_id)
	{
		zlog_debug(ZLOG_VOIP, "==============Call os_time_destroy===========");
		os_time_destroy(call->time_id);
		call->time_id = 0;
	}
	zlog_debug(ZLOG_VOIP, "==============Call os_time_destroy===========voip_app_call_session_delete");
	voip_app_call_session_delete(call->app, call);
	return ERROR;
}


static int voip_app_call_timeout(void *p)
{
	voip_call_t *call = (voip_call_t *)p;
	zassert(call != NULL);
	struct timeval now;
	os_get_monotonic (&now);
	os_timeval_adjust(now);

	zlog_debug(ZLOG_VOIP, "==============Call timeout===========time=%u.%u msec",
			now.tv_sec*TIMER_MSEC_MICRO, now.tv_sec/TIMER_MSEC_MICRO);

	if(call->active)
	{
		voip_app_pjsip_call_stop(call);

		if((call->index + 1) == call->num)
		{
			if(APP_CALL_ID_UI == call->source)
				x5b_app_call_result_api(NULL, E_CALL_RESULT_FAIL, 0, E_CMD_TO_AUTO);
			zlog_debug(ZLOG_VOIP, "==============voip_app_call_timeout===========voip_app_call_session_delete");
			voip_app_call_session_delete(call->app, call);
			return OK;
		}
		call->time_id = 0;
		voip_app_call_next_number(call);
		//voip_event_ready_add(voip_app_call_next_number, call, NULL, 0, 0);
	}
	return OK;
}

int voip_app_multi_call_next()
{
	zassert(voip_app != NULL);
	if(voip_app->session && voip_app->session->active && voip_app->session->num &&
			((voip_app->session->index + 1) < voip_app->session->num) )
		voip_app_call_next_number(voip_app->session);
		;//voip_event_ready_add(voip_app_call_next_number, voip_app->session, NULL, 0, 0);
	return OK;
}





static int _voip_app_call_start_api(voip_app_t *app,
		app_call_source_t source, u_int8 building,
		u_int8 unit, u_int16 room, char *number)
{
	int ret = 0;
	voip_call_t call;
	memset(&call, 0, sizeof(voip_call_t));
	zassert(app != NULL);
	if(voip_app_state_get(app) == APP_STATE_TALK_IDLE)
	{
		if(number)
			ret = voip_app_call_make_cli(&call, number);
		else
			ret = voip_app_call_make(&call,  source, building, unit, room);
		if(ret == OK)
		{
			if(voip_app_call_session_create(app, &call) == OK)
			{
				if(app->session)
				{
					ret = voip_app_osip_call_start(app->session);
					if(APP_CALL_ID_UI == source)
						x5b_app_call_result_api(NULL, E_CALL_RESULT_CALLING, app->session->index + 1, E_CMD_TO_AUTO);
					if(app->session->num > 1)
					{
						if(app->session->time_id)
						{
							zlog_debug(ZLOG_VOIP, "==============Call os_time_destroy===========");
							os_time_destroy(app->session->time_id);
							app->session->time_id = 0;
						}
						app->session->time_id = os_time_create_once(voip_app_call_timeout, app->session, \
							app->session->time_interval);
					}
					//voip_app_state_set(app, APP_STATE_TALK_CALLING);
					return OK;
				}
				else
				{
					if(APP_CALL_ID_UI == source)
						x5b_app_call_result_api(NULL, E_CALL_RESULT_FAIL, 0, E_CMD_TO_AUTO);
					zlog_err(ZLOG_VOIP,
								"Can not set Current Call Session");
					voip_app_state_set(app, APP_STATE_TALK_IDLE);
					return ERROR;
				}
			}
			else
			{
				zlog_err(ZLOG_VOIP,
							"Can not Create Call Session");
				if(APP_CALL_ID_UI == source)
					x5b_app_call_result_api(NULL, E_CALL_RESULT_FAIL, 0, E_CMD_TO_AUTO);
			}
		}
		else
		{
			if(APP_CALL_ID_UI == source)
				x5b_app_call_result_api(NULL, E_CALL_RESULT_FAIL, 0, E_CMD_TO_AUTO);
			zlog_err(ZLOG_VOIP,
						"Can not make Call info");
		}
	}
	else
		zlog_warn(ZLOG_VOIP, "voip app is on talking state");
	voip_app_state_set(app, APP_STATE_TALK_IDLE);
	return ERROR;
}


static int _voip_app_call_stop_api(voip_app_t *app, voip_call_t *call)
{
	zassert(app != NULL);
	zassert(call != NULL);
	voip_app_state_t state = voip_app_state_get(app);
	if(state != APP_STATE_TALK_IDLE)
	{
		if(voip_app_pjsip_call_stop(call) == OK)
		{
			if(call->source == APP_CALL_ID_UI)
			{
				if(app->local_stop != TRUE &&
						voip_app_state_get(app) != APP_STATE_TALK_IDLE)
					x5b_app_call_result_api(NULL, E_CALL_RESULT_STOP, 0, E_CMD_TO_AUTO);
				//voip_app_state_set(app, APP_STATE_TALK_IDLE);
				return OK;
			}
			return OK;
		}
	}
	else
		zlog_warn(ZLOG_VOIP, "voip app is on IDLE state(%d)", state);
	voip_app_state_set(app, APP_STATE_TALK_IDLE);
	return ERROR;
}



/*
 * voip app UI
 */
int voip_app_stop_call_event_ui(voip_event_t *ev)
{
	//zassert(ev != NULL);
	zassert(voip_app != NULL);
	if(voip_app->session)
	{
		voip_app->local_stop = TRUE;
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
	u_int16 room = 0;
	u_int8 building = 0;
	u_int8 unit = 0;
	voip_app->local_stop = FALSE;
	x5b_app_call_room_param_get(ev->data, &building, &unit, &room);

	if(VOIP_APP_DEBUG(EVENT))
		zlog_debug(ZLOG_VOIP, "app start call @'%d'", room);

	if(voip_app_state_get(voip_app) == APP_STATE_TALK_IDLE)
	{
		 return _voip_app_call_start_api(voip_app,
				 APP_CALL_ID_UI, building, unit, room, NULL);
	}
	else
		zlog_warn(ZLOG_VOIP, "voip app is on talking state %d", voip_app_state_get(voip_app));
	return ERROR;
}

int voip_app_start_call_event_ui_phone(voip_event_t *ev)
{
	zassert(ev != NULL);
	zassert(voip_app != NULL);
	u_int16 room = 0;
	u_int8 building = 0;
	u_int8 unit = 0;
	voip_app->local_stop = FALSE;
	if(VOIP_APP_DEBUG(EVENT))
		zlog_debug(ZLOG_VOIP, "app start call @'%s'", ev->data);

	if(voip_app_state_get(voip_app) == APP_STATE_TALK_IDLE)
	{
		 return _voip_app_call_start_api(voip_app,
				 APP_CALL_ID_UI, building, unit, room, ev->data);
	}
	else
		zlog_warn(ZLOG_VOIP, "voip app is on talking state %d", voip_app_state_get(voip_app));
	return ERROR;
}

/*
 * voip app cli/web
 */
#if 0
int voip_app_call_spilt_from_web(char *input, u_int8 *building,
		u_int8 *unit, u_int16 *room, char *username, char *userid)
{
	if(input[0] == '@')
	{
		if(building)
			*building = 0;

		if(unit)
			*unit = 0;

		if(room)
			*room = 0;
		//strncpy(phonelist, input+1, MIN(len, strlen(input)-1));
		return OK;
	}
	else
	{
		char *brk = strstr(input, ":");
		if(brk)
		{
			u_int16 build = 0;
			u_int16 unit_inde = 0;
			u_int16 room_number = 0;
			brk += 1;
			if(strstr(input, ":"))
			{
				sscanf(input, "%d:%d:%d", &build, &unit_inde, &room_number);
			}
			else
			{
				sscanf(input, "%d:%d", &unit_inde, &room_number);
			}

			if(voip_dbase_get_room_phone(build, unit_inde, room_number) <= 0)
			{
				zlog_err(ZLOG_VOIP,
							"Can not get Phone Number by Room");
				return ERROR;
			}
			return OK;
		}
		else
		{
			u_int16 room_number = atoi(input);
			if(voip_dbase_get_room_phone(0, 0, room_number, phonelist) <= 0)
			{
				zlog_err(ZLOG_VOIP,
							"Can not get Phone Number by Room");
				return ERROR;
			}
			return OK;
		}
	}
	return ERROR;
}
#endif


int voip_app_start_call_event_cli_web(app_call_source_t source, u_int8 building,
		u_int8 unit, u_int16 room, char *number)
{
	zassert(voip_app != NULL);
	if(VOIP_APP_DEBUG(EVENT))
		zlog_debug(ZLOG_VOIP, "app start call @'%d'", room);
	voip_app->local_stop = FALSE;
	if(voip_app_state_get(voip_app) == APP_STATE_TALK_IDLE)
	{
		 return _voip_app_call_start_api(voip_app,
				 source, building, unit, room, number);
	}
	else
		zlog_warn(ZLOG_VOIP, "voip app is on talking state");
	return ERROR;
}



int voip_app_stop_call_event_cli_web(voip_call_t *call)
{
	zassert(voip_app != NULL);
	voip_app->local_stop = TRUE;
	return _voip_app_call_stop_api(voip_app, call ? call:voip_app->session);
}

BOOL voip_app_call_event_from_ui()
{
	zassert(voip_app != NULL);
	if(voip_app->session && voip_app->session->source == APP_CALL_ID_UI)
		return TRUE;
	else
		return FALSE;
}
BOOL voip_app_call_event_from_cli_web()
{
	zassert(voip_app != NULL);
	if(voip_app->session && (voip_app->session->source == APP_CALL_ID_CLI ||
			voip_app->session->source == APP_CALL_ID_WEB))
		return TRUE;
	else
		return FALSE;
}

void * voip_app_call_event_current()
{
	zassert(voip_app != NULL);
	return voip_app->session;
}

#ifdef VOIP_APP_DEBUG
#endif
