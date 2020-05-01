/*
 * v9_video_user.h
 *
 *  Created on: 2018��12��18��
 *      Author: DELL
 */

#ifndef __V9_USER_DB_H__
#define __V9_USER_DB_H__


#include "v9_video.h"
//#include "v9_user_db.h"


#define V9_USER_DEBUG_EVENT		0X001
#define V9_USER_DEBUG_ERROR		0X002
#define V9_USER_DEBUG_WARN		0X004



#define V9_USER_DEBUG(n)			(V9_SDK_DEBUG_ ## n & __user_debug_flag)
#define V9_USER_DEBUG_ON(n)		(__user_debug_flag |= (V9_SDK_DEBUG_ ## n ))
#define V9_USER_DEBUG_OFF(n)		(__user_debug_flag &= ~(V9_SDK_DEBUG_ ## n ))



#pragma pack(1)
typedef struct
{
	u_int8			groupid;					   // 组ID  0： 黑名单 1： 白名单
	char			groupname[APP_USERNAME_MAX];// 组名
}user_group_t;

typedef struct
{
	//u_int8			ID;
	user_group_t	gtbl[APP_GROUP_MAX];
}v9_user_group_t;
#pragma pack(0)

#define GROUP_ACTIVE_BIT			(0x80)
#define GROUP_ACTIVE(n)		((n)&0x80)
#define GROUP_INDEX(n)			((n)&0x7f)
#define ID_INDEX(n)			V9_APP_BOARD_HW_ID((n))

#define V9_USER_GROUP_FILE			SYSCONFDIR"/.usergroupdb"


typedef struct
{
	u_int8			ID;									// 计算板ID
	u_int8			group;								// 所属组ID  0： 黑名单 1： 白名单
	char			username[APP_USERNAME_MAX];			// 姓名
	char			userid[APP_USERNAME_MAX];			// 证件号
	u_int8			gender;								// 人员性别  0： 女 1： 男
	char			picname[APP_PATH_MAX];
	char			text[APP_USER_TEXT_MAX];			//备注信息
	sql_snapfea_key	key;								// 预留位，便于拓展，默认置空
	int				res;
}v9_video_user_t;


extern int __user_debug_flag;



extern v9_user_group_t _group_tbl[ID_INDEX(APP_BOARD_CALCU_4)];

extern int v9_video_usergroup_add(u_int32 id, const char * groupname);
extern int v9_video_usergroup_del(u_int32 id, const int group);
extern int v9_video_usergroup_rename(u_int32 id, const int group, const char * groupname);
extern const char * v9_video_usergroup_idtoname(u_int32 id, const int group);
extern u_int32 v9_video_usergroup_nametoid(u_int32 id, const char * groupname);
extern int v9_video_usergroup_show(u_int32 id, struct vty *vty);

typedef int (*v9_vidoe_callback)(v9_video_user_t *, void*);

extern int v9_video_user_foreach(u_int32 id, int groupid, v9_vidoe_callback cb, void *pVoid);


extern int v9_video_user_load();
extern int v9_video_user_clean(void);
extern int v9_video_user_exit();

extern int v9_video_user_count(u_int32 id, int group, int *pValue);

extern int v9_video_user_add_user(u_int32 id, BOOL gender, int group, char *user, char *user_id, char *pic, char *text);
extern int v9_video_user_update_user(u_int32 id, BOOL gender, int group, char *user, char *user_id, char *pic, char *text);
extern int v9_video_user_del_user(u_int32 id, char *user_id);
extern int v9_video_user_del_group(u_int32 id,  u_int8 group);
extern int v9_video_user_lookup_user(u_int32 id, char *user_id, v9_video_user_t *user);
extern int v9_video_user_lookup_user_url(u_int32 id, char *user_id, v9_video_user_t *user);
extern int v9_video_user_dir_add(u_int32 id, const char *dirname);

extern int v9_video_user_show(struct vty *vty, BOOL detail);




#endif /* __V9_USER_DB_H__ */


