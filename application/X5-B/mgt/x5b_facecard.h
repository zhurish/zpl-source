/*
 * x5b_facecard.h
 *
 *  Created on: 2019年5月15日
 *      Author: DELL
 */

#ifndef SRC_SWPLATFORM_APPLICATION_X5_B_MGT_X5B_FACECARD_H_
#define SRC_SWPLATFORM_APPLICATION_X5_B_MGT_X5B_FACECARD_H_

#include "x5b_dbase.h"

#define APP_MULTI_FACE_MAX	8
#define APP_MULTI_CARD_MAX	8
//#define APP_CARDID_UINT_64

#pragma pack(1)

typedef struct
{
	char 			imgid[APP_IMG_ID_MAX];
	u_int32			faceid;
	u_int8			use_flag:1;
	u_int8			use_res:3;
	u_int8			res:4;
}user_face_t;

typedef struct
{
#ifndef APP_CARDID_UINT_64
	s_int8 			cardid[APP_CARD_ID_MAX+1];
#else
	u_int64 		cardid;
#endif
	u_int32			start_time;
	u_int32			stop_time;
	u_int8			card_type:4;//(1:2)
	u_int8			use_flag:1;
	u_int8			use_res:3;
}user_card_t;


typedef struct
{
	NODE			node;
	char 			username[APP_USERNAME_MAX];
	char 			userid[APP_ID_MAX];
	u_int8			card_max;
	u_int8			face_max;
	user_card_t 	cardtbl[APP_MULTI_CARD_MAX];
	user_face_t 	facetbl[APP_MULTI_FACE_MAX];
}user_face_card_t;

#pragma pack(0)

int x5b_user_clean(void);
int x5b_user_exit();
int x5b_user_load();
int x5b_user_save_config(void);
/***********************************************/
user_face_card_t * x5b_user_lookup_by_username(char *username, char *user_id);
user_face_card_t * x5b_user_lookup_by_cardid(s_int8 *cardid);
user_face_card_t * x5b_user_lookup_by_faceid(u_int32 faceid);
/*************************************************************************/
user_face_card_t * x5b_user_add(char *username, char *user_id);
int x5b_user_del(char *username, char *user_id);
/*************************************************************************/
int x5b_user_add_card(char *username, char *user_id, s_int8 *card_id, u_int32 start, u_int32 stop, u_int8 type);
int x5b_user_update_card(char *username, char *userid, s_int8 *cardid, u_int32 start,
					  u_int32 stop, u_int8 type, u_int8 cid);
int x5b_user_del_card(char *username, char *user_id, s_int8 *card_id);
int x5b_user_get_card(char *username, char *userid, s_int8 *cardid, u_int8 cid);
/*************************************************************************/
int x5b_user_add_face(char *username, char *user_id, char *img, u_int32 faceid);
int x5b_user_update_face(char *username, char *userid, char *img, u_int32 faceid);
int x5b_user_del_face(char *username, char *user_id, u_int32 face_id);
int x5b_user_get_face(char *username, char *user_id, u_int32 *face_id, u_int8 fid);
/*************************************************************************/
user_card_t * x5b_user_get_card_info(s_int8 *cardid);
user_face_t * x5b_user_get_face_info(u_int32 faceid);

int voip_facecard_web_select_all(void);
int voip_facecard_cli_show_all(struct vty *vty, BOOL detail);

#endif /* SRC_SWPLATFORM_APPLICATION_X5_B_MGT_X5B_FACECARD_H_ */
