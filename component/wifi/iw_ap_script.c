/*
 * iw_ap_script.c
 *
 *  Created on: Oct 14, 2018
 *      Author: zhurish
 */

#include "zebra.h"
#include "vty.h"
#include "if.h"

#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "vrf.h"
#include "interface.h"

#include "iw_config.h"
#include "iw_ap.h"
#include "iw_interface.h"


static int iw_ap_macacl_default_config(iw_ap_t *iw_ap, BOOL deny, FILE *fp)
{
	NODE index;
	iw_ap_mac_t *pstNode = NULL;
	LIST *list = NULL;
	if(!iw_ap)
		return ERROR;
	if(deny)
		list = iw_ap->dmac_list;
	else
		list = iw_ap->mac_list;
	if(!list)
		return ERROR;
	for(pstNode = (iw_ap_mac_t *)lstFirst(list);
			pstNode != NULL;  pstNode = (iw_ap_connect_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode)
		{
			fprintf(fp, "%02x:%02x:%02x:%02x:%02x:%02x\n", pstNode->MAC[0], pstNode->MAC[1],
					pstNode->MAC[2], pstNode->MAC[3], pstNode->MAC[4], pstNode->MAC[5]);
		}
	}
	return OK;
}

static int iw_ap_macacl_config(iw_ap_t *iw_ap, FILE *fp, int mode)
{
	FILE *mfp = NULL;
	char path[128];
	if(!iw_ap || !fp)
		return ERROR;
	//macaddr_acl：可选，指定MAC地址过滤规则，0表示除非在禁止列表否则允许，1表示除非在允许列表否则禁止，2表示使用外部RADIUS服务器
	if(mode == 0)
	{
		ifindex_t ifindex = ifindex2ifkernel(iw_ap->ifindex);
		memset(path, 0, sizeof(path));
		snprintf(path, sizeof(path), "%s/deny-maclist-%s", DAEMON_ENV_DIR, ifkernelindex2kernelifname(ifindex));
		fprintf(fp, "macaddr_acl=%d\n", mode);
		//指定禁止MAC列表文件所在
		fprintf(fp, "deny_mac_file=%s\n", path);
		fflush(fp);
		mfp = fopen(path, "w+");
		if(mfp)
		{
			iw_ap_macacl_default_config(iw_ap,  TRUE, mfp);
			fflush(mfp);
			fclose(mfp);
		}
		return OK;
	}
	else if(mode == 1)
	{
		ifindex_t ifindex = ifindex2ifkernel(iw_ap->ifindex);
		memset(path, 0, sizeof(path));
		snprintf(path, sizeof(path), "%s/accept-maclist-%s", DAEMON_ENV_DIR, ifkernelindex2kernelifname(ifindex));
		fprintf(fp, "macaddr_acl=%d\n", mode);
		//指定允许MAC列表文件所在
		fprintf(fp, "accept_mac_file=%s\n", path);
		fflush(fp);
		mfp = fopen(path, "w+");
		if(mfp)
		{
			iw_ap_macacl_default_config(iw_ap,  FALSE, mfp);
			fflush(mfp);
			fclose(mfp);
		}
		return OK;
	}
	else
		return ERROR;
}

static int iw_ap_hw_default_config(iw_ap_t *iw_ap, FILE *fp)
{
	if(!iw_ap && !fp)
		return ERROR;
	fprintf(fp, "\n#HW config\n");
	fprintf(fp, "driver=%s\n", iw_ap->driver ? "none":"nl80211");
	fprintf(fp, "ht_coex=0\n");
	fprintf(fp, "ht_capab=[SHORT-GI-20][SHORT-GI-40]\n");
	//fprintf(fp, "ht_capab=[SHORT-GI-20][SHORT-GI-40][TX-STBC][RX-STBC1]\n");
	fflush(fp);
	switch(iw_ap->hw_mode)
	{
	case IW_HW_MODE_IEEE80211ANY:
		fprintf(fp, "hw_mode=any\n");
		break;
	case IW_HW_MODE_IEEE80211A:
		fprintf(fp, "hw_mode=a\n");
		fprintf(fp, "ieee80211a=1\n");
		break;
	case IW_HW_MODE_IEEE80211B:
		fprintf(fp, "hw_mode=b\n");
		fprintf(fp, "ieee80211b=1\n");
		break;
	case IW_HW_MODE_IEEE80211D:
		//fprintf(fp, "hw_mode=b\n");
		fprintf(fp, "ieee80211d=1\n");
		break;
	case IW_HW_MODE_IEEE80211H:
		//Dfprintf(fp, "hw_mode=b\n");
		fprintf(fp, "ieee80211h=1\n");
		break;
	case IW_HW_MODE_IEEE80211G:
		fprintf(fp, "hw_mode=g\n");
		//fprintf(fp, "ieee80211g=1\n");
		fprintf(fp, "ieee80211d=1\n");
		fprintf(fp, "ieee80211n=1\n");
		break;
	case IW_HW_MODE_IEEE80211N:
		fprintf(fp, "hw_mode=a/g\n");
		fprintf(fp, "ieee80211n=1\n");
		break;

	case IW_HW_MODE_IEEE80211AD:
		fprintf(fp, "hw_mode=ad\n");
		break;
	case IW_HW_MODE_IEEE80211AC:
		fprintf(fp, "hw_mode=a\n");
		fprintf(fp, "ieee80211ac=1\n");
		break;
	case IW_HW_MODE_IEEE80211AB:
		fprintf(fp, "hw_mode=ab\n");
		break;
	case IW_HW_MODE_IEEE80211AG:
		fprintf(fp, "hw_mode=ag\n");
		break;
	case IW_HW_MODE_IEEE80211AN:
		fprintf(fp, "hw_mode=an\n");
		break;
	case IW_HW_MODE_IEEE80211BG:
		fprintf(fp, "hw_mode=bg\n");
		break;
	case IW_HW_MODE_IEEE80211NG:
		fprintf(fp, "hw_mode=ng\n");
		break;
	case IW_HW_MODE_IEEE80211BGN:
		fprintf(fp, "hw_mode=bgn\n");
		break;
	default:
		fprintf(fp, "hw_mode=any\n");
		break;
	}
	//IEEE 802.11ax related configuration
	//fprintf(fp, "ieee80211ax=1\n");

	//IEEE 802.1X-2004 related configuration
	//fprintf(fp, "ieee80211x=1\n");
/*
# ieee80211w: Whether management frame protection (MFP) is enabled
# 0 = disabled (default)
# 1 = optional
# 2 = required
#ieee80211w=0
*/
	fflush(fp);
	return OK;
}

static int iw_ap_auth_default_config(iw_ap_t *iw_ap, FILE *fp)
{
	if(!iw_ap && !fp)
		return ERROR;
	fprintf(fp, "\n#auth config\n");
	switch(iw_ap->auth)
	{
	case IW_ENCRY_NONE:
		fprintf(fp, "auth_algs=1\n");
		fprintf(fp, "wpa=0\n");
		fprintf(fp, "wpa_key_mgmt=WPA-PSK\n");
		break;
	case IW_ENCRY_WEP_OPEN:
		fprintf(fp, "auth_algs=1\n");
		fprintf(fp, "wpa=0\n");
		fprintf(fp, "wep_default_key=0\n");
		fprintf(fp, "wep_key0=%s\n", iw_ap->password[0].password);
		fprintf(fp, "wep_key1=%s\n", iw_ap->password[0].password);
		fprintf(fp, "wep_key2=%s\n", iw_ap->password[0].password);
		fprintf(fp, "wep_key3=%s\n", iw_ap->password[0].password);
		break;
	case IW_ENCRY_WEP_PRIVATE:
		fprintf(fp, "auth_algs=2\n");
		fprintf(fp, "wpa=0\n");
		fprintf(fp, "wep_default_key=0\n");
		fprintf(fp, "wep_key0=%s\n", iw_ap->password[0].password);
		fprintf(fp, "wep_key1=%s\n", iw_ap->password[0].password);
		fprintf(fp, "wep_key2=%s\n", iw_ap->password[0].password);
		fprintf(fp, "wep_key3=%s\n", iw_ap->password[0].password);
		break;
	case IW_ENCRY_WPA_PSK:
		fprintf(fp, "auth_algs=1\n");
		fprintf(fp, "wpa=1\n");
		fprintf(fp, "wpa_key_mgmt=WPA-PSK\n");
		fprintf(fp, "wpa_disable_eapol_key_retries=0\n");
		fprintf(fp, "wpa_passphrase=%s\n", iw_ap->password[0].password);
		break;
	case IW_ENCRY_WPA2_PSK:
		fprintf(fp, "auth_algs=1\n");
		fprintf(fp, "wpa=2\n");
		fprintf(fp, "wpa_key_mgmt=WPA-PSK\n");
		fprintf(fp, "wpa_disable_eapol_key_retries=0\n");
		fprintf(fp, "wpa_passphrase=%s\n", iw_ap->password[0].password);
		break;
	case IW_ENCRY_WPA2WPA_PSK:
		fprintf(fp, "auth_algs=1\n");
		fprintf(fp, "wpa=3\n");
		fprintf(fp, "wpa_key_mgmt=WPA-PSK\n");
		fprintf(fp, "wpa_disable_eapol_key_retries=0\n");
		fprintf(fp, "wpa_passphrase=%s\n", iw_ap->password[0].password);
		break;
	default:
		fprintf(fp, "auth_algs=1\n");
		fprintf(fp, "wpa=3\n");
		fprintf(fp, "wpa_key_mgmt=WPA-PSK\n");
		fprintf(fp, "wpa_disable_eapol_key_retries=0\n");
		fprintf(fp, "wpa_passphrase=%s\n", iw_ap->password[0].password);
		break;
	}
	fflush(fp);
	if(iw_ap->auth == IW_ENCRY_WPA_PSK ||
		iw_ap->auth == IW_ENCRY_WPA2_PSK ||
		iw_ap->auth == IW_ENCRY_WPA2WPA_PSK )
	{
		switch(iw_ap->encryption)
		{
		case IW_ALGO_AUTO:
			fprintf(fp, "wpa_pairwise=CCMP\n");
			break;
		case IW_ALGO_CCMP:
			fprintf(fp, "wpa_pairwise=CCMP\n");
			break;
		case IW_ALGO_TKIP:
			fprintf(fp, "wpa_pairwise=TKIP\n");
			break;
		case IW_ALGO_TKIP_CCMP:
			fprintf(fp, "wpa_pairwise=TKIP CCMP\n");
			break;
		default:
			fprintf(fp, "wpa_pairwise=CCMP\n");
			break;
		}
	}
	fflush(fp);
	return OK;
}

static int iw_ap_default_config(iw_ap_t *iw_ap, FILE *fp)
{
	if(!iw_ap && !fp)
		return ERROR;
	fprintf(fp, "\n#misc config\n");
/*
# Device Name
# User-friendly description of device; up to 32 octets encoded in UTF-8
#device_name=Wireless AP

# Manufacturer
# The manufacturer of the device (up to 64 ASCII characters)
#manufacturer=Company

# Model Name
# Model of the device (up to 32 ASCII characters)
#model_name=WAP

# Model Number
# Additional device description (up to 32 ASCII characters)
#model_number=123

# Serial Number
# Serial number of the device (up to 32 characters)
#serial_number=12345

# Primary Device Type
# Used format: <categ>-<OUI>-<subcateg>
# categ = Category as an integer value
# OUI = OUI and type octet as a 4-octet hex-encoded value; 0050F204 for
#       default WPS OUI
# subcateg = OUI-specific Sub Category as an integer value
# Examples:
#   1-0050F204-1 (Computer / PC)
#   1-0050F204-2 (Computer / Server)
#   5-0050F204-1 (Storage / NAS)
#   6-0050F204-1 (Network Infrastructure / AP)
#device_type=6-0050F204-1

# OS Version
# 4-octet operating system version number (hex string)
#os_version=01020300
*/
	fprintf(fp, "driver=nl80211\n");
	fprintf(fp, "logger_syslog=127\n");
	fprintf(fp, "logger_syslog_level=2\n");
	fprintf(fp, "logger_stdout=127\n");
	fprintf(fp, "logger_stdout_level=2\n");

	fprintf(fp, "ctrl_interface="DAEMON_ENV_DIR"/hostapd\n");

	fprintf(fp, "bss_load_update_period=60\n");
	fprintf(fp, "chan_util_avg_period=600\n");
	fprintf(fp, "preamble=1\n");

	fprintf(fp, "disassoc_low_ack=1\n");
	fprintf(fp, "uapsd_advertisement_enabled=1\n");
	fprintf(fp, "okc=0\n");
	fprintf(fp, "disable_pmksa_caching=1\n");
	fflush(fp);
	return OK;
}


static int iw_ap_user_config(iw_ap_t *iw_ap, FILE *fp)
{
	if(!iw_ap && !fp)
		return ERROR;
	ifindex_t ifindex = ifindex2ifkernel(iw_ap->ifindex);
	fprintf(fp, "\n# main config\n");
	fprintf(fp, "country_code=%02d\n", iw_ap->country_code);
	fflush(fp);
	fprintf(fp, "bssid=%02x:%02x:%02x:%02x:%02x:%02x\n", iw_ap->BSSID[0], iw_ap->BSSID[1],
			iw_ap->BSSID[2], iw_ap->BSSID[3], iw_ap->BSSID[4], iw_ap->BSSID[5]);

	//Beacon interval in kus (1.024 ms) (default: 100; range 15..65535)
	fprintf(fp, "beacon_int=%d\n", iw_ap->beacon);
	fprintf(fp, "channel=%d\n", iw_ap->channel);

	if(iw_ap->acs_num_scans)
		fprintf(fp, "acs_num_scans=%d\n", iw_ap->acs_num_scans);

	//fprintf(fp, "#dtim_period=2\n");

	if(iw_ap->max_num_sta)
		fprintf(fp, "max_num_sta=%d\n", iw_ap->max_num_sta);

	if(iw_ap->ap_isolate)
		fprintf(fp, "ap_isolate=1\n");
	if(iw_ap->bridge)
	{
		fprintf(fp, "bridge=%s\n",  ifkernelindex2kernelifname(ifindex2ifkernel(iw_ap->bridge)));
	}
	fprintf(fp, "interface=%s\n", ifkernelindex2kernelifname(ifindex));

	fprintf(fp, "ssid=%s\n", iw_ap->SSID);

	if(iw_ap->hiden_ssid)
		fprintf(fp, "ignore_broadcast_ssid=1\n");
	else
		fprintf(fp, "ignore_broadcast_ssid=0\n");

	if(iw_ap->wmm_enabled)
		fprintf(fp, "wmm_enabled=1\n");

	fflush(fp);
	return OK;
}


static int iw_ap_script_check(iw_ap_t *iw_ap)
{
	if(!iw_ap)
		return ERROR;

/*	iw_ap->ifindex = ifindex;
	ifp = if_lookup_by_index(iw_ap->ifindex);
	if(ifp)
		nsm_interface_mac_get_api(ifp, iw_ap->BSSID, NSM_MAC_MAX);
	*/
	if(iw_ap->ifindex &&
		strlen(iw_ap->SSID) &&
		str_isempty(iw_ap->BSSID, sizeof(iw_ap->BSSID)))
	{
		struct interface *ifp = NULL;
		ifp = if_lookup_by_index(iw_ap->ifindex);
		if(ifp)
			nsm_interface_mac_get_api(ifp, iw_ap->BSSID, NSM_MAC_MAX);

		//printf("============%s==========:ifindex=0x%x SSID=%s BSSID=%s\r\n",
		//   __func__,iw_ap->ifindex, iw_ap->SSID, iw_ap->BSSID);
	}
	if(iw_ap->ifindex &&
		strlen(iw_ap->SSID) &&
		!str_isempty(iw_ap->BSSID, sizeof(iw_ap->BSSID))/* &&
		//strlen(iw_ap->BSSID)
		iw_ap->hw_mode*/)
		return OK;
	return ERROR;
}

int iw_ap_make_script(iw_ap_t *iw_ap)
{
	FILE *fp = NULL;
	char path[128];
	if(!iw_ap)
		return ERROR;
	memset(path, 0, sizeof(path));
	if(iw_ap_script_check(iw_ap) != OK)
	{
		//printf("============%s==========:ifindex=0x%x SSID=%s BSSID=%s\r\n",
		//	   __func__,iw_ap->ifindex, iw_ap->SSID, iw_ap->BSSID);
		return ERROR;
	}
	snprintf(path, sizeof(path), "%s/hostapd-%s.conf", DAEMON_ENV_DIR,
			ifkernelindex2kernelifname(ifindex2ifkernel(iw_ap->ifindex)));
	if(access(path, F_OK) == 0)
	{
		remove(path);
		sync();
	}
	fp = fopen(path, "w+");
	if(fp)
	{
		fflush(fp);
		iw_ap_default_config(iw_ap, fp);
		//printf("============%s==========:path=%s\r\n",
		//	   __func__, path);
		iw_ap_hw_default_config(iw_ap, fp);
		iw_ap_user_config(iw_ap, fp);

		iw_ap_auth_default_config(iw_ap, fp);
		if((iw_ap->mac_list && lstCount(iw_ap->mac_list)) || (iw_ap->dmac_list && lstCount(iw_ap->dmac_list)))
			iw_ap_macacl_config(iw_ap, fp, iw_ap->macaddr_acl);
		fflush(fp);
		fclose(fp);
		return OK;
	}
	return ERROR;
}

#if 1//def DOUBLE_PROCESS
int iw_ap_running_script(iw_ap_t *iw_ap)
{
	///usr/sbin/hostapd -s -P /var/run/wifi-phy0.pid -B /var/run/hostapd-phy0.conf
	char *kname = NULL;
	char path[128];
	char pidpath[128];
	char confpath[128];
	char *argv[] = {"-s", "-P", NULL, "-B", NULL, NULL};
	if(!iw_ap)
		return ERROR;
	kname = ifkernelindex2kernelifname(ifindex2ifkernel(iw_ap->ifindex));
	if(!kname)
		return ERROR;
	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path), "hostapd.%s", kname);

	memset(pidpath, 0, sizeof(pidpath));
	snprintf(pidpath, sizeof(pidpath), "%s/hostapd-%s.pid",DAEMON_VTY_DIR, kname);
	memset(confpath, 0, sizeof(confpath));
	snprintf(confpath, sizeof(confpath), "%s/hostapd-%s.conf", DAEMON_ENV_DIR, kname);

	if(os_pid_get(pidpath) > 0)
	{
		if(iw_ap->change)
		{
			kill(os_pid_get(pidpath), SIGTERM);
			os_msleep(100);
			remove(pidpath);
		}
		else
			return OK;
	}
	argv[2] = pidpath;
	argv[4] = confpath;

	if(iw_ap_make_script(iw_ap) == OK)
	{
		iw_ap->change = FALSE;
		if(IW_DEBUG(EVENT))
		{
			zlog_debug(ZLOG_WIFI, "running AP domain process on interface %s ",ifindex2ifname(iw_ap->ifindex));
		}
#ifdef DOUBLE_PROCESS
		os_process_register(PROCESS_DEAMON, path, "/usr/sbin/hostapd", FALSE, argv);
#endif
		return OK;
	}
	return ERROR;
}
///tmp/app/var/hostapd-wlan0.pid
int iw_ap_stop_script(iw_ap_t *iw_ap)
{
	int pid = 0;
	char pidpath[128];
	char *kname = NULL;
	if(!iw_ap)
		return ERROR;
	kname = ifkernelindex2kernelifname(ifindex2ifkernel(iw_ap->ifindex));
	if(!kname)
		return ERROR;
	memset(pidpath, 0, sizeof(pidpath));
	snprintf(pidpath, sizeof(pidpath), "%s/hostapd-%s.pid",DAEMON_VTY_DIR, kname);
	pid = os_pid_get(pidpath);
	if(pid <= 0)
		return ERROR;
	if(pid > 0)
		kill(pid, SIGTERM);
	os_msleep(100);
	remove(pidpath);
	return OK;
}

int iw_ap_script_is_running(iw_ap_t *iw_ap)
{
	char *kname = NULL;
	char pidpath[128];
	if(!iw_ap)
		return ERROR;
	kname = ifkernelindex2kernelifname(ifindex2ifkernel(iw_ap->ifindex));
	if(!kname)
		return ERROR;

	memset(pidpath, 0, sizeof(pidpath));
	snprintf(pidpath, sizeof(pidpath), "%s/hostapd-%s.pid",DAEMON_VTY_DIR, kname);
	if(os_pid_get(pidpath) > 0)
	{
		if(name2pid("hostapd") > 0)
			return OK;
	}
	return ERROR;
}


#else
int iw_ap_running_config(iw_ap_t *iw_ap)
{
	if(!iw_ap)
		return ERROR;
	return OK;
}

int iw_ap_stop_script(iw_ap_t *iw_ap)
{
	if(!iw_ap)
		return ERROR;
	return OK;
}
#endif
/*
 *
wpa_pairwise=CCMP TKIP
 *
IW_ENCRY_WPA_PSK,		//wpa= 1 WPA-PSK wpa_pairwise=CCMP/AUTO
IW_ENCRY_WPA2_PSK,		//wpa= 2 WPA2-PSK wpa_pairwise=CCMP/AUTO
IW_ENCRY_WPA2WPA_PSK,	//wpa= 3 WPA2-PSK wpa_pairwise=CCMP/AUTO
wpa=3
wpa_pairwise=CCMP
wpa_key_mgmt=WPA-PSK

IW_ENCRY_NONE
wpa=0
auth_algs=1
wpa_key_mgmt=WPA-PSK
 *
 *
 *open
wep_key0=1234567890
wep_key1=1234567891
wep_key2=1234567892
wep_key3=1234567893
wep_default_key=0
auth_algs=1
wpa=0

 *shared
wep_key0=1234567890
wep_key1=1234567891
wep_key2=1234567892
wep_key3=1234567893
wep_default_key=0
auth_algs=2
wpa=0

 */

/*
 * /usr/sbin/hostapd -s -P /var/run/wifi-phy0.pid -B /var/run/hostapd-phy0.conf
 *
root@OpenWrt:/# cat /var/run/hostapd-phy0.conf
driver=nl80211
logger_syslog=127
logger_syslog_level=2
logger_stdout=127
logger_stdout_level=2
country_code=00
ieee80211d=1
hw_mode=g
beacon_int=100
channel=5


ieee80211n=1
ht_coex=0
ht_capab=[SHORT-GI-20][SHORT-GI-40][TX-STBC][RX-STBC1]

interface=wlan0
ctrl_interface=/var/run/hostapd
ap_isolate=1
bss_load_update_period=60
chan_util_avg_period=600
disassoc_low_ack=1
preamble=1
wmm_enabled=1
ignore_broadcast_ssid=0
uapsd_advertisement_enabled=1
wpa_passphrase=12345678
auth_algs=1
wpa=3
wpa_pairwise=CCMP
ssid=openwrt
bridge=br-lan
wpa_disable_eapol_key_retries=0
wpa_key_mgmt=WPA-PSK
okc=0
disable_pmksa_caching=1
macaddr_acl=0
deny_mac_file=/var/run/hostapd-wlan0.maclist
bssid=66:e8:c1:e2:4f:47



root@OpenWrt:/#
root@OpenWrt:/#
 *
 */
/*
##### hostapd configuration file ##############################################
# Empty lines and lines starting with # are ignored

# AP netdevice name (without 'ap' postfix, i.e., wlan0 uses wlan0ap for
# management frames); ath0 for madwifi
interface=wlan1

# In case of madwifi, atheros, and nl80211 driver interfaces, an additional
# configuration parameter, bridge, may be used to notify hostapd if the
# interface is included in a bridge. This parameter is not used with Host AP
# driver. If the bridge parameter is not set, the drivers will automatically
# figure out the bridge interface (assuming sysfs is enabled and mounted to
# /sys) and this parameter may not be needed.
#
# For nl80211, this parameter can be used to request the AP interface to be
# added to the bridge automatically (brctl may refuse to do this before hostapd
# has been started to change the interface mode). If needed, the bridge
# interface is also created.
#bridge=br0

# Driver interface type (hostap/wired/madwifi/test/none/nl80211/bsd);
# default: hostap). nl80211 is used with all Linux mac80211 drivers.
# Use driver=none if building hostapd as a standalone RADIUS server that does
# not control any wireless/wired driver.
driver=nl80211

# hostapd event logger configuration
#
# Two output method: syslog and stdout (only usable if not forking to
# background).
#
# Module bitfield (ORed bitfield of modules that will be logged; -1 = all
# modules):
# bit 0 (1) = IEEE 802.11
# bit 1 (2) = IEEE 802.1X
# bit 2 (4) = RADIUS
# bit 3 (8) = WPA
# bit 4 (16) = driver interface
# bit 5 (32) = IAPP
# bit 6 (64) = MLME
#
# Levels (minimum value for logged events):
#  0 = verbose debugging
#  1 = debugging
#  2 = informational messages
#  3 = notification
#  4 = warning
#
logger_syslog=-1
logger_syslog_level=2
logger_stdout=-1
logger_stdout_level=2

# Interface for separate control program. If this is specified, hostapd
# will create this directory and a UNIX domain socket for listening to requests
# from external programs (CLI/GUI, etc.) for status information and
# configuration. The socket file will be named based on the interface name, so
# multiple hostapd processes/interfaces can be run at the same time if more
# than one interface is used.
# /var/run/hostapd is the recommended directory for sockets and by default,
# hostapd_cli will use it when trying to connect with hostapd.
ctrl_interface=/var/run/hostapd

# Access control for the control interface can be configured by setting the
# directory to allow only members of a group to use sockets. This way, it is
# possible to run hostapd as root (since it needs to change network
# configuration and open raw sockets) and still allow GUI/CLI components to be
# run as non-root users. However, since the control interface can be used to
# change the network configuration, this access needs to be protected in many
# cases. By default, hostapd is configured to use gid 0 (root). If you
# want to allow non-root users to use the contron interface, add a new group
# and change this value to match with that group. Add users that should have
# control interface access to this group.
#
# This variable can be a group name or gid.
#ctrl_interface_group=wheel
ctrl_interface_group=0

##### IEEE 802.11 related configuration #######################################

# SSID to be used in IEEE 802.11 management frames
ssid=Nazgul
# Alternative formats for configuring SSID
# (double quoted string, hexdump, printf-escaped string)
#ssid2="test"
#ssid2=74657374
#ssid2=P"hello\nthere"

# UTF-8 SSID: Whether the SSID is to be interpreted using UTF-8 encoding
#utf8_ssid=1

# Country code (ISO/IEC 3166-1). Used to set regulatory domain.
# Set as needed to indicate country in which device is operating.
# This can limit available channels and transmit power.
#country_code=US

# Enable IEEE 802.11d. This advertises the country_code and the set of allowed
# channels and transmit power levels based on the regulatory limits. The
# country_code setting must be configured with the correct country for
# IEEE 802.11d functions.
# (default: 0 = disabled)
#ieee80211d=1

# Enable IEEE 802.11h. This enables radar detection and DFS support if
# available. DFS support is required on outdoor 5 GHz channels in most countries
# of the world. This can be used only with ieee80211d=1.
# (default: 0 = disabled)
#ieee80211h=1

# Add Power Constraint element to Beacon and Probe Response frames
# This config option adds Power Constraint element when applicable and Country
# element is added. Power Constraint element is required by Transmit Power
# Control. This can be used only with ieee80211d=1.
# Valid values are 0..255.
#local_pwr_constraint=3

# Set Spectrum Management subfield in the Capability Information field.
# This config option forces the Spectrum Management bit to be set. When this
# option is not set, the value of the Spectrum Management bit depends on whether
# DFS or TPC is required by regulatory authorities. This can be used only with
# ieee80211d=1 and local_pwr_constraint configured.
#spectrum_mgmt_required=1

# Operation mode (a = IEEE 802.11a, b = IEEE 802.11b, g = IEEE 802.11g,
# ad = IEEE 802.11ad (60 GHz); a/g options are used with IEEE 802.11n, too, to
# specify band)
# Default: IEEE 802.11b
hw_mode=g

# Channel number (IEEE 802.11)
# (default: 0, i.e., not set)
# Please note that some drivers do not use this value from hostapd and the
# channel will need to be configured separately with iwconfig.
#
# If CONFIG_ACS build option is enabled, the channel can be selected
# automatically at run time by setting channel=acs_survey or channel=0, both of
# which will enable the ACS survey based algorithm.
channel=10

# ACS tuning - Automatic Channel Selection
# See: http://wireless.kernel.org/en/users/Documentation/acs
#
# You can customize the ACS survey algorithm with following variables:
#
# acs_num_scans requirement is 1..100 - number of scans to be performed that
# are used to trigger survey data gathering of an underlying device driver.
# Scans are passive and typically take a little over 100ms (depending on the
# driver) on each available channel for given hw_mode. Increasing this value
# means sacrificing startup time and gathering more data wrt channel
# interference that may help choosing a better channel. This can also help fine
# tune the ACS scan time in case a driver has different scan dwell times.
#
# Defaults:
#acs_num_scans=5

# Beacon interval in kus (1.024 ms) (default: 100; range 15..65535)
beacon_int=100

# DTIM (delivery traffic information message) period (range 1..255):
# number of beacons between DTIMs (1 = every beacon includes DTIM element)
# (default: 2)
dtim_period=2

# Maximum number of stations allowed in station table. New stations will be
# rejected after the station table is full. IEEE 802.11 has a limit of 2007
# different association IDs, so this number should not be larger than that.
# (default: 2007)
max_num_sta=255

# RTS/CTS threshold; 2347 = disabled (default); range 0..2347
# If this field is not included in hostapd.conf, hostapd will not control
# RTS threshold and 'iwconfig wlan# rts <val>' can be used to set it.
rts_threshold=2347

# Fragmentation threshold; 2346 = disabled (default); range 256..2346
# If this field is not included in hostapd.conf, hostapd will not control
# fragmentation threshold and 'iwconfig wlan# frag <val>' can be used to set
# it.
fragm_threshold=2346

# Rate configuration
# Default is to enable all rates supported by the hardware. This configuration
# item allows this list be filtered so that only the listed rates will be left
# in the list. If the list is empty, all rates are used. This list can have
# entries that are not in the list of rates the hardware supports (such entries
# are ignored). The entries in this list are in 100 kbps, i.e., 11 Mbps = 110.
# If this item is present, at least one rate have to be matching with the rates
# hardware supports.
# default: use the most common supported rate setting for the selected
# hw_mode (i.e., this line can be removed from configuration file in most
# cases)
#supported_rates=10 20 55 110 60 90 120 180 240 360 480 540

# Basic rate set configuration
# List of rates (in 100 kbps) that are included in the basic rate set.
# If this item is not included, usually reasonable default set is used.
#basic_rates=10 20
#basic_rates=10 20 55 110
#basic_rates=60 120 240

# Short Preamble
# This parameter can be used to enable optional use of short preamble for
# frames sent at 2 Mbps, 5.5 Mbps, and 11 Mbps to improve network performance.
# This applies only to IEEE 802.11b-compatible networks and this should only be
# enabled if the local hardware supports use of short preamble. If any of the
# associated STAs do not support short preamble, use of short preamble will be
# disabled (and enabled when such STAs disassociate) dynamically.
# 0 = do not allow use of short preamble (default)
# 1 = allow use of short preamble
#preamble=1

# Station MAC address -based authentication
# Please note that this kind of access control requires a driver that uses
# hostapd to take care of management frame processing and as such, this can be
# used with driver=hostap or driver=nl80211, but not with driver=madwifi.
# 0 = accept unless in deny list
# 1 = deny unless in accept list
# 2 = use external RADIUS server (accept/deny lists are searched first)
macaddr_acl=0

# Accept/deny lists are read from separate files (containing list of
# MAC addresses, one per line). Use absolute path name to make sure that the
# files can be read on SIGHUP configuration reloads.
accept_mac_file=/etc/hostapd.accept
deny_mac_file=/etc/hostapd.deny

# IEEE 802.11 specifies two authentication algorithms. hostapd can be
# configured to allow both of these or only one. Open system authentication
# should be used with IEEE 802.1X.
# Bit fields of allowed authentication algorithms:
# bit 0 = Open System Authentication
# bit 1 = Shared Key Authentication (requires WEP)
auth_algs=3

# Send empty SSID in beacons and ignore probe request frames that do not
# specify full SSID, i.e., require stations to know SSID.
# default: disabled (0)
# 1 = send empty (length=0) SSID in beacon and ignore probe request for
#     broadcast SSID
# 2 = clear SSID (ASCII 0), but keep the original length (this may be required
#     with some clients that do not support empty SSID) and ignore probe
#     requests for broadcast SSID
ignore_broadcast_ssid=0

# Additional vendor specfic elements for Beacon and Probe Response frames
# This parameter can be used to add additional vendor specific element(s) into
# the end of the Beacon and Probe Response frames. The format for these
# element(s) is a hexdump of the raw information elements (id+len+payload for
# one or more elements)
#vendor_elements=dd0411223301

# TX queue parameters (EDCF / bursting)
# tx_queue_<queue name>_<param>
# queues: data0, data1, data2, data3, after_beacon, beacon
# (data0 is the highest priority queue)
# parameters:
#   aifs: AIFS (default 2)
#   cwmin: cwMin (1, 3, 7, 15, 31, 63, 127, 255, 511, 1023)
#   cwmax: cwMax (1, 3, 7, 15, 31, 63, 127, 255, 511, 1023); cwMax >= cwMin
#   burst: maximum length (in milliseconds with precision of up to 0.1 ms) for
#          bursting
#
# Default WMM parameters (IEEE 802.11 draft; 11-03-0504-03-000e):
# These parameters are used by the access point when transmitting frames
# to the clients.
#
# Low priority / AC_BK = background
#tx_queue_data3_aifs=7
#tx_queue_data3_cwmin=15
#tx_queue_data3_cwmax=1023
#tx_queue_data3_burst=0
# Note: for IEEE 802.11b mode: cWmin=31 cWmax=1023 burst=0
#
# Normal priority / AC_BE = best effort
#tx_queue_data2_aifs=3
#tx_queue_data2_cwmin=15
#tx_queue_data2_cwmax=63
#tx_queue_data2_burst=0
# Note: for IEEE 802.11b mode: cWmin=31 cWmax=127 burst=0
#
# High priority / AC_VI = video
#tx_queue_data1_aifs=1
#tx_queue_data1_cwmin=7
#tx_queue_data1_cwmax=15
#tx_queue_data1_burst=3.0
# Note: for IEEE 802.11b mode: cWmin=15 cWmax=31 burst=6.0
#
# Highest priority / AC_VO = voice
#tx_queue_data0_aifs=1
#tx_queue_data0_cwmin=3
#tx_queue_data0_cwmax=7
#tx_queue_data0_burst=1.5
# Note: for IEEE 802.11b mode: cWmin=7 cWmax=15 burst=3.3

# 802.1D Tag (= UP) to AC mappings
# WMM specifies following mapping of data frames to different ACs. This mapping
# can be configured using Linux QoS/tc and sch_pktpri.o module.
# 802.1D Tag 802.1D Designation Access Category WMM Designation
# 1 BK AC_BK Background
# 2 - AC_BK Background
# 0 BE AC_BE Best Effort
# 3 EE AC_BE Best Effort
# 4 CL AC_VI Video
# 5 VI AC_VI Video
# 6 VO AC_VO Voice
# 7 NC AC_VO Voice
# Data frames with no priority information: AC_BE
# Management frames: AC_VO
# PS-Poll frames: AC_BE

# Default WMM parameters (IEEE 802.11 draft; 11-03-0504-03-000e):
# for 802.11a or 802.11g networks
# These parameters are sent to WMM clients when they associate.
# The parameters will be used by WMM clients for frames transmitted to the
# access point.
#
# note - txop_limit is in units of 32microseconds
# note - acm is admission control mandatory flag. 0 = admission control not
# required, 1 = mandatory
# note - here cwMin and cmMax are in exponent form. the actual cw value used
# will be (2^n)-1 where n is the value given here
#
wmm_enabled=1
#
# WMM-PS Unscheduled Automatic Power Save Delivery [U-APSD]
# Enable this flag if U-APSD supported outside hostapd (eg., Firmware/driver)
#uapsd_advertisement_enabled=1
#
# Low priority / AC_BK = background
wmm_ac_bk_cwmin=4
wmm_ac_bk_cwmax=10
wmm_ac_bk_aifs=7
wmm_ac_bk_txop_limit=0
wmm_ac_bk_acm=0
# Note: for IEEE 802.11b mode: cWmin=5 cWmax=10
#
# Normal priority / AC_BE = best effort
wmm_ac_be_aifs=3
wmm_ac_be_cwmin=4
wmm_ac_be_cwmax=10
wmm_ac_be_txop_limit=0
wmm_ac_be_acm=0
# Note: for IEEE 802.11b mode: cWmin=5 cWmax=7
#
# High priority / AC_VI = video
wmm_ac_vi_aifs=2
wmm_ac_vi_cwmin=3
wmm_ac_vi_cwmax=4
wmm_ac_vi_txop_limit=94
wmm_ac_vi_acm=0
# Note: for IEEE 802.11b mode: cWmin=4 cWmax=5 txop_limit=188
#
# Highest priority / AC_VO = voice
wmm_ac_vo_aifs=2
wmm_ac_vo_cwmin=2
wmm_ac_vo_cwmax=3
wmm_ac_vo_txop_limit=47
wmm_ac_vo_acm=0
# Note: for IEEE 802.11b mode: cWmin=3 cWmax=4 burst=102

# Static WEP key configuration
#
# The key number to use when transmitting.
# It must be between 0 and 3, and the corresponding key must be set.
# default: not set
#wep_default_key=0
# The WEP keys to use.
# A key may be a quoted string or unquoted hexadecimal digits.
# The key length should be 5, 13, or 16 characters, or 10, 26, or 32
# digits, depending on whether 40-bit (64-bit), 104-bit (128-bit), or
# 128-bit (152-bit) WEP is used.
# Only the default key must be supplied; the others are optional.
# default: not set
#wep_key0=123456789a
#wep_key1="vwxyz"
#wep_key2=0102030405060708090a0b0c0d
#wep_key3=".2.4.6.8.0.23"

# Station inactivity limit
#
# If a station does not send anything in ap_max_inactivity seconds, an
# empty data frame is sent to it in order to verify whether it is
# still in range. If this frame is not ACKed, the station will be
# disassociated and then deauthenticated. This feature is used to
# clear station table of old entries when the STAs move out of the
# range.
#
# The station can associate again with the AP if it is still in range;
# this inactivity poll is just used as a nicer way of verifying
# inactivity; i.e., client will not report broken connection because
# disassociation frame is not sent immediately without first polling
# the STA with a data frame.
# default: 300 (i.e., 5 minutes)
#ap_max_inactivity=300
#
# The inactivity polling can be disabled to disconnect stations based on
# inactivity timeout so that idle stations are more likely to be disconnected
# even if they are still in range of the AP. This can be done by setting
# skip_inactivity_poll to 1 (default 0).
#skip_inactivity_poll=0

# Disassociate stations based on excessive transmission failures or other
# indications of connection loss. This depends on the driver capabilities and
# may not be available with all drivers.
#disassoc_low_ack=1

# Maximum allowed Listen Interval (how many Beacon periods STAs are allowed to
# remain asleep). Default: 65535 (no limit apart from field size)
#max_listen_interval=100

# WDS (4-address frame) mode with per-station virtual interfaces
# (only supported with driver=nl80211)
# This mode allows associated stations to use 4-address frames to allow layer 2
# bridging to be used.
#wds_sta=1

# If bridge parameter is set, the WDS STA interface will be added to the same
# bridge by default. This can be overridden with the wds_bridge parameter to
# use a separate bridge.
#wds_bridge=wds-br0

# Start the AP with beaconing disabled by default.
#start_disabled=0

# Client isolation can be used to prevent low-level bridging of frames between
# associated stations in the BSS. By default, this bridging is allowed.
#ap_isolate=1

# Fixed BSS Load value for testing purposes
# This field can be used to configure hostapd to add a fixed BSS Load element
# into Beacon and Probe Response frames for testing purposes. The format is
# <station count>:<channel utilization>:<available admission capacity>
#bss_load_test=12:80:20000

##### IEEE 802.11n related configuration ######################################

# ieee80211n: Whether IEEE 802.11n (HT) is enabled
# 0 = disabled (default)
# 1 = enabled
# Note: You will also need to enable WMM for full HT functionality.
#ieee80211n=1

# ht_capab: HT capabilities (list of flags)
# LDPC coding capability: [LDPC] = supported
# Supported channel width set: [HT40-] = both 20 MHz and 40 MHz with secondary
# channel below the primary channel; [HT40+] = both 20 MHz and 40 MHz
# with secondary channel below the primary channel
# (20 MHz only if neither is set)
# Note: There are limits on which channels can be used with HT40- and
# HT40+. Following table shows the channels that may be available for
# HT40- and HT40+ use per IEEE 802.11n Annex J:
# freq HT40- HT40+
# 2.4 GHz 5-13 1-7 (1-9 in Europe/Japan)
# 5 GHz 40,48,56,64 36,44,52,60
# (depending on the location, not all of these channels may be available
# for use)
# Please note that 40 MHz channels may switch their primary and secondary
# channels if needed or creation of 40 MHz channel maybe rejected based
# on overlapping BSSes. These changes are done automatically when hostapd
# is setting up the 40 MHz channel.
# Spatial Multiplexing (SM) Power Save: [SMPS-STATIC] or [SMPS-DYNAMIC]
# (SMPS disabled if neither is set)
# HT-greenfield: [GF] (disabled if not set)
# Short GI for 20 MHz: [SHORT-GI-20] (disabled if not set)
# Short GI for 40 MHz: [SHORT-GI-40] (disabled if not set)
# Tx STBC: [TX-STBC] (disabled if not set)
# Rx STBC: [RX-STBC1] (one spatial stream), [RX-STBC12] (one or two spatial
# streams), or [RX-STBC123] (one, two, or three spatial streams); Rx STBC
# disabled if none of these set
# HT-delayed Block Ack: [DELAYED-BA] (disabled if not set)
# Maximum A-MSDU length: [MAX-AMSDU-7935] for 7935 octets (3839 octets if not
# set)
# DSSS/CCK Mode in 40 MHz: [DSSS_CCK-40] = allowed (not allowed if not set)
# PSMP support: [PSMP] (disabled if not set)
# L-SIG TXOP protection support: [LSIG-TXOP-PROT] (disabled if not set)
#ht_capab=[HT40-][SHORT-GI-20][SHORT-GI-40]

# Require stations to support HT PHY (reject association if they do not)
#require_ht=1

# If set non-zero, require stations to perform scans of overlapping
# channels to test for stations which would be affected by 40 MHz traffic.
# This parameter sets the interval in seconds between these scans. This
# is useful only for testing that stations properly set the OBSS interval,
# since the other parameters in the OBSS scan parameters IE are set to 0.
#obss_interval=0

##### IEEE 802.11ac related configuration #####################################

# ieee80211ac: Whether IEEE 802.11ac (VHT) is enabled
# 0 = disabled (default)
# 1 = enabled
# Note: You will also need to enable WMM for full VHT functionality.
#ieee80211ac=1

# vht_capab: VHT capabilities (list of flags)
#
# vht_max_mpdu_len: [MAX-MPDU-7991] [MAX-MPDU-11454]
# Indicates maximum MPDU length
# 0 = 3895 octets (default)
# 1 = 7991 octets
# 2 = 11454 octets
# 3 = reserved
#
# supported_chan_width: [VHT160] [VHT160-80PLUS80]
# Indicates supported Channel widths
# 0 = 160 MHz & 80+80 channel widths are not supported (default)
# 1 = 160 MHz channel width is supported
# 2 = 160 MHz & 80+80 channel widths are supported
# 3 = reserved
#
# Rx LDPC coding capability: [RXLDPC]
# Indicates support for receiving LDPC coded pkts
# 0 = Not supported (default)
# 1 = Supported
#
# Short GI for 80 MHz: [SHORT-GI-80]
# Indicates short GI support for reception of packets transmitted with TXVECTOR
# params format equal to VHT and CBW = 80Mhz
# 0 = Not supported (default)
# 1 = Supported
#
# Short GI for 160 MHz: [SHORT-GI-160]
# Indicates short GI support for reception of packets transmitted with TXVECTOR
# params format equal to VHT and CBW = 160Mhz
# 0 = Not supported (default)
# 1 = Supported
#
# Tx STBC: [TX-STBC-2BY1]
# Indicates support for the transmission of at least 2x1 STBC
# 0 = Not supported (default)
# 1 = Supported
#
# Rx STBC: [RX-STBC-1] [RX-STBC-12] [RX-STBC-123] [RX-STBC-1234]
# Indicates support for the reception of PPDUs using STBC
# 0 = Not supported (default)
# 1 = support of one spatial stream
# 2 = support of one and two spatial streams
# 3 = support of one, two and three spatial streams
# 4 = support of one, two, three and four spatial streams
# 5,6,7 = reserved
#
# SU Beamformer Capable: [SU-BEAMFORMER]
# Indicates support for operation as a single user beamformer
# 0 = Not supported (default)
# 1 = Supported
#
# SU Beamformee Capable: [SU-BEAMFORMEE]
# Indicates support for operation as a single user beamformee
# 0 = Not supported (default)
# 1 = Supported
#
# Compressed Steering Number of Beamformer Antennas Supported: [BF-ANTENNA-2]
#   Beamformee's capability indicating the maximum number of beamformer
#   antennas the beamformee can support when sending compressed beamforming
#   feedback
# If SU beamformer capable, set to maximum value minus 1
# else reserved (default)
#
# Number of Sounding Dimensions: [SOUNDING-DIMENSION-2]
# Beamformer's capability indicating the maximum value of the NUM_STS parameter
# in the TXVECTOR of a VHT NDP
# If SU beamformer capable, set to maximum value minus 1
# else reserved (default)
#
# MU Beamformer Capable: [MU-BEAMFORMER]
# Indicates support for operation as an MU beamformer
# 0 = Not supported or sent by Non-AP STA (default)
# 1 = Supported
#
# MU Beamformee Capable: [MU-BEAMFORMEE]
# Indicates support for operation as an MU beamformee
# 0 = Not supported or sent by AP (default)
# 1 = Supported
#
# VHT TXOP PS: [VHT-TXOP-PS]
# Indicates whether or not the AP supports VHT TXOP Power Save Mode
#  or whether or not the STA is in VHT TXOP Power Save mode
# 0 = VHT AP doesnt support VHT TXOP PS mode (OR) VHT Sta not in VHT TXOP PS
#  mode
# 1 = VHT AP supports VHT TXOP PS mode (OR) VHT Sta is in VHT TXOP power save
#  mode
#
# +HTC-VHT Capable: [HTC-VHT]
# Indicates whether or not the STA supports receiving a VHT variant HT Control
# field.
# 0 = Not supported (default)
# 1 = supported
#
# Maximum A-MPDU Length Exponent: [MAX-A-MPDU-LEN-EXP0]..[MAX-A-MPDU-LEN-EXP7]
# Indicates the maximum length of A-MPDU pre-EOF padding that the STA can recv
# This field is an integer in the range of 0 to 7.
# The length defined by this field is equal to
# 2 pow(13 + Maximum A-MPDU Length Exponent) -1 octets
#
# VHT Link Adaptation Capable: [VHT-LINK-ADAPT2] [VHT-LINK-ADAPT3]
# Indicates whether or not the STA supports link adaptation using VHT variant
# HT Control field
# If +HTC-VHTcapable is 1
#  0 = (no feedback) if the STA does not provide VHT MFB (default)
#  1 = reserved
#  2 = (Unsolicited) if the STA provides only unsolicited VHT MFB
#  3 = (Both) if the STA can provide VHT MFB in response to VHT MRQ and if the
#      STA provides unsolicited VHT MFB
# Reserved if +HTC-VHTcapable is 0
#
# Rx Antenna Pattern Consistency: [RX-ANTENNA-PATTERN]
# Indicates the possibility of Rx antenna pattern change
# 0 = Rx antenna pattern might change during the lifetime of an association
# 1 = Rx antenna pattern does not change during the lifetime of an association
#
# Tx Antenna Pattern Consistency: [TX-ANTENNA-PATTERN]
# Indicates the possibility of Tx antenna pattern change
# 0 = Tx antenna pattern might change during the lifetime of an association
# 1 = Tx antenna pattern does not change during the lifetime of an association
#vht_capab=[SHORT-GI-80][HTC-VHT]
#
# Require stations to support VHT PHY (reject association if they do not)
#require_vht=1

# 0 = 20 or 40 MHz operating Channel width
# 1 = 80 MHz channel width
# 2 = 160 MHz channel width
# 3 = 80+80 MHz channel width
#vht_oper_chwidth=1
#
# center freq = 5 GHz + (5 * index)
# So index 42 gives center freq 5.210 GHz
# which is channel 42 in 5G band
#
#vht_oper_centr_freq_seg0_idx=42
#
# center freq = 5 GHz + (5 * index)
# So index 159 gives center freq 5.795 GHz
# which is channel 159 in 5G band
#
#vht_oper_centr_freq_seg1_idx=159

##### IEEE 802.1X-2004 related configuration ##################################

# Require IEEE 802.1X authorization
#ieee8021x=1

# IEEE 802.1X/EAPOL version
# hostapd is implemented based on IEEE Std 802.1X-2004 which defines EAPOL
# version 2. However, there are many client implementations that do not handle
# the new version number correctly (they seem to drop the frames completely).
# In order to make hostapd interoperate with these clients, the version number
# can be set to the older version (1) with this configuration value.
#eapol_version=2

# Optional displayable message sent with EAP Request-Identity. The first \0
# in this string will be converted to ASCII-0 (nul). This can be used to
# separate network info (comma separated list of attribute=value pairs); see,
# e.g., RFC 4284.
#eap_message=hello
#eap_message=hello\0networkid=netw,nasid=foo,portid=0,NAIRealms=example.com

# WEP rekeying (disabled if key lengths are not set or are set to 0)
# Key lengths for default/broadcast and individual/unicast keys:
# 5 = 40-bit WEP (also known as 64-bit WEP with 40 secret bits)
# 13 = 104-bit WEP (also known as 128-bit WEP with 104 secret bits)
#wep_key_len_broadcast=5
#wep_key_len_unicast=5
# Rekeying period in seconds. 0 = do not rekey (i.e., set keys only once)
#wep_rekey_period=300

# EAPOL-Key index workaround (set bit7) for WinXP Supplicant (needed only if
# only broadcast keys are used)
eapol_key_index_workaround=0

# EAP reauthentication period in seconds (default: 3600 seconds; 0 = disable
# reauthentication).
#eap_reauth_period=3600

# Use PAE group address (01:80:c2:00:00:03) instead of individual target
# address when sending EAPOL frames with driver=wired. This is the most common
# mechanism used in wired authentication, but it also requires that the port
# is only used by one station.
#use_pae_group_addr=1

##### Integrated EAP server ###################################################

# Optionally, hostapd can be configured to use an integrated EAP server
# to process EAP authentication locally without need for an external RADIUS
# server. This functionality can be used both as a local authentication server
# for IEEE 802.1X/EAPOL and as a RADIUS server for other devices.

# Use integrated EAP server instead of external RADIUS authentication
# server. This is also needed if hostapd is configured to act as a RADIUS
# authentication server.
eap_server=0

# Path for EAP server user database
# If SQLite support is included, this can be set to "sqlite:/path/to/sqlite.db"
# to use SQLite database instead of a text file.
#eap_user_file=/etc/hostapd.eap_user

# CA certificate (PEM or DER file) for EAP-TLS/PEAP/TTLS
#ca_cert=/etc/hostapd.ca.pem

# Server certificate (PEM or DER file) for EAP-TLS/PEAP/TTLS
#server_cert=/etc/hostapd.server.pem

# Private key matching with the server certificate for EAP-TLS/PEAP/TTLS
# This may point to the same file as server_cert if both certificate and key
# are included in a single file. PKCS#12 (PFX) file (.p12/.pfx) can also be
# used by commenting out server_cert and specifying the PFX file as the
# private_key.
#private_key=/etc/hostapd.server.prv

# Passphrase for private key
#private_key_passwd=secret passphrase

# Server identity
# EAP methods that provide mechanism for authenticated server identity delivery
# use this value. If not set, "hostapd" is used as a default.
#server_id=server.example.com

# Enable CRL verification.
# Note: hostapd does not yet support CRL downloading based on CDP. Thus, a
# valid CRL signed by the CA is required to be included in the ca_cert file.
# This can be done by using PEM format for CA certificate and CRL and
# concatenating these into one file. Whenever CRL changes, hostapd needs to be
# restarted to take the new CRL into use.
# 0 = do not verify CRLs (default)
# 1 = check the CRL of the user certificate
# 2 = check all CRLs in the certificate path
#check_crl=1

# Cached OCSP stapling response (DER encoded)
# If set, this file is sent as a certificate status response by the EAP server
# if the EAP peer requests certificate status in the ClientHello message.
# This cache file can be updated, e.g., by running following command
# periodically to get an update from the OCSP responder:
# openssl ocsp \
# -no_nonce \
# -CAfile /etc/hostapd.ca.pem \
# -issuer /etc/hostapd.ca.pem \
# -cert /etc/hostapd.server.pem \
# -url http://ocsp.example.com:8888/ \
# -respout /tmp/ocsp-cache.der
#ocsp_stapling_response=/tmp/ocsp-cache.der

# dh_file: File path to DH/DSA parameters file (in PEM format)
# This is an optional configuration file for setting parameters for an
# ephemeral DH key exchange. In most cases, the default RSA authentication does
# not use this configuration. However, it is possible setup RSA to use
# ephemeral DH key exchange. In addition, ciphers with DSA keys always use
# ephemeral DH keys. This can be used to achieve forward secrecy. If the file
# is in DSA parameters format, it will be automatically converted into DH
# params. This parameter is required if anonymous EAP-FAST is used.
# You can generate DH parameters file with OpenSSL, e.g.,
# "openssl dhparam -out /etc/hostapd.dh.pem 1024"
#dh_file=/etc/hostapd.dh.pem

# Fragment size for EAP methods
#fragment_size=1400

# Finite cyclic group for EAP-pwd. Number maps to group of domain parameters
# using the IANA repository for IKE (RFC 2409).
#pwd_group=19

# Configuration data for EAP-SIM database/authentication gateway interface.
# This is a text string in implementation specific format. The example
# implementation in eap_sim_db.c uses this as the UNIX domain socket name for
# the HLR/AuC gateway (e.g., hlr_auc_gw). In this case, the path uses "unix:"
# prefix. If hostapd is built with SQLite support (CONFIG_SQLITE=y in .config),
# database file can be described with an optional db=<path> parameter.
#eap_sim_db=unix:/tmp/hlr_auc_gw.sock
#eap_sim_db=unix:/tmp/hlr_auc_gw.sock db=/tmp/hostapd.db

# Encryption key for EAP-FAST PAC-Opaque values. This key must be a secret,
# random value. It is configured as a 16-octet value in hex format. It can be
# generated, e.g., with the following command:
# od -tx1 -v -N16 /dev/random | colrm 1 8 | tr -d ' '
#pac_opaque_encr_key=000102030405060708090a0b0c0d0e0f

# EAP-FAST authority identity (A-ID)
# A-ID indicates the identity of the authority that issues PACs. The A-ID
# should be unique across all issuing servers. In theory, this is a variable
# length field, but due to some existing implementations requiring A-ID to be
# 16 octets in length, it is strongly recommended to use that length for the
# field to provid interoperability with deployed peer implementations. This
# field is configured in hex format.
#eap_fast_a_id=101112131415161718191a1b1c1d1e1f

# EAP-FAST authority identifier information (A-ID-Info)
# This is a user-friendly name for the A-ID. For example, the enterprise name
# and server name in a human-readable format. This field is encoded as UTF-8.
#eap_fast_a_id_info=test server

# Enable/disable different EAP-FAST provisioning modes:
#0 = provisioning disabled
#1 = only anonymous provisioning allowed
#2 = only authenticated provisioning allowed
#3 = both provisioning modes allowed (default)
#eap_fast_prov=3

# EAP-FAST PAC-Key lifetime in seconds (hard limit)
#pac_key_lifetime=604800

# EAP-FAST PAC-Key refresh time in seconds (soft limit on remaining hard
# limit). The server will generate a new PAC-Key when this number of seconds
# (or fewer) of the lifetime remains.
#pac_key_refresh_time=86400

# EAP-SIM and EAP-AKA protected success/failure indication using AT_RESULT_IND
# (default: 0 = disabled).
#eap_sim_aka_result_ind=1

# Trusted Network Connect (TNC)
# If enabled, TNC validation will be required before the peer is allowed to
# connect. Note: This is only used with EAP-TTLS and EAP-FAST. If any other
# EAP method is enabled, the peer will be allowed to connect without TNC.
#tnc=1

##### IEEE 802.11f - Inter-Access Point Protocol (IAPP) #######################

# Interface to be used for IAPP broadcast packets
#iapp_interface=eth0

##### RADIUS client configuration #############################################
# for IEEE 802.1X with external Authentication Server, IEEE 802.11
# authentication with external ACL for MAC addresses, and accounting

# The own IP address of the access point (used as NAS-IP-Address)
own_ip_addr=127.0.0.1

# Optional NAS-Identifier string for RADIUS messages. When used, this should be
# a unique to the NAS within the scope of the RADIUS server. For example, a
# fully qualified domain name can be used here.
# When using IEEE 802.11r, nas_identifier must be set and must be between 1 and
# 48 octets long.
#nas_identifier=ap.example.com

# RADIUS authentication server
#auth_server_addr=127.0.0.1
#auth_server_port=1812
#auth_server_shared_secret=secret

# RADIUS accounting server
#acct_server_addr=127.0.0.1
#acct_server_port=1813
#acct_server_shared_secret=secret

# Secondary RADIUS servers; to be used if primary one does not reply to
# RADIUS packets. These are optional and there can be more than one secondary
# server listed.
#auth_server_addr=127.0.0.2
#auth_server_port=1812
#auth_server_shared_secret=secret2
#
#acct_server_addr=127.0.0.2
#acct_server_port=1813
#acct_server_shared_secret=secret2

# Retry interval for trying to return to the primary RADIUS server (in
# seconds). RADIUS client code will automatically try to use the next server
# when the current server is not replying to requests. If this interval is set,
# primary server will be retried after configured amount of time even if the
# currently used secondary server is still working.
#radius_retry_primary_interval=600

# Interim accounting update interval
# If this is set (larger than 0) and acct_server is configured, hostapd will
# send interim accounting updates every N seconds. Note: if set, this overrides
# possible Acct-Interim-Interval attribute in Access-Accept message. Thus, this
# value should not be configured in hostapd.conf, if RADIUS server is used to
# control the interim interval.
# This value should not be less 600 (10 minutes) and must not be less than
# 60 (1 minute).
#radius_acct_interim_interval=600

# Request Chargeable-User-Identity (RFC 4372)
# This parameter can be used to configure hostapd to request CUI from the
# RADIUS server by including Chargeable-User-Identity attribute into
# Access-Request packets.
#radius_request_cui=1

# Dynamic VLAN mode; allow RADIUS authentication server to decide which VLAN
# is used for the stations. This information is parsed from following RADIUS
# attributes based on RFC 3580 and RFC 2868: Tunnel-Type (value 13 = VLAN),
# Tunnel-Medium-Type (value 6 = IEEE 802), Tunnel-Private-Group-ID (value
# VLANID as a string). Optionally, the local MAC ACL list (accept_mac_file) can
# be used to set static client MAC address to VLAN ID mapping.
# 0 = disabled (default)
# 1 = option; use default interface if RADIUS server does not include VLAN ID
# 2 = required; reject authentication if RADIUS server does not include VLAN ID
#dynamic_vlan=0

# VLAN interface list for dynamic VLAN mode is read from a separate text file.
# This list is used to map VLAN ID from the RADIUS server to a network
# interface. Each station is bound to one interface in the same way as with
# multiple BSSIDs or SSIDs. Each line in this text file is defining a new
# interface and the line must include VLAN ID and interface name separated by
# white space (space or tab).
# If no entries are provided by this file, the station is statically mapped
# to <bss-iface>.<vlan-id> interfaces.
#vlan_file=/etc/hostapd.vlan

# Interface where 802.1q tagged packets should appear when a RADIUS server is
# used to determine which VLAN a station is on.  hostapd creates a bridge for
# each VLAN.  Then hostapd adds a VLAN interface (associated with the interface
# indicated by 'vlan_tagged_interface') and the appropriate wireless interface
# to the bridge.
#vlan_tagged_interface=eth0

# Bridge (prefix) to add the wifi and the tagged interface to. This gets the
# VLAN ID appended. It defaults to brvlan%d if no tagged interface is given
# and br%s.%d if a tagged interface is given, provided %s = tagged interface
# and %d = VLAN ID.
#vlan_bridge=brvlan

# When hostapd creates a VLAN interface on vlan_tagged_interfaces, it needs
# to know how to name it.
# 0 = vlan<XXX>, e.g., vlan1
# 1 = <vlan_tagged_interface>.<XXX>, e.g. eth0.1
#vlan_naming=0

# Arbitrary RADIUS attributes can be added into Access-Request and
# Accounting-Request packets by specifying the contents of the attributes with
# the following configuration parameters. There can be multiple of these to
# add multiple attributes. These parameters can also be used to override some
# of the attributes added automatically by hostapd.
# Format: <attr_id>[:<syntax:value>]
# attr_id: RADIUS attribute type (e.g., 26 = Vendor-Specific)
# syntax: s = string (UTF-8), d = integer, x = octet string
# value: attribute value in format indicated by the syntax
# If syntax and value parts are omitted, a null value (single 0x00 octet) is
# used.
#
# Additional Access-Request attributes
# radius_auth_req_attr=<attr_id>[:<syntax:value>]
# Examples:
# Operator-Name = "Operator"
#radius_auth_req_attr=126:s:Operator
# Service-Type = Framed (2)
#radius_auth_req_attr=6:d:2
# Connect-Info = "testing" (this overrides the automatically generated value)
#radius_auth_req_attr=77:s:testing
# Same Connect-Info value set as a hexdump
#radius_auth_req_attr=77:x:74657374696e67

#
# Additional Accounting-Request attributes
# radius_acct_req_attr=<attr_id>[:<syntax:value>]
# Examples:
# Operator-Name = "Operator"
#radius_acct_req_attr=126:s:Operator

# Dynamic Authorization Extensions (RFC 5176)
# This mechanism can be used to allow dynamic changes to user session based on
# commands from a RADIUS server (or some other disconnect client that has the
# needed session information). For example, Disconnect message can be used to
# request an associated station to be disconnected.
#
# This is disabled by default. Set radius_das_port to non-zero UDP port
# number to enable.
#radius_das_port=3799
#
# DAS client (the host that can send Disconnect/CoA requests) and shared secret
#radius_das_client=192.168.1.123 shared secret here
#
# DAS Event-Timestamp time window in seconds
#radius_das_time_window=300
#
# DAS require Event-Timestamp
#radius_das_require_event_timestamp=1

##### RADIUS authentication server configuration ##############################

# hostapd can be used as a RADIUS authentication server for other hosts. This
# requires that the integrated EAP server is also enabled and both
# authentication services are sharing the same configuration.

# File name of the RADIUS clients configuration for the RADIUS server. If this
# commented out, RADIUS server is disabled.
#radius_server_clients=/etc/hostapd.radius_clients

# The UDP port number for the RADIUS authentication server
#radius_server_auth_port=1812

# The UDP port number for the RADIUS accounting server
# Commenting this out or setting this to 0 can be used to disable RADIUS
# accounting while still enabling RADIUS authentication.
#radius_server_acct_port=1813

# Use IPv6 with RADIUS server (IPv4 will also be supported using IPv6 API)
#radius_server_ipv6=1

##### WPA/IEEE 802.11i configuration ##########################################

# Enable WPA. Setting this variable configures the AP to require WPA (either
# WPA-PSK or WPA-RADIUS/EAP based on other configuration). For WPA-PSK, either
# wpa_psk or wpa_passphrase must be set and wpa_key_mgmt must include WPA-PSK.
# Instead of wpa_psk / wpa_passphrase, wpa_psk_radius might suffice.
# For WPA-RADIUS/EAP, ieee8021x must be set (but without dynamic WEP keys),
# RADIUS authentication server must be configured, and WPA-EAP must be included
# in wpa_key_mgmt.
# This field is a bit field that can be used to enable WPA (IEEE 802.11i/D3.0)
# and/or WPA2 (full IEEE 802.11i/RSN):
# bit0 = WPA
# bit1 = IEEE 802.11i/RSN (WPA2) (dot11RSNAEnabled)
wpa=2

# WPA pre-shared keys for WPA-PSK. This can be either entered as a 256-bit
# secret in hex format (64 hex digits), wpa_psk, or as an ASCII passphrase
# (8..63 characters) that will be converted to PSK. This conversion uses SSID
# so the PSK changes when ASCII passphrase is used and the SSID is changed.
# wpa_psk (dot11RSNAConfigPSKValue)
# wpa_passphrase (dot11RSNAConfigPSKPassPhrase)
#wpa_psk=0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef
wpa_passphrase=123456789

# Optionally, WPA PSKs can be read from a separate text file (containing list
# of (PSK,MAC address) pairs. This allows more than one PSK to be configured.
# Use absolute path name to make sure that the files can be read on SIGHUP
# configuration reloads.
#wpa_psk_file=/etc/hostapd.wpa_psk

# Optionally, WPA passphrase can be received from RADIUS authentication server
# This requires macaddr_acl to be set to 2 (RADIUS)
# 0 = disabled (default)
# 1 = optional; use default passphrase/psk if RADIUS server does not include
# Tunnel-Password
# 2 = required; reject authentication if RADIUS server does not include
# Tunnel-Password
#wpa_psk_radius=0

# Set of accepted key management algorithms (WPA-PSK, WPA-EAP, or both). The
# entries are separated with a space. WPA-PSK-SHA256 and WPA-EAP-SHA256 can be
# added to enable SHA256-based stronger algorithms.
# (dot11RSNAConfigAuthenticationSuitesTable)
#wpa_key_mgmt=WPA-PSK WPA-EAP
wpa_key_mgmt=WPA-PSK

# Set of accepted cipher suites (encryption algorithms) for pairwise keys
# (unicast packets). This is a space separated list of algorithms:
# CCMP = AES in Counter mode with CBC-MAC [RFC 3610, IEEE 802.11i/D7.0]
# TKIP = Temporal Key Integrity Protocol [IEEE 802.11i/D7.0]
# Group cipher suite (encryption algorithm for broadcast and multicast frames)
# is automatically selected based on this configuration. If only CCMP is
# allowed as the pairwise cipher, group cipher will also be CCMP. Otherwise,
# TKIP will be used as the group cipher.
# (dot11RSNAConfigPairwiseCiphersTable)
# Pairwise cipher for WPA (v1) (default: TKIP)
wpa_pairwise=TKIP CCMP
# Pairwise cipher for RSN/WPA2 (default: use wpa_pairwise value)
#rsn_pairwise=CCMP
rsn_pairwise=CCMP TKIP

# Time interval for rekeying GTK (broadcast/multicast encryption keys) in
# seconds. (dot11RSNAConfigGroupRekeyTime)
#wpa_group_rekey=600

# Rekey GTK when any STA that possesses the current GTK is leaving the BSS.
# (dot11RSNAConfigGroupRekeyStrict)
#wpa_strict_rekey=1

# Time interval for rekeying GMK (master key used internally to generate GTKs
# (in seconds).
#wpa_gmk_rekey=86400

# Maximum lifetime for PTK in seconds. This can be used to enforce rekeying of
# PTK to mitigate some attacks against TKIP deficiencies.
#wpa_ptk_rekey=600

# Enable IEEE 802.11i/RSN/WPA2 pre-authentication. This is used to speed up
# roaming be pre-authenticating IEEE 802.1X/EAP part of the full RSN
# authentication and key handshake before actually associating with a new AP.
# (dot11RSNAPreauthenticationEnabled)
#rsn_preauth=1
#
# Space separated list of interfaces from which pre-authentication frames are
# accepted (e.g., 'eth0' or 'eth0 wlan0wds0'. This list should include all
# interface that are used for connections to other APs. This could include
# wired interfaces and WDS links. The normal wireless data interface towards
# associated stations (e.g., wlan0) should not be added, since
# pre-authentication is only used with APs other than the currently associated
# one.
#rsn_preauth_interfaces=eth0

# peerkey: Whether PeerKey negotiation for direct links (IEEE 802.11e) is
# allowed. This is only used with RSN/WPA2.
# 0 = disabled (default)
# 1 = enabled
#peerkey=1

# ieee80211w: Whether management frame protection (MFP) is enabled
# 0 = disabled (default)
# 1 = optional
# 2 = required
#ieee80211w=0

# Association SA Query maximum timeout (in TU = 1.024 ms; for MFP)
# (maximum time to wait for a SA Query response)
# dot11AssociationSAQueryMaximumTimeout, 1...4294967295
#assoc_sa_query_max_timeout=1000

# Association SA Query retry timeout (in TU = 1.024 ms; for MFP)
# (time between two subsequent SA Query requests)
# dot11AssociationSAQueryRetryTimeout, 1...4294967295
#assoc_sa_query_retry_timeout=201

# disable_pmksa_caching: Disable PMKSA caching
# This parameter can be used to disable caching of PMKSA created through EAP
# authentication. RSN preauthentication may still end up using PMKSA caching if
# it is enabled (rsn_preauth=1).
# 0 = PMKSA caching enabled (default)
# 1 = PMKSA caching disabled
#disable_pmksa_caching=0

# okc: Opportunistic Key Caching (aka Proactive Key Caching)
# Allow PMK cache to be shared opportunistically among configured interfaces
# and BSSes (i.e., all configurations within a single hostapd process).
# 0 = disabled (default)
# 1 = enabled
#okc=1

# SAE threshold for anti-clogging mechanism (dot11RSNASAEAntiCloggingThreshold)
# This parameter defines how many open SAE instances can be in progress at the
# same time before the anti-clogging mechanism is taken into use.
#sae_anti_clogging_threshold=5

# Enabled SAE finite cyclic groups
# SAE implementation are required to support group 19 (ECC group defined over a
# 256-bit prime order field). All groups that are supported by the
# implementation are enabled by default. This configuration parameter can be
# used to specify a limited set of allowed groups. The group values are listed
# in the IANA registry:
# http://www.iana.org/assignments/ipsec-registry/ipsec-registry.xml#ipsec-registry-9
#sae_groups=19 20 21 25 26

##### IEEE 802.11r configuration ##############################################

# Mobility Domain identifier (dot11FTMobilityDomainID, MDID)
# MDID is used to indicate a group of APs (within an ESS, i.e., sharing the
# same SSID) between which a STA can use Fast BSS Transition.
# 2-octet identifier as a hex string.
#mobility_domain=a1b2

# PMK-R0 Key Holder identifier (dot11FTR0KeyHolderID)
# 1 to 48 octet identifier.
# This is configured with nas_identifier (see RADIUS client section above).

# Default lifetime of the PMK-RO in minutes; range 1..65535
# (dot11FTR0KeyLifetime)
#r0_key_lifetime=10000

# PMK-R1 Key Holder identifier (dot11FTR1KeyHolderID)
# 6-octet identifier as a hex string.
#r1_key_holder=000102030405

# Reassociation deadline in time units (TUs / 1.024 ms; range 1000..65535)
# (dot11FTReassociationDeadline)
#reassociation_deadline=1000

# List of R0KHs in the same Mobility Domain
# format: <MAC address> <NAS Identifier> <128-bit key as hex string>
# This list is used to map R0KH-ID (NAS Identifier) to a destination MAC
# address when requesting PMK-R1 key from the R0KH that the STA used during the
# Initial Mobility Domain Association.
#r0kh=02:01:02:03:04:05 r0kh-1.example.com 000102030405060708090a0b0c0d0e0f
#r0kh=02:01:02:03:04:06 r0kh-2.example.com 00112233445566778899aabbccddeeff
# And so on.. One line per R0KH.

# List of R1KHs in the same Mobility Domain
# format: <MAC address> <R1KH-ID> <128-bit key as hex string>
# This list is used to map R1KH-ID to a destination MAC address when sending
# PMK-R1 key from the R0KH. This is also the list of authorized R1KHs in the MD
# that can request PMK-R1 keys.
#r1kh=02:01:02:03:04:05 02:11:22:33:44:55 000102030405060708090a0b0c0d0e0f
#r1kh=02:01:02:03:04:06 02:11:22:33:44:66 00112233445566778899aabbccddeeff
# And so on.. One line per R1KH.

# Whether PMK-R1 push is enabled at R0KH
# 0 = do not push PMK-R1 to all configured R1KHs (default)
# 1 = push PMK-R1 to all configured R1KHs whenever a new PMK-R0 is derived
#pmk_r1_push=1

##### Neighbor table ##########################################################
# Maximum number of entries kept in AP table (either for neigbor table or for
# detecting Overlapping Legacy BSS Condition). The oldest entry will be
# removed when adding a new entry that would make the list grow over this
# limit. Note! WFA certification for IEEE 802.11g requires that OLBC is
# enabled, so this field should not be set to 0 when using IEEE 802.11g.
# default: 255
#ap_table_max_size=255

# Number of seconds of no frames received after which entries may be deleted
# from the AP table. Since passive scanning is not usually performed frequently
# this should not be set to very small value. In addition, there is no
# guarantee that every scan cycle will receive beacon frames from the
# neighboring APs.
# default: 60
#ap_table_expiration_time=3600


##### Wi-Fi Protected Setup (WPS) #############################################

# WPS state
# 0 = WPS disabled (default)
# 1 = WPS enabled, not configured
# 2 = WPS enabled, configured
#wps_state=2

# Whether to manage this interface independently from other WPS interfaces
# By default, a single hostapd process applies WPS operations to all configured
# interfaces. This parameter can be used to disable that behavior for a subset
# of interfaces. If this is set to non-zero for an interface, WPS commands
# issued on that interface do not apply to other interfaces and WPS operations
# performed on other interfaces do not affect this interface.
#wps_independent=0

# AP can be configured into a locked state where new WPS Registrar are not
# accepted, but previously authorized Registrars (including the internal one)
# can continue to add new Enrollees.
#ap_setup_locked=1

# Universally Unique IDentifier (UUID; see RFC 4122) of the device
# This value is used as the UUID for the internal WPS Registrar. If the AP
# is also using UPnP, this value should be set to the device's UPnP UUID.
# If not configured, UUID will be generated based on the local MAC address.
#uuid=12345678-9abc-def0-1234-56789abcdef0

# Note: If wpa_psk_file is set, WPS is used to generate random, per-device PSKs
# that will be appended to the wpa_psk_file. If wpa_psk_file is not set, the
# default PSK (wpa_psk/wpa_passphrase) will be delivered to Enrollees. Use of
# per-device PSKs is recommended as the more secure option (i.e., make sure to
# set wpa_psk_file when using WPS with WPA-PSK).

# When an Enrollee requests access to the network with PIN method, the Enrollee
# PIN will need to be entered for the Registrar. PIN request notifications are
# sent to hostapd ctrl_iface monitor. In addition, they can be written to a
# text file that could be used, e.g., to populate the AP administration UI with
# pending PIN requests. If the following variable is set, the PIN requests will
# be written to the configured file.
#wps_pin_requests=/var/run/hostapd_wps_pin_requests

# Device Name
# User-friendly description of device; up to 32 octets encoded in UTF-8
#device_name=Wireless AP

# Manufacturer
# The manufacturer of the device (up to 64 ASCII characters)
#manufacturer=Company

# Model Name
# Model of the device (up to 32 ASCII characters)
#model_name=WAP

# Model Number
# Additional device description (up to 32 ASCII characters)
#model_number=123

# Serial Number
# Serial number of the device (up to 32 characters)
#serial_number=12345

# Primary Device Type
# Used format: <categ>-<OUI>-<subcateg>
# categ = Category as an integer value
# OUI = OUI and type octet as a 4-octet hex-encoded value; 0050F204 for
#       default WPS OUI
# subcateg = OUI-specific Sub Category as an integer value
# Examples:
#   1-0050F204-1 (Computer / PC)
#   1-0050F204-2 (Computer / Server)
#   5-0050F204-1 (Storage / NAS)
#   6-0050F204-1 (Network Infrastructure / AP)
#device_type=6-0050F204-1

# OS Version
# 4-octet operating system version number (hex string)
#os_version=01020300

# Config Methods
# List of the supported configuration methods
# Available methods: usba ethernet label display ext_nfc_token int_nfc_token
# nfc_interface push_button keypad virtual_display physical_display
# virtual_push_button physical_push_button
#config_methods=label virtual_display virtual_push_button keypad

# WPS capability discovery workaround for PBC with Windows 7
# Windows 7 uses incorrect way of figuring out AP's WPS capabilities by acting
# as a Registrar and using M1 from the AP. The config methods attribute in that
# message is supposed to indicate only the configuration method supported by
# the AP in Enrollee role, i.e., to add an external Registrar. For that case,
# PBC shall not be used and as such, the PushButton config method is removed
# from M1 by default. If pbc_in_m1=1 is included in the configuration file,
# the PushButton config method is left in M1 (if included in config_methods
# parameter) to allow Windows 7 to use PBC instead of PIN (e.g., from a label
# in the AP).
#pbc_in_m1=1

# Static access point PIN for initial configuration and adding Registrars
# If not set, hostapd will not allow external WPS Registrars to control the
# access point. The AP PIN can also be set at runtime with hostapd_cli
# wps_ap_pin command. Use of temporary (enabled by user action) and random
# AP PIN is much more secure than configuring a static AP PIN here. As such,
# use of the ap_pin parameter is not recommended if the AP device has means for
# displaying a random PIN.
#ap_pin=12345670

# Skip building of automatic WPS credential
# This can be used to allow the automatically generated Credential attribute to
# be replaced with pre-configured Credential(s).
#skip_cred_build=1

# Additional Credential attribute(s)
# This option can be used to add pre-configured Credential attributes into M8
# message when acting as a Registrar. If skip_cred_build=1, this data will also
# be able to override the Credential attribute that would have otherwise been
# automatically generated based on network configuration. This configuration
# option points to an external file that much contain the WPS Credential
# attribute(s) as binary data.
#extra_cred=hostapd.cred

# Credential processing
#   0 = process received credentials internally (default)
#   1 = do not process received credentials; just pass them over ctrl_iface to
# external program(s)
#   2 = process received credentials internally and pass them over ctrl_iface
# to external program(s)
# Note: With wps_cred_processing=1, skip_cred_build should be set to 1 and
# extra_cred be used to provide the Credential data for Enrollees.
#
# wps_cred_processing=1 will disabled automatic updates of hostapd.conf file
# both for Credential processing and for marking AP Setup Locked based on
# validation failures of AP PIN. An external program is responsible on updating
# the configuration appropriately in this case.
#wps_cred_processing=0

# AP Settings Attributes for M7
# By default, hostapd generates the AP Settings Attributes for M7 based on the
# current configuration. It is possible to override this by providing a file
# with pre-configured attributes. This is similar to extra_cred file format,
# but the AP Settings attributes are not encapsulated in a Credential
# attribute.
#ap_settings=hostapd.ap_settings

# WPS UPnP interface
# If set, support for external Registrars is enabled.
#upnp_iface=br0

# Friendly Name (required for UPnP)
# Short description for end use. Should be less than 64 characters.
#friendly_name=WPS Access Point

# Manufacturer URL (optional for UPnP)
#manufacturer_url=http://www.example.com/

# Model Description (recommended for UPnP)
# Long description for end user. Should be less than 128 characters.
#model_description=Wireless Access Point

# Model URL (optional for UPnP)
#model_url=http://www.example.com/model/

# Universal Product Code (optional for UPnP)
# 12-digit, all-numeric code that identifies the consumer package.
#upc=123456789012

# WPS RF Bands (a = 5G, b = 2.4G, g = 2.4G, ag = dual band)
# This value should be set according to RF band(s) supported by the AP if
# hw_mode is not set. For dual band dual concurrent devices, this needs to be
# set to ag to allow both RF bands to be advertized.
#wps_rf_bands=ag

# NFC password token for WPS
# These parameters can be used to configure a fixed NFC password token for the
# AP. This can be generated, e.g., with nfc_pw_token from wpa_supplicant. When
# these parameters are used, the AP is assumed to be deployed with a NFC tag
# that includes the matching NFC password token (e.g., written based on the
# NDEF record from nfc_pw_token).
#
#wps_nfc_dev_pw_id: Device Password ID (16..65535)
#wps_nfc_dh_pubkey: Hexdump of DH Public Key
#wps_nfc_dh_privkey: Hexdump of DH Private Key
#wps_nfc_dev_pw: Hexdump of Device Password

##### Wi-Fi Direct (P2P) ######################################################

# Enable P2P Device management
#manage_p2p=1

# Allow cross connection
#allow_cross_connection=1

#### TDLS (IEEE 802.11z-2010) #################################################

# Prohibit use of TDLS in this BSS
#tdls_prohibit=1

# Prohibit use of TDLS Channel Switching in this BSS
#tdls_prohibit_chan_switch=1

##### IEEE 802.11v-2011 #######################################################
# Time advertisement
# 0 = disabled (default)
# 2 = UTC time at which the TSF timer is 0
#time_advertisement=2

# Local time zone as specified in 8.3 of IEEE Std 1003.1-2004:
# stdoffset[dst[offset][,start[/time],end[/time]]]
#time_zone=EST5

# WNM-Sleep Mode (extended sleep mode for stations)
# 0 = disabled (default)
# 1 = enabled (allow stations to use WNM-Sleep Mode)
#wnm_sleep_mode=1

# BSS Transition Management
# 0 = disabled (default)
# 1 = enabled
#bss_transition=1


##### IEEE 802.11u-2011 #######################################################


# Enable Interworking service
#interworking=1


# Access Network Type
# 0 = Private network
# 1 = Private network with guest access
# 2 = Chargeable public network
# 3 = Free public network
# 4 = Personal device network
# 5 = Emergency services only network
# 14 = Test or experimental
# 15 = Wildcard
#access_network_type=0


# Whether the network provides connectivity to the Internet
# 0 = Unspecified
# 1 = Network provides connectivity to the Internet
#internet=1


# Additional Step Required for Access
# Note: This is only used with open network, i.e., ASRA shall ne set to 0 if
# RSN is used.
#asra=0


# Emergency services reachable
#esr=0


# Unauthenticated emergency service accessible
#uesa=0


# Venue Info (optional)
# The available values are defined in IEEE Std 802.11u-2011, 7.3.1.34.
# Example values (group,type):
# 0,0 = Unspecified
# 1,7 = Convention Center
# 1,13 = Coffee Shop
# 2,0 = Unspecified Business
# 7,1  Private Residence
#venue_group=7
#venue_type=1


# Homogeneous ESS identifier (optional; dot11HESSID)
# If set, this shall be identifical to one of the BSSIDs in the homogeneous
# ESS and this shall be set to the same value across all BSSs in homogeneous
# ESS.
#hessid=02:03:04:05:06:07


# Roaming Consortium List
# Arbitrary number of Roaming Consortium OIs can be configured with each line
# adding a new OI to the list. The first three entries are available through
# Beacon and Probe Response frames. Any additional entry will be available only
# through ANQP queries. Each OI is between 3 and 15 octets and is configured as
# a hexstring.
#roaming_consortium=021122
#roaming_consortium=2233445566


# Venue Name information
# This parameter can be used to configure one or more Venue Name Duples for
# Venue Name ANQP information. Each entry has a two or three character language
# code (ISO-639) separated by colon from the venue name string.
# Note that venue_group and venue_type have to be set for Venue Name
# information to be complete.
#venue_name=eng:Example venue
#venue_name=fin:Esimerkkipaikka
# Alternative format for language:value strings:
# (double quoted string, printf-escaped string)
#venue_name=P"eng:Example\nvenue"


# Network Authentication Type
# This parameter indicates what type of network authentication is used in the
# network.
# format: <network auth type indicator (1-octet hex str)> [redirect URL]
# Network Authentication Type Indicator values:
# 00 = Acceptance of terms and conditions
# 01 = On-line enrollment supported
# 02 = http/https redirection
# 03 = DNS redirection
#network_auth_type=00
#network_auth_type=02http://www.example.com/redirect/me/here/


# IP Address Type Availability
# format: <1-octet encoded value as hex str>
# (ipv4_type & 0x3f) << 2 | (ipv6_type & 0x3)
# ipv4_type:
# 0 = Address type not available
# 1 = Public IPv4 address available
# 2 = Port-restricted IPv4 address available
# 3 = Single NATed private IPv4 address available
# 4 = Double NATed private IPv4 address available
# 5 = Port-restricted IPv4 address and single NATed IPv4 address available
# 6 = Port-restricted IPv4 address and double NATed IPv4 address available
# 7 = Availability of the address type is not known
# ipv6_type:
# 0 = Address type not available
# 1 = Address type available
# 2 = Availability of the address type not known
#ipaddr_type_availability=14


# Domain Name
# format: <variable-octet str>[,<variable-octet str>]
#domain_name=example.com,another.example.com,yet-another.example.com


# 3GPP Cellular Network information
# format: <MCC1,MNC1>[;<MCC2,MNC2>][;...]
#anqp_3gpp_cell_net=244,91;310,026;234,56


# NAI Realm information
# One or more realm can be advertised. Each nai_realm line adds a new realm to
# the set. These parameters provide information for stations using Interworking
# network selection to allow automatic connection to a network based on
# credentials.
# format: <encoding>,<NAI Realm(s)>[,<EAP Method 1>][,<EAP Method 2>][,...]
# encoding:
# 0 = Realm formatted in accordance with IETF RFC 4282
# 1 = UTF-8 formatted character string that is not formatted in
#    accordance with IETF RFC 4282
# NAI Realm(s): Semi-colon delimited NAI Realm(s)
# EAP Method: <EAP Method>[:<[AuthParam1:Val1]>][<[AuthParam2:Val2]>][...]
# AuthParam (Table 8-188 in IEEE Std 802.11-2012):
# ID 2 = Non-EAP Inner Authentication Type
# 1 = PAP, 2 = CHAP, 3 = MSCHAP, 4 = MSCHAPV2
# ID 3 = Inner authentication EAP Method Type
# ID 5 = Credential Type
# 1 = SIM, 2 = USIM, 3 = NFC Secure Element, 4 = Hardware Token,
# 5 = Softoken, 6 = Certificate, 7 = username/password, 9 = Anonymous,
# 10 = Vendor Specific
#nai_realm=0,example.com;example.net
# EAP methods EAP-TLS with certificate and EAP-TTLS/MSCHAPv2 with
# username/password
#nai_realm=0,example.org,13[5:6],21[2:4][5:7]


# QoS Map Set configuration
#
# Comma delimited QoS Map Set in decimal values
# (see IEEE Std 802.11-2012, 8.4.2.97)
#
# format:
# [<DSCP Exceptions[DSCP,UP]>,]<UP 0 range[low,high]>,...<UP 7 range[low,high]>
#
# There can be up to 21 optional DSCP Exceptions which are pairs of DSCP Value
# (0..63 or 255) and User Priority (0..7). This is followed by eight DSCP Range
# descriptions with DSCP Low Value and DSCP High Value pairs (0..63 or 255) for
# each UP starting from 0. If both low and high value are set to 255, the
# corresponding UP is not used.
#
# default: not set
#qos_map_set=53,2,22,6,8,15,0,7,255,255,16,31,32,39,255,255,40,47,255,255


##### Hotspot 2.0 #############################################################


# Enable Hotspot 2.0 support
#hs20=1


# Disable Downstream Group-Addressed Forwarding (DGAF)
# This can be used to configure a network where no group-addressed frames are
# allowed. The AP will not forward any group-address frames to the stations and
# random GTKs are issued for each station to prevent associated stations from
# forging such frames to other stations in the BSS.
#disable_dgaf=1


# OSU Server-Only Authenticated L2 Encryption Network
#osen=1


# ANQP Domain ID (0..65535)
# An identifier for a set of APs in an ESS that share the same common ANQP
# information. 0 = Some of the ANQP information is unique to this AP (default).
#anqp_domain_id=1234


# Deauthentication request timeout
# If the RADIUS server indicates that the station is not allowed to connect to
# the BSS/ESS, the AP can allow the station some time to download a
# notification page (URL included in the message). This parameter sets that
# timeout in seconds.
#hs20_deauth_req_timeout=60


# Operator Friendly Name
# This parameter can be used to configure one or more Operator Friendly Name
# Duples. Each entry has a two or three character language code (ISO-639)
# separated by colon from the operator friendly name string.
#hs20_oper_friendly_name=eng:Example operator
#hs20_oper_friendly_name=fin:Esimerkkioperaattori


# Connection Capability
# This can be used to advertise what type of IP traffic can be sent through the
# hotspot (e.g., due to firewall allowing/blocking protocols/ports).
# format: <IP Protocol>:<Port Number>:<Status>
# IP Protocol: 1 = ICMP, 6 = TCP, 17 = UDP
# Port Number: 0..65535
# Status: 0 = Closed, 1 = Open, 2 = Unknown
# Each hs20_conn_capab line is added to the list of advertised tuples.
#hs20_conn_capab=1:0:2
#hs20_conn_capab=6:22:1
#hs20_conn_capab=17:5060:0


# WAN Metrics
# format: <WAN Info>:<DL Speed>:<UL Speed>:<DL Load>:<UL Load>:<LMD>
# WAN Info: B0-B1: Link Status, B2: Symmetric Link, B3: At Capabity
#    (encoded as two hex digits)
#    Link Status: 1 = Link up, 2 = Link down, 3 = Link in test state
# Downlink Speed: Estimate of WAN backhaul link current downlink speed in kbps;
# 1..4294967295; 0 = unknown
# Uplink Speed: Estimate of WAN backhaul link current uplink speed in kbps
# 1..4294967295; 0 = unknown
# Downlink Load: Current load of downlink WAN connection (scaled to 255 = 100%)
# Uplink Load: Current load of uplink WAN connection (scaled to 255 = 100%)
# Load Measurement Duration: Duration for measuring downlink/uplink load in
# tenths of a second (1..65535); 0 if load cannot be determined
#hs20_wan_metrics=01:8000:1000:80:240:3000


# Operating Class Indication
# List of operating classes the BSSes in this ESS use. The Global operating
# classes in Table E-4 of IEEE Std 802.11-2012 Annex E define the values that
# can be used in this.
# format: hexdump of operating class octets
# for example, operating classes 81 (2.4 GHz channels 1-13) and 115 (5 GHz
# channels 36-48):
#hs20_operating_class=5173


# OSU icons
# <Icon Width>:<Icon Height>:<Language code>:<Icon Type>:<Name>:<file path>
#hs20_icon=32:32:eng:image/png:icon32:/tmp/icon32.png
#hs20_icon=64:64:eng:image/png:icon64:/tmp/icon64.png


# OSU SSID (see ssid2 for format description)
# This is the SSID used for all OSU connections to all the listed OSU Providers.
#osu_ssid="example"


# OSU Providers
# One or more sets of following parameter. Each OSU provider is started by the
# mandatory osu_server_uri item. The other parameters add information for the
# last added OSU provider.
#
#osu_server_uri=https://example.com/osu/
#osu_friendly_name=eng:Example operator
#osu_friendly_name=fin:Esimerkkipalveluntarjoaja
#osu_nai=anonymous@example.com
#osu_method_list=1 0
#osu_icon=icon32
#osu_icon=icon64
#osu_service_desc=eng:Example services
#osu_service_desc=fin:Esimerkkipalveluja
#
#osu_server_uri=...

##### TESTING OPTIONS #########################################################
#
# The options in this section are only available when the build configuration
# option CONFIG_TESTING_OPTIONS is set while compiling hostapd. They allow
# testing some scenarios that are otherwise difficult to reproduce.
#
# Ignore probe requests sent to hostapd with the given probability, must be a
# floating point number in the range [0, 1).
#ignore_probe_probability=0.0
#
# Ignore authentication frames with the given probability
#ignore_auth_probability=0.0
#
# Ignore association requests with the given probability
#ignore_assoc_probability=0.0
#
# Ignore reassociation requests with the given probability
#ignore_reassoc_probability=0.0
#
# Corrupt Key MIC in GTK rekey EAPOL-Key frames with the given probability
#corrupt_gtk_rekey_mic_probability=0.0


##### Multiple BSSID support ##################################################
#
# Above configuration is using the default interface (wlan#, or multi-SSID VLAN
# interfaces). Other BSSIDs can be added by using separator 'bss' with
# default interface name to be allocated for the data packets of the new BSS.
#
# hostapd will generate BSSID mask based on the BSSIDs that are
# configured. hostapd will verify that dev_addr & MASK == dev_addr. If this is
# not the case, the MAC address of the radio must be changed before starting
# hostapd (ifconfig wlan0 hw ether <MAC addr>). If a BSSID is configured for
# every secondary BSS, this limitation is not applied at hostapd and other
# masks may be used if the driver supports them (e.g., swap the locally
# administered bit)
#
# BSSIDs are assigned in order to each BSS, unless an explicit BSSID is
# specified using the 'bssid' parameter.
# If an explicit BSSID is specified, it must be chosen such that it:
# - results in a valid MASK that covers it and the dev_addr
# - is not the same as the MAC address of the radio
# - is not the same as any other explicitly specified BSSID
#
# Please note that hostapd uses some of the values configured for the first BSS
# as the defaults for the following BSSes. However, it is recommended that all
# BSSes include explicit configuration of all relevant configuration items.
#
#bss=wlan0_0
#ssid=test2
# most of the above items can be used here (apart from radio interface specific
# items, like channel)

#bss=wlan0_1
#bssid=00:13:10:95:fe:0b
# ...
 */
