/*
 * uci_ubus.h
 *
 *  Created on: 2019年2月11日
 *      Author: DELL
 */

#ifndef __UCI_UBUS_H__
#define __UCI_UBUS_H__


#ifdef PL_OPENWRT_UCI
#if 1//def  BUILD_OPENWRT

#define UCI_UBUS_BUF_MAX	1024
#define UCI_UBUS_CB_MAX		16
#define UCI_UBUS_UDP_PORT	65530


#define UCI_UBUS_DEBUG		1



typedef int	(*uci_ubus_cb)(void *, char *, int);

typedef struct uci_ubus_s
{
	int		accept;
	int		sock;
	char	buf[UCI_UBUS_BUF_MAX];
	int		len;
	uci_ubus_cb cb[UCI_UBUS_CB_MAX];
	void 	*cb_argvs[UCI_UBUS_CB_MAX];
	void	*master;
	void	*t_accept;
	void	*t_read;

	int		debug;
}uci_ubus_t;

//extern uci_ubus_t uci_ubus_ctx;

extern int uci_ubus_cb_install(uci_ubus_cb *cb, void *);
extern int uci_ubus_cb_uninstall(uci_ubus_cb *cb, void *);

extern int uci_ubus_debug(BOOL enable);

extern int uci_ubus_init(void *m);
extern int uci_ubus_reset(void);
extern int uci_ubus_exit(void);
#endif /* BUILD_OPENWRT */
#endif /* PL_OPENWRT_UCI */

#endif /* __UCI_UBUS_H__ */
