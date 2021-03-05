/*
 * modem_event.h
 *
 *  Created on: Jul 26, 2018
 *      Author: zhurish
 */

#ifndef __MODEM_EVENT_H__
#define __MODEM_EVENT_H__

#ifdef __cplusplus
extern "C" {
#endif

//#include "modem.h"

//#define __MODEM_EVENT_DEBUG

typedef struct modem_s modem_t;

typedef enum modem_event_s
{
	MODEM_EV_NONE = 0,

	MODEM_EV_REBOOT, 			//重启

	MODEM_EV_REMOVE,			//模块拔除：断开网络

	MODEM_EV_INSTER,			//模块插入：完成模块信息获取

	MODEM_EV_INIT, 			//初始化：初始化模块功能，设置网络注册信息

	MODEM_EV_REMOVE_CARD,		//SIM 卡 移除：断开网络，进入检测USIM卡

	MODEM_EV_SWITCH_CARD,		//切换SIM 卡：断开网络，重新拨号

	MODEM_EV_INSTER_CARD,		//SIM 卡 插入：获取USIM卡信息

	MODEM_EV_NWSETUP, 			//网络设置：初始化网络参数，APN等相关参数

	MODEM_EV_ATTACH, 			//网络激活，进行拨号

	MODEM_EV_UNATTACH, 			//网络去激活，

	MODEM_EV_ONLINE,			//网络上线：DHCP获取IP地址信息

	MODEM_EV_OFFLINE,			//网络下线：释放网络，释放DHCP地址，断开网络

	MODEM_EV_DETECTION, 		//模块检测事件(before active network)

	MODEM_EV_DELAY,				//延时 (after active network)

	MODEM_EV_REDIALOG, 			//重拨号

	MODEM_EV_DIALOG, 			//电话拨号		  --> ONLINE

	MODEM_EV_MESSAGE, 			//发短信

	MODEM_EV_MAX,

}modem_event;


extern int modem_event_add_api(modem_t *, modem_event, ospl_bool );
extern int modem_event_del_api(modem_t *, modem_event, ospl_bool );
extern int modem_event_reload(modem_t *, modem_event, ospl_bool );


extern const char *modem_event_string(modem_event );
extern modem_event modem_event_inster(modem_t *, modem_event );
extern modem_event modem_event_remove(modem_t *, modem_event );
extern modem_event modem_event_card_remove(modem_t *, modem_event );
extern modem_event modem_event_card_inster(modem_t *, modem_event );
extern modem_event modem_event_card_switch(modem_t *, modem_event );
extern modem_event modem_event_online(modem_t *, modem_event );
extern modem_event modem_event_offline(modem_t *, modem_event );
extern modem_event modem_event_delay(modem_t *, modem_event );
extern modem_event modem_event_detection(modem_t *, modem_event );

extern modem_event modem_event_init(modem_t *, modem_event );
extern modem_event modem_event_network_setup(modem_t *, modem_event );

extern modem_event modem_event_network_attach(modem_t *, modem_event );
extern modem_event modem_event_network_unattach(modem_t *, modem_event );
extern modem_event modem_event_reboot(modem_t *, modem_event );
extern modem_event modem_event_dailog(modem_t *, modem_event );

extern modem_event modem_event_redailog(modem_t *, modem_event );
extern modem_event modem_event_message(modem_t *, modem_event );



extern modem_event modem_event_process(modem_t *, modem_event );

#ifdef __MODEM_EVENT_DEBUG
#define MODEM_EV_DEBUG(fmt,...)	modem_debug_printf(stderr, __func__, __LINE__,fmt, ##__VA_ARGS__)
#define MODEM_EV_WARN(fmt,...)		modem_debug_printf(stderr, __func__, __LINE__,fmt, ##__VA_ARGS__)
#define MODEM_EV_ERROR(fmt,...)	modem_debug_printf(stderr, __func__, __LINE__,fmt, ##__VA_ARGS__)
#else
#define MODEM_EV_DEBUG(fmt,...)
#define MODEM_EV_WARN(fmt,...)
#define MODEM_EV_ERROR(fmt,...)
#endif


#ifdef __cplusplus
}
#endif

#endif /* __MODEM_EVENT_H__ */
