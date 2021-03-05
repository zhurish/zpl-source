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
	ospl_bool		initialization;
	int		handle;				//计算板SDK操作符

/*	ospl_uint8	address[V9_VIDEO_SDK_NAME_MAX];
	ospl_uint16	port;*/

	ospl_uint8	username[APP_USERNAME_MAX];
	ospl_uint8	password[APP_USERNAME_MAX];

	//ospl_bool		init;
	ospl_bool		login;

	int		status;
	int		interval;
	void		*t_timeout;

	ospl_bool		getstate;
	void		*device;

	ospl_bool		find;
	int		type;
	int		mode;
	int		datatype;
	void		*board;
	ospl_bool		nfs;
	//int		debug;
}v9_video_sdk_t;

extern int __sdk_debug_flag;


char *v9_video_sdk_errnostr(int err);

int v9_video_sdk_init(v9_video_sdk_t *sdk, void *board);
int v9_video_sdk_task_init ();
//int v9_video_sdk_start(ospl_uint32 id);
//int v9_video_sdk_stop(ospl_uint32 id);

int v9_video_sdk_restart_all();

v9_video_sdk_t * v9_video_sdk_lookup(ospl_uint8 id);
int v9_video_sdk_show(struct vty * vty, int id, int debug);

/*****************************************************************/
int v9_video_sdk_reboot_api(ospl_uint32 id);
int v9_video_sdk_reset_api(ospl_uint32 id);
int v9_video_sdk_getvch_api(ospl_uint32 id);
int v9_video_sdk_update_api(ospl_uint32 id, char *filename);

int v9_video_sdk_set_vch_api(ospl_uint32 id, int cnum, void *p);
int v9_video_sdk_add_vch_api(ospl_uint32 id, int ch, char *url);
int v9_video_sdk_del_vch_api(ospl_uint32 id, int ch);
int v9_video_sdk_get_vch_api(ospl_uint32 id, int ch, void *p);
int v9_video_sdk_lookup_vch_api(ospl_uint32 id, int ch);

int v9_video_sdk_get_rtsp_status_api(ospl_uint32 id);//获取RTSP 状态

// 请求/关闭抓拍信息
int v9_video_sdk_open_snap_api(ospl_uint32 id, ospl_uint32 type);
int v9_video_sdk_close_snap_api(ospl_uint32 id);


//抓拍上传地址配置
int v9_video_sdk_set_snap_dir_api(ospl_uint32 id, ospl_bool http, char *address, int port,
								  char *user, char *pass, char *dir);
/*
 * address:10.10.10.254
 * dir:/mnt/diska1/board1/
 */
int v9_video_sdk_nfsdir_api(ospl_uint32 id, char *address, char *user, char *pass, char *dir);


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

/*
 * 查询抓拍统计
 */
int v9_video_sdk_query_api(ospl_uint32 id, ospl_uint32 ch, ospl_uint32 nStartTime, ospl_uint32 nEndTime, void *data);



// 告警信息配置
int v9_video_sdk_alarm_config_set_api(ospl_uint32 id, ospl_uint32 ch, void *data);
int v9_video_sdk_alarm_config_get_api(ospl_uint32 id, ospl_int32 ch, void *data);

/*
 * 获取图像特征值
 */
int v9_video_sdk_get_keyvalue_api(ospl_uint32 id, char* pic, void* UserInfo);


/*
 * 获取特征值余弦相似度
 */
int v9_video_sdk_get_sosine_similarity_api(const ospl_float* p_fFirstArray, const ospl_float* p_fSecondArray, int p_nlength, ospl_float* p_fResult);



/*
 * 黑白名单
 */
/*
 * 在组里添加用户
 */
int v9_video_sdk_add_user_api(ospl_uint32 id, ospl_bool gender, int group, char *user, char *ID, char *pic, char *text, ospl_bool add);
/*
 * 删除用户
 */
int v9_video_sdk_del_user_api(ospl_uint32 id, int group, char *ID);
/*
 * 批量删除某个分组的用户
 */
int v9_video_sdk_del_group_user_api(ospl_uint32 id, void *p_pstUserList);

/*
 * 查询用户 当用户不存在时返回-3
 */
int v9_video_sdk_get_user_api(ospl_uint32 id, char* ID, void* UserInfo);


/*
 * 添加某个分组
 */
int v9_video_sdk_add_group_api(ospl_uint32 id, int group, char *name);
/*
 * 删除某个分组
 */
int v9_video_sdk_del_group_api(ospl_uint32 id, int group);


int v9_video_sdk_ntp_api(ospl_uint32 id, char *ntps, int TimingInterval);
int v9_video_sdk_timer_api(ospl_uint32 id);

int v9_video_sdk_get_config(struct vty *vty, ospl_uint32 id, v9_video_sdk_t *sdk);

#ifdef __cplusplus
}
#endif

#endif /* __V9_VIDEO_SDK_H__ */
