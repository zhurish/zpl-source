/*
 * web_app.c
 *
 *  Created on: Mar 23, 2019
 *      Author: zhurish
 */
#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"

#include "web_util.h"
#include "web_jst.h"
#include "web_app.h"
#include "web_api.h"


struct module_list module_list_webserver = 
{ 
	.module=MODULE_WEB, 
	.name="WEB", 
	.module_init=web_app_module_init, 
	.module_exit=web_app_module_exit, 
	.module_task_init=web_app_module_task_init, 
	.module_task_exit=web_app_module_task_exit, 
	.module_cmd_init=cmd_webserver_init, 
	.module_write_config=NULL, 
	.module_show_config=NULL,
	.module_show_debug=NULL, 
	.flags = ZPL_MODULE_NEED_INIT,
	.taskid=0,
};


int web_app_quit_api(void)
{
	zassert(web_app != NULL);
	web_app->finished = zpl_true;
	return OK;
}

int web_app_reload_api(void)
{
	zassert(web_app != NULL);
	web_app->finished = zpl_true;
	web_app->reload = zpl_true;
	return OK;
}
/****************************************************************************/
int web_app_enable_set_api(zpl_bool enable)
{
	zassert(web_app != NULL);
	if(web_app->enable == enable)
		return OK;
	if(web_app->enable && !enable)
	{
		web_app->finished = zpl_true;
		web_app->enable = zpl_false;
		return OK;
	}
	if(!web_app->enable && enable)
	{
		web_app->finished = zpl_false;
		web_app->enable = zpl_true;
		return OK;
	}
	return ERROR;
}

int web_app_proto_set_api(web_proto proto)
{
	if(web_app->proto == proto)
		return OK;
	web_app->proto = proto;

	if (web_app->proto == WEB_PROTO_HTTP)
	{
		if (web_app->address && web_app->port)
		{
			char tmp[128];
			memset(tmp, 0, sizeof(tmp));
			snprintf(tmp, sizeof(tmp), "http://%s:%d", web_app->address,
					web_app->port);
			web_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, tmp);
		}
		else if (web_app->address && !web_app->port)
		{
			char tmp[128];
			memset(tmp, 0, sizeof(tmp));
			snprintf(tmp, sizeof(tmp), "http://%s:%d", web_app->address,
					WEB_LISTEN_PORT);
			web_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, tmp);
		}
		else if (!web_app->address && web_app->port)
		{
			char tmp[128];
			memset(tmp, 0, sizeof(tmp));
			snprintf(tmp, sizeof(tmp), "http://*:%d", web_app->port);
			web_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, tmp);
		}
		else
			web_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, WEBGUI_LISTEN);
	}
#if ME_COM_SSL
	else if (web_app->proto == WEB_PROTO_HTTPS)
	{
		if (web_app->address && web_app->ssl_port)
		{
			char tmp[128];
			memset(tmp, 0, sizeof(tmp));
			snprintf(tmp, sizeof(tmp), "https://%s:%d", web_app->address,
					web_app->ssl_port);
			web_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, tmp);
		}
		else if (web_app->address && !web_app->ssl_port)
		{
			char tmp[128];
			memset(tmp, 0, sizeof(tmp));
			snprintf(tmp, sizeof(tmp), "https://%s:%d", web_app->address,
					WEB_LISTEN_SSL_PORT);
			web_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, tmp);
		}
		else if (!web_app->address && web_app->ssl_port)
		{
			char tmp[128];
			memset(tmp, 0, sizeof(tmp));
			snprintf(tmp, sizeof(tmp), "https://*:%d", web_app->ssl_port);
			web_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, tmp);
		}
		else
			web_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, WEBGUI_SSL_LISTEN);
	}
#endif
	return OK;
}

web_proto web_app_proto_get_api(void)
{
	if(web_app)
		return web_app->proto;
	return WEB_PROTO_HTTP;
}

int web_app_address_set_api(char *address)
{
	zassert(web_app != NULL);

	if (web_app->address)
	{
		XFREE(MTYPE_WEB_DOC, web_app->address);
		web_app->address = NULL;
	}
	if (web_app->endpoints)
	{
		XFREE(MTYPE_WEB_DOC, web_app->endpoints);
		web_app->endpoints = NULL;
	}
	if(address)
		web_app->address = XSTRDUP(MTYPE_WEB_DOC, address);

	if (web_app->proto == WEB_PROTO_HTTP)
	{
		if (web_app->address && web_app->port)
		{
			char tmp[128];
			memset(tmp, 0, sizeof(tmp));
			snprintf(tmp, sizeof(tmp), "http://%s:%d", web_app->address,
					web_app->port);
			web_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, tmp);
		}
		else if (web_app->address && !web_app->port)
		{
			char tmp[128];
			memset(tmp, 0, sizeof(tmp));
			snprintf(tmp, sizeof(tmp), "http://%s:%d", web_app->address,
					WEB_LISTEN_PORT);
			web_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, tmp);
		}
		else if (!web_app->address && web_app->port)
		{
			char tmp[128];
			memset(tmp, 0, sizeof(tmp));
			snprintf(tmp, sizeof(tmp), "http://*:%d", web_app->port);
			web_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, tmp);
		}
		else
			web_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, WEBGUI_LISTEN);
	}
#if ME_COM_SSL
	else if (web_app->proto == WEB_PROTO_HTTPS)
	{
		if (web_app->address && web_app->ssl_port)
		{
			char tmp[128];
			memset(tmp, 0, sizeof(tmp));
			snprintf(tmp, sizeof(tmp), "https://%s:%d", web_app->address,
					web_app->ssl_port);
			web_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, tmp);
		}
		else if (web_app->address && !web_app->ssl_port)
		{
			char tmp[128];
			memset(tmp, 0, sizeof(tmp));
			snprintf(tmp, sizeof(tmp), "https://%s:%d", web_app->address,
					WEB_LISTEN_SSL_PORT);
			web_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, tmp);
		}
		else if (!web_app->address && web_app->ssl_port)
		{
			char tmp[128];
			memset(tmp, 0, sizeof(tmp));
			snprintf(tmp, sizeof(tmp), "https://*:%d", web_app->ssl_port);
			web_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, tmp);
		}
		else
			web_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, WEBGUI_SSL_LISTEN);
	}
#endif
	return web_app_reload_api();
}

int web_app_port_set_api(zpl_bool ssl, zpl_uint16 port)
{
	zassert(web_app != NULL);

	if (web_app->endpoints)
	{
		XFREE(MTYPE_WEB_DOC, web_app->endpoints);
		web_app->endpoints = NULL;
	}
#if ME_COM_SSL
	if(ssl)
		web_app->ssl_port = port ? port:WEB_LISTEN_SSL_PORT;
	else
#endif
		web_app->port = port ? port:WEB_LISTEN_PORT;

	if (web_app->proto == WEB_PROTO_HTTP)
	{
		if (web_app->address && web_app->port)
		{
			char tmp[128];
			memset(tmp, 0, sizeof(tmp));
			snprintf(tmp, sizeof(tmp), "http://%s:%d", web_app->address,
					web_app->port);
			web_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, tmp);
		}
		else if (web_app->address && !web_app->port)
		{
			char tmp[128];
			memset(tmp, 0, sizeof(tmp));
			snprintf(tmp, sizeof(tmp), "http://%s:%d", web_app->address,
					WEB_LISTEN_PORT);
			web_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, tmp);
		}
		else if (!web_app->address && web_app->port)
		{
			char tmp[128];
			memset(tmp, 0, sizeof(tmp));
			snprintf(tmp, sizeof(tmp), "http://*:%d", web_app->port);
			web_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, tmp);
		}
		else
			web_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, WEBGUI_LISTEN);
	}
#if ME_COM_SSL
	else if (web_app->proto == WEB_PROTO_HTTPS)
	{
		if (web_app->address && web_app->ssl_port)
		{
			char tmp[128];
			memset(tmp, 0, sizeof(tmp));
			snprintf(tmp, sizeof(tmp), "https://%s:%d", web_app->address,
					web_app->ssl_port);
			web_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, tmp);
		}
		else if (web_app->address && !web_app->ssl_port)
		{
			char tmp[128];
			memset(tmp, 0, sizeof(tmp));
			snprintf(tmp, sizeof(tmp), "https://%s:%d", web_app->address,
					WEB_LISTEN_SSL_PORT);
			web_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, tmp);
		}
		else if (!web_app->address && web_app->ssl_port)
		{
			char tmp[128];
			memset(tmp, 0, sizeof(tmp));
			snprintf(tmp, sizeof(tmp), "https://*:%d", web_app->ssl_port);
			web_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, tmp);
		}
		else
			web_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, WEBGUI_SSL_LISTEN);
	}
#endif
	return web_app_reload_api();
}

int web_app_debug_set_api(zpl_int32 level)
{
	websSetDebug(level?1:0);
//#if ME_GOAHEAD_LOGGING
	websSetLogLevel(level);
//#endif /* ME_GOAHEAD_LOGGING */
	return OK;
}

int web_app_debug_get_api(void)
{
	return websGetLogLevel();
}
int web_app_gopass_api(const char *username, const char *password,
					   const char *cipher, const char *realm, char *encodedPassword)
{
	//int web_gopass(const char *username, const char *password, const char *cipher,
	//		const char *realm,  char *encodedPassword)
	return web_gopass(username, password, cipher,
			realm, encodedPassword);
}

int web_app_gopass_roles_api(const char *username, const char *roles[])
{
	return web_gopass_roles(WEBGUI_AUTH, username, roles);
}
//extern int web_gopass_roles(const char *authFile, const char *username, const char *roles[]);

int web_app_gopass_save_api(const char *username,
		const char *roles[], char *encodedPassword)
{
	return web_gopass_save(WEBGUI_AUTH, username,
			roles, encodedPassword);
}

int web_app_auth_save_api(void)
{
	return web_auth_save(WEBGUI_AUTH);
}


int web_app_username_add_api(const char *username, const char *password, const char *roles)
{
	//webserver encryption username root password admintsl123456! cipher md5 realm goahead.com
	char encodedPassword[1024];
	memset(encodedPassword, 0, sizeof(encodedPassword));
	if(web_app_gopass_api(username, password,
						   "md5", "goahead.com", encodedPassword) == OK)
	{
		if(websAddUser(username, encodedPassword, roles))
			return OK;
	}
	return ERROR;
}

int web_app_username_lookup_api(const char *username)
{
	if(websLookupUser(username))
		return OK;
	return ERROR;
}

int web_app_username_del_api(const char *username)
{
	if(websRemoveUser(username) == 0)
		return OK;
	return ERROR;
}
/*

WebsUser *websAddUser(cchar *username, cchar *password, cchar *proles)
PUBLIC int websRemoveUser(cchar *username)
WebsRole *websAddRole(cchar *name, WebsHash abilities)
PUBLIC int websRemoveRole(cchar *name)
*/


static int web_app_username_tbl(struct vty *vty)
{
	WebsKey     *kp = NULL, *ap = NULL;
	WebsRole    *role = NULL;
	WebsUser    *user = NULL;
	WebsHash    roles, users;

	roles = websGetRoles();
	if (roles >= 0 && vty->type != VTY_FILE) {
		for (kp = hashFirst(roles); kp; kp = hashNext(roles, kp)) {
			role = kp->content.value.symbol;
			vty_out(vty, " webserver rolename %s abilities ", kp->name.value.string);
			for (ap = hashFirst(role->abilities); ap; ap = hashNext(role->abilities, ap)) {
				vty_out(vty, "%s,",ap->name.value.string);
			}
			vty_out(vty, "%s",VTY_NEWLINE);
		}
	}
	//webserver username USER password PASS roles ROLES
	users = websGetUsers();
	if (users >= 0) {
		for (kp = hashFirst(users); kp; kp = hashNext(users, kp)) {
			user = kp->content.value.symbol;
			vty_out(vty, " webserver username %s password %s roles %s%s", user->name, user->password, user->roles, VTY_NEWLINE);
		}
	}
	return 0;
}


int web_app_debug_write_config(struct vty *vty)
{
	if(websGetDebug())
	{
		switch(web_app_debug_get_api())
		{
			case WEBS_CRIT:
			case WEBS_ALERT:
			case WEBS_EMERG:
				vty_out(vty, "debug webserver alerts%s",VTY_NEWLINE);
				break;
			case WEBS_ERROR:
				vty_out(vty, "debug webserver errors%s",VTY_NEWLINE);
				break;
			case WEBS_WARN:
				vty_out(vty, "debug webserver warnings%s",VTY_NEWLINE);
				break;
			case WEBS_INFO:
				vty_out(vty, "debug webserver informational%s",VTY_NEWLINE);
				break;
			case WEBS_NOTICE:
				vty_out(vty, "debug webserver notifications%s",VTY_NEWLINE);
				break;
			case WEBS_DEBUG:
				vty_out(vty, "debug webserver debugging%s",VTY_NEWLINE);
				break;
			case WEBS_TRAP:
				vty_out(vty, "debug webserver trapping%s",VTY_NEWLINE);
				break;
			case WEBS_CONFIG:
				vty_out(vty, "debug webserver config%s",VTY_NEWLINE);
				break;
			case WEBS_VERBOSE:
				vty_out(vty, "debug webserver verbose%s",VTY_NEWLINE);
				break;
			default:
				break;
		}
	}
	return 0;
}

int web_app_write_config(struct vty *vty)
{
	if(web_app)
	{
		if(web_app->enable)
		{
			vty_out(vty, "template webserver%s",VTY_NEWLINE);
			//vty_out(vty, "service web%s", VTY_NEWLINE);
#if ME_COM_SSL
			if(web_app->proto == WEB_PROTO_HTTPS)
				vty_out(vty, " webserver https enable%s", VTY_NEWLINE);
#endif
			if(web_app->address)
				vty_out(vty, " webserver listen address %s%s", web_app->address, VTY_NEWLINE);

#if ME_COM_SSL
			if(web_app->proto == WEB_PROTO_HTTPS && web_app->ssl_port != WEB_LISTEN_SSL_PORT)
				vty_out(vty, " webserver listen port %d%s", web_app->ssl_port, VTY_NEWLINE);
#endif
			if(web_app->proto == WEB_PROTO_HTTP && web_app->port != WEB_LISTEN_PORT)
				vty_out(vty, " webserver listen port %d%s", web_app->port, VTY_NEWLINE);

			web_app_username_tbl(vty);

			vty_out(vty, "!%s",VTY_NEWLINE);
		}
	}
	return OK;
}
