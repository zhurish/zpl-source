/*
 * cmd_serial.c
 *
 *  Created on: Sep 13, 2018
 *      Author: zhurish
 */


#include "auto_include.h"
#include <zplos_include.h>
#include "route_types.h"
#include "nsm_event.h"
#include "zmemory.h"
#include "if.h"
#include "vrf.h"
#include "prefix.h"
#include "command.h"
#include "nsm_serial.h"

DEFUN (serial_clock_rate,
		serial_clock_rate_cmd,
		"clock rate (2400|4800|9600|19200|57600|64000|115200)",
		"clock configure\n"
		"rate configure\n"
		"rate 2400\n"
		"rate 4800\n"
		"rate 9600\n"
		"rate 19200\n"
		"rate 57600\n"
		"rate 64000\n"
		"rate 115200\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(ifp)
		ret = nsm_serial_interface_clock(ifp, atoi(argv[0]));
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_serial_clock_rate,
		no_serial_clock_rate_cmd,
		"no clock rate",
		NO_STR
		"clock configure\n"
		"rate configure\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(ifp)
		ret = nsm_serial_interface_clock(ifp, NSM_SERIAL_CLOCK_DEFAULT);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (serial_data_bit,
		serial_data_bit_cmd,
		"data bit (5|6|7|8)",
		"serial data weigh configure\n"
		"data bit configure\n"
		"5 bit\n"
		"6 bit\n"
		"7 bit\n"
		"8 bit\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(ifp)
		ret = nsm_serial_interface_databit(ifp, atoi(argv[0]));
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_serial_data_bit,
		no_serial_data_bit_cmd,
		"no data bit",
		NO_STR
		"serial data weigh configure\n"
		"data bit configure\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(ifp)
		ret = nsm_serial_interface_databit(ifp, NSM_SERIAL_DATA_DEFAULT);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (serial_stop_bit,
		serial_stop_bit_cmd,
		"stop bit (1|2)",
		"serial stop weigh configure\n"
		"stop bit configure\n"
		"1 bit\n"
		"2 bit\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(ifp)
		ret = nsm_serial_interface_stopbit(ifp, atoi(argv[0]));
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_serial_stop_bit,
		no_serial_stop_bit_cmd,
		"no stop bit",
		NO_STR
		"serial stop weigh configure\n"
		"stop bit configure\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(ifp)
		ret = nsm_serial_interface_stopbit(ifp, NSM_SERIAL_STOP_DEFAULT);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (serial_parity,
		serial_parity_cmd,
		"parity mode (none|even|odd|mark|space)",
		"serial parity mode configure\n"
		"mode configure\n"
		"none mode\n"
		"even mode\n"
		"odd mode\n"
		"mark mode\n"
		"space mode\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(ifp)
	{
		enum parity_mode mode = TTY_PARITY_NONE;
		if(os_memcmp(argv[0], "none", 3) == 0)
			mode = TTY_PARITY_NONE;
		else if(os_memcmp(argv[0], "even", 3) == 0)
			mode = TTY_PARITY_EVEN;
		else if(os_memcmp(argv[0], "odd", 3) == 0)
			mode = TTY_PARITY_ODD;
		else if(os_memcmp(argv[0], "mark", 3) == 0)
			mode = TTY_PARITY_MARK;
		else if(os_memcmp(argv[0], "space", 3) == 0)
			mode = TTY_PARITY_SPACE;
		ret = nsm_serial_interface_parity(ifp, mode);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_serial_parity,
		no_serial_parity_cmd,
		"no parity mode",
		NO_STR
		"serial parity mode configure\n"
		"mode configure\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(ifp)
		ret = nsm_serial_interface_parity(ifp, NSM_SERIAL_PARITY_DEFAULT);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (serial_flow,
		serial_flow_cmd,
		"flow-control (none|sorfware|hardware)",
		"serial flow-control\n"
		"none mode\n"
		"sorfware mode\n"
		"hardware mode\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(ifp)
	{
		enum flow_ctl_mode mode = TTY_FLOW_CTL_NONE;
		if(os_memcmp(argv[0], "none", 3) == 0)
			mode = TTY_FLOW_CTL_NONE;
		else if(os_memcmp(argv[0], "sorfware", 3) == 0)
			mode = TTY_FLOW_CTL_SW;
		else if(os_memcmp(argv[0], "hardware", 3) == 0)
			mode = TTY_FLOW_CTL_HW;
		ret = nsm_serial_interface_flow_control(ifp, mode);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_serial_flow,
		no_serial_flow_cmd,
		"no flow-control",
		NO_STR
		"serial flow-control\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(ifp)
		ret = nsm_serial_interface_flow_control(ifp, NSM_SERIAL_PARITY_DEFAULT);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


static void cmd_base_serial_init(int node)
{
	install_element(node, CMD_CONFIG_LEVEL, &serial_clock_rate_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_serial_clock_rate_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &serial_data_bit_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_serial_data_bit_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &serial_stop_bit_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_serial_stop_bit_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &serial_parity_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_serial_parity_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &serial_flow_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_serial_flow_cmd);
}

void cmd_serial_init(void)
{
	cmd_base_serial_init(SERIAL_INTERFACE_NODE);
}
