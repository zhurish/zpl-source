/*
 * x5_b_app_ac.h
 *
 *  Created on: 2019年4月3日
 *      Author: DELL
 */

#ifndef __X5_B_APP_AC_H__
#define __X5_B_APP_AC_H__


extern int x5b_app_face_config_json(x5b_app_mgt_t *app, void *info, int to);

extern int x5b_app_device_json(x5b_app_mgt_t *app, char *device,
						char *address, char *dire, int topf, int to);

extern int x5b_app_sip_config_parse(x5b_app_mgt_t *mgt, os_tlv_t *tlv);
extern int x5b_app_call_phone_number(x5b_app_mgt_t *mgt, os_tlv_t *tlv, BOOL list);


extern int x5b_app_call_user_list(char *json, u_int8 *building, u_int8 *unit,
		u_int16 *room_number, void *call_phone);

extern int x5b_app_call_user_list_test();

#endif /* __X5_B_APP_AC_H__ */
