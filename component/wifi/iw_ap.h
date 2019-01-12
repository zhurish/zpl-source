/*
 * iw_ap.h
 *
 *  Created on: Oct 11, 2018
 *      Author: zhurish
 */

#ifndef __IW_AP_H__
#define __IW_AP_H__


#include "zebra.h"
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
#define IW_AP_WMM_DEFAULT			TRUE
#define IW_AP_MAX_STA_DEFAULT		128
#define IW_AP_CLIENT_SCAN_DEFAULT	30

typedef struct iw_ap_connect_s
{
	NODE				node;
	ifindex_t			ifindex;
	u_int8 				BSSID[IW_SSID_NAME_MAX];
	u_int16				inactive_time;
	int					signal;
	u_int32				rx_bytes;
	u_int32				rx_packets;
	u_int32				tx_bytes;
	u_int32				tx_packets;
	u_int32				tx_retries;
	u_int32				tx_failed;
	u_int32				rx_drop;
	double				tx_bitrate;
	double				rx_bitrate;
	double				throughput;
	BOOL				authorized;
	BOOL				authenticated;
	BOOL				associated;
	BOOL				WME_WMM;
	//u_int8				preamble;
	BOOL				MFP;
	BOOL				TDLS;
	u_int8				period;
	u_int8				beacon;
	u_int32				connected_time;
	u_int8				TTL;
}iw_ap_connect_t;

typedef struct iw_ap_mac_s
{
	NODE				node;
	u_int8 				MAC[6];
}iw_ap_mac_t;


typedef struct iw_ap_pass_s
{
	char password[IW_SSID_PASS_MAX];
	char encrypt_password[IW_SSID_PASS_MAX];
}iw_ap_pass_t;

typedef struct iw_ap_s
{
	ifindex_t			ifindex;
	void				*mutex;

	iw_hw_mode_t		hw_mode;
	iw_encryption_t 	encryption;
	iw_authentication_t auth;
	//iw_network_t		network;

	char 				SSID[IW_SSID_NAME_MAX];
	BOOL				hiden_ssid;
/*	char 				password[IW_SSID_PASS_MAX];
	char 				encrypt_password[IW_SSID_PASS_MAX];*/
	u_int8				wep_key;
	iw_ap_pass_t		password[1];

	u_int8 				BSSID[IW_SSID_NAME_MAX];
	u_int8				channel;
	u_int8				beacon;
	u_int8				power;

	u_int8				driver;
	u_int8				country_code;
	int					signal;
	int					bitrate;
	BOOL				wmm_enabled;
	BOOL				ap_isolate;
	double				freq;
	ifindex_t			bridge;

	int					rts_threshold;
	int 				frag;
	//分片阈值 1400 RTS/CTS 阈值
	//RTS thr=100 B   Fragment thr=1400 B

	u_int16				acs_num_scans;

	u_int16				max_num_sta;
	LIST				*ap_list;
	void				*ap_mutex;

	u_int8				macaddr_acl;
	LIST				*mac_list;
	LIST				*dmac_list;

	u_int8				ap_client_delay;
	BOOL				change;
	u_int32				crc_sum;
#ifndef IW_ONCE_TASK
	int					taskid;
#else
	void				*master;
	void				*t_thread;
	void				*s_thread;
#endif
}iw_ap_t;


extern int iw_ap_init(iw_ap_t *, ifindex_t ifindex);
extern int iw_ap_exit(iw_ap_t *);

extern int iw_ap_task_start(iw_ap_t *iw_ap);
extern int iw_ap_task_exit(iw_ap_t *iw_ap);

extern iw_ap_t * iw_ap_lookup_api(struct interface *);


extern int iw_ap_ssid_set_api(iw_ap_t *, char *);
extern int iw_ap_ssid_del_api(iw_ap_t *);

extern int iw_ap_password_set_api(iw_ap_t *, char *);
extern int iw_ap_password_del_api(iw_ap_t *);

extern int iw_ap_channel_set_api(iw_ap_t *, int );
extern int iw_ap_channel_del_api(iw_ap_t *, int );

extern int iw_ap_power_set_api(iw_ap_t *, int );
extern int iw_ap_power_del_api(iw_ap_t *, int );

extern int iw_ap_signal_set_api(iw_ap_t *, int );

extern int iw_ap_rts_threshold_set_api(iw_ap_t *iw_ap, int rts_threshold);
extern int iw_ap_frag_set_api(iw_ap_t *iw_ap, int frag);
/*
extern int iw_dev_mode_set(struct interface *ifp, char * value);
extern int iw_dev_txpower_set(struct interface *ifp, char * value);
extern int iw_dev_rts_threshold_set(struct interface *ifp, char * value);
extern int iw_dev_frag_set(struct interface *ifp, char * value);
extern int iw_dev_rate_set(struct interface *ifp, char * value);
extern int iw_dev_bit_set(struct interface *ifp, char * value);
extern int iw_dev_channel_set(struct interface *ifp, struct vty *vty, char *  value);
*/

extern int iw_ap_beacon_set_api(iw_ap_t *iw_ap, int beacon);
extern int iw_ap_bitrate_set_api(iw_ap_t *iw_ap, int bitrate);
extern int iw_ap_isolate_set_api(iw_ap_t *iw_ap, BOOL enable);
extern int iw_ap_bridge_set_api(iw_ap_t *iw_ap, ifindex_t bridge);
extern int iw_ap_country_set_api(iw_ap_t *iw_ap, int country);
extern int iw_ap_wmm_set_api(iw_ap_t *iw_ap, BOOL enable);

extern int iw_ap_freq_set_api(iw_ap_t *iw_ap, double freq);
extern int iw_ap_scan_num_set_api(iw_ap_t *iw_ap, int value);
extern int iw_ap_client_num_set_api(iw_ap_t *iw_ap, int value);

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
 * MAC ACL
 */
extern iw_ap_mac_t * iw_ap_mac_lookup_api(iw_ap_t *iw_ap, u_int8 *mac, BOOL accept);
extern int iw_ap_mac_add_api(iw_ap_t *iw_ap, u_int8 *mac, BOOL accept);
extern int iw_ap_mac_del_api(iw_ap_t *iw_ap, u_int8 *mac, BOOL accept);




extern int iw_ap_make_script(iw_ap_t *iw_ap);
extern int iw_ap_running_script(iw_ap_t *iw_ap);
extern int iw_ap_stop_script(iw_ap_t *iw_ap);



/*
 * ap connect
 */
extern int iw_ap_connect_add_api(iw_ap_t *, iw_ap_connect_t *);
extern int iw_ap_connect_del_api(iw_ap_t *, u_int8 *bssid);
extern iw_ap_connect_t * iw_ap_connect_lookup_api(iw_ap_t *, u_int8 *bssid);
extern int iw_ap_connect_callback_api(iw_ap_t *, int (*cb)(iw_ap_connect_t *, void *), void *pVoid);

extern int iw_ap_connect_show(iw_ap_t *iw_ap, struct vty *vty, BOOL detail);

extern int iw_ap_config(iw_ap_t *iw_ap, struct vty *vty);


#endif /* __IW_AP_H__ */
