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
#include "vrf.h"
#include "interface.h"
#include "eloop.h"
#include "cJSON.h"

#include "nexthop.h"
#include "rib.h"
/*
#include "redistribute.h"
#include "debug.h"
#include "routemap.h"
#include "zebra_fpm.h"
#include "zebra_rnh.h"

#include "zserv.h"

*/

#include "x5_b_app.h"
#include "x5_b_cmd.h"
#include "x5_b_ctl.h"
#include "x5_b_util.h"
#include "web_x5b.h"
#include "x5b_facecard.h"
#ifdef PL_VOIP_MODULE
#include "voip_def.h"
#endif


extern int os_write_file(const char *name, const char *string, int len);
extern int os_read_file(const char *name, const char *string, int len);

typedef struct x5b_app_mgt_lease_s
{
	u_int32		a_address;
	u_int32		c_address;
	u_int16		a_state;
	u_int16		c_state;
#define X5B_APP_AC_LEASE SYSCONFDIR"/.x5app-ac.lease"
}x5b_app_mgt_lease_t;

int x5b_app_AC_state_load(x5b_app_mgt_t *mgt)
{
	x5b_app_mgt_lease_t lease;
	zassert(mgt != NULL);
	if(os_read_file(X5B_APP_AC_LEASE, &lease, sizeof(x5b_app_mgt_lease_t)) == OK)
	{
		mgt->app_a.address = lease.a_address;
		mgt->app_c.address = lease.c_address;
		mgt->app_a.reg_state = lease.a_state;
		mgt->app_c.reg_state = lease.c_state;
	}
	return OK;
}

int x5b_app_AC_state_save(x5b_app_mgt_t *mgt)
{
	x5b_app_mgt_lease_t lease;
	zassert(mgt != NULL);
	lease.a_address = mgt->app_a.address;
	lease.c_address = mgt->app_c.address;
	lease.a_state = mgt->app_a.reg_state;
	lease.c_state = mgt->app_c.reg_state;
	os_write_file(X5B_APP_AC_LEASE, &lease, sizeof(x5b_app_mgt_lease_t));
	return OK;
}


int inet64_to_mac(u_int64 value, u_int8 *dst)
{
	unsigned i;
	zassert(dst != NULL);
	u_int64 temp = value;
	for (i = 0; i < 8; i++) {
		dst[7-i] = temp & 0xff;
		temp >>= 8;
	}
	return OK;
}

u_int64 mac_to_inet64(u_int8 *dst)
{
	unsigned i;
	u_int64 temp = 0;
	zassert(dst != NULL);
	for (i = 0; i < 8; i++) {
		temp |= dst[i] & 0xff;
		temp <<= 8;
	}
	return temp;
}



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
#ifdef PL_VOIP_MODULE
#ifdef PL_OSIP_MODULE
		voip_sip_source_interface_set_api(ifp->ifindex);
#endif
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
    struct timespec getTime;
	int	zone = 0;
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
	sntpTime.tv_sec = timesp;
	if(X5_B_ESP32_DEBUG(TIME))
	{
		char timespstr[128];
		memset(timespstr, 0, sizeof(timespstr));
		snprintf(timespstr, sizeof(timespstr), "%s", os_time_fmt("/", timesp));
		zlog_debug(ZLOG_APP, "set sync system time:%d set realtime:%d (%s--%s)",
				   timesp, sntpTime.tv_sec,
				   timespstr, os_time_fmt("/", sntpTime.tv_sec));
	}
	if(clock_gettime(CLOCK_REALTIME, &getTime)== 0)
	{
		if(abs(getTime.tv_sec - sntpTime.tv_sec) <= 2)
		{
			super_system("/etc/init.d/sysntpd restart");
			return OK;
		}
	}
	//zlog_debug(ZLOG_APP, "===========get stm32 times:->%d realtime=%d", timesp, sntpTime.tv_sec);
	zone = 5;
	while(zone)
	{
		errno = 0;
		if(clock_settime(CLOCK_REALTIME, &sntpTime)!= 0)//SET SYSTEM LOCAL TIME
		{
			zlog_err(ZLOG_APP, "set system realtime by clock_settime is error:%s", strerror(errno));
			zone--;
		}
		else
		{
			zlog_err(ZLOG_APP, "set system realtime by clock_settime is success:%s", strerror(errno));
			break;
		}
	}
	if(zone)
	{
		sync();
		//os_sleep(1);
		super_system("/etc/init.d/sysntpd restart");
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
#ifdef PL_VOIP_MODULE
#ifdef PL_OSIP_MODULE
		voip_event_ready_add(start ? voip_app_start_call_event_ui:voip_app_stop_call_event_ui,
				NULL, call, sizeof(x5b_app_call_t), 0);
#endif
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
#ifdef PL_VOIP_MODULE
#ifdef PL_OSIP_MODULE
		voip_event_ready_add(start ? voip_app_start_call_event_ui_phone:voip_app_stop_call_event_ui,
				NULL, call, strlen(call), 0);
#endif
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

int x5b_app_stop_call(BOOL start, x5b_app_call_t *call)
{
	if(1/* call && strlen(call->data) >= 4*/)
	{
#ifdef PL_VOIP_MODULE
#ifdef PL_OSIP_MODULE
	voip_event_ready_add(voip_app_stop_call_event_ui, NULL, NULL, 0, 0);
#endif
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
				 CHECK_FLAG (rib->flags, NEXTHOP_FLAG_FIB) &&
				 rib->type == ZEBRA_ROUTE_DHCP)
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
/*
	u_int32 local_address;			//MUST: local IP address/DHCP(0.0.0.0)
	u_int32 local_netmask;
	u_int32 local_gateway;
	u_int32 local_dns;
*/
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
#ifdef PL_VOIP_MODULE
#ifdef PL_OSIP_MODULE
	if(sip_config && info)
	{
		info->iface = 0;
		info->l_port = htons(sip_config->sip_local_port);
		info->sip_address = htonl(sip_config->sip_server);
		info->sip_port = htons(sip_config->sip_port);
		info->proxy_address = htonl(sip_config->sip_proxy_server);
		info->proxy_port = htons(sip_config->sip_proxy_port);
		info->proto = sip_config->proto;
		info->dtmf		= sip_config->dtmf - 1;
		info->codec = sip_config->payload;
		return OK;
	}
#endif
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
		info->codec = pl_pjsip->payload;
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
#ifdef BUILD_OPENWRT
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
		}
	}
	if(wan_state.address == ERROR || wan_state.address == 0)
	{
		// net link down
		if(app->wan_state.address != 0)
		{
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
#ifdef BUILD_OPENWRT
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

struct time_zone
{
	char *time_zone;
	int	offset;
}time_zone_tbl[] =
{
	{ "UTC", 0 },
	{ "Africa/Abidjan", 0 },
	{ "Africa/Accra", 0 },
	{ "Africa/Addis Ababa", -3 },
	{ "Africa/Algiers", -1 },
	{ "Africa/Asmara", -3 },
	{ "Africa/Bamako", 0 },
	{ "Africa/Bangui", -1 },
	{ "Africa/Banjul", 0 },
	{ "Africa/Bissau", 0 },
	{ "Africa/Blantyre", -2 },
	{ "Africa/Brazzaville", -1 },
	{ "Africa/Bujumbura", -2 },
		{ "Africa/Cairo", -2 },
		{ "Africa/Ceuta", -2 },
		{ "Africa/Conakry", 0 },
		{ "Africa/Dakar", 0 },
		{ "Africa/Dar es Salaam", -3 },
		{ "Africa/Djibouti", 03 },
		{ "Africa/Douala", -2 },
		{ "Africa/Freetown", 0 },
		{ "Africa/Gaborone", -2 },
		{ "Africa/Harare", -2 },
		{ "Africa/Johannesburg", -2 },
		{ "Africa/Juba", -3 },
		{ "Africa/Kampala", -3 },
		{ "Africa/Khartoum", -2 },
		{ "Africa/Kigali", -2 },
		{ "Africa/Kinshasa", -1 },
		{ "Africa/Lagos", -1 },
		{ "Africa/Libreville", -1 },
		{ "Africa/Lome", 0 },
		{ "Africa/Luanda", -1 },
		{ "Africa/Lubumbashi", -2 },
		{ "Africa/Lusaka", -2 },
		{ "Africa/Malabo", -1 },
		{ "Africa/Maputo", -2 },
		{ "Africa/Maseru", -2 },
		{ "Africa/Mbabane", -2 },
		{ "Africa/Mogadishu", -3 },
		{ "Africa/Monrovia", 0 },
		{ "Africa/Nairobi", -3 },
		{ "Africa/Ndjamena", -1 },
		{ "Africa/Niamey", -1 },
		{ "Africa/Nouakchott", 0 },
		{ "Africa/Ouagadougou", 0 },
		{ "Africa/Porto-Novo", -1 },
		{ "Africa/Sao Tome", 0 },
		{ "Africa/Tripoli", -2 },
		{ "Africa/Tunis", -1 },
		{ "Africa/Windhoek", -2 },
		{ "America/Adak", 0 },
		{ "America/Anchorage", 0 },
		{ "America/Anguilla", 4 },
		{ "America/Antigua", 4 },
		{ "America/Araguaina", 3 },
		{ "America/Argentina/Buenos Aires", 3 },
		{ "America/Argentina/Catamarca", 3 },
		{ "America/Argentina/Cordoba", 3 },
		{ "America/Argentina/Jujuy", 3 },
		{ "America/Argentina/La Rioja", 3 },
		{ "America/Argentina/Mendoza", 3 },
		{ "America/Argentina/Rio Gallegos", 3 },
		{ "America/Argentina/Salta", 3 },
		{ "America/Argentina/San Juan", 3 },
		{ "America/Argentina/San Luis", 3 },
		{ "America/Argentina/Tucuman", 3 },
		{ "America/Argentina/Ushuaia", 3 },
		{ "America/Aruba", 4 },
		{ "America/Asuncion", 4 },
		{ "America/Atikokan", 5 },
		{ "America/Bahia", 3 },
		{ "America/Bahia Banderas", 5 },
		{ "America/Barbados", 4 },
		{ "America/Belem", 3 },
		{ "America/Belize", 6 },
		{ "America/Blanc-Sablon", 4 },
		{ "America/Boa Vista", 4 },
		{ "America/Bogota", 5 },
		{ "America/Boise", 3 },
		{ "America/Cambridge Bay", 3 },
		{ "America/Campo Grande", 1 },
		{ "America/Cancun", 5 },
		{ "America/Caracas", 4 },
		{ "America/Cayenne", 3 },
		{ "America/Cayman", 5 },
		{ "America/Chicago", 5 },
		{ "America/Chihuahua", 0 },
		{ "America/Costa Rica", 6 },
		{ "America/Creston", 7 },
		{ "America/Cuiaba", 4 },
		{ "America/Curacao", 4 },
		{ "America/Danmarkshavn", 0 },
		{ "America/Dawson", 3 },
		{ "America/Dawson Creek", 7 },
		{ "America/Denver", 6 },
		{ "America/Detroit", 4 },
		{ "America/Dominica", 4 },
		{ "America/Edmonton", 6 },
		{ "America/Eirunepe", 5 },
		{ "America/El Salvador", 6 },
		{ "America/Fort Nelson", 7 },
		{ "America/Fortaleza", 3 },
		{ "America/Glace Bay", 3 },
		{ "America/Godthab", 3 },
		{ "America/Goose Bay", 3 },
		{ "America/Grand Turk", 4 },
		{ "America/Grenada", 4 },
		{ "America/Guadeloupe", 4 },
		{ "America/Guatemala", 6 },
		{ "America/Guayaquil", 5 },
		{ "America/Guyana", 4 },
		{ "America/Halifax", 4 },
		{ "America/Havana", 5 },
		{ "America/Hermosillo", 7 },
		{ "America/Indiana/Indianapolis", 5 },
		{ "America/Indiana/Knox", 6 },
		{ "America/Indiana/Marengo", 5 },
		{ "America/Indiana/Petersburg", 5 },
		{ "America/Indiana/Tell City", 6 },
		{ "America/Indiana/Vevay", 5 },
		{ "America/Indiana/Vincennes", 5 },
		{ "America/Indiana/Winamac", 5 },
		{ "America/Inuvik", 7 },
		{ "America/Iqaluit", 5 },
		{ "America/Jamaica", 5 },
		{ "America/Juneau", 8 },
		{ "America/Kentucky/Louisville", 5 },
		{ "America/Kentucky/Monticello", 5 },
		{ "America/Kralendijk", 4 },
		{ "America/La Paz", 4 },
		{ "America/Lima", 5 },
		{ "America/Los Angeles", 7 },
		{ "America/Lower Princes", 4 },
		{ "America/Maceio", 3 },
		{ "America/Managua", 6 },
		{ "America/Manaus", 4 },
		{ "America/Marigot", 4 },
		{ "America/Martinique", 4 },
		{ "America/Matamoros", 6 },
		{ "America/Mazatlan", 7 },
		{ "America/Menominee", 6 },
		{ "America/Merida", 6 },
		{ "America/Metlakatla", 8 },
		{ "America/Mexico City", 6 },
		{ "America/Miquelon", 3 },
		{ "America/Moncton", 4 },
		{ "America/Monterrey", 6 },
		{ "America/Montevideo", 3 },
		{ "America/Montserrat", 4 },
		{ "America/Nassau", 5 },
		{ "America/New York", 5 },
		{ "America/Nipigon", 5 },
		{ "America/Nome", 8 },
		{ "America/Noronha", 2 },
		{ "America/North Dakota/Beulah", 6 },
		{ "America/North Dakota/Center", 6 },
		{ "America/North Dakota/New Salem", 6 },
		{ "America/Ojinaga", 6 },
		{ "America/Panama", 5 },
		{ "America/Pangnirtung", 5 },
		{ "America/Paramaribo", 3 },
		{ "America/Phoenix", 7 },
		{ "America/Port of Spain", 4 },
		{ "America/Port-au-Prince", 5 },
		{ "America/Porto Velho", 4 },
		{ "America/Puerto Rico", 4 },
		{ "America/Punta Arenas", 3 },
		{ "America/Rainy River", 6 },
		{ "America/Rankin Inlet", 6 },
		{ "America/Recife", 3 },
		{ "America/Regina", 6 },
		{ "America/Resolute", 6 },
		{ "America/Rio Branco", 5 },
		{ "America/Santarem", 3 },
		{ "America/Santiago", 4 },
		{ "America/Santo Domingo", 4 },
		{ "America/Sao Paulo", 3 },
		{ "America/Scoresbysund", 1 },
		{ "America/Sitka", 8 },
		{ "America/St Barthelemy", 4 },
		{ "America/St Johns", 3 },
		{ "America/St Kitts", 4 },
		{ "America/St Lucia", 4 },
		{ "America/St Thomas", 4 },
		{ "America/St Vincent", 4 },
		{ "America/Swift Current", 6 },
		{ "America/Tegucigalpa", 6 },
		{ "America/Thule", 4 },
		{ "America/Thunder Bay", 5 },
		{ "America/Tijuana", 7 },
		{ "America/Toronto", 5 },
		{ "America/Tortola", 4 },
		{ "America/Vancouver", 7 },
		{ "America/Whitehorse", 7 },
		{ "America/Winnipeg", 6 },
		{ "America/Yakutat", 8 },
		{ "America/Yellowknife", 6 },
		{ "Antarctica/Casey", 8 },
		{ "Antarctica/Davis", 7 },
		{ "Antarctica/DumontDUrville", 10 },
		{ "Antarctica/Macquarie",11 },
		{ "Antarctica/Mawson", 5 },
		{ "Antarctica/McMurdo", -12 },
		{ "Antarctica/Palmer", 3 },
		{ "Antarctica/Rothera", 3 },
		{ "Antarctica/Syowa", 3 },
		{ "Antarctica/Troll", -2 },
		{ "Antarctica/Vostok", -6 },
		{ "Arctic/Longyearbyen", -2 },
		{ "Asia/Aden", -3 },
		{ "Asia/Almaty", -6 },
		{ "Asia/Amman", -2 },
		{ "Asia/Anadyr", -12 },
		{ "Asia/Aqtau", -5 },
		{ "Asia/Aqtobe", -5 },
		{ "Asia/Ashgabat", -5 },
		{ "Asia/Atyrau", -5 },
		{ "Asia/Baghdad", -3 },
		{ "Asia/Bahrain", -3 },
		{ "Asia/Baku", -4 },
		{ "Asia/Bangkok", -7 },
		{ "Asia/Barnaul", -7 },
		{ "Asia/Beirut", -3 },
		{ "Asia/Bishkek", -6 },
		{ "Asia/Brunei", -8 },
		{ "Asia/Chita", -9 },
		{ "Asia/Choibalsan", -8 },
		{ "Asia/Colombo", -5 },
		{ "Asia/Damascus", -3 },
		{ "Asia/Dhaka", -6 },
		{ "Asia/Dili", -9 },
		{ "Asia/Dubai", -4 },
		{ "Asia/Dushanbe", -5 },
		{ "Asia/Famagusta", -3 },
		{ "Asia/Gaza", -3 },
		{ "Asia/Hebron", -3 },
		{ "Asia/Ho Chi Minh", -7 },
		{ "Asia/Hong Kong", -8 },
		{ "Asia/Hovd", -7 },
		{ "Asia/Irkutsk", -8 },
		{ "Asia/Jakarta", -7 },
		{ "Asia/Jayapura", -9 },
		{ "Asia/Jerusalem", 0 },
		{ "Asia/Kabul", -4 },
		{ "Asia/Kamchatka", -12 },
		{ "Asia/Karachi", -5 },
		{ "Asia/Kathmandu", -5 },
		{ "Asia/Khandyga", -9 },
		{ "Asia/Kolkata", -5 },
		{ "Asia/Krasnoyarsk", -7 },
		{ "Asia/Kuala Lumpur", -8 },
		{ "Asia/Kuching", -8 },
		{ "Asia/Kuwait", -3 },
		{ "Asia/Macau", -8 },
		{ "Asia/Magadan",-11 },
		{ "Asia/Makassar", -8 },
		{ "Asia/Manila", -8 },
		{ "Asia/Muscat", -4 },
		{ "Asia/Nicosia", -3 },
		{ "Asia/Novokuznetsk", -7 },
		{ "Asia/Novosibirsk", -7 },
		{ "Asia/Omsk", -6 },
		{ "Asia/Oral", -5 },
		{ "Asia/Phnom Penh", -7 },
		{ "Asia/Pontianak", -7 },
		{ "Asia/Pyongyang", -9 },
		{ "Asia/Qatar", -3 },
		{ "Asia/Qostanay", -6 },
		{ "Asia/Qyzylorda", -5 },
		{ "Asia/Riyadh", -3 },
		{ "Asia/Sakhalin",-11 },
		{ "Asia/Samarkand", -5 },
		{ "Asia/Seoul", -9 },
		{ "Asia/Shanghai", -8 },
		{ "Asia/Singapore", -8 },
		{ "Asia/Srednekolymsk",-11 },
		{ "Asia/Taipei", -8 },
		{ "Asia/Tashkent", -5 },
		{ "Asia/Tbilisi", -4 },
		{ "Asia/Tehran", -3 },
		{ "Asia/Thimphu", -6 },
		{ "Asia/Tokyo", -9 },
		{ "Asia/Tomsk", -7 },
		{ "Asia/Ulaanbaatar", -8 },
		{ "Asia/Urumqi", -6 },
		{ "Asia/Ust-Nera", -10 },
		{ "Asia/Vientiane", -7 },
		{ "Asia/Vladivostok", -10 },
		{ "Asia/Yakutsk", -9 },
		{ "Asia/Yangon", -6 },
		{ "Asia/Yekaterinburg", -5 },
		{ "Asia/Yerevan", -4 },
		{ "Atlantic/Azores", 0 },
		{ "Atlantic/Bermuda", 3 },
		{ "Atlantic/Canary", -1 },
		{ "Atlantic/Cape Verde", 3 },
		{ "Atlantic/Faroe", -1 },
		{ "Atlantic/Madeira", -1 },
		{ "Atlantic/Reykjavik", 0 },
		{ "Atlantic/South Georgia", 2 },
		{ "Atlantic/St Helena", 0 },
		{ "Atlantic/Stanley", -3 },
		{ "Australia/Adelaide", -9 },
		{ "Australia/Brisbane", -10 },
		{ "Australia/Broken Hill", -9 },
		{ "Australia/Currie", -10 },
		{ "Australia/Darwin", -9 },
		{ "Australia/Eucla", -8 },
		{ "Australia/Hobart", -10 },
		{ "Australia/Lindeman", -10 },
		{ "Australia/Lord Howe", -10 },
		{ "Australia/Melbourne", -10 },
		{ "Australia/Perth", -8 },
		{ "Australia/Sydney", -10 },
		{ "Etc/GMT", 0 },
		{ "Etc/GMT+1", 1 },
		{ "Etc/GMT+2", 2 },
		{ "Etc/GMT+3", 3 },
		{ "Etc/GMT+4", 4 },
		{ "Etc/GMT+5", 5 },
		{ "Etc/GMT+6", 6 },
		{ "Etc/GMT+7", 7 },
		{ "Etc/GMT+8", 8 },
		{ "Etc/GMT+9", 9 },
		{ "Etc/GMT+10", 10 },
		{ "Etc/GMT+11", 11 },
		{ "Etc/GMT+12", 12 },

		{ "Etc/GMT-1", -1 },
		{ "Etc/GMT-2", -2 },
		{ "Etc/GMT-3", -3 },
		{ "Etc/GMT-4", -4 },
		{ "Etc/GMT-5", -5 },
		{ "Etc/GMT-6", -6 },
		{ "Etc/GMT-7", -7 },
		{ "Etc/GMT-8", -8 },
		{ "Etc/GMT-9", -9 },
		{ "Etc/GMT-10", -10 },
		{ "Etc/GMT-11", -11 },
		{ "Etc/GMT-12", -12 },
		{ "Europe/Amsterdam", 2 },
		{ "Europe/Andorra", 2 },
		{ "Europe/Astrakhan", 4 },
		{ "Europe/Athens", 3 },
		{ "Europe/Belgrade", 2 },
		{ "Europe/Berlin", 2 },
		{ "Europe/Bratislava", 2 },
		{ "Europe/Brussels", 2 },
		{ "Europe/Bucharest", 3 },
		{ "Europe/Budapest", 2 },
		{ "Europe/Busingen", 2 },
		{ "Europe/Chisinau", 3 },
		{ "Europe/Copenhagen", 2 },
		{ "Europe/Dublin", 2 },
		{ "Europe/Gibraltar", 2 },
		{ "Europe/Guernsey", -1 },
		{ "Europe/Helsinki", 3 },
		{ "Europe/Isle of Man", -1 },
		{ "Europe/Istanbul", -3 },
		{ "Europe/Jersey", -1 },
		{ "Europe/Kaliningrad", -2 },
		{ "Europe/Kiev", 3 },
		{ "Europe/Kirov", -3 },
		{ "Europe/Lisbon", -1 },
		{ "Europe/Ljubljana", -2 },
		{ "Europe/London", -1 },
		{ "Europe/Luxembourg", -2 },
		{ "Europe/Madrid", -2 },
		{ "Europe/Malta", -2 },
		{ "Europe/Mariehamn", 3 },
		{ "Europe/Minsk", -3 },
		{ "Europe/Monaco", -2 },
		{ "Europe/Moscow", -3 },
		{ "Europe/Oslo", -2 },
		{ "Europe/Paris", -2 },
		{ "Europe/Podgorica", -2 },
		{ "Europe/Prague", -2 },
		{ "Europe/Riga", 3 },
		{ "Europe/Rome", -2 },
		{ "Europe/Samara", -4 },
		{ "Europe/San Marino", -2 },
		{ "Europe/Sarajevo", -2 },
		{ "Europe/Saratov", -4 },
		{ "Europe/Simferopol", -3 },
		{ "Europe/Skopje", -2 },
		{ "Europe/Sofia", 3 },
		{ "Europe/Stockholm", -2 },
		{ "Europe/Tallinn", 3 },
		{ "Europe/Tirane", -2 },
		{ "Europe/Ulyanovsk", -4 },
		{ "Europe/Uzhgorod", 3 },
		{ "Europe/Vaduz", -2 },
		{ "Europe/Vatican", -2 },
		{ "Europe/Vienna", -2 },
		{ "Europe/Vilnius", 3 },
		{ "Europe/Volgograd", -4 },
		{ "Europe/Warsaw", -2 },
		{ "Europe/Zagreb", -2 },
		{ "Europe/Zaporozhye", 3 },
		{ "Europe/Zurich", -2 },
		{ "Indian/Antananarivo", -3 },
		{ "Indian/Chagos", -6 },
		{ "Indian/Christmas", -7 },
		{ "Indian/Cocos", -6 },
		{ "Indian/Comoro", -3 },
		{ "Indian/Kerguelen", -5 },
		{ "Indian/Mahe", -4 },
		{ "Indian/Maldives", -5 },
		{ "Indian/Mauritius", -4 },
		{ "Indian/Mayotte", -3 },
		{ "Indian/Reunion", -4 },
		{ "Pacific/Apia", -12 },
		{ "Pacific/Auckland", -12 },
		{ "Pacific/Bougainville",-11 },
		{ "Pacific/Chatham", -12 },
		{ "Pacific/Chuuk", -10 },
		{ "Pacific/Easter", -6 },
		{ "Pacific/Efate",-11 },
		{ "Pacific/Enderbury", -13 },
		{ "Pacific/Fakaofo", -13 },
		{ "Pacific/Fiji", -12 },
		{ "Pacific/Funafuti", -12 },
		{ "Pacific/Galapagos", 6 },
		{ "Pacific/Gambier", 9 },
		{ "Pacific/Guadalcanal",-11 },
		{ "Pacific/Guam", 10 },
		{ "Pacific/Honolulu", 10 },
		{ "Pacific/Kiritimati", -14 },
		{ "Pacific/Kosrae",-11 },
		{ "Pacific/Kwajalein", -12 },
		{ "Pacific/Majuro", -12 },
		{ "Pacific/Marquesas", 9 },
		{ "Pacific/Midway", 11 },
		{ "Pacific/Nauru", -12 },
		{ "Pacific/Niue", 11 },
		{ "Pacific/Norfolk",-11 },
		{ "Pacific/Noumea",-11 },
		{ "Pacific/Pago Pago", 11 },
		{ "Pacific/Palau", -9 },
		{ "Pacific/Pitcairn", 8 },
		{ "Pacific/Pohnpei",-11 },
		{ "Pacific/Port Moresby", -10 },
		{ "Pacific/Rarotonga", 10 },
		{ "Pacific/Saipan", 10 },
		{ "Pacific/Tahiti", 10 },
		{ "Pacific/Tarawa", -12 },
		{ "Pacific/Tongatapu", -13 },
		{ "Pacific/Wake", -12 },
		{ "Pacific/Wallis", -12 },
};


int x5b_app_timezone_offset_api(char * res)
{
	int i = 0;
	char tmp[APP_ID_MAX];
	int	 ret = 0;
	if(res)
	{
		for(i = 0; i < sizeof(time_zone_tbl)/sizeof(time_zone_tbl[0]); i++)
		{
			if(strcmp(time_zone_tbl[i].time_zone, res) == 0)
			{
				return time_zone_tbl[i].offset;
			}
		}
	}
	else
	{
		memset(tmp, 0, sizeof(tmp));
#ifdef PL_OPENWRT_UCI
		ret |= os_uci_get_string("system.@system[0].zonename", tmp);
#else
		ret = OK;
		strcpy(tmp, "Etc/GMT-8");
#endif
		if(ret != OK)
		{
			return 0;
		}
		for(i = 0; i < sizeof(time_zone_tbl)/sizeof(time_zone_tbl[0]); i++)
		{
			if(strcmp(time_zone_tbl[i].time_zone, tmp) == 0)
			{
				return time_zone_tbl[i].offset;
			}
		}
	}
	return 0;
}

#if 0
int x5b_app_rtc_time_get(int to, rtcTimeType *ti)
{
	u_int32 timesp = os_time(NULL);
	struct tm tm;
	memset(&tm, 0, sizeof(struct tm));
	localtime_r(&timesp, &tm);
	if(ti)
	{
		ti->year = tm.tm_year + 1900 - 2000;
		ti->mon = tm.tm_mon + 1;
		ti->day = tm.tm_mday;
		ti->hour = tm.tm_hour;
		ti->min = tm.tm_min;
		ti->sec = tm.tm_sec;
		ti->week = tm.tm_wday;

		zlog_debug(ZLOG_APP, "============%s->SYNC ker:->%d/%d/%d %d:%d:%d", __func__,
				   tm.tm_year, tm.tm_mon, ti->day, ti->hour, ti->min, ti->sec);

		zlog_debug(ZLOG_APP, "============%s->SYNC A:->%d/%d/%d %d:%d:%d", __func__,
				   ti->year, ti->mon, ti->day, ti->hour, ti->min, ti->sec);
#if 0
		timeinfo.tm_year = time->year + 2000 - 1900;
		timeinfo.tm_mon = time->mon - 1;
		timeinfo.tm_mday = time->day;
		timeinfo.tm_hour = time->hour;
		timeinfo.tm_min = time->min;
		timeinfo.tm_sec = time->sec;
		timeinfo.tm_wday = time->week;

		struct tm
		{
		  int tm_sec;			/* Seconds.	[0-60] (1 leap second) */
		  int tm_min;			/* Minutes.	[0-59] */
		  int tm_hour;			/* Hours.	[0-23] */
		  int tm_mday;			/* Day.		[1-31] */
		  int tm_mon;			/* Month.	[0-11] */
		  int tm_year;			/* Year	- 1900.  */
		  int tm_wday;			/* Day of week.	[0-6] */
		  int tm_yday;			/* Days in year.[0-365]	*/
		  int tm_isdst;			/* DST.		[-1/0/1]*/

		# ifdef	__USE_BSD
		  long int tm_gmtoff;		/* Seconds east of UTC.  */
		  const char *tm_zone;		/* Timezone abbreviation.  */
		# else
		  long int __tm_gmtoff;		/* Seconds east of UTC.  */
		  const char *__tm_zone;	/* Timezone abbreviation.  */
		# endif
		};
#endif
	}
	return OK;
}
#endif
//北京时间=12:00-（9-8）=11:00
/*
int x5b_app_call_result_api(int res)
{
	if(x5_b_a_mgt)
		return x5_b_a_call_result_api(x5_b_a_mgt, res);
	return ERROR;
}

int x5b_app_open_result_api(int res)
{
	if(x5_b_a_mgt)
		return x5_b_a_open_result_api(x5_b_a_mgt, res);
	return ERROR;
}


int x5b_app_open_door_api(int res)
{
	if(x5_b_a_mgt)
		return x5_b_a_open_door_api(x5_b_a_mgt, res);
	return ERROR;
}
*/
