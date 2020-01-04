/*
 * v9_video_sdk.h
 *
 *  Created on: 2019年12月1日
 *      Author: zhurish
 */

#ifndef __V9_VIDEO_SDK_H__
#define __V9_VIDEO_SDK_H__

#include "v9_video.h"

#ifdef V9_VIDEO_SDK_API
#define V9_VIDEO_SDK_DEVICE_MAX	2
#define V9_VIDEO_SDK_DEVICE		"127.0.0.1"
#define V9_VIDEO_SDK_PORT		40001
#define V9_VIDEO_SDK_UPDATE_PORT		40002
#define V9_VIDEO_SDK_USERNAME	"admin"
#define V9_VIDEO_SDK_PASSWORD	"admin"
#define V9_VIDEO_SDK_TIMEOUT		50000
#endif

#define V9_VIDEO_SDK_NAME_MAX		32

typedef struct v9_video_sdk_s
{
	void 	*master;
	int		id;
	int		handle;				//计算板SDK操作符

	u_int8	address[V9_VIDEO_SDK_NAME_MAX];
	u_int16	port;
	u_int8	username[V9_APP_USERNAME_MAX];
	u_int8	password[V9_APP_PASSWORD_MAX];

	//BOOL	init;
	BOOL	login;

	int		status;
	int		interval;
	void	*t_timeout;

	BOOL	getstate;
	void	*device;

	BOOL	find;
	int		type;
	int		mode;
	int		datatype;
}v9_video_sdk_t;


int v9_video_sdk_update_address(u_int32 id, u_int32 ip);

int v9_video_sdk_init(void);
int v9_video_sdk_task_init ();
int v9_video_sdk_start(u_int32 id);
int v9_video_sdk_stop(u_int32 id);

int v9_video_sdk_restart_all();

v9_video_sdk_t * v9_video_sdk_lookup(u_int8 id);
int v9_video_sdk_show(struct vty * vty, int id, int debug);

/*****************************************************************/
int v9_video_sdk_reboot_api(u_int32 id, BOOL reset);
int v9_video_sdk_getvch_api(u_int32 id);
int v9_video_sdk_update_api(u_int32 id, char *filename);

int v9_video_sdk_set_vch_api(u_int32 id, int cnum, void *p);
int v9_video_sdk_add_vch_api(u_int32 id, int ch, char *url);
int v9_video_sdk_del_vch_api(u_int32 id, int ch);

// 请求/关闭抓拍信息
int v9_video_sdk_open_snap_api(u_int32 id, int type);
int v9_video_sdk_close_snap_api(u_int32 id);


//抓拍上传地址配置
int v9_video_sdk_set_snap_dir_api(u_int32 id, BOOL http, char *address, int port,
								  char *user, char *pass, char *dir);


// 获取/设置人脸识别配置参数
int v9_video_sdk_recognize_config_set_api(u_int32 id, int nOutSimilarity,
									  int nRegisterQuality, BOOL nOpenUpload);
int v9_video_sdk_recognize_config_get_api(u_int32 id, int *nOutSimilarity,
									  int *nRegisterQuality, BOOL *nOpenUpload);
// 安全帽配置
int v9_video_sdk_helmet_config_set_api(u_int32 id, u_int32 ch, void *data);
int v9_video_sdk_helmet_config_get_api(u_int32 id, u_int32 ch, void *data);

// 抓拍策略配置
int v9_video_sdk_snap_config_set_api(u_int32 id, u_int32 ch, void *data);
int v9_video_sdk_snap_config_get_api(u_int32 id, u_int32 ch, void *data);

//原图输出
int v9_video_sdk_original_pic_enable_set_api(u_int32 id, BOOL enable);
int v9_video_sdk_original_pic_enable_get_api(u_int32 id, BOOL *enable);


int v9_video_sdk_query_api(u_int32 id, u_int32 ch, u_int32 nStartTime, u_int32 nEndTime, void *data);


/*
 * 黑白名单
 */
int v9_video_sdk_add_user_api(u_int32 id, BOOL gender, int group, char *user, char *ID, char *pic, BOOL edit);
int v9_video_sdk_del_user_api(u_int32 id, char *ID);
int v9_video_sdk_del_group_user_api(u_int32 id, void *p_pstUserList);
int v9_video_sdk_del_group_api(u_int32 id, int group);
int v9_video_sdk_get_user_api(u_int32 id, char* ID, void* UserInfo);

int v9_video_sdk_add_user_all_api(BOOL gender, int group, char *user, char *ID, char *pic, BOOL edit);
int v9_video_sdk_del_user_all_api(char *ID);
int v9_video_sdk_del_group_user_all_api(void *p_pstUserList);
int v9_video_sdk_del_group_all_api(int group);
int v9_video_sdk_get_user_all_api(char* ID, void* UserInfo);


#endif /* __V9_VIDEO_SDK_H__ */
