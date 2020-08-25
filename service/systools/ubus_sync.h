/*
 * ubus_sync.h
 *
 *  Created on: 2019年2月11日
 *      Author: DELL
 */

#ifndef __UBUS_SYNC_H__
#define __UBUS_SYNC_H__


#ifdef PL_UBUS_MODULE

#define UBUS_SYNC_BUF_MAX	1024
#define UBUS_SYNC_CB_MAX		16
#define UBUS_SYNC_UDP_PORT	65530


#define UBUS_SYNC_DEBUG		1



typedef int	(*ubus_sync_cb)(void *, char *, int);

typedef struct ubus_sync_s
{
	int		accept;
	int		sock;
	char	buf[UBUS_SYNC_BUF_MAX];
	int		len;
	ubus_sync_cb cb[UBUS_SYNC_CB_MAX];
	void 	*cb_argvs[UBUS_SYNC_CB_MAX];
	void	*master;
	void	*t_accept;
	void	*t_read;

	int		debug;
}ubus_sync_t;

//extern ubus_sync_t ubus_sync_ctx;

extern int ubus_sync_hook_install(ubus_sync_cb *cb, void *);
extern int ubus_sync_hook_uninstall(ubus_sync_cb *cb, void *);

extern int ubus_sync_debug(BOOL enable);

extern int ubus_sync_init(void *m);
extern int ubus_sync_reset(void);
extern int ubus_sync_exit(void);


#endif /* PL_UBUS_MODULE */

#endif /* __UBUS_SYNC_H__ */
