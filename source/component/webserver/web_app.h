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




extern int web_html_updownload_init(void);
extern int web_html_menu_init(void);
/*
 * admin
 */
extern int web_html_admin_init(void);

#ifdef ZPL_SERVICE_SNTPC
extern int web_html_sntp_init(void);
#endif /*ZPL_SERVICE_SNTPC*/

#ifdef ZPL_SERVICE_SYSLOG
extern int web_html_syslog_init(void);
#endif /*ZPL_SERVICE_SYSLOG*/

extern int web_html_network_init(void);
extern int web_html_netservice_init(void);

#ifdef ZPL_WIFI_MODULE
extern int web_html_wireless_init(void);
#endif


extern int web_html_upgrade_init(void);
extern int web_html_system_init(void);
extern int web_html_sysinfo_init(void);

 
#ifdef __cplusplus
}
#endif
 
#endif /* __WEB_APP_H__ */
