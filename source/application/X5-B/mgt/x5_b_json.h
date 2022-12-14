/*
 * x5_b_app_ac.h
 *
 *  Created on: 2019年4月3日
 *      Author: DELL
 */

#ifndef __X5_B_APP_AC_H__
#define __X5_B_APP_AC_H__


#ifdef __cplusplus
extern "C" {
#endif

extern int x5b_app_face_config_json(x5b_app_mgt_t *app, void *info, int to);

extern int x5b_app_device_json(x5b_app_mgt_t *app, char *device,
						char *address, char *dire, int topf, int to);

extern int x5b_app_sip_config_parse(x5b_app_mgt_t *mgt, os_tlv_t *tlv);
extern int x5b_app_call_phone_number(x5b_app_mgt_t *mgt, os_tlv_t *tlv, zpl_bool list);


extern int x5b_app_call_user_list(char *json, zpl_uint8 *building, zpl_uint8 *unit,
		zpl_uint16 *room_number, void *call_phone);

extern int x5b_app_call_user_list_test();

#ifdef __cplusplus
}
#endif

#endif /* __X5_B_APP_AC_H__ */
