/*
 * os_platform.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef OS_PLATFORM_H_
#define OS_PLATFORM_H_

#define OS_LINUX
//#define OS_VXWORKS
//#define OS_OPENBSD
//#define OS_OTHER

//#define OS_PROCESS
#define OS_THREAD


#include "pthread.h"
#include "os_memory.h"
#include <linux/version.h>
#ifndef RT_TABLE_MAIN
//#define RT_TABLE_MAIN 0
#endif
//#define USE_IPSTACK_KERNEL
#ifndef USE_IPSTACK_KERNEL
#define IPNET
#endif

#define OS_STACK	1
#define IPCOM_STACK	2
#define IF_IUSPV_SUPPORT //support sub interface, eg:0/1/1.22


//#define OS_PL_LOG_DEBUG_DETIAL

//#define _OS_DEBUG_
//#define _OS_SHELL_DEBUG_
//#define _OS_DEBUG_
#define FD_IS_STDOUT(f)	((f) < 3)

#ifndef min
#define min(a,b)	( (a) < (b)? (a):(b) )
#endif
#ifndef max
#define max(a,b)	( (a) > (b)? (a):(b) )
#endif
#ifndef MIN
#define MIN(a,b)	( (a) < (b)? (a):(b) )
#endif
#ifndef MAX
#define MAX(a,b)	( (a) > (b)? (a):(b) )
#endif

#ifndef INT_MAX_MIN_SPACE
#define INT_MAX_MIN_SPACE(v, m, M)	( ((v) > (m)) && ((v) < (M)) )
#endif

#ifndef NOT_INT_MAX_MIN_SPACE
#define NOT_INT_MAX_MIN_SPACE(v, m, M)	( ((v) < (m)) && ((v) > (M)) )
#endif

#if 0
typedef int 		(*FUNCPTR) (...);     /* ptr to function returning int */
typedef void 		(*VOIDFUNCPTR) (...); /* ptr to function returning void */
typedef double 		(*DBLFUNCPTR) (...);  /* ptr to function returning double*/
typedef float 		(*FLTFUNCPTR) (...);  /* ptr to function returning float */
#endif


#ifndef u_int8
typedef unsigned char u_int8;
#endif

#ifndef u_int16
typedef unsigned short u_int16;
#endif

#ifndef u_int32
typedef unsigned int u_int32;
#endif

#ifndef u_int64
//typedef unsigned long long int u_int64;
typedef __uint64_t u_int64;
#endif

#ifndef u_long
typedef unsigned long u_long;
#endif

#ifndef u_char
typedef unsigned char u_char;
#endif

#ifndef u_short
typedef unsigned short u_short;
#endif

#ifndef u_int
typedef unsigned int u_int;
#endif

#ifndef s_int8
typedef signed char s_int8;
#endif

#ifndef s_int16
typedef signed short s_int16;
#endif

#ifndef s_int32
typedef signed int s_int32;
#endif

#ifndef s_int64
//typedef long long int s_int64;
typedef __int64_t s_int64;
#endif

#ifndef s_long
typedef signed long s_long;
#endif

#ifndef s_char
typedef signed char s_char;
#endif

#ifndef s_short
typedef signed short s_short;
#endif

#ifndef s_int
typedef signed int s_int;
#endif

#ifndef llong
typedef long long llong;
#endif

#ifndef u_llong
typedef unsigned long long u_llong;
#endif

#ifndef s_float
typedef float s_float;
#endif

#ifndef s_double
typedef double s_double;
#endif


//#ifndef HAVE_OS_TRUE
typedef enum {
  TRUE  = 1,
  FALSE = 0,
} BOOL;
//#endif

//typedef int BOOL;

enum
{
  OK  = 0,
  ERROR = -1,
//#ifndef HAVE_OS_TIMEOUT
  OS_TIMEOUT  = -2,
//#endif
  OS_CTRL_X  = -3,
  OS_TRY_AGAIN  = -4,
  OS_EXIST		= -100,
  OS_NOTEXIST  = -101,
};

#define OS_WAIT_NO	0
#define OS_WAIT_FOREVER	-1



#if defined( _OS_SHELL_DEBUG_)||defined( _OS_DEBUG_)
#define OS_DEBUG(format, ...) printf (format, ##__VA_ARGS__)
#define OS_SHELL_DEBUG(format, ...) printf (format, ##__VA_ARGS__)
#define OS_SERVICE_DEBUG(format, ...) printf (format, ##__VA_ARGS__)
#else
#define OS_DEBUG(format, ...)
#define OS_SHELL_DEBUG(format, ...)
#define OS_SERVICE_DEBUG(format, ...)
#endif




#endif /* OS_PLATFORM_H_ */
