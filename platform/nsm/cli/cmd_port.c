/*
 * cmd_port.c
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */



#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"
#include "vty_include.h"



DEFUN(nsm_interface_switchport,
	  nsm_interface_switchport_cmd,
	  "switchport",
	  "set this interface Switchport\n")
{
	int ret = 0;
	struct interface *ifp = vty->index;
	if (ifp)
	{
		if (ifp->if_type == IF_ETHERNET || 
			ifp->if_type == IF_GIGABT_ETHERNET || 
			ifp->if_type == IF_LAG)
		{
			ret = nsm_interface_mode_set_api(ifp, IF_MODE_ACCESS_L2);
			if (ret == OK)
			{
				if (ifp->if_type == IF_LAG)
					vty->node = LAG_INTERFACE_NODE;
				else
					vty->node = INTERFACE_NODE;
			}
			return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
		}
		else
			return CMD_SUCCESS;
	}
	else if (vty->index_range)
	{
		zpl_uint32 i = 0;
		for (i = 0; i < vty->index_range; i++)
		{
			ifp = vty->vty_range_index[i];
			if (ifp->if_type == IF_ETHERNET || 
				ifp->if_type == IF_GIGABT_ETHERNET || 
				ifp->if_type == IF_LAG)
			{
				ret = nsm_interface_mode_set_api(ifp, IF_MODE_ACCESS_L2);
				if (ret == OK)
				{
					if (ifp->if_type == IF_LAG)
						vty->node = LAG_INTERFACE_NODE;
					else
						vty->node = INTERFACE_NODE;
				}
				if (ret != OK)
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}
#ifdef ZPL_NSM_L3MODULE
DEFUN(no_nsm_interface_switchport,
	  no_nsm_interface_switchport_cmd,
	  "no switchport",
	  NO_STR
	  "set this interface Switchport\n")
{
	int ret = 0;
	struct interface *ifp = vty->index;
	if (ifp)
	{
		if (ifp->if_type == IF_ETHERNET || 
			ifp->if_type == IF_GIGABT_ETHERNET || 
			ifp->if_type == IF_LAG)
		{
			ret = nsm_interface_mode_set_api(ifp, IF_MODE_L3);
			if (ret == OK)
			{
				if (ifp->if_type == IF_LAG)
					vty->node = LAG_INTERFACE_L3_NODE;
				else
					vty->node = INTERFACE_L3_NODE;
			}
			return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
		}
		else
			return CMD_SUCCESS;
	}
	else if (vty->index_range)
	{
		zpl_uint32 i = 0;
		for (i = 0; i < vty->index_range; i++)
		{
			ifp = vty->vty_range_index[i];
			if (ifp->if_type == IF_ETHERNET || 
				ifp->if_type == IF_GIGABT_ETHERNET || 
				ifp->if_type == IF_LAG)
			{
				ret = nsm_interface_mode_set_api(ifp, IF_MODE_L3);
				if (ret == OK)
				{
					if (ifp->if_type == IF_LAG)
						vty->node = LAG_INTERFACE_L3_NODE;
					else
						vty->node = INTERFACE_L3_NODE;
				}
				if (ret != OK)
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}
#endif

DEFUN (nsm_interface_switchport_mode,
		nsm_interface_switchport_mode_cmd,
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
	else if(vty->index_range)
	{
  		zpl_uint32 i = 0;
		for(i = 0; i < vty->index_range; i++)
		{
			if(vty->vty_range_index[i])
			{
				if(nsm_interface_mode_get_api(vty->vty_range_index[i], &mode) == OK)
				{
					if(os_memcmp(argv[0], "access", 3) == 0)
						newmode = IF_MODE_ACCESS_L2;
					else
						newmode = IF_MODE_TRUNK_L2;
					if(newmode !=mode)
						ret = nsm_interface_mode_set_api(vty->vty_range_index[i], newmode);
					if(ret != OK)
						return CMD_WARNING;
				}
				else
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}


DEFUN (no_nsm_interface_switchport_mode,
		no_nsm_interface_switchport_mode_cmd,
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
	else if(vty->index_range)
	{
  		zpl_uint32 i = 0;
		for(i = 0; i < vty->index_range; i++)
		{
			if(vty->vty_range_index[i])
			{
				if(nsm_interface_mode_get_api(vty->vty_range_index[i], &mode) == OK)
				{
					newmode = IF_MODE_ACCESS_L2;
					if(newmode !=mode)
						ret = nsm_interface_mode_set_api(vty->vty_range_index[i], newmode);
					if(ret != OK)
						return CMD_WARNING;
				}
				else
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

/* port */

#ifdef ZPL_NSM_MAC
DEFUN (mac_address_learn,
		mac_address_learn_cmd,
		CMD_MAC_ADDRESS_LEARN_STR" (enable|disable)",
		CMD_MAC_ADDRESS_LEARN_STR_HELP
		"Enable\n"
		"Disable\n")
{
	int ret = 0;
	zpl_bool value = zpl_false;
	struct interface *ifp = vty->index;	
	if(ifp)
	{
		if(nsm_port_learning_get_api(ifp, &value) == OK)
		{
			if(os_memcmp(argv[0], "enable", 3) == 0)
			{
				if(value)
					return CMD_SUCCESS;
				else
				{
					ret = nsm_port_learning_set_api(ifp, zpl_true);
				}	
			}
			else
			{
				if(!value)
					return CMD_SUCCESS;
				else
				{
					ret = nsm_port_learning_set_api(ifp, zpl_false);
				}	
			}
		}
		return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	else if(vty->index_range)
	{
  		zpl_uint32 i = 0;
		for(i = 0; i < vty->index_range; i++)
		{
			if(vty->vty_range_index[i])
			{
				if(nsm_port_learning_get_api(vty->vty_range_index[i], &value) == OK)
				{
					if(os_memcmp(argv[0], "enable", 3) == 0)
					{
						if(value)
							return CMD_SUCCESS;
						else
						{
							ret = nsm_port_learning_set_api(vty->vty_range_index[i], zpl_true);
						}	
					}
					else
					{
						if(!value)
							return CMD_SUCCESS;
						else
						{
							ret = nsm_port_learning_set_api(vty->vty_range_index[i], zpl_false);
						}	
					}
					if(ret != OK)
						return CMD_WARNING;
				}
				else
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN (mac_address_learn_software,
		mac_address_learn_software_cmd,
		CMD_MAC_ADDRESS_LEARN_STR" software (enable|disable)",
		CMD_MAC_ADDRESS_LEARN_STR_HELP
		"Software\n"
		"Enable\n"
		"Disable\n")
{
	int ret = 0;
	zpl_bool value = zpl_false;
	struct interface *ifp = vty->index;
	
	if(ifp)
	{
		if(nsm_port_sw_learning_get_api(ifp, &value) == OK)
		{
			if(os_memcmp(argv[0], "enable", 3) == 0)
			{
				if(value)
					return CMD_SUCCESS;
				else
				{
					ret = nsm_port_sw_learning_set_api(ifp, zpl_true);
				}	
			}
			else
			{
				if(!value)
					return CMD_SUCCESS;
				else
				{
					ret = nsm_port_sw_learning_set_api(ifp, zpl_false);
				}	
			}
		}
		return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	else if(vty->index_range)
	{
  		zpl_uint32 i = 0;
		for(i = 0; i < vty->index_range; i++)
		{
			if(vty->vty_range_index[i])
			{
				if(nsm_port_sw_learning_get_api(vty->vty_range_index[i], &value) == OK)
				{
					if(os_memcmp(argv[0], "enable", 3) == 0)
					{
						if(value)
							return CMD_SUCCESS;
						else
						{
							ret = nsm_port_sw_learning_set_api(vty->vty_range_index[i], zpl_true);
						}	
					}
					else
					{
						if(!value)
							return CMD_SUCCESS;
						else
						{
							ret = nsm_port_sw_learning_set_api(vty->vty_range_index[i], zpl_false);
						}	
					}
					if(ret != OK)
						return CMD_WARNING;
				}
				else
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}
#endif

DEFUN (jumboframe_enable,
		jumboframe_enable_cmd,
		"jumboframe (enable|disable)",
		"Jumbo Frame\n"
		"Enable\n"
		"Disable\n")
{
	int ret = 0;
	zpl_bool value = zpl_false;
	struct interface *ifp = vty->index;
	
	if(ifp)
	{
		if(nsm_port_jumbo_get_api(ifp, &value) == OK)
		{
			if(os_memcmp(argv[0], "enable", 3) == 0)
			{
				if(value)
					return CMD_SUCCESS;
				else
				{
					ret = nsm_port_jumbo_set_api(ifp, zpl_true);
				}	
			}
			else
			{
				if(!value)
					return CMD_SUCCESS;
				else
				{
					ret = nsm_port_jumbo_set_api(ifp, zpl_false);
				}	
			}
		}
		return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	else if(vty->index_range)
	{
  		zpl_uint32 i = 0;
		for(i = 0; i < vty->index_range; i++)
		{
			if(vty->vty_range_index[i])
			{
				if(nsm_port_jumbo_get_api(vty->vty_range_index[i], &value) == OK)
				{
					if(os_memcmp(argv[0], "enable", 3) == 0)
					{
						if(value)
							return CMD_SUCCESS;
						else
						{
							ret = nsm_port_jumbo_set_api(vty->vty_range_index[i], zpl_true);
						}	
					}
					else
					{
						if(!value)
							return CMD_SUCCESS;
						else
						{
							ret = nsm_port_jumbo_set_api(vty->vty_range_index[i], zpl_false);
						}	
					}
					if(ret != OK)
						return CMD_WARNING;
				}
				else
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}


DEFUN (port_loopback,
		port_loopback_cmd,
		"port-loopback",
		"Port Loopback\n")
{
	int ret = 0;
	zpl_bool value = zpl_false;
	struct interface *ifp = vty->index;
	
	if(ifp)
	{
		if(nsm_port_loopback_get_api(ifp, &value) == OK)
		{
			if(!value)
			{
				ret = nsm_port_loopback_set_api(ifp, zpl_true);	
			}
			else
				return CMD_SUCCESS;
		}
		return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	else if(vty->index_range)
	{
  		zpl_uint32 i = 0;
		for(i = 0; i < vty->index_range; i++)
		{
			if(vty->vty_range_index[i])
			{
				if(nsm_port_loopback_get_api(vty->vty_range_index[i], &value) == OK)
				{
					if(!value)
						ret = nsm_port_loopback_set_api(vty->vty_range_index[i], zpl_true);	
					if(ret != OK)
						return CMD_WARNING;
				}
				else
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN (no_port_loopback,
		no_port_loopback_cmd,
		"no port-loopback",
		NO_STR
		"Port Loopback\n")
{
	int ret = 0;
	zpl_bool value = zpl_false;
	struct interface *ifp = vty->index;
	
	if(ifp)
	{
		if(nsm_port_loopback_get_api(ifp, &value) == OK)
		{
			if(value)
			{
				ret = nsm_port_loopback_set_api(ifp, zpl_false);	
			}
			else
				return CMD_SUCCESS;
		}
		return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	else if(vty->index_range)
	{
  		zpl_uint32 i = 0;
		for(i = 0; i < vty->index_range; i++)
		{
			if(vty->vty_range_index[i])
			{
				if(nsm_port_loopback_get_api(vty->vty_range_index[i], &value) == OK)
				{
					if(value)
						ret = nsm_port_loopback_set_api(vty->vty_range_index[i], zpl_false);	
					if(ret != OK)
						return CMD_WARNING;
				}
				else
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN (port_protect,
		port_protect_cmd,
		"port-protect",
		"Port protect\n")
{
	int ret = 0;
	zpl_bool value = zpl_false;
	struct interface *ifp = vty->index;
	
	if(ifp)
	{
		if(nsm_port_protect_get_api(ifp, &value) == OK)
		{
			if(!value)
			{
				ret = nsm_port_protect_set_api(ifp, zpl_true);	
			}
			else
				return CMD_SUCCESS;
		}
		return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	else if(vty->index_range)
	{
  		zpl_uint32 i = 0;
		for(i = 0; i < vty->index_range; i++)
		{
			if(vty->vty_range_index[i])
			{
				if(nsm_port_protect_get_api(vty->vty_range_index[i], &value) == OK)
				{
					if(!value)
						ret = nsm_port_protect_set_api(vty->vty_range_index[i], zpl_true);	
					if(ret != OK)
						return CMD_WARNING;
				}
				else
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN (no_port_protect,
		no_port_protect_cmd,
		"no port-protect",
		NO_STR
		"Port protect\n")
{
	int ret = 0;
	zpl_bool value = zpl_false;
	struct interface *ifp = vty->index;
	
	if(ifp)
	{
		if(nsm_port_protect_get_api(ifp, &value) == OK)
		{
			if(value)
			{
				ret = nsm_port_protect_set_api(ifp, zpl_false);	
			}
			else
				return CMD_SUCCESS;
		}
		return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	else if(vty->index_range)
	{
  		zpl_uint32 i = 0;
		for(i = 0; i < vty->index_range; i++)
		{
			if(vty->vty_range_index[i])
			{
				if(nsm_port_protect_get_api(vty->vty_range_index[i], &value) == OK)
				{
					if(value)
						ret = nsm_port_protect_set_api(vty->vty_range_index[i], zpl_false);	
					if(ret != OK)
						return CMD_WARNING;
				}
				else
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN (port_pause,
		port_pause_cmd,
		"port-pause (transmit|receive)",
		"Port Pause\n"
		"Port Pause Transmit Enable\n"
		"Port Pause Receive Enable\n")
{
	int ret = 0;
	zpl_bool tx = zpl_false, rx = zpl_false;
	struct interface *ifp = vty->index;
	
	if(ifp)
	{
		if(nsm_port_pause_get_api(ifp, &tx, &rx) == OK)
		{
			if(strncmp(argv[0], "tran", 4) == 0)
				tx = zpl_true;
			if(strncmp(argv[0], "receive", 4) == 0)
				rx = zpl_true;
			ret = nsm_port_pause_set_api(ifp, tx, rx);	
		}
		return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	else if(vty->index_range)
	{
  		zpl_uint32 i = 0;
		for(i = 0; i < vty->index_range; i++)
		{
			if(vty->vty_range_index[i])
			{
				if(nsm_port_pause_get_api(vty->vty_range_index[i], &tx, &rx) == OK)
				{
					if(strncmp(argv[0], "tran", 4) == 0)
						tx = zpl_true;
					if(strncmp(argv[0], "receive", 4) == 0)
						rx = zpl_true;
					ret = nsm_port_pause_set_api(vty->vty_range_index[i], tx, rx);	
					if(ret != OK)
						return CMD_WARNING;
				}
				else
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN (no_port_pause,
		no_port_pause_cmd,
		"no port-pause (transmit|receive)",
		NO_STR
		"Port Pause\n"
		"Port Pause Transmit Enable\n"
		"Port Pause Receive Enable\n")
{
	int ret = 0;
	zpl_bool tx = zpl_false, rx = zpl_false;
	struct interface *ifp = vty->index;
	
	if(ifp)
	{
		if(nsm_port_pause_get_api(ifp, &tx, &rx) == OK)
		{
			if(strncmp(argv[0], "tran", 4) == 0)
				tx = zpl_false;
			if(strncmp(argv[0], "receive", 4) == 0)
				rx = zpl_false;
			ret = nsm_port_pause_set_api(ifp, tx, rx);
		}
		return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	else if(vty->index_range)
	{
  		zpl_uint32 i = 0;
		for(i = 0; i < vty->index_range; i++)
		{
			if(vty->vty_range_index[i])
			{
				if(nsm_port_pause_get_api(vty->vty_range_index[i], &tx, &rx) == OK)
				{
					if(strncmp(argv[0], "tran", 4) == 0)
						tx = zpl_false;
					if(strncmp(argv[0], "receive", 4) == 0)
						rx = zpl_false;
					ret = nsm_port_pause_set_api(vty->vty_range_index[i], tx, rx);	
					if(ret != OK)
						return CMD_WARNING;
				}
				else
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}


DEFUN (port_flowcontrol,
		port_flowcontrol_cmd,
		"flowcontrol (transmit|receive)",
		"Port Flowcontrol\n"
		"Flowcontrol Transmit Enable\n"
		"Flowcontrol Receive Enable\n")
{
	int ret = 0;
	zpl_bool tx = zpl_false, rx = zpl_false;
	struct interface *ifp = vty->index;
	
	if(ifp)
	{
		if(nsm_port_flowcontrol_get_api(ifp, &tx, &rx) == OK)
		{
			if(strncmp(argv[0], "tran", 4) == 0)
				tx = zpl_true;
			if(strncmp(argv[0], "receive", 4) == 0)
				rx = zpl_true;
			ret = nsm_port_flowcontrol_set_api(ifp, tx, rx);	
		}
		return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	else if(vty->index_range)
	{
  		zpl_uint32 i = 0;
		for(i = 0; i < vty->index_range; i++)
		{
			if(vty->vty_range_index[i])
			{
				if(nsm_port_flowcontrol_get_api(vty->vty_range_index[i], &tx, &rx) == OK)
				{
					if(strncmp(argv[0], "tran", 4) == 0)
						tx = zpl_true;
					if(strncmp(argv[0], "receive", 4) == 0)
						rx = zpl_true;
					ret = nsm_port_flowcontrol_set_api(vty->vty_range_index[i], tx, rx);	
					if(ret != OK)
						return CMD_WARNING;
				}
				else
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN (no_port_flowcontrol,
		no_port_flowcontrol_cmd,
		"no flowcontrol (transmit|receive)",
		NO_STR
		"Port Flowcontrol\n"
		"Flowcontrol Transmit Enable\n"
		"Flowcontrol Receive Enable\n")
{
	int ret = 0;
	zpl_bool tx = zpl_false, rx = zpl_false;
	struct interface *ifp = vty->index;
	
	if(ifp)
	{
		if(nsm_port_flowcontrol_get_api(ifp, &tx, &rx) == OK)
		{
			if(strncmp(argv[0], "tran", 4) == 0)
				tx = zpl_false;
			if(strncmp(argv[0], "receive", 4) == 0)
				rx = zpl_false;
			ret = nsm_port_flowcontrol_set_api(ifp, tx, rx);
		}
		return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	else if(vty->index_range)
	{
  		zpl_uint32 i = 0;
		for(i = 0; i < vty->index_range; i++)
		{
			if(vty->vty_range_index[i])
			{
				if(nsm_port_flowcontrol_get_api(vty->vty_range_index[i], &tx, &rx) == OK)
				{
					if(strncmp(argv[0], "tran", 4) == 0)
						tx = zpl_false;
					if(strncmp(argv[0], "receive", 4) == 0)
						rx = zpl_false;
					ret = nsm_port_flowcontrol_set_api(vty->vty_range_index[i], tx, rx);	
					if(ret != OK)
						return CMD_WARNING;
				}
				else
					return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}


DEFUN (show_unit_board_info,
		show_unit_board_info_cmd,
		"show unit slot info ",
		SHOW_STR
		"Uint\n"
		"Slot\n"
		"Information\n")
{
	unit_board_show(vty);
	return CMD_SUCCESS;
}


static void cmd_port_base_init (int node)
{
#ifdef ZPL_NSM_MAC	
	install_element(node, CMD_CONFIG_LEVEL, &mac_address_learn_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &mac_address_learn_software_cmd);
#endif	
	install_element(node, CMD_CONFIG_LEVEL, &jumboframe_enable_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &port_loopback_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_port_loopback_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &port_protect_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_port_protect_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &port_pause_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_port_pause_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &port_flowcontrol_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_port_flowcontrol_cmd);	
}

void cmd_port_init (void)
{
	install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_unit_board_info_cmd);


	install_element(INTERFACE_NODE, CMD_CONFIG_LEVEL, &nsm_interface_switchport_cmd);
	install_element(INTERFACE_RANGE_NODE, CMD_CONFIG_LEVEL, &nsm_interface_switchport_cmd);
#ifdef ZPL_NSM_L3MODULE	
	install_element(INTERFACE_NODE, CMD_CONFIG_LEVEL, &no_nsm_interface_switchport_cmd);
	install_element(INTERFACE_RANGE_NODE, CMD_CONFIG_LEVEL, &no_nsm_interface_switchport_cmd);
	install_element(LAG_INTERFACE_NODE, CMD_CONFIG_LEVEL, &no_nsm_interface_switchport_cmd);
#endif

	install_element(INTERFACE_L3_NODE, CMD_CONFIG_LEVEL, &nsm_interface_switchport_cmd);
	install_element(INTERFACE_L3_RANGE_NODE, CMD_CONFIG_LEVEL, &nsm_interface_switchport_cmd);


	install_element(LAG_INTERFACE_NODE, CMD_CONFIG_LEVEL, &nsm_interface_switchport_cmd);
	install_element(LAG_INTERFACE_L3_NODE, CMD_CONFIG_LEVEL, &nsm_interface_switchport_cmd);

	cmd_port_base_init (INTERFACE_NODE);
	cmd_port_base_init (INTERFACE_L3_NODE);
	cmd_port_base_init (INTERFACE_RANGE_NODE);
	cmd_port_base_init (INTERFACE_L3_RANGE_NODE);	
	cmd_port_base_init (LAG_INTERFACE_NODE);
	cmd_port_base_init (LAG_INTERFACE_L3_NODE);	
/*
	install_element(EPON_INTERFACE_NODE, CMD_CONFIG_LEVEL, &nsm_interface_switchport_cmd);
	install_element(EPON_INTERFACE_NODE, CMD_CONFIG_LEVEL, &no_nsm_interface_switchport_cmd);
	install_element(EPON_INTERFACE_RANGE_NODE, CMD_CONFIG_LEVEL, &nsm_interface_switchport_cmd);
	install_element(EPON_INTERFACE_RANGE_NODE, CMD_CONFIG_LEVEL, &no_nsm_interface_switchport_cmd);

	install_element(EPON_INTERFACE_L3_NODE, CMD_CONFIG_LEVEL, &nsm_interface_switchport_cmd);
	install_element(EPON_INTERFACE_L3_RANGE_NODE, CMD_CONFIG_LEVEL, &nsm_interface_switchport_cmd);

	install_element(E1_INTERFACE_L3_NODE, CMD_CONFIG_LEVEL, &no_nsm_interface_switchport_cmd);
	install_element(E1_INTERFACE_L3_RANGE_NODE, CMD_CONFIG_LEVEL, &no_nsm_interface_switchport_cmd);
*/

	install_element(INTERFACE_NODE, CMD_CONFIG_LEVEL, &nsm_interface_switchport_mode_cmd);
	install_element(INTERFACE_NODE, CMD_CONFIG_LEVEL, &no_nsm_interface_switchport_mode_cmd);
	install_element(INTERFACE_RANGE_NODE, CMD_CONFIG_LEVEL, &nsm_interface_switchport_mode_cmd);
	install_element(INTERFACE_RANGE_NODE, CMD_CONFIG_LEVEL, &no_nsm_interface_switchport_mode_cmd);

	install_element(LAG_INTERFACE_NODE, CMD_CONFIG_LEVEL, &nsm_interface_switchport_mode_cmd);
	install_element(LAG_INTERFACE_NODE, CMD_CONFIG_LEVEL, &no_nsm_interface_switchport_mode_cmd);
/*
	install_element(EPON_INTERFACE_NODE, CMD_CONFIG_LEVEL, &nsm_interface_switchport_mode_cmd);
	install_element(EPON_INTERFACE_NODE, CMD_CONFIG_LEVEL, &no_nsm_interface_switchport_mode_cmd);

	install_element(EPON_INTERFACE_RANGE_NODE, CMD_CONFIG_LEVEL, &nsm_interface_switchport_mode_cmd);
	install_element(EPON_INTERFACE_RANGE_NODE, CMD_CONFIG_LEVEL, &no_nsm_interface_switchport_mode_cmd);
*/	
}


