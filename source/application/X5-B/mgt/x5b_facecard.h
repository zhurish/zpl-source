/*
 * x5b_facecard.h
 *
 *  Created on: 2019年5月15日
 *      Author: DELL
 */

#ifndef SRC_SWPLATFORM_APPLICATION_X5_B_MGT_X5B_FACECARD_H_
#define SRC_SWPLATFORM_APPLICATION_X5_B_MGT_X5B_FACECARD_H_

#ifdef __cplusplus
extern "C" {
#endif


#ifdef X5B_APP_DATABASE
#include "x5b_dbase.h"

#define APP_MULTI_FACE_MAX	8
#define APP_MULTI_CARD_MAX	8
//#define APP_CARDID_UINT_64

#pragma pack(1)

typedef struct
{
	char 			imgid[APP_IMG_ID_MAX];
	zpl_uint32			faceid;
	zpl_uint8			use_flag:1;
	zpl_uint8			use_res:3;
	zpl_uint8			res:4;
}user_face_t;

typedef struct
{
#ifndef APP_CARDID_UINT_64
	zpl_int8 			cardid[APP_CARD_ID_MAX+1];
#else
	zpl_uint64 		cardid;
#endif
	zpl_uint32			start_time;
	zpl_uint32			stop_time;
	zpl_uint8			card_type:4;//(1:2)
	zpl_uint8			use_flag:1;
	zpl_uint8			use_res:3;
}user_card_t;


typedef struct
{
	NODE			node;
	char 			username[APP_USERNAME_MAX];
	char 			userid[APP_ID_MAX];
	zpl_uint8			card_max;
	zpl_uint8			face_max;
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
user_face_card_t * x5b_user_lookup_by_cardid(zpl_int8 *cardid);
user_face_card_t * x5b_user_lookup_by_faceid(zpl_uint32 faceid);
/*************************************************************************/
user_face_card_t * x5b_user_add(char *username, char *user_id);
int x5b_user_del(char *username, char *user_id);
/*************************************************************************/
int x5b_user_add_card(char *username, char *user_id, zpl_int8 *card_id, zpl_uint32 start, zpl_uint32 stop, zpl_uint8 type);
int x5b_user_update_card(char *username, char *userid, zpl_int8 *cardid, zpl_uint32 start,
					  zpl_uint32 stop, zpl_uint8 type, zpl_uint8 cid);
int x5b_user_del_card(char *username, char *user_id, zpl_int8 *card_id);
int x5b_user_get_card(char *username, char *userid, zpl_int8 *cardid, zpl_uint8 cid);
/*************************************************************************/
int x5b_user_add_face(char *username, char *user_id, char *img, zpl_uint32 faceid);
int x5b_user_update_face(char *username, char *userid, char *img, zpl_uint32 faceid);
int x5b_user_del_face(char *username, char *user_id, zpl_uint32 face_id);
int x5b_user_get_face(char *username, char *user_id, zpl_uint32 *face_id, zpl_uint8 fid);
/*************************************************************************/
user_card_t * x5b_user_get_card_info(zpl_int8 *cardid);
user_face_t * x5b_user_get_face_info(zpl_uint32 faceid);

int voip_facecard_web_select_all(void);
int voip_facecard_cli_show_all(struct vty *vty, zpl_bool detail);
#endif


#ifdef __cplusplus
}
#endif


#endif /* SRC_SWPLATFORM_APPLICATION_X5_B_MGT_X5B_FACECARD_H_ */
