/*
 * cmd_v9_app.c
 *
 *  Created on: 2019年11月25日
 *      Author: DELL
 */




#include "zebra.h"
#include "vty.h"
#include "if.h"

#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "vrf.h"
#include "interface.h"
#include "template.h"

#include "application.h"


#ifdef APP_V9_MODULE



static int app_write_config(struct vty *vty, void *pVoid);

DEFUN (app_template,
		app_template_cmd,
		"template app video",
		"Template configure\n"
		"APP configure\n"
		"Video configure\n")
{
	//int ret = ERROR;
	template_t * temp = nsm_template_lookup_name (FALSE, "app video");
	if(temp)
	{
		vty->node = TEMPLATE_NODE;
		memset(vty->prompt, 0, sizeof(vty->prompt));
		sprintf(vty->prompt, "%s", temp->prompt);
		return CMD_SUCCESS;
	}
	else
	{
		temp = nsm_template_new (FALSE);
		if(temp)
		{
			temp->module = 0;
			strcpy(temp->name, "app video");
			strcpy(temp->prompt, "app-video"); /* (config-app-esp)# */
			temp->write_template = app_write_config;
			temp->pVoid = v9_video_app_tmp();
			nsm_template_install(temp, 0);

			vty->node = TEMPLATE_NODE;
			memset(vty->prompt, 0, sizeof(vty->prompt));
			sprintf(vty->prompt, "%s", temp->prompt);
			return CMD_SUCCESS;
		}
	}
	return CMD_WARNING;
}

DEFUN (no_app_template,
		no_app_template_cmd,
		"no template app video ",
		NO_STR
		"Template configure\n"
		"APP configure\n"
		"Video configure\n")
{
	template_t * temp = nsm_template_lookup_name (FALSE, "app video");
	if(temp)
	{
		//x5b_app_free();
		nsm_template_free(temp);
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}


DEFUN (v9_app_board_disabled,
	   v9_app_board_disabled_cmd,
		"video board <1-4> (enable|disable)",
		"Video Configure\n"
		"Board Configure\n"
		"Board ID\n"
		"Enable\n"
		"Disable\n")
{
	if(strstr(argv[1], "enable"))
		v9_video_board_disabled(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), FALSE);
	else
		v9_video_board_disabled(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), TRUE);
	return CMD_SUCCESS;
}



DEFUN (v9_app_stream_param,
	   v9_app_stream_param_cmd,
		"video stream board <1-4> channel <1-4> (mainstream|secondary) PARAM",
		"Video Configure\n"
		"Video Stream Configure\n"
		"Board Configure\n"
		"Board ID(0:dynamic)\n"
		"Channel Configure\n"
		"Channel ID(0:dynamic)\n"
		"Main Stream Configure\n"
		"Secondary Stream Configure\n"
		"Stream Param Value\n")
{//
	if(argv[2] && strstr(argv[2], "main"))
		v9_video_board_stream_update_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), argv[3], NULL);
	else
		v9_video_board_stream_update_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), NULL, argv[3]);
	return CMD_SUCCESS;
}


DEFUN (v9_app_stream_add,
	   v9_app_stream_add_cmd,
		"video stream board <1-4> channel <1-4> ip A.B.C.D port <1-65530> username USE password PASS fps <480-1080>",
		"Video Configure\n"
		"Video Stream Configure\n"
		"Board Configure\n"
		"Board ID(0:dynamic)\n"
		"Channel Configure\n"
		"Channel ID(0:dynamic)\n"
		IP_STR
		CMD_KEY_IPV4_HELP
		"Port Configure\n"
		"Port Value\n"
		"Username Configure\n"
		"Username Value\n"
		"Password Configure\n"
		"Password Value\n"
		"FPS Configure\n"
		"FPS Value\n")
{
	//vty_out(vty, "---------argc = %d ----------%s", argc, VTY_NEWLINE);

	if(!v9_video_board_isactive(V9_APP_BOARD_CALCU_ID(atoi(argv[0]))))
	{
		if(vty->type == VTY_TERM)
		{
			vty_out(vty, "Board ID %d is not active %s", V9_APP_BOARD_CALCU_ID(atoi(argv[0])), VTY_NEWLINE);
			return CMD_WARNING;
		}
	}

	if(argc == 7)
		v9_video_board_stream_add_api(atoi(argv[0]), atoi(argv[1]), ntohl(inet_addr(argv[2])), atoi(argv[3]), argv[4], argv[5], atoi(argv[6]), NULL, NULL);
	else if(argc == 6)
		v9_video_board_stream_add_api(atoi(argv[0]), atoi(argv[1]), ntohl(inet_addr(argv[2])), atoi(argv[3]), argv[4], argv[5], 1080, NULL, NULL);
	else if(argc == 5)
		v9_video_board_stream_add_api(atoi(argv[0]), atoi(argv[1]), ntohl(inet_addr(argv[2])), 0, argv[3], argv[4], 1080, NULL, NULL);

	return CMD_SUCCESS;
}


/*
DEFUN (v9_app_stream_add,
	   v9_app_stream_add_cmd,
		"video stream channel <1-4> ip A.B.C.D port <1-65530> username USE password PASS fps <480-1080>",
		"Video Configure\n"
		"Board Configure\n"
		"Board ID\n"
		"Enable\n"
		"Disable\n")
{
	vty_out(vty, "---------argc = %d ----------%s", argc, VTY_NEWLINE);
	if(argc == 6)
		v9_video_stream_add_api(0, atoi(argv[0]), ntohl(inet_addr(argv[1])), atoi(argv[2]), argv[3], argv[4], atoi(argv[5]));
	else if(argc == 5)
		v9_video_stream_add_api(0, atoi(argv[0]), ntohl(inet_addr(argv[1])), atoi(argv[2]), argv[3], argv[4], 1080);
	else if(argc == 4)
		v9_video_stream_add_api(0, 0, ntohl(inet_addr(argv[0])), atoi(argv[1]), argv[2], argv[3], 1080);

	return CMD_SUCCESS;
}*/

ALIAS(v9_app_stream_add,
	   v9_app_stream_add_1_cmd,
		"video stream board <1-4> channel <1-4> ip A.B.C.D port <1-65530> username USE password PASS",
		"Video Configure\n"
		"Video Stream Configure\n"
		"Board Configure\n"
		"Board ID(0:dynamic)\n"
		"Channel Configure\n"
		"Channel ID(0:dynamic)\n"
		IP_STR
		CMD_KEY_IPV4_HELP
		"Port Configure\n"
		"Port Value\n"
		"Username Configure\n"
		"Username Value\n"
		"Password Configure\n"
		"Password Value\n");

ALIAS(v9_app_stream_add,
	   v9_app_stream_add_2_cmd,
		"video stream board <1-4> channel <1-4> ip A.B.C.D username USE password PASS",
		"Video Configure\n"
		"Video Stream Configure\n"
		"Board Configure\n"
		"Board ID(0:dynamic)\n"
		"Channel Configure\n"
		"Channel ID(0:dynamic)\n"
		IP_STR
		CMD_KEY_IPV4_HELP
		"Username Configure\n"
		"Username Value\n"
		"Password Configure\n"
		"Password Value\n");




DEFUN (v9_app_board_active,
	   v9_app_board_active_cmd,
		"video board <1-4> (active|inactive)",
		"Video Configure\n"
		"Board Configure\n"
		"Board ID\n"
		"Active\n"
		"Inactive\n")
{
	if(strstr(argv[1], "enable"))
		v9_video_board_active(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), TRUE);
	else
		v9_video_board_active(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), FALSE);
	return CMD_SUCCESS;
}


/* show */
DEFUN (v9_app_board_show,
	   v9_app_board_show_cmd,
		"show video board status [detail]",
		SHOW_STR
		"Video Configure\n"
		"Board Configure\n"
		"Status Information\n")
{
	if(argc == 1)
		v9_video_board_show(vty, TRUE);
	else
		v9_board_show(vty, 0, TRUE);
	return CMD_SUCCESS;
}


DEFUN (v9_app_video_rtsp_show,
	   v9_app_video_rtsp_show_cmd,
		"show video rtsp information",
		SHOW_STR
		"Video Configure\n"
		"Video Stream Configure\n"
		"Status Information\n")
{
	if(argc == 1 && argv[0])
		v9_video_board_stream_show(vty, V9_APP_BOARD_CALCU_ID(atoi(argv[0])), TRUE);
	else
		v9_video_board_stream_show(vty, 0, TRUE);
	return CMD_SUCCESS;
}

ALIAS(v9_app_video_rtsp_show,
	  v9_app_video_rtsp_id_show_cmd,
		"show video <1-4> rtsp information",
		SHOW_STR
		"Video Configure\n"
		"Board ID\n"
		"Video Stream Configure\n"
		"Status Information\n");


DEFUN (v9_app_video_usergroup_show,
	   v9_app_video_usergroup_show_cmd,
		"show video usergroup information",
		SHOW_STR
		"Video Configure\n"
		"Video Stream Configure\n"
		"Status Information\n")
{
	if(argc == 1 && argv[0])
		v9_video_usergroup_show(vty, V9_APP_BOARD_CALCU_ID(atoi(argv[0])));
	else
		v9_video_usergroup_show(vty, 0);
	return CMD_SUCCESS;
}

ALIAS(v9_app_video_usergroup_show,
	  v9_app_video_usergroup_id_show_cmd,
		"show video <1-4> usergroup information",
		SHOW_STR
		"Video Configure\n"
		"Board ID\n"
		"Video Stream Configure\n"
		"Status Information\n");


DEFUN (v9_app_time_sync_cmd,
	   v9_app_time_sync_cmd_cmd,
		"sync time",
		"Sync Configure\n"
		"Time Information\n")
{
#ifdef V9_SLIPNET_ENABLE
	v9_cmd_sync_time_test();
#else
		if(v9_serial)
			v9_serial->timer_sync = 1;
#endif
	return CMD_SUCCESS;
}

DEFUN (v9_app_sdk_show,
	   v9_app_sdk_show_cmd,
		"show video sdk",
		SHOW_STR
		"Video Configure\n"
		"SDK Information\n")
{
	v9_video_sdk_show(vty, 0, TRUE);
	return CMD_SUCCESS;
}


DEFUN (v9_app_user_show,
	   v9_app_user_show_cmd,
		"show video user",
		SHOW_STR
		"Video Configure\n"
		"User Information\n")
{
	v9_video_user_show(vty, TRUE);
	return CMD_SUCCESS;
}


#ifdef V9_VIDEO_SDK_API
/*
 * test
 */
DEFUN (v9_sdk_hw_reboot,
	   v9_sdk_hw_reboot_cmd,
		"video sdk <1-4> reboot [reload]",
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Reboot\n"
		"Reset Configure\n")
{
	if(argc == 2)
	{
		if(v9_video_sdk_reset_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0]))) == OK)
		{
			return CMD_SUCCESS;
		}
	}
	else
	{
		if(v9_video_sdk_reboot_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0]))) == OK)
		{
			return CMD_SUCCESS;
		}
	}
	vty_out(vty, "get Video SDK User information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (v9_sdk_hw_update,
	   v9_sdk_hw_update_cmd,
		"video sdk <1-4> update FILENAME",
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Update Configure\n"
		"Filename\n")
{
	if(v9_video_sdk_update_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), argv[1]) == OK)
	{
		return CMD_SUCCESS;
	}
	vty_out(vty, "get Video SDK User information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}


DEFUN (v9_sdk_hw_original_pic_enable,
	   v9_sdk_hw_original_pic_enable_cmd,
		"video sdk <1-4> original picture (enable|disabled)",
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Original Configure\n"
		"Picture Configure\n"
		"Enable\n"
		"Disable\n")
{
	if(strstr(argv[1], "enable"))
	{
		if(v9_video_sdk_original_pic_enable_set_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), TRUE) == OK)
		{
			return CMD_SUCCESS;
		}
	}
	else
	{
		if(v9_video_sdk_original_pic_enable_set_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), FALSE) == OK)
		{
			return CMD_SUCCESS;
		}
	}
	vty_out(vty, "get Video SDK User information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (v9_sdk_hw_set_recognize,
	   v9_sdk_hw_set_recognize_cmd,
		"video sdk <1-4> recognize similarity <0-100> registerquality <0-100>",
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Recognize Configure\n"
		"Similarity Configure\n"
		"Similarity Value\n"
		"Registerquality Configure\n"
		"Registerquality Value\n")
{
	BOOL nOpenUpload = FALSE;
	if(v9_video_sdk_recognize_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), NULL, NULL, &nOpenUpload) == OK)
	{
		if(v9_video_sdk_recognize_config_set_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), atoi(argv[2]), nOpenUpload) == OK)
		{
			return CMD_SUCCESS;
		}
	}
	vty_out(vty, "get Video SDK User information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (v9_sdk_hw_set_recognize_openupload,
	   v9_sdk_hw_set_recognize_openupload_cmd,
		"video sdk <1-4> recognize openupload (enable|disabled)",
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Recognize Configure\n"
		"OpenUpload Configure\n"
		"Enable Upload\n"
		"Disable Upload\n")
{
	int nOutSimilarity = 0, nRegisterQuality = 0;
	BOOL nOpenUpload = FALSE;
	if(strstr(argv[1], "enable"))
		nOpenUpload = TRUE;
	if(v9_video_sdk_recognize_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), &nOutSimilarity, &nRegisterQuality, NULL) == OK)
	{
		if(v9_video_sdk_recognize_config_set_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), nOutSimilarity, nRegisterQuality, nOpenUpload) == OK)
		{
			return CMD_SUCCESS;
		}
	}
	vty_out(vty, "get Video SDK User information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (v9_sdk_hw_set_snapopen,
	   v9_sdk_hw_set_snapopen_cmd,
		"video sdk <1-4> snap (open|close)",
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Snap Configure\n"
		"Enable\n"
		"Disable\n")
{
	if(strstr(argv[1], "open"))
	{
		if(v9_video_sdk_open_snap_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), 1) == OK)
		{
			return CMD_SUCCESS;
		}
	}
	else
	{
		if(v9_video_sdk_close_snap_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0]))) == OK)
		{
			return CMD_SUCCESS;
		}
	}
	vty_out(vty, "get Video SDK User information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}

#if 0
/*
 * helmet 安全帽
 */
DEFUN (v9_sdk_hw_set_helmet_sentimage,
	   v9_sdk_hw_set_helmet_sentimage_cmd,
		"video sdk <1-4> channel <1-4> helmet sentimage (enable|disabled)",
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Channel Configure\n"
		"Channel ID\n"
		"Helmet Configure\n"
		"Sent Image Configure\n"
		"Enable Sent Image\n"
		"Disable Sent Image\n")
{
	ST_SDKHelmetInfo HelmetInfo;
	memset(&HelmetInfo, 0, sizeof(ST_SDKHelmetInfo));
	if(v9_video_sdk_helmet_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &HelmetInfo) == OK)
	{
		HelmetInfo.nSentImage = strstr(argv[2], "enable") ? TRUE:FALSE;
		if(v9_video_sdk_helmet_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &HelmetInfo) == OK)
		{
			return CMD_SUCCESS;
		}
	}
	vty_out(vty, "get/set Video SDK helmet sentimage information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (v9_sdk_hw_set_helmet_tracking,
	   v9_sdk_hw_set_helmet_tracking_cmd,
		"video sdk <1-4> channel <1-4> helmet tracking (enable|disabled)",
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Channel Configure\n"
		"Channel ID\n"
		"Helmet Configure\n"
		"Tracking Configure\n"
		"Enable Tracking\n"
		"Disable Tracking\n")
{
	ST_SDKHelmetInfo HelmetInfo;
	memset(&HelmetInfo, 0, sizeof(ST_SDKHelmetInfo));
	if(v9_video_sdk_helmet_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &HelmetInfo) == OK)
	{
		HelmetInfo.nUseTracking = strstr(argv[2], "enable") ? TRUE:FALSE;
		if(v9_video_sdk_helmet_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &HelmetInfo) == OK)
		{
			return CMD_SUCCESS;
		}
	}
	vty_out(vty, "get/set Video SDK helmet tracking information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (v9_sdk_hw_set_helmet_drawrectangle,
	   v9_sdk_hw_set_helmet_drawrectangle_cmd,
		"video sdk <1-4> channel <1-4> helmet drawrectangle (enable|disabled)",
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Channel Configure\n"
		"Channel ID\n"
		"Helmet Configure\n"
		"DrawRectangle Configure\n"
		"Enable DrawRectangle\n"
		"Disable DrawRectangle\n")
{
	ST_SDKHelmetInfo HelmetInfo;
	memset(&HelmetInfo, 0, sizeof(ST_SDKHelmetInfo));
	if(v9_video_sdk_helmet_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &HelmetInfo) == OK)
	{
		HelmetInfo.nDrawRectangle = strstr(argv[2], "enable") ? TRUE:FALSE;
		if(v9_video_sdk_helmet_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &HelmetInfo) == OK)
		{
			return CMD_SUCCESS;
		}
	}
	vty_out(vty, "get/set Video SDK helmet DrawRectangle information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}


DEFUN (v9_sdk_hw_set_helmet_imageratio,
	   v9_sdk_hw_set_helmet_imageratio_cmd,
		"video sdk <1-4> channel <1-4> helmet imageratio <0-100>",
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Channel Configure\n"
		"Channel ID\n"
		"Helmet Configure\n"
		"ImageRatio Configure\n"
		"ImageRatio Value\n")
{
	ST_SDKHelmetInfo HelmetInfo;
	memset(&HelmetInfo, 0, sizeof(ST_SDKHelmetInfo));
	if(v9_video_sdk_helmet_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &HelmetInfo) == OK)
	{
		HelmetInfo.nImageRatio = atoi(argv[2]);
		if(v9_video_sdk_helmet_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &HelmetInfo) == OK)
		{
			return CMD_SUCCESS;
		}
	}
	vty_out(vty, "get/set Video SDK helmet ImageRatio information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (v9_sdk_hw_set_helmet_snapinterval,
	   v9_sdk_hw_set_helmet_snapinterval_cmd,
		"video sdk <1-4> channel <1-4> helmet snapinterval <0-100>",
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Channel Configure\n"
		"Channel ID\n"
		"Helmet Configure\n"
		"SnapInterval Configure\n"
		"SnapInterval Value\n")
{
	ST_SDKHelmetInfo HelmetInfo;
	memset(&HelmetInfo, 0, sizeof(ST_SDKHelmetInfo));
	if(v9_video_sdk_helmet_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &HelmetInfo) == OK)
	{
		HelmetInfo.nSnapInterval = atoi(argv[2]);
		if(v9_video_sdk_helmet_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &HelmetInfo) == OK)
		{
			return CMD_SUCCESS;
		}
	}
	vty_out(vty, "get/set Video SDK helmet SnapInterval information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (v9_sdk_hw_set_helmet_alarminterval,
	   v9_sdk_hw_set_helmet_alarminterval_cmd,
		"video sdk <1-4> channel <1-4> helmet alarminterval <0-100>",
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Channel Configure\n"
		"Channel ID\n"
		"Helmet Configure\n"
		"AlarmInterval Configure\n"
		"AlarmInterval Value\n")
{
	ST_SDKHelmetInfo HelmetInfo;
	memset(&HelmetInfo, 0, sizeof(ST_SDKHelmetInfo));
	if(v9_video_sdk_helmet_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &HelmetInfo) == OK)
	{
		HelmetInfo.nAlarmInterval = atoi(argv[2]);
		if(v9_video_sdk_helmet_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &HelmetInfo) == OK)
		{
			return CMD_SUCCESS;
		}
	}
	vty_out(vty, "get/set Video SDK helmet AlarmInterval information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}


DEFUN (v9_sdk_hw_set_helmet_snapratio,
	   v9_sdk_hw_set_helmet_snapratio_cmd,
		"video sdk <1-4> channel <1-4> helmet snapratio <0-100>",
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Channel Configure\n"
		"Channel ID\n"
		"Helmet Configure\n"
		"SnapRatio Configure\n"
		"SnapRatio Value\n")
{
	ST_SDKHelmetInfo HelmetInfo;
	memset(&HelmetInfo, 0, sizeof(ST_SDKHelmetInfo));
	if(v9_video_sdk_helmet_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &HelmetInfo) == OK)
	{
		HelmetInfo.nSnapRatio = atoi(argv[2]);
		if(v9_video_sdk_helmet_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &HelmetInfo) == OK)
		{
			return CMD_SUCCESS;
		}
	}
	vty_out(vty, "get/set Video SDK helmet SnapRatio information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (v9_sdk_hw_set_helmet_threshold,
	   v9_sdk_hw_set_helmet_threshold_cmd,
		"video sdk <1-4> channel <1-4> helmet threshold <0-100>",
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Channel Configure\n"
		"Channel ID\n"
		"Helmet Configure\n"
		"Threshold Configure\n"
		"Threshold Value\n")
{
	ST_SDKHelmetInfo HelmetInfo;
	memset(&HelmetInfo, 0, sizeof(ST_SDKHelmetInfo));
	if(v9_video_sdk_helmet_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &HelmetInfo) == OK)
	{
		HelmetInfo.nThreshold = atoi(argv[2]);
		if(v9_video_sdk_helmet_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &HelmetInfo) == OK)
		{
			return CMD_SUCCESS;
		}
	}
	vty_out(vty, "get/set Video SDK helmet Threshold information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}
#endif

/*
 * sanp 抓拍
 */
DEFUN (v9_sdk_hw_set_sanp_confi,
	   v9_sdk_hw_set_sanp_confi_cmd,
		"video sdk <1-4> channel <1-4> sanp confi <0-100>",
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Channel Configure\n"
		"Channel ID\n"
		"Sanp Configure\n"
		"Confi Configure\n"
		"Confi Value\n")
{
	ST_SDKSnapInfo pstSnapInfo;
	memset(&pstSnapInfo, 0, sizeof(ST_SDKSnapInfo));
	if(v9_video_sdk_snap_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &pstSnapInfo) == OK)
	{
		pstSnapInfo.nConfi = atoi(argv[2]);
		if(v9_video_sdk_snap_config_set_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &pstSnapInfo) == OK)
		{
			return CMD_SUCCESS;
		}
	}
	vty_out(vty, "get/set Video SDK Sanp Confi information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}


DEFUN (v9_sdk_hw_set_sanp_qulityscore,
	   v9_sdk_hw_set_sanp_qulityscore_cmd,
		"video sdk <1-4> channel <1-4> sanp qulityscore <0-100>",
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Channel Configure\n"
		"Channel ID\n"
		"Sanp Configure\n"
		"QulityScore Configure\n"
		"QulityScore Value\n")
{
	ST_SDKSnapInfo pstSnapInfo;
	memset(&pstSnapInfo, 0, sizeof(ST_SDKSnapInfo));
	if(v9_video_sdk_snap_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &pstSnapInfo) == OK)
	{
		pstSnapInfo.nQulityScore = atoi(argv[2]);
		if(v9_video_sdk_snap_config_set_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &pstSnapInfo) == OK)
		{
			return CMD_SUCCESS;
		}
	}
	vty_out(vty, "get/set Video SDK Sanp QulityScore information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (v9_sdk_hw_set_sanp_attr,
	   v9_sdk_hw_set_sanp_attr_cmd,
		"video sdk <1-4> channel <1-4> sanp attr (enable|disabled)",
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Channel Configure\n"
		"Channel ID\n"
		"Sanp Configure\n"
		"Attr Configure\n"
		"Enable Attr\n"
		"Disable Attr\n")
{
	ST_SDKSnapInfo pstSnapInfo;
	memset(&pstSnapInfo, 0, sizeof(ST_SDKSnapInfo));
	if(v9_video_sdk_snap_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &pstSnapInfo) == OK)
	{
		pstSnapInfo.nAttrEnable = strstr(argv[2], "enable") ? TRUE:FALSE;
		if(v9_video_sdk_snap_config_set_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &pstSnapInfo) == OK)
		{
			return CMD_SUCCESS;
		}
	}
	vty_out(vty, "get/set Video SDK Sanp AttrEnable information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}


DEFUN (v9_sdk_hw_set_sanp_feature,
	   v9_sdk_hw_set_sanp_feature_cmd,
		"video sdk <1-4> channel <1-4> sanp feature (enable|disabled)",
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Channel Configure\n"
		"Channel ID\n"
		"Sanp Configure\n"
		"Feature Configure\n"
		"Enable Feature\n"
		"Disable Feature\n")
{
	ST_SDKSnapInfo pstSnapInfo;
	memset(&pstSnapInfo, 0, sizeof(ST_SDKSnapInfo));
	if(v9_video_sdk_snap_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &pstSnapInfo) == OK)
	{
		pstSnapInfo.nFeatureEnable = strstr(argv[2], "enable") ? TRUE:FALSE;
		if(v9_video_sdk_snap_config_set_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &pstSnapInfo) == OK)
		{
			return CMD_SUCCESS;
		}
	}
	vty_out(vty, "get/set Video SDK Sanp FeatureEnable information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (v9_sdk_hw_set_sanp_align,
	   v9_sdk_hw_set_sanp_align_cmd,
		"video sdk <1-4> channel <1-4> sanp align (enable|disabled)",
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Channel Configure\n"
		"Channel ID\n"
		"Sanp Configure\n"
		"Align Configure\n"
		"Enable Align\n"
		"Disable Align\n")
{
	ST_SDKSnapInfo pstSnapInfo;
	memset(&pstSnapInfo, 0, sizeof(ST_SDKSnapInfo));
	if(v9_video_sdk_snap_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &pstSnapInfo) == OK)
	{
		pstSnapInfo.nAlignEnable = strstr(argv[2], "enable") ? TRUE:FALSE;
		if(v9_video_sdk_snap_config_set_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &pstSnapInfo) == OK)
		{
			return CMD_SUCCESS;
		}
	}
	vty_out(vty, "get/set Video SDK Sanp AlignEnable information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (v9_sdk_hw_set_sanp_area,
	   v9_sdk_hw_set_sanp_area_cmd,
		"video sdk <1-4> channel <1-4> sanp area (enable|disabled)",
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Channel Configure\n"
		"Channel ID\n"
		"Sanp Configure\n"
		"Area Configure\n"
		"Enable Area\n"
		"Disable Area\n")
{
	ST_SDKSnapInfo pstSnapInfo;
	memset(&pstSnapInfo, 0, sizeof(ST_SDKSnapInfo));
	if(v9_video_sdk_snap_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &pstSnapInfo) == OK)
	{
		pstSnapInfo.nAreaEnable = strstr(argv[2], "enable") ? TRUE:FALSE;
		if(v9_video_sdk_snap_config_set_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &pstSnapInfo) == OK)
		{
			return CMD_SUCCESS;
		}
	}
	vty_out(vty, "get/set Video SDK Sanp AreaEnable information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}


DEFUN (v9_sdk_hw_set_sanp_tripwire,
	   v9_sdk_hw_set_sanp_tripwire_cmd,
		"video sdk <1-4> channel <1-4> sanp tripwire (enable|disabled)",
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Channel Configure\n"
		"Channel ID\n"
		"Sanp Configure\n"
		"TripWire Configure\n"
		"Enable TripWire\n"
		"Disable TripWire\n")
{
	ST_SDKSnapInfo pstSnapInfo;
	memset(&pstSnapInfo, 0, sizeof(ST_SDKSnapInfo));
	if(v9_video_sdk_snap_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &pstSnapInfo) == OK)
	{
		pstSnapInfo.nTripWireEnable = strstr(argv[2], "enable") ? TRUE:FALSE;
		if(v9_video_sdk_snap_config_set_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &pstSnapInfo) == OK)
		{
			return CMD_SUCCESS;
		}
	}
	vty_out(vty, "get/set Video SDK Sanp TripWireEnable information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (v9_sdk_hw_set_sanp_snapmode,
	   v9_sdk_hw_set_sanp_snapmode_cmd,
		"video sdk <1-4> channel <1-4> sanp snapmode (disabled|realtime|leave|timing|into)",
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Channel Configure\n"
		"Channel ID\n"
		"Sanp Configure\n"
		"SnapMode Configure\n"
		"Enable TripWire\n"
		"Disable TripWire\n")
{
	ST_SDKSnapInfo pstSnapInfo;
	memset(&pstSnapInfo, 0, sizeof(ST_SDKSnapInfo));
	if(v9_video_sdk_snap_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &pstSnapInfo) == OK)
	{
		if(strstr(argv[2], "disabled"))
			pstSnapInfo.nSnapMode = 0;
		else if(strstr(argv[2], "realtime"))
			pstSnapInfo.nSnapMode = 1;
		else if(strstr(argv[2], "leave"))
			pstSnapInfo.nSnapMode = 2;
		else if(strstr(argv[2], "timing"))
			pstSnapInfo.nSnapMode = 3;
		else if(strstr(argv[2], "into"))
			pstSnapInfo.nSnapMode = 4;
		if(v9_video_sdk_snap_config_set_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &pstSnapInfo) == OK)
		{
			return CMD_SUCCESS;
		}
	}
	vty_out(vty, "get/set Video SDK Sanp SnapMode information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}


DEFUN (v9_sdk_hw_set_sanp_intervaltime,
	   v9_sdk_hw_set_sanp_intervaltime_cmd,
		"video sdk <1-4> channel <1-4> sanp intervaltime <0-100>",
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Channel Configure\n"
		"Channel ID\n"
		"Sanp Configure\n"
		"IntervalTime Configure\n"
		"IntervalTime Value\n")
{
	ST_SDKSnapInfo pstSnapInfo;
	memset(&pstSnapInfo, 0, sizeof(ST_SDKSnapInfo));
	if(v9_video_sdk_snap_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &pstSnapInfo) == OK)
	{
		pstSnapInfo.nIntervalTime = atoi(argv[2]);
		if(v9_video_sdk_snap_config_set_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &pstSnapInfo) == OK)
		{
			return CMD_SUCCESS;
		}
	}
	vty_out(vty, "get/set Video SDK Sanp IntervalTime information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (v9_sdk_hw_set_sanp_maxsize,
	   v9_sdk_hw_set_sanp_maxsize_cmd,
		"video sdk <1-4> channel <1-4> sanp maxsize <0-100>",
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Channel Configure\n"
		"Channel ID\n"
		"Sanp Configure\n"
		"MaxSize Configure\n"
		"MaxSize Value\n")
{
	ST_SDKSnapInfo pstSnapInfo;
	memset(&pstSnapInfo, 0, sizeof(ST_SDKSnapInfo));
	if(v9_video_sdk_snap_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &pstSnapInfo) == OK)
	{
		pstSnapInfo.nMaxSize = atoi(argv[2]);
		if(v9_video_sdk_snap_config_set_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &pstSnapInfo) == OK)
		{
			return CMD_SUCCESS;
		}
	}
	vty_out(vty, "get/set Video SDK Sanp MaxSize information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (v9_sdk_hw_set_sanp_minsize,
	   v9_sdk_hw_set_sanp_minsize_cmd,
		"video sdk <1-4> channel <1-4> sanp minsize <0-100>",
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Channel Configure\n"
		"Channel ID\n"
		"Sanp Configure\n"
		"MinSize Configure\n"
		"MinSize Value\n")
{
	ST_SDKSnapInfo pstSnapInfo;
	memset(&pstSnapInfo, 0, sizeof(ST_SDKSnapInfo));
	if(v9_video_sdk_snap_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &pstSnapInfo) == OK)
	{
		pstSnapInfo.nMinSize = atoi(argv[2]);
		if(v9_video_sdk_snap_config_set_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &pstSnapInfo) == OK)
		{
			return CMD_SUCCESS;
		}
	}
	vty_out(vty, "get/set Video SDK Sanp MinSize information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}



/****************************************************/

DEFUN (v9_sdk_hw_get_helmet,
	   v9_sdk_hw_get_helmet_cmd,
		"show video sdk <1-4> helmet channel <1-4>",
		SHOW_STR
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Helmet Configure\n"
		"Channel Configure\n"
		"Channel ID\n")
{
	ST_SDKHelmetInfo HelmetInfo;
	memset(&HelmetInfo, 0, sizeof(ST_SDKHelmetInfo));
#if 0
	if(v9_video_sdk_helmet_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &HelmetInfo) == OK)
	{
		vty_out(vty, " nChannelId              :%d%s", HelmetInfo.nChannelId, VTY_NEWLINE);
		vty_out(vty, " nSentImage              :%d%s", HelmetInfo.nSentImage, VTY_NEWLINE);
		vty_out(vty, " nImageRatio             :%d%s", HelmetInfo.nImageRatio, VTY_NEWLINE);
		vty_out(vty, " nUseTracking            :%s%s", HelmetInfo.nUseTracking ? "Enable":"Disable", VTY_NEWLINE);
		vty_out(vty, " nDrawRectangle          :%s%s", HelmetInfo.nDrawRectangle ? "Enable":"Disable", VTY_NEWLINE);
		vty_out(vty, " nSnapInterval           :%d%s", HelmetInfo.nSnapInterval, VTY_NEWLINE);
		vty_out(vty, " nAlarmInterval          :%d%s", HelmetInfo.nAlarmInterval, VTY_NEWLINE);
		vty_out(vty, " nSnapRatio              :%d%s", HelmetInfo.nSnapRatio, VTY_NEWLINE);

		vty_out(vty, " nUploadMode             :%d%s", HelmetInfo.nUploadMode, VTY_NEWLINE);
		vty_out(vty, " nThreshold              :%d%s", HelmetInfo.nThreshold, VTY_NEWLINE);
		return CMD_SUCCESS;
	}
#endif
	vty_out(vty, "get Video SDK helmet information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}


DEFUN (v9_sdk_hw_get_snapinfo,
	   v9_sdk_hw_get_snapinfo_cmd,
		"show video sdk <1-4> snapinfo channel <1-4>",
		SHOW_STR
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Snapinfo Configure\n"
		"Channel Configure\n"
		"Channel ID\n")
{
	ST_SDKSnapInfo stSnapInfo;
	memset(&stSnapInfo, 0, sizeof(ST_SDKSnapInfo));
	if(v9_video_sdk_snap_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), atoi(argv[1]), &stSnapInfo) == OK)
	{
		vty_out(vty, " nChannelId              :%d%s", stSnapInfo.nChannelId, VTY_NEWLINE);
		vty_out(vty, " nConfi                  :%d%s", stSnapInfo.nConfi, VTY_NEWLINE);
		vty_out(vty, " nQulityScore            :%d%s", stSnapInfo.nQulityScore, VTY_NEWLINE);
		vty_out(vty, " nAttrEnable             :%s%s", stSnapInfo.nAttrEnable ? "Enable":"Disable", VTY_NEWLINE);
		vty_out(vty, " nFeatureEnable          :%s%s", stSnapInfo.nFeatureEnable ? "Enable":"Disable", VTY_NEWLINE);
		vty_out(vty, " nAlignEnable            :%s%s", stSnapInfo.nAlignEnable ? "Enable":"Disable", VTY_NEWLINE);
		vty_out(vty, " nAreaEnable             :%s%s", stSnapInfo.nAreaEnable ? "Enable":"Disable", VTY_NEWLINE);
		vty_out(vty, " nTripWireEnable         :%s%s", stSnapInfo.nTripWireEnable ? "Enable":"Disable", VTY_NEWLINE);
		//vty_out(vty, " nFeatureEnable        :%s%s", stSnapInfo.nFeatureEnable ? "Enable":"Disable", VTY_NEWLINE);

		vty_out(vty, " nSnapMode               :%d%s", stSnapInfo.nSnapMode, VTY_NEWLINE);
		vty_out(vty, " nIntervalTime           :%d%s", stSnapInfo.nIntervalTime, VTY_NEWLINE);
		vty_out(vty, " nMaxSize                :%d%s", stSnapInfo.nMaxSize, VTY_NEWLINE);

		vty_out(vty, " nMinSize                :%d%s", stSnapInfo.nMinSize, VTY_NEWLINE);
		vty_out(vty, " nAreaConfigWidth        :%d%s", stSnapInfo.nAreaConfigWidth, VTY_NEWLINE);
		vty_out(vty, " nAreaConfigHeight       :%d%s", stSnapInfo.nAreaConfigHeight, VTY_NEWLINE);
		vty_out(vty, " nTripConfigWidth        :%d%s", stSnapInfo.nTripConfigWidth, VTY_NEWLINE);
		vty_out(vty, " nTripConfigHeight       :%d%s", stSnapInfo.nTripConfigHeight, VTY_NEWLINE);
		//vty_out(vty, " nAreaConfigWidth              :%s%s", stSnapInfo.nAreaConfigWidth, VTY_NEWLINE);


		return CMD_SUCCESS;
	}
	vty_out(vty, "get Video SDK SnapInfo information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}


DEFUN (v9_sdk_hw_get_recognize,
	   v9_sdk_hw_get_recognize_cmd,
		"show video sdk <1-4> recognize",
		SHOW_STR
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Recognize Configure\n")
{
	int nOutSimilarity = 0, nRegisterQuality = 0;
	BOOL nOpenUpload = FALSE;
	if(v9_video_sdk_recognize_config_get_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), &nOutSimilarity, &nRegisterQuality, &nOpenUpload) == OK)
	{
		vty_out(vty, " nOutSimilarity              :%d%s", nOutSimilarity, VTY_NEWLINE);
		vty_out(vty, " nRegisterQuality            :%d%s", nRegisterQuality, VTY_NEWLINE);
		vty_out(vty, " nOpenUpload                 :%s%s", nOpenUpload ? "Enable":"Disable", VTY_NEWLINE);

		return CMD_SUCCESS;
	}
	vty_out(vty, "get Video SDK recognize information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}



/*
 * 黑白名单
 */

/*
int v9_video_sdk_add_user_api(u_int32 id, BOOL gender, int group, char *user, char *ID, char *pic, BOOL edit);
int v9_video_sdk_del_user_api(u_int32 id, char *ID);
int v9_video_sdk_del_group_user_api(u_int32 id, void *p_pstUserList);
int v9_video_sdk_del_group_api(u_int32 id, int group);
int v9_video_sdk_get_user_api(u_int32 id, char* ID, void* UserInfo);
*/
DEFUN (v9_sdk_hw_clean_user,
	   v9_sdk_hw_clean_user_cmd,
		"video sdk <1-4> clean alluser",
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Clean Configure\n"
		"All User Configure\n")
{
	v9_video_user_clean ();
	if(v9_video_sdk_del_group_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), -1) == OK)
	{
		return CMD_SUCCESS;
	}
	vty_out(vty, "Clean Video SDK User information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (v9_sdk_hw_clean_rtsp,
	   v9_sdk_hw_clean_rtsp_cmd,
		"video sdk <1-4> clean allrtsp",
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Clean Configure\n"
		"All RTSP Configure\n")
{
	v9_video_board_stream_cleanup_api();
	if(v9_video_sdk_del_vch_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), -1) == OK)
	{
		return CMD_SUCCESS;
	}
	vty_out(vty, "Clean Video SDK RTSP information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (v9_sdk_hw_del_user,
	   v9_sdk_hw_del_user_cmd,
		"video sdk <1-4> del username USERID",
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Delete Configure\n"
		"Username Configure\n"
		"User ID\n")
{
	if(v9_video_sdk_del_user_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), 0, argv[1]) == OK)
	{
		return CMD_SUCCESS;
	}
	vty_out(vty, "del Video SDK User information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (v9_sdk_hw_get_user,
	   v9_sdk_hw_get_user_cmd,
		"show video sdk <1-4> username USERID",
		SHOW_STR
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n"
		"Username Configure\n"
		"User ID\n")
{
	ST_SDKUserInfo pstUserInfo;
	memset(&pstUserInfo, 0, sizeof(ST_SDKUserInfo));
	if(v9_video_sdk_get_user_api(V9_APP_BOARD_CALCU_ID(atoi(argv[0])), argv[1], &pstUserInfo) == OK)
	{
		vty_out(vty, " GroupID               :%d%s", pstUserInfo.nGroupID, VTY_NEWLINE);
		vty_out(vty, " UserName              :%s%s", pstUserInfo.szUserName, VTY_NEWLINE);
		vty_out(vty, " UserID                :%s%s", pstUserInfo.szUserID, VTY_NEWLINE);
		vty_out(vty, " Gender                :%s%s", pstUserInfo.nGender ? "Man":"Woman", VTY_NEWLINE);
/*
		vty_out(vty, " GroupID               :%d%s", pstUserInfo.nGroupID, VTY_NEWLINE);
		vty_out(vty, " GroupID               :%d%s", pstUserInfo.nGroupID, VTY_NEWLINE);
		vty_out(vty, " GroupID               :%d%s", pstUserInfo.nGroupID, VTY_NEWLINE);
		vty_out(vty, " GroupID               :%d%s", pstUserInfo.nGroupID, VTY_NEWLINE);
		vty_out(vty, " GroupID               :%d%s", pstUserInfo.nGroupID, VTY_NEWLINE);
		int							nGroupID;										// 所属组ID  0： 黑名单 1： 白名单
		char						szUserName[EAIS_SDK_MAX_COMMON_LEN];			// 姓名
		char						szUserID[EAIS_SDK_MAX_COMMON_LEN];				// 证件号
		int							nGender;										// 人员性别  0： 女 1： 男
		int        			        nFaceLen;										// 人脸图片数据长度（1, 1024 * 1024]字节
		unsigned char*				szPictureData;									// 图片内容
		char						szComment[EAIS_SDK_USER_FACE_COMMENT_LEN];		// 用户自定义备注字段
		int							nRegisterTime;									// 注册时间，从1970-01-01 00:00:00 (utc) 开始计时的秒数
		char						szReserved[256];								// 预留位，便于拓展，默认置空
*/

		return CMD_SUCCESS;
	}
	vty_out(vty, "get Video SDK User information ERROR%s", VTY_NEWLINE);
	return CMD_WARNING;
}


#ifdef V9_SQLDB_TEST
DEFUN (v9_sdk_sqldb_user,
	   v9_sdk_sqldb_user_cmd,
		"video sqldb-test <0-3> ",
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n")
{
	v9_user_sqldb_test(atoi(argv[0]), vty);
	return CMD_SUCCESS;
}

DEFUN (v9_sdk_sqldb_cap_tbl,
	   v9_sdk_sqldb_cap_tbl_cmd,
		"video sqldb-tbl <0-1>",
		"Video Configure\n"
		"SDK Configure\n"
		"Board ID Value\n")
{
	v9_video_sqldb_test(1, atoi(argv[0]), vty);
	return CMD_SUCCESS;
}
#ifdef V9_SLIPNET_ENABLE
DEFUN (v9_led_ctl,
	   v9_led_ctl_cmd,
		"video led-status <0-1>",
		"Video Configure\n"
		"led Configure\n"
		"Status Value\n")
{
	if(v9_serial)
	v9_cmd_sync_led(v9_serial, 1, atoi(argv[0]));
	return CMD_SUCCESS;
}
#endif
#endif

DEFUN (v9_app_sdk_show_config,
	   v9_app_sdk_show_config_cmd,
		"show video sdk config",
		SHOW_STR
		"Video Configure\n"
		"SDK Information\n")
{
	v9_video_sdk_config_show(vty);
	return CMD_SUCCESS;
}

#endif

DEFUN (debug_video_event,
	   debug_video_event_cmd,
       "debug video (user|serial|sdk) (event|error|warn)",
       DEBUG_STR
       "Video configuration\n"
       "User Debug Configuration\n"
       "Serial Debug Configuration\n"
       "SDK Debug Configuration\n"
       "Debug option set event\n"
       "Debug option set error\n"
       "Debug option set warn\n")
{
  if (strncmp ("user", argv[0], strlen (argv[0])) == 0)
  {
	  if (strncmp ("event", argv[1], strlen (argv[1])) == 0)
		  v9_user_debug_api(TRUE, V9_USER_DEBUG_EVENT);
	  if (strncmp ("error", argv[1], strlen (argv[1])) == 0)
		  v9_user_debug_api(TRUE, V9_USER_DEBUG_ERROR);
	  if (strncmp ("warn", argv[1], strlen (argv[1])) == 0)
		  v9_user_debug_api(TRUE, V9_USER_DEBUG_WARN);
  }
  else if (strncmp ("serial", argv[0], strlen (argv[0])) == 0)
  {
	  if (strncmp ("event", argv[1], strlen (argv[1])) == 0)
		  v9_serial_debug_api(TRUE, V9_APP_DEBUG_EVENT);
	  if (strncmp ("error", argv[1], strlen (argv[1])) == 0)
		  v9_serial_debug_api(TRUE, V9_APP_DEBUG_ERROR);
	  if (strncmp ("warn", argv[1], strlen (argv[1])) == 0)
		  v9_serial_debug_api(TRUE, V9_APP_DEBUG_WARN);
  }
  else if (strncmp ("sdk", argv[0], strlen (argv[0])) == 0)
  {
	  if (strncmp ("event", argv[1], strlen (argv[1])) == 0)
		  v9_video_sdk_debug_api(TRUE, V9_SDK_DEBUG_EVENT);
	  if (strncmp ("error", argv[1], strlen (argv[1])) == 0)
		  v9_video_sdk_debug_api(TRUE, V9_SDK_DEBUG_ERROR);
	  if (strncmp ("warn", argv[1], strlen (argv[1])) == 0)
		  v9_video_sdk_debug_api(TRUE, V9_SDK_DEBUG_WARN);
  }
  return CMD_SUCCESS;
}

DEFUN (debug_video_sqldb,
	   debug_video_sqldb_cmd,
       "debug video sqldb (msg|db|state|sqlcmd)",
       DEBUG_STR
       "Video configuration\n"
       "Sqldb Debug Configuration\n"
       "Debug option set msg\n"
       "Debug option set db\n"
       "Debug option set state\n"
	   "Debug option set sqlcmd\n")
{
	  if (strncmp ("msg", argv[1], strlen (argv[1])) == 0)
		  v9_sqldb_debug_api(TRUE, V9_USER_DEBUG_EVENT);
	  if (strncmp ("db", argv[1], strlen (argv[1])) == 0)
		  v9_sqldb_debug_api(TRUE, V9_USER_DEBUG_ERROR);
	  if (strncmp ("state", argv[1], strlen (argv[1])) == 0)
		  v9_sqldb_debug_api(TRUE, V9_USER_DEBUG_WARN);
	  if (strncmp ("sqlcmd", argv[1], strlen (argv[1])) == 0)
		  v9_sqldb_debug_api(TRUE, V9_USER_DEBUG_WARN);
  return CMD_SUCCESS;
}

DEFUN (debug_video_appsdk,
	   debug_video_appsdk_cmd,
       "debug video (sreial|sdk) (msg|web|state)",
       DEBUG_STR
       "Video configuration\n"
       "Serial Debug Configuration\n"
       "SDK Debug Configuration\n"
       "Debug option set msg\n"
       "Debug option set web\n"
       "Debug option set state\n")
{
	  if (strncmp ("sdk", argv[0], strlen (argv[0])) == 0)
	  {
		  if (strncmp ("msg", argv[1], strlen (argv[1])) == 0)
			  v9_video_sdk_debug_api(TRUE, V9_SDK_DEBUG_MSG);
		  if (strncmp ("web", argv[1], strlen (argv[1])) == 0)
			  v9_video_sdk_debug_api(TRUE, V9_SDK_DEBUG_WEB);
		  if (strncmp ("state", argv[1], strlen (argv[1])) == 0)
			  v9_video_sdk_debug_api(TRUE, V9_SDK_DEBUG_STATE);
	  }
	  else if (strncmp ("serial", argv[0], strlen (argv[0])) == 0)
	  {
		  if (strncmp ("msg", argv[1], strlen (argv[1])) == 0)
			  v9_serial_debug_api(TRUE, V9_APP_DEBUG_MSG);
		  if (strncmp ("web", argv[1], strlen (argv[1])) == 0)
			  v9_serial_debug_api(TRUE, V9_APP_DEBUG_WEB);
		  if (strncmp ("state", argv[1], strlen (argv[1])) == 0)
			  v9_serial_debug_api(TRUE, V9_APP_DEBUG_STATE);
	  }
  return CMD_SUCCESS;
}

DEFUN (debug_video_sreial,
	   debug_video_sreial_cmd,
       "debug video sreial (hex|send|recv|update|timer|uci)",
       DEBUG_STR
       "Video configuration\n"
       "Sreial Debug Configuration\n"
       "Debug option set hex\n"
       "Debug option set send\n"
       "Debug option set recv\n"
	   "Debug option set update\n"
	   "Debug option set timer\n"
	   "Debug option set uci\n")
{
	  if (strncmp ("hex", argv[1], strlen (argv[1])) == 0)
		  v9_serial_debug_api(TRUE, V9_APP_DEBUG_HEX);
	  if (strncmp ("recv", argv[1], strlen (argv[1])) == 0)
		  v9_serial_debug_api(TRUE, V9_APP_DEBUG_RECV);
	  if (strncmp ("send", argv[1], strlen (argv[1])) == 0)
		  v9_serial_debug_api(TRUE, V9_APP_DEBUG_SEND);
	  if (strncmp ("update", argv[1], strlen (argv[1])) == 0)
		  v9_serial_debug_api(TRUE, V9_APP_DEBUG_UPDATE);
	  if (strncmp ("timer", argv[1], strlen (argv[1])) == 0)
		  v9_serial_debug_api(TRUE, V9_APP_DEBUG_TIME);
	  if (strncmp ("uci", argv[1], strlen (argv[1])) == 0)
		  v9_serial_debug_api(TRUE, V9_APP_DEBUG_UCI);
  return CMD_SUCCESS;
}

DEFUN (no_debug_video_event,
	   no_debug_video_event_cmd,
       "no debug video (user|serial|sdk) (event|error|warn)",
	   NO_STR
       DEBUG_STR
       "Video configuration\n"
       "User Debug Configuration\n"
       "Serial Debug Configuration\n"
       "SDK Debug Configuration\n"
       "Debug option set event\n"
       "Debug option set error\n"
       "Debug option set warn\n")
{
  if (strncmp ("user", argv[0], strlen (argv[0])) == 0)
  {
	  if (strncmp ("event", argv[1], strlen (argv[1])) == 0)
		  v9_user_debug_api(TRUE, V9_USER_DEBUG_EVENT);
	  if (strncmp ("error", argv[1], strlen (argv[1])) == 0)
		  v9_user_debug_api(TRUE, V9_USER_DEBUG_ERROR);
	  if (strncmp ("warn", argv[1], strlen (argv[1])) == 0)
		  v9_user_debug_api(TRUE, V9_USER_DEBUG_WARN);
  }
  else if (strncmp ("serial", argv[0], strlen (argv[0])) == 0)
  {
	  if (strncmp ("event", argv[1], strlen (argv[1])) == 0)
		  v9_serial_debug_api(TRUE, V9_APP_DEBUG_EVENT);
	  if (strncmp ("error", argv[1], strlen (argv[1])) == 0)
		  v9_serial_debug_api(TRUE, V9_APP_DEBUG_ERROR);
	  if (strncmp ("warn", argv[1], strlen (argv[1])) == 0)
		  v9_serial_debug_api(TRUE, V9_APP_DEBUG_WARN);
  }
  else if (strncmp ("sdk", argv[0], strlen (argv[0])) == 0)
  {
	  if (strncmp ("event", argv[1], strlen (argv[1])) == 0)
		  v9_video_sdk_debug_api(TRUE, V9_SDK_DEBUG_EVENT);
	  if (strncmp ("error", argv[1], strlen (argv[1])) == 0)
		  v9_video_sdk_debug_api(TRUE, V9_SDK_DEBUG_ERROR);
	  if (strncmp ("warn", argv[1], strlen (argv[1])) == 0)
		  v9_video_sdk_debug_api(TRUE, V9_SDK_DEBUG_WARN);
  }
  return CMD_SUCCESS;
}

DEFUN (no_debug_video_sqldb,
	   no_debug_video_sqldb_cmd,
       "no debug video sqldb (msg|db|state|sqlcmd)",
	   NO_STR
       DEBUG_STR
       "Video configuration\n"
       "Sqldb Debug Configuration\n"
       "Debug option set msg\n"
       "Debug option set db\n"
       "Debug option set state\n"
	   "Debug option set sqlcmd\n")
{
	  if (strncmp ("msg", argv[1], strlen (argv[1])) == 0)
		  v9_sqldb_debug_api(TRUE, V9_USER_DEBUG_EVENT);
	  if (strncmp ("db", argv[1], strlen (argv[1])) == 0)
		  v9_sqldb_debug_api(TRUE, V9_USER_DEBUG_ERROR);
	  if (strncmp ("state", argv[1], strlen (argv[1])) == 0)
		  v9_sqldb_debug_api(TRUE, V9_USER_DEBUG_WARN);
	  if (strncmp ("sqlcmd", argv[1], strlen (argv[1])) == 0)
		  v9_sqldb_debug_api(TRUE, V9_USER_DEBUG_WARN);
  return CMD_SUCCESS;
}

DEFUN (no_debug_video_appsdk,
	   no_debug_video_appsdk_cmd,
       "no debug video (sreial|sdk) (msg|web|state)",
	   NO_STR
       DEBUG_STR
       "Video configuration\n"
       "Serial Debug Configuration\n"
       "SDK Debug Configuration\n"
       "Debug option set msg\n"
       "Debug option set web\n"
       "Debug option set state\n")
{
	  if (strncmp ("sdk", argv[0], strlen (argv[0])) == 0)
	  {
		  if (strncmp ("msg", argv[1], strlen (argv[1])) == 0)
			  v9_video_sdk_debug_api(TRUE, V9_SDK_DEBUG_MSG);
		  if (strncmp ("web", argv[1], strlen (argv[1])) == 0)
			  v9_video_sdk_debug_api(TRUE, V9_SDK_DEBUG_WEB);
		  if (strncmp ("state", argv[1], strlen (argv[1])) == 0)
			  v9_video_sdk_debug_api(TRUE, V9_SDK_DEBUG_STATE);
	  }
	  else if (strncmp ("serial", argv[0], strlen (argv[0])) == 0)
	  {
		  if (strncmp ("msg", argv[1], strlen (argv[1])) == 0)
			  v9_serial_debug_api(TRUE, V9_APP_DEBUG_MSG);
		  if (strncmp ("web", argv[1], strlen (argv[1])) == 0)
			  v9_serial_debug_api(TRUE, V9_APP_DEBUG_WEB);
		  if (strncmp ("state", argv[1], strlen (argv[1])) == 0)
			  v9_serial_debug_api(TRUE, V9_APP_DEBUG_STATE);
	  }
  return CMD_SUCCESS;
}

DEFUN (no_debug_video_sreial,
	   no_debug_video_sreial_cmd,
       "no debug video sreial (hex|send|recv|update|timer|uci)",
	   NO_STR
       DEBUG_STR
       "Video configuration\n"
       "Sreial Debug Configuration\n"
       "Debug option set hex\n"
       "Debug option set send\n"
       "Debug option set recv\n"
	   "Debug option set update\n"
	   "Debug option set timer\n"
	   "Debug option set uci\n")
{
	  if (strncmp ("hex", argv[1], strlen (argv[1])) == 0)
		  v9_serial_debug_api(TRUE, V9_APP_DEBUG_HEX);
	  if (strncmp ("recv", argv[1], strlen (argv[1])) == 0)
		  v9_serial_debug_api(TRUE, V9_APP_DEBUG_RECV);
	  if (strncmp ("send", argv[1], strlen (argv[1])) == 0)
		  v9_serial_debug_api(TRUE, V9_APP_DEBUG_SEND);
	  if (strncmp ("update", argv[1], strlen (argv[1])) == 0)
		  v9_serial_debug_api(TRUE, V9_APP_DEBUG_UPDATE);
	  if (strncmp ("timer", argv[1], strlen (argv[1])) == 0)
		  v9_serial_debug_api(TRUE, V9_APP_DEBUG_TIME);
	  if (strncmp ("uci", argv[1], strlen (argv[1])) == 0)
		  v9_serial_debug_api(TRUE, V9_APP_DEBUG_UCI);
  return CMD_SUCCESS;
}

DEFUN (show_debug_video,
	   show_debug_video_cmd,
       "show debug video",
	   SHOW_STR
       DEBUG_STR
       "Video configuration\n")
{
	v9_video_debug_config(vty, TRUE);
	return CMD_SUCCESS;
}

static int app_write_config(struct vty *vty, void *pVoid)
{
	if(pVoid)
	{
		vty_out(vty, "template app video%s",VTY_NEWLINE);
		v9_video_board_stream_write_config(vty);
		return 1;
	}
	return 0;
}

static void cmd_app_global_init(void)
{
	install_element(ENABLE_NODE, &v9_app_board_show_cmd);

	install_element(ENABLE_NODE, &v9_app_video_rtsp_show_cmd);
	install_element(ENABLE_NODE, &v9_app_video_rtsp_id_show_cmd);
	install_element(ENABLE_NODE, &v9_app_sdk_show_cmd);
	install_element(ENABLE_NODE, &v9_app_sdk_show_config_cmd);
	install_element(ENABLE_NODE, &v9_app_time_sync_cmd_cmd);

	install_element(ENABLE_NODE, &v9_app_video_usergroup_show_cmd);
	install_element(ENABLE_NODE, &v9_app_video_usergroup_id_show_cmd);

	install_element(ENABLE_NODE, &v9_app_user_show_cmd);
}


static void cmd_app_temp_init(int node)
{
	install_element(node, &v9_app_board_disabled_cmd);

	install_element(node, &v9_app_stream_add_cmd);
	install_element(node, &v9_app_stream_add_1_cmd);
	install_element(node, &v9_app_stream_add_2_cmd);

	install_element(node, &v9_app_stream_param_cmd);

	install_element(node, &v9_sdk_hw_original_pic_enable_cmd);
	install_element(node, &v9_sdk_hw_set_recognize_cmd);
	install_element(node, &v9_sdk_hw_set_recognize_openupload_cmd);

	install_element(node, &v9_sdk_hw_set_snapopen_cmd);

#if 0
	install_element(node, &v9_sdk_hw_set_helmet_sentimage_cmd);
	install_element(node, &v9_sdk_hw_set_helmet_tracking_cmd);
	install_element(node, &v9_sdk_hw_set_helmet_drawrectangle_cmd);
	install_element(node, &v9_sdk_hw_set_helmet_imageratio_cmd);
	install_element(node, &v9_sdk_hw_set_helmet_snapinterval_cmd);
	install_element(node, &v9_sdk_hw_set_helmet_alarminterval_cmd);
	install_element(node, &v9_sdk_hw_set_helmet_snapratio_cmd);
	install_element(node, &v9_sdk_hw_set_helmet_threshold_cmd);
#endif

	install_element(node, &v9_sdk_hw_set_sanp_confi_cmd);
	install_element(node, &v9_sdk_hw_set_sanp_qulityscore_cmd);
	install_element(node, &v9_sdk_hw_set_sanp_attr_cmd);
	install_element(node, &v9_sdk_hw_set_sanp_feature_cmd);
	install_element(node, &v9_sdk_hw_set_sanp_align_cmd);
	install_element(node, &v9_sdk_hw_set_sanp_area_cmd);
	install_element(node, &v9_sdk_hw_set_sanp_tripwire_cmd);
	install_element(node, &v9_sdk_hw_set_sanp_snapmode_cmd);
	install_element(node, &v9_sdk_hw_set_sanp_intervaltime_cmd);
	install_element(node, &v9_sdk_hw_set_sanp_maxsize_cmd);
	install_element(node, &v9_sdk_hw_set_sanp_minsize_cmd);
}


static void cmd_app_debug_init(void)
{
	install_element(TEMPLATE_NODE, &v9_app_board_active_cmd);

	install_element(ENABLE_NODE, &debug_video_event_cmd);
	install_element(ENABLE_NODE, &debug_video_sqldb_cmd);
	install_element(ENABLE_NODE, &debug_video_appsdk_cmd);
	install_element(ENABLE_NODE, &debug_video_sreial_cmd);

	install_element(ENABLE_NODE, &no_debug_video_event_cmd);
	install_element(ENABLE_NODE, &no_debug_video_sqldb_cmd);
	install_element(ENABLE_NODE, &no_debug_video_appsdk_cmd);
	install_element(ENABLE_NODE, &no_debug_video_sreial_cmd);

	install_element(ENABLE_NODE, &show_debug_video_cmd);

#ifdef V9_VIDEO_SDK_API
	install_element(ENABLE_NODE, &v9_sdk_hw_reboot_cmd);
	install_element(ENABLE_NODE, &v9_sdk_hw_update_cmd);
/*
	install_element(ENABLE_NODE, &v9_sdk_hw_original_pic_enable_cmd);
	install_element(ENABLE_NODE, &v9_sdk_hw_set_recognize_cmd);
	install_element(ENABLE_NODE, &v9_sdk_hw_set_recognize_openupload_cmd);

	install_element(ENABLE_NODE, &v9_sdk_hw_set_snapopen_cmd);


	install_element(ENABLE_NODE, &v9_sdk_hw_set_helmet_sentimage_cmd);
	install_element(ENABLE_NODE, &v9_sdk_hw_set_helmet_tracking_cmd);
	install_element(ENABLE_NODE, &v9_sdk_hw_set_helmet_drawrectangle_cmd);
	install_element(ENABLE_NODE, &v9_sdk_hw_set_helmet_imageratio_cmd);
	install_element(ENABLE_NODE, &v9_sdk_hw_set_helmet_snapinterval_cmd);
	install_element(ENABLE_NODE, &v9_sdk_hw_set_helmet_alarminterval_cmd);
	install_element(ENABLE_NODE, &v9_sdk_hw_set_helmet_snapratio_cmd);
	install_element(ENABLE_NODE, &v9_sdk_hw_set_helmet_threshold_cmd);*/


	install_element(ENABLE_NODE, &v9_sdk_hw_get_recognize_cmd);
	install_element(ENABLE_NODE, &v9_sdk_hw_get_snapinfo_cmd);
	install_element(ENABLE_NODE, &v9_sdk_hw_get_helmet_cmd);
	install_element(ENABLE_NODE, &v9_sdk_hw_clean_user_cmd);
	install_element(ENABLE_NODE, &v9_sdk_hw_clean_rtsp_cmd);
	install_element(ENABLE_NODE, &v9_sdk_hw_del_user_cmd);
	install_element(ENABLE_NODE, &v9_sdk_hw_get_user_cmd);
#ifdef V9_SQLDB_TEST
	install_element(ENABLE_NODE, &v9_sdk_sqldb_user_cmd);
	install_element(ENABLE_NODE, &v9_sdk_sqldb_cap_tbl_cmd);
#ifdef V9_SLIPNET_ENABLE
	install_element(ENABLE_NODE, &v9_led_ctl_cmd);
#endif
#endif

#endif
}


void cmd_app_v9_init(void)
{
	template_t * temp = nsm_template_new (FALSE);
	if(temp)
	{
		temp->module = 0;
		strcpy(temp->name, "app video");
		strcpy(temp->prompt, "app-video"); /* (config-app-esp)# */
		//temp->prompt[64];
		//temp->id;
		//temp->pVoid;
		temp->pVoid = v9_video_app_tmp();
		temp->write_template = app_write_config;
		//temp->show_template = app_write_config;
		nsm_template_install(temp, 0);

		install_element(CONFIG_NODE, &app_template_cmd);
		install_element(CONFIG_NODE, &no_app_template_cmd);

/*		install_element(TEMPLATE_NODE, &v9_app_board_disabled_cmd);
extern int v9_video_debug_config(struct vty *vty, BOOL detail);
		install_element(TEMPLATE_NODE, &v9_app_stream_add_cmd);
		install_element(TEMPLATE_NODE, &v9_app_stream_add_1_cmd);
		install_element(TEMPLATE_NODE, &v9_app_stream_add_2_cmd);*/

		cmd_app_temp_init(TEMPLATE_NODE);

		cmd_app_global_init();
		cmd_app_debug_init();
	}
}




#endif

