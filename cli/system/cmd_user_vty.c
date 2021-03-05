/*
 * cmd_user_vty.c
 *
 *  Created on: Jan 2, 2018
 *      Author: zhurish
 */

#include "zebra.h"
#include "command.h"
#include "hash.h"
#include "linklist.h"
#include "memory.h"
#include "version.h"
#include "host.h"
#include "vty.h"

#include "vty_user.h"

DEFUN (username,
		username_cmd,
		"username WORD",
		"Establish User Name Authentication\n"
		"User Name\n")
{
	ospl_bool enc = ospl_false;
	int encrypt = 0;
	if (host_config_get_api(API_GET_ENCRYPT_CMD, &encrypt) != OK) {
		return CMD_WARNING;
	}
	if(encrypt)
		enc = ospl_true;
	if (argc == 1)
		return vty_user_create(vty, argv[0], NULL, ospl_false, enc);
	else if (argc == 2)
		return vty_user_create(vty, argv[0], argv[1], ospl_false, enc);
	return CMD_WARNING;
}

ALIAS(username,
		username_password_cmd,
		"username WORD password WORD",
		"Establish User Name Authentication\n"
		"User Name\n"
		"Specify the password for the user\n"
		"The UNENCRYPTED (cleartext) user password\n")

DEFUN (no_username,
		no_username_cmd,
		"no username WORD",
		NO_STR
		"Establish User Name Authentication\n"
		"User Name\n") {

	if (argc == 1)
		return vty_user_delete(vty, argv[0], ospl_false, ospl_false);
	return CMD_WARNING;
}

DEFUN (no_username_password,
		no_username_password_cmd,
		"no username WORD password",
		NO_STR
		"Establish User Name Authentication\n"
		"User Name\n"
		"Specify the password for the user\n") {

	if (argc == 1)
		return vty_user_delete(vty, argv[0], ospl_true, ospl_false);
	return CMD_WARNING;

}

DEFUN (username_enable_password,
		username_enable_password_cmd,
		"username WORD enable password WORD",
		"Establish User Name Authentication\n"
		"User Name\n"
		"Modify enable password parameters\n"
		"Specify the password for the user\n"
		"The UNENCRYPTED (cleartext) user password\n")
{
	ospl_bool enc = ospl_false;
	int encrypt = 0;
	if (host_config_get_api(API_GET_ENCRYPT_CMD, &encrypt) != OK) {
		return CMD_WARNING;
	}
	if(encrypt)
		enc = ospl_true;
	if (argc == 2)
		return vty_user_create(vty, argv[0], argv[1], ospl_true, enc);
	return CMD_WARNING;

}

DEFUN (no_username_enable_password,
		no_username_enable_password_cmd,
		"no username WORD enable password",
		NO_STR
		"Establish User Name Authentication\n"
		"User Name\n"
		"Modify enable password parameters\n"
		"Specify the password for the user\n") {

	if (argc == 2)
		return vty_user_delete(vty, argv[0], ospl_true, ospl_true);
	return CMD_WARNING;

}

DEFUN (username_privilege_level,
		username_privilege_level_cmd,
		"username WORD privilege <1-4>",
		"Establish User Name Authentication\n"
		"User name\n"
		"Set user privilege level\n"
		"User privilege level\n")
{
	return vty_user_setting_privilege(vty, argv[0], atoi(argv[1]));
}

DEFUN (no_username_privilege,
		no_username_privilege_cmd,
		"no username WORD privilege",
		NO_STR
		"Establish User Name Authentication\n"
		"User name\n"
		"User privilege level\n")
{
	return vty_user_setting_privilege(vty, argv[0], ENABLE_LEVEL);
}

DEFUN_HIDDEN (vty_user_switch,
		vty_user_switch_cmd,
		"switch username WORD",
		"Change to anther user\n"
		"Establish User Name Authentication\n"
		"User name\n")
{
	return vty_user_change(vty, argv[0]);
}

int cmd_vty_user_init() {
	//install_node (&user_node, NULL);

	install_element(ENABLE_NODE, &vty_user_switch_cmd);
	install_element(CONFIG_NODE, &vty_user_switch_cmd);

	install_element(CONFIG_NODE, &username_cmd);
	install_element(CONFIG_NODE, &username_password_cmd);
	//install_element (CONFIG_NODE, &username_password_encrypt_cmd);
	install_element(CONFIG_NODE, &no_username_cmd);
	install_element(CONFIG_NODE, &no_username_password_cmd);
	install_element(CONFIG_NODE, &username_privilege_level_cmd);
	install_element(CONFIG_NODE, &no_username_privilege_cmd);
	install_element(CONFIG_NODE, &username_enable_password_cmd);
	install_element(CONFIG_NODE, &no_username_enable_password_cmd);
	//install_element (CONFIG_NODE, &username_enable_password_encrypt_cmd);
	return OK;
}

