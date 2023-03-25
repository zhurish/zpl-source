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


DEFUN_HIDDEN (media_channel_enable,
		media_channel_enable_cmd,
		"media channel <0-1> (main|sub) (create|destroy|start|stop)" ,
		MEDIA_CHANNEL_STR
		"Channel Number Select\n"
		"Main Channel Configure\n"
		"Submain Channel Configure\n"
		"Create\n"
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
	if(strstr(argv[2],"create"))
	{
		if(zpl_media_channel_lookup(channel,  channel_index) == NULL)
		{
			ret = zpl_media_channel_create( channel,  channel_index);
		}
		else
		{
			vty_out(vty, " media channel %d %s is already exist. %s", channel, argv[1], VTY_NEWLINE);	
		}
	}
	else if(strstr(argv[2],"destroy"))
	{
		if(zpl_media_channel_lookup(channel,  channel_index) != NULL)
		{
			/*if(ZPL_TST_BIT(zpl_media_channel_state(channel,  channel_index), ZPL_MEDIA_STATE_START))
				vty_out(vty, " media channel %d %s is already start. %s", channel, argv[1], VTY_NEWLINE);
			else if(ZPL_TST_BIT(zpl_media_channel_state(channel,  channel_index), ZPL_MEDIA_STATE_ACTIVE))
				vty_out(vty, " media channel %d %s is already active. %s", channel, argv[1], VTY_NEWLINE);	
			else*/
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


DEFUN_HIDDEN (media_encode_enable,
		media_encode_enable_cmd,
		"media channel encode <0-8> (create|destroy|active|inactive|start|stop)" ,
		MEDIA_CHANNEL_STR
		"Encode Channel Configure\n"
		"Encode Channel\n"
		"Create\n"
		"Destroy\n"
		"Active\n"
		"Inactive\n"
		"Start\n"
		"Stop\n")
{
	int ret = ERROR;
	zpl_int32 channel = -1;
	zpl_bool channel_index = zpl_false;
	channel = atoi(argv[0]);
	zpl_media_channel_t *mchannel = zpl_media_channel_lookup(0,0);
	zpl_media_video_encode_t *encode = zpl_media_video_encode_lookup(channel,  channel_index);
	if(strstr(argv[1],"create") && mchannel)
	{
		if(encode == NULL)
		{
			encode = zpl_media_video_encode_create(channel,  channel_index);
			if(encode)
				ret = OK;
		}
		else
		{
			vty_out(vty, " media encode channel %d %s is already exist. %s", channel, argv[1], VTY_NEWLINE);	
		}
	}
	else if(strstr(argv[1],"destroy"))
	{
		if(encode != NULL)
		{
			ret = zpl_media_video_encode_destroy(encode);
		}
		else
		{
			vty_out(vty, " media encode channel %d %s is not exist. %s", channel, argv[1], VTY_NEWLINE);	
		}
	}
	else if(strstr(argv[1],"active"))
	{
		if(encode == NULL)
		{
			vty_out(vty, " media encode channel %d %s is not exist.%s", channel, argv[1], VTY_NEWLINE);
		}
		else
		{
			if(!zpl_media_video_encode_state_check(encode, ZPL_MEDIA_STATE_ACTIVE))
				ret = zpl_media_video_encode_hal_create(encode);
			else
			{
				vty_out(vty, " media encode channel %d %s is already active %s", channel, argv[1], VTY_NEWLINE);
			}	
		}
	}
	else if(strstr(argv[1],"inactive"))
	{
		if(encode == NULL)
		{
			vty_out(vty, " media encode channel %d %s is not exist.%s", channel, argv[1], VTY_NEWLINE);
		}
		else
		{
			if(zpl_media_video_encode_state_check(encode, ZPL_MEDIA_STATE_ACTIVE))
				ret = zpl_media_video_encode_hal_destroy(encode);
			else
			{
				vty_out(vty, " media encode channel %d %s is not active %s", channel, argv[1], VTY_NEWLINE);
			}	
		}
	}
	if(strstr(argv[1],"start"))
	{
		if(encode == NULL)
		{
			vty_out(vty, " media encode channel %d %s is not exist.%s", channel, argv[1], VTY_NEWLINE);
		}
		else
		{
			if(zpl_media_video_encode_state_check(encode, ZPL_MEDIA_STATE_ACTIVE))
			{
				if(!zpl_media_video_encode_state_check(encode, ZPL_MEDIA_STATE_START))
					ret = zpl_media_video_encode_start(mchannel->t_master,  encode);
				else
				{
					vty_out(vty, " media encode channel %d %s is already start %s", channel, argv[1], VTY_NEWLINE);
				}	
			}
			else
			{
				vty_out(vty, " media encode channel %d %s is not active %s", channel, argv[1], VTY_NEWLINE);
			}	
		}
	}
	else if(strstr(argv[1],"stop"))
	{
		if(encode == NULL)
		{
			vty_out(vty, " media encode channel %d %s is not exist.%s", channel, argv[1], VTY_NEWLINE);
		}
		else
		{
			if(zpl_media_video_encode_state_check(encode, ZPL_MEDIA_STATE_ACTIVE))
			{
				if(zpl_media_video_encode_state_check(encode, ZPL_MEDIA_STATE_START))
					ret = zpl_media_video_encode_stop(encode);
				else
				{
					vty_out(vty, " media encode channel %d %s is not start %s", channel, argv[1], VTY_NEWLINE);
				}	
			}
			else
			{
				vty_out(vty, " media encode channel %d %s is not active %s", channel, argv[1], VTY_NEWLINE);
			}		
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN_HIDDEN (media_encode_connect_vpsschn,
		media_encode_connect_vpsschn_cmd,
		"media channel encode <0-8> connect vpsschn <0-4> <0-2>" ,
		MEDIA_CHANNEL_STR
		"Encode Channel Configure\n"
		"Encode Channel\n"
		"Connect\n"
		"Vpss Channel Configure\n"
		"Vpss Group\n"
		"Vpss Channel\n")
{
	int ret = ERROR;
	zpl_int32 encchannel = -1;
	zpl_bool ecnchannel_type = zpl_false;
	zpl_int32 vpss_group = -1;
	zpl_int32 vpss_channel = -1;
	zpl_bool hwbind = zpl_false;
	encchannel = atoi(argv[0]);
	vpss_group = atoi(argv[1]);
	vpss_channel = atoi(argv[2]);
	zpl_media_video_vpsschn_t *vpsschn = zpl_media_video_vpsschn_lookup(vpss_group, vpss_channel);
	zpl_media_video_encode_t *encode = zpl_media_video_encode_lookup(encchannel,  ecnchannel_type);
	if(encode == NULL)
	{
		vty_out(vty, " media encode channel %d is not exist %s", encchannel, VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	if(vpsschn == NULL)
	{
		vty_out(vty, " media vpsschn group %d channel %d is not exist %s", vpss_group, vpss_channel, VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	if(argc == 4)
		hwbind = zpl_true;
	ret = zpl_media_video_encode_source_set(encchannel, vpsschn);
	ret |= zpl_media_video_vpsschn_connect(vpss_group, vpss_channel, encchannel, hwbind);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS_HIDDEN(media_encode_connect_vpsschn,
		media_encode_connect_vpsschn_alias_cmd,
		"media channel encode <0-8> connect vpsschn <0-4> <0-2> (hwbind|)" ,
		MEDIA_CHANNEL_STR
		"Encode Channel Configure\n"
		"Encode Channel\n"
		"Connect\n"
		"Vpss Channel Configure\n"
		"Vpss Group\n"
		"Vpss Channel\n"
		"Hardware Bind\n");

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



DEFUN_HIDDEN (media_vpsschn_enable,
		media_vpsschn_enable_cmd,
		"media channel vpsschn <0-4> <0-2> (create|destroy|active|inactive|start|stop)" ,
		MEDIA_CHANNEL_STR
		"Vpss Channel Configure\n"
		"Vpss Group\n"
		"Vpss Channel\n"
		"Create\n"
		"Destroy\n"
		"Active\n"
		"Inactive\n"
		"Start\n"
		"Stop\n")
{
	int ret = ERROR;
	zpl_int32 vpss_group = -1;
	zpl_int32 vpss_channel = -1;
	zpl_video_size_t output_size;
	output_size.width = 1920;			//宽度
	output_size.height = 1080;			//高度
	vpss_group = atoi(argv[0]);
	vpss_channel = atoi(argv[1]);

	zpl_media_channel_t *mchannel = zpl_media_channel_lookup(0,0);
	zpl_media_video_vpsschn_t *vpsschn = zpl_media_video_vpsschn_lookup(vpss_group, vpss_channel);
	if(strstr(argv[2],"create") && mchannel)
	{
		if(vpsschn == NULL)
		{
			vpsschn = zpl_media_video_vpsschn_create(vpss_group, vpss_channel, output_size);
			if(vpsschn)
				ret = OK;
		}
		else
		{
			vty_out(vty, " media vpss group %d channel %d is already exist. %s", vpss_group, vpss_channel, VTY_NEWLINE);	
		}
	}
	else if(strstr(argv[2],"destroy"))
	{
		if(vpsschn != NULL)
		{
			ret = zpl_media_video_vpsschn_destroy(vpsschn);
		}
		else
		{
			vty_out(vty, " media vpss group %d channel %d is not exist. %s", vpss_group, vpss_channel, VTY_NEWLINE);	
		}
	}
	else if(strstr(argv[2],"active"))
	{
		if(vpsschn == NULL)
		{
			vty_out(vty, " media vpss group %d channel %d is not exist.%s", vpss_group, vpss_channel, VTY_NEWLINE);
		}
		else
		{
			if(!zpl_media_video_vpsschn_state_check(vpsschn, ZPL_MEDIA_STATE_ACTIVE))
				ret = zpl_media_video_vpsschn_hal_create(vpsschn);
			else
			{
				vty_out(vty, " media vpss group %d channel %d is already active %s", vpss_group, vpss_channel, VTY_NEWLINE);
			}	
		}
	}
	else if(strstr(argv[2],"inactive"))
	{
		if(vpsschn == NULL)
		{
			vty_out(vty, " media vpss group %d channel %d is not exist.%s", vpss_group, vpss_channel, VTY_NEWLINE);
		}
		else
		{
			if(zpl_media_video_vpsschn_state_check(vpsschn, ZPL_MEDIA_STATE_ACTIVE))
				ret = zpl_media_video_vpsschn_hal_destroy(vpsschn);
			else
			{
				vty_out(vty, " media vpss group %d channel %d is not active %s", vpss_group, vpss_channel, VTY_NEWLINE);
			}	
		}
	}
	if(strstr(argv[2],"start"))
	{
		if(vpsschn == NULL)
		{
			vty_out(vty, " media vpss group %d channel %d is not exist.%s", vpss_group, vpss_channel, VTY_NEWLINE);
		}
		else
		{
			if(zpl_media_video_vpsschn_state_check(vpsschn, ZPL_MEDIA_STATE_ACTIVE))
			{
				if(!zpl_media_video_vpsschn_state_check(vpsschn, ZPL_MEDIA_STATE_START))
					ret = zpl_media_video_vpsschn_start(mchannel->t_master,  vpsschn);
				else
				{
					vty_out(vty, " media vpss group %d channel %d is already start %s", vpss_group, vpss_channel, VTY_NEWLINE);
				}	
			}
			else
			{
				vty_out(vty, " media vpss group %d channel %d is not active %s", vpss_group, vpss_channel, VTY_NEWLINE);
			}	
		}
	}
	else if(strstr(argv[2],"stop"))
	{
		if(vpsschn == NULL)
		{
			vty_out(vty, " media vpss group %d channel %d is not exist.%s", vpss_group, vpss_channel, VTY_NEWLINE);
		}
		else
		{
			if(zpl_media_video_vpsschn_state_check(vpsschn, ZPL_MEDIA_STATE_ACTIVE))
			{
				if(zpl_media_video_vpsschn_state_check(vpsschn, ZPL_MEDIA_STATE_START))
					ret = zpl_media_video_vpsschn_stop(vpsschn);
				else
				{
					vty_out(vty, " media vpss group %d channel %d is not start %s", vpss_group, vpss_channel, VTY_NEWLINE);
				}	
			}
			else
			{
				vty_out(vty, " media vpss group %d channel %d is not active %s", vpss_group, vpss_channel, VTY_NEWLINE);
			}		
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN_HIDDEN (media_vpsschn_connect_inputchn,
		media_vpsschn_connect_inputchn_cmd,
		"media channel vpsschn <0-4> <0-2> connect inputchn <0-1> <0-3> <0-8>" ,
		MEDIA_CHANNEL_STR
		"Vpss Channel Configure\n"
		"Vpss Group\n"
		"Vpss Channel\n"
		"Connect\n"
		"Input Channel Configure\n"
		"Input Device ID\n"
		"Input Pipe\n"
		"Input Channel\n")
{
	int ret = ERROR;
	zpl_int32 devnum = -1;
	zpl_int32 input_pipe = -1;
	zpl_int32 input_channel = -1;
	zpl_int32 vpss_group = -1;
	zpl_int32 vpss_channel = -1;
	zpl_bool hwbind = zpl_false;
	
	vpss_group = atoi(argv[0]);
	vpss_channel = atoi(argv[1]);
	devnum = atoi(argv[2]);
	input_pipe = atoi(argv[3]);
	input_channel = atoi(argv[4]);

	zpl_media_video_inputchn_t *inputchn = zpl_media_video_inputchn_lookup(devnum, input_pipe, input_channel);
	zpl_media_video_vpsschn_t *vpsschn = zpl_media_video_vpsschn_lookup(vpss_group, vpss_channel);
	if(inputchn == NULL)
	{
		vty_out(vty, " media inputchn pipe %d channel %d is not exist %s", input_pipe, input_channel, VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	if(vpsschn == NULL)
	{
		vty_out(vty, " media vpsschn group %d channel %d is not exist %s", vpss_group, vpss_channel, VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	if(argc == 6 && argv[5])
		hwbind = zpl_true;
	ret = zpl_media_video_vpsschn_source_set(vpss_group, vpss_channel, inputchn);
	ret |= zpl_media_video_inputchn_connect(inputchn, vpss_group, vpss_channel, hwbind);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS_HIDDEN(media_vpsschn_connect_inputchn,
		media_vpsschn_connect_inputchn_alias_cmd,
		"media channel vpsschn <0-4> <0-2> connect inputchn <0-1> <0-3> <0-8> (hwbind|)" ,
		MEDIA_CHANNEL_STR
		"Vpss Channel Configure\n"
		"Vpss Group\n"
		"Vpss Channel\n"
		"Connect\n"
		"Input Channel Configure\n"
		"Input Device ID\n"
		"Input Pipe\n"
		"Input Channel\n"
		"Hardware Bind\n");

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


DEFUN_HIDDEN (media_inputchn_enable,
		media_inputchn_enable_cmd,
		"media channel inputchn <0-1> <0-3> <0-8> (create|destroy|active|inactive|start|stop)" ,
		MEDIA_CHANNEL_STR
		"Input Channel Configure\n"
		"Input Device ID\n"
		"Input Pipe\n"
		"Input Channel\n"
		"Create\n"
		"Destroy\n"
		"Active\n"
		"Inactive\n"
		"Start\n"
		"Stop\n")
{
	int ret = ERROR;
	zpl_int32 input_pipe = -1;
	zpl_int32 input_channel = -1;
	zpl_int32 devnum;
	zpl_video_size_t output_size;
	output_size.width = 1920;			//宽度
	output_size.height = 1080;			//高度
	devnum = atoi(argv[0]);
	input_pipe = atoi(argv[1]);
	input_channel = atoi(argv[2]);

	zpl_media_channel_t *mchannel = zpl_media_channel_lookup(0,0);
	zpl_media_video_inputchn_t *inputchn = zpl_media_video_inputchn_lookup(devnum, input_pipe, input_channel);
	if(strstr(argv[3],"create") && mchannel)
	{
		if(inputchn == NULL)
		{
			inputchn = zpl_media_video_inputchn_create(devnum, input_pipe, input_channel, output_size);
			if(inputchn)
				ret = OK;
		}
		else
		{
			vty_out(vty, " media input pipe %d channel %d is already exist. %s", input_pipe, input_channel, VTY_NEWLINE);	
		}
	}
	else if(strstr(argv[3],"destroy"))
	{
		if(inputchn != NULL)
		{
			ret = zpl_media_video_inputchn_destroy(inputchn);
		}
		else
		{
			vty_out(vty, " media input pipe %d channel %d is not exist. %s", input_pipe, input_channel, VTY_NEWLINE);	
		}
	}
	else if(strstr(argv[3],"active"))
	{
		if(inputchn == NULL)
		{
			vty_out(vty, " media input pipe %d channel %d is not exist.%s", input_pipe, input_channel, VTY_NEWLINE);
		}
		else
		{
			if(!zpl_media_video_inputchn_state_check(inputchn, ZPL_MEDIA_STATE_ACTIVE))
				ret = zpl_media_video_inputchn_hal_create(inputchn);
			else
			{
				vty_out(vty, " media input pipe %d channel %d is already active %s", input_pipe, input_channel, VTY_NEWLINE);
			}	
		}
	}
	else if(strstr(argv[3],"inactive"))
	{
		if(inputchn == NULL)
		{
			vty_out(vty, " media input pipe %d channel %d is not exist.%s", input_pipe, input_channel, VTY_NEWLINE);
		}
		else
		{
			if(zpl_media_video_inputchn_state_check(inputchn, ZPL_MEDIA_STATE_ACTIVE))
				ret = zpl_media_video_inputchn_hal_destroy(inputchn);
			else
			{
				vty_out(vty, " media input pipe %d channel %d is not active %s", input_pipe, input_channel, VTY_NEWLINE);
			}	
		}
	}
	if(strstr(argv[3],"start"))
	{
		if(inputchn == NULL)
		{
			vty_out(vty, " media input pipe %d channel %d is not exist.%s", input_pipe, input_channel, VTY_NEWLINE);
		}
		else
		{
			if(zpl_media_video_inputchn_state_check(inputchn, ZPL_MEDIA_STATE_ACTIVE))
			{
				if(!zpl_media_video_inputchn_state_check(inputchn, ZPL_MEDIA_STATE_START))
					ret = zpl_media_video_inputchn_start(mchannel->t_master,  inputchn);
				else
				{
					vty_out(vty, " media input pipe %d channel %d is already start %s", input_pipe, input_channel, VTY_NEWLINE);
				}	
			}
			else
			{
				vty_out(vty, " media input pipe %d channel %d is not active (0x%x)%s", input_pipe, input_channel, inputchn->flags, VTY_NEWLINE);
			}	
		}
	}
	else if(strstr(argv[3],"stop"))
	{
		if(inputchn == NULL)
		{
			vty_out(vty, " media input pipe %d channel %d is not exist.%s", input_pipe, input_channel, VTY_NEWLINE);
		}
		else
		{
			if(zpl_media_video_inputchn_state_check(inputchn, ZPL_MEDIA_STATE_ACTIVE))
			{
				if(zpl_media_video_inputchn_state_check(inputchn, ZPL_MEDIA_STATE_START))
					ret = zpl_media_video_inputchn_stop(inputchn);
				else
				{
					vty_out(vty, " media input pipe %d channel %d is not start %s", input_pipe, input_channel, VTY_NEWLINE);
				}	
			}
			else
			{
				vty_out(vty, " media input pipe %d channel %d is not active %s", input_pipe, input_channel, VTY_NEWLINE);
			}		
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN_HIDDEN (media_inputchn_mipidev,
		media_inputchn_mipidev_cmd,
		"media channel inputchn <0-1> <0-3> <0-8> snstype <0-100> snsdev <0-2> mipidev <0-2>" ,
		MEDIA_CHANNEL_STR
		"Input Channel Configure\n"
		"Input Device ID\n"
		"Input Pipe\n"
		"Input Channel\n"
		"sns type\n"
		"snstype id\n"
		"snsdev\n"
		"snsdev id\n"
		"mipidev\n"
		"mipidev id\n")
{
	int ret = ERROR;
	zpl_int32 input_pipe = -1;
	zpl_int32 input_channel = -1;
	zpl_int32 devnum;
	int snstype = -1,  mipidev = -1,  snsdev = -1;

	devnum = atoi(argv[0]);
	input_pipe = atoi(argv[1]);
	input_channel = atoi(argv[2]);

	snstype = atoi(argv[3]);
	snsdev = atoi(argv[4]);
	mipidev = atoi(argv[5]);
	zpl_media_video_inputchn_t *inputchn = zpl_media_video_inputchn_lookup(devnum, input_pipe, input_channel);
	if(inputchn)
		ret = zpl_media_video_inputchn_hal_param(inputchn,  snstype,  mipidev, snsdev);
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
	//ret = zpl_media_video_vpssgrp_show(vty);
	//zpl_media_config_write("./media.json");
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
		//install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_connect_encode_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_encode_enable_cmd);
		
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_encode_connect_vpsschn_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_encode_connect_vpsschn_alias_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_vpsschn_enable_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_vpsschn_connect_inputchn_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_vpsschn_connect_inputchn_alias_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_inputchn_enable_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_inputchn_mipidev_cmd);

		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_record_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &media_channel_alarm_capture_cmd);

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