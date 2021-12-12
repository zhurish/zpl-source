/*
 * modem_event.c
 *
 *  Created on: Jul 26, 2018
 *      Author: zhurish
 */


#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"

#include "modem.h"
#include "modem_attty.h"
#include "modem_client.h"
#include "modem_event.h"
#include "modem_machine.h"
#include "modem_serial.h"
#include "modem_process.h"
#include "modem_atcmd.h"
#include "modem_control.h"
#include "modem_mgtlayer.h"

/*
struct modem_event_action
{
	int				flag;
	modem_event 	event;
	modem_t			*modem;
	modem_client_t	*client;
	void			*pVoid;
};
*/


const char *modem_event_string(modem_event event)
{
	switch(event)
	{
	case MODEM_EV_NONE: //检测
		return "NONE";
		break;
	case MODEM_EV_INIT: //检测
		return "INIT";
		break;
	case MODEM_EV_REBOOT: //检测
		return "REBOOT";
		break;

	case MODEM_EV_MESSAGE: //检测
		return "MESSAGE";
		break;
	case MODEM_EV_DETECTION: //检测
		return "DETECTION";
		break;
	case MODEM_EV_REMOVE:	//模块拔除
		return "REMOVE";
		break;
	case MODEM_EV_INSTER:	//模块插入
		return "INSTER";
		break;
	case MODEM_EV_REMOVE_CARD://SIM 卡 移除
		return "REMOVE CARD";
		break;
	case MODEM_EV_INSTER_CARD://SIM 卡 插入
		return "INSTER CARD";
		break;
	case MODEM_EV_SWITCH_CARD://
		return "SWITCH CARD";
		break;
	case MODEM_EV_NWSETUP://
		return "NW SETUP";
		break;
	case MODEM_EV_ATTACH://
		return "ATTACH";
		break;
	case MODEM_EV_UNATTACH://
		return "UNATTACH";
		break;
	case MODEM_EV_ONLINE:	//网络上线
		return "ONLINE";
		break;
	case MODEM_EV_OFFLINE:	//网络下线
		return "OFFLINE";
		break;

	case MODEM_EV_DELAY:		//延时
		return "DELAY";
		break;
	case MODEM_EV_DIALOG: //检测
		return "DIALOG";
		break;
	case MODEM_EV_REDIALOG: //检测
		return "REDIALOG";
		break;
	default:
		return "UNKNOWN";
		break;
	}
}
/*
 *添加事件
 */
int modem_event_add_api(modem_t *modem, modem_event event, zpl_bool lock)
{
	assert(modem);
	if(modem)
	{
		//modem_client_t	*client = modem->client;
		//if(client)
		//	client->event = event;
		modem->event = event;
		modem_process_add_api(event, modem, lock);
	}
	//else
	//	modem_process_add_api(event, NULL, lock);
	return OK;
}

/*
 *添加事件
 */
int modem_event_del_api(modem_t *modem, modem_event event, zpl_bool lock)
{
	assert(modem);
	if(modem)
	{
		//modem_client_t	*client = modem->client;
		//if(client)
		//	client->event = event;
		modem_process_del_api(event, modem, lock);
	}
	//else
	//	modem_process_add_api(event, NULL, lock);
	return OK;
}

static int modem_event_reload_thread(modem_t *modem)
{
	if(modem && modem->event)
	{
		modem_event_del_api(modem, MODEM_EV_MAX, zpl_true);
		modem_event_add_api(modem, modem->a_event, zpl_true);
		modem->state = MODEM_MACHINE_STATE_NONE;
		MODEM_EV_DEBUG("Into %s",__func__);
		if(MODEM_IS_DEBUG(EVENT))
			zlog_debug(MODULE_MODEM, "Handle %s on time delay thread.", modem_event_string(modem->a_event));
		modem->t_time = 0;
		modem->a_event = MODEM_EV_NONE;
		return OK;
	}
	return OK;
}


int modem_event_reload(modem_t *modem, modem_event event, zpl_bool lock)
{
	if(modem->t_time)
	{
		if(os_time_lookup(modem->t_time))
		{
			os_time_cancel(modem->t_time);
			os_time_restart(modem->t_time, 10000);
			return OK;
		}
	}
/*	if(MODEM_IS_DEBUG(EVENT))
		zlog_debug(MODULE_MODEM, "modem add event %s.", modem_event_string(modem->a_event));*/
	modem->a_event = event;
	modem->t_time = os_time_create_once(modem_event_reload_thread, modem, 10000);
	return OK;
}

/*
 * 事件调度开始
 */
static int modem_event_start(modem_t *modem, modem_event event)
{
	assert(modem);
	//MODEM_EV_DEBUG("Into %s",__func__);

	if(modem)
	{
		modem_client_t	*client = modem->client;
		if(client)
		{
			/*
			 * 确认AT通道打开
			 */
			if(modem_attty_isopen(client) != OK)
			{
				//MODEM_EV_DEBUG("Level 0 %s",__func__);
				return modem_attty_open(client);
			}
			else
			{
				//MODEM_EV_DEBUG("Level 1 %s",__func__);
				return OK;
			}
		}
		else if(modem->serialname)
		{
			/*
			 * 更新 邦定的client
			 */
			modem_serial_t * serial = modem_serial_lookup_api(modem->serialname, 0);
			if(serial)
			{
				if(serial->client)
					modem->client = serial->client;
				if(modem->client)
				{
					if(modem_attty_isopen(modem->client) != OK)
					{
						//MODEM_EV_DEBUG("Level 2 %s",__func__);
						return modem_attty_open(modem->client);
					}
					//MODEM_EV_DEBUG("Level 3 %s",__func__);
					return OK;
				}
			}
		}
	}
	MODEM_EV_DEBUG("Level %s",__func__);
	return ERROR;
}

/*
 * 事件调度结束
 */
static int modem_event_end(modem_t *modem)
{
	assert(modem);
	//MODEM_EV_DEBUG("Into %s",__func__);
	if(modem)
	{
		//modem_client_t	*client = modem->client;
		//if(client)
		//	client->event = 0;
	}
	//MODEM_EV_DEBUG("Level %s",__func__);
	return OK;
}

/*
 * 模块插入事件
 */
modem_event modem_event_inster(modem_t *modem, modem_event event)
{
	modem_event nextevent = MODEM_EV_NONE;
	assert(modem);
	MODEM_EV_DEBUG("Into %s",__func__);

	if(MODEM_IS_DEBUG(EVENT))
		zlog_debug(MODULE_MODEM, "Handle %s event on %s", modem_event_string(event),
				(const char*)modem_module_name(modem));

	if(modem_mgtlayer_inster(modem) != OK)
		nextevent = MODEM_EV_INSTER;
	else
	{
		nextevent = MODEM_EV_INIT;
	}
	MODEM_EV_DEBUG("Level %s",__func__);
	return nextevent;
}

/*
 * 模块功能初始化
 */
modem_event modem_event_init(modem_t *modem, modem_event event)
{
	modem_event nextevent = MODEM_EV_NONE;
	assert(modem);
	//MODEM_EV_DEBUG("Into %s",__func__);
	if(MODEM_IS_DEBUG(EVENT))
		zlog_debug(MODULE_MODEM, "Handle %s event on %s", modem_event_string(event),
				modem_module_name(modem));

	if(modem_mgtlayer_init(modem) != OK)
		nextevent = MODEM_EV_INIT;
	else
	{
		nextevent = MODEM_EV_INSTER_CARD;
	}

	//MODEM_EV_DEBUG("Level %s",__func__);
	return nextevent;
}

/*
 * 模块拔出事件
 */
modem_event modem_event_remove(modem_t *modem, modem_event event)
{
	//modem_serial_t *serial = NULL;
	modem_event nextevent = MODEM_EV_NONE;
	assert(modem);
	MODEM_EV_DEBUG("Into %s",__func__);
	if(MODEM_IS_DEBUG(EVENT))
		zlog_debug(MODULE_MODEM, "Handle %s event on %s", modem_event_string(event),
				modem_module_name(modem));
	/*
	 * Stop network
	 */
	if(modem_mgtlayer_remove(modem) != OK)
		nextevent = MODEM_EV_REMOVE;
	else
	{
		modem->state = modem->newstate;
		modem_event_del_api(modem, MODEM_EV_MAX, zpl_false);
	}
	MODEM_EV_DEBUG("Level %s",__func__);
	return nextevent;
}

/*
 * USIM卡拔出事件
 */
modem_event modem_event_card_remove(modem_t *modem, modem_event event)
{
	modem_event nextevent = MODEM_EV_NONE;
	assert(modem);
	MODEM_EV_DEBUG("Into %s",__func__);
	if(MODEM_IS_DEBUG(EVENT))
		zlog_debug(MODULE_MODEM, "Handle %s event on %s", modem_event_string(event),
				modem_module_name(modem));
	/*
	 * Stop network
	 */
	if(modem_mgtlayer_remove_usim(modem) != OK)
		nextevent = MODEM_EV_REMOVE_CARD;
	else
	{
		modem->state = modem->newstate;
		nextevent = MODEM_EV_NONE;
	}
	//modem_event_offline(modem, MODEM_EV_OFFLINE);
	MODEM_EV_DEBUG("Level %s",__func__);
	return nextevent;
}

/*
 * USIM卡插入事件
 */
modem_event modem_event_card_inster(modem_t *modem, modem_event event)
{
	modem_event nextevent = MODEM_EV_NONE;
	assert(modem);
	MODEM_EV_DEBUG("Into %s",__func__);
	if(MODEM_IS_DEBUG(EVENT))
		zlog_debug(MODULE_MODEM, "Handle %s event on %s", modem_event_string(event),
				modem_module_name(modem));

	if(modem_mgtlayer_inster_usim(modem) != OK)
		nextevent = MODEM_EV_INSTER_CARD;//NO CARD redetection
	else
	{
		modem->state = modem->newstate;
		nextevent = MODEM_EV_NWSETUP;//CARD is ready, setup and active network
	}
	/*
	 * Active network
	 */
	MODEM_EV_DEBUG("Level %s",__func__);
	return nextevent;
}

/*
 * USIM卡事件，手动切换USIM卡
 */
modem_event modem_event_card_switch(modem_t *modem, modem_event event)
{
	modem_event nextevent = MODEM_EV_NONE;
	assert(modem);
	MODEM_EV_DEBUG("Into %s",__func__);
	if(MODEM_IS_DEBUG(EVENT))
		zlog_debug(MODULE_MODEM, "Handle %s event on %s", modem_event_string(event),
				modem_module_name(modem));
	/*
	 * Stop network
	 */
	if(modem_mgtlayer_switch_usim(modem) != OK)
		nextevent = MODEM_EV_SWITCH_CARD;//NO CARD redetection
	else
	{
		modem->state = modem->newstate;
		nextevent = MODEM_EV_INSTER_CARD;//CARD is ready, setup and active network
	}
	/*
	 * Active network
	 */
	MODEM_EV_DEBUG("Level %s",__func__);
	return nextevent;
}

/*
 * 模块网络初始化事件
 */
modem_event modem_event_network_setup(modem_t *modem, modem_event event)
{
	modem_event nextevent = MODEM_EV_NONE;
	assert(modem);
	MODEM_EV_DEBUG("Into %s",__func__);
	if(MODEM_IS_DEBUG(EVENT))
		zlog_debug(MODULE_MODEM, "Handle %s event on %s", modem_event_string(event),
				modem_module_name(modem));

	if(modem_mgtlayer_network_setup(modem) != OK)
		nextevent = MODEM_EV_NWSETUP;
	else
	{
		modem->state = modem->newstate;
		nextevent = MODEM_EV_ATTACH;
	}
	MODEM_EV_DEBUG("Level %s",__func__);
	return nextevent;
}

/*
 * 完成拨号,网络上线
 */
modem_event modem_event_online(modem_t *modem, modem_event event)
{
	modem_event nextevent = MODEM_EV_NONE;
	assert(modem);
	MODEM_EV_DEBUG("Into %s",__func__);
	if(MODEM_IS_DEBUG(EVENT))
		zlog_debug(MODULE_MODEM, "Handle %s event on %s", modem_event_string(event),
				modem_module_name(modem));

	if(modem_mgtlayer_network_online(modem) != OK)
		nextevent = MODEM_EV_ONLINE;
	else
	{
		modem->state = modem->newstate;
		nextevent = MODEM_EV_DETECTION;
		modem->uptime = os_time(NULL);		//network UP time
	}
	MODEM_EV_DEBUG("Level %s",__func__);
	return nextevent;
}


/*
 *
 * 网络激活事件，拨号
 */
modem_event modem_event_network_attach(modem_t *modem, modem_event event)
{
	modem_event nextevent = MODEM_EV_NONE;
	assert(modem);
	MODEM_EV_DEBUG("Into %s",__func__);
	if(MODEM_IS_DEBUG(EVENT))
		zlog_debug(MODULE_MODEM, "Handle %s event on %s", modem_event_string(event),
				modem_module_name(modem));

	if(modem_mgtlayer_network_attach(modem) != OK)
		nextevent = MODEM_EV_ATTACH;
	else	
	{
		modem->state = modem->newstate;
		nextevent = MODEM_EV_ONLINE;
	}
	MODEM_EV_DEBUG("Level %s",__func__);
	return nextevent;
}

/*
 * 网络激活事件，拨号
 */
modem_event modem_event_network_unattach(modem_t *modem, modem_event event)
{
	modem_event nextevent = MODEM_EV_NONE;
	assert(modem);
	MODEM_EV_DEBUG("Into %s",__func__);
	if(MODEM_IS_DEBUG(EVENT))
		zlog_debug(MODULE_MODEM, "Handle %s event on %s", modem_event_string(event),
				modem_module_name(modem));

	if(modem_mgtlayer_network_unattach(modem) != OK)
		nextevent = MODEM_EV_UNATTACH;
	else
	{
		modem->state = modem->newstate;
	}
	MODEM_EV_DEBUG("Level %s",__func__);
	return nextevent;
}

/*
 * 网络下线事件
 */
modem_event modem_event_offline(modem_t *modem, modem_event event)
{
	modem_event nextevent = MODEM_EV_NONE;
	assert(modem);
	MODEM_EV_DEBUG("Into %s",__func__);
	if(MODEM_IS_DEBUG(EVENT))
		zlog_debug(MODULE_MODEM, "Handle %s event on %s", modem_event_string(event),
				modem_module_name(modem));

	if(modem_mgtlayer_network_offline(modem) != OK)
		nextevent = MODEM_EV_ONLINE;
	else
	{
		modem->state = modem->newstate;
		modem->downtime = os_time(NULL);		//network DOWN time
		nextevent = MODEM_EV_DELAY;
	}
	MODEM_EV_DEBUG("Level %s",__func__);
	return nextevent;
}



modem_event modem_event_delay(modem_t *modem, modem_event event)
{
	modem_event nextevent = MODEM_EV_DELAY;
	assert(modem);
	modem->time_axis = os_time (NULL);
	if(modem->time_axis >= (modem->time_base + modem->delay))
	{
		modem->time_base = modem->time_axis;
		MODEM_EV_DEBUG("Into %s",__func__);
		if(MODEM_IS_DEBUG(EVENT))
			zlog_debug(MODULE_MODEM, "Handle %s event on %s", modem_event_string(event),
					modem_module_name(modem));

		if(modem_mgtlayer_delay(modem) != OK)
			nextevent = MODEM_EV_NONE;
		else
		{
			modem->state = modem->newstate;
		}
		MODEM_EV_DEBUG("Level %s",__func__);
	}
	return nextevent;
}

modem_event modem_event_detection(modem_t *modem, modem_event event)
{
	modem_event nextevent = MODEM_EV_DETECTION;
	assert(modem);
	modem->detime_axis = os_time (NULL);
	if(modem->detime_axis >= (modem->detime_base + modem->dedelay))
	{
		modem->detime_base = modem->detime_axis;
		MODEM_EV_DEBUG("Into %s",__func__);
		if(MODEM_IS_DEBUG(EVENT))
			zlog_debug(MODULE_MODEM, "Handle %s event on %s", modem_event_string(event),
					modem_module_name(modem));

		if(modem_mgtlayer_network_detection(modem) != OK)
			nextevent = MODEM_EV_NONE;
		else
		{
			modem->state = modem->newstate;
		}
		MODEM_EV_DEBUG("Level %s",__func__);
	}
	return nextevent;
}


modem_event modem_event_reboot(modem_t *modem, modem_event event)
{
	modem_event nextevent = MODEM_EV_NONE;
	assert(modem);
	MODEM_EV_DEBUG("Into %s",__func__);
	if(MODEM_IS_DEBUG(EVENT))
		zlog_debug(MODULE_MODEM, "Handle %s event on %s", modem_event_string(event),
				modem_module_name(modem));

	modem->state = modem->newstate;

	MODEM_EV_DEBUG("Level %s",__func__);
	return nextevent;
}

modem_event modem_event_dailog(modem_t *modem, modem_event event)
{
	modem_event nextevent = MODEM_EV_NONE;
	assert(modem);
	MODEM_EV_DEBUG("Into %s",__func__);
	if(MODEM_IS_DEBUG(EVENT))
		zlog_debug(MODULE_MODEM, "Handle %s event on %s", modem_event_string(event),
				modem_module_name(modem));

	if(modem_mgtlayer_dialog(modem) != OK)
		nextevent = MODEM_EV_DIALOG;
	else
	{
		modem->state = modem->newstate;
	}
	MODEM_EV_DEBUG("Level %s",__func__);
	return nextevent;
}

modem_event modem_event_redailog(modem_t *modem, modem_event event)
{
	modem_event nextevent = MODEM_EV_NONE;
	assert(modem);
	MODEM_EV_DEBUG("Into %s",__func__);
	if(MODEM_IS_DEBUG(EVENT))
		zlog_debug(MODULE_MODEM, "Handle %s event on %s", modem_event_string(event),
				modem_module_name(modem));
				
	if(modem_mgtlayer_redialog(modem) != OK)
		nextevent = MODEM_EV_REDIALOG;
	else
	{
		modem->state = modem->newstate;
	}
	MODEM_EV_DEBUG("Level %s",__func__);
	return nextevent;
}

modem_event modem_event_message(modem_t *modem, modem_event event)
{
	modem_event nextevent = MODEM_EV_NONE;
	assert(modem);
	MODEM_EV_DEBUG("Into %s",__func__);
	if(MODEM_IS_DEBUG(EVENT))
		zlog_debug(MODULE_MODEM, "Handle %s event on %s", modem_event_string(event),
				modem_module_name(modem));
				
	if(modem_mgtlayer_message(modem) != OK)
		nextevent = MODEM_EV_REDIALOG;
	else
	{
		modem->state = modem->newstate;
	}
	MODEM_EV_DEBUG("Level %s",__func__);
	return nextevent;
}

/********************************************************/

modem_event modem_event_process(modem_t *modem, modem_event event)
{
	modem_event nextevent = MODEM_EV_NONE;
	assert(modem);
	if( event != MODEM_EV_REMOVE)
	{
		if(modem_event_start(modem, event) != OK)
		{
			modem_event_end(modem);
			if(MODEM_IS_DEBUG(EVENT))
				zlog_err(MODULE_MODEM, "Handle %s event on %s, AT Channel is not Open.",
						modem_event_string(event), modem_module_name(modem));
			fprintf(stdout, "Handle %s event on %s, AT Channel is not Open.",
									modem_event_string(event), modem_module_name(modem));
			return MODEM_EV_INSTER;
		}
		if(modem_atcmd_isopen(modem->client) != OK)
		{
			modem_event_end(modem);
			return MODEM_EV_INSTER;
		}
		modem_mgtlayer_open(modem->client);
	}
	switch(event)
	{
	case MODEM_EV_INIT: //检测
		nextevent = modem_event_init(modem,  event);
		break;
	case MODEM_EV_REBOOT: //检测
		nextevent = modem_event_reboot(modem,  event);
		break;
	case MODEM_EV_DIALOG: //检测
		nextevent = modem_event_dailog(modem,  event);
		break;
	case MODEM_EV_REDIALOG: //检测
		nextevent = modem_event_redailog(modem,  event);
		break;
		
	case MODEM_EV_MESSAGE: //检测
		nextevent = modem_event_message(modem,  event);
		break;
	case MODEM_EV_DETECTION: //检测
		nextevent = modem_event_detection(modem,  event);
		break;
	case MODEM_EV_REMOVE:	//模块拔除
		nextevent = modem_event_remove(modem,  event);
		break;
	case MODEM_EV_INSTER:	//模块插入
		nextevent = modem_event_inster(modem,  event);
		break;
	case MODEM_EV_REMOVE_CARD://SIM 卡 移除
		nextevent = modem_event_card_remove(modem,  event);
		break;
	case MODEM_EV_INSTER_CARD://SIM 卡 插入
		nextevent = modem_event_card_inster(modem,  event);
		break;
	case MODEM_EV_SWITCH_CARD://切换SIM 卡
		nextevent = modem_event_card_switch(modem,  event);
		break;

	case MODEM_EV_NWSETUP://设置
		nextevent = modem_event_network_setup(modem,  event);
		break;

	case MODEM_EV_ATTACH://设置
		nextevent = modem_event_network_attach(modem,  event);
		break;
	case MODEM_EV_UNATTACH://设置
		nextevent = modem_event_network_unattach(modem,  event);
		break;

	case MODEM_EV_ONLINE:	//网络上线
		nextevent = modem_event_online(modem,  event);
		break;
	case MODEM_EV_OFFLINE:	//网络下线
		nextevent = modem_event_offline(modem,  event);
		break;
	case MODEM_EV_DELAY:		//延时
		nextevent = modem_event_delay(modem,  event);
		break;
	default:
		break;
	}
	modem_event_end(modem);
	if( event != MODEM_EV_REMOVE)
	{
		modem_machine_state_action(modem);

		if(nextevent != MODEM_EV_NONE)
			modem_event_add_api(modem, nextevent, zpl_false);
	}
	return nextevent;
}
