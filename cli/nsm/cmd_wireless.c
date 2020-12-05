/*
 * cmd_wireless.c
 *
 *  Created on: Oct 11, 2018
 *      Author: zhurish
 */

#include "zebra.h"
#include "vty.h"
#include "if.h"

#include "buffer.h"
#include "command.h"

#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "nsm_vrf.h"
#include "nsm_interface.h"
#include "if_name.h"

#ifdef PL_WIFI_MODULE
#include "iw_config.h"
#include "iw_ap.h"
#include "iw_client.h"
#include "iw_interface.h"

DEFUN (wireless_enable,
	   wireless_enable_cmd,
		"dev enable",
		"Device configure\n"
		"Enable\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	ret = nsm_iw_enable_api(ifp, TRUE);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_wireless_enable,
		no_wireless_enable_cmd,
		"no dev enable",
		NO_STR
		"Device configure\n"
		"Enable\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	ret = nsm_iw_enable_api(ifp, FALSE);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (work_mode,
		work_mode_cmd,
		"work-mode (ap|manage)",
		"working mode configure\n"
		"AP mode\n"
		"Manage(Client) mode\n")
{
	int ret = ERROR;
	iw_mode_t mode = IW_MODE_NONE, oldmode = IW_MODE_NONE;
	struct interface *ifp = vty->index;
	if(ifp)
	{
		if(nsm_iw_mode_get_api(ifp, &oldmode) == OK)
		{
			if(strncmp(argv[0], "ap", 2) == 0)
				mode = IW_MODE_AP;
			else if(strncmp(argv[0], "manage", 2) == 0)
				mode = IW_MODE_MANAGE;
			if(oldmode != mode)
				ret = nsm_iw_mode_set_api(ifp, mode);
			else
				ret = OK;
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_work_mode,
		no_work_mode_cmd,
		"no work-mode",
		NO_STR
		"working mode configure\n")
{
	int ret = ERROR;
	iw_mode_t mode = IW_MODE_NONE, oldmode = IW_MODE_NONE;
	struct interface *ifp = vty->index;
	if(ifp)
	{
		if(nsm_iw_mode_get_api(ifp, &oldmode) == OK)
		{
			if(oldmode != mode)
				ret = nsm_iw_mode_set_api(ifp, mode);
			else
				ret = OK;
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (radio_type,
		radio_type_cmd,
		"radio-type (ieee80211a|ieee80211b|ieee80211d|ieee80211h|ieee80211g|ieee80211n|" \
			"ieee80211ab|ieee80211ac|ieee80211ad|ieee80211ag|ieee80211an|" \
			"ieee80211bg|ieee80211ng|ieee80211bgn)",
		"radio type configure\n"
		"IEEE802.11A mode\n"
		"IEEE802.11B mode\n"
		"IEEE802.11D mode\n"
		"IEEE802.11H mode\n"
		"IEEE802.11G mode\n"
		"IEEE802.11N mode\n"
		"IEEE802.11A/B mode\n"
		"IEEE802.11A/C mode\n"
		"IEEE802.11A/D mode\n"
		"IEEE802.11A/G mode\n"
		"IEEE802.11A/N mode\n"
		"IEEE802.11B/G mode\n"
		"IEEE802.11N/G mode\n"
		"IEEE802.11B/G/N mode\n")
{
	int ret = ERROR;
	iw_hw_mode_t mode = IW_HW_MODE_IEEE80211ANY;
	struct interface *ifp = vty->index;
	if(ifp && nsm_iw_mode(ifp) == IW_MODE_AP)
	{
		if(os_strlen(argv[0]) == os_strlen("ieee80211a"))
		{
			if(strncmp(argv[0], "ieee80211a", os_strlen("ieee80211a")) == 0)
				mode = IW_HW_MODE_IEEE80211A;
			else if(strncmp(argv[0], "ieee80211b", os_strlen("ieee80211a")) == 0)
				mode = IW_HW_MODE_IEEE80211B;
			else if(strncmp(argv[0], "ieee80211g", os_strlen("ieee80211a")) == 0)
				mode = IW_HW_MODE_IEEE80211G;
			else if(strncmp(argv[0], "ieee80211n", os_strlen("ieee80211a")) == 0)
				mode = IW_HW_MODE_IEEE80211N;
			else if(strncmp(argv[0], "ieee80211d", os_strlen("ieee80211a")) == 0)
				mode = IW_HW_MODE_IEEE80211D;
			else if(strncmp(argv[0], "ieee80211h", os_strlen("ieee80211a")) == 0)
				mode = IW_HW_MODE_IEEE80211H;
		}
		else if(os_strlen(argv[0]) == os_strlen("ieee80211ac"))
		{
			if(strncmp(argv[0], "ieee80211ac", os_strlen("ieee80211ac")) == 0)
				mode = IW_HW_MODE_IEEE80211AC;
			else if(strncmp(argv[0], "ieee80211bg", os_strlen("ieee80211ac")) == 0)
				mode = IW_HW_MODE_IEEE80211BG;
			else if(strncmp(argv[0], "ieee80211ng", os_strlen("ieee80211ac")) == 0)
				mode = IW_HW_MODE_IEEE80211NG;
			else if(strncmp(argv[0], "ieee80211ad", os_strlen("ieee80211ac")) == 0)
				mode = IW_HW_MODE_IEEE80211AD;
			else if(strncmp(argv[0], "ieee80211ab", os_strlen("ieee80211ac")) == 0)
				mode = IW_HW_MODE_IEEE80211AB;
			else if(strncmp(argv[0], "ieee80211ag", os_strlen("ieee80211ac")) == 0)
				mode = IW_HW_MODE_IEEE80211AG;
			else if(strncmp(argv[0], "ieee80211an", os_strlen("ieee80211ac")) == 0)
				mode = IW_HW_MODE_IEEE80211AN;
		}
		else if(os_strlen(argv[0]) == os_strlen("ieee80211bgn"))
		{
			if(strncmp(argv[0], "ieee80211bgn", os_strlen("ieee80211bgn")) == 0)
				mode = IW_HW_MODE_IEEE80211BGN;
		}
		if(iw_ap_lookup_api(ifp))
			ret = iw_ap_hw_mode_set_api(iw_ap_lookup_api(ifp), mode);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_radio_type,
		no_radio_type_cmd,
		"no radio-type",
		NO_STR
		"radio type configure\n")
{
	int ret = ERROR;
	iw_hw_mode_t mode = IW_HW_MODE_IEEE80211ANY;
	struct interface *ifp = vty->index;
	if(ifp && nsm_iw_mode(ifp) == IW_MODE_AP)
	{
		if(iw_ap_lookup_api(ifp))
			ret = iw_ap_hw_mode_set_api(iw_ap_lookup_api(ifp), mode);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

/*
 * AP Manage
 */
DEFUN (work_channel,
		work_channel_cmd,
		"work-channel (auto|<1-14>)",
		"work channel configure\n"
		"auto mode\n"
		"channel\n")
{
	int ret = ERROR;
	int channel = 0;
	struct interface *ifp = vty->index;
	if(ifp && nsm_iw_mode(ifp) == IW_MODE_AP)
	{
		if(strstr(argv[0], "auto"))
			channel = IW_VALUE_AUTO;
		else
			channel = atoi(argv[0]);
		if(iw_ap_lookup_api(ifp))
			ret = iw_ap_channel_set_api(iw_ap_lookup_api(ifp), channel);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_work_channel,
		no_work_channel_cmd,
		"no work-channel",
		NO_STR
		"work channel configure\n")
{
	int ret = ERROR;
	int channel = IW_VALUE_AUTO;
	struct interface *ifp = vty->index;
	if(ifp && nsm_iw_mode(ifp) == IW_MODE_AP)
	{
		if(iw_ap_lookup_api(ifp))
			ret = iw_ap_channel_set_api(iw_ap_lookup_api(ifp), channel);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (power_level,
		power_level_cmd,
		"power-level (auto|off|<10-100>)",
		"power level configure\n"
		"auto mode\n"
		"turn off\n"
		"power value(step 10 mw)\n")
{
	int ret = ERROR;
	int power = 0;
	struct interface *ifp = vty->index;
	if(ifp && nsm_iw_mode(ifp) == IW_MODE_AP)
	{
		if(strstr(argv[0], "auto"))
			power = IW_VALUE_AUTO;
		else if(strstr(argv[0], "off"))
			power = IW_VALUE_OFF;
		else
			power = atoi(argv[0]);
		if(iw_ap_lookup_api(ifp))
			ret = iw_ap_power_set_api(iw_ap_lookup_api(ifp), power);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_power_level,
		no_power_level_cmd,
		"no power-level",
		NO_STR
		"power level configure\n")
{
	int ret = ERROR;
	int power = IW_VALUE_AUTO;
	struct interface *ifp = vty->index;
	if(ifp && nsm_iw_mode(ifp) == IW_MODE_AP)
	{
		if(iw_ap_lookup_api(ifp))
			ret = iw_ap_power_set_api(iw_ap_lookup_api(ifp), power);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (beacon_interval,
		beacon_interval_cmd,
		"beacon-interval <10-100>",
		"beacon configure\n"
		"interval value\n")
{
	int ret = ERROR;
	int interval = 0;
	struct interface *ifp = vty->index;
	if(ifp && nsm_iw_mode(ifp) == IW_MODE_AP)
	{
		if(strstr(argv[0], "auto"))
			interval = 0;
		else
			interval = atoi(argv[0]);
		if(iw_ap_lookup_api(ifp))
			ret = iw_ap_power_set_api(iw_ap_lookup_api(ifp), interval);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_beacon_interval,
		no_beacon_interval_cmd,
		"no beacon-interval",
		NO_STR
		"beacon configure\n")
{
	int ret = ERROR;
	int interval = 0;
	struct interface *ifp = vty->index;
	if(ifp && nsm_iw_mode(ifp) == IW_MODE_AP)
	{
		if(iw_ap_lookup_api(ifp))
			ret = iw_ap_power_set_api(iw_ap_lookup_api(ifp), interval);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (netwotrk_name,
		netwotrk_name_cmd,
		"network-name NAME",
		"Network name configure\n"
		"Name string\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(ifp/* && iw_ap_lookup_api(ifp)*/)
	{
		if((nsm_iw_mode(ifp) == IW_MODE_MANAGE || nsm_iw_mode(ifp) == IW_MODE_CLIENT) && iw_client_lookup_api(ifp))
			ret = iw_client_db_set_api(iw_client_lookup_api(ifp), argv[0], NULL);

		if(nsm_iw_mode(ifp) == IW_MODE_AP && iw_ap_lookup_api(ifp))
			ret = iw_ap_ssid_set_api(iw_ap_lookup_api(ifp), argv[0]);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_netwotrk_name,
		no_netwotrk_name_cmd,
		"no network-name",
		NO_STR
		"Network name configure\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(ifp/* && iw_ap_lookup_api(ifp)*/)
	{
		if((nsm_iw_mode(ifp) == IW_MODE_MANAGE || nsm_iw_mode(ifp) == IW_MODE_CLIENT) && iw_client_lookup_api(ifp))
			ret = iw_client_db_del_api(iw_client_lookup_api(ifp), NULL, FALSE);

		if(nsm_iw_mode(ifp) == IW_MODE_AP && iw_ap_lookup_api(ifp))
			ret = iw_ap_ssid_del_api(iw_ap_lookup_api(ifp));
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (authentication_password,
		authentication_password_cmd,
		"authentication password PASS ",
		"authentication configure\n"
		"Password configure\n"
		"Password string\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(ifp/* && iw_ap_lookup_api(ifp)*/)
	{
		if((nsm_iw_mode(ifp) == IW_MODE_MANAGE || nsm_iw_mode(ifp) == IW_MODE_CLIENT))
		{
			iw_client_t *client = iw_client_lookup_api(ifp);
			ret = iw_client_db_set_api(client, client->cu.SSID, argv[0]);
		}
		if(nsm_iw_mode(ifp) == IW_MODE_AP && iw_ap_lookup_api(ifp))
			ret = iw_ap_password_set_api(iw_ap_lookup_api(ifp), argv[0]);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_authentication_password,
		no_authentication_password_cmd,
		"no authentication password",
		NO_STR
		"authentication configure\n"
		"Password configure\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(ifp/* && iw_ap_lookup_api(ifp)*/)
	{
		if((nsm_iw_mode(ifp) == IW_MODE_MANAGE || nsm_iw_mode(ifp) == IW_MODE_CLIENT))
		{
			iw_client_t *client = iw_client_lookup_api(ifp);
			ret = iw_client_db_del_api(client, client->cu.SSID, TRUE);
		}
		if(nsm_iw_mode(ifp) == IW_MODE_AP && iw_ap_lookup_api(ifp))
			ret = iw_ap_password_del_api(iw_ap_lookup_api(ifp));
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ap_authentication_method,
		ap_authentication_method_cmd,
		"authentication-method (wpa-psk|wpa2-psk|wpa-wpa2-psk) ",
		"authentication method configure\n"
		"WPA-PSK encrypt\n"
		"WPA2-PSK encrypt\n"
		"WPA2WPA-PSK encrypt\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	iw_authentication_t auth = IW_ENCRY_NONE;
	if(ifp && iw_ap_lookup_api(ifp))
	{
		if(strncmp(argv[0], "wpa-psk", 7) == 0)
			auth = IW_ENCRY_WPA_PSK;
		if(strncmp(argv[0], "wpa2-psk", 7) == 0)
			auth = IW_ENCRY_WPA2_PSK;
		if(strncmp(argv[0], "wpa-wpa2-psk", 7) == 0)
			auth = IW_ENCRY_WPA2WPA_PSK;

		if(iw_ap_lookup_api(ifp))
			ret = iw_ap_auth_set_api(iw_ap_lookup_api(ifp), auth);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ap_authentication_method,
		no_ap_authentication_method_cmd,
		"no authentication-method",
		NO_STR
		"authentication method configure\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	iw_authentication_t auth = IW_ENCRY_WPA2WPA_PSK;
	if(ifp && iw_ap_lookup_api(ifp))
	{
		if(iw_ap_lookup_api(ifp))
			ret = iw_ap_auth_del_api(iw_ap_lookup_api(ifp), auth);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ap_encryption_method,
		ap_encryption_method_cmd,
		"encryption-method (auto|ccmp|tkip|tkip-ccmp) ",
		"encryption configure\n"
		"method configure\n"
		"AUTO Type\n"
		"CCMP Type\n"
		"TKIP Type\n"
		"CCMP-TKIP Type\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	iw_encryption_t encryption = IW_ALGO_AUTO;
	if(ifp && iw_ap_lookup_api(ifp))
	{
		if(strlen(argv[0]) > 4)
		{
			if(strncmp(argv[0], "tkip-ccmp", 7) == 0)
				encryption = IW_ALGO_TKIP_CCMP;
		}
		else
		{
			if(strncmp(argv[0], "auto", 4) == 0)
				encryption = IW_ALGO_AUTO;
			if(strncmp(argv[0], "ccmp", 4) == 0)
				encryption = IW_ALGO_CCMP;
			if(strncmp(argv[0], "tkip", 4) == 0)
				encryption = IW_ALGO_TKIP;
		}
		if(iw_ap_lookup_api(ifp))
			ret = iw_ap_encryption_set_api(iw_ap_lookup_api(ifp), encryption);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ap_encryption_method,
		no_ap_encryption_method_cmd,
		"no encryption-method",
		NO_STR
		"encryption method configure\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	iw_encryption_t encryption = IW_ALGO_AUTO;
	if(ifp && iw_ap_lookup_api(ifp))
	{
		if(iw_ap_lookup_api(ifp))
			ret = iw_ap_encryption_del_api(iw_ap_lookup_api(ifp), encryption);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}



DEFUN (ap_mac_acl,
		ap_mac_acl_cmd,
		"mac (permit|deny) "CMD_MAC_STR,
		"Mac configure\n"
		"Permit configure\n"
		"Deny configure\n"
		CMD_MAC_STR_HELP)
{
	int ret = ERROR;
	u_int8 mac[8];
	BOOL accept = FALSE;
	struct interface *ifp = vty->index;
	if(ifp && (nsm_iw_mode(ifp) == IW_MODE_AP))
	{
		memset(mac, 0, sizeof(mac));
		VTY_IMAC_GET(argv[1], mac);
		if(strstr(argv[0], "permit"))
			accept = TRUE;
		else
			accept = FALSE;
		if(iw_ap_lookup_api(ifp))
		{
			if(!iw_ap_mac_lookup_api(iw_ap_lookup_api(ifp), mac, accept))
				ret = iw_ap_mac_add_api(iw_ap_lookup_api(ifp), mac, accept);
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ap_mac_acl,
		no_ap_mac_acl_cmd,
		"no mac (permit|deny) "CMD_MAC_STR,
		NO_STR
		"Mac configure\n"
		"Permit configure\n"
		"Deny configure\n"
		CMD_MAC_STR_HELP)
{
	int ret = ERROR;
	u_int8 mac[8];
	BOOL accept = FALSE;
	struct interface *ifp = vty->index;
	if(ifp && (nsm_iw_mode(ifp) == IW_MODE_AP))
	{
		memset(mac, 0, sizeof(mac));
		VTY_IMAC_GET(argv[1], mac);
		if(strstr(argv[0], "permit"))
			accept = TRUE;
		else
			accept = FALSE;
		if(iw_ap_lookup_api(ifp))
		{
			if(iw_ap_mac_lookup_api(iw_ap_lookup_api(ifp), mac, accept))
				ret = iw_ap_mac_del_api(iw_ap_lookup_api(ifp), mac, accept);
			else
				ret = OK;
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (signal_level,
		signal_level_cmd,
		"signal-level (<10-100>|off|auto)",//NdBm|off|auto
		"signal configure\n"
		"signal value(dBm)\n"
		"turn off\n"
		"auto value\n")
{
	int ret = ERROR;
	int interval = 0;
	struct interface *ifp = vty->index;
	if(ifp && nsm_iw_mode(ifp) == IW_MODE_AP)
	{
		if(strstr(argv[0], "auto"))
			interval = IW_VALUE_AUTO;
		else if(strstr(argv[0], "off"))
			interval = IW_VALUE_OFF;
		else
			interval = atoi(argv[0]);
		if(iw_ap_lookup_api(ifp))
			ret = iw_ap_signal_set_api(iw_ap_lookup_api(ifp), interval);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_signal_level,
		no_signal_level_cmd,
		"no signal-level",
		NO_STR
		"signal configure\n")
{
	int ret = ERROR;
	int interval = IW_VALUE_AUTO;
	struct interface *ifp = vty->index;
	if(ifp && nsm_iw_mode(ifp) == IW_MODE_AP)
	{
		if(iw_ap_lookup_api(ifp))
			ret = iw_ap_signal_set_api(iw_ap_lookup_api(ifp), interval);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ap_bitrate,
		ap_bitrate_cmd,
		"bitrate (<16-1000000000>|auto|fixed)",
		"bitrate configure\n"
		"bitrate value(Kbps)\n"
		"auto value\n"
		"fixed value\n")
{
	int ret = ERROR;
	int interval = IW_VALUE_AUTO;
	struct interface *ifp = vty->index;
	if(ifp && nsm_iw_mode(ifp) == IW_MODE_AP)
	{
		if(strstr(argv[0], "auto"))
			interval = IW_VALUE_AUTO;
		else if(strstr(argv[0], "fixed"))
			interval = IW_VALUE_FIEXD;
		else
			interval = atoi(argv[0]);
		if(iw_ap_lookup_api(ifp))
			ret = iw_ap_bitrate_set_api(iw_ap_lookup_api(ifp), interval);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ap_bitrate,
		no_ap_bitrate_cmd,
		"no bitrate",
		NO_STR
		"bitrate configure\n")
{
	int ret = ERROR;
	int interval = IW_VALUE_AUTO;
	struct interface *ifp = vty->index;
	if(ifp && nsm_iw_mode(ifp) == IW_MODE_AP)
	{
		if(iw_ap_lookup_api(ifp))
			ret = iw_ap_bitrate_set_api(iw_ap_lookup_api(ifp), interval);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}



DEFUN (ap_threshold_rts,
		ap_threshold_rts_cmd,
		"threshold rts (<16-1000>|auto|fixed|off)",
		"threshold configure\n"
		"rts configure\n"
		"rts value(Kbps)\n"
		"auto value\n"
		"fixed value\n"
		"turn off\n")
{
	int ret = ERROR;
	int interval = IW_VALUE_AUTO;
	struct interface *ifp = vty->index;
	if(ifp && nsm_iw_mode(ifp) == IW_MODE_AP)
	{
		if(strstr(argv[0], "auto"))
			interval = IW_VALUE_AUTO;
		else if(strstr(argv[0], "fixed"))
			interval = IW_VALUE_FIEXD;
		else if(strstr(argv[0], "off"))
			interval = IW_VALUE_OFF;
		else
			interval = atoi(argv[0]);
		if(iw_ap_lookup_api(ifp))
			ret = iw_ap_rts_threshold_set_api(iw_ap_lookup_api(ifp), interval);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ap_threshold_rts,
		no_ap_threshold_rts_cmd,
		"no threshold rts",
		NO_STR
		"threshold configure\n"
		"rts configure\n")
{
	int ret = ERROR;
	int interval = IW_VALUE_AUTO;
	struct interface *ifp = vty->index;
	if(ifp && nsm_iw_mode(ifp) == IW_MODE_AP)
	{
		if(iw_ap_lookup_api(ifp))
			ret = iw_ap_rts_threshold_set_api(iw_ap_lookup_api(ifp), interval);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ap_threshold_frame,
		ap_threshold_frame_cmd,
		"threshold frame (<64-1564>|auto|fixed|off)",
		"threshold configure\n"
		"frame configure\n"
		"frame value(Byte)\n"
		"auto value\n"
		"fixed value\n"
		"turn off\n")
{
	int ret = ERROR;
	int interval = IW_VALUE_AUTO;
	struct interface *ifp = vty->index;
	if(ifp && nsm_iw_mode(ifp) == IW_MODE_AP)
	{
		if(strstr(argv[0], "auto"))
			interval = IW_VALUE_AUTO;
		else if(strstr(argv[0], "fixed"))
			interval = IW_VALUE_FIEXD;
		else if(strstr(argv[0], "off"))
			interval = IW_VALUE_OFF;
		else
			interval = atoi(argv[0]);
		if(iw_ap_lookup_api(ifp))
			ret = iw_ap_frag_set_api(iw_ap_lookup_api(ifp), interval);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ap_threshold_frame,
		no_ap_threshold_frame_cmd,
		"no threshold frame",
		NO_STR
		"threshold configure\n"
		"frame configure\n")
{
	int ret = ERROR;
	int interval = IW_VALUE_AUTO;
	struct interface *ifp = vty->index;
	if(ifp && nsm_iw_mode(ifp) == IW_MODE_AP)
	{
		if(iw_ap_lookup_api(ifp))
			ret = iw_ap_frag_set_api(iw_ap_lookup_api(ifp), interval);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ap_isolate_enable,
		ap_isolate_enable_cmd,
		"isolate enable",
		"isolate configure\n"
		"enable configure\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(ifp && nsm_iw_mode(ifp) == IW_MODE_AP)
	{
		if(iw_ap_lookup_api(ifp))
			ret = iw_ap_isolate_set_api(iw_ap_lookup_api(ifp), TRUE);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ap_isolate_enable,
		no_ap_isolate_enable_cmd,
		"no isolate enable",
		NO_STR
		"isolate configure\n"
		"enable configure\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(ifp && nsm_iw_mode(ifp) == IW_MODE_AP)
	{
		if(iw_ap_lookup_api(ifp))
			ret = iw_ap_isolate_set_api(iw_ap_lookup_api(ifp), FALSE);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ap_wmm_enable,
		ap_wmm_enable_cmd,
		"wmm-enable",
		"wmm-enable configure\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(ifp && nsm_iw_mode(ifp) == IW_MODE_AP)
	{
		if(iw_ap_lookup_api(ifp))
			ret = iw_ap_wmm_set_api(iw_ap_lookup_api(ifp), TRUE);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ap_wmm_enable,
		no_ap_wmm_enable_cmd,
		"no wmm-enable",
		NO_STR
		"wmm-enable configure\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(ifp && nsm_iw_mode(ifp) == IW_MODE_AP)
	{
		if(iw_ap_lookup_api(ifp))
			ret = iw_ap_wmm_set_api(iw_ap_lookup_api(ifp), FALSE);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (ap_country,
		ap_country_cmd,
		"country-code <0-256>",
		"country-code configure\n"
		"Code value")
{
	int ret = ERROR;
	int value = 0;
	struct interface *ifp = vty->index;
	if(ifp && nsm_iw_mode(ifp) == IW_MODE_AP)
	{
		value = atoi(argv[0]);
		if(iw_ap_lookup_api(ifp))
			ret = iw_ap_country_set_api(iw_ap_lookup_api(ifp), value);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ap_country,
		no_ap_country_cmd,
		"no country-code",
		NO_STR
		"country-code configure\n")
{
	int ret = ERROR;
	int value = 0;
	struct interface *ifp = vty->index;
	if(ifp && nsm_iw_mode(ifp) == IW_MODE_AP)
	{
		if(iw_ap_lookup_api(ifp))
			ret = iw_ap_country_set_api(iw_ap_lookup_api(ifp), value);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (ap_accept_client_num,
		ap_accept_client_num_cmd,
		"accept client <0-256>",
		"accept configure\n"
		"client configure\n"
		"number value")
{
	int ret = ERROR;
	int value = 0;
	struct interface *ifp = vty->index;
	if(ifp && nsm_iw_mode(ifp) == IW_MODE_AP)
	{
		value = atoi(argv[0]);
		if(iw_ap_lookup_api(ifp))
			ret = iw_ap_client_num_set_api(iw_ap_lookup_api(ifp), value);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_ap_accept_client_num,
		no_ap_accept_client_num_cmd,
		"no accept client",
		NO_STR
		"accept configure\n"
		"client configure\n")
{
	int ret = ERROR;
	int value = 0;
	struct interface *ifp = vty->index;
	if(ifp && nsm_iw_mode(ifp) == IW_MODE_AP)
	{
		if(iw_ap_lookup_api(ifp))
			ret = iw_ap_client_num_set_api(iw_ap_lookup_api(ifp), value);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


/*
 * client
 */
/*
DEFUN (ap_name,
		ap_name_pass_cmd,
		"ap-name NAME password PASS",
		"AP Name configure\n"
		"Name string\n"
		"AP Password configure\n"
		"Password string\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(ifp && iw_client_lookup_api(ifp))
	{
		if((nsm_iw_mode(ifp) == IW_MODE_MANAGE || nsm_iw_mode(ifp) == IW_MODE_CLIENT))
			ret = iw_client_db_set_api(iw_client_lookup_api(ifp), argv[0], (argc == 2) ? argv[1]:NULL);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS (ap_name,
		ap_name_cmd,
		"ap-name NAME",
		"AP Name configure\n"
		"Name string\n");

DEFUN (no_ap_name,
		no_ap_name_cmd,
		"no ap-name NAME",
		NO_STR
		"AP Name configure\n"
		"Name string\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(ifp && (nsm_iw_mode(ifp) == IW_MODE_MANAGE || nsm_iw_mode(ifp) == IW_MODE_CLIENT))
	{
		if(iw_client_lookup_api(ifp))
			ret = iw_client_db_del_api(iw_client_lookup_api(ifp), argv[0]);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
*/


DEFUN (scan_interval,
		scan_interval_cmd,
		"scan-interval <10-100>",
		"scan configure\n"
		"interval value\n")
{
	int ret = ERROR;
	int interval = 0;
	struct interface *ifp = vty->index;
	if(ifp && (nsm_iw_mode(ifp) == IW_MODE_MANAGE || nsm_iw_mode(ifp) == IW_MODE_CLIENT))
	{
		if(strstr(argv[0], "auto"))
			interval = 0;
		else
			interval = atoi(argv[0]);
		if(iw_ap_lookup_api(ifp))
			ret = iw_client_scan_interval_api(iw_ap_lookup_api(ifp), interval);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_scan_interval,
		no_scan_interval_cmd,
		"no scan-interval",
		NO_STR
		"scan configure\n")
{
	int ret = ERROR;
	int interval = 0;
	struct interface *ifp = vty->index;
	if(ifp && (nsm_iw_mode(ifp) == IW_MODE_MANAGE || nsm_iw_mode(ifp) == IW_MODE_CLIENT))
	{
		if(iw_client_lookup_api(ifp))
			ret = iw_client_scan_interval_api(iw_client_lookup_api(ifp), interval);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (scan_num,
		scan_num_cmd,
		"scan-num <1-512>",
		"scan configure\n"
		"number value\n")
{
	int ret = ERROR;
	int interval = 0;
	struct interface *ifp = vty->index;
	if(ifp && (nsm_iw_mode(ifp) == IW_MODE_MANAGE || nsm_iw_mode(ifp) == IW_MODE_CLIENT))
	{
		if(strstr(argv[0], "auto"))
			interval = 0;
		else
			interval = atoi(argv[0]);
		if(iw_ap_lookup_api(ifp))
			ret = iw_client_scan_max_api(iw_ap_lookup_api(ifp), interval);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_scan_num,
		no_scan_num_cmd,
		"no scan-num",
		NO_STR
		"scan configure\n")
{
	int ret = ERROR;
	int interval = 0;
	struct interface *ifp = vty->index;
	if(ifp && (nsm_iw_mode(ifp) == IW_MODE_MANAGE || nsm_iw_mode(ifp) == IW_MODE_CLIENT))
	{
		if(iw_client_lookup_api(ifp))
			ret = iw_client_scan_max_api(iw_client_lookup_api(ifp), interval);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

static int iw_dev_show_cmd(struct vty *vty, int ifindex, char *cmd)
{
	struct listnode *node = NULL;
	struct interface *ifp = NULL;
	struct list *if_list = NULL;
	if (ifindex)
	{
		ifp = if_lookup_by_index(ifindex);
		if (ifp && if_is_wireless(ifp))
		{
			if(!nsm_iw_enable_get_api(ifp))
			{
				vty_out(vty, " wireless is not enable%s", VTY_NEWLINE);
				return OK;
			}
			vty_out(vty, "%s", VTY_NEWLINE);
			if(strstr(cmd, "channel"))
				nsm_iw_channel_freq_show(ifp, vty);
			else if(strstr(cmd, "freq"))
				nsm_iw_channel_freq_show(ifp, vty);
			else if(strstr(cmd, "info"))
				nsm_iw_ap_info_show(ifp, vty);
			else if(strstr(cmd, "capab"))
				nsm_iw_capabilities_show(ifp, vty);
			return OK;
		}
		return OK;
	}
	if_list = if_list_get();
	if (if_list)
	{
		for (ALL_LIST_ELEMENTS_RO(if_list, node, ifp))
		{
			if (ifp && if_is_wireless(ifp))
			{
				if(!nsm_iw_enable_get_api(ifp))
				{
					vty_out(vty, " wireless is not enable%s", VTY_NEWLINE);
					continue;
				}
				vty_out(vty, "%s", VTY_NEWLINE);
				if(strstr(cmd, "channel"))
					nsm_iw_channel_freq_show(ifp, vty);
				else if(strstr(cmd, "freq"))
					nsm_iw_channel_freq_show(ifp, vty);
				else if(strstr(cmd, "info"))
					nsm_iw_ap_info_show(ifp, vty);
				else if(strstr(cmd, "capab"))
					nsm_iw_capabilities_show(ifp, vty);
			}
		}
		vty_out(vty, "%s", VTY_NEWLINE);
	}
	return OK;
}

static int iw_client_show_cmd(struct vty *vty, int ifindex, int cmd)
{
	struct listnode *node = NULL;
	struct interface *ifp = NULL;
	struct list *if_list = NULL;
	if (ifindex)
	{
		ifp = if_lookup_by_index(ifindex);
		if (ifp && if_is_wireless(ifp))
		{
			if(!nsm_iw_enable_get_api(ifp))
			{
				vty_out(vty, " wireless is not enable%s", VTY_NEWLINE);
				return OK;
			}
			if (iw_client_lookup_api(ifp))
			{
				vty_out(vty, "%s", VTY_NEWLINE);
				if(strstr(cmd, "neighbor"))
					iw_client_neighbor_show(iw_client_lookup_api(ifp), vty, TRUE);
				else if(strstr(cmd, "connect"))
					iw_client_connect_ap_show(iw_client_lookup_api(ifp), vty);
				else if(strstr(cmd, "scan"))
					iw_client_scan_ap_show(iw_client_lookup_api(ifp), vty);
				else if(strstr(cmd, "station"))
					iw_client_station_dump_show(iw_client_lookup_api(ifp), vty);
			}
		}
		return OK;
	}
	if_list = if_list_get();
	if (if_list)
	{
		for (ALL_LIST_ELEMENTS_RO(if_list, node, ifp))
		{
			if (ifp && if_is_wireless(ifp))
			{
				if(!nsm_iw_enable_get_api(ifp))
				{
					vty_out(vty, " wireless is not enable%s", VTY_NEWLINE);
					continue;
				}
				if (iw_client_lookup_api(ifp))
				{
					vty_out(vty, "%s", VTY_NEWLINE);
					if(strstr(cmd, "neighbor"))//显示附近wifi
						iw_client_neighbor_show(iw_client_lookup_api(ifp), vty, TRUE);
					else if(strstr(cmd, "connect"))
						iw_client_connect_ap_show(iw_client_lookup_api(ifp), vty);
					else if(strstr(cmd, "scan"))
						iw_client_scan_ap_show(iw_client_lookup_api(ifp), vty);
					else if(strstr(cmd, "station"))
						iw_client_station_dump_show(iw_client_lookup_api(ifp), vty);
				}
			}
		}
		vty_out(vty, "%s", VTY_NEWLINE);
	}
	return OK;
}
//显示当前连接到AP的设备
static int iw_ap_show_cmd(struct vty *vty, int ifindex, BOOL detail)
{
	struct listnode *node = NULL;
	struct interface *ifp = NULL;
	struct list *if_list = NULL;
	if(ifindex)
	{
		ifp = if_lookup_by_index(ifindex);
		if(ifp && if_is_wireless(ifp))
		{
			if(!nsm_iw_enable_get_api(ifp))
			{
				vty_out(vty, " wireless is not enable%s", VTY_NEWLINE);
				return OK;
			}
			if(iw_ap_lookup_api(ifp))
			{
				vty_out(vty, "%s", VTY_NEWLINE);
				iw_ap_connect_show(iw_ap_lookup_api(ifp), vty, detail);
			}
		}
		return OK;
	}
	if_list = if_list_get();
	if (if_list)
	{
		for (ALL_LIST_ELEMENTS_RO(if_list, node, ifp))
		{
			if(ifp && if_is_wireless(ifp))
			{
				if(!nsm_iw_enable_get_api(ifp))
				{
					vty_out(vty, " wireless is not enable%s", VTY_NEWLINE);
					continue;
				}
				if(iw_ap_lookup_api(ifp))
				{
					vty_out(vty, "%s", VTY_NEWLINE);
					iw_ap_connect_show(iw_ap_lookup_api(ifp), vty, detail);
				}
			}
		}
		vty_out(vty, "%s", VTY_NEWLINE);
	}
	return OK;
}



DEFUN (show_wireless_neighbor,
		show_wireless_neighbor_cmd,
		"show wireless neighbor",
		SHOW_STR
		"Wireless configure\n"
		"neighbor information\n")
{
	ifindex_t ifindex = 0;
	if(argc)
		ifindex = if_ifindex_make("wireless", argv[0]);
	iw_client_show_cmd(vty, ifindex, "neighbor");
	return CMD_SUCCESS;
}

ALIAS (show_wireless_neighbor,
		show_wireless_neighbor_interface_cmd,
		"show wireless " CMD_USP_STR " neighbor" ,
		SHOW_STR
		"Wireless Interface\n"
		"neighbor information\n"
		"Wireless interface\n"
		CMD_USP_STR_HELP);


DEFUN (show_wireless_connect,
		show_wireless_connect_cmd,
		"show wireless connect",
		SHOW_STR
		"Wireless configure\n"
		"connect information\n")
{
	ifindex_t ifindex = 0;
	if(argc == 2)
	{
		ifindex = if_ifindex_make("wireless", argv[0]);
		iw_client_show_cmd(vty, ifindex, "connect");
	}
	else if(argc == 1)
	{
		if(strstr(argv[0], "station"))
			iw_client_show_cmd(vty, ifindex, "station");
		else
		{
			ifindex = if_ifindex_make("wireless", argv[0]);
			iw_client_show_cmd(vty, ifindex, "connect");
		}
	}
	else
		iw_client_show_cmd(vty, ifindex, "connect");

	return CMD_SUCCESS;
}

ALIAS (show_wireless_connect,
		show_wireless_connect_station_cmd,
		"show wireless connect (station-dump|)",
		SHOW_STR
		"Wireless configure\n"
		"connect information\n"
		"Station information\n");

ALIAS (show_wireless_connect,
		show_wireless_connect_station_interface_cmd,
		"show wireless " CMD_USP_STR " connect (station-dump|)" ,
		SHOW_STR
		"Wireless interface\n"
		CMD_USP_STR_HELP
		"connect information\n"
		"Station information\n");

ALIAS (show_wireless_connect,
		show_wireless_connect_interface_cmd,
		"show wireless " CMD_USP_STR " connect" ,
		SHOW_STR
		"Wireless interface\n"
		CMD_USP_STR_HELP
		"connect information\n");

DEFUN (scan_wireless_neighbor,
		scan_wireless_neighbor_cmd,
		"scan wireless neighbor",
		"scanning\n"
		"Wireless configure\n"
		"neighbor information\n")
{
	ifindex_t ifindex = 0;
	if(argc)
		ifindex = if_ifindex_make("wireless", argv[0]);
	iw_client_show_cmd(vty, ifindex, "scan");
	return CMD_SUCCESS;
}

ALIAS (scan_wireless_neighbor,
		scan_wireless_neighbor_interface_cmd,
		"scan wireless " CMD_USP_STR " neighbor",
		"scanning\n"
		"Wireless configure\n"
		CMD_USP_STR_HELP
		"neighbor information\n");


DEFUN (show_wireless_info,
		show_wireless_info_cmd,
		"show wireless (freq|channel|capabilities)",
		SHOW_STR
		"Wireless configure\n"
		"Freq information\n"
		"Channel information\n"
		"Capabilities information\n")
{
	ifindex_t ifindex = 0;
	if(argc == 2)
	{
		ifindex = if_ifindex_make("wireless", argv[0]);
		iw_dev_show_cmd(vty, ifindex, argv[1]);
	}
	else if(argc == 1)
	{
		if(strncmp(argv[0], "freq", 3) == 0)
			iw_dev_show_cmd(vty, ifindex, argv[0]);
		else if(strncmp(argv[0], "channel", 3) == 0)
			iw_dev_show_cmd(vty, ifindex, argv[0]);
		else if(strncmp(argv[0], "capabilities", 3) == 0)
			iw_dev_show_cmd(vty, ifindex, argv[0]);
		else
		{
			ifindex = if_ifindex_make("wireless", argv[0]);
			iw_dev_show_cmd(vty, ifindex, "info");
		}
	}
	else
		iw_dev_show_cmd(vty, ifindex, "info");

	return CMD_SUCCESS;
}

ALIAS (show_wireless_info,
		show_wireless_info_detail_cmd,
		"show wireless",
		SHOW_STR
		"Wireless interface\n");

ALIAS (show_wireless_info,
		show_wireless_info_interface_cmd,
		"show wireless " CMD_USP_STR,
		SHOW_STR
		"Wireless interface\n"
		CMD_USP_STR_HELP);

ALIAS (show_wireless_info,
		show_wireless_info_interface_detail_cmd,
		"show wireless " CMD_USP_STR " (freq|channel|capabilities)",
		SHOW_STR
		"Wireless interface\n"
		CMD_USP_STR_HELP
		"Freq information\n"
		"Channel information\n"
		"Capabilities information\n");


DEFUN (show_wireless_client,
		show_wireless_client_cmd,
		"show wireless client",
		SHOW_STR
		"Wireless configure\n"
		"client information\n")
{
	ifindex_t ifindex = 0;
	if(argc == 2)
	{
		ifindex = if_ifindex_make("wireless", argv[0]);
		iw_ap_show_cmd(vty, ifindex, TRUE);
	}
	else if(argc == 1)
	{
		if(strncmp(argv[0], "detail", 3) == 0)
			iw_ap_show_cmd(vty, ifindex, TRUE);
		else
		{
			ifindex = if_ifindex_make("wireless", argv[0]);
			iw_ap_show_cmd(vty, ifindex, FALSE);
		}
	}
	else
		iw_ap_show_cmd(vty, ifindex, FALSE);

	return CMD_SUCCESS;
}

ALIAS (show_wireless_client,
		show_wireless_client_detail_cmd,
		"show wireless client (detail|)",
		SHOW_STR
		"Wireless configure\n"
		"Client information\n"
		"Detail information\n");

ALIAS (show_wireless_client,
		show_wireless_client_interface_cmd,
		"show wireless " CMD_USP_STR " client" ,
		SHOW_STR
		"Wireless Interface\n"
		CMD_USP_STR_HELP
		"Client information\n");

ALIAS (show_wireless_client,
		show_wireless_client_interface_detail_cmd,
		"show wireless " CMD_USP_STR " client (detail|)" ,
		SHOW_STR
		"Wireless Interface\n"
		CMD_USP_STR_HELP
		"Client information\n"
		"Detail information\n");
/*
dtim-interval dtim-interval
ap-auth-mode no-auth
ap-blacklist mac ap-mac1 [ to ap-mac2 ]
ap-whitelist mac ap-mac1 [ to ap-mac2 ]
multicast rate { rate_1 | rate_2 | rate_5_5 | rate_6 | rate_9 | rate_11 | rate_12 | rate_18 | rate_24 | rate_36 | rate_48 | rate_54 }
security-policy wpa2
wpa2 authentication-method psk { pass-phrase | hex } cipher cipher-key encryption-method ccmp
*/

DEFUN (debug_wireless_set,
		debug_wireless_set_cmd,
		"debug wireless (event|db|scan|ap|ap-accept)",
		DEBUG_STR
		"Wireless configure\n"
		"Event information\n"
		"DB information\n"
		"Scan information\n"
		"AP information\n"
		"AP-Accept information\n")
{
	if (strstr(argv[0], "event"))
		IW_DEBUG_ON(EVENT);
	else if (strstr(argv[0], "db"))
		IW_DEBUG_ON(DB);
	else if (strstr(argv[0], "scan"))
		IW_DEBUG_ON(SCAN);
	else if (strstr(argv[0], "ap"))
		IW_DEBUG_ON(AP);
	else if (strstr(argv[0], "accept"))
		IW_DEBUG_ON(AP_ACCEPT);
	if (argc == 2 && strstr(argv[1], "detail"))
		IW_DEBUG_ON(DETAIL);
	return CMD_SUCCESS;
}

ALIAS (debug_wireless_set,
		debug_wireless_set_detail_cmd,
		"debug wireless (event|db|scan|ap|ap-accept) (detail|)",
		DEBUG_STR
		"Wireless configure\n"
		"Event information\n"
		"DB information\n"
		"Scan information\n"
		"AP information\n"
		"AP-Accept information\n");


DEFUN (no_debug_wireless_set,
		no_debug_wireless_set_cmd,
		"no debug wireless (event|db|scan|ap|ap-accept)",
		NO_STR
		DEBUG_STR
		"Wireless configure\n"
		"Event information\n"
		"DB information\n"
		"Scan information\n"
		"AP information\n"
		"AP-Accept information\n")
{
	if (strstr(argv[0], "event"))
		IW_DEBUG_OFF(EVENT);
	else if (strstr(argv[0], "db"))
		IW_DEBUG_OFF(DB);
	else if (strstr(argv[0], "scan"))
		IW_DEBUG_OFF(SCAN);
	else if (strstr(argv[0], "ap"))
		IW_DEBUG_OFF(AP);
	else if (strstr(argv[0], "accept"))
		IW_DEBUG_OFF(AP_ACCEPT);
	if (argc == 2 && strstr(argv[1], "detail"))
		IW_DEBUG_OFF(DETAIL);
	return CMD_SUCCESS;
}

ALIAS (no_debug_wireless_set,
		no_debug_wireless_set_detail_cmd,
		"no debug wireless (event|db|scan|ap|ap-accept) (detail|)",
		NO_STR
		DEBUG_STR
		"Wireless configure\n"
		"Event information\n"
		"DB information\n"
		"Scan information\n"
		"AP information\n"
		"AP-Accept information\n");


static void cmd_base_wireless_show_init(int node)
{
	install_element(node, &show_wireless_neighbor_cmd);
	install_element(node, &show_wireless_neighbor_interface_cmd);

	install_element(node, &show_wireless_connect_cmd);
	install_element(node, &show_wireless_connect_station_cmd);
	install_element(node, &show_wireless_connect_interface_cmd);
	install_element(node, &show_wireless_connect_station_interface_cmd);

	install_element(node, &scan_wireless_neighbor_cmd);
	install_element(node, &scan_wireless_neighbor_interface_cmd);


	install_element(node, &show_wireless_info_cmd);
	install_element(node, &show_wireless_info_detail_cmd);
	install_element(node, &show_wireless_info_interface_cmd);
	install_element(node, &show_wireless_info_interface_detail_cmd);

	install_element(node, &show_wireless_client_cmd);
	install_element(node, &show_wireless_client_detail_cmd);
	install_element(node, &show_wireless_client_interface_cmd);
	install_element(node, &show_wireless_client_interface_detail_cmd);
}

static void cmd_base_wireless_init(int node)
{

	install_element(node, &wireless_enable_cmd);
	install_element(node, &no_wireless_enable_cmd);

	install_element(node, &work_mode_cmd);
	install_element(node, &no_work_mode_cmd);

	install_element(node, &radio_type_cmd);
	install_element(node, &no_radio_type_cmd);

	install_element(node, &work_channel_cmd);
	install_element(node, &no_work_channel_cmd);

	install_element(node, &power_level_cmd);
	install_element(node, &no_power_level_cmd);

	install_element(node, &beacon_interval_cmd);
	install_element(node, &no_beacon_interval_cmd);

	install_element(node, &netwotrk_name_cmd);
	install_element(node, &no_netwotrk_name_cmd);

	install_element(node, &authentication_password_cmd);
	install_element(node, &no_authentication_password_cmd);

	install_element(node, &ap_authentication_method_cmd);
	install_element(node, &no_ap_authentication_method_cmd);

	install_element(node, &ap_encryption_method_cmd);
	install_element(node, &no_ap_encryption_method_cmd);

	install_element(node, &ap_mac_acl_cmd);
	install_element(node, &no_ap_mac_acl_cmd);

	install_element(node, &signal_level_cmd);
	install_element(node, &no_signal_level_cmd);

	install_element(node, &ap_bitrate_cmd);
	install_element(node, &no_ap_bitrate_cmd);

	install_element(node, &ap_threshold_rts_cmd);
	install_element(node, &no_ap_threshold_rts_cmd);

	install_element(node, &ap_threshold_frame_cmd);
	install_element(node, &no_ap_threshold_frame_cmd);


	install_element(node, &ap_isolate_enable_cmd);
	install_element(node, &no_ap_isolate_enable_cmd);

	install_element(node, &ap_wmm_enable_cmd);
	install_element(node, &no_ap_wmm_enable_cmd);

	install_element(node, &ap_country_cmd);
	install_element(node, &no_ap_country_cmd);

	install_element(node, &ap_accept_client_num_cmd);
	install_element(node, &no_ap_accept_client_num_cmd);

/*	install_element(node, &ap_name_pass_cmd);
	install_element(node, &ap_name_cmd);
	install_element(node, &no_ap_name_cmd);*/

	install_element(node, &scan_interval_cmd);
	install_element(node, &no_scan_interval_cmd);

	install_element(node, &scan_num_cmd);
	install_element(node, &no_scan_num_cmd);
}

void cmd_wireless_init(void)
{
	cmd_base_wireless_init(WIRELESS_INTERFACE_NODE);
	cmd_base_wireless_show_init(ENABLE_NODE);
	cmd_base_wireless_show_init(CONFIG_NODE);
	cmd_base_wireless_show_init(WIRELESS_INTERFACE_NODE);

	install_element(ENABLE_NODE, &debug_wireless_set_cmd);
	install_element(ENABLE_NODE, &debug_wireless_set_detail_cmd);

	install_element(ENABLE_NODE, &no_debug_wireless_set_cmd);
	install_element(ENABLE_NODE, &no_debug_wireless_set_detail_cmd);

	install_element(CONFIG_NODE, &debug_wireless_set_cmd);
	install_element(CONFIG_NODE, &debug_wireless_set_detail_cmd);

	install_element(CONFIG_NODE, &no_debug_wireless_set_cmd);
	install_element(CONFIG_NODE, &no_debug_wireless_set_detail_cmd);
}
#endif
