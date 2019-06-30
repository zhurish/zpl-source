/*
 * x5b_dbase.h
 *
 *  Created on: 2018��12��18��
 *      Author: DELL
 */

#ifndef __X5B_DBASE_H__
#define __X5B_DBASE_H__


#include "zebra.h"
//#include "voip_app.h"
//#include "voip_sip.h"

#define APP_USERNAME_MAX		32
#define APP_ID_MAX				32
#define APP_CARD_ID_MAX			16
#define APP_IMG_ID_MAX			32
#define APP_MULTI_NUMBER_MAX	8

#define X5B_DBASE_FILE	SYSCONFDIR"/dbase"
#define X5B_DBCARD_FILE	SYSCONFDIR"/card"
#define X5B_DBFACECARD_FILE	SYSCONFDIR"/facecard"

#pragma pack(1)
typedef struct
{
	u_int8			user_type:4;
	u_int8			use_flag:1;
	u_int8			use_res:3;
	char 			phone[APP_ID_MAX];
	char 			username[APP_USERNAME_MAX];
	char 			user_id[APP_ID_MAX];
	//char 			card_id[APP_CARD_ID_MAX];
	//char 			img_id[APP_IMG_ID_MAX];
	//u_int32			face_id;
	//u_int32			start_time;
	//u_int32			stop_time;
	//u_int8			card_type:4;//(1:2)
	//u_int8			make_card:2;
	//u_int8			make_face:2;
}room_phone_t;

typedef struct
{
	NODE			node;
	char 			username[APP_USERNAME_MAX];
	char 			user_id[APP_ID_MAX];
	char 			card_id[APP_CARD_ID_MAX + 1];
	char 			img_id[APP_IMG_ID_MAX];
	u_int32			face_id;
	u_int32			start_time;
	u_int32			stop_time;
	u_int8			card_type:4;//(1:2)
	u_int8			make_card:2;
	u_int8			make_face:2;
}face_card_t;


typedef struct voip_dbase_s
{
	NODE			node;
	u_int8			number;
	u_int8			building;
	u_int8 			unit;
	u_int16			room_number;
	room_phone_t	phonetab[APP_MULTI_NUMBER_MAX];
}voip_dbase_t;
#pragma pack(0)


extern int voip_dbase_enable(BOOL enable);
extern BOOL voip_dbase_isenable();
extern int voip_dbase_load();
extern int voip_dbase_clean(void);
extern int voip_dbase_exit();

extern voip_dbase_t * voip_dbase_node_lookup_by_phonenumber(char *phone);
extern voip_dbase_t * voip_dbase_node_lookup_by_username(char *username, char *user_id);
extern voip_dbase_t * voip_dbase_lookup_by_room(u_int8 building, u_int8 unit, u_int16 room_number);
extern int voip_dbase_add_room(u_int8 building, u_int8 unit, u_int16 room_number);
extern int voip_dbase_del_room(u_int8 building, u_int8 unit, u_int16 room_number);
extern int voip_dbase_add_room_phone(u_int8 building, u_int8 unit, u_int16 room_number, char *phone, char *username, char *user_id);
extern int voip_dbase_del_room_phone(u_int8 building, u_int8 unit, u_int16 room_number, char *phone, char *username, char *user_id);
extern int voip_dbase_update_room_phone(u_int8 building, u_int8 unit, u_int16 room_number, char *phone, char *username, char *user_id);

extern int voip_dbase_get_room_phone(u_int8 building, u_int8 unit, u_int16 room_number, char *phone);
#ifdef PL_VOIP_MODULE
extern int voip_dbase_get_call_phone(u_int8 building, u_int8 unit, u_int16 room_number, void *call_phone);
#endif
extern int voip_dbase_get_phone_by_user(u_int8 building, u_int8 unit, u_int16 room_number, char *username, char *user_id, char *phone);
extern int voip_dbase_get_user_by_phone(char *phone, char *username, char *user_id);
extern int voip_dbase_get_room_phone_by_user(char *user_id, u_int16 *room_number,
		char *phone, char *username);

extern int voip_dbase_show_room_phone(struct vty *vty, u_int8 building, u_int8 unit, u_int16 room_number, char *username, char *user_id);

extern int voip_ubus_dbase_sync(int cmd);


/*****************************************************************/
extern int voip_card_clean(void);
extern int voip_card_exit();
extern int voip_card_load();
extern face_card_t * voip_card_node_lookup_by_username(char *username, char *user_id);
extern face_card_t * voip_card_node_lookup_by_cardid(char *carid);

extern int voip_card_update_cardid(char *user_id, face_card_t *info);
extern int voip_card_update_cardid_by_userid(char *user_id, face_card_t *info);
extern int voip_card_update_face_by_userid(char *user_id, face_card_t *info);
extern int voip_card_lookup_by_cardid_userid(char *carid, char *userid);

extern int voip_card_add_cardid(face_card_t *card);
extern int voip_card_del_cardid(char *username, char *user_id, char *carid);

extern int voip_card_cli_show_all(struct vty *vty);
extern int show_voip_card_info(struct vty *vty);
extern int voip_card_web_select_all(void);

extern int card_id_string_to_hex(const char *id, int len, u_int8 *cardNumber);

#endif /* __X5B_DBASE_H__ */


