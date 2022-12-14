/*
 * os_def.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __OS_DEF_H__
#define __OS_DEF_H__

#ifndef ZPL_BUILD_LINUX
#define ZPL_BUILD_LINUX
#endif
//#define OS_VXWORKS
//#define OS_OPENBSD
//#define OS_OTHER

//#define OS_PROCESS


#if defined( _OS_SHELL_DEBUG_)||defined( _OS_DEBUG_)
#define OS_DEBUG(format, ...) printf (format, ##__VA_ARGS__)
#define OS_SHELL_DEBUG(format, ...) printf (format, ##__VA_ARGS__)
#define OS_SERVICE_DEBUG(format, ...) printf (format, ##__VA_ARGS__)
#else
#define OS_DEBUG(format, ...)
#define OS_SHELL_DEBUG(format, ...)
#define OS_SERVICE_DEBUG(format, ...)
#endif



#endif /* __OS_DEF_H__ */
