/*
 * v9_video_user.h
 *
 *  Created on: 2018��12��18��
 *      Author: DELL
 */

#ifndef __V9_USER_DB_H__
#define __V9_USER_DB_H__


#include "v9_video.h"

#define V9_USER_DB_FILE	SYSCONFDIR"/userdb"
//#define V9_USER_DB_NOSDK

#ifdef EAIS_SDK_MAX_COMMON_LEN
#define APP_USERNAME_MAX EAIS_SDK_MAX_COMMON_LEN
#else
#define APP_USERNAME_MAX 64
#endif

#pragma pack(1)
typedef struct
{
	NODE			node;
	u_int8			ID;
	u_int8			group;										// 所属组ID  0： 黑名单 1： 白名单
	char			username[APP_USERNAME_MAX];			// 姓名
	char			userid[APP_USERNAME_MAX];				// 证件号
	u_int8			gender;										// 人员性别  0： 女 1： 男
	char			picname[APP_USERNAME_MAX];
	//int        	nFaceLen;										// 人脸图片数据长度（1, 1024 * 1024]字节
	//unsigned char*szPictureData;									// 图片内容
	//char			szComment[EAIS_SDK_USER_FACE_COMMENT_LEN];		// 用户自定义备注字段
	//int				nRegisterTime;									// 注册时间，从1970-01-01 00:00:00 (utc) 开始计时的秒数
	//char			szReserved[256];								// 预留位，便于拓展，默认置空
}v9_video_user_t;

#pragma pack(0)

typedef int (*v9_vidoe_callback)(v9_video_user_t *, void*);

extern int v9_video_user_load();
extern int v9_video_user_clean(void);
extern int v9_video_user_exit();
extern int v9_video_user_foreach(v9_vidoe_callback cb, void *pVoid);

extern v9_video_user_t * v9_video_user_lookup_by_userid(int id, char *user_id);

extern int v9_video_user_add_user(u_int32 id, BOOL gender, int group, char *user, char *user_id, char *pic);
extern int v9_video_user_update_user(u_int32 id, BOOL gender, int group, char *user, char *user_id, char *pic);
extern int v9_video_user_del_user(u_int32 id, char *user_id);

extern int v9_video_user_show(struct vty *vty, BOOL detail);


#endif /* __V9_USER_DB_H__ */


