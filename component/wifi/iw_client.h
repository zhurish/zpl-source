/*
 * iw_client.h
 *
 *  Created on: Jul 15, 2018
 *      Author: zhurish
 */

#ifndef __IW_CLIENT_H__
#define __IW_CLIENT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zplos_include.h"
#include "vty.h"
#include "if.h"

#include "iw_config.h"


#define IW_CLIENT_SCAN_DEFAULT	60

#define IW_CLIENT_CON_DEFAULT	20

typedef struct iw_client_db_s
{
	NODE				node;
	char 				SSID[IW_SSID_NAME_MAX];
	char 				password[IW_SSID_PASS_MAX];
	char 				encrypt_password[IW_SSID_PASS_MAX];
	ifindex_t					ifindex;
}iw_client_db_t;

typedef struct iw_client_ap_s
{
	NODE				node;
	zpl_uint8 				BSSID[IW_SSID_NAME_MAX];
	char 				SSID[IW_SSID_NAME_MAX];
	zpl_uint8				channel;
	double				freq;
	zpl_uint8				qaul;
	zpl_uint8				max_qaul;
	int					signal;
	int					nosie;
	zpl_uint16				bitrate;
	zpl_uint8				beacon;
	iw_authentication_t	auth;
	ifindex_t					ifindex;

	zpl_uint8				ttl;
	zpl_uint8				connect;
}iw_client_ap_t;


typedef struct iw_client_s
{
	ifindex_t					ifindex;
#ifndef IW_ONCE_TASK
	zpl_uint32					taskid;
#endif
	zpl_bool				scan_enable;
	zpl_uint32				scan_interval;
	zpl_bool				connect_enable;
	zpl_uint32				connect_delay;

	zpl_bool				auto_connect;
	void				*mutex;

	zpl_uint32				scan_max;
	LIST				*ap_list;
	LIST				*ap_unlist;
	void				*ap_mutex;
	iw_client_ap_t		*ap;

	LIST				*db_list;
	void				*db_mutex;

	iw_client_db_t		*now;
	iw_client_db_t		cu;

	void				*scan_result;
#ifdef IW_ONCE_TASK
	void				*master;
	void				*scan_thread;
	void				*connect_thread;
#endif
}iw_client_t;

extern int iw_client_init(iw_client_t *, ifindex_t ifindex);
extern int iw_client_exit(iw_client_t *);
extern int iw_client_enable(iw_client_t *iw_client, zpl_bool enable);

extern int iw_client_task_start(iw_client_t *iw_client);
extern int iw_client_task_exit(iw_client_t *iw_client);

extern iw_client_t * iw_client_lookup_api(struct interface *ifp);

/*
 * auto connect
 */
extern int iw_client_connect_start(iw_client_t *);
extern int iw_client_connect_exit(iw_client_t *);

extern int iw_client_connect_api(iw_client_t *iw_client, zpl_bool auto_connect);
extern int iw_client_disconnect_api(iw_client_t *iw_client);

//show current connect information 显示当前连接的wifi
extern int iw_client_connect_ap_show(iw_client_t *iw_client, struct vty *vty);
//显示扫描的wifi
extern int iw_client_scan_ap_show(iw_client_t *iw_client, struct vty *vty);
extern int iw_client_station_dump_show(iw_client_t *iw_client, struct vty *vty);

extern int iw_client_connect_interval_api(iw_client_t *iw_client, zpl_uint32 connect_interval);
//扫描附近wifi时间间隔
extern int iw_client_scan_interval_api(iw_client_t *iw_client, zpl_uint32 scan_interval);
extern int iw_client_scan_max_api(iw_client_t *iw_client, zpl_uint32 scan_max);
/*
 * DB
 */
extern int iw_client_db_set_api(iw_client_t *, char *ssid, char *pass);
extern int iw_client_db_del_api(iw_client_t *, char *ssid, zpl_bool pass);

extern iw_client_db_t * iw_client_db_lookup_api(iw_client_t *, char *ssid);
extern int iw_client_db_callback_api(iw_client_t *, int (*cb)(iw_client_db_t *, void *), void *pVoid);

/*
 * AP scan
 */
//显示附近wifi
extern int iw_client_neighbor_show(iw_client_t *, struct vty *vty, zpl_bool all);
extern int iw_client_ap_set_api(iw_client_t *, zpl_uint8 *bssid, iw_client_ap_t *ap);
extern int iw_client_ap_del_api(iw_client_t *, zpl_uint8 *bssid, char *ssid);
extern iw_client_ap_t * iw_client_ap_lookup_api(iw_client_t *, zpl_uint8 *bssid, char *ssid);
extern int iw_client_neighbor_callback_api(iw_client_t *, int (*cb)(iw_client_ap_t *, void *), void *pVoid);

extern int iw_client_scan_start(iw_client_t *);
extern int iw_client_scan_exit(iw_client_t *);

 
#ifdef __cplusplus
}
#endif
 
#endif /* __IW_CLIENT_H__ */
