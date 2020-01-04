/*
 * v9_video.h
 *
 *  Created on: 2019年11月26日
 *      Author: DELL
 */

#ifndef __V9_VIDEO_H__
#define __V9_VIDEO_H__

#define V9_VIDEO_SDK_API

#ifdef V9_VIDEO_SDK_API
#include "EAIS_SDKApi.h"
#endif


/* board */

#define APP_BOARD_CALCU_ID(n)			((n)+1)

/* rtsp */
#define RTSP_PORT_DEFAULT			554

#define V9_APP_CHANNEL_MAX			32
#define V9_APP_BOARD_MAX			5


#define V9_APP_USERNAME_MAX		32
#define V9_APP_PASSWORD_MAX		32
#define V9_APP_VIDEO_URL_MAX		128



#define APP_BOARD_ADDRESS_PREFIX		0x0A0A0A00
#define APP_BOARD_ADDRESS_MAIN		254

enum
{
	APP_BOARD_MAIN	= 1,
	APP_BOARD_CALCU_1,
	APP_BOARD_CALCU_2,
	APP_BOARD_CALCU_3,
	APP_BOARD_CALCU_4,
};





int v9_app_module_init();
int v9_app_module_exit();
int v9_app_module_task_init(void);
int v9_app_module_task_exit(void);

#endif /* __V9_VIDEO_H__ */
