/*
 * cmd_security.c
 *
 *  Created on: Apr 20, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include <zplos_include.h>
#include "if.h"
#include "command.h"
#include "prefix.h"
#include "vty.h"
#include "nsm_include.h"
#include "hal_include.h"



/*
 * storm control
 */
DEFUN (stormcontrol_control,
		stormcontrol_control_cmd,
		"storm control (broadcast|multicast|unicast) <0-1000000000>" ,
		"Storm control\n"
		"Control\n"
		"broadcast\n"
		"multicast\n"
		"unicast\n"
		"value of bps\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_stormcontrol_enable_get_api(ifp))
	{
		ret = nsm_stormcontrol_enable_set_api(ifp, zpl_true);
	}
	if(nsm_stormcontrol_enable_get_api(ifp))
	{
		zpl_uint32 storm_unicast = 0;
		zpl_uint32 storm_multicast = 0;
		zpl_uint32 storm_broadcast = 0;

		if(strncmp(argv[0], "broadcast", 3) == 0)
		{
			storm_broadcast = atoi(argv[1]);
			ret = nsm_stormcontrol_broadcast_set_api(ifp,  storm_broadcast, NSM_STORMCONTROL_RATE);
		}
		else if(strncmp(argv[0], "multicast", 3) == 0)
		{
			storm_multicast = atoi(argv[1]);
			ret = nsm_stormcontrol_multicast_set_api(ifp,  storm_multicast, NSM_STORMCONTROL_RATE);
		}
		else if(strncmp(argv[0], "unicast", 3) == 0)
		{
			storm_unicast = atoi(argv[1]);
			ret = nsm_stormcontrol_unicast_set_api(ifp,  storm_unicast, NSM_STORMCONTROL_RATE);
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (stormcontrol_control_percent,
		stormcontrol_control_percent_cmd,
		"storm control (broadcast|multicast|unicast) percent <0-100>" ,
		"Storm control\n"
		"Control\n"
		"broadcast\n"
		"multicast\n"
		"unicast\n"
		"percent\n"
		"value of percent\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_stormcontrol_enable_get_api(ifp))
	{
		ret = nsm_stormcontrol_enable_set_api(ifp, zpl_true);
	}
	if(nsm_stormcontrol_enable_get_api(ifp))
	{
		zpl_uint32 storm_unicast = 0;
		zpl_uint32 storm_multicast = 0;
		zpl_uint32 storm_broadcast = 0;
		
		if(strncmp(argv[0], "broadcast", 3) == 0)
		{
			storm_broadcast = atoi(argv[1]);
			ret = nsm_stormcontrol_broadcast_set_api(ifp,  storm_broadcast, NSM_STORMCONTROL_PERCENT);
		}
		else if(strncmp(argv[0], "multicast", 3) == 0)
		{
			storm_multicast = atoi(argv[1]);
			ret = nsm_stormcontrol_multicast_set_api(ifp,  storm_multicast, NSM_STORMCONTROL_PERCENT);
		}
		else if(strncmp(argv[0], "unicast", 3) == 0)
		{
			storm_unicast = atoi(argv[1]);
			ret = nsm_stormcontrol_unicast_set_api(ifp,  storm_unicast, NSM_STORMCONTROL_PERCENT);
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (stormcontrol_control_packet,
		stormcontrol_control_packet_cmd,
		"storm control (broadcast|multicast|unicast) pps <0-1000000000>" ,
		"Storm control\n"
		"Control\n"
		"broadcast\n"
		"multicast\n"
		"unicast\n"
		"Pakcet\n"
		"value of Pakcet\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_stormcontrol_enable_get_api(ifp))
	{
		ret = nsm_stormcontrol_enable_set_api(ifp, zpl_true);
	}
	if(nsm_stormcontrol_enable_get_api(ifp))
	{
		zpl_uint32 storm_unicast = 0;
		zpl_uint32 storm_multicast = 0;
		zpl_uint32 storm_broadcast = 0;
		
		if(strncmp(argv[0], "broadcast", 3) == 0)
		{
			storm_broadcast = atoi(argv[1]);
			ret = nsm_stormcontrol_broadcast_set_api(ifp,  storm_broadcast, NSM_STORMCONTROL_PACKET);
		}
		else if(strncmp(argv[0], "multicast", 3) == 0)
		{
			storm_multicast = atoi(argv[1]);
			ret = nsm_stormcontrol_multicast_set_api(ifp,  storm_multicast, NSM_STORMCONTROL_PACKET);
		}
		else if(strncmp(argv[0], "unicast", 3) == 0)
		{
			storm_unicast = atoi(argv[1]);
			ret = nsm_stormcontrol_unicast_set_api(ifp,  storm_unicast, NSM_STORMCONTROL_PACKET);
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_stormcontrol_control,
		no_stormcontrol_control_cmd,
		"no storm control (broadcast|multicast|unicast)" ,
		NO_STR
		"Storm control\n"
		"Control\n"
		"broadcast\n"
		"multicast\n"
		"unicast\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(nsm_stormcontrol_enable_get_api(ifp))
	{
		ret = nsm_stormcontrol_enable_set_api(ifp, zpl_true);
	}
	if(nsm_stormcontrol_enable_get_api(ifp))
	{
		zpl_uint32 storm_unicast = 0;
		zpl_uint32 storm_multicast = 0;
		zpl_uint32 storm_broadcast = 0;
		
		if(strncmp(argv[0], "broadcast", 3) == 0)
		{
			storm_broadcast = 0;
			ret = nsm_stormcontrol_broadcast_set_api(ifp,  storm_broadcast, NSM_STORMCONTROL_RATE);
		}
		else if(strncmp(argv[0], "multicast", 3) == 0)
		{
			storm_multicast = 0;
			ret = nsm_stormcontrol_multicast_set_api(ifp,  storm_multicast, NSM_STORMCONTROL_RATE);
		}
		else if(strncmp(argv[0], "unicast", 3) == 0)
		{
			storm_unicast = 0;
			ret = nsm_stormcontrol_unicast_set_api(ifp,  storm_unicast, NSM_STORMCONTROL_RATE);
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


int cmd_security_init(void)
{
	install_element(INTERFACE_NODE, CMD_CONFIG_LEVEL, &stormcontrol_control_cmd);
	install_element(INTERFACE_NODE, CMD_CONFIG_LEVEL, &stormcontrol_control_percent_cmd);
	install_element(INTERFACE_NODE, CMD_CONFIG_LEVEL, &stormcontrol_control_packet_cmd);
	install_element(INTERFACE_NODE, CMD_CONFIG_LEVEL, &no_stormcontrol_control_cmd);	
	return OK;
}