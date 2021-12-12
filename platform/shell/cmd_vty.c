/*
 * cmd_vty.c
 *
 *  Created on: Jan 2, 2018
 *      Author: zhurish
 */


#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"
#include "vty_include.h"

#include <arpa/telnet.h>
#include <termios.h>




//extern int vty_exec_timeout(struct vty *vty, const char *min_str,
//		const char *sec_str);


DEFUN (who,
		who_cmd,
		"who",
		"Display who is on vty\n")
{
	zpl_uint32  i;
	struct vty *v;

	for (i = 0; i < vector_active(cli_shell.vtyvec); i++)
		if ((v = vector_slot(cli_shell.vtyvec, i)) != NULL)
			vty_out(vty, "%svty[%d] connected from %s.%s",
					v->config ? "*" : " ", i, v->address, VTY_NEWLINE);
	return CMD_SUCCESS;
}

/* Move to vty configuration mode. */
DEFUN (line_vty,
		line_vty_cmd,
		"line vty",
		"Configure a terminal line\n"
		"Virtual terminal\n")
{
	vty->node = VTY_NODE;
	return CMD_SUCCESS;
}

DEFUN (exec_timeout_min,
		exec_timeout_min_cmd,
		"exec-timeout <0-35791>",
		"Set timeout value\n"
		"Timeout value in minutes\n")
{
	return vty_exec_timeout(vty, argv[0], NULL);
}

DEFUN (exec_timeout_sec,
		exec_timeout_sec_cmd,
		"exec-timeout <0-35791> <0-2147483>",
		"Set the EXEC timeout\n"
		"Timeout in minutes\n"
		"Timeout in seconds\n")
{
	return vty_exec_timeout(vty, argv[0], argv[1]);
}

DEFUN (no_exec_timeout,
		no_exec_timeout_cmd,
		"no exec-timeout",
		NO_STR
		"Set the EXEC timeout\n")
{
	return vty_exec_timeout(vty, NULL, NULL);
}

/* Set vty access class. */
DEFUN (vty_access_class,
		vty_access_class_cmd,
		"access-class WORD",
		"Filter connections based on an IP access list\n"
		"IP access list\n")
{
	if(host_config_set_api (API_SET_ACCESS_CMD, argv[0]) != OK)
		return CMD_WARNING;
	return CMD_SUCCESS;
}

/* Clear vty access class. */
DEFUN (no_vty_access_class,
		no_vty_access_class_cmd,
		"no access-class [WORD]",
		NO_STR
		"Filter connections based on an IP access list\n"
		"IP access list\n")
{
	if (_global_host.mutx)
		os_mutex_lock(_global_host.mutx, OS_WAIT_FOREVER);
	if (!_global_host.vty_accesslist_name
			|| (argc && strcmp(_global_host.vty_accesslist_name, argv[0])))
	{
		vty_out(vty, "Access-class is not currently applied to vty%s",VTY_NEWLINE);
		if (_global_host.mutx)
			os_mutex_unlock(_global_host.mutx);
		return CMD_WARNING;
	}

	if(host_config_set_api (API_SET_ACCESS_CMD, NULL) != OK)
		return CMD_WARNING;
	return CMD_SUCCESS;
}

#ifdef HAVE_IPV6
/* Set vty access class. */
DEFUN (vty_ipv6_access_class,
		vty_ipv6_access_class_cmd,
		"ipv6 access-class WORD",
		IPV6_STR
		"Filter connections based on an IP access list\n"
		"IPv6 access list\n")
{
	if(host_config_set_api (API_SET_IPV6ACCESS_CMD, argv[0]) != OK)
		return CMD_WARNING;
	return CMD_SUCCESS;
}

/* Clear vty access class. */
DEFUN (no_vty_ipv6_access_class,
		no_vty_ipv6_access_class_cmd,
		"no ipv6 access-class [WORD]",
		NO_STR
		IPV6_STR
		"Filter connections based on an IP access list\n"
		"IPv6 access list\n")
{
	if (_global_host.mutx)
		os_mutex_lock(_global_host.mutx, OS_WAIT_FOREVER);
	if (! _global_host.vty_ipv6_accesslist_name ||
			(argc && strcmp(_global_host.vty_ipv6_accesslist_name, argv[0])))
	{
		vty_out (vty, "IPv6 access-class is not currently applied to vty%s",
				VTY_NEWLINE);
		if (_global_host.mutx)
			os_mutex_unlock(_global_host.mutx);
		return CMD_WARNING;
	}

	if(host_config_set_api (API_SET_IPV6ACCESS_CMD, NULL) != OK)
		return CMD_WARNING;
	return CMD_SUCCESS;
}
#endif /* HAVE_IPV6 */

/* vty login. */
DEFUN (vty_login,
		vty_login_cmd,
		"login",
		"Enable password checking\n")
{
	int no_password_check = 0;
	if(host_config_set_api (API_SET_NOPASSCHK_CMD, &no_password_check) != OK)
		return CMD_WARNING;
	return CMD_SUCCESS;
}

DEFUN (no_vty_login,
		no_vty_login_cmd,
		"no login",
		NO_STR
		"Enable password checking\n")
{
	int no_password_check = 1;
	if(host_config_set_api (API_SET_NOPASSCHK_CMD, &no_password_check) != OK)
		return CMD_WARNING;
	return CMD_SUCCESS;
}


DEFUN (terminal_monitor,
		terminal_monitor_cmd,
		"terminal monitor",
		"Set terminal line parameters\n"
		"Copy debug output to the current terminal line\n")
{
	vty->monitor = 1;
	return CMD_SUCCESS;
}

DEFUN (terminal_no_monitor,
		terminal_no_monitor_cmd,
		"terminal no monitor",
		"Set terminal line parameters\n"
		NO_STR
		"Copy debug output to the current terminal line\n")
{
	vty->monitor = 0;
	return CMD_SUCCESS;
}

ALIAS(terminal_no_monitor,
		no_terminal_monitor_cmd,
		"no terminal monitor",
		NO_STR
		"Set terminal line parameters\n"
		"Copy debug output to the current terminal line\n")


DEFUN (terminal_trapping,
		terminal_trapping_cmd,
		"terminal trapping",
		"Set terminal line parameters\n"
		"Copy debug output to the current terminal line\n")
{
	vty->trapping = 1;
	return CMD_SUCCESS;
}

DEFUN (no_terminal_trapping,
		no_terminal_trapping_cmd,
		"no terminal trapping",
		NO_STR
		"Set terminal line parameters\n"
		"Copy debug output to the current terminal line\n")
{
	vty->trapping = 0;
	return CMD_SUCCESS;
}

DEFUN (show_history,
		show_history_cmd,
		"show history",
		SHOW_STR
		"Display the session command history\n")
{
	zpl_uint32 index;

	for (index = vty->hindex + 1; index != vty->hindex;) {
		if (index == VTY_MAXHIST) {
			index = 0;
			continue;
		}

		if (vty->hist[index] != NULL)
			vty_out(vty, "  %s%s", vty->hist[index], VTY_NEWLINE);

		index++;
	}

	return CMD_SUCCESS;
}

/* vty login. */
DEFUN (log_commands,
		log_commands_cmd,
		"log commands",
		"Logging control\n"
		"Log all commands (can't be unset without restart)\n")
{
	cli_shell.do_log_commands = 1;
	return CMD_SUCCESS;
}

DEFUN_HIDDEN (exit_platform,
		exit_platform_cmd,
		"exit-platform",
		"exit platform\n")
{
	/*	vty_terminate ();
	 vrf_terminate ();
	 //	cmd_terminate ();
	 exit(0);*/
	kill(getpid(), SIGTERM);
	return CMD_SUCCESS;
}

/* Display current configuration. */
static int vty_config_write(struct vty *vty)
{
	if (_global_host.mutx)
		os_mutex_lock(_global_host.mutx, OS_WAIT_FOREVER);
	vty_out(vty, "line vty%s", VTY_NEWLINE);

	if (_global_host.vty_accesslist_name)
		vty_out(vty, " access-class %s%s", _global_host.vty_accesslist_name,
				VTY_NEWLINE);

	if (_global_host.vty_ipv6_accesslist_name)
		vty_out(vty, " ipv6 access-class %s%s", _global_host.vty_ipv6_accesslist_name,
				VTY_NEWLINE);

	/* exec-timeout */
	if (_global_host.vty_timeout_val != VTY_TIMEOUT_DEFAULT)
		vty_out(vty, " exec-timeout %ld %ld%s", _global_host.vty_timeout_val / 60,
				_global_host.vty_timeout_val % 60, VTY_NEWLINE);

	/* login */
	if (_global_host.no_password_check)
		vty_out(vty, " no login%s", VTY_NEWLINE);

	if (cli_shell.do_log_commands)
		vty_out(vty, "log commands%s", VTY_NEWLINE);

	vty_out(vty, "!%s", VTY_NEWLINE);
	if (_global_host.mutx)
		os_mutex_unlock(_global_host.mutx);
	return CMD_SUCCESS;
}

struct cmd_node vty_node =
{
	VTY_NODE,
	"%s(config-line)# ",
	1,
};

int cmd_vty_init()
{

	/* Install bgp top node. */
	install_node(&vty_node, vty_config_write);

	install_element(VIEW_NODE, CMD_CONFIG_LEVEL, &who_cmd);
	install_element(VIEW_NODE, CMD_VIEW_LEVEL, &show_history_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &line_vty_cmd);

	install_element(VIEW_NODE, CMD_CONFIG_LEVEL, &exit_platform_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &exit_platform_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &exit_platform_cmd);

	install_element(CONFIG_NODE, CMD_VIEW_LEVEL, &show_history_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &log_commands_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &terminal_monitor_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &terminal_no_monitor_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &no_terminal_monitor_cmd);


	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &terminal_trapping_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &no_terminal_trapping_cmd);


	install_default(VTY_NODE);
	install_element(VTY_NODE, CMD_CONFIG_LEVEL, &exec_timeout_min_cmd);
	install_element(VTY_NODE, CMD_CONFIG_LEVEL, &exec_timeout_sec_cmd);
	install_element(VTY_NODE, CMD_CONFIG_LEVEL, &no_exec_timeout_cmd);
	install_element(VTY_NODE, CMD_CONFIG_LEVEL, &vty_access_class_cmd);
	install_element(VTY_NODE, CMD_CONFIG_LEVEL, &no_vty_access_class_cmd);
	install_element(VTY_NODE, CMD_CONFIG_LEVEL, &vty_login_cmd);
	install_element(VTY_NODE, CMD_CONFIG_LEVEL, &no_vty_login_cmd);

#ifdef HAVE_IPV6
	install_element (VTY_NODE, CMD_CONFIG_LEVEL, &vty_ipv6_access_class_cmd);
	install_element (VTY_NODE, CMD_CONFIG_LEVEL, &no_vty_ipv6_access_class_cmd);
#endif /* HAVE_IPV6 */
	return 0;
}
