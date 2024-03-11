/**
 * @file      : web_api.h
 * @brief     : Description
 * @author    : zhurish (zhurish@163.com)
 * @version   : 1.0
 * @date      : 2024-02-05
 * 
 * @copyright : Copyright (c) - 2024 zhurish(zhurish@163.com).Co.Ltd. All rights reserved.
 * 
 */
#ifndef __WEB_API_H__
#define __WEB_API_H__

#ifdef __cplusplus
extern "C" {
#endif


#define HAS_BOOL 1
#include "src/goahead.h"

//#define __WEBGUI_DEBUG



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
#define WEBGUI_AUTH 		WEBGUI_SRC_DIR"/web/auth.txt"
#define WEBGUI_DOCUMENTS 	WEBGUI_SRC_DIR"/web/theme/bootstrap"
#else
#define WEBGUI_ROUTE 		REAL_SYSWEBDIR"/route.txt"
#define WEBGUI_AUTH 		REAL_SYSWEBDIR"/auth.txt"
#define WEBGUI_DOCUMENTS 	SYSWWWDIR
#endif

#define WEBGUI_VUEROUTE 	REAL_SYSWEBDIR"/vue-router.json"
#define WEBGUI_VUEMENU 		REAL_SYSWEBDIR"/vue-menu.json"

#define WEB_SYSTEM_LOG 		SYSLOGDIR"/boot.log"

#define WEB_LOGIN_USERNAME 		"root"
#define WEB_LOGIN_PASSWORD 		"admin123456"



//#define WEB_OPENWRT_UCI 		1
#define WEB_OPENWRT_PROCESS 	1//	WEB_OPENWRT_UCI


#define _WEB_DEBUG_ENABLE 1

#if defined(_WEB_DEBUG_ENABLE)
#define _WEB_DBG_ERR(format, ...) 		zlog_err (MODULE_WEB, format, ##__VA_ARGS__)
#define _WEB_DBG_WARN(format, ...) 		zlog_warn (MODULE_WEB, format, ##__VA_ARGS__)
#define _WEB_DBG_INFO(format, ...) 		zlog_info (MODULE_WEB, format, ##__VA_ARGS__)
#define _WEB_DBG_DEBUG(format, ...) 	zlog_debug (MODULE_WEB, format, ##__VA_ARGS__)
#define _WEB_DBG_TRAP(format, ...) 		zlog_trap (MODULE_WEB, format, ##__VA_ARGS__)
#else
#define _WEB_DBG_ERR(format, ...)
#define _WEB_DBG_WARN(format, ...)
#define _WEB_DBG_INFO(format, ...)
#define _WEB_DBG_DEBUG(format, ...)
#define _WEB_DBG_TRAP(format, ...)
#endif

#define _WEB_DEBUG_MSG		0X01
#define _WEB_DEBUG_DETAIL	0X200
#define _WEB_DEBUG_EVENT	0X800
#define _WEB_DEBUG_TRACE	0X400
#define _WEB_DEBUG_RAW		0X200
#define _WEB_DEBUG_HEADER   0x1000      /**< trace HTTP header */


#define WEB_IS_DEBUG(n)		(_WEB_DEBUG_ ## n & _web_app_debug)
#define WEB_DEBUG_ON(n)		{ _web_app_debug |= (_WEB_DEBUG_ ## n ); __websLogLevel |= (_WEB_DEBUG_ ## n );}
#define WEB_DEBUG_OFF(n)	{ _web_app_debug &= ~(_WEB_DEBUG_ ## n ); __websLogLevel &= ~(_WEB_DEBUG_ ## n );}


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
	zpl_taskid_t taskid;
	zpl_bool enable;
	zpl_bool finished;
	zpl_bool init;
	zpl_bool reload;
	zpl_int32 debug_level;

	web_proto proto;
	char *address;
	zpl_uint16 port;
#if ME_COM_SSL
	zpl_uint16 ssl_port;
#endif
	char *endpoints;
	char *documents;
	char *web_route;
	char *web_auth;
	char *cfgBaseDir;

	//页面文件
	char *web_login;
	char *web_main;
	char *web_logout;

	web_type webtype;
	web_os webos;
}web_app_t;


extern web_app_t *web_app;
extern int _web_app_debug;




extern int web_app_module_init(void);
extern int web_app_module_exit(void);
extern int web_app_module_task_init (void);
extern int web_app_module_task_exit (void);


extern int web_app_quit_api(void);
extern int web_app_reload_api(void);

extern int web_app_enable_set_api(zpl_bool enable);
extern int web_app_proto_set_api(web_proto proto);
extern web_proto web_app_proto_get_api(void);
extern int web_app_address_set_api(char *address);
extern int web_app_port_set_api(zpl_bool ssl, zpl_uint16 port);
extern int web_app_debug_set_api(zpl_int32 level);
extern int web_app_debug_get_api(void);

extern int web_app_username_add_api(const char *username, const char *password, const char *roles);
extern int web_app_username_lookup_api(const char *username);
extern int web_app_username_del_api(const char *username);

extern int web_app_gopass_api(const char *username, const char *password,
							   const char *cipher, const char *realm, char *encodedPassword);
extern int web_app_gopass_roles_api(const char *username, const char *roles[]);
extern int web_app_gopass_save_api(const char *username,
		const char *roles[], char *encodedPassword);
extern int web_app_auth_save_api(void);

extern int web_app_debug_write_config(struct vty *vty);
extern int web_app_write_config(struct vty *vty);
extern int cmd_webserver_init(void);
 

extern const char * web_type_string(web_app_t *wp);
extern const char * web_os_type_string(web_app_t *wp);
extern web_type web_type_get(void);
extern web_os web_os_get(void);


extern int webs_username_password_update(void *pwp, char *username, char *password);

#ifdef __cplusplus
}
#endif
 
#endif /* __WEB_API_H__ */
