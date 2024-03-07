/**
 * @file      : web_app.h
 * @brief     : Description
 * @author    : zhurish (zhurish@163.com)
 * @version   : 1.0
 * @date      : 2024-02-05
 * 
 * @copyright : Copyright (c) - 2024 zhurish(zhurish@163.com).Co.Ltd. All rights reserved.
 * 
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
extern int web_html_service_init(void);

extern int web_html_upgrade_init(void);
extern int web_html_system_init(void);
extern int web_html_sysinfo_init(void);

#ifdef ZPL_MQTT_MODULE
extern int web_html_mqtt_init(void);
#endif
#ifdef ZPL_NSM_DHCP
extern int web_html_dhcp_init(void);
#endif /*ZPL_NSM_DHCP*/

extern int web_html_interface_init(void);
extern int web_html_vlan_init(void);

#ifdef __cplusplus
}
#endif
 
#endif /* __WEB_APP_H__ */
