/*
 * cmd_user_vty.c
 *
 *  Created on: Jan 2, 2018
 *      Author: zhurish
 */


#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"
#include "vty_include.h"

DEFUN (username,
		username_cmd,
		"username WORD",
		"Establish User Name Authentication\n"
		"User Name\n")
{
	zpl_bool enc = zpl_false;
	int encrypt = 0;
	if (host_config_get_api(API_GET_ENCRYPT_CMD, &encrypt) != OK) {
		return CMD_WARNING;
	}
	if(encrypt)
		enc = zpl_true;
	if (argc == 1)
		return vty_user_create(vty, argv[0], NULL, zpl_false, enc);
	else if (argc == 2)
		return vty_user_create(vty, argv[0], argv[1], zpl_false, enc);
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
		return vty_user_delete(vty, argv[0], zpl_false, zpl_false);
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
		return vty_user_delete(vty, argv[0], zpl_true, zpl_false);
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
	zpl_bool enc = zpl_false;
	int encrypt = 0;
	if (host_config_get_api(API_GET_ENCRYPT_CMD, &encrypt) != OK) {
		return CMD_WARNING;
	}
	if(encrypt)
		enc = zpl_true;
	if (argc == 2)
		return vty_user_create(vty, argv[0], argv[1], zpl_true, enc);
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
		return vty_user_delete(vty, argv[0], zpl_true, zpl_true);
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
	return vty_user_setting_privilege(vty, argv[0], CMD_ENABLE_LEVEL);
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



static struct cmd_node user_node =
{
	USER_NODE,
	"%s(config)# ",
	1
};

int cmd_vty_user_init(void) {

	install_node (&user_node, config_write_vty_user);

	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &vty_user_switch_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &vty_user_switch_cmd);

	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &username_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &username_password_cmd);
	//install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &username_password_encrypt_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_username_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_username_password_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &username_privilege_level_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_username_privilege_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &username_enable_password_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_username_enable_password_cmd);
	//install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &username_enable_password_encrypt_cmd);
	return OK;
}

