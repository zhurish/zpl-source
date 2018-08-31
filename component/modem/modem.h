/*
 * modem.h
 *
 *  Created on: Jul 24, 2018
 *      Author: zhurish
 */

#ifndef __MODEM_H__
#define __MODEM_H__

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

#define MODEM_IS_DEBUG(n) (modem_debug & MODEM_DEBUG_ ## n)
#define MODEM_ON_DEBUG(n) (modem_debug |= MODEM_DEBUG_ ## n)
#define MODEM_OFF_DEBUG(n) (modem_debug &= ~MODEM_DEBUG_ ## n)



#define MODEM_STRING_MAX	64
#define MTYPE_MODEM 		MTYPE_ZCLIENT
#define MTYPE_MODEM_CLIENT	MTYPE_ZCLIENT
#define ZLOG_MODEM			ZLOG_MASC


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

	BOOL				active;
	modem_dial_type		dialtype;
	modem_stack_type	ipstack;

	BOOL				bSecondary;

	char				apn[MODEM_STRING_MAX];

	u_int32				profile;

	char				svc[MODEM_STRING_MAX];				//service code

	char				pin[MODEM_STRING_MAX];

	char				puk[MODEM_STRING_MAX];

	modem_network_type	network;

	modem_machine		state;
	modem_machine		newstate;

	modem_event			event;
	modem_event			nextevent;

	int					uptime;		//network UP time
	int					downtime;	//network DOWN time

	/*
	 * for detection event
	 */
	int					dedelay;
	int					detime_base;
	int					detime_axis;
	/*
	 * for delay event
	 */
	int					delay;
	int					time_base;
	int					time_axis;

	void				*pppd;			// point to modem_pppd_t
	void				*dhcp;			// point to modem_dhcp_t

	char				serialname[MODEM_STRING_MAX];
	void				*serial;		// point to modem-serial <name>
	void				*client;		// point modem_client_t

	void				*ppp_serial;
	void				*dial_serial;
	void				*test_serial;

	void				*eth0;
	void				*eth1;
	void				*eth2;
	//void				*ifp;

	int					pid[MODEM_DIAL_MAX+1];
}modem_t;


typedef struct modem_main_s
{
	LIST	*list;
	void	*mutex;

}modem_main_t;

extern int modem_debug;

extern modem_main_t gModemmain;

typedef int (*modem_cb)(modem_t *, void *);



extern int modem_main_init(void);
extern int modem_main_exit(void);

extern int modem_main_trywait(int);

extern int modem_main_add_api(char *name);
extern modem_t * modem_main_lookup_api(char *name);
extern int modem_main_del_api(char *name);

extern int modem_main_callback_api(modem_cb cb, void *pVoid);

extern int modem_main_bind_api(char *name, char *serialname);
extern int modem_main_unbind_api(char *name, char *serialname);

/*
extern int modem_interface_bind_api(char *name, char *ifname);
extern int modem_interface_unbind_api(char *name);
*/

extern int modem_module_init(void);
extern int modem_main_process(void *pVoid);


extern int modem_interface_add(modem_t *modem, char *name);
extern int modem_serial_interface_add(modem_t *modem, char *name);

#ifdef __MODEM_DEBUG
extern void modem_debug_printf(void *fp,char *func, int line, const char *format, ...);
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


#endif /* __MODEM_H__ */
