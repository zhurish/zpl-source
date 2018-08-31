/*
 * cmd_port.c
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
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



DEFUN (switchport_mode,
		switchport_mode_cmd,
		"switchport mode (access|trunk)",
		"Switchport interface\n"
		"Switchport mode\n"
		"access port\n"
		"trunk port\n")
{
	int ret = 0;
	int mode = 0,newmode = 0;
	struct interface *ifp = vty->index;
	if(ifp)
	{
		if(nsm_interface_mode_get_api(ifp, &mode) == OK)
		{
			if(os_memcmp(argv[0], "access", 3) == 0)
				newmode = IF_MODE_ACCESS_L2;
			else
				newmode = IF_MODE_TRUNK_L2;
			if(newmode !=mode)
				ret = nsm_interface_mode_set_api(ifp, newmode);
		}
		return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	return CMD_WARNING;
}


DEFUN (no_switchport_mode,
		no_switchport_mode_cmd,
		"no switchport mode ",
		NO_STR
		"Switchport interface\n"
		"Switchport mode\n")
{
	int ret = 0;
	int mode = 0,newmode = 0;
	struct interface *ifp = vty->index;
	if(ifp)
	{
		if(nsm_interface_mode_get_api(ifp, &mode) == OK)
		{
			newmode = IF_MODE_ACCESS_L2;
			if(newmode !=mode)
				ret = nsm_interface_mode_set_api(ifp, newmode);
		}
		return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	return CMD_WARNING;
}

int nsm_port_interface_config(struct vty *vty, struct interface *ifp)
{
	struct nsm_interface *nsm = ifp->info[ZLOG_NSM];
	if(nsm)
	{
		switch (nsm->duplex)
		{
		case NSM_IF_DUPLEX_NONE:
		case NSM_IF_DUPLEX_AUTO:
			break;
		case NSM_IF_DUPLEX_FULL:
			vty_out(vty, " duplex full%s", VTY_NEWLINE);
			break;
		case NSM_IF_DUPLEX_HALF:
			vty_out(vty, " duplex half%s", VTY_NEWLINE);
			break;
		default:
			break;
		}
		switch (nsm->speed)
		{
		//10|100|1000|10000|auto
		case NSM_IF_SPEED_NONE:
		case NSM_IF_SPEED_AUTO:
			break;
		case NSM_IF_SPEED_10M:
			vty_out(vty, " speed 10%s", VTY_NEWLINE);
			break;
		case NSM_IF_SPEED_100M:
			vty_out(vty, " speed 100%s", VTY_NEWLINE);
			break;
		case NSM_IF_SPEED_1000M:
			vty_out(vty, " speed 1000%s", VTY_NEWLINE);
			break;
		case NSM_IF_SPEED_10000M:
			vty_out(vty, " speed 10000%s", VTY_NEWLINE);
			break;
		case NSM_IF_SPEED_1000M_B:
			vty_out(vty, " speed 1000%s", VTY_NEWLINE);
			break;
		case NSM_IF_SPEED_1000M_MP:
			vty_out(vty, " speed 1000%s", VTY_NEWLINE);
			break;
		case NSM_IF_SPEED_1000M_MP_NO_FIBER:
			vty_out(vty, " speed 1000%s", VTY_NEWLINE);
			break;
		default:
			break;
		}
		switch (nsm->linkdetect)
		{
		case IF_LINKDETECT_ON:
			vty_out(vty, " link-detect%s", VTY_NEWLINE);
			break;
		case IF_LINKDETECT_OFF:
			vty_out(vty, " no link-detect%s", VTY_NEWLINE);
			break;
		default:
			break;
		}
		if (nsm->multicast != IF_ZEBRA_MULTICAST_UNSPEC)
			vty_out(vty, " %smulticast%s",
					nsm->multicast == IF_ZEBRA_MULTICAST_ON ?
							"" : "no ", VTY_NEWLINE);
	}
	return OK;
}


void cmd_port_init (void)
{
	struct nsm_client *nsm = nsm_client_new ();
	nsm->interface_add_cb = NULL;
	nsm->interface_delete_cb = NULL;
	nsm->interface_write_config_cb = nsm_port_interface_config;
	nsm_client_install (nsm, NSM_PORT);

	install_element(INTERFACE_NODE, &switchport_mode_cmd);
	install_element(INTERFACE_NODE, &no_switchport_mode_cmd);
}


