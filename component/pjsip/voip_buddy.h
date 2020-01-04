/*
 * voip_buddy.h
 *
 *  Created on: 2019年10月19日
 *      Author: zhurish
 */

#ifndef __VOIP_BUDDY_H__
#define __VOIP_BUDDY_H__


#define BUDDY_USERNAME_MAX			32
#define BUDDY_PHONE_MAX				32
#define BUDDY_ADDRESS_MAX			64
#define BUDDY_EMAIL_MAX				32
#define BUDDY_COMPANY_MAX			64
#define BUDDY_GROUP_MAX				32

#define BUDDY_MULTI_NUMBER_MAX	8

#define BUDDY_DBASE_FILE	SYSCONFDIR"/buddydbase"




#pragma pack(1)
typedef struct
{
	char 		phone[BUDDY_PHONE_MAX];
	u_int8		active:1;
	u_int8		res:7;
}buddy_phone_t;

typedef struct
{
	NODE				node;
	char 				username[BUDDY_USERNAME_MAX];
	u_int32				userid;
	char 				address[BUDDY_ADDRESS_MAX];
	char 				email[BUDDY_EMAIL_MAX];
	char 				company[BUDDY_COMPANY_MAX];
	char 				group[BUDDY_GROUP_MAX];
	u_int32				groupid;
	buddy_phone_t 		buddy_phone[BUDDY_MULTI_NUMBER_MAX];
	void				*pVoid;
}buddy_user_t;


/*typedef struct buddy_dbase_s
{
	NODE			node;
	u_int8			number;
	u_int8			building;
	u_int8 			unit;
	u_int16			room_number;
	room_phone_t	phonetab[APP_MULTI_NUMBER_MAX];
}voip_dbase_t;*/
#pragma pack(0)

int buddy_dbase_clean(void);
int buddy_dbase_exit();
int buddy_dbase_load();

buddy_user_t * buddy_dbase_node_lookup_by_phonenumber(char *phone);
buddy_user_t * buddy_dbase_node_lookup_by_private_ID(int (*pri_cmp)(void *p1, void *p2), void *p2);
buddy_user_t * buddy_dbase_lookup_by_username(char *username);

int buddy_dbase_username_add(char *username, u_int32	userid);
int buddy_dbase_username_del(char *username, u_int32 userid);

int buddy_dbase_username_add_phone(char *username, char *phone);
int buddy_dbase_username_del_phone(char *username, char *phone);

#endif /* __VOIP_BUDDY_H__ */
