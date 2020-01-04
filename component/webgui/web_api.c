/*
 * web_app.c
 *
 *  Created on: Mar 23, 2019
 *      Author: zhurish
 */
#include "zebra.h"
#include "vty.h"
#include "if.h"

#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"

#include "web_util.h"
#include "web_jst.h"
#include "web_app.h"
#include "web_api.h"


int web_app_quit_api()
{
	zassert(web_app != NULL);
	web_app->finished = TRUE;
	return OK;
}

int web_app_reload_api()
{
	zassert(web_app != NULL);
	web_app->finished = TRUE;
	web_app->reload = TRUE;
	return OK;
}
/****************************************************************************/
int web_app_enable_set_api(BOOL enable)
{
	zassert(web_app != NULL);
	if(web_app->enable == enable)
		return OK;
	if(web_app->enable && !enable)
	{
		web_app->finished = TRUE;
		web_app->enable = FALSE;
		return OK;
	}
	if(!web_app->enable && enable)
	{
		web_app->finished = FALSE;
		web_app->enable = TRUE;
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

web_proto web_app_proto_get_api()
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

int web_app_port_set_api(BOOL ssl, u_int16 port)
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

int web_app_debug_set_api(int level)
{
	websSetDebug(level?1:0);
	websSetLogLevel(level);
	return OK;
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
extern int web_gopass_roles(const char *authFile, const char *username, const char *roles[]);

int web_app_gopass_save_api(const char *username,
		const char *roles[], char *encodedPassword)
{
	return web_gopass_save(WEBGUI_AUTH, username,
			roles, encodedPassword);
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

			vty_out(vty, "!%s",VTY_NEWLINE);
		}
	}
	return OK;
}
