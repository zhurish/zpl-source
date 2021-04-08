/*
 * v9_video_sdk.h
 *
 *  Created on: 2019年12月1日
 *      Author: zhurish
 */

#ifndef __V9_VIDEO_SDK_H__
#define __V9_VIDEO_SDK_H__

#ifdef __cplusplus
extern "C" {
#endif

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

	ospl_uint8	address[V9_VIDEO_SDK_NAME_MAX];
	ospl_uint16	port;
	ospl_uint8	username[V9_APP_USERNAME_MAX];
	ospl_uint8	password[V9_APP_PASSWORD_MAX];

	//ospl_bool	init;
	ospl_bool	login;

	int		status;
	int		interval;
	void	*t_timeout;

	ospl_bool	getstate;
	void	*device;

	ospl_bool	find;
	int		type;
	int		mode;
	int		datatype;
}v9_video_sdk_t;


int v9_video_sdk_update_address(ospl_uint32 id, ospl_uint32 ip);

int v9_video_sdk_init(void);
int v9_video_sdk_task_init ();
int v9_video_sdk_start(ospl_uint32 id);
int v9_video_sdk_stop(ospl_uint32 id);

int v9_video_sdk_restart_all();

v9_video_sdk_t * v9_video_sdk_lookup(ospl_uint8 id);
int v9_video_sdk_show(struct vty * vty, int id, int debug);

/*****************************************************************/
int v9_video_sdk_reboot_api(ospl_uint32 id, ospl_bool reset);
int v9_video_sdk_getvch_api(ospl_uint32 id);
int v9_video_sdk_update_api(ospl_uint32 id, char *filename);

int v9_video_sdk_set_vch_api(ospl_uint32 id, int cnum, void *p);
int v9_video_sdk_add_vch_api(ospl_uint32 id, int ch, char *url);
int v9_video_sdk_del_vch_api(ospl_uint32 id, int ch);

// 请求/关闭抓拍信息
int v9_video_sdk_open_snap_api(ospl_uint32 id, ospl_uint32 type);
int v9_video_sdk_close_snap_api(ospl_uint32 id);


//抓拍上传地址配置
int v9_video_sdk_set_snap_dir_api(ospl_uint32 id, ospl_bool http, char *address, int port,
								  char *user, char *pass, char *dir);


// 获取/设置人脸识别配置参数
int v9_video_sdk_recognize_config_set_api(ospl_uint32 id, int nOutSimilarity,
									  int nRegisterQuality, ospl_bool nOpenUpload);
int v9_video_sdk_recognize_config_get_api(ospl_uint32 id, int *nOutSimilarity,
									  int *nRegisterQuality, ospl_bool *nOpenUpload);
// 安全帽配置
int v9_video_sdk_helmet_config_set_api(ospl_uint32 id, ospl_uint32 ch, void *data);
int v9_video_sdk_helmet_config_get_api(ospl_uint32 id, ospl_uint32 ch, void *data);

// 抓拍策略配置
int v9_video_sdk_snap_config_set_api(ospl_uint32 id, ospl_uint32 ch, void *data);
int v9_video_sdk_snap_config_get_api(ospl_uint32 id, ospl_uint32 ch, void *data);

//原图输出
int v9_video_sdk_original_pic_enable_set_api(ospl_uint32 id, ospl_bool enable);
int v9_video_sdk_original_pic_enable_get_api(ospl_uint32 id, ospl_bool *enable);


int v9_video_sdk_query_api(ospl_uint32 id, ospl_uint32 ch, ospl_uint32 nStartTime, ospl_uint32 nEndTime, void *data);


/*
 * 黑白名单
 */
int v9_video_sdk_add_user_api(ospl_uint32 id, ospl_bool gender, int group, char *user, char *ID, char *pic, ospl_bool edit);
int v9_video_sdk_del_user_api(ospl_uint32 id, char *ID);
int v9_video_sdk_del_group_user_api(ospl_uint32 id, void *p_pstUserList);
int v9_video_sdk_del_group_api(ospl_uint32 id, int group);
int v9_video_sdk_get_user_api(ospl_uint32 id, char* ID, void* UserInfo);

int v9_video_sdk_add_user_all_api(ospl_bool gender, int group, char *user, char *ID, char *pic, ospl_bool edit);
int v9_video_sdk_del_user_all_api(char *ID);
int v9_video_sdk_del_group_user_all_api(void *p_pstUserList);
int v9_video_sdk_del_group_all_api(int group);
int v9_video_sdk_get_user_all_api(char* ID, void* UserInfo);


#ifdef __cplusplus
}
#endif

#endif /* __V9_VIDEO_SDK_H__ */
