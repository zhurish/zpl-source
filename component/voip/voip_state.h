/*
 * voip_state.h
 *
 *  Created on: Dec 31, 2018
 *      Author: zhurish
 */

#ifndef __VOIP_STATE_H__
#define __VOIP_STATE_H__

/*
typedef enum voip_state_s
{
	VOIP_STATE_NONE,
	//注册状态
	VOIP_STATE_UNREGISTER,			//未注册
	VOIP_STATE_REGISTER_FAILED,		//注册失败
	VOIP_STATE_REGISTER_TIMEOUT,	//注册超时
	VOIP_STATE_REGISTER_ERROR,		//注册错误
	VOIP_STATE_REGISTER_SUCCESS,

	//呼叫状态
	VOIP_STATE_CALL_IDLE,		//空闲
	VOIP_STATE_CALL_FAILED,		//呼叫失败
	VOIP_STATE_CALL_TIMEOUT,	//呼叫超时
	VOIP_STATE_CALL_NORESPONE,	//呼叫无人接听
	VOIP_STATE_CALL_ERROR,		//呼叫错误
	VOIP_STATE_CALL_STOP,		//呼叫挂断
	VOIP_STATE_CALL_SUCCESS,
	VOIP_STATE_CALL_BUSY,		//呼叫忙
	VOIP_STATE_CALLING,			//呼叫中

	VOIP_STATE_TALK,			//通话中
	VOIP_STATE_BYE,			//
}voip_state_t;



extern voip_state_t voip_state_get();
extern int voip_state_set(voip_state_t state);
*/

#endif /* __VOIP_STATE_H__ */
