/*
 * x5-b-ctl.c
 *
 *  Created on: 2018年12月28日
 *      Author: DELL
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
#include "eloop.h"
#include "cJSON.h"

#include "nexthop.h"
#include "nsm_rib.h"


#include "x5_b_app.h"
#include "x5_b_cmd.h"
#include "x5_b_ctl.h"
#include "x5_b_util.h"
#include "x5_b_web.h"
#include "x5b_facecard.h"
#ifdef PL_PJSIP_MODULE
#include "voip_def.h"
#endif




int x5b_app_local_mac_address_get(u_int8 *address)
{
	int ret = 0;
	struct interface *ifp = if_lookup_by_name("ethernet 0/0/2");
	if(ifp)
	{
		ret = nsm_interface_mac_get_api(ifp, address, 6);
		return  ret;
	}
	return ERROR;
}

int x5b_app_local_address_get(u_int32 *address)
{
	int ret = 0;
	struct interface *ifp = if_lookup_by_name("ethernet 0/0/2");
	if(ifp)
	{
		struct prefix p_address;
		ret = nsm_interface_address_get_api(ifp, &p_address);
		if(ret == OK)
		{
			if(address)
				*address = ntohl(p_address.u.prefix4.s_addr);
		}
		return  ret;
	}
	return ERROR;
}
#ifdef PL_OPENWRT_UCI
static int x5b_app_local_address_set(char *address, u_int32 mask)
{
	int ret = 0;
	struct prefix cp;
	struct interface *ifp = if_lookup_by_name("ethernet 0/0/2");
	if(ifp && address)
	{
		struct in_addr netmask;
		ret = str2prefix_ipv4 (address, (struct prefix_ipv4 *)&cp);
		if (ret <= 0)
		{
			return ERROR;
		}

		netmask.s_addr = htonl(mask);
		cp.prefixlen = ip_masklen(netmask);
		if(mask == 0)
			cp.prefixlen = 24;
#ifdef PL_PJSIP_MODULE

#ifdef PL_PJSIP_MODULE
		pl_pjsip_source_interface_set_api(ifp->ifindex);
#endif
#endif
		ret = nsm_interface_address_set_api(ifp, &cp, FALSE);
		return  ret;
	}
	return ERROR;
}
#endif


int x5b_app_factory_set(x5b_app_factory_t *data)
{
	//data->local_address;			//����IP��ַ��IP/DHCP��
	int ret = 0;
#ifdef PL_OPENWRT_UCI
	zassert(data != NULL);
	/*
	option proto "static"
	option ipaddr "192.168.3.100"
	option netmask "255.255.255.0"
	option gateway "192.168.3.1"
	option dns "192.168.3.1"
	*/
	if(data->local_address)
	{
		os_uci_set_string("network.wan.proto", "static");

		os_uci_set_string("network.wan.ipaddr", inet_address(ntohl(data->local_address)));

		if(data->local_netmask)
			os_uci_set_string("network.wan.netmask", inet_address(ntohl(data->local_netmask)));
		if(data->local_gateway)
			os_uci_set_string("network.wan.gateway", inet_address(ntohl(data->local_gateway)));
		if(data->local_dns)
			os_uci_set_string("network.wan.dns", inet_address(ntohl(data->local_dns)));

		os_uci_save_config("network");
		ret |= x5b_app_local_address_set(inet_address(ntohl(data->local_address)), ntohl(data->local_netmask));
	}
	else
	{
		os_uci_set_string("network.wan.proto", "dhcp");
		os_uci_save_config("network");
	}
#endif
	return ret;
}



int x5b_app_rtc_tm_set(int timesp)
{
    struct timespec sntpTime;	/* storage for retrieved time value */
	int	local_timesp = 0, value = 0;
	sntpTime.tv_sec = sntpTime.tv_nsec = rand();
	//zone = x5b_app_timezone_offset_api(NULL);
/*	if(zone < 0)
	{
		if(X5_B_ESP32_DEBUG(TIME))
			zlog_debug(ZLOG_APP, "get local system timezone:%d", zone);
	    sntpTime.tv_sec = timesp - OS_SEC_HOU_V(abs(zone));
	}
	else
	{
		if(X5_B_ESP32_DEBUG(TIME))
			zlog_debug(ZLOG_APP, "get local system timezone:%d", zone);
		sntpTime.tv_sec = timesp + OS_SEC_HOU_V((zone));
	}*/
	local_timesp = os_time(NULL);
/*	if(timesp < zone)
	{
		char timespstr[128];
		memset(timespstr, 0, sizeof(timespstr));
		snprintf(timespstr, sizeof(timespstr), "%s", os_time_fmt("/", timesp));
		zlog_debug(ZLOG_APP, "remote time less than local(:%d < %d) (%s < %s)",
				   timesp, zone,
				   timespstr, os_time_fmt("/", zone));
		return OK;
	}*/
	//sntpTime.tv_sec = zone;
	if(X5_B_ESP32_DEBUG(TIME))
	{
		char timespstr[128];
		memset(timespstr, 0, sizeof(timespstr));
		snprintf(timespstr, sizeof(timespstr), "%s", os_time_fmt("/", timesp));
		zlog_debug(ZLOG_APP, "set sync system time:%d set realtime:%d (%s--%s)",
				   timesp, local_timesp,
				   timespstr, os_time_fmt("/", local_timesp));
	}

	value = timesp - local_timesp;
	if(abs(value) <= 2)
	{
#ifdef X5B_APP_TIMESYNC_C
		if(!x5b_app_mode_X5CM())
#endif
			super_system("/etc/init.d/sysntpd restart");
		return OK;
	}
#ifdef PL_BUILD_X86
	return OK;
#endif
	sntpTime.tv_sec = timesp;
	//zlog_debug(ZLOG_APP, "===========get stm32 times:->%d realtime=%d", timesp, sntpTime.tv_sec);
	value = 5;
	while(value)
	{
		errno = 0;
		if(clock_settime(CLOCK_REALTIME, &sntpTime)!= 0)//SET SYSTEM LOCAL TIME
		{
			if(X5_B_ESP32_DEBUG(TIME))
				zlog_err(ZLOG_APP, "set system realtime by clock_settime is error:%s", strerror(errno));
			value--;
		}
		else
		{
			if(X5_B_ESP32_DEBUG(TIME))
				zlog_debug(ZLOG_APP, "set system realtime by clock_settime is success:%s", strerror(errno));
			break;
		}
	}
	if(value > 0)
	{
		sync();
		//os_sleep(1);
		//super_system("/etc/init.d/sysntpd restart");
		return OK;
	}
	else
		return ERROR;
}


/*
 * call
 */

int x5b_app_start_call(BOOL start, x5b_app_call_t *call)
{
	if(call /*&& strlen(call->data) >= 4*/)
	{
#ifdef PL_PJSIP_MODULE

#ifdef PL_PJSIP_MODULE
		voip_event_t event;
		memset(&event, 0, sizeof(voip_event_t));
		memcpy(event.data, call, sizeof(x5b_app_call_t));
		event.dlen = sizeof(x5b_app_call_t);
		if(start)
			voip_app_start_call_event_ui(&event);
		else
			voip_app_stop_call_event_ui(&event);
#endif
#endif
		return OK;
	}
	return ERROR;
}

int x5b_app_start_call_phone(BOOL start, char *call)
{
	if(call /*&& strlen(call->data) >= 4*/)
	{
#ifdef PL_PJSIP_MODULE

#ifdef PL_PJSIP_MODULE
		voip_event_t event;
		memset(&event, 0, sizeof(voip_event_t));
		memcpy(event.data, call, strlen(call));
		event.dlen = strlen(call);
		if(start)
			voip_app_start_call_event_ui_phone(&event);
		else
			voip_app_stop_call_event_ui(&event);
#endif
#endif
		return OK;
	}
	return ERROR;
}

int x5b_app_start_call_user(BOOL start, char *call)
{
	if(call /*&& strlen(call->data) >= 4*/)
	{
#ifdef PL_PJSIP_MODULE

#ifdef PL_PJSIP_MODULE
		voip_event_t event;
		memset(&event, 0, sizeof(voip_event_t));
		memcpy(event.data, call, strlen(call));
		event.dlen = strlen(call);
		if(start)
			voip_app_start_call_event_ui_user(&event);
		else
			voip_app_stop_call_event_ui(&event);
#endif
#endif
		return OK;
	}
	return ERROR;
}

int x5b_app_stop_call(BOOL start, x5b_app_call_t *call)
{
	if(1/* call && strlen(call->data) >= 4*/)
	{
#ifdef PL_PJSIP_MODULE

#ifdef PL_PJSIP_MODULE
		voip_app_stop_call_event_ui(NULL);
#endif
#endif
		return OK;
	}
	return ERROR;
}


static int x5b_route_lookup_default_one(struct route_node *rn, struct rib *rib, ifindex_t ifindex, union g_addr *gate)
{
	struct nexthop *nexthop, *tnexthop;
	int recursing = 0;
	/* Nexthop information. */
	for (ALL_NEXTHOPS_RO(rib->nexthop, nexthop, tnexthop, recursing))
	{
		if (nexthop == rib->nexthop)
		{
			if ( CHECK_FLAG(rib->flags, ZEBRA_FLAG_SELECTED) &&
				 CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB) &&
				 CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE) &&
				 (rib->type == ZEBRA_ROUTE_SYSTEM ||
					rib->type == ZEBRA_ROUTE_KERNEL ||
					rib->type == ZEBRA_ROUTE_DHCP ||
					rib->type == ZEBRA_ROUTE_STATIC) )
			{
				if (nexthop->ifindex == ifindex && rn->p.prefixlen == 0 &&
						nexthop->gate.ipv4.s_addr)
				{
					if (gate)
						memcpy (gate, &nexthop->gate, sizeof(union g_addr));
					return OK;
				}
			}

		}
	}
	return ERROR;
}

static int x5b_route_lookup_default(ifindex_t ifindex, u_int32 *local_gateway)
{
	struct route_table *table;
	struct route_node *rn;
	struct rib *rib;
	union g_addr gate;
	table = nsm_vrf_table (AFI_IP, SAFI_UNICAST, 0);
	if (!table)
		return ERROR;
	for (rn = route_top (table); rn; rn = route_next (rn))
		RNODE_FOREACH_RIB (rn, rib)
		{
			if(rn && rib)
			{
				if(x5b_route_lookup_default_one(rn, rib, ifindex, &gate) == OK)
				{
					if(local_gateway)
						*local_gateway = ntohl(gate.ipv4.s_addr);
					return OK;
				}
			}
		}
	return ERROR;
}

static int x5b_route_lookup_dns(u_int32 *local_dns)
{
	FILE *f = NULL;
	char buf[512];
	f = fopen("/tmp/resolv.conf.auto", "r");
	if (f)
	{
		memset(buf, 0, sizeof(buf));
		while (fgets(buf, sizeof(buf), f))
		{
			if(strstr(buf, "nameserver"))
			{
				char *p = buf + strlen("nameserver") + 1;
				if(inet_addr(p) != inet_addr("127.0.0.1"))
				{
					if(local_dns)
						*local_dns = inet_addr(p);
					fclose(f);
					return OK;
				}
			}
			memset(buf, 0, sizeof(buf));
		}
		fclose(f);
	}
	return ERROR;
}


int x5b_app_local_network_info_get(x5b_app_netinfo_t *info)
{
	int ret = 0;
	struct interface *ifp = if_lookup_by_name("ethernet 0/0/2");
	if(ifp)
	{
		struct prefix address;
		struct in_addr netmask;
		ret = nsm_interface_address_get_api(ifp, &address);
		if(ret == OK && info)
		{
			info->local_address = ntohl(address.u.prefix4.s_addr);
			masklen2ip (address.prefixlen, &netmask);
			info->local_netmask = ntohl(netmask.s_addr);
			x5b_route_lookup_default(ifp->ifindex, &info->local_gateway);
			x5b_route_lookup_dns(&info->local_dns);
			if(X5_B_ESP32_DEBUG(STATE))
				zlog_debug(ZLOG_APP, "get local network information address=%x  netmask=%x  gateway=%x  dns=%x",
						   info->local_address,info->local_netmask, info->local_gateway, info->local_dns);
		}
		return  ret;
	}
	return ERROR;
}




int x5b_app_local_register_info_get(x5b_app_phone_register_ack_t *info)
{
#ifdef PL_PJSIP_MODULE

#ifdef PL_PJSIP_MODULE
	if(pl_pjsip && info)
	{
		info->iface = 0;
		info->l_port = htons(pl_pjsip->sip_local.sip_port);
		info->sip_address = htonl(inet_addr(pl_pjsip->sip_server.sip_address));
		info->sip_port = htons(pl_pjsip->sip_server.sip_port);
		info->proxy_address = htonl(inet_addr(pl_pjsip->sip_proxy.sip_address));
		info->proxy_port = htons(pl_pjsip->sip_proxy.sip_port);
		info->proto = pl_pjsip->proto;
		info->dtmf		= pl_pjsip->dtmf - 1;
		info->codec = pl_pjsip->sip_codec.payload;
		return OK;
	}
#endif
#endif
	return ERROR;
}

int x5b_app_call_room_param_get(void *data, u_int8 *building,
		u_int8 *unit, u_int16 *room)
{
	x5b_app_call_t *input = (x5b_app_call_t *)data;
	zassert(data != NULL);
	//zassert(phonelist != NULL);

	if(building)
		*building = input->building;

	if(unit)
		*unit = input->unit;

	if(room)
		*room = input->room_number;

/*	if(voip_dbase_get_room_phone(input->building, input->unit, input->room_number, phonelist) <= 0)
	{
		zlog_err(ZLOG_APP,
					"Can not get Phone Number by Room");
		return ERROR;
	}*/
	return OK;
}

/*
 *
 *
 *
 */
#ifdef PL_BUILD_OPENWRT
/*
 * swconfig dev switch0 port 2 show | grep link
 */
int x5b_app_network_port_status_get(x5b_app_mgt_t *app)
{
	char buf[512];
	struct wan_state_s wan_state;
	zassert(app != NULL);
	//app->wan_state.t_thread = NULL;

	wan_state.link_phy = E_CMD_NETWORK_STATE_PHY_DOWN;
	wan_state.address = get_hostip_byname("www.baidu.com");

	memset(buf, 0, sizeof(buf));

	if(super_output_system("swconfig dev switch0 port 0 show | grep link", buf, sizeof(buf)) == OK)
	{
		if(strlen(buf))
		{
			if(strstr(buf, "link:up"))
			{
				if(app->wan_state.link_phy == E_CMD_NETWORK_STATE_PHY_DOWN)
				{
					app->wan_state.link_phy = E_CMD_NETWORK_STATE_PHY_UP;
					//x5b_app_network_port_status_api(app, E_CMD_NETWORK_STATE_PHY_UP, E_CMD_TO_AUTO);
				}
			}
			else if(strstr(buf, "link:down"))
			{
				if(app->wan_state.link_phy == E_CMD_NETWORK_STATE_PHY_UP)
				{
					app->wan_state.link_phy = E_CMD_NETWORK_STATE_PHY_DOWN;
					//x5b_app_network_port_status_api(app, E_CMD_NETWORK_STATE_PHY_DOWN, E_CMD_TO_AUTO);
					//app->wan_state.t_thread = eloop_add_timer(app->master, x5b_app_network_port_statuc_event, app, app->wan_state.interval);
					return OK;
				}
			}
		}
	}

	if(wan_state.address != ERROR && wan_state.address > 0)
	{
		// net link up
		if(app->wan_state.address == 0)
		{
			app->wan_state.address = wan_state.address;
			//x5b_app_network_port_status_api(app, E_CMD_NETWORK_STATE_UP, E_CMD_TO_AUTO);
		}
	}
	if(wan_state.address == ERROR || wan_state.address == 0)
	{
		// net link down
		if(app->wan_state.address != 0)
		{
			//x5b_app_network_port_status_api(app, E_CMD_NETWORK_STATE_DOWN, E_CMD_TO_AUTO);
			app->wan_state.address = 0;
		}
	}
	//app->wan_state.t_thread = eloop_add_timer(app->master, x5b_app_network_port_statuc_event, app, app->wan_state.interval);
	return OK;
}

static int x5b_app_network_port_status_event(struct eloop *eloop)
{
	zassert(eloop != NULL);
	x5b_app_mgt_t *app = ELOOP_ARG(eloop);
	char buf[512];
	struct wan_state_s wan_state;

	app->wan_state.t_thread = NULL;

	wan_state.link_phy = E_CMD_NETWORK_STATE_PHY_DOWN;
	wan_state.address = get_hostip_byname("www.baidu.com");

	memset(buf, 0, sizeof(buf));

	if(super_output_system("swconfig dev switch0 port 0 show | grep link", buf, sizeof(buf)) == OK)
	{
		if(strlen(buf))
		{
			if(strstr(buf, "link:up"))
			{
				if(app->wan_state.link_phy == E_CMD_NETWORK_STATE_PHY_DOWN)
				{
					app->wan_state.link_phy = E_CMD_NETWORK_STATE_PHY_UP;
					x5b_app_network_port_status_api(app, E_CMD_NETWORK_STATE_PHY_UP, E_CMD_TO_AUTO);
				}
			}
			else if(strstr(buf, "link:down"))
			{
				if(app->wan_state.link_phy == E_CMD_NETWORK_STATE_PHY_UP)
				{
					voip_app_sip_register_start(FALSE);
					app->wan_state.link_phy = E_CMD_NETWORK_STATE_PHY_DOWN;
					x5b_app_network_port_status_api(app, E_CMD_NETWORK_STATE_PHY_DOWN, E_CMD_TO_AUTO);
					app->wan_state.t_thread = eloop_add_timer(app->master, x5b_app_network_port_status_event, app, app->wan_state.interval);
					return OK;
				}
			}
		}
	}

	if(wan_state.address != ERROR && wan_state.address > 0)
	{
		// net link up
		if(app->wan_state.address == 0)
		{
			app->wan_state.address = wan_state.address;
			x5b_app_network_port_status_api(app, E_CMD_NETWORK_STATE_UP, E_CMD_TO_AUTO);
			voip_app_sip_register_start(TRUE);
		}
	}
	if(wan_state.address == ERROR || wan_state.address == 0)
	{
		// net link down
		if(app->wan_state.address != 0)
		{
			voip_app_sip_register_start(FALSE);
			x5b_app_network_port_status_api(app, E_CMD_NETWORK_STATE_DOWN, E_CMD_TO_AUTO);
			app->wan_state.address = 0;
		}
	}
	app->wan_state.t_thread = eloop_add_timer(app->master, x5b_app_network_port_status_event, app, app->wan_state.interval);
	if(app->mutex)
		os_mutex_lock(app->mutex, OS_WAIT_FOREVER);
	if(!app->upgrade && app->r_thread == NULL)
		x5b_app_event_active(app, X5B_READ_EV, 0, 0);
		//X5B_APP_MODULE_ID_C
		//x5b_app_read_eloop_reload(app);
	if(app->mutex)
		os_mutex_unlock(app->mutex);
	//zlog_debug(ZLOG_APP, "=================x5b_app_network_port_status_event -> x5b_app_read_eloop_reload");
	return OK;
/*	E_CMD_NETWORK_STATE_PHY_DOWN,
	E_CMD_NETWORK_STATE_PHY_UP,
	E_CMD_NETWORK_STATE_DOWN,
	E_CMD_NETWORK_STATE_UP,*/
}
#endif

BOOL x5b_app_port_status_get()
{
	if(x5b_app_mgt)
		return (x5b_app_mgt->wan_state.link_phy == E_CMD_NETWORK_STATE_PHY_UP) ? TRUE:FALSE;
	return FALSE;
}

int x5b_app_network_event_init(x5b_app_mgt_t *app)
{
	zassert(app != NULL);
	if(app->wan_state.interval == 0)
	{
		memset(&app->wan_state, 0, sizeof(app->wan_state));
		app->wan_state.interval = 2;
	}
	if(app->wan_state.t_thread)
		eloop_cancel(app->wan_state.t_thread);
#ifdef PL_BUILD_OPENWRT
	if(app->master)
		app->wan_state.t_thread = eloop_add_timer(app->master, x5b_app_network_port_status_event, app, app->wan_state.interval);
#endif
	return OK;
}
int x5b_app_network_event_exit(x5b_app_mgt_t *app)
{
	zassert(app != NULL);
	if(app->wan_state.t_thread)
		eloop_cancel(app->wan_state.t_thread);
	app->wan_state.t_thread = NULL;
	memset(&app->wan_state, 0, sizeof(app->wan_state));
	return OK;
}













#ifdef X5B_APP_IO_LOG
/**********************************************/
/*
 *
 *
 *
 */

typedef struct
{
    u_int8 effective;
    u_int8 logType;
    u_int8 openType;
    u_int8 openResult;
    u_int32 openTime;
    u_int32 eraseCnt;
    u_int32 cnt;
}openLogHeardType;


typedef struct
{
    openLogHeardType    hdr;
    u_int8             data[48];
}openLogType;

typedef struct
{
	u_int8 cardIssueType;
	u_int8 cardType;
	u_int8 cardNumber[8];
	u_int8 cardNumberLength;
}openLogUseCardDataType;


typedef enum
{
	UNKNOW_CARD,
	TAG_FUKAI_CARD,
    TAG_CQ_BUS_CARD,
	TAG_TERMINUS_CARD,
	TAG_ID_CARD,
    TAG_PHONE_CARD = 6,
    TAG_BAND_CARD =7,
    TAG_DESFIRE_CARD = 8,

}tagTypeEnum;


typedef enum
{
    OPENLOG_FORM_KEY,
    OPENLOG_FORM_CARD,
    OPENLOG_FORM_REMOTE,
    OPENLOG_FORM_PWD,
    OPENLOG_FORM_USER_PWD,
    OPENLOG_FORM_QRCODE,
    OPENLOG_FORM_DTMF
}openLogTypeEnum;

typedef enum
{
	OPEN_FOR_CARD,
	OPEN_FOR_ONCE_PWD,
	OPEN_FOR_APP_REMOTE,
	OPEN_FOR_IN_DOOR,
	OPEN_FOR_FACE,
	OPEN_FOR_FINGER,
    OPEN_FOR_PWD,
    OPEN_FOR_DTMF,
    OPEN_FOR_PHONE_BT,
    OPEN_FOR_QRCODE,
    OPEN_FOR_BAND,
    OPEN_FOR_USER_PWD,
}openDoorTypeEnum;

typedef enum
{
    INIT_TAG = 0,
    ID_TAG,
    USER_TAG,
    MANAGENT_TAG,
	TEMP_TAG ,
    APP_TAG = 6,
    CONFIG_TAG = 8,
    UINT_ADMIN_TAG,
    UINT_MANAGENT_TAG
}tagPowerEnum;

#define SEC_DAY_V		(60)*(60)*(8)

int x5b_app_a_thlog_log(char *format)
{
/*
	char data_tmp[128];
	int len = 0;
	struct tm stm;
	os_memset(data_tmp, 0, sizeof(data_tmp));
	time_t openTime = 0;
*/
	//face_card_t *card = NULL;
	user_face_card_t  *card = NULL;
	int i = 0, offset = 0;
	char cardNumber[128];
	char *type = NULL, *result = NULL, *card_type = NULL, *card_level = NULL;
	openLogType *a_log = (openLogType *)format;
	openLogUseCardDataType *card_data = a_log->data;
	a_log->hdr.openTime = ntohl(a_log->hdr.openTime);
/*
	openTime = (time_t)a_log->hdr.openTime + SEC_DAY_V;

	memset(&stm, 0, sizeof(struct tm));
	localtime_r(&openTime, &stm);
	len = strftime(data_tmp, sizeof(data_tmp), "%Y/%m/%d %H:%M:%S", &stm);
	zlog_debug(ZLOG_APP, "openTime=0X%x -> %d :%s", a_log->hdr.openTime, openTime, data_tmp);
*/

	a_log->hdr.eraseCnt = ntohl(a_log->hdr.eraseCnt);
	a_log->hdr.cnt = ntohl(a_log->hdr.cnt);
	if(a_log->hdr.openResult == 0)
		result = "OK";
	else
		result = "FAIL";

	memset(cardNumber, 0, sizeof(cardNumber));
	for(i = 0; i < card_data->cardNumberLength; i++)
	{
		sprintf(cardNumber + offset, "%02x", card_data->cardNumber[i]);
		offset+=2;
	}

	//if(x5b_app_mgt->make_card/* || x5b_app_mgt->make_card && !x5b_app_mgt->make_edit*/)
	{
#ifdef PL_OPENWRT_UCI
		os_uci_set_string("facecard.db.cardid", cardNumber);
		os_uci_save_config("facecard");
		os_uci_set_string("userauth.db.cardid", cardNumber);
		os_uci_save_config("userauth");
#endif
		//return OK;
	}

/*	switch(a_log->hdr.logType)
	{

	}*/
	switch(a_log->hdr.openType)
	{
	case OPEN_FOR_CARD:
		type = "CARD";
		break;
	case OPEN_FOR_ONCE_PWD:
		type = "ONCE PWD";
		break;
	case OPEN_FOR_APP_REMOTE:
		type = "APP";
		break;
	case OPEN_FOR_IN_DOOR:
		type = "IN DOOR";
		break;
	case OPEN_FOR_FACE:
		type = "FACE";
		break;
	case OPEN_FOR_FINGER:
		type = "FINGER";
		break;
	case OPEN_FOR_PWD:
		type = "PWD";
		break;
	case OPEN_FOR_DTMF:
		type = "DTMF";
		break;
	case OPEN_FOR_PHONE_BT:
		type = "PHONE BT";
		break;
	case OPEN_FOR_QRCODE:
		type = "QRCODE";
		break;
	case OPEN_FOR_BAND:
		type = "BAND";
		break;
	case OPEN_FOR_USER_PWD:
		type = "USER PWD";
		break;
	default:
		type = "Unknown";
		break;
	}

	switch(card_data->cardType)
	{
	case TAG_FUKAI_CARD:
		card_type = "FUKAI";
		break;
	case TAG_CQ_BUS_CARD:
		card_type = "BUS";
		break;
	case TAG_TERMINUS_CARD:
		card_type = "TERMINUS";
		break;
	case TAG_ID_CARD:
		card_type = "ID";
		break;
	case TAG_PHONE_CARD:
		card_type = "PHONE";
		break;
	case TAG_BAND_CARD:
		card_type = "BAND";
		break;
	case TAG_DESFIRE_CARD:
		card_type = "DESFIRE";
		break;
	case UNKNOW_CARD:
	default:
		card_type = "Unknown";
		break;
	}
	switch(card_data->cardIssueType)
	{
	case INIT_TAG:
		card_level = "INIT TAG";
		break;
	case ID_TAG:
		card_level = "ID TAG";
		break;
	case USER_TAG:
		card_level = "USER TAG";
		break;
	case MANAGENT_TAG:
		card_level = "MANAGENT TAG";
		break;
	case TEMP_TAG:
		card_level = "TEMP TAG";
		break;
	case APP_TAG:
		card_level = "APP TAG";
		break;
	case CONFIG_TAG:
		card_level = "CONFIG TAG";
		break;
	case UINT_ADMIN_TAG:
		card_level = "UINT ADMIN TAG";
		break;
	case UINT_MANAGENT_TAG:
		card_level = "UINT MANAGENT TAG";
		break;
	default:
		card_level = "Unknown";
		break;
	}

	card =  x5b_user_lookup_by_cardid(cardNumber);
	//card =  voip_card_node_lookup_by_cardid(cardNumber);
	if(card)
	{
		if(X5_B_ESP32_DEBUG(MSG))
			zlog_debug(ZLOG_APP, "type=%s result=%s ID:%s username=%s userid=%s CARD-TYPE:%s Privilege:%s",
				   type, result, cardNumber, card->username, card->userid, card_type, card_level);
	}
	else
	{
		if(X5_B_ESP32_DEBUG(MSG))
			zlog_debug(ZLOG_APP, "type=%s result=%s ID:%s username=Unknown userid=Unknown CARD-TYPE:%s Privilege:%s",
				   type, result, cardNumber, card_type, card_level);
	}
	return OK;

	//zlog_debug(ZLOG_APP, "=======================================");
	if(card)
	{
		voip_thlog_log4(a_log->hdr.openTime + SEC_DAY_V, type, result,
				"%s username=%s userid=%s CARD-TYPE:%s Privilege:%s",
				cardNumber, card->username, card->userid, card_type, card_level);
		zlog_debug(ZLOG_APP, "type=%s result=%s ID:%s username=%s userid=%s CARD-TYPE:%s Privilege:%s",
				   type, result, cardNumber, card->username, card->userid, card_type, card_level);
	}
	else
	{
		voip_thlog_log4(a_log->hdr.openTime + SEC_DAY_V, type, result,
				"%s %s username=Unknown userid=Unknown CARD-TYPE:%s Privilege:%s",
			cardNumber, card_type, card_level);
		zlog_debug(ZLOG_APP, "type=%s result=%s ID:%s username=Unknown userid=Unknown CARD-TYPE:%s Privilege:%s",
				   type, result, cardNumber, card_type, card_level);
	}
	return OK;
}
#endif

int x5b_app_c_log_card(char *format)
{
	char cardNumber[128];
	char *brk = NULL, str = format;
	brk = strstr(str, "ID:");
	if(!brk)
		return ERROR;
	brk += 3;
	str = brk;
	brk = strstr(str, " ");
	if(!brk)
		return ERROR;
	if(brk && str && brk > str)
	{
		memset(cardNumber, 0, sizeof(cardNumber));
		memcpy(cardNumber, str, brk-str);
#ifdef PL_OPENWRT_UCI
		os_uci_set_string("facecard.db.cardid", cardNumber);
		os_uci_save_config("facecard");
		os_uci_set_string("userauth.db.cardid", cardNumber);
		os_uci_save_config("userauth");
#endif
	}
	return OK;
}

