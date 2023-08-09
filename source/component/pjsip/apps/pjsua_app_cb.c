/*
 * pjsua_app_cb.c
 *
 *  Created on: Jun 22, 2019
 *      Author: zhurish
 */
#include <pjsua-lib/pjsua.h>
#include "pjsua_app_config.h"
#include "pjsua_app_cb.h"
#include "pjsua_app_common.h"
#include "pjsua_app_cfgapi.h"
#include "pjsip_app_api.h"

#define THIS_FILE "pjmeida_file.c"

int pjsip_app_callback_init(void *p, pjsip_callback_tbl *cb)
{
	//pjapp_config_t *app = p;
	//memcpy(&app->cbtbl, cb, sizeof(pjsip_callback_tbl));
	return PJ_SUCCESS;
}


int pjsip_app_register_state_callback(pjsip_callback_tbl *cb, pjsua_call_id id, void *pVoid, pj_uint32_t state)
{
	if(cb && cb->pjsip_reg_state)
	{
		(cb->pjsip_reg_state)(id, pVoid, state);
	}
	return PJ_SUCCESS;
}

int pjsip_app_call_state_callback(pjsip_callback_tbl *cb, pjsua_call_id id, void *pVoid, pj_uint32_t state)
{
	if(cb && cb->pjsip_call_state)
	{
		(cb->pjsip_call_state)(id, pVoid, state);
	}
	return PJ_SUCCESS;
}

int pjsip_app_media_state_callback(pjsip_callback_tbl *cb, pjsua_call_id id, void *pVoid, pj_uint32_t state)
{
	if(cb && cb->pjsip_media_state)
	{
		(cb->pjsip_media_state)(id, pVoid, state);
	}
	return PJ_SUCCESS;
}

int pjsip_app_dtmf_recv_callback(pjsip_callback_tbl *cb, pjsua_call_id id, void *pVoid, pj_uint32_t state)
{
	if(cb && cb->pjsip_dtmf_recv)
	{
		(cb->pjsip_dtmf_recv)(id, pVoid, state);
	}
	return PJ_SUCCESS;
}

int pjsip_app_call_takeup_callback(pjsip_callback_tbl *cb, pjsua_call_id id, void *pVoid, pj_uint32_t state)
{
	if(cb && cb->pjsip_call_takeup)
	{
		(cb->pjsip_call_takeup)(id, pVoid, state);
	}
	return PJ_SUCCESS;
}

int pjsip_app_call_timeout_callback(pjsip_callback_tbl *cb, pjsua_call_id id, void *pVoid, pj_uint32_t state)
{
	if(cb && cb->pjsip_call_timeout)
	{
		(cb->pjsip_call_timeout)(id, pVoid, state);
	}
	return PJ_SUCCESS;
}

int pjsip_app_call_hangup_callback(pjsip_callback_tbl *cb, pjsua_call_id id, void *pVoid, pj_uint32_t state)
{
	if(cb && cb->pjsip_call_hangup)
	{
		(cb->pjsip_call_hangup)(id, pVoid, state);
	}
	return PJ_SUCCESS;
}

int pjsip_app_call_incoming_callback(pjsip_callback_tbl *cb, pjsua_call_id id, void *pVoid, pj_uint32_t state)
{
	if(cb && cb->pjsip_call_incoming)
	{
		(cb->pjsip_call_incoming)(id, pVoid, state);
	}
	return PJ_SUCCESS;
}


/*
 * dtmf recv callback
 */
static int voip_app_dtmf_recv_callback(int id, void *p, int input)
{
	if (input == '#')
	{
		if(pjapp_incoming_call())
		{
			PJ_LOG(1, (THIS_FILE, "HUIFU/NONE Product recv dtmf:%c and open door", input));
			return PJ_SUCCESS;
		}
	}
	else
	{
		PJ_LOG(1, (THIS_FILE, "module recv dtmf:%c", input));
		return -1;
	}
	//PJ_LOG(1, (THIS_FILE, "module recv dtmf:%c", input);
	return PJ_SUCCESS;
}

static int voip_app_register_state_callback(int id, void *p, int input)
{
	pjsua_acc_info reginfo;
	memset(&reginfo, 0, sizeof(reginfo));
	if(pjsua_acc_get_info(id, &reginfo) == PJ_SUCCESS)
	{
		pjapp_cfg_account_set_api(id, &reginfo);
	}
	return PJ_SUCCESS;
}

static int voip_app_call_state_callback(int id, void *p, int input)
{
	//PJ_LOG(1, (THIS_FILE, "call state -> :%d", input);
	if(input == PJSIP_INV_STATE_NULL)
	{
	}
	else if(input == PJSIP_INV_STATE_CALLING)
	{
	}
	else if(input == PJSIP_INV_STATE_INCOMING)
	{
		//V_APP_DEBUG("==============%s: os_time_destroy for INCOMING===========", __func__);
	}
	else if(input == PJSIP_INV_STATE_EARLY)
	{
	}
	else if(input == PJSIP_INV_STATE_CONNECTING)
	{
	}
	else if(input == PJSIP_INV_STATE_CONFIRMED)
	{
	}
	else if(input == PJSIP_INV_STATE_DISCONNECTED)
	{
	}
	return PJ_SUCCESS;
}


static int voip_app_call_incoming_callback(int id, void *p, int input)
{
	if (pjapp_incoming_call())
	{
		if (pjapp_current_call() == id)
		{
			pjsua_call_info *call_info = p;
			if (input == 0)
			{

				PJ_LOG(1, (THIS_FILE,
						   " Incoming call for!\r\n"
						   "   Media count: %d audio & %d video\r\n"
						   "   From: %.*s\r\n"
						   "   To: %.*s\r\n"
						   "   Contact: %.*s\r\n",
						   call_info->rem_aud_cnt, call_info->rem_vid_cnt,
						   (int)call_info->remote_info.slen,
						   call_info->remote_info.ptr,
						   (int)call_info->local_info.slen,
						   call_info->local_info.ptr,
						   (int)call_info->remote_contact.slen,
						   call_info->remote_contact.ptr));
			}
		}
	}
	return PJ_SUCCESS;
}


int pjsip_callback_init(void)
{
	pjsip_callback_tbl cb;
	cb.pjsip_dtmf_recv = voip_app_dtmf_recv_callback;
	cb.pjsip_call_state = voip_app_call_state_callback;
	cb.pjsip_reg_state = voip_app_register_state_callback;
	cb.pjsip_reg_state = voip_app_register_state_callback;
	cb.cli_account_state_get = pjapp_cfg_account_set_api;
	cb.pjsip_call_incoming = voip_app_call_incoming_callback;
	//pjsip_app_callback_init(&_pjAppCfg.app_cfg, &cb);
	return PJ_SUCCESS;
}