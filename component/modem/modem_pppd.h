/*
 * modem_pppd.h
 *
 *  Created on: Jul 27, 2018
 *      Author: zhurish
 */

#ifndef __MODEM_PPPD_H__
#define __MODEM_PPPD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "os_list.h"
#include "modem.h"


//#define _MODEM_PPPD_DEBUG

#ifdef _MODEM_PPPD_DEBUG
#define MODEM_PPPD_PEERS_BASE	"/etc/ppp/peers/"
#define MODEM_PPPD_BASE			"/etc/ppp/"
#else
#define MODEM_PPPD_PEERS_BASE	"/etc/ppp/peers/"
#define MODEM_PPPD_BASE			"/etc/ppp/"
#endif
#define MODEM_PPPD_CONNECT		"-chat-connect"
#define MODEM_PPPD_DISCONNECT	"-chat-disconnect"
#define MODEM_PPPD_SECRETS		"-secrets"
#define MODEM_PPPD_OPTIONS		"-options"

#define PPPD_OPTIONS_MAX	64


typedef struct pppd_options_s
{
	/*
	 * PPP options
	 */
	ospl_bool hide_password;
        //在PAP包中隐藏密码
    ospl_bool show_password;
        //在日志消息文件中，显示密码
    ospl_bool	detach;

	ospl_uint32 idle;// <n>
        //当连接空闲超过n秒时，PPPD将断开连接。
	ospl_uint32 holdoff;// <n>
        //指定连接结束后，多久才能开始下次连接。
	ospl_uint32 connect_delay;// <n>
        //connect后的等待时间，单位毫秒，默认值1000
    ospl_uint32 active_filter;// <filter_expression>
        //包过滤器

    ospl_bool persist;
    ospl_uint32 maxfail;// <n>
        //连续失败n次后，结束。0表示无限制，默认值为10

    ospl_bool noipx;
        //禁用IPXCP和IPX协议
    ospl_bool ip;
        //禁止IP地址协商
    ospl_bool noip;
        //禁止IPCP协商和IP通信

    ospl_bool mn;
        //禁止魔法数字协商。使用此选项后，PPPD无法检测到环回线路
    ospl_bool pc;
        //禁止protocol field compression协商
    ospl_bool vj;
        //禁止使用Van Jacobson式的IP头compression协商

    ospl_bool noipdefault;
        //peer通过IPCP协商提供本地IP地址
    ospl_bool passive;
        //使能LCP中的"passive"选项。此时，PPPD将尝试发起一个连接，如果没有接收到peer的回复，将会被动的等待
    ospl_bool silent;
        //PPPD只会在收到一个peer的LCP包后，才会发起连接
    ospl_bool all;
        //不允许或请求关于LCP和IPCP的任何协商选项
    ospl_bool ac;
        //禁止地址/控制的compression协商
    ospl_bool am;
        //禁止使用异步的字符映射协商
    ospl_bool proxyarp;
        //在系统的ARP表中，添加一个入口，包括peer的IP地址和以太网地址
    ospl_bool login;
        //使用系统的密码数据库来认证peer使用的PAP

    ospl_bool asyncmap;
        //使用异步的字符映射（32位）。如\x01'使用0x00000001表示，'\x1f'使用0x80000000表示
    ospl_bool auth;
        //要求peer在发送或接收网络包时，必须做自我认证。该选项在未来可能是PPPD的标准，请不要禁用。若想禁用一些特殊的peer，请使用call
    char crtscts[PPPD_OPTIONS_MAX];
        //使用硬件流控制来控制串口的数据流向，如RTS/CTS
    ospl_bool xonxoff;
        //使用软件流控制来控制串口的数据流向，如RTS/CTS
    char escape[PPPD_OPTIONS_MAX];
        //指定传输时，需要避免的某些字符
    ospl_bool local;
        //不使用调制解调器的控制线
    ospl_bool modem;
        //使用调制解调器的控制线
    ospl_bool lock;
        //指定PPPD使用一个UUCP式的锁，来访问一个独占式串行设备
	ospl_bool	debug;
	char	logfile[MODEM_STRING_MAX];

	/*
	 * IP options
	 */
	ospl_uint32 mru;
	ospl_uint32 mtu;
	ospl_uint32 netmask;
        //设置掩码
    ospl_uint32 ms_dns[2];
    ospl_uint32 demand;
        //根据需要启动连接，例如出现数据流量时。当使用该选项时，必须通过命令行或options文件指定远程IP地址。PPPD将会为IP流量初始化并使能网卡。当有数据流量时，PPPD将会连接到peer，并且执行协商、认证等，然后传输数据。
    char domain[PPPD_OPTIONS_MAX];// <d>
        //附加域名到主机名，用于认证。如获取到的主机名为porsche，而完整的域名是porsche.qsa.com,则需要指定"domain qsa.com"

    char name[PPPD_OPTIONS_MAX];// <n>
        //为认证设置本地系统的名字
    char usehostname[PPPD_OPTIONS_MAX];
        //优先使用hostname作为本地系统的名字
	char remotename[PPPD_OPTIONS_MAX];// <n>
        //为认证设置远程系统的名字
	ospl_bool defaultroute;
	ospl_bool usepeerdns;


	/*
	 * LCP options
	 */
    ospl_uint32 lcp_echo_interval; //n
        //若使用该选项，PPPD将每隔n秒发送一个LCP回显请求帧到peer。一般情况下，peer会回复一个LCP回显应答。该选项可以和lcp-echo-failure一起使用，用于检测peer断开的情况
    ospl_uint32 lcp_echo_failure; //n
        //若使用该选项，当PPPD发送LCP回显请求帧后，未收到LCP回显应答，将认为peer已经死亡。
    ospl_uint32 lcp_restart; //<n>
        //设置LCP传送超时重启间隔，默认值3秒
    ospl_uint32 lcp_max_terminate; //<n>
        //设置LCP终止请求的最大传送次数，默认值3秒
    ospl_uint32 lcp_max_configure; //<n>
        //设置LCP配置请求的最大传送次数，默认值10秒
    ospl_uint32 lcp_max_failure; //<n>
        //设置在开始发送配置拒绝前，收到的最大配置NAKs返回，默认值10次

	/*
	 * IPCP options
	 */
    ospl_uint32 ipcp_restart; //<n>
        //设置IPCP传送超时重启间隔，默认值3秒
    ospl_uint32 ipcp_max_terminate;// <n>
        //设置IPCP终止请求的最大传送次数，默认值3秒
    ospl_uint32 ipcp_max_configure;// <n>
        //设置IPCP配置请求的最大传送次数，默认值10秒
    ospl_uint32 ipcp_accept_local;
        //若设置了该选项，即使已指定了本地IP地址，PPPD也会会接受peer关于服务端本地IP地址的意见
    ospl_uint32 ipcp_accept_remote;
        //若设置了该选项，即使已指定了远程IP地址，PPPD也会会接受peer关于服务端远程IP地址的意见

	/*
	 * PAP options
	 */
    ospl_bool pap;
    ospl_uint32 pap_restart;// <n>
        //设置PAP传送超时重启间隔，默认值3秒
    ospl_uint32 pap_max_authreq;// <n>
        //设置PAP认证请求的最大传送次数，默认值10
    ospl_uint32 pap_timeout;
        //PPPD允许peer PAP认证的最大时间，0表示无限制

	/*
	 * CHAP options
	 */
    ospl_bool chap;
    ospl_uint32 chap_restart;
        //设置CHAP传送超时重启间隔，默认值3秒
    ospl_uint32 chap_max_challenge;
        //设置CHAPchallenge的最大传送次数，默认值10
    ospl_uint32 chap_interval;// <n>
        //若设置了该选项，PPPD将每隔n秒rechallenge peer

}pppd_options_t;

typedef struct modem_pppd_s
{
	NODE			node;
	ospl_bool			active;
	void			*modem;

	// PPPD options
	pppd_options_t 	pppd_options;

	char			dial_name[MODEM_STRING_MAX];
	char			connect[MODEM_STRING_MAX];
	char			disconnect[MODEM_STRING_MAX];
	char			secrets[MODEM_STRING_MAX];
	char			option[MODEM_STRING_MAX];

	ospl_uint32			checksum;
	ospl_uint32			t_time;

	ospl_bool			linkup;
	ospl_uint32 			taskid;
}modem_pppd_t;



typedef struct modem_main_pppd_s
{
	LIST	*list;
	void	*mutex;

}modem_main_pppd_t;

typedef int (*modem_pppd_cb)(modem_pppd_t *, void *);


extern int modem_pppd_init(void);
extern int modem_pppd_exit(void);
extern int modem_pppd_add_api(modem_t *);
extern int modem_pppd_del_api(modem_pppd_t *);
extern int modem_pppd_lookup_api(void *);
extern int modem_pppd_update_api(modem_pppd_t *);
extern int modem_pppd_set_api(modem_t *, pppd_options_t *);
extern int modem_pppd_callback_api(modem_pppd_cb , void *);

extern ospl_bool modem_pppd_islinkup(modem_t *);
extern ospl_bool modem_pppd_isconnect(modem_t *);
extern int modem_pppd_connect(modem_t *);
extern int modem_pppd_disconnect(modem_t *);



#ifdef _MODEM_PPPD_DEBUG
#define MODEM_PPPD_DEBUG(fmt,...)	modem_debug_printf(stderr, __func__, __LINE__,fmt, ##__VA_ARGS__)
#define MODEM_PPPD_WARN(fmt,...)		modem_debug_printf(stderr, __func__, __LINE__,fmt, ##__VA_ARGS__)
#define MODEM_PPPD_ERROR(fmt,...)	modem_debug_printf(stderr, __func__, __LINE__,fmt, ##__VA_ARGS__)
#else
#define MODEM_PPPD_DEBUG(fmt,...)
#define MODEM_PPPD_WARN(fmt,...)
#define MODEM_PPPD_ERROR(fmt,...)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __MODEM_PPPD_H__ */
