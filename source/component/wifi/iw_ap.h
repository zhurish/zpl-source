/*
 * iw_ap.h
 *
 *  Created on: Oct 11, 2018
 *      Author: zhurish
 */

#ifndef __IW_AP_H__
#define __IW_AP_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "zplos_include.h"
#include "vty.h"
#include "if.h"

#include "iw_config.h"

#define IW_AP_CONNECT_TTL_DEFAULT	2

#define IW_AP_HW_MODE_DEFAULT		IW_HW_MODE_IEEE80211G
#define IW_AP_ENCRYPTION_DEFAULT	IW_ALGO_AUTO
#define IW_AP_AUTH_DEFAULT			IW_ENCRY_WPA2WPA_PSK
#define IW_AP_CHANNEL_DEFAULT		0
#define IW_AP_BEACON_DEFAULT		100
#define IW_AP_COUNTRY_DEFAULT		0
#define IW_AP_DRIVER_DEFAULT		0
#define IW_AP_WMM_DEFAULT			zpl_true
#define IW_AP_MAX_STA_DEFAULT		128
#define IW_AP_CLIENT_SCAN_DEFAULT	30

typedef struct iw_ap_connect_s
{
	NODE				node;
	ifindex_t			ifindex;
	zpl_uint8 				BSSID[IW_SSID_NAME_MAX];
	zpl_uint16				inactive_time;
	int					signal;
	zpl_uint32				rx_bytes;
	zpl_uint32				rx_packets;
	zpl_uint32				tx_bytes;
	zpl_uint32				tx_packets;
	zpl_uint32				tx_retries;
	zpl_uint32				tx_failed;
	zpl_uint32				rx_drop;
	zpl_double				tx_bitrate;
	zpl_double				rx_bitrate;
	zpl_double				throughput;
	zpl_bool				authorized;
	zpl_bool				authenticated;
	zpl_bool				associated;
	zpl_bool				WME_WMM;
	//zpl_uint8				preamble;
	zpl_bool				MFP;
	zpl_bool				TDLS;
	zpl_uint8				period;
	zpl_uint8				beacon;
	zpl_uint32				connected_time;
	zpl_uint8				IPSTACK_TTL;
}iw_ap_connect_t;

typedef struct iw_ap_mac_s
{
	NODE				node;
	zpl_uint8 				MAC[6];
}iw_ap_mac_t;


typedef struct iw_ap_pass_s
{
	zpl_char password[IW_SSID_PASS_MAX];
	zpl_char encrypt_password[IW_SSID_PASS_MAX];
}iw_ap_pass_t;

typedef struct iw_ap_s
{
	ifindex_t			ifindex;
	void				*mutex;

	iw_hw_mode_t		hw_mode;
	iw_encryption_t 	encryption;
	iw_authentication_t auth;
	//iw_network_t		network;

	zpl_char 				SSID[IW_SSID_NAME_MAX];
	zpl_bool				hiden_ssid;
/*	char 				password[IW_SSID_PASS_MAX];
	char 				encrypt_password[IW_SSID_PASS_MAX];*/
	zpl_uint8				wep_key;
	iw_ap_pass_t		password[1];

	zpl_uint8 				BSSID[IW_SSID_NAME_MAX];
	zpl_uint8				channel;
	zpl_uint8				beacon;
	zpl_uint8				power;

	zpl_uint8				driver;
	zpl_uint8				country_code;
	zpl_int					signal;
	zpl_int					bitrate;
	zpl_bool				wmm_enabled;
	zpl_bool				ap_isolate;
	zpl_double				freq;
	ifindex_t			bridge;

	zpl_int					rts_threshold;
	zpl_int 				frag;
	//分片阈值 1400 RTS/CTS 阈值
	//RTS thr=100 B   Fragment thr=1400 B

	zpl_uint16				acs_num_scans;

	zpl_uint16				max_num_sta;
	LIST				*ap_list;
	void				*ap_mutex;

	zpl_uint8				macaddr_acl;
	LIST				*mac_list;
	LIST				*dmac_list;

	zpl_uint8				ap_client_delay;
	zpl_bool				change;
	zpl_uint32				crc_sum;
#ifndef IW_ONCE_TASK
	zpl_taskid_t					taskid;
#else
	void				*master;
	void				*t_thread;
	void				*s_thread;
#endif
}iw_ap_t;


extern int iw_ap_init(iw_ap_t *, ifindex_t ifindex);
extern int iw_ap_exit(iw_ap_t *);
extern int iw_ap_enable(iw_ap_t *iw_ap, zpl_bool enable);

extern int iw_ap_task_start(iw_ap_t *iw_ap);
extern int iw_ap_task_exit(iw_ap_t *iw_ap);

extern iw_ap_t * iw_ap_lookup_api(struct interface *);


extern int iw_ap_ssid_set_api(iw_ap_t *, char *);
extern int iw_ap_ssid_del_api(iw_ap_t *);

extern int iw_ap_password_set_api(iw_ap_t *, char *);
extern int iw_ap_password_del_api(iw_ap_t *);

extern int iw_ap_channel_set_api(iw_ap_t *, zpl_uint8 );
extern int iw_ap_channel_del_api(iw_ap_t *, zpl_uint8 );

extern int iw_ap_power_set_api(iw_ap_t *, zpl_uint8 );
extern int iw_ap_power_del_api(iw_ap_t *, zpl_uint8 );

extern int iw_ap_signal_set_api(iw_ap_t *, int );

extern int iw_ap_rts_threshold_set_api(iw_ap_t *iw_ap, zpl_int rts_threshold);
extern int iw_ap_frag_set_api(iw_ap_t *iw_ap, zpl_int frag);
/*
extern int iw_dev_mode_set(struct interface *ifp, char * value);
extern int iw_dev_txpower_set(struct interface *ifp, char * value);
extern int iw_dev_rts_threshold_set(struct interface *ifp, char * value);
extern int iw_dev_frag_set(struct interface *ifp, char * value);
extern int iw_dev_rate_set(struct interface *ifp, char * value);
extern int iw_dev_bit_set(struct interface *ifp, char * value);
extern int iw_dev_channel_set(struct interface *ifp, struct vty *vty, char *  value);
*/

extern int iw_ap_beacon_set_api(iw_ap_t *iw_ap, zpl_uint8 beacon);
extern int iw_ap_bitrate_set_api(iw_ap_t *iw_ap, zpl_int bitrate);
extern int iw_ap_isolate_set_api(iw_ap_t *iw_ap, zpl_bool enable);
extern int iw_ap_bridge_set_api(iw_ap_t *iw_ap, ifindex_t bridge);
extern int iw_ap_country_set_api(iw_ap_t *iw_ap, zpl_uint8 country);
extern int iw_ap_wmm_set_api(iw_ap_t *iw_ap, zpl_bool enable);

extern int iw_ap_freq_set_api(iw_ap_t *iw_ap, zpl_double freq);
extern int iw_ap_scan_num_set_api(iw_ap_t *iw_ap, zpl_uint16 value);
extern int iw_ap_client_num_set_api(iw_ap_t *iw_ap, zpl_uint16 value);

extern int iw_ap_auth_set_api(iw_ap_t *, iw_authentication_t );
extern int iw_ap_auth_del_api(iw_ap_t *, iw_authentication_t );

extern int iw_ap_encryption_set_api(iw_ap_t *, iw_encryption_t );
extern int iw_ap_encryption_del_api(iw_ap_t *, iw_encryption_t );

/*
extern int iw_ap_network_set_api(iw_ap_t *, iw_network_t );
extern int iw_ap_network_del_api(iw_ap_t *, iw_network_t );
*/
extern int iw_ap_hw_mode_set_api(iw_ap_t *, iw_hw_mode_t );


/*
 * MAC ACL 接入点过滤MAC
 */
extern iw_ap_mac_t * iw_ap_mac_lookup_api(iw_ap_t *iw_ap, zpl_uint8 *mac, zpl_bool accept);
extern int iw_ap_mac_add_api(iw_ap_t *iw_ap, zpl_uint8 *mac, zpl_bool accept);
extern int iw_ap_mac_del_api(iw_ap_t *iw_ap, zpl_uint8 *mac, zpl_bool accept);



#ifdef ZPL_BUILD_OS_OPENWRT
extern int _iw_bridge_check_interface(char *br, char *wa);
#endif

extern int iw_ap_make_script(iw_ap_t *iw_ap);
extern int iw_ap_running_script(iw_ap_t *iw_ap);
extern int iw_ap_stop_script(iw_ap_t *iw_ap);
extern int iw_ap_script_is_running(iw_ap_t *iw_ap);


/*
 * ap connect
 */
extern int iw_ap_connect_add_api(iw_ap_t *, iw_ap_connect_t *);
extern int iw_ap_connect_del_api(iw_ap_t *, zpl_uint8 *bssid);
extern iw_ap_connect_t * iw_ap_connect_lookup_api(iw_ap_t *, zpl_uint8 *bssid);
extern int iw_ap_connect_callback_api(iw_ap_t *, int (*cb)(iw_ap_connect_t *, void *), void *pVoid);
//显示当前连接到AP的设备
extern int iw_ap_connect_show(iw_ap_t *iw_ap, struct vty *vty, zpl_bool detail);

extern int iw_ap_config(iw_ap_t *iw_ap, struct vty *vty);

 
#ifdef __cplusplus
}
#endif
 
#endif /* __IW_AP_H__ */
