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


extern int web_login_app(void);
extern int web_updownload_app(void);
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

#ifdef APP_X5BA_MODULE
#ifdef ZPL_PJSIP_MODULE

#endif /*ZPL_PJSIP_MODULE*/
extern int web_factory_app(void);
extern int web_card_app(void);
extern int web_switch_app(void);
#endif

#ifdef APP_V9_MODULE
extern int web_boardcard_app(void);
extern int web_general_app(void);
extern int web_rtsp_app(void);
extern int web_algorithm_app(void);
extern int web_facelib_app(void);
extern int web_db_app(void);
#endif

extern int web_upgrade_app(void);
extern int web_system_app(void);

 
#ifdef __cplusplus
}
#endif
 
#endif /* __WEB_APP_H__ */
