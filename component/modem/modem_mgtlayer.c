/*
 * modem_mgtlayer.c
 *
 *  Created on: Aug 22, 2018
 *      Author: zhurish
 */


#include "zebra.h"
#include "log.h"
#include "memory.h"
#include "str.h"
#include "os_util.h"
#include "tty_com.h"

#include "modem.h"
#include "modem_attty.h"
#include "modem_client.h"
#include "modem_message.h"
#include "modem_event.h"
#include "modem_machine.h"
#include "modem_dhcp.h"
#include "modem_pppd.h"
#include "modem_qmi.h"
#include "modem_control.h"
#include "modem_usim.h"
#include "modem_state.h"
#include "modem_atcmd.h"
#include "modem_mgtlayer.h"
#include "modem_serial.h"
#include "modem_usb_driver.h"



static int _modem_information_init(modem_client_t *client)
{
	assert(client);
	if(modem_factory_atcmd_get(client) != OK)
		return ERROR;

	if(modem_product_atcmd_get(client) != OK)
		return ERROR;

	if(modem_product_id_atcmd_get(client) != OK)
		return ERROR;

	if(modem_serial_number_atcmd_get(client) != OK)
		return ERROR;

	if(modem_version_atcmd_get(client) != OK)// -->
		return ERROR;

	if(modem_IMEI_atcmd_get(client) != OK)
		return ERROR;

	//modem_signal_atcmd_get(client);
	return OK;
}


static int _modem_capability_init(modem_client_t *client)
{
	int ret = 0;
	assert(client);
	//return OK;
	ret = modem_activity_atcmd_get(client);
	if(ret == OK)
	{
		if(client->activity == CPAS_NONE)
		{
			ret = modem_open_atcmd_set(client);
			if(ret == OK)
			{
				modem_bitmap_set(&client->hw_state, MODEM_STATE_HW_CFUN);
				//client->activity = 1;
				//modem_open_atcmd_get(client);
			}
		}
	}
	//return OK;
	//设置网络注册类型
	if(!modem_bitmap_chk(&client->hw_state, MODEM_STATE_HW_CREG))
	{
		ret = modem_cell_information_atcmd_set(client);
		if(ret != OK)
			return ERROR;
		modem_bitmap_set(&client->hw_state, MODEM_STATE_HW_CREG);
	}
	modem_cell_information_atcmd_get(client);
	return ret;;
}



static int _modem_signal_detection(modem_client_t *client)
{
	int ret = 0;
	assert(client);
	ret = modem_signal_atcmd_get(client);
	modem_signal_state_update(client);
	return ret;
}

static int _modem_network_detection(modem_client_t *client)
{
	int ret = 0;
	assert(client);
	assert(client->modem);

	if(modem_usim_detection(client->modem) != OK)// -->
		return ERROR;

	if(_modem_signal_detection(client) != OK)// -->
		return ERROR;
/*	if( ((modem_t*)client->modem)->dialtype == MODEM_DIAL_PPP )
		return OK;*/
	if(modem_nwservingcell_atcmd_get(client) != OK)// -->
		return ERROR;

	if(modem_cell_information_atcmd_get(client) != OK)// -->
		return ERROR;

	if(modem_operator_atcmd_get(client) != OK)// -->
		return ERROR;

	if(modem_nwinfo_atcmd_get(client) != OK)// -->
		return ERROR;

	if(modem_nwaddr_atcmd_get(client) != OK)// -->
		return ERROR;

	return ret;
}

static int _modem_network_setup(modem_client_t *client)
{
	int ret = 0;
	assert(client);

	if(!modem_bitmap_chk(&client->hw_state, MODEM_STATE_HW_NWSCANMODE))
	{
		ret = modem_nwscanmode_atcmd_set(client);
		if(ret != OK)
			return ERROR;
		modem_bitmap_set(&client->hw_state, MODEM_STATE_HW_NWSCANMODE);
	}

	if(!modem_bitmap_chk(&client->hw_state, MODEM_STATE_HW_PDP))
	{
		if(modem_nwpdp_atcmd_set(client) != OK)// -->
			return ERROR;
		if(ret == OK)
			modem_bitmap_set(&client->hw_state, MODEM_STATE_HW_PDP);
	}
	_modem_network_detection(client);
	return ret;
}

int modem_mgtlayer_usim_detection(modem_t *modem)
{
	assert(modem);
	return modem_usim_detection(modem);
}

int modem_mgtlayer_signal_detection(modem_t *modem)
{
	assert(modem);
	return _modem_signal_detection(modem->client);
}
/****************************************************************************/
int modem_mgtlayer_open(modem_client_t *client)
{
	assert(client);
	if(!client->init)
	{
		client->init = ospl_true;
		return modem_echo_atcmd_set(client, client->echo);
	}
	else
	{
		if(client->echo != client->echoold)
			return modem_echo_atcmd_set(client, client->echo);
		else
			return OK;
	}
	return OK;
}

int modem_mgtlayer_close(modem_client_t *client)
{
	assert(client);
	if(client->init)
	{
		client->init = ospl_false;
	}
	client->activity = CPAS_NONE;
	modem_bitmap_bzero(&client->hw_state);
	return OK;
}
/*
 * 模块插入事件调用，初始化模块，获取模块信息
 */
int modem_mgtlayer_inster(modem_t *modem)
{
	int ret = ERROR;
	assert(modem);
	ret = _modem_information_init(modem->client);
	return ret;
}

/*
 * 网络功能初始化 MODEM_EV_NWINIT事件调用
 */
int modem_mgtlayer_init(modem_t *modem)
{
	assert(modem);
	return _modem_capability_init(modem->client);
}


/*
 * 模块插入事件调用，初始化模块，获取模块信息
 */
int modem_mgtlayer_remove(modem_t *modem)
{
	if( modem->dialtype == MODEM_DIAL_PPP )
		modem_pppd_disconnect(modem);
	else
		modem_dhcpc_exit(modem);

	modem_mgtlayer_close(modem->client);

	modem_serial_unbind_api(modem->serialname, ((modem_serial_t*)modem->serial)->hw_channel);

	return OK;
}

/*
 * USIM卡拔出事件调用
 */
int modem_mgtlayer_remove_usim(modem_t *modem)
{
	if( modem->dialtype == MODEM_DIAL_PPP )
		modem_pppd_disconnect(modem);
	else
		modem_dhcpc_exit(modem);
	return OK;
}
/*
 * USIM卡插入事件调用
 */
int modem_mgtlayer_inster_usim(modem_t *modem)
{
	assert(modem);
	return modem_usim_detection(modem);
}
/*
 * USIM卡切换事件调用
 */
int modem_mgtlayer_switch_usim(modem_t *modem)
{
	return OK;
}

/*
 * 网络参数初始化，APN，Profile等信息
 */
extern int modem_mgtlayer_network_setup(modem_t *modem)
{
	assert(modem);
	if( modem->dialtype != MODEM_DIAL_PPP )
		return _modem_network_setup(modem->client);
	return OK;
}

/*
 * 网络激活，完成拨号上网
 */
int modem_mgtlayer_network_attach(modem_t *modem)
{
	assert(modem);
	if( modem->dialtype == MODEM_DIAL_NONE ||
		modem->dialtype == MODEM_DIAL_DHCP)
		return modem_dhcpc_attach(modem);
	else if( modem->dialtype == MODEM_DIAL_QMI )
		return modem_dhcpc_attach(modem);
	else if( modem->dialtype == MODEM_DIAL_GOBINET )
		return modem_dhcpc_attach(modem);
	else if( modem->dialtype == MODEM_DIAL_PPP )
		return modem_pppd_connect(modem);
	return ERROR;
}

/*
 * 网络去激活，禁止拨号上网
 */
int modem_mgtlayer_network_unattach(modem_t *modem)
{
	assert(modem);
	if( modem->dialtype == MODEM_DIAL_NONE ||
		modem->dialtype == MODEM_DIAL_DHCP)
		return modem_dhcpc_unattach(modem);
	else if( modem->dialtype == MODEM_DIAL_QMI )
		return modem_dhcpc_unattach(modem);
	else if( modem->dialtype == MODEM_DIAL_GOBINET )
		return modem_dhcpc_unattach(modem);
	else if( modem->dialtype == MODEM_DIAL_PPP )
		return modem_pppd_disconnect(modem);
	return OK;
}

/*
 * 网络接通，通过DHCP获取IP地址信息
 */
int modem_mgtlayer_network_online(modem_t *modem)
{
	assert(modem);
	if( modem->dialtype == MODEM_DIAL_NONE ||
		modem->dialtype == MODEM_DIAL_DHCP)
		return modem_dhcpc_start(modem);
	else if( modem->dialtype == MODEM_DIAL_QMI )
		return modem_dhcpc_start(modem);
	else if( modem->dialtype == MODEM_DIAL_GOBINET )
		return modem_dhcpc_start(modem);
	else if( modem->dialtype == MODEM_DIAL_PPP )
		return OK;//return modem_pppd_connect(modem);
	return ERROR;
}

/*
 * 网络去接通，释放DHCP获取的IP地址信息
 */
int modem_mgtlayer_network_offline(modem_t *modem)
{
	assert(modem);
	if( modem->dialtype == MODEM_DIAL_NONE ||
		modem->dialtype == MODEM_DIAL_DHCP)
		return modem_dhcpc_exit(modem);
	else if( modem->dialtype == MODEM_DIAL_QMI )
		return modem_dhcpc_exit(modem);
	else if( modem->dialtype == MODEM_DIAL_GOBINET )
		return modem_dhcpc_exit(modem);
	else if( modem->dialtype == MODEM_DIAL_PPP )
		return modem_pppd_disconnect(modem);
	return OK;
}

/*
 * 网络状态检测事件，拨上号后检测
 */
int modem_mgtlayer_network_detection(modem_t *modem)
{
	assert(modem);
/*	modem->detime_axis = os_time (NULL);
	if(modem->detime_axis >= (modem->detime_base + modem->dedelay))
	{
		modem->detime_base = os_time (NULL);*/
		if( modem->dialtype == MODEM_DIAL_PPP )
		{
			if(modem_pppd_isconnect(modem))
				;
			if(modem_pppd_islinkup(modem))
				;
		}
		return _modem_network_detection(modem->client);
//	}
	return OK;
}

/*
 * 延时事件
 */
int modem_mgtlayer_delay(modem_t *modem)
{
/*	modem->time_axis = os_time (NULL);
	if(modem->time_axis >= (modem->time_base + modem->delay))
	{*/
		//modem_machine_state_action(modem);
		modem_machine_state(modem);
		//modem->time_base = os_time (NULL);

		switch(modem->state)
		{
		case MDMS(NO_USIM_CARD):
			modem_event_add_api(modem, MODEM_EV_INSTER_CARD, ospl_false);
			break;
		case MDMS(NO_SIGNAL):
			modem_event_add_api(modem, MODEM_EV_DETECTION, ospl_false);
			break;
		case MDMS(NO_ADDR):
			modem_event_add_api(modem, MODEM_EV_DETECTION, ospl_false);
			break;
		case MDMS(NO_SERVICE):
			modem_event_add_api(modem, MODEM_EV_DETECTION, ospl_false);
			break;
		default:
			break;
		}
	//}
	if( modem->dialtype == MODEM_DIAL_PPP )
	{
		if(modem_pppd_isconnect(modem))
			;
		if(modem_pppd_islinkup(modem))
			;
	}
	return OK;
}

/*
 * 重新拨号
 */
int modem_mgtlayer_redialog(modem_t *modem)
{
	return OK;
}

/*
 * 拨号
 */
int modem_mgtlayer_dialog(modem_t *modem)
{
	return OK;
}

/*
 * 短信事件
 */
int modem_mgtlayer_message(modem_t *modem)
{
	return OK;
}
/****************************************************************************/

