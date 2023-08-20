/*
 * zpl_media_cmd.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */
#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "zpl_media_api.h"

#ifdef ZPL_SHELL_MODULE

#define MEDIA_CHANNEL_STR	"MultiMedia Configure\n" \
							"MultiMedia Channel Select\n"

static int mediaservice_write_config(struct vty *vty, void *pVoid);

/*
* media channel <0-1> (main|sub)
* media channel <0-1> (main|sub) (destroy|start|stop|requst-idr|reset)
* media channel <0-1> (main|sub) record enable
* media channel <0-1> (main|sub) capture enable
* media channel <0-1> (main|sub) osd (channel|datetime|bitrate|label|other) (enable|disable)
media channel <0-1> (main|sub) osd (channel|datetime|bitrate|label|other) bgalpha <0-128> fgalpha <5-110>
media channel <0-1> (main|sub) osd (label|other) keystr .LINE
media channel <0-1> (main|sub) osd (channel|datetime|bitrate|label|other) x <1-4096> y <1-4096>

media channel audio <0-1> (input|output)
media channel audio <0-1> (input|output) (destroy|start|stop)
media channel audio <0-1> connect local-output
media channel audio codec (enable|disable)
media channel audio codec (micgain|boost|in-volume|out-volume) value <0-100>

media channel <0-1> (main|sub) record (enable|disable)
media channel <0-1> (main|sub) alarm capture (enable|disable)
media channel <0-1> (main|sub) multicast A.B.C.D <1024-65530> local A.B.C.D enable
media channel <0-1> (main|sub) multicast disable
show media channel <0-1> (main|sub) extradata
show media channel extradata (brief|)
show media channel <0-1> (main|sub) extradata (brief|)
show media channel info
show media channel encode info
show media channel vpsschn info
show media channel inputchn info
show media channel rtp-session info
show media channel audio info
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
		"media channel <0-1> (main|sub) (destroy|start|stop|requst-idr|reset)" ,
		MEDIA_CHANNEL_STR
		"Channel Number Select\n"
		"Main Channel Configure\n"
		"Submain Channel Configure\n"
		"Destroy\n"
		"Start\n"
		"Stop\n"
		"Requst IDR\n"
		"Reset Channel\n")
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
	else if(strstr(argv[2],"start"))
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
	else if(strstr(argv[2],"requst"))
	{
		if(zpl_media_channel_lookup(channel,  channel_index) == NULL)
		{
			vty_out(vty, " media channel %d %s is not exist.%s", channel, argv[1], VTY_NEWLINE);
		}
		else
		{
			if(ZPL_TST_BIT(zpl_media_channel_state(channel,  channel_index), ZPL_MEDIA_STATE_START))
			{
				zpl_media_channel_t *mchn = zpl_media_channel_lookup(channel,  channel_index);
				if(mchn)
					ret = zpl_media_channel_hal_request_IDR(mchn);
			}
			else
			{
				vty_out(vty, " media channel %d %s is not active %s", channel, argv[1], VTY_NEWLINE);
			}		
		}
	}
	else if(strstr(argv[2],"reset"))
	{
		if(zpl_media_channel_lookup(channel,  channel_index) == NULL)
		{
			vty_out(vty, " media channel %d %s is not exist.%s", channel, argv[1], VTY_NEWLINE);
		}
		else
		{
			ret = zpl_media_channel_reset(channel,  channel_index, 0);	
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (media_channel_osd,
		media_channel_osd_cmd,
		"media channel <0-1> (main|sub) osd (channel|datetime|bitrate|label|other) (enable|disable)" ,
		MEDIA_CHANNEL_STR
		"Channel Number Select\n"
		"Main Configure\n"
		"Submain Configure\n"
		"OSD Configure\n"
		"Channel Information\n"
		"Datetime Information\n"
		"Bitrate Information\n"
		"Label Information\n"
		"Other Information\n"
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
		if(zpl_media_channel_area_lookup(mchannel, area_type))
		{
			ret = zpl_media_channel_area_osd_active(mchannel, osd_type, 1);
			return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
		}
		marea = zpl_media_channel_area_create(mchannel, ZPL_MEDIA_AREA_OSD);
		if(marea == NULL)
		{
			vty_out(vty, " media channel %d %s create area of %s failed.%s", channel, argv[1], argv[2], VTY_NEWLINE);
			return CMD_WARNING;
		}
		ret = zpl_media_channel_area_add(mchannel, marea);
		if(ret == OK)
			ret = zpl_media_channel_area_osd_active(mchannel, osd_type, 1);
	}
	if(strncmp(argv[3],"disable", 4) == 0)
	{
		marea = zpl_media_channel_area_lookup(mchannel, area_type);
		if(marea)
		{
			ret = zpl_media_channel_area_osd_active(mchannel, osd_type, 0);
			if(ret == OK)
				ret = zpl_media_channel_area_del(mchannel, area_type);
			if(ret == OK)	
				ret = zpl_media_channel_area_destroy(marea);
		}
		else
			ret = OK;
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (media_channel_alpha_osd,
		media_channel_osd_alpha_cmd,
		"media channel <0-1> (main|sub) osd (channel|datetime|bitrate|label|other) bgalpha <0-128> fgalpha <5-110>" ,
		MEDIA_CHANNEL_STR
		"Channel Number Select\n"
		"Main Configure\n"
		"Submain Configure\n"
		"OSD Configure\n"
		"Channel Information\n"
		"Datetime Information\n"
		"Bitrate Information\n"
		"Label Information\n"
		"Other Information\n"
		"Backgroud Alpha Config\n"
		"Backgroud Alpha Value\n"
		"Alpha Config\n"
		"Alpha Value\n")
{
	int ret = ERROR, balpha = 0, falpha = 0;
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
	else if(strncmp(argv[2],"other", 4) == 0)
		osd_type = ZPL_MEDIA_OSD_OTHER;
	balpha = atoi(argv[3]);
	falpha = atoi(argv[4]);
	mchannel = zpl_media_channel_lookup(channel,  channel_index);
	if(mchannel == NULL)
	{
		vty_out(vty, " media channel %d %s is not exist.%s", channel, argv[1], VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(zpl_media_channel_area_lookup(mchannel, area_type))
	{
		ret = zpl_media_channel_area_osd_alpha(mchannel, osd_type,  balpha, falpha);
	}
	else
	{
		marea = zpl_media_channel_area_create(mchannel, ZPL_MEDIA_AREA_OSD);
		if(marea == NULL)
		{
			vty_out(vty, " media channel %d %s create area of %s failed.%s", channel, argv[1], argv[2], VTY_NEWLINE);
			return CMD_WARNING;
		}
		ret = zpl_media_channel_area_add(mchannel, marea);
		if(ret == OK)
			ret = zpl_media_channel_area_osd_alpha(mchannel, osd_type,  balpha, falpha);
		if(ret != OK)
		{	
			vty_out(vty, " media channel %d %s of area %s is not exist.%s", channel, argv[1], argv[2], VTY_NEWLINE);
			return CMD_WARNING;
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (media_channel_keystr_osd,
		media_channel_osd_keystr_cmd,
		"media channel <0-1> (main|sub) osd (label|other) keystr .LINE" ,
		MEDIA_CHANNEL_STR
		"Channel Number Select\n"
		"Main Configure\n"
		"Submain Configure\n"
		"OSD Configure\n"
		"Channel Information\n"
		"Datetime Information\n"
		"Bitrate Information\n"
		"Label Information\n"
		"Other Information\n"
		"Point X\n"
		"X Value\n"
		"Point Y\n"
		"Y Value\n")
{
	int ret = ERROR;
	char *message;
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
	else if(strncmp(argv[2],"other", 4) == 0)
		osd_type = ZPL_MEDIA_OSD_OTHER;

	message = argv_concat(argv, argc, 3);

	mchannel = zpl_media_channel_lookup(channel,  channel_index);
	if(mchannel == NULL)
	{
		vty_out(vty, " media channel %d %s is not exist.%s", channel, argv[1], VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(zpl_media_channel_area_lookup(mchannel, area_type))
	{
		ret = zpl_media_channel_areaosd_keystr(mchannel, osd_type, message);
	}
	else
	{
		marea = zpl_media_channel_area_create(mchannel, ZPL_MEDIA_AREA_OSD);
		if(marea == NULL)
		{
			vty_out(vty, " media channel %d %s create area of %s failed.%s", channel, argv[1], argv[2], VTY_NEWLINE);
			return CMD_WARNING;
		}
		ret = zpl_media_channel_area_add(mchannel, marea);
		if(ret == OK)
			ret = zpl_media_channel_areaosd_keystr(mchannel, osd_type, message);
		if(ret != OK)
		{	
			vty_out(vty, " media channel %d %s of area %s is not exist.%s", channel, argv[1], argv[2], VTY_NEWLINE);
			return CMD_WARNING;
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (media_channel_point_osd,
		media_channel_osd_point_cmd,
		"media channel <0-1> (main|sub) osd (channel|datetime|bitrate|label|other) x <1-4096> y <1-4096>" ,
		MEDIA_CHANNEL_STR
		"Channel Number Select\n"
		"Main Configure\n"
		"Submain Configure\n"
		"OSD Configure\n"
		"Channel Information\n"
		"Datetime Information\n"
		"Bitrate Information\n"
		"Label Information\n"
		"Other Information\n"
		"Point X\n"
		"X Value\n"
		"Point Y\n"
		"Y Value\n")
{
	int ret = ERROR, x = 10, y = 10;
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
	else if(strncmp(argv[2],"other", 4) == 0)
		osd_type = ZPL_MEDIA_OSD_OTHER;
	x = atoi(argv[3]);
	y = atoi(argv[4]);

	mchannel = zpl_media_channel_lookup(channel,  channel_index);
	if(mchannel == NULL)
	{
		vty_out(vty, " media channel %d %s is not exist.%s", channel, argv[1], VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(zpl_media_channel_area_lookup(mchannel, area_type))
	{
		zpl_point_t m_point;
		m_point.x = x;
		m_point.y = y;
		ret = zpl_media_channel_area_osd_point(mchannel, osd_type, m_point);
	}
	else
	{
		marea = zpl_media_channel_area_create(mchannel, ZPL_MEDIA_AREA_OSD);
		if(marea == NULL)
		{
			vty_out(vty, " media channel %d %s create area of %s failed.%s", channel, argv[1], argv[2], VTY_NEWLINE);
			return CMD_WARNING;
		}
		ret = zpl_media_channel_area_add(mchannel, marea);
		if(ret == OK)
		{
			zpl_point_t m_point;
			m_point.x = x;
			m_point.y = y;
			ret = zpl_media_channel_area_osd_point(mchannel, osd_type, m_point);
		}
		if(ret != OK)
		{	
			vty_out(vty, " media channel %d %s of area %s is not exist.%s", channel, argv[1], argv[2], VTY_NEWLINE);
			return CMD_WARNING;
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (media_channel_audio_enable,
		media_channel_audio_enable_cmd,
		"media channel audio <0-1> (input|output)" ,
		MEDIA_CHANNEL_STR
		"Audio Channel Configure\n"
		"Channel Number Select\n"
		"Input Channel\n"
		"Output Channel\n")
{
	int ret = ERROR;
	ZPL_MEDIA_CHANNEL_E channel = ZPL_MEDIA_CHANNEL_AUDIO_0;
	ZPL_MEDIA_CHANNEL_TYPE_E channel_index = ZPL_MEDIA_CHANNEL_TYPE_INPUT;
	
	channel = ZPL_MEDIA_CHANNEL_AUDIO_0 + atoi(argv[0]);
	if(strstr(argv[1],"input"))
		channel_index = ZPL_MEDIA_CHANNEL_TYPE_INPUT;
	else if(strstr(argv[1],"output"))
		channel_index = ZPL_MEDIA_CHANNEL_TYPE_OUTPUT;

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

DEFUN_HIDDEN (media_channel_audio_enable_alsa,
		media_channel_audio_enable_alsa_cmd,
		"media channel audio <0-1> (input|output) (destroy|start|stop)" ,
		MEDIA_CHANNEL_STR
		"Audio Channel Configure\n"
		"Channel Number Select\n"
		"Input Channel\n"
		"Output Channel\n"
		"Destroy\n"
		"Start\n"
		"Stop\n")
{
	int ret = ERROR;
	ZPL_MEDIA_CHANNEL_E channel = ZPL_MEDIA_CHANNEL_AUDIO_0;
	ZPL_MEDIA_CHANNEL_TYPE_E channel_index = ZPL_MEDIA_CHANNEL_TYPE_INPUT;
	
	channel = ZPL_MEDIA_CHANNEL_AUDIO_0 + atoi(argv[0]);
	if(strstr(argv[1],"input"))
		channel_index = ZPL_MEDIA_CHANNEL_TYPE_INPUT;
	else if(strstr(argv[1],"output"))
		channel_index = ZPL_MEDIA_CHANNEL_TYPE_OUTPUT;

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
	else if(strstr(argv[2],"start"))
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

DEFUN (media_channel_audio_connect_loutput,
		media_channel_audio_connect_loutput_cmd,
		"media channel audio <0-1> connect local-output" ,
		MEDIA_CHANNEL_STR
		"Audio Channel Configure\n"
		"Channel Number Select\n"
		"Connect Configure\n"
		"Local Output Channel\n")
{
	int ret = ERROR;
	ZPL_MEDIA_CHANNEL_E channel = ZPL_MEDIA_CHANNEL_AUDIO_0;
	zpl_media_audio_channel_t *audio = NULL;
	channel = ZPL_MEDIA_CHANNEL_AUDIO_0 + atoi(argv[0]);

	audio = zpl_media_audio_lookup(channel, zpl_true);

	if(audio != NULL)
	{
		ret = zpl_media_audio_connect_local_output(audio, ZPL_MEDIA_CONNECT_SW, channel);
	}
	else
	{
		vty_out(vty, " media channel %d %s is not exist. %s", channel, argv[1], VTY_NEWLINE);	
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (media_channel_audio_volume,
		media_channel_audio_volume_cmd,
		"media channel audio <0-1> volume <0-100>" ,
		MEDIA_CHANNEL_STR
		"Audio Channel Configure\n"
		"Channel Number Select\n"
		"Volume Configure\n"
		"Local Output Channel\n")
{
	int ret = ERROR;
	ZPL_MEDIA_CHANNEL_E channel = ZPL_MEDIA_CHANNEL_AUDIO_0;
	zpl_media_audio_channel_t *audio = NULL;
	channel = ZPL_MEDIA_CHANNEL_AUDIO_0 + atoi(argv[0]);
	int value = atoi(argv[1]);
	audio = zpl_media_audio_lookup(channel, zpl_true);

	if(audio != NULL)
	{
		ret = zpl_media_audio_volume(audio, value);
	}
	else
	{
		vty_out(vty, " media channel %d %s is not exist. %s", channel, argv[1], VTY_NEWLINE);	
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (media_channel_audio_inner_codec,
		media_channel_audio_inner_codec_cmd,
		"media channel audio codec (enable|disable)" ,
		MEDIA_CHANNEL_STR
		"Audio Channel Configure\n"
		"Codec Configure\n"
		"Enable Configure\n"
		"Disable Configure\n")
{
	int ret = ERROR;
	zpl_bool codec_enable = zpl_false;
	ZPL_MEDIA_CHANNEL_E channel = ZPL_MEDIA_CHANNEL_AUDIO_0;
	zpl_media_audio_channel_t *audio = NULL;
	channel = ZPL_MEDIA_CHANNEL_AUDIO_0;
	if(strncmp(argv[0],"enable", 4) == 0)
		codec_enable = zpl_true;
	audio = zpl_media_audio_lookup(channel, zpl_true);

	if(audio != NULL)
	{
		ret = zpl_media_audio_codec_enable(audio, codec_enable);
	}
	else
	{
		vty_out(vty, " media channel %d %s is not exist. %s", channel, argv[1], VTY_NEWLINE);	
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (media_channel_audio_codec_param,
		media_channel_audio_codec_param_cmd,
		"media channel audio codec (micgain|boost|in-volume|out-volume) value <0-100>" ,
		MEDIA_CHANNEL_STR
		"Audio Channel Configure\n"
		"Codec Configure\n"
		"Mic Gain Configure,[0,16]\n"
		"Boost Configure,[0,1]\n"
		"Input Volume Configure,[19,50]\n"
		"Output Volume Configure,[-121, 6]\n"
		"Value Configure\n"
		"Value\n")
{
	int ret = ERROR;
	int value = 0;
	ZPL_MEDIA_CHANNEL_E channel = ZPL_MEDIA_CHANNEL_AUDIO_0;
	zpl_media_audio_channel_t *audio = NULL;
	channel = ZPL_MEDIA_CHANNEL_AUDIO_0;
	value = atoi(argv[1]);
	audio = zpl_media_audio_lookup(channel, zpl_true);

	if(audio != NULL)
	{
		if(strncmp(argv[0],"micgain", 5) == 0)
		{
			ret = zpl_media_audio_codec_micgain(audio, value);//0-16
		}
		else if(strncmp(argv[0],"boost", 5) == 0)
		{
			ret = zpl_media_audio_codec_boost(audio, value?1:0);
		}
		else if(strncmp(argv[0],"in-volume", 5) == 0)
		{
			ret = zpl_media_audio_codec_input_volume(audio, value);//[19,50]
		}
		else if(strncmp(argv[0],"out-volume", 5) == 0)
		{
			ret = zpl_media_audio_codec_output_volume(audio, value-94);//[-121, 6]
		}
	}
	else
	{
		vty_out(vty, " media channel %d %s is not exist. %s", channel, argv[1], VTY_NEWLINE);	
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


static int show_video_channel_callback(zpl_media_channel_t *chn, zpl_void *obj)
{
	struct vty *vty = (struct vty *)obj;
	zpl_video_extradata_t lextradata;
	int brief = 1;
	zpl_char hexformat[2048];
	char *media_typestr[] = {"unknow", "video", "audio"};

	if (chn && vty)
	{
		if (chn && chn->media_type == ZPL_MEDIA_VIDEO)
		{
			vty_out(vty, "-----------------------------------------%s", VTY_NEWLINE);
			memset(&lextradata, 0, sizeof(zpl_video_extradata_t));
			zpl_media_channel_extradata_get(chn, &lextradata);
			vty_out(vty, "channel            : %d/%d%s", chn->channel, chn->channel_index, VTY_NEWLINE);
			vty_out(vty, " type              : %s%s", media_typestr[chn->media_type], VTY_NEWLINE);
			if (lextradata.fPPSSize)
			{
				vty_out(vty, " PPS Len           : %d%s", lextradata.fPPSSize, VTY_NEWLINE);
				if (brief)
				{
					memset(hexformat, 0, sizeof(hexformat));
					os_loghex(hexformat, sizeof(hexformat), lextradata.fPPS, lextradata.fPPSSize);
					vty_out(vty, "  PPS Date         : %s%s", hexformat, VTY_NEWLINE);
				}
			}
			if (lextradata.fSPSSize)
			{
				vty_out(vty, " SPS Len           : %d%s", lextradata.fSPSSize, VTY_NEWLINE);
				if (brief)
				{
					memset(hexformat, 0, sizeof(hexformat));
					os_loghex(hexformat, sizeof(hexformat), lextradata.fSPS, lextradata.fSPSSize);
					vty_out(vty, "  SPS Date         : %s%s", hexformat, VTY_NEWLINE);
				}
			}
			if (lextradata.fVPSSize)
			{
				vty_out(vty, " VPS Len           : %d%s", lextradata.fVPSSize, VTY_NEWLINE);
				if (brief)
				{
					memset(hexformat, 0, sizeof(hexformat));
					os_loghex(hexformat, sizeof(hexformat), lextradata.fVPS, lextradata.fVPSSize);
					vty_out(vty, "  VPS Date         : %s%s", hexformat, VTY_NEWLINE);
				}
			}
			if (lextradata.fSEISize)
			{
				vty_out(vty, " SEI Len           : %d%s", lextradata.fSEISize, VTY_NEWLINE);
				if (brief)
				{
					memset(hexformat, 0, sizeof(hexformat));
					os_loghex(hexformat, sizeof(hexformat), lextradata.fSEI, lextradata.fSEISize);
					vty_out(vty, "  SEI Date         : %s%s", hexformat, VTY_NEWLINE);
				}
			}
		}
	}
	return OK;
}

DEFUN (show_video_channel_extradata_info,
		show_video_channel_extradata_info_cmd,
		"show media channel <0-1> (main|sub) extradata" ,
		SHOW_STR
		MEDIA_CHANNEL_STR
		"Channel Number Select\n"
		"Main Configure\n"
		"Submain Configure\n"
		"Extradata Configure\n")
{
	int ret = ERROR;
	int brief = 1;
	ZPL_MEDIA_CHANNEL_E channel = ZPL_MEDIA_CHANNEL_NONE;
	ZPL_MEDIA_CHANNEL_TYPE_E channel_index = ZPL_MEDIA_CHANNEL_TYPE_NONE;
	if(argv[0][0] != 'b')
	{
		VTY_GET_INTEGER("channel",channel, argv[0]);
		if(strstr(argv[1],"main"))
			channel_index = ZPL_MEDIA_CHANNEL_TYPE_MAIN;
		else if(strstr(argv[1],"sub"))
			channel_index = ZPL_MEDIA_CHANNEL_TYPE_SUB;
	}
	else
		brief = 1;	
	if(argc >= 3)
		brief = 1;	
	ret = zpl_media_channel_foreach(show_video_channel_callback, vty);	
	//ret = zpl_media_channel_extradata_show( channel, channel_index, brief, vty);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS (show_video_channel_extradata_info,
		show_video_channel_extradata_brief_cmd,
		"show media channel extradata (brief|)" ,
		SHOW_STR
		MEDIA_CHANNEL_STR
		"Extradata Configure\n"
		"Brief Information\n")

ALIAS (show_video_channel_extradata_info,
		show_video_channel_extradata_info_brief_cmd,
		"show media channel <0-1> (main|sub) extradata (brief|)" ,
		SHOW_STR
		MEDIA_CHANNEL_STR
		"Channel Number Select\n"
		"Main Configure\n"
		"Submain Configure\n"
		"Extradata Configure\n"
		"Brief Information\n")

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


DEFUN (show_media_rtpsess_info,
		show_media_rtpsess_info_cmd,
		"show media channel rtp-session info" ,
		SHOW_STR
		MEDIA_CHANNEL_STR
		"Rtp Session Configure\n"
		"Information\n")
{
	int ret = ERROR;
	ret = zpl_mediartp_session_show(vty);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (show_audio_chn_info,
		show_audio_chn_info_cmd,
		"show media channel audio info" ,
		SHOW_STR
		MEDIA_CHANNEL_STR
		"Audio Configure\n"
		"Information\n")
{
	int ret = ERROR;
	ret = zpl_media_audio_show(vty);
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

DEFUN (media_channel_request_hdr,
		media_channel_request_hdr_cmd,
		"media channel <0-1> (main|sub) request-hdr" ,
		MEDIA_CHANNEL_STR
		"Channel Number Select\n"
		"Main Configure\n"
		"Submain Configure\n"
		"Request HDR Configure\n")
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
		zpl_media_channel_hal_request_IDR(chn);
		ret = OK;
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

static int media_write_config_one(zpl_media_channel_t *chn, struct vty *vty)
{
	if(chn)
	{
		if(chn->media_type == ZPL_MEDIA_VIDEO)
		{
			vty_out(vty, " media channel %d %s enable%s", chn->channel, 
				(chn->channel_index==ZPL_MEDIA_CHANNEL_TYPE_MAIN)?"main":"sub", VTY_NEWLINE);

			//media channel <0-1> (main|sub) record (enable|disable)
			//media channel <0-1> (main|sub) alarm capture (enable|disable)

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
#if 0
media channel <0-1> (main|sub) multicast A.B.C.D <1024-65530> local A.B.C.D enable
media channel <0-1> (main|sub) multicast disable
#endif
			zpl_media_channel_area_config(chn, vty);
		}
		else if(chn->media_type == ZPL_MEDIA_AUDIO)
		{
			zpl_media_audio_channel_t *audio = chn->media_param.audio_media.halparam;
			vty_out(vty, " media channel audio %d %s enable%s", chn->channel-ZPL_MEDIA_CHANNEL_AUDIO_0, 
				(chn->channel_index==ZPL_MEDIA_CHANNEL_TYPE_INPUT)?"input":"output", VTY_NEWLINE);
/*
			if(ZPL_TST_BIT(chn->flags, ZPL_MEDIA_STATE_START))
				vty_out(vty, " media channel audio %d %s %s%s", chn->channel-ZPL_MEDIA_CHANNEL_AUDIO_0, 
					(chn->channel_index==ZPL_MEDIA_CHANNEL_TYPE_INPUT)?"input":"output", 
					(ZPL_TST_BIT(chn->flags, ZPL_MEDIA_STATE_START))?"active":"inactive", VTY_NEWLINE);
*/
			//media channel audio <0-1> volume <0-100>
			//media channel audio codec (micgain|boost|in-volume|out-volume) value <0-100>
			if(audio)
			{
				if(audio->b_input)
				{
					if(audio->audio_param.input.hw_connect_out/*&& audio->audio_param.input.output*/)
						vty_out(vty, " media channel audio %d connect local-output%s", chn->channel-ZPL_MEDIA_CHANNEL_AUDIO_0, VTY_NEWLINE);
				}
				
				if(!audio->b_input)
					vty_out(vty, " media channel audio %d volume %d%s", chn->channel-ZPL_MEDIA_CHANNEL_AUDIO_0, audio->audio_param.output.volume, VTY_NEWLINE);
				if(audio->b_inner_codec_enable)
				{
					vty_out(vty, " media channel audio codec enable%s", VTY_NEWLINE);
					if(audio->b_input)
					{
						vty_out(vty, " media channel audio codec micgain value %d%s", audio->micgain, VTY_NEWLINE);
						vty_out(vty, " media channel audio codec boost value %d%s", audio->boost, VTY_NEWLINE);
						vty_out(vty, " media channel audio codec in-volume value %d%s", audio->in_volume, VTY_NEWLINE);
					}
					if(!audio->b_input)
						vty_out(vty, " media channel audio codec out-volume value %d%s", audio->out_volume, VTY_NEWLINE);
				}
			}	
		}
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

		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_audio_enable_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_audio_enable_alsa_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_audio_connect_loutput_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_audio_inner_codec_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_audio_codec_param_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_audio_volume_cmd);

		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_record_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_alarm_capture_cmd);

		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_multicast_enable_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_multicast_disable_cmd);

		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_osd_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_osd_keystr_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_osd_alpha_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_osd_point_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_request_hdr_cmd);
		
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
	install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_audio_chn_info_cmd);

	install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_video_channel_extradata_info_cmd);
	install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_video_channel_extradata_brief_cmd);
	install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_video_channel_extradata_info_brief_cmd);

	install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_media_rtpsess_info_cmd);
}

#endif