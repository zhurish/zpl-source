/*
 * v9_video_sdk.h
 *
 *  Created on: 2019年12月1日
 *      Author: zhurish
 */

#ifndef __V9_VIDEO_SDK_H__
#define __V9_VIDEO_SDK_H__

//#define V9_VIDEO_SDK_API

#include "v9_video.h"


//#define V9_VIDEO_SDK_DEVICE_MAX			2
//#define V9_VIDEO_SDK_DEVICE				"127.0.0.1"
#define V9_VIDEO_SDK_PORT				40001
#define V9_VIDEO_SDK_UPDATE_PORT		40002
#define V9_VIDEO_SDK_USERNAME			"admin"
#define V9_VIDEO_SDK_PASSWORD			"admin"
#define V9_VIDEO_SDK_TIMEOUT			50000

/*
#ifdef V9_VIDEO_SDK_API

#endif
*/

#define V9_VIDEO_SDK_NAME_MAX		32



#define V9_SDK_DEBUG_EVENT		0X001
#define V9_SDK_DEBUG_ERROR		0X002
#define V9_SDK_DEBUG_WARN		0X004
#define V9_SDK_DEBUG_WEB		0X040
#define V9_SDK_DEBUG_MSG		0X080
#define V9_SDK_DEBUG_STATE		0X100


#define V9_SDK_DEBUG(n)			(V9_SDK_DEBUG_ ## n & __sdk_debug_flag)
#define V9_SDK_DEBUG_ON(n)		(__sdk_debug_flag |= (V9_SDK_DEBUG_ ## n ))
#define V9_SDK_DEBUG_OFF(n)		(__sdk_debug_flag &= ~(V9_SDK_DEBUG_ ## n ))


#define V9_SDK_ID(n)		V9_APP_BOARD_HW_ID(((v9_video_board_t *)((n)->board))->id)

typedef struct v9_video_sdk_s
{
	void 		*master;
	BOOL		initialization;
	int		handle;				//计算板SDK操作符

/*	u_int8	address[V9_VIDEO_SDK_NAME_MAX];
	u_int16	port;*/

	u_int8	username[APP_USERNAME_MAX];
	u_int8	password[APP_USERNAME_MAX];

	//BOOL		init;
	BOOL		login;

	int		status;
	int		interval;
	void		*t_timeout;

	BOOL		getstate;
	void		*device;

	BOOL		find;
	int		type;
	int		mode;
	int		datatype;
	void		*board;
	//int		debug;
}v9_video_sdk_t;

extern int __sdk_debug_flag;


char *v9_video_sdk_errnostr(int err);

int v9_video_sdk_init(v9_video_sdk_t *sdk, void *board);
int v9_video_sdk_task_init ();
//int v9_video_sdk_start(u_int32 id);
//int v9_video_sdk_stop(u_int32 id);

int v9_video_sdk_restart_all();

v9_video_sdk_t * v9_video_sdk_lookup(u_int8 id);
int v9_video_sdk_show(struct vty * vty, int id, int debug);

/*****************************************************************/
int v9_video_sdk_reboot_api(u_int32 id);
int v9_video_sdk_reset_api(u_int32 id);
int v9_video_sdk_getvch_api(u_int32 id);
int v9_video_sdk_update_api(u_int32 id, char *filename);

int v9_video_sdk_set_vch_api(u_int32 id, int cnum, void *p);
int v9_video_sdk_add_vch_api(u_int32 id, int ch, char *url);
int v9_video_sdk_del_vch_api(u_int32 id, int ch);
int v9_video_sdk_get_vch_api(u_int32 id, int ch, void *p);
int v9_video_sdk_lookup_vch_api(u_int32 id, int ch);

int v9_video_sdk_get_rtsp_status_api(u_int32 id);//获取RTSP 状态

// 请求/关闭抓拍信息
int v9_video_sdk_open_snap_api(u_int32 id, int type);
int v9_video_sdk_close_snap_api(u_int32 id);


//抓拍上传地址配置
int v9_video_sdk_set_snap_dir_api(u_int32 id, BOOL http, char *address, int port,
								  char *user, char *pass, char *dir);
/*
 * address:10.10.10.254
 * dir:/mnt/diska1/board1/
 */
int v9_video_sdk_nfsdir_api(u_int32 id, char *address, char *user, char *pass, char *dir);


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

/*
 * 查询抓拍统计
 */
int v9_video_sdk_query_api(u_int32 id, u_int32 ch, u_int32 nStartTime, u_int32 nEndTime, void *data);



// 告警信息配置
int v9_video_sdk_alarm_config_set_api(u_int32 id, u_int32 ch, void *data);
int v9_video_sdk_alarm_config_get_api(u_int32 id, s_int32 ch, void *data);

/*
 * 获取图像特征值
 */
int v9_video_sdk_get_keyvalue_api(u_int32 id, char* pic, void* UserInfo);


/*
 * 获取特征值余弦相似度
 */
int v9_video_sdk_get_sosine_similarity_api(const float* p_fFirstArray, const float* p_fSecondArray, int p_nlength, float* p_fResult);



/*
 * 黑白名单
 */
/*
 * 在组里添加用户
 */
int v9_video_sdk_add_user_api(u_int32 id, BOOL gender, int group, char *user, char *ID, char *pic, char *text, BOOL add);
/*
 * 删除用户
 */
int v9_video_sdk_del_user_api(u_int32 id, int group, char *ID);
/*
 * 批量删除某个分组的用户
 */
int v9_video_sdk_del_group_user_api(u_int32 id, void *p_pstUserList);

/*
 * 查询用户 当用户不存在时返回-3
 */
int v9_video_sdk_get_user_api(u_int32 id, char* ID, void* UserInfo);


/*
 * 添加某个分组
 */
int v9_video_sdk_add_group_api(u_int32 id, int group, char *name);
/*
 * 删除某个分组
 */
int v9_video_sdk_del_group_api(u_int32 id, int group);


int v9_video_sdk_ntp_api(u_int32 id, char *ntps, int TimingInterval);
int v9_video_sdk_timer_api(u_int32 id);

int v9_video_sdk_get_config(struct vty *vty, u_int32 id, v9_video_sdk_t *sdk);

#endif /* __V9_VIDEO_SDK_H__ */
