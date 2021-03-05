/*
 * pjsua_app_cb.h
 *
 *  Created on: Jun 22, 2019
 *      Author: zhurish
 */

#ifndef __PJSIP_PJSUA_APP_CB_H__
#define __PJSIP_PJSUA_APP_CB_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "ospl_type.h"

typedef int (*pjsip_register_state_cb)(pjsua_call_id, void *, ospl_uint32);
typedef int (*pjsip_call_state_cb)(pjsua_call_id, void *, ospl_uint32);
typedef int (*pjsip_media_state_cb)(pjsua_call_id, void *, ospl_uint32);
typedef int (*pjsip_dtmf_recv_cb)(pjsua_call_id, void *, ospl_uint32);
typedef int (*pjsip_call_takeup_cb)(pjsua_call_id, void *, ospl_uint32);
typedef int (*pjsip_call_timeout_cb)(pjsua_call_id, void *, ospl_uint32);
typedef int (*pjsip_call_hangup_cb)(pjsua_call_id, void *, ospl_uint32);

typedef struct pjsip_callback_tbl
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
}pjsip_callback_tbl;

//extern pjsip_callback_tbl_t	cbtbl;


int pjsip_app_callback_init(void *p, pjsip_callback_tbl *cb);
int pjsip_app_register_state_callback(pjsip_callback_tbl *cb, pjsua_call_id id, void *pVoid, ospl_uint32 state);
int pjsip_app_call_state_callback(pjsip_callback_tbl *cb, pjsua_call_id id, void *pVoid, ospl_uint32 state);
int pjsip_app_media_state_callback(pjsip_callback_tbl *cb, pjsua_call_id id, void *pVoid, ospl_uint32 state);
int pjsip_app_dtmf_recv_callback(pjsip_callback_tbl *cb, pjsua_call_id id, void *pVoid, ospl_uint32 state);
int pjsip_app_call_takeup_callback(pjsip_callback_tbl *cb, pjsua_call_id id, void *pVoid, ospl_uint32 state);
int pjsip_app_call_timeout_callback(pjsip_callback_tbl *cb, pjsua_call_id id, void *pVoid, ospl_uint32 state);
int pjsip_app_call_hangup_callback(pjsip_callback_tbl *cb, pjsua_call_id id, void *pVoid, ospl_uint32 state);
int pjsip_app_call_incoming_callback(pjsip_callback_tbl *cb, pjsua_call_id id, void *pVoid, ospl_uint32 state);

#ifdef __cplusplus
}
#endif

#endif /* __PJSIP_PJSUA_APP_CB_H__ */
