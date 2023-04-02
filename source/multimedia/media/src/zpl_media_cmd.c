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

/*
* media channel <0-1> (main|sub)
* media channel <0-1> (main|sub) (enable|disable)
* media channel <0-1> (main|sub) (start|stop)
* media channel <0-1> (main|sub) encode <0-7>
* media channel <0-1> (main|sub) vpsschn group <0-3> channel <0-2>
* media channel <0-1> (main|sub) inpuchn dev <0-1> pipe <0-1> channel <0-7>
* media channel <0-1> (main|sub) inpuchn dev <0-1> pipe <0-1> channel <0-7> snstype <0-15> snsdev <0-1> mipidev <0-1>
* media channel <0-1> (main|sub) record enable
* media channel <0-1> (main|sub) capture enable
* media channel <0-1> (main|sub) osd-channel enable
* media channel <0-1> (main|sub) osd-datetime enable
* media channel <0-1> (main|sub) osd-bitrate enable
* media channel <0-1> (main|sub) osd-label enable
* media channel <0-1> (main|sub) osd-rect enable
* media channel <0-1> (main|sub) osd-other enable
*/

DEFUN (media_channel_enable,
		media_channel_enable_cmd,
		"media channel <0-1> (main|sub)" ,
		MEDIA_CHANNEL_STR
		"Channel Number Select\n"
		"Main Channel Configure\n"
		"Submain Channel Configure\n")
{
	int ret = ERROR;
	ZPL_MEDIA_CHANNEL_E channel = -1;
	ZPL_MEDIA_CHANNEL_TYPE_E channel_index = ZPL_MEDIA_CHANNEL_TYPE_SUB;
	
	channel = atoi(argv[0]);
	if(strstr(argv[1],"main"))
		channel_index = ZPL_MEDIA_CHANNEL_TYPE_MAIN;
	else if(strstr(argv[1],"sub"))
		channel_index = ZPL_MEDIA_CHANNEL_TYPE_SUB;

	if(zpl_media_channel_lookup(channel,  channel_index) == NULL)
	{
		ret = zpl_media_channel_create( channel,  channel_index);
	}
	else
	{
		vty_out(vty, " media channel %d %s is already exist. %s", channel, argv[1], VTY_NEWLINE);	
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN_HIDDEN (media_channel_enable_alsa,
		media_channel_enable_alsa_cmd,
		"media channel <0-1> (main|sub) (destroy|start|stop)" ,
		MEDIA_CHANNEL_STR
		"Channel Number Select\n"
		"Main Channel Configure\n"
		"Submain Channel Configure\n"
		"Destroy\n"
		"Start\n"
		"Stop\n")
{
	int ret = ERROR;
	ZPL_MEDIA_CHANNEL_E channel = -1;
	ZPL_MEDIA_CHANNEL_TYPE_E channel_index = ZPL_MEDIA_CHANNEL_TYPE_SUB;
	
	channel = atoi(argv[0]);
	if(strstr(argv[1],"main"))
		channel_index = ZPL_MEDIA_CHANNEL_TYPE_MAIN;
	else if(strstr(argv[1],"sub"))
		channel_index = ZPL_MEDIA_CHANNEL_TYPE_SUB;
	if(strstr(argv[2],"destroy"))
	{
		if(zpl_media_channel_lookup(channel,  channel_index) != NULL)
		{
			if(ZPL_TST_BIT(zpl_media_channel_state(channel,  channel_index), ZPL_MEDIA_STATE_START))
				vty_out(vty, " media channel %d %s is already start. %s", channel, argv[1], VTY_NEWLINE);	
			else
				ret = zpl_media_channel_destroy( channel,  channel_index);
		}
		else
		{
			vty_out(vty, " media channel %d %s is not exist. %s", channel, argv[1], VTY_NEWLINE);	
		}
	}
	if(strstr(argv[2],"start"))
	{
		if(zpl_media_channel_lookup(channel,  channel_index) == NULL)
		{
			vty_out(vty, " media channel %d %s is not exist.%s", channel, argv[1], VTY_NEWLINE);
		}
		else
		{
			if(ZPL_TST_BIT(zpl_media_channel_state(channel,  channel_index), ZPL_MEDIA_STATE_ACTIVE))
			{
				if(!ZPL_TST_BIT(zpl_media_channel_state(channel,  channel_index), ZPL_MEDIA_STATE_START))
					ret = zpl_media_channel_start(channel,  channel_index);
				else
				{
					vty_out(vty, " media channel %d %s is already start %s", channel, argv[1], VTY_NEWLINE);
				}	
			}
			else
			{
				vty_out(vty, " media channel %d %s is not active %s", channel, argv[1], VTY_NEWLINE);
			}	
		}
	}
	else if(strstr(argv[2],"stop"))
	{
		if(zpl_media_channel_lookup(channel,  channel_index) == NULL)
		{
			vty_out(vty, " media channel %d %s is not exist.%s", channel, argv[1], VTY_NEWLINE);
		}
		else
		{
			if(ZPL_TST_BIT(zpl_media_channel_state(channel,  channel_index), ZPL_MEDIA_STATE_ACTIVE))
			{
				if(ZPL_TST_BIT(zpl_media_channel_state(channel,  channel_index), ZPL_MEDIA_STATE_START))
					ret = zpl_media_channel_stop(channel,  channel_index);
				else
				{
					vty_out(vty, " media channel %d %s is not start %s", channel, argv[1], VTY_NEWLINE);
				}	
			}
			else
			{
				vty_out(vty, " media channel %d %s is not active %s", channel, argv[1], VTY_NEWLINE);
			}		
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}



DEFUN (media_channel_osd,
		media_channel_osd_cmd,
		"media channel <0-1> (main|sub) (channel|datetime|bitrate|label|rect|other) osd (enable|disable)" ,
		MEDIA_CHANNEL_STR
		"Channel Number Select\n"
		"Main Configure\n"
		"Submain Configure\n"
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
	ZPL_MEDIA_CHANNEL_E channel = -1;
	ZPL_MEDIA_CHANNEL_TYPE_E channel_index = ZPL_MEDIA_CHANNEL_TYPE_SUB;
	zpl_media_channel_t *mchannel = NULL;
	zpl_media_area_t *marea = NULL;
	ZPL_MEDIA_AREA_E area_type = ZPL_MEDIA_AREA_OSD;
	ZPL_MEDIA_OSD_TYPE_E osd_type = ZPL_MEDIA_OSD_NONE;
	channel = atoi(argv[0]);
	if(strstr(argv[1],"main"))
		channel_index = ZPL_MEDIA_CHANNEL_TYPE_MAIN;
	else if(strstr(argv[1],"sub"))
		channel_index = ZPL_MEDIA_CHANNEL_TYPE_SUB;

	if(strncmp(argv[2],"channel", 4) == 0)
		osd_type = ZPL_MEDIA_OSD_CHANNAL;
	else if(strncmp(argv[2],"datetime", 4) == 0)
		osd_type = ZPL_MEDIA_OSD_DATETIME;
	else if(strncmp(argv[2],"bitrate", 4) == 0)
		osd_type = ZPL_MEDIA_OSD_BITRATE;
	else if(strncmp(argv[2],"label", 4) == 0)
		osd_type = ZPL_MEDIA_OSD_LABEL;
	else if(strncmp(argv[2],"rect", 4) == 0)
		osd_type = ZPL_MEDIA_OSD_RECT;
	else if(strncmp(argv[2],"other", 4) == 0)
		osd_type = ZPL_MEDIA_OSD_OTHER;

	mchannel = zpl_media_channel_lookup(channel,  channel_index);
	if(mchannel == NULL)
	{
		vty_out(vty, " media channel %d %s is not exist.%s", channel, argv[1], VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(strncmp(argv[3],"enable", 4) == 0)
	{
		if(zpl_media_channel_area_lookup(mchannel, area_type, osd_type))
		{
			vty_out(vty, " media channel %d %s is already have osd of %s.%s", channel, argv[1], argv[2], VTY_NEWLINE);
			return CMD_WARNING;
		}
		marea = zpl_media_area_create(ZPL_MEDIA_AREA_OSD);
		if(marea == NULL)
		{
			vty_out(vty, " media channel %d %s create area of %s failed.%s", channel, argv[1], argv[2], VTY_NEWLINE);
			return CMD_WARNING;
		}
		zpl_media_area_osd_default(marea, osd_type);
		ret = zpl_media_channel_area_add(mchannel, marea);
	}
	if(strncmp(argv[3],"disable", 4) == 0)
	{
		marea = zpl_media_channel_area_lookup(mchannel, area_type, osd_type);
		if(!marea)
		{
			vty_out(vty, " media channel %d %s is not exits osd of %s.%s", channel, argv[1], argv[2], VTY_NEWLINE);
			return CMD_WARNING;
		}
		ret = zpl_media_channel_area_del(mchannel, area_type, osd_type);
		zpl_media_area_destroy(marea);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}




DEFUN (media_channel_record,
		media_channel_record_cmd,
		"media channel <0-1> (main|sub) record (enable|disable)" ,
		MEDIA_CHANNEL_STR
		"Channel Number Select\n"
		"Main Configure\n"
		"Submain Configure\n"
		"Media Record\n"
		"Enable\n"
		"Disable\n")
{
	int ret = ERROR;
	ZPL_MEDIA_CHANNEL_E channel = -1;
	ZPL_MEDIA_CHANNEL_TYPE_E channel_index = ZPL_MEDIA_CHANNEL_TYPE_SUB;
	zpl_media_channel_t	*chn = NULL;
	VTY_GET_INTEGER("channel",channel, argv[0]);
	
	if(strstr(argv[1],"main"))
		channel_index = ZPL_MEDIA_CHANNEL_TYPE_MAIN;
	else if(strstr(argv[1],"sub"))
		channel_index = ZPL_MEDIA_CHANNEL_TYPE_SUB;

	chn = zpl_media_channel_lookup(channel,  channel_index);
	if(chn && strncmp(argv[2],"enable", 4) == 0)
	{
		if(!zpl_media_channel_record_state(channel,  channel_index))
			ret = zpl_media_channel_record_enable(channel,  channel_index,  zpl_true);
		else
			ret = OK;	
	}
	else if(chn && strncmp(argv[2],"disable", 4) == 0)
	{
		if(zpl_media_channel_record_state(channel,  channel_index))
			ret = zpl_media_channel_record_enable(channel,  channel_index,  zpl_false);
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
		"Main Configure\n"
		"Submain Configure\n"
		"Alarm Capture\n"
		"Enable\n"
		"Disable\n")
{
	int ret = ERROR;
	ZPL_MEDIA_CHANNEL_E channel = -1;
	ZPL_MEDIA_CHANNEL_TYPE_E channel_index = ZPL_MEDIA_CHANNEL_TYPE_SUB;
	zpl_media_channel_t	*chn = NULL;
	VTY_GET_INTEGER("channel",channel, argv[0]);
	
	if(strstr(argv[1],"main"))
		channel_index = ZPL_MEDIA_CHANNEL_TYPE_MAIN;
	else if(strstr(argv[1],"sub"))
		channel_index = ZPL_MEDIA_CHANNEL_TYPE_SUB;

	chn = zpl_media_channel_lookup(channel,  channel_index);
	if(chn && strncmp(argv[2],"enable", 4) == 0)
	{
		if(!zpl_media_channel_capture_state(channel,  channel_index))
			ret = zpl_media_channel_capture_enable(channel,  channel_index,  zpl_true);
		else
			ret = OK;	
	}
	else if(chn && strncmp(argv[2],"disable", 4) == 0)
	{
		if(zpl_media_channel_capture_state(channel,  channel_index))
			ret = zpl_media_channel_capture_enable(channel,  channel_index,  zpl_false);
		else
			ret = OK;	
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

#ifdef ZPL_LIBORTP_MODULE
DEFUN (media_channel_multicast_enable,
		media_channel_multicast_enable_cmd,
		"media channel <0-1> (main|sub) multicast A.B.C.D <1024-65530> local A.B.C.D enable" ,
		MEDIA_CHANNEL_STR
		"Channel Number Select\n"
		"Main Configure\n"
		"Submain Configure\n"
		"Multicast Configure\n"
		CMD_KEY_IPV4_HELP
		"Multicast Port Value\n"
		"Local Configure\n"
		CMD_KEY_IPV4_HELP
		"Enable\n")
{
	int ret = ERROR;
	ZPL_MEDIA_CHANNEL_E channel = -1;
	ZPL_MEDIA_CHANNEL_TYPE_E channel_index = ZPL_MEDIA_CHANNEL_TYPE_SUB;
	zpl_media_channel_t	*chn = NULL;
	VTY_GET_INTEGER("channel",channel, argv[0]);
	
	if(strstr(argv[1],"main"))
		channel_index = ZPL_MEDIA_CHANNEL_TYPE_MAIN;
	else if(strstr(argv[1],"sub"))
		channel_index = ZPL_MEDIA_CHANNEL_TYPE_SUB;

	chn = zpl_media_channel_lookup(channel,  channel_index);
	if(chn)
	{
		if(!zpl_media_channel_multicast_state(channel,  channel_index))
			ret = zpl_media_channel_multicast_enable(channel,  channel_index, zpl_true, argv[2], atoi(argv[3]), argv[4]);
		else
			ret = OK;	
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (media_channel_multicast_disable,
		media_channel_multicast_disable_cmd,
		"media channel <0-1> (main|sub) multicast disable" ,
		MEDIA_CHANNEL_STR
		"Channel Number Select\n"
		"Main Configure\n"
		"Submain Configure\n"
		"Multicast Configure\n"
		"Disable\n")
{
	int ret = ERROR;
	ZPL_MEDIA_CHANNEL_E channel = -1;
	ZPL_MEDIA_CHANNEL_TYPE_E channel_index = ZPL_MEDIA_CHANNEL_TYPE_SUB;
	zpl_media_channel_t	*chn = NULL;
	VTY_GET_INTEGER("channel",channel, argv[0]);
	
	if(strstr(argv[1],"main"))
		channel_index = ZPL_MEDIA_CHANNEL_TYPE_MAIN;
	else if(strstr(argv[1],"sub"))
		channel_index = ZPL_MEDIA_CHANNEL_TYPE_SUB;

	chn = zpl_media_channel_lookup(channel,  channel_index);
	if(chn )
	{
		if(zpl_media_channel_multicast_state(channel,  channel_index))
			ret = zpl_media_channel_multicast_enable(channel,  channel_index, zpl_false, NULL, 0, NULL);
		else
			ret = OK;	
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
DEFUN (show_video_channel_sadadinfo,
		show_video_channel_sadadinfo_cmd,
		"show rtp sched" ,
		SHOW_STR
		MEDIA_CHANNEL_STR
		"Information\n")
{
	rtp_sched_test();
	return CMD_SUCCESS;
}
#endif

DEFUN (show_video_channel_info,
		show_video_channel_info_cmd,
		"show media channel info" ,
		SHOW_STR
		MEDIA_CHANNEL_STR
		"Information\n")
{
	int ret = ERROR;
	ret = zpl_media_channel_show(vty);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (show_video_encode_info,
		show_video_encode_info_cmd,
		"show media channel encode info" ,
		SHOW_STR
		MEDIA_CHANNEL_STR
		"Encode Channel Configure\n"
		"Information\n")
{
	int ret = ERROR;
	ret = zpl_media_video_encode_show(vty);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (show_video_vpsschn_info,
		show_video_vpsschn_info_cmd,
		"show media channel vpsschn info" ,
		SHOW_STR
		MEDIA_CHANNEL_STR
		"Vpss Channel Configure\n"
		"Information\n")
{
	int ret = ERROR;
	ret = zpl_media_video_vpsschn_show(vty);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (show_video_inputchn_info,
		show_video_inputchn_info_cmd,
		"show media channel inputchn info" ,
		SHOW_STR
		MEDIA_CHANNEL_STR
		"Input Channel Configure\n"
		"Information\n")
{
	int ret = ERROR;
	ret = zpl_media_video_inputchn_show(vty);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}




DEFUN (mediaservice_template,
		mediaservice_template_cmd,
		"template multimedia",
		"Template Configure\n"
		"Mediaservice Configure\n")
{
	template_t * temp = lib_template_lookup_name (zpl_false, "multimedia");
	if(temp)
	{
		vty->node = TEMPLATE_NODE;
		memset(vty->prompt, 0, sizeof(vty->prompt));
		sprintf(vty->prompt, "%s", temp->prompt);

		return CMD_SUCCESS;
	}
	else
	{
		temp = lib_template_new (zpl_false);
		if(temp)
		{
			temp->module = 0;
			strcpy(temp->name, "multimedia");
			strcpy(temp->prompt, "multimedia"); /* (config-app-esp)# */
			temp->write_template = mediaservice_write_config;
			temp->pVoid = NULL;
			lib_template_install(temp, 0);

			vty->node = TEMPLATE_NODE;
			memset(vty->prompt, 0, sizeof(vty->prompt));
			sprintf(vty->prompt, "%s", temp->prompt);
			return CMD_SUCCESS;
		}
	}
	return CMD_WARNING;
}

DEFUN (no_mediaservice_template,
		no_mediaservice_template_cmd,
		"no template multimedia",
		NO_STR
		"Template Configure\n"
		"Mediaservice Configure\n")
{
	template_t * temp = lib_template_lookup_name (zpl_false, "multimedia");
	if(temp)
	{
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
			(chn->channel_index==ZPL_MEDIA_CHANNEL_TYPE_MAIN)?"main":"sub", VTY_NEWLINE);

		if(ZPL_TST_BIT(chn->flags, ZPL_MEDIA_STATE_START))
			vty_out(vty, " media channel %d %s %s%s", chn->channel, 
				(chn->channel_index==ZPL_MEDIA_CHANNEL_TYPE_MAIN)?"main":"sub", 
				(ZPL_TST_BIT(chn->flags, ZPL_MEDIA_STATE_START))?"active":"inactive", VTY_NEWLINE);

		if(chn->p_record.enable)
			vty_out(vty, " media channel %d %s record %s%s", chn->channel, 
				(chn->channel_index==ZPL_MEDIA_CHANNEL_TYPE_MAIN)?"main":"sub", 
				chn->p_record.enable?"enable":"disable", VTY_NEWLINE);

		if(chn->p_capture.enable)
			vty_out(vty, " media channel %d %s alarm capture %s%s", chn->channel, 
				(chn->channel_index==ZPL_MEDIA_CHANNEL_TYPE_MAIN)?"main":"sub", 
				chn->p_capture.enable?"enable":"disable", VTY_NEWLINE);

#ifdef ZPL_LIBORTP_MODULE
		if(chn->p_mucast.enable)
		{
			zpl_mediartp_session_t* my_session = chn->p_mucast.param;
			if(my_session)
				vty_out(vty, " media channel %d %s multicast %s %d %s enable%s", chn->channel, 
					(chn->channel_index==ZPL_MEDIA_CHANNEL_TYPE_MAIN)?"main":"sub", 
					my_session->address, my_session->rtp_port, my_session->local_address, VTY_NEWLINE);
		}
#endif
		return OK;
	}
	else
		return OK;
}

static int mediaservice_write_config(struct vty *vty, void *pVoid)
{
	vty_out(vty, "template multimedia%s",VTY_NEWLINE);
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
			strcpy(temp->name, "multimedia");
			strcpy(temp->prompt, "multimedia"); /* (config-app-esp)# */
			temp->pVoid = NULL;
			temp->write_template = mediaservice_write_config;
			lib_template_install(temp, 2);
		}
		install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mediaservice_template_cmd);
		install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_mediaservice_template_cmd);

		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_enable_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_enable_alsa_cmd);

/*		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_encode_enable_cmd);
		
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_encode_connect_vpsschn_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_encode_connect_vpsschn_alias_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_vpsschn_enable_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_vpsschn_connect_inputchn_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_vpsschn_connect_inputchn_alias_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_inputchn_enable_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_inputchn_mipidev_cmd);
*/
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_record_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_alarm_capture_cmd);
#ifdef ZPL_LIBORTP_MODULE
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_multicast_enable_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_multicast_disable_cmd);
		install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &show_video_channel_sadadinfo_cmd);
#endif
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_osd_cmd);
	}
	return;
}

void cmd_video_init(void)
{
	cmd_mediaservice_init();

	install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_video_vpsschn_info_cmd);
	install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_video_inputchn_info_cmd);
	install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_video_encode_info_cmd);
	install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_video_channel_info_cmd);
}

#endif