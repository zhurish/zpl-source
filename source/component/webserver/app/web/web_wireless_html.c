/*
 * web_wireless_html.c
 *
 *  Created on: 2019年8月11日
 *      Author: zhurish
 */
#define HAS_BOOL 1
#include "zplos_include.h"
#include "vty.h"
#include "if.h"

#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "zmemory.h"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "nsm_ipvrf.h"
#include "nsm_interface.h"
//#include "nsm_client.h"

#ifdef ZPL_WIFI_MODULE

#include "iw_config.h"
#include "iw_ap.h"
#include "iw_client.h"
#include "iw_interface.h"

#include "web_util.h"
#include "web_jst.h"
#include "web_app.h"
#include "web_api.h"


static int web_wireless_disable_job(void *p);
//附近wifi
static int web_iw_client_neighbor_show_one(iw_client_ap_t *ap, Webs *wp)
{
	if (ap)
	{
		char buf[64];
		char freq[32];
		struct prefix prefix_eth;
		union prefix46constptr pu;
		prefix_eth.family = AF_ETHERNET;

		memcpy(prefix_eth.u.prefix_eth.octet, ap->BSSID, NSM_MAC_MAX);
		pu.p = &prefix_eth;

		if (wp->iValue)
			websWrite(wp, "%s", ",");

		//FREQ
		memset(freq, 0, sizeof(freq));
		iw_print_freq_value(freq, sizeof(freq), ap->freq);

		websWrite(wp,
				  "{\"name\":\"%s\", \"enable\":true, \"mac\":\"%s\", \"encrytype\":\"%s\", \"signal\":%d, \
				\"qual\":\"%d\", \"freq\":\"%s\", \"interface\":\"%s\", \"state\":\"%s\"}",
				  ap->SSID, prefix_2_address_str(pu, buf, sizeof(buf)),
				  ap->auth ? "on" : "off", 110 - abs(ap->signal), ap->qaul, freq,
				  ifindex2ifname(ap->ifindex), ap->connect ? "connect" : "disconnect");

		wp->iValue++;
	}
	return OK;
}
static int web_wireless_neighbor_tbl(Webs *wp, char *path, char *query)
{
	struct interface *ifp = NULL;
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	//websWriteHeader(wp, "Content-Type", "application/json"/*"text/plain"*/);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	websWrite(wp, "%s", "[");
	wp->iValue = 0;
	ifp = if_lookup_by_name("wireless 0/0/1");
	if (ifp)
	{
		iw_t *iw = nsm_iw_get(ifp);
		if (iw && iw->enable)
		{
			if (iw->mode == IW_MODE_MANAGE || iw->mode == IW_MODE_CLIENT)
			{
				//if(iw->private.ap)
				iw_client_neighbor_callback_api(&iw->private.client, web_iw_client_neighbor_show_one, wp);
			}
		}
	}
	wp->iValue = 0;
	websWrite(wp, "%s", "]");
	websDone(wp);
	return OK;
}

//扫描或者断开连接
static int web_wireless_scan_disconnect(Webs *wp, void *p)
{
	char *btnid = NULL;
	int ret = 0;
	btnid = webs_get_var(wp, T("BTNID"), T(""));
	if (NULL == btnid)
	{
		return ERROR; //web_return_text_plain(wp, ERROR);
	}
	if (strstr(btnid, "ap_disconnect"))
	{
		struct interface *ifp = if_lookup_by_name("wireless 0/0/1");
		if (ifp)
		{
			iw_t *iw = nsm_iw_get(ifp);
			if (iw && iw->enable)
			{
				if (iw->mode == IW_MODE_MANAGE || iw->mode == IW_MODE_CLIENT)
				{
					//if(iw->private.client)
					ret = iw_client_disconnect_api(&iw->private.client);
				}
			}
		}
	}
	if (strstr(btnid, "ap_scan"))
	{
		struct interface *ifp = if_lookup_by_name("wireless 0/0/1");
		if (ifp)
		{
			iw_t *iw = nsm_iw_get(ifp);
			if (iw && iw->enable)
			{
				if (iw->mode == IW_MODE_MANAGE || iw->mode == IW_MODE_CLIENT)
				{
					iw->private.client.scan_enable = zpl_true;
#ifdef IW_ONCE_TASK
					if (iw->private.client.scan_thread)
					{
						thread_cancel(iw->private.client.scan_thread);
						iw->private.client.scan_thread = NULL;
					}
#endif
					iw_client_scan_start(&iw->private.client);
					//ret = iw_client_disconnect_api(&iw->private.client);
					os_sleep(2);
				}
			}
		}
	}
	if (ret != OK)
		return ERROR; //
	return web_return_text_plain(wp, ret);
	//return OK;
}
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0)
//连接到wifi
static int web_wireless_sta(Webs *wp, char *path, char *query)
{
	char *ssid = NULL;
	char *pass = NULL;
	struct interface *ifp = NULL;
	ssid = webs_get_var(wp, T("name"), T(""));
	if (NULL == ssid)
	{
		return web_return_text_plain(wp, ERROR);
	}
	_WEB_DBG_TRAP("%s: name=%s\r\n", __func__, ssid);
	pass = webs_get_var(wp, T("password"), T(""));
	if (NULL == pass)
	{
		return web_return_text_plain(wp, ERROR);
	}
	_WEB_DBG_TRAP("%s: password=%s\r\n", __func__, pass);

	ifp = if_lookup_by_name("wireless 0/0/1");
	if (ifp)
	{
		iw_t *iw = nsm_iw_get(ifp);
		if (iw /* && iw->enable*/)
		{
			if (iw->mode == IW_MODE_MANAGE || iw->mode == IW_MODE_CLIENT)
			{
				websSetStatus(wp, 200);
				websWriteHeaders(wp, -1, 0);
				websWriteHeader(wp, "Content-Type", "application/json");
				websWriteEndHeaders(wp);

				iw_client_db_set_api(&iw->private.client, ssid, pass);
				websWrite(wp,
						  "{\"response\":\"%s\", \"enable\":%s, \"name\":\"%s\", \"mode\":\"%s\", \"password\":\"%s\"}",
						  "OK", iw->enable ? "true" : "false", ssid, "STA", pass);
				websDone(wp);
				vty_execute_shell(NULL, "write memory");
				return OK;
			}
		}
	}
	web_return_text_plain(wp, ERROR);
	return ERROR;
}
#endif

//已连接WIFI
static int web_already_connect_show_one(iw_ap_connect_t *ap, Webs *wp)
{
	if (ap)
	{
		char buf[128];
		struct prefix prefix_eth;
		union prefix46constptr pu;
		prefix_eth.family = AF_ETHERNET;
		memcpy(prefix_eth.u.prefix_eth.octet, ap->BSSID, NSM_MAC_MAX);
		pu.p = &prefix_eth;

		//SSID BSSID
		memset(buf, 0, sizeof(buf));

		websWrite(wp, "{\"name\":\"%s\", \"enable\":true, \"mode\":\"%s\", \"mac\":\"%s\", \"signal\":\"%d\", \"auth\":\"%s\", \"connect\":\"%d\"},",
				  " ", "AP", prefix_2_address_str(pu, buf, sizeof(buf)), 110 - abs(ap->signal),
				  ap->authenticated ? "yes" : "no", ap->connected_time);
	}
	return OK;
}

static int web_wireless_client_tbl(Webs *wp, char *path, char *query)
{
	struct interface *ifp = NULL;
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	//websWriteHeader(wp, "Content-Type", "application/json"/*"text/plain"*/);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	websWrite(wp, "%s", "[");
	wp->iValue = 0;
	ifp = if_lookup_by_name("wireless 0/0/1");
	if (ifp)
	{
		iw_t *iw = nsm_iw_get(ifp);
		if (iw && iw->enable)
		{
			if (iw->mode == IW_MODE_AP)
			{
				//if(iw->private.ap)
				iw_ap_connect_callback_api(&iw->private.ap, web_already_connect_show_one, wp);
				//iw_ap_connect_callback_api(iw_ap, iw_ap_connect_show_one, vty);
			}
		}
	}
	/*
	websWrite(wp,"{\"name\":\"%s\", \"mode\":\"%s\", \"mac\":\"%s\", \"signal\":\"%s\"},",
			"abcdnd1", "AP", "48:45:20:a5:89:2a", "56");
	websWrite(wp,"{\"name\":\"%s\", \"mode\":\"%s\", \"mac\":\"%s\", \"signal\":\"%s\"},",
			"abcdnd2", "AP", "48:45:20:a5:89:2b", "57");
	websWrite(wp,"{\"name\":\"%s\", \"mode\":\"%s\", \"mac\":\"%s\", \"signal\":\"%s\"}",
			"abcdnd3", "AP", "48:45:20:a5:89:2c", "58");
*/
	wp->iValue = 0;
	websWrite(wp, "%s", "]");
	websDone(wp);
	return OK;
}

//设备wifi参数
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0)
static int web_wireless_ap(Webs *wp, char *path, char *query)
{
	char *name = NULL;
	//char *mode = NULL;
	char *password = NULL;
	char *encrytype = NULL;
	char *type = NULL;
	char *freq = NULL;
	char *chennel = NULL;
	char *hiden = NULL;

	int ret = 0;
	iw_t *iw = NULL;
	struct interface *ifp = NULL;
	ifp = if_lookup_by_name("wireless 0/0/1");
	if (ifp)
	{
		iw = nsm_iw_get(ifp);
		if (iw)
		{
			if (iw->mode != IW_MODE_AP)
			{
				ret |= nsm_iw_mode_set_api(ifp, IW_MODE_AP);
			}
		}
		else
			return web_return_text_plain(wp, ERROR);
	}
	else
		return web_return_text_plain(wp, ERROR);

	name = webs_get_var(wp, T("name"), T(""));
	if (NULL == name)
	{
		return web_return_text_plain(wp, ERROR);
	}

	ret |= iw_ap_ssid_set_api(&iw->private.ap, name);

	encrytype = webs_get_var(wp, T("encrytype"), T(""));
	if (NULL == encrytype)
	{
		return web_return_text_plain(wp, ERROR);
	}
	if (strstr(encrytype, "NONE"))
		ret |= iw_ap_auth_set_api(&iw->private.ap, IW_ENCRY_NONE);
	else if (strstr(encrytype, "SHARED"))
		ret |= iw_ap_auth_set_api(&iw->private.ap, IW_ENCRY_WEP_PRIVATE);
	else if (strstr(encrytype, "OPEN"))
		ret |= iw_ap_auth_set_api(&iw->private.ap, IW_ENCRY_WEP_OPEN);
	else if (strstr(encrytype, "WPA2-PSK"))
		ret |= iw_ap_auth_set_api(&iw->private.ap, IW_ENCRY_WPA2_PSK);
	else if (strstr(encrytype, "WPA-PSK"))
		ret |= iw_ap_auth_set_api(&iw->private.ap, IW_ENCRY_WPA_PSK);
	else if (strstr(encrytype, "WPA-WPA2-PSK"))
		ret |= iw_ap_auth_set_api(&iw->private.ap, IW_ENCRY_WPA2WPA_PSK);

	if (!strstr(encrytype, "NONE"))
	{
		password = webs_get_var(wp, T("password"), T(""));
		if (NULL == password)
		{
			return web_return_text_plain(wp, ERROR);
		}
		ret |= iw_ap_password_set_api(&iw->private.ap, password);
	}

	type = webs_get_var(wp, T("type"), T(""));
	if (NULL == type)
	{
		return web_return_text_plain(wp, ERROR);
	}
	//ret |= iw_ap_hw_mode_set_api(&iw->private.ap, type);

	freq = webs_get_var(wp, T("freq"), T(""));
	if (NULL == freq)
	{
		return web_return_text_plain(wp, ERROR);
	}
	//ret |= iw_ap_freq_set_api(&iw->private.ap, freq);

	hiden = webs_get_var(wp, T("hiden"), T(""));

	if (hiden && strstr(hiden, "true"))
		iw->private.ap.hiden_ssid = zpl_true;
	else
		iw->private.ap.hiden_ssid = zpl_false;

	/*
	chennel = webs_get_var(wp, T("chennel"), T(""));
	if (NULL == chennel)
	{
		return web_return_text_plain(wp, ERROR);
	}
	ret |= iw_ap_channel_set_api(&iw->private.ap, atoi(chennel));
*/

	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "application/json");
	websWriteEndHeaders(wp);

	websWrite(wp,
			  "{\"response\":\"%s\", \"enable\":%s, \"name\":\"%s\", \"mode\":\"%s\", \"password\":\"%s\", \"encrytype\":\"%s\", "
			  "\"type\":\"%s\", \"freq\":\"%s\", \"chennel\":\"%s\", \"hiden\":\"%s\"}",
			  "OK", iw->enable ? "true" : "false", name, "AP", password ? password : "", encrytype, type, freq, chennel, hiden ? hiden : "false");
	websDone(wp);
	vty_execute_shell(NULL, "write memory");
	return OK;
}
#else
static int web_wireless_ap(Webs *wp, char *path, char *query)
{
	char *name = NULL;
	//char *mode = NULL;
	char *password = NULL;
	char *encrytype = NULL;
	char *type = NULL;
	char *freq = NULL;
	char *chennel = NULL;
	char *hiden = NULL;

	int ret = 0;
	char tmp[64];
	memset(tmp, 0, sizeof(tmp));

	//zlog_debug(MODULE_WEB, "=======%s======", __func__);

	os_uci_get_integer("wireless.radio0.disabled", &ret);
	if (ret == 1)
		os_uci_set_integer("wireless.radio0.disabled", 0);
	os_uci_get_string("wireless.radio0.mode", tmp);
	if (!strstr(tmp, "ap"))
	{
		os_uci_set_string("wireless.radio0.mode", "ap");
	}

	name = webs_get_var(wp, T("name"), T(""));
	if (NULL == name)
	{
		zlog_debug(MODULE_WEB, "=======%s======name", __func__);
		return web_return_text_plain(wp, ERROR);
	}
	os_uci_set_string("wireless.radio0.ssid", name);

	encrytype = webs_get_var(wp, T("encrytype"), T(""));
	if (NULL == encrytype)
	{
		zlog_debug(MODULE_WEB, "=======%s======encrytype", __func__);
		return web_return_text_plain(wp, ERROR);
	}
	
	//zlog_debug(MODULE_WEB, "=======%s======encrytype=%s", __func__, encrytype);

	if (strstr(encrytype, "NONE"))
		os_uci_set_string("wireless.radio0.encryption", "none");
	else if (strstr(encrytype, "SHARED"))
		os_uci_set_string("wireless.radio0.encryption", "shared");
	else if (strstr(encrytype, "OPEN"))
		os_uci_set_string("wireless.radio0.encryption", "open");
	else if (strstr(encrytype, "WPA2-PSK"))
		os_uci_set_string("wireless.radio0.encryption", "wpa2psk");
	else if (strstr(encrytype, "WPA-PSK"))
		os_uci_set_string("wireless.radio0.encryption", "wpapsk");
	else if (strstr(encrytype, "WPA-WPA2-PSK"))
		os_uci_set_string("wireless.radio0.encryption", "psk+psk2");
	
	if (!strstr(encrytype, "NONE"))
	{
		password = webs_get_var(wp, T("password"), T(""));
		if (NULL == password)
		{
			zlog_debug(MODULE_WEB, "=======%s======password", __func__);
			return web_return_text_plain(wp, ERROR);
		}
		//zlog_debug(MODULE_WEB, "=======%s======password=%s", __func__, password);
		os_uci_set_string("wireless.radio0.key", password);
	}

	type = webs_get_var(wp, T("type"), T(""));
	/*
	if (NULL == type)
	{
		zlog_debug(MODULE_WEB, "=======%s======type", __func__);
		return web_return_text_plain(wp, ERROR);
	}
	if (strstr(type, "Auto"))
		os_uci_set_string("wireless.radio0.encryption", "none");
	else if (strstr(type, "11b/g/n"))
		os_uci_set_string("wireless.radio0.encryption", "shared");
	else if (strstr(type, "11b/g"))
		os_uci_set_string("wireless.radio0.encryption", "open");
	else if (strstr(type, "11b"))
		os_uci_set_string("wireless.radio0.encryption", "wpa2psk");
	else if (strstr(type, "119"))
		os_uci_set_string("wireless.radio0.encryption", "wpapsk");

	//os_uci_set_integer("wireless.ra0.mode", atoi(type));
	*/
	freq = webs_get_var(wp, T("freq"), T(""));
	if (NULL == freq)
	{
		zlog_debug(MODULE_WEB, "=======%s======freq", __func__);
		return web_return_text_plain(wp, ERROR);
	}
	os_uci_set_integer("wireless.ra0.ht", atoi(freq));

	hiden = webs_get_var(wp, T("hiden"), T(""));

	if (hiden && strstr(hiden, "true"))
		os_uci_set_integer("wireless.ra0.hidessid", 1);
	else
		os_uci_set_integer("wireless.ra0.hidessid", 0);
	//os_uci_get_string("wireless.ra0.channel", chennel);

	chennel = webs_get_var(wp, T("chennel"), T(""));
	if (NULL == chennel)
	{
		return web_return_text_plain(wp, ERROR);
	}
	os_uci_save_config("wireless");

	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "application/json");
	websWriteEndHeaders(wp);

	websWrite(wp,
			  "{\"response\":\"%s\", \"enable\":%s, \"name\":\"%s\", \"mode\":\"%s\", \"password\":\"%s\", \"encrytype\":\"%s\", "
			  "\"type\":\"%s\", \"freq\":\"%s\", \"chennel\":\"%s\", \"hiden\":\"%s\"}",
			  "OK", "true", name ? name : "", "AP", password ? password : "", encrytype ? encrytype : "", 
			  type ? type : "", freq ? freq : "", chennel ? chennel : "", hiden ? hiden : "false");
	websDone(wp);

	web_return_text_plain(wp, OK);

	os_job_add(OS_JOB_NONE,web_wireless_disable_job, NULL);

	return OK;
}
#endif

static char *web_iw_hw_mode_string(iw_hw_mode_t mode)
{
	switch (mode)
	{
	case IW_HW_MODE_IEEE80211ANY:
		return "Auto";
		break;
	case IW_HW_MODE_IEEE80211A:
		return "11a";
		break;
	case IW_HW_MODE_IEEE80211B:
		return "11b";
		break;
	case IW_HW_MODE_IEEE80211D:
		return "11d";
		break;
	case IW_HW_MODE_IEEE80211H:
		return "11h";
		break;
	case IW_HW_MODE_IEEE80211G:
		return "11g";
		break;
	case IW_HW_MODE_IEEE80211AD:
		return "11a/d";
		break;
	case IW_HW_MODE_IEEE80211AC:
		return "11a/c";
		break;
	case IW_HW_MODE_IEEE80211N:
		return "11n";
		break;
	case IW_HW_MODE_IEEE80211AB:
		return "11a/b";
		break;
	case IW_HW_MODE_IEEE80211AG:
		return "11a/g";
		break;
	case IW_HW_MODE_IEEE80211AN:
		return "11a/n";
		break;
	case IW_HW_MODE_IEEE80211BG:
		return "11b/g";
		break;
	case IW_HW_MODE_IEEE80211NG:
		return "11n/g";
		break;
	case IW_HW_MODE_IEEE80211BGN:
		return "11b/g/n";
		break;
	default:
		return "Unknown";
		break;
	}
	return "Unknown";
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
static int web_wireless_action_get(Webs *wp, char *path, char *query)
{
#ifdef WEB_OPENWRT_PROCESS
	char tmp[64];
	zpl_uint32 val = 0, mode = 0;
	memset(tmp, 0, sizeof(tmp));
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "application/json");
	websWriteEndHeaders(wp);
	os_uci_get_string("wireless.radio0.mode", tmp);
	if (strstr(tmp, "ap"))
		mode = IW_MODE_AP;
	else
		mode = IW_MODE_MANAGE;

	if (mode == IW_MODE_AP)
	{
		char name[64], password[64], encrytype[64], freq[64], chennel[64];
		zpl_uint32 type = 0, hiden = 0;
		memset(name, 0, sizeof(name));
		memset(password, 0, sizeof(password));
		memset(encrytype, 0, sizeof(encrytype));
		memset(freq, 0, sizeof(freq));
		memset(chennel, 0, sizeof(chennel));

		os_uci_get_integer("wireless.radio0.disabled", &val);
		os_uci_get_string("wireless.radio0.ssid", name);
		os_uci_get_string("wireless.radio0.encryption", encrytype);
		os_uci_get_string("wireless.radio0.key", password);

		os_uci_get_integer("wireless.ra0.mode", &type);
		os_uci_get_string("wireless.ra0.ht", freq);
		os_uci_get_string("wireless.ra0.channel", chennel);
		os_uci_get_integer("wireless.ra0.hidessid", &hiden);

		if (strstr(encrytype, "none"))
		{
			memset(encrytype, 0, sizeof(encrytype));
			strcpy(encrytype, "NONE");
		}
		else if (strstr(encrytype, "shared"))
		{
			memset(encrytype, 0, sizeof(encrytype));
			strcpy(encrytype, "SHARED");
		}
		else if (strstr(encrytype, "open"))
		{
			memset(encrytype, 0, sizeof(encrytype));
			strcpy(encrytype, "OPEN");
		}
		else if (strncmp(encrytype, "psk-mixed", 5) == 0 || strncmp(encrytype, "psk+psk2", 5) == 0 || 
			strncmp(encrytype, "mixed", 5) == 0 || strncmp(encrytype, "Mixed", 5) == 0 )
		{
			memset(encrytype, 0, sizeof(encrytype));
			strcpy(encrytype, "WPA-WPA2-PSK");
		}		
		else if (strncmp(encrytype, "wpa2-psk", 4) == 0 || strncmp(encrytype, "WPA2-psk", 4) == 0 || 
			strncmp(encrytype, "PSK2", 4) == 0 || strncmp(encrytype, "psk2", 4) == 0)
		{
			memset(encrytype, 0, sizeof(encrytype));
			strcpy(encrytype, "WPA2-PSK");
		}
		else if (strncmp(encrytype, "wpa1-psk", 4) == 0 || strncmp(encrytype, "WPA1-psk", 4) == 0 || 
			strncmp(encrytype, "WPA", 3) == 0 || strncmp(encrytype, "wpa", 3) == 0 ||
			strncmp(encrytype, "PSK", 3) == 0 || strncmp(encrytype, "psk", 3) == 0)
		{
			memset(encrytype, 0, sizeof(encrytype));
			strcpy(encrytype, "WPA-PSK");
		}

		websWrite(wp,
				  "{\"response\":\"%s\", \"enable\":%s, \"name\":\"%s\", \"mode\":\"%s\", \"password\":\"%s\", \"encrytype\":\"%s\", "
				  "\"type\":\"%s\", \"freq\":\"%s\", \"chennel\":\"%s\", \"hiden\":\"%s\"}",
				  "OK", val ? "false" : "true", name, "AP", password,
				  encrytype, web_iw_hw_mode_string(type), freq,
				  chennel,
				  hiden ? "true" : "false");
	}
	else if (mode == IW_MODE_MANAGE)
	{
		os_uci_get_integer("wireless.radio0.ApCliEnable", &val);
		websWrite(wp, "{\"response\":\"%s\", \"enable\":%s, \"name\":\"%s\", \"mode\":\"%s\"}", "OK", val ? "false" : "true", " ", "STA");
	}
	websDone(wp);
	return OK;
#endif
}
#endif
static int web_wireless_action(Webs *wp, char *path, char *query)
{
	char *strval = NULL;

	strval = webs_get_var(wp, T("ACTION"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	if (strstr(strval, "GET"))
	{
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0)
		/*
		strval = webs_get_var(wp, T("mode"), T(""));
		if (NULL == strval)
		{
			return ERROR;
		}
		_WEB_DBG_TRAP("%s: mode=%s\r\n", __func__, strval);
*/
		//if(strstr(strval, "AP"))
		{
			struct interface *ifp = NULL;
			wp->iValue = 0;
			ifp = if_lookup_by_name("wireless 0/0/1");
			if (ifp)
			{
				iw_t *iw = nsm_iw_get(ifp);
				if (iw)
				{
					websSetStatus(wp, 200);
					websWriteHeaders(wp, -1, 0);
					websWriteHeader(wp, "Content-Type", "application/json");
					websWriteEndHeaders(wp);

					if (iw->mode == IW_MODE_AP)
					{
						char authstr[32];
						memset(authstr, 0, sizeof(authstr));
						sprintf(authstr, "%s", iw_auth_string(iw->private.ap.auth));
						websWrite(wp,
								  "{\"response\":\"%s\", \"enable\":%s, \"name\":\"%s\", \"mode\":\"%s\", \"password\":\"%s\", \"encrytype\":\"%s\", "
								  "\"type\":\"%s\", \"freq\":\"%s\", \"chennel\":\"%s\", \"hiden\":\"%s\"}",
								  "OK", iw->enable ? "true" : "false", iw->private.ap.SSID, "AP", iw->private.ap.password[0].password,
								  strupr(authstr), web_iw_hw_mode_string(iw->private.ap.hw_mode), "Auto",
								  (iw->private.ap.channel != IW_AP_CHANNEL_DEFAULT) ? itoa(iw->private.ap.channel, 10) : "Auto",
								  iw->private.ap.hiden_ssid ? "true" : "false");
					}
					else if (iw->mode == IW_MODE_MANAGE)
						websWrite(wp,
								  "{\"response\":\"%s\", \"enable\":%s, \"name\":\"%s\", \"mode\":\"%s\"}", "OK", iw->enable ? "true" : "false", " ", "STA");
					else if (iw->mode == IW_MODE_NONE)
					{
						websWrite(wp,
								  "{\"response\":\"%s\", \"enable\":false, \"name\":\"%s\", \"mode\":\"%s\"}", "OK", " ", "STA");
					}
					websDone(wp);
					return OK;
				}
				else
					return web_return_text_plain(wp, ERROR);
			}
			else
				return web_return_text_plain(wp, ERROR);
		}
#else
		//zlog_debug(MODULE_WEB, "=======%s======%s", __func__, strval);
		return web_wireless_action_get(wp, path, query);
#endif
	}
	else
	{
		strval = webs_get_var(wp, T("mode"), T(""));
		if (NULL == strval)
		{
			return web_return_text_plain(wp, ERROR);
		}
		//zlog_debug(MODULE_WEB, "=======%s======%s", __func__, strval);
		if (strstr(strval, "AP"))
			return web_wireless_ap(wp, path, query);
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0)			
		else
			return web_wireless_sta(wp, path, query);
#else	
		return ERROR;
#endif
	}
	return OK;
}

static int web_wireless_disable_job(void *p)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0)
	struct interface *ifp = p;
	os_sleep(1);
	nsm_iw_enable_api(ifp, zpl_false);
	vty_execute_shell(NULL, "write memory");
#else
#ifdef WEB_OPENWRT_PROCESS
	os_sleep(1);
	super_system("/etc/init.d/network restart");
#endif
#endif
	return OK;
}

static int web_wireless_client_disable(Webs *wp, void *p)
{
	int ret = ERROR;
	char *strval = NULL;
	struct interface *ifp = NULL;
	strval = webs_get_var(wp, T("enable"), T(""));
	if (strval)
	{
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0)
		ifp = if_lookup_by_name("wireless 0/0/1");
		if (ifp)
		{
			iw_t *iw = nsm_iw_get(ifp);
			if (iw)
			{
				//_WEB_DBG_TRAP("============%s==============:strval=%s\r\n", __func__, strval);
				if (strstr(strval, "true"))
				{
					nsm_iw_enable_api(ifp, zpl_true);
					vty_execute_shell(NULL, "write memory");
					return web_return_text_plain(wp, OK);
				}
				else if (strstr(strval, "false"))
				{
					os_job_add(OS_JOB_NONE,web_wireless_disable_job, ifp);
/*					nsm_iw_enable_api(ifp, zpl_false);
					vty_execute_shell(NULL, "write memory");*/
					return web_return_text_plain(wp, OK);
				}
			}
			else
			{
				//return web_return_text_plain(wp, ERROR);
				return ERROR;
			}
		}
		else
		{
			//return web_return_text_plain(wp, ERROR);
			return ERROR;
		}
#else
		_WEB_DBG_TRAP( "=======%s======%s", __func__, strval);
		if (strstr(strval, "true"))
		{
#ifdef WEB_OPENWRT_PROCESS
			os_uci_set_integer("wireless.radio0.disabled", 0);
			os_uci_save_config("wireless");
			os_job_add(OS_JOB_NONE,web_wireless_disable_job, NULL);
#endif
			return web_return_text_plain(wp, OK);
		}
		else if (strstr(strval, "false"))
		{
#ifdef WEB_OPENWRT_PROCESS
			os_uci_set_integer("wireless.radio0.disabled", 1);
			os_uci_save_config("wireless");
			os_job_add(OS_JOB_NONE,web_wireless_disable_job, NULL);
#endif
			return web_return_text_plain(wp, OK);
		}
		else
		{
			//return web_return_text_plain(wp, ERROR);
			return ERROR;
		}
#endif
		//vty_execute_shell(NULL, "write memory");
		//return web_return_text_plain(wp, OK);
	}
	strval = webs_get_var(wp, T("mac"), T(""));
	if (NULL == strval)
	{
		return ERROR; //web_return_text_plain(wp, ERROR);
	}
	ifp = if_lookup_by_name("wireless 0/0/1");
	if (ifp)
	{
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0)
		iw_t *iw = nsm_iw_get(ifp);
		if (iw)
		{
			if (iw->mode == IW_MODE_AP)
			{
				//if(iw->private.ap)
				ret = iw_ap_mac_add_api(&iw->private.ap, strval, zpl_false);
			}
		}
#endif
	}
	if (ret != OK)
		return ERROR; //
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0)		
	vty_execute_shell(NULL, "write memory");
#endif	
	return web_return_text_plain(wp, ret);
}

//#endif

int web_wireless_app(void)
{
//#ifdef ZPL_WIFI_MODULE
	websFormDefine("wireless", web_wireless_action);
	web_button_add_hook("wireless", "active", web_wireless_client_disable, NULL);
	web_button_add_hook("wireless", "ap_disable", web_wireless_client_disable, NULL); //禁止设备连接到AP
	websFormDefine("clienttbl-tbl", web_wireless_client_tbl);						  //已经连接到AP的客户端列表

	websFormDefine("wireless-neighbor", web_wireless_neighbor_tbl);						  //附近wifi
	web_button_add_hook("wireless", "ap_disconnect", web_wireless_scan_disconnect, NULL); //断开连接
	web_button_add_hook("wireless", "ap_scan", web_wireless_scan_disconnect, NULL);		  //扫描附近wifi
//#endif
	return 0;
}
#endif
