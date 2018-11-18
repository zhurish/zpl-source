/*
 * modem_dhcp.h
 *
 *  Created on: Jul 28, 2018
 *      Author: zhurish
 */

#ifndef __MODEM_DHCP_H__
#define __MODEM_DHCP_H__


//#define _MODEM_DHCPC_DEBUG
#define _MODEM_DHCP_DEBUG
//#define MODEM_DHCPC_PROCESS

extern int modem_dhcpc_attach(modem_t *modem);
extern int modem_dhcpc_unattach(modem_t *modem);

extern BOOL modem_dhcpc_isconnect(modem_t *modem);
extern BOOL modem_dhcpc_islinkup(modem_t *modem);
extern int modem_dhcpc_start(modem_t *modem);
extern int modem_dhcpc_exit(modem_t *modem);





#ifdef _MODEM_DHCP_DEBUG
#define MODEM_HDCP_DEBUG(fmt,...)	modem_debug_printf(stderr, __func__, __LINE__,fmt, ##__VA_ARGS__)
#else
#define MODEM_HDCP_DEBUG(fmt,...)
#endif


#endif /* __MODEM_DHCP_H__ */
