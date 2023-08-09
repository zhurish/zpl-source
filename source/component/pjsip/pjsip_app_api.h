/*
 * pjsip_app_api.h
 *
 *  Created on: Jun 15, 2019
 *      Author: zhurish
 */

#ifndef __PJSIP_APP_API_H__
#define __PJSIP_APP_API_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "pjsua_app_common.h"


#define PJSIP_NUMBER_MAX		32
#define PJSIP_USERNAME_MAX		32
#define PJSIP_PASSWORD_MAX		32
#define PJSIP_ADDRESS_MAX		32
#define PJSIP_FILE_MAX			64
#define PJSIP_DATA_MAX			128
#define PJSIP_CODEC_MAX			PJAPP_CODEC_MAX


#define PJSIP_ENABLE_DEFAULT			zpl_true
#define PJSIP_PORT_DEFAULT				5060
#define PJSIP_PROXY_PORT_DEFAULT		5060
#define PJSIP_100_REL_DEFAULT			zpl_false
#define PJSIP_PROTO_DEFAULT				PJSIP_PROTO_UDP
#define PJSIP_DTMF_DEFAULT				PJSIP_DTMF_RFC2833
#define PJSIP_EXPIRES_DEFAULT			360
#define PJSIP_DEFAULT_CLOCK_RATE		8000
#define PJSIP_RTP_PORT_DEFAULT			8888

#ifdef PJSUA_DEFAULT_CLOCK_RATE
#undef PJSUA_DEFAULT_CLOCK_RATE
#define PJSUA_DEFAULT_CLOCK_RATE PJSIP_DEFAULT_CLOCK_RATE
#endif

typedef enum pjsip_dtmf_s
{
	PJSIP_DTMF_INFO = 1,
	PJSIP_DTMF_RFC2833,
	PJSIP_DTMF_INBAND,
}pjsip_dtmf_t;

typedef enum pjsip_transport_s
{
	PJSIP_PROTO_UDP,
	PJSIP_PROTO_TCP,
	PJSIP_PROTO_TLS,
	PJSIP_PROTO_DTLS,
}pjsip_transport_t;

typedef enum pjsip_user_mode_s
{
	PJSIP_USER_NONE,
	PJSIP_USER_STANDBY,
	PJSIP_USER_SWITCH,
}pjsip_user_mode_t;

typedef enum pjsip_connect_mode_s
{
	PJSIP_CONNECT_NONE,
	PJSIP_CONNECT_STANDBY,
	PJSIP_CONNECT_SWITCH,
}pjsip_connect_mode_t;

typedef enum pjsip_reg_proxy_s
{
	PJSIP_REGISTER_NONE,
	PJSIP_REGISTER_NO_PROXY,
	PJSIP_REGISTER_OUTBOUND_PROXY,
	PJSIP_REGISTER_ACC_ONLY,
	PJSIP_REGISTER_ALL
}pjsip_reg_proxy_t;

typedef enum pjsip_srtp_s
{
	PJSIP_SRTP_DISABLE,
	PJSIP_SRTP_OPTIONAL,
	PJSIP_SRTP_MANDATORY,
	PJSIP_SRTP_OPTIONAL_DUP,
}pjsip_srtp_t;

typedef enum pjsip_srtp_sec_s
{
	PJSIP_SRTP_SEC_NO,
	PJSIP_SRTP_SEC_TLS,
	PJSIP_SRTP_SEC_SIPS,
}pjsip_srtp_sec_t;

typedef enum pjsip_timer_s
{
	PJSIP_TIMER_INACTIVE,
	PJSIP_TIMER_OPTIONAL,
	PJSIP_TIMER_MANDATORY,
	PJSIP_TIMER_ALWAYS,
}pjsip_timer_t;

typedef enum pjsip_echo_mode_s
{
	PJSIP_ECHO_DISABLE,
	PJSIP_ECHO_DEFAULT,
	PJSIP_ECHO_SPEEX,
	PJSIP_ECHO_SUPPRESSER,
	PJSIP_ECHO_WEBRTXC,
}pjsip_echo_mode_t;

typedef enum pjsip_srtp_keying_s
{
	PJSIP_SRTP_KEYING_SDES,
	PJSIP_SRTP_KEYING_DTLS,
}pjsip_srtp_keying_t;

typedef enum pjsip_accept_redirect_s
{
	PJSIP_ACCEPT_REDIRECT_REJECT,
	PJSIP_ACCEPT_REDIRECT_FOLLOW,
	PJSIP_ACCEPT_REDIRECT_FOLLOW_REPLACE,
	PJSIP_ACCEPT_REDIRECT_ASK
}pjsip_accept_redirect_t;



typedef enum pjsip_state_s
{
	//信息响应
	PJSIP_STATE_100 	= 100,			//100试呼叫（Trying）
	PJSIP_STATE_180 	= 180,			//180振铃（Ringing）
	PJSIP_STATE_181 	= 181,			//181呼叫正在前转（Call is Being Forwarded）
	PJSIP_STATE_182 	= 182,			//182排队
	//成功响应
	PJSIP_STATE_200	= 200,			//200成功响应（OK）

	//重定向响应
	PJSIP_STATE_300	= 300,			//300多重选择
	PJSIP_STATE_301	= 301,			//永久迁移
	PJSIP_STATE_302	= 302,			//302临时迁移（Moved Temporarily）
	PJSIP_STATE_303	= 303,			//见其它
	PJSIP_STATE_305	= 305,			//使用代理
	PJSIP_STATE_380	= 380,			//代换服务

	//客户出错
	PJSIP_STATE_400 	= 400,		//400错误请求（Bad Request）
	PJSIP_STATE_401 	= 401,		//401未授权（Unauthorized）
	PJSIP_STATE_402 	= 402,		//要求付款
	PJSIP_STATE_403 	= 403,		//403禁止（Forbidden）
	PJSIP_STATE_404	= 404,		//404用户不存在（Not Found）
	PJSIP_STATE_405	= 405,		//不允许的方法
	PJSIP_STATE_406	= 406,		//不接受
	PJSIP_STATE_407	= 407,		//要求代理权
	PJSIP_STATE_408 	= 408,		//408请求超时（Request Timeout）
	PJSIP_STATE_410	= 410,		//消失
	PJSIP_STATE_413	= 413,		//请求实体太大
	PJSIP_STATE_414 	= 414,		//请求URI太大
	PJSIP_STATE_415 	= 414,		//不支持的媒体类型
	PJSIP_STATE_416 	= 416,		//不支持的URI方案
	PJSIP_STATE_420	= 420,		//分机无人接听
	PJSIP_STATE_421	= 421,		//要求转机
	PJSIP_STATE_423 	= 423,		//间隔太短
	PJSIP_STATE_480	= 480,		//480暂时无人接听（Temporarily Unavailable）
	PJSIP_STATE_481 	= 481,		//呼叫腿/事务不存在

	PJSIP_STATE_482 	= 482,		//相环探测
	PJSIP_STATE_483 	= 483,		//跳频太高
	PJSIP_STATE_484 	= 484,		//地址不完整
	PJSIP_STATE_485 	= 485,		//不清楚
	PJSIP_STATE_486	= 486,		//486线路忙（Busy Here）

	PJSIP_STATE_487 	= 487,		//终止请求
	PJSIP_STATE_488 	= 488,		//此处不接受
	PJSIP_STATE_491 	= 491,		//代处理请求
	PJSIP_STATE_493 	= 493,		//难以辨认

	//服务器出错
	PJSIP_STATE_500	= 500,		//内部服务器错误
	PJSIP_STATE_501	= 501,		//没实现的
	PJSIP_STATE_502	= 502,		//无效网关
	PJSIP_STATE_503	= 503,		//不提供此服务
	PJSIP_STATE_504 	= 504,		//504服务器超时（Server Time-out）
	PJSIP_STATE_505	= 505,		//SIP版本不支持
	PJSIP_STATE_513	= 513,		//消息太长

	//全局故障
	PJSIP_STATE_600 = 600,		//600全忙（Busy Everywhere）
	PJSIP_STATE_603 = 603,		//拒绝
	PJSIP_STATE_604 = 604,		//都不存在
	PJSIP_STATE_606 = 606,		//不接受
}pjsip_state_t;


/************************************************************************/
/************************************************************************/

/************************************************************************/
/************************************************************************/
/************************************************************************/
char *pjapp_cfg_dtmf_name(pjsip_dtmf_t );
char *pjapp_cfg_transport_name(pjsip_transport_t );
char *pjapp_cfg_reg_proxy_name(pjsip_reg_proxy_t );
char *pjapp_cfg_srtp_name(pjsip_srtp_t );
char *pjapp_cfg_srtp_sec_name(pjsip_srtp_sec_t );
char *pjapp_cfg_timer_name(pjsip_timer_t );
char *pjapp_cfg_echo_mode_name(pjsip_echo_mode_t );
char *pjapp_cfg_accept_redirect_name(pjsip_accept_redirect_t );
char *pjapp_cfg_register_state_name(pjapp_register_state_t );
char *pjapp_cfg_connect_state_name(pjapp_connect_state_t );
char *pjapp_cfg_call_state_name(pjapp_call_state_t );
/************************************************************************/

/************************************************************************/

/************************************************************************/
int pjapp_cfg_account_set_api(pjsua_acc_id id, void *p);

/************************************************************************/
/************************************************************************/
int pjapp_cfg_app_add_acc(char *sip_url, char *sip_srv, char *realm,
		char *user, char *pass, pjsua_acc_id *accid);
int pjapp_cfg_app_del_acc(pjsua_acc_id accid);
int pjapp_cfg_app_mod_acc(pjsua_acc_id accid, char *sip_url, char *sip_srv, char *realm,
		char *user, char *pass);
int pjapp_cfg_app_select_acc(pjsua_acc_id accid, pj_uint32_t type);
int pjapp_cfg_app_reg_acc(pj_bool_t reg);
int pjapp_cfg_app_list_acc(pjsua_acc_id accid);
/************************************************************************/
/************************************************************************/
int pjapp_cfg_app_start_call(pjsua_acc_id accid, char *num, pjsua_call_id *callid);
int pjapp_cfg_app_stop_call(pjsua_call_id callid, pj_bool_t all);
int pjapp_cfg_app_answer_call(pjsua_call_id callid, pj_uint32_t st_code);
int pjapp_cfg_app_hold_call(pjsua_call_id callid);
int pjapp_cfg_app_reinvite_call(pjsua_call_id callid);
int pjapp_cfg_app_dtmf_call(pjsua_call_id callid, pj_uint32_t type, pj_uint32_t code);
int pjapp_cfg_app_select_call(pjsua_call_id callid, pj_uint32_t type);
/************************************************************************/

/************************************************************************/


#ifdef __cplusplus
}
#endif

#endif /* __PJSIP_APP_API_H__ */
