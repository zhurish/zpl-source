/*
 * web_app.h
 *
 *  Created on: 2019年8月9日
 *      Author: DELL
 */

#ifndef __WEB_APP_H__
#define __WEB_APP_H__


#ifdef __cplusplus
extern "C" {
#endif

struct web_menu_table
{
  const char *path;
  const char *name;
  const char *component;
  int meta;
  struct web_menu_table *children;
};

extern int web_login_app(void);
extern int web_updownload_app(void);
extern int web_menu_app(void);
/*
 * admin
 */
extern int web_admin_app(void);

#ifdef ZPL_SERVICE_SNTPC
extern int web_sntp_app(void);
#endif /*ZPL_SERVICE_SNTPC*/

#ifdef ZPL_SERVICE_SYSLOG
extern int web_syslog_app(void);
#endif /*ZPL_SERVICE_SYSLOG*/

extern int web_network_app(void);
extern int web_netservice_app(void);

#ifdef ZPL_WIFI_MODULE
extern int web_wireless_app(void);
#endif

extern int web_sip_app(void);


extern int web_upgrade_app(void);
extern int web_system_app(void);

 
#ifdef __cplusplus
}
#endif
 
#endif /* __WEB_APP_H__ */
