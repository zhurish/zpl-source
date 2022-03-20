/*
 * zpl_errno.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_ERRNO_H__
#define __ZPL_ERRNO_H__

#ifdef __cplusplus
extern "C"
{
#endif

enum zpl_errno
{
	OK = 0,
	ERROR = -1,
	OS_TIMEOUT = -500001,	   //超时
	OS_CLOSE = -500002,		   //关闭
	OS_TRY_AGAIN = -500003,	   //再一次
	OS_CTRL_X = -500004,		   // CTRL 信号
	OS_EXIST = -500005,	   //已存在
	OS_NOTEXIST = -500006,	   //不存在
	OS_UNKNOWN_CMD = -500007, //
	OS_NO_SDKSPUUORT = -500008,

	ZPL_OK = 0,
	ZPL_ERROR = -1,
	ZPL_OS_TIMEOUT = -500001,
	ZPL_OS_CLOSE = -500002,
	ZPL_OS_TRY_AGAIN = -500003,
	ZPL_OS_CTRL_X = -500004,
	ZPL_OS_EXIST = -500005,
	ZPL_OS_NOTEXIST = -500006,
	ZPL_UNKNOWN_CMD = -500007,
	ZPL_NO_SDKSPUUORT = -500008,
	ZPL_ERRNO_UNKNOW
};

extern void zpl_errno_set(int zpl_errno);
extern int zpl_errno_get(void);
extern const char *zpl_strerror(int zpl_errno);
/* Safe version of strerror -- never returns NULL. */
extern const char *ipstack_strerror(int errnum);
const char *ipstack_ipcomstrerror(int errnum);



#ifdef __cplusplus
}
#endif

#endif /* __ZPL_ERRNO_H__ */
