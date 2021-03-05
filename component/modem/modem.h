/*
 * modem.h
 *
 *  Created on: Jul 24, 2018
 *      Author: zhurish
 */

#ifndef __MODEM_H__
#define __MODEM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "os_list.h"
#include "modem_enum.h"
#include "modem_machine.h"


#define __MODEM_DEBUG

#define MODEM_DEBUG_DRIVER  1
#define MODEM_DEBUG_CLIENT  2
#define MODEM_DEBUG_ATCMD  	4
#define MODEM_DEBUG_EVENT  	8
#define MODEM_DEBUG_MANAGE  0X10


#define MODEM_DEBUG_DETAIL  0X80000000

#define MODEM_IS_DEBUG(n) (modem_debug_conf & MODEM_DEBUG_ ## n)
#define MODEM_ON_DEBUG(n) (modem_debug_conf |= MODEM_DEBUG_ ## n)
#define MODEM_OFF_DEBUG(n) (modem_debug_conf &= ~MODEM_DEBUG_ ## n)



#define MODEM_STRING_MAX	64



#define MODEM_PRODUCT_NAME_MAX	64
#define MODEM_STRING_MAX		64

/*
 * detection time
 */
#define MODEM_DECHK_TIME	5		/* detection information before active network */

#define MODEM_DELAY_CHK_TIME	10		/* detection information before active network */
#define MODEM_DELAY_TIME		40		/* detection information after active network */



/*
 * modem profile <name>
 */
typedef struct modem_s
{
	NODE				node;
	char				name[MODEM_STRING_MAX];
	ospl_bool				bSecondary;
	ospl_bool				active;
	ospl_bool				proxy;
	modem_dial_type		dialtype;
	modem_stack_type	ipstack;

	char				apn[MODEM_STRING_MAX];

	ospl_uint32				profile;

	char				svc[MODEM_STRING_MAX];				//service code

	char				pin[MODEM_STRING_MAX];

	char				puk[MODEM_STRING_MAX];

	modem_network_type	network;

	void				*pppd;			// point to modem_pppd_t
	void				*dhcp;			// point to modem_dhcp_t

	char				serialname[MODEM_STRING_MAX];
	void				*serial;		// point to modem-serial
	void				*client;		// point modem_client_t
	void				*mutex;

	void				*ppp_serial;
	void				*dial_serial;
	void				*test_serial;

	void				*eth0;
	void				*eth1;
	void				*eth2;

	modem_machine		state;
	modem_machine		newstate;

	modem_event			event;
	modem_event			nextevent;

	ospl_uint32				uptime;		//network UP time
	ospl_uint32				downtime;	//network DOWN time

	/*
	 * for detection event
	 */
	ospl_uint32				dedelay;
	ospl_uint32				detime_base;
	ospl_uint32				detime_axis;
	/*
	 * for delay event
	 */
	ospl_uint32				delay;
	ospl_uint32				time_base;
	ospl_uint32				time_axis;

	void				*proxy_data;


	modem_event			a_event;
	ospl_uint32				t_time;
	ospl_uint32				checksum;

	ospl_pid_t					pid[MODEM_DIAL_MAX+1];
}modem_t;


typedef struct modem_main_s
{
	LIST	*list;
	void	*mutex;

}modem_main_t;

extern ospl_uint32 modem_debug_conf;

extern modem_main_t gModemmain;

typedef int (*modem_cb)(modem_t *, void *);



extern int modem_main_init(void);
extern int modem_main_exit(void);

extern int modem_main_trywait(ospl_uint32);

extern int modem_main_add_api(char *name);
extern modem_t * modem_main_lookup_api(char *name);
extern int modem_main_del_api(char *name);

extern int modem_main_callback_api(modem_cb cb, void *pVoid);

extern int modem_main_bind_api(char *name, char *serialname);
extern int modem_main_unbind_api(char *name, char *serialname);

extern int modem_main_lock(modem_t *modem);
extern int modem_main_unlock(modem_t *modem);

/*
extern int modem_interface_bind_api(char *name, char *ifname);
extern int modem_interface_unbind_api(char *name);
*/


extern int modem_module_init (void);
extern int modem_module_exit (void);
extern int modem_task_init (void);
extern int modem_task_exit (void);

extern int modem_main_process(void *pVoid);


extern int modem_interface_update_kernel(modem_t *modem, char *name);
extern int modem_serial_interface_update_kernel(modem_t *modem, char *name);
extern int modem_serial_devname_update_kernel(modem_t *modem, char *name);


extern int modem_bind_interface_update(modem_t *modem);



extern int modem_ansync_add(int (*cb)(void *), int fd, char *name);
extern int modem_ansync_del(int value);
extern int modem_ansync_timer_add(int (*cb)(void *), int fd, char *name);
extern int modem_ansync_timer_del(void *value);
/*
 * CMD
 */
extern void cmd_modem_init (void);
extern int modem_debug_config(struct vty *vty);



#ifdef __MODEM_DEBUG
extern void modem_debug_printf(void *fp,char *func, ospl_uint32 line, const char *format, ...);
#if 1
#define MODEM_DEBUG(fmt,...)	modem_debug_printf(stderr, __func__, __LINE__,fmt, ##__VA_ARGS__)
#define MODEM_WARN(fmt,...)		modem_debug_printf(stderr, __func__, __LINE__,fmt, ##__VA_ARGS__)
#define MODEM_ERROR(fmt,...)	modem_debug_printf(stderr, __func__, __LINE__,fmt, ##__VA_ARGS__)
#else
#define MODEM_DEBUG(fmt,...)
#define MODEM_WARN(fmt,...)
#define MODEM_ERROR(fmt,...)
#endif
#else
#define MODEM_DEBUG(fmt,...)
#define MODEM_WARN(fmt,...)
#define MODEM_ERROR(fmt,...)
#endif
/*
CDMA：                      码分多址
CDMA2000：             由TIA（电信行业协会）定义的3G CDMA解决方案
DCE：                         数字通讯设备,类似于传统的串口通讯中的MODEM。
DTE：                          数字终端设备,类似于传统的串口通讯中的计算机。
DTMF：                       双音多频
ESN: ：                       电子序列号
Handset(Path)：       连接普通的手持式电话接收机的音频通路,通常指双平衡电路。
Headset（Path）： 连接带MIC 的耳机的音频通路,通常指单端电路。
IMSI：                          国际移动设备标识号,由移动设备国家代码MCC（3 位）、网络服务商代码MNC(联通：00；                             移动：01)和设备标识号MIN共15 位号码组成。
IS-95：                        高通在1993年发布的第一代CDMA 标准。
IS-95A：                      在IS-95 的基础上增强了语音编码
IS-95B：                     在IS-95 的基础上增强了数据功能
IS-2000：                   是基于IS-95B 的第一代3G 标准
MCC：                        移动设备国家代码
MIN：                          移动设备标识号
MNC：                        移动网络代码
MS：                           移动设备,通常指手机
NID：                          网络ID 号
SID：                          系统ID 号
PSTN：                      公用电话交换网络,传统的电话网络。
RSSI：                       接收信号强度指示
SMS：                        短消息服务
V24-V25;V42：         是一种数据压缩法则。
 */


#ifdef __cplusplus
}
#endif

#endif /* __MODEM_H__ */
