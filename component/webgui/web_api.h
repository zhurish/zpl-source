/*
 * web_app.h
 *
 *  Created on: Mar 23, 2019
 *      Author: zhurish
 */

#ifndef __WEB_API_H__
#define __WEB_API_H__

//#define __WEBGUI_DEBUG

#define T(n)	(n)

#define WEBGUI_LISTEN 		"http://*:8080"
#if ME_COM_SSL
#define WEBGUI_SSL_LISTEN 		"https://*:443"
#endif
#define WEB_LISTEN_PORT 		 	8080
#if ME_COM_SSL
#define WEB_LISTEN_SSL_PORT 		443
#endif

#ifdef __WEBGUI_DEBUG
#define WEBGUI_ROUTE 		WEBGUI_SRC_DIR"/web/route.txt"
#define WEBGUI_AUTH 			WEBGUI_SRC_DIR"/web/auth.txt"
#define WEBGUI_HOME 			SYSWEBDIR
#define WEBGUI_DOCUMENTS 	WEBGUI_SRC_DIR"/web/theme/bootstrap"
#else
#define WEBGUI_ROUTE 		SYSWEBDIR"/route.txt"
#define WEBGUI_AUTH 			SYSWEBDIR"/auth.txt"
#define WEBGUI_HOME 			SYSWEBDIR
#define WEBGUI_DOCUMENTS 	SYSWWWDIR
#endif

#define WEB_LOGIN_HTML 		"/html/login.html"
#define WEB_LOGOUT_HTML 	"/html/login.html"
#define WEB_MAIN_HTML 		"/html/bootstrap.html"



#define WEB_SYSTEM_LOG 		"/var/log/boot.log"

//#define WEB_OPENWRT_UCI 		1
#define WEB_OPENWRT_PROCESS 	1//	WEB_OPENWRT_UCI


typedef enum
{
	WEB_TYPE_HOME_WIFI,
	WEB_TYPE_HOME_SWITCH,
	WEB_TYPE_HOME_ROUTE,
}web_type;

typedef enum
{
	WEB_OS_OPENWRT,
	WEB_OS_LINUX,
}web_os;

typedef enum
{
	WEB_PROTO_HTTP,
#if ME_COM_SSL
	WEB_PROTO_HTTPS,
#endif
}web_proto;

typedef struct web_app_s
{
	int taskid;
	BOOL enable;
	BOOL finished;
	BOOL init;
	BOOL reload;
	u_int8 debug_level;

	web_proto proto;
	char *address;
	u_int16 port;
#if ME_COM_SSL
	u_int16 ssl_port;
#endif
	char *endpoints;
	char *documents;
	char *web_route;
	char *web_auth;

	//页面文件
	char *web_login;
	char *web_main;
	char *web_logout;

	web_type webtype;
	web_os webos;
}web_app_t;


extern web_app_t *web_app;

extern int web_app_module_init();
extern int web_app_module_exit();
extern int web_app_module_task_init ();
extern int web_app_module_task_exit ();


extern int web_app_quit_api();
extern int web_app_reload_api();

extern int web_app_enable_set_api(BOOL enable);
extern int web_app_proto_set_api(web_proto proto);
extern web_proto web_app_proto_get_api();
extern int web_app_address_set_api(char *address);
extern int web_app_port_set_api(BOOL ssl, u_int16 port);
extern int web_app_debug_set_api(int level);

extern int web_app_gopass_api(const char *username, const char *password,
							   const char *cipher, const char *realm, char *encodedPassword);
extern int web_app_gopass_roles_api(const char *username, const char *roles[]);
extern int web_app_gopass_save_api(const char *username,
		const char *roles[], char *encodedPassword);

extern int web_app_write_config(struct vty *vty);
extern void cmd_webserver_init(void);

#endif /* __WEB_API_H__ */
