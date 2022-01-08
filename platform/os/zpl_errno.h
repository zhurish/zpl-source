/*
 * zpl_errno.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_ERRNO_H__
#define __ZPL_ERRNO_H__

#ifdef __cplusplus
extern "C" {
#endif

enum zpl_errno 
{
  	OK  = 0,
  	ERROR = -1,
  	OS_TIMEOUT  = -2,		//超时
  	OS_CLOSE  = -3,			//关闭
  	OS_TRY_AGAIN  = -4,		//再一次
  	OS_CTRL_X  = -5,		//CTRL 信号
  	OS_EXIST		= -100,	//已存在
  	OS_NOTEXIST  = -101,	//不存在

  	ZPL_OK  = 0,
  	ZPL_ERROR = -1,
  	ZPL_OS_TIMEOUT  = -2,
	ZPL_OS_CLOSE  = -3,
  	ZPL_OS_TRY_AGAIN  = -4,
  	ZPL_OS_CTRL_X  = -5,
  	ZPL_OS_EXIST		= -100,
  	ZPL_OS_NOTEXIST  = -101,

	ZPL_ERRNO_EPERM = -100001,	/* Operation not permitted */
	ZPL_ERRNO_ENOENT = -100002,	/* No such file or directory */
	ZPL_ERRNO_ESRCH = -100003,	/* No such process */
	ZPL_ERRNO_EINTR = -100004,	/* Interrupted system call */
	ZPL_ERRNO_EIO = -100005,	/* I/O error */
	ZPL_ERRNO_ENXIO = -100006,	/* No such device or address */
	ZPL_ERRNO_E2BIG = -100007,	/* Argument list too long */
	ZPL_ERRNO_ENOEXEC = -100008,	/* Exec format error */
	ZPL_ERRNO_EBADF = -100009,	/* Bad file number */
	ZPL_ERRNO_ECHILD = -100010,	/* No child processes */
	ZPL_ERRNO_EAGAIN = -100011,	/* Try again */
	ZPL_ERRNO_ENOMEM = -100012,	/* Out of memory */
	ZPL_ERRNO_EACCES = -100013,	/* Permission denied */
	ZPL_ERRNO_EFAULT = -100014,	/* Bad address */
	ZPL_ERRNO_ENOTBLK = -100015,	/* Block device required */
	ZPL_ERRNO_EBUSY = -100016,	/* Device or resource busy */
	ZPL_ERRNO_EEXIST = -100017,	/* File exists */
	ZPL_ERRNO_EXDEV = -100018,	/* Cross-device link */
	ZPL_ERRNO_ENODEV = -100019,	/* No such device */
	ZPL_ERRNO_ENOTDIR = -100020,	/* Not a directory */
	ZPL_ERRNO_EISDIR = -100021,	/* Is a directory */
	ZPL_ERRNO_EINVAL = -100022,	/* Invalid argument */
	ZPL_ERRNO_ENFILE = -100023,	/* File table overflow */
	ZPL_ERRNO_EMFILE = -100024,	/* Too many open files */
	ZPL_ERRNO_ENOTTY = -100025,	/* Not a typewriter */
	ZPL_ERRNO_ETXTBSY = -100026,	/* Text file busy */
	ZPL_ERRNO_EFBIG = -100027,	/* File too large */
	ZPL_ERRNO_ENOSPC = -100028,	/* No space left on device */
	ZPL_ERRNO_ESPIPE = -100029,	/* Illegal seek */
	ZPL_ERRNO_EROFS = -100030,	/* Read-only file system */
	ZPL_ERRNO_EMLINK = -100031,	/* Too many links */
	ZPL_ERRNO_EPIPE = -100032,	/* Broken pipe */
	ZPL_ERRNO_EDOM = -100033,	/* Math argument out of domain of func */
	ZPL_ERRNO_ERANGE = -100034,	/* Math result not representable */
};

#ifdef __cplusplus
}
#endif

#endif /* __ZPL_ERRNO_H__ */
