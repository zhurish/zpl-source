/*
 * webgui_app.c
 *
 *  Created on: Mar 23, 2019
 *      Author: zhurish
 */
//#include "zebra.h"
#ifdef HAVE_CONFIG_H
#include "plconfig.h"
#endif /* HAVE_CONFIG_H */

#include "os_platform.h"

#include "module.h"
/*
#include "getopt.h"
#include <log.h>
#include "command.h"
*/
#include <netinet/in.h>
#include "memory.h"
#include "zassert.h"
#include "log.h"
/*
#include "prefix.h"
#include "network.h"
#include "vty.h"
#include "buffer.h"
#include "host.h"
#include "eloop.h"
*/
#include "os_list.h"
#include "os_sem.h"
#include "os_task.h"

#include "goahead.h"
#include "webgui_app.h"


webgui_app_t *webgui_app = NULL;

static int webgui_task(void *argv);

/*MTYPE_WEB
MTYPE_WEB_AUTH
MTYPE_WEB_ROUTE
MTYPE_WEB_DOC
MTYPE_WEB_DATA
MTYPE_WEB_TMP*/
static int webgui_app_init(webgui_app_t *web);
static int webgui_app_exit(webgui_app_t *web);

int webgui_module_init()
{
	webgui_app = XMALLOC(MTYPE_WEB, sizeof(webgui_app_t));
	memset(webgui_app, 0, sizeof(webgui_app_t));
	if(webgui_app->documents)
		XFREE(MTYPE_WEB_DOC, webgui_app->documents);
	webgui_app->documents = XSTRDUP(MTYPE_WEB_DOC, WEBGUI_DOCUMENTS);

	if(webgui_app->endpoints)
		XFREE(MTYPE_WEB_DOC, webgui_app->endpoints);
	webgui_app->endpoints = XSTRDUP(MTYPE_WEB_DOC, WEBGUI_LISTEN);

	if(webgui_app->web_route)
		XFREE(MTYPE_WEB_ROUTE, webgui_app->web_route);
	webgui_app->web_route = XSTRDUP(MTYPE_WEB_ROUTE, WEBGUI_ROUTE);

	if(webgui_app->web_auth)
		XFREE(MTYPE_WEB_AUTH, webgui_app->web_auth);
	webgui_app->web_auth = XSTRDUP(MTYPE_WEB_AUTH, WEBGUI_AUTH);

	return OK;
}

int webgui_module_exit()
{
	if(webgui_app)
	{
		if(webgui_app->address)
			XFREE(MTYPE_WEB_DATA, webgui_app->address);

		if(webgui_app->documents)
			XFREE(MTYPE_WEB_DOC, webgui_app->documents);

		if(webgui_app->web_route)
			XFREE(MTYPE_WEB_ROUTE, webgui_app->web_route);

		if(webgui_app->web_auth)
			XFREE(MTYPE_WEB_AUTH, webgui_app->web_auth);

		XFREE(MTYPE_WEB, webgui_app);
		webgui_app = NULL;
	}
	return OK;
}

int webgui_module_task_init ()
{
	zassert(webgui_app != NULL);
	if(webgui_app->taskid == 0)
		webgui_app->taskid = os_task_create("webTask", OS_TASK_DEFAULT_PRIORITY,
	               0, webgui_task, webgui_app, OS_TASK_DEFAULT_STACK);
	if(webgui_app->taskid)
		return OK;
	return ERROR;
}

int webgui_module_task_exit ()
{
	zassert(webgui_app != NULL);
	webgui_app->quit = TRUE;
	return OK;
}

static int webgui_init(webgui_app_t *web)
{
	zassert(web != NULL);
	websSetDebug(1);
	logSetPath("stdout:2");
	websSetLogLevel(9);
	//logSetPath(argv[++argind]);
    if (websOpen(web->documents, web->web_route) < 0) {
        error("Cannot initialize server. Exiting.");
        return -1;
    }
#if ME_GOAHEAD_AUTH
    if (websLoad(web->web_auth) < 0) {
        error("Cannot load %s", web->web_auth);
        return -1;
    }
#endif
    if (websListen(web->endpoints) < 0) {
        return -1;
    }

    webgui_app_init(web);

    web->enable = TRUE;
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
static int webgui_task(void *argv)
{
	//int ret = 0;
	zassert(argv != NULL);
	webgui_app_t *web = argv;
	while(!os_load_config_done())
	{
		os_sleep(1);
	}
	webgui_init(web);
	while(web->enable)
	{
		if(web->waiting)
		{
			os_sleep(1);
			continue;
		}
	    websServiceEvents(&web->finished);
	    if(web->quit)
	    	break;
	    else
	    {
	    	if(web->waiting)
	    	{
	    		webgui_app_exit(web);
	    		websClose();
	    	}
	    }
	}
	webgui_app_exit(web);
    websClose();
    webgui_module_exit();
	return OK;
}


int webgui_quit_api()
{
	zassert(webgui_app != NULL);
	webgui_app->quit = TRUE;
	webgui_app->finished = TRUE;
	return OK;
}

int webgui_reset_api()
{
	zassert(webgui_app != NULL);
	webgui_app->quit = FALSE;
	webgui_app->finished = TRUE;
	webgui_app->waiting = TRUE;
	return OK;
}

static int webgui_app_init(webgui_app_t *web)
{
	web_jst_html_init();
	web_login_app(NULL);
	web_updownload_app(NULL);
	web_jst_init();
	web_jst_onclick_init();
	web_admin_app(NULL);
	return OK;
}


static int webgui_app_exit(webgui_app_t *web)
{
	return OK;
}
