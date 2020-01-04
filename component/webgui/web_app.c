/*
 * web_app.c
 *
 *  Created on: 2019年10月23日
 *      Author: DELL
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

web_app_t *web_app = NULL;

static int
web_app_task (void *argv);

static int
web_app_html_init (web_app_t *web);
//static int web_app_html_exit(web_app_t *web);

int
web_app_module_init ()
{
	web_app = XMALLOC(MTYPE_WEB, sizeof(web_app_t));
	memset (web_app, 0, sizeof(web_app_t));

	web_app->webtype = WEB_TYPE_HOME_WIFI;
	web_app->webos = WEB_OS_OPENWRT;

#if ME_COM_SSL
	web_app->proto = WEB_PROTO_HTTPS;
	web_app->ssl_port = WEB_LISTEN_SSL_PORT;
#else
	web_app->proto = WEB_PROTO_HTTP;
	web_app->port = WEB_LISTEN_PORT;
#endif

	if (web_app->documents)
		XFREE(MTYPE_WEB_DOC, web_app->documents);
	web_app->documents = XSTRDUP(MTYPE_WEB_DOC, WEBGUI_DOCUMENTS);

	if (web_app->endpoints)
		XFREE(MTYPE_WEB_DOC, web_app->endpoints);

	if (web_app->proto == WEB_PROTO_HTTP)
	{
		if (web_app->address && web_app->port)
		{
			char tmp[128];
			memset (tmp, 0, sizeof(tmp));
			snprintf (tmp, sizeof(tmp), "http://%s:%d", web_app->address,
					  web_app->port);
			web_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, tmp);
		}
		else if (web_app->address && !web_app->port)
		{
			char tmp[128];
			memset (tmp, 0, sizeof(tmp));
			snprintf (tmp, sizeof(tmp), "http://%s:%d", web_app->address,
			WEB_LISTEN_PORT);
			web_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, tmp);
		}
		else if (!web_app->address && web_app->port)
		{
			char tmp[128];
			memset (tmp, 0, sizeof(tmp));
			snprintf (tmp, sizeof(tmp), "http://*:%d", web_app->port);
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
			memset (tmp, 0, sizeof(tmp));
			snprintf (tmp, sizeof(tmp), "https://%s:%d", web_app->address,
					  web_app->ssl_port);
			web_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, tmp);
		}
		else if (web_app->address && !web_app->ssl_port)
		{
			char tmp[128];
			memset (tmp, 0, sizeof(tmp));
			snprintf (tmp, sizeof(tmp), "https://%s:%d", web_app->address,
			WEB_LISTEN_SSL_PORT);
			web_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, tmp);
		}
		else if (!web_app->address && web_app->ssl_port)
		{
			char tmp[128];
			memset (tmp, 0, sizeof(tmp));
			snprintf (tmp, sizeof(tmp), "https://*:%d", web_app->ssl_port);
			web_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, tmp);
		}
		else
			web_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, WEBGUI_SSL_LISTEN);
	}
#endif

	if (web_app->web_route)
		XFREE(MTYPE_WEB_ROUTE, web_app->web_route);
	web_app->web_route = XSTRDUP(MTYPE_WEB_ROUTE, WEBGUI_ROUTE);

	if (web_app->web_auth)
		XFREE(MTYPE_WEB_AUTH, web_app->web_auth);
	web_app->web_auth = XSTRDUP(MTYPE_WEB_AUTH, WEBGUI_AUTH);

	if (web_app->web_login)
		XFREE(MTYPE_WEB_DOC, web_app->web_login);
	web_app->web_login = XSTRDUP(MTYPE_WEB_DOC, WEB_LOGIN_HTML);

	if (web_app->web_logout)
		XFREE(MTYPE_WEB_DOC, web_app->web_logout);
	web_app->web_logout = XSTRDUP(MTYPE_WEB_DOC, WEB_LOGOUT_HTML);

	if (web_app->web_main)
		XFREE(MTYPE_WEB_DOC, web_app->web_main);
	web_app->web_main = XSTRDUP(MTYPE_WEB_DOC, WEB_MAIN_HTML);

	return OK;
}

int
web_app_module_exit ()
{
	if (web_app)
	{
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
		if (web_app->documents)
		{
			XFREE(MTYPE_WEB_DOC, web_app->documents);
			web_app->documents = NULL;
		}
		if (web_app->web_route)
		{
			XFREE(MTYPE_WEB_ROUTE, web_app->web_route);
			web_app->web_route = NULL;
		}
		if (web_app->web_auth)
		{
			XFREE(MTYPE_WEB_AUTH, web_app->web_auth);
			web_app->web_auth = NULL;
		}
		if (web_app->web_login)
		{
			XFREE(MTYPE_WEB_DOC, web_app->web_login);
			web_app->web_login = NULL;
		}
		if (web_app->web_logout)
		{
			XFREE(MTYPE_WEB_DOC, web_app->web_logout);
			web_app->web_logout = NULL;
		}
		if (web_app->web_main)
		{
			XFREE(MTYPE_WEB_DOC, web_app->web_main);
			web_app->web_main = NULL;
		}
		XFREE(MTYPE_WEB, web_app);
		web_app = NULL;
	}
	return OK;
}

int
web_app_module_task_init ()
{
	zassert(web_app != NULL);
	if (web_app->taskid == 0)
		web_app->taskid = os_task_create("webTask", OS_TASK_DEFAULT_PRIORITY, 0,
										 web_app_task, web_app,
										 OS_TASK_DEFAULT_STACK);
	if (web_app->taskid)
		return OK;
	return ERROR;
}

int
web_app_module_task_exit ()
{
	zassert(web_app != NULL);
	web_app->finished = TRUE;
	return OK;
}

static int
web_app_init (web_app_t *web)
{
	websUploadSetDir (WEB_UPLOAD_BASE);
#if ME_GOAHEAD_AUTO_LOGIN
	websSetAutoLoginHtml (0, web->web_login);
	websSetAutoLoginHtml (1, web->web_main);
	websSetAutoLoginHtml (2, web->web_logout);
#endif
	if (websOpen (web->documents, web->web_route) < 0)
	{
		error ("Cannot initialize server. Exiting.");
		return -1;
	}
#if ME_GOAHEAD_AUTH
	if (websLoad (web->web_auth) < 0)
	{
		error ("Cannot load %s", web->web_auth);
		return -1;
	}
#endif
	if (websListen (web->endpoints) < 0)
	{
		return -1;
	}
	web->init = TRUE;
	return 0;
}

static int
web_app_start (web_app_t *web)
{
	zassert(web != NULL);
	websSetDebug (1);
	logSetPath ("stdout:2");

	websSetLogLevel (9);

	web_app_init (web);

	web_app_html_init (web);
	web->init = TRUE;
	return OK;
}

static int
web_app_exit (web_app_t *web)
{
	if (web->enable && web->init)
	{
		websClose ();
		web->enable = FALSE;
		web->finished = FALSE;
		web->init = FALSE;
		web->reload = FALSE;
	}
	return OK;
}
/*
 * sudo goahead -v --home /etc/goahead /var/www/goahead
 * home:/etc/goahead
 * auth:auth.txt
 * route:route.txt
 * documents:/var/www/goahead
 * home:/etc/goahead
 *
 */
static int
web_app_task (void *argv)
{
	zassert(argv != NULL);
	web_app_t *web = argv;
	while (!os_load_config_done ())
	{
		os_sleep (1);
	}
	web->enable = TRUE;
	while (!web->enable)
	{
		os_sleep (1);
	}
	web_app_start (web);
	while (1)
	{
		if (!web->enable)
		{
			os_sleep (1);
			continue;
		}
		if (web->init == FALSE)
		{
			web_app_init (web);
		}
		websServiceEvents (&web->finished);
		if (web->reload)
		{
			web_app_exit (web);
			os_sleep (1);
			web_app_init (web);
		}
		else
		{
			if (web->enable)
				break;
		}
	}
	web_app_exit (web);
	return OK;
}

static int
web_app_html_init (web_app_t *web)
{
	web_button_cb_init ();
	web_updownload_cb_init ();

	web_button_jst_init ();
	web_system_jst_init ();
	web_html_jst_init ();
	web_arp_jst_init ();
	web_dhcp_jst_init ();
	web_dns_jst_init ();
	web_dos_jst_init ();
	web_firewall_jst_init ();
	web_interface_jst_init ();
	web_mac_jst_init ();
	web_port_jst_init ();
	web_ppp_jst_init ();
	web_qos_jst_init ();
	web_route_jst_init ();
	web_serial_jst_init ();
	web_tunnel_jst_init ();
	web_vlan_jst_init ();

	web_login_app ();
	web_updownload_app ();
	web_admin_app ();
	web_sntp_app ();
	web_syslog_app ();
	web_network_app ();
#ifdef PL_WIFI_MODULE
	web_wireless_app ();
#endif

#ifdef APP_X5BA_MODULE
	web_switch_app ();
	web_sip_app ();
	web_factory_app ();
	web_card_app ();
#endif

#ifdef APP_V9_MODULE
	web_boardcard_app();
	web_general_app();
	web_rtsp_app();
	web_algorithm_app();
	web_facelib_app();
#endif

	web_system_app ();

	return OK;
}

/*

 static int web_app_html_exit(web_app_t *web)
 {
 return OK;
 }
 */

