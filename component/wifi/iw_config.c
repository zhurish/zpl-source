/*
 * iw_config.c
 *
 *  Created on: Oct 11, 2018
 *      Author: zhurish
 */
#include "zebra.h"
#include "iw_config.h"


int	iw_debug_conf = IW_DEBUG_EVENT|IW_DEBUG_DB|IW_DEBUG_SCAN|IW_DEBUG_AP|IW_DEBUG_AP_ACCEPT|IW_DEBUG_DETAIL;

char * iw_mode_string(iw_mode_t mode)
{
	switch(mode)
	{
	case IW_MODE_IBSS:
		return "ibss";
		break;
	case IW_MODE_MANAGE:
		return "manage";
		break;
	case IW_MODE_AP:
		return "ap";
		break;
	case IW_MODE_MONITOR:
		return "monitor";
		break;
	case IW_MODE_MESH:
		return "mesh";
		break;
	case IW_MODE_WDS:
		return "wds";
		break;

/*	case IW_MODE_CLIENT:
		return "client";
		break;*/
	default:
		return "Unknown";
		break;
	}
	return "Unknown";
}

char * iw_hw_mode_string(iw_hw_mode_t mode)
{
	switch(mode)
	{
	case IW_HW_MODE_IEEE80211ANY:
		return "ieee80211any";
		break;
	case IW_HW_MODE_IEEE80211A:
		return "ieee80211a";
		break;
	case IW_HW_MODE_IEEE80211B:
		return "ieee80211b";
		break;
	case IW_HW_MODE_IEEE80211D:
		return "ieee80211d";
		break;
	case IW_HW_MODE_IEEE80211H:
		return "ieee80211h";
		break;
	case IW_HW_MODE_IEEE80211G:
		return "ieee80211g";
		break;
	case IW_HW_MODE_IEEE80211AD:
		return "ieee80211ad";
		break;
	case IW_HW_MODE_IEEE80211AC:
		return "ieee80211ac";
		break;
	case IW_HW_MODE_IEEE80211N:
		return "ieee80211n";
		break;
	case IW_HW_MODE_IEEE80211AB:
		return "ieee80211ab";
		break;
	case IW_HW_MODE_IEEE80211AG:
		return "ieee80211ag";
		break;
	case IW_HW_MODE_IEEE80211AN:
		return "ieee80211an";
		break;
	case IW_HW_MODE_IEEE80211BG:
		return "ieee80211bg";
		break;
	case IW_HW_MODE_IEEE80211NG:
		return "ieee80211ng";
		break;
	case IW_HW_MODE_IEEE80211BGN:
		return "ieee80211bgn";
		break;
	default:
		return "Unknown";
		break;
	}
	return "Unknown";
}


char * iw_auth_string(iw_authentication_t mode)
{
	switch(mode)
	{
	case IW_ENCRY_NONE:
		return "none";
		break;
	case IW_ENCRY_WEP_OPEN:
		return "open";
		break;
	case IW_ENCRY_WEP_PRIVATE:
		return "shared";
		break;
	case IW_ENCRY_WPA_PSK:
		return "wpa-psk";
		break;
	case IW_ENCRY_WPA2_PSK:
		return "wpa2-psk";
		break;
	case IW_ENCRY_WPA2WPA_PSK:
		return "wpa2wpa-psk";
		break;
	default:
		return "Unknown";
		break;
	}
	return "Unknown";
}

char * iw_encryption_string(iw_encryption_t mode)
{
	switch(mode)
	{
	case IW_ALGO_NONE:
		return "none";
		break;
	case IW_ALGO_WEP:
		return "wep";
		break;
	case IW_ALGO_AUTO:
		return "auto";
		break;
	case IW_ALGO_CCMP:
		return "ccmp";
		break;
	case IW_ALGO_TKIP:
		return "tkip";
		break;
	case IW_ALGO_TKIP_CCMP:
		return "tkip-ccmp";
		break;
	case IW_ALGO_IGTK:
		return "igtk";
		break;
	case IW_ALGO_PMK:
		return "pmk";
		break;
	case IW_ALGO_GCMP:
		return "gcmp";
		break;
	case IW_ALGO_SMS4:
		return "sms4";
		break;
	case IW_ALGO_KRK:
		return "prk";
		break;
	case IW_ALGO_GCMP_256:
		return "gcmp-256";
		break;
	case IW_ALGO_CCMP_256:
		return "ccmp-256";
		break;
	case IW_ALGO_BIP_GMAC_128:
		return "gmac-128";
		break;
	case IW_ALGO_BIP_GMAC_256:
		return "gmac-256";
		break;
	case IW_ALGO_BIP_CMAC_256:
		return "cmac-256";
		break;
	default:
		return "Unknown";
		break;
	}
	return "Unknown";
}

/*
char * iw_network_string(iw_network_t mode)
{
	switch(mode)
	{
	case IW_NETWORK_11_B_G_N:
		return "80211bgn";
		break;
	case IW_NETWORK_11_B_G:
		return "80211bg";
		break;
	case IW_NETWORK_11_G:
		return "80211g";
		break;
	case IW_NETWORK_11_B:
		return "80211b";
		break;
	case IW_NETWORK_11_AN:
		return "80211an";
		break;
	case IW_NETWORK_11_GN:
		return "80211gn";
		break;
	case IW_NETWORK_11_N:
		return "80211n";
		break;
	default:
		return "Unknown";
		break;
	}
	return "Unknown";
}
*/
