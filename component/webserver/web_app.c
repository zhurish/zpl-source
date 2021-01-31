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
web_app_start (web_app_t *web);

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
	if(WEBGUI_ROUTE)
		web_app->web_route = XSTRDUP(MTYPE_WEB_ROUTE, WEBGUI_ROUTE);

	if (web_app->web_auth)
		XFREE(MTYPE_WEB_AUTH, web_app->web_auth);
	if(WEBGUI_AUTH)
		web_app->web_auth = XSTRDUP(MTYPE_WEB_AUTH, WEBGUI_AUTH);
#if ME_GOAHEAD_LOGIN_HTML
	if (web_app->web_login)
		XFREE(MTYPE_WEB_DOC, web_app->web_login);
	if(WEB_LOGIN_HTML)
		web_app->web_login = XSTRDUP(MTYPE_WEB_DOC, WEB_LOGIN_HTML);

	if (web_app->web_logout)
		XFREE(MTYPE_WEB_DOC, web_app->web_logout);
	if(WEB_LOGOUT_HTML)
		web_app->web_logout = XSTRDUP(MTYPE_WEB_DOC, WEB_LOGOUT_HTML);

	if (web_app->web_main)
		XFREE(MTYPE_WEB_DOC, web_app->web_main);
	if(WEB_MAIN_HTML)
		web_app->web_main = XSTRDUP(MTYPE_WEB_DOC, WEB_MAIN_HTML);
#endif
	WEB_DEBUG_ON(MSG);
	WEB_DEBUG_ON(DETAIL);
	WEB_DEBUG_ON(EVENT);
	_web_app_debug = _WEB_DEBUG_MSG|_WEB_DEBUG_DETAIL|_WEB_DEBUG_EVENT;

	web_app_start (web_app);
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
#if ME_GOAHEAD_LOGIN_HTML
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
#endif
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
#if ME_GOAHEAD_LOGIN_HTML
	websSetAutoLoginHtml (0, web->web_login);
	websSetAutoLoginHtml (1, web->web_main);
	websSetAutoLoginHtml (2, web->web_logout);
#endif
	if (websOpen (web->documents, web->web_route) < 0)
	{
		zlog_err (MODULE_WEB, "Cannot initialize server. Exiting.");
		return -1;
	}
#if ME_GOAHEAD_AUTH
	if (websLoad (web->web_auth) < 0)
	{
		zlog_err (MODULE_WEB,"Cannot load %s", web->web_auth);
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

#if !ME_GOAHEAD_EXTLOG
static void webLogDefaultHandler(int flags, cchar *buf)
{
/*    char    prefix[ME_GOAHEAD_LIMIT_STRING];

    if (logFd >= 0) {
        if (flags & WEBS_RAW_MSG) {
            write(logFd, buf, (int) slen(buf));
        } else {
            fmt(prefix, sizeof(prefix), "%s: %d: ", ME_NAME, flags & WEBS_LEVEL_MASK);
            write(logFd, prefix, (int) slen(prefix));
            write(logFd, buf, (int) slen(buf));
            write(logFd, "\n", 1);
#if ME_WIN_LIKE || ME_UNIX_LIKE
            if (flags & WEBS_ERROR_MSG && websGetBackground()) {
                syslog(LOG_ERR, "%s", buf);
            }
#endif
        }
    }*/

	if(flags & WEBS_ASSERT_MSG)
	{
		//extern void pl_zlog (__FILE__, __FUNCTION__, __LINE__, MODULE_WEB, (flags & WEBS_LEVEL_MASK), "%s", buf);
		zlog_err(MODULE_WEB, "%s", buf);
		return;
	}
	if(flags & WEBS_ERROR_MSG)
	{
		zlog_err(MODULE_WEB, "%s", buf);
		return;
	}
	if(flags & WEBS_TRACE_MSG)
	{
		zlog_trap(MODULE_WEB, "%s", buf);
		return;
	}
	switch(flags & WEBS_LEVEL_MASK)
	{
		case LOG_ERR:
			zlog_err(MODULE_WEB, "%s", buf);
			break;
		case LOG_WARNING:
			zlog_warn(MODULE_WEB, "%s", buf);
			break;
		case LOG_INFO:
			zlog_info(MODULE_WEB, "%s", buf);
			break;
		case LOG_NOTICE:
			zlog_notice(MODULE_WEB, "%s", buf);
			break;
		case LOG_DEBUG:
			zlog_debug(MODULE_WEB, "%s", buf);
			break;
		case LOG_TRAP:
			zlog_trap(MODULE_WEB, "%s", buf);
			break;
		default:
			break;
	}
}
#endif /* ME_GOAHEAD_EXTLOG */

static int
web_app_start (web_app_t *web)
{
	zassert(web != NULL);
	websSetDebug (1);
#if ME_GOAHEAD_LOGFILE
	logSetPath ("stdout:2");
#endif /* ME_GOAHEAD_EXTLOG */
	websSetLogLevel (WEBS_TRAP);
	websAccessControlSet(ACCESS_CONTROL_ALLOW_ORIGIN);
	websAccessControlSet(ACCESS_CONTROL_ALLOW_METHODS);
	websAccessControlSet(ACCESS_CONTROL_ALLOW_HEADERS);
#if !ME_GOAHEAD_EXTLOG
	logSetHandler(webLogDefaultHandler);
#endif /* ME_GOAHEAD_EXTLOG */
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
	//web_app_start (web);
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
	web_button_init ();

	web_updownload_cb_init ();

	web_system_jst_init ();
	web_html_jst_init ();
	web_arp_jst_init ();
#ifdef PL_DHCP_MODULE
	web_dhcp_jst_init ();
#endif
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
#ifdef PL_SERVICE_SNTPC
	web_sntp_app ();
#endif
#ifdef PL_SERVICE_SYSLOG
	web_syslog_app ();
#endif

	web_network_app ();
	web_netservice_app();

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
	web_db_app();
#endif
	web_upgrade_app();
	web_system_app ();

	return OK;
}

/*

 static int web_app_html_exit(web_app_t *web)
 {
 return OK;
 }
 */

