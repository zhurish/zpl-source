/*
 * ubus_sync.h
 *
 *  Created on: 2019年2月11日
 *      Author: DELL
 */

#ifndef __UBUS_SYNC_H__
#define __UBUS_SYNC_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ZPL_SERVICE_UBUS_SYNC

#define UBUS_SYNC_BUF_MAX	1024
#define UBUS_SYNC_CB_MAX		16
#define UBUS_SYNC_UDP_PORT	65530


#define UBUS_SYNC_DEBUG		1



typedef int	(*ubus_sync_cb)(void *, char *, zpl_uint32);

typedef struct ubus_sync_s
{
	int		accept;
	int		sock;
	char	buf[UBUS_SYNC_BUF_MAX];
	zpl_uint32		len;
	ubus_sync_cb cb[UBUS_SYNC_CB_MAX];
	void 	*cb_argvs[UBUS_SYNC_CB_MAX];
	void	*master;
	void	*t_accept;
	void	*t_read;

	zpl_uint32		debug;
}ubus_sync_t;

//extern ubus_sync_t ubus_sync_ctx;

extern int ubus_sync_hook_install(ubus_sync_cb *cb, void *);
extern int ubus_sync_hook_uninstall(ubus_sync_cb *cb, void *);

extern int ubus_sync_debug(zpl_bool enable);

extern int ubus_sync_init(void *m);
extern int ubus_sync_reset(void);
extern int ubus_sync_exit(void);


#endif /* ZPL_SERVICE_UBUS_SYNC */
 
#ifdef __cplusplus
}
#endif

#endif /* __UBUS_SYNC_H__ */
