/*
 * v9_video.h
 *
 *  Created on: 2019年11月26日
 *      Author: DELL
 */

#ifndef __V9_VIDEO_H__
#define __V9_VIDEO_H__

#ifdef __cplusplus
extern "C" {
#endif

#define V9_VIDEO_SDK_API

#ifdef V9_VIDEO_SDK_API
#include "EAIS_SDKApi.h"
#endif

#define V9_VIDEO_APP_DEBUG
#define V9_VIDEO_APP_DEBUG_HW

#define V9_SQLDB_BASE

#define V9_SQLDB_TEST
//#define V9_DEBUGING_TEST
/* board */

#define V9_APP_BOARD_CALCU_ID(n)			((n)+1)
#define V9_APP_BOARD_HW_ID(n)				((n)-1)
#define V9_APP_BOARD_HW_CH(n)				((n)-1)
/* rtsp */
#define RTSP_PORT_DEFAULT			554

#define V9_APP_CHANNEL_MAX			32
#define V9_APP_BOARD_MAX			5


#ifdef EAIS_SDK_MAX_COMMON_LEN
#define APP_USERNAME_MAX EAIS_SDK_MAX_COMMON_LEN
#else
#define APP_USERNAME_MAX 64
#endif

#ifdef EAIS_SDK_MAX_PATH_LEN
#define APP_PATH_MAX EAIS_SDK_MAX_PATH_LEN
#else
#define APP_PATH_MAX 256
#endif

#ifdef EAIS_SDK_MAX_GROUP_NUM
#define APP_GROUP_MAX EAIS_SDK_MAX_GROUP_NUM
#else
#define APP_GROUP_MAX 10
#endif

#ifdef EAIS_SDK_USER_FACE_COMMENT_LEN
#define APP_USER_TEXT_MAX EAIS_SDK_USER_FACE_COMMENT_LEN
#else
#define APP_USER_TEXT_MAX 128
#endif

/*
#define V9_APP_USERNAME_MAX			32
#define V9_APP_PASSWORD_MAX			32
*/
#define V9_APP_VIDEO_URL_MAX		128
#define V9_APP_VIDEO_URL_PARAM_MAX	64

#define V9_APP_DIR_NAME_MAX		128

#define APP_BOARD_ADDRESS_PREFIX		0x0A0A0A00
#define APP_BOARD_ADDRESS_MAIN			254
#define APP_FEATURE_MAX				1024

#if defined( V9_VIDEO_APP_DEBUG)||defined( V9_VIDEO_APP_DEBUG_HW)
#define V9_DEBUG(format, ...) 			zlog_debug (MODULE_APP, format, ##__VA_ARGS__)
#define V9_EVENT_DEBUG(format, ...) 	zlog_debug (MODULE_APP, format, ##__VA_ARGS__)
#define V9_DB_DEBUG(format, ...) 		zlog_debug (MODULE_APP, format, ##__VA_ARGS__)
#define V9_SDK_DBGPRF(format, ...) 		zlog_debug (MODULE_APP, format, ##__VA_ARGS__)
#else
#define V9_DEBUG(format, ...)
#define V9_EVENT_DEBUG(format, ...)
#define V9_DB_DEBUG(format, ...)
#define V9_SDK_DBGPRF(format, ...)
#endif

/* serial */
#define V9_APP_DEBUG_EVENT		0X01
#define V9_APP_DEBUG_HEX		0X02
#define V9_APP_DEBUG_RECV		0X04
#define V9_APP_DEBUG_SEND		0X08
#define V9_APP_DEBUG_UPDATE		0X10
#define V9_APP_DEBUG_TIME		0X20
#define V9_APP_DEBUG_WEB		0X40
#define V9_APP_DEBUG_MSG		0X80
#define V9_APP_DEBUG_STATE		0X100
#define V9_APP_DEBUG_UCI		0X200
#define V9_APP_DEBUG_ERROR		0X400
#define V9_APP_DEBUG_WARN		0X800
/* board */
#define V9_APP_DEBUG_BOARD_EVENT	0X01
#define V9_APP_DEBUG_BOARD_STATE	0X02




#define WEB_USER_PRIVATE_INDEX 		0
#define WEB_CAPDB_PRIVATE_INDEX 	1
#define WEB_CAPDB_WARN_INDEX 		2

enum
{
	APP_BOARD_MAIN	= 1,
	APP_BOARD_CALCU_1,
	APP_BOARD_CALCU_2,
	APP_BOARD_CALCU_3,
	APP_BOARD_CALCU_4,
};

// 抓拍特征值
typedef struct
{
	int							feature_len;									// 特征值个数，长度=个数*sizoof(ospl_float)
	union
	{
		void*						ckey_data;
		ospl_float*						feature_data;
	}feature;																	// 特征值
	ospl_float						input_value;									// 相似度（输入）
	ospl_float						output_result;									// 相似度（输出）
	int							(*feature_memcmp)(ospl_float *, ospl_float *, int,  ospl_float *);
	ospl_bool						nomem;
}sql_snapfea_key;


extern int v9_app_snapfea_key_alloc(sql_snapfea_key *key, ospl_bool nomem);
extern int v9_app_snapfea_key_free(sql_snapfea_key *key);
extern char * v9_app_age_string(ospl_uint32 age);

extern int v9_sqldb_debug_api(ospl_bool enable, ospl_uint32 flag);
extern int v9_user_debug_api(ospl_bool enable, ospl_uint32 flag);
extern int v9_video_sdk_debug_api(ospl_bool enable, ospl_uint32 flag);
extern int v9_serial_debug_api(ospl_bool enable, ospl_uint32 flag);
extern int v9_video_debug_config(struct vty *vty, ospl_bool detail);

extern int v9_app_module_init();
extern int v9_app_module_exit();
extern int v9_app_module_task_init(void);
extern int v9_app_module_task_exit(void);

#ifdef __cplusplus
}
#endif

#endif /* __V9_VIDEO_H__ */
