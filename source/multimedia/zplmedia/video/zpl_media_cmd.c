/*
 * zpl_media_cmd.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "lib_include.h"
#include "zpl_media.h"
#include "zpl_media_api.h"
#include "zpl_media_internal.h"

#ifdef ZPL_SHELL_MODULE

#define MEDIA_CHANNEL_STR	"MultiMedia Configure\n" \
							"MultiMedia Channel Select\n"

static int mediaservice_write_config(struct vty *vty, void *pVoid);


DEFUN (media_channel_enable,
		media_channel_enable_cmd,
		"media channel <0-1> (main|sub) (enable|disable)" ,
		MEDIA_CHANNEL_STR
		"Channel Number Select\n"
		"Main configure\n"
		"Submain configure\n"
		"Enable\n"
		"Disable\n")
{
	int ret = ERROR;
	zpl_int32 channel = -1;
	ZPL_MEDIA_CHANNEL_INDEX_E channel_index = ZPL_MEDIA_CHANNEL_INDEX_SUB;
	
	//VTY_GET_INTEGER("channel",channel, argv[0]);
	channel = atoi(argv[0]);
	if(strstr(argv[1],"main"))
		channel_index = ZPL_MEDIA_CHANNEL_INDEX_MAIN;
	else if(strstr(argv[1],"sub"))
		channel_index = ZPL_MEDIA_CHANNEL_INDEX_SUB;
	if(strstr(argv[2],"enable"))
	{
		if(zpl_media_channel_lookup(channel,  channel_index) == NULL)
			ret = zpl_media_channel_create( channel,  channel_index);
		if(zpl_media_channel_lookup(channel,  channel_index) != NULL)
			ret = zpl_media_channel_active(channel,  channel_index);
	}
	else if(strstr(argv[2],"disable"))
	{
		if(zpl_media_channel_lookup(channel,  channel_index) != NULL && 
			zpl_media_channel_state(channel,  channel_index) != ZPL_MEDIA_STATE_ACTIVE)
			ret = zpl_media_channel_destroy( channel,  channel_index);
		else
			ret = ERROR;	
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (media_channel_file_enable,
		media_channel_file_enable_cmd,
		"media channel FILENAME (enable|disable)" ,
		MEDIA_CHANNEL_STR
		"Channel Number Select\n"
		"Media File Name Configure\n"
		"Enable\n"
		"Disable\n")
{
	int ret = ERROR;

	if(strstr(argv[1],"enable"))
	{
		if(zpl_media_channel_filelookup(argv[0]) == NULL)
			zpl_media_channel_filecreate(argv[0], 1);
	}
	else if(strstr(argv[1],"disable"))
	{
		if(zpl_media_channel_filelookup(argv[0]) != NULL)
			ret = zpl_media_channel_filedestroy(argv[0]);
		else
			ret = ERROR;	
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (media_channel_active,
		media_channel_active_cmd,
		"media channel <0-1> (main|sub) (active|inactive)" ,
		MEDIA_CHANNEL_STR
		"Channel Number Select\n"
		"Main configure\n"
		"Submain configure\n"
		"Active\n"
		"Inactive\n")
{
	int ret = ERROR;
	zpl_int32 channel = -1;
	ZPL_MEDIA_CHANNEL_INDEX_E channel_index = ZPL_MEDIA_CHANNEL_INDEX_SUB;
	VTY_GET_INTEGER("channel",channel, argv[0]);
	//channel = atoi(argv[0]);
	if(strstr(argv[1],"main"))
		channel_index = ZPL_MEDIA_CHANNEL_INDEX_MAIN;
	else if(strstr(argv[1],"sub"))
		channel_index = ZPL_MEDIA_CHANNEL_INDEX_SUB;
	if(strncmp(argv[2],"active", 4) == 0)
	{
		if(zpl_media_channel_lookup(channel,  channel_index) != NULL && 
			zpl_media_channel_state(channel,  channel_index) != ZPL_MEDIA_STATE_ACTIVE)
			ret = zpl_media_channel_start(channel,  channel_index);
	}
	else if(strncmp(argv[2],"inactive", 4) == 0)
	{
		if(zpl_media_channel_lookup(channel,  channel_index) != NULL && 
			zpl_media_channel_state(channel,  channel_index) == ZPL_MEDIA_STATE_ACTIVE)
			ret = zpl_media_channel_stop( channel,  channel_index);
		else
			ret = ERROR;	
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (media_channel_record,
		media_channel_record_cmd,
		"media channel <0-1> (main|sub) record (enable|disable)" ,
		MEDIA_CHANNEL_STR
		"Channel Number Select\n"
		"Main configure\n"
		"Submain configure\n"
		"Media Record\n"
		"Enable\n"
		"Disable\n")
{
	int ret = ERROR;
	zpl_int32 channel = -1;
	ZPL_MEDIA_CHANNEL_INDEX_E channel_index = ZPL_MEDIA_CHANNEL_INDEX_SUB;
	zpl_media_channel_t	*chn = NULL;
	VTY_GET_INTEGER("channel",channel, argv[0]);
	
	if(strstr(argv[1],"main"))
		channel_index = ZPL_MEDIA_CHANNEL_INDEX_MAIN;
	else if(strstr(argv[1],"sub"))
		channel_index = ZPL_MEDIA_CHANNEL_INDEX_SUB;

	chn = zpl_media_channel_lookup(channel,  channel_index);
	if(chn && strncmp(argv[2],"enable", 4) == 0)
	{
		if(!zpl_media_channel_record_state(chn))
			ret = zpl_media_channel_record_enable(chn,  zpl_true);
		else
			ret = OK;	
	}
	else if(chn && strncmp(argv[2],"disable", 4) == 0)
	{
		if(zpl_media_channel_record_state(chn))
			ret = zpl_media_channel_record_enable(chn,  zpl_false);
		else
			ret = OK;	
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (media_channel_alarm_capture,
		media_channel_alarm_capture_cmd,
		"media channel <0-1> (main|sub) alarm capture (enable|disable)" ,
		MEDIA_CHANNEL_STR
		"Channel Number Select\n"
		"Main configure\n"
		"Submain configure\n"
		"Alarm Capture\n"
		"Enable\n"
		"Disable\n")
{
	int ret = ERROR;
	zpl_int32 channel = -1;
	ZPL_MEDIA_CHANNEL_INDEX_E channel_index = ZPL_MEDIA_CHANNEL_INDEX_SUB;
	zpl_media_channel_t	*chn = NULL;
	VTY_GET_INTEGER("channel",channel, argv[0]);
	
	if(strstr(argv[1],"main"))
		channel_index = ZPL_MEDIA_CHANNEL_INDEX_MAIN;
	else if(strstr(argv[1],"sub"))
		channel_index = ZPL_MEDIA_CHANNEL_INDEX_SUB;

	chn = zpl_media_channel_lookup(channel,  channel_index);
	if(chn && strncmp(argv[2],"enable", 4) == 0)
	{
		if(!zpl_media_channel_capture_state(chn))
			ret = zpl_media_channel_capture_enable(chn,  zpl_true);
		else
			ret = OK;	
	}
	else if(chn && strncmp(argv[2],"disable", 4) == 0)
	{
		if(zpl_media_channel_capture_state(chn))
			ret = zpl_media_channel_capture_enable(chn,  zpl_false);
		else
			ret = OK;	
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (media_channel_osd,
		media_channel_osd_cmd,
		"media channel <0-1> (main|sub) (channel|datetime|bitrate|label|rect|other) osd (enable|disable)" ,
		MEDIA_CHANNEL_STR
		"Channel Number Select\n"
		"Main configure\n"
		"Submain configure\n"
		"Channel Information\n"
		"Datetime Information\n"
		"Bitrate Information\n"
		"Label Information\n"
		"Rect Information\n"
		"Other Information\n"
		"OSD Of media\n"
		"Enable\n"
		"Disable\n")
{
	int ret = ERROR;
	ret = zpl_video_vpssgrp_show(vty);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}



DEFUN (show_video_vpssgrp_info,
		show_video_vpssgrp_info_cmd,
		"show media vpssgrp info" ,
		SHOW_STR
		"MultiMedia Configure\n"
		"vpssgrp configure\n"
		"information\n")
{
	int ret = ERROR;
	ret = zpl_video_vpssgrp_show(vty);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (show_video_vpss_channel_info,
		show_video_vpss_channel_info_cmd,
		"show media vpss channel info" ,
		SHOW_STR
		"MultiMedia Configure\n"
		"vpss configure\n"
		"vpss channel configure\n"
		"information\n")
{
	int ret = ERROR;
	ret = zpl_video_vpss_channel_show(vty);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (show_video_input_pipe_info,
		show_video_input_pipe_info_cmd,
		"show media input pipe info" ,
		SHOW_STR
		"MultiMedia Configure\n"
		"input configure\n"
		"input pipe configure\n"
		"information\n")
{
	int ret = ERROR;
	ret = zpl_video_input_pipe_show(vty);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (show_video_input_info,
		show_video_input_info_cmd,
		"show media input info" ,
		SHOW_STR
		"MultiMedia Configure\n"
		"input configure\n"
		"information\n")
{
	int ret = ERROR;
	ret = zpl_video_input_show(vty);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (show_video_encode_info,
		show_video_encode_info_cmd,
		"show media encode info" ,
		SHOW_STR
		"MultiMedia Configure\n"
		"encode configure\n"
		"information\n")
{
	int ret = ERROR;
	ret = zpl_video_encode_show(vty);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
//extern int rtp_send_h264_test(void);
DEFUN (show_video_channel_info,
		show_video_channel_info_cmd,
		"show media channel info" ,
		SHOW_STR
		MEDIA_CHANNEL_STR
		"information\n")
{
	int ret = ERROR;
	ret = zpl_media_channel_show(vty);
	//	rtp_send_h264_test();
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (video_channel_action,
		video_channel_handle_cmd,
		"video channel (enable|start|stop|disable|destroy)" ,
		"video configure"
		"channel configure"
		"information\n")
{
	int ret = ERROR;
	/* active:1 使能所有 ，:2 去使能所有 :3 开始所有 :4 停止所有 :-1 销毁所有*/
	zpl_uint32 active;
	if(strncmp(argv[0],"ena", 3) == 0)
		active = 1;
	if(strncmp(argv[0],"sta", 3) == 0)
		active = 3;
	if(strncmp(argv[0],"sto", 3) == 0)
		active = 4;
	if(strncmp(argv[0],"dis", 3) == 0)
		active = 2;
	if(strncmp(argv[0],"des", 3) == 0)
		active = -1;
	if(active != 0)	
		ret = zpl_media_channel_handle_all(active);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

#ifdef ZPL_HISIMPP_MODULE
DEFUN (video_h264_test,
		video_h264_test_cmd,
		"video h264 test (start|stop)" ,
		"video configure"
		"channel configure"
		"information\n")
{
	extern int SAMPLE_VENC_H264_TEST(void);
	extern int SAMPLE_VENC_H264_TEST_STOP(void);
	if(strncmp(argv[0],"sta", 3) == 0)
		SAMPLE_VENC_H264_TEST();
	if(strncmp(argv[0],"sto", 3) == 0)
		SAMPLE_VENC_H264_TEST_STOP();
	return CMD_SUCCESS;
}
#endif





DEFUN (mediaservice_template,
		mediaservice_template_cmd,
		"template mediaservice",
		"Template configure\n"
		"Mediaservice configure\n")
{
	template_t * temp = lib_template_lookup_name (zpl_false, "mediaservice");
	if(temp)
	{
		vty->node = TEMPLATE_NODE;
		memset(vty->prompt, 0, sizeof(vty->prompt));
		sprintf(vty->prompt, "%s", temp->prompt);
		//web_app_enable_set_api(zpl_true);
		return CMD_SUCCESS;
	}
	else
	{
		temp = lib_template_new (zpl_false);
		if(temp)
		{
			temp->module = 0;
			strcpy(temp->name, "mediaservice");
			strcpy(temp->prompt, "mediaservice"); /* (config-app-esp)# */
			temp->write_template = mediaservice_write_config;
			temp->pVoid = NULL;
			lib_template_install(temp, 0);

			vty->node = TEMPLATE_NODE;
			memset(vty->prompt, 0, sizeof(vty->prompt));
			sprintf(vty->prompt, "%s", temp->prompt);
			//web_app_enable_set_api(zpl_true);
			return CMD_SUCCESS;
		}
	}
	return CMD_WARNING;
}

DEFUN (no_mediaservice_template,
		no_mediaservice_template_cmd,
		"no template mediaservice",
		NO_STR
		"Template configure\n"
		"Mediaservice configure\n")
{
	template_t * temp = lib_template_lookup_name (zpl_false, "mediaservice");
	if(temp)
	{
		//web_app_enable_set_api(zpl_false);
		lib_template_free(temp);
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}


static int media_write_config_one(zpl_media_channel_t *chn, struct vty *vty)
{
	if(chn)
	{
		vty_out(vty, " media channel %d %s enable%s", chn->channel, 
			(chn->channel_index==ZPL_MEDIA_CHANNEL_INDEX_MAIN)?"main":"sub", VTY_NEWLINE);

		if(chn->state==ZPL_MEDIA_STATE_ACTIVE)
			vty_out(vty, " media channel %d %s %s%s", chn->channel, 
				(chn->channel_index==ZPL_MEDIA_CHANNEL_INDEX_MAIN)?"main":"sub", 
				(chn->state==ZPL_MEDIA_STATE_ACTIVE)?"active":"inactive", VTY_NEWLINE);

		if(chn->p_record.enable)
			vty_out(vty, " media channel %d %s record %s%s", chn->channel, 
				(chn->channel_index==ZPL_MEDIA_CHANNEL_INDEX_MAIN)?"main":"sub", 
				chn->p_record.enable?"enable":"disable", VTY_NEWLINE);

		if(chn->p_capture.enable)
			vty_out(vty, " media channel %d %s alarm capture %s%s", chn->channel, 
				(chn->channel_index==ZPL_MEDIA_CHANNEL_INDEX_MAIN)?"main":"sub", 
				chn->p_capture.enable?"enable":"disable", VTY_NEWLINE);
		return OK;
	}
	else
		return OK;
}

static int mediaservice_write_config(struct vty *vty, void *pVoid)
{
	vty_out(vty, "template mediaservice%s",VTY_NEWLINE);
	zpl_media_channel_foreach(media_write_config_one, vty);
	vty_out(vty, "!%s",VTY_NEWLINE);
	return OK;
}


static void cmd_mediaservice_init(void)
{
	{
		template_t * temp = lib_template_new (zpl_false);
		if(temp)
		{
			temp->module = 2;
			strcpy(temp->name, "mediaservice");
			strcpy(temp->prompt, "mediaservice"); /* (config-app-esp)# */
			temp->pVoid = NULL;
			temp->write_template = mediaservice_write_config;
			//temp->show_debug = webserver_debug_write_config;
			lib_template_install(temp, 2);
		}
		install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mediaservice_template_cmd);
		install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_mediaservice_template_cmd);

		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_file_enable_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_enable_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_active_cmd);

		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_record_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_alarm_capture_cmd);

		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_osd_cmd);
	}
	return OK;
}

void cmd_video_init(void)
{
#ifdef ZPL_HISIMPP_MODULE
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &video_h264_test_cmd);
#endif
	cmd_mediaservice_init();
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &video_channel_handle_cmd);
	install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_video_vpssgrp_info_cmd);
	install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_video_vpss_channel_info_cmd);

	install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_video_input_pipe_info_cmd);
	install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_video_input_info_cmd);
	install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_video_encode_info_cmd);
	install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_video_channel_info_cmd);
}

#endif