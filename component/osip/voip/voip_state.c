/*
 * voip_state.c
 *
 *  Created on: Dec 31, 2018
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
#include "vty.h"



#include "voip_def.h"
#include "voip_state.h"
#include "voip_app.h"
#include "voip_osip.h"
#include "voip_stream.h"

#include "application.h"

//#include "voip_app.h"


osip_call_error_t osip_call_error_get(void *priv)
{
	voip_osip_t *osip = (voip_osip_t *)priv;
	if(osip == NULL)
		return SIP_STATE_CALL_IDLE;
	return osip->call_error;
}

int osip_call_error_set(void *priv, osip_call_error_t call_error)
{
	voip_osip_t *osip = (voip_osip_t *)priv;
	zassert(osip != NULL);
	if(call_error < SIP_STATE_100)
		return ERROR;
	osip->call_error = call_error;
	if(SIP_STATE_400 < call_error && call_error < SIP_STATE_606)
	{
		if(osip->callparam && osip->callparam->source == APP_CALL_ID_UI)
		{
			x5b_app_call_result_api(NULL, E_CALL_RESULT_FAIL, 0, E_CMD_TO_AUTO);
		}
	}
	return OK;
}


osip_call_state_t osip_call_state_get(void *priv)
{
	voip_osip_t *osip = (voip_osip_t *)priv;
	if(osip == NULL)
		return SIP_STATE_CALL_IDLE;
	return osip->call_state;
}

int osip_call_state_set(void *priv, osip_call_state_t state)
{
	voip_osip_t *osip = (voip_osip_t *)priv;
	zassert(osip != NULL);
	if(state < SIP_STATE_CALL_IDLE && state > SIP_STATE_CALL_RELEASED)
		return ERROR;
	osip->call_state = state;
	return OK;
}

osip_state_t osip_register_state_get(void *priv)
{
	voip_osip_t *osip = (voip_osip_t *)priv;
	if(osip == NULL)
		return SIP_STATE_NONE;
	return osip->register_state;
}

int osip_register_state_set(void *priv, osip_state_t state)
{
	voip_osip_t *osip = (voip_osip_t *)priv;
	if(osip == NULL)
		return ERROR;
	osip->register_state = state;
	voip_status_register_api(state);
	return OK;
}


media_state_t voip_media_state_get(void *priv)
{
	voip_stream_t *media = (voip_stream_t *)priv;
	if(media == NULL)
		return SIP_STATE_MEDIA_IDLE;
	return media->state;
}

int voip_media_state_set(void *priv, media_state_t state)
{
	voip_stream_t *media = (voip_stream_t *)priv;
	if(media == NULL)
		return ERROR;
	if(state != SIP_STATE_MEDIA_IDLE && state != SIP_STATE_MEDIA_CONNECTED)
		return ERROR;
	media->state = state;
	return OK;
}




