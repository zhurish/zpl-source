/*
 * x5_b_call.h
 *
 *  Created on: 2019年10月19日
 *      Author: zhurish
 */

#ifndef __X5_B_CALL_H__
#define __X5_B_CALL_H__

#ifdef __cplusplus
extern "C" {
#endif


//开门信令
extern int x5b_app_open_door_api(x5b_app_mgt_t *app, int res, int to);
extern int x5b_app_open_door_by_cardid_api(x5b_app_mgt_t *app, zpl_uint8 *cardid, int clen, int to);

//呼叫结果上报
extern int x5b_app_call_result_api(x5b_app_mgt_t *app, int res, int inde, int to);
extern int x5b_app_call_internal_result_api(x5b_app_mgt_t *app, int res, int inde, int to);

//号码注册状态上报
extern int x5b_app_register_status_api(x5b_app_mgt_t *app, int res, int to);
extern int x5b_app_register_information_ack(x5b_app_mgt_t *mgt, int to);


#ifdef X5B_APP_DATABASE
/* 房间号鉴权 应答 */
extern int x5b_app_authentication_ack_api(x5b_app_mgt_t *mgt, voip_dbase_t *dbtest, int res, int to);
/* 接收到房间号鉴权请求 */
extern int x5b_app_read_authentication_tlv(x5b_app_mgt_t *mgt, os_tlv_t *tlv);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __X5_B_CALL_H__ */
