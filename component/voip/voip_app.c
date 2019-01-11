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


int voip_app_ev_stop_call(event_node_t *ev)
{
	if(voip_state_get() > VOIP_STATE_NONE)
	{
		//sip call
		if(voip_sip_call( NULL, voip_sip_config.sip_user,
				voip_sip_config.sip_password, SIP_CTL_TIMEOUT, FALSE) != OK)
			return ERROR;
		voip_stream_stop_api();
		return OK;
	}
	else
		return OK;
}


int voip_app_ev_start_call(event_node_t *ev)
{
	int ret = 0;
	u_int32 *num = (u_int32 *)ev->data;
	x5_b_room_position_t *room = ev->data;
	voip_position_room_t out;
	V_APP_DEBUG("-----------%s: room=0x%x", __func__, *num);
	if(VOIP_APP_DEBUG(EVENT))
		zlog_debug(ZLOG_VOIP, "start call '%s'", room->data);
	ret = voip_estate_mgt_get_phone_number(room, &out);
	if(ret == OK)
	{
		//sip call
		if(VOIP_APP_DEBUG(EVENT))
			zlog_debug(ZLOG_VOIP, "start call %s", room->data);

		if(voip_sip_call(out.phone, voip_sip_config.sip_user,
				voip_sip_config.sip_password, SIP_CTL_TIMEOUT, TRUE) != OK)
		{
			x5_b_a_call_result_api(x5_b_a_mgt, E_CALL_RESULT_UNREGISTER);
			return ERROR;
		}
		x5_b_a_call_result_api(x5_b_a_mgt, E_CALL_RESULT_CALLING);
		if(voip_sip_state_get_api() == VOIP_STATE_CALL_SUCCESS)
		{
			//V_APP_DEBUG("-----------%s:voip_create_stream_and_start", __func__);
#ifdef VOIP_APP_DEBUG
			if(voip_dbtest_isenable())
			{
				voip_dbtest_getremote(room->data, stream_remote_test.r_rtp_address, &stream_remote_test.r_rtp_port);
			}
			ret = voip_create_stream_and_start_api(&stream_remote_test);
#else
			voip_stream_remote_t remote;
			ret = voip_stream_remote_get_api(&remote);
			if(ret == OK)
				ret = voip_create_stream_and_start_api(&remote);
#endif
		}
		return ret;
	}
	else
	{
		return ERROR;
	}
	return ERROR;
}




#ifdef VOIP_APP_DEBUG
int voip_app_call_test(BOOL enable, char *address, int port, int lport)
{
	memset(&stream_remote_test, 0, sizeof(stream_remote_test));
	stream_remote_test.r_rtp_port = port ? port:5555;
	voip_stream->l_rtp_port = lport ? lport:5555;
	strcpy(stream_remote_test.r_rtp_address, address);
	if(enable)
		x5_b_start_call(TRUE, NULL);
	else
		x5_b_start_call(FALSE, NULL);
	return OK;
}
#endif

/*int voip_app_start_stream(char *room)
{

}*/
