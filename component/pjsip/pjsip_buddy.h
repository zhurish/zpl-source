/*
 * pjsip_buddy.h
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

#define BUDDY_DBASE_FILE	SYSCONFDIR"/buddydbase"




#pragma pack(1)
typedef struct
{
	char 		phone[BUDDY_PHONE_MAX];
	ospl_uint8		active:1;
	ospl_uint8		res:7;
}pjsip_buddy_phone_t;

typedef struct
{
	NODE				node;
	char 				username[BUDDY_USERNAME_MAX];
	ospl_uint32				userid;
	char 				address[BUDDY_ADDRESS_MAX];
	char 				email[BUDDY_EMAIL_MAX];
	char 				company[BUDDY_COMPANY_MAX];
	char 				group[BUDDY_GROUP_MAX];
	ospl_uint32				groupid;
	pjsip_buddy_phone_t 		pjsip_buddy_phone[BUDDY_MULTI_NUMBER_MAX];
	void				*pVoid;
}pjsip_buddy_t;

#pragma pack(0)

int pjsip_buddy_clean(void);
int pjsip_buddy_exit();
int pjsip_buddy_load();
int pjsip_buddy_update_save(void);

pjsip_buddy_t * pjsip_buddy_node_lookup_by_phonenumber(char *phone);
pjsip_buddy_t * pjsip_buddy_node_lookup_by_private_ID(int (*pri_cmp)(void *p1, void *p2), void *p2);
pjsip_buddy_t * pjsip_buddy_lookup_by_username(char *username);

int pjsip_buddy_username_add(char *username, ospl_uint32 userid);
int pjsip_buddy_username_del(char *username, ospl_uint32 userid);

int pjsip_buddy_username_add_phone(char *username, char *phone);
int pjsip_buddy_username_del_phone(char *username, char *phone);

#ifdef __cplusplus
}
#endif

#endif /* __PJSIP_BUDDY_H__ */
