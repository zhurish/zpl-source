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

#include "voip_def.h"
#include "voip_state.h"
#include "voip_stream.h"
#include "voip_task.h"
#include "voip_app.h"
#include "voip_api.h"
#include "voip_osip.h"

#include "application.h"



//voip_call_t voip_call;

#ifdef VOIP_APP_DEBUG
//static voip_stream_remote_t stream_remote_test;
#endif


voip_app_t  *voip_app = NULL;


int voip_app_gl_init()
{
	if(voip_app == NULL)
		voip_app = XMALLOC(MTYPE_VOIP_APP, sizeof(voip_app_t));
	zassert(voip_app != NULL);
	if(voip_app)
	{
		voip_app->voip_task = XMALLOC(MTYPE_VOIP_TOP, sizeof(voip_task_t));
		voip_app->voip_sip = XMALLOC(MTYPE_VOIP_SIP, sizeof(voip_sip_t));
		voip_app->voip_stream = XMALLOC(MTYPE_VOIP_MEDIA, sizeof(voip_stream_t));
		zassert(voip_app->voip_task != NULL);
		zassert(voip_app->voip_sip != NULL);
		zassert(voip_app->voip_stream != NULL);
		//memset(&voip_app->voip_call, 0, sizeof(voip_call_t));
		voip_app->debug = 0xffff;
		return OK;
	}
	return ERROR;
}

int voip_app_gl_exit()
{
	zassert(voip_app != NULL);
	zassert(voip_app->voip_task != NULL);
	zassert(voip_app->voip_sip != NULL);
	zassert(voip_app->voip_stream != NULL);

	XFREE(MTYPE_VOIP_TOP, voip_app->voip_task);
	XFREE(MTYPE_VOIP_SIP, voip_app->voip_sip);
	XFREE(MTYPE_VOIP_MEDIA, voip_app->voip_stream);

	voip_app->voip_task = NULL;
	voip_app->voip_sip = NULL;
	voip_app->voip_stream = NULL;

	XFREE(MTYPE_VOIP_APP, voip_app);
	voip_app = NULL;
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
	zassert(sip_config != NULL);

	zassert(sip_config->osip != NULL);
	if(sip_config && sip_config->osip)
	{
		if(reg)
			return voip_osip_register_api(sip_config->osip);
		else
			return voip_osip_unregister_api(sip_config->osip);
	}
	return ERROR;
}

/*
 * dtmf recv callback
 */
int voip_app_dtmf_recv_callback(char input, int cmd)
{
	if(input == '#')
	{
		int ret = 0;

		if (VOIP_APP_DEBUG(EVENT))
			zlog_debug(ZLOG_VOIP,
					"module recv dtmf:%c and open door", input);

		if(voip_app->session)
			voip_app->session->open_timer = os_time(NULL);

		if(x5b_app_customizer() == CUSTOMIZER_SECOM)
		{
			s_int8 cardid[32];

			memset(cardid, 0, sizeof(cardid));

			ret = x5b_user_get_card(voip_app->session->phonetab[voip_app->session->index].username,
							  voip_app->session->phonetab[voip_app->session->index].user_id,
							  cardid, 0);
			if(ret == OK)
			{
				u_int8 ID[8];
				//zlog_debug(ZLOG_VOIP, "asddddddddd:%s->%d", cardid, strlen(cardid));
				if(strlen(((char *)cardid)))
				{
					card_id_string_to_hex(cardid, strlen(((char *)cardid)), ID);
				}
				x5b_app_cardid_respone(NULL, ID, strlen(cardid)/2, E_CMD_TO_A);

				if(x5b_app_mode_X5CM())
					x5b_app_open_door_api(NULL, cmd, E_CMD_TO_C);
			}
			if(voip_app && voip_app->session)
			{
				if(strlen(cardid))
					voip_thlog_log1(voip_app->session->building, voip_app->session->unit,
								voip_app->session->room_number, cardid,
								voip_app->session->phonetab[voip_app->session->index].phone,
								" user->%s, ID->%s; Recv '#' Msg, and Open Door",
								voip_app->session->phonetab[voip_app->session->index].username,
								voip_app->session->phonetab[voip_app->session->index].user_id);
				else
					voip_thlog_log1(voip_app->session->building, voip_app->session->unit,
								voip_app->session->room_number, NULL,
								voip_app->session->phonetab[voip_app->session->index].phone,
								" user->%s, ID->%s; Recv '#' Msg, and Open Door",
								voip_app->session->phonetab[voip_app->session->index].username,
								voip_app->session->phonetab[voip_app->session->index].user_id);
			}
		}
		else
		{
			if(voip_app && voip_app->session)
			{
				voip_thlog_log1(voip_app->session->building, voip_app->session->unit,
								voip_app->session->room_number, NULL,
								voip_app->session->phonetab[voip_app->session->index].phone,
								" user->%s, ID->%s; Recv '#' Msg, and Open Door",
								voip_app->session->phonetab[voip_app->session->index].username,
								voip_app->session->phonetab[voip_app->session->index].user_id);
			}
			//voip_update_dblog(0, TRUE);
			//start_timer
			//voip_thlog_log("Recv '#' Msg, and Open Door");
			//voip_thlog_log1(u_int8 building, u_int8 unit, u_int16 room, char *phone, const char *format, ...);
			if(x5b_app_mode_X5CM())
				x5b_app_open_door_api(NULL, cmd, E_CMD_TO_C);
			return x5b_app_open_door_api(NULL, cmd, E_CMD_TO_A);
			//return x5b_app_open_door_api(cmd);
		}
	}
	else
	{
		if (VOIP_APP_DEBUG(EVENT))
			zlog_debug(ZLOG_VOIP,
					"module recv dtmf:%c", input);
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
		//if(app->call_session[i].active == FALSE)
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
	char phone[SIP_NUMBER_MAX];
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

static int voip_app_call_session_flush(voip_app_t *app)
{
	int i = 0;
	if(app)
	{
		for(i = 0; i < VOIP_MULTI_CALL_MAX; i++)
		{
			if(app->call_session[i] != NULL)
			{
				XFREE(MTYPE_VOIP_SESSION, app->call_session[i]);
				app->call_session[i] = NULL;
			}
		}
		if(app->session)
			app->session = NULL;
	}
	return OK;
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

/*
 * call instance
 */
/*void * voip_app_call_ID_instance_lookup(voip_call_t *call, int instance)
{
	voip_sip_t *sip = NULL;
	zassert(call != NULL);
	zassert(call->app != NULL);
	zassert(call->app->voip_sip != NULL);
	sip = call->app->voip_sip;
	callparam_t * param = voip_osip_call_lookup_instance_bycid(sip->osip, instance);
	call->sip_session = param;
	return param;
}*/

static int voip_app_call_ID_instance_delete_cb(voip_call_t *call)
{
	//zassert(call != NULL);
	zlog_err(ZLOG_VOIP, "voip_app_call_ID_instance_delete_cb -------------------- >");
	if(call)
	{
		//if((call->index + 1) == call->num)
		{
			if(call->app)
			{
				//if(call->app->session && call->app->session == call)
				//	call->app->session = NULL;

				//if(call && call->app)
				{
					zlog_debug(ZLOG_VOIP, "==============voip_app_call_ID_instance_delete_cb===========voip_app_call_session_delete");
					os_time_create_once(voip_app_call_session_flush, call->app, 1000);
					//voip_app_call_session_delete(call->app, call);
				}
				if(call->app->voip_stream &&
						voip_media_state_get(call->app->voip_stream) != SIP_STATE_MEDIA_CONNECTED)
				{
					if(call->app->local_stop != TRUE &&
							voip_app_state_get(call->app) != APP_STATE_TALK_IDLE)
						x5b_app_call_result_api(NULL, E_CALL_RESULT_STOP, 0, E_CMD_TO_AUTO);
				}
#ifdef PL_OPENWRT_UCI
				os_uci_del("voipconfig", "testing", "callnum", NULL);
				os_uci_set_integer("voipconfig.testing.callstate", 0);
				//os_uci_set_integer("voipconfig.testing.callstate", 0);
				voip_status_talk_api(FALSE);
				//os_uci_save_config("voipconfig");
#endif
			}
		}
/*		call->app = NULL;
		call->sip_session = NULL;
		call->instance = 0;*/
	}
	else
	{
#ifdef PL_OPENWRT_UCI
		os_uci_del("voipconfig", "testing", "callnum", NULL);
		os_uci_set_integer("voipconfig.testing.callstate", 0);
		//os_uci_set_integer("voipconfig.testing.callstate", 0);
		voip_status_talk_api(FALSE);
		//os_uci_save_config("voipconfig");
#endif
		if(call && call->app->voip_stream &&
				voip_media_state_get(call->app->voip_stream) != SIP_STATE_MEDIA_CONNECTED)
		{
			if(call->app->local_stop != TRUE&&
					voip_app_state_get(call->app) != APP_STATE_TALK_IDLE)
				x5b_app_call_result_api(NULL, E_CALL_RESULT_STOP, 0, E_CMD_TO_AUTO);
		}
	}
	if(call && call->app)
		voip_app_state_set(call->app, APP_STATE_TALK_IDLE);
	else
		voip_app_state_set(voip_app, APP_STATE_TALK_IDLE);
	return OK;
}

static int voip_app_call_ID_instance_update(voip_call_t *call, int instance)
{
	zassert(call != NULL);
	if(!call->sip_session)
	{
		voip_sip_t *sip = NULL;
		zassert(call != NULL);
		zassert(call->app != NULL);
		zassert(call->app->voip_sip != NULL);
		sip = call->app->voip_sip;
		callparam_t * param = voip_osip_call_lookup_instance_bycid(sip->osip, instance);
		call->sip_session = param;
	}
	if(call->sip_session)
	{
		callparam_t * param = call->sip_session;
		param->pVoid = call;
		param->source = call->source;
		param->delete_cb = voip_app_call_ID_instance_delete_cb;
		return OK;
	}
	zlog_err(ZLOG_VOIP,
				"Can not Update Call Session by instance or no current session %d", instance);
	return ERROR;
}



static int voip_app_osip_call_start(voip_call_t *call)
{
	int ret = 0;
	voip_sip_t *sip = NULL;
	zassert(call != NULL);
	zassert(call->app != NULL);
	zassert(call->app->voip_sip != NULL);
	sip = call->app->voip_sip;
	if(sip && sip->osip)
	{
		if(call->active && call->num)
		{
			//call->index = 0;
			if(voip_osip_call_start_api(sip->osip, NULL, call->phonetab[call->index].phone, &call->instance) == OK)
			{
				zlog_debug(ZLOG_VOIP,"=====get Call instance '%d'", call->instance);
				if(call->instance)
				{
					call->start_timer = os_time(NULL);
					ret = voip_app_call_ID_instance_update(call, call->instance);
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


static int voip_app_osip_call_stop(voip_call_t *call)
{
	voip_sip_t *sip = NULL;
	zassert(call != NULL);
	zassert(call->app != NULL);
	zassert(call->app->voip_sip != NULL);
	sip = call->app->voip_sip;
	if(sip && sip->osip)
	{
		return voip_osip_call_stop_api(sip->osip, call->instance);
	}
	zlog_err(ZLOG_VOIP,
				"Can not Stop Call to '%s'", call->phonetab[call->index].phone);
	return ERROR;
}


/*
 * voip media
 */

static int voip_app_media_start(voip_app_t *app, voip_stream_remote_t *remote, int type, int id)
{
	int ret = 0;
	zassert(app != NULL);
	zassert(remote != NULL);
	//voip_stream_remote_t remote;
	if(VOIP_APP_DEBUG(EVENT))
		zlog_debug(ZLOG_VOIP, "app create voip stream %s:%d", remote->r_rtp_address, remote->r_rtp_port);
	if(voip_app_state_get(app)== APP_STATE_TALK_IDLE ||
			voip_app_state_get(app)== APP_STATE_TALK_CALLING /*||
			voip_app_state_get(app)== APP_STATE_TALK_FAILED*/)
	{
/*
		memset(&remote, 0, sizeof(voip_stream_remote_t));
		strcpy(remote.r_rtp_address, address);
		remote.r_rtp_port = port;
		remote.r_payload = pyload;
*/
		x5b_app_call_result_api(NULL, E_CALL_RESULT_TALKLING, 0, E_CMD_TO_AUTO);

		voip_app_state_set(app, APP_STATE_TALK_SUCCESS);
		if(app->session)
		{
#ifdef PL_OPENWRT_UCI
			os_uci_set_string("voipconfig.testing.callnum",
							  app->session->phonetab[app->session->index].phone);
			os_uci_set_integer("voipconfig.testing.callstate", 1);
			//os_uci_save_config("voipconfig");
			voip_status_talk_api(TRUE);
#endif
			//voip_add_dblog(itoa(app->session->room_number, 0), app->session->phonetab[app->session->index].phone);
			//voip_thlog_log("Recv '#' Msg, and Open Door");
			//voip_thlog_log1(app->session->building, app->session->unit, app->session->room_number, app->session->phonetab[app->session->index].phone,
			//	" user->%s, ID->%s; start Talking", app->session->phonetab[app->session->index].username, app->session->phonetab[app->session->index].user_id);
		}
		if(app && app->session && app->session->time_id)
		{
			os_time_destroy(app->session->time_id);
			app->session->time_id = 0;
		}
		ret = voip_stream_create_and_start_api(remote);
		if(ret == OK)
		{
			VOIP_MEDIA_INC(app->voip_stream);
			voip_app_state_set(app, APP_STATE_TALK_RUNNING);
		}
/*		if(APP_CALL_ID_UI == source)
		{
			if(VOIP_APP_DEBUG(EVENT))
				zlog_debug(ZLOG_VOIP, "app send call talking state to ESP");
			x5b_app_call_result_api(E_CALL_RESULT_TALKLING);
		}*/
		zlog_debug(ZLOG_VOIP, "===================%s:", __func__);
	}
	else
		zlog_warn(ZLOG_VOIP, "voip app is on talking state");
	return ret;
}

static int voip_app_media_stop(voip_app_t *app, voip_stream_remote_t *remote, int type, int id)
{
	int ret = 0;
	zassert(app != NULL);
	//zassert(remote != NULL);
	if(type == 0)
		app->local_stop = FALSE;

	if(voip_media_state_get(app->voip_stream) == SIP_STATE_MEDIA_CONNECTED)
	{
		if(app->session)
			app->session->stop_timer = os_time(NULL);

		//if(voip_app_call_event_from_cli_web())
		{
#ifdef PL_OPENWRT_UCI
			os_uci_set_string("voipconfig.testing.callnum", " ");
			os_uci_set_integer("voipconfig.testing.callstate", 0);
			//os_uci_save_config("voipconfig");
			voip_status_talk_api(FALSE);
#endif
		}
		VOIP_MEDIA_DEC(app->voip_stream);

		if(VOIP_APP_DEBUG(EVENT))
			zlog_debug(ZLOG_VOIP, "app stop voip stream and set call state to IDLE");

		if(app->local_stop != TRUE &&
				voip_app_state_get(app) != APP_STATE_TALK_IDLE)
			x5b_app_call_result_api(NULL, E_CALL_RESULT_STOP, 0, E_CMD_TO_AUTO);

		if(VOIP_MEDIA_USE(app->voip_stream) == 0)
		{
			voip_stream_stop_api();
			voip_app_state_set(app, APP_STATE_TALK_IDLE);
			//voip_update_dblog(0, FALSE);
			//voip_thlog_log1(app->session->building, app->session->unit, app->session->room_number, app->session->phonetab[app->session->index].phone,
			//	" user->%s, ID->%s; Stop Talking", app->session->phonetab[app->session->index].username, app->session->phonetab[app->session->index].user_id);
			//memset(&voip_app->voip_call, 0, sizeof(voip_call_t));

			voip_media_state_set(app->voip_stream, SIP_STATE_MEDIA_IDLE);
		}
/*		if(APP_CALL_ID_UI == source)
			x5b_app_call_result_api(E_CALL_RESULT_STOP);*/
	}
	else
		zlog_warn(ZLOG_VOIP, "voip app is on IDLE state");
	return ret;
}

int voip_app_media_callback_init(voip_app_t *app)
{
	voip_sip_t *sip = NULL;
	osip_media_callback_t call_back;
	zassert(app != NULL);
	zassert(app->voip_sip != NULL);
	sip = app->voip_sip;
	if(sip && sip->osip)
	{
		call_back.media_start_cb = voip_app_media_start;
		call_back.pVoidStart = app;
		call_back.media_stop_cb = voip_app_media_stop;
		call_back.pVoidStop = app;

		voip_osip_media_callback(sip->osip, &call_back);
		return OK;
	}
	zlog_err(ZLOG_VOIP,
				"Can not Install Media Callback Func");
	return ERROR;
}

/*
 * multi call
 */
static int voip_app_call_timeout(void *p);

static int voip_app_call_next_number(void *p)
{
	voip_call_t *call = VOIP_EVENT_ARGV(p);
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
		voip_app_osip_call_stop(call);

		if((call->index + 1) == call->num)
		{
			if(APP_CALL_ID_UI == call->source)
				x5b_app_call_result_api(NULL, E_CALL_RESULT_FAIL, 0, E_CMD_TO_AUTO);
			zlog_debug(ZLOG_VOIP, "==============voip_app_call_timeout===========voip_app_call_session_delete");
			voip_app_call_session_delete(call->app, call);
			return OK;
		}
		call->time_id = 0;
		voip_event_ready_add(voip_app_call_next_number, call, NULL, 0, 0);
	}
	return OK;
}

int voip_app_multi_call_next()
{
	zassert(voip_app != NULL);
	if(voip_app->session && voip_app->session->active && voip_app->session->num &&
			((voip_app->session->index + 1) < voip_app->session->num) )
		voip_event_ready_add(voip_app_call_next_number, voip_app->session, NULL, 0, 0);
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
					voip_app_state_set(app, APP_STATE_TALK_CALLING);
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
		if(voip_app_osip_call_stop(call) == OK)
		{
			if(call->source == APP_CALL_ID_UI)
			{
				if(app->local_stop != TRUE &&
						voip_app_state_get(app) != APP_STATE_TALK_IDLE)
					x5b_app_call_result_api(NULL, E_CALL_RESULT_STOP, 0, E_CMD_TO_AUTO);
				voip_app_state_set(app, APP_STATE_TALK_IDLE);
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
	zassert(ev != NULL);
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
