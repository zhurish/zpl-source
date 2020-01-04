/*
 * pjsua_app_cb.c
 *
 *  Created on: Jun 22, 2019
 *      Author: zhurish
 */

#include "pjsua_app_common.h"
#include "pjsua_app_cb.h"


int pjsip_app_callback_init(void *p, pjsip_callback_tbl *cb)
{
	pjsua_app_config *app = p;
	memcpy(&app->cbtbl, cb, sizeof(pjsip_callback_tbl));
	return PJ_SUCCESS;
}


int pjsip_app_register_state_callback(pjsip_callback_tbl *cb, int id, void *pVoid, int state)
{
	if(cb && cb->pjsip_reg_state)
	{
		(cb->pjsip_reg_state)(id, pVoid, state);
	}
	return PJ_SUCCESS;
}

int pjsip_app_call_state_callback(pjsip_callback_tbl *cb, int id, void *pVoid, int state)
{
	if(cb && cb->pjsip_call_state)
	{
		(cb->pjsip_call_state)(id, pVoid, state);
	}
	return PJ_SUCCESS;
}

int pjsip_app_media_state_callback(pjsip_callback_tbl *cb, int id, void *pVoid, int state)
{
	if(cb && cb->pjsip_media_state)
	{
		(cb->pjsip_media_state)(id, pVoid, state);
	}
	return PJ_SUCCESS;
}

int pjsip_app_dtmf_recv_callback(pjsip_callback_tbl *cb, int id, void *pVoid, int state)
{
	if(cb && cb->pjsip_dtmf_recv)
	{
		(cb->pjsip_dtmf_recv)(id, pVoid, state);
	}
	return PJ_SUCCESS;
}

int pjsip_app_call_takeup_callback(pjsip_callback_tbl *cb, int id, void *pVoid, int state)
{
	if(cb && cb->pjsip_call_takeup)
	{
		(cb->pjsip_call_takeup)(id, pVoid, state);
	}
	return PJ_SUCCESS;
}

int pjsip_app_call_timeout_callback(pjsip_callback_tbl *cb, int id, void *pVoid, int state)
{
	if(cb && cb->pjsip_call_timeout)
	{
		(cb->pjsip_call_timeout)(id, pVoid, state);
	}
	return PJ_SUCCESS;
}

int pjsip_app_call_hangup_callback(pjsip_callback_tbl *cb, int id, void *pVoid, int state)
{
	if(cb && cb->pjsip_call_hangup)
	{
		(cb->pjsip_call_hangup)(id, pVoid, state);
	}
	return PJ_SUCCESS;
}

int pjsip_app_call_incoming_callback(pjsip_callback_tbl *cb, int id, void *pVoid, int state)
{
	if(cb && cb->pjsip_call_incoming)
	{
		(cb->pjsip_call_incoming)(id, pVoid, state);
	}
	return PJ_SUCCESS;
}
