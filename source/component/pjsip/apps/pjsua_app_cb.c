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
#include "pjsua_app_api.h"

#define THIS_FILE "pjmeida_file.c"



int pjapp_user_register_state_callback(pjapp_user_callback_tbl *cb, pjsua_call_id id, void *pVoid, pj_uint32_t state)
{
	if(cb && cb->pjsip_reg_state)
	{
		(cb->pjsip_reg_state)(id, pVoid, state);
	}
	return PJ_SUCCESS;
}

int pjapp_user_call_state_callback(pjapp_user_callback_tbl *cb, pjsua_call_id id, void *pVoid, pj_uint32_t state)
{
	if(cb && cb->pjsip_call_state)
	{
		(cb->pjsip_call_state)(id, pVoid, state);
	}
	return PJ_SUCCESS;
}

int pjapp_user_media_state_callback(pjapp_user_callback_tbl *cb, pjsua_call_id id, void *pVoid, pj_uint32_t state)
{
	if(cb && cb->pjsip_media_state)
	{
		(cb->pjsip_media_state)(id, pVoid, state);
	}
	return PJ_SUCCESS;
}

int pjapp_user_recv_tdmf_callback(pjapp_user_callback_tbl *cb, pjsua_call_id id, void *pVoid, pj_uint32_t state)
{
	if(cb && cb->pjsip_dtmf_recv)
	{
		(cb->pjsip_dtmf_recv)(id, pVoid, state);
	}
	return PJ_SUCCESS;
}

int pjapp_user_call_takeup_callback(pjapp_user_callback_tbl *cb, pjsua_call_id id, void *pVoid, pj_uint32_t state)
{
	if(cb && cb->pjsip_call_takeup)
	{
		(cb->pjsip_call_takeup)(id, pVoid, state);
	}
	return PJ_SUCCESS;
}

int pjapp_user_call_timeout_callback(pjapp_user_callback_tbl *cb, pjsua_call_id id, void *pVoid, pj_uint32_t state)
{
	if(cb && cb->pjsip_call_timeout)
	{
		(cb->pjsip_call_timeout)(id, pVoid, state);
	}
	return PJ_SUCCESS;
}

int pjapp_user_call_hangup_callback(pjapp_user_callback_tbl *cb, pjsua_call_id id, void *pVoid, pj_uint32_t state)
{
	if(cb && cb->pjsip_call_hangup)
	{
		(cb->pjsip_call_hangup)(id, pVoid, state);
	}
	return PJ_SUCCESS;
}

int pjapp_user_call_incoming_callback(pjapp_user_callback_tbl *cb, pjsua_call_id id, void *pVoid, pj_uint32_t state)
{
	if(cb && cb->pjsip_call_incoming)
	{
		(cb->pjsip_call_incoming)(id, pVoid, state);
	}
	return PJ_SUCCESS;
}

/***************************************************************************/
/***************************************************************************/
static int pjapp_url_get_id(char *url, char *id, char *ip, pj_uint16_t *port)
{
	char tmp[128];
	char *p = url, *brk = NULL;
	brk = strstr(p, ":");
	if(brk)
	{
		brk++;
		if(brk)
		{
			memset(tmp, 0, sizeof(tmp));
			sscanf(brk, "%[^@]", tmp);
			if(id)
				strcpy(id, tmp);
		}
	}
	brk = strstr(p, "@");
	if(brk)
	{
		brk++;
		if(brk)
		{
			memset(tmp, 0, sizeof(tmp));
			if(strstr(brk, ":"))
			{
				sscanf(brk, "%[^:]", tmp);
				if(ip)
					strcpy(ip, tmp);

				brk = strstr(brk, ":");
				brk++;
				if(brk)
				{
					if(port)
						*port = atoi(brk);
					return PJ_SUCCESS;
				}
			}
			else
			{
				if(ip)
					strcpy(ip, tmp);
				return PJ_SUCCESS;
			}
		}
	}
	return PJ_SUCCESS;
}

/***************************************************************************/
/***************************************************************************/
static int pjapp_account_state_callback(pjsua_acc_id id, void *p)
{
	pjsua_acc_info *info = p;
	pjapp_username_t *userinfo = NULL;
	pjsua_acc_config *acc = NULL;
	if(info->has_registration)
	{
		char username[64];
		char address[64];
		pj_uint16_t port;
		memset(username, 0, sizeof(username));
		memset(address, 0, sizeof(address));
		//sip:100@192.168.0.103:5060
		pjapp_url_get_id(info->acc_uri.ptr, username, address, &port);

		//printf("==============%s============(%s-%s-%d)\r\n", __func__, user_tmp.sip_user, srv_tmp.sip_address, srv_tmp.sip_port);
		//printf("==============%s============(%d:%d)(%s)\r\n", __func__, id, info->id, info->acc_uri.ptr);
		acc = pjapp_account_acc_lookup(username);
		if(acc)
			userinfo = acc->user_data;
		if(userinfo)
		{
			if(info->status == PJSIP_SC_OK)
				userinfo->sip_state = PJAPP_STATE_REGISTER_SUCCESS;
			else
				userinfo->sip_state = PJAPP_STATE_REGISTER_FAILED;
			userinfo->id = info->id;

			userinfo->is_default = info->is_default;
			userinfo->is_current = ((id == current_acc)? PJ_TRUE:PJ_FALSE);

			if(info->status != PJSIP_SC_OK)
				userinfo->register_svr.state = PJAPP_STATE_CONNECT_FAILED;
			else
			{
				userinfo->register_svr.state = PJAPP_STATE_CONNECT_SUCCESS;
				pjsua_acc_set_online_status(id, PJ_TRUE);
			}
		}
	}
	return PJ_SUCCESS;
}

/*
 * dtmf recv callback
 */
static int pjapp_recv_dtmf_callback(int id, void *p, int input)
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

static int pjapp_register_state_callback(int id, void *p, int input)
{
	pjsua_acc_info reginfo;
	memset(&reginfo, 0, sizeof(reginfo));
	if(pjsua_acc_get_info(id, &reginfo) == PJ_SUCCESS)
	{
		pjapp_account_state_callback(id, &reginfo);
	}
	return PJ_SUCCESS;
}
/**
 * @brief 
 * @param  id:               Parameter Description
 * @param  p:                Parameter Description
 * @param  input:            Parameter Description
 * @return : int 
 */
static int pjapp_call_state_callback(int id, void *p, int input)
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

/**
 * @brief 
 * @param  id                Param doc
 * @param  p                 Param doc
 * @param  input             Param doc
 * @return int 
 */
static int pjapp_call_incoming_callback(int id, void *p, int input)
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

/**
 * @brief 
 * @param  cb                Param doc
 * @return int 
 */
int pjapp_user_callback_init(pjapp_user_callback_tbl *cb)
{
	cb->pjsip_reg_state = pjapp_register_state_callback;
	cb->pjsip_call_state = pjapp_call_state_callback;
	cb->pjsip_call_incoming = pjapp_call_incoming_callback;
	//cb->pjsip_media_state = pjapp_recv_dtmf_callback;
	cb->pjsip_dtmf_recv = pjapp_recv_dtmf_callback;

	//cb->pjsip_call_takeup;
	//cb->pjsip_call_timeout;
	//cb->pjsip_call_hangup;

	cb->cli_account_state_get = pjapp_account_state_callback;
	return PJ_SUCCESS;
}