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
#include "voip_task.h"
#include "voip_event.h"
#include "voip_app.h"
#include "voip_api.h"
#include "voip_sip.h"
//#include "estate_mgt.h"
#include "application.h"
#include "voip_stream.h"


voip_call_t voip_call;

#ifdef VOIP_APP_DEBUG
static voip_stream_remote_t stream_remote_test;
#endif

/*
 * stop call event handle
 */
int voip_app_ev_local_stop_call(event_node_t *ev)
{
	if(VOIP_APP_DEBUG(EVENT))
		zlog_debug(ZLOG_VOIP, "app stop call by local");
	if(voip_sip_call_state_get_api() != VOIP_SIP_CALL_IDLE)
	{
		if(VOIP_APP_DEBUG(EVENT))
			zlog_debug(ZLOG_VOIP, "app send call stop cmd to SIP");

		if(voip_sip_call_stop() != OK)
			return ERROR;

		if(VOIP_APP_DEBUG(EVENT))
			zlog_debug(ZLOG_VOIP, "app send call stop state to ESP");
		x5b_app_call_result_api(E_CALL_RESULT_STOP);

		if(voip_sip_call_state_get_api() >= VOIP_SIP_TALK)
		{
			if(VOIP_APP_DEBUG(EVENT))
				zlog_debug(ZLOG_VOIP, "app stop voip stream and set call state to IDLE");
			voip_stream_stop_api();
			voip_sip_call_state_set_api(VOIP_SIP_CALL_IDLE);
		}
		return OK;
	}
	else
		return OK;
}

int voip_app_ev_remote_stop_call(event_node_t *ev)
{
	if(voip_sip_call_state_get_api() != VOIP_SIP_CALL_IDLE)
	{
		if(VOIP_APP_DEBUG(EVENT))
			zlog_debug(ZLOG_VOIP, "app send call stop state to ESP");

		x5b_app_call_result_api(E_CALL_RESULT_STOP);

		//if(voip_sip_call_state_get_api() >= VOIP_SIP_TALK)
		{
			if(VOIP_APP_DEBUG(EVENT))
				zlog_debug(ZLOG_VOIP, "app stop voip stream and set call state to IDLE");
			voip_stream_stop_api();
			voip_sip_call_state_set_api(VOIP_SIP_CALL_IDLE);
		}
		return OK;
	}
	else
		return OK;
}



int voip_app_ev_start_stream(event_node_t *ev)
{
	int ret = 0;
	voip_stream_remote_t *remote = (voip_stream_remote_t *)ev->data;
	if(VOIP_APP_DEBUG(EVENT))
		zlog_debug(ZLOG_VOIP, "app create voip stream %s:%d", remote->r_rtp_address, remote->r_rtp_port);
	if(voip_sip_call_state_get_api() >= VOIP_SIP_CALL_RINGING)
	{
/*		zlog_debug(ZLOG_VOIP,"-----------%s:voip_create_stream_and_start %s:%d(sip state=%d)",
				__func__, remote->r_rtp_address, remote->r_rtp_port, voip_sip_call_state_get_api());
		*/
		ret = voip_create_stream_and_start_api(remote);
		if(VOIP_APP_DEBUG(EVENT))
			zlog_debug(ZLOG_VOIP, "app send call talking state to ESP");
		x5b_app_call_result_api(E_CALL_RESULT_TALKLING);
	}
	return ret;
}

/*
 * start call event handle
 */
#if 1
int voip_app_ev_start_call(event_node_t *ev)
{
	int ret = OK;
	if(strlen(ev->data))
		strcpy(voip_call.phone, ev->data);
	else
	{
		if(VOIP_APP_DEBUG(EVENT))
			zlog_debug(ZLOG_VOIP, "app start call Error, phone number is empty");
		return ERROR;
	}
	//V_APP_DEBUG("-----------%s: room=0x%x", __func__, *num);
	if(VOIP_APP_DEBUG(EVENT))
		zlog_debug(ZLOG_VOIP, "app start call @'%s'", voip_call.phone);

	if(ret == OK)
	{
		//sip call
		if(voip_sip_register_state_get_api() != VOIP_SIP_REGISTER_SUCCESS)
		{
			x5b_app_call_result_api(E_CALL_RESULT_UNREGISTER);
			if(VOIP_APP_DEBUG(EVENT))
				zlog_debug(ZLOG_VOIP, "app call @'%s' Error, phone ‘%s’ is not register",
						voip_call.phone, voip_call.phone);
			return ERROR;
		}

		if(voip_sip_call_state_get_api() != VOIP_SIP_CALL_IDLE)
		{
			if(VOIP_APP_DEBUG(EVENT))
				zlog_debug(ZLOG_VOIP, "app call Error, phone ‘%s’ is talking ", voip_call.phone);
			x5b_app_call_result_api(E_CALL_RESULT_TALKLING);
			return ERROR;
		}
/*
		if(VOIP_APP_DEBUG(EVENT))
			zlog_debug(ZLOG_VOIP, "start call %s", voip_call.phone);
*/

		if(voip_sip_call_start(voip_call.phone) != OK)
		{
			x5b_app_call_result_api(E_CALL_RESULT_FAIL);
			if(VOIP_APP_DEBUG(EVENT))
				zlog_debug(ZLOG_VOIP, "app call Error, phone ‘%s’ ", voip_call.phone);

			return ERROR;
		}
		if(VOIP_APP_DEBUG(EVENT))
			zlog_debug(ZLOG_VOIP, "app calling %s", voip_call.phone);

		x5b_app_call_result_api(E_CALL_RESULT_CALLING);
		return OK;
	}
	else
	{
		return ERROR;
	}
	return ERROR;
}

#else

int voip_app_ev_start_call(event_node_t *ev)
{
	int ret = 0;
	strcpy(voip_call.phone, ev->data);
	//V_APP_DEBUG("-----------%s: room=0x%x", __func__, *num);
	if(VOIP_APP_DEBUG(EVENT))
		zlog_debug(ZLOG_VOIP, "start call '%s'", voip_call.phone);

	if(ret == OK)
	{
		//sip call
		if(VOIP_APP_DEBUG(EVENT))
			zlog_debug(ZLOG_VOIP, "start call %s", voip_call.phone);

/*		if(voip_sip_register_state_get_api() != VOIP_SIP_REGISTER_SUCCESS)
		{
			x5b_app_call_result_api(E_CALL_RESULT_UNREGISTER);
			zlog_debug(ZLOG_VOIP, "call error, phone ‘%s’ is not register ", voip_call.phone);
			return ERROR;
		}

		if(voip_sip_call_state_get_api() != VOIP_SIP_CALL_IDLE)
		{
			zlog_debug(ZLOG_VOIP, "call error, phone ‘%s’ is talking ", voip_call.phone);
			x5b_app_call_result_api(E_CALL_RESULT_ONLINE);
			return ERROR;
		}

		if(voip_sip_call_start(voip_call.phone) != OK)
		{
			zlog_debug(ZLOG_VOIP, "call error, phone ‘%s’ ", voip_call.phone);
			return ERROR;
		}*/
		x5b_app_call_result_api(E_CALL_RESULT_CALLING);

		if(os_read_string("/app/etc/remote-test.txt",stream_remote_test.r_rtp_address, sizeof(stream_remote_test.r_rtp_address)) == OK)
		{
			stream_remote_test.r_rtp_port = 5555;
			voip_stream->l_rtp_port = 5555;
			ret = voip_create_stream_and_start_api(&stream_remote_test);
		}
		return ret;
	}
	else
	{
		return ERROR;
	}
	return ERROR;
}
#endif


/*
 * register event handle
 */
int voip_app_ev_register(event_node_t *ev)
{
	//sip register
	BOOL *enable = (BOOL *)ev->data;
	if(voip_sip_register_start(enable) != OK)
		return ERROR;
	//voip_stream_stop_api();
	return OK;
}








#ifdef VOIP_APP_DEBUG
int voip_app_call_test(BOOL enable, char *address, int port, int lport)
{
	memset(&stream_remote_test, 0, sizeof(stream_remote_test));
	stream_remote_test.r_rtp_port = port ? port:5555;
	voip_stream->l_rtp_port = lport ? lport:5555;
	strcpy(stream_remote_test.r_rtp_address, address);
	if(enable)
		x5b_app_start_call(TRUE, NULL);
	else
		x5b_app_start_call(FALSE, NULL);
	return OK;
}
#endif


