/*
 * web_jst.h
 *
 *  Created on: 2019年8月3日
 *      Author: zhurish
 */

#ifndef __APP_WEB_JST_H__
#define __APP_WEB_JST_H__

#ifdef __cplusplus
extern "C" {
#endif

//#include "webgui_app.h"



/*
 * jst
 */
extern int web_system_jst_init(void);
extern int web_html_jst_init(void);
extern int web_arp_jst_init(void);
#ifdef ZPL_DHCP_MODULE
extern int web_dhcp_jst_init(void);
#endif
extern int web_dns_jst_init(void);
extern int web_dos_jst_init(void);
extern int web_firewall_jst_init(void);
extern int web_interface_jst_init(void);
extern int web_mac_jst_init(void);
extern int web_port_jst_init(void);
extern int web_ppp_jst_init(void);
extern int web_qos_jst_init(void);
extern int web_route_jst_init(void);
extern int web_serial_jst_init(void);
extern int web_tunnel_jst_init(void);
extern int web_vlan_jst_init(void);

 
#ifdef __cplusplus
}
#endif
 

#endif /* __APP_WEB_JST_H__ */
