/*
 * platform.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef PLATFORM_H_
#define PLATFORM_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HAVE_PLCONFIG_H
#define HAVE_PLCONFIG_H
#endif




#define OS_LINUX
//#define OS_VXWORKS
//#define OS_OPENBSD
//#define OS_OTHER

//#define OS_PROCESS
#define OS_THREAD

#define OS_STACK	1
#define IPCOM_STACK	2


//#define OS_PL_LOG_DEBUG_DETIAL

//#define _OS_DEBUG_
//#define _OS_SHELL_DEBUG_
//#define _OS_DEBUG_



#if defined( _OS_SHELL_DEBUG_)||defined( _OS_DEBUG_)
#define OS_DEBUG(format, ...) printf (format, ##__VA_ARGS__)
#define OS_SHELL_DEBUG(format, ...) printf (format, ##__VA_ARGS__)
#define OS_SERVICE_DEBUG(format, ...) printf (format, ##__VA_ARGS__)
#else
#define OS_DEBUG(format, ...)
#define OS_SHELL_DEBUG(format, ...)
#define OS_SERVICE_DEBUG(format, ...)
#endif


#include "product.h"


#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_H_ */
