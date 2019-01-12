/*
 * voip_error.h
 *
 *  Created on: 2019年1月2日
 *      Author: DELL
 */

#ifndef __VOIP_ERROR_H__
#define __VOIP_ERROR_H__

enum
{
	VOIP_E_NONE,
	//物管测
	VOIP_E_NO_ROOM,			//无房间号
	VOIP_E_NO_PHONE,		//无电话号码
	VOIP_E_WARN_PHONE,		//号码错误

	//SIP测
};


extern int voip_set_errno(int err);
extern int voip_get_errno(int *err);
extern int voip_errno();


#endif /* __VOIP_ERROR_H__ */
