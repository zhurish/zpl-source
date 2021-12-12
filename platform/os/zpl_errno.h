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
  	OS_TIMEOUT  = -2,
  	OS_CLOSE  = -3,
  	OS_TRY_AGAIN  = -4,
  	OS_CTRL_X  = -5,
  	OS_EXIST		= -100,
  	OS_NOTEXIST  = -101,

  	ZPL_OK  = 0,
  	ZPL_ERROR = -1,
  	ZPL_OS_TIMEOUT  = -2,
	ZPL_OS_CLOSE  = -3,
  	ZPL_OS_TRY_AGAIN  = -4,
  	ZPL_OS_CTRL_X  = -5,
  	ZPL_OS_EXIST		= -100,
  	ZPL_OS_NOTEXIST  = -101,

	ZPL_ERRNO_EPERM = -1001,	/* Operation not permitted */
	ZPL_ERRNO_ENOENT = -1002,	/* No such file or directory */
	ZPL_ERRNO_ESRCH = -1003,	/* No such process */
	ZPL_ERRNO_EINTR = -1004,	/* Interrupted system call */
	ZPL_ERRNO_EIO = -1005,	/* I/O error */
	ZPL_ERRNO_ENXIO = -1006,	/* No such device or address */
	ZPL_ERRNO_E2BIG = -1007,	/* Argument list too long */
	ZPL_ERRNO_ENOEXEC = -1008,	/* Exec format error */
	ZPL_ERRNO_EBADF = -1009,	/* Bad file number */
	ZPL_ERRNO_ECHILD = -1010,	/* No child processes */
	ZPL_ERRNO_EAGAIN = -1011,	/* Try again */
	ZPL_ERRNO_ENOMEM = -1012,	/* Out of memory */
	ZPL_ERRNO_EACCES = -1013,	/* Permission denied */
	ZPL_ERRNO_EFAULT = -1014,	/* Bad address */
	ZPL_ERRNO_ENOTBLK = -1015,	/* Block device required */
	ZPL_ERRNO_EBUSY = -1016,	/* Device or resource busy */
	ZPL_ERRNO_EEXIST = -1017,	/* File exists */
	ZPL_ERRNO_EXDEV = -1018,	/* Cross-device link */
	ZPL_ERRNO_ENODEV = -1019,	/* No such device */
	ZPL_ERRNO_ENOTDIR = -1020,	/* Not a directory */
	ZPL_ERRNO_EISDIR = -1021,	/* Is a directory */
	ZPL_ERRNO_EINVAL = -1022,	/* Invalid argument */
	ZPL_ERRNO_ENFILE = -1023,	/* File table overflow */
	ZPL_ERRNO_EMFILE = -1024,	/* Too many open files */
	ZPL_ERRNO_ENOTTY = -1025,	/* Not a typewriter */
	ZPL_ERRNO_ETXTBSY = -1026,	/* Text file busy */
	ZPL_ERRNO_EFBIG = -1027,	/* File too large */
	ZPL_ERRNO_ENOSPC = -1028,	/* No space left on device */
	ZPL_ERRNO_ESPIPE = -1029,	/* Illegal seek */
	ZPL_ERRNO_EROFS = -1030,	/* Read-only file system */
	ZPL_ERRNO_EMLINK = -1031,	/* Too many links */
	ZPL_ERRNO_EPIPE = -1032,	/* Broken pipe */
	ZPL_ERRNO_EDOM = -1033,	/* Math argument out of domain of func */
	ZPL_ERRNO_ERANGE = -1034,	/* Math result not representable */
};

#ifdef __cplusplus
}
#endif

#endif /* __ZPL_ERRNO_H__ */
