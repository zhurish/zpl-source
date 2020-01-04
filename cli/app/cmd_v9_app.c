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
	template_t * temp = nsm_template_lookup_name ("app video");
	if(temp)
	{
		vty->node = TEMPLATE_NODE;
		memset(vty->prompt, 0, sizeof(vty->prompt));
		sprintf(vty->prompt, "%s", temp->prompt);
		return CMD_SUCCESS;
	}
	else
	{
		temp = nsm_template_new ();
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
	template_t * temp = nsm_template_lookup_name ("app video");
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
		v9_video_board_disabled(APP_BOARD_CALCU_ID(atoi(argv[0])), FALSE);
	else
		v9_video_board_disabled(APP_BOARD_CALCU_ID(atoi(argv[0])), TRUE);
	return CMD_SUCCESS;
}




DEFUN (v9_app_stream_add,
	   v9_app_stream_add_cmd,
		"video stream board <0-4> channel <0-4> ip A.B.C.D port <1-65530> username USE password PASS fps <480-1080>",
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
{//CMD_KEY_IPV4
	vty_out(vty, "---------argc = %d ----------%s", argc, VTY_NEWLINE);
	if(argc == 7)
		v9_video_stream_add_api(atoi(argv[0]), atoi(argv[1]), ntohl(inet_addr(argv[2])), atoi(argv[3]), argv[4], argv[5], atoi(argv[6]));
	else if(argc == 6)
		v9_video_stream_add_api(atoi(argv[0]), atoi(argv[1]), ntohl(inet_addr(argv[2])), atoi(argv[3]), argv[4], argv[5], 1080);
	else if(argc == 5)
		v9_video_stream_add_api(atoi(argv[0]), atoi(argv[1]), ntohl(inet_addr(argv[2])), 0, argv[4], argv[5], 1080);

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
		"video stream board <0-4> channel <0-4> ip A.B.C.D port <1-65530> username USE password PASS",
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
		"video stream board <0-4> channel <0-4> ip A.B.C.D username USE password PASS",
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
		v9_video_board_active(APP_BOARD_CALCU_ID(atoi(argv[0])), TRUE);
	else
		v9_video_board_active(APP_BOARD_CALCU_ID(atoi(argv[0])), FALSE);
	return CMD_SUCCESS;
}


/* show */
DEFUN (v9_app_board_show,
	   v9_app_board_show_cmd,
		"show board status",
		SHOW_STR
		"Board Configure\n"
		"Status Information\n")
{
	v9_board_show(vty, 0, TRUE);
	return CMD_SUCCESS;
}

DEFUN (v9_app_video_board_show,
	   v9_app_video_board_show_cmd,
		"show video board information",
		SHOW_STR
		"Video Configure\n"
		"Board Configure\n"
		"Status Information\n")
{
	v9_video_board_show(vty, TRUE);
	return CMD_SUCCESS;
}

DEFUN (v9_app_video_channel_show,
	   v9_app_video_channel_show_cmd,
		"show video stream information",
		SHOW_STR
		"Video Configure\n"
		"Video Stream Configure\n"
		"Status Information\n")
{
	if(argv[0])
		v9_video_channel_show(vty, APP_BOARD_CALCU_ID(atoi(argv[0])), TRUE);
	else
		v9_video_channel_show(vty, 0, TRUE);
	return CMD_SUCCESS;
}

ALIAS(v9_app_video_channel_show,
	  v9_app_video_channel_id_show_cmd,
		"show video <1-4> stream information",
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
	v9_cmd_sync_time_test();
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
		"video sdk reboot <0-4> [reload]",
		SHOW_STR
		"Video Configure\n"
		"User Information\n")
{
	if(argc == 2)
		v9_video_sdk_reboot_api(atoi(argv[0]), TRUE);
	else
		v9_video_sdk_reboot_api(atoi(argv[0]), FALSE);
	return CMD_SUCCESS;
}

DEFUN (v9_sdk_hw_update,
	   v9_sdk_hw_update_cmd,
		"video sdk update <0-4> FILENAME",
		SHOW_STR
		"Video Configure\n"
		"User Information\n")
{
	v9_video_sdk_update_api(atoi(argv[0]), argv[1]);
	return CMD_SUCCESS;
}


DEFUN (v9_sdk_hw_snap,
	   v9_sdk_hw_snap_cmd,
		"video sdk snap <0-4> (open|close)",
		SHOW_STR
		"Video Configure\n"
		"User Information\n")
{
	if(strstr(argv[1], "open"))
		v9_video_sdk_open_snap_api(atoi(argv[0]), 0);
	else
		v9_video_sdk_close_snap_api(atoi(argv[0]));
	return CMD_SUCCESS;
}



DEFUN (v9_sdk_hw_original_pic_enable,
	   v9_sdk_hw_original_pic_enable_cmd,
		"video sdk original_pic_enable <0-4> (enable|disable)",
		SHOW_STR
		"Video Configure\n"
		"User Information\n")
{
	if(strstr(argv[1], "enable"))
		v9_video_sdk_original_pic_enable_set_api(atoi(argv[0]), TRUE);
	else
		v9_video_sdk_original_pic_enable_set_api(atoi(argv[0]), FALSE);
	return CMD_SUCCESS;
}



DEFUN (v9_sdk_hw_get_test,
	   v9_sdk_hw_get_test_cmd,
		"video sdk get_test <0-4> <1-11>",
		SHOW_STR
		"Video Configure\n"
		"User Information\n")
{
	int nOutSimilarity = 0, nRegisterQuality = 0;
	BOOL nOpenUpload = FALSE;
	ST_SDKHelmetInfo HelmetInfo;
	memset(&HelmetInfo, 0, sizeof(ST_SDKHelmetInfo));
	ST_SDKSnapInfo stSnapInfo;
	memset(&stSnapInfo, 0, sizeof(ST_SDKSnapInfo));
	switch(atoi(argv[1]))
	{
		case 1:
			v9_video_sdk_recognize_config_get_api(atoi(argv[0]), &nOutSimilarity, &nRegisterQuality, &nOpenUpload);
			break;
		case 2:
			v9_video_sdk_helmet_config_get_api(atoi(argv[0]), 0, &HelmetInfo);
			break;
		case 3:
			v9_video_sdk_snap_config_get_api(atoi(argv[0]), 0, &stSnapInfo);
			break;
		case 4:
			break;
		case 5:
			break;
		case 6:
			break;
		case 7:
			break;
		case 8:
			break;
		case 9:
			break;
		case 10:
			break;
	}

	return CMD_SUCCESS;
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

int v9_video_sdk_add_user_all_api(BOOL gender, int group, char *user, char *ID, char *pic, BOOL edit);
int v9_video_sdk_del_user_all_api(char *ID);
int v9_video_sdk_del_group_user_all_api(void *p_pstUserList);
int v9_video_sdk_del_group_all_api(int group);
int v9_video_sdk_get_user_all_api(char* ID, void* UserInfo);
*/
#endif


static int app_write_config(struct vty *vty, void *pVoid)
{
	if(pVoid)
	{
		vty_out(vty, "template app video%s",VTY_NEWLINE);
		v9_video_channel_write_config(vty);
		return 1;
	}
	return 0;
}


static void cmd_app_global_init(void)
{
	install_element(ENABLE_NODE, &v9_app_board_show_cmd);
	install_element(ENABLE_NODE, &v9_app_video_board_show_cmd);
	install_element(ENABLE_NODE, &v9_app_video_channel_show_cmd);
	install_element(ENABLE_NODE, &v9_app_video_channel_id_show_cmd);
	install_element(ENABLE_NODE, &v9_app_sdk_show_cmd);
	install_element(ENABLE_NODE, &v9_app_time_sync_cmd_cmd);

	install_element(ENABLE_NODE, &v9_app_user_show_cmd);
}

static void cmd_app_debug_init(void)
{
	install_element(TEMPLATE_NODE, &v9_app_board_active_cmd);
#ifdef V9_VIDEO_SDK_API
	install_element(ENABLE_NODE, &v9_sdk_hw_reboot_cmd);
	install_element(ENABLE_NODE, &v9_sdk_hw_update_cmd);

	install_element(ENABLE_NODE, &v9_sdk_hw_snap_cmd);
	install_element(ENABLE_NODE, &v9_sdk_hw_original_pic_enable_cmd);
	install_element(ENABLE_NODE, &v9_sdk_hw_get_test_cmd);
#endif
}


void cmd_app_v9_init(void)
{
	template_t * temp = nsm_template_new ();
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

		install_element(TEMPLATE_NODE, &v9_app_board_disabled_cmd);

		install_element(TEMPLATE_NODE, &v9_app_stream_add_cmd);
		install_element(TEMPLATE_NODE, &v9_app_stream_add_1_cmd);
		install_element(TEMPLATE_NODE, &v9_app_stream_add_2_cmd);

		cmd_app_global_init();
		cmd_app_debug_init();
	}
}




#endif

