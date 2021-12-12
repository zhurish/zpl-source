/*
 * v9_video_user.h
 *
 *  Created on: 2018��12��18��
 *      Author: DELL
 */

#ifndef __V9_USER_DB_H__
#define __V9_USER_DB_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "v9_video.h"
//#include "v9_user_db.h"


#define V9_USER_DEBUG_EVENT		0X001
#define V9_USER_DEBUG_ERROR		0X002
#define V9_USER_DEBUG_WARN		0X004



#define V9_USER_DEBUG(n)			(V9_USER_DEBUG_ ## n & __user_debug_flag)
#define V9_USER_DEBUG_ON(n)		(__user_debug_flag |= (V9_USER_DEBUG_ ## n ))
#define V9_USER_DEBUG_OFF(n)		(__user_debug_flag &= ~(V9_USER_DEBUG_ ## n ))



#pragma pack(1)
typedef struct
{
	zpl_uint8			groupid;					   // 组ID  0： 黑名单 1： 白名单
	char			groupname[APP_USERNAME_MAX];// 组名
}user_group_t;

typedef struct
{
	//zpl_uint8			ID;
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
	zpl_uint8			ID;									// 计算板ID
	zpl_uint8			group;								// 所属组ID  0： 黑名单 1： 白名单
	char			username[APP_USERNAME_MAX];			// 姓名
	char			userid[APP_USERNAME_MAX];			// 证件号
	zpl_uint8			gender;								// 人员性别  0： 女 1： 男
	char			picname[APP_PATH_MAX];
	char			text[APP_USER_TEXT_MAX];			//备注信息
	sql_snapfea_key	key;								// 预留位，便于拓展，默认置空
	int				res;
}v9_video_user_t;


extern int __user_debug_flag;



extern v9_user_group_t _group_tbl[ID_INDEX(APP_BOARD_CALCU_4)];

extern int v9_video_usergroup_add(zpl_uint32 id, const char * groupname);
extern int v9_video_usergroup_del(zpl_uint32 id, const int group);
extern int v9_video_usergroup_rename(zpl_uint32 id, const int group, const char * groupname);
extern const char * v9_video_usergroup_idtoname(zpl_uint32 id, const int group);
extern zpl_uint32 v9_video_usergroup_nametoid(zpl_uint32 id, const char * groupname);
extern int v9_video_usergroup_show(zpl_uint32 id, struct vty *vty);

typedef int (*v9_vidoe_callback)(v9_video_user_t *, void*);

extern int v9_video_user_foreach(zpl_uint32 id, int groupid, v9_vidoe_callback cb, void *pVoid);


extern int v9_video_user_load();
extern int v9_video_user_clean(void);
extern int v9_video_user_exit();

extern int v9_video_user_count(zpl_uint32 id, int group, int *pValue);

extern int v9_video_user_add_user(zpl_uint32 id, zpl_bool gender, int group, char *user, char *user_id, char *pic, char *text);
extern int v9_video_user_update_user(zpl_uint32 id, zpl_bool gender, int group, char *user, char *user_id, char *pic, char *text);
extern int v9_video_user_del_user(zpl_uint32 id, char *user_id);
extern int v9_video_user_del_group(zpl_uint32 id,  zpl_uint8 group);
extern int v9_video_user_lookup_user(zpl_uint32 id, char *user_id, v9_video_user_t *user);
extern int v9_video_user_lookup_user_url(zpl_uint32 id, char *user_id, v9_video_user_t *user);
extern int v9_video_user_dir_add(zpl_uint32 id, const char *dirname);

extern int v9_video_user_show(struct vty *vty, zpl_bool detail);




#ifdef __cplusplus
}
#endif

#endif /* __V9_USER_DB_H__ */


