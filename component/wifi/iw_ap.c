/*
 * iw_ap.c
 *
 *  Created on: Oct 11, 2018
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
#include "nsm_vrf.h"
#include "nsm_interface.h"

#include "iw_config.h"
#include "iw_ap.h"
#include "iw_interface.h"
#include "iwlib.h"


static int iw_ap_start(iw_ap_t *iw_ap);
static int iw_ap_stop(iw_ap_t *iw_ap);

static int iw_ap_connect_cleanup(iw_ap_t *iw_ap, BOOL use)
{
	NODE index;
	LIST *list = NULL;
	iw_ap_connect_t *pstNode = NULL;
	if(!iw_ap && !iw_ap->ap_list)
		return ERROR;
	if(use)
		list = iw_ap->ap_list;
	if(iw_ap->ap_mutex)
		os_mutex_lock(iw_ap->ap_mutex, OS_WAIT_FOREVER);
	for(pstNode = (iw_ap_connect_t *)lstFirst(list);
			pstNode != NULL;  pstNode = (iw_ap_connect_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode)
		{
			lstDelete(list, (NODE*)pstNode);
			XFREE(MTYPE_WIFI_CLIENT, pstNode);
		}
	}
	if(iw_ap->ap_mutex)
		os_mutex_unlock(iw_ap->ap_mutex);
	return OK;
}



static int iw_ap_connect_add_node(iw_ap_t *iw_ap, iw_ap_connect_t *value)
{
	iw_ap_connect_t *node = NULL;
	if(!iw_ap || !value || !iw_ap->ap_list)
		return ERROR;
	node = XMALLOC(MTYPE_WIFI_CLIENT, sizeof(iw_ap_connect_t));
	if(node)
	{
		os_memset(node, 0, sizeof(iw_ap_connect_t));
		os_memcpy(node, value, sizeof(iw_ap_connect_t));
		node->ifindex = iw_ap->ifindex;
		node->TTL = IW_AP_CONNECT_TTL_DEFAULT;
		lstAdd(iw_ap->ap_list, (NODE *)node);
		//lstAdd(iw_ap->ap_list, (NODE *)node);
		return OK;
	}
	return ERROR;
}

static int iw_ap_connect_del_node(iw_ap_t *iw_ap, iw_ap_connect_t *node)
{
	if(iw_ap && node && iw_ap->ap_list)
	{
		lstDelete(iw_ap->ap_list, (NODE *)node);
		XFREE(MTYPE_WIFI_CLIENT, node);
		return OK;
	}
	return ERROR;
}

static iw_ap_connect_t * iw_ap_connect_lookup_node(iw_ap_t *iw_ap, u_int8 *bssid)
{
	iw_ap_connect_t *pstNode = NULL;
	NODE index;
	u_int8 BSSID[IW_SSID_NAME_MAX];
	memset(BSSID, 0, IW_SSID_NAME_MAX);
	if(!iw_ap || !bssid || !iw_ap->ap_list)
		return NULL;
	if(bssid)
		memcpy(BSSID, bssid, NSM_MAC_MAX);

	for(pstNode = (iw_ap_connect_t *)lstFirst(iw_ap->ap_list);
			pstNode != NULL;  pstNode = (iw_ap_connect_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(bssid)
		{
			if(memcmp(pstNode->BSSID, BSSID, NSM_MAC_MAX) == 0)
			{
				return pstNode;
			}
		}
	}
	return NULL;
}

int iw_ap_connect_add_api(iw_ap_t *iw_ap, iw_ap_connect_t *value)
{
	iw_ap_connect_t *client = NULL;
	if(!iw_ap || !value || !iw_ap->ap_list)
		return ERROR;
	if(iw_ap->ap_mutex)
		os_mutex_lock(iw_ap->ap_mutex, OS_WAIT_FOREVER);
	client = iw_ap_connect_lookup_node(iw_ap, value->BSSID);
	if(client)
	{
		memcpy(&client->ifindex, &value->ifindex, sizeof(iw_ap_connect_t) - sizeof(NODE));
		client->ifindex = iw_ap->ifindex;
		client->TTL = IW_AP_CONNECT_TTL_DEFAULT;

		//zlog_debug(ZLOG_WIFI, "update TTL ");
		if(iw_ap->ap_mutex)
			os_mutex_unlock(iw_ap->ap_mutex);
		return OK;
	}
	else
	{
		//zlog_debug(ZLOG_WIFI, "add connect node ");
		iw_ap_connect_add_node(iw_ap, value);
		if(iw_ap->ap_mutex)
			os_mutex_unlock(iw_ap->ap_mutex);
		return OK;
	}
	if(iw_ap->ap_mutex)
		os_mutex_unlock(iw_ap->ap_mutex);
	return ERROR;
}

int iw_ap_connect_del_api(iw_ap_t *iw_ap, u_int8 *bssid)
{
	iw_ap_connect_t *client = NULL;
	if(!iw_ap || !bssid || !iw_ap->ap_list)
		return ERROR;

	if(iw_ap->ap_mutex)
		os_mutex_lock(iw_ap->ap_mutex, OS_WAIT_FOREVER);
	client = iw_ap_connect_lookup_node(iw_ap, bssid);
	if(client)
	{
		iw_ap_connect_del_node(iw_ap, client);
		if(iw_ap->ap_mutex)
			os_mutex_unlock(iw_ap->ap_mutex);
		return OK;
	}
	if(iw_ap->ap_mutex)
		os_mutex_unlock(iw_ap->ap_mutex);
	return ERROR;
}


static int iw_ap_connect_update(iw_ap_t *iw_ap, u_int8 TTL)
{
	NODE index;
	iw_ap_connect_t *client = NULL;
	if(!iw_ap || !iw_ap->ap_list)
		return ERROR;

	if(iw_ap->ap_mutex)
		os_mutex_lock(iw_ap->ap_mutex, OS_WAIT_FOREVER);
	for(client = (iw_ap_connect_t *)lstFirst(iw_ap->ap_list);
			client != NULL;  client = (iw_ap_connect_t *)lstNext((NODE*)&index))
	{
		index = client->node;
		if(client)
		{
			//client->TTL = IW_AP_CONNECT_TTL_DEFAULT;
			client->TTL--;
		}
	}
	for(client = (iw_ap_connect_t *)lstFirst(iw_ap->ap_list);
			client != NULL;  client = (iw_ap_connect_t *)lstNext((NODE*)&index))
	{
		index = client->node;
		if(client && client->TTL == 0)
		{
			lstDelete(iw_ap->ap_list, (NODE *)client);
			XFREE(MTYPE_WIFI_CLIENT, client);
		}
	}
	if(iw_ap->ap_mutex)
		os_mutex_unlock(iw_ap->ap_mutex);
	return ERROR;
}

iw_ap_connect_t * iw_ap_connect_lookup_api(iw_ap_t *iw_ap, u_int8 *bssid)
{
	iw_ap_connect_t *client = NULL;
	if(!iw_ap || !bssid || !iw_ap->ap_list)
		return NULL;
	if(iw_ap->ap_mutex)
		os_mutex_lock(iw_ap->ap_mutex, OS_WAIT_FOREVER);
	client = iw_ap_connect_lookup_node(iw_ap, bssid);
	if(iw_ap->ap_mutex)
		os_mutex_unlock(iw_ap->ap_mutex);
	return client;
}

static int iw_ap_connect_show_one(iw_ap_connect_t *ap, struct vty *vty)
{
	if(ap && vty)
	{
		char buf[128];
		struct prefix prefix_eth;
		union prefix46constptr pu;
		prefix_eth.family = AF_ETHERNET;
		memcpy(prefix_eth.u.prefix_eth.octet, ap->BSSID, NSM_MAC_MAX);
		pu.p = &prefix_eth;

		//SSID BSSID
		memset(buf, 0, sizeof(buf));
		if(vty->detail)
		{
			//Station 2c:57:31:7b:e3:88 (on wlan0)
			vty_out(vty, " Station         : %s (%s)%s", 	prefix_2_address_str (pu, buf, sizeof(buf)),
					ifindex2ifname(ap->ifindex), VTY_NEWLINE);
		}
		else
			vty_out(vty, "%-18s ", 	prefix_2_address_str (pu, buf, sizeof(buf)));

		//SIGNAL
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%d dBm", ap->signal);
		if(vty->detail)
			vty_out(vty, "  Signal         : %s %s", buf, VTY_NEWLINE);
		else
			vty_out(vty, "%-10s ", buf);
		//THREOUGHPUT
		if(ap->throughput > 0.0)
		{
			memset(buf, 0, sizeof(buf));
			sprintf(buf, "%.3f Mbps", ap->throughput);
			if(vty->detail)
			{
				vty_out(vty, "  throughput     : %s %s", buf, VTY_NEWLINE);
			}
			else
				vty_out(vty, "%-12s ", buf);
		}
		else
		{
			sprintf(buf, "%.3f Mbps", ap->tx_bitrate + ap->tx_bitrate);
			if(vty->detail)
			{
				vty_out(vty, "  throughput     : %s %s", buf, VTY_NEWLINE);
			}
			else
				vty_out(vty, "%-12s ", buf);
		}
		//auth
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%s", ap->authenticated ? "YES":"NO");
		if(vty->detail)
		{
			vty_out(vty, "  authenticated  : %s %s", buf, VTY_NEWLINE);
		}
		else
			vty_out(vty, "%-8s ", buf);

		//connect time
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%d seconds", ap->connected_time);
		if(vty->detail)
			vty_out(vty, "  connected time : %s %s", buf, VTY_NEWLINE);
		else
			vty_out(vty, "%-12s ", buf);

		if(!vty->detail)
			vty_out(vty, "%-20s%s", ifindex2ifname(ap->ifindex), VTY_NEWLINE);

		if(vty->detail)
		{
			vty_out(vty, "  inactive time  : %d %s", ap->inactive_time, VTY_NEWLINE);
			vty_out(vty, "  rx bytes       : %d %s", ap->rx_bytes, VTY_NEWLINE);
			vty_out(vty, "  rx packets     : %d %s", ap->rx_packets, VTY_NEWLINE);
			vty_out(vty, "  tx bytes       : %d %s", ap->tx_bytes, VTY_NEWLINE);
			vty_out(vty, "  tx packets     : %d %s", ap->tx_packets, VTY_NEWLINE);

			vty_out(vty, "  tx retries     : %d %s", ap->tx_retries, VTY_NEWLINE);
			vty_out(vty, "  tx failed      : %d %s", ap->tx_failed, VTY_NEWLINE);
			vty_out(vty, "  rx drop        : %s %s", ap->rx_drop, VTY_NEWLINE);

			vty_out(vty, "  inactive time  : %.2f MBit/s%s", ap->tx_bitrate, VTY_NEWLINE);
			vty_out(vty, "  inactive time  : %.2f MBit/s %s", ap->tx_bitrate, VTY_NEWLINE);
			vty_out(vty, "  authorized     : %s %s", ap->authorized ? "YES":"NO", VTY_NEWLINE);
			vty_out(vty, "  associated     : %s %s", ap->associated ? "YES":"NO", VTY_NEWLINE);
			vty_out(vty, "  WME/WMM        : %s %s", ap->WME_WMM ? "YES":"NO", VTY_NEWLINE);
			vty_out(vty, "  MFP            : %s %s", ap->MFP ? "YES":"NO", VTY_NEWLINE);
			vty_out(vty, "  TDLS peer      : %s %s", ap->TDLS ? "YES":"NO", VTY_NEWLINE);
			vty_out(vty, "  DTIM period    : %d %s", ap->period, VTY_NEWLINE);
			vty_out(vty, "  beacon interval: %s %s", ap->beacon, VTY_NEWLINE);
		}
	}
	return OK;
}

int iw_ap_connect_show(iw_ap_t *iw_ap, struct vty *vty, BOOL detail)
{
	if(iw_ap && vty && iw_ap->ap_list && lstCount(iw_ap->ap_list))
	{
		if(!detail)
		{
			vty_out(vty,"%-18s %-10s %-12s %-8s %-12s %-20s%s",
					"------------------", "----------", "------------",
						"--------",  "------------", "--------------------", VTY_NEWLINE);
			vty_out(vty,"%-18s %-10s %-8s %-8s %-10s %-20s%s",
						"      BSSID      ", "  SIGNAL  ",
						"THREOUGHPUT", "  AUTH  ", "CONNECT TIME", "     INTERFACE",VTY_NEWLINE);
			vty_out(vty,"%-18s %-10s %-12s %-8s %-12s %-20s%s",
					"------------------", "----------", "------------",
						"--------",  "------------", "--------------------", VTY_NEWLINE);
		}
		vty->detail = detail;
		iw_ap_connect_callback_api(iw_ap, iw_ap_connect_show_one, vty);
		vty->detail = FALSE;
		return OK;//iw_ap_connect_callback_api(iw_ap, iw_ap_connect_show_one, vty);
	}
	return OK;
}

int iw_ap_connect_callback_api(iw_ap_t *iw_ap, int (*cb)(iw_ap_connect_t *, void *), void *pVoid)
{
	int ret = OK;
	iw_ap_connect_t *pstNode = NULL;
	NODE index;
	if(!iw_ap || !iw_ap->ap_list)
		return ERROR;
	if(iw_ap->ap_mutex)
		os_mutex_lock(iw_ap->ap_mutex, OS_WAIT_FOREVER);
	for(pstNode = (iw_ap_connect_t *)lstFirst(iw_ap->ap_list);
			pstNode != NULL;  pstNode = (iw_ap_connect_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode && cb)
		{
			ret = (cb)(pstNode, pVoid);
			if(ret != OK)
				break;
		}
	}
	if(iw_ap->ap_mutex)
		os_mutex_unlock(iw_ap->ap_mutex);
	return ret;
}

/*
 * global
 */

int iw_ap_ssid_set_api(iw_ap_t *iw_ap, char *ssid)
{
	if(!iw_ap || !ssid)
		return ERROR;
	if(iw_ap->mutex)
		os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);
	if(ssid)
	{
		memset(iw_ap->SSID, 0, sizeof(iw_ap->SSID));
		strcpy(iw_ap->SSID, ssid);
		iw_ap_start(iw_ap);
	}
	if(iw_ap->mutex)
		os_mutex_unlock(iw_ap->mutex);
	return OK;
}

int iw_ap_ssid_del_api(iw_ap_t *iw_ap)
{
	if(!iw_ap)
		return ERROR;
	if(iw_ap->mutex)
		os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);
	memset(iw_ap->SSID, 0, sizeof(iw_ap->SSID));
	iw_ap_stop(iw_ap);
	if(iw_ap->mutex)
		os_mutex_unlock(iw_ap->mutex);
	return OK;
}

int iw_ap_password_set_api(iw_ap_t *iw_ap, char *pass)
{
	if(!iw_ap || !pass)
		return ERROR;
	if(iw_ap->mutex)
		os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);
	if(pass)
	{
		memset(&iw_ap->password[0], 0, sizeof(iw_ap->password[0]));
		if(os_memcmp(pass, "*#@", 3) == 0)
		{
			memset(iw_ap->password[0].encrypt_password, 0, sizeof(iw_ap->password[0].encrypt_password));
			strcpy(iw_ap->password[0].encrypt_password, pass);
		}
		else
		{
			strcpy(iw_ap->password[0].password, pass);
			memset(iw_ap->password[0].encrypt_password, 0, sizeof(iw_ap->password[0].encrypt_password));
			md5_encrypt_password(iw_ap->password[0].password, iw_ap->password[0].encrypt_password);
		}
		iw_ap_start(iw_ap);
	}
	if(iw_ap->mutex)
		os_mutex_unlock(iw_ap->mutex);
	return OK;
}

int iw_ap_password_del_api(iw_ap_t *iw_ap)
{
	if(!iw_ap)
		return ERROR;
	if(iw_ap->mutex)
		os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);
	memset(&iw_ap->password[0], 0, sizeof(iw_ap->password[0]));
	iw_ap_start(iw_ap);
	if(iw_ap->mutex)
		os_mutex_unlock(iw_ap->mutex);
	return OK;
}

int iw_ap_channel_set_api(iw_ap_t *iw_ap, int channel)
{
	char channel_str[64];
	if(!iw_ap )
		return ERROR;
	if(iw_ap->mutex)
		os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);
	iw_ap->channel = channel;
	memset(channel_str, 0, sizeof(channel_str));
	if(channel)
		snprintf(channel_str, sizeof(channel_str), "%d", channel);
	else if(channel == IW_VALUE_AUTO)
		snprintf(channel_str, sizeof(channel_str), "%s", "auto");
	iw_dev_channel_set(if_lookup_by_index(iw_ap->ifindex), channel_str);
	iw_ap_start(iw_ap);
	if(iw_ap->mutex)
		os_mutex_unlock(iw_ap->mutex);
	return OK;
}

int iw_ap_channel_del_api(iw_ap_t *iw_ap, int channel)
{
	char channel_str[64];
	if(!iw_ap )
		return ERROR;
	if(iw_ap->mutex)
		os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);
	iw_ap->channel = IW_AP_CHANNEL_DEFAULT;
	memset(channel_str, 0, sizeof(channel_str));
	if(channel)
		snprintf(channel_str, sizeof(channel_str), "%d", channel);
	else if(channel == IW_VALUE_AUTO)
		snprintf(channel_str, sizeof(channel_str), "%s", "auto");
	iw_dev_channel_set(if_lookup_by_index(iw_ap->ifindex), channel_str);
	iw_ap_start(iw_ap);
	if(iw_ap->mutex)
		os_mutex_unlock(iw_ap->mutex);
	return OK;
}

int iw_ap_power_set_api(iw_ap_t *iw_ap, int power)
{
	if(!iw_ap )
		return ERROR;
	if(iw_ap->mutex)
		os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);
	iw_ap->power = power;
	iw_ap_start(iw_ap);
	if(iw_ap->mutex)
		os_mutex_unlock(iw_ap->mutex);
	return OK;
}

int iw_ap_power_del_api(iw_ap_t *iw_ap, int power)
{
	if(!iw_ap )
		return ERROR;
	if(iw_ap->mutex)
		os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);
	iw_ap->power = power;
	iw_ap_start(iw_ap);
	if(iw_ap->mutex)
		os_mutex_unlock(iw_ap->mutex);
	return OK;
}

int iw_ap_signal_set_api(iw_ap_t *iw_ap, int signal)
{
	char value_str[64];
	if(!iw_ap )
		return ERROR;
	if(iw_ap->mutex)
		os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);
	iw_ap->signal = signal;
	memset(value_str, 0, sizeof(value_str));
	if(signal)
		snprintf(value_str, sizeof(value_str), "%ddBm", signal);
	else if(signal == IW_VALUE_AUTO)
		snprintf(value_str, sizeof(value_str), "%s", "auto");
	else if(signal == IW_VALUE_OFF)
		snprintf(value_str, sizeof(value_str), "%s", "off");
	iw_dev_txpower_set(if_lookup_by_index(iw_ap->ifindex), value_str);

	iw_ap_start(iw_ap);
	if(iw_ap->mutex)
		os_mutex_unlock(iw_ap->mutex);
	return OK;
}

int iw_ap_beacon_set_api(iw_ap_t *iw_ap, int beacon)
{
	if(!iw_ap )
		return ERROR;
	if(iw_ap->mutex)
		os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);
	iw_ap->beacon = beacon;
	iw_ap_start(iw_ap);
	if(iw_ap->mutex)
		os_mutex_unlock(iw_ap->mutex);
	return OK;
}

int iw_ap_bitrate_set_api(iw_ap_t *iw_ap, int bitrate)
{
	//k|M|G
	char value_str[64];
	if(!iw_ap )
		return ERROR;
	if(iw_ap->mutex)
		os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);
	iw_ap->bitrate = bitrate;
	memset(value_str, 0, sizeof(value_str));
	if(bitrate)
	{
		if(bitrate > 10000000000)
			snprintf(value_str, sizeof(value_str), "%dG", bitrate);
		else if(bitrate > 10000000)
			snprintf(value_str, sizeof(value_str), "%dM", bitrate);
		else if(bitrate > 1000)
			snprintf(value_str, sizeof(value_str), "%dk", bitrate);
	}
	else if(bitrate == IW_VALUE_AUTO)
		snprintf(value_str, sizeof(value_str), "%s", "auto");
	else if(bitrate == IW_VALUE_FIEXD)
		snprintf(value_str, sizeof(value_str), "%s", "fixed");

	iw_dev_rate_set(if_lookup_by_index(iw_ap->ifindex), value_str);

	iw_ap_start(iw_ap);
	if(iw_ap->mutex)
		os_mutex_unlock(iw_ap->mutex);
	return OK;
}

int iw_ap_isolate_set_api(iw_ap_t *iw_ap, BOOL enable)
{
	if(!iw_ap )
		return ERROR;
	if(iw_ap->mutex)
		os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);
	iw_ap->ap_isolate = enable;
	iw_ap_start(iw_ap);
	if(iw_ap->mutex)
		os_mutex_unlock(iw_ap->mutex);
	return OK;
}

int iw_ap_bridge_set_api(iw_ap_t *iw_ap, ifindex_t bridge)
{
	if(!iw_ap )
		return ERROR;
	if(iw_ap->mutex)
		os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);
	iw_ap->bridge = bridge;
	iw_ap_start(iw_ap);
	if(iw_ap->mutex)
		os_mutex_unlock(iw_ap->mutex);
	return OK;
}


int iw_ap_rts_threshold_set_api(iw_ap_t *iw_ap, int rts_threshold)
{
	char value_str[64];
	if(!iw_ap )
		return ERROR;
	if(iw_ap->mutex)
		os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);
	iw_ap->rts_threshold = rts_threshold;
	memset(value_str, 0, sizeof(value_str));
	if(rts_threshold)
		snprintf(value_str, sizeof(value_str), "%d", rts_threshold);
	else if(rts_threshold == IW_VALUE_AUTO)
		snprintf(value_str, sizeof(value_str), "%s", "auto");
	else if(rts_threshold == IW_VALUE_FIEXD)
		snprintf(value_str, sizeof(value_str), "%s", "fixed");
	else if(rts_threshold == IW_VALUE_OFF)
		snprintf(value_str, sizeof(value_str), "%s", "off");
	iw_dev_rts_threshold_set(if_lookup_by_index(iw_ap->ifindex), value_str);

	//iw_ap_start(iw_ap);
	if(iw_ap->mutex)
		os_mutex_unlock(iw_ap->mutex);
	return OK;
}

int iw_ap_frag_set_api(iw_ap_t *iw_ap, int frag)
{
	char value_str[64];
	if(!iw_ap )
		return ERROR;
	if(iw_ap->mutex)
		os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);
	iw_ap->frag = frag;
	memset(value_str, 0, sizeof(value_str));
	if(frag)
		snprintf(value_str, sizeof(value_str), "%d", frag);
	else if(frag == IW_VALUE_AUTO)
		snprintf(value_str, sizeof(value_str), "%s", "auto");
	else if(frag == IW_VALUE_FIEXD)
		snprintf(value_str, sizeof(value_str), "%s", "fixed");
	else if(frag == IW_VALUE_OFF)
		snprintf(value_str, sizeof(value_str), "%s", "off");
	iw_dev_frag_set(if_lookup_by_index(iw_ap->ifindex), value_str);

	//iw_ap_start(iw_ap);
	if(iw_ap->mutex)
		os_mutex_unlock(iw_ap->mutex);
	return OK;
}

int iw_ap_country_set_api(iw_ap_t *iw_ap, int country)
{
	if(!iw_ap )
		return ERROR;
	if(iw_ap->mutex)
		os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);
	iw_ap->country_code = country;
	iw_ap_start(iw_ap);
	if(iw_ap->mutex)
		os_mutex_unlock(iw_ap->mutex);
	return OK;
}

int iw_ap_wmm_set_api(iw_ap_t *iw_ap, BOOL enable)
{
	if(!iw_ap )
		return ERROR;
	if(iw_ap->mutex)
		os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);
	iw_ap->wmm_enabled = enable;
	iw_ap_start(iw_ap);
	if(iw_ap->mutex)
		os_mutex_unlock(iw_ap->mutex);
	return OK;
}

int iw_ap_freq_set_api(iw_ap_t *iw_ap, double freq)
{
	if(!iw_ap )
		return ERROR;
	if(iw_ap->mutex)
		os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);
	iw_ap->freq = freq;
	iw_ap_start(iw_ap);
	if(iw_ap->mutex)
		os_mutex_unlock(iw_ap->mutex);
	return OK;
}

int iw_ap_scan_num_set_api(iw_ap_t *iw_ap, int value)
{
	if(!iw_ap )
		return ERROR;
	if(iw_ap->mutex)
		os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);
	iw_ap->acs_num_scans = value;
	iw_ap_start(iw_ap);
	if(iw_ap->mutex)
		os_mutex_unlock(iw_ap->mutex);
	return OK;
}

int iw_ap_client_num_set_api(iw_ap_t *iw_ap, int value)
{
	if(!iw_ap )
		return ERROR;
	if(iw_ap->mutex)
		os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);
	iw_ap->max_num_sta = value;
	iw_ap_start(iw_ap);
	if(iw_ap->mutex)
		os_mutex_unlock(iw_ap->mutex);
	return OK;
}


int iw_ap_auth_set_api(iw_ap_t *iw_ap, iw_authentication_t auth)
{
	if(!iw_ap )
		return ERROR;
	if(iw_ap->mutex)
		os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);
	iw_ap->auth = auth;
	iw_ap_start(iw_ap);
	if(iw_ap->mutex)
		os_mutex_unlock(iw_ap->mutex);
	return OK;
}

int iw_ap_auth_del_api(iw_ap_t *iw_ap, iw_authentication_t auth)
{
	if(!iw_ap )
		return ERROR;
	if(iw_ap->mutex)
		os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);
	iw_ap->auth = IW_AP_AUTH_DEFAULT;
	iw_ap_start(iw_ap);
	if(iw_ap->mutex)
		os_mutex_unlock(iw_ap->mutex);
	return OK;
}

int iw_ap_encryption_set_api(iw_ap_t *iw_ap, iw_encryption_t encryption)
{
	if(!iw_ap )
		return ERROR;
	if(iw_ap->mutex)
		os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);
	iw_ap->encryption = encryption;
	iw_ap_start(iw_ap);
	if(iw_ap->mutex)
		os_mutex_unlock(iw_ap->mutex);
	return OK;
}

int iw_ap_encryption_del_api(iw_ap_t *iw_ap, iw_encryption_t encryption)
{
	if(!iw_ap )
		return ERROR;
	if(iw_ap->mutex)
		os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);
	iw_ap->encryption = IW_AP_ENCRYPTION_DEFAULT;
	iw_ap_start(iw_ap);
	if(iw_ap->mutex)
		os_mutex_unlock(iw_ap->mutex);
	return OK;
}

int iw_ap_hw_mode_set_api(iw_ap_t *iw_ap, iw_hw_mode_t mode)
{
	if(!iw_ap )
		return ERROR;
	if(iw_ap->mutex)
		os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);
	iw_ap->hw_mode = mode;
	iw_ap_start(iw_ap);
	if(iw_ap->mutex)
		os_mutex_unlock(iw_ap->mutex);
	return OK;
}

/*int iw_ap_network_set_api(iw_ap_t *iw_ap, iw_network_t network)
{
	if(!iw_ap )
		return ERROR;
	if(iw_ap->mutex)
		os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);
	iw_ap->network = network;
	if(iw_ap->mutex)
		os_mutex_unlock(iw_ap->mutex);
	return OK;
}

int iw_ap_network_del_api(iw_ap_t *iw_ap, iw_network_t network)
{
	if(!iw_ap )
		return ERROR;
	if(iw_ap->mutex)
		os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);
	iw_ap->network = network;
	if(iw_ap->mutex)
		os_mutex_unlock(iw_ap->mutex);
	return OK;
}*/


iw_ap_t * iw_ap_lookup_api(struct interface *ifp)
{
	iw_t * iw = nsm_iw_get(ifp);
	if(iw)
	{
		if(iw->mode == IW_MODE_AP)
			return &iw->private.ap;
	}
	return NULL;
}


/*
 * MAC ACL list
 */



static int iw_ap_maclist_cleanup(iw_ap_t *iw_ap, BOOL accept)
{
	NODE index;
	LIST *list = NULL;
	iw_ap_mac_t *pstNode = NULL;
	if(!iw_ap)
		return ERROR;
	if(accept)
		list = iw_ap->mac_list;
	else
		list = iw_ap->dmac_list;
	if(!list)
		return ERROR;
	if(iw_ap->ap_mutex)
		os_mutex_lock(iw_ap->ap_mutex, OS_WAIT_FOREVER);
	for(pstNode = (iw_ap_mac_t *)lstFirst(list);
			pstNode != NULL;  pstNode = (iw_ap_mac_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode)
		{
			lstDelete(list, (NODE*)pstNode);
			XFREE(MTYPE_WIFI_DB, pstNode);
		}
	}
	iw_ap_start(iw_ap);
	if(iw_ap->ap_mutex)
		os_mutex_unlock(iw_ap->ap_mutex);
	return OK;
}


iw_ap_mac_t * iw_ap_mac_lookup_api(iw_ap_t *iw_ap, u_int8 *mac, BOOL accept)
{
	iw_ap_mac_t *pstNode = NULL;
	NODE index;
	LIST *list = NULL;
	u_int8 MAC[IW_SSID_NAME_MAX];
	memset(MAC, 0, IW_SSID_NAME_MAX);
	if(!iw_ap || !mac)
		return NULL;
	if(mac)
		memcpy(MAC, mac, NSM_MAC_MAX);

	if(accept)
		list = iw_ap->mac_list;
	else
		list = iw_ap->dmac_list;
	if(!list)
		return ERROR;
	if(iw_ap->ap_mutex)
		os_mutex_lock(iw_ap->ap_mutex, OS_WAIT_FOREVER);

	for(pstNode = (iw_ap_mac_t *)lstFirst(list);
			pstNode != NULL;  pstNode = (iw_ap_mac_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(mac)
		{
			if(memcmp(pstNode->MAC, MAC, NSM_MAC_MAX) == 0)
			{
				if(iw_ap->ap_mutex)
					os_mutex_unlock(iw_ap->ap_mutex);
				return pstNode;
			}
		}
	}
	if(iw_ap->ap_mutex)
		os_mutex_unlock(iw_ap->ap_mutex);
	return NULL;
}


int iw_ap_mac_add_api(iw_ap_t *iw_ap, u_int8 *mac, BOOL accept)
{
	iw_ap_mac_t *node = NULL;
	if(!iw_ap || !mac)
		return ERROR;
	if(iw_ap_mac_lookup_api(iw_ap, mac,  accept))
		return ERROR;

	if(accept)
	{
		if(!iw_ap->mac_list)
			return ERROR;
	}
	else
	{
		if(!iw_ap->dmac_list)
			return ERROR;
	}

	if(iw_ap->ap_mutex)
		os_mutex_lock(iw_ap->ap_mutex, OS_WAIT_FOREVER);

	node = XMALLOC(MTYPE_WIFI_DB, sizeof(iw_ap_mac_t));
	if(node)
	{
		os_memset(node, 0, sizeof(iw_ap_mac_t));
		memcpy(node->MAC, mac, NSM_MAC_MAX);
		if(accept)
			lstAdd(iw_ap->mac_list, (NODE *)node);
		else
			lstAdd(iw_ap->dmac_list, (NODE *)node);
		iw_ap_start(iw_ap);
		if(iw_ap->ap_mutex)
			os_mutex_unlock(iw_ap->ap_mutex);
		return OK;
	}
	if(iw_ap->ap_mutex)
		os_mutex_unlock(iw_ap->ap_mutex);
	return ERROR;
}

int iw_ap_mac_del_api(iw_ap_t *iw_ap, u_int8 *mac, BOOL accept)
{
	iw_ap_mac_t *node = NULL;
	if(!iw_ap || !mac)
		return ERROR;
	node = iw_ap_mac_lookup_api(iw_ap, mac,  accept);
	if(!node)
		return ERROR;
	if(accept)
	{
		if(!iw_ap->mac_list)
			return ERROR;
	}
	else
	{
		if(!iw_ap->dmac_list)
			return ERROR;
	}
	if(iw_ap->ap_mutex)
		os_mutex_lock(iw_ap->ap_mutex, OS_WAIT_FOREVER);
	if(iw_ap && node)
	{
		if(accept)
			lstDelete(iw_ap->mac_list, (NODE *)node);
		else
			lstDelete(iw_ap->dmac_list, (NODE *)node);
		XFREE(MTYPE_WIFI_CLIENT, node);
		iw_ap_start(iw_ap);
		if(iw_ap->ap_mutex)
			os_mutex_unlock(iw_ap->ap_mutex);
		return OK;
	}
	if(iw_ap->ap_mutex)
		os_mutex_unlock(iw_ap->ap_mutex);
	return ERROR;
}


int iw_ap_config(iw_ap_t *iw_ap, struct vty *vty)
{
	NODE index;
	char buf[128];
	iw_ap_mac_t *pstNode = NULL;
	assert(iw_ap != NULL);
	if(!vty)
		return ERROR;

	if(iw_ap->ap_mutex)
		os_mutex_lock(iw_ap->ap_mutex, OS_WAIT_FOREVER);
	if(iw_ap->hw_mode != IW_AP_HW_MODE_DEFAULT)
		vty_out(vty, " radio-type %s%s",
			iw_hw_mode_string(iw_ap->hw_mode), VTY_NEWLINE);

	if (os_strlen(iw_ap->SSID))
		vty_out(vty, " network-name %s%s", (iw_ap->SSID),VTY_NEWLINE);


	if(iw_ap->auth != IW_AP_AUTH_DEFAULT)
		vty_out(vty, " authentication-method %s%s",
			iw_auth_string(iw_ap->auth), VTY_NEWLINE);
	if(iw_ap->encryption != IW_AP_ENCRYPTION_DEFAULT)
		vty_out(vty, " encryption-method %s%s",
			iw_encryption_string(iw_ap->encryption), VTY_NEWLINE);

	if(iw_ap->auth == IW_ENCRY_WPA_PSK ||		//= 1 WPA-PSK CCMP/AUTO
		iw_ap->auth == IW_ENCRY_WPA2_PSK ||		//= 2 WPA2-PSK CCMP/AUTO
		iw_ap->auth == IW_ENCRY_WPA2WPA_PSK)
	{
		if(vty->type == VTY_FILE)
		{
			if (os_strlen(iw_ap->password[0].password))
				vty_out(vty, " authentication password %s%s", iw_ap->password[0].password, VTY_NEWLINE);
		}
		else
		{
			if (os_strlen(iw_ap->password[0].encrypt_password))
				vty_out(vty, " authentication password %s%s", iw_ap->password[0].encrypt_password, VTY_NEWLINE);
		}
	}

	if(iw_ap->auth == IW_ENCRY_WEP_OPEN ||
		iw_ap->auth == IW_ENCRY_WEP_PRIVATE)
	{

		//vty_out(vty, " ap-password %d%s", iw_ap->wep_key, VTY_NEWLINE);
		if(vty->type == VTY_FILE)
		{
			if (os_strlen(iw_ap->password[0].password))
				vty_out(vty, " authentication password %s%s", iw_ap->password[0].password, VTY_NEWLINE);
		}
		else
		{
			if (os_strlen(iw_ap->password[0].encrypt_password))
				vty_out(vty, " authentication password %s%s", iw_ap->password[0].encrypt_password, VTY_NEWLINE);
		}

/*		if (os_strlen(iw_ap->password[1].password))
			vty_out(vty, " ap-password %s%s", iw_ap->password[1].encrypt_password, VTY_NEWLINE);

		if (os_strlen(iw_ap->password[2].password))
			vty_out(vty, " ap-password %s%s", iw_ap->password[2].encrypt_password, VTY_NEWLINE);

		if (os_strlen(iw_ap->password[3].password))
			vty_out(vty, " ap-password %s%s", iw_ap->password[3].encrypt_password, VTY_NEWLINE);*/
	}

	if (iw_ap->channel != IW_AP_CHANNEL_DEFAULT)
		vty_out(vty, " work-channel %d%s", iw_ap->channel, VTY_NEWLINE);
/*	else
		vty_out(vty, " work-channel auto%s", VTY_NEWLINE);*/

	if (iw_ap->power)
		vty_out(vty, " power-level %d%s", iw_ap->power, VTY_NEWLINE);
/*	else if (iw_ap->power == IW_VALUE_AUTO)
		vty_out(vty, " power-level auto%s", VTY_NEWLINE);*/
	else if(iw_ap->power == IW_VALUE_OFF)
		vty_out(vty, " power-level off%s", VTY_NEWLINE);

	if(iw_ap->beacon != IW_AP_BEACON_DEFAULT)
		vty_out(vty," beacon-interval %d%s", iw_ap->beacon, VTY_NEWLINE);

	if(iw_ap->country_code != IW_AP_COUNTRY_DEFAULT)
		vty_out(vty," country-code %02d%s", iw_ap->country_code, VTY_NEWLINE);

	if(iw_ap->signal)
		vty_out(vty," signal-level %d%s", iw_ap->signal, VTY_NEWLINE);
	else if(iw_ap->signal == IW_VALUE_FIEXD)
		vty_out(vty," signal-level off%s", VTY_NEWLINE);
/*	else if(iw_ap->signal == IW_VALUE_AUTO)
		vty_out(vty," signal-level auto%s", VTY_NEWLINE);*/

	if(iw_ap->bitrate)
		vty_out(vty," bitrate %d%s", iw_ap->bitrate, VTY_NEWLINE);
	else if(iw_ap->bitrate == IW_VALUE_FIEXD)
		vty_out(vty," bitrate fixed%s", VTY_NEWLINE);
/*	else if(iw_ap->bitrate == IW_VALUE_AUTO)
		vty_out(vty," bitrate auto%s", VTY_NEWLINE);*/

	if(iw_ap->rts_threshold)
		vty_out(vty," threshold rts %d%s", iw_ap->rts_threshold, VTY_NEWLINE);
/*	else if(iw_ap->rts_threshold == IW_VALUE_AUTO)
		vty_out(vty," threshold rts auto%s", VTY_NEWLINE);*/
	else if(iw_ap->rts_threshold == IW_VALUE_FIEXD)
		vty_out(vty," threshold rts fixed%s", VTY_NEWLINE);
	else if(iw_ap->rts_threshold == IW_VALUE_OFF)
		vty_out(vty," threshold rts off%s", VTY_NEWLINE);

	if(iw_ap->frag)
		vty_out(vty," threshold frame %d%s", iw_ap->frag, VTY_NEWLINE);
/*	else if(iw_ap->frag == IW_VALUE_AUTO)
		vty_out(vty," threshold frame auto%s", VTY_NEWLINE);*/
	else if(iw_ap->frag == IW_VALUE_FIEXD)
		vty_out(vty," threshold frame fixed%s", VTY_NEWLINE);
	else if(iw_ap->frag == IW_VALUE_OFF)
		vty_out(vty," threshold frame off%s", VTY_NEWLINE);

	if(iw_ap->wmm_enabled != IW_AP_WMM_DEFAULT)
		vty_out(vty," no wmm-enable %s", VTY_NEWLINE);
	if(iw_ap->ap_isolate)
		vty_out(vty," isolate enable %s", VTY_NEWLINE);

/*
	if(iw_ap->bridge)
		vty_out(vty," ap-bridge %s%s", ifindex2ifname(iw_ap->bridge), VTY_NEWLINE);
*/

	if(iw_ap->max_num_sta != IW_AP_MAX_STA_DEFAULT)
		vty_out(vty," accept client %d%s", iw_ap->max_num_sta, VTY_NEWLINE);

	//vty_out(vty," ap-password %d%s", iw_ap->channel, VTY_NEWLINE);

	if(iw_ap->mac_list && lstCount(iw_ap->mac_list))
	{
		for(pstNode = (iw_ap_mac_t *)lstFirst(iw_ap->mac_list);
				pstNode != NULL;  pstNode = (iw_ap_mac_t *)lstNext((NODE*)&index))
		{
			index = pstNode->node;
			if(pstNode)
			{
				memset(buf, 0, sizeof(buf));
				sprintf(buf, "%s", if_mac_out_format(pstNode->MAC));
				vty_out(vty, " mac permit %s%s",buf, VTY_NEWLINE);
			}
		}
	}
	if(iw_ap->dmac_list && lstCount(iw_ap->dmac_list))
	{
		for(pstNode = (iw_ap_mac_t *)lstFirst(iw_ap->dmac_list);
				pstNode != NULL;  pstNode = (iw_ap_mac_t *)lstNext((NODE*)&index))
		{
			index = pstNode->node;
			if(pstNode)
			{
				memset(buf, 0, sizeof(buf));
				sprintf(buf, "%s", if_mac_out_format(pstNode->MAC));
				vty_out(vty, " mac deny %s%s",buf, VTY_NEWLINE);
			}
		}
	}
	if(iw_ap->ap_mutex)
		os_mutex_unlock(iw_ap->ap_mutex);
	return OK;
}

/*
root@OpenWrt:/# iwconfig wlan0
wlan0     IEEE 802.11  Mode:Master
          RTS thr=100 B   Fragment thr=1400 B
          Power Management:off

root@OpenWrt:/#

*/


#ifdef PL_BUILD_OPENWRT
int _iw_bridge_check_interface(char *br, char *wa)
{
	char buf[512];
	FILE *f = NULL;
/*	if(wa && strlen(wa))
	{
		memset(cmdtmp, 0, sizeof(cmdtmp));
		if(super_output_system("brctl show", cmdtmp, sizeof(cmdtmp)) == OK)
		{
			if(strstr(cmdtmp, wa))
				return OK;
		}
	}*/
	super_system("brctl show > /tmp/.brctl");
	f = fopen("/tmp/.brctl", "r");
	if (f)
	{
		memset(buf, 0, sizeof(buf));
		while (fgets(buf, sizeof(buf), f))
		{
			if(strstr(buf, wa))
			{
				fclose(f);
				remove("/tmp/.brctl");
				sync();
				return OK;
			}
			memset(buf, 0, sizeof(buf));
		}
		fclose(f);
	}
	remove("/tmp/.brctl");
	sync();
	return ERROR;
}
#endif

static int iw_ap_scan_thread(struct thread * thread)
{
	assert(thread != NULL);
	iw_ap_t *iw_ap = THREAD_ARG(thread);
	if(iw_ap && iw_ap->master)
	{
#ifdef PL_BUILD_OPENWRT
		struct interface *ifp = NULL;
		char cmdtmp[128];
		ifp = if_lookup_by_index(iw_ap->ifindex);
		if(ifp)
		{
			if(_iw_bridge_check_interface("br-lan", ifp->k_name) != OK)
			{
				memset(cmdtmp, 0, sizeof(cmdtmp));
				sprintf(cmdtmp, "brctl addif br-lan %s", ifp->k_name);
				//zlog_debug(ZLOG_WIFI, "=======%s======%s", __func__, cmdtmp);
				super_system(cmdtmp);
			}
		}
		else
		{
			//zlog_debug(ZLOG_WIFI, "=======%s==1====", __func__);
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0)
			super_system("brctl addif br-lan wlan0");
#else
			super_system("brctl addif br-lan ra0");
#endif
		}
#endif
		if(iw_ap->mutex)
			os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);

		if(iw_ap_script_is_running(iw_ap) != OK)
			iw_ap_running_script(iw_ap);

		iw_ap->s_thread = NULL;

		iw_ap_connect_scanning(iw_ap);

		iw_ap_connect_update(iw_ap, 0);

		iw_ap->s_thread = thread_add_timer(iw_ap->master,iw_ap_scan_thread,
							iw_ap, iw_ap->ap_client_delay);
		//zlog_debug(ZLOG_WIFI, "%s", __func__);
		if(iw_ap->mutex)
			os_mutex_unlock(iw_ap->mutex);
	}
	return OK;
}


static int iw_ap_start_thread(struct thread * thread)
{
	assert(thread != NULL);
	iw_ap_t *iw_ap = THREAD_ARG(thread);
	if(iw_ap && iw_ap->master)
	{
#ifdef PL_BUILD_OPENWRT
		struct interface *ifp = NULL;
		char cmdtmp[128];
		ifp = if_lookup_by_index(iw_ap->ifindex);
		if(ifp)
		{
			if(_iw_bridge_check_interface("br-lan", ifp->k_name) != OK)
			{
				memset(cmdtmp, 0, sizeof(cmdtmp));
				sprintf(cmdtmp, "brctl addif br-lan %s", ifp->k_name);
				//zlog_debug(ZLOG_WIFI, "=======%s======%s", __func__, cmdtmp);
				super_system(cmdtmp);
			}
		}
		else
		{
			//zlog_debug(ZLOG_WIFI, "=======%s==1====", __func__);
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0)
			super_system("brctl addif br-lan wlan0");
#else
			super_system("brctl addif br-lan ra0");
#endif
		}
#endif
		if(iw_ap->mutex)
			os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);
		iw_ap->t_thread = NULL;

		//if(iw_ap_make_script(iw_ap) == OK)
		iw_ap_running_script(iw_ap);
		//zlog_debug(ZLOG_WIFI, "%s", __func__);

		if(iw_ap->s_thread && iw_ap->master)
			thread_cancel(iw_ap->s_thread);
		iw_ap->s_thread = NULL;
		iw_ap->s_thread = thread_add_timer(iw_ap->master, iw_ap_scan_thread,
					iw_ap, iw_ap->ap_client_delay/2);

		if(iw_ap->mutex)
			os_mutex_unlock(iw_ap->mutex);
	}
	return OK;
}

static int iw_ap_start(iw_ap_t *iw_ap)
{
	int crc_sum = 0;
	assert(iw_ap != NULL);
	//if(iw_ap->ap_mutex)
	//	os_mutex_lock(iw_ap->ap_mutex, OS_WAIT_FOREVER);
#ifndef IW_ONCE_TASK
	//int size = sizeof(u_int8) + sizeof(BOOL) + sizeof(u_int32) + sizeof(int);
#else
	//int size = sizeof(u_int8) + sizeof(BOOL) + sizeof(u_int32) + sizeof(void) * 3;
#endif

	crc_sum = crc_checksum((unsigned char *)iw_ap,  (int)(&iw_ap->ap_client_delay) - (int)(iw_ap));
	if(iw_ap->crc_sum != crc_sum)
	{
		iw_ap->crc_sum = crc_sum;
		iw_ap->change = TRUE;
		//zlog_debug(ZLOG_WIFI, "=======%s====change==", __func__);
	}
	//zlog_debug(ZLOG_WIFI, "%s", __func__);

	if(iw_ap->t_thread && iw_ap->master)
		thread_cancel(iw_ap->t_thread);
	iw_ap->t_thread = NULL;

	//zlog_debug(ZLOG_WIFI, "%s thread_cancel", __func__);

	if(iw_ap->t_thread == NULL && iw_ap->master)
		iw_ap->t_thread = thread_add_timer(iw_ap->master, iw_ap_start_thread, iw_ap, 10);

	//zlog_debug(ZLOG_WIFI, "%s thread_add_timer", __func__);
	//iw_ap->thread = thread_add_timer(zebrad.master, iw_ap_start_thread, iw_ap, 2);
	//if(iw_ap->ap_mutex) iw_ap->t_thread,
	//	os_mutex_unlock(iw_ap->ap_mutex);
	return OK;
}

static int iw_ap_stop(iw_ap_t *iw_ap)
{
	assert(iw_ap != NULL);
	//if(iw_ap->ap_mutex)
	//	os_mutex_lock(iw_ap->ap_mutex, OS_WAIT_FOREVER);
	//os_job_add(iw_ap_stop_script, iw_ap);
	if(iw_ap->t_thread)
		thread_cancel(iw_ap->t_thread);
	iw_ap->t_thread = NULL;
	if(iw_ap->s_thread)
		thread_cancel(iw_ap->s_thread);
	iw_ap->s_thread = NULL;
	//iw_ap->t_thread = NULL;
	//iw_ap->s_thread = NULL;
	iw_ap->change = FALSE;
	//zlog_debug(ZLOG_WIFI, "%s", __func__);
	iw_ap_stop_script(iw_ap);

	//if(iw_ap->ap_mutex)
	//	os_mutex_unlock(iw_ap->ap_mutex);
	return OK;
}

#ifndef IW_ONCE_TASK
static int iw_ap_task(iw_ap_t *iw_ap)
{
	while(!os_load_config_done())
	{
		os_sleep(1);
	}
	while(1)
	{
/*		iw_client_connect_process(iw_client);
		os_sleep(iw_client->connect_delay);*/
		struct thread thread;
		//os_log_reopen(ZLOG_NSM);
		while (thread_fetch (master_thread[PL_WIFI_MODULE], &thread))
			thread_call (&thread);
	}
	return OK;
}
#endif

int iw_ap_task_start(iw_ap_t *iw_ap)
{
#ifndef IW_ONCE_TASK
	if(!iw_ap)
		return ERROR;
	/* Make master thread emulator. */
	master_thread[PL_WIFI_MODULE] = thread_master_module_create (PL_WIFI_MODULE);

	if(iw_ap->taskid == 0)
		iw_ap->taskid = os_task_create("iwApTask", OS_TASK_DEFAULT_PRIORITY,
	               0, iw_ap_task, iw_ap, OS_TASK_DEFAULT_STACK);
	if(iw_ap->taskid)
		return OK;
	return ERROR;
#else
	if(!iw_ap)
		return ERROR;
	/* Make master thread emulator. */
	iw_ap->master = master_thread[PL_WIFI_MODULE] = thread_master_module_create (PL_WIFI_MODULE);
	return OK;
#endif
}


int iw_ap_task_exit(iw_ap_t *iw_ap)
{
	if(!iw_ap)
		return ERROR;
#ifndef IW_ONCE_TASK
	if(iw_ap->taskid)
	{
		iw_ap_stop(iw_ap);
		thread_master_free(master_thread[PL_WIFI_MODULE]);
		if(os_task_destroy(iw_ap->taskid)==OK)
			iw_ap->taskid = 0;
	}
#else
	iw_ap_stop(iw_ap);
#endif
	return OK;
}


static int iw_ap_default_init(iw_ap_t *iw_ap, ifindex_t ifindex)
{
	struct interface *ifp = NULL;
	assert(iw_ap != NULL);
	assert(ifindex);
#ifdef PL_BUILD_OPENWRT
	char cmdtmp[128];
#endif
	iw_ap->hw_mode			= IW_AP_HW_MODE_DEFAULT;
	iw_ap->encryption		= IW_AP_ENCRYPTION_DEFAULT;
	iw_ap->auth				= IW_AP_AUTH_DEFAULT;
	iw_ap->channel			= IW_AP_CHANNEL_DEFAULT;
	iw_ap->beacon			= IW_AP_BEACON_DEFAULT;
	//iw_ap->power;

	iw_ap->driver			= IW_AP_DRIVER_DEFAULT;
	iw_ap->country_code		= IW_AP_COUNTRY_DEFAULT;
	iw_ap->wmm_enabled		= IW_AP_WMM_DEFAULT;
	iw_ap->max_num_sta		= IW_AP_MAX_STA_DEFAULT;
	iw_ap->ap_client_delay 	= IW_AP_CLIENT_SCAN_DEFAULT;
	iw_ap->ifindex = ifindex;
	ifp = if_lookup_by_index(iw_ap->ifindex);
	if(ifp)
		nsm_interface_mac_get_api(ifp, iw_ap->BSSID, NSM_MAC_MAX);

	printf("===========%s==============(%s):bssid=%02x:%02x:%02x:%02x:%02x:%02x\n", __func__,
		   ifp? ifp->name:"NULL",
		   iw_ap->BSSID[0], iw_ap->BSSID[1],
			iw_ap->BSSID[2], iw_ap->BSSID[3], iw_ap->BSSID[4], iw_ap->BSSID[5]);

#ifdef PL_BUILD_OPENWRT
	if(ifp)
	{
		if(_iw_bridge_check_interface("br-lan", ifp->k_name) != OK)
		{
			memset(cmdtmp, 0, sizeof(cmdtmp));
			sprintf(cmdtmp, "brctl addif br-lan %s", ifp->k_name);
			//zlog_debug(ZLOG_WIFI, "=============%s", cmdtmp);
			super_system(cmdtmp);
		}
	}
	else
	{
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0)
			super_system("brctl addif br-lan wlan0");
#else
			super_system("brctl addif br-lan ra0");
#endif
	}
#endif
	return OK;
}

int iw_ap_init(iw_ap_t *iw_ap, ifindex_t ifindex)
{
	assert(iw_ap != NULL);
	assert(ifindex);
	os_memset(iw_ap, 0, sizeof(iw_ap_t));

	iw_ap->ap_list = malloc(sizeof(LIST));
	iw_ap->ap_mutex = os_mutex_init();
	lstInit(iw_ap->ap_list);

	iw_ap->mac_list = malloc(sizeof(LIST));
	lstInit(iw_ap->mac_list);
	iw_ap->dmac_list = malloc(sizeof(LIST));
	lstInit(iw_ap->dmac_list);

	iw_ap->mutex = os_mutex_init();

	iw_ap_default_init(iw_ap, ifindex);
	iw_ap_task_start(iw_ap);
	return OK;
}


int iw_ap_exit(iw_ap_t *iw_ap)
{
#ifdef PL_BUILD_OPENWRT
	char cmdtmp[128];
	struct interface *ifp = NULL;
#endif
	assert(iw_ap != NULL);
	iw_ap_task_exit(iw_ap);

	if(lstCount(iw_ap->ap_list))
	{
		iw_ap_connect_cleanup(iw_ap, TRUE);
		lstFree(iw_ap->ap_list);
		//iw_ap->ap_list = NULL;
	}
	if(lstCount(iw_ap->mac_list))
	{
		iw_ap_maclist_cleanup(iw_ap, TRUE);
		lstFree(iw_ap->mac_list);
		//iw_ap->ap_list = NULL;
	}
	if(lstCount(iw_ap->dmac_list))
	{
		iw_ap_maclist_cleanup(iw_ap, FALSE);
		lstFree(iw_ap->dmac_list);
		//iw_ap->ap_list = NULL;
	}
	if(iw_ap->ap_mutex)
		os_mutex_exit(iw_ap->ap_mutex);
	iw_ap->ap_mutex = NULL;

	if(iw_ap->ap_list)
		free(iw_ap->ap_list);
	iw_ap->ap_list = NULL;

	if(iw_ap->dmac_list)
		free(iw_ap->mac_list);
	iw_ap->mac_list = NULL;

	if(iw_ap->dmac_list)
		free(iw_ap->dmac_list);
	iw_ap->dmac_list = NULL;

	if(iw_ap->mutex)
		os_mutex_exit(iw_ap->mutex);
	iw_ap->mutex = NULL;

#ifdef PL_BUILD_OPENWRT
	ifp = if_lookup_by_index(iw_ap->ifindex);
	if(ifp)
	{
		if(_iw_bridge_check_interface("br-lan", ifp->k_name) == OK)
		{
			memset(cmdtmp, 0, sizeof(cmdtmp));
			sprintf(cmdtmp, "brctl delif br-lan %s", ifp->k_name);
			//zlog_debug(ZLOG_WIFI, "=============%s", cmdtmp);
			super_system(cmdtmp);
		}
	}
	else
	{
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0)
			super_system("brctl delif br-lan wlan0");
#else
			super_system("brctl delif br-lan ra0");
#endif
	}
#endif
	return OK;
}

int iw_ap_enable(iw_ap_t *iw_ap, BOOL enable)
{
	if(!iw_ap)
		return ERROR;
	if(enable)
	{
		printf("============%s==============: into iw_ap_start\r\n", __func__);
		if(iw_ap->mutex)
			os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);
		printf("============%s==============:execue iw_ap_start\r\n", __func__);
		//iw_ap_task_start(iw_ap);
		iw_ap_start(iw_ap);

		if(iw_ap->mutex)
			os_mutex_unlock(iw_ap->mutex);
		printf("============%s==============:level iw_ap_start\r\n", __func__);
	}
	else
	{
		printf("============%s==============:into iw_ap_stop\r\n", __func__);
		if(iw_ap->mutex)
			os_mutex_lock(iw_ap->mutex, OS_WAIT_FOREVER);

		printf("============%s==============:execue iw_ap_stop\r\n", __func__);
		//iw_ap_task_exit(iw_ap);
		iw_ap_stop(iw_ap);

		printf("============%s==============:level iw_ap_stop\r\n", __func__);
		if(iw_ap->ap_list && lstCount(iw_ap->ap_list))
		{
			iw_ap_connect_cleanup(iw_ap, TRUE);
			//lstFree(iw_ap->ap_list);
		}

		if(iw_ap->mutex)
			os_mutex_unlock(iw_ap->mutex);
	}
	return OK;
}
