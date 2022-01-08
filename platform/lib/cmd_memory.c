/*
 * cmd_memory.c
 *
 *  Created on: Jan 2, 2018
 *      Author: zhurish
 */


#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"
#include "vty_include.h"



DEFUN (show_memory,
       show_memory_cmd,
       "show memory",
       "Show running system information\n"
       "Memory statistics\n")
{
  return vty_show_memory_cmd(vty);
}

DEFUN (show_system_clock,
		show_system_clock_cmd,
		"show system clock",
		SHOW_STR
		"Displays system information\n"
		"Displays Clock information\n")
{
	vty_out(vty, "%s", VTY_NEWLINE);
	vty_out(vty, "system current time  : %s %s", os_time_fmt("/",os_time (NULL)), VTY_NEWLINE);
	vty_out(vty, "system running time  : %s %s", os_time_string(os_monotonic_time ()), VTY_NEWLINE);
	vty_out(vty, "system hw-clock time : %s %s", os_time_fmt("/",os_monotonic_time ()), VTY_NEWLINE);

	if(argc == 1)
	{
		struct timeval tv;
		vty_out(vty, "os time              : %u %s", os_time (NULL), VTY_NEWLINE);
		vty_out(vty, "os monotonic time    : %u %s", os_monotonic_time (), VTY_NEWLINE);
		os_gettimeofday (&tv);
		vty_out(vty, "os timeofday         : %u.%u %s", tv.tv_sec, tv.tv_usec/1000, VTY_NEWLINE);
		os_get_realtime (&tv);
		vty_out(vty, "os realtime          : %u.%u %s", tv.tv_sec, tv.tv_usec/1000, VTY_NEWLINE);
		os_get_monotonic (&tv);
		vty_out(vty, "os monotonic         : %u.%u %s", tv.tv_sec, tv.tv_usec/1000, VTY_NEWLINE);
	}
	vty_out(vty, "%s", VTY_NEWLINE);
	return CMD_SUCCESS;
}

ALIAS_HIDDEN(show_system_clock,
		show_system_clock_detal_cmd,
		"show system clock (detail|)",
		SHOW_STR
		"Displays system information\n"
		"Displays Clock information\n"
		"Dtail information\n");


void
cmd_memory_init (void)
{
  install_element (VIEW_NODE, CMD_VIEW_LEVEL, &show_memory_cmd);
  install_element (VIEW_NODE, CMD_VIEW_LEVEL, &show_system_clock_cmd);
  install_element (VIEW_NODE, CMD_VIEW_LEVEL, &show_system_clock_detal_cmd);
}
