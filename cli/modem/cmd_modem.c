/*
 * cmd_modem.c
 *
 *  Created on: Jul 29, 2018
 *      Author: zhurish
 */



/*
 * cmd_routeid.c
 *
 *  Created on: Jan 2, 2018
 *      Author: zhurish
 */


#include "zebra.h"
#include "log.h"
#include "memory.h"
#include "str.h"
#include "os_util.h"
#include "tty_com.h"

#include "if.h"
#include "vty.h"
#include "command.h"
#include "memory.h"


#include "modem.h"
#include "modem_attty.h"
#include "modem_client.h"
#include "modem_message.h"
#include "modem_event.h"
#include "modem_machine.h"
#include "modem_dhcp.h"
#include "modem_pppd.h"
#include "modem_serial.h"
#include "modem_process.h"
#include "modem_product.h"


struct modem_vty_cb
{
	struct vty *vty;
	char name[128];
	BOOL detail;
};
/***********************************************************************/
/************************  modem-profile ******************************/
DEFUN (modem_profile,
		modem_profile_cmd,
       "modem-profile NAME",
	   "Modem profile\n"
       "Specify the profile name\n")
{
	modem_t *modem = NULL;
	modem = modem_main_lookup_api(argv[0]);
	if(modem)
	{
		vty->index = modem;
		vty->node = MODEM_PROFILE_NODE;
		return CMD_SUCCESS;
	}
	if(modem_main_add_api (argv[0]) == OK)
	{
		vty->index = modem_main_lookup_api(argv[0]);
		vty->node = MODEM_PROFILE_NODE;
		return CMD_SUCCESS;
	}
	vty_out(vty, "Can not create modem profile name%s",VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (no_modem_profile,
		no_modem_profile_cmd,
       "no modem-profile NAME",
	   NO_STR
	   "Modem profile\n"
       "Specify the profile name\n")
{
	modem_t *modem = NULL;
	modem = modem_main_lookup_api(argv[0]);
	if(!modem)
	{
		vty_out(vty, "Can not delete modem profile name%s",VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(modem_main_del_api (argv[0]) == OK)
	{
		return CMD_SUCCESS;
	}
	vty_out(vty, "Can not create modem profile name%s",VTY_NEWLINE);
	return CMD_WARNING;
}


DEFUN (modem_channel_bind,
		modem_channel_bind_cmd,
       "bind modem-channel NAME",
	   "Bind configure\n"
	   "Modem Channel\n"
       "Specify the profile name\n")
{
	modem_t *modem = vty->index;
	if(modem)
	{
		if(!modem_serial_lookup_api(argv[0]))
		{
			vty_out(vty, "Can not find modem channel name%s",VTY_NEWLINE);
			return CMD_WARNING;
		}
		if(modem_main_bind_api(modem->name, argv[0]) == OK)
			return CMD_SUCCESS;
	}
	vty_out(vty, "Can not bind modem channel name%s",VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (no_modem_channel_bind,
		no_modem_channel_bind_cmd,
       "no bind modem-channel",
	   NO_STR
	   "Bind configure\n"
	   "Modem Channel\n"
       "Specify the profile name\n")
{
	modem_t *modem = vty->index;
	if(modem)
	{
		if(modem_main_unbind_api(modem->name, NULL) == OK)
			return CMD_SUCCESS;
	}
	vty_out(vty, "Can not bind modem channel name%s",VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (modem_pin,
		modem_pin_cmd,
       "modem pin code NAME",
	   "Modem configure\n"
	   "PIN configure\n"
	   "PIN Code\n"
       "Specify the PIN Code\n")
{
	modem_t *modem = vty->index;
	if(modem)
	{
		if(modem_main_pin_set_api(modem, argv[0]) == OK)
			return CMD_SUCCESS;
	}
	vty_out(vty, "Can not set modem PIN Code%s",VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (no_modem_pin,
		no_modem_pin_cmd,
       "no modem pin code",
	   NO_STR
	   "Modem configure\n"
	   "PIN configure\n"
	   "PIN Code\n")
{
	modem_t *modem = vty->index;
	if(modem)
	{
		if(modem_main_pin_set_api(modem, NULL) == OK)
			return CMD_SUCCESS;
	}
	vty_out(vty, "Can not unset modem PIN Code%s",VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (modem_puk,
		modem_puk_cmd,
       "modem puk code NAME",
	   "Modem configure\n"
	   "PUK configure\n"
	   "PUK Code\n"
       "Specify the PUK Code\n")
{
	modem_t *modem = vty->index;
	if(modem)
	{
		if(modem_main_puk_set_api(modem, argv[0]) == OK)
			return CMD_SUCCESS;
	}
	vty_out(vty, "Can not set modem PUK Code%s",VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (no_modem_puk,
		no_modem_puk_cmd,
       "no modem puk code",
	   NO_STR
	   "Modem configure\n"
	   "PUK configure\n"
	   "PUK Code\n")
{
	modem_t *modem = vty->index;
	if(modem)
	{
		if(modem_main_puk_set_api(modem, NULL) == OK)
			return CMD_SUCCESS;
	}
	vty_out(vty, "Can not unset modem PUK Code%s",VTY_NEWLINE);
	return CMD_WARNING;
}


DEFUN (modem_apn,
		modem_apn_cmd,
       "modem apn-name NAME",
	   "Modem configure\n"
	   "APN configure\n"
       "Specify the APN name\n")
{
	modem_t *modem = vty->index;
	if(modem)
	{
		if(modem_main_apn_set_api(modem, argv[0]) == OK)
			return CMD_SUCCESS;
	}
	vty_out(vty, "Can not set modem APN Code%s",VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (no_modem_apn,
		no_modem_apn_cmd,
       "no modem apn-name",
	   NO_STR
	   "Modem configure\n"
	   "APN configure\n")
{
	modem_t *modem = vty->index;
	if(modem)
	{
		if(modem_main_apn_set_api(modem, NULL) == OK)
			return CMD_SUCCESS;
	}
	vty_out(vty, "Can not unset modem APN Code%s",VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (modem_svc_set,
		modem_svc_set_cmd,
       "modem svc-code NAME",
	   "Modem configure\n"
	   "Service Code configure\n"
       "Specify the Service Code name\n")
{
	modem_t *modem = vty->index;
	if(modem)
	{
		if(modem_main_svc_set_api(modem, argv[0]) == OK)
			return CMD_SUCCESS;
	}
	vty_out(vty, "Can not set modem SVC Code%s",VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (no_modem_svc_set,
		no_modem_svc_set_cmd,
       "no modem svc-code",
	   NO_STR
	   "Modem configure\n"
	   "Service Code configure\n")
{
	modem_t *modem = vty->index;
	if(modem)
	{
		if(modem_main_svc_set_api(modem, NULL) == OK)
			return CMD_SUCCESS;
	}
	vty_out(vty, "Can not unset modem SVC Code%s",VTY_NEWLINE);
	return CMD_WARNING;
}


DEFUN (modem_ipstack_set,
		modem_ipstack_set_cmd,
       "modem ipstack (v4|v6|both)",
	   "Modem configure\n"
	   "IP Stack configure\n"
       "IPV4 enable\n"
	   "IPV6 enable\n"
	   "IPV4 and IPV6 enable\n")
{
	modem_t *modem = vty->index;
	if(modem)
	{
		modem_stack_type oldtype = MODEM_IPV4, type = MODEM_IPV4;
		if(modem_main_ip_get_api(modem, &oldtype) != OK)
		{
			vty_out(vty, "Can not get modem ipstack%s",VTY_NEWLINE);
			return CMD_WARNING;
		}
		if(memcmp(argv[0], "v4", 2) == 0)
			type = MODEM_IPV4;
		else if(memcmp(argv[0], "v6", 2) == 0)
			type = MODEM_IPV6;
		else if(memcmp(argv[0], "both", 2) == 0)
			type = MODEM_BOTH;
		if(oldtype == type)
			return CMD_SUCCESS;
		if(modem_main_ip_set_api(modem, type) == OK)
			return CMD_SUCCESS;
	}
	vty_out(vty, "Can not set modem ipstack%s",VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (no_modem_ipstack_set,
		no_modem_ipstack_set_cmd,
       "modem ipstack",
	   "Modem configure\n"
	   "IP Stack configure\n")
{
	modem_t *modem = vty->index;
	if(modem)
	{
		modem_stack_type oldtype = MODEM_IPV4;
		if(modem_main_ip_get_api(modem, &oldtype) != OK)
		{
			vty_out(vty, "Can not get modem ipstack%s",VTY_NEWLINE);
			return CMD_WARNING;
		}

		if(oldtype == MODEM_IPV4)
			return CMD_SUCCESS;
		if(modem_main_ip_set_api(modem, MODEM_IPV4) == OK)
			return CMD_SUCCESS;
	}
	vty_out(vty, "Can not unset modem ipstack%s",VTY_NEWLINE);
	return CMD_WARNING;
}



DEFUN (modem_secondary_set,
		modem_secondary_set_cmd,
       "modem apn secondary",
	   "Modem configure\n"
	   "Double APN configure\n"
       "Secondary APN for modem\n")
{
	modem_t *modem = vty->index;
	if(modem)
	{
		BOOL oldtype = FALSE, type = TRUE;
		if(modem_main_secondary_get_api(modem, &oldtype) != OK)
		{
			vty_out(vty, "Can not get modem Secondary APN%s",VTY_NEWLINE);
			return CMD_WARNING;
		}
		if(oldtype == type)
			return CMD_SUCCESS;
		if(modem_main_secondary_set_api(modem, type) == OK)
			return CMD_SUCCESS;
	}
	vty_out(vty, "Can not set modem Secondary APN%s",VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (no_modem_secondary_set,
		no_modem_secondary_set_cmd,
	    "no modem apn secondary",
		NO_STR
		"Modem configure\n"
		"Double APN configure\n"
	    "Secondary APN for modem\n")
{
	modem_t *modem = vty->index;
	if(modem)
	{
		BOOL oldtype = FALSE, type = FALSE;
		if(modem_main_secondary_get_api(modem, &oldtype) != OK)
		{
			vty_out(vty, "Can not get modem Secondary APN%s",VTY_NEWLINE);
			return CMD_WARNING;
		}
		if(oldtype == type)
			return CMD_SUCCESS;
		if(modem_main_secondary_set_api(modem, type) == OK)
			return CMD_SUCCESS;
	}
	vty_out(vty, "Can not set modem Secondary APN%s",VTY_NEWLINE);
	return CMD_WARNING;
}



DEFUN (modem_profile_set,
		modem_profile_set_cmd,
       "modem profile-id <1-16>",
	   "Modem configure\n"
	   "profile-id configure\n"
       "ID value of profile\n")
{
	modem_t *modem = vty->index;
	if(modem)
	{
		int profile = 0;
		if(modem_main_profile_get_api(modem, &profile) != OK)
		{
			vty_out(vty, "Can not get modem profile-id%s",VTY_NEWLINE);
			return CMD_WARNING;
		}
		if(profile == atoi(argv[0]))
			return CMD_SUCCESS;
		profile = atoi(argv[0]);
		if(modem_main_profile_set_api(modem, profile) == OK)
			return CMD_SUCCESS;
	}
	vty_out(vty, "Can not set modem profile-id%s",VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (no_modem_profile_set,
		no_modem_profile_set_cmd,
       "no modem profile-id",
	   NO_STR
	   "Modem configure\n"
	   "profile-id configure\n")
{
	modem_t *modem = vty->index;
	if(modem)
	{
		int profile = 0;
		if(modem_main_profile_get_api(modem, &profile) != OK)
		{
			vty_out(vty, "Can not get modem profile-id%s",VTY_NEWLINE);
			return CMD_WARNING;
		}
		if(profile == 1)
			return CMD_SUCCESS;
		profile = 1;
		if(modem_main_profile_set_api(modem, profile) == OK)
			return CMD_SUCCESS;
	}
	vty_out(vty, "Can not set modem profile-id%s",VTY_NEWLINE);
	return CMD_WARNING;
}


DEFUN (modem_dial_set,
		modem_dial_set_cmd,
       "modem dial-mode (none|dhcp|ppp|qmi|gobinet)",
	   "Modem configure\n"
	   "Dial modem configure\n"
       "NONE mode\n"
	   "DHCP mode\n"
	   "PPP mode\n"
	   "QMI mode\n"
	   "GOBINET mode\n")
{
	modem_t *modem = vty->index;
	if(modem)
	{
		modem_dial_type type = MODEM_DIAL_NONE, oldtype = MODEM_DIAL_NONE;
		if(modem_main_dial_get_api(modem, &oldtype) != OK)
		{
			vty_out(vty, "Can not get modem dial mode%s",VTY_NEWLINE);
			return CMD_WARNING;
		}

		if(strncmp(argv[0], "none", 3) == 0)
			type = MODEM_DIAL_NONE;
		else if(strncmp(argv[0], "dhcp", 3) == 0)
			type = MODEM_DIAL_DHCP;
		else if(strncmp(argv[0], "ppp", 3) == 0)
			type = MODEM_DIAL_PPP;
		else if(strncmp(argv[0], "qmi", 3) == 0)
			type = MODEM_DIAL_QMI;
		else if(strncmp(argv[0], "gobinet", 3) == 0)
			type = MODEM_DIAL_GOBINET;

		if(type == oldtype)
			return CMD_SUCCESS;

		if(modem_main_dial_set_api(modem, type) == OK)
			return CMD_SUCCESS;
	}
	vty_out(vty, "Can not set modem dial mode%s",VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (no_modem_dial_set,
		no_modem_dial_set_cmd,
       "no modem dial-mode",
	   "Modem configure\n"
	   "Dial modem configure\n")
{
	modem_t *modem = vty->index;
	if(modem)
	{
		modem_dial_type type = MODEM_DIAL_NONE, oldtype = MODEM_DIAL_NONE;
		if(modem_main_dial_get_api(modem, &oldtype) != OK)
		{
			vty_out(vty, "Can not get modem dial mode%s",VTY_NEWLINE);
			return CMD_WARNING;
		}
		if(type == oldtype)
			return CMD_SUCCESS;

		if(modem_main_dial_set_api(modem, type) == OK)
			return CMD_SUCCESS;
	}
	vty_out(vty, "Can not set modem dial mode%s",VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (modem_network_set,
		modem_network_set_cmd,
       "modem network-type (auto|gsm|cdma1x|gprs|hscsd|wap|edge|td-scdma|wcdma|evdo|td-lte|fdd-lte)",
	   "Modem configure\n"
	   "network-type configure\n"
       "NONE mode\n"
	   "DHCP mode\n"
	   "PPP mode\n"
	   "QMI mode\n"
	   "GOBINET mode\n")
{
	modem_t *modem = vty->index;
	if(modem)
	{
		modem_network_type type = NETWORK_AUTO, oldtype = NETWORK_AUTO;
		if(modem_main_network_get_api(modem, &oldtype) != OK)
		{
			vty_out(vty, "Can not get modem network type%s",VTY_NEWLINE);
			return CMD_WARNING;
		}

		type = modem_network_type_get(argv[0]);

		if(type == oldtype)
			return CMD_SUCCESS;

		if(modem_main_network_set_api(modem, type) == OK)
			return CMD_SUCCESS;
	}
	vty_out(vty, "Can not set modem network type%s",VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (no_modem_network_set,
		no_modem_network_set_cmd,
       "no modem network-type",
	   "Modem configure\n"
	   "network-type configure\n")
{
	modem_t *modem = vty->index;
	if(modem)
	{
		modem_network_type type = NETWORK_AUTO, oldtype = NETWORK_AUTO;
		if(modem_main_network_get_api(modem, &oldtype) != OK)
		{
			vty_out(vty, "Can not get modem network type%s",VTY_NEWLINE);
			return CMD_WARNING;
		}
		if(type == oldtype)
			return CMD_SUCCESS;

		if(modem_main_network_set_api(modem, type) == OK)
			return CMD_SUCCESS;
	}
	vty_out(vty, "Can not set modem network type%s",VTY_NEWLINE);
	return CMD_WARNING;
}


DEFUN (modem_bind_interface_set,
		modem_bind_interface_set_cmd,
       "bind interface (serial|ethernet) " CMD_USP_STR,
	   "bind configure\n"
       "Interface configure\n"
	   "Serial interface\n"
	   "Ethernet interface\n"
	   CMD_USP_STR_HELP)
{
	modem_t *modem = vty->index;
	struct interface *ifp = NULL;
	if(os_strstr(argv[0], "serial"))
	{
		if(modem->ppp_serial)
		{
			struct interface *ifp1 = modem->ppp_serial;
			vty_out(vty, "This modem is already bind to interface %s%s", ifp1->name, VTY_NEWLINE);
			return CMD_WARNING;
		}

		ifp = if_lookup_by_name(if_ifname_format(argv[0], argv[1]));

		if(ifp && ifp->ll_type != ZEBRA_LLT_MODEM)
		{
			vty_out(vty, "Can not bind this interface %s %s%s",argv[0], argv[1], VTY_NEWLINE);
			return CMD_WARNING;
		}
		if(!ifp)
		{
			if(modem_interface_add_api(if_ifname_format(argv[0], argv[1])) != OK)
			{
				vty_out(vty, "Can not bind to interface %s %s%s",argv[0], argv[1], VTY_NEWLINE);
				return CMD_WARNING;
			}
		}
		ifp = if_lookup_by_name(if_ifname_format(argv[0], argv[1]));

		if(ifp && ifp->ll_type == ZEBRA_LLT_MODEM)
		{
			modem->ppp_serial = ifp;
			return CMD_SUCCESS;
		}
	}
	else if(os_strstr(argv[0], "ethernet"))
	{
		if(modem->eth0)
		{
			struct interface *ifp1 = modem->eth0;
			vty_out(vty, "This modem is already bind to interface %s%s", ifp1->name, VTY_NEWLINE);
			return CMD_WARNING;
		}

		ifp = if_lookup_by_name(if_ifname_format(argv[0], argv[1]));

		if(ifp && ifp->ll_type != ZEBRA_LLT_MODEM)
		{
			vty_out(vty, "Can not bind this interface %s %s%s",argv[0], argv[1], VTY_NEWLINE);
			return CMD_WARNING;
		}
		if(!ifp)
		{
			if(modem_interface_add_api(if_ifname_format(argv[0], argv[1])) != OK)
			{
				vty_out(vty, "Can not bind to interface %s %s%s",argv[0], argv[1], VTY_NEWLINE);
				return CMD_WARNING;
			}
		}
		ifp = if_lookup_by_name(if_ifname_format(argv[0], argv[1]));

		if(ifp && ifp->ll_type == ZEBRA_LLT_MODEM)
		{
			modem->eth0 = ifp;
			return CMD_SUCCESS;
		}
	}
	return CMD_WARNING;
}

DEFUN (modem_bind_interface_vals_set,
		modem_bind_interface_vals_set_cmd,
       "bind interface serial " CMD_USP_STR " (ppp|dial)",
	   "bind configure\n"
       "Interface configure\n"
	   "Serial interface\n"
	   CMD_USP_STR_HELP
		"ppp interface\n"
		"dial interface\n")
{
	modem_t *modem = vty->index;
	struct interface *ifp = NULL;
	if(os_strstr(argv[1], "ppp"))
	{
		if(modem->ppp_serial)
		{
			struct interface *ifp1 = modem->ppp_serial;
			vty_out(vty, "This modem is already bind to modem%s", ifp1->name, VTY_NEWLINE);
			return CMD_WARNING;
		}

		ifp = if_lookup_by_name(if_ifname_format("serial", argv[0]));

		if(ifp && ifp->ll_type != ZEBRA_LLT_MODEM)
		{
			vty_out(vty, "Can not bind this interface %s %s%s", "serial", argv[0], VTY_NEWLINE);
			return CMD_WARNING;
		}
		if(!ifp)
		{
			if(modem_interface_add_api(if_ifname_format("serial", argv[0])) != OK)
			{
				vty_out(vty, "Can not bind to interface %s %s%s","serial", argv[0], VTY_NEWLINE);
				return CMD_WARNING;
			}
		}
		ifp = if_lookup_by_name(if_ifname_format("serial", argv[0]));

		if(ifp && ifp->ll_type == ZEBRA_LLT_MODEM)
		{
			modem->ppp_serial = ifp;
			return CMD_SUCCESS;
		}
	}
	else if(os_strstr(argv[1], "dial"))
	{
		if(modem->dial_serial)
		{
			struct interface *ifp1 = modem->dial_serial;
			vty_out(vty, "This modem is already bind to modem%s", ifp1->name, VTY_NEWLINE);
			return CMD_WARNING;
		}

		ifp = if_lookup_by_name(if_ifname_format("serial", argv[0]));

		if(ifp && ifp->ll_type != ZEBRA_LLT_MODEM)
		{
			vty_out(vty, "Can not bind this interface %s %s%s","serial", argv[0], VTY_NEWLINE);
			return CMD_WARNING;
		}
		if(!ifp)
		{
			if(modem_interface_add_api(if_ifname_format("serial", argv[0])) != OK)
			{
				vty_out(vty, "Can not bind to interface %s %s%s","serial", argv[0], VTY_NEWLINE);
				return CMD_WARNING;
			}
		}
		ifp = if_lookup_by_name(if_ifname_format("serial", argv[0]));

		if(ifp && ifp->ll_type == ZEBRA_LLT_MODEM)
		{
			modem->dial_serial = ifp;
			return CMD_SUCCESS;
		}
	}
	return CMD_WARNING;
}

DEFUN (modem_bind_interface_vale_set,
		modem_bind_interface_vale_set_cmd,
       "bind interface ethernet " CMD_USP_STR " (secondary|third)",
	   "bind configure\n"
       "Interface configure\n"
	   "Ethernet interface\n"
	   CMD_USP_STR_HELP
	   "secondary interface\n"
	   "third interface\n")
{
	modem_t *modem = vty->index;
	struct interface *ifp = NULL;
	if(os_strstr(argv[1], "secondly"))
	{
		if(modem->eth1)
		{
			struct interface *ifp1 = modem->eth1;
			vty_out(vty, "This modem is already bind to modem%s", ifp1->name, VTY_NEWLINE);
			return CMD_WARNING;
		}

		ifp = if_lookup_by_name(if_ifname_format("ethernet", argv[0]));

		if(ifp && ifp->ll_type != ZEBRA_LLT_MODEM)
		{
			vty_out(vty, "Can not bind this interface %s %s%s", "ethernet", argv[0], VTY_NEWLINE);
			return CMD_WARNING;
		}
		if(!ifp)
		{
			if(modem_interface_add_api(if_ifname_format("ethernet", argv[0])) != OK)
			{
				vty_out(vty, "Can not bind to interface %s %s%s","ethernet", argv[0], VTY_NEWLINE);
				return CMD_WARNING;
			}
		}
		ifp = if_lookup_by_name(if_ifname_format("ethernet", argv[0]));

		if(ifp && ifp->ll_type == ZEBRA_LLT_MODEM)
		{
			modem->eth1 = ifp;
			return CMD_SUCCESS;
		}
	}
	else if(os_strstr(argv[1], "third"))
	{
		if(modem->eth2)
		{
			struct interface *ifp1 = modem->eth2;
			vty_out(vty, "This modem is already bind to modem%s", ifp1->name, VTY_NEWLINE);
			return CMD_WARNING;
		}

		ifp = if_lookup_by_name(if_ifname_format("ethernet", argv[0]));

		if(ifp && ifp->ll_type != ZEBRA_LLT_MODEM)
		{
			vty_out(vty, "Can not bind this interface %s %s%s","ethernet", argv[0], VTY_NEWLINE);
			return CMD_WARNING;
		}
		if(!ifp)
		{
			if(modem_interface_add_api(if_ifname_format("ethernet", argv[0])) != OK)
			{
				vty_out(vty, "Can not bind to interface %s %s%s","ethernet", argv[0], VTY_NEWLINE);
				return CMD_WARNING;
			}
		}
		ifp = if_lookup_by_name(if_ifname_format("ethernet", argv[0]));

		if(ifp && ifp->ll_type == ZEBRA_LLT_MODEM)
		{
			modem->eth2 = ifp;
			return CMD_SUCCESS;
		}
	}
	return CMD_WARNING;
}

DEFUN (no_modem_bind_interface_set,
		no_modem_bind_interface_set_cmd,
		"no bind interface (serial|ethernet)",
		NO_STR
		"bind configure\n"
		"Interface configure\n"
		"Serial interface\n"
		"Ethernet interface\n")
{
	modem_t *modem = vty->index;
	if(modem->eth0 == NULL)
	{
		return CMD_SUCCESS;
	}
	if(os_strstr(argv[0], "serial"))
		modem->ppp_serial = NULL;
	else if(os_strstr(argv[0], "ethernet"))
		modem->eth0 = NULL;
	return CMD_SUCCESS;
}

DEFUN (no_modem_bind_interface_vals_set,
		no_modem_bind_interface_vals_set_cmd,
		"no bind interface serial (ppp|dial)",
		NO_STR
		"bind configure\n"
		"Interface configure\n"
		"Serial interface\n"
		"ppp interface\n"
		"dial interface\n")
{
	modem_t *modem = vty->index;
	if(os_strstr(argv[0], "ppp"))
	{
		if(modem->ppp_serial == NULL)
		{
			return CMD_SUCCESS;
		}
		modem->ppp_serial = NULL;
	}
	if(os_strstr(argv[0], "dial"))
	{
		if(modem->dial_serial == NULL)
		{
			return CMD_SUCCESS;
		}
		modem->dial_serial = NULL;
	}
	return CMD_SUCCESS;
}

DEFUN (no_modem_bind_interface_vale_set,
		no_modem_bind_interface_vale_set_cmd,
		"no bind interface ethernet (secondary|third)",
		NO_STR
		"bind configure\n"
		"Interface configure\n"
		"Ethernet interface\n"
		"secondary interface\n"
		"third interface\n")
{
	modem_t *modem = vty->index;
	if(os_strstr(argv[0], "secondary"))
	{
		if(modem->eth1 == NULL)
		{
			return CMD_SUCCESS;
		}
		modem->eth1 = NULL;
	}
	if(os_strstr(argv[0], "third"))
	{
		if(modem->eth2 == NULL)
		{
			return CMD_SUCCESS;
		}
		modem->eth2 = NULL;
	}
	return CMD_SUCCESS;
}


static int modem_profile_write_cb(modem_t *modem, struct vty *vty)
{
	if(modem)
	{
		if(os_strlen(modem->name))
		{
			modem_serial_t *serial = modem->serial;

			vty_out(vty, "modem-profile %s%s", modem->name, VTY_NEWLINE);

			if(modem->dialtype != MODEM_DIAL_NONE)
				vty_out(vty, " modem dial-mode %s%s", modem_dial_string(modem->dialtype), VTY_NEWLINE);

			switch(modem->ipstack)
			{
			case MODEM_IPV4:
				vty_out(vty, " modem ipstack %s%s", "v4", VTY_NEWLINE);
				break;
			case MODEM_IPV6:
				vty_out(vty, " modem ipstack %s%s", "v6", VTY_NEWLINE);
				break;
			case MODEM_BOTH:
				vty_out(vty, " modem ipstack %s%s", "both", VTY_NEWLINE);
				break;
			}

			if(os_strlen(modem->apn))
				vty_out(vty, " modem apn-name %s%s", modem->apn, VTY_NEWLINE);

			if(os_strlen(modem->svc))
				vty_out(vty, " modem svc-code %s%s", modem->svc, VTY_NEWLINE);

			if(os_strlen(modem->pin))
				vty_out(vty, " modem pin code %s%s", modem->pin, VTY_NEWLINE);

			if(os_strlen(modem->puk))
				vty_out(vty, " modem puk code %s%s", modem->puk, VTY_NEWLINE);

			if(modem->profile != 0)
				vty_out(vty, " modem profile-id %d%s", modem->profile, VTY_NEWLINE);

			if(modem->network != NETWORK_AUTO)
				vty_out(vty, " modem network-type %s%s", modem_network_type_string(modem->network), VTY_NEWLINE);

			if(modem->bSecondary)
				vty_out(vty, " modem apn secondary%s", VTY_NEWLINE);

			if(modem->eth0)
			{
				struct interface *ifp = modem->eth0;
				vty_out(vty, " bind interface %s%s", ifp->name, VTY_NEWLINE);
			}
			if(modem->eth1)
			{
				struct interface *ifp = modem->eth1;
				vty_out(vty, " bind interface %s secondary%s", ifp->name, VTY_NEWLINE);
			}
			if(modem->eth2)
			{
				struct interface *ifp = modem->eth2;
				vty_out(vty, " bind interface %s third%s", ifp->name, VTY_NEWLINE);
			}
			if(modem->ppp_serial)
			{
				struct interface *ifp = modem->ppp_serial;
				vty_out(vty, " bind interface %s%s", ifp->name, VTY_NEWLINE);
			}
			if(modem->dial_serial)
			{
				struct interface *ifp = modem->dial_serial;
				vty_out(vty, " bind interface %s dial%s", ifp->name, VTY_NEWLINE);
			}

			if(serial && os_strlen(serial->name))
				vty_out(vty, " bind modem-channel %s%s", serial->name, VTY_NEWLINE);

			vty_out(vty, "!%s", VTY_NEWLINE);
		}
	}
	return CMD_SUCCESS;
}

static int modem_write_config(struct vty *vty)
{
	modem_main_callback_api(modem_profile_write_cb, vty);
	return CMD_SUCCESS;
}



static int show_modem_information_one_cb(modem_t *modem, struct vty *vty)
{
	if(modem)
	{
		modem_serial_t *serial = modem->serial;
		//struct interface *ifp = modem->ifp;
		if(os_strlen(modem->name))
		{
			vty_out(vty, "Modem Profile                : %s%s", modem->name, VTY_NEWLINE);
			vty_out(vty, "  Active                     : %s%s", modem->active? "ACTIVE":"INACTIVE", VTY_NEWLINE);
			vty_out(vty, "  Dial mode                  : %s%s", modem_dial_string(modem->dialtype), VTY_NEWLINE);
			switch(modem->ipstack)
			{
			case MODEM_IPV4:
				vty_out(vty, "  IP Stack                   : %s%s", "v4", VTY_NEWLINE);
				break;
			case MODEM_IPV6:
				vty_out(vty, "  IP Stack                   : %s%s", "v6", VTY_NEWLINE);
				break;
			case MODEM_BOTH:
				vty_out(vty, "  IP Stack                   : %s%s", "v4v6", VTY_NEWLINE);
				break;
			}
			vty_out(vty, "  APN                        : %s%s", os_strlen(modem->apn) ? modem->apn:" ", VTY_NEWLINE);
			vty_out(vty, "  PIN                        : %s%s", os_strlen(modem->pin) ? modem->pin:" ", VTY_NEWLINE);
			vty_out(vty, "  PUK                        : %s%s", os_strlen(modem->puk) ? modem->puk:" ", VTY_NEWLINE);
			vty_out(vty, "  Service Code               : %s%s", os_strlen(modem->svc) ? modem->svc:" ", VTY_NEWLINE);
			vty_out(vty, "  Network Type               : %s%s", modem_network_type_string(modem->network), VTY_NEWLINE);
			vty_out(vty, "  Profile ID                 : %d%s", modem->profile, VTY_NEWLINE);
			vty_out(vty, "  Secondary APN              : %s%s", modem->bSecondary? "secondary":" ", VTY_NEWLINE);

	/*		if(ifp)
			{
				vty_out(vty, " Bind Interface              : %s%s", ifp->name, VTY_NEWLINE);
			}*/
			if(modem->eth0)
			{
				struct interface *ifp = modem->eth0;
				vty_out(vty, "  Bind interface             :%s%s", ifp->name, VTY_NEWLINE);
			}
			if(modem->eth1)
			{
				struct interface *ifp = modem->eth1;
				vty_out(vty, "  Bind interface             :%s secondary%s", ifp->name, VTY_NEWLINE);
			}
			if(modem->eth2)
			{
				struct interface *ifp = modem->eth2;
				vty_out(vty, "  Bind interface             :%s third%s", ifp->name, VTY_NEWLINE);
			}
			if(modem->ppp_serial)
			{
				struct interface *ifp = modem->ppp_serial;
				vty_out(vty, "  Bind interface             :%s%s", ifp->name, VTY_NEWLINE);
			}
			if(modem->dial_serial)
			{
				struct interface *ifp = modem->dial_serial;
				vty_out(vty, "  Bind interface             :%s dial%s", ifp->name, VTY_NEWLINE);
			}
			vty_out(vty, "  state                      : %d%s", modem->state, VTY_NEWLINE);
			vty_out(vty, "  newstate                   : %d%s", modem->newstate, VTY_NEWLINE);

			vty_out(vty, "  detection delay            : %d%s", modem->dedelay, VTY_NEWLINE);
			vty_out(vty, "  check delay                : %d%s", modem->delay, VTY_NEWLINE);


			vty_out(vty, "  state                      : %d%s", modem->state, VTY_NEWLINE);
			if(serial)
			{
				vty_out(vty, "   Modem Channel             : %s%s", serial->name, VTY_NEWLINE);
			}
			if(modem->client)
			{
				modem_client_t *client = modem->client;
				vty_out(vty, "     bus/device              : %x:%x%s", client->bus,client->device, VTY_NEWLINE);
				vty_out(vty, "     vender/product          : %x:%x%s",
						client->vendor,client->product, VTY_NEWLINE);

				vty_out(vty, "     factory/product         : %s/%s%s",
						client->factory_name,client->product_name,VTY_NEWLINE);
				vty_out(vty, "     ID/version              : %s/%s%s",
						client->product_iden,client->version,VTY_NEWLINE);

				vty_out(vty, "     AT ECHO                 : %s%s", client->echo ? "on":"off",VTY_NEWLINE);

				vty_out(vty, "     IMEI                    : %s%s", client->IMEI_number,VTY_NEWLINE);
				vty_out(vty, "     CCID                    : %s%s", client->CCID_number,VTY_NEWLINE);
				vty_out(vty, "     IMSI                    : %s%s", client->IMSI_number,VTY_NEWLINE);
				//vty_out(vty, "     IMEI                    : %s%s", client->IMEI_number,VTY_NEWLINE);

				vty_out(vty, "     LAC                     : %s%s", client->LAC,VTY_NEWLINE);
				vty_out(vty, "     CI                      : %s%s", client->CI,VTY_NEWLINE);
				vty_out(vty, "     ACT                     : %s%s", client->nw_act,VTY_NEWLINE);
				vty_out(vty, "     BAND                    : %s%s", client->nw_band, VTY_NEWLINE);
				vty_out(vty, "     Channel                 : %d%s", client->nw_channel,VTY_NEWLINE);
				vty_out(vty, "     Signal                  : %d%s", client->signal,VTY_NEWLINE);
				vty_out(vty, "     Biterr                  : %d%s", client->bit_error,VTY_NEWLINE);
				//modem_activity_en	activity;
				if(client->attty)
					vty_out(vty, "      AT device              : %s%s", client->attty->devname,VTY_NEWLINE);
				if(client->pppd)
					vty_out(vty, "      PPPD device            : %s%s", client->pppd->devname,VTY_NEWLINE);
				if(client->driver && client->driver->dialog)
					vty_out(vty, "       Dialog                : %s%s", client->driver->dialog->devname,VTY_NEWLINE);
				if(client->driver && client->driver->attty)
					vty_out(vty, "       AT device             : %s%s", client->driver->attty->devname,VTY_NEWLINE);
				if(client->driver && client->driver->pppd)
					vty_out(vty, "       PPPD device           : %s%s", client->driver->pppd->devname,VTY_NEWLINE);
				if(client->driver && client->driver->usetty)
					vty_out(vty, "       Use device            : %s%s", client->driver->usetty->devname,VTY_NEWLINE);
			}
		}
	}
	return CMD_SUCCESS;
}




static int show_modem_information_cb(modem_t *modem, struct modem_vty_cb *user)
{
	if(modem && user && user->vty)
	{
		if( os_strlen(modem->name) &&
			os_strlen(user->name) &&
			(strcmp(modem->name, user->name) == 0) )
		{
			show_modem_information_one_cb(modem, user->vty);
		}
		else
			show_modem_information_one_cb(modem, user->vty);
	}
	return CMD_SUCCESS;
}



DEFUN (show_modem_profile,
		show_modem_profile_cmd,
       "show modem-profile",
	   SHOW_STR
	   "Modem profile\n")
{
	struct modem_vty_cb user;
	os_memset(&user, 0, sizeof(user));
	user.vty = vty;
	if(argc == 1 && argv[0])
	{
		strcpy(user.name, argv[0]);
	}
	modem_main_callback_api(show_modem_information_cb, &user);
	return CMD_SUCCESS;
}

ALIAS (show_modem_profile,
		show_modem_profile_name_cmd,
       "show modem-profile NAME",
	   SHOW_STR
	   "Modem profile\n"
       "Specify the profile name\n");


static int show_modem_machine_state_cb(modem_t *modem, struct modem_vty_cb *user)
{
	if(modem && user && user->vty)
	{
		if( os_strlen(modem->name) &&
			os_strlen(user->name) &&
			(strcmp(modem->name, user->name) == 0) )
		{
			modem_machine_state_show(modem, user->vty, user->detail);
		}
		else
			modem_machine_state_show(modem, user->vty, user->detail);
	}
	return CMD_SUCCESS;
}

DEFUN (show_modem_machine_state,
	show_modem_machine_state_cmd,
    "show modem-machine-state",
	SHOW_STR
	"Modem machine state\n")
{
	struct modem_vty_cb user;
	os_memset(&user, 0, sizeof(user));
	user.vty = vty;
	if(argc == 1 && argv[0])
	{
		user.detail = TRUE;
	}
	modem_main_callback_api(show_modem_machine_state_cb, &user);
	return CMD_SUCCESS;
}

ALIAS (show_modem_machine_state,
	show_modem_machine_state_detail_cmd,
	"show modem-machine-state (detail|)",
	SHOW_STR
	"Modem machine state\n"
    "Detail information\n");

/*static int modem_profile_write_cb(modem_t *modem, struct vty *vty)
{
	if(modem)
	{
		modem_serial_t *serial = modem->serial;
		if(os_strlen(modem->name))
		{
			vty_out(vty, "modem-profile %s%s", modem->name, VTY_NEWLINE);
			if(serial && os_strlen(serial->name))
				vty_out(vty, " bind modem-channel %s%s", modem->serialname, VTY_NEWLINE);

			vty_out(vty, "exit%s",VTY_NEWLINE);
		}
	}
	return CMD_SUCCESS;
}*/

/***********************************************************************/
/************************  modem-profile ******************************/
DEFUN (modem_channel,
		modem_channel_cmd,
       "modem-channel NAME",
	   "Modem Channel\n"
       "Specify the channel name\n")
{
	modem_serial_t *modem = NULL;
	modem = modem_serial_lookup_api(argv[0]);
	if(modem)
	{
		vty->index = modem;
		vty->node = MODEM_CHANNEL_NODE;
		return CMD_SUCCESS;
	}
	if(modem_serial_add_api (argv[0]) == OK)
	{
		vty->index = modem_serial_lookup_api(argv[0]);
		vty->node = MODEM_CHANNEL_NODE;
		return CMD_SUCCESS;
	}
	vty_out(vty, "Can not create modem profile name%s",VTY_NEWLINE);
	return CMD_WARNING;
}

DEFUN (no_modem_channel,
		no_modem_channel_cmd,
       "no modem-channel NAME",
	   NO_STR
	   "Modem Channel\n"
       "Specify the channel name\n")
{
	modem_serial_t *modem = NULL;
	modem = modem_serial_lookup_api(argv[0]);
	if(!modem)
	{
		vty_out(vty, "Can not delete modem profile name%s",VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(modem_serial_del_api (argv[0]) == OK)
	{
		return CMD_SUCCESS;
	}
	vty_out(vty, "Can not create modem profile name%s",VTY_NEWLINE);
	return CMD_WARNING;
}

static int modem_channel_write_cb(modem_serial_t *serial, struct vty *vty)
{
	if(serial)
	{
		if(os_strlen(serial->name))
		{
			vty_out(vty, "modem-channel %s%s", serial->name, VTY_NEWLINE);

			vty_out(vty, "!%s",VTY_NEWLINE);
		}
	}
	return CMD_SUCCESS;
}


static int modem_serial_write_cb(struct vty *vty)
{
	//modem_main_callback_api(modem_profile_write_cb, vty);
	modem_serial_callback_api(modem_channel_write_cb, vty);
	return 1;
}


DEFUN (show_modem_usb_driver_param,
		show_modem_usb_driver_param_cmd,
       "show modem-usb-driver",
	   SHOW_STR
	   "Modem usb driver\n"
       "Specify the profile name\n")
{
	show_modem_usb_driver(vty);
	return CMD_SUCCESS;
}


#ifdef __MODEM_DEBUG

extern int modem_start;
DEFUN (modem_start_test,
		modem_start_test_cmd,
       "modem-start-test",
	   "Modem-start-test\n")
{
	modem_start = 1;
	return CMD_SUCCESS;
}

DEFUN (modem_test,
		modem_test_cmd,
       "modem-test",
       "syslog-debug\n"
	   "dest")
{
	extern int modem_cmd_test(struct vty *vty);
	modem_cmd_test(vty);
	return CMD_SUCCESS;
}

DEFUN (modem_close,
		modem_close_cmd,
       "modem-close",
       "syslog-debug\n"
	   "dest")
{
	extern int modem_cmd_test_close();
	modem_cmd_test_close();
	return CMD_SUCCESS;
}
DEFUN (modem_open,
		modem_open_cmd,
       "modem-open",
       "syslog-debug\n"
	   "dest")
{
	extern int modem_cmd_test_open();
	modem_cmd_test_open();
	return CMD_SUCCESS;
}
#endif

static void cmd_modem_set_init (int node)
{
	install_element(node, &modem_channel_bind_cmd);
	install_element(node, &no_modem_channel_bind_cmd);

	install_element(node, &modem_pin_cmd);
	install_element(node, &no_modem_pin_cmd);

	install_element(node, &modem_puk_cmd);
	install_element(node, &no_modem_puk_cmd);

	install_element(node, &modem_apn_cmd);
	install_element(node, &no_modem_apn_cmd);

	install_element(node, &modem_svc_set_cmd);
	install_element(node, &no_modem_svc_set_cmd);

	install_element(node, &modem_ipstack_set_cmd);
	install_element(node, &no_modem_ipstack_set_cmd);

	install_element(node, &modem_secondary_set_cmd);
	install_element(node, &no_modem_secondary_set_cmd);

	install_element(node, &modem_profile_set_cmd);
	install_element(node, &no_modem_profile_set_cmd);

	install_element(node, &modem_dial_set_cmd);
	install_element(node, &no_modem_dial_set_cmd);

	install_element(node, &modem_network_set_cmd);
	install_element(node, &no_modem_network_set_cmd);
}

static void cmd_modem_interface_init (int node)
{
	install_element(node, &modem_bind_interface_set_cmd);
	install_element(node, &no_modem_bind_interface_set_cmd);

	install_element(node, &modem_bind_interface_vale_set_cmd);
	install_element(node, &no_modem_bind_interface_vale_set_cmd);

	install_element(node, &modem_bind_interface_vals_set_cmd);
	install_element(node, &no_modem_bind_interface_vals_set_cmd);
}

static void cmd_modem_show_init (int node)
{
	install_element(node, &show_modem_profile_cmd);
	install_element(node, &show_modem_profile_name_cmd);
	install_element(node, &show_modem_usb_driver_param_cmd);
	install_element(node, &show_modem_machine_state_cmd);
	install_element(node, &show_modem_machine_state_detail_cmd);
}

void cmd_modem_init (void)
{
	reinstall_node(MODEM_PROFILE_NODE, modem_write_config);
	reinstall_node(MODEM_CHANNEL_NODE, modem_serial_write_cb);

	install_default(MODEM_PROFILE_NODE);
	install_default_basic(MODEM_PROFILE_NODE);

	install_default(MODEM_CHANNEL_NODE);
	install_default_basic(MODEM_CHANNEL_NODE);

	install_element(CONFIG_NODE, &modem_profile_cmd);
	install_element(CONFIG_NODE, &no_modem_profile_cmd);

	install_element(CONFIG_NODE, &modem_channel_cmd);
	install_element(CONFIG_NODE, &no_modem_channel_cmd);


	//cmd_modem_interface_init(INTERFACE_L3_NODE);
	cmd_modem_interface_init(MODEM_PROFILE_NODE);

	cmd_modem_set_init(MODEM_PROFILE_NODE);

	cmd_modem_show_init(ENABLE_NODE);
	cmd_modem_show_init(CONFIG_NODE);

#ifdef __MODEM_DEBUG
	install_element(ENABLE_NODE, &modem_start_test_cmd);

	install_element(ENABLE_NODE, &modem_open_cmd);
	install_element(ENABLE_NODE, &modem_close_cmd);
	install_element(ENABLE_NODE, &modem_test_cmd);
#endif
}

/*
 * modem-profile NAME
 *  bind modem-channel NAME
 * exit
 *
 * modem-channel NAME
 * exit
 *
 */

