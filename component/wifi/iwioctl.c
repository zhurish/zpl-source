/*
 * wifi.c
 *
 *  Created on: Jul 15, 2018
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

#include "iw_config.h"
#include "iw_ap.h"
#include "iw_client.h"
#include "iw_interface.h"

#include "iwlib.h"

#include "iwioctl.h"

#define _LINUX_IF_H
#include "dirent.h"
#include "linux/wireless.h"

#ifndef CONFIG_IW_TOOLS
//#define DAEMON_VTY_DIR	"/var/tmp/"
#define NET_BASE_DIR	"/sys/class/net/"
#endif

iw_tmp_t iw_tmp =
{
#ifndef CONFIG_IW_TOOLS
	.filename = WIFI_FILE_NAME,
#endif
};

const char * kname2ifname(const char *kname)
{
	ifindex_t ifindex = ifname2kernelifindex(kname);
	if(ifindex)
		return ifindex2ifname(ifindex);
	return "Unknown";
}

static int iw_vty_obuf_swap(struct vty *vty, char *buf, int len)
{
	int i = 0, j = 0;
	char *p = XREALLOC(MTYPE_VTY_OUT_BUF, p, len + 256);
	if (!p)
		return -1;
	while(1)
	{
		if(buf[i] == '\n')
		{
			p[j++] = '\r';
			p[j++] = '\n';
		}
		else
			p[j++] = buf[i];
		if(i == len)
			break;
		i++;
	}
	buffer_put(vty->obuf, (u_char *) p, j);
	XFREE(MTYPE_VTY_OUT_BUF, p);
	return 0;
}

int iw_printf(const char *format, ...)
{
	if (iw_tmp.vty)
	{
		va_list args;
		int len = 0;
		int size = 1024;
		char buf[1024];
		char *p = NULL;
		struct vty *vty = iw_tmp.vty;
		if (vty_shell(vty))
		{
			va_start(args, format);
			vprintf(format, args);
			va_end(args);
		}
		else
		{
			/* Try to write to initial buffer.  */
			va_start(args, format);
			len = vsnprintf(buf, sizeof(buf), format, args);
			va_end(args);

			/* Initial buffer is not enough.  */
			if (len < 0 || len >= size)
			{
				while (1)
				{
					if (len > -1)
						size = len + 1;
					else
						size = size * 2;

					p = XREALLOC(MTYPE_VTY_OUT_BUF, p, size);
					if (!p)
						return -1;

					va_start(args, format);
					len = vsnprintf(p, size, format, args);
					va_end(args);

					if (len > -1 && len < size)
						break;
				}
			}
			/* When initial buffer is enough to store all output.  */
			if (!p)
				p = buf;
			/* Pointer p must point out buffer. */
			if(vty->type == VTY_TERM)
				iw_vty_obuf_swap(vty, (u_char *) p, len);
			else
				buffer_put(vty->obuf, (u_char *) p, len);
			/* If p is not different with buf, it is allocated buffer.  */
			if (p != buf)
				XFREE(MTYPE_VTY_OUT_BUF, p);
			return len;
		}
	}
	return 0;
}


int iw_fprintf(void *fp, const char *format, ...)
{
	if(fp)
	{
		va_list args;
		int len = 0;
		char buf[1024];
		os_memset(buf, 0, sizeof(buf));
		va_start(args, format);
		len = vsnprintf(buf, sizeof(buf), format, args);
		va_end(args);
		zlog_err(ZLOG_WIFI, "%s",buf);
	}
	else
	{
		va_list args;
		int len = 0;
		char buf[1024];
		os_memset(buf, 0, sizeof(buf));
		/* Try to write to initial buffer.  */
		va_start(args, format);
		len = vsnprintf(buf, sizeof(buf), format, args);
		va_end(args);
		iw_printf(buf);
	}
	return 0;
}

const char *wifi_file_path(void)
{
	static char filepath[256];
	os_memset(filepath, 0, sizeof(filepath));
	os_sprintf(filepath, "%s/%s", DAEMON_VTY_DIR, WIFI_FILE_NAME);
	return filepath;
}


static int ifname_to_phyid(char *name)
{
	char buf[200];
	int fd = 0;
	assert(name != NULL);
	os_memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf), "%s%s/phy80211/index", NET_BASE_DIR, name);
	fd = open(buf, O_RDONLY);
	if (fd < 0)
		return -1;
	os_memset(buf, 0, sizeof(buf));
	if (read(fd, buf, sizeof(buf) - 1) < 0) {
		close(fd);
		return -1;
	}
	close(fd);
	buf[strlen(buf)] = '\0';
	return atoi(buf);
}

//iwconfig wlan0 txpower NmW|NdBm|off|auto
//iwconfig wlan0 rts 100/auto|fixed|off
//iwconfig wlan0 frag 1300/auto|fixed|off
//iwconfig wlan0 rate {N[k|M|G]|auto|fixed
//iwconfig wlan0 bit {N[k|M|G]|auto|fixed
//dev <devname> set mcast_rate <rate in Mbps>
//dev <devname> set power_save <on|off>
//dev <devname> set bitrates [legacy-<2.4|5> <legacy rate in Mbps>*]
//	[ht-mcs-<2.4|5> <MCS index>*] [vht-mcs-<2.4|5> <NSS:MCSx,MCSy... |
//	NSS:MCSx-MCSy>*] [sgi-2.4|lgi-2.4] [sgi-5|lgi-5]

int iw_dev_mode_set(struct interface *ifp, char * value)
{
	assert(ifp != NULL);
	if(ifp->k_ifindex && if_is_wireless(ifp))
	{
		const char *tmpcmd[5] = {"iwconfig", ifp->k_name, "mode", value, NULL};
		return iw_main(4, tmpcmd);
	}
	return ERROR;
}

int iw_dev_txpower_set(struct interface *ifp, char * value)
{
	assert(ifp != NULL);
	if(ifp->k_ifindex && if_is_wireless(ifp))
	{
		const char *tmpcmd[5] = {"iwconfig", ifp->k_name, "txpower", value, NULL};
		return iw_main(4, tmpcmd);
	}
	return ERROR;
}

int iw_dev_rts_threshold_set(struct interface *ifp, char * value)
{
	assert(ifp != NULL);
	if(ifp->k_ifindex && if_is_wireless(ifp))
	{
		const char *tmpcmd[5] = {"iwconfig", ifp->k_name, "rts", value, NULL};
		return iw_main(4, tmpcmd);
	}
	return ERROR;
}

int iw_dev_frag_set(struct interface *ifp, char * value)
{
	assert(ifp != NULL);
	if(ifp->k_ifindex && if_is_wireless(ifp))
	{
		const char *tmpcmd[5] = {"iwconfig", ifp->k_name, "frag", value, NULL};
		return iw_main(4, tmpcmd);
	}
	return ERROR;
}

int iw_dev_rate_set(struct interface *ifp, char * value)
{
	assert(ifp != NULL);
	if(ifp->k_ifindex && if_is_wireless(ifp))
	{
		const char *tmpcmd[5] = {"iwconfig", ifp->k_name, "rate", value, NULL};
		return iw_main(4, tmpcmd);
	}
	return ERROR;
}

int iw_dev_bit_set(struct interface *ifp, char * value)
{
	assert(ifp != NULL);
	if(ifp->k_ifindex && if_is_wireless(ifp))
	{
		const char *tmpcmd[5] = {"iwconfig", ifp->k_name, "bit", value, NULL};
		return iw_main(4, tmpcmd);
	}
	return ERROR;
}

int iw_dev_channel_set(struct interface *ifp, char *  value)
{
	assert(ifp != NULL);
	if(ifp->k_ifindex && if_is_wireless(ifp))
	{
		const char *tmpcmd[5] = {"iwconfig", ifp->k_name, "channel", value, NULL};
		return iw_main(4, tmpcmd);
	}
	return ERROR;
}
/*int iw_dev_txpower_set(struct interface *ifp, struct vty *vty, char *type, int value)
{
	if(ifp->k_ifindex && if_is_wireless(ifp))
	{
		char tmpcmd[256];
		remove(wifi_file_path());
		os_memset(tmpcmd, 0, sizeof(tmpcmd));
		if(value >= 0)
			os_sprintf(tmpcmd, "iw dev %s set txpower %s %d", ifp->k_name, type, value);
		else
			os_sprintf(tmpcmd, "iw dev %s set txpower %s ", ifp->k_name, type);
		super_system(tmpcmd);
		//wifi_show_from_file(vty);
	}
	return OK;
}


int iw_dev_channel_set(struct interface *ifp, struct vty *vty, int value)
{
	if(ifp->k_ifindex && if_is_wireless(ifp))
	{
		char tmpcmd[256];
		remove(wifi_file_path());
		os_memset(tmpcmd, 0, sizeof(tmpcmd));
		if(value >= 0)
			os_sprintf(tmpcmd, "iw dev %s set channel %d", ifp->k_name, value);
		else
			os_sprintf(tmpcmd, "iw dev %s set channel auto", ifp->k_name);
		super_system(tmpcmd);
		//wifi_show_from_file(vty);
	}
	return OK;
}*/

/*
int iw_ap_dev_channel_set(struct interface *ifp, int chan)
{
	if(ifp->k_ifindex && if_is_wireless(ifp))
	{
		struct iwreq iwr;

		 set channel
		memset(&iwr, 0, sizeof(iwr));
		strncpy(iwr.ifr_name, ifp->k_name, sizeof(iwr.ifr_name)-1);
		iwr.u.freq.flags = IW_FREQ_FIXED;
		iwr.u.freq.m = chan;
		if (if_ioctl(SIOCSIWFREQ, &iwr) < 0)
		{
			return -1;
		}
		return 0;
	}
	return -1;
}
*/

/*********************************************************************************/
/*********************************************************************************/

/*
 * iw list = iw phy phy0 info
 * Wiphy phy0
 */
static int iw_phy_information_split(struct vty *vty, char *phy, char *name)
{
	FILE *fp = NULL;
	char buf[512];
	char *s;
	char *filepath = wifi_file_path();
	fp = fopen(filepath, "r");
	if(fp)
	{
		while (fgets(buf, sizeof(buf), fp))
		{
			for (s = buf + strlen(buf);
					(s > buf) && isspace((int) *(s - 1)); s--)
				;
			*s = '\0';

			if(strstr(buf, phy))
			{
				vty_out(vty, " Interface %s:%s", name, VTY_NEWLINE);
			}
			else
				vty_out(vty, "%s%s", buf, VTY_NEWLINE);
		}
		fclose(fp);
	}
	return OK;
}

int iw_phy_capabilities_show(struct interface *ifp, struct vty *vty)
{
	assert(ifp != NULL);
	assert(vty != NULL);
	if(ifp->k_ifindex && if_is_wireless(ifp))
	{
		char tmpcmd[128];
		int phy = ifname_to_phyid(ifp->k_name);
		if(phy >= 0)
		{
			remove(wifi_file_path());
			os_memset(tmpcmd, 0, sizeof(tmpcmd));
			os_sprintf(tmpcmd, "iw phy phy%d info >> %s 2>&1", phy, wifi_file_path());
			super_system(tmpcmd);
			os_memset(tmpcmd, 0, sizeof(tmpcmd));
			os_sprintf(tmpcmd, "phy%d", phy);
			iw_phy_information_split(vty, tmpcmd, ifp->name);
		}
	}
	return OK;
}

static int iw_dev_information_split(struct vty *vty, char *kname, char *name)
{
	FILE *fp = NULL;
	char buf[512];
	char *s = NULL;
	char *filepath = wifi_file_path();
	fp = fopen(filepath, "r");
	if(fp)
	{
		while (fgets(buf, sizeof(buf), fp))
		{
			for (s = buf + strlen(buf);
					(s > buf) && isspace((int) *(s - 1)); s--)
				;
			*s = '\0';
			if(strstr(buf, kname))
			{
				vty_out(vty, " Interface         : %s %s", name, VTY_NEWLINE);
			}
			else
			{
				s = strstr(buf, "addr");
				if(s)
					vty_out(vty, "  MAC Address      : %s%s", s+5, VTY_NEWLINE);

				s = strstr(buf, "ssid");
				if(s)
					vty_out(vty, "  AP SSID          : %s%s", s+5, VTY_NEWLINE);

				s = strstr(buf, "type");
				if(s)
					vty_out(vty, "  AP Type          : %s%s", s+5, VTY_NEWLINE);

				s = strstr(buf, "channel");
				if(s)
				{
					char tmp[512];
					char *p = NULL;
					p = s + strlen("channel ");
					sscanf(p, "%s[^,]", tmp);
					vty_out(vty, "  Channel          : %s%s", tmp, VTY_NEWLINE);
				}
				s = strstr(buf, "width");
				if(s)
				{
					char tmp[512];
					char *p = NULL;
					p = s + strlen("width: ");
					sscanf(p, "%s[^,]", tmp);
					vty_out(vty, "  Width            : %s%s", tmp, VTY_NEWLINE);
				}
			}
		}
		fclose(fp);
	}
	return OK;
}
/*
root@OpenWrt:/home# iw dev wlan0 info
Interface wlan0
        ifindex 67
        wdev 0x34
        addr 66:e8:c1:e2:4f:47
        ssid OpenWrt
        type AP
        wiphy 0
        channel 5 (2432 MHz), width: 20 MHz, center1: 2432 MHz
root@OpenWrt:/home#
*/
int iw_ap_dev_information_show(struct interface *ifp, struct vty *vty)
{
	assert(ifp != NULL);
	assert(vty != NULL);
	if(ifp->k_ifindex && if_is_wireless(ifp))
	{
		char tmpcmd[128];
		remove(wifi_file_path());
		os_memset(tmpcmd, 0, sizeof(tmpcmd));
		os_sprintf(tmpcmd, "iw dev %s info >> %s 2>&1", ifp->k_name, wifi_file_path());
		super_system(tmpcmd);
		iw_dev_information_split(vty, ifp->k_name, ifp->name);
	}
	return OK;
}

/*
[zhurish@localhost wifi]$ iw dev wlp3s0 station dump
Station b0:df:c1:d7:a8:50 (on wlp3s0)
	inactive time:	406 ms
	rx bytes:	2497207009
	rx packets:	5136379
	tx bytes:	756774164
	tx packets:	2638315
	tx retries:	490182
	tx failed:	11
	signal:  	-44 [-44] dBm
	signal avg:	-44 [-44] dBm
	tx bitrate:	81.0 MBit/s MCS 4 40MHz
	rx bitrate:	81.0 MBit/s MCS 4 40MHz
	authorized:	yes
	authenticated:	yes
	preamble:	long
	WMM/WME:	yes
	MFP:		no
	TDLS peer:	no
	connected time:	35458 seconds
[zhurish@localhost wifi]$
 */
static int iw_dev_station_dump_split(struct vty *vty, char *name, char *kname)
{
	FILE *fp = NULL;
	char buf[512];
	char *s = NULL;
	char *filepath = wifi_file_path();
	fp = fopen(filepath, "r");
	if(fp)
	{
		while (fgets(buf, sizeof(buf), fp))
		{
			for (s = buf + strlen(buf); (s > buf) && isspace((int) *(s - 1)); s--)
				;
			*s = '\0';
			if(strstr(buf, "Station"))
			{
				//Station b0:df:c1:d7:a8:50 (on wlp3s0)
				//	/
				s = strstr(buf, kname);
				if(s)
				{
					strcpy(s, name);
				}
				vty_out(vty, "	%s%s", buf, VTY_NEWLINE);
			}
			else
				vty_out(vty, "%s%s", buf, VTY_NEWLINE);
		}
		fclose(fp);
	}
	return OK;
}

int iw_client_dev_station_dump_show(struct interface *ifp, struct vty *vty)
{
	assert(ifp != NULL);
	assert(vty != NULL);
	if(ifp->k_ifindex && if_is_wireless(ifp))
	{
		char tmpcmd[128];
		remove(wifi_file_path());
		os_memset(tmpcmd, 0, sizeof(tmpcmd));
		os_sprintf(tmpcmd, "iw dev %s station dump >> %s 2>&1", ifp->k_name, wifi_file_path());
		super_system(tmpcmd);
		iw_dev_station_dump_split(vty, ifp->name, ifp->k_name);
	}
	return OK;
}

/*
 * show support channel/freq
 */
int iw_dev_channel_support_show(struct interface *ifp, struct vty *vty)
{
	assert(ifp != NULL);
	assert(vty != NULL);
	if(ifp->k_ifindex && if_is_wireless(ifp))
	{
		const char *tmpcmd[4] = {"iwlist", ifp->k_name, "channel", NULL};
		iw_tmp.vty = vty;
		iwlist_main(3, tmpcmd);
		iw_tmp.vty = NULL;
	}
	return OK;
}

/*
 * show current connect information
 */
int iw_client_dev_connect_show(struct interface *ifp, struct vty *vty)
{
	assert(ifp != NULL);
	assert(vty != NULL);
	if(ifp->k_ifindex && if_is_wireless(ifp))
	{
		iwlist_detail_set(1);
		const char *tmpcmd[3] = {"iwconfig", ifp->k_name, NULL};
		iw_tmp.vty = vty;
		iw_main(2, tmpcmd);
		iw_tmp.vty = NULL;
		iwlist_detail_set(0);
	}
	return OK;
}

/*
 * scanning AP
 */
int iw_client_dev_scan_ap_show(struct interface *ifp, struct vty *vty, int detail)
{
	assert(ifp != NULL);
	assert(vty != NULL);
	if(ifp->k_ifindex && if_is_wireless(ifp))
	{
		if(IW_DEBUG(EVENT))
		{
			zlog_debug(ZLOG_WIFI, "scanning network on interface %s ",ifp->name);
		}
		iwlist_detail_set(detail);
		const char *tmpcmd[4] = {"iwlist", ifp->k_name, "scanning", NULL};
		iw_tmp.vty = vty;
		iwlist_main(3, tmpcmd);
/*		const char *tmpcmd1[4] = {"iwlist", ifp->k_name, "auth", NULL};
		iwlist_main(3, tmpcmd1);
		const char *tmpcmd2[4] = {"iwlist", ifp->k_name, "wpakeys", NULL};
		iwlist_main(3, tmpcmd2);*/
		iw_tmp.vty = NULL;
		iwlist_detail_set(0);
	}
	return OK;
}

static int iw_client_dev_connect_script(struct interface *ifp, iw_client_ap_t *ap, char *ssid, char *password)
{
	FILE *fp = NULL;
	char path[256];
	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path), "%s/wpa_supplicant.%s", IW_SYSCONFDIR, ifp->k_name);
	fp = fopen(path,"w+");
	if(fp)
	{
		fprintf(fp, "country=00\n");
		fprintf(fp, "network={\n");
		fprintf(fp, "	scan_ssid=1\n");
		fprintf(fp, "	ssid=\"%s\"\n", ssid);
		if(ap->auth == IW_ENCRY_WPA2WPA_PSK)
		{
			fprintf(fp, "	key_mgmt=WPA-PSK WPA-EAP\n");
			fprintf(fp, "	psk=\"%s\"\n", password);
			fprintf(fp, "	proto=RSN WPA\n");
			fprintf(fp, "	pairwise=TKIP CCMP\n");
			fprintf(fp, "	group=CCMP TKIP\n");
		}
		else if(ap->auth == IW_ENCRY_WPA2_PSK)
		{
			fprintf(fp, "	key_mgmt=WPA-PSK WPA-EAP\n");
			fprintf(fp, "	psk=\"%s\"\n", password);
			fprintf(fp, "	proto=RSN\n");
			fprintf(fp, "	pairwise=TKIP CCMP\n");
			fprintf(fp, "	group=CCMP TKIP\n");
		}
		else if(ap->auth == IW_ENCRY_WPA_PSK)
		{
			fprintf(fp, "	key_mgmt=WPA-PSK WPA-EAP\n");
			fprintf(fp, "	psk=\"%s\"\n", password);
			fprintf(fp, "	proto=WPA\n");
			fprintf(fp, "	pairwise=TKIP CCMP\n");
			fprintf(fp, "	group=CCMP TKIP\n");
		}
		else if(ap->auth == IW_ENCRY_WEP_PRIVATE)
		{
			fprintf(fp, "	key_mgmt=NONE\n");
			fprintf(fp, "	wep_key0=\"%s\"\n", password);
			//fprintf(fp, "	proto=RSN\n");
			//fprintf(fp, "	pairwise=TKIP CCMP\n");
			//fprintf(fp, "	group=CCMP TKIP WEP104 WEP40\n");
		}
		else if(ap->auth == IW_ENCRY_WEP_OPEN)
		{
			fprintf(fp, "	key_mgmt=NONE\n");
			fprintf(fp, "	wep_key0=\"%s\"\n", password);
			//fprintf(fp, "	proto=RSN\n");
			//fprintf(fp, "	pairwise=TKIP CCMP\n");
			//fprintf(fp, "	group=CCMP TKIP WEP104 WEP40\n");
		}
		else if(ap->auth == IW_ENCRY_NONE)
		{
			fprintf(fp, "	key_mgmt=NONE\n");
			//fprintf(fp, "	psk=\"%s\"\n", password);
			//fprintf(fp, "	proto=RSN\n");
			//fprintf(fp, "	pairwise=TKIP CCMP\n");
			//fprintf(fp, "	group=CCMP TKIP WEP104 WEP40\n");
		}
		//fprintf(fp, "	beacon_int=100\n");
		fprintf(fp, "}\n");
		fflush(fp);
		fclose(fp);
		return 0;
	}
	return -1;
}

#if 1//def DOUBLE_PROCESS
int iw_client_dev_connect(struct interface *ifp, iw_client_ap_t *ap, char *ssid, char *password)
{
	int pid = 0;
	char path[128];
	char pidpath[128];
	char confpath[128];
	assert(ifp != NULL);
	assert(ssid != NULL);
	char *argv[] = {"-B", "-P", NULL, "-D", "nl80211", "-i", ifp->k_name, "-c", NULL, NULL};
	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path), "wpa_supp.%s",ifp->k_name);

	memset(pidpath, 0, sizeof(pidpath));
	snprintf(pidpath, sizeof(pidpath), "%s/wpa_supplicant-%s.pid",DAEMON_VTY_DIR, ifp->k_name);
	memset(confpath, 0, sizeof(confpath));
	snprintf(confpath, sizeof(confpath), "%s/wpa_supplicant.%s", IW_SYSCONFDIR,ifp->k_name);
	pid = os_pid_get(pidpath);
	if(pid > 0)
	{
		if(pid2name(pid))
		{
			return 0;
		}
		remove(pidpath);
	}
	argv[2] = pidpath;
	argv[8] = confpath;
	if(iw_client_dev_connect_script(ifp, ap, ssid, password) == 0)
	{
		if(IW_DEBUG(EVENT))
		{
			zlog_debug(ZLOG_WIFI, "running connect process on interface %s ",ifp->name);
		}
		os_process_register(PROCESS_DEAMON, path, "/usr/sbin/wpa_supplicant", FALSE, argv);
		//TODO check if is connect
		return 0;
	}
	return -1;
}

#ifndef PL_DHCPC_MODULE
int iw_client_dev_start_dhcpc(struct interface *ifp)
{
	int pid = 0;
	char path[128];
	char pidpath[128];
	assert(ifp != NULL);
	char *argv1[] = {"-p", NULL, "-i",ifp->k_name, "-s", "/lib/netifd/dhcp.script", NULL, NULL};
	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path), "udhcpc.%s",ifp->k_name);

	memset(pidpath, 0, sizeof(pidpath));
	snprintf(pidpath, sizeof(pidpath), "%s/udhcpc-%s.pid",DAEMON_VTY_DIR, ifp->k_name);

	pid = os_pid_get(pidpath);
	if(pid > 0)
	{
		if(pid2name(pid))
		{
			return 0;
		}
		remove(pidpath);
	}

	argv1[1] = pidpath;
	if(IW_DEBUG(EVENT))
	{
		zlog_debug(ZLOG_WIFI, "running dhcpc process on interface %s ",ifp->name);
	}
	os_process_register(PROCESS_DEAMON, path, "udhcpc", FALSE, argv1);
	//udhcpc -p /var/run/udhcpc-wlan0.pid -s /lib/netifd/dhcp.script -f -t 0
	//  		-i wlan0 -x hostname:OpenWrt -C -O
	return 0;
}
#endif
#else

int iw_client_dev_connect(struct interface *ifp, iw_client_ap_t *ap, char *ssid, char *password)
{
	int pid = 0;
	char pidpath[128];
	memset(pidpath, 0, sizeof(pidpath));
	snprintf(pidpath, sizeof(pidpath), "%s/wpa_supplicant-%s.pid",DAEMON_VTY_DIR,ifp->k_name);
	pid = os_pid_get(pidpath);
	if(pid > 0)
	{
		if(pid2name(pid))
		{
			return 0;
		}
		remove(pidpath);
	}
	pid = child_process_create();
	if(pid == 0)
	{
		char path[128];
		char confpath[128];
		char *argv[] = {"-B", "-P", NULL, "-D", "nl80211", "-i", ifp->k_name, "-c", NULL, NULL};
		memset(path, 0, sizeof(path));
		snprintf(path, sizeof(path), "wpa_supp.%s",ifp->k_name);
		memset(confpath, 0, sizeof(confpath));
		snprintf(confpath, sizeof(confpath), "%s/wpa_supplicant.%s", SYSCONFDIR,ifp->k_name);
		argv[2] = pidpath;
		argv[8] = confpath;
		if(iw_client_dev_connect_script(ifp, ap, ssid, password) == 0)
		{
			super_system_execvp("/usr/sbin/wpa_supplicant", argv);
		}
		exit(0);
	}
	else
	{
		//waitpid(pid, NULL, 0);
		return 0;
	}
	return -1;
}

int iw_client_dev_start_dhcpc(struct interface *ifp)
{
	int pid = 0;
	char pidpath[128];
	memset(pidpath, 0, sizeof(pidpath));
	snprintf(pidpath, sizeof(pidpath), "%s/udhcpc-%s.pid", DAEMON_VTY_DIR,ifp->k_name);
	pid = os_pid_get(pidpath);
	if(pid > 0)
	{
		if(pid2name(pid))
		{
			return 0;
		}
		remove(pidpath);
	}
	pid = child_process_create();
	if (pid == 0)
	{
		char *argv1[] = { "-p", NULL, "-i", ifp->k_name, "-s", "/lib/netifd/dhcp.script", NULL, NULL };
		argv1[1] = pidpath;
		super_system_execvp("udhcpc", argv1);
	}
	else if (pid)
	{
		//waitpid(pid, NULL, 0);
		return 0;
	}
	//udhcpc -p /var/run/udhcpc-wlan0.pid -s /lib/netifd/dhcp.script -f -t 0
	//  		-i wlan0 -x hostname:OpenWrt -C -O
	return -1;
}
#endif

int iw_client_dev_disconnect(struct interface *ifp)
{
	assert(ifp != NULL);
	if(ifp->k_ifindex && if_is_wireless(ifp))
	{
		pid_t	pid = 0;
		char pidpath[128];
#ifndef PL_DHCPC_MODULE
		memset(pidpath, 0, sizeof(pidpath));
		snprintf(pidpath, sizeof(pidpath), "%s/udhcpc-%s.pid", DAEMON_VTY_DIR, ifp->k_name);
		pid = os_pid_get(pidpath);
		if(pid > 0)
		{
			if(pid2name(pid))
			{
				child_process_destroy(pid);
			}
			remove(pidpath);
		}
		os_msleep(100);
		remove(pidpath);
#endif
		memset(pidpath, 0, sizeof(pidpath));
		snprintf(pidpath, sizeof(pidpath), "%s/wpa_supplicant-%s.pid",DAEMON_VTY_DIR,ifp->k_name);
		pid = os_pid_get(pidpath);
		if(pid > 0)
		{
			if(pid2name(pid))
			{
				child_process_destroy(pid);
			}
			remove(pidpath);
		}
		os_msleep(100);
		remove(pidpath);
/*		char tmpcmd[128];
		remove(wifi_file_path());
		os_memset(tmpcmd, 0, sizeof(tmpcmd));
		os_sprintf(tmpcmd, "iw dev %s disconnect >> %s 2>&1", ifp->k_name, wifi_file_path());
		super_system(tmpcmd);*/
		//wifi_show_from_file(vty);
	}
	return OK;
}


int iw_client_dev_is_connect(char *ifname, u_int8 *bssid)
{
	assert(ifname != NULL);
	return iw_is_connect(ifname, bssid);
}

static int iw_client_scan_swap(struct wireless_scan * wscan, iw_client_ap_t *ap)
{
	assert(wscan != NULL);
	assert(ap != NULL);
	if(wscan->has_ap_addr)
	{
		memcpy(ap->BSSID, wscan->ap_addr.sa_data, NSM_MAC_MAX);
/*
		wscan->b.has_nwid = 1;
		memcpy(&(wscan->b.nwid), &(event->u.nwid), sizeof(iwparam));
*/
		if(wscan->b.has_freq)
		{
			ap->freq = wscan->b.freq;
		}
/*
		wscan->b.mode = event->u.mode;
		if ((wscan->b.mode < IW_NUM_OPER_MODE) && (wscan->b.mode >= 0))
			wscan->b.has_mode = 1;
*/
		if(wscan->b.has_essid)
		{
			strcpy(ap->SSID, wscan->b.essid);
		}
		if(wscan->b.has_key)
		{
			if (wscan->b.key_flags & IW_ENCODE_DISABLED)
				ap->auth = 0;
			else if (wscan->b.key_flags & IW_ENCODE_NOKEY)
				ap->auth = IW_ENCRY_WEP_OPEN;
			else
			{
				if (wscan->b.key_flags & IW_ENCODE_RESTRICTED)
					ap->auth = IW_ENCRY_WEP_PRIVATE;
				else if (wscan->b.key_flags & IW_ENCODE_OPEN)
					ap->auth = IW_ENCRY_WEP_OPEN;
			}
		}
		if(wscan->has_ie)
		{
			if( (wscan->auth & 0xff00) && (wscan->auth & 0xff) )
				ap->auth = IW_ENCRY_WPA2WPA_PSK;
			else if( (wscan->auth & 0xff00) && !(wscan->auth & 0xff) )
				ap->auth = IW_ENCRY_WPA2_PSK;
			else if( !(wscan->auth & 0xff00) && (wscan->auth & 0xff) )
				ap->auth = IW_ENCRY_WPA_PSK;
		}
		if(wscan->has_stats)
		{
			ap->qaul = wscan->stats.qual.qual;

			if (wscan->stats.qual.updated & IW_QUAL_RCPI)
			{
				/* Deal with signal level in RCPI */
				/* RCPI = int{(Power in dBm +110)*2} for 0dbm > Power > -110dBm */
				if (!(wscan->stats.qual.updated & IW_QUAL_LEVEL_INVALID))
				{
					ap->signal = (int)(wscan->stats.qual.level / 2.0) - 110.0;
				}
			}
			else
			{
				if ((wscan->stats.qual.updated & IW_QUAL_DBM) &&
						!(wscan->stats.qual.updated & IW_QUAL_LEVEL_INVALID))
				{
						int dblevel = wscan->stats.qual.level;
						/* Implement a range for dBm [-192; 63] */
						if (wscan->stats.qual.level >= 64)
							dblevel -= 0x100;
						ap->signal = dblevel;
				}
				else
				{
					if(!(wscan->stats.qual.updated & IW_QUAL_LEVEL_INVALID))
						ap->signal = wscan->stats.qual.level;
				}
			}
			//ap->signal = wscan->stats.qual.level;
			ap->nosie = wscan->stats.qual.noise;
		}

		if(wscan->has_maxbitrate)
		{
			ap->bitrate = wscan->maxbitrate.value;
		}
		return 0;
	}
	return -1;
}


int iw_client_scan_process(iw_client_t *iw_client)
{
	int ret = 0;
	if(!iw_client)
		return ERROR;
	struct interface *ifp = if_lookup_by_index(iw_client->ifindex);
	wireless_scan_head context;
	struct wireless_scan * wscan = NULL;
	iw_client_ap_t ap;
	//zlog_debug(ZLOG_PAL, "%s into(%s)", __func__, ifp->name);
	if(!(ifp && ifp->k_ifindex && if_is_wireless(ifp)))
		return OK;

	if(IW_DEBUG(EVENT))
	{
		zlog_debug(ZLOG_WIFI, "scanning network on interface %s ",ifp->name);
	}
	//zlog_debug(ZLOG_PAL, "%s start(%s)", __func__, ifp->k_name);
	ret = iw_start_scan(ifp->k_name, &context);
	if(ret == 0 && context.result)
	{
		for(wscan = context.result; wscan; wscan = wscan->next)
		{
			memset(&ap, 0, sizeof(ap));
			if(iw_client_scan_swap(wscan, &ap) == 0)
			{
				//zlog_debug(ZLOG_PAL, "%s iw_client_ap_set_api", __func__);
				ap.ifindex = iw_client->ifindex;
				//if(testvty)
				//	iw_client_ap_show_one(&ap, testvty);
				iw_client_ap_set_api(iw_client, ap.BSSID, &ap);
				iw_client->scan_result = context.result;
				if(IW_DEBUG(SCAN))
				{
					char buf[128];
					struct prefix prefix_eth;
					union prefix46constptr pu;
					prefix_eth.family = AF_ETHERNET;
					memcpy(prefix_eth.u.prefix_eth.octet, ap.BSSID, NSM_MAC_MAX);
					pu.p = &prefix_eth;
					memset(buf, 0, sizeof(buf));
					zlog_debug(ZLOG_WIFI, " scanning network: %s", ifindex2ifname(ap.ifindex));
					zlog_debug(ZLOG_WIFI, "  SSID        %s", ap.SSID);
					zlog_debug(ZLOG_WIFI, "   BSSID      %s ",prefix_2_address_str (pu, buf, sizeof(buf)));
					zlog_debug(ZLOG_WIFI, "   signal     %d/%d dBm", ap.signal, ap.nosie);
					memset(buf, 0, sizeof(buf));
					iw_print_freq_value(buf, sizeof(buf), ap.freq);
					zlog_debug(ZLOG_WIFI, "   FREQ       %s", buf);
				}
			}
			//else
			//	zlog_debug(ZLOG_PAL, "%s iw_client_scan_swap", __func__);
		}
		iw_free_scan(&context);
	}
	//zlog_debug(ZLOG_PAL, "%s stop", __func__);
	//iw_client_ap_update(iw_client);
	return OK;
}

int iw_client_scan_process_exit(iw_client_t *iw_client)
{
	if(iw_client && iw_client->scan_result)
	{
		struct wireless_scan * wscan = NULL;
		struct wireless_scan * next = NULL;
		for(wscan = iw_client->scan_result; wscan; wscan = wscan->next)
		{
			next = wscan->next;
			free(wscan);
			wscan = next;
			if(!wscan)
				break;
		}
		return 0;
	}
	return 0;
}



/*******************************************************************************/
/*******************************************************************************/
/*
root@OpenWrt:/# iw dev wlan0 station dump
Station 2c:57:31:7b:e3:88 (on wlan0)
        inactive time:  320 ms
        rx bytes:       32645
        rx packets:     434
        tx bytes:       16062
        tx packets:     130
        tx retries:     238
        tx failed:      3
        rx drop misc:   1
        signal:         -41 [-41, -108] dBm
        signal avg:     -41 [-41, -107] dBm
        tx bitrate:     58.5 MBit/s MCS 6
        rx bitrate:     6.0 MBit/s
        expected throughput:    15.563Mbps
        authorized:     yes
        authenticated:  yes
        associated:     yes
        preamble:       short
        WMM/WME:        yes
        MFP:            no
        TDLS peer:      no
        DTIM period:    2
        beacon interval:100
        short preamble: yes
        short slot time:yes
        connected time: 17 seconds
root@OpenWrt:/#
 */
static char * digit_offset(char *src)
{
/*	char *p = src;
	assert(src);
	int i = 0, j = 0, count = os_strlen(src);
	for(i = 0; i < count; i++)
	{
		if(isdigit((int)p[i]))
		{
			return (src + i);
		}
	}*/
	int n = strcspn(src, "0123456789");
	if(n == 0)
		return NULL;
	else
		return src+n;
}

static int iw_dev_station_dump_add(iw_ap_t *iw_ap, struct interface *ifp)
{
	FILE *fp = NULL;
	int flag = 0;
	char buf[512];
	char *s = NULL;
	iw_ap_connect_t connect;
	char *filepath = wifi_file_path();
	fp = fopen(filepath, "r");
	if(fp)
	{
		memset(buf, 0, sizeof(buf));
		while (fgets(buf, sizeof(buf), fp))
		{
			for (s = buf + strlen(buf); (s > buf) && isspace((int) *(s - 1)); s--)
				;
			*s = '\0';
			if(strstr(buf, "Station"))
			{
				//Station 2c:57:31:7b:e3:88 (on wlan0)
				//	/
				s = strstr(buf, ifp->k_name);
				if(s)
				{
					if(flag)
					{
						flag = 0;
						connect.ifindex = ifp->ifindex;
						iw_ap_connect_add_api(iw_ap, &connect);
						if(IW_DEBUG(AP_ACCEPT))
						{
							char sbuf[128];
							struct prefix prefix_eth;
							union prefix46constptr pu;
							prefix_eth.family = AF_ETHERNET;
							memcpy(prefix_eth.u.prefix_eth.octet, connect.BSSID, NSM_MAC_MAX);
							pu.p = &prefix_eth;
							memset(sbuf, 0, sizeof(sbuf));
							zlog_debug(ZLOG_WIFI, " ACCEPT network: %s", ifindex2ifname(iw_ap->ifindex));
							zlog_debug(ZLOG_WIFI, "   BSSID      %s ",prefix_2_address_str (pu, sbuf, sizeof(sbuf)));
							zlog_debug(ZLOG_WIFI, "   signal     %d dBm", connect.signal);
							zlog_debug(ZLOG_WIFI, "   authorized     %s", connect.authorized ? "YES":"NO");
							zlog_debug(ZLOG_WIFI, "   authenticated  %s", connect.authenticated ? "YES":"NO");
							zlog_debug(ZLOG_WIFI, "   associated     %s", connect.associated ? "YES":"NO");
						}
					}
					memset(&connect, 0, sizeof(connect));
					flag = 1;
				}
				if(flag)
				{
					sscanf(buf+strlen("Station "), "%02x:%02x:%02x:%02x:%02x:%02x",
							&connect.BSSID[0], &connect.BSSID[1],
							&connect.BSSID[2], &connect.BSSID[3],
							&connect.BSSID[4], &connect.BSSID[5]);
				}
			}
			else
			{
				if(flag)
				{
					if(strstr(buf, "inactive"))
					{
						s = digit_offset(buf);
						if(s)
							sscanf(s, "%d",&connect.inactive_time);
					}
					else if(strstr(buf, "rx") && strstr(buf, "bytes"))
					{
						s = digit_offset(buf);
						if(s)
							sscanf(s, "%d",&connect.rx_bytes);
					}
					else if(strstr(buf, "rx") && strstr(buf, "packets"))
					{
						s = digit_offset(buf);
						if(s)
							sscanf(s, "%d",&connect.rx_packets);
					}
					else if(strstr(buf, "tx") && strstr(buf, "bytes"))
					{
						s = digit_offset(buf);
						if(s)
							sscanf(s, "%d",&connect.tx_bytes);
					}
					else if(strstr(buf, "tx") && strstr(buf, "packets"))
					{
						s = digit_offset(buf);
						if(s)
							sscanf(s, "%d",&connect.tx_packets);
					}
					else if(strstr(buf, "retries"))
					{
						s = digit_offset(buf);
						if(s)
							sscanf(s, "%d",&connect.tx_retries);
					}
					else if(strstr(buf, "failed"))
					{
						s = digit_offset(buf);
						if(s)
							sscanf(s, "%d",&connect.tx_failed);
					}
					else if(strstr(buf, "drop"))
					{
						s = digit_offset(buf);
						if(s)
							sscanf(s, "%d",&connect.rx_drop);
					}
					else if(strstr(buf, "signal"))
					{
						if(strstr(buf, "avg"))
						{
							s = digit_offset(buf);
							if(s)
								sscanf(s, "%d",&connect.signal);
							if(strstr(s-1, "-"))
								connect.signal = 0 - connect.signal;
						}
					}
					else if(strstr(buf, "tx") && strstr(buf, "bitrate"))
					{
						s = digit_offset(buf);
						if(s)
							sscanf(s, "%f",&connect.tx_bitrate);
					}
					else if(strstr(buf, "rx") && strstr(buf, "bitrate"))
					{
						s = digit_offset(buf);
						if(s)
							sscanf(s, "%f",&connect.rx_bitrate);
					}
					else if(strstr(buf, "throughput"))
					{
						s = digit_offset(buf);
						if(s)
							sscanf(s, "%f",&connect.throughput);
					}
					else if(strstr(buf, "authorized"))
					{
						if(strstr(buf, "yes"))
							connect.authorized = TRUE;
					}
					else if(strstr(buf, "authenticated"))
					{
						if(strstr(buf, "yes"))
							connect.authenticated = TRUE;
					}
					else if(strstr(buf, "associated"))
					{
						if(strstr(buf, "yes"))
							connect.associated = TRUE;
					}
					else if(strstr(buf, "WMM/WME"))
					{
						if(strstr(buf, "yes"))
							connect.WME_WMM = TRUE;
					}
					else if(strstr(buf, "MFP"))
					{
						if(strstr(buf, "yes"))
							connect.MFP = TRUE;
					}
					else if(strstr(buf, "TDLS"))
					{
						if(strstr(buf, "yes"))
							connect.TDLS = TRUE;
					}
					else if(strstr(buf, "period"))
					{
						s = digit_offset(buf);
						if(s)
							sscanf(s, "%d",&connect.period);
					}
					else if(strstr(buf, "beacon"))
					{
						s = digit_offset(buf);
						if(s)
							sscanf(s, "%d",&connect.beacon);
					}
					else if(strstr(buf, "connected"))
					{
						s = digit_offset(buf);
						if(s)
							sscanf(s, "%d",&connect.connected_time);
					}
				}
			}
			memset(buf, 0, sizeof(buf));
		}
		if(flag)
		{
			flag = 0;
			iw_ap_connect_add_api(iw_ap, &connect);
			if(IW_DEBUG(AP_ACCEPT))
			{
				char sbuf[128];
				struct prefix prefix_eth;
				union prefix46constptr pu;
				prefix_eth.family = AF_ETHERNET;
				memcpy(prefix_eth.u.prefix_eth.octet, connect.BSSID, NSM_MAC_MAX);
				pu.p = &prefix_eth;
				memset(sbuf, 0, sizeof(sbuf));
				zlog_debug(ZLOG_WIFI, " ACCEPT network: %s", ifindex2ifname(iw_ap->ifindex));
				zlog_debug(ZLOG_WIFI, "   BSSID      %s ",prefix_2_address_str (pu, sbuf, sizeof(sbuf)));
				zlog_debug(ZLOG_WIFI, "   signal     %d dBm", connect.signal);
				zlog_debug(ZLOG_WIFI, "   authorized     %s", connect.authorized ? "YES":"NO");
				zlog_debug(ZLOG_WIFI, "   authenticated  %s", connect.authenticated ? "YES":"NO");
				zlog_debug(ZLOG_WIFI, "   associated     %s", connect.associated ? "YES":"NO");
			}
		}
		fclose(fp);
	}
	return OK;
}

static int iw_ap_connect_dump(iw_ap_t *iw_ap, struct interface *ifp)
{
	assert(ifp != NULL);
	assert(iw_ap != NULL);
	if(ifp->k_ifindex && if_is_wireless(ifp))
	{
		char tmpcmd[128];
		remove(wifi_file_path());
		os_memset(tmpcmd, 0, sizeof(tmpcmd));
		os_sprintf(tmpcmd, "iw dev %s station dump >> %s 2>&1", ifp->k_name, wifi_file_path());
		super_system(tmpcmd);
		iw_dev_station_dump_add(iw_ap, ifp);
	}
	return OK;
}

int iw_ap_connect_scanning(iw_ap_t *iw_ap)
{
	//iw_ap_connect_t connect;
	assert(iw_ap != NULL);
	struct interface *ifp = if_lookup_by_index(iw_ap->ifindex);
	if(ifp)
	{
		iw_ap_connect_dump(iw_ap, ifp);
		//iw_ap_connect_add_api(iw_ap, &connect);
	}
	return OK;
}

int iw_dev_mode(struct interface *ifp)
{
	assert(ifp != NULL);
	if(ifp->k_ifindex && if_is_wireless(ifp))
	{
		int mode = -1;
		if(iw_kernel_mode(ifp->k_name, &mode) == 0)
			return mode;
	}
	return ERROR;
}

/*
2.使用iwconfig将它设置成Ad-hoc模式：
ifconfig wlan0 down
iwconfig wlan0 mode ad-hoc
iwconfig wlan0 key off
#iwconfig wlan0 channel 1
#iwconfig wlan0 key 123456789 open
iwconfig wlan0 essid "SmartHome-S3C6410"
ifconfig wlan0 up
ifconfig wlan0 192.168.0.1
*
root@OpenWrt:/home# iw dev wlan0 info
Interface wlan0
        ifindex 67
        wdev 0x34
        addr 66:e8:c1:e2:4f:47
        ssid OpenWrt
        type AP
        wiphy 0
        channel 5 (2432 MHz), width: 20 MHz, center1: 2432 MHz
root@OpenWrt:/home#

root@OpenWrt:/home# iwconfig wlan0
wlan0     IEEE 802.11  ESSID:"xiaozhu"
          Mode:Managed  Frequency:2.417 GHz  Access Point: B0:DF:C1:D7:A8:50
          Bit Rate=1 Mb/s
          RTS thr:off   Fragment thr:off
          Encryption key:off
          Power Management:off
          Link Quality=67/70  Signal level=-43 dBm
          Rx invalid nwid:0  Rx invalid crypt:0  Rx invalid frag:0
          Tx excessive retries:0  Invalid misc:1   Missed beacon:0
 *
 *
root@OpenWrt:/home# cat wpa.conf
ctrl_interface=/var/run/wpa_supplicant
network={
        ssid="xiaozhu"
        psk="VxWorks616479633"
        key_mgmt=WPA-EAP WPA-PSK IEEE8021X NONE
        pairwise=TKIP CCMP
        group=CCMP TKIP WEP104 WEP40
}
country=00
network={
        scan_ssid=1
        ssid="xiaozhu"
        key_mgmt=WPA-PSK
        psk="VxWorks616479633"
        proto=RSN
        bssid=B0:DF:C1:D7:A8:50
        beacon_int=100
}
 *
 * wpa_supplicant Dnl80211 -iwlan0 -c /home/wpa.conf -B
 * udhcpc -i wlan0
 *
 * /usr/sbin/wpa_supplicant -B -P /var/run/wpa_supplicant-wlan0.pid -D nl80211
 * 		-i wlan0 -c /var/run/wpa_supp
 *
 * udhcpc -p /var/run/udhcpc-br-wan.pid -s /lib/netifd/dhcp.script -f -t 0 -i
 * 		br-wan -x hostname:openwrt -C -O 121
 *
 */

//export LD_LIBRARY_PATH="/usr/lib/x86_64-linux-gnu/:$LD_LIBRARY_PATH"
