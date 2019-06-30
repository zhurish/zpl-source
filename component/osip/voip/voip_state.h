/*
 * voip_state.h
 *
 *  Created on: Dec 31, 2018
 *      Author: zhurish
 */

#ifndef __VOIP_STATE_H__
#define __VOIP_STATE_H__

//#include "voip_app.h"

typedef enum sip_state_s
{
	//注册状态		SIP层面
	SIP_STATE_NONE,
	SIP_STATE_UNREGISTER,			//未注册
	SIP_STATE_REGISTER_FAILED,		//注册失败
	SIP_STATE_REGISTER_SUCCESS,

/*
	//APP通话状态	APP层面
	SIP_STATE_TALK_IDLE,			//通话空闲
	SIP_STATE_TALK_FAILED,			//通话建立失败
	SIP_STATE_TALK_SUCCESS,			//通话建立
	SIP_STATE_TALK_RUNNING,			//通话中
*/

	//媒体状态		媒体层面
	SIP_STATE_MEDIA_IDLE,			//媒体空闲
	SIP_STATE_MEDIA_CONNECTED,		//媒体建立


	//SIP呼叫状态		SIP层面
	SIP_STATE_CALL_IDLE,			//呼叫空闲
	SIP_STATE_CALL_TRYING,
	SIP_STATE_CALL_RINGING,
	SIP_STATE_CALL_PICKING,
	SIP_STATE_CALL_FAILED,			//呼叫失败
	SIP_STATE_CALL_SUCCESS,			//呼叫建立
	SIP_STATE_CALL_CANCELLED,			//
	SIP_STATE_CALL_CLOSED,				//
	SIP_STATE_CALL_RELEASED,


	//信息响应
	SIP_STATE_100 	= 100,			//100试呼叫（Trying）
	SIP_STATE_180 	= 180,			//180振铃（Ringing）
	SIP_STATE_181 	= 181,			//181呼叫正在前转（Call is Being Forwarded）
	SIP_STATE_182 	= 182,			//182排队
	//成功响应
	SIP_STATE_200	= 200,			//200成功响应（OK）

	//重定向响应
	SIP_STATE_300	= 300,			//300多重选择
	SIP_STATE_301	= 301,			//永久迁移
	SIP_STATE_302	= 302,			//302临时迁移（Moved Temporarily）
	SIP_STATE_303	= 303,			//见其它
	SIP_STATE_305	= 305,			//使用代理
	SIP_STATE_380	= 380,			//代换服务

	//客户出错
	SIP_STATE_400 	= 400,		//400错误请求（Bad Request）
	SIP_STATE_401 	= 401,		//401未授权（Unauthorized）
	SIP_STATE_402 	= 402,		//要求付款
	SIP_STATE_403 	= 403,		//403禁止（Forbidden）
	SIP_STATE_404	= 404,		//404用户不存在（Not Found）
	SIP_STATE_405	= 405,		//不允许的方法
	SIP_STATE_406	= 406,		//不接受
	SIP_STATE_407	= 407,		//要求代理权
	SIP_STATE_408 	= 408,		//408请求超时（Request Timeout）
	SIP_STATE_410	= 410,		//消失
	SIP_STATE_413	= 413,		//请求实体太大
	SIP_STATE_414 	= 414,		//请求URI太大
	SIP_STATE_415 	= 414,		//不支持的媒体类型
	SIP_STATE_416 	= 416,		//不支持的URI方案
	SIP_STATE_420	= 420,		//分机无人接听
	SIP_STATE_421	= 421,		//要求转机
	SIP_STATE_423 	= 423,		//间隔太短
	SIP_STATE_480	= 480,		//480暂时无人接听（Temporarily Unavailable）
	SIP_STATE_481 	= 481,		//呼叫腿/事务不存在

	SIP_STATE_482 	= 482,		//相环探测
	SIP_STATE_483 	= 483,		//跳频太高
	SIP_STATE_484 	= 484,		//地址不完整
	SIP_STATE_485 	= 485,		//不清楚
	SIP_STATE_486	= 486,		//486线路忙（Busy Here）

	SIP_STATE_487 	= 487,		//终止请求
	SIP_STATE_488 	= 488,		//此处不接受
	SIP_STATE_491 	= 491,		//代处理请求
	SIP_STATE_493 	= 493,		//难以辨认

	//服务器出错
	SIP_STATE_500	= 500,		//内部服务器错误
	SIP_STATE_501	= 501,		//没实现的
	SIP_STATE_502	= 502,		//无效网关
	SIP_STATE_503	= 503,		//不提供此服务
	SIP_STATE_504 	= 504,		//504服务器超时（Server Time-out）
	SIP_STATE_505	= 505,		//SIP版本不支持
	SIP_STATE_513	= 513,		//消息太长

	//全局故障
	SIP_STATE_600 = 600,		//600全忙（Busy Everywhere）
	SIP_STATE_603 = 603,		//拒绝
	SIP_STATE_604 = 604,		//都不存在
	SIP_STATE_606 = 606,		//不接受
}osip_state_t, media_state_t, osip_call_state_t, osip_call_error_t;



extern osip_call_error_t osip_call_error_get(void *priv);
extern int osip_call_error_set(void *osip, osip_call_error_t state);


extern osip_call_state_t osip_call_state_get(void *priv);
extern int osip_call_state_set(void *priv, osip_call_state_t state);

extern osip_state_t osip_register_state_get(void *priv);
extern int osip_register_state_set(void *priv, osip_state_t state);



extern media_state_t voip_media_state_get(void *priv);
extern int voip_media_state_set(void *priv, media_state_t state);

#endif /* __VOIP_STATE_H__ */
