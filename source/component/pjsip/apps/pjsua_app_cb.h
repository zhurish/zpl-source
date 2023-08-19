/*
 * pjsua_app_cb.h
 *
 *  Created on: Jun 22, 2019
 *      Author: zhurish
 */

#ifndef __PJSIP_PJAPP_CB_H__
#define __PJSIP_PJAPP_CB_H__

#ifdef __cplusplus
extern "C" {
#endif


typedef int (*pjsip_register_state_cb)(pjsua_call_id, void *, pj_uint32_t);
typedef int (*pjsip_call_state_cb)(pjsua_call_id, void *, pj_uint32_t);
typedef int (*pjsip_media_state_cb)(pjsua_call_id, void *, pj_uint32_t);
typedef int (*pjsip_dtmf_recv_cb)(pjsua_call_id, void *, pj_uint32_t);
typedef int (*pjsip_call_takeup_cb)(pjsua_call_id, void *, pj_uint32_t);
typedef int (*pjsip_call_timeout_cb)(pjsua_call_id, void *, pj_uint32_t);
typedef int (*pjsip_call_hangup_cb)(pjsua_call_id, void *, pj_uint32_t);

typedef struct pjapp_user_callback_tbl
{
	pjsip_register_state_cb pjsip_reg_state;
	pjsip_call_state_cb 	pjsip_call_state;
	pjsip_call_state_cb 	pjsip_call_incoming;
	pjsip_media_state_cb 	pjsip_media_state;
	pjsip_dtmf_recv_cb 		pjsip_dtmf_recv;
	pjsip_call_takeup_cb 	pjsip_call_takeup;
	pjsip_call_timeout_cb 	pjsip_call_timeout;
	pjsip_call_hangup_cb 	pjsip_call_hangup;

	int (*cli_account_state_get)(pjsua_acc_id, void *);
}pjapp_user_callback_tbl;


int pjapp_user_callback_init(pjapp_user_callback_tbl *cb);
int pjapp_user_register_state_callback(pjapp_user_callback_tbl *cb, pjsua_call_id id, void *pVoid, pj_uint32_t state);
int pjapp_user_call_state_callback(pjapp_user_callback_tbl *cb, pjsua_call_id id, void *pVoid, pj_uint32_t state);
int pjapp_user_media_state_callback(pjapp_user_callback_tbl *cb, pjsua_call_id id, void *pVoid, pj_uint32_t state);
int pjapp_user_recv_tdmf_callback(pjapp_user_callback_tbl *cb, pjsua_call_id id, void *pVoid, pj_uint32_t state);
int pjapp_user_call_takeup_callback(pjapp_user_callback_tbl *cb, pjsua_call_id id, void *pVoid, pj_uint32_t state);
int pjapp_user_call_timeout_callback(pjapp_user_callback_tbl *cb, pjsua_call_id id, void *pVoid, pj_uint32_t state);
int pjapp_user_call_hangup_callback(pjapp_user_callback_tbl *cb, pjsua_call_id id, void *pVoid, pj_uint32_t state);
int pjapp_user_call_incoming_callback(pjapp_user_callback_tbl *cb, pjsua_call_id id, void *pVoid, pj_uint32_t state);


#ifdef __cplusplus
}
#endif

#endif /* __PJSIP_PJAPP_CB_H__ */
