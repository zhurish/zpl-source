/*
 * iw_config.h
 *
 *  Created on: Oct 10, 2018
 *      Author: zhurish
 */

#ifndef __IW_CONFIG_H__
#define __IW_CONFIG_H__

/*
#define IW_MAX_FREQUENCIES	32
#define IW_MAX_BITRATES		32
#define IW_MAX_TXPOWER		8
#define IW_MAX_SPY		8
#define IW_MAX_AP		64
*/

#define IW_DEV_CHANNEL_MAX	32
#define IW_DEV_BITRATES_MAX	32
#define IW_DEV_TXPOWER_MAX	8

#define IW_SSID_NAME_MAX	64
#define IW_SSID_PASS_MAX	64

#define IW_CLIENT_DB_FILE	SYSCONFDIR"/.iw-client.db"
#define IW_CLIENT_DB_OLD_FILE	SYSCONFDIR"/.iw-client.db.old"

#define IW_SYSCONFDIR	BASE_DIR "/etc"

#define IW_ONCE_TASK


#define IW_DEBUG_EVENT		0X1
#define IW_DEBUG_DB			0X2
#define IW_DEBUG_SCAN		0X4
#define IW_DEBUG_AP			0X8
#define IW_DEBUG_AP_ACCEPT	0X10

#define IW_DEBUG_DETAIL		0X800

#define IW_DEBUG(n)		iw_debug_conf & IW_DEBUG_ ## n

#define IW_DEBUG_ON(n)		iw_debug_conf |= IW_DEBUG_ ## n
#define IW_DEBUG_OFF(n)		iw_debug_conf &= ~( IW_DEBUG_ ## n )

//iw: managed, ibss, monitor, mesh, wds
//iwconfig: managed|ad-hoc|master|...
/* Modes of operation */
#if 0
#define IW_MODE_AUTO	0	/* Let the driver decides */
#define IW_MODE_ADHOC	1	/* Single cell network */
#define IW_MODE_INFRA	2	/* Multi cell network, roaming, ... */
#define IW_MODE_MASTER	3	/* Synchronisation master or Access Point */
#define IW_MODE_REPEAT	4	/* Wireless Repeater (forwarder) */
#define IW_MODE_SECOND	5	/* Secondary master/repeater (backup) */
#define IW_MODE_MONITOR	6	/* Passive monitor (listen only) */
#endif
typedef enum iw_mode_s
{
	IW_MODE_NONE = 0,
	IW_MODE_IBSS,		//Ad-Hoc (IBSS) mode
	IW_MODE_ADHOC = IW_MODE_IBSS,
	IW_MODE_MANAGE,		//Station (STA) infrastructure mode
	IW_MODE_AP,			//AccessPoint (AP) infrastructure mode
	IW_MODE_MONITOR,	//Monitor (MON) mode
	IW_MODE_MESH,
	IW_MODE_WDS,		//Wireless Distribution System (WDS) mode
	IW_MODE_CLIENT = IW_MODE_MANAGE,
}iw_mode_t;


typedef enum
{
	IW_ENCRY_NONE = 0,
	IW_ENCRY_WEP_OPEN,
	IW_ENCRY_WEP_PRIVATE,
	IW_ENCRY_WPA_PSK,		//= 1 WPA-PSK CCMP/AUTO
	IW_ENCRY_WPA2_PSK,		//= 2 WPA2-PSK CCMP/AUTO
	IW_ENCRY_WPA2WPA_PSK,	//= 3 WPA2-PSK CCMP/AUTO
}iw_authentication_t;

typedef enum
{
	IW_ALGO_NONE = 0,
	IW_ALGO_AUTO,
	IW_ALGO_WEP,
	IW_ALGO_CCMP,
	IW_ALGO_TKIP,
	IW_ALGO_TKIP_CCMP,
	IW_ALGO_IGTK,
	IW_ALGO_PMK,
	IW_ALGO_GCMP,
	IW_ALGO_SMS4,
	IW_ALGO_KRK,
	IW_ALGO_GCMP_256,
	IW_ALGO_CCMP_256,
	IW_ALGO_BIP_GMAC_128,
	IW_ALGO_BIP_GMAC_256,
	IW_ALGO_BIP_CMAC_256,
}iw_encryption_t;

/*typedef enum iw_network_s
{
	IW_NETWORK_11_B_G_N = 0,
	IW_NETWORK_11_B_G,
	IW_NETWORK_11_A,
	IW_NETWORK_11_G,
	IW_NETWORK_11_B,
	IW_NETWORK_11_AN,
	IW_NETWORK_11_GN,
	IW_NETWORK_11_N,
}iw_network_t;*/

typedef enum iw_hw_mode_s
{
	IW_HW_MODE_IEEE80211ANY = 0,
	IW_HW_MODE_IEEE80211A,	//5G
	IW_HW_MODE_IEEE80211B,	//2.4G
	IW_HW_MODE_IEEE80211D,	//
	IW_HW_MODE_IEEE80211H,	//
	IW_HW_MODE_IEEE80211G,	//2.4G
	IW_HW_MODE_IEEE80211AD,
	IW_HW_MODE_IEEE80211AC,	//5G
	IW_HW_MODE_IEEE80211N,	//2.4G/5G

	IW_HW_MODE_IEEE80211AB,
	IW_HW_MODE_IEEE80211AG,
	IW_HW_MODE_IEEE80211AN,
	IW_HW_MODE_IEEE80211BG,
	IW_HW_MODE_IEEE80211NG,

	IW_HW_MODE_IEEE80211BGN,
}iw_hw_mode_t;


typedef enum
{
	IW_VALUE_AUTO = 0,
	IW_VALUE_FIEXD = -1,
	IW_VALUE_OFF = -2,
}iw_value_t;


typedef struct iw_dev_freq_s
{
	unsigned char	active;
	unsigned char	channel;
	double	freq;
}iw_dev_freq_t, iw_dev_channel_s;


typedef struct iw_dev_bitrate_s
{
	unsigned char	active;
	unsigned char	channel;
	unsigned int	bitrate;
}iw_dev_bitrate_t;

//#include "wireless.h"

typedef struct iw_dev_s
{
	iw_dev_freq_t 		freq[IW_DEV_CHANNEL_MAX];
	iw_dev_freq_t 		cu_freq;

	unsigned char		num_bitrates;
	iw_dev_bitrate_t	bitrates[IW_DEV_BITRATES_MAX];
	iw_dev_bitrate_t	cu_bitrates;
	iw_dev_bitrate_t	broadcast_bitrates;

}iw_dev_t;


extern int	iw_debug_conf;

extern char * iw_mode_string(iw_mode_t mode);
extern char * iw_hw_mode_string(iw_hw_mode_t mode);
extern char * iw_auth_string(iw_authentication_t mode);
extern char * iw_encryption_string(iw_encryption_t mode);
//extern char * iw_network_string(iw_network_t mode);

#endif /* __IW_CONFIG_H__ */
