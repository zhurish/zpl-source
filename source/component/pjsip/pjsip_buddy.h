/*
 * pjapp_buddy.h
 *
 *  Created on: 2019年10月19日
 *      Author: zhurish
 */

#ifndef __PJSIP_BUDDY_H__
#define __PJSIP_BUDDY_H__


#ifdef __cplusplus
extern "C" {
#endif

#define BUDDY_USERNAME_MAX			32
#define BUDDY_PHONE_MAX				32
#define BUDDY_ADDRESS_MAX			64
#define BUDDY_EMAIL_MAX				32
#define BUDDY_COMPANY_MAX			64
#define BUDDY_GROUP_MAX				32

#define BUDDY_MULTI_NUMBER_MAX	8

#define BUDDY_DBASE_FILE	SYSCONF_REAL_DIR"/buddydbase"




#pragma pack(1)
typedef struct
{
	char 		phone[BUDDY_PHONE_MAX];
	pj_uint8_t		active:1;
	pj_uint8_t		res:7;
}pjapp_buddy_phone_t;

typedef struct
{
	NODE				node;
	char 				username[BUDDY_USERNAME_MAX];
	pj_uint32_t				userid;
	char 				address[BUDDY_ADDRESS_MAX];
	char 				email[BUDDY_EMAIL_MAX];
	char 				company[BUDDY_COMPANY_MAX];
	char 				group[BUDDY_GROUP_MAX];
	pj_uint32_t				groupid;
	pjapp_buddy_phone_t 		pjapp_buddy_phone[BUDDY_MULTI_NUMBER_MAX];
	void				*pVoid;
}pjapp_buddy_t;

#pragma pack(0)

int pjapp_buddy_clean(void);
int pjapp_buddy_exit(void);
int pjapp_buddy_load(void);
int pjapp_buddy_update_save(void);

pjapp_buddy_t * pjapp_buddy_node_lookup_by_phonenumber(char *phone);
pjapp_buddy_t * pjapp_buddy_node_lookup_by_private_ID(int (*pri_cmp)(void *p1, void *p2), void *p2);
pjapp_buddy_t * pjapp_buddy_lookup_by_username(char *username);

int pjapp_buddy_username_add(char *username, pj_uint32_t userid);
int pjapp_buddy_username_del(char *username, pj_uint32_t userid);

int pjapp_buddy_username_add_phone(char *username, char *phone);
int pjapp_buddy_username_del_phone(char *username, char *phone);

#ifdef __cplusplus
}
#endif

#endif /* __PJSIP_BUDDY_H__ */
