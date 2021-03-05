/*
 * application.h
 *
 *  Created on: Dec 22, 2018
 *      Author: zhurish
 */

#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef APP_X5BA_MODULE
#define X5_B_A_DEBUG
#include "X5-B/mgt/x5_b_global.h"
#include "X5-B/mgt/x5_b_app.h"
#include "X5-B/mgt/x5_b_cmd.h"
#include "X5-B/mgt/x5_b_ctl.h"
#include "X5-B/mgt/x5_b_web.h"
#include "X5-B/mgt/x5_b_util.h"
#include "X5-B/mgt/x5b_dbase.h"
#include "X5-B/mgt/x5b_facecard.h"
#include "X5-B/mgt/x5_b_test.h"

extern void cmd_app_x5b_init(void);
#endif

#ifdef APP_V9_MODULE
#include "V9/v9_device.h"
#include "V9/v9_util.h"
#include "V9/v9_video.h"
#include "V9/v9_serial.h"
#include "V9/v9_slipnet.h"
#include "V9/v9_cmd.h"

#include "V9/v9_video_disk.h"
#include "V9/v9_user_db.h"
#include "V9/v9_video_db.h"

#include "V9/v9_board.h"
#include "V9/v9_video_sdk.h"
#include "V9/v9_video_user.h"
#include "V9/v9_video_board.h"
#include "V9/v9_video_api.h"

extern void cmd_app_v9_init(void);
#endif


extern int app_module_init(void);
extern int app_module_exit(void);

extern int app_module_task_init(void);
extern int app_module_task_exit(void);

extern void cmd_app_init(void);


#ifdef __cplusplus
}
#endif

#endif /* __APPLICATION_H__ */
