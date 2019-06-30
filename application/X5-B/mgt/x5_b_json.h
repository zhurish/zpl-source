/*
 * x5_b_app_ac.h
 *
 *  Created on: 2019年4月3日
 *      Author: DELL
 */

#ifndef __X5_B_APP_AC_H__
#define __X5_B_APP_AC_H__

#include "x5b_dbase.h"
typedef struct
{
	char 			username[APP_USERNAME_MAX];
	char 			user_id[APP_ID_MAX];

	char 			cardid[APP_CARD_ID_MAX];
	u_int32 		face_id;
	char 			face_img[APP_IMG_ID_MAX];
	u_int32 		start_date;
	u_int32 		stop_date;
	char 			cardtype[APP_ID_MAX];
	u_int8 			make_edit;
	u_int8 			make_card;
	u_int8 			make_face;
}uci_face_card_t;

#define X5B_APP_IMG_ANSYNC

#ifndef X5B_APP_IMG_ANSYNC
extern int base64_encode(char *in_str, int in_len, char **out_str);
extern int base64_decode(char *in_str, int in_len, char **output);

extern int x5b_face_img_base64_enc(char *input, char **out);
#endif


int x5b_app_add_card_json(x5b_app_mgt_t *app, void *info, int to);
int x5b_app_del_card_json(x5b_app_mgt_t *app, void *info, int to);


int x5b_app_make_card(uci_face_card_t *info);
int x5b_app_del_card(uci_face_card_t *info);
//extern int x5b_app_make_card_json(x5b_app_mgt_t *app, void *info, char *img, int to);
//extern int x5b_app_delete_card_json(x5b_app_mgt_t *app, void *info, char *img, int to);

/*extern int x5b_app_make_card_pack(x5b_app_mgt_t *app, void *info, int to);
extern int x5b_app_delete_card_pack(x5b_app_mgt_t *app, void *info, int to);*/
extern int x5b_app_add_face_card(x5b_app_mgt_t *app, void *info, int to);
extern int x5b_app_delete_face_card(x5b_app_mgt_t *app, void *info, int to);
extern int x5b_app_delete_face_card_hw(x5b_app_mgt_t *app, void *info, int to);

//extern int x5b_app_req_card_tlv_json(x5b_app_mgt_t *mgt, os_tlv_t *tlv);
extern int x5b_app_make_card_test(void);

extern int x5b_app_face_config_json(x5b_app_mgt_t *app, void *info, int to);

extern int x5b_app_face_id_request(x5b_app_mgt_t *app, int to);
extern int x5b_app_face_id_respone(x5b_app_mgt_t *mgt, os_tlv_t *tlv);
extern int x5b_app_face_load_respone(x5b_app_mgt_t *mgt, os_tlv_t *tlv);
extern int x5b_app_face_img_show(void *app, char *userid);
extern int x5b_app_json_test(void);


extern int x5b_app_device_json(x5b_app_mgt_t *app, char *device,
						char *address, char *dire, int to);
extern int x5b_app_sip_load_respone(x5b_app_mgt_t *mgt, os_tlv_t *tlv);
extern int x5b_app_call_respone(x5b_app_mgt_t *mgt, os_tlv_t *tlv);
//int asdadasdsas();

#endif /* __X5_B_APP_AC_H__ */
