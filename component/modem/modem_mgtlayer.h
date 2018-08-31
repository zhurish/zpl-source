/*
 * modem_mgtlayer.h
 *
 *  Created on: Aug 22, 2018
 *      Author: zhurish
 */

#ifndef __MODEM_MGTLAYER_H__
#define __MODEM_MGTLAYER_H__


extern int modem_mgtlayer_open(modem_client_t *client);
extern int modem_mgtlayer_close(modem_client_t *client);

/*
 * 模块插入事件调用，初始化模块，获取模块信息
 */
extern int modem_mgtlayer_inster(modem_t *modem);

/*
 * 网络功能初始化 MODEM_EV_NWINIT事件调用
 */
extern int modem_mgtlayer_init(modem_t *modem);


/*
 * 模块插入事件调用，初始化模块，获取模块信息
 */
extern int modem_mgtlayer_remove(modem_t *modem);

/*
 * USIM卡拔出事件调用
 */
extern int modem_mgtlayer_remove_usim(modem_t *modem);
/*
 * USIM卡插入事件调用
 */
extern int modem_mgtlayer_inster_usim(modem_t *modem);
/*
 * USIM卡切换事件调用
 */
extern int modem_mgtlayer_switch_usim(modem_t *modem);

/*
 * 网络参数初始化，APN，Profile等信息
 */
extern int modem_mgtlayer_network_setup(modem_t *modem);

/*
 * 网络激活，完成拨号上网
 */
extern int modem_mgtlayer_network_attach(modem_t *modem);

/*
 * 网络去激活，禁止拨号上网
 */
extern int modem_mgtlayer_network_unattach(modem_t *modem);

/*
 * 网络接通，通过DHCP获取IP地址信息
 */
extern int modem_mgtlayer_network_online(modem_t *modem);

/*
 * 网络去接通，释放DHCP获取的IP地址信息
 */
extern int modem_mgtlayer_network_offline(modem_t *modem);

/*
 * 网络状态检测事件，拨上号后检测
 */
extern int modem_mgtlayer_network_detection(modem_t *modem);

/*
 * 延时事件
 */
extern int modem_mgtlayer_delay(modem_t *modem);

/*
 * 重新拨号
 */
extern int modem_mgtlayer_redialog(modem_t *modem);

/*
 * 拨号
 */
extern int modem_mgtlayer_dialog(modem_t *modem);


/*
 * 短信事件
 */
extern int modem_mgtlayer_message(modem_t *modem);



#endif /* __MODEM_MGTLAYER_H__ */
