/*
 * cmd_nvram_env.c
 *
 *  Created on: 2019年3月5日
 *      Author: DELL
 */


#include "zebra.h"
#include "filter.h"
#include "buffer.h"
#include "command.h"
#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "str.h"
#include "vty.h"
#include "os_nvram.h"

DEFUN (os_nvram_env_add_cli,
		os_nvram_env_add_cli_cmd,
		"nvram-env (add|set|update) NAME STRING",
		"Nvram Environment Configure\n"
		"Add New Environment\n"
		"Set Environment\n"
		"Update Environment\n"
		"Environment Name\n"
		"Environment Value\n")
{
	if(strstr(argv[0], "add"))
	{
		if(os_nvram_env_add(argv[1], argv[2]) == OK)
			return CMD_SUCCESS;
		vty_out(vty, "Can not add this ('%s') Environment%s", argv[1], VTY_NEWLINE);
		return CMD_WARNING;
	}
	else if(strstr(argv[0], "set"))
	{
		if(os_nvram_env_set(argv[1], argv[2]) == OK)
			return CMD_SUCCESS;
		vty_out(vty, "Can not set this ('%s') Environment%s", argv[1], VTY_NEWLINE);
		return CMD_WARNING;
	}
	else if(strstr(argv[0], "update"))
	{
		if(os_nvram_env_set(argv[1], argv[2]) == OK)
			return CMD_SUCCESS;
		vty_out(vty, "Can not update this ('%s') Environment%s", argv[1], VTY_NEWLINE);
		return CMD_WARNING;
	}
	return CMD_WARNING;
}

DEFUN (os_nvram_env_del_cli,
		os_nvram_env_del_cli_cmd,
		"nvram-env del NAME",
		"Nvram Environment Configure\n"
		"Delete Environment\n"
		"Environment Name\n")
{
	if(os_nvram_env_del(argv[0]) == OK)
		return CMD_SUCCESS;
	vty_out(vty, "Can not delete this ('%s') Environment%s", argv[0], VTY_NEWLINE);
	return CMD_WARNING;
}


static int nvram_show_one(struct vty *vty, os_nvram_env_t *node)
{
	if(vty && node)
	{
		vty_out(vty, " %s=%s%s", node->name, node->ptr.va_p, VTY_NEWLINE);
	}
	return OK;
}

DEFUN (os_nvram_env_show_cli,
		os_nvram_env_show_cli_cmd,
		"show nvram-env [NAME]",
		SHOW_STR
		"Nvram Environment Configure\n"
		"Environment Name\n")
{
	if(argc && argv[0])
	{
		os_nvram_env_show(argv[0], nvram_show_one, vty);
	}
	else
		os_nvram_env_show(NULL, nvram_show_one, vty);
	return CMD_SUCCESS;
}

int cmd_nvram_env_init()
{
	install_element(ENABLE_NODE, &os_nvram_env_add_cli_cmd);
	install_element(ENABLE_NODE, &os_nvram_env_del_cli_cmd);
	install_element(CONFIG_NODE, &os_nvram_env_add_cli_cmd);
	install_element(CONFIG_NODE, &os_nvram_env_del_cli_cmd);

	install_element(ENABLE_NODE, &os_nvram_env_show_cli_cmd);
	install_element(CONFIG_NODE, &os_nvram_env_show_cli_cmd);
	return OK;
}
