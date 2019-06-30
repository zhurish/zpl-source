/*
 * webgui_app.h
 *
 *  Created on: Mar 23, 2019
 *      Author: zhurish
 */

#ifndef __WEBGUI_APP_H__
#define __WEBGUI_APP_H__

#define T(n)	(n)

#define WEBGUI_LISTEN 		"http://*:8080"
#define WEBGUI_ROUTE 		SYSWEBDIR"/route.txt"
#define WEBGUI_AUTH 		SYSWEBDIR"/auth.txt"
#define WEBGUI_HOME 		SYSWEBDIR
#define WEBGUI_DOCUMENTS 	SYSWWWDIR


typedef struct webgui_app_s
{
	int			taskid;
	BOOL		enable;
	BOOL		finished;
	BOOL		quit;
	BOOL		waiting;
	u_int8		debug_level;
	char		*address;
	u_int16		port;
	char		*endpoints;
	char		*documents;
    char 		*web_route;
    char 		*web_auth;

	char		*menu1;
    char 		*menu2;
    char 		*menu3;
	char		*tab_menu1;
    char 		*tab_menu2;
}webgui_app_t;


extern webgui_app_t *webgui_app;

extern int webgui_module_init();
extern int webgui_module_exit();
extern int webgui_module_task_init ();
extern int webgui_module_task_exit ();

extern int webgui_quit_api();
extern int webgui_reset_api();

extern int web_login_app(char *actionname);
extern int web_updownload_app(char *actionname);
extern int web_jst_init(void);
extern int web_jst_html_init(void);
extern int web_jst_onclick_init(void);

extern char * web_encoded_password(char *username, char *password, char *realm, char *cipher);


/*
 * admin
 */
extern int web_admin_app(char *actionname);

#endif /* __WEBGUI_APP_H__ */
