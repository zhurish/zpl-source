/*
 * cmd_web.c
 *
 *  Created on: 2019年10月22日
 *      Author: zhurish
 */


#include "auto_include.h"
#include <zplos_include.h>
#include "command.h"
#include "vty.h"
#include "template.h"
#ifdef ZPL_WEBGUI_MODULE
#include "web_api.h"
#endif

static int webserver_write_config(struct vty *vty, void *pVoid);

DEFUN (webserver_template,
		webserver_template_cmd,
		"template webserver",
		"Template configure\n"
		"Webserver configure\n")
{
	template_t * temp = lib_template_lookup_name (zpl_false, "webserver");
	if(temp)
	{
		vty->node = TEMPLATE_NODE;
		memset(vty->prompt, 0, sizeof(vty->prompt));
		sprintf(vty->prompt, "%s", temp->prompt);
		web_app_enable_set_api(zpl_true);
		return CMD_SUCCESS;
	}
	else
	{
		temp = lib_template_new (zpl_false);
		if(temp)
		{
			temp->module = 0;
			strcpy(temp->name, "webserver");
			strcpy(temp->prompt, "webserver"); /* (config-app-esp)# */
			temp->write_template = webserver_write_config;
			temp->pVoid = NULL;
			lib_template_install(temp, 0);

			vty->node = TEMPLATE_NODE;
			memset(vty->prompt, 0, sizeof(vty->prompt));
			sprintf(vty->prompt, "%s", temp->prompt);
			web_app_enable_set_api(zpl_true);
			return CMD_SUCCESS;
		}
	}
	return CMD_WARNING;
}

DEFUN (no_webserver_template,
		no_webserver_template_cmd,
		"no template webserver",
		NO_STR
		"Template configure\n"
		"Webserver configure\n")
{
	template_t * temp = lib_template_lookup_name (zpl_false, "webserver");
	if(temp)
	{
		web_app_enable_set_api(zpl_false);
		lib_template_free(temp);
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

#if ME_COM_SSL
DEFUN (webserver_https_enable,
	   webserver_https_enable_cmd,
		"webserver https enable" ,
		"Webserver configure\n"
		"HTTPS Protocol\n"
		"Enable\n")
{
	int ret = ERROR;
	ret = web_app_proto_set_api(WEB_PROTO_HTTPS);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_webserver_https_enable,
		no_webserver_https_enable_cmd,
		"no webserver https enable",
		NO_STR
		"Webserver configure\n"
		"HTTPS Protocol\n"
		"Enable\n")
{
	int ret = ERROR;
	ret = web_app_proto_set_api(WEB_PROTO_HTTP);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
#endif


DEFUN (webserver_address,
		webserver_address_cmd,
		"webserver listen address " CMD_KEY_IPV4,
		"Webserver configure\n"
		"Listen Address\n"
		"Address\n"
		CMD_KEY_IPV4_HELP)
{
	int ret = ERROR;
	ret = web_app_address_set_api((argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (no_webserver_address,
		no_webserver_address_cmd,
		"no webserver listen address",
		NO_STR
		"Webserver configure\n"
		"Listen Address\n"
		"Address\n")
{
	int ret = ERROR;
	ret = web_app_address_set_api(NULL);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (webserver_port,
		webserver_port_cmd,
		"webserver listen port <80-65530>",
		"Webserver configure\n"
		"Listen Port\n"
		"Port Configure\n"
		"TCP Port\n")
{
	int ret = ERROR;
	if(web_app_proto_get_api() == WEB_PROTO_HTTP)
		ret = web_app_port_set_api(zpl_false, atoi(argv[0]));
	else
		ret = web_app_port_set_api(zpl_true, atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (no_webserver_port,
		no_webserver_port_cmd,
		"no webserver listen port",
		NO_STR
		"Webserver configure\n"
		"Listen Port\n"
		"Port Configure\n")
{
	int ret = ERROR;
	if(web_app_proto_get_api() == WEB_PROTO_HTTP)
		ret = web_app_port_set_api(zpl_false, 0);
	else
		ret = web_app_port_set_api(zpl_true, 0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (webserver_username,
	   webserver_username_cmd,
		"webserver username USER password PASS roles ROLES",
		"Webserver configure\n"
		"Username configure\n"
		"Username String\n"
		"Password configure\n"
		"Password String\n"
		"Roles configure\n"
		"Roles Name String\n")
{
	int ret = ERROR;
	if(web_app_username_lookup_api(argv[0]) == OK)
	{
		if (vty->type != VTY_FILE)
			vty_out(vty, " Username '%s' is already exist.%s", argv[0], VTY_NEWLINE);
		return CMD_WARNING;
	}
	ret = web_app_username_add_api(argv[0], argv[1], argv[2]);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_webserver_username,
		no_webserver_username_cmd,
		"no webserver username USER",
		NO_STR
		"Webserver configure\n"
		"Username configure\n"
		"Username String\n")
{
	int ret = ERROR;
	if(web_app_username_lookup_api(argv[0]) != OK)
	{
		if (vty->type != VTY_FILE)
			vty_out(vty, " Username '%s' is not exist.%s", argv[0], VTY_NEWLINE);
		return CMD_WARNING;
	}
	ret = web_app_username_del_api(argv[0]);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (webserver_gopass,
		webserver_gopass_cmd,
		"webserver encryption username USER password PASS cipher CIPHER realm REALM",
		NO_STR
		"Webserver configure\n"
		"Encryption Password\n"
		"Username configure\n"
		"Username String\n"
		"Password configure\n"
		"Password String\n"
		"Cipher configure\n"
		"Cipher String\n"
		"Realm configure\n"
		"Realm String\n")
{
	int ret = ERROR;
	char encodedPassword[1024];
	memset(encodedPassword, 0, sizeof(encodedPassword));
	ret = web_app_gopass_api(argv[0], argv[1], argv[2], argv[3], encodedPassword);
	if(ret == OK)
	{
		vty_out(vty, "encoded password:%s%s", encodedPassword, VTY_NEWLINE);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (webserver_gopass_roles,
		webserver_gopass_roles_cmd,
		"webserver roles USER roles .LINE",
		NO_STR
		"Webserver configure\n"
		"Username configure\n"
		"Username String\n"
		"Roles configure\n"
		"Roles String\n")
{
	int ret = ERROR, i = 0;
	char *roles[16] = {NULL};
	for(i = 0; i < 16l; i++)
	{
		if(argv[1+i])
			roles[i] = argv[1+i];
		else
			roles[i] = NULL;
	}
	ret = web_app_gopass_roles_api(argv[0], roles);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (debug_webserver_level,
	   debug_webserver_level_cmd,
		"debug webserver "LOG_LEVELS,
		DEBUG_STR
		"Webserver configure\n"
		LOG_LEVEL_DESC)
{
	int ret = OK, level = 0;
	if(argc == 1)
	{
		if ((level = zlog_priority_match(argv[0])) == ZLOG_DISABLED)
			return CMD_ERR_NO_MATCH;
	}
	web_app_debug_set_api(level);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_debug_webserver_level,
	   no_debug_webserver_level_cmd,
		"no debug webserver",
		NO_STR
		DEBUG_STR
		"Webserver configure\n")
{
	int ret = OK;
	web_app_debug_set_api(WEBS_ERROR);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

static int webserver_debug_write_config(struct vty *vty, void *pVoid)
{
	web_app_debug_write_config(vty);
	return 0;
}

static int webserver_write_config(struct vty *vty, void *pVoid)
{
	web_app_write_config(vty);
	return 0;
}


void cmd_webserver_init(void)
{
	{
		template_t * temp = lib_template_new (zpl_false);
		if(temp)
		{
			temp->module = 1;
			strcpy(temp->name, "webserver");
			strcpy(temp->prompt, "webserver"); /* (config-app-esp)# */
			temp->pVoid = NULL;
			temp->write_template = webserver_write_config;
			temp->show_debug = webserver_debug_write_config;
			lib_template_install(temp, 1);
		}

		install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &webserver_template_cmd);
		install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_webserver_template_cmd);

		install_element(CONFIG_NODE, CMD_ENABLE_LEVEL, &debug_webserver_level_cmd);
		install_element(CONFIG_NODE, CMD_ENABLE_LEVEL, &no_debug_webserver_level_cmd);
		install_element(ENABLE_NODE, CMD_ENABLE_LEVEL, &debug_webserver_level_cmd);
		install_element(ENABLE_NODE, CMD_ENABLE_LEVEL, &no_debug_webserver_level_cmd);
/*
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &webserver_address_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &no_webserver_address_cmd);
*/
#if ME_COM_SSL
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &webserver_https_enable_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &no_webserver_https_enable_cmd);
#endif
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &webserver_address_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &no_webserver_address_cmd);

		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &webserver_port_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &no_webserver_port_cmd);

		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &webserver_username_cmd);
		install_element(TEMPLATE_NODE, CMD_CONFIG_LEVEL, &no_webserver_username_cmd);


		install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &webserver_gopass_cmd);
		install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &webserver_gopass_roles_cmd);
	}
}
