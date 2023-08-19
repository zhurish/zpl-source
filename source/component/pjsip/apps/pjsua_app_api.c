/*
 * pjsip_app_api.c
 *
 *  Created on: Jun 15, 2019
 *      Author: zhurish
 */

#include <pjsua-lib/pjsua.h>
#include "pjsua_app_config.h"
#include "pjsua_app_cb.h"
#include "pjsua_app_common.h"
#include "pjsua_app_cfgapi.h"

#include "pjsua_app.h"
#include "pjsua_app_api.h"


/***************************************************************************************/
/***************************************************************************************/
char *pjapp_cfg_dtmf_name(pjsip_dtmf_t dtmf)
{
	switch(dtmf)
	{
	case PJSIP_DTMF_INFO:
		return "SIP-INFO";
		break;
	case PJSIP_DTMF_RFC2833:
		return "RFC2833";
		break;
	case PJSIP_DTMF_INBAND:
		return "INBAND";
		break;
	default:
		return "UNKNOWN";
		break;
	}
}

char *pjapp_cfg_transport_name(pjsip_transport_t dtmf)
{
	switch(dtmf)
	{
	case PJSIP_PROTO_UDP:
		return "UDP";
		break;
	case PJSIP_PROTO_TCP:
		return "TCP";
		break;
	case PJSIP_PROTO_TLS:
		return "TLS";
		break;
	case PJSIP_PROTO_DTLS:
		return "DTLS";
		break;
	default:
		return "UNKNOWN";
		break;
	}
}

char *pjapp_cfg_reg_proxy_name(pjsip_reg_proxy_t dtmf)
{
	switch(dtmf)
	{
	case PJSIP_REGISTER_NONE:
		return "NONE";
		break;
	case PJSIP_REGISTER_NO_PROXY:
		return "NO-PROXY";
		break;
	case PJSIP_REGISTER_OUTBOUND_PROXY:
		return "OUTBOUND-PROXY";
		break;
	case PJSIP_REGISTER_ACC_ONLY:
		return "ACC-ONLY";
		break;
	case PJSIP_REGISTER_ALL:
		return "ALL";
		break;
	default:
		return "UNKNOWN";
		break;
	}
}

char *pjapp_cfg_srtp_name(pjsip_srtp_t dtmf)
{
	switch(dtmf)
	{
	case PJSIP_SRTP_DISABLE:
		return "DISABLE";
		break;
	case PJSIP_SRTP_OPTIONAL:
		return "OPTIONAL";
		break;
	case PJSIP_SRTP_MANDATORY:
		return "MANDATORY";
		break;
	case PJSIP_SRTP_OPTIONAL_DUP:
		return "OPTIONAL-DUP";
		break;
	default:
		return "UNKNOWN";
		break;
	}
}

char *pjapp_cfg_srtp_sec_name(pjsip_srtp_sec_t dtmf)
{
	switch(dtmf)
	{
	case PJSIP_SRTP_SEC_NO:
		return "NO";
		break;
	case PJSIP_SRTP_SEC_TLS:
		return "SEC-TLS";
		break;
	case PJSIP_SRTP_SEC_SIPS:
		return "SEC-SIPS";
		break;
	default:
		return "UNKNOWN";
		break;
	}
}

char *pjapp_cfg_timer_name(pjsip_timer_t dtmf)
{
	switch(dtmf)
	{
	case PJSIP_TIMER_INACTIVE:
		return "INACTIVE";
		break;
	case PJSIP_TIMER_OPTIONAL:
		return "OPTIONAL";
		break;
	case PJSIP_TIMER_MANDATORY:
		return "MANDATORY";
		break;
	case PJSIP_TIMER_ALWAYS:
		return "ALWAYS";
		break;
	default:
		return "UNKNOWN";
		break;
	}
}

char *pjapp_cfg_echo_mode_name(pjsip_echo_mode_t dtmf)
{
	switch(dtmf)
	{
	case PJSIP_ECHO_DEFAULT:
		return "DEFAULT";
		break;
	case PJSIP_ECHO_SPEEX:
		return "SPEEX";
		break;
	case PJSIP_ECHO_SUPPRESSER:
		return "SUPPRESSER";
		break;
	case PJSIP_ECHO_WEBRTXC:
		return "WEBRTXC";
		break;
	default:
		return "UNKNOWN";
		break;
	}
}

char *pjapp_cfg_accept_redirect_name(pjsip_accept_redirect_t dtmf)
{
	switch(dtmf)
	{
	case PJSIP_ACCEPT_REDIRECT_REJECT:
		return "REDIRECT-REJECT";
		break;
	case PJSIP_ACCEPT_REDIRECT_FOLLOW:
		return "REDIRECT-FOLLOW";
		break;
	case PJSIP_ACCEPT_REDIRECT_FOLLOW_REPLACE:
		return "REDIRECT-FOLLOW-REPLACE";
		break;
	case PJSIP_ACCEPT_REDIRECT_ASK:
		return "REDIRECT-ASK";
		break;
	default:
		return "UNKNOWN";
		break;
	}
}

char *pjapp_cfg_register_state_name(pjapp_register_state_t dtmf)
{
	switch(dtmf)
	{
	case PJAPP_STATE_UNREGISTER:
		return "UNREGISTER";
		break;
	case PJAPP_STATE_REGISTER_FAILED:
		return "REGISTER-FAILED";
		break;
	case PJAPP_STATE_REGISTER_SUCCESS:
		return "REGISTER-SUCCESS";
		break;
	default:
		return "UNKNOWN";
		break;
	}
}

char *pjapp_cfg_connect_state_name(pjapp_connect_state_t dtmf)
{
	switch(dtmf)
	{
	case PJAPP_STATE_UNCONNECT:
		return "UNCONNECT";
		break;
	case PJAPP_STATE_CONNECT_FAILED:
		return "CONNECT-FAILED";
		break;
	case PJAPP_STATE_CONNECT_SUCCESS:
		return "CONNECT-SUCCESS";
		break;
	default:
		return "UNKNOWN";
		break;
	}
}

char *pjapp_cfg_call_state_name(pjapp_call_state_t dtmf)
{
	switch(dtmf)
	{
	case PJAPP_STATE_CALL_IDLE:
		return "IDLE";
		break;
	case PJAPP_STATE_CALL_TRYING:
		return "TRYING";
		break;
	case PJAPP_STATE_CALL_RINGING:
		return "RINGING";
		break;
	case PJAPP_STATE_CALL_PICKING:
		return "PICKING";
		break;
	case PJAPP_STATE_CALL_FAILED:
		return "FAILED";
		break;
	case PJAPP_STATE_CALL_SUCCESS:
		return "SUCCESS";
		break;
	case PJAPP_STATE_CALL_CANCELLED:
		return "CANCELLED";
		break;
	case PJAPP_STATE_CALL_CLOSED:
		return "CLOSED";
		break;
	case PJAPP_STATE_CALL_RELEASED:
		return "RELEASED";
		break;
	default:
		return "UNKNOWN";
		break;
	}
}

/***************************************************************************************/
/***************************************************************************/
/***************************************************************************/

/***************************************************************************/

/***************************************************************************/
/***************************************************************************/
int pjapp_account_add_api(char *sip_url, char *sip_srv, char *realm,
		char *user, char *pass, pjsua_acc_id *accid)
{
    pjsua_acc_config acc_cfg;
    pj_status_t status;
	char cmd[128], tmp[128];
	memset(cmd, 0, sizeof(cmd));
	memset(tmp, 0, sizeof(tmp));
    pjsua_acc_config_default(&acc_cfg);
	snprintf(cmd, sizeof(cmd), "sip:%s@%s", user, sip_url);
	snprintf(tmp, sizeof(tmp), "sip:%s", sip_srv);
    acc_cfg.id = pj_str(cmd);
    acc_cfg.reg_uri = pj_str(tmp);
    acc_cfg.cred_count = 1;
    acc_cfg.cred_info[0].scheme = pj_str("Digest");
    acc_cfg.cred_info[0].realm = pj_str(realm);
    acc_cfg.cred_info[0].username = pj_str(user);
    acc_cfg.cred_info[0].data_type = 0;
    acc_cfg.cred_info[0].data = pj_str(pass);

    acc_cfg.rtp_cfg = _pjAppCfg.rtp_cfg;
    pjapp_config_video_init(&acc_cfg);

    status = pjsua_acc_add(&acc_cfg, PJ_TRUE, NULL);
    if (status != PJ_SUCCESS) {
    	return -1;
    }
    return PJ_SUCCESS;
}

int pjapp_account_del_api(pjsua_acc_id accid)
{
    pj_status_t status;
    if (!pjsua_acc_is_valid(accid))
    {
    	return -1;
    }
    status = pjsua_acc_del(accid);
    if (status != PJ_SUCCESS) {
    	return -1;
    }
	return -1;
}

int pjapp_account_mod_api(pjsua_acc_id accid, char *sip_url, char *sip_srv, char *realm,
		char *user, char *pass)
{
	return -1;
}

int pjapp_account_default_api(pjsua_acc_id accid, pj_uint32_t type)
{
    pj_status_t status;
    if (pjsua_acc_is_valid(accid))
    {
    	status = pjsua_acc_set_default(accid);
        if (status != PJ_SUCCESS) {
        	return -1;
        }
        return PJ_SUCCESS;
    }
    return -1;
}

int pjapp_account_register_api(pj_bool_t reg)
{
	if(pjsua_acc_is_valid(current_acc))
		return pjsua_acc_set_registration(current_acc, reg);
	return -1;
}

int pjapp_account_show_api(pjsua_acc_id accid)
{
	pjsua_acc_id acc_ids[16];
	pj_uint32_t count = PJ_ARRAY_SIZE(acc_ids);
	pj_uint32_t i;
	//static const pj_str_t header = { "Account list:\n", 15 };
	pjsua_enum_accs (acc_ids, &count);

	for (i = 0; i < (int) count; ++i)
	{
		char acc_info[80];
		char out_str[160];
		pjsua_acc_info info;

		pjsua_acc_get_info (acc_ids[i], &info);

		if (_pjAppCfg.cbtbl.cli_account_state_get)
			(_pjAppCfg.cbtbl.cli_account_state_get) (acc_ids[i], &info);

		if (!info.has_registration)
		{
			pj_ansi_snprintf (acc_info, sizeof(acc_info), "%.*s",
							  (int) info.status_text.slen,
							  info.status_text.ptr);

		}
		else
		{
			pj_ansi_snprintf (acc_info, sizeof(acc_info),
							  "%d/%.*s (expires=%d)", info.status,
							  (int) info.status_text.slen, info.status_text.ptr,
							  info.expires);

		}

		pj_ansi_snprintf (out_str, sizeof(out_str), " %c[%2d] %.*s: %s\n",
						  (acc_ids[i] == current_acc ? '*' : ' '), acc_ids[i],
						  (int) info.acc_uri.slen, info.acc_uri.ptr, acc_info);
		//pj_cli_sess_write_msg (cval->sess, out_str, pj_ansi_strlen (out_str));

		pj_bzero (out_str, sizeof(out_str));
		pj_ansi_snprintf (out_str, sizeof(out_str),
						  "       Online status: %.*s\n",
						  (int) info.online_status_text.slen,
						  info.online_status_text.ptr);

		//pj_cli_sess_write_msg (cval->sess, out_str, pj_ansi_strlen (out_str));
	}
	return PJ_SUCCESS;
}


int pjapp_user_start_call_api(pjsua_acc_id accid, char *num, pjsua_call_id *callid)
{

	pj_str_t call_uri_arg;
	char cmd[512];
	memset(cmd, '\0', sizeof(cmd));
#if 0
	if(_pjAppCfg->sip_user.sip_state == PJAPP_STATE_REGISTER_SUCCESS &&
			_pjAppCfg->sip_user.register_svr &&
			strlen(_pjAppCfg->sip_user.register_svr->sip_address))
	{
		if(_pjAppCfg->sip_user.register_svr->sip_port == PJSIP_PORT_DEFAULT)
			snprintf(cmd, sizeof(cmd), "sip:%s@%s",
					num, _pjAppCfg->sip_user.register_svr->sip_address);
		else
			snprintf(cmd, sizeof(cmd), "sip:%s@%s:%d",
					num,
					_pjAppCfg->sip_user.register_svr->sip_address,
					_pjAppCfg->sip_user.register_svr->sip_port);
	}
	else if(_pjAppCfg->sip_user_sec.sip_state == PJAPP_STATE_REGISTER_SUCCESS &&
			_pjAppCfg->sip_user_sec.register_svr &&
			strlen(_pjAppCfg->sip_user_sec.register_svr->sip_address))
	{
		if(_pjAppCfg->sip_user_sec.register_svr->sip_port == PJSIP_PORT_DEFAULT)
			snprintf(cmd, sizeof(cmd), "sip:%s@%s",
					num, _pjAppCfg->sip_user_sec.register_svr->sip_address);
		else
			snprintf(cmd, sizeof(cmd), "sip:%s@%s:%d",
					num,
					_pjAppCfg->sip_user_sec.register_svr->sip_address,
					_pjAppCfg->sip_user_sec.register_svr->sip_port);
	}
	else
	{
		return -1;
	}
#endif
	//if(_pjAppCfg.current_call != PJSUA_INVALID_ID)
	//	return -1;
	//zlog_debug(MODULE_VOIP, "========%s->voip_volume_control_api", __func__);
	//voip_volume_control_api(zpl_true);
	//zlog_debug(MODULE_VOIP, "========%s-> enter pjapp_cfg_app_start_call", __func__);
	snprintf(cmd, sizeof(cmd), "sip:%s@%s:%d",
					num,
					"192.168.10.102",
					5060);
	//char *pj_call_str = (char *)(cmd + 9);
	call_uri_arg = pj_str(cmd);
	//call_uri_arg = pj_str("sip:1003@192.168.3.254");
/*	pjsua_call_setting_default(&_pjAppCfg._pjAppCfg.call_opt);
	_pjAppCfg.app_cfg.app_cfg.call_opt.aud_cnt = _pjAppCfg.app_cfg.app_cfg.aud_cnt;
	_pjAppCfg.app_cfg.app_cfg.call_opt.vid_cnt = _pjAppCfg.app_cfg.app_cfg.vid.vid_cnt;*/
	if(pjsua_call_make_call(current_acc/*current_acc*/, &call_uri_arg,
				&_pjAppCfg.call_opt, NULL, NULL, &_pjAppCfg.current_call) == PJ_SUCCESS)
	{
		if(callid)
			*callid = _pjAppCfg.current_call;
		//zlog_debug(MODULE_VOIP, "========%s-> level pjapp_cfg_app_start_call", __func__);
		return PJ_SUCCESS;
	}
/*
 * handle SIGUSR2 nostop noprint
*/
	//zlog_debug(MODULE_VOIP, "========%s-> level pjapp_cfg_app_start_call", __func__);
	//voip_volume_control_api(zpl_false);
	return -1;
}

int pjapp_user_stop_call_api(pjsua_call_id callid, pj_bool_t all)
{
    if (_pjAppCfg.current_call == PJSUA_INVALID_ID)
    {
    	return -1;
    }
    else
    {
    	int ret = 0;
		if (all)
		{
			pjsua_call_hangup_all();
			//voip_volume_control_api(zpl_false);
			return PJ_SUCCESS;
		}
		else
		{
			if(callid == PJSUA_INVALID_ID)
				ret = pjsua_call_hangup(callid, 0, NULL, NULL);
			else
				ret = pjsua_call_hangup(_pjAppCfg.current_call, 0, NULL, NULL);
			if(ret == PJ_SUCCESS)
			{
				//voip_volume_control_api(zpl_false);
				return PJ_SUCCESS;
			}
		}
    }
	//voip_volume_control_api(zpl_false);
	return -1;
}
/* Make multi call */
#if 0
int pjapp_cfg_app_start_multi_call(pjsua_acc_id accid, char *num, int *callid)
//static pj_status_t cmd_make_multi_call(pj_cli_cmd_val *cval)
{
	struct pjapp_input_result result;
	char dest[64] = { 0 };
	char out_str[128];
	int i, count;
	pj_str_t tmp = pj_str(dest);

	pj_ansi_snprintf(out_str, sizeof(out_str),
			"(You currently have %d calls)\n", pjsua_call_get_count());

	count = 3;								//(int)pj_strtol(&cval->argv[1]);
	if (count < 1)
		return PJ_SUCCESS;

	pj_strncpy_with_null(&tmp, &cval->argv[2], sizeof(dest));

	/* input destination. */
	get_input_url(tmp.ptr, tmp.slen, cval, &result);
	if (result.nb_result != PJAPP_NO_NB)
	{
		pjsua_buddy_info binfo;
		if (result.nb_result == -1 || result.nb_result == 0)
		{
			/*	    static const pj_str_t err_msg =
			 {"You can't do that with make call!\n", 35};
			 pj_cli_sess_write_msg(cval->sess, err_msg.ptr, err_msg.slen);*/
			return PJ_SUCCESS;
		}
		pjsua_buddy_get_info(result.nb_result - 1, &binfo);
		pj_strncpy(&tmp, &binfo.uri, sizeof(dest));
	}
	else
	{
		tmp = pj_str(result.uri_result);
	}

	for (i = 0; i < count; ++i)
	{
		pj_status_t status;
		status = pjsua_call_make_call(current_acc, &tmp, &_pjAppCfg.app_cfg.app_cfg.call_opt,
				NULL, NULL, NULL);
		if (status != PJ_SUCCESS)
			break;
	}
	return PJ_SUCCESS;
}
#endif
/***************************************************************************/
int pjapp_user_answer_call_api(pjsua_call_id callid, pj_uint32_t st_code)
{
	pjsua_call_info call_info;
	if ((st_code < 100) || (st_code > 699))
		return -1;
	if (_pjAppCfg.current_call != PJSUA_INVALID_ID)
	{
		pjsua_call_get_info(_pjAppCfg.current_call, &call_info);
	}
	else
	{
		/* Make compiler happy */
		call_info.role = PJSIP_ROLE_UAC;
		call_info.state = PJSIP_INV_STATE_DISCONNECTED;
	}

	if (_pjAppCfg.current_call == PJSUA_INVALID_ID
			|| call_info.role != PJSIP_ROLE_UAS
			|| call_info.state >= PJSIP_INV_STATE_CONNECTING)
	{
		return -1;
	}
	else
	{
		char contact[120];
		pj_str_t hname =
		{ "Contact", 7 };
		pj_str_t hvalue;
		pjsip_generic_string_hdr hcontact;

		pjsua_msg_data_init(&_pjAppCfg.msg_data);

		if (st_code / 100 == 3)
		{
			/*			 if (cval->argc < 3)
			 {
			 static const pj_str_t err_msg = {"Enter URL to be put in Contact\n",  32};
			 return PJ_SUCCESS;
			 }*/

			hvalue = pj_str(contact);
			pjsip_generic_string_hdr_init2(&hcontact, &hname, &hvalue);

			pj_list_push_back(&_pjAppCfg.msg_data.hdr_list, &hcontact);
		}

		/*
		 * Must check again!
		 * Call may have been disconnected while we're waiting for
		 * keyboard input.
		 */
		if (_pjAppCfg.current_call == PJSUA_INVALID_ID)
		{
			//static const pj_str_t err_msg =
			//		{ "Call has been disconnected\n", 28 };
			//pj_cli_sess_write_msg(cval->sess, err_msg.ptr, err_msg.slen);
			return -1;
		}

		if (pjsua_call_answer2(_pjAppCfg.current_call, &_pjAppCfg.call_opt,
				st_code, NULL, &_pjAppCfg.msg_data) == PJ_SUCCESS)
			return PJ_SUCCESS;
	}
	return -1;
}

int pjapp_user_hold_call_api(pjsua_call_id callid)
{
    if (callid != PJSUA_INVALID_ID)
    {
    	if(pjsua_call_set_hold(callid, NULL) == PJ_SUCCESS)
    		return PJ_SUCCESS;
    }
    else
    {
    	//PJ_LOG(3,(THIS_FILE, "No current call"));
    	return -1;
    }
    return PJ_SUCCESS;
}

int pjapp_user_reinvite_call_api(pjsua_call_id callid)
{
	if (callid != PJSUA_INVALID_ID)
	{
		/*
		 * re-INVITE
		 */
		_pjAppCfg.call_opt.flag |= PJSUA_CALL_UNHOLD;
		if(pjsua_call_reinvite2 (callid, &_pjAppCfg.call_opt, NULL) == PJ_SUCCESS)
			return PJ_SUCCESS;
	}
	else
	{
		//PJ_LOG(3,(THIS_FILE, "No current call"));
		return -1;
	}
	return PJ_SUCCESS;
}

int pjapp_user_send_dtmf_api(pjsua_call_id callid, pj_uint32_t type, pj_uint32_t code)
{
	if (_pjAppCfg.current_call == PJSUA_INVALID_ID)
	{
		return -1;
	}
	if (type == 1)
	{
		char body[64];
		pj_uint32_t call = _pjAppCfg.current_call;
		pj_status_t status;
		pj_str_t dtmf_digi = pj_str("INFO");
		memset(body, 0, sizeof(body));

		pj_ansi_snprintf(body, sizeof(body), "%c",code);

		dtmf_digi = pj_str(body);

		if (!pjsua_call_has_media(_pjAppCfg.current_call))
		{
			//PJ_LOG(3, (THIS_FILE, "Media is not established yet!"));
			return -1;
		}
		if (call != _pjAppCfg.current_call)
		{
			//static const pj_str_t err_msg =
			//		{ "Call has been disconnected\n", 28 };
			//pj_cli_sess_write_msg(cval->sess, err_msg.ptr, err_msg.slen);
			return -1;
		}

		status = pjsua_call_dial_dtmf(_pjAppCfg.current_call, &dtmf_digi);
		if (status != PJ_SUCCESS)
		{
			// pjsua_perror(THIS_FILE, "Unable to send DTMF", status);
			return -1;
		}
		return PJ_SUCCESS;
	}
	else
	{
		char body[64];
		const pj_str_t SIP_INFO = pj_str("INFO");
		pj_uint32_t call = _pjAppCfg.current_call;
		pj_status_t status;

		if (call != _pjAppCfg.current_call)
		{
			return -1;
		}

		pjsua_msg_data_init(&_pjAppCfg.msg_data);
		_pjAppCfg.msg_data.content_type = pj_str("application/dtmf-relay");

		pj_ansi_snprintf(body, sizeof(body), "Signal=%c\n"
				"Duration=160", code);

		_pjAppCfg.msg_data.msg_body = pj_str(body);

		status = pjsua_call_send_request(_pjAppCfg.current_call, &SIP_INFO,
				&_pjAppCfg.msg_data);
		if (status != PJ_SUCCESS)
		{
			return -1;
		}
		return PJ_SUCCESS;
	}
	return PJ_SUCCESS;
}

int pjapp_user_select_call_api(pjsua_call_id callid, pj_uint32_t type)
{
	/*
	 * Cycle next/prev dialog.
	 */
	if (type == 1)
	{
		pjapp_find_next_call ();
	}
	else
	{
		pjapp_find_prev_call ();
	}

	if (_pjAppCfg.current_call != PJSUA_INVALID_ID)
	{
		pjsua_call_info call_info;

		if(pjsua_call_get_info (_pjAppCfg.current_call, &call_info) == PJ_SUCCESS)
			return PJ_SUCCESS;
	}
	else
	{
		return -1;
	}
	return PJ_SUCCESS;
}
/***************************************************************************/
/***************************************************************************/

/***************************************************************************/

